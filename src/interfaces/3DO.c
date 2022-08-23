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


uint16_t transmitReport[MAX_CONTROLERS]; //Only one controller at a time for the moment
uint8_t nbBits = 0;
uint8_t nbExternalBits = 0;

volatile bool updateReport = false;
uint16_t currentReport[MAX_CONTROLERS] = {0xFFFF};
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
  CHAN_INPUT,
  CHAN_MAX
} DMA_chan_t;

int channel[CHAN_MAX];
uint instr_jmp[CHAN_MAX];

uint max_usb_controller = 0;

dma_channel_config config[CHAN_MAX];

uint8_t controler_buffer[201] = {0xFF};


void startDMA(uint8_t access, uint8_t *buffer, uint32_t nbWord) {
  if (access == CHAN_OUTPUT)
    dma_channel_transfer_from_buffer_now(channel[access], buffer, nbWord);
  if (access == CHAN_INPUT)
    dma_channel_transfer_to_buffer_now(channel[access], buffer, nbWord);
}

void setupDMAOutput() {
  channel[CHAN_OUTPUT] = dma_claim_unused_channel(true);
  config[CHAN_OUTPUT] = dma_channel_get_default_config(channel[CHAN_OUTPUT]);
  channel_config_set_transfer_data_size(&config[CHAN_OUTPUT], DMA_SIZE_8);
  channel_config_set_read_increment(&config[CHAN_OUTPUT], true);
  channel_config_set_write_increment(&config[CHAN_OUTPUT], false);
  channel_config_set_irq_quiet(&config[CHAN_OUTPUT], true);
  channel_config_set_dreq(&config[CHAN_OUTPUT], DREQ_PIO0_TX0 + CHAN_OUTPUT);
  dma_channel_set_write_addr(channel[CHAN_OUTPUT], &pio0->txf[CHAN_OUTPUT], false);
  dma_channel_set_config(channel[CHAN_OUTPUT], &config[CHAN_OUTPUT], false);

  bus_ctrl_hw->priority = BUSCTRL_BUS_PRIORITY_DMA_W_BITS | BUSCTRL_BUS_PRIORITY_DMA_R_BITS;
}

void setupDMAInput() {
  channel[CHAN_INPUT] = dma_claim_unused_channel(true);
  config[CHAN_INPUT] = dma_channel_get_default_config(channel[CHAN_INPUT]);
  channel_config_set_transfer_data_size(&config[CHAN_INPUT], DMA_SIZE_8);
  channel_config_set_read_increment(&config[CHAN_INPUT], false);
  channel_config_set_write_increment(&config[CHAN_INPUT], true);
  channel_config_set_irq_quiet(&config[CHAN_INPUT], true);
  channel_config_set_dreq(&config[CHAN_INPUT], DREQ_PIO0_RX0 + CHAN_OUTPUT);
  dma_channel_set_read_addr(channel[CHAN_INPUT], &pio0->rxf[CHAN_OUTPUT], false);
  dma_channel_set_config(channel[CHAN_INPUT], &config[CHAN_INPUT], false);

  bus_ctrl_hw->priority = BUSCTRL_BUS_PRIORITY_DMA_W_BITS | BUSCTRL_BUS_PRIORITY_DMA_R_BITS;
}

// print_dma_ctrl(dma_channel_hw_addr(channel[CHAN_INPUT]));
void print_dma_ctrl(dma_channel_hw_t *channel) {
    uint32_t ctrl = channel->ctrl_trig;
    int rgsz = (ctrl & DMA_CH0_CTRL_TRIG_RING_SIZE_BITS) >> DMA_CH0_CTRL_TRIG_RING_SIZE_LSB;
    printf("(%08x) ber %d rer %d wer %d busy %d trq %d cto %d rgsl %d rgsz %d inw %d inr %d sz %d hip %d en %d\n",
           (uint) ctrl,
           ctrl & DMA_CH0_CTRL_TRIG_AHB_ERROR_BITS ? 1 : 0,
           ctrl & DMA_CH0_CTRL_TRIG_READ_ERROR_BITS ? 1 : 0,
           ctrl & DMA_CH0_CTRL_TRIG_WRITE_ERROR_BITS ? 1 : 0,
           ctrl & DMA_CH0_CTRL_TRIG_BUSY_BITS ? 1 : 0,
           (int) ((ctrl & DMA_CH0_CTRL_TRIG_TREQ_SEL_BITS) >> DMA_CH0_CTRL_TRIG_TREQ_SEL_LSB),
           (int) ((ctrl & DMA_CH0_CTRL_TRIG_CHAIN_TO_BITS) >> DMA_CH0_CTRL_TRIG_CHAIN_TO_LSB),
           ctrl & DMA_CH0_CTRL_TRIG_RING_SEL_BITS ? 1 : 0,
           rgsz ? (1 << rgsz) : 0,
           ctrl & DMA_CH0_CTRL_TRIG_INCR_WRITE_BITS ? 1 : 0,
           ctrl & DMA_CH0_CTRL_TRIG_INCR_READ_BITS ? 1 : 0,
           1 << ((ctrl & DMA_CH0_CTRL_TRIG_DATA_SIZE_BITS) >> DMA_CH0_CTRL_TRIG_DATA_SIZE_LSB),
           ctrl & DMA_CH0_CTRL_TRIG_HIGH_PRIORITY_BITS ? 1 : 0,
           ctrl & DMA_CH0_CTRL_TRIG_EN_BITS ? 1 : 0);
}

volatile uint8_t external_buffer = 0;

void core1_entry() {
  // while(1) {
  //   uint32_t val = pio_sm_get_blocking(pio0, sm_output);
  //   printf("%d => val %x\n", external_buffer, val);
  //   controler_buffer[external_buffer++] = (uint8_t)val;
  // }
}

// pio0 interrupt handler
void on_pio0_irq() {
  updateReport = true;
  external_buffer = 2;
  dma_channel_abort(channel[CHAN_OUTPUT]);
  dma_channel_abort(channel[CHAN_INPUT]);
  pio_sm_drain_tx_fifo(pio0, sm_output);
  while (!pio_sm_is_rx_fifo_empty(pio0, sm_output)) pio_sm_get(pio0, sm_output);
  pio_sm_restart(pio0, sm_output);
  pio_sm_exec(pio0, sm_output, instr_jmp[sm_output]);

  memcpy(&controler_buffer[0], &currentReport[0], max_usb_controller*sizeof(_3do_joypad_report));
  startDMA(CHAN_OUTPUT, &controler_buffer[0], 201);
  pio_sm_set_enabled(pio0, sm_output, true);
  startDMA(CHAN_INPUT, &controler_buffer[(max_usb_controller+1)*sizeof(_3do_joypad_report)], 199);
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

  instr_jmp[sm_output] = pio_encode_jmp(offset);
  setupDMAOutput();
  setupDMAInput();
  pio_gpio_init(pio0, DATA_IN_PIN);
  gpio_pull_up(DATA_IN_PIN);
  pio_sm_set_consecutive_pindirs(pio0, sm_output, DATA_IN_PIN, 1, false);
  pio_gpio_init(pio0, DATA_OUT_PIN);
  pio_sm_set_consecutive_pindirs(pio0, sm_output, DATA_OUT_PIN, 1, true);
  multicore_launch_core1(core1_entry);
}


void update_3do_joypad(_3do_joypad_report report, uint8_t instance) {
  if (instance >= MAX_CONTROLERS) return;
  uint16_t report_value;
  memcpy(&report_value, &report, 2);
  currentReport[instance] = report_value;
  deviceAttached[instance] = true;
  max_usb_controller = (max_usb_controller < (instance+1))?(instance+1):max_usb_controller;
}

_3do_joypad_report new3doPadReport() {
  _3do_joypad_report report;
  report.id = 0b100;
  report.tail = 0b00;
  return report;
}


