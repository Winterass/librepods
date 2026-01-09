# LibrePods for Windows

LibrePods for Windows brings all the exclusive AirPods features to your Windows PC, liberating them from Apple's ecosystem.

## Features

- ✅ **Noise Control Modes**: Switch between Off, Transparency, Adaptive Transparency, and Noise Cancellation
- ✅ **Battery Monitoring**: Real-time battery levels for left, right, and case
- ✅ **Ear Detection**: Automatic music play/pause when you put in or take out your AirPods
- ✅ **Conversational Awareness**: Automatically lowers volume when you speak
- ✅ **Head Gestures**: Nod or shake your head to control actions
- ✅ **Device Customization**: Rename your AirPods
- 🚧 **Hearing Aid Features**: Requires Apple VendorID spoofing (advanced setup)
- 🚧 **Transparency Mode Customization**: Requires Apple VendorID spoofing
- 🚧 **Multi-device Connectivity**: Requires Apple VendorID spoofing

## System Requirements

- **Operating System**: Windows 10 (version 1809 or later) or Windows 11
- **Bluetooth**: Bluetooth 4.0 or later
- **.NET Runtime**: .NET 8.0 (included in installer)
- **RAM**: 256 MB minimum
- **Disk Space**: 100 MB

## Device Compatibility

| Status | Device                | Features                                                   |
| ------ | --------------------- | ---------------------------------------------------------- |
| ✅      | AirPods Pro (2nd Gen) | Fully supported and tested                                 |
| ✅      | AirPods Pro (3rd Gen) | Fully supported (except heartrate monitoring)              |
| ✅      | AirPods Max           | Fully supported                                            |
| ⚠️      | Other AirPods models  | Basic features (battery status, ear detection) should work |

## Installation

### Option 1: MSI Installer (Recommended)

1. Download the latest `LibrePods-Windows-Setup.msi` from the [Releases](https://github.com/Winterass/librepods/releases) page
2. Run the installer and follow the on-screen instructions
3. Launch LibrePods from the Start Menu

### Option 2: Portable Version

1. Download `LibrePods-Windows-Portable.zip` from the [Releases](https://github.com/Winterass/librepods/releases) page
2. Extract the ZIP file to a folder of your choice
3. Run `LibrePods.Windows.exe`

## Setup Guide

### First-Time Setup

1. **Pair your AirPods with Windows**
   - Open Windows Settings → Bluetooth & devices
   - Put your AirPods in pairing mode (press and hold the button on the case)
   - Click "Add device" and select your AirPods
   - Wait for Windows to complete pairing

2. **Launch LibrePods**
   - Open LibrePods from the Start Menu or desktop shortcut
   - The application will minimize to the system tray

3. **Connect to AirPods**
   - Click the LibrePods icon in the system tray
   - Click "Connect" in the main window
   - Your AirPods should connect automatically

### Using LibrePods

#### System Tray
- **Left Click**: Open main window
- **Right Click**: Quick access menu with noise control modes

#### Main Window
The main window has three tabs:

1. **Status**: View battery levels and ear detection status
2. **Noise Control**: Switch between noise control modes and enable conversational awareness
3. **Settings**: Rename your device and view app information

## Advanced Features

### Device ID Spoofing (Required for Some Features)

Some advanced features (Hearing Aid, Multi-device connectivity, Transparency customization) require your Bluetooth adapter to identify as an Apple device.

⚠️ **WARNING**: Modifying Bluetooth device information requires advanced knowledge and can potentially cause issues. Proceed at your own risk.

#### Method 1: Registry Modification (Requires Admin Rights)

1. Open Registry Editor (`regedit.exe`) as Administrator
2. Navigate to: `HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\BTHPORT\Parameters`
3. Create or modify the following DWORD values:
   - `VendorID` = `004C` (Apple's Vendor ID)
   - `ProductID` = `0000`
   - `Version` = `0000`
4. Restart your computer
5. Re-pair your AirPods

#### Method 2: Bluetooth Stack Modification

This method requires technical expertise and is not recommended for most users. Refer to the Linux implementation for guidance on modifying `/etc/bluetooth/main.conf`.

### L2CAP vs RFCOMM

The current implementation uses RFCOMM as a fallback due to limitations in Windows UWP Bluetooth APIs. For full L2CAP support with PSM 0x1001 (AAP protocol), consider:

1. Using Win32 Bluetooth APIs (requires native code)
2. Contributing to improve the Bluetooth implementation
3. Using a third-party Bluetooth library that supports L2CAP

## Building from Source

### Prerequisites

- Visual Studio 2022 or later (Community Edition is fine)
- .NET 8.0 SDK
- Windows 10/11 SDK

### Build Steps

1. Clone the repository:
   ```bash
   git clone https://github.com/Winterass/librepods.git
   cd librepods/windows
   ```

2. Open `LibrePods.sln` in Visual Studio

3. Restore NuGet packages:
   ```
   Tools → NuGet Package Manager → Restore NuGet Packages
   ```

4. Build the solution:
   ```
   Build → Build Solution (F7)
   ```

5. Run the application:
   ```
   Debug → Start Without Debugging (Ctrl+F5)
   ```

### Command Line Build

```bash
cd windows
dotnet restore
dotnet build --configuration Release
```

The output will be in `LibrePods.Windows\bin\Release\net8.0-windows10.0.19041.0\`

## Creating an Installer

The installer can be created using WiX Toolset or Inno Setup. Configuration files will be provided in the `installer/` directory.

### Using WiX Toolset (MSI)

1. Install [WiX Toolset](https://wixtoolset.org/)
2. Open Visual Studio
3. Add a WiX Setup Project to the solution
4. Configure the project to include LibrePods.Windows output
5. Build the installer project

### Using Inno Setup (EXE)

1. Install [Inno Setup](https://jrsoftware.org/isinfo.php)
2. Use the provided `installer/setup.iss` script
3. Compile the script to create the installer

## Troubleshooting

### AirPods Not Found

- **Ensure AirPods are paired**: Check Windows Bluetooth settings
- **Try re-pairing**: Remove and re-add your AirPods in Windows Bluetooth settings
- **Check Bluetooth adapter**: Make sure your Bluetooth adapter is working and supports Bluetooth 4.0+

### Connection Issues

- **Restart LibrePods**: Close and reopen the application
- **Restart Bluetooth service**:
  ```
  Services → Bluetooth Support Service → Restart
  ```
- **Update Bluetooth drivers**: Check for driver updates from your PC manufacturer

### Features Not Working

- **Noise Control not switching**: Ensure AirPods firmware is up to date
- **Ear detection not triggering**: Check that ear detection is enabled in AirPods settings
- **Advanced features unavailable**: These require Device ID spoofing (see Advanced Features section)

### Application Crashes

1. Check the log file: `%APPDATA%\LibrePods\logs\librepods.log`
2. Report issues on GitHub with the log file
3. Try running as Administrator (some features may require elevated privileges)

## Known Limitations

- **L2CAP Support**: Windows UWP Bluetooth APIs have limited L2CAP support. The current implementation uses RFCOMM as a fallback
- **Advanced Features**: Hearing Aid, Multi-device, and Transparency customization require Apple VendorID spoofing
- **Spatial Audio**: Not currently supported
- **Find My**: Not supported (requires iCloud integration)

## Contributing

Contributions are welcome! Please see the main [CONTRIBUTING.md](../CONTRIBUTING.md) for guidelines.

### Areas for Improvement

- Full L2CAP implementation using Win32 APIs
- System tray icon with battery indicator
- Toast notifications for events
- Head tracking visualization
- More robust Bluetooth connection handling
- Installer scripts

## License

LibrePods for Windows is licensed under GPL-3.0-or-later.

```
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
```

All AirPods images, symbols, and the SF Pro font are the property of Apple Inc.

## Acknowledgments

- Based on the AAP protocol documentation by @tyalie
- Inspired by the Android and Linux implementations
- Built with love by the LibrePods community

## Support

- **Issues**: Report bugs on [GitHub Issues](https://github.com/Winterass/librepods/issues)
- **Discussions**: Join conversations on [GitHub Discussions](https://github.com/Winterass/librepods/discussions)
- **Documentation**: Check the [main README](../README.md) for more information
