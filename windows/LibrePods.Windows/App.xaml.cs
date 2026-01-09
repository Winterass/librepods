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

using System.Windows;
using LibrePods.Core.Utils;
using LibrePods.Windows.Services;

namespace LibrePods.Windows;

public partial class App : Application
{
    private AirPodsService? _airPodsService;

    protected override void OnStartup(StartupEventArgs e)
    {
        base.OnStartup(e);

        Logger.Info("LibrePods for Windows starting...");
        Logger.MinimumLevel = Logger.LogLevel.Debug;

        // Initialize the AirPods service
        _airPodsService = new AirPodsService();
        _airPodsService.InitializeAsync().GetAwaiter().GetResult();
    }

    protected override void OnExit(ExitEventArgs e)
    {
        Logger.Info("LibrePods for Windows shutting down...");
        _airPodsService?.Dispose();
        base.OnExit(e);
    }
}
