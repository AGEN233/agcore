# ESP-IDF 平台源码
list(APPEND AGCORE_SRCS
    "platform/espidf/agcore_port_espidf.c"
)

# ESP-IDF 平台头文件目录
list(APPEND AGCORE_INCLUDE_DIRS
    "."
    "platform"
    "platform/espidf"
)

# ESP-IDF 平台依赖
list(APPEND AGCORE_REQUIRES
    # dependency scan 阶段尚未加载 CONFIG，BLE 依赖必须在此声明。
    bt
    esp_timer
    esp_hw_support
    nvs_flash
    esp_partition
    mbedtls
    spi_flash
    spiffs
    heap
    esp_driver_gpio
    esp_driver_rmt
    esp_driver_spi
    esp_driver_uart
    esp_lcd
    freertos
)

# ESP-IDF 对 include/fragment 的相对路径解析基于注册文件所在目录。
# 模块按 AGCORE 根目录收集路径，在注册前统一转换为绝对路径。
foreach(agcore_path_list IN ITEMS
        AGCORE_SRCS AGCORE_INCLUDE_DIRS AGCORE_PRIVATE_INCLUDE_DIRS AGCORE_LDFRAGMENTS)
    set(agcore_absolute_paths)
    foreach(agcore_path IN LISTS ${agcore_path_list})
        get_filename_component(agcore_absolute_path "${agcore_path}"
            ABSOLUTE BASE_DIR "${AGCORE_ROOT_DIR}")
        list(APPEND agcore_absolute_paths "${agcore_absolute_path}")
    endforeach()
    set(${agcore_path_list} ${agcore_absolute_paths})
endforeach()

# 注册 AGCORE ESP-IDF 组件
idf_component_register(
    SRCS ${AGCORE_SRCS}
    INCLUDE_DIRS ${AGCORE_INCLUDE_DIRS}
    PRIV_INCLUDE_DIRS ${AGCORE_PRIVATE_INCLUDE_DIRS}
    REQUIRES ${AGCORE_REQUIRES}
    LDFRAGMENTS ${AGCORE_LDFRAGMENTS}
    WHOLE_ARCHIVE
)

# 定义当前 AGCORE 平台
target_compile_definitions(${COMPONENT_LIB} PUBLIC
    AGCORE_PLATFORM_IS_ESPIDF=1
)
# 应用需要组件 target 的模块配置
agcore_espidf_link_bootstrap()
agcore_espidf_configure_console()
