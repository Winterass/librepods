#include "BluezBluetoothMonitor.h"
#include "logger.h"

#include <QDebug>
#include <QDBusObjectPath>
#include <QDBusMetaType>

BluezBluetoothMonitor::BluezBluetoothMonitor(QObject *parent)
    : IBluetoothMonitor(parent), m_dbus(QDBusConnection::systemBus()), m_bleManager(new BleManager(this))
{
    // Register meta-types for D-Bus interaction
    qDBusRegisterMetaType<QDBusObjectPath>();
    qDBusRegisterMetaType<ManagedObjectList>();

    connect(m_bleManager, &BleManager::deviceFound, this, &BluezBluetoothMonitor::deviceFound);

    if (!m_dbus.isConnected())
    {
        LOG_WARN("Failed to connect to system D-Bus");
        return;
    }

    registerDBusService();
}

BluezBluetoothMonitor::~BluezBluetoothMonitor()
{
    if (m_bleManager)
    {
        m_bleManager->stopScan();
    }
    m_dbus.disconnectFromBus(m_dbus.name());
}

void BluezBluetoothMonitor::start()
{
    checkAlreadyConnectedDevices();
    startScan();
}

void BluezBluetoothMonitor::startScan()
{
    if (m_bleManager)
    {
        m_bleManager->startScan();
    }
}

void BluezBluetoothMonitor::stopScan()
{
    if (m_bleManager)
    {
        m_bleManager->stopScan();
    }
}

bool BluezBluetoothMonitor::isScanning() const
{
    return m_bleManager && m_bleManager->isScanning();
}

void BluezBluetoothMonitor::registerDBusService()
{
    // Match signals for PropertiesChanged on any BlueZ Device interface
    if (!m_dbus.connect("", "", "org.freedesktop.DBus.Properties", "PropertiesChanged",
                        this, SLOT(onPropertiesChanged(QString, QVariantMap, QStringList))))
    {
        LOG_WARN("Failed to connect to D-Bus PropertiesChanged signal");
    }
}

bool BluezBluetoothMonitor::isAirPodsDevice(const QString &devicePath)
{
    QDBusInterface deviceInterface("org.bluez", devicePath, "org.freedesktop.DBus.Properties", m_dbus);

    // Get UUIDs to check if it's an AirPods device
    QDBusReply<QVariant> uuidsReply = deviceInterface.call("Get", "org.bluez.Device1", "UUIDs");
    if (!uuidsReply.isValid())
    {
        return false;
    }

    QStringList uuids = uuidsReply.value().toStringList();
    return uuids.contains("74ec2172-0bad-4d01-8f77-997b2be0722a");
}

QString BluezBluetoothMonitor::getDeviceName(const QString &devicePath)
{
    QDBusInterface deviceInterface("org.bluez", devicePath, "org.freedesktop.DBus.Properties", m_dbus);
    QDBusReply<QVariant> nameReply = deviceInterface.call("Get", "org.bluez.Device1", "Name");
    if (nameReply.isValid())
    {
        return nameReply.value().toString();
    }
    return "Unknown";
}

bool BluezBluetoothMonitor::checkAlreadyConnectedDevices()
{
    if (!m_dbus.isConnected())
    {
        return false;
    }

    QDBusInterface objectManager("org.bluez", "/", "org.freedesktop.DBus.ObjectManager", m_dbus);
    QDBusMessage reply = objectManager.call("GetManagedObjects");

    if (reply.type() == QDBusMessage::ErrorMessage)
    {
        LOG_WARN("Failed to get managed objects: " << reply.errorMessage());
        return false;
    }

    QVariant firstArg = reply.arguments().constFirst();
    QDBusArgument arg = firstArg.value<QDBusArgument>();
    ManagedObjectList managedObjects;
    arg >> managedObjects;

    bool deviceFound = false;

    for (auto it = managedObjects.constBegin(); it != managedObjects.constEnd(); ++it)
    {
        const QDBusObjectPath &objPath = it.key();
        const QMap<QString, QVariantMap> &interfaces = it.value();

        if (interfaces.contains("org.bluez.Device1"))
        {
            const QVariantMap &deviceProps = interfaces.value("org.bluez.Device1");

            // Check if the device has the necessary properties
            if (!deviceProps.contains("UUIDs") || !deviceProps.contains("Connected") ||
                !deviceProps.contains("Address") || !deviceProps.contains("Name"))
            {
                continue;
            }

            QStringList uuids = deviceProps["UUIDs"].toStringList();
            bool isAirPods = uuids.contains("74ec2172-0bad-4d01-8f77-997b2be0722a");

            if (isAirPods)
            {
                bool connected = deviceProps["Connected"].toBool();
                if (connected)
                {
                    QString macAddress = deviceProps["Address"].toString();
                    QString deviceName = deviceProps["Name"].toString();
                    emit deviceConnected(macAddress, deviceName);
                    LOG_DEBUG("Found already connected AirPods: " << macAddress << " Name: " << deviceName);
                    deviceFound = true;
                }
            }
        }
    }
    return deviceFound;
}

void BluezBluetoothMonitor::onPropertiesChanged(const QString &interface, const QVariantMap &changedProps, const QStringList &invalidatedProps)
{
    Q_UNUSED(invalidatedProps);

    if (interface != "org.bluez.Device1")
    {
        return;
    }

    if (changedProps.contains("Connected"))
    {
        bool connected = changedProps["Connected"].toBool();
        QString path = QDBusContext::message().path();

        if (!isAirPodsDevice(path))
        {
            return;
        }

        QDBusInterface deviceInterface("org.bluez", path, "org.freedesktop.DBus.Properties", m_dbus);

        // Get the device address
        QDBusReply<QVariant> addrReply = deviceInterface.call("Get", "org.bluez.Device1", "Address");
        if (!addrReply.isValid())
        {
            return;
        }
        QString macAddress = addrReply.value().toString();
        QString deviceName = getDeviceName(path);

        if (connected)
        {
            emit deviceConnected(macAddress, deviceName);
            LOG_DEBUG("AirPods device connected:" << macAddress << " Name:" << deviceName);
        }
        else
        {
            emit deviceDisconnected(macAddress, deviceName);
            LOG_DEBUG("AirPods device disconnected:" << macAddress << " Name:" << deviceName);
        }
    }
}
