target_sources(${COMPONENT_LIB} PRIVATE
    "display_driver.c"
    "panel/st7735/display_st7735.c"
    "panel/st7789/display_st7789.c"
)

target_include_directories(${COMPONENT_LIB} PUBLIC
    "inc"
    "panel/st7735"
    "panel/st7789"
)
