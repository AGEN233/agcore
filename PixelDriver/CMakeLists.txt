list(APPEND AGCORE_SRCS
    "PixelDriver/pixel_ws2812b.c"
)

list(APPEND AGCORE_INCLUDE_DIRS
    "PixelDriver"
)

list(APPEND AGCORE_REQUIRES
    esp_driver_rmt
    esp_driver_spi
)
