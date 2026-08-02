set(AGLOG_EASYLOGGER_SRCS
    "easylogger/src/elog.c"
    "easylogger/src/elog_utils.c"
    "easylogger/src/elog_async.c"
)

# --- agcore 侧适配 ---
set(AGLOG_AGCORE_SRCS
    "easylogger/port/elog_port.c"
    "agcore_log_adapter.c"
)

target_sources(${COMPONENT_LIB} PRIVATE
    ${AGLOG_EASYLOGGER_SRCS}
    ${AGLOG_AGCORE_SRCS}
)

target_include_directories(${COMPONENT_LIB} PUBLIC
    "."
    "easylogger/inc"
)
