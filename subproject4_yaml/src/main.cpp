#include "simple_log.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <filesystem>
#include <fstream>

#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#define GETCWD _getcwd
#else
#include <unistd.h>
#define GETCWD getcwd
#endif

// 调试函数：打印当前环境信息
void printDebugInfo() {
    std::cout << "\n=== 调试信息 ===" << std::endl;
    
    // 获取并显示当前工作目录
    char cwd[1024];
    if (GETCWD(cwd, sizeof(cwd)) != nullptr) {
        std::cout << "当前工作目录: " << cwd << std::endl;
    } else {
        std::cerr << "无法获取工作目录" << std::endl;
    }
    
    // 显示程序路径
    #ifdef _WIN32
        char exe_path[MAX_PATH];
        GetModuleFileNameA(nullptr, exe_path, MAX_PATH);
        std::cout << "可执行文件路径: " << exe_path << std::endl;
    #endif
    
    // 测试配置文件路径
    std::vector<std::string> test_paths = {
        "build/config/log_config.yaml",  // 从项目根目录访问 build/config
        "../build/config/log_config.yaml", // 如果在子目录中运行
        "config/log_config.yaml",  // 直接运行时的路径
        std::string(cwd) + "/config/log_config.yaml"
    };
    
    std::cout << "\n检查配置文件位置:" << std::endl;
    for (const auto& path : test_paths) {
        std::ifstream file(path);
        if (file.good()) {
            std::cout << "✓ 找到: " << path << std::endl;
            file.close();
        } else {
            std::cout << "✗ 没有: " << path << std::endl;
        }
    }
    std::cout << "================\n" << std::endl;
}


int main() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif
    
    //printDebugInfo();
    
    std::cout << "=== 测试 ===" << std::endl;
    
    // 创建日志器对象
    spdlog::logger logger("default");
    

    // 从配置文件初始化日志器
    bool success = SimpleLogger::createFromConfig("build/config/log_config.yaml", logger);
    
    if (!success) 
    {
        std::cerr << "创建日志器失败" << std::endl;
        return 1;
    }
    
    // 测试日志
    logger.info("程序启动");
    logger.debug("调试信息");
    logger.warn("警告信息");
    logger.error("错误信息");
    
    // 测试格式化
    for (int i = 0; i < 100000000; ++i) {
        logger.info("测试 {} - 值: {:.2f}", i, 3.14 * i);
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
    
    // 测试文件滚动
    std::cout << "\n测试文件滚动..." << std::endl;
    for (int i = 0; i < 2000000; ++i) {
        logger.info("滚动测试{} - {}", i, std::string(50, 'X'));
        
        if ((i + 1) % 5 == 0) {
            logger.flush();
            std::cout << "已写入 " << i + 1 << " 条日志" << std::endl;
        }
    }
    
    // 清理
    spdlog::shutdown();
    
    std::cout << "\n测试完成" << std::endl;
    return 0;
}