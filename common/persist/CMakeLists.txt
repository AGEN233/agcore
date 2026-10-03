list(APPEND AGCORE_SRCS
    "common/persist/agcore_persist.c"
    "common/persist/agcore_nvs.c"
    "common/persist/agcore_flash.c"
    "common/persist/agcore_fs.c"
)

list(APPEND AGCORE_INCLUDE_DIRS
    "common/persist/inc"
)
