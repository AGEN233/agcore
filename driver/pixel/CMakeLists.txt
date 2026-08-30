target_sources(${COMPONENT_LIB} PRIVATE
    "pixel_driver.c"
)

target_include_directories(${COMPONENT_LIB} PUBLIC
    "."
)
