# Quick Start Guide - LibrePods for Windows

Get started with LibrePods on Windows in 5 minutes!

## Prerequisites

- Windows 10 (1809+) or Windows 11
- AirPods (any model, Pro 2 recommended)
- Bluetooth 4.0 or later
- .NET 8.0 Runtime (installer will prompt if missing)

## Installation

### Quick Install

1. Download `LibrePods-Windows-Setup.msi` from [Releases](https://github.com/Winterass/librepods/releases)
2. Double-click the installer
3. Follow the setup wizard
4. Launch from Start Menu

### First Run

1. **Pair your AirPods with Windows**
   - Settings → Bluetooth & devices → Add device
   - Put AirPods in pairing mode
   - Select them from the list

2. **Launch LibrePods**
   - Find it in Start Menu or desktop shortcut
   - The app opens and shows the main window

3. **Connect**
   - Click the "Connect" button
   - Your AirPods should connect automatically

## Basic Usage

### View Battery Status
- Battery levels show on the "Status" tab
- Left, Right, and Case percentages
- Charging status indicated

### Switch Noise Control
1. Go to "Noise Control" tab
2. Select your desired mode:
   - **Off** - No noise control
   - **Transparency** - Hear surroundings
   - **Adaptive Transparency** - Smart adjustment
   - **Noise Cancellation** - Block noise

### Enable Ear Detection
- Automatic! When you remove AirPods, music pauses
- When you put them back in, music resumes

### Rename Your Device
1. Go to "Settings" tab
2. Type new name
3. Click "Rename Device"
4. Re-pair AirPods for name to take effect

## Features Quick Reference

| Feature | Tab | Description |
|---------|-----|-------------|
| Battery Levels | Status | Real-time battery for Left, Right, Case |
| Ear Detection | Status | Shows which AirPods are in your ears |
| Noise Control | Noise Control | Switch between Off/Transparency/ANC |
| Conversational Awareness | Noise Control | Auto-lower volume when speaking |
| Rename Device | Settings | Change AirPods name |

## Troubleshooting

### AirPods Not Found?
- Make sure they're paired in Windows Settings
- Try removing and re-pairing
- Restart Bluetooth service

### Not Connecting?
- Close and reopen LibrePods
- Check AirPods battery (need charge to connect)
- Ensure no other device is connected to them

### Features Not Working?
- Ensure AirPods firmware is up to date
- Some advanced features need Device ID spoofing (see full README)

## Advanced Features

For hearing aid, transparency customization, and other advanced features, see:
- [Full Windows README](README.md)
- [Implementation Details](IMPLEMENTATION.md)

## Need Help?

- Check [Troubleshooting Guide](README.md#troubleshooting) in full README
- [Report an issue](https://github.com/Winterass/librepods/issues)
- [Join discussions](https://github.com/Winterass/librepods/discussions)

## Next Steps

- Configure Conversational Awareness sensitivity
- Set up system tray auto-start
- Explore Device ID spoofing for advanced features
- Check for updates regularly

---

**Enjoy your liberated AirPods on Windows!** 🎧
