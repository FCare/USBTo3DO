#include "LCD_Test.h" //LCD setup
#include "USB_Host.h" //USB Setup

int main(void)
{

	LCD_0in96_init();
	USB_Host_init();

	while(1) {
		USB_Host_loop();
	}

	LCD_0in96_deinit();
  return 0;
}
