if(CONFIG_AGCORE_BLE_ENABLE)
    list(APPEND AGCORE_SRCS
        "pipe/transport/ble/agcore_ble.c"
        "pipe/transport/ble/agcore_ble_adv.c"
        "pipe/transport/ble/agcore_ble_gap.c"
        "pipe/transport/ble/agcore_ble_gatt.c"
        "pipe/transport/ble/agcore_ble_pipe.c"
    )

    list(APPEND AGCORE_INCLUDE_DIRS
        "pipe/transport/ble/inc"
    )
endif()
