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

#ifndef LOGSRESOURCE_H
#define LOGSRESOURCE_H

#include <QObject>
#include <QHash>

#include "jsontypes.h"
#include "restresource.h"
#include "httpreply.h"

namespace guhserver {

class HttpRequest;

class LogsResource : public RestResource
{
    Q_OBJECT
public:
    explicit LogsResource(QObject *parent = 0);

    QString name() const override;

    HttpReply *proccessRequest(const HttpRequest &request, const QStringList &urlTokens) override;

private:
    // Process method
    HttpReply *proccessGetRequest(const HttpRequest &request, const QStringList &urlTokens) override;

    // Get methods
    HttpReply *getLogEntries(const QString &filterString);


};

}

#endif // LOGSRESOURCE_H
