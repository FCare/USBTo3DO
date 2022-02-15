cmake_minimum_required(VERSION 3.13)
include(pico-sdk/pico_sdk_init.cmake)

project(USB_LCD_Pico)

pico_sdk_init()

include_directories(./lib/Config)
include_directories(./lib/GUI)
include_directories(./lib/Icm20948)
include_directories(./lib/LCD)


add_executable(USB_LCD_Pico
src/main.c
)

# enable usb output, disable uart output
pico_enable_stdio_usb(USB_LCD_Pico 1)
pico_enable_stdio_uart(USB_LCD_Pico 1)

# create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(USB_LCD_Pico)

target_link_libraries(USB_LCD_Pico examples LCD Icm20948 GUI Fonts Config pico_stdlib hardware_spi)