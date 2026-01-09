# Testing Checklist for LibrePods Windows

This checklist should be used by testers to verify the Windows implementation works correctly.

## Prerequisites Verification

- [ ] Windows 10 (version 1809+) or Windows 11
- [ ] .NET 8.0 SDK installed (`dotnet --version`)
- [ ] Visual Studio 2022 or later (optional, for VS build)
- [ ] AirPods (any model) available for testing
- [ ] AirPods are charged and in pairing mode
- [ ] Bluetooth adapter working (Bluetooth 4.0+)

## Build Verification

### Using Build Script
- [ ] Navigate to `windows/` directory
- [ ] Run `build.bat` (Windows) or `build.sh` (cross-platform)
- [ ] Verify output in `publish/win-x64/`
- [ ] No build errors reported

### Using Visual Studio
- [ ] Open `LibrePods.sln` in Visual Studio
- [ ] Restore NuGet packages (right-click solution → Restore)
- [ ] Build solution (F7)
- [ ] Verify both projects build successfully
- [ ] Run application (F5 or Ctrl+F5)

## Application Launch

- [ ] Application starts without errors
- [ ] Main window appears with three tabs
- [ ] UI elements render correctly
- [ ] No console errors or exceptions

## Bluetooth Connection

- [ ] AirPods are paired with Windows (in Windows Settings)
- [ ] Click "Connect" button in LibrePods
- [ ] Connection status changes to "Connected"
- [ ] No error messages during connection
- [ ] Disconnect button appears

## Battery Status Testing

- [ ] Battery levels appear for Left AirPod
- [ ] Battery level appears for Right AirPod
- [ ] Battery level appears for Case
- [ ] Battery percentages are accurate (compare with iPhone/Mac)
- [ ] Charging status shows correctly when charging
- [ ] Battery updates in real-time

## Ear Detection Testing

- [ ] Put Left AirPod in ear → shows "In Ear"
- [ ] Put Right AirPod in ear → shows "In Ear"
- [ ] Remove Left AirPod → shows "Out"
- [ ] Remove Right AirPod → shows "Out"
- [ ] Status updates in real-time (within 1-2 seconds)

## Automatic Media Control

- [ ] Start playing music/video in any app
- [ ] Remove both AirPods → media pauses
- [ ] Put AirPods back in → media resumes
- [ ] Remove one AirPod → media pauses (if enabled)
- [ ] Works with Spotify, YouTube, Windows Media Player, etc.

## Noise Control Mode Testing

### Off Mode
- [ ] Select "Off" radio button
- [ ] AirPods switch to Off mode
- [ ] Can hear background noise normally

### Transparency Mode
- [ ] Select "Transparency" radio button
- [ ] AirPods switch to Transparency
- [ ] Can hear surroundings clearly
- [ ] Voice sounds natural

### Adaptive Transparency Mode (AirPods Pro 2+)
- [ ] Select "Adaptive Transparency" radio button
- [ ] AirPods switch to Adaptive mode
- [ ] Loud sounds are reduced
- [ ] Normal sounds pass through

### Noise Cancellation
- [ ] Select "Noise Cancellation" radio button
- [ ] AirPods switch to ANC
- [ ] Background noise is blocked
- [ ] Noticeable difference from Off mode

### Mode Switching
- [ ] Switching between modes is instant (< 1 second)
- [ ] No audio glitches during switching
- [ ] Selected mode persists after reconnect

## Conversational Awareness (AirPods Pro 2+)

- [ ] Enable "Conversational Awareness" checkbox
- [ ] Status shows "Inactive"
- [ ] Start speaking while music plays
- [ ] Status changes to "Active"
- [ ] Volume reduces automatically
- [ ] Stop speaking → volume returns
- [ ] Disable checkbox → feature turns off

## Device Renaming

- [ ] Enter new name in "Device Name" field
- [ ] Click "Rename Device" button
- [ ] Success message appears
- [ ] Note: May need re-pairing to see change in Windows

## Disconnect and Reconnect

- [ ] Click "Disconnect" button
- [ ] Connection status changes to "Disconnected"
- [ ] All features become disabled
- [ ] Click "Connect" button
- [ ] Reconnects successfully
- [ ] All previous settings restored

## Application Behavior

### Window Management
- [ ] Minimize window → works correctly
- [ ] Restore window → works correctly
- [ ] Close window → app minimizes to tray (if implemented)
- [ ] Application doesn't freeze or hang

### Resource Usage
- [ ] Check Task Manager
- [ ] CPU usage < 5% when idle
- [ ] Memory usage < 200 MB
- [ ] No memory leaks over time (monitor for 10+ minutes)

### Error Handling
- [ ] Turn off Bluetooth → app handles gracefully
- [ ] Turn AirPods off → app detects disconnect
- [ ] Open case while connected → status updates
- [ ] No crashes during error scenarios

## Advanced Features Testing (Optional)

### Device ID Spoofing (Advanced Users Only)
- [ ] Follow Device ID spoofing instructions
- [ ] Restart computer
- [ ] Re-pair AirPods
- [ ] Hearing aid features become available
- [ ] Multi-device features work

### Head Gestures (If Supported)
- [ ] Nod head → gesture detected and logged
- [ ] Shake head → gesture detected and logged
- [ ] Gestures trigger configured actions

## Logging and Debugging

- [ ] Check log file location (if implemented)
- [ ] Logs contain useful information
- [ ] No excessive logging
- [ ] Error messages are clear and actionable

## Installer Testing (If Built)

- [ ] Run installer on clean Windows machine
- [ ] Installer checks for .NET runtime
- [ ] Installs successfully
- [ ] Desktop/Start Menu shortcuts created
- [ ] Application runs after install
- [ ] Uninstaller works correctly

## Documentation Verification

- [ ] README.md is clear and accurate
- [ ] QUICKSTART.md works for new users
- [ ] IMPLEMENTATION.md matches actual code
- [ ] Troubleshooting section is helpful
- [ ] All links in documentation work

## Cross-Model Testing (If Available)

Test with different AirPods models:
- [ ] AirPods 1/2 (basic features)
- [ ] AirPods 3 (basic + some features)
- [ ] AirPods Pro 1 (noise control)
- [ ] AirPods Pro 2 (all features)
- [ ] AirPods Max (all features)

## Known Issues to Verify

- [ ] RFCOMM fallback works for connection
- [ ] Some advanced features may not work without L2CAP
- [ ] VendorID spoofing requirements are clear

## Test Results Summary

**Date**: _______________
**Tester**: _______________
**Windows Version**: _______________
**AirPods Model**: _______________
**Build Version**: _______________

**Overall Result**: ☐ Pass ☐ Fail ☐ Partial

**Issues Found**:
1. _______________________________________________
2. _______________________________________________
3. _______________________________________________

**Notes**:
_______________________________________________
_______________________________________________
_______________________________________________

## Reporting Issues

If you encounter issues:
1. Check the Troubleshooting section in README.md
2. Collect logs and error messages
3. Note your Windows version, .NET version, AirPods model
4. Create detailed GitHub issue with steps to reproduce
5. Include screenshots if applicable

## Success Criteria

For the implementation to be considered successful:
- ✅ Builds without errors
- ✅ Connects to AirPods reliably
- ✅ Battery status displays correctly
- ✅ Noise control modes work
- ✅ Ear detection functions
- ✅ No crashes or major bugs
- ✅ UI is responsive and intuitive
