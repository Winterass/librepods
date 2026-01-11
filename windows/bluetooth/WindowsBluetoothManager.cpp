#include "WindowsBluetoothManager.h"
#include <QCoreApplication>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>

WindowsBluetoothManager::WindowsBluetoothManager(QObject *parent)
    : QObject(parent)
    , m_process(nullptr)
    , m_connected(false)
    , m_connectionTimer(new QTimer(this))
{
    m_connectionTimer->setSingleShot(true);
    connect(m_connectionTimer, &QTimer::timeout, this, &WindowsBluetoothManager::onConnectionTimeout);
}

WindowsBluetoothManager::~WindowsBluetoothManager()
{
    disconnect();
    stopBumbleProcess();
}

bool WindowsBluetoothManager::connectToDevice(const QString &address)
{
    if (m_connected) {
        qWarning() << "Already connected to" << m_deviceAddress;
        return false;
    }

    m_deviceAddress = address;
    
    // Start the Bumble bridge process if not already running
    if (!m_process || m_process->state() != QProcess::Running) {
        startBumbleProcess();
    }

    // Send connect command
    sendCommand("CONNECT", address);
    
    // Start connection timeout
    m_connectionTimer->start(CONNECTION_TIMEOUT_MS);
    
    return true;
}

void WindowsBluetoothManager::disconnect()
{
    if (!m_connected) {
        return;
    }

    sendCommand("DISCONNECT");
    m_connected = false;
    emit disconnected();
}

bool WindowsBluetoothManager::sendData(const QByteArray &data)
{
    if (!m_connected) {
        qWarning() << "Cannot send data: not connected";
        return false;
    }

    // Convert to hex string for transmission
    QString hexData = data.toHex();
    sendCommand("SEND", hexData);
    return true;
}

void WindowsBluetoothManager::startBumbleProcess()
{
    if (m_process) {
        stopBumbleProcess();
    }

    m_process = new QProcess(this);
    
    // Find the bridge script in the application directory
    QString scriptPath = QCoreApplication::applicationDirPath() + 
                        "/windows/bluetooth/" + BRIDGE_SCRIPT_NAME;
    
    // Check if script exists
    if (!QFile::exists(scriptPath)) {
        // Try alternative location (source tree)
        scriptPath = QDir::currentPath() + "/windows/bluetooth/" + BRIDGE_SCRIPT_NAME;
        if (!QFile::exists(scriptPath)) {
            emit error("Bumble bridge script not found: " + scriptPath);
            return;
        }
    }

    connect(m_process, &QProcess::readyRead, this, &WindowsBluetoothManager::onProcessReadyRead);
    connect(m_process, &QProcess::errorOccurred, this, &WindowsBluetoothManager::onProcessErrorOccurred);
    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &WindowsBluetoothManager::onProcessFinished);

    // Start Python with the bridge script
    QStringList arguments;
    arguments << scriptPath;
    
    m_process->start("python", arguments);
    
    if (!m_process->waitForStarted(5000)) {
        emit error("Failed to start Bumble bridge process");
        delete m_process;
        m_process = nullptr;
    } else {
        qDebug() << "Bumble bridge process started successfully";
    }
}

void WindowsBluetoothManager::stopBumbleProcess()
{
    if (!m_process) {
        return;
    }

    if (m_process->state() == QProcess::Running) {
        sendCommand("QUIT");
        m_process->waitForFinished(3000);
        if (m_process->state() == QProcess::Running) {
            m_process->kill();
            m_process->waitForFinished(1000);
        }
    }

    m_process->deleteLater();
    m_process = nullptr;
}

void WindowsBluetoothManager::sendCommand(const QString &command, const QString &data)
{
    if (!m_process || m_process->state() != QProcess::Running) {
        qWarning() << "Cannot send command: process not running";
        return;
    }

    // Format: COMMAND[:DATA]\n
    QString line = command;
    if (!data.isEmpty()) {
        line += ":" + data;
    }
    line += "\n";
    
    m_process->write(line.toUtf8());
    m_process->waitForBytesWritten(1000);
}

void WindowsBluetoothManager::onProcessReadyRead()
{
    if (!m_process) {
        return;
    }

    m_receiveBuffer.append(m_process->readAllStandardOutput());
    
    // Process complete lines
    while (m_receiveBuffer.contains('\n')) {
        int newlinePos = m_receiveBuffer.indexOf('\n');
        QByteArray line = m_receiveBuffer.left(newlinePos);
        m_receiveBuffer.remove(0, newlinePos + 1);
        
        QString lineStr = QString::fromUtf8(line).trimmed();
        if (!lineStr.isEmpty()) {
            processReceivedLine(lineStr);
        }
    }
}

void WindowsBluetoothManager::processReceivedLine(const QString &line)
{
    qDebug() << "Received from bridge:" << line;

    // Parse response format: STATUS:message or DATA:hexdata
    QStringList parts = line.split(':', Qt::SkipEmptyParts);
    if (parts.isEmpty()) {
        return;
    }

    QString messageType = parts[0];
    QString payload = parts.size() > 1 ? parts.mid(1).join(':') : QString();

    if (messageType == "CONNECTED") {
        m_connectionTimer->stop();
        m_connected = true;
        emit connected();
    }
    else if (messageType == "DISCONNECTED") {
        m_connected = false;
        emit disconnected();
    }
    else if (messageType == "DATA") {
        // Convert hex string back to binary
        QByteArray data = QByteArray::fromHex(payload.toUtf8());
        emit dataReceived(data);
    }
    else if (messageType == "ERROR") {
        emit error(payload);
        if (m_connectionTimer->isActive()) {
            m_connectionTimer->stop();
        }
    }
    else if (messageType == "LOG") {
        qDebug() << "Bridge log:" << payload;
    }
}

void WindowsBluetoothManager::onProcessErrorOccurred(QProcess::ProcessError error)
{
    QString errorMsg;
    switch (error) {
        case QProcess::FailedToStart:
            errorMsg = "Failed to start Bumble bridge (check Python installation)";
            break;
        case QProcess::Crashed:
            errorMsg = "Bumble bridge crashed";
            break;
        case QProcess::Timedout:
            errorMsg = "Bumble bridge operation timed out";
            break;
        case QProcess::ReadError:
        case QProcess::WriteError:
            errorMsg = "Bumble bridge communication error";
            break;
        default:
            errorMsg = "Unknown Bumble bridge error";
    }
    
    emit this->error(errorMsg);
}

void WindowsBluetoothManager::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    qDebug() << "Bumble bridge process finished with exit code" << exitCode;
    
    if (m_connected) {
        m_connected = false;
        emit disconnected();
    }

    if (exitStatus == QProcess::CrashExit) {
        emit error("Bumble bridge crashed unexpectedly");
    } else if (exitCode != 0) {
        emit error(QString("Bumble bridge exited with code %1").arg(exitCode));
    }
}

void WindowsBluetoothManager::onConnectionTimeout()
{
    if (!m_connected) {
        emit error("Connection timeout - failed to connect to AirPods");
        stopBumbleProcess();
    }
}
