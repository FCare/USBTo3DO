#include "bsp/board.h"

#include <stdlib.h>

#include "psClassic.h"
#include "hid_gamepad.h"
#include "hid_parser.h"

bool map_ps_classic(uint8_t instance, uint8_t *id, controler_type *type, void **res, void *ctrl_v)
 {
  hid_controller *ctrl =(hid_controller*)ctrl_v;
  _3do_joypad_report *result = malloc(sizeof(_3do_joypad_report));
  *result = new3doPadReport();
  *type = JOYPAD;
  *id = ctrl->index + instance;
  hid_buttons *btn = & ctrl->buttons[ctrl->index];

  result->up = btn->ABS_Y < 64;
  result->down = btn->ABS_Y > 64;
  result->left = btn->ABS_X < 64;
  result->right = btn->ABS_X > 64;
  result->X = btn->TRIGGER || btn->BASE3;
  result->P = btn->BASE4;
  result->A = btn->TOP;
  result->B = btn->THUMB2;
  result->C = btn->THUMB;
  result->L = btn->TOP2 || btn->BASE;
  result->R = btn->PINKIE || btn->BASE2;

  #ifdef _DEBUG_MAPPER_
  //used for mapping debug
  printf("(up, down, left, right) (%d %d %d %d) (X,P,A,B,C,L,R)(%d %d %d %d %d %d %d)\n",
  result->up, result->down, result->left, result->right, result->X, result->P, result->A, result->B, result->C, result->L, result->R);
  #endif
  *res = (void *)(result);
  return true;
}