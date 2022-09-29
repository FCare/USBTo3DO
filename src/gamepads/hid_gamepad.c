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

#include "dragonrise.h"
#include "wiiadapter.h"
#include "retroBit.h"
#include "8bitdo.h"
#include "saturnAdapter.h"
#include "psClassic.h"
#include "thrustmaster.h"
#include "logitech.h"
#include "ps4.h"

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

static mapping_3do *currentMapping = NULL;

#define NB_GAMEPAD_SUPPORTED 16
#define NB_GAMEPAD_IN_LIST 16
static mapping_3do map[NB_GAMEPAD_IN_LIST] = {
  {0x0079, 0x0011, map_dragonRise, NULL, NULL}, //0079:0011 DragonRise Inc. Gamepad
  {0x0f0d, 0x00c1, map_retroBit, NULL, NULL}, //USB Gamepad Manufacturer: SWITCH CO.,LTD. SerialNumber: GH-SP-5027-1 H2
  {0x1d79, 0x0301, map_wii_classic_adapter, NULL, NULL}, //1d79:0301 Dell Dell USB Keyboard Hub //REQUIRE MULTI CONTROLLER SUPPORT //
  {0x0e8f, 0x3010, map_saturn_adapter, NULL, NULL}, //0e8f:3010 GreenAsia Inc. Dell USB Keyboard Hub
  {0x054c, 0x0cda, map_ps_classic, NULL, NULL}, // 054c:0cda Sony Corp. PlayStation Classic controller
  {0x044f, 0xb108, map_hotas_x_pc, NULL, NULL}, //044f:b108 ThrustMaster, Inc. T-Flight Hotas X Flight Stick
  {0x044f, 0xb109, map_hotas_x_ps3, NULL, NULL}, //044f:b109 ThrustMaster, Inc. T.Flight Hotas PS3
  {0x044f, 0xb66a, map_t80_pc, NULL, NULL}, //044f:b66a ThrustMaster, Inc. Thrustmaster T80
  {0x054c, 0x09cc, map_ds4, NULL, NULL}, //Dualshock4
  {0x054c, 0x05c4, map_ds4, NULL, NULL}, //Dualshock4
  {0x0f0d, 0x005e, map_ds4, NULL, NULL}, // Hori FC4
  {0x0f0d, 0x00ee, map_ds4, NULL, NULL}, // Hori PS4 Mini (PS4-099U)
  {0x1f4f, 0x1002, map_ds4, NULL, NULL},
  {0x2dc8, 0x5006, map_8bitDo_M30, NULL, NULL}, //8bitDo  M30 controler in dinput mode (B pressed at power up)
  {0x057e, 0x2009, map_8bitDo_SN30Pro, NULL, NULL}, //8bitDo  SN30 controler is seen as a nintendo switch controller
  {0x046d, 0xc215, map_logitech_extreme_pro, NULL, NULL}, // 046d:c215 Logitech, Inc. Extreme 3D Pro
};

static inline bool is_supported_controller(uint8_t dev_addr)
{
  uint16_t vid, pid;
  currentMapping = NULL;
  tuh_vid_pid_get(dev_addr, &vid, &pid);
  for (int i = 0; i<NB_GAMEPAD_SUPPORTED; i++) {
    TU_LOG1("Compare %4x to %4x and %4x to %4x\n", vid, map[i].vid, pid, map[i].pid);
    if ((vid == map[i].vid) && (pid == map[i].pid)) {
      currentMapping = &map[i];
      return true;
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
  tuh_vid_pid_get(dev_addr, &vid, &pid);

  TU_LOG1("HID device address = %d, instance = %d is mounted\r\n", dev_addr, instance);
  TU_LOG1("VID = %04x, PID = %04x\r\n", vid, pid);

  // Sony DualShock 4 [CUH-ZCT2x]
  if (is_supported_controller(dev_addr))
  {
    // request to receive report
    // tuh_hid_report_received_cb() will be invoked when report is available
    if ( !tuh_hid_receive_report(dev_addr, instance) )
    {
      TU_LOG1("Error: cannot request to receive report\r\n");
    }
  }
}

// Invoked when device with hid interface is un-mounted
void tuh_hid_umount_cb(uint8_t dev_addr, uint8_t instance)
{
  TU_LOG1("HID device address = %d, instance = %d is unmounted\r\n", dev_addr, instance);

}

void process_hid(uint8_t const* report, int8_t dev_addr, uint8_t instance, uint16_t len) {
  uint8_t const report_id = report[0];
  //Device like wii adapter are sending multiple report since it is a hub.
  // So needs to add multiple controller support for this case...
  // all buttons state is stored in ID 1
  if (currentMapping != NULL)
  {
    uint8_t id;
    void *newReport = NULL;
    controler_type type;
    if (currentMapping->mapper(report, len, dev_addr, instance, &id, &type, &newReport)) {
      if (newReport != NULL) {
        if (type == JOYPAD) {
          _3do_joypad_report padReport = *((_3do_joypad_report*)newReport);
          TU_LOG1("New Joypad report (down %d up %d right %d left %d A %d B %d C %d P %d X %d R %d L %d)\n",
          padReport.down, padReport.up, padReport.right, padReport.left, padReport.A, padReport.B, padReport.C,
          padReport.P, padReport.X, padReport.R, padReport.L );
          update_3do_joypad(padReport, id);
        }
      }
      if (type == JOYSTICK) {
        TU_LOG1("HID joystick Report %02x%02x%02x%02x%02x%02x%02x%02x%02x\n", ((uint8_t*)newReport)[0], ((uint8_t*)newReport)[1],((uint8_t*)newReport)[2], ((uint8_t*)newReport)[3], ((uint8_t*)newReport)[4], ((uint8_t*)newReport)[5], ((uint8_t*)newReport)[6], ((uint8_t*)newReport)[7], ((uint8_t*)newReport)[8]);
        update_3do_joystick(*((_3do_joystick_report*)newReport), id);
      }
    }
    if(newReport != NULL) free(newReport);
  }
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