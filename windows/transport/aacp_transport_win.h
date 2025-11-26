#pragma once

#include <windows.h>
#include <cstdint>
#include <string>
#include <vector>
#include <array>

#include "../driver/AacpTransportPublic.h"

// Lightweight user-mode wrapper that opens the KMDF transport device and
// exposes blocking read/write semantics. This is intended to mirror the
// behavior used on Android/Linux where reconnect/keepalive logic runs in
// userland.
class AACPTransport {
public:
    AACPTransport();
    ~AACPTransport();

    bool Open();
    void Close();

    bool OpenChannel(AACP_CHANNEL_TARGET channel, const std::array<uint8_t, 6>& address, uint16_t psm);
    bool CloseChannel(AACP_CHANNEL_TARGET channel);

    bool Write(AACP_CHANNEL_TARGET channel, const std::vector<uint8_t>& payload, uint32_t timeoutMs);
    bool Read(AACP_CHANNEL_TARGET channel, std::vector<uint8_t>& outBuffer, uint32_t timeoutMs);

    // Discovery helpers (WinRT/Win32) used to locate Apple-originated BLE devices.
    std::vector<std::wstring> ScanForAppleDevices();

    // Keepalive is used to re-arm idle timers and detect broken channels.
    bool Keepalive();

private:
    HANDLE deviceHandle_ = INVALID_HANDLE_VALUE;
    bool SendIoctl(ULONG code, void* inBuffer, DWORD inSize, void* outBuffer, DWORD outSize, DWORD* bytesReturned);
};

