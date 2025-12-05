#pragma once

#include <string>
#include "spdlog/spdlog.h"

// 最简单的日志创建器 
class SimpleLogger {
public:
    // 从配置文件创建日志器
    static bool createFromConfig(const std::string& config_file, spdlog::logger& logger);
    
private:
    // 字符串转日志级别
    static spdlog::level::level_enum stringToLevel(const std::string& level_str);
};