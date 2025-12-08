#include "simple_log.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/sinks/daily_file_sink.h"
#include "yaml-cpp/yaml.h"
#include <iostream>
#include <filesystem>
#include <algorithm>
#include <cctype>

namespace fs = std::filesystem;

// 字符串转日志级别
spdlog::level::level_enum SimpleLogger::stringToLevel(const std::string& level_str) {
    std::string level = level_str;
    std::transform(level.begin(), level.end(), level.begin(), ::tolower);
    
    if (level == "trace") return spdlog::level::trace;
    if (level == "debug") return spdlog::level::debug;
    if (level == "info") return spdlog::level::info;
    if (level == "warn") return spdlog::level::warn;
    if (level == "error") return spdlog::level::err;
    if (level == "critical") return spdlog::level::critical;
    if (level == "off") return spdlog::level::off;
    
    return spdlog::level::info;
}

// 从配置文件创建日志器 
bool SimpleLogger::createFromConfig(const std::string& config_file, spdlog::logger& logger) {
    
    // 立即打印，确定函数入口已被执行（在 try 之前）
    std::cerr << "[DEBUG] enter createFromConfig, config_file param: " << config_file << std::endl;

    try 
    {
        // 1. 直接加载YAML配置文件
        YAML::Node config = YAML::LoadFile(config_file);
        
        // 3. 文件输出配置
        if (config["file"]) 
        {
            YAML::Node file = config["file"];
            
            if (file["enabled"].as<bool>(true)) 
            {
                std::string filename = file["filename"].as<std::string>("logs/app.log");
                bool append = file["append"].as<bool>(true);
                
                // 确保日志目录存在
                fs::path log_path(filename);
                if (log_path.has_parent_path()) {
                    fs::create_directories(log_path.parent_path());
                }
                
                // 滚动策略
                std::string rotation_strategy = "size"; //设置默认值
                if (file["rotation_strategy"]) {
                    rotation_strategy = file["rotation_strategy"].as<std::string>();
                }
                
                // 创建sink并设置日志器
                // 按时间滚动配置
                if (rotation_strategy == "time") 
                {
                    
                    int hour = 0, minute = 0;
                    
                    if (file["time_config"]) {
                        YAML::Node time_config = file["time_config"];
                        if (time_config["hour"]) {
                            hour = time_config["hour"].as<int>();
                        }
                        if (time_config["minute"]) {
                            minute = time_config["minute"].as<int>();
                        }
                    }
                    
                    std::cout << "创建按时间滚动的日志文件: " << filename << std::endl;
                    
                    // 创建时间滚动sink
                    auto sink = std::make_shared<spdlog::sinks::daily_file_sink_mt>(
                        filename, hour, minute, append);
                    
                    // 设置级别和格式
                    if (file["level"]) {
                        sink->set_level(stringToLevel(file["level"].as<std::string>()));
                    }
                    
                    if (file["pattern"]) {
                        sink->set_pattern(file["pattern"].as<std::string>());
                    }
                    
                    // 重新初始化日志器
                    logger = spdlog::logger("main", sink);
                    
                } 
                // 按大小滚动配置
                else if(rotation_strategy == "size")
                {
                    //设置默认参数
                    size_t max_size = 5 * 1024 * 1024;  // 默认5MB
                    size_t max_files = 5;               // 默认5个文件
                    
                    //从YAML配置中读取大小滚动参数
                    if (file["size_config"]) 
                    {
                        YAML::Node size_config = file["size_config"];
                        if (size_config["max_size"]) 
                        {
                            max_size = size_config["max_size"].as<size_t>();
                        }
                        if (size_config["max_files"]) 
                        {
                            max_files = size_config["max_files"].as<size_t>();
                        }
                    }
                    
                    std::cout << "创建按大小滚动的日志文件: " << filename << std::endl;
                    std::cout << "最大大小: " << max_size << " bytes, 最大文件数: " << max_files << std::endl;
                    
                    // 创建大小滚动sink
                    auto sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
                        filename, max_size, max_files, append);
                    
                    // 设置级别和格式
                    if (file["level"]) 
                    {
                        sink->set_level(stringToLevel(file["level"].as<std::string>()));
                    }
                    
                    if (file["pattern"]) 
                    {
                        sink->set_pattern(file["pattern"].as<std::string>());
                    }
                    
                    // 重新初始化日志器
                    logger = spdlog::logger("main", sink);
                }
                
                // 设置日志器级别
                logger.set_level(spdlog::level::info);
                
                // 设置错误处理器
                spdlog::set_error_handler([](const std::string& msg) {
                    std::cerr << "[SPDLOG ERROR] " << msg << std::endl;
                });
                
                std::cout << "日志器创建成功" << std::endl;
                return true;
            }
        }
        
        std::cerr << "文件配置未启用或不存在" << std::endl;
        return false;
        
    } catch (const YAML::BadFile& e) {
        std::cerr << "配置文件不存在: " << config_file << std::endl;
    } catch (const YAML::Exception& e) {
        std::cerr << "YAML解析错误: " << e.what() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "创建日志器失败: " << e.what() << std::endl;
    }
    
    return false;
}