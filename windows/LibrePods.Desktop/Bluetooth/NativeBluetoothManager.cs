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

using System.Runtime.InteropServices;
using System.Net.Sockets;
using LibrePods.Core.Models;
using LibrePods.Core.Protocol;
using LibrePods.Core.Utils;

namespace LibrePods.Desktop.Bluetooth;

/// <summary>
/// Native Bluetooth manager using Win32 APIs for L2CAP support
/// Implements direct L2CAP connection to AirPods via PSM 0x1001
/// </summary>
public class NativeBluetoothManager : IDisposable
{
    #region P/Invoke Declarations

    // Win32 socket functions from ws2_32.dll
    [DllImport("ws2_32.dll", SetLastError = true)]
    private static extern IntPtr socket(int af, int type, int protocol);

    [DllImport("ws2_32.dll", SetLastError = true)]
    private static extern int connect(IntPtr socket, ref SOCKADDR_BTH addr, int addrlen);

    [DllImport("ws2_32.dll", SetLastError = true)]
    private static extern int send(IntPtr socket, byte[] buf, int len, int flags);

    [DllImport("ws2_32.dll", SetLastError = true)]
    private static extern int recv(IntPtr socket, byte[] buf, int len, int flags);

    [DllImport("ws2_32.dll", SetLastError = true)]
    private static extern int closesocket(IntPtr socket);

    [DllImport("ws2_32.dll", SetLastError = true)]
    private static extern int WSAGetLastError();

    [DllImport("ws2_32.dll", SetLastError = true)]
    private static extern int WSAStartup(ushort wVersionRequested, ref WSADATA lpWSAData);

    [DllImport("ws2_32.dll", SetLastError = true)]
    private static extern int WSACleanup();

    // WSADATA structure for WSAStartup
    [StructLayout(LayoutKind.Sequential)]
    private struct WSADATA
    {
        public ushort wVersion;
        public ushort wHighVersion;
        public ushort iMaxSockets;
        public ushort iMaxUdpDg;
        public IntPtr lpVendorInfo;
        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 257)]
        public byte[] szDescription;
        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 129)]
        public byte[] szSystemStatus;
    }

    // Bluetooth address structure for L2CAP
    [StructLayout(LayoutKind.Sequential)]
    private struct SOCKADDR_BTH
    {
        public short addressFamily;     // AF_BTH = 32
        public ulong btAddr;            // Bluetooth device address
        public Guid serviceClassId;     // Not used for L2CAP PSM connections
        public uint port;               // PSM (Protocol Service Multiplexer)
    }

    // Constants
    private const int AF_BTH = 32;                  // Bluetooth address family
    private const int SOCK_SEQPACKET = 5;           // Sequenced packet socket
    private const int BTHPROTO_L2CAP = 0x0100;      // L2CAP protocol
    private const uint AAP_PSM = 0x1001;            // AirPods Accessory Protocol PSM
    private const int INVALID_SOCKET = -1;
    private const int SOCKET_ERROR = -1;
    private static readonly IntPtr INVALID_SOCKET_HANDLE = (IntPtr)(-1);

    #endregion

    #region Static Initialization

    /// <summary>
    /// Static constructor to initialize Winsock
    /// </summary>
    static NativeBluetoothManager()
    {
        // Initialize Winsock 2.2
        WSADATA wsaData = new WSADATA
        {
            szDescription = new byte[257],
            szSystemStatus = new byte[129]
        };
        
        ushort version = 0x0202; // Version 2.2 (high byte = major, low byte = minor)
        int result = WSAStartup(version, ref wsaData);
        
        if (result != 0)
        {
            Logger.Error($"WSAStartup failed with error: {result}");
            throw new Exception($"Failed to initialize Winsock. Error code: {result}");
        }
        
        Logger.Debug($"Winsock initialized successfully. Version: {wsaData.wVersion:X4}");
    }

    #endregion

    #region Fields and Properties

    private IntPtr _socketHandle = IntPtr.Zero;
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

    #endregion

    #region Connection Management

    /// <summary>
    /// Connects to AirPods device using native L2CAP socket
    /// </summary>
    /// <param name="deviceAddress">Bluetooth address in format XX:XX:XX:XX:XX:XX or numeric</param>
    public async Task<bool> ConnectAsync(string deviceAddress)
    {
        try
        {
            Logger.Info($"Connecting to device via L2CAP: {deviceAddress}");

            // Parse Bluetooth address
            ulong bluetoothAddress = ParseBluetoothAddress(deviceAddress);
            Logger.Debug($"Parsed Bluetooth address: {bluetoothAddress:X12}");

            // Create L2CAP socket
            _socketHandle = socket(AF_BTH, SOCK_SEQPACKET, BTHPROTO_L2CAP);

            if (_socketHandle == IntPtr.Zero || _socketHandle == INVALID_SOCKET_HANDLE)
            {
                int error = WSAGetLastError();
                Logger.Error($"Failed to create L2CAP socket. WSA Error: {error}");
                throw new Exception($"Failed to create L2CAP socket. Error code: {error}");
            }

            Logger.Debug("L2CAP socket created successfully");

            // Setup L2CAP connection to PSM 0x1001
            SOCKADDR_BTH addr = new SOCKADDR_BTH
            {
                addressFamily = AF_BTH,
                btAddr = bluetoothAddress,
                serviceClassId = Guid.Empty,
                port = AAP_PSM
            };

            // Connect to AirPods
            Logger.Debug($"Connecting to PSM {AAP_PSM:X4}...");
            int result = await Task.Run(() => connect(_socketHandle, ref addr, Marshal.SizeOf(addr)));

            if (result == SOCKET_ERROR)
            {
                int error = WSAGetLastError();
                closesocket(_socketHandle);
                _socketHandle = IntPtr.Zero;
                Logger.Error($"Failed to connect via L2CAP. WSA Error: {error}");
                
                // Provide helpful error messages
                string errorMessage = error switch
                {
                    10061 => "Connection refused. Ensure AirPods are powered on and in range.",
                    10060 => "Connection timeout. AirPods may be out of range or already connected to another device.",
                    10065 => "No route to host. Check Bluetooth adapter and drivers.",
                    _ => $"Connection failed with error code: {error}"
                };
                
                throw new Exception(errorMessage);
            }

            _isConnected = true;
            CurrentStatus.DeviceAddress = deviceAddress;
            CurrentStatus.IsConnected = true;

            Logger.Info("L2CAP connection established successfully");
            OnConnectionChanged?.Invoke(true);

            // Send AAP handshake
            await SendHandshakeAsync();

            // Start receiving loop
            StartReceiving();

            return true;
        }
        catch (Exception ex)
        {
            Logger.Error("L2CAP connection failed", ex);
            Disconnect();
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
        
        if (_receiveTask != null)
        {
            try
            {
                _receiveTask.Wait(TimeSpan.FromSeconds(2));
            }
            catch (Exception ex)
            {
                Logger.Warning($"Error waiting for receive task: {ex.Message}");
            }
        }

        if (_socketHandle != IntPtr.Zero && _socketHandle != INVALID_SOCKET_HANDLE)
        {
            closesocket(_socketHandle);
            _socketHandle = IntPtr.Zero;
        }

        _isConnected = false;
        CurrentStatus.IsConnected = false;
        OnConnectionChanged?.Invoke(false);

        Logger.Info("Disconnected");
    }

    #endregion

    #region Address Parsing

    /// <summary>
    /// Parses Bluetooth address from various formats
    /// Supports: XX:XX:XX:XX:XX:XX, XXXXXXXXXXXX, or numeric ulong
    /// </summary>
    private ulong ParseBluetoothAddress(string address)
    {
        if (string.IsNullOrWhiteSpace(address))
        {
            throw new ArgumentException("Bluetooth address cannot be empty");
        }

        try
        {
            // Remove common separators
            string cleanAddress = address.Replace(":", "")
                                        .Replace("-", "")
                                        .Replace(" ", "")
                                        .Trim();

            // Try to parse as hex
            if (cleanAddress.Length == 12)
            {
                return ulong.Parse(cleanAddress, System.Globalization.NumberStyles.HexNumber);
            }

            // Try to parse as numeric
            if (ulong.TryParse(cleanAddress, out ulong numericAddress))
            {
                return numericAddress;
            }

            throw new FormatException($"Invalid Bluetooth address format: {address}");
        }
        catch (Exception ex)
        {
            Logger.Error($"Failed to parse Bluetooth address '{address}'", ex);
            throw new ArgumentException($"Invalid Bluetooth address: {address}", ex);
        }
    }

    #endregion

    #region AAP Protocol

    /// <summary>
    /// Sends AAP handshake sequence
    /// </summary>
    private async Task SendHandshakeAsync()
    {
        Logger.Debug("Sending AAP handshake...");

        // Handshake packet
        await SendPacketAsync(AAPProtocol.GetHandshakePacket());
        await Task.Delay(100);

        // Enable features (Conversational Awareness, Adaptive Transparency)
        await SendPacketAsync(AAPProtocol.GetEnableFeaturesPacket());
        await Task.Delay(100);

        // Request notifications
        await SendPacketAsync(AAPProtocol.GetRequestNotificationsPacket());

        Logger.Debug("AAP handshake completed");
    }

    /// <summary>
    /// Sends a packet over the L2CAP socket
    /// </summary>
    private async Task SendPacketAsync(byte[] data)
    {
        if (!_isConnected || _socketHandle == IntPtr.Zero || _socketHandle == INVALID_SOCKET_HANDLE)
        {
            Logger.Warning("Cannot send packet: not connected");
            return;
        }

        try
        {
            await Task.Run(() =>
            {
                int sent = send(_socketHandle, data, data.Length, 0);
                
                if (sent == SOCKET_ERROR)
                {
                    int error = WSAGetLastError();
                    Logger.Error($"Failed to send packet. WSA Error: {error}");
                }
                else if (sent > 0)
                {
                    Logger.Debug($"Sent {sent} bytes: {data.ToHexString()}");
                }
            });
        }
        catch (Exception ex)
        {
            Logger.Error("Error sending packet", ex);
        }
    }

    #endregion

    #region Receiving

    /// <summary>
    /// Starts the receive loop in a background task
    /// </summary>
    private void StartReceiving()
    {
        _receiveCts = new CancellationTokenSource();
        _receiveTask = Task.Run(async () => await ReceiveLoopAsync(_receiveCts.Token));
    }

    /// <summary>
    /// Main receive loop for incoming AAP packets
    /// </summary>
    private async Task ReceiveLoopAsync(CancellationToken ct)
    {
        Logger.Info("Started L2CAP receive loop");
        byte[] buffer = new byte[1024];

        while (!ct.IsCancellationRequested && _isConnected)
        {
            try
            {
                int received = await Task.Run(() => recv(_socketHandle, buffer, buffer.Length, 0), ct);

                if (received > 0)
                {
                    byte[] packet = new byte[received];
                    Array.Copy(buffer, packet, received);
                    
                    Logger.Debug($"Received {received} bytes: {packet.ToHexString()}");
                    ProcessPacket(packet);
                }
                else if (received == 0)
                {
                    Logger.Info("Connection closed by remote device");
                    _isConnected = false;
                    break;
                }
                else
                {
                    int error = WSAGetLastError();
                    // Only log if not cancelled
                    if (!ct.IsCancellationRequested)
                    {
                        Logger.Error($"Receive error: {error}");
                    }
                    break;
                }
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

        Logger.Info("L2CAP receive loop stopped");
    }

    /// <summary>
    /// Processes received AAP packets
    /// </summary>
    private void ProcessPacket(byte[] data)
    {
        try
        {
            // Battery status
            var batteries = AAPProtocol.ParseBatteryPacket(data);
            if (batteries.Any())
            {
                Logger.Info($"Battery update: {batteries.Count} component(s)");
                foreach (var battery in batteries)
                {
                    Logger.Debug($"  {battery.Component}: {battery.Level}% ({battery.Status})");
                    
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
                Logger.Info($"Noise control mode: {noiseMode.Value}");
                CurrentStatus.NoiseControl.Mode = noiseMode.Value;
                OnNoiseControlChanged?.Invoke(noiseMode.Value);
                OnStatusUpdate?.Invoke(CurrentStatus);
                return;
            }

            // Ear detection
            var earStatus = AAPProtocol.ParseEarDetectionPacket(data);
            if (earStatus != null)
            {
                Logger.Info($"Ear detection: Left={earStatus.LeftInEar}, Right={earStatus.RightInEar}");
                CurrentStatus.EarDetection = earStatus;
                OnEarDetectionChanged?.Invoke(earStatus);
                OnStatusUpdate?.Invoke(CurrentStatus);
                return;
            }

            // Conversational awareness
            var caStatus = AAPProtocol.ParseConversationalAwarenessPacket(data);
            if (caStatus.HasValue)
            {
                Logger.Info($"Conversational awareness: enabled={caStatus.Value.enabled}, active={caStatus.Value.active}");
                CurrentStatus.ConversationalAwarenessEnabled = caStatus.Value.enabled;
                CurrentStatus.ConversationalAwarenessActive = caStatus.Value.active;
                OnStatusUpdate?.Invoke(CurrentStatus);
                return;
            }

            // Head gestures
            var gesture = AAPProtocol.ParseHeadGesturePacket(data);
            if (gesture.HasValue && gesture != HeadGesture.Unknown)
            {
                Logger.Info($"Head gesture detected: {gesture.Value}");
                OnHeadGesture?.Invoke(gesture.Value);
                return;
            }

            // If we got here, packet was not recognized
            Logger.Debug("Received unknown or unhandled packet type");
        }
        catch (Exception ex)
        {
            Logger.Error("Error processing packet", ex);
        }
    }

    #endregion

    #region Public Control Methods

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

    #endregion

    #region IDisposable

    public void Dispose()
    {
        Disconnect();
        _receiveCts?.Dispose();
    }

    #endregion
}
