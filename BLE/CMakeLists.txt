list(APPEND AGCORE_SRCS
    "BLE/agcore_ble.c"
    "BLE/agcore_ble_adv.c"
    "BLE/agcore_ble_data.c"
    "BLE/agcore_ble_gap.c"
    "BLE/agcore_ble_gatt.c"
)

list(APPEND AGCORE_INCLUDE_DIRS
    "BLE"
)

list(APPEND AGCORE_REQUIRES
    bt
)
