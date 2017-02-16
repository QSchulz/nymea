/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stürz <simon.stuerz@guh.io>                   *
 *  Copyright (C) 2014 Michael Zanetti <michael_zanetti@gmx.net>           *
 *                                                                         *
 *  This file is part of guh.                                              *
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

#ifndef DEVICEPLUGINLIRCD_H
#define DEVICEPLUGINLIRCD_H

#include "plugin/deviceplugin.h"

#include <QProcess>

class LircClient;

class DevicePluginLircd: public DevicePlugin
{
    Q_OBJECT

    Q_PLUGIN_METADATA(IID "guru.guh.DevicePlugin" FILE "devicepluginlircd.json")
    Q_INTERFACES(DevicePlugin)

public:
    explicit DevicePluginLircd();

    DeviceManager::HardwareResources requiredHardware() const override;

//    QVariantMap configuration() const override;
//    void setConfiguration(const QVariantMap &configuration) override;

private slots:
    void buttonPressed(const QString &remoteName, const QString &buttonName, int repeat);

private:
    LircClient *m_lircClient;
//    QVariantMap m_config;
};

#endif // DEVICEPLUGINBOLIRCD_H
