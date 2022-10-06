#ifndef _THRUSTMASTER_H_
#define _THRUSTMASTER_H_

#include "3DO.h"

extern bool map_t80_pc(uint8_t instance, uint8_t *id, controler_type *type, void **res, void *ctrl_v);
extern bool map_hotas_x_pc(uint8_t instance, uint8_t *id, controler_type *type, void **res, void *ctrl_v);
extern bool map_hotas_x_ps3(uint8_t instance, uint8_t *id, controler_type *type, void **res, void *ctrl_v);
extern bool map_hotas_x_flight_pc(uint8_t instance, uint8_t *id, controler_type *type, void **res, void *ctrl_v);
#endif