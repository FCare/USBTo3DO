#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "pico/multicore.h"

#include "3do_interface.h"

#define CS_CTRL_PIN 6 //  Data out from 3do
#define CLK_PIN 2 // Clk from 3do
#define DATA_IN_PIN 6//Data pin from next controlers
#define DATA_OUT_PIN 3 // Data to 3do

volatile uint16_t transmitReport; //Only one controller at a time for the moment
volatile uint16_t currentReport = 0xFFFF;

volatile uint64_t lastFall = 0;

enum {
  LEVEL_LOW  = (1<<0),
  LEVEL_HIGH = (1<<1),
  EDGE_FALL  = (1<<2),
  EDGE_RISE  = (1<<3)
} gpioMode;


void core1_entry() {

    int val = 0;
    bool oldState = gpio_get(CLK_PIN);
    while (1){
      bool state = gpio_get(CLK_PIN);
      if (oldState != state) {
        oldState = state;
        if (state) {
          //Rising edge
          gpio_put(DATA_OUT_PIN, transmitReport & 0x1);
          transmitReport = (transmitReport>>1) | 0x8000;
        } else {
          //Falling edge
          uint64_t start = time_us_64();
          if ((start - lastFall) >= 800) {
            transmitReport = currentReport;
            gpio_put(DATA_OUT_PIN, transmitReport & 0x1);
            transmitReport = (transmitReport>>1) | 0x8000;
          }
          lastFall = start;
        }
      }
    }
}

void interface_3do_init() {
  stdio_init_all();
  // gpio_init(CS_CTRL_PIN);
  // gpio_set_dir(CS_CTRL_PIN, GPIO_OUT); //Output
  gpio_init(CLK_PIN);
  gpio_set_dir(CLK_PIN, GPIO_IN); //Input
  gpio_init(DATA_IN_PIN);
  gpio_set_dir(DATA_IN_PIN, GPIO_IN); //Input
  gpio_pull_up(DATA_IN_PIN);
  gpio_init(DATA_OUT_PIN);
  gpio_set_dir(DATA_OUT_PIN, GPIO_OUT); //Output
  gpio_put(DATA_OUT_PIN, 1);

  // gpio_set_drive_strength(CLK_PIN, GPIO_DRIVE_STRENGTH_12MA);
  gpio_set_drive_strength(DATA_OUT_PIN, GPIO_DRIVE_STRENGTH_12MA);

  multicore_launch_core1(core1_entry);
}


void update_3do_status(_3do_report report) {
  uint16_t report_value;
  memcpy(&report_value, &report, 2);
  printf("Report is %x\n", report_value);
  // multicore_fifo_push_blocking(report_value);
  currentReport = report_value;
}

_3do_report new3doPadReport() {
  _3do_report report;
  report.id = 0b001;
  report.tail = 0b00;
  return report;
}


