/*
    LibrePods - AirPods liberated from Apple's ecosystem
    Copyright (C) 2025 LibrePods contributors

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

using LibrePods.Core.Utils;
using Windows.Media.Control;

namespace LibrePods.Windows.Audio;

/// <summary>
/// Manages Windows media playback control for ear detection integration
/// </summary>
public class MediaController
{
    private GlobalSystemMediaTransportControlsSessionManager? _sessionManager;
    private GlobalSystemMediaTransportControlsSession? _currentSession;

    public async Task InitializeAsync()
    {
        try
        {
            Logger.Info("Initializing media controller...");
            _sessionManager = await GlobalSystemMediaTransportControlsSessionManager.RequestAsync();
            
            if (_sessionManager != null)
            {
                _sessionManager.CurrentSessionChanged += OnCurrentSessionChanged;
                UpdateCurrentSession();
                Logger.Info("Media controller initialized");
            }
        }
        catch (Exception ex)
        {
            Logger.Error("Failed to initialize media controller", ex);
        }
    }

    private void OnCurrentSessionChanged(GlobalSystemMediaTransportControlsSessionManager sender, CurrentSessionChangedEventArgs args)
    {
        UpdateCurrentSession();
    }

    private void UpdateCurrentSession()
    {
        _currentSession = _sessionManager?.GetCurrentSession();
    }

    /// <summary>
    /// Plays media if paused
    /// </summary>
    public async Task PlayAsync()
    {
        try
        {
            if (_currentSession != null)
            {
                Logger.Debug("Sending play command");
                await _currentSession.TryPlayAsync();
            }
        }
        catch (Exception ex)
        {
            Logger.Error("Failed to play media", ex);
        }
    }

    /// <summary>
    /// Pauses media if playing
    /// </summary>
    public async Task PauseAsync()
    {
        try
        {
            if (_currentSession != null)
            {
                Logger.Debug("Sending pause command");
                await _currentSession.TryPauseAsync();
            }
        }
        catch (Exception ex)
        {
            Logger.Error("Failed to pause media", ex);
        }
    }

    /// <summary>
    /// Toggles play/pause
    /// </summary>
    public async Task TogglePlayPauseAsync()
    {
        try
        {
            if (_currentSession != null)
            {
                Logger.Debug("Toggling play/pause");
                await _currentSession.TryTogglePlayPauseAsync();
            }
        }
        catch (Exception ex)
        {
            Logger.Error("Failed to toggle play/pause", ex);
        }
    }

    /// <summary>
    /// Gets current playback status
    /// </summary>
    public async Task<bool> IsPlayingAsync()
    {
        try
        {
            if (_currentSession != null)
            {
                var playbackInfo = await _currentSession.TryGetMediaPropertiesAsync();
                return _currentSession.GetPlaybackInfo().PlaybackStatus == 
                    GlobalSystemMediaTransportControlsSessionPlaybackStatus.Playing;
            }
        }
        catch (Exception ex)
        {
            Logger.Error("Failed to get playback status", ex);
        }
        
        return false;
    }
}
