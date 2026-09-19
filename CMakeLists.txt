set(AGCORE_SRCS
    "platform/espidf/agcore_port_espidf.c"
)

set(AGCORE_INCLUDE_DIRS
    "."
    "platform/inc"
    "platform/espidf"
)

set(AGCORE_REQUIRES
    esp_timer
    nvs_flash
    esp_partition
    spiffs
    heap
    bt
    esp_driver_gpio
    esp_driver_rmt
    esp_driver_spi
    esp_driver_uart
    esp_lcd
    freertos
)

idf_component_register(
    SRCS ${AGCORE_SRCS}
    INCLUDE_DIRS ${AGCORE_INCLUDE_DIRS}
    REQUIRES ${AGCORE_REQUIRES}
    LDFRAGMENTS "common/initcall/linker.lf"
    WHOLE_ARCHIVE
)

add_subdirectory(common/console)
add_subdirectory(common/initcall)
add_subdirectory(common/lifecycle)
add_subdirectory(common/persist)
add_subdirectory(common/utils)
add_subdirectory(common/version)
add_subdirectory(pipe/data)
add_subdirectory(transport/ble)
add_subdirectory(driver/pixel)
add_subdirectory(driver/display)
