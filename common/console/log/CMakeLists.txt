set(AGLOG_EASYLOGGER_SRCS
    "vendor/easylogger/src/elog.c"
    "vendor/easylogger/src/elog_utils.c"
    "vendor/easylogger/src/elog_async.c"
)

# --- agcore 侧适配 ---
set(AGLOG_AGCORE_SRCS
    "vendor/easylogger/port/elog_port.c"
    "agcore_log.c"
)

target_sources(${COMPONENT_LIB} PRIVATE
    ${AGLOG_EASYLOGGER_SRCS}
    ${AGLOG_AGCORE_SRCS}
)

target_include_directories(${COMPONENT_LIB} PUBLIC
    "."
    "vendor/easylogger/inc"
)
