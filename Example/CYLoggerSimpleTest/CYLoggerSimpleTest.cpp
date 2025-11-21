#include <iostream>
#include "Inc/ICYLogger.hpp"

int main() {
    std::cout << "CYLogger Simple Test" << std::endl;
    
    // 初始化日志系统
    auto logger = cry::ICYLogger::GetInstance(cry::ELogLevelFilter::LOG_FILTER_ALL);
    if (!logger) {
        std::cerr << "Failed to get logger instance!" << std::endl;
        return 1;
    }
    
    // 添加一个控制台输出
    logger->AddApender(cry::ELogType::LOG_TYPE_NONE, "TestChannel", "ConsoleLog", cry::ELogFileMode::LOG_MODE_FILE_APPEND);
    
    // 写入不同级别的日志
    CY_LOG_DIRECT_TRACE("This is a trace message");
    CY_LOG_DIRECT_DEBUG("This is a debug message");
    CY_LOG_DIRECT_INFO("This is an info message");
    CY_LOG_DIRECT_WARN("This is a warning message");
    CY_LOG_DIRECT_ERROR("This is an error message");
    CY_LOG_DIRECT_FATAL("This is a fatal message");
    
    // 清理资源
    cry::ICYLogger::FreeInstance();
    
    std::cout << "Test completed!" << std::endl;
    return 0;
}