#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"

#include "3do_interface.h"

static char event_str[128];
static void gpio_cb(uint gpio, uint32_t events);

#define PS_PIN 2 //Data out from 3do
#define CLK_PIN 3 //Clk from 3do
#define DATA_IN_PIN 4 //Data pin from next controlers
#define DATA_OUT_PIN 6 //Data to 3do

static _3do_report currentReport; //Only one controller at a time for the moment
static uint16_t transmitReport; //Only one controller at a time for the moment

typedef enum {
  STORE_MODE = 0,
  PUSH_MODE
} transmitMode;

enum {
  LEVEL_LOW  = (1<<0),
  LEVEL_HIGH = (1<<1),
  EDGE_FALL  = (1<<2),
  EDGE_RISE  = (1<<3)
} gpioMode;

static transmitMode mode;

void interface_3do_init() {
  stdio_init_all();

  gpio_set_dir(CLK_PIN, false); //Input
  gpio_set_dir(PS_PIN, false); //Input
  gpio_set_dir(DATA_IN_PIN, false); //Input
  gpio_set_dir(DATA_OUT_PIN, true); //Output
  gpio_pull_up(DATA_IN_PIN);

  gpio_set_irq_enabled_with_callback(PS_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &gpio_cb);
  gpio_set_irq_enabled(CLK_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true);
  gpio_pull_up(DATA_OUT_PIN);
  mode = STORE_MODE;
  memset((void*)&currentReport, 0xFF, 2);
}

static void gpio_event_string(char *buf, uint32_t events);

static void clk_cb(uint32_t events) {
    // Put the GPIO event(s) that just happened into event_str
    // so we can print it
    if ((events & EDGE_RISE) && (mode == PUSH_MODE)) {
      gpio_put(DATA_OUT_PIN, transmitReport & 0x1);
      // printf("send %d\n", transmitReport & 0x1);
      transmitReport >>= 1;
      transmitReport |= (gpio_get(DATA_IN_PIN) << 15);
    }
    // gpio_event_string(event_str, events);
    // LOG_3DO("CLK %s\n", event_str);
}

static void ps_cb(uint32_t events) {
  if (events & EDGE_FALL) {
    memcpy(&transmitReport, &currentReport, 2);
    // printf("Got 0x%x\n", transmitReport);
    mode = PUSH_MODE;
  }
  if (events & EDGE_RISE) {
    mode = STORE_MODE;
    // printf("On Hold\n");
    gpio_put(DATA_OUT_PIN, true);
  }
  // gpio_event_string(event_str, events);
  // LOG_3DO("PS %s\n", event_str);
}

static void gpio_cb(uint gpio, uint32_t events) {
  if (gpio == PS_PIN) {
    ps_cb(events);
  }
  if (gpio == CLK_PIN) {
    clk_cb(events);
  }
}


void update_3do_status(_3do_report report) {
  uint16_t report_value;
  memcpy(&report_value, &report, 2);
  printf("Report is %x\n", report_value);
  currentReport = report; //Would be better to get a pointer shared per controller
}


static const char *gpio_irq_str[] = {
        "LEVEL_LOW",  // 0x1
        "LEVEL_HIGH", // 0x2
        "EDGE_FALL",  // 0x4
        "EDGE_RISE"   // 0x8
};

static void gpio_event_string(char *buf, uint32_t events) {
    for (uint i = 0; i < 4; i++) {
        uint mask = (1 << i);
        if (events & mask) {
            // Copy this event string into the user string
            const char *event_str = gpio_irq_str[i];
            while (*event_str != '\0') {
                *buf++ = *event_str++;
            }
            events &= ~mask;

            // If more events add ", "
            if (events) {
                *buf++ = ',';
                *buf++ = ' ';
            }
        }
    }
    *buf++ = '\0';
}