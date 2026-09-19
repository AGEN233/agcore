target_sources(${COMPONENT_LIB} PRIVATE
    "log/vendor/easylogger/src/elog.c"
    "log/vendor/easylogger/src/elog_utils.c"
    "log/vendor/easylogger/src/elog_async.c"
    "log/vendor/easylogger/port/elog_port.c"
    "log/agcore_log.c"
    "shell/agcore_console_shell.c"
)

target_include_directories(${COMPONENT_LIB} PUBLIC
    "log"
    "log/vendor/easylogger/inc"
    "shell/inc"
)
