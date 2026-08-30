target_sources(${COMPONENT_LIB} PRIVATE
    "src/agcore_ble.c"
    "src/agcore_ble_adv.c"
    "src/agcore_ble_gap.c"
    "src/agcore_ble_gatt.c"
    "src/agcore_ble_pipe.c"
)

target_include_directories(${COMPONENT_LIB} PUBLIC
    "inc"
)
