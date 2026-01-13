# Build Instructions

This document provides detailed instructions for building LibrePods on different platforms.

## Table of Contents
- [Windows](#windows)
- [Linux](#linux)
- [Common Issues](#common-issues)

---

## Windows

### Prerequisites

1. **Install Python 3.10+**
   ```powershell
   winget install Python.Python.3.12
   ```

2. **Install Bumble and dependencies**
   ```powershell
   pip install -r requirements-windows.txt
   ```

3. **Install Qt6**
   
   Option A: Qt Online Installer
   - Download from https://www.qt.io/download-qt-installer
   - Install Qt 6.5+ with MSVC 2022 or MinGW
   - Select: Core, Quick, Bluetooth, Multimedia components

   Option B: vcpkg
   ```powershell
   git clone https://github.com/Microsoft/vcpkg.git
   cd vcpkg
   .\bootstrap-vcpkg.bat
   .\vcpkg install qt6-base:x64-windows qt6-connectivity:x64-windows
   ```

4. **Install OpenSSL**
   
   Option A: Pre-built binaries from https://slproweb.com/products/Win32OpenSSL.html
   
   Option B: vcpkg
   ```powershell
   .\vcpkg install openssl:x64-windows
   ```

5. **Install Build Tools**
   - Visual Studio 2022 Community (with C++ Desktop Development)
   - Or MinGW via chocolatey: `choco install mingw`

### Building with Visual Studio

```powershell
# Clone repository
git clone https://github.com/Winterass/librepods.git
cd librepods

# Create build directory
mkdir build
cd build

# Configure (adjust Qt path)
cmake .. -G "Visual Studio 17 2022" -A x64 ^
  -DCMAKE_PREFIX_PATH="C:\Qt\6.5.0\msvc2022_64"

# Build
cmake --build . --config Release

# Output: build/windows/Release/librepods-windows.exe
```

### Building with MinGW

```powershell
# Clone repository
git clone https://github.com/Winterass/librepods.git
cd librepods

# Create build directory
mkdir build
cd build

# Configure
cmake .. -G "MinGW Makefiles" ^
  -DCMAKE_PREFIX_PATH="C:\Qt\6.5.0\mingw_64"

# Build
cmake --build .

# Output: build/windows/librepods-windows.exe
```

### Running

```powershell
# Make sure Python and Bumble are available
python --version
pip show bumble

# Run the application
cd build/windows/Release  # or build/windows for MinGW
.\librepods-windows.exe --help
.\librepods-windows.exe --debug --address XX:XX:XX:XX:XX:XX
```

---

## Linux

### Prerequisites

**Arch Linux / EndeavourOS:**
```bash
sudo pacman -S qt6-base qt6-connectivity qt6-multimedia-ffmpeg \
               qt6-multimedia openssl libpulse cmake
```

**Debian / Ubuntu:**
```bash
sudo apt-get install qt6-base-dev qt6-declarative-dev \
                     qt6-connectivity-dev qt6-multimedia-dev \
                     libssl-dev libpulse-dev cmake \
                     qml6-module-qtquick-controls \
                     qml6-module-qtqml-workerscript
```

**Fedora:**
```bash
sudo dnf install qt6-qtbase-devel qt6-qtconnectivity-devel \
                 qt6-qtmultimedia-devel qt6-qtdeclarative-devel \
                 openssl-devel pulseaudio-libs-devel cmake
```

### Building

```bash
# Clone repository
git clone https://github.com/Winterass/librepods.git
cd librepods

# Create build directory
mkdir build
cd build

# Configure
cmake ..

# Build
make -j $(nproc)

# Output: build/linux/librepods
```

### Running

```bash
cd build/linux
./librepods --help
./librepods --debug
```

### Installing

```bash
cd build
sudo make install
```

This installs:
- Binary to `/usr/local/bin/librepods`
- Desktop file to `/usr/local/share/applications/`
- Icon to `/usr/local/share/icons/hicolor/512x512/apps/`

---

## Common Issues

### Windows: Python not found
**Error:** "Failed to start Bumble bridge process"

**Solution:**
```powershell
# Verify Python is in PATH
python --version

# If not, add to PATH or use full path in code
where python
```

### Windows: Bumble import error
**Error:** "Bumble not installed"

**Solution:**
```powershell
pip install --upgrade bumble
pip show bumble
```

### Windows: Qt not found
**Error:** "Could not find a package configuration file provided by Qt6"

**Solution:**
```powershell
# Set CMAKE_PREFIX_PATH to Qt installation
cmake .. -DCMAKE_PREFIX_PATH="C:\Qt\6.5.0\msvc2022_64"
```

### Linux: Qt6 modules missing
**Error:** "Could not find a package configuration file provided by Qt6"

**Solution:**
```bash
# Install missing Qt6 QML modules
sudo apt install qml6-module-qtquick-controls qml6-module-qtquick-templates
```

### Linux: PulseAudio not found
**Error:** "Could NOT find PkgConfig (missing: PULSEAUDIO)"

**Solution:**
```bash
# Debian/Ubuntu
sudo apt install libpulse-dev

# Fedora
sudo dnf install pulseaudio-libs-devel

# Arch
sudo pacman -S libpulse
```

### General: OpenSSL not found
**Error:** "Could not find OpenSSL"

**Solution:**

Windows:
```powershell
# Download from https://slproweb.com/products/Win32OpenSSL.html
# Or use vcpkg
.\vcpkg install openssl:x64-windows
```

Linux:
```bash
# Debian/Ubuntu
sudo apt install libssl-dev

# Fedora
sudo dnf install openssl-devel
```

### Windows: WinUSB driver issues
**Error:** Bumble can't access Bluetooth adapter

**Solution:**
1. Download Zadig from https://zadig.akeo.ie/
2. Run as Administrator
3. Options → List All Devices
4. Select Bluetooth adapter
5. Install WinUSB driver
6. Revert from Device Manager after use if needed

---

## Development Tips

### Enable Debug Logging

Windows:
```powershell
.\librepods-windows.exe --debug
```

Linux:
```bash
./librepods --debug
```

### Clean Build

```bash
# Remove build directory
rm -rf build

# Recreate and build
mkdir build && cd build
cmake .. && make -j $(nproc)
```

### IDE Setup

**Visual Studio Code:**
1. Install CMake Tools extension
2. Open workspace folder
3. Select kit (MSVC or MinGW)
4. Configure and build via CMake extension

**CLion:**
1. Open project
2. CLion automatically detects CMakeLists.txt
3. Configure toolchain in Settings
4. Build via Run menu

**Qt Creator:**
1. File → Open File or Project
2. Select CMakeLists.txt
3. Configure kit
4. Build and run

---

## Contributing

When building for development:

1. Fork the repository
2. Create feature branch
3. Make changes
4. Test on your platform
5. Submit PR with:
   - Description of changes
   - Platform tested on
   - Build logs if fixing build issues

---

## Getting Help

- **Issues:** https://github.com/Winterass/librepods/issues
- **Discussions:** https://github.com/Winterass/librepods/discussions
- **Documentation:** See platform-specific README files
