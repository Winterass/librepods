# LibrePods for Windows 11

![Windows Support](https://img.shields.io/badge/Windows-11-blue)
![Platform](https://img.shields.io/badge/platform-Windows%2011-lightgrey)

A native Windows 11 application to control your AirPods with full feature support.

## Features

- **Noise Control Modes**: Switch between Off, Transparency, Adaptive, and Noise Cancellation
- **Conversational Awareness**: Volume automatically lowers when you speak
- **Battery Monitoring**: Real-time battery status for left, right, and case
- **Auto Play/Pause**: Automatically pause/resume when removing/wearing AirPods
- **Media Controls**: Play, pause, skip tracks from your AirPods
- **System Tray**: Runs in the system tray for easy access
- **Autostart**: Launch LibrePods automatically on Windows startup

## System Requirements

- Windows 11 (build 22000 or later)
- Bluetooth adapter (built-in or USB)
- Python 3.10 or later
- Visual Studio 2022 or MinGW (for building from source)

## Prerequisites Installation

### 1. Install Python and Bumble

```powershell
# Install Python 3.10+ from python.org
# Or use winget:
winget install Python.Python.3.12

# Install Bumble Bluetooth stack
pip install bumble pyusb libusb
```

#### Configure Bumble for Windows

Bumble requires WinUSB driver for your Bluetooth adapter:

1. Download [Zadig](https://zadig.akeo.ie/)
2. Run Zadig as Administrator
3. Options → List All Devices
4. Select your Bluetooth adapter
5. Select WinUSB driver
6. Click "Replace Driver"

**Note**: You can revert to the original driver from Device Manager after use.

See [Bumble Windows Documentation](https://github.com/google/bumble/blob/main/docs/mkdocs/src/platforms/windows.md) for more details.

### 2. Install Qt6

#### Option A: Qt Online Installer (Recommended)

1. Download [Qt Online Installer](https://www.qt.io/download-qt-installer)
2. Install Qt 6.5 or later with the following components:
   - Qt 6.x for Windows (MSVC 2022 or MinGW)
   - Qt Quick
   - Qt Bluetooth
   - Qt Multimedia

#### Option B: vcpkg

```powershell
# Install vcpkg
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat

# Install Qt6
.\vcpkg install qt6-base:x64-windows
.\vcpkg install qt6-connectivity:x64-windows
.\vcpkg install qt6-multimedia:x64-windows
.\vcpkg install qt6-declarative:x64-windows
```

### 3. Install OpenSSL

#### Option A: Pre-built Binaries

Download from [Shining Light Productions](https://slproweb.com/products/Win32OpenSSL.html)

#### Option B: vcpkg

```powershell
.\vcpkg install openssl:x64-windows
```

### 4. Install Build Tools

#### Visual Studio 2022

Download [Visual Studio 2022 Community](https://visualstudio.microsoft.com/vs/community/)

Install with:
- Desktop development with C++
- Windows 11 SDK

#### Or MinGW

```powershell
# Using chocolatey
choco install mingw
```

## Building from Source

### Using Visual Studio

```powershell
# Clone the repository
git clone https://github.com/Winterass/librepods.git
cd librepods/windows

# Create build directory
mkdir build
cd build

# Configure with CMake (adjust Qt6 path)
cmake .. -G "Visual Studio 17 2022" -A x64 ^
  -DCMAKE_PREFIX_PATH="C:\Qt\6.5.0\msvc2022_64"

# Build
cmake --build . --config Release

# The executable will be in build/Release/librepods-windows.exe
```

### Using MinGW

```powershell
# Clone the repository
git clone https://github.com/Winterass/librepods.git
cd librepods/windows

# Create build directory
mkdir build
cd build

# Configure with CMake
cmake .. -G "MinGW Makefiles" ^
  -DCMAKE_PREFIX_PATH="C:\Qt\6.5.0\mingw_64"

# Build
cmake --build .

# The executable will be in build/librepods-windows.exe
```

## Installation

### Binary Release (Coming Soon)

Download the installer from the [Releases](https://github.com/Winterass/librepods/releases) page.

### From Build

After building:

```powershell
# Install to Program Files
cmake --install . --prefix "C:\Program Files\LibrePods"

# Or run directly from build directory
.\Release\librepods-windows.exe
```

## Usage

### First Launch

1. Make sure your AirPods are paired with Windows (via Settings > Bluetooth)
2. Run LibrePods
3. The app will appear in the system tray
4. Left-click the tray icon to open the main window
5. Right-click for quick controls

### Configuration

- **Autostart**: Enable in Settings to start with Windows
- **Notifications**: Control battery and connection notifications
- **Ear Detection**: Configure auto-pause behavior
- **Noise Control**: Set preferred modes and defaults

## Troubleshooting

### AirPods Not Connecting

**Problem**: LibrePods can't connect to AirPods

**Solutions**:
1. Ensure AirPods are paired in Windows Settings
2. Check if Python and Bumble are installed: `pip show bumble`
3. Verify WinUSB driver is installed for Bluetooth adapter (using Zadig)
4. Check Python is in PATH: `python --version`
5. Try running with debug mode: `librepods-windows.exe --debug`

### Bumble Bridge Not Starting

**Problem**: "Failed to start Bumble bridge process" error

**Solutions**:
1. Verify Python installation: `python --version`
2. Check Bumble installation: `pip show bumble`
3. Ensure `bumble_bridge.py` is in the same directory as the executable
4. Run manually: `python bumble_bridge.py` (should wait for commands)

### WinUSB Driver Issues

**Problem**: Bumble can't access Bluetooth adapter

**Solutions**:
1. Reinstall WinUSB driver using Zadig (as Administrator)
2. After use, revert to original driver from Device Manager
3. Try different USB Bluetooth adapter if built-in doesn't work

### Media Controls Not Working

**Problem**: Play/pause gestures on AirPods don't work

**Solutions**:
1. Ensure a media player is running (Spotify, iTunes, etc.)
2. Windows media keys should work system-wide
3. Try manual play/pause from LibrePods tray menu
4. Check if other media control apps are interfering

### Audio Not Switching to AirPods

**Problem**: Audio doesn't automatically switch when wearing AirPods

**Solutions**:
1. This is a Windows limitation - manual switch may be required
2. Set AirPods as default audio device in Windows Sound settings
3. Some media players remember last device - check player settings

### High CPU Usage

**Problem**: LibrePods using too much CPU

**Solutions**:
1. Check if multiple Python processes are running (Task Manager)
2. Restart LibrePods
3. Update to latest version
4. Report issue with logs

### Python Process Not Closing

**Problem**: Python processes remain after closing LibrePods

**Solutions**:
1. Use Task Manager to end `python.exe` processes
2. Ensure proper shutdown via tray icon → Exit
3. If issue persists, report as bug

## Debug Mode

Run with debug logging:

```powershell
librepods-windows.exe --debug
```

This will:
- Show console window with logs
- Enable verbose Bumble logging
- Help diagnose connection issues

## Known Limitations

1. **Device Switching**: Windows doesn't support automatic audio device switching like macOS
2. **SMTC Integration**: Full System Media Transport Controls integration requires Windows 10 1903+
3. **Hearing Aid Features**: Require Windows 11 and Apple DeviceID spoofing
4. **Multi-device**: Seamless handoff between devices limited by Windows Bluetooth stack

## Comparison with Linux Version

| Feature | Windows | Linux |
|---------|---------|-------|
| Noise Control | ✅ | ✅ |
| Battery Status | ✅ | ✅ |
| Ear Detection | ✅ | ✅ |
| Media Controls | ✅ | ✅ |
| Conversational Awareness | ✅ | ✅ |
| Hearing Aid | ⚠️ Limited | ✅ |
| Auto Device Switch | ❌ | ✅ |
| Bluetooth Stack | Bumble | BlueZ |

## Advanced Configuration

### Custom Bluetooth Adapter

If you have multiple Bluetooth adapters:

Edit `bumble_bridge.py` and change:
```python
transport_spec = "usb:0"  # Change to usb:1, usb:2, etc.
```

### Running as Service

To run LibrePods in the background without console:

1. Enable autostart in Settings
2. Add `--hide` flag to startup command
3. Use NSSM to run as Windows Service (advanced)

## Uninstallation

### From Installer
Use Windows "Add or Remove Programs"

### Manual
1. Delete installation directory
2. Remove registry key: `HKCU\Software\Microsoft\Windows\CurrentVersion\Run\LibrePods`
3. Remove settings: `%APPDATA%\AirPodsTrayApp`

## Contributing

Contributions are welcome! Areas needing help:

- [ ] Full SMTC integration (Windows Runtime C++/WinRT)
- [ ] Automatic audio device switching
- [ ] Windows installer creation (NSIS/WiX)
- [ ] Code signing certificate
- [ ] Better Bluetooth adapter detection
- [ ] Native L2CAP support (bypass Bumble)

## License

LibrePods is licensed under GPLv3. See [LICENSE](../LICENSE) for details.

## Support

- **Issues**: [GitHub Issues](https://github.com/Winterass/librepods/issues)
- **Discussions**: [GitHub Discussions](https://github.com/Winterass/librepods/discussions)
- **Email**: See repository for contact

## Acknowledgments

- [Bumble Bluetooth Stack](https://github.com/google/bumble) - Windows Bluetooth support
- Qt Framework - Cross-platform UI
- Original Linux implementation by @kavishdevar and contributors
