#include "qt_aacptransport.h"

#include "logger.h"

#include <QBluetoothServiceInfo>
#include <QBluetoothUuid>
#include <QTimer>

QtAACPTransport::QtAACPTransport(QObject *parent)
    : AACPTransport(parent)
{
}

QtAACPTransport::~QtAACPTransport()
{
    cleanupSocket();
}

void QtAACPTransport::setupSession(const QBluetoothDeviceInfo &deviceInfo)
{
    LOG_INFO("Connecting to device: " << deviceInfo.name());
    m_deviceInfo = deviceInfo;
    m_retryCount = 0;

    cleanupSocket();
    m_socket = new QBluetoothSocket(QBluetoothServiceInfo::L2capProtocol, this);

    connect(m_socket, &QBluetoothSocket::connected, this, &QtAACPTransport::handleConnected);
    connect(m_socket, &QBluetoothSocket::readyRead, this, &QtAACPTransport::handleReadyRead);
    connect(m_socket, &QBluetoothSocket::disconnected, this, &QtAACPTransport::handleDisconnected);
    connect(m_socket, QOverload<QBluetoothSocket::SocketError>::of(&QBluetoothSocket::errorOccurred),
            this, &QtAACPTransport::handleError);

    m_socket->connectToService(deviceInfo.address(), QBluetoothUuid("74ec2172-0bad-4d01-8f77-997b2be0722a"));
}

void QtAACPTransport::disconnectFromDevice()
{
    cleanupSocket();
    emit disconnected();
}

bool QtAACPTransport::sendPacket(const QByteArray &packet)
{
    if (m_socket && m_socket->isOpen())
    {
        m_socket->write(packet);
        return true;
    }
    LOG_ERROR("Socket is not open, cannot write packet");
    return false;
}

void QtAACPTransport::setPacketCallback(PacketCallback callback)
{
    m_packetCallback = std::move(callback);
}

void QtAACPTransport::setConnectedCallback(ConnectedCallback callback)
{
    m_connectedCallback = std::move(callback);
}

bool QtAACPTransport::isConnected() const
{
    return m_socket && m_socket->isOpen() && m_socket->state() == QBluetoothSocket::ConnectedState;
}

QString QtAACPTransport::address() const
{
    return m_deviceInfo.address().toString();
}

void QtAACPTransport::setRetryAttempts(int attempts)
{
    m_retryAttempts = attempts;
}

QList<AACPTransport::ControlCommandStatus> QtAACPTransport::controlCommandStatuses() const
{
    QList<ControlCommandStatus> statuses;
    for (auto it = m_controlCommandStatus.begin(); it != m_controlCommandStatus.end(); ++it)
    {
        statuses.append({it.key(), it.value()});
    }
    return statuses;
}

std::optional<AACPTransport::ControlCommandStatus> QtAACPTransport::controlCommandStatus(AACPTransport::ControlCommandIdentifier identifier) const
{
    if (m_controlCommandStatus.contains(identifier))
    {
        return ControlCommandStatus{identifier, m_controlCommandStatus.value(identifier)};
    }
    return std::nullopt;
}

void QtAACPTransport::setControlCommandStatus(AACPTransport::ControlCommandIdentifier identifier, const QByteArray &value)
{
    m_controlCommandStatus.insert(identifier, value);
}

void QtAACPTransport::clearControlCommandStatuses()
{
    m_controlCommandStatus.clear();
}

void QtAACPTransport::handleConnected()
{
    LOG_INFO("Connected to device, setting up session");
    m_retryCount = 0;
    if (m_connectedCallback)
    {
        m_connectedCallback();
    }
}

void QtAACPTransport::handleReadyRead()
{
    if (!m_packetCallback)
    {
        LOG_WARN("Packet callback is not set, discarding data");
        return;
    }

    QByteArray data = m_socket->readAll();
    m_packetCallback(data);
}

void QtAACPTransport::handleDisconnected()
{
    LOG_WARN("Device disconnected");
    emit disconnected();
}

void QtAACPTransport::handleError(QBluetoothSocket::SocketError error)
{
    LOG_ERROR("Socket error: " << error << ", " << m_socket->errorString());
    scheduleReconnect();
}

void QtAACPTransport::cleanupSocket()
{
    if (m_socket)
    {
        m_socket->disconnect(this);
        m_socket->close();
        m_socket->deleteLater();
        m_socket = nullptr;
    }
}

void QtAACPTransport::scheduleReconnect()
{
    if (m_retryCount >= m_retryAttempts)
    {
        LOG_ERROR("Failed to connect after " << m_retryAttempts << " attempts");
        return;
    }

    m_retryCount++;
    LOG_INFO("Retrying connection (attempt " << m_retryCount << ")");
    QTimer::singleShot(1500, this, [this]()
                       { setupSession(m_deviceInfo); });
}
