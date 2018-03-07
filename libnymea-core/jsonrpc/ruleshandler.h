/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stürz <simon.stuerz@guh.io>                   *
 *  Copyright (C) 2014 Michael Zanetti <michael_zanetti@gmx.net>           *
 *                                                                         *
 *  This file is part of nymea.                                            *
 *                                                                         *
 *  nymea is free software: you can redistribute it and/or modify          *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, version 2 of the License.                *
 *                                                                         *
 *  nymea is distributed in the hope that it will be useful,               *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with nymea. If not, see <http://www.gnu.org/licenses/>.          *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef RULESHANDLER_H
#define RULESHANDLER_H

#include "jsonhandler.h"

namespace nymeaserver {

class RulesHandler : public JsonHandler
{
    Q_OBJECT
public:
    explicit RulesHandler(QObject *parent = 0);

    QString name() const override;

    Q_INVOKABLE JsonReply *GetRules(const QVariantMap &params);
    Q_INVOKABLE JsonReply *GetRuleDetails(const QVariantMap &params);

    Q_INVOKABLE JsonReply *AddRule(const QVariantMap &params);
    Q_INVOKABLE JsonReply *EditRule(const QVariantMap &params);
    Q_INVOKABLE JsonReply *RemoveRule(const QVariantMap &params);
    Q_INVOKABLE JsonReply *FindRules(const QVariantMap &params);

    Q_INVOKABLE JsonReply *EnableRule(const QVariantMap &params);
    Q_INVOKABLE JsonReply *DisableRule(const QVariantMap &params);

    Q_INVOKABLE JsonReply *ExecuteActions(const QVariantMap &params);
    Q_INVOKABLE JsonReply *ExecuteExitActions(const QVariantMap &params);

signals:
    void RuleRemoved(const QVariantMap &params);
    void RuleAdded(const QVariantMap &params);
    void RuleActiveChanged(const QVariantMap &params);
    void RuleConfigurationChanged(const QVariantMap &params);

private slots:
    void ruleRemovedNotification(const RuleId &ruleId);
    void ruleAddedNotification(const Rule &rule);
    void ruleActiveChangedNotification(const Rule &rule);
    void ruleConfigurationChangedNotification(const Rule &rule);

};

}

#endif // RULESHANDLER_H
