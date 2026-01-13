#ifndef WINDOWSMEDIACONTROLLER_H
#define WINDOWSMEDIACONTROLLER_H

#include <QObject>
#include <QString>

/**
 * @brief Windows Media Controller using System Media Transport Controls (SMTC)
 * 
 * Provides media playback control (play/pause/skip) using Windows 10/11 SMTC.
 * Similar functionality to MediaController on Linux using MPRIS.
 */
class WindowsMediaController : public QObject
{
    Q_OBJECT

public:
    enum MediaState {
        Unknown,
        Playing,
        Paused,
        Stopped
    };
    Q_ENUM(MediaState)

    explicit WindowsMediaController(QObject *parent = nullptr);
    ~WindowsMediaController();

    /**
     * @brief Initialize the media controller
     * @return true if initialization was successful
     */
    bool initialize();

    /**
     * @brief Get current media playback state
     * @return Current MediaState
     */
    MediaState getCurrentMediaState() const;

    /**
     * @brief Play media
     * @return true if successful
     */
    bool play();

    /**
     * @brief Pause media
     * @return true if successful
     */
    bool pause();

    /**
     * @brief Toggle play/pause
     * @return true if successful
     */
    bool togglePlayPause();

    /**
     * @brief Skip to next track
     * @return true if successful
     */
    bool skipNext();

    /**
     * @brief Skip to previous track
     * @return true if successful
     */
    bool skipPrevious();

signals:
    /**
     * @brief Emitted when media playback state changes
     * @param state New media state
     */
    void mediaStateChanged(MediaState state);

    /**
     * @brief Emitted when media metadata changes
     * @param title Track title
     * @param artist Artist name
     */
    void metadataChanged(const QString &title, const QString &artist);

private:
    class SMTCHandler; // Forward declaration for platform-specific implementation
    SMTCHandler *m_handler;
    MediaState m_currentState;
    bool m_initialized;
};

#endif // WINDOWSMEDIACONTROLLER_H
