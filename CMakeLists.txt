cmake_minimum_required(VERSION 3.16)

project(librepods VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Determine platform and build appropriate target
if(WIN32)
    message(STATUS "Building for Windows")
    add_subdirectory(windows)
elseif(UNIX AND NOT APPLE)
    message(STATUS "Building for Linux")
    add_subdirectory(linux)
elseif(ANDROID)
    message(STATUS "Building for Android")
    # Android build is handled separately via Gradle
else()
    message(WARNING "Unsupported platform")
endif()
