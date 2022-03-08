#include "bsp/board.h"

#include "8bitdo.h"

_3do_report map_8bitDo(void *report_p, uint8_t instance, uint8_t *controler_id) {
  xbox360_report* report = (xbox360_report *)report_p;
  _3do_report result = new3doPadReport();
  *controler_id = instance;

  result.up = (report->BTN_Z > 0)?1:0;
  result.down = (report->BTN_Z < 0)?1:0;
  result.left = (report->BTN_Y < 0)?1:0;
  result.right = (report->BTN_Y > 0)?1:0;
  result.X = report->BTN_START;
  result.P = report->BTN_SELECT | report->BTN_SELECT | report->BTN_WEST;
  result.A = report->BTN_SOUTH;
  result.B = report->BTN_EAST;
  result.C = (report->BTN_B != 0)?1:0;
  result.L = report->BTN_NORTH | report->BTN_TL;
  result.R = ((report->BTN_A != 0)?1:0) | report->BTN_TR;

  CTRL_DEBUG("Touch 0x%X (down %d up %d right %d left %d A %d B %d C %d P %d X %d R %d L %d)\n",
result, result.down, result.up, result.right, result.left, result.A, result.B, result.C,
result.P, result.X, result.R, result.L );
  return result;
}