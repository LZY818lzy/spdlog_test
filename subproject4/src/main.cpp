#include <iostream>

#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/sinks/daily_file_sink.h"
//#include "spdlog/async.h"

#include "../../common_include/CConfig.h"

#include <filesystem> 

#ifdef _WIN32
#include <windows.h>
#endif


// 辅助函数：将字符串日志级别转换为spdlog的level枚举
spdlog::level::level_enum stringToLevel(const std::string& level_str) {
    if (level_str == "trace") return spdlog::level::trace;
    if (level_str == "debug") return spdlog::level::debug;
    if (level_str == "info") return spdlog::level::info;
    if (level_str == "warn") return spdlog::level::warn;
    if (level_str == "error") return spdlog::level::err;
    if (level_str == "critical") return spdlog::level::critical;
    if (level_str == "off") return spdlog::level::off;
    return spdlog::level::info; // 默认级别
}

// 设置和初始化日志系统
bool setupLogging() 
{
    try {

        // 1. 获取配置实例并加载配置文件
        auto config = CConfig::GetInstance();
        
        // 2. 获取日志配置参数
        bool log_console = config->GetBool("log_console");
        std::string level_str = config->GetString("level");
        std::string pattern = config->GetString("pattern");
        std::string filename = config->GetString("filename");
        bool immediate_flush = config->GetBool("immediate_flush");
        
        // 获取滚动策略
        int rotation_strategy = config->GetInt("rotation_strategy");
        int max_size = config->GetInt("max_size");
        int max_files = config->GetInt("max_files");
        int hour = config->GetInt("hour");
        int minute = config->GetInt("minute");
        
        // 3. 创建sinks（日志输出目标）
        //采用容器存储多个sink————可以同时输出到控制台和文件
        std::vector<spdlog::sink_ptr> sinks;

        
        // 控制台sink
        if (log_console) {
            auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
            console_sink->set_pattern(pattern);
            sinks.push_back(console_sink);
        }
        
        // 文件sink（根据滚动策略选择不同的sink）
        if (rotation_strategy == 1) 
        {
            // 按大小滚动
            std::cout << "创建按大小滚动的日志文件: " << filename << std::endl;
            std::cout << "最大大小: " << max_size << " kb, 最大文件数: " << max_files << std::endl;
            // 注意：max_size的单位是字节，配置文件中的5是KB，需要转换
            auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>
            (
                filename, 
                max_size * 1024, 
                max_files
            );
            file_sink->set_pattern(pattern);
            sinks.push_back(file_sink);

            
        } 
        else if (rotation_strategy == 2) 
        {
            // 按时间滚动（每天指定时间）
            std::cout << "创建按时间滚动的日志文件: " << filename << std::endl;
            std::cout << "滚动时间: " << hour << ":" << minute << std::endl;
            // 注意：hour和minute需要在0-23和0-59之间
            auto file_sink = std::make_shared<spdlog::sinks::daily_file_sink_mt>
            (
                filename,
                hour, 
                minute
            );
            file_sink->set_pattern(pattern);
            sinks.push_back(file_sink);
        } 
        
        // 4. 创建组合logger
        auto logger = std::make_shared<spdlog::logger>("main_logger", sinks.begin(), sinks.end());
    

        // 5. 设置日志级别
        logger->set_level(stringToLevel(level_str));
        
        // 6. 设置立即刷新（如果需要）
        if (immediate_flush) {
            logger->flush_on(spdlog::level::trace); // 所有级别都立即刷新
        }
        
        // 7. 将创建的日志器设置为全局默认日志器
        spdlog::set_default_logger(logger);
        
        
        // 8. 记录初始化成功的日志
        spdlog::info("日志系统初始化成功!");
        spdlog::info("日志级别: {}", level_str);
        spdlog::info("日志文件: {}", filename);
        spdlog::info("控制台输出: {}", log_console ? "启用" : "禁用");
        
        return true;
        
    } catch (const spdlog::spdlog_ex& e) {
        std::cerr << "日志初始化失败: " << e.what() << std::endl;
        return false;
    } catch (const std::exception& e) {
        std::cerr << "发生异常: " << e.what() << std::endl;
        return false;
    }
}


int main() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

     // 打印当前工作目录
    std::cout << "当前工作目录: " << std::filesystem::current_path() << std::endl;

    //1、加载配置文件
    CConfig* p_config = CConfig::GetInstance(); //单例类
    if (p_config->Load("../config/subproject4.yaml") == false) //把配置文件内容载入到内存            
    {
        std::cerr << "无法加载配置文件!!!..........." << std::endl;
        exit(-1);
    }
    
    
    std::cout << "=== 测试 ===" << std::endl;

     // 2、调用setupLogging函数初始化日志系统
    if (!setupLogging()) {
        std::cerr << "日志系统初始化失败!" << std::endl;
        return -1;
    }

    std::cout << "日志系统初始化成功，开始测试..." << std::endl;
    
    // 3、测试文件滚动（如果是大小滚动）
    std::cout << "\n=== 测试文件滚动 ===" << std::endl;
    for (int i = 0; i < 1000; ++i) {
        spdlog::info("滚动测试 {} - 这是一条用于测试文件滚动的消息，长度较长以确保能触发滚动", i);
        
        // 每写入100条日志输出一次进度
        if ((i + 1) % 100 == 0) {
            spdlog::default_logger()->flush();  // 刷新默认日志器
            std::cout << "已写入 " << (i + 1) << " 条日志" << std::endl;
        }
    }
    
    // 4、程序结束日志
    spdlog::info("程序正常结束");
    spdlog::default_logger()->flush();  // 刷新默认日志器

    // 9、清理日志系统
    spdlog::shutdown();
    
    std::cout << "\n=== 测试完成 ===" << std::endl;
    std::cout << "请检查 logs/ 目录下的日志文件" << std::endl;
    return 0;
}



void functionToTest() {
    // 占位函数，用于调试断点设置
    CConfig* p_config = CConfig::GetInstance(); //单例类
    std::string name = p_config->GetString("name2"); //测试取值函数是否可用
    int  some_int_value = p_config->GetInt("some_int_value2"); //测试取值函数是否可用
}

