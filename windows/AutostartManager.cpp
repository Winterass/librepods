#include "AutostartManager.h"
#include <QCoreApplication>
#include <QDebug>
#include <QDir>

#ifdef _WIN32
#include <windows.h>
#endif

WindowsAutostartManager::WindowsAutostartManager(QObject *parent)
    : QObject(parent)
{
}

WindowsAutostartManager::~WindowsAutostartManager()
{
}

bool WindowsAutostartManager::autoStartEnabled() const
{
    return checkAutoStartEntry();
}

void WindowsAutostartManager::setAutoStartEnabled(bool enabled)
{
    if (autoStartEnabled() == enabled) {
        return;
    }

    bool success = false;
    if (enabled) {
        success = createAutoStartEntry();
    } else {
        success = removeAutoStartEntry();
    }

    if (success) {
        emit autoStartEnabledChanged(enabled);
    }
}

bool WindowsAutostartManager::createAutoStartEntry()
{
#ifdef _WIN32
    HKEY hKey;
    LONG result = RegOpenKeyExW(
        HKEY_CURRENT_USER,
        REGISTRY_KEY_PATH,
        0,
        KEY_SET_VALUE,
        &hKey
    );

    if (result != ERROR_SUCCESS) {
        qWarning() << "Failed to open registry key for autostart:" << result;
        return false;
    }

    // Get application path
    QString appPath = QCoreApplication::applicationFilePath();
    appPath = QDir::toNativeSeparators(appPath);
    
    // Add --hide flag to start minimized
    appPath += " --hide";
    
    // Convert to wide string
    std::wstring wAppPath = appPath.toStdWString();

    // Set registry value
    result = RegSetValueExW(
        hKey,
        APP_REGISTRY_NAME,
        0,
        REG_SZ,
        (const BYTE*)wAppPath.c_str(),
        (wAppPath.length() + 1) * sizeof(wchar_t)
    );

    RegCloseKey(hKey);

    if (result != ERROR_SUCCESS) {
        qWarning() << "Failed to set registry value for autostart:" << result;
        return false;
    }

    qDebug() << "Autostart enabled successfully";
    return true;
#else
    qWarning() << "WindowsAutostartManager only works on Windows";
    return false;
#endif
}

bool WindowsAutostartManager::removeAutoStartEntry()
{
#ifdef _WIN32
    HKEY hKey;
    LONG result = RegOpenKeyExW(
        HKEY_CURRENT_USER,
        REGISTRY_KEY_PATH,
        0,
        KEY_SET_VALUE,
        &hKey
    );

    if (result != ERROR_SUCCESS) {
        qWarning() << "Failed to open registry key for autostart:" << result;
        return false;
    }

    // Delete registry value
    result = RegDeleteValueW(hKey, APP_REGISTRY_NAME);
    RegCloseKey(hKey);

    if (result != ERROR_SUCCESS && result != ERROR_FILE_NOT_FOUND) {
        qWarning() << "Failed to delete registry value for autostart:" << result;
        return false;
    }

    qDebug() << "Autostart disabled successfully";
    return true;
#else
    qWarning() << "WindowsAutostartManager only works on Windows";
    return false;
#endif
}

bool WindowsAutostartManager::checkAutoStartEntry() const
{
#ifdef _WIN32
    HKEY hKey;
    LONG result = RegOpenKeyExW(
        HKEY_CURRENT_USER,
        REGISTRY_KEY_PATH,
        0,
        KEY_QUERY_VALUE,
        &hKey
    );

    if (result != ERROR_SUCCESS) {
        return false;
    }

    // Check if value exists
    result = RegQueryValueExW(hKey, APP_REGISTRY_NAME, nullptr, nullptr, nullptr, nullptr);
    RegCloseKey(hKey);

    return result == ERROR_SUCCESS;
#else
    return false;
#endif
}
