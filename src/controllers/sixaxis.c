#include "bsp/board.h"

#include <stdlib.h>
#include <math.h>

#include "sixaxis.h"

struct sixaxis_led {
	uint8_t time_enabled; /* the total time the led is active (0xff means forever) */
	uint8_t duty_length;  /* how long a cycle is in deciseconds (0 means "really fast") */
	uint8_t enabled;
	uint8_t duty_off; /* % of duty_length the led is off (0xff means 100%) */
	uint8_t duty_on;  /* % of duty_length the led is on (0xff mean 100%) */
} __packed;

struct sixaxis_rumble {
	uint8_t padding;
	uint8_t right_duration; /* Right motor duration (0xff means forever) */
	uint8_t right_motor_on; /* Right (small) motor on/off, only supports values of 0 or 1 (off/on) */
	uint8_t left_duration;    /* Left motor duration (0xff means forever) */
	uint8_t left_motor_force; /* left (large) motor, supports force values from 0 to 255 */
} __packed;

struct sixaxis_output_report {
	struct sixaxis_rumble rumble;
	uint8_t padding[4];
	uint8_t leds_bitmap; /* bitmap of enabled LEDs: LED_1 = 0x02, LED_2 = 0x04, ... */
	struct sixaxis_led led[4];    /* LEDx at (4 - x) */
	struct sixaxis_led _reserved; /* LED5, not actually soldered */
} __packed;

union sixaxis_output_report_01 {
	struct sixaxis_output_report data;
	uint8_t buf[36];
};

bool map_sixaxis(uint8_t instance, uint8_t *id, controler_type *type, void **res, void *ctrl_v)
{
  hid_controller *ctrl =(hid_controller*)ctrl_v;
  _3do_joypad_report *result = malloc(sizeof(_3do_joypad_report));
  *result = new3doPadReport();
  *type = JOYPAD;
  *id = ctrl->index + instance;
  hid_buttons *btn = & ctrl->buttons[ctrl->index];

  result->up = btn->TOP2 || (btn->ABS_Y < 64);
  result->down = btn->BASE || (btn->ABS_Y > 192);
  result->left = btn->BASE2 || (btn->ABS_X < 64);
  result->right = btn->PINKIE || (btn->ABS_X > 192);
  result->X = btn->BASE7 || btn->TRIGGER;
  result->P = btn->TOP;
  result->A = btn->BASE10;
  result->B = btn->BASE9;
  result->C = btn->BASE8;
  result->L = btn->BASE5 || btn->BASE3;
  result->R = btn->BASE6 || btn->BASE4;

  if (ctrl->hasHat[ctrl->index]) {
    result->up |= btn->HAT_UP;
    result->down |= btn->HAT_DOWN;
    result->left |= btn->HAT_LEFT;
    result->right |= btn->HAT_RIGHT;
  }

  *res = (void *)(result);

  return true;
}

bool mount_sixaxis(uint8_t dev_addr, uint8_t instance) {
  //Need to send dedicated packet to start the controller
  uint8_t buffer[17];
  tuh_hid_get_report(dev_addr, instance, 0xF2, 0x3, &buffer[0], 17, NULL);
  tuh_hid_get_report(dev_addr, instance, 0xF2, 0x3, &buffer[0], 17, NULL);
  tuh_hid_get_report(dev_addr, instance, 0xF5, 0x3, &buffer[0], 8, NULL);

  //Set default report
  static const union sixaxis_output_report_01 default_report = {
  		.buf = {
  			0x01, 0xff, 0x00, 0xff, 0x00,
  			0x00, 0x00, 0x00, 0x00, 0x00,
  			0xff, 0x27, 0x10, 0x00, 0x32,
  			0xff, 0x27, 0x10, 0x00, 0x32,
  			0xff, 0x27, 0x10, 0x00, 0x32,
  			0xff, 0x27, 0x10, 0x00, 0x32,
  			0x00, 0x00, 0x00, 0x00, 0x00
  		}
  	};
  	struct sixaxis_output_report report;
  	int n;

  	/* Initialize the report with default values */
  	memcpy(&report, &default_report, sizeof(struct sixaxis_output_report));

  #ifdef CONFIG_SONY_FF
  	report.rumble.right_motor_on = sc->right ? 1 : 0;
  	report.rumble.left_motor_force = sc->left;
  #endif

  	report.leds_bitmap |= 0x1 << (instance+1);

  tuh_hid_set_report(dev_addr, instance, 0x1, 0x2, &report,sizeof(struct sixaxis_output_report));
  return true;
}