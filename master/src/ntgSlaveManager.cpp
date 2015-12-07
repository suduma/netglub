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

#include "ntgSlaveManager.h"
#include "ntgRpcSslTransport.h"
#include "ntgRpcHandler.h"
#include "ntgSlaveProxy.h"
#include "ntgLogger.h"

#include <QSslSocket>

#define TIMEOUT 5000

NtgSlaveManager::NtgSlaveManager(const QHostAddress & ip, quint16 port,
                                 const QSslConfiguration & tlsConf,
                                 NtgTransformManager * transformManager)
                                   : _transformManager(transformManager)
{
  _defaultConf = tlsConf;
  if( ! this->listen(ip, port))
    qFatal("Could not listen on : %s:%d", qPrintable(ip.toString()), port);

}


NtgSlaveProxy * NtgSlaveManager::getSlaveProxy(QString proxyName) const
{
  return _slaveInfos.value(proxyName)->proxy;
}


QList<NtgSlaveProxy*> NtgSlaveManager::getSlavesWithTransformType(const QString & transformType) const
{
  QList<NtgSlaveProxy*> slaves;
  foreach(SlaveInfo * info, _slaveInfos)
  {
    if(info->proxy->hasTransform(transformType))
      slaves.append(info->proxy);
  }
  return slaves;
}


QStringList NtgSlaveManager::availableTransforms() const
{
  QStringList list;
  foreach(SlaveInfo * sInfo, _slaveInfos)
    list.append(sInfo->proxy->availableTransforms());
  return list.toSet().toList();
}


void NtgSlaveManager::incomingConnection(int socketDescriptor)
{
  NtgRpcHandler * handler = new NtgRpcHandler();
  NtgRpcSslTransport * transport = new NtgRpcSslTransport(_defaultConf, handler);
  NtgSlaveCreds * creds = new NtgSlaveCreds();
  SlaveInfo * info = new SlaveInfo();
  NtgSlaveProxy * proxy = NULL;

  connect(transport, SIGNAL(transportClosed()), this, SLOT(_removeSlave()));

  if( ! transport->connectToSlave(socketDescriptor, creds))
  {
    qCritical("Could not establish connection !");
    goto error;
  }

  if( ! _validateSlaveCreds(creds))
  {
    NTG_LOG("SlaveManager", "slave with invalid credentials tried to connect:"
            " serial=" + creds->serial + " ip=" + creds->ip.toString());
    qCritical("Invalid credentials for slave, refused !");
    transport->refuseSlave();
    goto error;
  }

  if( ! transport->authorizeSlave())
  {
    qCritical("Could not authorize slave !");
    goto error;
  }

  handler->setTransport(transport);
  proxy = NtgSlaveProxy::makeProxy(handler, _transformManager->transformsModels());
  if( ! proxy )
  {
    qCritical("Could not create slave proxy!");
    goto error;
  }

  info->transport = transport;
  info->creds = creds;
  info->proxy = proxy;

  _slaveInfos.insert(proxy->objectName(), info);
  
  NTG_LOG("SlaveManager", "New slave connected : serial=" + creds->serial + " ip=" + creds->ip.toString());
  qDebug()  << "New slave connected";
  emit newSlave(proxy->objectName());
  return;

error:
//   delete handler;
  delete transport;
  delete creds;
  delete info;
}


bool NtgSlaveManager::_validateSlaveCreds(NtgSlaveCreds * creds)
{
  if( ! creds )
    return false;

  // do things with the creds ...

  return true;
}


void NtgSlaveManager::_removeSlave()
{
  NtgRpcSslTransport * t = qobject_cast<NtgRpcSslTransport*>(sender());
  if( ! t ) return;

  QString slaveProxyName;
  foreach(SlaveInfo * info, _slaveInfos)
  {
    if(info->transport == t)
    {
      slaveProxyName = info->proxy->objectName();
      break;
    }
  }
  if( ! slaveProxyName.isEmpty() )
  {
    SlaveInfo * info = _slaveInfos.take(slaveProxyName);
    info->transport->disconnect();
    delete info;
    emit slaveGone(slaveProxyName);
  }
}

NtgSlaveManager::SlaveInfo::SlaveInfo()
  : transport(NULL)
  , proxy(NULL)
  , creds(NULL)
{}

NtgSlaveManager::SlaveInfo::~SlaveInfo()
{
  transport->deleteLater();
  proxy->deleteLater();
  delete creds;
}


QHash<QString,NtgTransformModel> NtgSlaveManager::allSlaveTransfomsModels() const
{
  QList<NtgTransformModel> slaveTransformModelList;
  foreach(SlaveInfo * sInfo, _slaveInfos)
    slaveTransformModelList.append(sInfo->proxy->models());

  QHash<QString,NtgTransformModel> slaveTransformModelHash;
  foreach(NtgTransformModel  transform, slaveTransformModelList)
  {
    slaveTransformModelHash.insert(transform.name, transform);
  }

  return slaveTransformModelHash;
}

bool NtgSlaveManager::isAvailableTransformForUser(const QString & name)
{
  return allSlaveTransfomsModels().contains(name);
}

