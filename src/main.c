#include "USB.h" //USB Setup
#include "3DO.h" //3DO Setup

#include <stdio.h>
#include "pico/stdlib.h"

int main(void)
{
	USB_Host_init();
	_3DO_init();
	TU_LOG1("3DO to USB\r\n");

	while(1) {
		// LCD_loop();
		USB_Host_loop();
	}

	// LCD_0in96_deinit();
  return 0;
}
