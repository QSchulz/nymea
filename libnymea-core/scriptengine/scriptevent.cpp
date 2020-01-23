/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright 2013 - 2020, nymea GmbH
* Contact: contact@nymea.io
*
* This file is part of nymea.
* This project including source code and documentation is protected by
* copyright law, and remains the property of nymea GmbH. All rights, including
* reproduction, publication, editing and translation, are reserved. The use of
* this project is subject to the terms of a license agreement to be concluded
* with nymea GmbH in accordance with the terms of use of nymea GmbH, available
* under https://nymea.io/license
*
* GNU General Public License Usage
* Alternatively, this project may be redistributed and/or modified under the
* terms of the GNU General Public License as published by the Free Software
* Foundation, GNU version 3. This project is distributed in the hope that it
* will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
* of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
* Public License for more details.
*
* You should have received a copy of the GNU General Public License along with
* this project. If not, see <https://www.gnu.org/licenses/>.
*
* For any further details and any questions please contact us under
* contact@nymea.io or see our FAQ/Licensing Information on
* https://nymea.io/license/faq
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "scriptevent.h"

#include <qqml.h>
#include <QQmlEngine>

namespace nymeaserver {

ScriptEvent::ScriptEvent(QObject *parent) : QObject(parent)
{
}

void ScriptEvent::classBegin()
{
    m_deviceManager = reinterpret_cast<DeviceManager*>(qmlEngine(this)->property("deviceManager").toULongLong());
    connect(m_deviceManager, &DeviceManager::eventTriggered, this, &ScriptEvent::onEventTriggered);
}

void ScriptEvent::componentComplete()
{

}

QString ScriptEvent::deviceId() const
{
    return m_deviceId;
}

void ScriptEvent::setDeviceId(const QString &deviceId)
{
    if (m_deviceId != deviceId) {
        m_deviceId = deviceId;
        emit deviceIdChanged();
    }
}

QString ScriptEvent::eventTypeId() const
{
    return m_eventTypeId;
}

void ScriptEvent::setEventTypeId(const QString &eventTypeId)
{
    if (m_eventTypeId != eventTypeId) {
        m_eventTypeId = eventTypeId;
        emit eventTypeIdChanged();
    }
}

QString ScriptEvent::eventName() const
{
    return m_eventName;
}

void ScriptEvent::setEventName(const QString &eventName)
{
    if (m_eventName != eventName) {
        m_eventName = eventName;
        emit eventNameChanged();
    }
}

void ScriptEvent::onEventTriggered(const Event &event)
{
    if (DeviceId(m_deviceId) != event.deviceId()) {
        return;
    }

    if (!m_eventTypeId.isEmpty() && event.eventTypeId() != m_eventTypeId) {
        return;
    }

    Device *device = m_deviceManager->findConfiguredDevice(event.deviceId());
    if (!m_eventName.isEmpty() && device->deviceClass().eventTypes().findByName(m_eventName).id() != event.eventTypeId()) {
        return;
    }

//    ScriptParams *params = new ScriptParams(event.params());
//    qmlEngine(this)->setObjectOwnership(params, QQmlEngine::JavaScriptOwnership);
    QVariantMap params;
    foreach (const Param &param, event.params()) {
        params.insert(param.paramTypeId().toString().remove(QRegExp("[{}]")), param.value());
        QString paramName = device->deviceClass().eventTypes().findById(event.eventTypeId()).paramTypes().findById(param.paramTypeId()).name();
        params.insert(paramName, param.value());
    }

    emit triggered(params);
}

}

