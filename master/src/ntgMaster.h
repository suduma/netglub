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

#ifndef NTGMASTER_H
#define NTGMASTER_H

#include <QObject>
#include <QSslConfiguration>
#include "ntgTypes.h"

class NtgSlaveManager;
class NtgTransformManager;
class NtgSessionManager;
class NtgXmlRpcServer;
class QSqlDatabase;

class NtgMaster : public QObject
{
  Q_OBJECT
  public:
    NtgMaster(const QHostAddress & ip, quint16 port, quint16 clientPort, const QSslConfiguration & tlsConf,
               QSqlDatabase * db, quint16 sessionTimeOut, quint16 connectionTimeOut,
               const QString & transformsPath, const QString & entitiesPath);
    ~NtgMaster();

  protected:
    NtgSlaveManager * _slaveManager;
    NtgTransformManager * _transformManager;
    NtgSessionManager * _sessionAuth;
    NtgXmlRpcServer * _xmlRpcServer;
    QSqlDatabase * _db;
};

#endif // NTGMASTER_H
