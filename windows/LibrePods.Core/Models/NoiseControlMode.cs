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

namespace LibrePods.Core.Models;

/// <summary>
/// Noise control mode for AirPods
/// </summary>
public enum NoiseControlMode : byte
{
    Off = 0x00,
    Transparency = 0x02,
    AdaptiveTransparency = 0x03,
    NoiseCancellation = 0x01
}

/// <summary>
/// Represents the noise control status
/// </summary>
public class NoiseControlStatus
{
    public NoiseControlMode Mode { get; set; }
    public bool IsAvailable { get; set; }

    public NoiseControlStatus(NoiseControlMode mode, bool isAvailable = true)
    {
        Mode = mode;
        IsAvailable = isAvailable;
    }
}
