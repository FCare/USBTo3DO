#include "bsp/board.h"

#include <stdlib.h>
#include <math.h>

#include "xbox360w.h"

controler_type controler_mode[MAX_CONTROLERS] = {NONE};
uint8_t controler_addr[MAX_CONTROLERS] = {0};
bool lastMode[MAX_CONTROLERS] = {0};

static void set_led(uint8_t dev_addr, uint8_t instance, led_state state) {
  uint8_t protocol;
  tuh_vendor_protocol_get(dev_addr, instance, &protocol);
  if (protocol == 129) {
    TU_LOG1("Can set led\n");
    uint8_t buffer[12] = {0x00};
    buffer[2] = 0x08;
    buffer[3] = 0x40 + state; //rotate
    tuh_vendor_send_packet_out(dev_addr, instance, &buffer[0], 12);
  }
}

static xbox360_report handle_xbox360_report(uint8_t const* report, uint16_t len) {
  xbox360_report status;

    /* dpad as buttons (left, right, up, down) */
    status.BTN_TRIGGER_HAPPY1 = (report[2] & 0x04) != 0;
    status.BTN_TRIGGER_HAPPY2 = (report[2] & 0x08) != 0;
    status.BTN_TRIGGER_HAPPY3 = (report[2] & 0x01) != 0;
    status.BTN_TRIGGER_HAPPY4 = (report[2] & 0x02) != 0;

    /*
    * This should be a simple else block. However historically
    * xbox360w has mapped DPAD to buttons while xbox360 did not. This
    * made no sense, but now we can not just switch back and have to
    * support both behaviors.
    */
    status.ABS_HAT0X = !!(report[2] & 0x08) - !!(report[2] & 0x04);
    status.ABS_HAT0Y = !!(report[2] & 0x02) - !!(report[2] & 0x01);

  	/* start/back buttons */
  	status.BTN_START = (report[2] & 0x10) != 0;
  	status.BTN_SELECT = (report[2] & 0x20) != 0;

  	/* stick press left/right */
  	status.BTN_THUMBL = (report[2] & 0x40) != 0;
  	status.BTN_THUMBR = (report[2] & 0x80) != 0;

  	/* buttons A,B,X,Y,TL,TR and MODE */
  	status.BTN_A = (report[3] & 0x10) != 0;
  	status.BTN_B = (report[3] & 0x20) != 0;
  	status.BTN_X = (report[3] & 0x40) != 0;
  	status.BTN_Y = (report[3] & 0x80) != 0;
  	status.BTN_TL = (report[3] & 0x01) != 0;
  	status.BTN_TR = (report[3] & 0x02) != 0;
  	status.BTN_MODE = (report[3] & 0x04) != 0;

    status.ABS_X = *((int16_t*) (report + 6));
    status.ABS_Y = *((int16_t*) (report + 8));
    status.ABS_RX = *((int16_t*) (report + 10));
    status.ABS_RY = *((int16_t*) (report + 12));

  	/* triggers left/right */
		status.BTN_TL2 = report[4];
		status.BTN_TR2 = report[5];
		status.ABS_Z = report[4];
		status.ABS_RZ = report[5];

  return status;
}

bool mount_xbox360w(uint8_t dev_addr, uint8_t instance) {
  controler_mode[instance] = JOYPAD;
  lastMode[instance] = 1; //First report gets BTN Mode UP
  controler_addr[instance] = dev_addr;
  set_led(dev_addr, instance, LED_TOP_LEFT_BLINK_AND_ON + instance%4);
  return false; //Do not consider it is added. Wait for first report
}
static int mode[MAX_CONTROLERS] = {0};

void led_xbox360w(void) {
  for (int i = 0; i<MAX_CONTROLERS; i++) {
    if (controler_mode[i] == JOYSTICK)
        set_led(controler_addr[i], i, LED_TOP_LEFT_ON + (i+mode[i])%4);
    if (controler_mode[i] == JOYSTICK)
      mode[i] = !mode[i];
  }
}

bool map_xbox360w(uint8_t *report_p, uint8_t len, uint8_t dev_addr,uint8_t instance, uint8_t *controler_id, controler_type* type, void** res) {
    uint8_t * int_report = (uint8_t *)report_p;
    *controler_id = instance;

    *type = controler_mode[instance];

    if (len == 2) {
      if ((int_report[0] & 0x08) && ((int_report[1] & 0x80) != 0)) {
        return false;
      }
    }

  if ((int_report[0] == 0x0) && (int_report[1] == 0x0)) return false;

  xbox360_report report  = handle_xbox360_report(&int_report[4],len - 5);

  if (report.BTN_MODE != lastMode[instance]) {
    lastMode[instance] = report.BTN_MODE;
    if (report.BTN_MODE) {
      switch (controler_mode[instance]){
        case JOYPAD:
          controler_mode[instance] = JOYSTICK;
          mode[instance] = 0;
          break;
        case JOYSTICK:
          controler_mode[instance] = JOYPAD;
          mode[instance] = 0;
          break;
        default:
          break;
      }
    }
    set_led(dev_addr, instance, LED_TOP_LEFT_BLINK_AND_ON + instance%4);
  }


  if (controler_mode[instance] == JOYPAD) {
    _3do_joypad_report *result = malloc(sizeof(_3do_joypad_report));
    *result = new3doPadReport();
    result->up = report.BTN_TRIGGER_HAPPY3 || (report.ABS_Y > 22500) || (report.ABS_RY > 22500);
    result->down = report.BTN_TRIGGER_HAPPY4 || (report.ABS_Y < -22500) || (report.ABS_RY < -22500);
    result->left = report.BTN_TRIGGER_HAPPY1 || (report.ABS_X < -22500)|| (report.ABS_RX < -22500);
    result->right = report.BTN_TRIGGER_HAPPY2 || (report.ABS_X > 22500)|| (report.ABS_RX > 22500);
    result->X = report.BTN_START;
    result->P = report.BTN_SELECT || report.BTN_Y;
    result->A = report.BTN_X;
    result->B = report.BTN_A;
    result->C = report.BTN_B;
    result->L = report.BTN_TL || (report.BTN_TL2 != 0);
    result->R = report.BTN_TR || (report.BTN_TR2 != 0);

    CTRL_DEBUG("Touch 0x%X (down %d up %d right %d left %d A %d B %d C %d P %d X %d R %d L %d)\n",
    result, result->down, result->up, result->right, result->left, result->A, result->B, result->C,
    result->P, result->X, result->R, result->L );
      *res = (void *)(result);
  }

  if (controler_mode[instance] == JOYSTICK) {
    _3do_joystick_report *result = malloc(sizeof(_3do_joystick_report));
    *result = new3doStickReport();

    uint8_t h_pos = (((int32_t)report.ABS_X + 0x8000) >> 8) & 0xFF;
    uint8_t v_pos = ((0x7FFF - (int32_t)report.ABS_Y ) >> 8) & 0xFF;

    int16_t accel = (report.ABS_Z + 256) %256;
    int16_t brake = (report.ABS_RZ + 256) %256;
    uint32_t length = sqrt((h_pos - 128)*(h_pos-128) + (v_pos-128)*(v_pos-128));

    int x = h_pos - 128;
    int y = v_pos - 128;

    int ax = abs(x);
    int ay = abs(y);

    double ratio = 1;
    if ((ax != 0) && (ay != 0)) ratio = (double)length/(double)((ax>=ay)?ax:ay);

    y = (int)(y*ratio + 128);
    x = (int)(x*ratio + 128);

    y = (y>255)?255:y;
    y = (y<0)?0:y;

    x = (x>255)?255:x;
    x = (x<0)?0:x;

    int d_pos = ((accel - brake)>>1) + 128;
    d_pos = (d_pos>255)?255:d_pos;
    d_pos = (d_pos<0)?0:d_pos;

    result->analog1 = x;
    result->analog2 = (y >> 2);
    result->analog3 = ((y & 0x3)<<6) | ((d_pos & 0xF0)>>4);
    result->analog4 = ((d_pos & 0x0F)<<4) | 0x2;

    result->FIRE = report.BTN_Y;
    result->up = report.BTN_TRIGGER_HAPPY3 || (report.ABS_RY > 22500);
    result->down = report.BTN_TRIGGER_HAPPY4 || (report.ABS_RY < -22500);
    result->left = report.BTN_TRIGGER_HAPPY1 || (report.ABS_RX < -22500);
    result->right = report.BTN_TRIGGER_HAPPY2 || (report.ABS_RX > 22500);
    result->X = report.BTN_START;
    result->P = report.BTN_SELECT;
    result->A = report.BTN_X;
    result->B = report.BTN_A;
    result->C = report.BTN_B;
    result->L = report.BTN_TL;
    result->R = report.BTN_TR;

    CTRL_DEBUG("Touch 0x%X (down %d up %d right %d left %d A %d B %d C %d P %d X %d R %d L %d FIRE %d)\n",
    result, result->down, result->up, result->right, result->left, result->A, result->B, result->C,
    result->P, result->X, result->R, result->L, result->FIRE );

    CTRL_DEBUG("brake %d accel %d v_pos %d h_pos %d d_pos %d ABS_Z %x ABS_RZ %x\n",brake, accel, (uint8_t)v_pos, (uint8_t)h_pos, (uint8_t)d_pos, (int32_t)report.ABS_Z , (int32_t)report.ABS_RZ);

    *res = (void *)(result);
  }

  return true;
}