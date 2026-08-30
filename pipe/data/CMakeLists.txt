target_sources(${COMPONENT_LIB} PRIVATE
    "src/agcore_data_queue.c"
    "src/agcore_data_route.c"
    "src/agcore_core_cmd.c"
    "protocol/src/agcore_data_protocol.c"
)

target_include_directories(${COMPONENT_LIB} PUBLIC
    "inc"
    "protocol/inc"
)
