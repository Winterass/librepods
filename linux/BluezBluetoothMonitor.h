#ifndef BLUEZBLUETOOTHMONITOR_H
#define BLUEZBLUETOOTHMONITOR_H

#include <QObject>
#include <QtDBus/QtDBus>

#include "bluetooth/IBluetoothMonitor.h"
#include "ble/blemanager.h"

// Forward declarations for D-Bus types
typedef QMap<QDBusObjectPath, QMap<QString, QVariantMap>> ManagedObjectList;
Q_DECLARE_METATYPE(ManagedObjectList)

class BluezBluetoothMonitor : public IBluetoothMonitor, protected QDBusContext
{
    Q_OBJECT
public:
    explicit BluezBluetoothMonitor(QObject *parent = nullptr);
    ~BluezBluetoothMonitor() override;

    void start() override;
    void startScan() override;
    void stopScan() override;
    bool isScanning() const override;
    bool checkAlreadyConnectedDevices() override;

private slots:
    void onPropertiesChanged(const QString &interface, const QVariantMap &changedProps, const QStringList &invalidatedProps);

private:
    QDBusConnection m_dbus;
    BleManager *m_bleManager;

    void registerDBusService();
    bool isAirPodsDevice(const QString &devicePath);
    QString getDeviceName(const QString &devicePath);
};

#endif // BLUEZBLUETOOTHMONITOR_H
