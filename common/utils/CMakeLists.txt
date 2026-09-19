target_sources(${COMPONENT_LIB} PRIVATE
    "agcore_check.c"
    "agcore_bytes.c"
)

target_include_directories(${COMPONENT_LIB} PUBLIC
    "inc"
)
