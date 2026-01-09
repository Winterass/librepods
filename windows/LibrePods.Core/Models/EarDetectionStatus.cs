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
/// Ear detection status for AirPods
/// </summary>
public class EarDetectionStatus
{
    public bool LeftInEar { get; set; }
    public bool RightInEar { get; set; }
    public bool BothInEar => LeftInEar && RightInEar;
    public bool NoneInEar => !LeftInEar && !RightInEar;
    public bool AnyInEar => LeftInEar || RightInEar;

    public EarDetectionStatus(bool leftInEar, bool rightInEar)
    {
        LeftInEar = leftInEar;
        RightInEar = rightInEar;
    }
}
