#include "bsp/board.h"

#include <stdlib.h>
#include <math.h>

#include "ps4.h"

#define CTRL_DEBUG //printf

// Sony DS4 report layout detail https://www.psdevwiki.com/ps4/DS4-USB
typedef struct TU_ATTR_PACKED
{
  uint8_t x, y, z, rz; // joystick

  struct {
    uint8_t dpad     : 4; // (hat format, 0x08 is released, 0=N, 1=NE, 2=E, 3=SE, 4=S, 5=SW, 6=W, 7=NW)
    uint8_t square   : 1; // west
    uint8_t cross    : 1; // south
    uint8_t circle   : 1; // east
    uint8_t triangle : 1; // north
  };

  struct {
    uint8_t l1     : 1;
    uint8_t r1     : 1;
    uint8_t l2     : 1;
    uint8_t r2     : 1;
    uint8_t share  : 1;
    uint8_t option : 1;
    uint8_t l3     : 1;
    uint8_t r3     : 1;
  };

  struct {
    uint8_t ps      : 1; // playstation button
    uint8_t tpad    : 1; // track pad click
    uint8_t counter : 6; // +1 each report
  };

  // comment out since not used by this example
  // uint8_t l2_trigger; // 0 released, 0xff fully pressed
  // uint8_t r2_trigger; // as above

  //  uint16_t timestamp;
  //  uint8_t  battery;
  //
  //  int16_t gyro[3];  // x, y, z;
  //  int16_t accel[3]; // x, y, z

  // there is still lots more info

} sony_ds4_report_t;

// check if 2 reports are different enough
static bool diff_report(sony_ds4_report_t const* rpt1, sony_ds4_report_t const* rpt2)
{
  bool result = false;

//only compare used buttons for the moment
  result |= rpt1->dpad != rpt2->dpad;
  result |= rpt1->square != rpt2->square;
  result |= rpt1->cross != rpt2->cross;
  result |= rpt1->circle != rpt2->circle;
  result |= rpt1->triangle != rpt2->triangle;

  result |= rpt1->l1 != rpt2->l1;
  result |= rpt1->r1 != rpt2->r1;
  result |= rpt1->l2 != rpt2->l2;
  result |= rpt1->r2 != rpt2->r2;

  result |= rpt1->ps != rpt2->ps;

  return result;
}

static sony_ds4_report_t old_report = {0};

static sony_ds4_report_t * handle_ds4_report(uint8_t const* report)
{
  // all buttons state is stored in ID 1
  if (report[0] == 1)
  {
    sony_ds4_report_t *ds4_report = malloc(sizeof(sony_ds4_report_t));
    memcpy(ds4_report, &report[1], sizeof(sony_ds4_report_t));
    if (diff_report(&old_report, ds4_report)) {
      // #ifdef _DEBUG_MAPPER_
      const char* dpad_str[] = { "N", "NE", "E", "SE", "S", "SW", "W", "NW", "none" };
      printf("(x, y, z, rz, dpad) = (%x, %x, %, %x, %x)\r\n", ds4_report->x, ds4_report->y, ds4_report->z, ds4_report->rz, ds4_report->dpad);
      printf("DPad = %s ", dpad_str[ds4_report->dpad]);
      if (ds4_report->square   ) printf("Square ");
      if (ds4_report->cross    ) printf("Cross ");
      if (ds4_report->circle   ) printf("Circle ");
      if (ds4_report->triangle ) printf("Triangle ");
      if (ds4_report->l1       ) printf("L1 ");
      if (ds4_report->r1       ) printf("R1 ");
      if (ds4_report->l2       ) printf("L2 ");
      if (ds4_report->r2       ) printf("R2 ");
      if (ds4_report->share    ) printf("Share ");
      if (ds4_report->option   ) printf("Option ");
      if (ds4_report->l3       ) printf("L3 ");
      if (ds4_report->r3       ) printf("R3 ");
      if (ds4_report->ps       ) printf("PS ");
      if (ds4_report->tpad     ) printf("TPad ");
      printf("\r\n");
      // #endif
      memcpy(&old_report,ds4_report, sizeof(sony_ds4_report_t));
      return ds4_report;
    } else {
      free(ds4_report);
      return NULL;
    }
  }
  return NULL;
}


bool map_ds4(uint8_t *report_p, uint8_t len, uint8_t dev_addr,uint8_t instance, uint8_t *controler_id, controler_type* type, void** res) {
  *controler_id = instance;
  _3do_joypad_report *result = malloc(sizeof(_3do_joypad_report));
  *result = new3doPadReport();
  *type = JOYPAD;

  sony_ds4_report_t* ds4_report  = handle_ds4_report(&report_p[0]);
  if (ds4_report == NULL) {
    *res = (void *)(result);
    return false;
  }
  result->up = ((ds4_report->dpad & 0xF)==0) || ((ds4_report->dpad & 0xF)==1) || ((ds4_report->dpad & 0xF)==7);
  result->down = ((ds4_report->dpad & 0xF)==3) || ((ds4_report->dpad & 0xF)==4) || ((ds4_report->dpad & 0xF)==5);
  result->left = ((ds4_report->dpad & 0xF)==5) || ((ds4_report->dpad & 0xF)==6) || ((ds4_report->dpad & 0xF)==7);
  result->right = ((ds4_report->dpad & 0xF)==1) || ((ds4_report->dpad & 0xF)==2) || ((ds4_report->dpad & 0xF)==3);
  result->X = ds4_report->triangle;
  result->P = ds4_report->ps;
  result->A = ds4_report->square;
  result->B = ds4_report->cross;
  result->C = ds4_report->circle;
  result->L = ds4_report->l1 || ds4_report->l2;
  result->R = ds4_report->r1 || ds4_report->r2;

  free(ds4_report);
  *res = (void *)(result);

  return true;
}