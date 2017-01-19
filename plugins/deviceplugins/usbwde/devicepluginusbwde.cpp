/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2016 Alexander Lampret <alexander.lampret@gmail.com>     *
 *                                                                         *
 *  This file is part of guh.                                              *
 *                                                                         *
 *  Guh is free software: you can redistribute it and/or modify            *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, version 2 of the License.                *
 *                                                                         *
 *  Guh is distributed in the hope that it will be useful,                 *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with guh. If not, see <http://www.gnu.org/licenses/>.            *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "devicepluginusbwde.h"

#include "plugin/device.h"
#include "devicemanager.h"
#include "plugininfo.h"

DevicePluginUsbWde::DevicePluginUsbWde() :
    m_bridgeDevice(0)
{
}

DeviceManager::HardwareResources DevicePluginUsbWde::requiredHardware() const
{
    return DeviceManager::HardwareResourceTimer;
}

DeviceManager::DeviceSetupStatus DevicePluginUsbWde::setupDevice(Device *device)
{
    if (device->deviceClassId() == wdeBridgeDeviceClassId) {
        if (m_bridgeDevice != 0) {
            qCWarning(dcUsbWde) << "Only one USB WDE device can be configured.";
            return DeviceManager::DeviceSetupStatusFailure;
        }
        m_serialPort = new QSerialPort(this);
        m_serialPort->setPortName(device->paramValue(interfaceParamTypeId).toString());
        m_serialPort->setBaudRate(device->paramValue(baudrateParamTypeId).toInt());
        if (!m_serialPort->open(QIODevice::ReadOnly)) {
            qCWarning(dcUsbWde) << device->name() << "can't bind to interface" << device->paramValue(interfaceParamTypeId);
            return DeviceManager::DeviceSetupStatusFailure;
        }
        m_bridgeDevice = device;
        connect(m_serialPort, SIGNAL(readyRead()), SLOT(handleReadyRead()));
        connect(m_serialPort, SIGNAL(error(QSerialPort::SerialPortError)), SLOT(handleError(QSerialPort::SerialPortError)));
    } else {
        m_deviceList.insert(device->paramValue(channelParamTypeId).toInt(), device);
    }
    return DeviceManager::DeviceSetupStatusSuccess;
}

void DevicePluginUsbWde::deviceRemoved(Device *device)
{
    if (device->deviceClassId() == wdeBridgeDeviceClassId) {
        m_serialPort->close();
        m_bridgeDevice = 0;
    } else {
        m_deviceList.remove(device->paramValue(channelParamTypeId).toInt());
    }
}

void DevicePluginUsbWde::handleReadyRead()
{
    m_readData.append(m_serialPort->readAll());
}

void DevicePluginUsbWde::handleError(QSerialPort::SerialPortError serialPortError)
{
    if (serialPortError == QSerialPort::ReadError) {
        qCWarning(dcUsbWde) << "An I/O error occurred while reading the data from port " << m_serialPort->portName() << ", error: " << m_serialPort->errorString();
    }
}

void DevicePluginUsbWde::guhTimer()
{
    if (!m_readData.isEmpty()) {
        // Handle data
        QList<QByteArray> parts = m_readData.split(';');
        QLocale german(QLocale::German);
        // Check if received string is valid (should start with $1 and ends with 0)
        if (parts.size() != 25 || !parts.at(0).contains("$1") || !parts.at(24).contains("0")) {
            m_readData.clear();
            return;
        }
        // Loop through 8 possible sensor channels
        for (int i = 1; i < 9; i++) {
            if (!parts.at(2 + i).isEmpty()) {
                // Create new device if it does not exist
                if (!m_deviceList.contains(i)) {
                    createNewSensor(i);
                } else {
                    Device* device = m_deviceList.value(i);
                    device->setStateValue(temperatureStateTypeId, german.toDouble(parts.at(2+i)));
                    device->setStateValue(humidityStateTypeId, parts.at(2+i+8).toInt());
                    device->setStateValue(lastUpdateStateTypeId, QDateTime::currentDateTime().toTime_t());
                }
            }
        }
        // Check if wind data is available
        if (!parts.at(19).isEmpty()) {
            // Create new device if it does not exist
            if (!m_deviceList.contains(9)) {
                createNewSensor(9);
            } else {
                Device* device = m_deviceList.value(9);
                device->setStateValue(temperatureStateTypeId, german.toDouble(parts.at(19)));
                device->setStateValue(humidityStateTypeId, parts.at(20).toInt());
                device->setStateValue(windStrengthStateTypeId, german.toDouble(parts.at(21)));
                device->setStateValue(rainStrengthStateTypeId, german.toDouble(parts.at(22)));
                device->setStateValue(isRainStateTypeId, (parts.at(23) == "1"));
                device->setStateValue(lastUpdateStateTypeId, QDateTime::currentDateTime().toTime_t());
            }
        }
        m_readData.clear();
    }
}

void DevicePluginUsbWde::createNewSensor(int channel)
{
    DeviceClassId createClassId;
    QString deviceName;
    QList<DeviceDescriptor> deviceDescriptors;
    createClassId = temperatureSensorDeviceClassId;
    deviceName = "Sensor channel " + QString::number(channel);
    if (channel == 9) {
        createClassId = windRainSensorDeviceClassId;
        deviceName = "Weather station";
    }
    DeviceDescriptor descriptor(createClassId, deviceName, deviceName);
    ParamList params;
    params.append(Param(nameParamTypeId, "Sensor " + QString::number(channel)));
    params.append(Param(channelParamTypeId, channel));
    descriptor.setParams(params);
    deviceDescriptors.append(descriptor);
    emit autoDevicesAppeared(createClassId, deviceDescriptors);
}
