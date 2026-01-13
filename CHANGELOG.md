## LibrePods root module changelog
_[See here](https://github.com/kavishdevar/librepods/releases)_

## [Unreleased] - Windows 11 Support

### Added
- Full Windows 11 support with native implementation
- Windows Bluetooth Manager using Bumble stack
- Windows Audio Controller using WASAPI
- Windows Media Controller with keyboard simulation fallback
- Windows Autostart Manager using Registry
- Python-C++ bridge for Bumble communication
- Comprehensive Windows build system with CMake
- Windows-specific documentation and build instructions
- Cross-platform root CMakeLists.txt

### Components
- `windows/bluetooth/WindowsBluetoothManager` - L2CAP communication via Bumble
- `windows/media/WindowsAudioController` - WASAPI volume and device control
- `windows/media/WindowsMediaController` - Media playback control
- `windows/AutostartManager` - Windows Registry autostart management
- `windows/bluetooth/bumble_bridge.py` - Python bridge for Bumble stack
- `windows/main.cpp` - Windows application entry point
- `requirements-windows.txt` - Python dependencies for Windows

### Documentation
- `windows/README.md` - Complete Windows setup and usage guide
- `BUILD_INSTRUCTIONS.md` - Cross-platform build documentation
- Updated main `README.md` with Windows support information
