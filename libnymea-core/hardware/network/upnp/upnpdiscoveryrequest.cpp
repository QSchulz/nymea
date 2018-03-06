/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015-2018 Simon Stürz <simon.stuerz@guh.io>              *
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

#include "upnpdiscoveryrequest.h"
#include "loggingcategories.h"

namespace guhserver {

UpnpDiscoveryRequest::UpnpDiscoveryRequest(UpnpDiscovery *upnpDiscovery, QPointer<UpnpDiscoveryReplyImplementation> reply):
    QObject(upnpDiscovery),
    m_upnpDiscovery(upnpDiscovery),
    m_reply(reply)
{
    m_timer = new QTimer(this);
    m_timer->setSingleShot(false);
    connect(m_timer, &QTimer::timeout, this, &UpnpDiscoveryRequest::onTimeout);
}

void UpnpDiscoveryRequest::discover(int timeout)
{
    m_ssdpSearchMessage = QByteArray("M-SEARCH * HTTP/1.1\r\n"
                                              "HOST:239.255.255.250:1900\r\n"
                                              "MAN:\"ssdp:discover\"\r\n"
                                              "MX:4\r\n"
                                              "ST: " + reply()->searchTarget().toUtf8() + "\r\n"
                                              "USR-AGENT: " + reply()->userAgent().toUtf8() + "\r\n\r\n");

    m_upnpDiscovery->sendToMulticast(m_ssdpSearchMessage);

    // All 500 ms the message will be broadcasterd. So the message will be sent timeout[s] * 2
    m_totalTriggers = timeout / 500;
    m_triggerCounter = 0;

    qCDebug(dcUpnp()) << "--> Discovery called.";
    m_timer->start(500);
}

void UpnpDiscoveryRequest::addDeviceDescriptor(const UpnpDeviceDescriptor &deviceDescriptor)
{
    // check if we allready have the device in the list
    bool isAlreadyInList = false;
    foreach (UpnpDeviceDescriptor upnpDeviceDescriptor, m_deviceList) {
        if (upnpDeviceDescriptor.uuid() == deviceDescriptor.uuid()) {
            isAlreadyInList = true;
        }
    }

    if (!isAlreadyInList) {
        m_deviceList.append(deviceDescriptor);
    }
}

QNetworkRequest UpnpDiscoveryRequest::createNetworkRequest(UpnpDeviceDescriptor deviveDescriptor)
{
    QNetworkRequest deviceRequest;
    deviceRequest.setUrl(deviveDescriptor.location());
    deviceRequest.setHeader(QNetworkRequest::ContentTypeHeader,QVariant("text/xml"));
    deviceRequest.setHeader(QNetworkRequest::UserAgentHeader,QVariant(reply()->userAgent()));

    return deviceRequest;
}

QList<UpnpDeviceDescriptor> UpnpDiscoveryRequest::deviceList() const
{
    return m_deviceList;
}

QPointer<UpnpDiscoveryReplyImplementation> UpnpDiscoveryRequest::reply()
{
    return m_reply;
}

void UpnpDiscoveryRequest::onTimeout()
{
    qCDebug(dcUpnp()) << "Send SSDP search message" << m_triggerCounter << "/" << m_totalTriggers;
    m_upnpDiscovery->sendToMulticast(m_ssdpSearchMessage);

    if (m_triggerCounter >= m_totalTriggers) {
        m_timer->stop();
        emit discoveryTimeout();
    } else {
        m_triggerCounter++;
    }
}

}
