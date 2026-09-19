target_sources(${COMPONENT_LIB} PRIVATE
    "agcore_initcall.c"
)

target_include_directories(${COMPONENT_LIB} PUBLIC
    "."
)
