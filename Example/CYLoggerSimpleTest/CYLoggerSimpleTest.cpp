#include <iostream>
#include "Inc/ICYLogger.hpp"

int main() {
    std::cout << "CYLogger Simple Test" << std::endl;
    
    // Initialize logging system
    auto logger = cry::ICYLogger::GetInstance(cry::ELogLevelFilter::LOG_FILTER_ALL);
    if (!logger) {
        std::cerr << "Failed to get logger instance!" << std::endl;
        return 1;
    }
    
    // Add a console output
    logger->AddApender(cry::ELogType::LOG_TYPE_NONE, "TestChannel", "ConsoleLog", cry::ELogFileMode::LOG_MODE_FILE_APPEND);
    
    // Write logs for each severity
    CY_LOG_DIRECT_TRACE("This is a trace message");
    CY_LOG_DIRECT_DEBUG("This is a debug message");
    CY_LOG_DIRECT_INFO("This is an info message");
    CY_LOG_DIRECT_WARN("This is a warning message");
    CY_LOG_DIRECT_ERROR("This is an error message");
    CY_LOG_DIRECT_FATAL("This is a fatal message");
    
    // Clean up resources
    cry::ICYLogger::FreeInstance();
    
    std::cout << "Test completed!" << std::endl;
    return 0;
}