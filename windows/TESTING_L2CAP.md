# Testing Native L2CAP Implementation

This document describes how to test the native L2CAP implementation for Windows.

## Prerequisites

- Windows 10 version 1809 or later
- .NET 9.0 SDK installed
- AirPods paired with Windows
- Bluetooth 4.0+ adapter with L2CAP support

## Building the Application

```powershell
cd windows
dotnet restore
dotnet build -c Release
```

## Running the Application

```powershell
cd LibrePods.Desktop\bin\Release\net9.0-windows10.0.22621.0
.\LibrePods.Desktop.exe
```

Or run directly from Visual Studio:
```
F5 (Debug) or Ctrl+F5 (Run without debugging)
```

## Test Scenarios

### 1. Basic Connection Test

**Steps:**
1. Ensure AirPods are paired with Windows
2. Launch LibrePods application
3. Click "Connect" button
4. Check the logs

**Expected Results:**
- Log shows: "Connecting to device via L2CAP: [address]"
- Log shows: "L2CAP socket created successfully"
- Log shows: "Connecting to PSM 1001..."
- Log shows: "L2CAP connection established successfully"
- Log shows: "AAP handshake completed"
- Connection status changes to "Connected"

**Common Issues:**
- **Error 10061**: AirPods not powered on or out of range
- **Error 10060**: AirPods already connected to another device
- **Error 10065**: Bluetooth driver issue - update drivers

### 2. Battery Status Test

**Steps:**
1. Connect to AirPods (see Test 1)
2. Wait for battery status updates (should be automatic)
3. Check battery levels displayed in UI

**Expected Results:**
- Log shows: "Battery update: X component(s)"
- Log shows battery details like "Left: 100% (Discharging)"
- UI displays correct battery percentages
- NO AT commands in logs (AT+B, RSF=, etc.)

**What to Look For:**
```
✅ Received X bytes: 04 00 04 00 04 00 03 02 01 64 02 01 04 01 63 01 01 08 01 11 02 01
✅ Battery update: 3 component(s)
✅   Right: 100% (Discharging)
✅   Left: 99% (Charging)
✅   Case: 17% (Discharging)

❌ Received: 41 54 2B 42  (AT commands = bad!)
❌ 0 batteries are parsed
```

### 3. Noise Control Mode Test

**Steps:**
1. Connect to AirPods
2. Click different noise control modes in UI
   - Off
   - Transparency
   - Adaptive Transparency
   - Noise Cancellation
3. Verify mode changes on AirPods

**Expected Results:**
- Log shows: "Setting noise control mode to: [mode]"
- Log shows: "Sent X bytes: 04 00 04 00 09 00 0D [mode byte] 00 00 00"
- AirPods physically change mode
- Log shows response: "Noise control mode: [mode]"

### 4. Ear Detection Test

**Steps:**
1. Connect to AirPods
2. Put AirPods in ears
3. Remove AirPods from ears
4. Put AirPods back in case

**Expected Results:**
- Log shows: "Ear detection: Left=[bool], Right=[bool]"
- UI updates to show in/out of ear status
- Media playback pauses/resumes accordingly

### 5. Conversational Awareness Test

**Steps:**
1. Connect to AirPods Pro 2
2. Enable Conversational Awareness in UI
3. Start speaking while wearing AirPods

**Expected Results:**
- Log shows: "Conversational awareness: enabled=true, active=true/false"
- Volume automatically lowers when speaking
- Volume returns to normal when stopped speaking

### 6. Reconnection Test

**Steps:**
1. Connect to AirPods
2. Put AirPods in case (disconnect)
3. Take AirPods out and wait for reconnection
4. Click "Connect" again

**Expected Results:**
- First connection succeeds
- Disconnect detected: "Connection closed by remote device"
- Reconnection succeeds with same steps as initial connection

### 7. Long-running Stability Test

**Steps:**
1. Connect to AirPods
2. Leave connected for 30+ minutes
3. Perform various operations intermittently

**Expected Results:**
- Connection remains stable
- Battery updates continue periodically
- No memory leaks
- No crashes or hangs

## Debugging

### Enable Verbose Logging

Check application logs at:
```
%APPDATA%\LibrePods\logs\librepods.log
```

### Common Log Patterns

**Successful L2CAP Connection:**
```
[Info] Connecting to device via L2CAP: XX:XX:XX:XX:XX:XX
[Debug] Parsed Bluetooth address: XXXXXXXXXXXX
[Debug] L2CAP socket created successfully
[Debug] Connecting to PSM 1001...
[Info] L2CAP connection established successfully
[Debug] Sending AAP handshake...
[Debug] Sent 16 bytes: 00 00 04 00 01 00 02 00 00 00 00 00 00 00 00 00
[Debug] Sent 14 bytes: 04 00 04 00 4D 00 FF 00 00 00 00 00 00 00
[Debug] Sent 10 bytes: 04 00 04 00 0F 00 FF FF FF FF
[Debug] AAP handshake completed
[Info] Started L2CAP receive loop
```

**Battery Update:**
```
[Debug] Received 22 bytes: 04 00 04 00 04 00 03 02 01 64 02 01 04 01 63 01 01 08 01 11 02 01
[Info] Battery update: 3 component(s)
[Debug]   Right: 100% (Discharging)
[Debug]   Left: 99% (Charging)
[Debug]   Case: 17% (Discharging)
```

### Network Trace (Advanced)

For low-level debugging, use Wireshark with Bluetooth capture:
1. Install Wireshark with Bluetooth support
2. Start capture on Bluetooth interface
3. Filter for `bthci_acl` and `l2cap.psm == 0x1001`
4. Analyze L2CAP packets

## Performance Metrics

Expected metrics for healthy connection:

- **Connection Time**: < 3 seconds
- **Battery Update Frequency**: Every 30-60 seconds (depends on AirPods)
- **Mode Change Response Time**: < 500ms
- **Memory Usage**: < 50MB (stable over time)
- **CPU Usage**: < 1% when idle, < 5% during active communication

## Troubleshooting

### Connection Fails Immediately

**Symptoms:**
- Error code 10061 or 10065
- Connection attempt fails within 1 second

**Solutions:**
1. Verify AirPods are paired in Windows Bluetooth settings
2. Ensure AirPods are not connected to another device
3. Update Bluetooth drivers to latest version
4. Try running as Administrator
5. Restart Bluetooth service:
   ```powershell
   Restart-Service bthserv
   ```

### Receiving AT Commands Instead of AAP Packets

**Symptoms:**
- Logs show: `Received: 41 54 2B 42`
- Battery status shows 0 batteries parsed

**This should NOT happen with native L2CAP!**

If you see this, the implementation is not working correctly:
1. Verify build configuration (Release vs Debug)
2. Check that NativeBluetoothManager is being used
3. Verify Win32 APIs are available on your system
4. File a bug report with full logs

### No Battery Updates

**Symptoms:**
- Connection succeeds
- No battery packets received after 60+ seconds

**Solutions:**
1. Take AirPods out of case (they must be active)
2. Wear the AirPods (some models only report when worn)
3. Wait up to 2 minutes for first update
4. Check logs for any error messages in receive loop

### Intermittent Disconnections

**Symptoms:**
- Connection drops randomly
- Log shows: "Connection closed by remote device"

**Solutions:**
1. Ensure AirPods battery is not critically low
2. Reduce Bluetooth interference (move away from Wi-Fi router, etc.)
3. Check for Bluetooth driver updates
4. Verify USB Bluetooth adapter is properly powered (if using one)

## Success Criteria

The implementation is working correctly if:

✅ Connection establishes within 3 seconds
✅ Battery packets received within 60 seconds
✅ NO AT commands in logs (41 54 2B 42, 52 53 46 3D, etc.)
✅ Noise control mode changes work immediately
✅ Ear detection events received correctly
✅ Connection remains stable for 30+ minutes
✅ No memory leaks or crashes
✅ All AAP protocol features functional

## Reporting Issues

If you encounter issues, please provide:

1. Windows version (run `winver`)
2. Bluetooth adapter model
3. AirPods model
4. Full application log file
5. Steps to reproduce
6. Expected vs actual behavior

Submit issues at: https://github.com/Winterass/librepods/issues
