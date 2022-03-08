#include "bsp/board.h"

#include "retroBit.h"
#include "hid_gamepad.h"

_3do_report map_retroBit(void* report_p,uint8_t instance) {
  hid_report_t* hid_report = (hid_report_t *)report_p;
  uint8_t* report = (uint8_t *)report_p;

  printf("%x %x %x %x %x %x %x, \r\n", report[0],report[1],report[2],report[3],report[4],report[5],report[6]);

    //Better to use 7 bytes of report directly

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