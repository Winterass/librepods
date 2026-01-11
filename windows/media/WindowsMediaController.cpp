#include "WindowsMediaController.h"
#include <QDebug>
#include <QTimer>

#ifdef _WIN32
#include <windows.h>
#include <wrl/client.h>
#include <windows.media.control.h>

using Microsoft::WRL::ComPtr;
using namespace ABI::Windows::Media::Control;
using namespace ABI::Windows::Foundation;
#endif

// Platform-specific SMTC handler implementation
class WindowsMediaController::SMTCHandler {
public:
#ifdef _WIN32
    ComPtr<IGlobalSystemMediaTransportControlsSessionManager> sessionManager;
    ComPtr<IGlobalSystemMediaTransportControlsSession> currentSession;
    bool initialized = false;

    bool initialize() {
        // Initialize Windows Runtime
        HRESULT hr = RoInitialize(RO_INIT_MULTITHREADED);
        if (FAILED(hr) && hr != RPC_E_CHANGED_MODE) {
            qWarning() << "Failed to initialize Windows Runtime:" << hr;
            return false;
        }

        // Get SMTC session manager
        // Note: This requires Windows 10 1903 or later
        // For full implementation, we would need to use Windows Runtime C++/WinRT
        
        initialized = true;
        return true;
    }

    void cleanup() {
        currentSession.Reset();
        sessionManager.Reset();
        if (initialized) {
            RoUninitialize();
            initialized = false;
        }
    }
#else
    bool initialized = false;
    bool initialize() { return false; }
    void cleanup() {}
#endif
};

WindowsMediaController::WindowsMediaController(QObject *parent)
    : QObject(parent)
    , m_handler(new SMTCHandler())
    , m_currentState(Unknown)
    , m_initialized(false)
{
}

WindowsMediaController::~WindowsMediaController()
{
    if (m_handler) {
        m_handler->cleanup();
        delete m_handler;
    }
}

bool WindowsMediaController::initialize()
{
    if (m_initialized) {
        return true;
    }

#ifdef _WIN32
    m_initialized = m_handler->initialize();
    
    if (!m_initialized) {
        qWarning() << "Windows Media Controller initialization failed";
        qWarning() << "Note: SMTC requires Windows 10 1903 or later";
        qWarning() << "Falling back to keyboard simulation for media controls";
    } else {
        qDebug() << "Windows Media Controller initialized successfully";
    }
    
    return m_initialized;
#else
    qWarning() << "WindowsMediaController only works on Windows";
    return false;
#endif
}

WindowsMediaController::MediaState WindowsMediaController::getCurrentMediaState() const
{
    return m_currentState;
}

bool WindowsMediaController::play()
{
#ifdef _WIN32
    // Note: Windows VK_MEDIA_PLAY_PAUSE toggles play/pause
    // Since we can't determine actual media state, we use this as best effort
    // Only send if we think we're not already playing
    if (m_currentState != Playing) {
        keybd_event(VK_MEDIA_PLAY_PAUSE, 0, 0, 0);
        keybd_event(VK_MEDIA_PLAY_PAUSE, 0, KEYEVENTF_KEYUP, 0);
        
        m_currentState = Playing;
        emit mediaStateChanged(m_currentState);
    }
    return true;
#else
    return false;
#endif
}

bool WindowsMediaController::pause()
{
#ifdef _WIN32
    // Note: Windows VK_MEDIA_PLAY_PAUSE toggles play/pause
    // Since we can't determine actual media state, we use this as best effort
    // Only send if we think we're playing
    if (m_currentState == Playing) {
        keybd_event(VK_MEDIA_PLAY_PAUSE, 0, 0, 0);
        keybd_event(VK_MEDIA_PLAY_PAUSE, 0, KEYEVENTF_KEYUP, 0);
        
        m_currentState = Paused;
        emit mediaStateChanged(m_currentState);
    }
    return true;
#else
    return false;
#endif
}

bool WindowsMediaController::togglePlayPause()
{
#ifdef _WIN32
    // Simulate media play/pause toggle key press
    keybd_event(VK_MEDIA_PLAY_PAUSE, 0, 0, 0);
    keybd_event(VK_MEDIA_PLAY_PAUSE, 0, KEYEVENTF_KEYUP, 0);
    
    // Toggle state (with proper handling of Unknown/Stopped)
    if (m_currentState == Playing) {
        m_currentState = Paused;
    } else {
        // For Unknown, Stopped, or Paused - assume we're starting playback
        m_currentState = Playing;
    }
    emit mediaStateChanged(m_currentState);
    return true;
#else
    return false;
#endif
}

bool WindowsMediaController::skipNext()
{
#ifdef _WIN32
    // Simulate media next track key press
    // VK_MEDIA_NEXT_TRACK is 0xB0
    keybd_event(VK_MEDIA_NEXT_TRACK, 0, 0, 0);
    keybd_event(VK_MEDIA_NEXT_TRACK, 0, KEYEVENTF_KEYUP, 0);
    return true;
#else
    return false;
#endif
}

bool WindowsMediaController::skipPrevious()
{
#ifdef _WIN32
    // Simulate media previous track key press
    // VK_MEDIA_PREV_TRACK is 0xB1
    keybd_event(VK_MEDIA_PREV_TRACK, 0, 0, 0);
    keybd_event(VK_MEDIA_PREV_TRACK, 0, KEYEVENTF_KEYUP, 0);
    return true;
#else
    return false;
#endif
}
