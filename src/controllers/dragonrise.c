#include "bsp/board.h"

#include "dragonrise.h"
#include "hid_gamepad.h"

_3do_report map_dragonRise(void* report_p) {
  hid_report_t* hid_report = (hid_report_t *)report_p;

  _3do_report result = new3doPadReport();

  result.up = (hid_report->rz < 0x7f)?1:0;
  result.down = (hid_report->rz > 0x7f)?1:0;
  result.left = (hid_report->z < 0x7f)?1:0;
  result.right = (hid_report->z > 0x7f)?1:0;
  result.X = hid_report->share;
  result.P = hid_report->l1;
  result.A = hid_report->square;
  result.B = hid_report->cross;
  result.C = hid_report->circle;
  result.L = hid_report->triangle | hid_report->l2;
  result.R = hid_report->r1 | hid_report->r2;
  return result;
}