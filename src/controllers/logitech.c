#include "bsp/board.h"

#include <stdlib.h>

#include "logitech.h"
#include "hid_gamepad.h"
#include "hid_parser.h"

bool map_logitech_extreme_pro(uint8_t instance, uint8_t *id, controler_type *type, void **res, void *ctrl_v)
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
  result->X = btn->BASE2;
  result->P = btn->BASE;
  result->A = btn->BASE3 || btn->THUMB2;
  result->B = btn->TOP || btn->BASE4;
  result->C = btn->BASE6 || btn->BASE5;
  result->L = btn->TOP2;
  result->R = btn->PINKIE;


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