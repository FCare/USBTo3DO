#ifndef __USB_HOST_H_INCLUDE__
#define __USB_HOST_H_INCLUDE__

#include "tusb.h"

typedef void (*key_cb)(int);

extern void USB_Host_init();

extern void USB_Host_loop();


extern void USB_Host_set_key_up_cb(key_cb key_up_cb);
extern void USB_Host_set_key_down_cb(key_cb key_up_cb);

#endif