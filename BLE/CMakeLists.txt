target_sources(${COMPONENT_LIB} PRIVATE
    "agcore_ble.c"
    "agcore_ble_adv.c"
    "agcore_ble_data.c"
    "agcore_ble_gap.c"
    "agcore_ble_gatt.c"
)

target_include_directories(${COMPONENT_LIB} PUBLIC
    "."
)
