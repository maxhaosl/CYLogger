# Collect all library sources
file(GLOB_RECURSE CYLOGGER_SOURCES
    "*.cpp"
    "*.hpp"
)

# Exclude example sources
list(FILTER CYLOGGER_SOURCES EXCLUDE REGEX ".*Example.*")

# Create the core library target
add_library(CYLogger ${CYLOGGER_SOURCES})

# Make the Src directory available as an include path
target_include_directories(CYLogger PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Target metadata
set_target_properties(CYLogger PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    PUBLIC_HEADER "../Inc/ICYLogger.hpp"
    PUBLIC_HEADER "../Inc/ICYLoggerDefine.hpp"
    PUBLIC_HEADER "../Inc/ICYLoggerPatternFilter.hpp"
    PUBLIC_HEADER "../Inc/ICYLoggerTemplateLayout.hpp"
)

# Explicitly set MSVC runtime library for this target
if(MSVC AND DEFINED CMAKE_MSVC_RUNTIME_LIBRARY)
    set_target_properties(CYLogger PROPERTIES
        MSVC_RUNTIME_LIBRARY "${CMAKE_MSVC_RUNTIME_LIBRARY}"
    )
endif()

# Core compilation definitions
target_compile_definitions(CYLogger PRIVATE
    $<$<BOOL:${BUILD_SHARED_LIBS}>:CYLOGGER_USE_DLL>
    $<$<BOOL:${BUILD_SHARED_LIBS}>:CYLOGGER_EXPORTS>
)

# Platform-specific preprocessor flags
if(WIN32)
    target_compile_definitions(CYLogger PRIVATE
        WIN32
        $<$<CONFIG:Debug>:_DEBUG>
        $<$<CONFIG:Release>:NDEBUG>
        _LIB
    )
elseif(APPLE)
    if(IOS)
        target_compile_definitions(CYLogger PRIVATE CYLOGGER_IOS_OS)
    else()
        target_compile_definitions(CYLogger PRIVATE CYLOGGER_MAC_OS)
    endif()
elseif(ANDROID)
    target_compile_definitions(CYLogger PRIVATE CYLOGGER_ANDROID_OS)
elseif(UNIX)
    target_compile_definitions(CYLogger PRIVATE CYLOGGER_LINUX_OS)
endif()

# Link against pthreads and optional dependencies
find_package(Threads REQUIRED)

if(USE_CYCOROUTINE)
    # Always link the static CYCoroutine target to avoid export confusion
    if(TARGET CYCoroutine_static)
        target_link_libraries(CYLogger PUBLIC Threads::Threads CYCoroutine_static)
    else()
        message(FATAL_ERROR "CYCoroutine_static target not found while USE_CYCOROUTINE=ON")
    endif()
else()
    target_link_libraries(CYLogger PUBLIC Threads::Threads)
endif()

# Windows extras
if(WIN32)
    target_link_libraries(CYLogger PUBLIC 
        ws2_32
        shell32
        kernel32
    )
endif()

# Installation rules
install(TARGETS CYLogger
    EXPORT CYLoggerTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# Export the CMake target files
install(EXPORT CYLoggerTargets
    FILE CYLoggerTargets.cmake
    NAMESPACE CYLogger::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/CYLogger
)

# Re-export CYCoroutine so that package consumers can reuse it
# Note: CYCoroutine only supports static library on Windows, but supports both static and shared on other platforms (macOS, Linux, Android)
if(TARGET CYCoroutine_static)
    install(TARGETS CYCoroutine_static
        EXPORT CYLoggerTargets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
endif()
