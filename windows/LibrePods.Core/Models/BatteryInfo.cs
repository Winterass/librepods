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
/// Represents the battery status of an AirPods component
/// </summary>
public class BatteryInfo
{
    public BatteryComponent Component { get; set; }
    public int Level { get; set; } // 0-100
    public BatteryStatus Status { get; set; }

    public BatteryInfo(BatteryComponent component, int level, BatteryStatus status)
    {
        Component = component;
        Level = level;
        Status = status;
    }
}

public enum BatteryComponent : byte
{
    Left = 0x04,
    Right = 0x02,
    Case = 0x08
}

public enum BatteryStatus : byte
{
    Unknown = 0x00,
    Charging = 0x01,
    Discharging = 0x02,
    Disconnected = 0x04
}
