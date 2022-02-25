cmake_minimum_required(VERSION 3.13)
include(pico-sdk/pico_sdk_init.cmake)

set(FAMILY rp2040)
set(BOARD pico_sdk)
set(PICO_TINYUSB_PATH "${CMAKE_CURRENT_SOURCE_DIR}/tinyusb")

set(PICO_BOARD waveshare_rp2040_zero)

include(${PICO_TINYUSB_PATH}/hw/bsp/${FAMILY}/family.cmake)

set(CFG_TUSB_CONFIG_FILE "include/tusb_config.h")


project(USB_LCD_Pico)

add_definitions(-DPICO_TINYUSB_PATH=${PICO_TINYUSB_PATH})

pico_sdk_init()

include_directories(./include)
include_directories(./include/controllers)
include_directories(./include/interfaces)
include_directories(./include/gamepads)


add_executable(USB_LCD_Pico
src/main.c
src/interfaces/USB.c
src/interfaces/3DO.c
src/gamepads/vendor_gamepad.c
src/gamepads/hid_gamepad.c
src/controllers/8bitdo.c
src/controllers/dragonrise.c
)

# enable usb output, disable uart output
# pico_enable_stdio_usb(USB_LCD_Pico 1)
pico_enable_stdio_uart(USB_LCD_Pico 1)

# create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(USB_LCD_Pico)

target_link_libraries(USB_LCD_Pico pico_stdlib pico_multicore tinyusb_board tinyusb_host)