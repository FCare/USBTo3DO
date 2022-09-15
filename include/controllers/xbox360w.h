#ifndef _XBOX_360_W_H_
#define _XBOX_360_W_H_

#include "3DO.h"
#include "xbox360_gamepads.h"

extern bool map_xbox360w(uint8_t* report_p, uint8_t len, uint8_t dev_addr, uint8_t instance, uint8_t *controler_id, controler_type* type, void** res);
extern bool mount_xbox360w(uint8_t dev_addr, uint8_t instance);
extern void led_xbox360w(void);

#endif