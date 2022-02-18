#ifndef __3DO_INTERFACE_H_INCLUDE__
#define __3DO_INTERFACE_H_INCLUDE__

extern void interface_3do_init();

#define LOG_3DO
#ifdef CFG_TUSB_DEBUG
#define LOG_3DO
#endif

typedef struct {
  uint16_t id: 3;
  uint16_t down : 1;
  uint16_t up : 1;
  uint16_t right : 1;
  uint16_t left : 1;
  uint16_t A : 1;
  uint16_t B : 1;
  uint16_t C : 1;
  uint16_t P : 1;
  uint16_t X : 1;
  uint16_t R : 1;
  uint16_t L : 1;
  uint16_t tail : 2;
} _3do_report;


typedef _3do_report (*mapper)(void *);

typedef struct {
   uint16_t vid;
   uint16_t pid;
   mapper mapper;
 } mapping_3do;

 extern void update_3do_status(_3do_report report);

#endif