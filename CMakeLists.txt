set(AGCORE_TOOLS_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/tools")
set(AGCORE_TOOLS_OUTPUT_DIR "${PROJECT_DIR}/tools")

file(MAKE_DIRECTORY "${AGCORE_TOOLS_OUTPUT_DIR}")
file(COPY_FILE
    "${AGCORE_TOOLS_SOURCE_DIR}/fw_pack.sh"
    "${AGCORE_TOOLS_OUTPUT_DIR}/fw_pack.sh"
    ONLY_IF_DIFFERENT
)
file(COPY_FILE
    "${AGCORE_TOOLS_SOURCE_DIR}/agcore_skey_build"
    "${AGCORE_TOOLS_OUTPUT_DIR}/agcore_skey_build"
    ONLY_IF_DIFFERENT
)

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
    esp_hw_support
    nvs_flash
    esp_partition
    mbedtls
    spi_flash
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
    LDFRAGMENTS "common/bootstrap/initcall/initcall.lf"
    WHOLE_ARCHIVE
)

add_subdirectory(common/console)
add_subdirectory(common/bootstrap)
add_subdirectory(common/lifecycle)
add_subdirectory(common/persist)
add_subdirectory(common/utils)
add_subdirectory(common/version)
add_subdirectory(pipe/data)
add_subdirectory(transport/ble)
add_subdirectory(driver/pixel)
add_subdirectory(driver/display)
