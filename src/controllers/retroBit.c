#include "bsp/board.h"

#include "retroBit.h"
#include "hid_gamepad.h"

static uint8_t oldReport[2][7];

bool map_retroBit(void* report_p, uint8_t len, uint8_t dev_addr, uint8_t instance, uint8_t *controler_id, _3do_report* result) {
  uint8_t* report = (uint8_t *)report_p;
  #if 0
  //used for mapping debug
    if (memcmp(&oldReport[instance], report, 7) != 0)
      printf("%x %x %x %x %x %x %x, \r\n", report[0],report[1],report[2],report[3],report[4],report[5],report[6]);
    memcpy(&oldReport[instance], report, 7);
  #endif
    //Better to use 7 bytes of report directly

  *controler_id = instance;

  result->up = (report[4] < 0x80)?1:0;
  result->down = (report[4] > 0x80)?1:0;
  result->left = (report[3] < 0x80)?1:0;
  result->right = (report[3] > 0x80)?1:0;
  result->X = ((report[1]>>1)&0x1) || ((report[1]>>4)&0x1);
  result->P = ((report[1]>>0)&0x1) || ((report[0]>>0)&0x1);
  result->A = (report[0]>>2)&0x1;
  result->B = (report[0]>>1)&0x1;
  result->C = (report[0]>>7)&0x1;
  result->L = ((report[0]>>4)&0x1) || ((report[0]>>3)&0x1);
  result->R = ((report[0]>>5)&0x1) || ((report[0]>>6)&0x1);

  #if 0
    //used for mapping debug
    printf("(up, down, left, right) (%d %d %d %d) (X,P,A,B,C,L,R)(%d %d %d %d %d %d %d)\n",
          result->up, result->down, result->left, result->right, result->X, result->P, result->A, result->B, result->C, result->L, result->R);
  #endif

  return true;
}