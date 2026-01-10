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
using LibrePods.Core.Protocol;
using LibrePods.Core.Utils;
using Windows.Devices.Bluetooth;
using Windows.Devices.Enumeration;

namespace LibrePods.Desktop.Bluetooth;

/// <summary>
/// Manages Bluetooth connection and communication with AirPods using native L2CAP
/// This class now delegates to NativeBluetoothManager for actual communication
/// </summary>
public class BluetoothManager : IDisposable
{
    private NativeBluetoothManager? _nativeManager;
    private BluetoothDevice? _device;

    public event Action<AirPodsStatus>? OnStatusUpdate;
    public event Action<NoiseControlMode>? OnNoiseControlChanged;
    public event Action<EarDetectionStatus>? OnEarDetectionChanged;
    public event Action<List<BatteryInfo>>? OnBatteryUpdate;
    public event Action<bool>? OnConnectionChanged;
    public event Action<HeadGesture>? OnHeadGesture;

    public bool IsConnected => _nativeManager?.IsConnected ?? false;
    public AirPodsStatus CurrentStatus => _nativeManager?.CurrentStatus ?? new();

    /// <summary>
    /// Scans for paired AirPods devices
    /// </summary>
    public async Task<List<DeviceInformation>> ScanForAirPodsAsync()
    {
        Logger.Info("Scanning for paired AirPods...");

        var selector = BluetoothDevice.GetDeviceSelector();
        var devices = await DeviceInformation.FindAllAsync(selector);

        var airPodsDevices = devices
            .Where(d => d.Name.Contains("AirPods", StringComparison.OrdinalIgnoreCase))
            .ToList();

        Logger.Info($"Found {airPodsDevices.Count} AirPods device(s)");
        return airPodsDevices;
    }

    /// <summary>
    /// Connects to AirPods device by address using native L2CAP
    /// </summary>
    public async Task<bool> ConnectAsync(string deviceAddress)
    {
        try
        {
            Logger.Info($"Connecting to device: {deviceAddress}");

            // Parse the Bluetooth address to get device info
            ulong bluetoothAddress;

            // Try to parse as colon-separated hex or direct numeric
            if (deviceAddress.Contains(':'))
            {
                bluetoothAddress = ulong.Parse(deviceAddress.Replace(":", ""), System.Globalization.NumberStyles.HexNumber);
            }
            else if (deviceAddress.Length > 12)
            {
                // Might be a device ID string, try to extract Bluetooth address from device
                Logger.Error($"Invalid device address format: {deviceAddress}");
                return false;
            }
            else
            {
                bluetoothAddress = ulong.Parse(deviceAddress, System.Globalization.NumberStyles.HexNumber);
            }

            // Get the Bluetooth device for device name
            _device = await BluetoothDevice.FromBluetoothAddressAsync(bluetoothAddress);

            // Create native manager with L2CAP support
            _nativeManager = new NativeBluetoothManager();
            
            // Wire up event handlers
            _nativeManager.OnStatusUpdate += (status) => OnStatusUpdate?.Invoke(status);
            _nativeManager.OnNoiseControlChanged += (mode) => OnNoiseControlChanged?.Invoke(mode);
            _nativeManager.OnEarDetectionChanged += (status) => OnEarDetectionChanged?.Invoke(status);
            _nativeManager.OnBatteryUpdate += (batteries) => OnBatteryUpdate?.Invoke(batteries);
            _nativeManager.OnConnectionChanged += (connected) => OnConnectionChanged?.Invoke(connected);
            _nativeManager.OnHeadGesture += (gesture) => OnHeadGesture?.Invoke(gesture);

            // Set device name if available
            if (_device != null)
            {
                _nativeManager.CurrentStatus.DeviceName = _device.Name;
            }

            // Connect using native L2CAP
            bool connected = await _nativeManager.ConnectAsync(deviceAddress);
            
            if (connected)
            {
                Logger.Info("Connected successfully via native L2CAP");
            }
            else
            {
                Logger.Error("Failed to connect via native L2CAP");
            }

            return connected;
        }
        catch (Exception ex)
        {
            Logger.Error("Failed to connect", ex);
            return false;
        }
    }

    /// <summary>
    /// Disconnects from the AirPods
    /// </summary>
    public void Disconnect()
    {
        _nativeManager?.Disconnect();
        _device?.Dispose();
        _device = null;
    }

    /// <summary>
    /// Sets the noise control mode
    /// </summary>
    public async Task SetNoiseControlModeAsync(NoiseControlMode mode)
    {
        if (_nativeManager != null)
        {
            await _nativeManager.SetNoiseControlModeAsync(mode);
        }
    }

    /// <summary>
    /// Sets conversational awareness
    /// </summary>
    public async Task SetConversationalAwarenessAsync(bool enabled)
    {
        if (_nativeManager != null)
        {
            await _nativeManager.SetConversationalAwarenessAsync(enabled);
        }
    }

    /// <summary>
    /// Renames the AirPods
    /// </summary>
    public async Task RenameAsync(string name)
    {
        if (_nativeManager != null)
        {
            await _nativeManager.RenameAsync(name);
        }
    }

    public void Dispose()
    {
        Disconnect();
        _nativeManager?.Dispose();
    }
}
