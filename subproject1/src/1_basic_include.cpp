// 1_basic_include.cpp
#include <spdlog/spdlog.h>

int main()
{
    spdlog::info("Welcome to spdlog!"); // 打印 info 级别信息
    spdlog::set_level(spdlog::level::debug); // 设置*全局*日志级别为 debug
    spdlog::debug("This message should be displayed..");    
    
    // 改变日志模式
    spdlog::set_pattern("[%H:%M:%S %z] [%n] [%^---%L---%$] [thread %t] %v");
    
    // 编译时日志级别（宏）
    SPDLOG_TRACE("Some trace message with param {}", 42); 
}