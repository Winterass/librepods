#ifndef WINDOWSBLEMONITOR_H
#define WINDOWSBLEMONITOR_H

#include <QBluetoothLocalDevice>
#include <QObject>
#include <QMap>

#include "bluetooth/IBluetoothMonitor.h"
#include "ble/blemanager.h"

class WindowsBluetoothMonitor : public IBluetoothMonitor
{
    Q_OBJECT
public:
    explicit WindowsBluetoothMonitor(QObject *parent = nullptr);
    ~WindowsBluetoothMonitor() override;

    void start() override;
    void startScan() override;
    void stopScan() override;
    bool isScanning() const override;
    bool checkAlreadyConnectedDevices() override;

private slots:
    void onDeviceConnected(const QBluetoothAddress &address);
    void onDeviceDisconnected(const QBluetoothAddress &address);
    void rememberDevice(const BleInfo &device);

private:
    QBluetoothLocalDevice m_localDevice;
    BleManager *m_bleManager;
    QMap<QString, QString> m_deviceNames;
};

#endif // WINDOWSBLEMONITOR_H
