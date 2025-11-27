cmake_minimum_required(VERSION 3.16)

project(librepods_windows_monitor LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Bluetooth)

add_library(librepods_windows_monitor STATIC
    bluetooth/WindowsBluetoothMonitor.cpp
    bluetooth/WindowsBluetoothMonitor.h
    ../bluetooth/BleInfo.h
    ../bluetooth/IBluetoothMonitor.h
    ../linux/ble/blemanager.cpp
    ../linux/ble/blemanager.h
)

target_link_libraries(librepods_windows_monitor PRIVATE Qt6::Bluetooth)

target_include_directories(librepods_windows_monitor
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/..
        ${CMAKE_CURRENT_LIST_DIR}/../linux
)
