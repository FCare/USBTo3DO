#include "bsp/board.h"

#include <stdlib.h>

#include "thrustmaster.h"
#include "hid_gamepad.h"

// #define _DEBUG_MAPPER_
#define CTRL_DEBUG //printf


bool map_t80_pc(uint8_t* report, uint8_t len, uint8_t dev_addr, uint8_t instance, uint8_t *controler_id, controler_type* type, void** res) {
  #ifdef _DEBUG_MAPPER_
  //used for mapping debug
      for (int i = 0; i<len; i++) {
        printf("%d=%x ", i, report[i]);
      }
      printf("\r\n");
  #endif
    //Better to use 7 bytes of report directly

  *controler_id = instance;

  _3do_joystick_report *result = malloc(sizeof(_3do_joystick_report));
  *result = new3doStickReport();
  *type = JOYSTICK;

  int x = report[1];
  int accel = 0xFF - report[9];
  int brake = 0xFF - report[8];
  int y = ((accel - brake) + 0x100)/2;
  int z = 0x80;

  result->analog1  = x;
  result->analog2  = y>>2;
  result->analog3  = ((y & 0x3)<<6) | ((z & 0xF0)>>4);
  result->analog4 = ((z & 0x0F)<<4) | 0x2;

  result->FIRE = report[7] & 0x1;
  result->up = ((report[5]&0xF) == 1) || ((report[5]&0xF) == 0) || ((report[5]&0xF) == 7);
  result->down = ((report[5]&0xF) == 6) || ((report[5]&0xF) == 4) || ((report[5]&0xF) == 3);
  result->left = ((report[5]&0xF) == 7) || ((report[5]&0xF) == 6) || ((report[5]&0xF) == 5);
  result->right = ((report[5]&0xF) == 1) || ((report[5]&0xF) == 2) || ((report[5]&0xF) == 3);
  result->X = (report[6] & 0x40) != 0;
  result->P = (report[6] & 0x80) != 0;
  result->A = (report[5] & 0x40) != 0;
  result->B = (report[5] & 0x80) != 0;
  result->X = (report[5] & 0x20) != 0;
  result->C = (report[5] & 0x10) != 0;
  result->L = (report[6] & 0x2) != 0;
  result->R = (report[6] & 0x1) != 0;

  CTRL_DEBUG("down %d up %d right %d left %d A %d B %d C %d P %d X %d R %d L %d FIRE %d\n",
  result->down, result->up, result->right, result->left, result->A, result->B, result->C,
  result->P, result->X, result->R, result->L, result->FIRE );

  CTRL_DEBUG("X-axis %d(%x) Y-Axis %d(%x, %x) Z-Axis %d(%x)\n", x, report[1], y, report[9], report[8], z, report[7]);

  *res = (void *)(result);
  return true;
}

bool map_hotas_x_pc(uint8_t* report, uint8_t len, uint8_t dev_addr, uint8_t instance, uint8_t *controler_id, controler_type* type, void** res) {
  #ifdef _DEBUG_MAPPER_
  //used for mapping debug
      for (int i = 0; i<len; i++) {
        printf("%d=%x ", i, report[i]);
      }
      printf("\r\n");
  #endif
    //Better to use 7 bytes of report directly

  *controler_id = instance;

  _3do_joystick_report *result = malloc(sizeof(_3do_joystick_report));
  *result = new3doStickReport();
  *type = JOYSTICK;

  int x = ((((report[4]<<8) | report[3]) + 1024) % 1024)>>2;
  int y = ((((report[6]<<8) | report[5]) + 1024) % 1024)>>2;
  int z = (report[7] + 256) % 256;

  result->analog1  = x;
  result->analog2  = y>>2;
  result->analog3  = ((y & 0x3)<<6) | ((z & 0xF0)>>4);
  result->analog4 = ((z & 0x0F)<<4) | 0x2;

  result->FIRE = report[0] & 0x1;
  result->up = report[12] != 0;
  result->down = report[13] != 0;
  result->left = report[11] != 0;
  result->right = report[10] != 0;
  result->X = ((report[0] & 0x10) != 0)||((report[1] & 0x04) != 0)||((report[1] & 0x02) != 0);
  result->P = ((report[1] & 0x01) != 0)||((report[1] & 0x08) != 0)||((report[0] & 0x80) != 0);
  result->A = (report[0] & 0x2) != 0;
  result->B = (report[0] & 0x8) != 0;
  result->C = (report[0] & 0x4) != 0;
  result->L = (report[0] & 0x20) != 0;
  result->R = (report[0] & 0x40) != 0;

  CTRL_DEBUG("down %d up %d right %d left %d A %d B %d C %d P %d X %d R %d L %d FIRE %d\n",
  result->down, result->up, result->right, result->left, result->A, result->B, result->C,
  result->P, result->X, result->R, result->L, result->FIRE );

  CTRL_DEBUG("X-axis %d(%x) Y-Axis %d(%x) Z-Axis %d(%x)\n", x, report[3], y, report[5], z, report[7]);

  *res = (void *)(result);
  return true;
}

bool map_hotas_x_ps3(uint8_t* report, uint8_t len, uint8_t dev_addr, uint8_t instance, uint8_t *controler_id, controler_type* type, void** res) {
  #ifdef _DEBUG_MAPPER_
  //used for mapping debug
      for (int i = 0; i<len; i++) {
        printf("%d=%x ", i, report[i]);
      }
      printf("\r\n");
  #endif
    //Better to use 7 bytes of report directly

  *controler_id = instance;

  _3do_joystick_report *result = malloc(sizeof(_3do_joystick_report));
  *result = new3doStickReport();
  *type = JOYSTICK;

  int cross = (report[0] + 8)%8;

  int x = report[3];
  int y = report[4];
  int z = report[6];

  result->analog1  = x;
  result->analog2  = y>>2;
  result->analog3  = ((y & 0x3)<<6) | ((z & 0xF0)>>4);
  result->analog4 = ((z & 0x0F)<<4) | 0x2;

  result->FIRE = (report[0] & 0x20)!=0;
  result->up = (report[2] == 1) || (report[2] == 0) || (report[2] == 7);
  result->down = (report[2] == 6) || (report[2] == 4) || (report[2] == 3);
  result->left = (report[2] == 7) || (report[2] == 6) || (report[2] == 5);
  result->right = (report[2] == 1) || (report[2] == 2) || (report[2] == 3);
  result->A = (report[0] & 0x10) != 0;
  result->B = (report[1] & 0x4) != 0;
  result->C = (report[1] & 0x8) != 0;
  result->L = (report[0] & 0x2) != 0;
  result->R = (report[0] & 0x4) != 0;
  result->X = ((report[0]& 0x01) != 0)||((report[1] & 0x01) != 0)||((report[0] & 0x40) != 0);
  result->P = ((report[0] & 0x80) != 0)||((report[0] & 0x8) != 0)||((report[1] & 0x02) != 0);


  CTRL_DEBUG("down %d up %d right %d left %d A %d B %d C %d P %d X %d R %d L %d FIRE %d\n",
  result->down, result->up, result->right, result->left, result->A, result->B, result->C,
  result->P, result->X, result->R, result->L, result->FIRE );

  CTRL_DEBUG("X-axis %d(%x) Y-Axis %d(%x) Z-Axis %d(%x)\n", x, report[3], y, report[5], z, report[7]);

  *res = (void *)(result);
  return true;
}