#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/dma.h"
#include "hardware/structs/bus_ctrl.h"
#include "hardware/gpio.h"
#include "hardware/irq.h"
#include "pico/multicore.h"
#include "sampling.pio.h"
#include "output.pio.h"

#include "3DO.h"

//Missing dynammic allocation of controllers

#define MAX_CONTROLERS 9


uint sm_sampling = 0;
uint sm_output = 0;

uint instr_jmp;

uint16_t transmitReport[MAX_CONTROLERS]; //Only one controller at a time for the moment
uint8_t nbBits = 0;
uint8_t nbExternalBits = 0;

volatile bool updateReport = false;
volatile uint16_t currentReport[MAX_CONTROLERS] = {0xFFFF};
volatile bool deviceAttached[MAX_CONTROLERS] = {false};
volatile bool deviceReported[MAX_CONTROLERS] = {false};
volatile uint8_t externalControllerId = MAX_CONTROLERS;

enum {
  LEVEL_LOW  = (1<<0),
  LEVEL_HIGH = (1<<1),
  EDGE_FALL  = (1<<2),
  EDGE_RISE  = (1<<3)
} gpioMode;


typedef enum {
  CHAN_OUTPUT = 0,
  CHAN_MAX
} DMA_chan_t;

int channel[CHAN_MAX];

dma_channel_config config[CHAN_MAX];

uint8_t controler_buffer[201] = {0};


void startDMA(uint8_t access, uint8_t *buffer, uint32_t nbWord) {
  dma_channel_transfer_from_buffer_now(channel[access], buffer, nbWord);
}

void setupDMA(uint8_t access) {
  channel[access] = dma_claim_unused_channel(true);
  config[access] = dma_channel_get_default_config(channel[access]);
  channel_config_set_transfer_data_size(&config[access], DMA_SIZE_8);
  channel_config_set_read_increment(&config[access], true);
  channel_config_set_write_increment(&config[access], false);
  channel_config_set_irq_quiet(&config[access], true);
  channel_config_set_dreq(&config[access], DREQ_PIO0_TX0 + access);
  dma_channel_set_write_addr(channel[access], &pio0->txf[sm_output], false);
  dma_channel_set_config(channel[access], &config[access], false);

  bus_ctrl_hw->priority = BUSCTRL_BUS_PRIORITY_DMA_W_BITS | BUSCTRL_BUS_PRIORITY_DMA_R_BITS;
}

void core1_entry() {

}

// pio0 interrupt handler
void on_pio0_irq() {
  updateReport = true;
  dma_channel_abort(CHAN_OUTPUT);
  pio_sm_drain_tx_fifo(pio0, sm_output);
  pio_sm_restart(pio0, sm_output);
  pio_sm_exec(pio0, sm_output, instr_jmp);
  pio_sm_set_enabled(pio0, sm_output, true);

  memcpy(&controler_buffer[0], &currentReport[0], sizeof(currentReport[0]));
  // printf("Report %2x\n", controler_buffer[0]);
  startDMA(CHAN_OUTPUT, &controler_buffer[0], 201);
  pio_interrupt_clear(pio0, 0);
  irq_clear(PIO0_IRQ_0);
}

void _3DO_init() {
  uint offset;
  stdio_init_all();
  gpio_init(CLK_PIN);
  gpio_set_dir(CLK_PIN, GPIO_IN); //Input

  pio_set_irq0_source_enabled(pio0, pis_interrupt0, true);
  irq_set_exclusive_handler(PIO0_IRQ_0, on_pio0_irq);
  irq_set_enabled(PIO0_IRQ_0, true);

  sm_sampling = CHAN_MAX;
  offset = pio_add_program(pio0, &sampling_program);
  sampling_program_init(pio0, sm_sampling, offset);

  sm_output = CHAN_OUTPUT;
  offset = pio_add_program(pio0, &output_program);
  output_program_init(pio0, sm_output, offset);

  instr_jmp = pio_encode_jmp(offset);

  for (int i=0; i< CHAN_MAX; i++) {
    setupDMA(i);
  }

  pio_gpio_init(pio0, DATA_OUT_PIN);
  pio_sm_set_consecutive_pindirs(pio0, sm_output, DATA_OUT_PIN, 1, true);

  multicore_launch_core1(core1_entry);
}


void update_3do_status(_3do_report report, uint8_t instance) {
  if (instance >= MAX_CONTROLERS) return;
  uint16_t report_value;
  memcpy(&report_value, &report, 2);
  currentReport[instance] = report_value;
  deviceAttached[instance] = true;
}

_3do_report new3doPadReport() {
  _3do_report report;
  report.id = 0b001;
  report.tail = 0b00;
  return report;
}


