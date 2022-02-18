#include "LCD_Test.h" //LCD setup
#include "USB_Host.h" //USB Setup

#include <stdio.h>
#include "pico/stdlib.h"

int main(void)
{
	USB_Host_init();
	interface_3do_init();
	LCD_0in96_init();

	while(1) {
		LCD_loop();
		USB_Host_loop();
	}

	LCD_0in96_deinit();
  return 0;
}
