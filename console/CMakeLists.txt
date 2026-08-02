# console 一层模块门面
# 负责聚合二层模块: log / shell
# 顶层共享件 agcore_console.c(h) 在此编入, 并把 console 顶层挂入 include 路径,
# 供 log / shell 无前缀引用 agcore_console.h(共享串口写锁)

target_sources(${COMPONENT_LIB} PRIVATE
    "agcore_console.c"
)

target_include_directories(${COMPONENT_LIB} PUBLIC
    "."
)

# 二层模块
add_subdirectory(log)
add_subdirectory(shell)
