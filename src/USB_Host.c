#include "USB_Host.h"


void USB_Host_init() {
    board_init();

    printf("TinyUSB Host CDC MSC HID Example\r\n");

    tusb_init();
}

void USB_Host_loop()
{
  // tinyusb host task
  tuh_task();
  led_blinking_task();

#if CFG_TUH_CDC
  cdc_task();
#endif

#if CFG_TUH_HID
  hid_app_task();
#endif
}