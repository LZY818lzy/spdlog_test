#include "simple_log.h"
#include <iostream>
#include <thread>
#include <chrono>

#ifdef _WIN32
#include <windows.h>
#endif

int main() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif
    
    std::cout << "=== 测试 ===" << std::endl;
    
    // 创建日志器对象
    spdlog::logger logger("default");
    

    // 从配置文件初始化日志器
    bool success = SimpleLogger::createFromConfig("../config/log_config.yaml", logger);
    
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