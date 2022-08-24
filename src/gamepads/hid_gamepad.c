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
#include "saturnAdapter.h"
#include "psClassic.h"

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

// Sony DS4 report layout detail https://www.psdevwiki.com/ps4/DS4-USB
typedef struct TU_ATTR_PACKED
{
  uint8_t x, y, z, rz; // joystick

  struct {
    uint8_t dpad     : 4; // (hat format, 0x08 is released, 0=N, 1=NE, 2=E, 3=SE, 4=S, 5=SW, 6=W, 7=NW)
    uint8_t square   : 1; // west
    uint8_t cross    : 1; // south
    uint8_t circle   : 1; // east
    uint8_t triangle : 1; // north
  };

  struct {
    uint8_t l1     : 1;
    uint8_t r1     : 1;
    uint8_t l2     : 1;
    uint8_t r2     : 1;
    uint8_t share  : 1;
    uint8_t option : 1;
    uint8_t l3     : 1;
    uint8_t r3     : 1;
  };

  struct {
    uint8_t ps      : 1; // playstation button
    uint8_t tpad    : 1; // track pad click
    uint8_t counter : 6; // +1 each report
  };

  // comment out since not used by this example
  // uint8_t l2_trigger; // 0 released, 0xff fully pressed
  // uint8_t r2_trigger; // as above

  //  uint16_t timestamp;
  //  uint8_t  battery;
  //
  //  int16_t gyro[3];  // x, y, z;
  //  int16_t accel[3]; // x, y, z

  // there is still lots more info

} sony_ds4_report_t;

static mapping_3do *currentMapping = NULL;

#define NB_GAMEPAD_SUPPORTED 5
#define NB_GAMEPAD_IN_LIST 5
static mapping_3do map[NB_GAMEPAD_IN_LIST] = {
  {0x0079, 0x0011, map_dragonRise}, //0079:0011 DragonRise Inc. Gamepad

  //NOT SUPPORTED YET
  {0x0f0d, 0x00c1, map_retroBit, NULL}, //USB Gamepad Manufacturer: SWITCH CO.,LTD. SerialNumber: GH-SP-5027-1 H2
  {0x1d79, 0x0301, map_wii_classic_adapter, NULL}, //1d79:0301 Dell Dell USB Keyboard Hub //REQUIRE MULTI CONTROLLER SUPPORT //
  {0x0e8f, 0x3010, map_saturn_adapter, NULL}, //0e8f:3010 GreenAsia Inc. Dell USB Keyboard Hub
  {0x054c, 0x0cda, map_ps_classic, NULL}, // 054c:0cda Sony Corp. PlayStation Classic controller
};

// check if device is Sony DualShock 4
static inline bool is_sony_ds4(uint8_t dev_addr)
{
  return false; //not tested based on Tiny USB example. Need to get a DS4 controler to implement
  uint16_t vid, pid;
  tuh_vid_pid_get(dev_addr, &vid, &pid);

  return ( (vid == 0x054c && (pid == 0x09cc || pid == 0x05c4)) // Sony DualShock4
           || (vid == 0x0f0d && pid == 0x005e)                 // Hori FC4
           || (vid == 0x0f0d && pid == 0x00ee)                 // Hori PS4 Mini (PS4-099U)
           || (vid == 0x1f4f && pid == 0x1002)                 // ASW GG xrd controller
         );
}

static inline bool is_supported_controller(uint8_t dev_addr)
{
  uint16_t vid, pid;
  currentMapping = false;
  tuh_vid_pid_get(dev_addr, &vid, &pid);
  for (int i = 0; i<NB_GAMEPAD_SUPPORTED; i++) {
    TU_LOG1("Comapre %x to %x and %x to %x\n", vid, map[i].vid, pid, map[i].pid);
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
  if ( is_sony_ds4(dev_addr) || is_supported_controller(dev_addr))
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

// check if different than 2
bool diff_than_2(uint8_t x, uint8_t y)
{
  return (x - y > 2) || (y - x > 2);
}

// check if 2 reports are different enough
bool diff_report(sony_ds4_report_t const* rpt1, sony_ds4_report_t const* rpt2)
{
  bool result;

  // x, y, z, rz must different than 2 to be counted
  result = diff_than_2(rpt1->x, rpt2->x) || diff_than_2(rpt1->y , rpt2->y ) ||
           diff_than_2(rpt1->z, rpt2->z) || diff_than_2(rpt1->rz, rpt2->rz);

  // check the reset with mem compare
  result |= memcmp(&rpt1->rz + 1, &rpt2->rz + 1, sizeof(sony_ds4_report_t)-4);

  return result;
}

void process_sony_ds4(uint8_t const* report, uint16_t len)
{
  const char* dpad_str[] = { "N", "NE", "E", "SE", "S", "SW", "W", "NW", "none" };

  // previous report used to compare for changes
  static sony_ds4_report_t prev_report = { 0 };

  uint8_t const report_id = report[0];
  report++;
  len--;
  // all buttons state is stored in ID 1
  if (report_id == 1)
  {
    sony_ds4_report_t ds4_report;
    memcpy(&ds4_report, report, sizeof(ds4_report));

    // counter is +1, assign to make it easier to compare 2 report
    prev_report.counter = ds4_report.counter;

    // only print if changes since it is polled ~ 5ms
    // Since count+1 after each report and  x, y, z, rz fluctuate within 1 or 2
    // We need more than memcmp to check if report is different enough
    if ( diff_report(&prev_report, &ds4_report) )
    {
      TU_LOG1("(x, y, z, rz, dpad) = (%x, %x, %, %x, %x)\r\n", ds4_report.x, ds4_report.y, ds4_report.z, ds4_report.rz, ds4_report.dpad);
      TU_LOG1("DPad = %s ", dpad_str[ds4_report.dpad]);

      if (ds4_report.square   ) TU_LOG1("Square ");
      if (ds4_report.cross    ) TU_LOG1("Cross ");
      if (ds4_report.circle   ) TU_LOG1("Circle ");
      if (ds4_report.triangle ) TU_LOG1("Triangle ");

      if (ds4_report.l1       ) TU_LOG1("L1 ");
      if (ds4_report.r1       ) TU_LOG1("R1 ");
      if (ds4_report.l2       ) TU_LOG1("L2 ");
      if (ds4_report.r2       ) TU_LOG1("R2 ");

      if (ds4_report.share    ) TU_LOG1("Share ");
      if (ds4_report.option   ) TU_LOG1("Option ");
      if (ds4_report.l3       ) TU_LOG1("L3 ");
      if (ds4_report.r3       ) TU_LOG1("R3 ");

      if (ds4_report.ps       ) TU_LOG1("PS ");
      if (ds4_report.tpad     ) TU_LOG1("TPad ");

      TU_LOG1("\r\n");
    }

    prev_report = ds4_report;
  }
}

void process_hid(uint8_t const* report, int8_t dev_addr, uint8_t instance, uint16_t len) {
  uint8_t const report_id = report[0];
  //Device like wii adapter are sending multiple report since it is a hub.
  // So needs to add multiple controller support for this case...
  // all buttons state is stored in ID 1
  if (currentMapping != NULL)
  {
    hid_report_t hid_report;
    memcpy(&hid_report, report, sizeof(hid_report));
    uint8_t id;
    void *newReport = NULL;
    controler_type type;
    TU_LOG1("New report\n");
    if (currentMapping->mapper(&hid_report, len, dev_addr, instance, &id, &type, &newReport)) {
      if (newReport != NULL) {
        if (type == JOYPAD) {
          TU_LOG1("Update joypad %p %p\n", newReport, &newReport);
          update_3do_joypad(*((_3do_joypad_report*)newReport), id);
        }
      }
      if (type == JOYSTICK) {
        printf("HID joystick Report %02x%02x%02x%02x%02x%02x%02x%02x%02x\n", ((uint8_t*)newReport)[0], ((uint8_t*)newReport)[1],((uint8_t*)newReport)[2], ((uint8_t*)newReport)[3], ((uint8_t*)newReport)[4], ((uint8_t*)newReport)[5], ((uint8_t*)newReport)[6], ((uint8_t*)newReport)[7], ((uint8_t*)newReport)[8]);
        update_3do_joystick(*((_3do_joystick_report*)newReport), id);
      }
    }
    if(newReport != NULL) free(newReport);
  }
}

// Invoked when received report from device via interrupt endpoint
void tuh_hid_report_received_cb(uint8_t dev_addr, uint8_t instance, uint8_t const* report, uint16_t len)
{
  if ( is_sony_ds4(dev_addr) )
  {
    process_sony_ds4(report, len);
  } else
    process_hid(report, dev_addr, instance, len);


  // continue to request to receive report
  if ( !tuh_hid_receive_report(dev_addr, instance) )
  {
    TU_LOG1("Error: cannot request to receive report\r\n");
  }
}