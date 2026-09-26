target_sources(${COMPONENT_LIB} PRIVATE
    "log/easylogger/src/elog.c"
    "log/easylogger/src/elog_utils.c"
    "log/easylogger/src/elog_async.c"
    "log/easylogger/port/elog_port.c"
    "agcore_console_adapter.c"
    "shell/agcore_console_shell.c"
)

target_include_directories(${COMPONENT_LIB} PUBLIC
    "inc"
    "log/easylogger/inc"
)
