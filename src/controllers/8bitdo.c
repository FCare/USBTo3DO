#include "bsp/board.h"

#include <stdlib.h>

#include "8bitdo.h"

// #define CTRL_DEBUG printf
// #define _DEBUG_MAPPER_

bool map_8bitDo_M30(uint8_t *report_p, uint8_t len, uint8_t dev_addr, uint8_t instance, uint8_t *controler_id, controler_type* type, void** res) {
  uint8_t* report = (uint8_t *)report_p;
  *controler_id = instance;

  _3do_joypad_report *result = malloc(sizeof(_3do_joypad_report));
  *result = new3doPadReport();
  *type = JOYPAD;

#ifdef _DEBUG_MAPPER_
//used for mapping debug
  for (int i =0; i< len; i++) {
    printf("%x ", report[i]);
  }
  printf("\n");
#endif

  result->up = (report[4] & 0x7f) == 0x0;
  result->down = (report[4] & 0x80) != 0x0;
  result->left = (report[3] & 0x7f) == 0x0;
  result->right = (report[3] & 0x80) != 0x0;
  result->P = ((report[1] & 0x08) != 0x0) | ((report[0] & 0x10) != 0x0);
  result->X = ((report[0] & 0x04) != 0x0) | ((report[1] & 0x04) != 0x0);
  result->A = ((report[0] & 0x01) != 0x0);
  result->B = ((report[0] & 0x02) != 0x0);
  result->C = ((report[0] & 0x80) != 0x0);
  result->L = ((report[0] & 0x08) != 0x0) | ((report[1] & 0x01) != 0x0);
  result->R = ((report[0] & 0x40) != 0x0) | ((report[1] & 0x02) != 0x0);

  CTRL_DEBUG("Touch 0x%X (down %d up %d right %d left %d A %d B %d C %d P %d X %d R %d L %d)\n",
result, result->down, result->up, result->right, result->left, result->A, result->B, result->C,
result->P, result->X, result->R, result->L );
  *res = (void *)(result);
  return true;
}