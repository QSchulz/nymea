/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2016-2018 Simon Stürz <simon.stuerz@guh.io>              *
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

#ifndef RADIO433BRENNENSTUHL_H
#define RADIO433BRENNENSTUHL_H

#include <QObject>

#include "libnymea.h"
#include "hardwareresource.h"
#include "hardware/radio433/radio433.h"
#include "radio433brennenstuhlgateway.h"

namespace guhserver {

class Radio433Brennenstuhl : public Radio433
{
    Q_OBJECT

public:
    explicit Radio433Brennenstuhl(QObject *parent = nullptr);

    bool available() const override;
    bool enabled() const override;

public slots:
    bool sendData(int delay, QList<int> rawData, int repetitions) override;

private slots:
    void brennenstuhlAvailableChanged(bool available);


protected:
    void setEnabled(bool enabled) override;

private:
    Radio433BrennenstuhlGateway *m_brennenstuhlTransmitter;
    bool m_available = false;
    bool m_enabled = false;
};

}

#endif // RADIO433BRENENSTUHL_H

