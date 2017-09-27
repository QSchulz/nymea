/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2017 Michael Zanetti <michael.zanetti@guh.io>            *
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

#ifndef AWSCONNECTOR_H
#define AWSCONNECTOR_H

#include <QObject>
#include <QFuture>

#include "MbedTLS/MbedTLSConnection.hpp"
#include <mqtt/Client.hpp>
#include <mqtt/Common.hpp>

class AWSConnector : public QObject, public awsiotsdk::mqtt::SubscriptionHandlerContextData, public awsiotsdk::DisconnectCallbackContextData
{
    Q_OBJECT
public:
    explicit AWSConnector(QObject *parent = 0);
    ~AWSConnector();

    void connect2AWS(const QString &endpoint, const QString &clientId, const QString &caFile, const QString &clientCertFile, const QString &clientPrivKeyFile);
    void disconnectAWS();
    bool isConnected() const;

    void pairDevice(const QString &idToken, const QString &authToken, const QString &cognitoUserId);

    void sendWebRtcHandshakeMessage(const QString &sessionId, const QVariantMap &map);

signals:
    void connected();
    void devicePaired(const QString &cognritoUserId, int errorCode);
    void webRtcHandshakeMessageReceived(const QString &transactionId, const QVariantMap &data);

private slots:
    void onConnected();
    void retrievePairedDeviceInfo();
    void registerDevice();

private:
    quint16 publish(const QString &topic, const QVariantMap &message);
    void subscribe(const QStringList &topics);
    void doSubscribe(const QStringList &topcis);
    static void publishCallback(uint16_t actionId, awsiotsdk::ResponseCode rc);
    static void subscribeCallback(uint16_t actionId, awsiotsdk::ResponseCode rc);
    static awsiotsdk::ResponseCode onSubscriptionReceivedCallback(awsiotsdk::util::String topic_name, awsiotsdk::util::String payload,
                                             std::shared_ptr<SubscriptionHandlerContextData> p_app_handler_data);
    static awsiotsdk::ResponseCode onDisconnected(awsiotsdk::util::String mqtt_client_id,
                        std::shared_ptr<DisconnectCallbackContextData> p_app_handler_data);

private:
    std::shared_ptr<awsiotsdk::network::MbedTLSConnection> m_networkConnection;
    std::shared_ptr<awsiotsdk::MqttClient> m_client;

    QString m_clientId;
    QFuture<void> m_connectingFuture;
    QStringList m_subscribedTopics;

    int m_transactionId = 0;
    QHash<quint16, QString> m_pairingRequests;

    static QHash<quint16, AWSConnector*> s_requestMap;
};

#endif // AWSCONNECTOR_H