/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stürz <simon.stuerz@guh.io>                   *
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

#include "tveventhandler.h"
#include "extern-plugininfo.h"

TvEventHandler::TvEventHandler(const QHostAddress &host, const int &port, QObject *parent) :
    QTcpServer(parent),
    m_host(host),
    m_port(port),
    m_expectingData(false)
{
    listen(QHostAddress::AnyIPv4, m_port);
}

void TvEventHandler::incomingConnection(qintptr socket)
{
    QTcpSocket* tcpSocket = new QTcpSocket(this);
    tcpSocket->setSocketDescriptor(socket);
    qCDebug(dcLgSmartTv) << "Event handler -> incoming connection" << tcpSocket->peerAddress().toString() << tcpSocket->peerName();

    connect(tcpSocket, &QTcpSocket::readyRead, this, &TvEventHandler::readClient);
    connect(tcpSocket, &QTcpSocket::disconnected, this, &TvEventHandler::onDisconnected);
}

void TvEventHandler::readClient()
{
    QTcpSocket* socket = (QTcpSocket*)sender();

    // reject everything, except the tv
    if(socket->peerAddress() != m_host){
        socket->close();
        socket->deleteLater();
        qCWarning(dcLgSmartTv) << "Event handler -> rejecting connection from " << socket->peerAddress().toString();
        return;
    }


    // the tv sends first the header (POST /udap/api/.... HTTP/1.1)
    // in the scond package the tv sends the information (xml format)
    while(!socket->atEnd()){
        QByteArray data = socket->readAll();

        // check if we got information
        if(data.startsWith("<?xml") && m_expectingData){
            m_expectingData = false;

            // Answere with OK
            QTextStream textStream(socket);
            textStream.setAutoDetectUnicode(true);
            textStream << "HTTP/1.0 200 OK\r\n"
                          "Content-Type: text/html; charset=\"utf-8\"\r\n"
                          "User-Agent: UDAP/2.0 guh\r\n"
                       << "Date: " << QDateTime::currentDateTime().toString() << "\n";

            emit eventOccured(data);
        }

        // check if we got header
        if (data.startsWith("POST") && !m_expectingData) {
            m_expectingData = true;
            QStringList tokens = QString(data).split(QRegExp("[ \r\n][ \r\n]*"));
            qCDebug(dcLgSmartTv) << "event handler -> event occured" << "http://" << m_host.toString() << ":" << m_port << tokens[1];
        }
    }
}

void TvEventHandler::onDisconnected()
{
    QTcpSocket* socket = (QTcpSocket*)sender();
    qCDebug(dcLgSmartTv) << "event handler -> client disconnected" << socket->peerAddress();
    socket->deleteLater();
}

