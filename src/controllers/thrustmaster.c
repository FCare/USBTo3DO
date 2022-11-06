#include "bsp/board.h"

#include <stdlib.h>

#include "thrustmaster.h"
#include "hid_gamepad.h"
#include "hid_parser.h"

// #define _DEBUG_MAPPER_
#define CTRL_DEBUG //printf


bool map_t80_pc(uint8_t instance, uint8_t *id, controler_type *type, void **res, void *ctrl_v)
{
  hid_controller *ctrl =(hid_controller*)ctrl_v;
  _3do_joystick_report *result = malloc(sizeof(_3do_joystick_report));
  *result = new3doStickReport();
  *type = JOYSTICK;
  *id = ctrl->index + instance;
  hid_buttons *btn = & ctrl->buttons[ctrl->index];
  int angle = btn->ABS_X;
  int speed = (0x100 + btn->ABS_RX - btn->ABS_RY)>>1;
  int throttle = 0x80;
  result->analog1  = angle;
  result->analog2  = (speed>>2);
  result->analog3  = ((speed & 0x3)<<6) | ((throttle & 0xF0)>>4);
  result->analog4 = ((throttle & 0x0F)<<4) | 0x2;

  result->up = btn->HAT_UP;
  result->down = btn->HAT_DOWN;
  result->left = btn->HAT_LEFT;
  result->right = btn->HAT_RIGHT;

  result->FIRE = btn->THUMB;
  result->X = btn->BASE4 || btn->BASE6;
  result->P = btn->BASE3 || btn->BASE5;
  result->A = btn->TOP;
  result->B = btn->TRIGGER;
  result->C = btn->THUMB2;
  result->L = btn->TOP2;
  result->R = btn->PINKIE;

#ifdef _DEBUG_MAPPER_
  //used for mapping debug
  printf("down %d up %d right %d left %d A %d B %d C %d P %d X %d R %d L %d FIRE %d\n",
  result->down, result->up, result->right, result->left, result->A, result->B, result->C,
  result->P, result->X, result->R, result->L, result->FIRE );

  printf("Angle %d Speed %d Throttle %d\n", angle, speed, throttle);
#endif
  *res = (void *)(result);

  return true;
}

bool map_hotas_x_flight_pc(uint8_t instance, uint8_t *id, controler_type *type, void **res, void *ctrl_v)
{
  hid_controller *ctrl =(hid_controller*)ctrl_v;
  _3do_joystick_report *result = malloc(sizeof(_3do_joystick_report));
  *result = new3doStickReport();
  *type = JOYSTICK;
  *id = ctrl->index + instance;
  hid_buttons *btn = & ctrl->buttons[ctrl->index];

  result->analog1  = btn->ABS_X;
  result->analog2  = btn->ABS_Y>>2;
  result->analog3  = ((btn->ABS_Y & 0x3)<<6) | ((btn->SLIDER & 0xF0)>>4);
  result->analog4 = ((btn->SLIDER & 0x0F)<<4) | 0x2;

  result->up = btn->HAT_UP;
  result->down = btn->HAT_DOWN;
  result->left = btn->HAT_LEFT;
  result->right = btn->HAT_RIGHT;

  result->FIRE = btn->TRIGGER;
  result->X = btn->BASE6;
  result->P = btn->BASE5;
  result->A = btn->THUMB || btn->BASE3;
  result->B = btn->TOP || btn->BASE4;
  result->C = btn->THUMB2 || btn->BASE || btn->BASE2;
  result->L = btn->PINKIE;
  result->R = btn->TOP2;

  #ifdef _DEBUG_MAPPER_
  //used for mapping debug
  printf("down %d up %d right %d left %d A %d B %d C %d P %d X %d R %d L %d FIRE %d\n",
  result->down, result->up, result->right, result->left, result->A, result->B, result->C,
  result->P, result->X, result->R, result->L, result->FIRE );

  printf("X-axis %d Y-Axis %d Z-Axis %d\n", btn->ABS_X, btn->ABS_X, btn->SLIDER);
  #endif
  *res = (void *)(result);

  return true;
}

bool map_hotas_x_pc(uint8_t instance, uint8_t *id, controler_type *type, void **res, void *ctrl_v)
{
  hid_controller *ctrl =(hid_controller*)ctrl_v;
  _3do_joystick_report *result = malloc(sizeof(_3do_joystick_report));
  *result = new3doStickReport();
  *type = JOYSTICK;
  *id = ctrl->index + instance;
  hid_buttons *btn = & ctrl->buttons[ctrl->index];

  result->analog1  = btn->ABS_X;
  result->analog2  = btn->ABS_Y>>2;
  result->analog3  = ((btn->ABS_Y & 0x3)<<6) | ((btn->ABS_RZ & 0xF0)>>4);
  result->analog4 = ((btn->ABS_RZ & 0x0F)<<4) | 0x2;

  result->up = btn->HAT_UP;
  result->down = btn->HAT_DOWN;
  result->left = btn->HAT_LEFT;
  result->right = btn->HAT_RIGHT;

  result->FIRE = btn->TRIGGER;
  result->X = btn->BASE6;
  result->P = btn->BASE5;
  result->A = btn->THUMB || btn->BASE3;
  result->B = btn->TOP || btn->BASE4;
  result->C = btn->THUMB2 || btn->BASE || btn->BASE2;
  result->L = btn->PINKIE || (btn->SLIDER <= 64);
  result->R = btn->TOP2 || (btn->SLIDER >= 192);

  #ifdef _DEBUG_MAPPER_
  //used for mapping debug
  printf("down %d up %d right %d left %d A %d B %d C %d P %d X %d R %d L %d FIRE %d\n",
  result->down, result->up, result->right, result->left, result->A, result->B, result->C,
  result->P, result->X, result->R, result->L, result->FIRE );

  printf("X-axis %d Y-Axis %d Z-Axis %d\n", btn->ABS_X, btn->ABS_X, btn->SLIDER);
  #endif
  *res = (void *)(result);

  return true;
}

bool map_hotas_x_ps3(uint8_t instance, uint8_t *id, controler_type *type, void **res, void *ctrl_v)
{
  hid_controller *ctrl =(hid_controller*)ctrl_v;
  _3do_joystick_report *result = malloc(sizeof(_3do_joystick_report));
  *result = new3doStickReport();
  *type = JOYSTICK;
  *id = ctrl->index + instance;
  hid_buttons *btn = & ctrl->buttons[ctrl->index];

  result->analog1  = btn->ABS_X;
  result->analog2  = btn->ABS_Y>>2;
  result->analog3  = ((btn->ABS_Y & 0x3)<<6) | ((btn->ABS_RZ & 0xF0)>>4);
  result->analog4 = ((btn->ABS_RZ & 0x0F)<<4) | 0x2;

  result->up = btn->HAT_UP;
  result->down = btn->HAT_DOWN;
  result->left = btn->HAT_LEFT;
  result->right = btn->HAT_RIGHT;

  result->FIRE = btn->PINKIE;
  result->X = btn->BASE6 || btn->BASE4;;
  result->P = btn->BASE5 || btn->BASE3;
  result->A = btn->TOP2 || btn->BASE2;
  result->B = btn->BASE5 || btn->BASE;
  result->C = btn->BASE6 || btn->THUMB2 || btn->TOP;
  result->L = btn->TRIGGER || (btn->ABS_Z <= 64);
  result->R = btn->THUMB || (btn->ABS_Z >= 192);

  #ifdef _DEBUG_MAPPER_
  //used for mapping debug
  printf("down %d up %d right %d left %d A %d B %d C %d P %d X %d R %d L %d FIRE %d\n",
  result->down, result->up, result->right, result->left, result->A, result->B, result->C,
  result->P, result->X, result->R, result->L, result->FIRE );

  printf("X-axis %d Y-Axis %d Z-Axis %d\n", btn->ABS_X, btn->ABS_X, btn->SLIDER);
  #endif
  *res = (void *)(result);

  return true;
}