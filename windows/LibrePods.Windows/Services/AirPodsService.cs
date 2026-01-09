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
using LibrePods.Windows.Audio;
using LibrePods.Windows.Bluetooth;
using Windows.Devices.Enumeration;

namespace LibrePods.Windows.Services;

/// <summary>
/// Main service that coordinates all LibrePods functionality
/// </summary>
public class AirPodsService : IDisposable
{
    private readonly BluetoothManager _bluetoothManager;
    private readonly MediaController _mediaController;
    private readonly EarDetectionController _earDetectionController;

    public event Action<bool>? OnConnectionChanged;
    public event Action<AirPodsStatus>? OnStatusUpdate;
    public event Action<List<BatteryInfo>>? OnBatteryUpdate;
    public event Action<EarDetectionStatus>? OnEarDetectionChanged;
    public event Action<NoiseControlMode>? OnNoiseControlChanged;
    public event Action<HeadGesture>? OnHeadGesture;

    public bool IsConnected => _bluetoothManager.IsConnected;
    public AirPodsStatus CurrentStatus => _bluetoothManager.CurrentStatus;

    public AirPodsService()
    {
        _bluetoothManager = new BluetoothManager();
        _mediaController = new MediaController();
        _earDetectionController = new EarDetectionController(_mediaController);

        // Wire up events
        _bluetoothManager.OnConnectionChanged += HandleConnectionChanged;
        _bluetoothManager.OnStatusUpdate += HandleStatusUpdate;
        _bluetoothManager.OnBatteryUpdate += HandleBatteryUpdate;
        _bluetoothManager.OnEarDetectionChanged += HandleEarDetectionChanged;
        _bluetoothManager.OnNoiseControlChanged += HandleNoiseControlChanged;
        _bluetoothManager.OnHeadGesture += HandleHeadGesture;
    }

    public async Task InitializeAsync()
    {
        Logger.Info("Initializing AirPods service...");
        
        try
        {
            await _mediaController.InitializeAsync();
            Logger.Info("AirPods service initialized successfully");
        }
        catch (Exception ex)
        {
            Logger.Error("Failed to initialize AirPods service", ex);
            throw;
        }
    }

    public async Task<List<DeviceInformation>> ScanForAirPodsAsync()
    {
        return await _bluetoothManager.ScanForAirPodsAsync();
    }

    public async Task<bool> ConnectAsync(string deviceAddress)
    {
        return await _bluetoothManager.ConnectAsync(deviceAddress);
    }

    public void Disconnect()
    {
        _bluetoothManager.Disconnect();
    }

    public async Task SetNoiseControlModeAsync(NoiseControlMode mode)
    {
        await _bluetoothManager.SetNoiseControlModeAsync(mode);
    }

    public async Task SetConversationalAwarenessAsync(bool enabled)
    {
        await _bluetoothManager.SetConversationalAwarenessAsync(enabled);
    }

    public async Task RenameAsync(string name)
    {
        await _bluetoothManager.RenameAsync(name);
    }

    private void HandleConnectionChanged(bool isConnected)
    {
        Logger.Info($"Connection changed: {isConnected}");
        OnConnectionChanged?.Invoke(isConnected);
    }

    private void HandleStatusUpdate(AirPodsStatus status)
    {
        OnStatusUpdate?.Invoke(status);
    }

    private void HandleBatteryUpdate(List<BatteryInfo> batteries)
    {
        OnBatteryUpdate?.Invoke(batteries);
    }

    private async void HandleEarDetectionChanged(EarDetectionStatus status)
    {
        OnEarDetectionChanged?.Invoke(status);
        
        // Handle automatic media control
        await _earDetectionController.HandleEarDetectionChangeAsync(status);
    }

    private void HandleNoiseControlChanged(NoiseControlMode mode)
    {
        Logger.Info($"Noise control mode changed: {mode}");
        OnNoiseControlChanged?.Invoke(mode);
    }

    private void HandleHeadGesture(HeadGesture gesture)
    {
        Logger.Info($"Head gesture detected: {gesture}");
        OnHeadGesture?.Invoke(gesture);
        
        // You can implement custom actions here
        // For example, answering calls on nod gesture
    }

    public void Dispose()
    {
        _bluetoothManager?.Dispose();
    }
}
