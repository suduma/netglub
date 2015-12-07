/******************************************************************************
**
** Copyright (C) 2009-2010 DIATEAM. All rights reserved.
**
** This file is part of the "NetGlub" program.
**
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program.  If not, see <http://www.gnu.org/licenses/>.
**
******************************************************************************/

#ifndef NTGDAEMON_H
#define NTGDAEMON_H

#include <sys/types.h>
#include <QCoreApplication>
#include <QSocketNotifier>
#include <QFile>

class NtgDaemon : public QCoreApplication
{
  Q_OBJECT
  public:
    enum ExitStatus
    {
      SUCCESSFUL = 0,
      FAILED = 1,
      STOPPED = 127
    };

    NtgDaemon(int& argc, char **& argv);
    virtual ~NtgDaemon();

    void daemonize();
    void execCommand(QString cmd);

    void setForeground(bool f);
    bool setPidfile(QString path);
    bool setLogfile(QString path);
    bool setNonRootUser(QString name);

    bool dropPrivileges();
    bool dropPrivilegesPermanently();
    bool regainPrivileges();

  signals:
    void reload(); // SIGHUP on Unix

  protected slots:
    void _handleSighup();

  protected:
    static void _hupSignalHandler(int sig);
    static void _otherSignalHandler(int sig);
    static void _qtMessageHandler(QtMsgType type, const char * msg);

    static int _sighupFd[2];
    static bool _sigHandlerOnce;
    static QFile * _logfile;
    static bool _logToConsole;

    void _cleanup();
    void _parseArgs(int argc, char ** argv);

    uid_t _userId;
    QSocketNotifier * _socketNotifier;
    QString _pidfilePath;
    QFile * _pidfile;
    bool _foreground;
    QString _command;
};

#endif // NTGDAEMON_H
