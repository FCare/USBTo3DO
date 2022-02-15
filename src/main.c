#include "LCD_Test.h" //LCD setup
#include "USB_Host.h" //USB Setup

int main(void)
{

	LCD_0in96_test();
	USB_Host_init();


	while(1) {
		USB_Host_loop();
	}

    return 0;
}
