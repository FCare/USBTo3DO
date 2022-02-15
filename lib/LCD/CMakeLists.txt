aux_source_directory(. DIR_LCD_SRCS)

include_directories(../Config)

add_library(LCD ${DIR_LCD_SRCS})
target_link_libraries(LCD PUBLIC Config)