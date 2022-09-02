#ifndef _THRUSTMASTER_H_
#define _THRUSTMASTER_H_

#include "3DO.h"

extern bool map_t80_pc(uint8_t* report_p, uint8_t len, uint8_t dev_addr, uint8_t instance, uint8_t *controler_id, controler_type* type, void** res);
extern bool map_hotas_x_pc(uint8_t* report_p, uint8_t len, uint8_t dev_addr, uint8_t instance, uint8_t *controler_id, controler_type* type, void** res);
extern bool map_hotas_x_ps3(uint8_t* report_p, uint8_t len, uint8_t dev_addr, uint8_t instance, uint8_t *controler_id, controler_type* type, void** res);

#endif