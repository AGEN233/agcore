target_sources(${COMPONENT_LIB} PRIVATE
    "display_driver.c"
    "display7735/display7735.c"
    "display7789/display7789.c"
)

target_include_directories(${COMPONENT_LIB} PUBLIC
    "."
    "display7735"
    "display7789"
)
