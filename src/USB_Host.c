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
  led_blinking_task();

#if CFG_TUH_CDC
  cdc_task();
#endif

#if CFG_TUH_HID
  hid_app_task();
#endif
}

static void led_blinking_task(void)
{
  static uint32_t start_ms = 0;
  static bool led_state = false;

  // Blink every interval ms
  if ( board_millis() - start_ms < blink_interval_ms) return; // not enough time
  start_ms += blink_interval_ms;

  board_led_write(led_state);
  led_state = 1 - led_state; // toggle
}