cmake_minimum_required(VERSION 3.16)

project(linux VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(LIBREPODS_ENABLE_WINDOWS_TARGET "Enable building the Windows monitor target" OFF)

set(QT_REQUIRED_COMPONENTS Quick Widgets Bluetooth)
if(NOT WIN32)
    list(APPEND QT_REQUIRED_COMPONENTS DBus)
endif()

find_package(Qt6 REQUIRED COMPONENTS ${QT_REQUIRED_COMPONENTS})
find_package(OpenSSL REQUIRED)
if(NOT WIN32)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(PULSEAUDIO REQUIRED libpulse)
endif()

qt_standard_project_setup()

qt_add_executable(librepods
    main.cpp
    logger.h
    media/mediacontroller.cpp
    media/mediacontroller.h
    media/pulseaudiocontroller.cpp
    media/pulseaudiocontroller.h
    airpods_packets.h
    trayiconmanager.cpp
    trayiconmanager.h
    enums.h
    battery.hpp
    bluetooth/BleInfo.h
    bluetooth/IBluetoothMonitor.h
    BluezBluetoothMonitor.cpp
    BluezBluetoothMonitor.h
    autostartmanager.hpp
    BasicControlCommand.hpp
    deviceinfo.hpp
    ble/aacptransport.h
    ble/qt_aacptransport.cpp
    ble/qt_aacptransport.h
    ../bluetooth/BleCryptoUtils.cpp
    ../bluetooth/BleCryptoUtils.h
    ble/blemanager.cpp
    ble/blemanager.h
    thirdparty/QR-Code-generator/qrcodegen.cpp
    thirdparty/QR-Code-generator/qrcodegen.hpp
    QRCodeImageProvider.hpp
    eardetection.hpp
    media/playerstatuswatcher.cpp
    media/playerstatuswatcher.h
    systemsleepmonitor.hpp
)

qt_add_qml_module(librepods
    URI linux
    VERSION 1.0
    QML_FILES
        Main.qml
        BatteryIndicator.qml
        SegmentedControl.qml
        PodColumn.qml
        Icon.qml
        KeysQRDialog.qml
        Onboarding.qml
)

# Add the resource file
qt_add_resources(librepods "resources"
    PREFIX "/icons"
    FILES
        assets/airpods.png
        assets/pod.png
        assets/pod_case.png
        assets/pod3.png
        assets/pod3_case.png
        assets/pod4_case.png
        assets/podpro.png
        assets/podpro_case.png
        assets/podmax.png
        assets/fonts/SF-Symbols-6.ttf
)

target_link_libraries(librepods
    PRIVATE Qt6::Quick Qt6::Widgets Qt6::Bluetooth OpenSSL::SSL OpenSSL::Crypto
)

target_include_directories(librepods PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)

if(NOT WIN32)
    target_link_libraries(librepods PRIVATE Qt6::DBus ${PULSEAUDIO_LIBRARIES})
    target_include_directories(librepods PRIVATE ${PULSEAUDIO_INCLUDE_DIRS})
endif()

include(GNUInstallDirs)
install(TARGETS librepods
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
install(FILES assets/me.kavishdevar.librepods.desktop
    DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/applications")
install(FILES assets/librepods.png
    DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/512x512/apps")

if(LIBREPODS_ENABLE_WINDOWS_TARGET)
    add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../windows windows-build)
endif()
