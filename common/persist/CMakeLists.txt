target_sources(${COMPONENT_LIB} PRIVATE
    "agcore_persist.c"
    "agcore_nvs.c"
    "agcore_flash.c"
    "agcore_fs.c"
)

target_include_directories(${COMPONENT_LIB} PUBLIC
    "inc"
)
