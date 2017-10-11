/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2014 Michael Zanetti <michael_zanetti@gmx.net>           *
 *  Copyright (C) 2015-2017 Simon Stürz <simon.stuerz@guh.io>              *
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

/*!
    \class guhserver::LogEngine
    \brief  The engine which creates the log databse and provides access to it.

    \ingroup logs
    \inmodule core

    The \l{LogEngine} creates a \l{https://sqlite.org/}{SQLite3} database to stores everything what's
    happening in the system. The database can be accessed from the API's. To controll the size of the database the
    limit of the databse are 8000 entries.


    \sa LogEntry, LogFilter, LogsResource, LoggingHandler
*/

/*! \fn void guhserver::LogEngine::logEntryAdded(const LogEntry &logEntry);
    This signal is emitted when an \a logEntry was added to the database.

    \sa LogEntry
*/

/*! \fn void guhserver::LogEngine::logDatabaseUpdated();
    This signal is emitted when the log database was updated. The log database
    will be updated when a \l{LogEntry} was added or when a device was removed
    and all corresponding \l{LogEntry}{LogEntries} were removed from the database.
*/

/*!
    \class guhserver::Logging
    \brief  The logging class provides enums and flags for the LogEngine.

    \ingroup logs
    \inmodule core

    \sa LogEngine, LogEntry, LogFilter
*/

/*! \fn guhserver::Logging::Logging(QObject *parent)
    Constructs the \l{Logging} object with the given \a parent.
*/

/*! \enum guhserver::Logging::LoggingError
    Represents the possible errors from the \l{LogEngine}.

    \value LoggingErrorNoError
        No error happened. Everything is fine.
    \value LoggingErrorLogEntryNotFound
        The requested \l{LogEntry} could not be found.
    \value LoggingErrorInvalidFilterParameter
        The given \l{LogFilter} contains an invalid paramter.
*/

/*! \enum guhserver::Logging::LoggingEventType
    Represents the event type of this \l{LogEntry}.

    \value LoggingEventTypeTrigger
        This event type describes an \l{Event} which has triggered.
    \value LoggingEventTypeActiveChange
        This event type describes a \l{Rule} which has changed its active status.
    \value LoggingEventTypeActionsExecuted
        This event type describes the actions execution of a \l{Rule}.
    \value LoggingEventTypeExitActionsExecuted
        This event type describes the  exit actions execution of a \l{Rule}.
    \value LoggingEventTypeEnabledChange

*/

/*! \enum guhserver::Logging::LoggingLevel
    Indicates if the corresponding \l{LogEntry} is an information or an alert.

    \value LoggingLevelInfo
        This \l{LogEntry} represents an information.
    \value LoggingLevelAlert
        This \l{LogEntry} represents an alert. Something is not ok.
*/

/*! \enum guhserver::Logging::LoggingSource
    Indicates from where the \l{LogEntry} was created. Can be used as flag.

    \value LoggingSourceSystem
        This \l{LogEntry} was created from the guh system (server).
    \value LoggingSourceEvents
        This \l{LogEntry} was created from an \l{Event} which trigged.
    \value LoggingSourceActions
        This \l{LogEntry} was created from an \l{Action} which was executed.
    \value LoggingSourceStates
        This \l{LogEntry} was created from an \l{State} which hase changed.
    \value LoggingSourceRules
        This \l{LogEntry} represents the enable/disable event from an \l{Rule}.
*/

#include "guhsettings.h"
#include "logengine.h"
#include "loggingcategories.h"
#include "logging.h"
#include "logvaluetool.h"

#include <QCoreApplication>
#include <QSqlDatabase>
#include <QSqlDriver>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlError>
#include <QMetaEnum>
#include <QDateTime>
#include <QFileInfo>
#include <QTime>

#define DB_SCHEMA_VERSION 3

namespace guhserver {

/*! Constructs the log engine with the given \a parent. */
LogEngine::LogEngine(QObject *parent):
    QObject(parent)
{
    m_db = QSqlDatabase::addDatabase("QSQLITE", "logs");
    m_db.setDatabaseName(GuhSettings::logPath());
    m_dbMaxSize = 50000;
    m_overflow = 100;

    if (QCoreApplication::instance()->organizationName() == "guh-test") {
        m_dbMaxSize = 20;
        qCDebug(dcLogEngine) << "Set logging dab max size to" << m_dbMaxSize << "for testing.";
    }

    qCDebug(dcLogEngine) << "Opening logging database" << m_db.databaseName();

    if (!m_db.isValid()) {
        qCWarning(dcLogEngine) << "Database not valid:" << m_db.lastError().driverText() << m_db.lastError().databaseText();
        rotate(m_db.databaseName());
    }
    if (!m_db.open()) {
        qCWarning(dcLogEngine) << "Error opening log database:" << m_db.lastError().driverText() << m_db.lastError().databaseText();
        rotate(m_db.databaseName());
    }

    if (!initDB()) {
        qCWarning(dcLogEngine()) << "Error initializing database. Trying to correct it.";
        rotate(m_db.databaseName());
        if (!initDB()) {
            qCWarning(dcLogEngine()) << "Error fixing log database. Giving up. Logs can't be stored.";
        }
    }

    connect(&m_housekeepingTimer, &QTimer::timeout, this, &LogEngine::checkDBSize);
    m_housekeepingTimer.setInterval(1); // Trigger on next idle event loop run
    m_housekeepingTimer.setSingleShot(true);

    checkDBSize();
}

/*! Destructs the \l{LogEngine}. */
LogEngine::~LogEngine()
{
    qCDebug(dcApplication) << "Shutting down \"Log Engine\"";
    m_db.close();
}

/*! Returns the list of \l{LogEntry}{LogEntries} of the database matching the given \a filter.

  \sa LogEntry, LogFilter
*/
QList<LogEntry> LogEngine::logEntries(const LogFilter &filter) const
{
    qCDebug(dcLogEngine) << "Read logging database" << m_db.databaseName();

    QList<LogEntry> results;
    QSqlQuery query;

    QString queryCall = "SELECT * FROM entries ORDER BY timestamp;";
    if (filter.isEmpty()) {
        query = m_db.exec(queryCall);
    } else {
        queryCall = QString("SELECT * FROM entries WHERE %1 ORDER BY timestamp;").arg(filter.queryString());
        query = m_db.exec(queryCall);
    }

    if (m_db.lastError().isValid()) {
        qCWarning(dcLogEngine) << "Error fetching log entries. Driver error:" << m_db.lastError().driverText() << "Database error:" << m_db.lastError().databaseText();
        return QList<LogEntry>();
    }

    while (query.next()) {
        LogEntry entry(
                    QDateTime::fromTime_t(query.value("timestamp").toLongLong()),
                    (Logging::LoggingLevel)query.value("loggingLevel").toInt(),
                    (Logging::LoggingSource)query.value("sourceType").toInt(),
                    query.value("errorCode").toInt());
        entry.setTypeId(query.value("typeId").toUuid());
        entry.setDeviceId(DeviceId(query.value("deviceId").toString()));
        entry.setValue(LogValueTool::convertVariantToString(LogValueTool::deserializeValue(query.value("value").toString())));
        entry.setEventType((Logging::LoggingEventType)query.value("loggingEventType").toInt());
        entry.setActive(query.value("active").toBool());
        results.append(entry);
    }
    qCDebug(dcLogEngine) << "Fetched" << results.count() << "entries for db query:" << queryCall;

    return results;
}

void LogEngine::setMaxLogEntries(int maxLogEntries, int overflow)
{
    m_dbMaxSize = maxLogEntries;
    m_overflow = overflow;
    checkDBSize();
}

/*! Removes all entries from the database. This method will be used for the tests. */
void LogEngine::clearDatabase()
{
    qCWarning(dcLogEngine) << "Clear logging database.";

    QString queryDeleteString = QString("DELETE FROM entries;");
    if (m_db.exec(queryDeleteString).lastError().type() != QSqlError::NoError) {
        qCWarning(dcLogEngine) << "Could not clear logging database. Driver error:" << m_db.lastError().driverText() << "Database error:" << m_db.lastError().databaseText();
    }

    emit logDatabaseUpdated();
}

void LogEngine::logSystemEvent(const QDateTime &dateTime, bool active, Logging::LoggingLevel level)
{
    LogEntry entry(dateTime, level, Logging::LoggingSourceSystem);
    entry.setEventType(Logging::LoggingEventTypeActiveChange);
    entry.setActive(active);
    appendLogEntry(entry);
}

void LogEngine::logEvent(const Event &event)
{
    QVariantList valueList;
    Logging::LoggingSource sourceType;
    if (event.isStateChangeEvent()) {
        sourceType = Logging::LoggingSourceStates;
        // There should only be one param
        if (!event.params().isEmpty())
            valueList << event.params().first().value();

    } else {
        sourceType = Logging::LoggingSourceEvents;
        foreach (const Param &param, event.params()) {
            valueList << param.value();
        }
    }

    LogEntry entry(sourceType);
    entry.setTypeId(event.eventTypeId());
    entry.setDeviceId(event.deviceId());
    if (valueList.count() == 1) {
        entry.setValue(valueList.first());
    } else {
        entry.setValue(valueList);
    }
    appendLogEntry(entry);
}

void LogEngine::logAction(const Action &action, Logging::LoggingLevel level, int errorCode)
{
    LogEntry entry(level, Logging::LoggingSourceActions, errorCode);
    entry.setTypeId(action.actionTypeId());
    entry.setDeviceId(action.deviceId());

    if (action.params().isEmpty()) {
        entry.setValue(QVariant());
    } else if (action.params().count() == 1) {
        entry.setValue(action.params().first().value());
    } else {
        QVariantList valueList;
        foreach (const Param &param, action.params()) {
            valueList << param.value();
        }
        entry.setValue(valueList);
    }
    appendLogEntry(entry);
}

void LogEngine::logRuleTriggered(const Rule &rule)
{
    LogEntry entry(Logging::LoggingSourceRules);
    entry.setTypeId(rule.id());
    entry.setEventType(Logging::LoggingEventTypeTrigger);
    appendLogEntry(entry);
}

void LogEngine::logRuleActiveChanged(const Rule &rule)
{
    LogEntry entry(Logging::LoggingSourceRules);
    entry.setTypeId(rule.id());
    entry.setActive(rule.active());
    entry.setEventType(Logging::LoggingEventTypeActiveChange);
    appendLogEntry(entry);
}

void LogEngine::logRuleEnabledChanged(const Rule &rule, const bool &enabled)
{
    LogEntry entry(Logging::LoggingSourceRules);
    entry.setTypeId(rule.id());
    entry.setEventType(Logging::LoggingEventTypeEnabledChange);
    entry.setActive(enabled);
    appendLogEntry(entry);
}

void LogEngine::logRuleActionsExecuted(const Rule &rule)
{
    LogEntry entry(Logging::LoggingSourceRules);
    entry.setTypeId(rule.id());
    entry.setEventType(Logging::LoggingEventTypeActionsExecuted);
    appendLogEntry(entry);
}

void LogEngine::logRuleExitActionsExecuted(const Rule &rule)
{
    LogEntry entry(Logging::LoggingSourceRules);
    entry.setTypeId(rule.id());
    entry.setEventType(Logging::LoggingEventTypeExitActionsExecuted);
    appendLogEntry(entry);
}

void LogEngine::removeDeviceLogs(const DeviceId &deviceId)
{
    qCDebug(dcLogEngine) << "Deleting log entries from device" << deviceId.toString();

    QString queryDeleteString = QString("DELETE FROM entries WHERE deviceId = '%1';").arg(deviceId.toString());
    if (m_db.exec(queryDeleteString).lastError().type() != QSqlError::NoError) {
        qCWarning(dcLogEngine) << "Error deleting log entries from device" << deviceId.toString() << ". Driver error:" << m_db.lastError().driverText() << "Database error:" << m_db.lastError().databaseText();
    } else {
        emit logDatabaseUpdated();
    }
}

void LogEngine::removeRuleLogs(const RuleId &ruleId)
{
    qCDebug(dcLogEngine) << "Deleting log entries from rule" << ruleId.toString();

    QString queryDeleteString = QString("DELETE FROM entries WHERE typeId = '%1';").arg(ruleId.toString());
    if (m_db.exec(queryDeleteString).lastError().type() != QSqlError::NoError) {
        qCWarning(dcLogEngine) << "Error deleting log entries from rule" << ruleId.toString() << ". Driver error:" << m_db.lastError().driverText() << "Database error:" << m_db.lastError().databaseText();
    } else {
        emit logDatabaseUpdated();
    }
}

QList<DeviceId> LogEngine::devicesInLogs() const
{
    QString queryString = QString("SELECT deviceId FROM entries WHERE deviceId != \"%1\" GROUP BY deviceId;").arg(QUuid().toString());
    QSqlQuery result = m_db.exec(queryString);
    QList<DeviceId> ret;
    if (result.lastError().type() != QSqlError::NoError) {
        qCWarning(dcLogEngine()) << "Error fetching device entries from log database:" << m_db.lastError().driverText() << m_db.lastError().databaseText();
        return ret;
    }
    if (!result.first()) {
        return ret;
    }
    do {
        ret.append(DeviceId::fromUuid(result.value("deviceId").toUuid()));
    } while (result.next());
    return ret;
}

void LogEngine::appendLogEntry(const LogEntry &entry)
{
    QString queryString = QString("INSERT INTO entries (timestamp, loggingEventType, loggingLevel, sourceType, typeId, deviceId, value, active, errorCode) values ('%1', '%2', '%3', '%4', '%5', '%6', '%7', '%8', '%9');")
            .arg(entry.timestamp().toTime_t())
            .arg(entry.eventType())
            .arg(entry.level())
            .arg(entry.source())
            .arg(entry.typeId().toString())
            .arg(entry.deviceId().toString())
            .arg(LogValueTool::serializeValue(entry.value()))
            .arg(entry.active())
            .arg(entry.errorCode());

    if (m_db.exec(queryString).lastError().type() != QSqlError::NoError) {
        qCWarning(dcLogEngine) << "Error writing log entry. Driver error:" << m_db.lastError().driverText() << "Database error:" << m_db.lastError().databaseText();
        qCWarning(dcLogEngine) << entry;
        return;
    }

    emit logEntryAdded(entry);

    if (++m_entryCount > m_dbMaxSize + m_overflow) {
        m_housekeepingTimer.start();
    }
}

void LogEngine::checkDBSize()
{
    QDateTime startTime = QDateTime::currentDateTime();
    QString queryString = "SELECT COUNT(*) FROM entries;";
    QSqlQuery result = m_db.exec(queryString);
    if (m_db.lastError().type() != QSqlError::NoError) {
        qWarning(dcLogEngine()) << "Failed to query entry count in db:" << m_db.lastError().databaseText();
        return;
    }
    if (!result.first()) {
        qWarning(dcLogEngine()) << "Failed retrieving entry count.";
        return;
    }
    m_entryCount = result.value(0).toInt();

    if (m_entryCount >= m_dbMaxSize) {
        // keep only the latest m_dbMaxSize entries
        if (!m_trimWarningPrinted) {
            qCDebug(dcLogEngine) << "Deleting oldest entries" << (m_entryCount - m_dbMaxSize) << "and keep only the latest" << m_dbMaxSize << "entries.";
            m_trimWarningPrinted = true;
        }
        QString queryDeleteString = QString("DELETE FROM entries WHERE ROWID IN (SELECT ROWID FROM entries ORDER BY timestamp DESC LIMIT -1 OFFSET %1);").arg(QString::number(m_dbMaxSize));
        if (m_db.exec(queryDeleteString).lastError().type() != QSqlError::NoError) {
            qCWarning(dcLogEngine) << "Error deleting oldest log entries to keep size. Driver error:" << m_db.lastError().driverText() << "Database error:" << m_db.lastError().databaseText();
        }
        m_entryCount = m_dbMaxSize;
        emit logDatabaseUpdated();
    }
    qCDebug(dcLogEngine()) << "Ran housekeeping on log database in" << startTime.msecsTo(QDateTime::currentDateTime()) << "ms.";
}

void LogEngine::rotate(const QString &dbName)
{
    int index = 1;
    while (QFileInfo(QString("%1.%2").arg(dbName).arg(index)).exists()) {
        index++;
    }
    qCDebug(dcLogEngine()) << "Backing up old database file to" << QString("%1.%2").arg(dbName).arg(index);
    QFile f(dbName);
    if (!f.rename(QString("%1.%2").arg(dbName).arg(index))) {
        qCWarning(dcLogEngine()) << "Error backing up old database.";
    } else {
        qCDebug(dcLogEngine()) << "Successfully moved old database";
    }
}

bool LogEngine::migrateDatabaseVersion2to3()
{
    // Changelog: serialize values of logentries in order to prevent typecast errors
    qCDebug(dcLogEngine()) << "Start migration of log database from version 2 to version 3";

    QDateTime startTime = QDateTime::currentDateTime();

    int migrationCounter = 0;
    int migrationProgress = 0;
    int entryCount = 0;

    // Count entries we have to migrate
    QString queryString = "SELECT COUNT(*) FROM entries WHERE value != '';";
    QSqlQuery countQuery = m_db.exec(queryString);
    if (m_db.lastError().type() != QSqlError::NoError) {
        qCWarning(dcLogEngine()) << "Failed to query entry count in db:" << m_db.lastError().databaseText();
        return false;
    }
    if (!countQuery.first()) {
        qCWarning(dcLogEngine()) << "Migration: Failed retrieving entry count.";
        return false;
    }
    entryCount = countQuery.value(0).toInt();

    qCDebug(dcLogEngine()) << "Entries to migrate:" << entryCount;

    // Select all entries
    QSqlQuery selectQuery = m_db.exec("SELECT * FROM entries;");
    if (m_db.lastError().isValid()) {
        qCWarning(dcLogEngine) << "Error migrating database verion 2 -> 3. Driver error:" << m_db.lastError().driverText() << "Database error:" << m_db.lastError().databaseText();
        return false;
    }

    // Migrate all selected entries
    while (selectQuery.next()) {
        QString oldValue = selectQuery.value("value").toString();
        QString newValue = LogValueTool::serializeValue(QVariant(oldValue));
        if (oldValue.isEmpty())
            continue;

        QString updateCall = QString("UPDATE entries SET value = '%1' WHERE timestamp = '%2' AND loggingLevel = '%3' AND sourceType = '%4' AND errorCode = '%5' AND typeId = '%6' AND deviceId = '%7' AND value = '%8' AND loggingEventType = '%9'AND active = '%10';")
                .arg(newValue)
                .arg(selectQuery.value("timestamp").toLongLong())
                .arg(selectQuery.value("loggingLevel").toInt())
                .arg(selectQuery.value("sourceType").toInt())
                .arg(selectQuery.value("errorCode").toInt())
                .arg(selectQuery.value("typeId").toUuid().toString())
                .arg(selectQuery.value("deviceId").toString())
                .arg(selectQuery.value("value").toString())
                .arg(selectQuery.value("loggingEventType").toInt())
                .arg(selectQuery.value("active").toBool());

        m_db.exec(updateCall);
        if (m_db.lastError().isValid()) {
            qCWarning(dcLogEngine) << "Error migrating database verion 2 -> 3. Driver error:" << m_db.lastError().driverText() << "Database error:" << m_db.lastError().databaseText();
            return false;
        }

        migrationCounter++;

        double percentage = migrationCounter * 100.0 / entryCount;
        if (qRound(percentage) != migrationProgress) {
            migrationProgress = qRound(percentage);
            qCDebug(dcLogEngine()) << QString("Migration progress: %1\%").arg(migrationProgress).toLatin1().data();
        }
    }

    QTime runTime = QTime(0,0,0,0).addMSecs(startTime.msecsTo(QDateTime::currentDateTime()));
    qCDebug(dcLogEngine()) << "Migration of" << migrationCounter << "done in" << runTime.toString("mm:ss.zzz");
    qCDebug(dcLogEngine()) << "Updating database version to" << DB_SCHEMA_VERSION;
    m_db.exec(QString("UPDATE metadata SET data = %1 WHERE key = 'version';").arg(DB_SCHEMA_VERSION));
    if (m_db.lastError().isValid()) {
        qCWarning(dcLogEngine) << "Error updating database verion 2 -> 3. Driver error:" << m_db.lastError().driverText() << "Database error:" << m_db.lastError().databaseText();
        return false;
    }

    qCDebug(dcLogEngine()) << "Migrated" << migrationCounter << "entries from database verion 2 -> 3 successfully.";
    return true;
}

bool LogEngine::initDB()
{
    m_db.close();
    m_db.open();

    if (!m_db.tables().contains("metadata")) {
        m_db.exec("CREATE TABLE metadata (key varchar(10), data varchar(40));");
        m_db.exec(QString("INSERT INTO metadata (key, data) VALUES('version', '%1');").arg(DB_SCHEMA_VERSION));
    }

    QSqlQuery query = m_db.exec("SELECT data FROM metadata WHERE key = 'version';");
    if (query.next()) {
        int version = query.value("data").toInt();
        if (DB_SCHEMA_VERSION == 3 && version == 2) {
            if (!migrateDatabaseVersion2to3()) {
                qCWarning(dcLogEngine()) << "Migration process failed.";
                return false;
            }
        }

        if (version != DB_SCHEMA_VERSION) {
            qCWarning(dcLogEngine) << "Log schema version not matching! Schema upgrade not implemented yet. Logging might fail.";
        } else {
            qCDebug(dcLogEngine) << QString("Log database schema version \"%1\" matches").arg(DB_SCHEMA_VERSION);
        }
    } else {
        qCWarning(dcLogEngine) << "Broken log database. Version not found in metadata table.";
        return false;
    }

    if (!m_db.tables().contains("sourceTypes")) {
        m_db.exec("CREATE TABLE sourceTypes (id int, name varchar(20), PRIMARY KEY(id));");
        //qCDebug(dcLogEngine) << m_db.lastError().databaseText();
        QMetaEnum logTypes = Logging::staticMetaObject.enumerator(Logging::staticMetaObject.indexOfEnumerator("LoggingSource"));
        Q_ASSERT_X(logTypes.isValid(), "LogEngine", "Logging has no enum LoggingSource");
        for (int i = 0; i < logTypes.keyCount(); i++) {
            m_db.exec(QString("INSERT INTO sourceTypes (id, name) VALUES(%1, '%2');").arg(i).arg(logTypes.key(i)));
        }
    }

    if (!m_db.tables().contains("loggingEventTypes")) {
        m_db.exec("CREATE TABLE loggingEventTypes (id int, name varchar(20), PRIMARY KEY(id));");
        //qCDebug(dcLogEngine) << m_db.lastError().databaseText();
        QMetaEnum logTypes = Logging::staticMetaObject.enumerator(Logging::staticMetaObject.indexOfEnumerator("LoggingEventType"));
        Q_ASSERT_X(logTypes.isValid(), "LogEngine", "Logging has no enum LoggingEventType");
        for (int i = 0; i < logTypes.keyCount(); i++) {
            m_db.exec(QString("INSERT INTO loggingEventTypes (id, name) VALUES(%1, '%2');").arg(i).arg(logTypes.key(i)));
        }
    }

    if (!m_db.tables().contains("entries")) {
        m_db.exec("CREATE TABLE entries "
                  "("
                  "timestamp int,"
                  "loggingLevel int,"
                  "sourceType int,"
                  "typeId varchar(38),"
                  "deviceId varchar(38),"
                  "value varchar(100),"
                  "loggingEventType int,"
                  "active bool,"
                  "errorCode int,"
                  "FOREIGN KEY(sourceType) REFERENCES sourceTypes(id),"
                  "FOREIGN KEY(loggingEventType) REFERENCES loggingEventTypes(id)"
                  ");");

        if (m_db.lastError().isValid()) {
            qCWarning(dcLogEngine) << "Error creating log table in database. Driver error:" << m_db.lastError().driverText() << "Database error:" << m_db.lastError().databaseText();
            return false;
        }


    }

    qCDebug(dcLogEngine) << "Initialized logging DB successfully. (maximum DB size:" << m_dbMaxSize << ")";
    return true;
}

}
