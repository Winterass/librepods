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

namespace LibrePods.Core.Utils;

/// <summary>
/// Helper methods for byte array operations
/// </summary>
public static class ByteUtils
{
    public static string ToHexString(this byte[] bytes)
    {
        return BitConverter.ToString(bytes).Replace("-", " ");
    }

    public static string ToHexString(this byte[] bytes, int offset, int length)
    {
        return BitConverter.ToString(bytes, offset, length).Replace("-", " ");
    }

    public static byte[] FromHexString(string hex)
    {
        hex = hex.Replace(" ", "").Replace("-", "");
        var bytes = new byte[hex.Length / 2];
        
        for (int i = 0; i < bytes.Length; i++)
        {
            bytes[i] = Convert.ToByte(hex.Substring(i * 2, 2), 16);
        }
        
        return bytes;
    }
}
