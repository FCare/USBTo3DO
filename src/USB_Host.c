#include "USB_Host.h"

#include "bsp/board.h"

enum  {
  BLINK_NOT_MOUNTED = 250,
  BLINK_MOUNTED = 1000,
  BLINK_SUSPENDED = 2500,
};

static uint32_t blink_interval_ms = BLINK_NOT_MOUNTED;

static void led_blinking_task(void);

void USB_Host_init() {
    board_init();

    printf("TinyUSB Host CDC MSC HID Example\r\n");

    tusb_init();
}

void USB_Host_loop()
{
  // tinyusb host task
  tuh_task();

#if CFG_TUH_HID
  hid_app_task();
#endif
}

void USB_Host_set_key_up_cb(key_cb key_up_cb){

}

void USB_Host_set_key_down_cb(key_cb key_up_cb){

}
