/*****************************************************************************
* | File      	:   LCD_0in96_test.c
* | Author      :   Waveshare team
* | Function    :
* | Info        :
*----------------
* |	This version:   V1.0
* | Date        :   2021-03-11
* | Info        :
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documnetation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to  whom the Software is
# furished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS OR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#
******************************************************************************/
#include "LCD_Test.h"
#include "LCD_0in96.h"

static UWORD *BlackImage;

static void drawImage(void) {
  Paint_DrawImage(gImage_0inch96_1,0,0,160,80);
  LCD_0IN96_Display(BlackImage);
}

static void drawText(void) {
  Paint_DrawPoint(2,2, BLACK, DOT_PIXEL_1X1,  DOT_FILL_RIGHTUP);
	Paint_DrawPoint(2,6, BLACK, DOT_PIXEL_2X2,  DOT_FILL_RIGHTUP);
	Paint_DrawPoint(2,11, BLACK, DOT_PIXEL_3X3, DOT_FILL_RIGHTUP);
	Paint_DrawPoint(2,16, BLACK, DOT_PIXEL_4X4, DOT_FILL_RIGHTUP);
	Paint_DrawPoint(2,21, BLACK, DOT_PIXEL_5X5, DOT_FILL_RIGHTUP);
	Paint_DrawLine( 10,  5, 40, 35, MAGENTA, DOT_PIXEL_2X2, LINE_STYLE_SOLID);
	Paint_DrawLine( 10, 35, 40,  5, MAGENTA, DOT_PIXEL_2X2, LINE_STYLE_SOLID);

	Paint_DrawLine( 80,  20, 110, 20, BLACK, DOT_PIXEL_1X1, LINE_STYLE_DOTTED);
	Paint_DrawLine( 95,   5,  95, 35, BLACK, DOT_PIXEL_1X1, LINE_STYLE_DOTTED);

	Paint_DrawRectangle(10, 5, 40, 35, RED, DOT_PIXEL_2X2,DRAW_FILL_EMPTY);
	Paint_DrawRectangle(45, 5, 75, 35, BLUE, DOT_PIXEL_2X2,DRAW_FILL_FULL);

	Paint_DrawCircle( 95,20, 15, RED  ,DOT_PIXEL_1X1,DRAW_FILL_EMPTY);
	Paint_DrawCircle(130,20, 15, GREEN   ,DOT_PIXEL_1X1,DRAW_FILL_FULL);

	Paint_DrawNum (23, 37 ,123.456789, &Font12,6,  0xfff0, 0x000f);
	Paint_DrawString_EN(1, 37, "ABC", &Font12, 0x000f, 0xfff0);
	Paint_DrawString_CN(1,44, "��ӭʹ��",  &Font24CN, BLUE, WHITE);

    /*3.Refresh the picture in RAM to LCD*/

    LCD_0IN96_Display(BlackImage);
}

bool reserved_addr(uint8_t addr) {
return (addr & 0x78) == 0 || (addr & 0x78) == 0x78;
}

int LCD_0in96_init(void)
{
    DEV_Delay_ms(100);
    printf("LCD_0in96_test Demo\r\n");
    if(DEV_Module_Init()!=0){
        return -1;
    }

    /* LCD Init */
    printf("0.96inch LCD demo...\r\n");
    LCD_0IN96_Init(HORIZONTAL);
    LCD_0IN96_Clear(WHITE);
    DEV_Delay_ms(1000);


    UDOUBLE Imagesize = LCD_0IN96_HEIGHT*LCD_0IN96_WIDTH*2;
    if((BlackImage = (UWORD *)malloc(Imagesize)) == NULL) {
        printf("Failed to apply for black memory...\r\n");
        exit(0);
    }
    // /*1.Create a new image cache named IMAGE_RGB and fill it with white*/
    Paint_NewImage((UBYTE *)BlackImage,LCD_0IN96.WIDTH,LCD_0IN96.HEIGHT, 0, WHITE);
    Paint_SetScale(65);
    Paint_Clear(WHITE);
    Paint_SetRotate(ROTATE_0);
    Paint_Clear(WHITE);

    LCD_0IN96_Display(BlackImage);
}
int LCD_0in96_deinit(void)
{
    /* Module Exit */
    free(BlackImage);
    BlackImage = NULL;

    DEV_Module_Exit();
    return 0;
}
