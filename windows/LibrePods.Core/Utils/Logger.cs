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
/// Logger for LibrePods
/// </summary>
public static class Logger
{
    public enum LogLevel
    {
        Debug,
        Info,
        Warning,
        Error
    }

    public static LogLevel MinimumLevel { get; set; } = LogLevel.Info;
    public static event Action<string, LogLevel>? OnLog;

    public static void Debug(string message) => Log(message, LogLevel.Debug);
    public static void Info(string message) => Log(message, LogLevel.Info);
    public static void Warning(string message) => Log(message, LogLevel.Warning);
    public static void Error(string message) => Log(message, LogLevel.Error);
    public static void Error(string message, Exception ex) => Log($"{message}: {ex.Message}\n{ex.StackTrace}", LogLevel.Error);

    private static void Log(string message, LogLevel level)
    {
        if (level < MinimumLevel) return;

        var timestamp = DateTime.Now.ToString("yyyy-MM-dd HH:mm:ss.fff");
        var logMessage = $"[{timestamp}] [{level}] {message}";
        
        Console.WriteLine(logMessage);
        OnLog?.Invoke(logMessage, level);
    }
}
