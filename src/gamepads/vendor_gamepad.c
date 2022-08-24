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
#include "8bitdo.h"
#include "xbox360w.h"

//Code is made for only one USB port. Will not work in case of HUB plugged

 // 046d:c21d Logitech, Inc. F310 Gamepad [XInput Mode]


static mapping_3do *currentMapping = NULL;

#define NB_GAMEPAD_SUPPORTED 2
static mapping_3do map[NB_GAMEPAD_SUPPORTED] = {
  {0x045e, 0x028e, map_8bitDo, NULL, NULL}, //045e:028e 8bitDo - M30 seen as Xbox360 controller
  {0x045e, 0x2a9, map_xbox360w, mount_xbox360w, led_xbox360w}, //Xbox 360 wireless receiver
  // {0x046d, 0xc21d, map_8bitDo} //Logitech F310
};

void vendor_gamepad_tick(void) {
  if (currentMapping != NULL) {
    if (currentMapping->tick != NULL)
      currentMapping->tick();
  }
}

void tuh_vendor_mount_cb(uint8_t dev_addr, uint8_t instance, uint8_t const* desc_report, uint16_t desc_len)
{
  _3do_joypad_report init = new3doPadReport();
  TU_LOG1("VENDOR device address = %d, instance = %d is mounted %x %x %d \r\n", dev_addr, instance, desc_report[0], desc_report[1], desc_len);
  uint16_t vid, pid;
  bool newControllerAdded = true;
  tuh_vid_pid_get(dev_addr, &vid, &pid);
  for (int i = 0; i<NB_GAMEPAD_SUPPORTED; i++) {
    if ((vid == map[i].vid) && (pid == map[i].pid)) {
      currentMapping = &map[i];
      TU_LOG1("Xbox360 compatible detected\r\n");
      break;
    }
  }

  if (currentMapping == NULL) return;

  if (currentMapping->mount != NULL) {
    newControllerAdded = currentMapping->mount(dev_addr, instance);
  }

  if ( !tuh_vendor_receive_report(dev_addr, instance) )
  {
    TU_LOG1("Error: cannot request to receive report\r\n");
  }
  if (newControllerAdded) update_3do_joypad(init, instance); //Send empty report to detect gamepad
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
  uint8_t id;
  void *newReport = NULL;
  controler_type type;
  if (currentMapping->mapper(report, len, dev_addr, instance, &id, &type, &newReport)) {
    if (type == JOYPAD) {
      update_3do_joypad(*((_3do_joypad_report*)newReport), id);
    }
    if (type == JOYSTICK) {
      update_3do_joystick(*((_3do_joystick_report*)newReport), id);
    }
  } else {
    update_3do_joypad(new3doPadReport(), id);
  }
  if(newReport != NULL) free(newReport);
}


  // continue to request to receive report
  if ( !tuh_vendor_receive_report(dev_addr, instance) )
  {
    TU_LOG1("Error: cannot request to receive report\r\n");
  }
}
