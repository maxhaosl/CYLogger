#include <iostream>
#include "ICYLoggerDefine.hpp"

// 简单的日志测试，不依赖完整的CYLogger实现
int main() {
    std::cout << "CYLogger Types Test" << std::endl;
    
    // 输出日志类型
    std::cout << "LOG_TYPE_TRACE: " << LOG_TYPE_TRACE << std::endl;
    std::cout << "LOG_TYPE_DEBUG: " << LOG_TYPE_DEBUG << std::endl;
    std::cout << "LOG_TYPE_INFO: " << LOG_TYPE_INFO << std::endl;
    std::cout << "LOG_TYPE_WARN: " << LOG_TYPE_WARN << std::endl;
    std::cout << "LOG_TYPE_ERROR: " << LOG_TYPE_ERROR << std::endl;
    std::cout << "LOG_TYPE_FATAL: " << LOG_TYPE_FATAL << std::endl;
    
    // 输出日志级别
    std::cout << "LOG_LEVEL_CONSOLE: " << LOG_LEVEL_CONSOLE << std::endl;
    std::cout << "LOG_LEVEL_TRACE: " << LOG_LEVEL_TRACE << std::endl;
    std::cout << "LOG_LEVEL_DEBUG: " << LOG_LEVEL_DEBUG << std::endl;
    std::cout << "LOG_LEVEL_INFO: " << LOG_LEVEL_INFO << std::endl;
    std::cout << "LOG_LEVEL_WARN: " << LOG_LEVEL_WARN << std::endl;
    std::cout << "LOG_LEVEL_ERROR: " << LOG_LEVEL_ERROR << std::endl;
    std::cout << "LOG_LEVEL_FATAL: " << LOG_LEVEL_FATAL << std::endl;
    
    // 输出日志过滤器
    std::cout << "LOG_FILTER_ALL: " << LOG_FILTER_ALL << std::endl;
    std::cout << "LOG_FILTER_WARNS_AND_ERRORS: " << LOG_FILTER_WARNS_AND_ERRORS << std::endl;
    std::cout << "LOG_FILTER_ERRORS: " << LOG_FILTER_ERRORS << std::endl;
    std::cout << "LOG_FILTER_NONE: " << LOG_FILTER_NONE << std::endl;
    
    return 0;
}