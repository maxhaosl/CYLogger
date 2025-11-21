#include "ICYLoggerDefine.hpp"
#include <iostream>

// 简单示例，不使用CYLogger的完整功能
int main() {
    std::cout << "CYLogger Simple Example" << std::endl;
    
    // 输出一些基本信息
    std::cout << "Log Types:" << std::endl;
    std::cout << "LOG_TYPE_TRACE: " << LOG_TYPE_TRACE << std::endl;
    std::cout << "LOG_TYPE_DEBUG: " << LOG_TYPE_DEBUG << std::endl;
    std::cout << "LOG_TYPE_INFO: " << LOG_TYPE_INFO << std::endl;
    std::cout << "LOG_TYPE_WARN: " << LOG_TYPE_WARN << std::endl;
    std::cout << "LOG_TYPE_ERROR: " << LOG_TYPE_ERROR << std::endl;
    std::cout << "LOG_TYPE_FATAL: " << LOG_TYPE_FATAL << std::endl;
    
    return 0;
}