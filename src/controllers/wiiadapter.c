#include "bsp/board.h"

#include "wiiadapter.h"
#include "hid_gamepad.h"

_3do_report map_wii_classic_adapter(void* report_p) {
  hid_report_t* hid_report = (hid_report_t *)report_p;
  printf("(x, y, z, rz, dpad) = (%x, %x, %x, %x, %x), \r\n", hid_report->x, hid_report->y, hid_report->z, hid_report->rz, hid_report->dpad);
  printf("(l1, r1, l2, r2) = (%x, %x, %x, %x, %x), \r\n", hid_report->l1, hid_report->r1, hid_report->l2, hid_report->r2);
  printf("(square, cross, circle,triangle) = (%x, %x, %x, %x, %x), \r\n", hid_report->square, hid_report->cross, hid_report->circle, hid_report->triangle);
  printf("(share, option, l3,r3) = (%x, %x, %x, %x, %x), \r\n", hid_report->share, hid_report->option, hid_report->l3, hid_report->r3);
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