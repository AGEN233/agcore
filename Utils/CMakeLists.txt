target_sources(${COMPONENT_LIB} PRIVATE
    "agcore_utils.c"
    "agcore_utils_bytes.c"
)

target_include_directories(${COMPONENT_LIB} PUBLIC
    "."
)
