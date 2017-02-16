/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stürz <simon.stuerz@guh.io>                   *
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

#ifndef BLUETOOTHLE_H
#define BLUETOOTHLE_H

#include <QObject>
#include <QDebug>
#include <QTimer>
#include <QBluetoothHostInfo>
#include <QBluetoothDeviceInfo>
#include <QBluetoothAddress>
#include <QBluetoothLocalDevice>
#include <QBluetoothDeviceDiscoveryAgent>

#include "libguh.h"
#include "typeutils.h"

class LIBGUH_EXPORT BluetoothScanner : public QObject
{
    Q_OBJECT
public:
    explicit BluetoothScanner(QObject *parent = 0);
    bool isAvailable();
    bool isRunning();
    bool discover(const PluginId &pluginId);

private:
    QBluetoothDeviceDiscoveryAgent *m_discoveryAgent;
    QList<QBluetoothDeviceInfo> m_deviceInfos;
    QTimer *m_timer;
    bool m_available;
    PluginId m_pluginId;

signals:
    void bluetoothDiscoveryFinished(const PluginId &pluginId, const QList<QBluetoothDeviceInfo> &deviceInfos);

private slots:
    void deviceDiscovered(const QBluetoothDeviceInfo &device);
    void onError(QBluetoothDeviceDiscoveryAgent::Error error);
    void discoveryTimeout();
};

#endif // BLUETOOTHLE_H
