set(AGCORE_SRCS
    "agcore.c"
    "data_channel/agcore_data_queue.c"
    "data_channel/agcore_dataevent_route.c"
)

set(AGCORE_INCLUDE_DIRS
    "."
    "data_channel"
)

set(AGCORE_REQUIRES
    esp_timer
    heap
    bt
    esp_driver_gpio
    esp_driver_rmt
    esp_driver_spi
    esp_lcd
)

idf_component_register(
    SRCS ${AGCORE_SRCS}
    INCLUDE_DIRS ${AGCORE_INCLUDE_DIRS}
    REQUIRES ${AGCORE_REQUIRES}
)

add_subdirectory(log)
add_subdirectory(pixel_driver)
add_subdirectory(display_driver)
add_subdirectory(BLE)
add_subdirectory(Utils)
add_subdirectory(Version)
