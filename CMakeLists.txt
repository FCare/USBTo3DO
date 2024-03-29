cmake_minimum_required(VERSION 3.13)
include(pico-sdk/pico_sdk_init.cmake)

set(FAMILY rp2040)
set(BOARD pico_sdk)
set(PICO_TINYUSB_PATH "${CMAKE_CURRENT_SOURCE_DIR}/tinyusb")

set(PICO_BOARD waveshare_rp2040_zero)

include(${PICO_TINYUSB_PATH}/hw/bsp/${FAMILY}/family.cmake)

set(CFG_TUSB_CONFIG_FILE "include/tusb_config.h")


project(USBto3Do)

add_definitions(-DPICO_TINYUSB_PATH=${PICO_TINYUSB_PATH})

pico_sdk_init()

include_directories(./include)
include_directories(./include/controllers)
include_directories(./include/interfaces)
include_directories(./include/gamepads)


add_executable(USBto3Do
src/main.c
src/interfaces/USB.c
src/interfaces/3DO.c
src/gamepads/vendor_gamepad.c
src/gamepads/hid_gamepad.c
src/gamepads/hid_parser.c
src/controllers/8bitdo.c
src/controllers/xbox360w.c
src/controllers/xbox360.c
src/controllers/dragonrise.c
src/controllers/wiiadapter.c
src/controllers/retroBit.c
src/controllers/saturnAdapter.c
src/controllers/psClassic.c
src/controllers/thrustmaster.c
src/controllers/logitech.c
src/controllers/ps4.c
src/controllers/sixaxis.c
)

pico_generate_pio_header(USBto3Do ${CMAKE_CURRENT_LIST_DIR}/src/interfaces/sampling.pio)
pico_generate_pio_header(USBto3Do ${CMAKE_CURRENT_LIST_DIR}/src/interfaces/output.pio)

# enable usb output, disable uart output
pico_enable_stdio_uart(USBto3Do 1)

# create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(USBto3Do)

target_link_libraries(USBto3Do pico_stdlib hardware_pio hardware_dma pico_multicore tinyusb_board tinyusb_host)