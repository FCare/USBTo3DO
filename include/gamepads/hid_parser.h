#ifndef _HID_PARSER_H_
#define _HID_PARSER_H_

#define HID_GENERIC_DESKTOP_CONTROLS  0x1

#define HID_MOUSE_USAGE 0x2
#define HID_JOYSTICK_USAGE  0x4
#define HID_JOYPAD_USAGE 0x5

#define HID_APPLICATION_COLLECTION 0x1
#define HID_LOGICAL_COLLECTION 0x2

#define HID_USAGE_TAG 0x0
#define HID_LOGICAL_MINIMUM_TAG 0x1
#define HID_LOGICAL_MAXIMUM_TAG 0x2
#define HID_PHYSICAL_MINIMUM_TAG 0x3
#define HID_PHYSICAL_MAXIMUM_TAG 0x4
#define HID_UNIT_TAG 0x6
#define HID_REPORT_SIZE_TAG 0x7
#define HID_INPUT_TAG 0x8
#define HID_REPORT_COUNT_TAG 0x9
#define HID_PUSH_TAG 0xA
#define HID_POP_TAG 0xB

#define HID_BUTTON_PAGE 0x9
#define HID_COLLECTION_TAG 0xA
#define HID_END_COLLECTION_TAG 0xC
#define HID_VENDOR_PAGE 0xFF00

#define HID_TYPE_MAIN 0x0
#define HID_TYPE_GLOBAL 0x1
#define HID_TYPE_LOGICAL 0x2
#define HID_TYPE_LOCAL 0x2


#define HID_X 0x30
#define HID_Y 0x31
#define HID_Z 0x32
#define HID_RX 0x33
#define HID_RY 0x34
#define HID_RZ 0x35
#define HID_SLIDER 0x36
#define HID_DIAL 0x37
#define HID_WHEEL 0x38
#define HID_HAT_SWITCH 0x39


#define NB_EVENT_MAX 64

typedef enum{
  TYPE_NONE = 0,
  TYPE_JOYSTICK,
  TYPE_JOYPAD,
  TYPE_MOUSE,
} input_type;

typedef enum{
  BTN_TRIGGER = 0,
  BTN_THUMB,
  BTN_THUMB2,
  BTN_TOP,
  BTN_TOP2,
  BTN_PINKIE,
  BTN_BASE,
  BTN_BASE2,
  BTN_BASE3,
  BTN_BASE4,
  BTN_BASE5,
  BTN_BASE6,
  BTN_HAT_UP,
  BTN_HAT_DOWN,
  BTN_HAT_RIGHT,
  BTN_HAT_LEFT,
} event_key;

typedef struct {
  uint8_t begin;
  uint8_t end;
  event_key key;
  float conversion;
} hid_event;

typedef struct {
  input_type type;
  uint8_t nb_events;
  hid_event events[NB_EVENT_MAX];
} hid_mapping;


typedef enum {
  UNKNOWN_USAGE = 0,
  BUTTON_USAGE,
  BUTTON_PAGE_USAGE,
} usage_enum;

typedef struct usage_s{
  usage_enum type;
  int begin;
  int end;
  int size;
  int count;
  int value;
  int offset;
  struct usage_s *next;
  struct usage_s *prev;
} usage;


extern void parse_hid_descriptor(uint8_t const* desc_report, uint16_t desc_len, hid_mapping* mapping);

#endif