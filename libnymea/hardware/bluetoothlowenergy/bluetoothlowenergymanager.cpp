/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2017-2018 Simon Stürz <simon.stuerz@guh.io>              *
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
    \class BluetoothLowEnergyManager
    \brief The BluetoothLowEnergyManager class helps to interact with Bluetooth LE devices.

    \ingroup hardware
    \inmodule libnymea

    \sa HardwareResource
*/

/*! \fn BluetoothLowEnergyManager::~BluetoothLowEnergyManager();
    The virtual destructor of the BluetoothLowEnergyManager.
*/

/*! \fn BluetoothDiscoveryReply *BluetoothLowEnergyManager::discoverDevices(int interval = 5000);
    This method starts a Bluetooth discovery process running for \a interval milli seconds. Returns a BluetoothDiscoveryReply object
    which will emits the \l{BluetoothDiscoveryReply::finished()}{finished()} signal when the
    \l{BluetoothDiscoveryReply::discoveredDevices()}{discoveredDevices()} list is ready.
*/

/*! \fn BluetoothLowEnergyDevice *BluetoothLowEnergyManager::registerDevice(const QBluetoothDeviceInfo &deviceInfo, const QLowEnergyController::RemoteAddressType &addressType = QLowEnergyController::RandomAddress);
    This method should be used to register a bluetooth device in your DevicePlugin. Returns a new BluetoothLowEnergyDevice object with the given \a deviceInfo and \a addressType.
*/

/*! \fn void BluetoothLowEnergyManager::unregisterDevice(BluetoothLowEnergyDevice *bluetoothDevice);
    This method should be used to unregister the given \a bluetoothDevice in your DevicePlugin if you don't need it any more.
*/


#include "bluetoothlowenergymanager.h"
#include "loggingcategories.h"

/*! Constructs a \l{BluetoothLowEnergyManager} with the given \a parent. */
BluetoothLowEnergyManager::BluetoothLowEnergyManager(QObject *parent) :
    HardwareResource("Bluetooth LE manager", parent)
{
}

/*! This method enables / disables this hardwareresource for all plugins. This method is available on the D-Bus. This can be usefull if a Bluetooth LE server
    needs access to the hardware. By disabling the bluetooth support, nymea will not allow to use the hardware until it gets reenabled.
*/
void BluetoothLowEnergyManager::EnableBluetooth(bool enabled)
{
    setEnabled(enabled);
}
