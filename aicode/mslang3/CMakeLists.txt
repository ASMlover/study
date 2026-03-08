# Maple Script Language - CMake Build Configuration
# =================================================

cmake_minimum_required(VERSION 3.20)

# Project Definition
# ------------------
project(maple
    VERSION 0.1.0
    DESCRIPTION "Maple Script Language - A bytecode-based scripting language"
    LANGUAGES CXX
)

# C++ Standard Requirement
# ------------------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# MSVC UTF-8 encoding support
if(MSVC)
    add_compile_options(/utf-8)
    # Note: /Zc:preprocessor removed as it causes issues with standard library headers
endif()

# Compiler Detection and Validation
# ---------------------------------
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    # MSVC 2022+ required for C++23 support
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "19.30")
        message(FATAL_ERROR "MSVC 2022 (19.30+) is required for C++23 support. "
                            "Current version: ${CMAKE_CXX_COMPILER_VERSION}")
    endif()
    message(STATUS "Using MSVC ${CMAKE_CXX_COMPILER_VERSION}")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    # GCC 13+ required for full C++23 support
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "13.0")
        message(FATAL_ERROR "GCC 13+ is required for C++23 support. "
                            "Current version: ${CMAKE_CXX_COMPILER_VERSION}")
    endif()
    message(STATUS "Using GCC ${CMAKE_CXX_COMPILER_VERSION}")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    # Clang 16+ required for full C++23 support
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "16.0")
        message(FATAL_ERROR "Clang 16+ is required for C++23 support. "
                            "Current version: ${CMAKE_CXX_COMPILER_VERSION}")
    endif()
    message(STATUS "Using Clang ${CMAKE_CXX_COMPILER_VERSION}")
else()
    message(WARNING "Untested compiler: ${CMAKE_CXX_COMPILER_ID}. "
                    "C++23 support may be incomplete.")
endif()

# Build Type Configuration
# ------------------------
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
endif()
set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "RelWithDebInfo" "MinSizeRel")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

# Debug Options
# -------------
option(ENABLE_DEBUG "Enable debug mode with all debug features" OFF)
option(ENABLE_STRESS_GC "Enable GC stress testing (trigger GC on every allocation)" OFF)

if(ENABLE_DEBUG)
    message(STATUS "Debug mode enabled - all debug features active")
endif()

if(ENABLE_STRESS_GC)
    message(STATUS "GC stress testing enabled")
endif()

# Output Directories
# ------------------
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Per-configuration output directories
foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR}/bin)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR}/lib)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR}/lib)
endforeach()

# Source Files Collection
# -----------------------
file(GLOB_RECURSE MAPLE_SOURCES
    CONFIGURE_DEPENDS
    ${CMAKE_SOURCE_DIR}/src/*.cc
)
# Filter out test files
list(FILTER MAPLE_SOURCES EXCLUDE REGEX "test\\.cc$")

file(GLOB_RECURSE MAPLE_HEADERS
    CONFIGURE_DEPENDS
    ${CMAKE_SOURCE_DIR}/src/*.hh
)

if(NOT MAPLE_SOURCES)
    message(WARNING "No source files found in ${CMAKE_SOURCE_DIR}/src/")
endif()

message(STATUS "Source files found: ${MAPLE_SOURCES}")

# Main Executable Target
# ----------------------
add_executable(maple ${MAPLE_SOURCES} ${MAPLE_HEADERS})

# Target Include Directories
# --------------------------
target_include_directories(maple
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src
)

# Compiler Warnings
# -----------------
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(maple PRIVATE
        /W3          # Enable warning level 3 (less strict than /W4 for C++23 compatibility)
        # /WX        # Temporarily disabled: Treat warnings as errors
        # /permissive- # Temporarily disabled: Causes conflicts with C++23 std::format
        /wd4996      # Disable deprecation warnings
        /Zc:__cplusplus # Enable updated __cplusplus macro
    )
    
    # Debug-specific flags for MSVC
    target_compile_options(maple PRIVATE
        $<$<CONFIG:Debug>:/Od /Zi>
        $<$<CONFIG:Release>:/O2>
    )
else()
    # GCC and Clang
    target_compile_options(maple PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        -Wshadow
        -Wnon-virtual-dtor
        -Wold-style-cast
        -Wcast-align
        -Wunused
        -Woverloaded-virtual
        -Wconversion
        -Wsign-conversion
        -Wnull-dereference
        -Wdouble-promotion
        -Wformat=2
        -Werror
    )
    
    # Additional flags for GCC only
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        target_compile_options(maple PRIVATE
            -Wmisleading-indentation
            -Wduplicated-cond
            -Wduplicated-branches
            -Wlogical-op
            -Wuseless-cast
        )
    endif()
    
    # Additional flags for Clang only
    if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        target_compile_options(maple PRIVATE
            -Wimplicit-fallthrough
        )
    endif()
    
    # Debug/Release specific flags
    target_compile_options(maple PRIVATE
        $<$<CONFIG:Debug>:-O0 -g -fno-omit-frame-pointer>
        $<$<CONFIG:Release>:-O3>
    )
endif()

# Debug Macro Definitions
# -----------------------
target_compile_definitions(maple PRIVATE
    # Debug feature macros - controlled by ENABLE_DEBUG option
    $<$<BOOL:${ENABLE_DEBUG}>:DEBUG_SCANNER>
    $<$<BOOL:${ENABLE_DEBUG}>:DEBUG_PARSER>
    $<$<BOOL:${ENABLE_DEBUG}>:DEBUG_COMPILE>
    $<$<BOOL:${ENABLE_DEBUG}>:DEBUG_TRACE_EXEC>
    $<$<BOOL:${ENABLE_DEBUG}>:DEBUG_LOG_GC>
    
    # GC stress testing - controlled by ENABLE_STRESS_GC option
    $<$<BOOL:${ENABLE_STRESS_GC}>:DEBUG_STRESS_GC>
    
    # Platform macros
    $<$<PLATFORM_ID:Windows>:PLATFORM_WINDOWS>
    $<$<PLATFORM_ID:Linux>:PLATFORM_LINUX>
    $<$<PLATFORM_ID:Darwin>:PLATFORM_MACOS>
    
    # Build type macros
    $<$<CONFIG:Debug>:BUILD_DEBUG>
    $<$<CONFIG:Release>:BUILD_RELEASE>
)

# Link Libraries
# --------------
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_link_options(maple PRIVATE
        $<$<CONFIG:Debug>:/DEBUG>
    )
else()
    # Link against math library on Unix systems
    if(UNIX AND NOT APPLE)
        target_link_libraries(maple PRIVATE m)
    endif()
    
    # Sanitizers for debug builds (optional, can be enabled)
    # target_compile_options(maple PRIVATE $<$<CONFIG:Debug>:-fsanitize=address,undefined>)
    # target_link_options(maple PRIVATE $<$<CONFIG:Debug>:-fsanitize=address,undefined>)
endif()

# Unit Tests
# ----------
# 创建不包含main.cc的源文件列表用于测试
set(MAPLE_LIB_SOURCES ${MAPLE_SOURCES})
list(FILTER MAPLE_LIB_SOURCES EXCLUDE REGEX "main\\.cc$")

add_executable(test_phase2
    tests/unit/test_phase2.cpp
    ${MAPLE_LIB_SOURCES}
)

target_include_directories(test_phase2
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src
)

target_compile_options(test_phase2 PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W3 /Zc:__cplusplus>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra>
)

# Installation Rules
# ------------------
install(TARGETS maple
    RUNTIME DESTINATION bin
)

# Print Configuration Summary
# ---------------------------
message(STATUS "")
message(STATUS "=== Maple Build Configuration ===")
message(STATUS "Project: ${PROJECT_NAME} v${PROJECT_VERSION}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Enable Debug: ${ENABLE_DEBUG}")
message(STATUS "Enable Stress GC: ${ENABLE_STRESS_GC}")
message(STATUS "Output Directory: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
message(STATUS "Source Files: ${MAPLE_SOURCES}")
message(STATUS "=================================")
message(STATUS "")
