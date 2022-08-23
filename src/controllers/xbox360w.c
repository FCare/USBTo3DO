#include "bsp/board.h"

#include <stdlib.h>

#include "xbox360w.h"

controler_type controler_mode[MAX_CONTROLERS] = {JOYPAD};
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
  set_led(dev_addr, instance, LED_ALL_BLINK);
  return false; //Do not consider it is added. Wait for first report
}

bool map_xbox360w(void *report_p, uint8_t len, uint8_t dev_addr,uint8_t instance, uint8_t *controler_id, controler_type* type, void** res) {
    uint8_t * int_report = (uint8_t *)report_p;
    *controler_id = instance;


    if (len == 2) {
      if ((int_report[0] & 0x08) && ((int_report[1] & 0x80) != 0)) {
        if (controler_mode[instance] == JOYPAD)
          set_led(dev_addr, instance, LED_TOP_LEFT_BLINK_AND_ON + instance%4);
        else
          set_led(dev_addr, instance, LED_ROTATE_TWO_LIGHTS);
        return false;
      }
    }


  if ((int_report[0] == 0x0) && (int_report[0] == 0x1)) return false;

  xbox360_report report  = handle_xbox360_report(&int_report[4],len - 5);

  if (report.BTN_MODE != lastMode[instance]) {
    lastMode[instance] = report.BTN_MODE;
    if (report.BTN_MODE) {
      controler_mode[instance] = (controler_mode[instance] == JOYPAD)?JOYSTICK:JOYPAD;
    }
    if (controler_mode[instance] == JOYPAD)
      set_led(dev_addr, instance, LED_TOP_LEFT_BLINK_AND_ON + instance%4);
    else
      set_led(dev_addr, instance, LED_ROTATE_TWO_LIGHTS);
  }

  *type = controler_mode[instance];

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

  if (controler_mode[instance] == JOYPAD) {
    _3do_joystick_report *result = malloc(sizeof(_3do_joystick_report));
    *result = new3doStickReport();


    result->v_pos = (report.ABS_Y + 32768) >> 8;
    result->h_pos = (report.ABS_X + 32768) >> 8;
    if (report.ABS_Z >= 0)
      result->d_pos = report.ABS_Z;
    else
      result->d_pos = report.ABS_Z + 255;
    if (report.ABS_RZ >= 0)
        result->FIRE = 0;
      else
        result->FIRE = 1;
    result->up = report.BTN_TRIGGER_HAPPY3 || (report.ABS_RY > 22500);
    result->down = report.BTN_TRIGGER_HAPPY4 || (report.ABS_RY < -22500);
    result->left = report.BTN_TRIGGER_HAPPY1 || (report.ABS_RX < -22500);
    result->right = report.BTN_TRIGGER_HAPPY2 || (report.ABS_RX > 22500);
    result->X = report.BTN_START;
    result->P = report.BTN_SELECT || report.BTN_Y;
    result->A = report.BTN_X;
    result->B = report.BTN_A;
    result->C = report.BTN_B;
    result->L = report.BTN_TL || (report.BTN_TL2 != 0);
    result->R = report.BTN_TR || (report.BTN_TR2 != 0);
    *res = (void *)(result);
  }

  return true;
}