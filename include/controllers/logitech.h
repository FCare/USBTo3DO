#ifndef _LOGITECH_H_
#define _LOGITECH_H_

#include "3DO.h"

extern bool map_logitech_extreme_pro(uint8_t* report_p, uint8_t len, uint8_t dev_addr, uint8_t instance, uint8_t *controler_id, controler_type* type, void** res);

#endif