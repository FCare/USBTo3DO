#ifndef _WII_ADAPTER_H_
#define _WII_ADAPTER_H_

#include "3DO.h"

extern bool map_wii_classic_adapter(void* report_p, uint8_t len, uint8_t dev_addr, uint8_t instance, uint8_t *controler_id, _3do_report* result);

#endif