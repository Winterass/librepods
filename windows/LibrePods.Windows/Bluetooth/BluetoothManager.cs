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

using System.Net.Sockets;
using LibrePods.Core.Models;
using LibrePods.Core.Protocol;
using LibrePods.Core.Utils;
using Windows.Devices.Bluetooth;
using Windows.Devices.Bluetooth.Rfcomm;
using Windows.Devices.Enumeration;
using Windows.Networking.Sockets;
using Windows.Storage.Streams;

namespace LibrePods.Windows.Bluetooth;

/// <summary>
/// Manages Bluetooth connection and communication with AirPods using Windows Bluetooth APIs
/// </summary>
public class BluetoothManager : IDisposable
{
    private const ushort AAP_PSM = 0x1001; // L2CAP PSM for AAP Protocol
    private StreamSocket? _socket;
    private DataWriter? _writer;
    private DataReader? _reader;
    private BluetoothDevice? _device;
    private CancellationTokenSource? _receiveCts;
    private Task? _receiveTask;
    private bool _isConnected;

    public event Action<AirPodsStatus>? OnStatusUpdate;
    public event Action<NoiseControlMode>? OnNoiseControlChanged;
    public event Action<EarDetectionStatus>? OnEarDetectionChanged;
    public event Action<List<BatteryInfo>>? OnBatteryUpdate;
    public event Action<bool>? OnConnectionChanged;
    public event Action<HeadGesture>? OnHeadGesture;

    public bool IsConnected => _isConnected;
    public AirPodsStatus CurrentStatus { get; private set; } = new();

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
    /// Connects to AirPods device by address
    /// </summary>
    public async Task<bool> ConnectAsync(string deviceAddress)
    {
        try
        {
            Logger.Info($"Connecting to device: {deviceAddress}");

            // Get the Bluetooth device
            _device = await BluetoothDevice.FromBluetoothAddressAsync(
                ulong.Parse(deviceAddress.Replace(":", ""), System.Globalization.NumberStyles.HexNumber));

            if (_device == null)
            {
                Logger.Error("Failed to get Bluetooth device");
                return false;
            }

            CurrentStatus.DeviceName = _device.Name;
            CurrentStatus.DeviceAddress = deviceAddress;

            // Note: Windows UWP Bluetooth API has limited L2CAP support
            // For production use, consider using Win32 Bluetooth APIs or RFCOMM fallback
            // This is a simplified implementation using StreamSocket
            
            _socket = new StreamSocket();
            
            // Try to connect using RFCOMM as fallback
            // In production, implement proper L2CAP connection with AAP_PSM
            var services = await _device.GetRfcommServicesAsync(BluetoothCacheMode.Uncached);
            
            if (services.Services.Count > 0)
            {
                var service = services.Services[0];
                await _socket.ConnectAsync(service.ConnectionHostName, service.ConnectionServiceName);
                
                _writer = new DataWriter(_socket.OutputStream);
                _reader = new DataReader(_socket.InputStream);
                _reader.InputStreamOptions = InputStreamOptions.Partial;

                _isConnected = true;
                OnConnectionChanged?.Invoke(true);
                CurrentStatus.IsConnected = true;

                // Send handshake
                await SendHandshakeAsync();

                // Start receiving data
                StartReceiving();

                Logger.Info("Connected successfully");
                return true;
            }

            Logger.Error("No RFCOMM services found");
            return false;
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
        Logger.Info("Disconnecting...");
        
        _receiveCts?.Cancel();
        _receiveTask?.Wait(TimeSpan.FromSeconds(2));
        
        _writer?.Dispose();
        _reader?.Dispose();
        _socket?.Dispose();
        _device?.Dispose();

        _isConnected = false;
        OnConnectionChanged?.Invoke(false);
        CurrentStatus.IsConnected = false;

        Logger.Info("Disconnected");
    }

    private async Task SendHandshakeAsync()
    {
        Logger.Debug("Sending handshake...");
        
        await SendPacketAsync(AAPProtocol.GetHandshakePacket());
        await Task.Delay(100);
        
        await SendPacketAsync(AAPProtocol.GetEnableFeaturesPacket());
        await Task.Delay(100);
        
        await SendPacketAsync(AAPProtocol.GetRequestNotificationsPacket());
        
        Logger.Debug("Handshake completed");
    }

    private async Task SendPacketAsync(byte[] data)
    {
        if (_writer == null || !_isConnected)
        {
            Logger.Warning("Cannot send packet: not connected");
            return;
        }

        try
        {
            Logger.Debug($"Sending: {data.ToHexString()}");
            _writer.WriteBytes(data);
            await _writer.StoreAsync();
        }
        catch (Exception ex)
        {
            Logger.Error("Failed to send packet", ex);
        }
    }

    private void StartReceiving()
    {
        _receiveCts = new CancellationTokenSource();
        _receiveTask = Task.Run(async () => await ReceiveLoopAsync(_receiveCts.Token));
    }

    private async Task ReceiveLoopAsync(CancellationToken ct)
    {
        Logger.Info("Started receive loop");
        
        while (!ct.IsCancellationRequested && _isConnected)
        {
            try
            {
                if (_reader == null) break;

                // Try to load at least 4 bytes (header)
                var loadResult = await _reader.LoadAsync(4);
                if (loadResult < 4) continue;

                var buffer = new byte[loadResult];
                _reader.ReadBytes(buffer);

                Logger.Debug($"Received: {buffer.ToHexString()}");
                
                ProcessPacket(buffer);
            }
            catch (Exception ex)
            {
                if (!ct.IsCancellationRequested)
                {
                    Logger.Error("Error in receive loop", ex);
                }
                break;
            }
        }
        
        Logger.Info("Receive loop stopped");
    }

    private void ProcessPacket(byte[] data)
    {
        try
        {
            // Battery status
            var batteries = AAPProtocol.ParseBatteryPacket(data);
            if (batteries.Any())
            {
                foreach (var battery in batteries)
                {
                    switch (battery.Component)
                    {
                        case BatteryComponent.Left:
                            CurrentStatus.LeftBattery = battery;
                            break;
                        case BatteryComponent.Right:
                            CurrentStatus.RightBattery = battery;
                            break;
                        case BatteryComponent.Case:
                            CurrentStatus.CaseBattery = battery;
                            break;
                    }
                }
                OnBatteryUpdate?.Invoke(batteries);
                OnStatusUpdate?.Invoke(CurrentStatus);
                return;
            }

            // Noise control mode
            var noiseMode = AAPProtocol.ParseNoiseControlPacket(data);
            if (noiseMode.HasValue)
            {
                CurrentStatus.NoiseControl.Mode = noiseMode.Value;
                OnNoiseControlChanged?.Invoke(noiseMode.Value);
                OnStatusUpdate?.Invoke(CurrentStatus);
                return;
            }

            // Ear detection
            var earStatus = AAPProtocol.ParseEarDetectionPacket(data);
            if (earStatus != null)
            {
                CurrentStatus.EarDetection = earStatus;
                OnEarDetectionChanged?.Invoke(earStatus);
                OnStatusUpdate?.Invoke(CurrentStatus);
                return;
            }

            // Conversational awareness
            var caStatus = AAPProtocol.ParseConversationalAwarenessPacket(data);
            if (caStatus.HasValue)
            {
                CurrentStatus.ConversationalAwarenessEnabled = caStatus.Value.enabled;
                CurrentStatus.ConversationalAwarenessActive = caStatus.Value.active;
                OnStatusUpdate?.Invoke(CurrentStatus);
                return;
            }

            // Head gestures
            var gesture = AAPProtocol.ParseHeadGesturePacket(data);
            if (gesture.HasValue && gesture != HeadGesture.Unknown)
            {
                OnHeadGesture?.Invoke(gesture.Value);
                return;
            }
        }
        catch (Exception ex)
        {
            Logger.Error("Error processing packet", ex);
        }
    }

    /// <summary>
    /// Sets the noise control mode
    /// </summary>
    public async Task SetNoiseControlModeAsync(NoiseControlMode mode)
    {
        Logger.Info($"Setting noise control mode to: {mode}");
        await SendPacketAsync(AAPProtocol.CreateSetNoiseControlPacket(mode));
    }

    /// <summary>
    /// Sets conversational awareness
    /// </summary>
    public async Task SetConversationalAwarenessAsync(bool enabled)
    {
        Logger.Info($"Setting conversational awareness: {enabled}");
        await SendPacketAsync(AAPProtocol.CreateSetConversationalAwarenessPacket(enabled));
    }

    /// <summary>
    /// Renames the AirPods
    /// </summary>
    public async Task RenameAsync(string name)
    {
        Logger.Info($"Renaming device to: {name}");
        await SendPacketAsync(AAPProtocol.CreateRenamePacket(name));
    }

    public void Dispose()
    {
        Disconnect();
    }
}
