#设置 PROJECT_NAME 变量为 subproject3
project(subproject3)

# 查找源文件
file(GLOB SOURCES "src/*.cpp")

# 创建可执行文件 - 会自动输出到根目录的 bin
add_executable(${PROJECT_NAME} ${SOURCES})

# 添加头文件包含路径
target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include # 本地头文件路径
    PUBLIC
        ${COMMON_INCLUDE_DIR} # 公共头文件路径
#        ${SPDLOG_INCLUDE_DIR} # spdlog 头文件路径
)
# 库的链接模式（Header-only vs. 独立编译）。
# 如果使用独立编译的静态库，定义 SPDLOG_COMPILED_LIB
target_compile_definitions(${PROJECT_NAME}  PRIVATE SPDLOG_COMPILED_LIB)
# 链接 spdlog 静态库
target_link_libraries(${PROJECT_NAME} PRIVATE ${SPDLOG_LIBRARY})

# 可选：设置可执行文件属性
set_target_properties(${PROJECT_NAME} PROPERTIES
    OUTPUT_NAME "subproject3"  # 确保输出名称一致
)

# 可选：检查库文件是否存在
if(NOT EXISTS ${SPDLOG_LIBRARY})
    message(WARNING "spdlog library not found at: ${SPDLOG_LIBRARY}")
endif()

message(STATUS "${PROJECT_NAME} 配置完成: ${PROJECT_NAME}")