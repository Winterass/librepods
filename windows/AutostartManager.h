#ifndef WINDOWSAUTOSTARTMANAGER_H
#define WINDOWSAUTOSTARTMANAGER_H

#include <QObject>
#include <QString>

#ifdef _WIN32
#include <windows.h>
#endif

/**
 * @brief Windows Autostart Manager using Windows Registry
 * 
 * Manages application autostart on Windows by manipulating the
 * HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Run registry key.
 * Similar to autostartmanager.hpp on Linux but using Windows Registry.
 */
class WindowsAutostartManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool autoStartEnabled READ autoStartEnabled WRITE setAutoStartEnabled NOTIFY autoStartEnabledChanged)

public:
    explicit WindowsAutostartManager(QObject *parent = nullptr);
    ~WindowsAutostartManager();

    /**
     * @brief Check if autostart is currently enabled
     * @return true if enabled
     */
    bool autoStartEnabled() const;

    /**
     * @brief Enable or disable autostart
     * @param enabled true to enable, false to disable
     */
    void setAutoStartEnabled(bool enabled);

signals:
    void autoStartEnabledChanged(bool enabled);

private:
    bool createAutoStartEntry();
    bool removeAutoStartEntry();
    bool checkAutoStartEntry() const;

    static constexpr const wchar_t* REGISTRY_KEY_PATH = 
        L"Software\\Microsoft\\Windows\\CurrentVersion\\Run";
    static constexpr const wchar_t* APP_REGISTRY_NAME = L"LibrePods";
};

#endif // WINDOWSAUTOSTARTMANAGER_H
