#ifndef _8_BIT_DO_H_
#define _8_BIT_DO_H_

#include "3DO.h"
#include "xbox360_gamepads.h"

extern bool map_8bitDo(void* report_p, uint8_t len, uint8_t dev_addr, uint8_t instance, uint8_t *controler_id, controler_type* type, void** res);

#endif