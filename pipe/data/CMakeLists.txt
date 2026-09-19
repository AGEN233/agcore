target_sources(${COMPONENT_LIB} PRIVATE
    "agcore_data_queue.c"
    "agcore_data_route.c"
    "agcore_core_cmd.c"
    "protocol/agcore_data_protocol.c"
)

target_include_directories(${COMPONENT_LIB} PUBLIC
    "inc"
    "protocol/inc"
)
