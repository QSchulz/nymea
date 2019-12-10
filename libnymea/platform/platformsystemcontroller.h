/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2019 Michael Zanetti <michael.zanetti@nymea.io>          *
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

#ifndef PLATFORMSYSTEMCONTROLLER_H
#define PLATFORMSYSTEMCONTROLLER_H

#include <QObject>
#include <QTimeZone>

class PlatformSystemController : public QObject
{
    Q_OBJECT
public:
    explicit PlatformSystemController(QObject *parent = nullptr);
    virtual ~PlatformSystemController() = default;

    virtual bool powerManagementAvailable() const;
    virtual bool reboot();
    virtual bool shutdown();

    virtual bool timeManagementAvailable() const;
    virtual bool automaticTimeAvailable() const;
    virtual bool automaticTime() const;
    virtual bool setTime(const QDateTime &time);
    virtual bool setAutomaticTime(bool automaticTime);
    virtual bool setTimeZone(const QTimeZone &timeZone);


signals:
    void availableChanged();
    void timeZoneManagementAvailableChanged();

    void timeConfigurationChanged();
};

Q_DECLARE_INTERFACE(PlatformSystemController, "io.nymea.PlatformSystemController")

#endif // PLATFORMSYSTEMCONTROLLER_H
