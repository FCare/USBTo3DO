/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021, Ha Thach (tinyusb.org)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include "bsp/board.h"
#include "tusb.h"

#include <stdlib.h>

#include "3DO.h"
#include "hid_gamepad.h"
#include "hid_parser.h"

#include "dragonrise.h"
#include "wiiadapter.h"
#include "retroBit.h"
#include "8bitdo.h"
#include "saturnAdapter.h"
#include "psClassic.h"
#include "thrustmaster.h"
#include "logitech.h"
#include "ps4.h"
#include "sixaxis.h"

//#define _DEBUG_MAPPER_

/* From https://www.kernel.org/doc/html/latest/input/gamepad.html
          ____________________________              __
         / [__ZL__]          [__ZR__] \               |
        / [__ TL __]        [__ TR __] \              | Front Triggers
     __/________________________________\__         __|
    /                                  _   \          |
   /      /\           __             (N)   \         |
  /       ||      __  |MO|  __     _       _ \        | Main Pad
 |    <===DP===> |SE|      |ST|   (W) -|- (E) |       |
  \       ||    ___          ___       _     /        |
  /\      \/   /   \        /   \     (S)   /\      __|
 /  \________ | LS  | ____ |  RS | ________/  \       |
|         /  \ \___/ /    \ \___/ /  \         |      | Control Sticks
|        /    \_____/      \_____/    \        |    __|
|       /                              \       |
 \_____/                                \_____/

     |________|______|    |______|___________|
       D-Pad    Left       Right   Action Pad
               Stick       Stick

                 |_____________|
                    Menu Pad

  Most gamepads have the following features:
  - Action-Pad 4 buttons in diamonds-shape (on the right side) NORTH, SOUTH, WEST and EAST.
  - D-Pad (Direction-pad) 4 buttons (on the left side) that point up, down, left and right.
  - Menu-Pad Different constellations, but most-times 2 buttons: SELECT - START.
  - Analog-Sticks provide freely moveable sticks to control directions, Analog-sticks may also
  provide a digital button if you press them.
  - Triggers are located on the upper-side of the pad in vertical direction. The upper buttons
  are normally named Left- and Right-Triggers, the lower buttons Z-Left and Z-Right.
  - Rumble Many devices provide force-feedback features. But are mostly just simple rumble motors.
 */

 // 046d:c21d Logitech, Inc. F310 Gamepad [XInput Mode]
 // 045e:028e Microsoft Corp. Xbox360 Controller => 8bitdo M30 seen as xBox controller

static mapping_hid_3do *currentMapping = NULL;
static hid_controller currentController[CFG_TUH_DEVICE_MAX] = {0};

uint8_t *last_report[CFG_TUH_DEVICE_MAX] = {NULL};
int16_t last_len[CFG_TUH_DEVICE_MAX] = {-1};

#define NB_GAMEPAD_SUPPORTED 17
#define NB_GAMEPAD_IN_LIST 17
static mapping_hid_3do map[NB_GAMEPAD_IN_LIST] = {
  {0x0079, 0x0011, " SWITCH CO.,LTD.", map_retroBit, NULL, NULL}, //0079:0011 DragonRise Inc. Gamepad
  {0x0079, 0x0011, NULL, map_dragonRise, NULL, NULL}, //0079:0011 DragonRise Inc. Gamepad
  {0x0e8f, 0x3010, NULL, map_saturn_adapter, NULL, NULL}, //0e8f:3010 GreenAsia Inc. Dell USB Keyboard Hub
  {0x1d79, 0x0301, NULL, map_wii_classic_adapter, NULL, NULL}, //1d79:0301 Dell Dell USB Keyboard Hub //REQUIRE MULTI CONTROLLER SUPPORT //
  {0x046d, 0xc215, NULL, map_logitech_extreme_pro, NULL, NULL}, // 046d:c215 Logitech, Inc. Extreme 3D Pro
  {0x044f, 0xb106, NULL, map_hotas_x_flight_pc, NULL, NULL}, //044f:b106 ThrustMaster, Inc. T.Flight Stick X
  {0x044f, 0xb108, NULL, map_hotas_x_pc, NULL, NULL}, //044f:b108 ThrustMaster, Inc. T-Flight Hotas X Flight Stick
  {0x044f, 0xb109, NULL, map_hotas_x_ps3, NULL, NULL}, //044f:b109 ThrustMaster, Inc. T.Flight Hotas PS3
  {0x044f, 0xb66a, NULL, map_t80_pc, NULL, NULL}, //044f:b66a ThrustMaster, Inc. Thrustmaster T80
  {0x2dc8, 0x5006, NULL, map_8bitDo_M30, NULL, NULL}, //8bitDo  M30 controler in dinput mode (B pressed at power up)
  {0x054c, 0x0cda, NULL, map_ps_classic, NULL, NULL}, // 054c:0cda Sony Corp. PlayStation Classic controller
  {0x054c, 0x09cc, NULL, map_ds4, NULL, NULL}, //Dualshock4
  {0x054c, 0x05c4, NULL, map_ds4, NULL, NULL}, //Dualshock4
  {0x0f0d, 0x005e, NULL, map_ds4, NULL, NULL}, // Hori FC4
  {0x0f0d, 0x00ee, NULL, map_ds4, NULL, NULL}, // Hori PS4 Mini (PS4-099U)
  {0x1f4f, 0x1002, NULL, map_ds4, NULL, NULL},
  {0x054c, 0x0268, NULL, map_sixaxis, mount_sixaxis, NULL},
  // {0x057e, 0x2009, NULL, map_8bitDo_SN30Pro, NULL, NULL}, //8bitDo  SN30 controler is seen as a nintendo switch controller
};

static inline bool is_supported_controller(uint8_t dev_addr)
{
  uint16_t vid, pid;
  uint8_t manuf[255];
  uint8_t prod[255];

  currentMapping = NULL;
  tuh_vid_pid_get(dev_addr, &vid, &pid);
  tuh_product_get(dev_addr, prod);
  tuh_manufacturer_get(dev_addr, manuf);

  for (int i = 0; i<NB_GAMEPAD_SUPPORTED; i++) {
    TU_LOG1("Compare 0x%04x to 0x%04x and 0x%04x to 0x%04x\n", vid, map[i].vid, pid, map[i].pid);
    if ((vid == map[i].vid) && (pid == map[i].pid)) {
      if (map[i].manuf != NULL) {
        TU_LOG1("Compare %s with %s\n", map[i].manuf,manuf);
        if (strcmp(map[i].manuf, manuf) == 0) {
          currentMapping = &map[i];
          return true;
        }
      } else {
        currentMapping = &map[i];
        return true;
      }
    }
  }
  return false;
}

//--------------------------------------------------------------------+
// MACRO TYPEDEF CONSTANT ENUM DECLARATION
//--------------------------------------------------------------------+

void hid_app_task(void)
{
  // nothing to do
}

void hid_gamepad_tick(void) {
  if (currentMapping != NULL) {
    if (currentMapping->tick != NULL)
      currentMapping->tick();
  }
}

static bool is3DOCompatible(hid_controller *ctrl) {
  bool hasaCompatibleHID = false;
  for (int hid =0; hid<ctrl->nb_HID; hid++) {
    hid_mapping *mapping = &ctrl->mapping[hid];
    ctrl->axis_status[hid] = 0;
    uint8_t nbAxis = 0;
    ctrl->hasHat[hid] = false;
    ctrl->nbButtons[hid] = 0;
    ctrl->isCompatible[hid] = false;
    for (int i=0; i<mapping->nb_events; i++) {
      hid_event *ev = &mapping->events[i];
      if ((ev->key >= HID_X) && (ev->key < HID_HAT_SWITCH))
      ctrl->axis_status[hid] |= 1<<(ev->key-HID_X);
      else if (ev->key == HID_HAT_SWITCH) {
        ctrl->hasHat[hid] = true;;
      } else ctrl->nbButtons[hid]++;
    }

    for (int i=0; i<8; i++) {
      nbAxis += (ctrl->axis_status[hid]>>i)&0x1;
    }
    TU_LOG1("HID %d Offset 0x%x NbButtons = %d, hasHat = %d, nbAxis = %d\n", hid, mapping->index, ctrl->nbButtons[hid], ctrl->hasHat[hid], nbAxis);
    if (ctrl->nbButtons[hid] < 11) {
      int missing_buttons = 11 - ctrl->nbButtons[hid];
      if (ctrl->axis_status[hid] & 0x3 == 0x3) ctrl->isCompatible[hid] = true;
      if (ctrl->hasHat[hid] && (ctrl->nbButtons[hid] >= 7)) ctrl->isCompatible[hid] = true;
    } else ctrl->isCompatible[hid] = true;
    hasaCompatibleHID |= ctrl->isCompatible[hid];
  }
  return hasaCompatibleHID;
}

static bool hasAJoystick(hid_controller *ctrl) {
  for (int i=0; i<ctrl->nb_HID; i++) {
    if (ctrl->mapping[i].type == TYPE_JOYSTICK) return true;
  }
  return false;
}

//--------------------------------------------------------------------+
// TinyUSB Callbacks
//--------------------------------------------------------------------+

// Invoked when device with hid interface is mounted
// Report descriptor is also available for use. tuh_hid_parse_report_descriptor()
// can be used to parse common/simple enough descriptor.
// Note: if report descriptor length > CFG_TUH_ENUMERATION_BUFSIZE, it will be skipped
// therefore report_desc = NULL, desc_len = 0
void tuh_hid_mount_cb(uint8_t dev_addr, uint8_t instance, uint8_t const* desc_report, uint16_t desc_len)
{
  uint16_t vid, pid;
  uint8_t manuf[255];
  uint8_t prod[255];
  int idx = 0;

  last_len[instance] = -1;
  if (last_report[instance] != NULL) free(last_report[instance]);
  last_report[instance] = NULL;

  tuh_vid_pid_get(dev_addr, &vid, &pid);
  tuh_product_get(dev_addr, prod);
  tuh_manufacturer_get(dev_addr, manuf);

  TU_LOG1("HID device address = %d, instance = %d is mounted\r\n", dev_addr, instance);
  TU_LOG1("VID = %04x, PID = %04x\r\n", vid, pid);
  TU_LOG1("Product = %s, Manufacturer = %s\r\n", prod, manuf);

  TU_LOG1("Descriptor length %d :", desc_len);
  TU_LOG1_MEM(desc_report, desc_len, 2);

  parse_hid_descriptor(desc_report, desc_len, &currentController[instance]);

  is_supported_controller(dev_addr);

  if (currentMapping != NULL)
    if (currentMapping->mount != NULL)
    currentMapping->mount(dev_addr, instance);

  if(is3DOCompatible(&currentController[instance])) {
    // request to receive report
    // tuh_hid_report_received_cb() will be invoked when report is available
    TU_LOG1("HID can be used with 3DO\n");
    if ( !tuh_hid_receive_report(dev_addr, instance) )
    {
      TU_LOG1("Error: cannot request to receive report\r\n");
    } else {
      TU_LOG1("Report requested\n");
    }
  } else {
    TU_LOG1("HID descriptor is incompatible with 3DO\n");
  }

}

// Invoked when device with hid interface is un-mounted
void tuh_hid_umount_cb(uint8_t dev_addr, uint8_t instance)
{
  TU_LOG1("HID device address = %d, instance = %d is unmounted\r\n", dev_addr, instance);
  last_len[instance] = -1;
  if (last_report[instance] != NULL) free(last_report[instance]);
  last_report[instance] = NULL;
  memset(&currentController[instance] ,0, sizeof(hid_controller));
}

static bool HIDMapper(uint8_t* report, uint8_t len, uint8_t instance, hid_controller *ctrl) {

  bool processReport = false;
  int8_t* last_report_to_test = NULL;
  hid_buttons last_buttons;
  uint8_t id = 0;

  if (last_len[instance] == -1) {
    last_report[instance] = (uint8_t*)calloc(ctrl->has_index?ctrl->nb_HID:1, len);
    last_len[instance] = len;
  }

  last_report_to_test = &last_report[instance][0];
  if (ctrl->has_index){
    id = report[0];
    ctrl->index = id-1;
    last_report_to_test = &last_report[instance][(ctrl->index)*len];
  }
  processReport |= (memcmp(report, last_report_to_test, len) != 0);


  if (!processReport) return false;
  if (!ctrl->isCompatible[ctrl->index]) return false;

  TU_LOG2("Got a report Controller[%d] %d\n", instance, id);
  TU_LOG2_MEM(report, len, 2);
  TU_LOG2_MEM(last_report_to_test, len, 2);


  memcpy(last_report_to_test, report, len);

  hid_mapping *mapping = &ctrl->mapping[ctrl->index];
  memcpy(&last_buttons, &ctrl->buttons[ctrl->index], sizeof(hid_buttons));
  memset(&ctrl->buttons[ctrl->index], 0, sizeof(hid_buttons));
  ctrl->buttons[ctrl->index].ABS_X = 0x80;
  ctrl->buttons[ctrl->index].ABS_Y = 0x80;
  ctrl->buttons[ctrl->index].ABS_Z = 0x80;
  ctrl->buttons[ctrl->index].ABS_RX = 0x80;
  ctrl->buttons[ctrl->index].ABS_RY = 0x80;
  ctrl->buttons[ctrl->index].ABS_RZ = 0x80;
  ctrl->buttons[ctrl->index].SLIDER = 0x80;
  ctrl->buttons[ctrl->index].DIAL = 0x80;
  ctrl->buttons[ctrl->index].WHEEL = 0x80;
  for (int i=0; i<mapping->nb_events; i++) {
    hid_event *ev = &mapping->events[i];
    uint32_t value = 0;
    uint32_t shifted_value = 0;
    for (int j=ev->begin; j<ev->end; j++) {
      uint8_t byte = report[j/8];
      uint8_t bit = (byte>>(j%8))&0x1;
      value |= (bit << (j - ev->begin));
    }
    // TU_LOG1("Event %d [%d,%d] got 0x%x => Key 0x%x %d\n", i, ev->begin, ev->end, value, ev->key, ev->shift);
    if (ev->end-ev->begin>=8)
      shifted_value = value >> ((ev->end-ev->begin)-8);
    else
      shifted_value = value << (8-(ev->end-ev->begin));
    switch(ev->key) {
      case HID_X:
        ctrl->buttons[ctrl->index].ABS_X = (uint8_t)(shifted_value);
        break;
      case HID_Y:
        ctrl->buttons[ctrl->index].ABS_Y = (uint8_t)(shifted_value);
        break;
      case HID_Z:
        ctrl->buttons[ctrl->index].ABS_Z = (uint8_t)(shifted_value);
        break;
      case HID_RX:
        ctrl->buttons[ctrl->index].ABS_RX = (uint8_t)(shifted_value);
        break;
      case HID_RY:
        ctrl->buttons[ctrl->index].ABS_RY = (uint8_t)(shifted_value);
        break;
      case HID_RZ:
        ctrl->buttons[ctrl->index].ABS_RZ = (uint8_t)(shifted_value);
        break;
      case HID_SLIDER:
        ctrl->buttons[ctrl->index].SLIDER = (uint8_t)(shifted_value);
        break;
      case HID_DIAL:
        ctrl->buttons[ctrl->index].DIAL = (uint8_t)(shifted_value);
        break;
      case HID_WHEEL:
        ctrl->buttons[ctrl->index].WHEEL = (uint8_t)(shifted_value);
        break;
      case HID_HAT_SWITCH:
        switch(value) {
          case 0:
            ctrl->buttons[ctrl->index].HAT_UP = 0b1;
          break;
          case 1:
            ctrl->buttons[ctrl->index].HAT_UP = 0b1;
            ctrl->buttons[ctrl->index].HAT_RIGHT = 0b1;
          break;
          case 2:
            ctrl->buttons[ctrl->index].HAT_RIGHT = 0b1;
          break;
          case 3:
            ctrl->buttons[ctrl->index].HAT_RIGHT = 0b1;
            ctrl->buttons[ctrl->index].HAT_DOWN = 0b1;
          break;
          case 4:
            ctrl->buttons[ctrl->index].HAT_DOWN = 0b1;
          break;
          case 5:
            ctrl->buttons[ctrl->index].HAT_DOWN = 0b1;
            ctrl->buttons[ctrl->index].HAT_LEFT = 0b1;
          break;
          case 6:
            ctrl->buttons[ctrl->index].HAT_LEFT = 0b1;
          break;
          case 7:
            ctrl->buttons[ctrl->index].HAT_LEFT = 0b1;
            ctrl->buttons[ctrl->index].HAT_UP = 0b1;
          break;
          default:
          break;
        }
        break;
      case 0:
        ctrl->buttons[ctrl->index].TRIGGER = (value & 0x1);
      break;
      case 1:
        ctrl->buttons[ctrl->index].THUMB = (value & 0x1);
      break;
      case 2:
        ctrl->buttons[ctrl->index].THUMB2 = (value & 0x1);
      break;
      case 3:
        ctrl->buttons[ctrl->index].TOP = (value & 0x1);
      break;
      case 4:
        ctrl->buttons[ctrl->index].TOP2 = (value & 0x1);
      break;
      case 5:
        ctrl->buttons[ctrl->index].PINKIE = (value & 0x1);
      break;
      case 6:
        ctrl->buttons[ctrl->index].BASE = (value & 0x1);
      break;
      case 7:
        ctrl->buttons[ctrl->index].BASE2 = (value & 0x1);
      break;
      case 8:
        ctrl->buttons[ctrl->index].BASE3 = (value & 0x1);
      break;
      case 9:
        ctrl->buttons[ctrl->index].BASE4 = (value & 0x1);
      break;
      case 10:
        ctrl->buttons[ctrl->index].BASE5 = (value & 0x1);
      break;
      case 11:
        ctrl->buttons[ctrl->index].BASE6 = (value & 0x1);
      break;
      case 12:
        ctrl->buttons[ctrl->index].BASE7 = (value & 0x1);
      break;
      case 13:
        ctrl->buttons[ctrl->index].BASE8 = (value & 0x1);
      break;
      case 14:
        ctrl->buttons[ctrl->index].BASE9 = (value & 0x1);
      break;
      case 15:
        ctrl->buttons[ctrl->index].BASE10 = (value & 0x1);
      break;
      default:
      break;
    }
  }

  if (memcmp(&last_buttons, &ctrl->buttons[ctrl->index], sizeof(hid_buttons)) == 0) return false;
  TU_LOG1("EV: ");
  for (int i=0; i<mapping->nb_events; i++) {
    hid_event *ev = &mapping->events[i];
    switch(ev->key) {
      case HID_X:
        TU_LOG1("X: %03d ", ctrl->buttons[ctrl->index].ABS_X);
        break;
      case HID_Y:
        TU_LOG1("Y: %03d ", ctrl->buttons[ctrl->index].ABS_Y);
        break;
      case HID_Z:
        TU_LOG1("Z: %03d ", ctrl->buttons[ctrl->index].ABS_Z);
        break;
      case HID_RX:
        TU_LOG1("RX: %03d ", ctrl->buttons[ctrl->index].ABS_RX);
        break;
      case HID_RY:
        TU_LOG1("RY: %03d ", ctrl->buttons[ctrl->index].ABS_RY);
        break;
      case HID_RZ:
        TU_LOG1("RZ: %03d ", ctrl->buttons[ctrl->index].ABS_RZ);
        break;
      case HID_SLIDER:
        TU_LOG1("Slider: %03d ", ctrl->buttons[ctrl->index].SLIDER);
        break;
      case HID_DIAL:
        TU_LOG1("Dial: %03d ", ctrl->buttons[ctrl->index].DIAL);
        break;
      case HID_WHEEL:
        TU_LOG1("Wheel: %03d ", ctrl->buttons[ctrl->index].WHEEL);
        break;
      case HID_HAT_SWITCH:
        if(ctrl->buttons[ctrl->index].HAT_UP) TU_LOG1("UP ");
        if(ctrl->buttons[ctrl->index].HAT_DOWN) TU_LOG1("DOWN ");
        if(ctrl->buttons[ctrl->index].HAT_RIGHT) TU_LOG1("RIGHT ");
        if(ctrl->buttons[ctrl->index].HAT_LEFT) TU_LOG1("LEFT ");
        break;
      case 0:
        if (ctrl->buttons[ctrl->index].TRIGGER) TU_LOG1("TRIGGER ");
      break;
      case 1:
        if (ctrl->buttons[ctrl->index].THUMB) TU_LOG1("THUMB ");
      break;
      case 2:
        if (ctrl->buttons[ctrl->index].THUMB2) TU_LOG1("THUMB2 ");
      break;
      case 3:
        if (ctrl->buttons[ctrl->index].TOP) TU_LOG1("TOP ");
      break;
      case 4:
        if (ctrl->buttons[ctrl->index].TOP2) TU_LOG1("TOP2 ");
      break;
      case 5:
        if (ctrl->buttons[ctrl->index].PINKIE) TU_LOG1("PINKIE ");
      break;
      case 6:
        if (ctrl->buttons[ctrl->index].BASE) TU_LOG1("BASE ");
      break;
      case 7:
        if (ctrl->buttons[ctrl->index].BASE2) TU_LOG1("BASE2 ");
      break;
      case 8:
        if (ctrl->buttons[ctrl->index].BASE3) TU_LOG1("BASE3 ");
      break;
      case 9:
        if (ctrl->buttons[ctrl->index].BASE4) TU_LOG1("BASE4 ");
      break;
      case 10:
        if (ctrl->buttons[ctrl->index].BASE5) TU_LOG1("BASE5 ");
      break;
      case 11:
        if (ctrl->buttons[ctrl->index].BASE6) TU_LOG1("BASE6 ");
      break;
      case 12:
        if (ctrl->buttons[ctrl->index].BASE7) TU_LOG1("BASE7 ");
      break;
      case 13:
        if (ctrl->buttons[ctrl->index].BASE8) TU_LOG1("BASE8 ");
      break;
      case 14:
        if (ctrl->buttons[ctrl->index].BASE9) TU_LOG1("BASE9 ");
      break;
      case 15:
        if (ctrl->buttons[ctrl->index].BASE10) TU_LOG1("BASE10 ");
      break;
      default:
      break;
    }
  }
  TU_LOG1("\n");
  return true;
}

static void default3DOMapper(uint8_t instance, uint8_t *id, controler_type *type, void **res, hid_controller *ctrl)
{
  //Default mapper only support joypad
  _3do_joypad_report *result = malloc(sizeof(_3do_joypad_report));
  *result = new3doPadReport();
  *type = JOYPAD;
  *id = ctrl->index + instance;
  hid_buttons *btn = & ctrl->buttons[ctrl->index];

  result->up = btn->ABS_Y <= 64;
  result->down = btn->ABS_Y >= 196;
  result->left = btn->ABS_X <= 64;
  result->right = btn->ABS_X >= 196;
  result->X = btn->TOP || btn->BASE5 || btn->BASE4;
  result->P = btn->BASE3 || btn->BASE6 || btn->BASE3;
  result->A = btn->TRIGGER;
  result->B = btn->THUMB;
  result->C = btn->THUMB2;
  result->L = btn->TOP2 || btn->BASE || (btn->ABS_RX >= 54);
  result->R = btn->PINKIE || btn->BASE2 || (btn->ABS_RX >= 64);

  if (ctrl->hasHat[ctrl->index]) {
    result->up |= btn->HAT_UP;
    result->down |= btn->HAT_DOWN;
    result->left |= btn->HAT_LEFT;
    result->right |= btn->HAT_RIGHT;
  }

  #ifdef _DEBUG_MAPPER_
    //used for mapping debug
    printf("(up, down, left, right) (%d %d %d %d) (X,P,A,B,C,L,R)(%d %d %d %d %d %d %d)\n",
          result->up, result->down, result->left, result->right, result->X, result->P, result->A, result->B, result->C, result->L, result->R);
  #endif

  *res = (void *)(result);

  return;
}

void process_hid(uint8_t const* report, int8_t dev_addr, uint8_t instance, uint16_t len) {
  uint8_t const report_id = report[0];
  //Device like wii adapter are sending multiple report since it is a hub.
  // So needs to add multiple controller support for this case...
  // all buttons state is stored in ID 1
  uint8_t id;
  void *newReport = NULL;
  controler_type type;
  bool ret = false;
  ret = HIDMapper(report, len, instance, &currentController[instance]);
  if (ret) {
    if (currentMapping != NULL)
      currentMapping->mapper(instance, &id, &type, &newReport, &currentController[instance]);
    else
      default3DOMapper(instance, &id, &type, &newReport, &currentController[instance]);
    if (newReport != NULL) {
      if (type == JOYPAD) {
        _3do_joypad_report padReport = *((_3do_joypad_report*)newReport);
        TU_LOG1("New Joypad report[%d] (down %d up %d right %d left %d A %d B %d C %d P %d X %d R %d L %d)\n",
        id, padReport.down, padReport.up, padReport.right, padReport.left, padReport.A, padReport.B, padReport.C,
        padReport.P, padReport.X, padReport.R, padReport.L );
        update_3do_joypad(padReport, id);
      }
      if (type == JOYSTICK) {
        _3do_joystick_report stickReport = *((_3do_joystick_report*)newReport);
        uint8_t x = stickReport.analog1;
        uint8_t y = (stickReport.analog2<<2)|(stickReport.analog3>>6);
        uint8_t trig = (stickReport.analog3<<4)|((stickReport.analog4>>4)&0xF);
        TU_LOG1("New Joystick report[%d] (axisX %d axisY %d Throttle %d down %d up %d right %d left %d A %d B %d C %d P %d X %d R %d L %d Fire %d)\n",
        id, x, y, trig,
        stickReport.down, stickReport.up, stickReport.right, stickReport.left, stickReport.A, stickReport.B, stickReport.C,
        stickReport.P, stickReport.X, stickReport.R, stickReport.L, stickReport.FIRE);
        update_3do_joystick(*((_3do_joystick_report*)newReport), id);
      }
    }
  }
  if(newReport != NULL) free(newReport);
}

// Invoked when received report from device via interrupt endpoint
void tuh_hid_report_received_cb(uint8_t dev_addr, uint8_t instance, uint8_t const* report, uint16_t len)
{

  process_hid(report, dev_addr, instance, len);
  // continue to request to receive report
  if ( !tuh_hid_receive_report(dev_addr, instance) )
  {
    TU_LOG1("Error: cannot request to receive report\r\n");
  }
}