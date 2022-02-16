#ifndef _LCD_TEST_H_
#define _LCD_TEST_H_

#include "DEV_Config.h"
#include "GUI_Paint.h"
#include "ImageData.h"
#include "Debug.h"
#include <stdlib.h> // malloc() free()

int LCD_0in96_init(void);
int LCD_0in96_deinit(void);
void LCD_loop(void);
#endif
