#ifndef _XBOX_360_CONTROLLER_H_
#define _XBOX_360_CONTROLLER_H_

#define CTRL_DEBUG // printf
// #define _DEBUG_MAPPER_

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
} xbox360_report;

extern void vendor_gamepad_tick(void);

#endif