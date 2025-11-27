#ifndef IBLUETOOTHMONITOR_H
#define IBLUETOOTHMONITOR_H

#include <QObject>

#include "bluetooth/BleInfo.h"

class IBluetoothMonitor : public QObject
{
    Q_OBJECT
public:
    explicit IBluetoothMonitor(QObject *parent = nullptr) : QObject(parent) {}
    ~IBluetoothMonitor() override = default;

    virtual void start() = 0;
    virtual void startScan() = 0;
    virtual void stopScan() = 0;
    virtual bool isScanning() const = 0;
    virtual bool checkAlreadyConnectedDevices() = 0;

signals:
    void deviceConnected(const QString &macAddress, const QString &deviceName);
    void deviceDisconnected(const QString &macAddress, const QString &deviceName);
    void deviceFound(const BleInfo &device);
};

#endif // IBLUETOOTHMONITOR_H
