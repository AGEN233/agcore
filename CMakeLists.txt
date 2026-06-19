set(AGCORE_SRCS)

set(AGCORE_INCLUDE_DIRS
    "."
)

set(AGCORE_REQUIRES
    esp_timer
    heap
)

include(${CMAKE_CURRENT_LIST_DIR}/PrintfLog/CMakeLists.txt)
include(${CMAKE_CURRENT_LIST_DIR}/PixelDriver/CMakeLists.txt)
include(${CMAKE_CURRENT_LIST_DIR}/BLE/CMakeLists.txt)
include(${CMAKE_CURRENT_LIST_DIR}/Utils/CMakeLists.txt)
include(${CMAKE_CURRENT_LIST_DIR}/Version/CMakeLists.txt)

list(APPEND AGCORE_SRCS
    "agcore.c"
    "DataChannel/agcore_data_queue.c"
    "DataChannel/agcore_dataevent_route.c"
)

list(APPEND AGCORE_INCLUDE_DIRS
    "DataChannel"
)

if(AGCORE_SRCS)
    idf_component_register(
        SRCS ${AGCORE_SRCS}
        INCLUDE_DIRS ${AGCORE_INCLUDE_DIRS}
        REQUIRES ${AGCORE_REQUIRES}
    )
else()
    idf_component_register(
        INCLUDE_DIRS ${AGCORE_INCLUDE_DIRS}
        REQUIRES ${AGCORE_REQUIRES}
    )
endif()
