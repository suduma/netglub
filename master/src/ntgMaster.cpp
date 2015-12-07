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

#include "ntgMaster.h"
#include "ntgTypes.h"
#include "ntgSlaveManager.h"
#include "ntgSlaveProxy.h"
#include "ntgTransformProxy.h"
#include "ntgXmlRpcServer.h"
#include "ntgSessionManager.h"
#include "ntgTransformManager.h"

#include <QSqlDatabase>

#include <QDebug>

NtgMaster::NtgMaster(const QHostAddress & ip, quint16 port, quint16 clientPort, const QSslConfiguration & tlsConf,
                     QSqlDatabase * db, quint16 sessionTimeOut, quint16 connectionTimeOut,
                     const QString & transformsPath, const QString & entitiesPath)
  : QObject()
  , _db(db)
{
  //qsrand(time(NULL));
  Ntg::initMetatypes();

  _transformManager = new NtgTransformManager(_db, transformsPath, entitiesPath);
  _sessionAuth = new NtgSessionManager(_db, _transformManager, sessionTimeOut*1000);
  _slaveManager = new NtgSlaveManager(ip, port, tlsConf, _transformManager);
  QSslConfiguration sslConf = tlsConf;
  sslConf.setPeerVerifyMode(QSslSocket::VerifyNone);
  sslConf.setProtocol(QSsl::SslV3);
  _xmlRpcServer = new NtgXmlRpcServer(_slaveManager, _sessionAuth, ip, clientPort, sslConf, connectionTimeOut);
}


NtgMaster::~NtgMaster()
{
  delete _slaveManager;
  delete _transformManager;
  delete _sessionAuth;
  delete _xmlRpcServer;
}

