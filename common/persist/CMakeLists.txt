target_sources(${COMPONENT_LIB} PRIVATE
    "agcore_persist.c"
    "nvs/agcore_nvs.c"
    "flash/agcore_flash.c"
    "fs/agcore_fs.c"
)

target_include_directories(${COMPONENT_LIB} PUBLIC
    "."
    "nvs"
    "flash"
    "fs"
)
