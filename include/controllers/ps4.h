#ifndef _PS4_H_
#define _PS4_H_

#include "3DO.h"
#include "hid_parser.h"

extern bool map_ds4(uint8_t instance, uint8_t *id, controler_type *type, void **res, void *ctrl);

#endif