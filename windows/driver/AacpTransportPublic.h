#pragma once

// Public definitions shared between the KMDF driver and user-mode transport wrapper.
// The IOCTL contract is intentionally small: open/close channels and buffered read/write.

#include <initguid.h>
#include <wdm.h>

// {C3A67B6B-5F06-4E72-9095-1C071CD1D0C4}
DEFINE_GUID(GUID_DEVINTERFACE_AACP_TRANSPORT,
    0xc3a67b6b, 0x5f06, 0x4e72, 0x90, 0x95, 0x1c, 0x07, 0x1c, 0xd1, 0xd0, 0xc4);

// PSM values for AACP per Apple accessory specification.
#define AACP_PSM_PRIMARY  0x002B
#define AACP_PSM_SECONDARY 0x002C

// Channel identifiers to disambiguate between RFCOMM and L2CAP flows.
typedef enum _AACP_CHANNEL_TARGET {
    AacpChannelPrimary = 0,
    AacpChannelSecondary = 1
} AACP_CHANNEL_TARGET;

// IOCTL payloads
#pragma pack(push, 1)
typedef struct _AACP_OPEN_CHANNEL_REQUEST {
    AACP_CHANNEL_TARGET Channel;
    ULONG RemoteL2capPsm; // 0x2B or 0x2C depending on channel
    UCHAR RemoteAddress[6]; // Bluetooth MAC, little endian
} AACP_OPEN_CHANNEL_REQUEST, *PAACP_OPEN_CHANNEL_REQUEST;

typedef struct _AACP_CLOSE_CHANNEL_REQUEST {
    AACP_CHANNEL_TARGET Channel;
} AACP_CLOSE_CHANNEL_REQUEST, *PAACP_CLOSE_CHANNEL_REQUEST;

// Read/Write requests are buffered; read writes into caller supplied buffer.
typedef struct _AACP_RW_REQUEST {
    AACP_CHANNEL_TARGET Channel;
    ULONG TimeoutMs;
    ULONG PayloadLength;
    UCHAR Payload[1];
} AACP_RW_REQUEST, *PAACP_RW_REQUEST;
#pragma pack(pop)

#define FILE_DEVICE_AACP_TRANSPORT 0x8030

#define IOCTL_AACP_OPEN_CHANNEL CTL_CODE(FILE_DEVICE_AACP_TRANSPORT, 0x800, METHOD_BUFFERED, FILE_WRITE_DATA)
#define IOCTL_AACP_CLOSE_CHANNEL CTL_CODE(FILE_DEVICE_AACP_TRANSPORT, 0x801, METHOD_BUFFERED, FILE_WRITE_DATA)
#define IOCTL_AACP_READ CTL_CODE(FILE_DEVICE_AACP_TRANSPORT, 0x802, METHOD_OUT_DIRECT, FILE_READ_DATA)
#define IOCTL_AACP_WRITE CTL_CODE(FILE_DEVICE_AACP_TRANSPORT, 0x803, METHOD_IN_DIRECT, FILE_WRITE_DATA)

// Simple keepalive to allow the user-mode wrapper to pulse the transport.
#define IOCTL_AACP_KEEPALIVE CTL_CODE(FILE_DEVICE_AACP_TRANSPORT, 0x804, METHOD_BUFFERED, FILE_ANY_ACCESS)

