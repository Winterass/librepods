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

using LibrePods.Core.Models;
using LibrePods.Core.Utils;

namespace LibrePods.Windows.Audio;

/// <summary>
/// Manages ear detection and automatic audio control
/// </summary>
public class EarDetectionController
{
    private readonly MediaController _mediaController;
    private EarDetectionStatus? _previousStatus;
    private bool _wasPlaying;

    public EarDetectionController(MediaController mediaController)
    {
        _mediaController = mediaController;
    }

    /// <summary>
    /// Handles ear detection status changes
    /// </summary>
    public async Task HandleEarDetectionChangeAsync(EarDetectionStatus status)
    {
        if (_previousStatus == null)
        {
            _previousStatus = status;
            return;
        }

        // Check if status changed
        bool leftChanged = status.LeftInEar != _previousStatus.LeftInEar;
        bool rightChanged = status.RightInEar != _previousStatus.RightInEar;

        if (!leftChanged && !rightChanged)
        {
            return;
        }

        Logger.Info($"Ear detection changed: Left={status.LeftInEar}, Right={status.RightInEar}");

        // Handle insertion (play if was playing)
        if (status.AnyInEar && !_previousStatus.AnyInEar)
        {
            Logger.Info("AirPods inserted, resuming playback");
            if (_wasPlaying)
            {
                await _mediaController.PlayAsync();
            }
        }
        // Handle removal (pause playback)
        else if (!status.AnyInEar && _previousStatus.AnyInEar)
        {
            Logger.Info("AirPods removed, pausing playback");
            _wasPlaying = await _mediaController.IsPlayingAsync();
            if (_wasPlaying)
            {
                await _mediaController.PauseAsync();
            }
        }

        _previousStatus = status;
    }
}
