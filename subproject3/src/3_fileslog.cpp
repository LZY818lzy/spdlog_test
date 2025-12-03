#ifdef _WIN32
#include <io.h>
#include <fcntl.h>
#include <windows.h>  // 添加这行，用于 SetConsoleOutputCP 和 CP_UTF8
#endif

//1、基础文件日志测试
#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"
#include <iostream>

void test_basic_file_log() {
    try {
        // 1. 创建 Logger 对象
        // 文件路径相对于程序执行目录
        auto logger = spdlog::basic_logger_mt("basic_logger", "logs/basic-log.txt");
        
        // 可选：设置日志级别，确保能输出 info 级别
        logger->set_level(spdlog::level::info);

        // 2. 使用 Logger 记录日志
        logger->info("Test message 1: Logger创建成功.");
        logger->warn("Test message 2: 发生了一个重要的警告.");
        
        // 3. 强制刷新，确保写入磁盘
        logger->flush();

        std::cout << "日志消息已发送到记录器。请检查文件 'logs/basic-log.txt'。" << std::endl;

    } catch (const spdlog::spdlog_ex& ex) {
        std::cerr << "日志初始化失败: " << ex.what() << std::endl;
    }
}


// 2、每日日志文件测试
#include "spdlog/spdlog.h"
#include "spdlog/sinks/daily_file_sink.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <string>

void test_daily_log() {
    try {
        // 定义固定的轮换时间点
        const int rotation_hour = 15;
        const int rotation_minute = 35;

        // 强制 spdlog 在遇到任何内部错误时，将错误信息打印到控制台
        spdlog::set_error_handler([](const std::string& msg) {
            std::cerr << "\n[!!! SPDLOG ERROR !!!] 轮换或文件操作失败: " << msg << std::endl;
        });

        // 创建 Daily Logger
        auto logger = spdlog::daily_logger_mt(
            "daily_logger", 
            "logs/daily_test.txt", // 目标文件
            rotation_hour, 
            rotation_minute
        );

        logger->set_level(spdlog::level::info);
        logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] %v");
        
        std::cout << "✅ 日志已配置为在今天 " 
                  << rotation_hour << ":" 
                  << rotation_minute << " 轮换。" << std::endl;
        std::cout << "-----------------------------------------------" << std::endl;

        // 2. 记录初始日志 (文件生成)
        for (int i = 0; i < 10; ++i) { 

            logger->info("初始日志 (轮换前) - 运行次数: {}", i);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        logger->flush();
        
        std::cout << "\n[文件状态] logs/daily_fixed_test.txt 已创建并包含初始日志." << std::endl;
        std::cout << "=========================================================" << std::endl;
        std::cout << "🚨 请手动进行以下操作:" << std::endl;
        std::cout << "   1. 将系统时间调整到" << rotation_hour << ":" << rotation_minute + 1 << " AM (例如 02:31 AM)。" << std::endl;
        std::cout << "   2. 完成后，请按 ENTER 键继续记录日志..." << std::endl;
        std::cout << "=========================================================\n" << std::endl;

        // 暂停等待用户输入
        std::cin.get();

        // 3. 记录轮换后的日志
        logger->warn("轮换后的第一条日志");
        logger->warn("确认：这行消息应该出现在新的日志文件中。");

         //强制刷新，确保新文件被创建和写入
        logger->flush();
        
        std::cout << "-----------------------------------------------" << std::endl;
        std::cout << "✅ 程序执行完毕，请检查 logs 目录下的文件。" << std::endl;

    } catch (const spdlog::spdlog_ex& ex) {
        std::cerr << "日志初始化失败: " << ex.what() << std::endl;
    }
}

// 3、轮转文件日志测试
#include "spdlog/spdlog.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include <iostream>
#include <chrono>   // 用于时间点和持续时间
#include <thread>   // 用于睡眠函数


void test_rotating_log_with_sleep() 
{
    try {
        // --- 1. 简化的配置参数 ---
        // 设置日志文件最大容量为 5 KB (5 * 1024 字节)
        const size_t max_size_bytes = 5 * 1024; 
        // 保留 3 个备份文件 (.0, .1, .2) + 1 个当前文件
        const size_t max_files_count = 3; 
        
        // --- 2. 创建 Rotating Logger ---
        auto logger = spdlog::rotating_logger_mt(
            "simple_rotating_logger",
            "logs/rotating_simple.txt", // 主日志文件路径
            max_size_bytes,
            max_files_count
        );

        logger->set_level(spdlog::level::info);
        // 设置日志格式：时间戳 + 消息
        logger->set_pattern("[%H:%M:%S.%e] %v");
        
        // 强制 spdlog 在遇到任何内部错误时，将错误信息打印到控制台
        spdlog::set_error_handler([](const std::string& msg) {
            std::cerr << "\n[!!! SPDLOG ERROR !!!] 轮转或文件操作失败: " << msg << std::endl;
        });

        std::cout << "--- 开始文件大小轮转测试 ---" << std::endl;
        std::cout << "主文件: logs/rotating_simple.txt" << std::endl;
        std::cout << "最大容量: " << max_size_bytes << " 字节 (5 KB)" << std::endl;
        std::cout << "最大备份文件数: " << max_files_count << std::endl;
        std::cout << "------------------------------------------" << std::endl;
        
        // --- 3. 循环写入日志，直到触发多次轮换 ---
        // 目标写入 1000 行日志，以确保触发多次 5KB 的轮换
        const int total_lines_to_log = 1000; 
        
        for (int i = 0; i < total_lines_to_log; ++i) {
            std::string log_message = "日志行号: " + std::to_string(i) + " - 这是一条用来填充文件大小的示例消息。";
            
            logger->info(log_message);
            
            // 确保日志立即写入磁盘，这样轮转检查才能立即生效
            logger->flush(); 

            // 稍微等待一下，方便您观察控制台和文件变化
            std::this_thread::sleep_for(std::chrono::milliseconds(200)); 
            
            // 每 100 行打印一个提示信息
            if ((i + 1) % 100 == 0) {
                 std::cout << "已写入 " << i + 1 << " 行日志。文件轮转可能已发生..." << std::endl;
            }
        }
        
        std::cout << "\n--- 日志写入完成 (共 " << total_lines_to_log << " 行)。 ---" << std::endl;
        std::cout << "请检查 'logs/' 目录下的文件，应有 logs/rotating_simple.txt、.0、.1、.2 等备份文件。" << std::endl;
        
    } catch (const spdlog::spdlog_ex& ex) {
        std::cerr << "日志初始化失败: " << ex.what() << std::endl;
    }
}

int main() {
//Windows 控制台乱码是因为 UTF-8 编码的源文件中的中文字符与控制台的默认编码（GBK/CP936）不匹配。
//解决方案：在程序启动时设置控制台为 UTF-8 模式
#ifdef _WIN32
    // 设置控制台为 UTF-8 模式
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    test_rotating_log_with_sleep();
    return 0;
}

