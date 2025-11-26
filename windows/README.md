# Windows AACP transport scaffold

This directory hosts a minimal KMDF driver and user-mode wrapper intended to expose the RFCOMM/L2CAP access paths required by AACP devices. The layout mirrors the Android/Linux logic documented elsewhere in the repository and provides IOCTL contracts that can be consumed from C++ or WinRT callers.

## Components

- `driver/`: KMDF driver skeleton with IOCTL handlers for open/close, read/write, and keepalive. The actual Bluetooth binding is intentionally stubbed; Windows Bluetooth kernel APIs (bthport) must be used to associate the RFCOMM/L2CAP socket with the PSM values 0x2B/0x2C.
- `transport/`: User-mode C++ wrapper that opens the device interface, issues IOCTLs, and provides reconnection-friendly helpers and scanning hooks. The scanning stub should be replaced with a WinRT-based discovery that filters manufacturer data for Apple (0x004C) and the proximity payload used in the Linux `BleManager` path.

## Reconnect/keepalive guidance

The wrapper exposes a `Keepalive` call so higher-level code can implement the same timers used on Android sessions. On timeout, close and re-open the channel with the desired PSM and peer address to mirror the retry strategy implemented in the Android session layer.

## Building

1. Use the Windows Driver Kit (WDK) with Visual Studio to create a KMDF project and import the sources from `driver/`.
2. Ensure the driver is signed or test-signed for deployment.
3. Build the user-mode wrapper as part of a DLL or static library and link against your application.

> **Note:** These files are scaffolding and do not ship a production-ready driver; they document the IOCTL contract and provide stubs to extend with full Bluetooth channel management.

