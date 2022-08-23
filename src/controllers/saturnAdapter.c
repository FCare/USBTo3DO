#include "bsp/board.h"

#include <stdlib.h>

#include "saturnAdapter.h"
#include "hid_gamepad.h"

static uint8_t oldReport[2][7];

bool map_saturn_adapter(void* report_p, uint8_t len, uint8_t dev_addr, uint8_t instance, uint8_t *controler_id, controler_type* type, void** res) {
  uint8_t* report = (uint8_t *)report_p;
  uint8_t id = report[0] - 1;
#ifdef _DEBUG_MAPPER_
//used for mapping debug
bool showTrace = false;
  if (memcmp(&oldReport[id][1], report[1], 6) != 0) {
    printf("%x %x %x %x %x %x %x, \r\n", report[0],report[1],report[2],report[3],report[4],report[5],report[6]);
    showTrace = true;
  }
  memcpy(&oldReport[id], report, 7);
#endif

  *controler_id = id;

  _3do_joypad_report *result = malloc(sizeof(_3do_joypad_report));
  *result = new3doPadReport();
  *type = JOYPAD;

  result->up = (((report[5]&0xF) == 0x0)||((report[5]&0xF) == 0x1)||((report[5]&0xF) == 0x7))?1:0;
  result->down = (((report[5]&0xF) == 0x3)||((report[5]&0xF) == 0x4)||((report[5]&0xF) == 0x5))?1:0;
  result->left = (((report[5]&0xF) == 0x5)||((report[5]&0xF) == 0x6)||((report[5]&0xF) == 0x7))?1:0;
  result->right = (((report[5]&0xF) == 0x1)||((report[5]&0xF) == 0x2)||((report[5]&0xF) == 0x3))?1:0;
  result->X = (report[6]>>5)&0x1;
  result->P = (report[6]>>0)&0x1;
  result->A = (report[5]>>4)&0x1;
  result->B = (report[5]>>5)&0x1;
  result->C = (report[5]>>6)&0x1;
  result->L = ((report[6]>>1)&0x1) || ((report[5]>>7)&0x1);
  result->R = ((report[6]>>3)&0x1) || ((report[6]>>2)&0x1);

#ifdef _DEBUG_MAPPER_
  //used for mapping debug
  if (showTrace) printf("(up, down, left, right) (%d %d %d %d) (X,P,A,B,C,L,R)(%d %d %d %d %d %d %d)\n",
        result->up, result->down, result->left, result->right, result->X, result->P, result->A, result->B, result->C, result->L, result->R);
#endif
  *res = (void *)(result);

  return true;
}