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

#include "ntgSlave.h"
#include "ntgLogger.h"

#include "ntgTransformManager.h"
#include "ntgRpcSslTransport.h"
#include "ntgRpcHandler.h"

#include <QTimer>

NtgSlave::NtgSlave(const QString & masterLocation,
                   const QString & transformDir,
                   const QString & pluginDir,
                   const QSslConfiguration & tlsConf,
                   quint16 reconnectionTimeOut)
  : QObject()
  , _masterLocation(masterLocation)
{
  Ntg::initMetatypes();
  _reconnectionTimeOut = reconnectionTimeOut;
  _rpcHandler = new NtgRpcHandler();
  _tm = new NtgTransformManager(transformDir, pluginDir, _rpcHandler);
  _transport = new NtgRpcSslTransport(tlsConf, _rpcHandler);
  connect(_transport, SIGNAL(transportClosed()), this, SLOT(reconnectToMaster()));
  
  reconnectToMaster();
}


NtgSlave::~NtgSlave()
{
  _transport->disconnect(this);
  delete _transport;
  delete _tm;
  delete _rpcHandler;
}


void NtgSlave::reconnectToMaster()
{
  if( ! _transport->connectToMaster(_masterLocation))
  {
    qWarning("Could not connect to master. retrying in %d seconds...",_reconnectionTimeOut);
    QTimer::singleShot(_reconnectionTimeOut*1000, this, SLOT(reconnectToMaster()));
    return;
  }

  _rpcHandler->setTransport(_transport);
  NTG_LOG("Slave", "slave connected to " + _masterLocation);
}
