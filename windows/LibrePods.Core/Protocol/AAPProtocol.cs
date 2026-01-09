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

namespace LibrePods.Core.Protocol;

/// <summary>
/// Parser and encoder for Apple Accessory Protocol (AAP) messages
/// Based on AAP Definitions.md
/// </summary>
public static class AAPProtocol
{
    // Standard AAP message prefixes
    private static readonly byte[] HANDSHAKE_PACKET = new byte[] 
    { 0x00, 0x00, 0x04, 0x00, 0x01, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    
    private static readonly byte[] ENABLE_FEATURES_PACKET = new byte[] 
    { 0x04, 0x00, 0x04, 0x00, 0x4d, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    
    private static readonly byte[] REQUEST_NOTIFICATIONS_PACKET = new byte[] 
    { 0x04, 0x00, 0x04, 0x00, 0x0F, 0x00, 0xFF, 0xFF, 0xFF, 0xFF };

    /// <summary>
    /// Gets the handshake packet required to establish AAP connection
    /// </summary>
    public static byte[] GetHandshakePacket() => HANDSHAKE_PACKET;

    /// <summary>
    /// Gets the packet to enable advanced features (Conversational Awareness, Adaptive Transparency)
    /// </summary>
    public static byte[] GetEnableFeaturesPacket() => ENABLE_FEATURES_PACKET;

    /// <summary>
    /// Gets the packet to request notifications from AirPods
    /// </summary>
    public static byte[] GetRequestNotificationsPacket() => REQUEST_NOTIFICATIONS_PACKET;

    /// <summary>
    /// Parses a battery status packet
    /// Format: 04 00 04 00 04 00 [count] ([component] 01 [level] [status] 01)*count
    /// </summary>
    public static List<BatteryInfo> ParseBatteryPacket(byte[] data)
    {
        var batteries = new List<BatteryInfo>();
        
        if (data.Length < 7) return batteries;
        if (data[0] != 0x04 || data[2] != 0x04 || data[4] != 0x04) return batteries;

        int batteryCount = data[6];
        int offset = 7;

        for (int i = 0; i < batteryCount && offset + 5 <= data.Length; i++)
        {
            var component = (BatteryComponent)data[offset];
            int level = data[offset + 2];
            var status = (BatteryStatus)data[offset + 3];
            
            batteries.Add(new BatteryInfo(component, level, status));
            offset += 5;
        }

        return batteries;
    }

    /// <summary>
    /// Parses a noise control mode packet
    /// Format: 04 00 04 00 09 00 0D [mode] 00 00 00
    /// </summary>
    public static NoiseControlMode? ParseNoiseControlPacket(byte[] data)
    {
        if (data.Length < 8) return null;
        if (data[0] != 0x04 || data[2] != 0x04 || data[4] != 0x09) return null;

        return (NoiseControlMode)data[7];
    }

    /// <summary>
    /// Creates a packet to set noise control mode
    /// Format: 04 00 04 00 09 00 0D [mode] 00 00 00
    /// </summary>
    public static byte[] CreateSetNoiseControlPacket(NoiseControlMode mode)
    {
        return new byte[] { 0x04, 0x00, 0x04, 0x00, 0x09, 0x00, 0x0D, (byte)mode, 0x00, 0x00, 0x00 };
    }

    /// <summary>
    /// Parses an ear detection packet
    /// Format: 04 00 04 00 01 00 [left] [right]
    /// </summary>
    public static EarDetectionStatus? ParseEarDetectionPacket(byte[] data)
    {
        if (data.Length < 8) return null;
        if (data[0] != 0x04 || data[2] != 0x04 || data[4] != 0x01) return null;

        bool leftInEar = data[6] == 0x01;
        bool rightInEar = data[7] == 0x01;

        return new EarDetectionStatus(leftInEar, rightInEar);
    }

    /// <summary>
    /// Parses a conversational awareness packet
    /// Format: 04 00 04 00 11 00 [enabled] [active]
    /// </summary>
    public static (bool enabled, bool active)? ParseConversationalAwarenessPacket(byte[] data)
    {
        if (data.Length < 8) return null;
        if (data[0] != 0x04 || data[2] != 0x04 || data[4] != 0x11) return null;

        bool enabled = data[6] == 0x01;
        bool active = data[7] == 0x01;

        return (enabled, active);
    }

    /// <summary>
    /// Creates a packet to set conversational awareness
    /// </summary>
    public static byte[] CreateSetConversationalAwarenessPacket(bool enabled)
    {
        return new byte[] { 0x04, 0x00, 0x04, 0x00, 0x11, 0x00, (byte)(enabled ? 0x01 : 0x00) };
    }

    /// <summary>
    /// Creates a packet to rename AirPods
    /// </summary>
    public static byte[] CreateRenamePacket(string name)
    {
        var nameBytes = System.Text.Encoding.UTF8.GetBytes(name);
        var packet = new byte[7 + nameBytes.Length];
        
        packet[0] = 0x04;
        packet[1] = 0x00;
        packet[2] = 0x04;
        packet[3] = 0x00;
        packet[4] = 0x05; // Rename command
        packet[5] = 0x00;
        packet[6] = (byte)nameBytes.Length;
        
        Array.Copy(nameBytes, 0, packet, 7, nameBytes.Length);
        
        return packet;
    }

    /// <summary>
    /// Parses head gesture data packet
    /// </summary>
    public static HeadGesture? ParseHeadGesturePacket(byte[] data)
    {
        if (data.Length < 8) return null;
        if (data[0] != 0x04 || data[2] != 0x04) return null;

        // Head gesture packets use message type 0x13
        if (data[4] == 0x13)
        {
            byte gestureType = data[6];
            return gestureType switch
            {
                0x01 => HeadGesture.Nod,
                0x02 => HeadGesture.Shake,
                _ => HeadGesture.Unknown
            };
        }

        return null;
    }
}

public enum HeadGesture
{
    Unknown,
    Nod,
    Shake
}
