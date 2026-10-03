# AGCORE Console / Log
list(APPEND AGCORE_SRCS
    # Console
    "common/console/agcore_console_adapter.c"

    # EasyLogger
    "common/console/log/easylogger/src/elog.c"
    "common/console/log/easylogger/src/elog_utils.c"
    "common/console/log/easylogger/src/elog_async.c"
    "common/console/log/easylogger/port/elog_port.c"

    # AGCORE Log
    "common/console/log/agcore_console_log.c"
)

# AGCORE Console / Log include
list(APPEND AGCORE_INCLUDE_DIRS
    "common/console"
    "common/console/log"
    "common/console/log/easylogger/inc"
)

# AGCORE Shell
if(CONFIG_AGCORE_CONSOLE_SHELL_ENABLE)

    # Shell sources
    list(APPEND AGCORE_SRCS
        # Letter Shell
        "common/console/shell/letter_shell/shell.c"
        "common/console/shell/letter_shell/shell_cmd_list.c"
        "common/console/shell/letter_shell/shell_ext.c"

        # AGCORE Shell
        "common/console/shell/agcore_console_shell.c"
    )

    # AGCORE Shell API 对外可见，Letter Shell 仅内部可见
    list(APPEND AGCORE_INCLUDE_DIRS "common/console/shell")
    list(APPEND AGCORE_PRIVATE_INCLUDE_DIRS "common/console/shell/letter_shell")
    list(APPEND AGCORE_LDFRAGMENTS "common/console/shell/letter_shell.lf")
endif()

# 组件注册后应用 Shell 编译配置。
function(agcore_espidf_configure_console)
    if(CONFIG_AGCORE_CONSOLE_SHELL_ENABLE)
        # 使用当前 ESP-IDF 工程名称作为 Shell 默认用户名
        idf_build_get_property(project_name PROJECT_NAME)

        # Letter Shell 配置
        target_compile_definitions(${COMPONENT_LIB} PRIVATE
            "SHELL_CFG_USER=\"agcore_shell_cfg_user.h\""
            "SHELL_DEFAULT_USER=\"${project_name}\""
        )

        # Letter Shell 为第三方源码，编译兼容选项仅作用于其自身
        set_source_files_properties(
            "${AGCORE_ROOT_DIR}/common/console/shell/letter_shell/shell.c"
            "${AGCORE_ROOT_DIR}/common/console/shell/letter_shell/shell_cmd_list.c"
            "${AGCORE_ROOT_DIR}/common/console/shell/letter_shell/shell_ext.c"
            TARGET_DIRECTORY ${COMPONENT_LIB}
            PROPERTIES COMPILE_OPTIONS
                "-fno-strict-aliasing;-Wno-error=incompatible-pointer-types;-Wno-error=cast-function-type;-w"
        )

        # AGCORE Shell 适配层需要兼容 Letter Shell 的命令函数类型转换
        set_source_files_properties(
            "${AGCORE_ROOT_DIR}/common/console/shell/agcore_console_shell.c"
            TARGET_DIRECTORY ${COMPONENT_LIB}
            PROPERTIES COMPILE_OPTIONS
                "-Wno-error=cast-function-type"
        )

    endif()
endfunction()
