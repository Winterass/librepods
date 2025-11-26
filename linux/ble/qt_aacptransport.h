#pragma once

#include "aacptransport.h"

#include <QBluetoothSocket>
#include <QMap>

class QtAACPTransport : public AACPTransport
{
    Q_OBJECT
public:
    explicit QtAACPTransport(QObject *parent = nullptr);
    ~QtAACPTransport() override;

    void setupSession(const QBluetoothDeviceInfo &deviceInfo) override;
    void disconnectFromDevice() override;
    bool sendPacket(const QByteArray &packet) override;

    void setPacketCallback(PacketCallback callback) override;
    void setConnectedCallback(ConnectedCallback callback) override;

    bool isConnected() const override;
    QString address() const override;

    void setRetryAttempts(int attempts) override;

    QList<ControlCommandStatus> controlCommandStatuses() const override;
    std::optional<ControlCommandStatus> controlCommandStatus(ControlCommandIdentifier identifier) const override;
    void setControlCommandStatus(ControlCommandIdentifier identifier, const QByteArray &value) override;
    void clearControlCommandStatuses() override;

private slots:
    void handleConnected();
    void handleReadyRead();
    void handleDisconnected();
    void handleError(QBluetoothSocket::SocketError error);

private:
    void cleanupSocket();
    void scheduleReconnect();

    QBluetoothDeviceInfo m_deviceInfo;
    QBluetoothSocket *m_socket = nullptr;
    PacketCallback m_packetCallback;
    ConnectedCallback m_connectedCallback;

    int m_retryAttempts = 3;
    int m_retryCount = 0;

    QMap<ControlCommandIdentifier, QByteArray> m_controlCommandStatus;
};
