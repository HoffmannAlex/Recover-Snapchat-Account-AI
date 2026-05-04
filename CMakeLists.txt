# CMakeLists.txt for Recover-Snapchat-Account-AI
# C Implementation - Version 2026.1

cmake_minimum_required(VERSION 3.10)
project(RecoverSnapchatAccountAI C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Version
set(VERSION_MAJOR 2026)
set(VERSION_MINOR 1)

# Source directory
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

# Source files
set(SOURCES
    ${SRC_DIR}/main.c
    ${SRC_DIR}/logging.c
    ${SRC_DIR}/password_generator.c
    ${SRC_DIR}/request_manager.c
    ${SRC_DIR}/proxy_manager.c
    ${SRC_DIR}/tor_manager.c
    ${SRC_DIR}/csrf_manager.c
    ${SRC_DIR}/monitoring.c
    ${SRC_DIR}/security_tester.c
)

# Header files (for IDE support)
set(HEADERS
    ${SRC_DIR}/logging.h
    ${SRC_DIR}/password_generator.h
    ${SRC_DIR}/request_manager.h
    ${SRC_DIR}/proxy_manager.h
    ${SRC_DIR}/tor_manager.h
    ${SRC_DIR}/csrf_manager.h
    ${SRC_DIR}/monitoring.h
    ${SRC_DIR}/security_tester.h
)

# Find required packages
find_package(CURL REQUIRED)
find_package(Threads REQUIRED)

# Create executable
add_executable(snapchat_security_test ${SOURCES} ${HEADERS})

# Include directories
target_include_directories(snapchat_security_test PRIVATE ${SRC_DIR})

# Link libraries
target_link_libraries(snapchat_security_test 
    CURL::libcurl
    Threads::Threads
    ${CMAKE_DL_LIBS}
)

# Platform specific settings
if(WIN32)
    target_link_libraries(snapchat_security_test ws2_32)
    target_compile_definitions(snapchat_security_test PRIVATE _WIN32_WINNT=0x0601)
endif()

# Compiler warnings
if(MSVC)
    target_compile_options(snapchat_security_test PRIVATE /W4)
else()
    target_compile_options(snapchat_security_test PRIVATE -Wall -Wextra -pedantic)
endif()

# Debug/Release configurations
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(snapchat_security_test PRIVATE DEBUG)
    if(NOT MSVC)
        target_compile_options(snapchat_security_test PRIVATE -g -O0)
    endif()
else()
    target_compile_definitions(snapchat_security_test PRIVATE NDEBUG)
    if(NOT MSVC)
        target_compile_options(snapchat_security_test PRIVATE -O3)
    endif()
endif()

# Define version macro
target_compile_definitions(snapchat_security_test PRIVATE 
    VERSION_MAJOR=${VERSION_MAJOR}
    VERSION_MINOR=${VERSION_MINOR}
)

# Installation
install(TARGETS snapchat_security_test RUNTIME DESTINATION bin)

# CPack configuration for packaging
set(CPACK_PACKAGE_NAME "RecoverSnapchatAccountAI")
set(CPACK_PACKAGE_VERSION_MAJOR ${VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${VERSION_MINOR})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Educational Security Testing Tool")
include(CPack)
