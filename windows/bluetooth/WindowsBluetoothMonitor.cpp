#include "WindowsBluetoothMonitor.h"

#include <QBluetoothDeviceInfo>
#include <QTimer>

#include "logger.h"

WindowsBluetoothMonitor::WindowsBluetoothMonitor(QObject *parent)
    : IBluetoothMonitor(parent), m_bleManager(new BleManager(this))
{
    connect(&m_localDevice, &QBluetoothLocalDevice::deviceConnected, this, &WindowsBluetoothMonitor::onDeviceConnected);
    connect(&m_localDevice, &QBluetoothLocalDevice::deviceDisconnected, this, &WindowsBluetoothMonitor::onDeviceDisconnected);
    connect(m_bleManager, &BleManager::deviceFound, this, &WindowsBluetoothMonitor::rememberDevice);
    connect(m_bleManager, &BleManager::deviceFound, this, &WindowsBluetoothMonitor::deviceFound);
}

WindowsBluetoothMonitor::~WindowsBluetoothMonitor()
{
    stopScan();
}

void WindowsBluetoothMonitor::start()
{
    if (m_localDevice.hostMode() == QBluetoothLocalDevice::HostPoweredOff)
    {
        m_localDevice.powerOn();
    }

    checkAlreadyConnectedDevices();
    startScan();
}

void WindowsBluetoothMonitor::startScan()
{
    if (m_bleManager && !m_bleManager->isScanning())
    {
        m_bleManager->startScan();
    }
}

void WindowsBluetoothMonitor::stopScan()
{
    if (m_bleManager)
    {
        m_bleManager->stopScan();
    }
}

bool WindowsBluetoothMonitor::isScanning() const
{
    return m_bleManager && m_bleManager->isScanning();
}

bool WindowsBluetoothMonitor::checkAlreadyConnectedDevices()
{
    bool found = false;
    const auto devices = m_localDevice.connectedDevices();
    for (const auto &address : devices)
    {
        const QString addressString = address.toString();
        const QString deviceName = m_deviceNames.value(addressString, QStringLiteral("AirPods"));
        emit deviceConnected(addressString, deviceName);
        found = true;
    }
    return found;
}

void WindowsBluetoothMonitor::onDeviceConnected(const QBluetoothAddress &address)
{
    const QString addressString = address.toString();
    emit deviceConnected(addressString, m_deviceNames.value(addressString, QStringLiteral("AirPods")));
}

void WindowsBluetoothMonitor::onDeviceDisconnected(const QBluetoothAddress &address)
{
    const QString addressString = address.toString();
    emit deviceDisconnected(addressString, m_deviceNames.value(addressString, QStringLiteral("AirPods")));
}

void WindowsBluetoothMonitor::rememberDevice(const BleInfo &device)
{
    m_deviceNames.insert(device.address, device.name);
}
