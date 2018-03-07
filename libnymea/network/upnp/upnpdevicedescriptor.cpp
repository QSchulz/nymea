/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stürz <simon.stuerz@guh.io>                   *
 *                                                                         *
 *  This file is part of nymea.                                            *
 *                                                                         *
 *  This library is free software; you can redistribute it and/or          *
 *  modify it under the terms of the GNU Lesser General Public             *
 *  License as published by the Free Software Foundation; either           *
 *  version 2.1 of the License, or (at your option) any later version.     *
 *                                                                         *
 *  This library is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      *
 *  Lesser General Public License for more details.                        *
 *                                                                         *
 *  You should have received a copy of the GNU Lesser General Public       *
 *  License along with this library; If not, see                           *
 *  <http://www.gnu.org/licenses/>.                                        *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*!
    \class UpnpDeviceDescriptor
    \brief Holds the description of an UPnP device.

    \ingroup types
    \inmodule libnymea


    \sa UpnpDevice
*/

#include "upnpdevicedescriptor.h"

/*! Constructs an UpnpDeviceDescriptor */
UpnpDeviceDescriptor::UpnpDeviceDescriptor()
{
}

/*! Sets the \a location URL of this UPnP device. */
void UpnpDeviceDescriptor::setLocation(const QUrl &location)
{
    m_location = location;
}

/*! Returns the location URL of this UPnP device. */
QUrl UpnpDeviceDescriptor::location() const
{
    return m_location;
}

/*! Sets the \a hostAddress of this UPnP device. */
void UpnpDeviceDescriptor::setHostAddress(const QHostAddress &hostAddress)
{
    m_hostAddress = hostAddress;
}

/*! Returns the host address of this UPnP device. */
QHostAddress UpnpDeviceDescriptor::hostAddress() const
{
    return m_hostAddress;
}

/*! Sets the \a port of this UPnP device. */
void UpnpDeviceDescriptor::setPort(const int &port)
{
    m_port = port;
}

/*! Returns the port of this UPnP device. */
int UpnpDeviceDescriptor::port() const
{
    return m_port;
}

/*! Sets the \a deviceType of this UPnP device. */
void UpnpDeviceDescriptor::setDeviceType(const QString &deviceType)
{
    m_deviceType = deviceType;
}

/*! Returns the type of this UPnP device. */
QString UpnpDeviceDescriptor::deviceType() const
{
    return m_deviceType;
}

/*! Sets the \a friendlyName of this UPnP device. */
void UpnpDeviceDescriptor::setFriendlyName(const QString &friendlyName)
{
    m_friendlyName = friendlyName;
}

/*! Returns the friendly name of this UPnP device. */
QString UpnpDeviceDescriptor::friendlyName() const
{
    return m_friendlyName;
}

/*! Sets the \a manufacturer of this UPnP device. */
void UpnpDeviceDescriptor::setManufacturer(const QString &manufacturer)
{
    m_manufacturer = manufacturer;
}

/*! Returns the manufacturer of this UPnP device. */
QString UpnpDeviceDescriptor::manufacturer() const
{
    return m_manufacturer;
}

/*! Sets the \a manufacturerURL of this UPnP device. */
void UpnpDeviceDescriptor::setManufacturerURL(const QUrl &manufacturerURL)
{
    m_manufacturerURL = manufacturerURL;
}

/*! Returns the manufacturer URL of this UPnP device. */
QUrl UpnpDeviceDescriptor::manufacturerURL() const
{
    return m_manufacturerURL;
}

/*! Sets the \a modelDescription of this UPnP device. */
void UpnpDeviceDescriptor::setModelDescription(const QString &modelDescription)
{
    m_modelDescription = modelDescription;
}

/*! Returns the model description of this UPnP device. */
QString UpnpDeviceDescriptor::modelDescription() const
{
    return m_modelDescription;
}

/*! Sets the \a modelName of this UPnP device. */
void UpnpDeviceDescriptor::setModelName(const QString &modelName)
{
    m_modelName = modelName;
}

/*! Returns the model name of this UPnP device. */
QString UpnpDeviceDescriptor::modelName() const
{
    return m_modelName;
}

/*! Sets the \a modelNumber of this UPnP device. */
void UpnpDeviceDescriptor::setModelNumber(const QString &modelNumber)
{
    m_modelNumber = modelNumber;
}

/*! Returns the model number of this UPnP device. */
QString UpnpDeviceDescriptor::modelNumber() const
{
    return m_modelNumber;
}

/*! Sets the \a modelURL of this UPnP device. */
void UpnpDeviceDescriptor::setModelURL(const QUrl &modelURL)
{
    m_modelURL = modelURL;
}

/*! Returns the model URL of this UPnP device. */
QUrl UpnpDeviceDescriptor::modelURL() const
{
    return m_modelURL;
}

/*! Sets the \a serialNumber of this UPnP device. */
void UpnpDeviceDescriptor::setSerialNumber(const QString &serialNumber)
{
    m_serialNumber = serialNumber;
}

/*! Returns the serial number of this UPnP device. */
QString UpnpDeviceDescriptor::serialNumber() const
{
    return m_serialNumber;
}

/*! Sets the \a uuid of this UPnP device. */
void UpnpDeviceDescriptor::setUuid(const QString &uuid)
{
    m_uuid = uuid;
}

/*! Returns the uuid of this UPnP device. */
QString UpnpDeviceDescriptor::uuid() const
{
    return m_uuid;
}

/*! Sets the \a upc (Universal Product Code) of this UPnP device. */
void UpnpDeviceDescriptor::setUpc(const QString &upc)
{
    m_upc = upc;
}

/*! Returns the UPC (Universal Product Code) of this UPnP device. */
QString UpnpDeviceDescriptor::upc() const
{
    return m_upc;
}

QDebug operator<<(QDebug debug, const UpnpDeviceDescriptor &upnpDeviceDescriptor)
{
    debug << "----------------------------------------------\n";
    debug << "UPnP device on " << upnpDeviceDescriptor.hostAddress().toString() << upnpDeviceDescriptor.port() << "\n";
    debug << "location              | " << upnpDeviceDescriptor.location() << "\n";
    debug << "friendly name         | " << upnpDeviceDescriptor.friendlyName() << "\n";
    debug << "manufacturer          | " << upnpDeviceDescriptor.manufacturer() << "\n";
    debug << "manufacturer URL      | " << upnpDeviceDescriptor.manufacturerURL().toString() << "\n";
    debug << "device type           | " << upnpDeviceDescriptor.deviceType() << "\n";
    debug << "model name            | " << upnpDeviceDescriptor.modelName() << "\n";
    debug << "model number          | " << upnpDeviceDescriptor.modelNumber() << "\n";
    debug << "model description     | " << upnpDeviceDescriptor.modelDescription() << "\n";
    debug << "model URL             | " << upnpDeviceDescriptor.modelURL().toString() << "\n";
    debug << "serial number         | " << upnpDeviceDescriptor.serialNumber() << "\n";
    debug << "UUID                  | " << upnpDeviceDescriptor.uuid() << "\n";
    debug << "UPC                   | " << upnpDeviceDescriptor.upc() << "\n\n";

    return debug;
}
