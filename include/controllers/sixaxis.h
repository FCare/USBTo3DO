#ifndef _SIXAXIS_H_
#define _SIXAXIS_H_

#include "3DO.h"
#include "hid_parser.h"

extern bool map_sixaxis(uint8_t instance, uint8_t *id, controler_type *type, void **res, void *ctrl);
extern bool mount_sixaxis(uint8_t dev_addr, uint8_t instance);

#endif