set(AGCORE_SRCS
    "agcore.c"
    "DataChannel/agcore_data_queue.c"
    "DataChannel/agcore_dataevent_route.c"
)

set(AGCORE_INCLUDE_DIRS
    "."
    "DataChannel"
)

set(AGCORE_REQUIRES
    esp_timer
    heap
    bt
    esp_driver_gpio
    esp_driver_rmt
    esp_driver_spi
)

idf_component_register(
    SRCS ${AGCORE_SRCS}
    INCLUDE_DIRS ${AGCORE_INCLUDE_DIRS}
    REQUIRES ${AGCORE_REQUIRES}
)

add_subdirectory(PrintfLog)
add_subdirectory(PixelDriver)
add_subdirectory(BLE)
add_subdirectory(Utils)
add_subdirectory(Version)
