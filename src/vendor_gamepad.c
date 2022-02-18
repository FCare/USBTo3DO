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

#include "3do_interface.h"

//Code is made for only one USB port. Will not work in case of HUB plugged


 // 046d:c21d Logitech, Inc. F310 Gamepad [XInput Mode]
 // 045e:028e Microsoft Corp. Xbox360 Controller => 8bitdo M30 seen as xBox controller

 typedef struct
 {
  int16_t ABS_X;
  int16_t ABS_Y;
  int16_t ABS_RX;
  int16_t ABS_RY;
  uint8_t BTN_TL2;
  uint8_t BTN_TR2;
  int8_t ABS_Z;
  int8_t ABS_RZ;
  uint8_t BTN_TRIGGER_HAPPY1;
  uint8_t BTN_TRIGGER_HAPPY2;
  uint8_t BTN_TRIGGER_HAPPY3;
  uint8_t BTN_TRIGGER_HAPPY4;
  uint8_t ABS_HAT0X;
  uint8_t ABS_HAT0Y;
  uint8_t BTN_START;
  uint8_t BTN_SELECT;
  uint8_t BTN_THUMBL;
  uint8_t BTN_THUMBR;
  uint8_t BTN_A;
  uint8_t BTN_B;
  uint8_t BTN_X;

  uint8_t BTN_MODE;
  uint8_t BTN_NORTH;
  uint8_t BTN_SOUTH;
  uint8_t BTN_WEST;
  uint8_t BTN_EAST;
  uint8_t BTN_TR;
  uint8_t BTN_TL;

  int8_t BTN_Y;
  int8_t BTN_C;
  int8_t BTN_Z;
} xbox360_report;

static _3do_report map_8bitDo(void* report_p);

static mapping_3do *currentMapping = NULL;

#define NB_GAMEPAD_SUPPORTED 1
static mapping_3do map[NB_GAMEPAD_SUPPORTED] = {
  //045e:028e 8bitDo - M30 seen as Xbox360 controller
  {0x045e, 0x028e, map_8bitDo}
};


static _3do_report map_8bitDo(void *report_p) {
  xbox360_report* report = (xbox360_report *)report_p;
  _3do_report result;

  result.id = 0b001;
  result.tail = 0b00;
  result.up = (report->BTN_Z > 0)?1:0;
  result.down = (report->BTN_Z < 0)?1:0;
  result.left = (report->BTN_Y < 0)?1:0;
  result.right = (report->BTN_Y > 0)?1:0;
  result.X = report->BTN_START;
  result.P = report->BTN_SELECT | report->BTN_SELECT | report->BTN_WEST;
  result.A = report->BTN_SOUTH;
  result.B = report->BTN_EAST;
  result.C = (report->BTN_B != 0)?1:0;
  result.L = report->BTN_NORTH | report->BTN_TL;
  result.R = ((report->BTN_A != 0)?1:0) | report->BTN_TR;
  return result;
}

static inline bool is_xbox360_controller(uint8_t dev_addr)
{
  uint16_t vid, pid;
  tuh_vid_pid_get(dev_addr, &vid, &pid);
  return ( (vid == 0x046d && pid == 0xc21d) //046d:c21d Logitech, Inc. F310 Gamepad
          ||  (vid == 0x045e && pid == 0x028e)
        );
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
  printf("VENDOR device address = %d, instance = %d is mounted\r\n", dev_addr, instance);
  if ( is_xbox360_controller(dev_addr) ) {
    uint16_t vid, pid;
    tuh_vid_pid_get(dev_addr, &vid, &pid);
    for (int i = 0; i<NB_GAMEPAD_SUPPORTED; i++) {
      if ((vid == map[i].vid) && (pid == map[i].pid)) {
        currentMapping = &map[i];
        break;
      }
    }
    printf("Xbox360 compatible detected\r\n");
  }
  if ( !tuh_vendor_receive_report(dev_addr, instance) )
  {
    printf("Error: cannot request to receive report\r\n");
  }
}

void tuh_vendor_umount_cb(uint8_t dev_addr, uint8_t instance)
{
}

void tuh_vendor_report_received_cb(uint8_t dev_addr, uint8_t instance, uint8_t const* report, uint16_t len)
{
LOG_3DO("Report Received :!\r\n");
for (int i=0; i< len; i++) LOG_3DO("0x%x ", report[i]);
LOG_3DO("\r\n");

uint16_t vid, pid;
tuh_vid_pid_get(dev_addr, &vid, &pid);
if (currentMapping == NULL) return;
if ((currentMapping->vid == vid) &&  (currentMapping->pid == pid)) {
  xbox360_report xbox360_report = handle_xbox360_report(report,len);
  update_3do_status(currentMapping->mapper(&xbox360_report));
}


  // continue to request to receive report
  if ( !tuh_vendor_receive_report(dev_addr, instance) )
  {
    printf("Error: cannot request to receive report\r\n");
  }
}
