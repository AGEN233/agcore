list(APPEND AGCORE_SRCS
    "pipe/data/agcore_data.c"
    "pipe/data/agcore_data_route.c"
    "pipe/data/agcore_data_core_cmd.c"
    "pipe/data/protocol/agcore_data_protocol.c"
)

list(APPEND AGCORE_INCLUDE_DIRS
    "pipe/data/inc"
    "pipe/data/inc/protocol"
)
