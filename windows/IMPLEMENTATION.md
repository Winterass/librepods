# Windows Implementation Details

This document provides technical details about the LibrePods Windows implementation.

## Architecture

### Project Structure

```
windows/
├── LibrePods.Core/              # Shared core library
│   ├── Models/                  # Data models (BatteryInfo, AirPodsStatus, etc.)
│   ├── Protocol/                # AAP protocol implementation
│   └── Utils/                   # Utility classes (Logger, ByteUtils)
├── LibrePods.Windows/           # WPF application
│   ├── Bluetooth/               # Bluetooth communication
│   ├── Audio/                   # Audio and media control
│   ├── Services/                # Background services
│   ├── App.xaml                 # Application definition
│   └── MainWindow.xaml          # Main UI
└── installer/                   # Installer configuration
```

### Technology Stack

- **.NET 8.0**: Modern .NET platform with C# 12
- **WPF (Windows Presentation Foundation)**: UI framework
- **Windows.Devices.Bluetooth**: UWP Bluetooth APIs
- **Windows.Media.Control**: Media session management
- **NAudio**: Audio library (for future enhancements)
- **Hardcodet.NotifyIcon.Wpf**: System tray integration

## Key Components

### 1. LibrePods.Core

#### AAPProtocol.cs
- Implements Apple Accessory Protocol (AAP) message parsing and encoding
- Based on `AAP Definitions.md` from the repository
- Handles handshake, feature enablement, and notification requests
- Parses battery, noise control, ear detection, and gesture packets

#### Models
- **BatteryInfo**: Battery status for left/right/case
- **NoiseControlMode**: Enum for noise control modes
- **EarDetectionStatus**: Ear insertion status
- **AirPodsStatus**: Complete device status

### 2. LibrePods.Windows

#### BluetoothManager.cs
- Manages Bluetooth connection using Windows APIs
- Implements AAP protocol communication
- Handles device scanning and pairing
- **Note**: Current implementation uses RFCOMM fallback due to UWP L2CAP limitations

#### MediaController.cs
- Integrates with Windows Media Control Session
- Controls media playback (play/pause/toggle)
- Monitors current media session

#### EarDetectionController.cs
- Handles ear detection events
- Automatically controls media playback
- Pauses when AirPods are removed, resumes when inserted

#### AirPodsService.cs
- Main service coordinating all functionality
- Manages component lifecycle
- Exposes events for UI updates

## Bluetooth Implementation

### Current Approach: RFCOMM Fallback

Windows UWP Bluetooth APIs have limited L2CAP support. The current implementation uses RFCOMM as a fallback:

```csharp
var services = await device.GetRfcommServicesAsync();
await socket.ConnectAsync(service.ConnectionHostName, service.ConnectionServiceName);
```

### Ideal Approach: L2CAP with PSM 0x1001

For full AAP protocol support, L2CAP with PSM 0x1001 is required. This can be achieved using:

1. **Win32 Bluetooth APIs** (via P/Invoke)
2. **Windows.Devices.Bluetooth.Background** (limited scenarios)
3. **Third-party libraries** (e.g., InTheHand.BluetoothLE)

#### Example Win32 Approach (Future Implementation)

```csharp
[DllImport("ws2_32.dll")]
static extern IntPtr socket(int af, int type, int protocol);

[DllImport("ws2_32.dll")]
static extern int connect(IntPtr s, ref SOCKADDR_BTH name, int namelen);

// Use BTHPROTO_L2CAP (2) and set PSM to 0x1001
```

## AAP Protocol Details

### Handshake Sequence

1. Send handshake packet: `00 00 04 00 01 00 02 00 ...`
2. Send feature enablement: `04 00 04 00 4d 00 ff ...`
3. Request notifications: `04 00 04 00 0F 00 FF FF FF FF`

### Message Format

Most AAP messages follow this structure:
```
[04 00] [04 00] [Message Type] [00] [Data...]
```

### Battery Status

Format: `04 00 04 00 04 00 [count] ([component] 01 [level] [status] 01)*`

- Component: Left (0x04), Right (0x02), Case (0x08)
- Level: 0-100
- Status: Unknown (0x00), Charging (0x01), Discharging (0x02), Disconnected (0x04)

### Noise Control

Format: `04 00 04 00 09 00 0D [mode] 00 00 00`

- Mode: Off (0x00), Transparency (0x02), Adaptive (0x03), ANC (0x01)

## UI Design

### Modern Windows Design

The UI follows modern Windows design principles:
- Fluent Design inspired styling
- Card-based layout
- Clean typography
- Responsive controls

### Tabs

1. **Status**: Battery and ear detection display
2. **Noise Control**: Mode selection and conversational awareness
3. **Settings**: Device configuration and about information

## Future Enhancements

### Planned Features

1. **System Tray Icon with Battery Indicator**
   - Show battery levels in icon
   - Quick access menu
   - Toast notifications

2. **Full L2CAP Implementation**
   - Use Win32 APIs for proper L2CAP support
   - Improve connection reliability

3. **Hearing Aid Features UI**
   - Audiogram configuration
   - Amplification controls
   - Requires VendorID spoofing

4. **Head Tracking Visualization**
   - Display head orientation
   - Configure gesture actions

5. **Transparency Mode Customization**
   - Amplification, balance, tone controls
   - Requires VendorID spoofing

### Known Limitations

- **L2CAP**: Current RFCOMM fallback may not support all features
- **VendorID Spoofing**: Advanced features require registry modifications
- **Spatial Audio**: Not yet implemented
- **Multi-device**: Requires VendorID spoofing

## Testing

### Unit Tests (TODO)

Create unit tests for:
- AAP protocol parsing
- Message encoding
- Battery calculation logic

### Integration Tests (TODO)

Test scenarios:
- Bluetooth connection/disconnection
- Protocol handshake
- Feature toggling (noise control, etc.)

### Manual Testing Checklist

- [ ] Connect to AirPods
- [ ] View battery levels
- [ ] Switch noise control modes
- [ ] Test ear detection with media playback
- [ ] Enable/disable conversational awareness
- [ ] Rename device
- [ ] Disconnect and reconnect

## Contributing

See the main repository's contributing guidelines. For Windows-specific contributions:

1. **Code Style**: Follow C# conventions and .NET guidelines
2. **Async/Await**: Use async methods for all I/O operations
3. **Error Handling**: Always catch and log exceptions
4. **UI Threading**: Use `Dispatcher.Invoke` for UI updates
5. **Documentation**: Update this file for architectural changes

## Resources

- [Windows Bluetooth APIs](https://docs.microsoft.com/en-us/uwp/api/windows.devices.bluetooth)
- [WPF Documentation](https://docs.microsoft.com/en-us/dotnet/desktop/wpf/)
- [AAP Protocol](https://github.com/tyalie/AAP-Protocol-Defintion)
- [LibrePods Android Implementation](../android/)
