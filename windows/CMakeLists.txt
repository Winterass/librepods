cmake_minimum_required(VERSION 3.16)

project(librepods-windows VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)

# Find Qt6 components
find_package(Qt6 REQUIRED COMPONENTS Core Quick Widgets Bluetooth)
find_package(OpenSSL REQUIRED)

qt_standard_project_setup()

# Windows-specific sources
set(WINDOWS_SOURCES
    main.cpp
    bluetooth/WindowsBluetoothManager.cpp
    bluetooth/WindowsBluetoothManager.h
    media/WindowsAudioController.cpp
    media/WindowsAudioController.h
    media/WindowsMediaController.cpp
    media/WindowsMediaController.h
    AutostartManager.cpp
    AutostartManager.h
)

# Include parent directory sources (from linux/)
set(PARENT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/..)
set(LINUX_DIR ${PARENT_DIR}/linux)

set(SHARED_SOURCES
    ${LINUX_DIR}/airpods_packets.h
    ${LINUX_DIR}/logger.h
    ${LINUX_DIR}/enums.h
    ${LINUX_DIR}/battery.hpp
    ${LINUX_DIR}/deviceinfo.hpp
    ${LINUX_DIR}/eardetection.hpp
    ${LINUX_DIR}/BasicControlCommand.hpp
    ${LINUX_DIR}/QRCodeImageProvider.hpp
    ${LINUX_DIR}/thirdparty/QR-Code-generator/qrcodegen.cpp
    ${LINUX_DIR}/thirdparty/QR-Code-generator/qrcodegen.hpp
)

# Main executable
qt_add_executable(librepods-windows
    ${WINDOWS_SOURCES}
    ${SHARED_SOURCES}
)

# QML module (reuse from Linux)
qt_add_qml_module(librepods-windows
    URI windows
    VERSION 1.0
    QML_FILES
        ${LINUX_DIR}/Main.qml
        ${LINUX_DIR}/BatteryIndicator.qml
        ${LINUX_DIR}/SegmentedControl.qml
        ${LINUX_DIR}/PodColumn.qml
        ${LINUX_DIR}/Icon.qml
        ${LINUX_DIR}/KeysQRDialog.qml
)

# Add resources
qt_add_resources(librepods-windows "resources"
    PREFIX "/icons"
    FILES
        ${LINUX_DIR}/assets/airpods.png
        ${LINUX_DIR}/assets/pod.png
        ${LINUX_DIR}/assets/pod_case.png
        ${LINUX_DIR}/assets/pod3.png
        ${LINUX_DIR}/assets/pod3_case.png
        ${LINUX_DIR}/assets/pod4_case.png
        ${LINUX_DIR}/assets/podpro.png
        ${LINUX_DIR}/assets/podpro_case.png
        ${LINUX_DIR}/assets/podmax.png
        ${LINUX_DIR}/assets/fonts/SF-Symbols-6.ttf
)

# Windows-specific libraries
target_link_libraries(librepods-windows
    PRIVATE 
    Qt6::Core
    Qt6::Quick
    Qt6::Widgets
    Qt6::Bluetooth
    OpenSSL::SSL
    OpenSSL::Crypto
    # Windows-specific libraries
    wsock32
    ws2_32
    ole32
    winmm
)

# Include directories
target_include_directories(librepods-windows PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${LINUX_DIR}
)

# Set Windows subsystem to GUI (not console)
if(WIN32)
    set_target_properties(librepods-windows PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
endif()

# Install targets
include(GNUInstallDirs)
install(TARGETS librepods-windows
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Install Python bridge script
install(FILES bluetooth/bumble_bridge.py
    DESTINATION ${CMAKE_INSTALL_BINDIR}/windows/bluetooth
)

# Install requirements file
install(FILES ${PARENT_DIR}/requirements-windows.txt
    DESTINATION ${CMAKE_INSTALL_BINDIR}
)
