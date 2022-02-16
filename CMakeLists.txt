cmake_minimum_required(VERSION 3.13)
include(pico-sdk/pico_sdk_init.cmake)

set(FAMILY rp2040)
set(BOARD pico_sdk)
set(PICO_TINYUSB_PATH "${CMAKE_CURRENT_SOURCE_DIR}/tinyusb")
include(${PICO_TINYUSB_PATH}/hw/bsp/${FAMILY}/family.cmake)

set(CFG_TUSB_CONFIG_FILE "include/tusb_config.h")


project(USB_LCD_Pico)

add_definitions(-DPICO_TINYUSB_PATH=${PICO_TINYUSB_PATH})

pico_sdk_init()

add_subdirectory(./lib/Config)
add_subdirectory(./lib/GUI)
add_subdirectory(./lib/LCD)
add_subdirectory(./lib/Fonts)


include_directories(./include)
include_directories(./lib/Config)
include_directories(./lib/GUI)
include_directories(./lib/LCD)
include_directories(./lib/Fonts)


add_executable(USB_LCD_Pico
src/main.c
src/LCD_0in96_test.c
src/ImageData.c
src/USB_Host.c
src/hid_app.c
)

# enable usb output, disable uart output
# pico_enable_stdio_usb(USB_LCD_Pico 1)
pico_enable_stdio_uart(USB_LCD_Pico 1)

# create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(USB_LCD_Pico)

target_link_libraries(USB_LCD_Pico LCD GUI Fonts Config pico_stdlib pico_multicore hardware_spi hardware_dma tinyusb_board tinyusb_host)