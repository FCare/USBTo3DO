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

 // 046d:c21d Logitech, Inc. F310 Gamepad [XInput Mode]
 // 045e:028e Microsoft Corp. Xbox360 Controller => 8bitdo M30 seen as xBox controller


static inline bool is_Xinput_controller(uint8_t dev_addr)
{
  uint16_t vid, pid;
  tuh_vid_pid_get(dev_addr, &vid, &pid);
  return ( (vid == 0x046d && pid == 0xc21d) //046d:c21d Logitech, Inc. F310 Gamepad
          ||  (vid == 0x045e && pid == 0x028e) //045e:028e 8bitDo - M30 seen as Xbox360 controller
        );
}

void tuh_vendor_mount_cb(uint8_t dev_addr, uint8_t instance, uint8_t const* desc_report, uint16_t desc_len)
{
  printf("VENDOR device address = %d, instance = %d is mounted\r\n", dev_addr, instance);
  if ( is_Xinput_controller(dev_addr) ) {
    printf("X-Pad compatible detected\r\n");
  }
  if ( !tuh_vendor_receive_report(dev_addr, instance) )
  {
    printf("Error: cannot request to receive report\r\n");
  }
}

void tuh_vendor_umount_cb(uint8_t dev_addr, uint8_t instance)
{
  printf("VENDOR device address = %d, instance = %d is unmounted\r\n", dev_addr, instance);
}

void tuh_vendor_report_received_cb(uint8_t dev_addr, uint8_t instance, uint8_t const* report, uint16_t len)
{
printf("Report Received :!\r\n");
for (int i=0; i< len; i++) printf("0x%x ", report[i]);
printf("\r\n");
  // continue to request to receive report
  if ( !tuh_vendor_receive_report(dev_addr, instance) )
  {
    printf("Error: cannot request to receive report\r\n");
  }
}
