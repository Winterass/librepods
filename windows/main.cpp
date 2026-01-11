/*
 * LibrePods Windows Main Application
 * 
 * This is a simplified Windows implementation that demonstrates
 * the integration of Windows-specific components.
 * 
 * For full feature parity with Linux, additional integration work is needed.
 */

#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include <QDebug>
#include <QCommandLineParser>

#include "bluetooth/WindowsBluetoothManager.h"
#include "media/WindowsAudioController.h"
#include "media/WindowsMediaController.h"
#include "AutostartManager.h"

// Include shared components from Linux
#include "../linux/logger.h"
#include "../linux/deviceinfo.hpp"
#include "../linux/battery.hpp"
#include "../linux/eardetection.hpp"

Q_LOGGING_CATEGORY(librepods, "librepods")

class WindowsAirPodsApp : public QObject
{
    Q_OBJECT

public:
    WindowsAirPodsApp(bool debugMode, QObject *parent = nullptr)
        : QObject(parent)
        , m_debugMode(debugMode)
        , m_bluetoothManager(new WindowsBluetoothManager(this))
        , m_audioController(new WindowsAudioController(this))
        , m_mediaController(new WindowsMediaController(this))
        , m_autostartManager(new WindowsAutostartManager(this))
        , m_deviceInfo(new DeviceInfo(this))
        , m_earDetection(new EarDetection(this))
        , m_trayIcon(new QSystemTrayIcon(this))
        , m_connected(false)
    {
        QLoggingCategory::setFilterRules(QString("librepods.debug=%1").arg(debugMode ? "true" : "false"));
        LOG_INFO("Initializing LibrePods for Windows");

        // Initialize components
        if (!m_audioController->initialize()) {
            LOG_ERROR("Failed to initialize audio controller");
        }

        if (!m_mediaController->initialize()) {
            LOG_WARN("Failed to initialize media controller (will use keyboard simulation)");
        }

        // Connect signals
        setupSignals();
        setupTrayIcon();

        LOG_INFO("LibrePods Windows initialized");
    }

    void connectToAirPods(const QString &address)
    {
        LOG_INFO("Connecting to AirPods at: " << address);
        m_bluetoothManager->connectToDevice(address);
    }

private:
    void setupSignals()
    {
        // Bluetooth signals
        connect(m_bluetoothManager, &WindowsBluetoothManager::connected,
                this, &WindowsAirPodsApp::onBluetoothConnected);
        connect(m_bluetoothManager, &WindowsBluetoothManager::disconnected,
                this, &WindowsAirPodsApp::onBluetoothDisconnected);
        connect(m_bluetoothManager, &WindowsBluetoothManager::dataReceived,
                this, &WindowsAirPodsApp::onDataReceived);
        connect(m_bluetoothManager, &WindowsBluetoothManager::error,
                this, &WindowsAirPodsApp::onBluetoothError);

        // Ear detection
        connect(m_earDetection, &EarDetection::statusChanged,
                this, &WindowsAirPodsApp::onEarDetectionChanged);

        // Device info signals
        connect(m_deviceInfo->getBattery(), &Battery::batteryChanged,
                this, &WindowsAirPodsApp::onBatteryChanged);
    }

    void setupTrayIcon()
    {
        // Load icon
        QIcon icon(":/icons/airpods.png");
        m_trayIcon->setIcon(icon);
        m_trayIcon->setToolTip("LibrePods");

        // Create context menu
        QMenu *trayMenu = new QMenu();

        QAction *showAction = new QAction("Show", this);
        connect(showAction, &QAction::triggered, this, []() {
            LOG_DEBUG("Show action triggered");
        });

        QAction *exitAction = new QAction("Exit", this);
        connect(exitAction, &QAction::triggered, qApp, &QApplication::quit);

        trayMenu->addAction(showAction);
        trayMenu->addSeparator();
        trayMenu->addAction(exitAction);

        m_trayIcon->setContextMenu(trayMenu);
        m_trayIcon->show();
    }

    void onBluetoothConnected()
    {
        LOG_INFO("Connected to AirPods");
        m_connected = true;
        m_trayIcon->showMessage("LibrePods", "Connected to AirPods",
                                QSystemTrayIcon::Information, 3000);
    }

    void onBluetoothDisconnected()
    {
        LOG_INFO("Disconnected from AirPods");
        m_connected = false;
        m_deviceInfo->reset();
        m_earDetection->reset();
    }

    void onBluetoothError(const QString &error)
    {
        LOG_ERROR("Bluetooth error: " << error);
        m_trayIcon->showMessage("LibrePods Error", error,
                               QSystemTrayIcon::Critical, 5000);
    }

    void onDataReceived(const QByteArray &data)
    {
        if (data.size() < 6) {
            return;
        }

        // Parse packet type
        quint8 packetType = static_cast<quint8>(data[4]);
        
        LOG_DEBUG("Received packet, type: 0x" << QString::number(packetType, 16));

        switch (packetType) {
            case 0x04: // Battery status
                if (m_deviceInfo->getBattery()->parseData(data)) {
                    LOG_DEBUG("Battery status updated");
                }
                break;

            case 0x06: // Ear detection
                if (m_earDetection->parseData(data)) {
                    LOG_DEBUG("Ear detection updated");
                }
                break;

            case 0x09: // Noise control
                if (data.size() >= 8) {
                    quint8 subType = static_cast<quint8>(data[6]);
                    if (subType == 0x0D) {
                        // Noise control mode
                        quint8 mode = static_cast<quint8>(data[7]);
                        LOG_DEBUG("Noise control mode: " << mode);
                        // Update device info
                    }
                }
                break;

            default:
                LOG_DEBUG("Unknown packet type");
                break;
        }
    }

    void onEarDetectionChanged()
    {
        bool primaryInEar = m_earDetection->isPrimaryInEar();
        bool secondaryInEar = m_earDetection->isSecondaryInEar();

        LOG_DEBUG("Ear detection: primary=" << primaryInEar 
                  << " secondary=" << secondaryInEar);

        // Handle auto-pause/resume
        if (!primaryInEar && !secondaryInEar) {
            // Both AirPods removed - pause media
            LOG_DEBUG("Both AirPods removed, pausing media");
            m_mediaController->pause();
        } else if (primaryInEar || secondaryInEar) {
            // At least one AirPod in ear - could resume
            LOG_DEBUG("AirPod(s) in ear");
        }
    }

    void onBatteryChanged()
    {
        Battery *battery = m_deviceInfo->getBattery();
        LOG_DEBUG("Battery updated - Left: " << battery->left() 
                  << "% Right: " << battery->right()
                  << "% Case: " << battery->pod_case() << "%");

        // Update tray tooltip
        QString tooltip = QString("LibrePods - L:%1% R:%2% C:%3%")
            .arg(battery->left())
            .arg(battery->right())
            .arg(battery->pod_case());
        m_trayIcon->setToolTip(tooltip);
    }

private:
    bool m_debugMode;
    WindowsBluetoothManager *m_bluetoothManager;
    WindowsAudioController *m_audioController;
    WindowsMediaController *m_mediaController;
    WindowsAutostartManager *m_autostartManager;
    DeviceInfo *m_deviceInfo;
    EarDetection *m_earDetection;
    QSystemTrayIcon *m_trayIcon;
    bool m_connected;
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setOrganizationName("LibrePods");
    app.setApplicationName("LibrePods");
    app.setApplicationVersion("0.1.0");

    // Parse command line arguments
    QCommandLineParser parser;
    parser.setApplicationDescription("LibrePods - AirPods for Windows");
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption debugOption(QStringList() << "d" << "debug",
        "Enable debug logging");
    parser.addOption(debugOption);

    QCommandLineOption hideOption(QStringList() << "hide",
        "Start minimized to tray");
    parser.addOption(hideOption);

    QCommandLineOption addressOption(QStringList() << "a" << "address",
        "Bluetooth MAC address of AirPods",
        "address");
    parser.addOption(addressOption);

    parser.process(app);

    bool debugMode = parser.isSet(debugOption);
    QString address = parser.value(addressOption);

    // Create main application
    WindowsAirPodsApp *mainApp = new WindowsAirPodsApp(debugMode);

    // Auto-connect if address provided
    if (!address.isEmpty()) {
        QTimer::singleShot(1000, [mainApp, address]() {
            mainApp->connectToAirPods(address);
        });
    }

    return app.exec();
}

#include "main.moc"
