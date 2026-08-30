target_sources(${COMPONENT_LIB} PRIVATE
    "agcore_console.c"
)

target_include_directories(${COMPONENT_LIB} PUBLIC
    "."
)

add_subdirectory(log)
add_subdirectory(shell)
