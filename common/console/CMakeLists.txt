target_sources(${COMPONENT_LIB} PRIVATE
    # EasyLogger
    "log/easylogger/src/elog.c"
    "log/easylogger/src/elog_utils.c"
    "log/easylogger/src/elog_async.c"
    "log/easylogger/port/elog_port.c"

    # AGCORE Log
    "log/agcore_console_log.c"

    # Letter Shell
    "shell/letter_shell/shell.c"
    "shell/letter_shell/shell_cmd_list.c"
    "shell/letter_shell/shell_ext.c"

    # AGCORE Console / Shell
    "agcore_console_adapter.c"
    "shell/agcore_console_shell.c"
)

target_include_directories(${COMPONENT_LIB} PUBLIC
    "."
    "log"
    "log/easylogger/inc"
    "shell"
    "shell/letter_shell"
)

# Letter Shell user configuration
target_compile_definitions(${COMPONENT_LIB} PRIVATE
    SHELL_CFG_USER=\"agcore_shell_cfg_user.h\"
)

set_source_files_properties(
    "${CMAKE_CURRENT_LIST_DIR}/shell/letter_shell/shell.c"
    "${CMAKE_CURRENT_LIST_DIR}/shell/letter_shell/shell_cmd_list.c"
    "${CMAKE_CURRENT_LIST_DIR}/shell/letter_shell/shell_ext.c"
    TARGET_DIRECTORY ${COMPONENT_LIB}
    PROPERTIES COMPILE_OPTIONS "-fno-strict-aliasing;-Wno-error=incompatible-pointer-types;-Wno-error=cast-function-type;-w"
)

set_source_files_properties(
    "${CMAKE_CURRENT_LIST_DIR}/shell/agcore_console_shell.c"
    TARGET_DIRECTORY ${COMPONENT_LIB}
    PROPERTIES COMPILE_OPTIONS "-Wno-error=cast-function-type"
)

__ldgen_add_fragment_files("${CMAKE_CURRENT_LIST_DIR}/shell/letter_shell.lf")
