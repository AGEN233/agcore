add_library(agcore_initcall STATIC IMPORTED GLOBAL)
set_target_properties(agcore_initcall PROPERTIES
    IMPORTED_LOCATION "${CMAKE_CURRENT_LIST_DIR}/initcall/initcall.a"
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_LIST_DIR}/initcall"
)

idf_build_get_property(agcore_target_arch IDF_TARGET_ARCH)
if(agcore_target_arch STREQUAL "riscv")
    set(agcore_security_archive "${CMAKE_CURRENT_LIST_DIR}/security/esp32_riscv_security.a")
elseif(agcore_target_arch STREQUAL "xtensa")
    set(agcore_security_archive "${CMAKE_CURRENT_LIST_DIR}/security/esp32_xtensa_security.a")
else()
    message(FATAL_ERROR "Unsupported AGCORE Security architecture: ${agcore_target_arch}")
endif()

if(NOT EXISTS "${agcore_security_archive}")
    message(FATAL_ERROR "AGCORE Security archive not found: ${agcore_security_archive}")
endif()

add_library(agcore_security STATIC IMPORTED GLOBAL)
set_target_properties(agcore_security PROPERTIES
    IMPORTED_LOCATION "${agcore_security_archive}"
)
target_link_libraries(agcore_security INTERFACE
    agcore_initcall
    idf::esp_hw_support
    idf::esp_partition
    idf::mbedtls
    idf::spi_flash
)

target_link_libraries(${COMPONENT_LIB} PUBLIC
    agcore_security
)
