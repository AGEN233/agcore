target_sources(${COMPONENT_LIB} PRIVATE
    "pixel_ws2812b.c"
)

target_include_directories(${COMPONENT_LIB} PUBLIC
    "."
)
