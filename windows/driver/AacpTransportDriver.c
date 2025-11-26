#include <ntddk.h>
#include <wdf.h>
#include "AacpTransportPublic.h"

// Basic KMDF skeleton to expose IOCTL entrypoints for the user-mode wrapper.
// The Bluetooth channel management code is left intentionally minimal; in a
// production driver, the L2CAP/RFCOMM binding should be backed by the bthport
// kernel APIs and appropriate security checks.

DRIVER_INITIALIZE DriverEntry;
EVT_WDF_DRIVER_DEVICE_ADD AacpTransportEvtDeviceAdd;
EVT_WDF_IO_QUEUE_IO_DEVICE_CONTROL AacpTransportEvtIoDeviceControl;

static NTSTATUS AacpTransportCreateDevice(_Inout_ PWDFDEVICE_INIT DeviceInit);

NTSTATUS DriverEntry(_In_ PDRIVER_OBJECT DriverObject, _In_ PUNICODE_STRING RegistryPath) {
    WDF_DRIVER_CONFIG config;
    WDF_DRIVER_CONFIG_INIT(&config, AacpTransportEvtDeviceAdd);

    return WdfDriverCreate(DriverObject, RegistryPath, WDF_NO_OBJECT_ATTRIBUTES, &config, WDF_NO_HANDLE);
}

NTSTATUS AacpTransportEvtDeviceAdd(_In_ WDFDRIVER Driver, _Inout_ PWDFDEVICE_INIT DeviceInit) {
    UNREFERENCED_PARAMETER(Driver);
    return AacpTransportCreateDevice(DeviceInit);
}

static NTSTATUS AacpTransportCreateDevice(_Inout_ PWDFDEVICE_INIT DeviceInit) {
    NTSTATUS status;
    WDFDEVICE device;
    WDF_IO_QUEUE_CONFIG queueConfig;

    WdfDeviceInitSetDeviceType(DeviceInit, FILE_DEVICE_UNKNOWN);
    WdfDeviceInitSetExclusive(DeviceInit, FALSE);

    status = WdfDeviceCreate(&DeviceInit, WDF_NO_OBJECT_ATTRIBUTES, &device);
    if (!NT_SUCCESS(status)) {
        return status;
    }

    status = WdfDeviceCreateDeviceInterface(device, &GUID_DEVINTERFACE_AACP_TRANSPORT, NULL);
    if (!NT_SUCCESS(status)) {
        return status;
    }

    WDF_IO_QUEUE_CONFIG_INIT_DEFAULT_QUEUE(&queueConfig, WdfIoQueueDispatchSequential);
    queueConfig.EvtIoDeviceControl = AacpTransportEvtIoDeviceControl;

    status = WdfIoQueueCreate(device, &queueConfig, WDF_NO_OBJECT_ATTRIBUTES, WDF_NO_HANDLE);
    return status;
}

static VOID AacpHandleOpenChannel(_In_ WDFREQUEST Request, _In_ size_t InputBufferLength) {
    PAACP_OPEN_CHANNEL_REQUEST parameters = NULL;

    if (InputBufferLength < sizeof(AACP_OPEN_CHANNEL_REQUEST)) {
        WdfRequestComplete(Request, STATUS_BUFFER_TOO_SMALL);
        return;
    }

    NTSTATUS status = WdfRequestRetrieveInputBuffer(Request, sizeof(AACP_OPEN_CHANNEL_REQUEST), (PVOID*)&parameters, NULL);
    if (!NT_SUCCESS(status)) {
        WdfRequestComplete(Request, status);
        return;
    }

    // TODO: Bind to bthport and create an L2CAP/RFCOMM socket for the target PSM.
    UNREFERENCED_PARAMETER(parameters);
    WdfRequestComplete(Request, STATUS_SUCCESS);
}

static VOID AacpHandleCloseChannel(_In_ WDFREQUEST Request, _In_ size_t InputBufferLength) {
    if (InputBufferLength < sizeof(AACP_CLOSE_CHANNEL_REQUEST)) {
        WdfRequestComplete(Request, STATUS_BUFFER_TOO_SMALL);
        return;
    }

    // TODO: Tear down channel state and free socket handles.
    WdfRequestComplete(Request, STATUS_SUCCESS);
}

static VOID AacpHandleRead(_In_ WDFREQUEST Request) {
    // NOTE: Real implementations must pin buffers and coordinate overlapped I/O
    // with the Bluetooth stack. This stub simply returns STATUS_PENDING to
    // emphasize asynchronous design.
    WdfRequestMarkCancelable(Request, NULL);
    WdfRequestCompleteWithInformation(Request, STATUS_PENDING, 0);
}

static VOID AacpHandleWrite(_In_ WDFREQUEST Request, _In_ size_t InputBufferLength) {
    if (InputBufferLength < sizeof(AACP_RW_REQUEST)) {
        WdfRequestComplete(Request, STATUS_BUFFER_TOO_SMALL);
        return;
    }

    // TODO: Marshal payload into the corresponding L2CAP channel.
    WdfRequestCompleteWithInformation(Request, STATUS_SUCCESS, 0);
}

static VOID AacpHandleKeepalive(_In_ WDFREQUEST Request) {
    // A lightweight keepalive to allow the user-mode wrapper to refresh idle timers.
    WdfRequestComplete(Request, STATUS_SUCCESS);
}

VOID AacpTransportEvtIoDeviceControl(_In_ WDFQUEUE Queue, _In_ WDFREQUEST Request,
    _In_ size_t OutputBufferLength, _In_ size_t InputBufferLength, _In_ ULONG IoControlCode) {
    UNREFERENCED_PARAMETER(Queue);
    UNREFERENCED_PARAMETER(OutputBufferLength);

    switch (IoControlCode) {
    case IOCTL_AACP_OPEN_CHANNEL:
        AacpHandleOpenChannel(Request, InputBufferLength);
        break;
    case IOCTL_AACP_CLOSE_CHANNEL:
        AacpHandleCloseChannel(Request, InputBufferLength);
        break;
    case IOCTL_AACP_READ:
        AacpHandleRead(Request);
        break;
    case IOCTL_AACP_WRITE:
        AacpHandleWrite(Request, InputBufferLength);
        break;
    case IOCTL_AACP_KEEPALIVE:
        AacpHandleKeepalive(Request);
        break;
    default:
        WdfRequestComplete(Request, STATUS_INVALID_DEVICE_REQUEST);
        break;
    }
}

