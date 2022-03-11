#include "bsp/board.h"

#include "8bitdo.h"

bool map_8bitDo(void *report_p, uint8_t len, uint8_t dev_addr, uint8_t instance, uint8_t *controler_id, _3do_report* result) {
  uint8_t* report = (uint8_t *)report_p;
  *controler_id = instance;

  result->up = report[9] == 0x7f;
  result->down = report[9] == 0x80;
  result->left = report[7] == 0x80;
  result->right = report[7] == 0x7f;
  result->X = (report[2] & 0x10) != 0;
  result->P = ((report[2] & 0x20) != 0) || ((report[3] & 0x84) != 0);
  result->A = ((report[3] & 0x10) != 0);
  result->B = ((report[3] & 0x20) != 0);
  result->C = ((report[5] & 0xFF) != 0);
  result->L = ((report[3] & 0x41) != 0);
  result->R = ((report[4] & 0xFF) != 0) || ((report[3] & 0x2) != 0);

  CTRL_DEBUG("Touch 0x%X (down %d up %d right %d left %d A %d B %d C %d P %d X %d R %d L %d)\n",
result, result->down, result->up, result->right, result->left, result->A, result->B, result->C,
result->P, result->X, result->R, result->L );
  return true;
}