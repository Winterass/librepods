#pragma once

#include <QObject>
#include <QBluetoothDeviceInfo>
#include <QByteArray>
#include <QList>
#include <functional>
#include <optional>

class AACPTransport : public QObject
{
    Q_OBJECT
public:
    enum class ControlCommandIdentifier : quint8
    {
        MIC_MODE = 0x01,
        BUTTON_SEND_MODE = 0x05,
        VOICE_TRIGGER = 0x12,
        SINGLE_CLICK_MODE = 0x14,
        DOUBLE_CLICK_MODE = 0x15,
        CLICK_HOLD_MODE = 0x16,
        DOUBLE_CLICK_INTERVAL = 0x17,
        CLICK_HOLD_INTERVAL = 0x18,
        LISTENING_MODE_CONFIGS = 0x1A,
        ONE_BUD_ANC_MODE = 0x1B,
        CROWN_ROTATION_DIRECTION = 0x1C,
        LISTENING_MODE = 0x0D,
        AUTO_ANSWER_MODE = 0x1E,
        CHIME_VOLUME = 0x1F,
        VOLUME_SWIPE_INTERVAL = 0x23,
        CALL_MANAGEMENT_CONFIG = 0x24,
        VOLUME_SWIPE_MODE = 0x25,
        ADAPTIVE_VOLUME_CONFIG = 0x26,
        SOFTWARE_MUTE_CONFIG = 0x27,
        CONVERSATION_DETECT_CONFIG = 0x28,
        SSL = 0x29,
        HEARING_AID = 0x2C,
        AUTO_ANC_STRENGTH = 0x2E,
        HPS_GAIN_SWIPE = 0x2F,
        HRM_STATE = 0x30,
        IN_CASE_TONE_CONFIG = 0x31,
        SIRI_MULTITONE_CONFIG = 0x32,
        HEARING_ASSIST_CONFIG = 0x33,
        ALLOW_OFF_OPTION = 0x34,
        STEM_CONFIG = 0x39,
        SLEEP_DETECTION_CONFIG = 0x35,
        ALLOW_AUTO_CONNECT = 0x36,
        EAR_DETECTION_CONFIG = 0x0A,
        AUTOMATIC_CONNECTION_CONFIG = 0x20,
        OWNS_CONNECTION = 0x06,
        PPE_TOGGLE_CONFIG = 0x37,
        PPE_CAP_LEVEL_CONFIG = 0x38
    };

    struct ControlCommandStatus
    {
        ControlCommandIdentifier identifier;
        QByteArray value;
    };

    using PacketCallback = std::function<void(const QByteArray &)>;
    using ConnectedCallback = std::function<void()>;

    explicit AACPTransport(QObject *parent = nullptr) : QObject(parent) {}
    ~AACPTransport() override = default;

    virtual void setupSession(const QBluetoothDeviceInfo &deviceInfo) = 0;
    virtual void disconnectFromDevice() = 0;
    virtual bool sendPacket(const QByteArray &packet) = 0;

    virtual void setPacketCallback(PacketCallback callback) = 0;
    virtual void setConnectedCallback(ConnectedCallback callback) = 0;

    virtual bool isConnected() const = 0;
    virtual QString address() const = 0;

    virtual void setRetryAttempts(int attempts) = 0;

    virtual QList<ControlCommandStatus> controlCommandStatuses() const = 0;
    virtual std::optional<ControlCommandStatus> controlCommandStatus(ControlCommandIdentifier identifier) const = 0;
    virtual void setControlCommandStatus(ControlCommandIdentifier identifier, const QByteArray &value) = 0;
    virtual void clearControlCommandStatuses() = 0;

signals:
    void disconnected();
};
