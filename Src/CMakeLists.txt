# Collect all library sources
file(GLOB_RECURSE CYLOGGER_SOURCES
    "*.cpp"
    "*.hpp"
)

# Exclude example sources
list(FILTER CYLOGGER_SOURCES EXCLUDE REGEX ".*Example.*")

# Determine Debug/Release output name suffix for CYLogger targets.
# Both shared and static libraries use the same naming: CYLogger (Release) / CYLoggerD (Debug).
# On Windows, static library additionally gets 'Static' in the name.
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(_CYLOGGER_SHARED_OUTPUT_NAME "CYLoggerD")
    if(WIN32)
        set(_CYLOGGER_STATIC_OUTPUT_NAME "CYLoggerStaticD")
    else()
        set(_CYLOGGER_STATIC_OUTPUT_NAME "CYLoggerD")
    endif()
else()
    set(_CYLOGGER_SHARED_OUTPUT_NAME "CYLogger")
    if(WIN32)
        set(_CYLOGGER_STATIC_OUTPUT_NAME "CYLoggerStatic")
    else()
        # Android convention: static lib is just CYLogger (no "Static" suffix)
        set(_CYLOGGER_STATIC_OUTPUT_NAME "CYLogger")
    endif()
endif()

# =============================================================================
# Shared library target (CYLogger.dll / CYLoggerD.dll)
# For iOS, always build the shared library target regardless of BUILD_SHARED_LIBS
# (iOS requires BUILD_SHARED_LIBS to be OFF for fmt static linking)
# For Static build pipeline, skip shared library since it requires CYCoroutine_shared
# =============================================================================
if((CYLOGGER_WANT_SHARED AND NOT CYLOGGER_SKIP_SHARED) OR CYLOGGER_BUILD_IOS_DYLIB)
    # Rename from CYLogger to CYLogger_shared so the alias CYLogger still works
    add_library(CYLogger_shared SHARED ${CYLOGGER_SOURCES})

        set_target_properties(CYLogger_shared PROPERTIES
            VERSION ${PROJECT_VERSION}
            SOVERSION ${PROJECT_VERSION_MAJOR}
            PUBLIC_HEADER "../Inc/ICYLogger.hpp"
            PUBLIC_HEADER "../Inc/ICYLoggerDefine.hpp"
            PUBLIC_HEADER "../Inc/ICYLoggerPatternFilter.hpp"
            PUBLIC_HEADER "../Inc/ICYLoggerTemplateLayout.hpp"
            OUTPUT_NAME "${_CYLOGGER_SHARED_OUTPUT_NAME}"
            PDB_NAME "${_CYLOGGER_SHARED_OUTPUT_NAME}"
            MACOSX_BUNDLE OFF
        )

    # PDB generation
    if(MSVC)
        if(CMAKE_CONFIGURATION_TYPES)
            foreach(_cfg ${CMAKE_CONFIGURATION_TYPES})
                string(TOUPPER "${_cfg}" _cfg_upper)
                set_target_properties(CYLogger_shared PROPERTIES
                    PDB_NAME "${_CYLOGGER_SHARED_OUTPUT_NAME}"
                    PDB_OUTPUT_DIRECTORY_${_cfg_upper} "${_CYLOGGER_OUTPUT_BASE}/${_cfg}"
                )
            endforeach()
        else()
            set_target_properties(CYLogger_shared PROPERTIES
                PDB_NAME "${_CYLOGGER_SHARED_OUTPUT_NAME}"
                PDB_OUTPUT_DIRECTORY "${_CYLOGGER_OUTPUT_BASE}/${CMAKE_BUILD_TYPE}"
            )
        endif()
    endif()

    # MSVC runtime
    if(MSVC AND DEFINED CMAKE_MSVC_RUNTIME_LIBRARY)
        set_target_properties(CYLogger_shared PROPERTIES
            MSVC_RUNTIME_LIBRARY "${CMAKE_MSVC_RUNTIME_LIBRARY}"
        )
    endif()

    # Compile definitions for shared lib
    target_compile_definitions(CYLogger_shared PRIVATE
        CYLOGGER_USE_DLL
        CYLOGGER_EXPORTS
    )

    # Platform definitions
    if(WIN32)
        target_compile_definitions(CYLogger_shared PRIVATE
            WIN32
            $<$<CONFIG:Debug>:_DEBUG>
            $<$<CONFIG:Release>:NDEBUG>
        )
    elseif(IOS)
        target_compile_definitions(CYLogger_shared PRIVATE CYLOGGER_IOS_OS)
    elseif(APPLE)
        target_compile_definitions(CYLogger_shared PRIVATE CYLOGGER_MAC_OS)
    elseif(ANDROID)
        target_compile_definitions(CYLogger_shared PRIVATE CYLOGGER_ANDROID_OS)
    elseif(UNIX)
        target_compile_definitions(CYLogger_shared PRIVATE CYLOGGER_LINUX_OS)
    endif()

    # Link dependencies for shared lib
    find_package(Threads REQUIRED)

    if(USE_CYCOMMON)
        if(TARGET CYCommon::static)
            target_link_libraries(CYLogger_shared PUBLIC Threads::Threads CYCommon::static fmt::fmt)
        elseif(TARGET CYCommon_static)
            target_link_libraries(CYLogger_shared PUBLIC Threads::Threads CYCommon_static fmt::fmt)
        else()
            target_link_libraries(CYLogger_shared PUBLIC Threads::Threads fmt::fmt)
        endif()
    else()
        target_link_libraries(CYLogger_shared PUBLIC Threads::Threads fmt::fmt)
    endif()

    if(USE_CYCOROUTINE)
        if(TARGET CYCoroutine::shared)
            target_link_libraries(CYLogger_shared PUBLIC CYCoroutine::shared)
        elseif(TARGET CYCoroutine_shared)
            target_link_libraries(CYLogger_shared PUBLIC CYCoroutine_shared)
        elseif(TARGET CYCoroutine::static)
            target_link_libraries(CYLogger_shared PUBLIC CYCoroutine::static)
        elseif(TARGET CYCoroutine_static)
            target_link_libraries(CYLogger_shared PUBLIC CYCoroutine_static)
        endif()
    endif()

    if(WIN32)
        target_link_libraries(CYLogger_shared PUBLIC
            ws2_32 shell32 kernel32
        )
    endif()

    # Alias: CYLogger_shared -> CYLogger (backward compat for install rules)
    # Only create if CYLogger_shared actually exists (not skipped)
    if(NOT CYLOGGER_SKIP_SHARED)
        add_library(CYLogger ALIAS CYLogger_shared)
    endif()

    target_include_directories(CYLogger_shared
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../Inc
    )

    if(USE_CYCOMMON AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/CYCommon/Inc")
        target_include_directories(CYLogger_shared PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/CYCommon/Inc")
    endif()

    install(TARGETS CYLogger_shared
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )
endif()

# =============================================================================
# Static library target (CYLoggerStatic.lib / CYLoggerStaticD.lib)
# =============================================================================
if(BUILD_STATIC_LIBS)
    # Always name the static target CYLoggerStatic for consistency
    add_library(CYLoggerStatic STATIC ${CYLOGGER_SOURCES})

        set_target_properties(CYLoggerStatic PROPERTIES
            VERSION ${PROJECT_VERSION}
            SOVERSION ${PROJECT_VERSION_MAJOR}
            OUTPUT_NAME "${_CYLOGGER_STATIC_OUTPUT_NAME}"
            PDB_NAME "${_CYLOGGER_STATIC_OUTPUT_NAME}"
        )

    # PDB generation
    if(MSVC)
        if(CMAKE_CONFIGURATION_TYPES)
            foreach(_cfg ${CMAKE_CONFIGURATION_TYPES})
                string(TOUPPER "${_cfg}" _cfg_upper)
                set_target_properties(CYLoggerStatic PROPERTIES
                    PDB_NAME "${_CYLOGGER_STATIC_OUTPUT_NAME}"
                    PDB_OUTPUT_DIRECTORY_${_cfg_upper} "${_CYLOGGER_OUTPUT_BASE}/${_cfg}"
                )
            endforeach()
        else()
            set_target_properties(CYLoggerStatic PROPERTIES
                PDB_NAME "${_CYLOGGER_STATIC_OUTPUT_NAME}"
                PDB_OUTPUT_DIRECTORY "${_CYLOGGER_OUTPUT_BASE}/${CMAKE_BUILD_TYPE}"
            )
        endif()
    endif()

    # MSVC runtime
    if(MSVC AND DEFINED CMAKE_MSVC_RUNTIME_LIBRARY)
        set_target_properties(CYLoggerStatic PROPERTIES
            MSVC_RUNTIME_LIBRARY "${CMAKE_MSVC_RUNTIME_LIBRARY}"
        )
    endif()

    # Compile definitions for static lib
    target_compile_definitions(CYLoggerStatic PRIVATE CYLOGGER_STATIC)

    # Platform definitions
    if(WIN32)
        target_compile_definitions(CYLoggerStatic PRIVATE
            WIN32
            $<$<CONFIG:Debug>:_DEBUG>
            $<$<CONFIG:Release>:NDEBUG>
        )
    elseif(IOS)
        target_compile_definitions(CYLoggerStatic PRIVATE CYLOGGER_IOS_OS)
    elseif(APPLE)
        target_compile_definitions(CYLoggerStatic PRIVATE CYLOGGER_MAC_OS)
    elseif(ANDROID)
        target_compile_definitions(CYLoggerStatic PRIVATE CYLOGGER_ANDROID_OS)
    elseif(UNIX)
        target_compile_definitions(CYLoggerStatic PRIVATE CYLOGGER_LINUX_OS)
    endif()

    target_include_directories(CYLoggerStatic
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../Inc
    )

    if(USE_CYCOMMON AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/CYCommon/Inc")
        target_include_directories(CYLoggerStatic PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/CYCommon/Inc")
    endif()

    # Link dependencies for static lib
    find_package(Threads REQUIRED)

    if(USE_CYCOMMON)
        if(TARGET CYCommon::static)
            target_link_libraries(CYLoggerStatic PUBLIC Threads::Threads CYCommon::static fmt::fmt)
        elseif(TARGET CYCommon_static)
            target_link_libraries(CYLoggerStatic PUBLIC Threads::Threads CYCommon_static fmt::fmt)
        else()
            target_link_libraries(CYLoggerStatic PUBLIC Threads::Threads fmt::fmt)
        endif()
    else()
        target_link_libraries(CYLoggerStatic PUBLIC Threads::Threads fmt::fmt)
    endif()

    if(USE_CYCOROUTINE)
        if(TARGET CYCoroutine::static)
            target_link_libraries(CYLoggerStatic PUBLIC CYCoroutine::static)
        elseif(TARGET CYCoroutine_static)
            target_link_libraries(CYLoggerStatic PUBLIC CYCoroutine_static)
        endif()
    endif()

    if(WIN32)
        target_link_libraries(CYLoggerStatic PUBLIC
            ws2_32 shell32 kernel32
        )
    endif()

    target_include_directories(CYLoggerStatic
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../Inc
    )

    if(USE_CYCOMMON AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/CYCommon/Inc")
        target_include_directories(CYLoggerStatic PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/CYCommon/Inc")
    endif()

    install(TARGETS CYLoggerStatic
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
endif()
