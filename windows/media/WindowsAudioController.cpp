#include "WindowsAudioController.h"
#include <QDebug>

#ifdef _WIN32
#include <comdef.h>
#include <wrl/client.h>

// Link required Windows libraries
#pragma comment(lib, "ole32.lib")

using Microsoft::WRL::ComPtr;
#endif

WindowsAudioController::WindowsAudioController(QObject *parent)
    : QObject(parent)
#ifdef _WIN32
    , m_deviceEnumerator(nullptr)
    , m_endpointVolume(nullptr)
    , m_initialized(false)
    , m_comInitialized(false)
#endif
{
}

WindowsAudioController::~WindowsAudioController()
{
#ifdef _WIN32
    cleanup();
#endif
}

bool WindowsAudioController::initialize()
{
#ifdef _WIN32
    if (m_initialized) {
        return true;
    }

    if (!initializeCOM()) {
        return false;
    }

    // Create device enumerator
    HRESULT hr = CoCreateInstance(
        __uuidof(MMDeviceEnumerator),
        nullptr,
        CLSCTX_ALL,
        __uuidof(IMMDeviceEnumerator),
        (void**)&m_deviceEnumerator
    );

    if (FAILED(hr)) {
        qWarning() << "Failed to create device enumerator:" << hr;
        return false;
    }

    // Get default audio endpoint
    IMMDevice *device = nullptr;
    hr = m_deviceEnumerator->GetDefaultAudioEndpoint(
        eRender,
        eConsole,
        &device
    );

    if (FAILED(hr)) {
        qWarning() << "Failed to get default audio endpoint:" << hr;
        return false;
    }

    // Get endpoint volume interface
    hr = device->Activate(
        __uuidof(IAudioEndpointVolume),
        CLSCTX_ALL,
        nullptr,
        (void**)&m_endpointVolume
    );

    device->Release();

    if (FAILED(hr)) {
        qWarning() << "Failed to activate endpoint volume:" << hr;
        return false;
    }

    m_initialized = true;
    qDebug() << "Windows Audio Controller initialized successfully";
    return true;
#else
    qWarning() << "WindowsAudioController only works on Windows";
    return false;
#endif
}

QString WindowsAudioController::getDefaultSink()
{
#ifdef _WIN32
    if (!m_initialized) {
        return QString();
    }

    IMMDevice *device = nullptr;
    HRESULT hr = m_deviceEnumerator->GetDefaultAudioEndpoint(
        eRender,
        eConsole,
        &device
    );

    if (FAILED(hr)) {
        qWarning() << "Failed to get default audio endpoint";
        return QString();
    }

    QString name = getDeviceName(device);
    device->Release();
    return name;
#else
    return QString();
#endif
}

bool WindowsAudioController::setVolume(float volume)
{
#ifdef _WIN32
    if (!m_initialized || !m_endpointVolume) {
        return false;
    }

    // Clamp volume to valid range
    volume = qBound(0.0f, volume, 1.0f);

    HRESULT hr = m_endpointVolume->SetMasterVolumeLevelScalar(volume, nullptr);
    if (FAILED(hr)) {
        qWarning() << "Failed to set volume:" << hr;
        return false;
    }

    emit volumeChanged(volume);
    return true;
#else
    Q_UNUSED(volume);
    return false;
#endif
}

float WindowsAudioController::getVolume()
{
#ifdef _WIN32
    if (!m_initialized || !m_endpointVolume) {
        return -1.0f;
    }

    float volume = 0.0f;
    HRESULT hr = m_endpointVolume->GetMasterVolumeLevelScalar(&volume);
    if (FAILED(hr)) {
        qWarning() << "Failed to get volume:" << hr;
        return -1.0f;
    }

    return volume;
#else
    return -1.0f;
#endif
}

bool WindowsAudioController::setMute(bool mute)
{
#ifdef _WIN32
    if (!m_initialized || !m_endpointVolume) {
        return false;
    }

    HRESULT hr = m_endpointVolume->SetMute(mute ? TRUE : FALSE, nullptr);
    if (FAILED(hr)) {
        qWarning() << "Failed to set mute:" << hr;
        return false;
    }

    return true;
#else
    Q_UNUSED(mute);
    return false;
#endif
}

bool WindowsAudioController::isMuted()
{
#ifdef _WIN32
    if (!m_initialized || !m_endpointVolume) {
        return false;
    }

    BOOL mute = FALSE;
    HRESULT hr = m_endpointVolume->GetMute(&mute);
    if (FAILED(hr)) {
        qWarning() << "Failed to get mute state:" << hr;
        return false;
    }

    return mute == TRUE;
#else
    return false;
#endif
}

QString WindowsAudioController::findDeviceByName(const QString &nameSubstring)
{
#ifdef _WIN32
    if (!m_initialized) {
        return QString();
    }

    IMMDeviceCollection *collection = nullptr;
    HRESULT hr = m_deviceEnumerator->EnumAudioEndpoints(
        eRender,
        DEVICE_STATE_ACTIVE,
        &collection
    );

    if (FAILED(hr)) {
        qWarning() << "Failed to enumerate audio endpoints";
        return QString();
    }

    UINT count = 0;
    collection->GetCount(&count);

    QString foundDeviceId;

    for (UINT i = 0; i < count; i++) {
        IMMDevice *device = nullptr;
        hr = collection->Item(i, &device);
        if (SUCCEEDED(hr)) {
            QString name = getDeviceName(device);
            
            if (name.contains(nameSubstring, Qt::CaseInsensitive)) {
                // Get device ID
                LPWSTR deviceId = nullptr;
                hr = device->GetId(&deviceId);
                if (SUCCEEDED(hr)) {
                    foundDeviceId = QString::fromWCharArray(deviceId);
                    CoTaskMemFree(deviceId);
                    device->Release();
                    break;
                }
            }
            device->Release();
        }
    }

    collection->Release();
    return foundDeviceId;
#else
    Q_UNUSED(nameSubstring);
    return QString();
#endif
}

bool WindowsAudioController::setDefaultDevice(const QString &deviceId)
{
#ifdef _WIN32
    // Note: Setting default audio device programmatically requires
    // Windows 10 build 2004 or later with IPolicyConfig interface
    // This is a more complex operation that may require elevated privileges
    
    // For now, we log that this feature is not yet implemented
    qWarning() << "setDefaultDevice not yet implemented for Windows";
    qDebug() << "Requested device ID:" << deviceId;
    
    // TODO: Implement using IPolicyConfig or similar interface
    return false;
#else
    Q_UNUSED(deviceId);
    return false;
#endif
}

#ifdef _WIN32
bool WindowsAudioController::initializeCOM()
{
    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    if (FAILED(hr) && hr != RPC_E_CHANGED_MODE) {
        qWarning() << "Failed to initialize COM:" << hr;
        return false;
    }
    
    m_comInitialized = true;
    return true;
}

void WindowsAudioController::cleanup()
{
    if (m_endpointVolume) {
        m_endpointVolume->Release();
        m_endpointVolume = nullptr;
    }

    if (m_deviceEnumerator) {
        m_deviceEnumerator->Release();
        m_deviceEnumerator = nullptr;
    }

    if (m_comInitialized) {
        CoUninitialize();
        m_comInitialized = false;
    }

    m_initialized = false;
}

QString WindowsAudioController::getDeviceName(IMMDevice *device)
{
    if (!device) {
        return QString();
    }

    IPropertyStore *propertyStore = nullptr;
    HRESULT hr = device->OpenPropertyStore(STGM_READ, &propertyStore);
    if (FAILED(hr)) {
        return QString();
    }

    PROPVARIANT varName;
    PropVariantInit(&varName);

    hr = propertyStore->GetValue(PKEY_Device_FriendlyName, &varName);
    QString name;
    if (SUCCEEDED(hr)) {
        name = QString::fromWCharArray(varName.pwszVal);
    }

    PropVariantClear(&varName);
    propertyStore->Release();

    return name;
}
#endif
