#include "LCD_Test.h" //LCD setup
#include "USB_Host.h" //USB Setup


static void on_key_up(int key) {

}

static void on_key_down(int key) {

}

int main(void)
{

	USB_Host_init();
	LCD_0in96_init();

	USB_Host_set_key_up_cb(on_key_up);
	USB_Host_set_key_down_cb(on_key_down);

	while(1) {
		LCD_loop();
		USB_Host_loop();
	}

	LCD_0in96_deinit();
  return 0;
}
