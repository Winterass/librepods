#ifndef WINDOWSAUDIOCONTROLLER_H
#define WINDOWSAUDIOCONTROLLER_H

#include <QObject>
#include <QString>

#ifdef _WIN32
#include <windows.h>
#include <mmdeviceapi.h>
#include <endpointvolume.h>
#include <audiopolicy.h>
#include <functiondiscoverykeys_devpkey.h>
#endif

/**
 * @brief Windows Audio Controller using WASAPI (Windows Audio Session API)
 * 
 * Provides volume control and audio device management for Windows.
 * Similar functionality to PulseAudioController on Linux.
 */
class WindowsAudioController : public QObject
{
    Q_OBJECT

public:
    explicit WindowsAudioController(QObject *parent = nullptr);
    ~WindowsAudioController();

    /**
     * @brief Initialize the audio controller
     * @return true if initialization was successful
     */
    bool initialize();

    /**
     * @brief Get the current default audio output device name
     * @return Device name/description
     */
    QString getDefaultSink();

    /**
     * @brief Set volume for the default output device
     * @param volume Volume level (0.0 to 1.0)
     * @return true if successful
     */
    bool setVolume(float volume);

    /**
     * @brief Get volume for the default output device
     * @return Volume level (0.0 to 1.0), or -1.0 on error
     */
    float getVolume();

    /**
     * @brief Mute or unmute the default output device
     * @param mute true to mute, false to unmute
     * @return true if successful
     */
    bool setMute(bool mute);

    /**
     * @brief Get mute state of the default output device
     * @return true if muted, false otherwise
     */
    bool isMuted();

    /**
     * @brief Find audio device by name/MAC address substring
     * @param nameSubstring Substring to search for in device names
     * @return Device ID if found, empty string otherwise
     */
    QString findDeviceByName(const QString &nameSubstring);

    /**
     * @brief Set the default audio output device
     * @param deviceId Device ID to set as default
     * @return true if successful
     */
    bool setDefaultDevice(const QString &deviceId);

signals:
    /**
     * @brief Emitted when the default audio device changes
     * @param deviceName New device name
     */
    void defaultDeviceChanged(const QString &deviceName);

    /**
     * @brief Emitted when volume changes
     * @param volume New volume level (0.0 to 1.0)
     */
    void volumeChanged(float volume);

private:
#ifdef _WIN32
    bool initializeCOM();
    void cleanup();
    QString getDeviceName(IMMDevice *device);

    IMMDeviceEnumerator *m_deviceEnumerator;
    IAudioEndpointVolume *m_endpointVolume;
    bool m_initialized;
    bool m_comInitialized;
#endif
};

#endif // WINDOWSAUDIOCONTROLLER_H
