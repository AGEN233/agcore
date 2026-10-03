# AGCORE 工具目录
set(AGCORE_TOOLS_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/tools")
set(AGCORE_TOOLS_OUTPUT_DIR "${PROJECT_DIR}/tools")

# 将 AGCORE 工具复制到项目 tools 目录
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

# 统一收集模块源码、头文件目录和依赖
set(AGCORE_ROOT_DIR "${CMAKE_CURRENT_LIST_DIR}")
set(AGCORE_SRCS)
set(AGCORE_INCLUDE_DIRS)
set(AGCORE_REQUIRES)
set(AGCORE_PRIVATE_INCLUDE_DIRS)
set(AGCORE_LDFRAGMENTS)

# 公共模块
include("${CMAKE_CURRENT_LIST_DIR}/common/console/CMakeLists.txt")
include("${CMAKE_CURRENT_LIST_DIR}/common/bootstrap/CMakeLists.txt")
include("${CMAKE_CURRENT_LIST_DIR}/common/lifecycle/CMakeLists.txt")
include("${CMAKE_CURRENT_LIST_DIR}/common/persist/CMakeLists.txt")
include("${CMAKE_CURRENT_LIST_DIR}/common/utils/CMakeLists.txt")
include("${CMAKE_CURRENT_LIST_DIR}/common/version/CMakeLists.txt")

# 数据管道
include("${CMAKE_CURRENT_LIST_DIR}/pipe/data/CMakeLists.txt")
include("${CMAKE_CURRENT_LIST_DIR}/pipe/transport/ble/CMakeLists.txt")

# 设备驱动
include("${CMAKE_CURRENT_LIST_DIR}/driver/pixel/CMakeLists.txt")
include("${CMAKE_CURRENT_LIST_DIR}/driver/display/CMakeLists.txt")

# 根据当前构建环境加载对应平台，完成最终注册
if(ESP_PLATFORM)
    include("${CMAKE_CURRENT_LIST_DIR}/platform/espidf/CMakeLists.txt")
else()
    message(FATAL_ERROR "AGCORE: unsupported platform")
endif()
