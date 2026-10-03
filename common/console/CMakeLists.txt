# AGCORE Console / Log
target_sources(${COMPONENT_LIB} PRIVATE
    # Console
    "agcore_console_adapter.c"

    # EasyLogger
    "log/easylogger/src/elog.c"
    "log/easylogger/src/elog_utils.c"
    "log/easylogger/src/elog_async.c"
    "log/easylogger/port/elog_port.c"

    # AGCORE Log
    "log/agcore_console_log.c"
)

# AGCORE Shell
if(CONFIG_AGCORE_CONSOLE_SHELL_ENABLE)
    target_sources(${COMPONENT_LIB} PRIVATE
        # Letter Shell
        "shell/letter_shell/shell.c"
        "shell/letter_shell/shell_cmd_list.c"
        "shell/letter_shell/shell_ext.c"

        # AGCORE Shell
        "shell/agcore_console_shell.c"
    )
endif()

target_include_directories(${COMPONENT_LIB} PUBLIC
    "."
    "log"
    "log/easylogger/inc"
    "shell"
    "shell/letter_shell"
)


# 使用当前 ESP-IDF 工程名称作为 Shell 默认用户名。
idf_build_get_property(project_name PROJECT_NAME)

# 引入agcore_shell_cfg_user.h
target_compile_definitions(${COMPONENT_LIB} PRIVATE
    "SHELL_CFG_USER=\"agcore_shell_cfg_user.h\""
    "SHELL_DEFAULT_USER=\"${project_name}\""
)

# Letter Shell 为第三方源码，编译兼容选项仅作用于其自身。
set_source_files_properties(
    "${CMAKE_CURRENT_LIST_DIR}/shell/letter_shell/shell.c"
    "${CMAKE_CURRENT_LIST_DIR}/shell/letter_shell/shell_cmd_list.c"
    "${CMAKE_CURRENT_LIST_DIR}/shell/letter_shell/shell_ext.c"
    TARGET_DIRECTORY ${COMPONENT_LIB}
    PROPERTIES COMPILE_OPTIONS
        "-fno-strict-aliasing;-Wno-error=incompatible-pointer-types;-Wno-error=cast-function-type;-w"
)

# AGCORE Shell 适配层需要兼容 Letter Shell 的命令函数类型转换。
set_source_files_properties(
    "${CMAKE_CURRENT_LIST_DIR}/shell/agcore_console_shell.c"
    TARGET_DIRECTORY ${COMPONENT_LIB}
    PROPERTIES COMPILE_OPTIONS
        "-Wno-error=cast-function-type"
)

# 注册 Letter Shell 命令表链接段。
__ldgen_add_fragment_files(
    "${CMAKE_CURRENT_LIST_DIR}/shell/letter_shell.lf"
)