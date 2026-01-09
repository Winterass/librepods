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
/// Represents the complete device status
/// </summary>
public class AirPodsStatus
{
    public string DeviceName { get; set; } = "AirPods";
    public string DeviceAddress { get; set; } = string.Empty;
    public bool IsConnected { get; set; }
    
    // Battery
    public BatteryInfo? LeftBattery { get; set; }
    public BatteryInfo? RightBattery { get; set; }
    public BatteryInfo? CaseBattery { get; set; }
    
    // Ear Detection
    public EarDetectionStatus EarDetection { get; set; } = new(false, false);
    
    // Noise Control
    public NoiseControlStatus NoiseControl { get; set; } = new(NoiseControlMode.Off);
    
    // Conversational Awareness
    public bool ConversationalAwarenessEnabled { get; set; }
    public bool ConversationalAwarenessActive { get; set; }
    
    // Head Gestures
    public bool HeadGesturesEnabled { get; set; }
    
    // Device Capabilities
    public AirPodsModel Model { get; set; } = AirPodsModel.Unknown;
    public HashSet<DeviceCapability> Capabilities { get; set; } = new();
}

public enum AirPodsModel
{
    Unknown,
    AirPods1,
    AirPods2,
    AirPods3,
    AirPods4,
    AirPods4ANC,
    AirPodsPro1,
    AirPodsPro2Lightning,
    AirPodsPro2USBC,
    AirPodsPro3,
    AirPodsMax,
    AirPodsMaxUSBC
}

public enum DeviceCapability
{
    ListeningMode,
    ConversationalAwareness,
    StemConfig,
    HeadGestures,
    LoudSoundReduction,
    SleepDetection,
    HearingAid,
    AdaptiveAudio,
    AdaptiveVolume,
    SwipeForVolume,
    HeartRateMonitor
}
