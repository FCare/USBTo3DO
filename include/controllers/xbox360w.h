#ifndef _XBOX_360_W_H_
#define _XBOX_360_W_H_

#include "3DO.h"
#include "xbox360_gamepads.h"

typedef enum {
  LED_OFF = 0,
  LED_ALL_BLINK,
  LED_TOP_LEFT_BLINK_AND_ON,
  LED_TOP_RIGHT_BLINK_AND_ON,
  LED_BOTTOM_LEFT_BLINK_AND_ON,
  LED_BOTTOM_RIGHT_BLINK_AND_ON,
  LED_TOP_LEFT_ON, //1
  LED_TOP_RIGHT_ON, //2
  LED_BOTTOM_LEFT_ON, //3
  LED_BOTTOM_RIGHT_ON, //4
  LED_ROTATE,
  LED_BLINK,
  LED_SLOW_BLINK,
  LED_ROTATE_TWO_LIGHTS,
  LED_ALL_SLOW_BLINKS,
  LED_BLINK_ONCE
} led_state;

extern bool map_xbox360w(void* report_p, uint8_t len, uint8_t dev_addr, uint8_t instance, uint8_t *controler_id, controler_type* type, void** res);
extern bool mount_xbox360w(uint8_t dev_addr, uint8_t instance);

#endif