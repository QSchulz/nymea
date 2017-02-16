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

#ifndef COUNTDOWN_H
#define COUNTDOWN_H

#include <QObject>
#include <QTime>
#include <QTimer>

class Countdown : public QObject
{
    Q_OBJECT
public:
    explicit Countdown(const QString &name, const QTime &time, const bool &repeating, QObject *parent = 0);

    void start();
    void stop();
    void restart();

    QString name() const;
    bool running() const;
    bool repeating() const;

    QTime time() const;
    QTime currentTime() const;

private:
    QString m_name;
    QTime m_time;
    QTime m_currentTime;
    QTimer *m_timer;
    bool m_repeating;
    bool m_running;

signals:
    void countdownTimeout();
    void runningStateChanged(const bool &running);

private slots:
    void onTimeout();

};

#endif // COUNTDOWN_H
