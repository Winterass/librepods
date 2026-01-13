#ifndef WINDOWSBLUETOOTHMANAGER_H
#define WINDOWSBLUETOOTHMANAGER_H

#include <QObject>
#include <QProcess>
#include <QString>
#include <QByteArray>
#include <QTimer>
#include <QLocalSocket>

/**
 * @brief Windows Bluetooth Manager using Bumble Python bridge
 * 
 * This class manages Bluetooth L2CAP communication on Windows by interfacing
 * with a Python process running the Bumble Bluetooth stack.
 * Communication is done via IPC (stdin/stdout) with the Python bridge.
 */
class WindowsBluetoothManager : public QObject
{
    Q_OBJECT

public:
    explicit WindowsBluetoothManager(QObject *parent = nullptr);
    ~WindowsBluetoothManager();

    /**
     * @brief Connect to AirPods at the specified Bluetooth address
     * @param address Bluetooth MAC address (format: XX:XX:XX:XX:XX:XX)
     * @return true if connection initiated successfully
     */
    bool connectToDevice(const QString &address);

    /**
     * @brief Disconnect from currently connected AirPods
     */
    void disconnect();

    /**
     * @brief Send data to connected AirPods
     * @param data Raw packet data to send
     * @return true if data was sent successfully
     */
    bool sendData(const QByteArray &data);

    /**
     * @brief Check if currently connected to AirPods
     * @return true if connected
     */
    bool isConnected() const { return m_connected; }

    /**
     * @brief Get the current device address
     * @return Bluetooth MAC address
     */
    QString deviceAddress() const { return m_deviceAddress; }

signals:
    /**
     * @brief Emitted when connection to AirPods is established
     */
    void connected();

    /**
     * @brief Emitted when disconnected from AirPods
     */
    void disconnected();

    /**
     * @brief Emitted when data is received from AirPods
     * @param data Raw packet data received
     */
    void dataReceived(const QByteArray &data);

    /**
     * @brief Emitted when an error occurs
     * @param error Error message
     */
    void error(const QString &error);

private slots:
    void onProcessReadyRead();
    void onProcessErrorOccurred(QProcess::ProcessError error);
    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onConnectionTimeout();

private:
    void startBumbleProcess();
    void stopBumbleProcess();
    void processReceivedLine(const QString &line);
    void sendCommand(const QString &command, const QString &data = QString());

    QProcess *m_process;
    QString m_deviceAddress;
    bool m_connected;
    QTimer *m_connectionTimer;
    QByteArray m_receiveBuffer;
    
    static constexpr int CONNECTION_TIMEOUT_MS = 10000; // 10 seconds
    static constexpr const char* BRIDGE_SCRIPT_NAME = "bumble_bridge.py";
};

#endif // WINDOWSBLUETOOTHMANAGER_H
