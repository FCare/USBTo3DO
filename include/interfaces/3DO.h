#ifndef __3DO_INTERFACE_H_INCLUDE__
#define __3DO_INTERFACE_H_INCLUDE__

#define MAX_CONTROLERS 9

extern void _3DO_init();

typedef struct {
  //LSB byte
uint16_t A : 1;
uint16_t left : 1;
uint16_t right : 1;
uint16_t up : 1;
uint16_t down : 1;
uint16_t id: 3;

// MSB Byte
uint16_t tail : 2;
uint16_t C : 1;
uint16_t P : 1;
uint16_t X : 1;
uint16_t R : 1;
uint16_t L : 1;
uint16_t B : 1;

} _3do_joypad_report;

typedef struct {
uint8_t id_0;
uint8_t id_1;
uint8_t id_2;

uint8_t analog1;
uint8_t analog2;
uint8_t analog3;
uint8_t analog4;

uint8_t left  : 1;
uint8_t right : 1;
uint8_t down  : 1;
uint8_t up    : 1;
uint8_t C     : 1;
uint8_t B     : 1;
uint8_t A     : 1;
uint8_t FIRE  : 1;

uint8_t tail  : 4;
uint8_t R     : 1;
uint8_t L     : 1;
uint8_t X     : 1;
uint8_t P     : 1;
} _3do_joystick_report;

typedef enum{
  NONE = 0,
  JOYPAD,
  JOYSTICK,
} controler_type;


typedef bool (*mapper)(void *, uint8_t len, uint8_t dev_addr, uint8_t instance, uint8_t *controler_id, controler_type* type, void** res);
typedef bool (*mountFunc)(uint8_t, uint8_t );
typedef void (*tick)(void);

typedef struct {
   uint16_t vid;
   uint16_t pid;
   mapper mapper;
   mountFunc mount;
   tick tick;
 } mapping_3do;

 extern void update_3do_joypad(_3do_joypad_report report, uint8_t instance);
 extern _3do_joypad_report new3doPadReport();

 extern void update_3do_joystick(_3do_joystick_report report, uint8_t instance);
 extern _3do_joystick_report new3doStickReport();

 #define CLK_PIN 2 // Clk from 3do
 #define DATA_OUT_PIN 3 // Data to 3do
 #define DATA_IN_PIN 4 //Data pin from next controlers
 #define CS_CTRL_PIN 5 //  Data out from 3do

#endif