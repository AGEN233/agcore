list(APPEND AGCORE_SRCS
    "driver/display/display_driver.c"
    "driver/display/panel/st7735/display_st7735.c"
    "driver/display/panel/st7789/display_st7789.c"
)

list(APPEND AGCORE_INCLUDE_DIRS
    "driver/display/inc"
    "driver/display/panel/st7735"
    "driver/display/panel/st7789"
)
