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

#include "3DO.h"
#include "8bitdo.h"

//Code is made for only one USB port. Will not work in case of HUB plugged

 // 046d:c21d Logitech, Inc. F310 Gamepad [XInput Mode]


static mapping_3do *currentMapping = NULL;

#define NB_GAMEPAD_SUPPORTED 2
static mapping_3do map[NB_GAMEPAD_SUPPORTED] = {
  //045e:028e 8bitDo - M30 seen as Xbox360 controller
  {0x045e, 0x028e, map_8bitDo},
  {0x45e, 0x2a9, map_8bitDo}
};


static inline bool is_xbox360_controller(uint8_t dev_addr)
{
  uint16_t vid, pid;
  tuh_vid_pid_get(dev_addr, &vid, &pid);
  TU_LOG1("Look for vid 0x%x, pid 0x%x\n", vid, pid);
  for (int i = 0; i < NB_GAMEPAD_SUPPORTED; i++) {
    if ((vid == map[i].vid) && (pid = map[i].pid)) return true;
  }
  return false;
}

static xbox360_report handle_xbox360_report(uint8_t const* report, uint16_t len) {
  xbox360_report status;

  status.ABS_X = *((int16_t*) (report + 12));
  status.ABS_Y = *((int16_t*) (report + 14));
  status.ABS_RX = *((int16_t*) (report + 16));
  status.ABS_RY = *((int16_t*) (report + 18));

  status.BTN_TL2 = report[10];
  status.BTN_TR2 = report[11];
  status.ABS_Z = report[10];
  status.ABS_RZ = report[11];

  status.BTN_TRIGGER_HAPPY1 = (report[2]>>2) & 0x1;
  status.BTN_TRIGGER_HAPPY2 = (report[2]>>3) & 0x1;
  status.BTN_TRIGGER_HAPPY3 = (report[2]>>0) & 0x1;
  status.BTN_TRIGGER_HAPPY4 = (report[2]>>1) & 0x1;
  status.ABS_HAT0X = !!(report[2] & 0x08) - !!(report[2] & 0x04);
  status.ABS_HAT0Y = !!(report[2] & 0x02) - !!(report[2] & 0x01);

  status.BTN_START = (report[2]>>4) & 0x1;
  status.BTN_SELECT = (report[2]>>5) & 0x1;
  status.BTN_THUMBL = (report[2]>>6) & 0x1;
  status.BTN_THUMBR = (report[2]>>7) & 0x1;

  status.BTN_A = report[4];
  status.BTN_B = report[5];
  status.BTN_X = report[6];
  status.BTN_Y = report[7];

  status.BTN_C = report[8];
  status.BTN_Z = report[9];

  status.BTN_TL = (report[3]>>0) & 0x1;
  status.BTN_TR = (report[3]>>1) & 0x1;
  status.BTN_MODE = (report[3]>>2) & 0x1;
  status.BTN_SOUTH = (report[3]>>4) & 0x1;
  status.BTN_EAST = (report[3]>>5) & 0x1;
  status.BTN_NORTH = (report[3]>>6) & 0x1;
  status.BTN_WEST = (report[3]>>7) & 0x1;

  return status;
}

void tuh_vendor_mount_cb(uint8_t dev_addr, uint8_t instance, uint8_t const* desc_report, uint16_t desc_len)
{
  _3do_report init = new3doPadReport();
  TU_LOG1("VENDOR device address = %d, instance = %d is mounted\r\n", dev_addr, instance);
  if ( is_xbox360_controller(dev_addr) ) {
    uint16_t vid, pid;
    tuh_vid_pid_get(dev_addr, &vid, &pid);
    for (int i = 0; i<NB_GAMEPAD_SUPPORTED; i++) {
      if ((vid == map[i].vid) && (pid == map[i].pid)) {
        currentMapping = &map[i];
        break;
      }
    }
    TU_LOG1("Xbox360 compatible detected\r\n");
  }
  if ( !tuh_vendor_receive_report(dev_addr, instance) )
  {
    TU_LOG1("Error: cannot request to receive report\r\n");
  }
  update_3do_status(init, instance); //Send empty report to detect gamepad
}

void tuh_vendor_umount_cb(uint8_t dev_addr, uint8_t instance)
{
}

void tuh_vendor_report_received_cb(uint8_t dev_addr, uint8_t instance, uint8_t const* report, uint16_t len)
{
TU_LOG1("Report Received :!\r\n");
for (int i=0; i< len; i++) TU_LOG1("0x%x ", report[i]);
TU_LOG1("\r\n");

uint16_t vid, pid;
tuh_vid_pid_get(dev_addr, &vid, &pid);
if (currentMapping == NULL) return;
if ((currentMapping->vid == vid) &&  (currentMapping->pid == pid)) {
  xbox360_report xbox360_report = handle_xbox360_report(report,len);
  uint8_t id;
  _3do_report newreport = currentMapping->mapper(&xbox360_report, instance, &id);
  update_3do_status(newreport, id);
}


  // continue to request to receive report
  if ( !tuh_vendor_receive_report(dev_addr, instance) )
  {
    printf("Error: cannot request to receive report\r\n");
  }
}
