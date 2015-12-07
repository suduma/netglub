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

#include "ntgSlaveProxy.h"

#include <QVariantList>
#include <QString>
#include <QHash>


#include "ntgRpcHandler.h"
#include "ntgTransformProxy.h"
#include "ntgLogger.h"

#define TM_NAME "TManager"

bool NtgSlaveProxy::_typesInited = false;
int NtgSlaveProxy::_instanceCounter = 0;


void NtgSlaveProxy::_initTypes()
{
  if(_typesInited)
    return;

  qRegisterMetaType<QList<NtgTransformModel> >();
  qRegisterMetaTypeStreamOperators<QList<NtgTransformModel> >("QList<TransformModel>");

  _typesInited = true;
}


NtgSlaveProxy::NtgSlaveProxy(NtgRpcHandler * handler)
{
  setObjectName(QString("SlaveProxy.%1").arg(_instanceCounter++));
  _initTypes();
  _handler = handler;
}


NtgSlaveProxy::~NtgSlaveProxy()
{
  delete _handler;
  qDeleteAll(_transformProxies);
}


void NtgSlaveProxy::updateModels(QHash<QString, NtgTransformModel*> masterTransformsHash)
{
  QVariant returnValue;
  QVariantList args;
  if( ! _handler->invokeMethod(TM_NAME, "models", args, &returnValue) )
  {
      qCritical("NtgSlaveProxy::updateModels : Call Failed"); // emit error() ?
    return;
  }
  QList<NtgTransformModel> modelList = returnValue.value<QList<NtgTransformModel> >();
  foreach(const NtgTransformModel & slaveTransformModel, modelList)
    if (!masterTransformsHash.contains(slaveTransformModel.name))
    {
    qWarning() << "No such slave transform announcement into database."
                << "\n\tDetail:" << slaveTransformModel.name;
    }
    else
    {
      if (!(*(masterTransformsHash.value(slaveTransformModel.name)) == slaveTransformModel))
      {
        qWarning() << "The slave transform announcement differs from master."
                   << "\n\tDetail:" << slaveTransformModel.name;
      }
      else
      {
         _models.insert(slaveTransformModel.name, slaveTransformModel);
      }
    }
}


QList<NtgTransformModel> NtgSlaveProxy::models() const
{
  return _models.values();
}


bool NtgSlaveProxy::hasTransform(QString type) const
{
  return _models.contains(type);
}


QStringList NtgSlaveProxy::availableTransforms() const
{
  QStringList list;
  foreach(const NtgTransformModel & model, _models)
    list.append(model.name);
  return list;
}


NtgTransformProxy * NtgSlaveProxy::createTransform(QString sessionId, QString transformType,bool enableCache)
{
  if( ! _models.contains(transformType))
    return NULL;

  QVariant returnValue;
  QVariantList args;
  args << transformType;
  if( ! _handler->invokeMethod(TM_NAME, "createTransform", args, &returnValue) )
  {
      qCritical("NtgSlaveProxy::createTransform  : Call Failed"); // emit error() ?
    return NULL;
  }

  QString tObjName = returnValue.toString();
  if( tObjName.isEmpty() )
  {
    qCritical("Creating transform '%s' failed",qPrintable(transformType));
    return NULL;
  }

  NTG_LOG("SlaveProxy", "new transform: name=" + tObjName + " type="+transformType);

  NtgTransformProxy * tProxy = new NtgTransformProxy(this,_handler,tObjName,_models.value(transformType),
                                                     sessionId, enableCache);
  _transformProxies.insert(tProxy->id(), tProxy);
  return tProxy;
}


bool NtgSlaveProxy::destroyTransform(NtgTransformProxy * proxy)
{
  if( ! proxy || ! _transformProxies.contains(proxy->id()))
    return false;

  NTG_LOG("SlaveProxy", "destroy transform: name=" + proxy->objectName() + " type="+proxy->name());
  _transformProxies.take(proxy->id());
  _handler->unRegisterSignalProxy(proxy,proxy->objName());
  delete proxy;
  return true;
}


QList<NtgTransformProxy *> NtgSlaveProxy::currentTransforms(QString transformType)
{
  QList<NtgTransformProxy*> proxies;
  foreach(NtgTransformProxy * p , _transformProxies)
  {
    if(p->name() == transformType)
      proxies.append(p);
  }
  return proxies;
}


NtgSlaveProxy * NtgSlaveProxy::makeProxy(NtgRpcHandler * handler, QHash<QString, NtgTransformModel*> masterTransformsHash)
{
  if( ! handler)
    return NULL;

  NtgSlaveProxy * proxy = new NtgSlaveProxy(handler);
  proxy->updateModels(masterTransformsHash);
  if( handler->failed() )
  {
    qCritical("Could not setup slave proxy: %s", qPrintable(handler->lastError()));
    delete proxy;
    return NULL;
  }
//  qDebug() << proxy->_models;
  return proxy;
}

