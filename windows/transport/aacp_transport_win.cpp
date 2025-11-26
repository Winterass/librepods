#include "aacp_transport_win.h"

#include <array>
#include <string_view>

using namespace std::literals;

AACPTransport::AACPTransport() = default;
AACPTransport::~AACPTransport() { Close(); }

bool AACPTransport::Open() {
    if (deviceHandle_ != INVALID_HANDLE_VALUE) {
        return true;
    }

    deviceHandle_ = CreateFileW(L"\\\\.\\AacpTransport", GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    return deviceHandle_ != INVALID_HANDLE_VALUE;
}

void AACPTransport::Close() {
    if (deviceHandle_ != INVALID_HANDLE_VALUE) {
        CloseHandle(deviceHandle_);
        deviceHandle_ = INVALID_HANDLE_VALUE;
    }
}

bool AACPTransport::SendIoctl(ULONG code, void* inBuffer, DWORD inSize, void* outBuffer, DWORD outSize, DWORD* bytesReturned) {
    if (deviceHandle_ == INVALID_HANDLE_VALUE) {
        return false;
    }
    return DeviceIoControl(deviceHandle_, code, inBuffer, inSize, outBuffer, outSize, bytesReturned, nullptr);
}

bool AACPTransport::OpenChannel(AACP_CHANNEL_TARGET channel, const std::array<uint8_t, 6>& address, uint16_t psm) {
    AACP_OPEN_CHANNEL_REQUEST request{};
    request.Channel = channel;
    request.RemoteL2capPsm = psm;
    memcpy(request.RemoteAddress, address.data(), address.size());

    return SendIoctl(IOCTL_AACP_OPEN_CHANNEL, &request, sizeof(request), nullptr, 0, nullptr);
}

bool AACPTransport::CloseChannel(AACP_CHANNEL_TARGET channel) {
    AACP_CLOSE_CHANNEL_REQUEST request{};
    request.Channel = channel;
    return SendIoctl(IOCTL_AACP_CLOSE_CHANNEL, &request, sizeof(request), nullptr, 0, nullptr);
}

bool AACPTransport::Write(AACP_CHANNEL_TARGET channel, const std::vector<uint8_t>& payload, uint32_t timeoutMs) {
    const size_t requestSize = sizeof(AACP_RW_REQUEST) + payload.size();
    std::vector<uint8_t> buffer(requestSize);

    auto* header = reinterpret_cast<PAACP_RW_REQUEST>(buffer.data());
    header->Channel = channel;
    header->TimeoutMs = timeoutMs;
    header->PayloadLength = static_cast<ULONG>(payload.size());
    memcpy(header->Payload, payload.data(), payload.size());

    return SendIoctl(IOCTL_AACP_WRITE, buffer.data(), static_cast<DWORD>(buffer.size()), nullptr, 0, nullptr);
}

bool AACPTransport::Read(AACP_CHANNEL_TARGET channel, std::vector<uint8_t>& outBuffer, uint32_t timeoutMs) {
    // Caller provides buffer size; driver returns the actual number of bytes read via DeviceIoControl.
    if (outBuffer.empty()) {
        outBuffer.resize(512);
    }

    AACP_RW_REQUEST request{};
    request.Channel = channel;
    request.TimeoutMs = timeoutMs;

    DWORD bytesReturned = 0;
    const BOOL ok = SendIoctl(IOCTL_AACP_READ, &request, sizeof(request), outBuffer.data(), static_cast<DWORD>(outBuffer.size()), &bytesReturned);
    if (!ok) {
        return false;
    }

    outBuffer.resize(bytesReturned);
    return true;
}

std::vector<std::wstring> AACPTransport::ScanForAppleDevices() {
    // The WinRT projection is preferred when available; for legacy hosts we fall back to BluetoothFindFirstDevice.
    // This placeholder only returns the device names that match the Apple manufacturer ID 0x004C.
    // Implementation should parse manufacturer data from advertisements like the Linux BleManager path.
    std::vector<std::wstring> names;
    names.emplace_back(L"(placeholder) Apple device with Mfr 0x004C");
    return names;
}

bool AACPTransport::Keepalive() {
    return SendIoctl(IOCTL_AACP_KEEPALIVE, nullptr, 0, nullptr, 0, nullptr);
}

