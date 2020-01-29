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

#ifndef BROWSERITEMRESULT_H
#define BROWSERITEMRESULT_H

#include <QObject>

#include "device.h"

class BrowserItemResult : public QObject
{
    Q_OBJECT

public:
    explicit BrowserItemResult(Device *device, const QString &itemId, const QLocale &locale, QObject *parent, quint32 timeout = 0);

    Device* device() const;
    QString itemId() const;
    QLocale locale() const;

    BrowserItem item() const;

    bool isFinished() const;
    Device::DeviceError status() const;

public slots:
    void finish(const BrowserItem &item);
    void finish(Device::DeviceError status);

signals:
    void finished();
    void aborted();

private:
    Device *m_device = nullptr;
    QString m_itemId;
    QLocale m_locale;

    BrowserItem m_item;

    bool m_finished = false;
    Device::DeviceError m_status = Device::DeviceErrorNoError;
};

#endif // BROWSERITEMRESULT_H