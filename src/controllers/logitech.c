#include "bsp/board.h"

#include <stdlib.h>

#include "logitech.h"
#include "hid_gamepad.h"

// #define _DEBUG_MAPPER_
#define CTRL_DEBUG //printf


bool map_logitech_extreme_pro(uint8_t* report_p, uint8_t len, uint8_t dev_addr, uint8_t instance, uint8_t *controler_id, controler_type* type, void** res) {
  // #ifdef _DEBUG_MAPPER_
  // //used for mapping debug
  //     for (int i = 0; i<len; i++) {
  //       printf("%d=%x ", i, report[i]);
  //     }
  //     printf("\r\n");
  // #endif
  //   //Better to use 7 bytes of report directly
  //
  // *controler_id = instance;
  //
  // _3do_joystick_report *result = malloc(sizeof(_3do_joystick_report));
  // *result = new3doStickReport();
  // *type = JOYSTICK;
  //
  // int x = report[1];
  // int accel = 0xFF - report[9];
  // int brake = 0xFF - report[8];
  // int y = ((accel - brake) + 0x100)/2;
  // int z = 0x80;
  //
  // result->analog1  = x;
  // result->analog2  = y>>2;
  // result->analog3  = ((y & 0x3)<<6) | ((z & 0xF0)>>4);
  // result->analog4 = ((z & 0x0F)<<4) | 0x2;
  //
  // result->FIRE = report[7] & 0x1;
  // result->up = ((report[5]&0xF) == 1) || ((report[5]&0xF) == 0) || ((report[5]&0xF) == 7);
  // result->down = ((report[5]&0xF) == 6) || ((report[5]&0xF) == 4) || ((report[5]&0xF) == 3);
  // result->left = ((report[5]&0xF) == 7) || ((report[5]&0xF) == 6) || ((report[5]&0xF) == 5);
  // result->right = ((report[5]&0xF) == 1) || ((report[5]&0xF) == 2) || ((report[5]&0xF) == 3);
  // result->X = (report[6] & 0x40) != 0;
  // result->P = (report[6] & 0x80) != 0;
  // result->A = (report[5] & 0x40) != 0;
  // result->B = (report[5] & 0x80) != 0;
  // result->X = (report[5] & 0x20) != 0;
  // result->C = (report[5] & 0x10) != 0;
  // result->L = (report[6] & 0x2) != 0;
  // result->R = (report[6] & 0x1) != 0;
  //
  // CTRL_DEBUG("down %d up %d right %d left %d A %d B %d C %d P %d X %d R %d L %d FIRE %d\n",
  // result->down, result->up, result->right, result->left, result->A, result->B, result->C,
  // result->P, result->X, result->R, result->L, result->FIRE );
  //
  // CTRL_DEBUG("X-axis %d(%x) Y-Axis %d(%x, %x) Z-Axis %d(%x)\n", x, report[1], y, report[9], report[8], z, report[7]);
  //
  // *res = (void *)(result);
  return true;
}