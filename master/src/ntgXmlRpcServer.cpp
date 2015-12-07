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

#include "ntgXmlRpcServer.h"
#include "ntgSlaveManager.h"
#include "ntgSlaveProxy.h"
#include "ntgTransformProxy.h"
#include "ntgSessionManager.h"
#include <QtGui/QPixmap>


NtgXmlRpcServer::NtgXmlRpcServer(NtgSlaveManager *slaveManager, NtgSessionManager * sessionAuth,
                                 const QHostAddress &ip, quint16 port, const QSslConfiguration &conf,
                                 quint16 connectionTimeOut)
  : _slaveManager(slaveManager)
  , _sessionAuth(sessionAuth)
  , _xmlRpcServer(ip,port,connectionTimeOut)
  , _methodProvider(_sessionAuth)
{
  _xmlRpcServer.setSslConfig(conf);
  _xmlRpcServer.setMethodProvider(&_methodProvider);

  _methodProvider.registerMethod("system.getCapabilities", Ntg::AUTH_NONE, this, "getCapabilities");
  _methodProvider.registerMethod("system.listMethods", Ntg::AUTH_NONE, this, "listMethods");
  _methodProvider.registerMethod("system.methodSignature", Ntg::AUTH_NONE, this, "methodSignature");
  _methodProvider.registerMethod("system.methodHelp", Ntg::AUTH_NONE, this, "methodSignature");

  _methodProvider.registerMethod("netglub.lastError", Ntg::AUTH_NONE, this, "lastError");

  _methodProvider.registerMethod("netglub.createSession", Ntg::AUTH_NONE, this, "createSession");
  _methodProvider.registerMethod("netglub.closeSession", Ntg::AUTH_USER, this, "closeSession","to close properly a session");

  _methodProvider.registerMethod("netglub.availableTransforms", Ntg::AUTH_USER, this, "availableTransforms");
  _methodProvider.registerMethod("netglub.createTransform", Ntg::AUTH_USER, this, "createTransform");
  _methodProvider.registerMethod("netglub.destroyTransform", Ntg::AUTH_USER, this, "destroyTransform");
  _methodProvider.registerMethod("netglub.execTransform", Ntg::AUTH_USER, this, "execTransform");
  _methodProvider.registerMethod("netglub.cancelTransform", Ntg::AUTH_USER, this, "cancelTransform");
  _methodProvider.registerMethod("netglub.infoOfTransform", Ntg::AUTH_USER, this, "infoOfTransform");
  _methodProvider.registerMethod("netglub.resultsOfTransform", Ntg::AUTH_USER, this, "resultsOfTransform");

  _methodProvider.registerMethod("netglub.getEntities", Ntg::AUTH_USER, this, "getEntities");
  _methodProvider.registerMethod("netglub.getUserAvailableEntities", Ntg::AUTH_USER, this, "getUserAvailableEntities");

  _methodProvider.registerMethod("netglub.createGraph", Ntg::AUTH_USER, this, "createGraph");
  _methodProvider.registerMethod("netglub.deleteGraph", Ntg::AUTH_USER, this, "deleteGraph");
  _methodProvider.registerMethod("netglub.setGraphEnableCache", Ntg::AUTH_USER, this, "setGraphEnableCache");
  _methodProvider.registerMethod("netglub.clearCache", Ntg::AUTH_USER, this, "clearCache");

}


QVariantMap NtgXmlRpcServer::getCapabilities(QString /*sessionId*/,QString /*serial*/)
{
  QVariantMap caps;

  QVariantMap introspectCap;
  introspectCap.insert("specUrl", "http://xmlrpc-c.sourceforge.net/xmlrpc-c/introspection.html");
  introspectCap.insert("specVersion", 1);
  caps.insert("introspect", introspectCap);

  return caps;
}


QVariantList NtgXmlRpcServer::listMethods(QString /*sessionId*/,QString /*serial*/)
{
  QVariantList methodList;
  foreach(const QString & name, _methodProvider.listMethods())
    methodList.append(name);
  return methodList;
}


QVariantList NtgXmlRpcServer::methodSignature(QString sessionId, QString /*serial*/ , QString method)
{
  QVariantList signatureList;
  bool skipFirst = false;
  if( method.startsWith("s_") )
  {
    method = method.mid(2);
    skipFirst = true;
  }
  QList<QList<QByteArray> > sigList = _methodProvider.methodSignature(method);
  if( sigList.isEmpty() )
  {
    _reportError(sessionId, QString("No such method \"%1\"").arg(method));
    return signatureList;
  }
  foreach(const QList<QByteArray> & typeList, sigList)
  {
    QVariantList signature;
    foreach(const QByteArray & type, typeList)
    {
      if( skipFirst )
      {
        skipFirst = false;
        continue;
      }
      if( type == "QString")
        signature.append(QString("string"));
      else if( type == "bool")
        signature.append(QString("boolean"));
      else if( type == "QDateTime")
        signature.append(QString("datetime.iso8601"));
      else if( type == "QVariantMap")
        signature.append(QString("struct"));
      else if( type == "QVariantList")
        signature.append(QString("array"));
      else if( type == "QByteArray")
        signature.append(QString("base64"));
      else
        signature.append(QString(type));
    }
    signatureList.append(QVariant(signature));
  }
  return signatureList;
}


QString NtgXmlRpcServer::methodHelp(QString /*sessionId*/, QString /*serial*/, QString method)
{
  return _methodProvider.methodHelp(method);
}


QString NtgXmlRpcServer::lastError(QString sessionId, QString /*serial*/)
{
  NtgSession * session = _sessionAuth->getSession(sessionId);
  if( ! session ) return "No such session";
  return session->lastError;
}


QString NtgXmlRpcServer::createSession(QString sessionId, QString serial)
{
  if( ! _sessionAuth->activateSession(sessionId, serial) )
  {
    _reportError(sessionId, "No user with that serial");
    closeSession(sessionId, serial);
    return "";
  }
  return sessionId;
}


void NtgXmlRpcServer::closeSession(QString sessionId, QString serial)
{
  Q_UNUSED(serial);
  _sessionAuth->closeSession(sessionId);
}


QVariantMap NtgXmlRpcServer::availableTransforms(QString sessionId, QString /*serial*/)
{
  QVariantMap modelsVariantMap;
  QHash<QString,NtgTransformModel*> availableUserTransformHash = _sessionAuth->getUserTransformsModels(sessionId);
  QHash<QString,NtgTransformModel> availableSlaveTransformHash = _slaveManager->allSlaveTransfomsModels();


  foreach(const NtgTransformModel * userTransform, availableUserTransformHash)
  {
    if (availableSlaveTransformHash.contains(userTransform->name))
    {
        if (userTransform->enable)
            modelsVariantMap.unite(transformToMap(*userTransform));
    }
    else
    {
      //TODO cette transform devra etre grisée dans l'IHM
      qDebug() << "no slave has this transform :" << userTransform->name;
    }
  }

//  foreach (const NtgTransformModel * utransform,availableSlaveTransformHash)
//  {
//    qDebug() << *utransform;
//  }

return modelsVariantMap;

}


QVariantMap NtgXmlRpcServer::getEntities(QString /*sessionId*/, QString /*serial*/)
{
  QHash<QString,NtgEntityModel *> entityModels = _sessionAuth->getAllEntitiesModels();
  QVariantMap modelsVariantMap;
  foreach(NtgEntityModel * entity, entityModels)
  {
    modelsVariantMap.unite(entityToMap(*entity));
  }
  return modelsVariantMap;

}


QVariantList NtgXmlRpcServer::getUserAvailableEntities(QString sessionId, QString /*serial*/)
{
  return QVariant(_sessionAuth->getUserEntitiesNames(sessionId)).toList();
}


int NtgXmlRpcServer::createTransform(QString sessionId, QString /*serial*/, int graphId, QString type)
{
  qDebug() << "NtgXmlRpcServer::createTransform";
  if (!_sessionAuth->isAvailableTransformForUser(sessionId, type) || !_slaveManager->isAvailableTransformForUser(type))
  {
      qDebug() << "No such transform, you should reload your session";
      _reportError(sessionId, "No such transform");
      return 0;
  }

  QList<NtgSlaveProxy*> proxies = _slaveManager->getSlavesWithTransformType(type);
  if( proxies.isEmpty() )
  {
    qDebug() << "No slave with that transform";
    _reportError(sessionId, "No slave with that transform");
    return 0;
  }

  NtgSlaveProxy * proxy = proxies.at(qrand() / (double)RAND_MAX * proxies.size());
  NtgUser * user = _sessionAuth->getUser(_sessionAuth->getSession(sessionId)->userName);
  bool enableCache = user->getGrahEnableCache(graphId);

  NtgSession * session = _sessionAuth->getSession(sessionId);

  if (user->reachedLimitTransformsForCommunity(session->ip.toString()))
  {
      qDebug() << "The user " << user->userName() << "reached the limit number of transforms for this day";
      _reportError(sessionId, "You have reached the limit number (50) of transforms for today !");
      return 0;
  }


  NtgTransformProxy * transform = proxy->createTransform(sessionId, type, enableCache);
  if ( ! transform )
  {
    qDebug() << "The transform creation failed";
    _reportError(sessionId, "The transform creation failed");
    return 0;
  }


  if ( ! user->hasGraph(graphId) && ! user->addGraphId(graphId))
  {
    qDebug() << "no graph with this Id";
    _reportError(sessionId, "no graph with this Id");
    return 0;
  }



  user->addTransformToGraph(graphId,transform->id());
  return transform->id();

}

bool NtgXmlRpcServer::destroyTransform(QString sessionId, QString /*serial*/, int graphId, int transformId)
{
  qDebug() << "NtgXmlRpcServer::destroyTransform";
  Q_UNUSED(graphId);
  NtgTransformProxy * tProxy = NtgTransformProxy::getTransform(transformId);
  if( ! tProxy )
  {
    _reportError(sessionId, "No such transform");
    return false;
  }

  NtgUser * user = _sessionAuth->getUser(_sessionAuth->getSession(sessionId)->userName);

  tProxy->slaveProxy()->destroyTransform(tProxy);

  return user->removeTransform(graphId,transformId);
}

bool NtgXmlRpcServer::execTransform(QString sessionId, QString /*serial*/, int graphId, int transformId,
                                    QVariantMap inputEntity, QVariantMap inputParams)
{
  qDebug() << "NtgXmlRpcServer::execTransform";
  Q_UNUSED(graphId);
  NtgEntity entity = _xmlRpcStructToEntity(inputEntity);
  QHash<QString,QString> params = _xmlRpcStructToStringHash(inputParams);
  NtgTransformProxy * tProxy = NtgTransformProxy::getTransform(transformId);
  if( ! tProxy )
  {
    _reportError(sessionId, "No such transform");
    return false;
  }
  tProxy->exec(entity, params);
  qDebug() << "NtgXmlRpcServer::execTransform ending";
  return true;
}

bool NtgXmlRpcServer::cancelTransform(QString sessionId, QString /*serial*/, int graphId, int transformId)
{
  qDebug() << "NtgXmlRpcServer::cancelTransform";
  Q_UNUSED(graphId);
  NtgTransformProxy * tProxy = NtgTransformProxy::getTransform(transformId);
  if( ! tProxy )
  {
    _reportError(sessionId, "No transform with that id");
    return false;
  }
  tProxy->cancel();
  tProxy->slaveProxy()->destroyTransform(tProxy);
  return true;
}

QVariantMap NtgXmlRpcServer::infoOfTransform(QString sessionId, QString /*serial*/, int graphId, int transformId)
{
  qDebug() << "NtgXmlRpcServer::infoOfTransform";
  Q_UNUSED(graphId);
  QVariantMap infos;
  NtgTransformProxy * tProxy = NtgTransformProxy::getTransform(transformId);
  if( ! tProxy )
  {
    _reportError(sessionId, "No transform with that id");
    return infos;
  }

  Ntg::TransformStatus status = tProxy->status();
  infos.insert("status", Ntg::transformStatusName(status));
  if( status == Ntg::Failed )
  {
    infos.insert("error", Ntg::transformErrorName(tProxy->error()));
    infos.insert("error msg", tProxy->errorString());
  }
  infos.insert("latest result index", tProxy->latestResultIndex());
  infos.insert("progress value", tProxy->progressValue());
  infos.insert("progress msg", tProxy->progressText());
  qDebug() << "NtgXmlRpcServer::infoOfTransform end";
  return infos;
}

QVariantList NtgXmlRpcServer::resultsOfTransform(QString sessionId, QString /*serial*/, int graphId,
                                                 int transformId, int startIndex, int toIndex)
{
  qDebug() << "NtgXmlRpcServer::resultsOfTransform";
  Q_UNUSED(graphId)
  QVariantList results;
  NtgTransformProxy * tProxy = NtgTransformProxy::getTransform(transformId);
  if( ! tProxy )
  {
    _reportError(sessionId, "No transform with that id");
    qDebug() << "NtgXmlRpcServer::resultsOfTransform ending 1";
    return results;
  }

  foreach(const NtgEntity & entity, tProxy->results(startIndex, toIndex))
    results.append(_entityToXmlRpcStruct(entity));
  qDebug() << "NtgXmlRpcServer::resultsOfTransform ending";
  return results;
}


int NtgXmlRpcServer::createGraph(QString sessionId, QString /*serial*/)
{
  NtgUser * user = _sessionAuth->getUser(_sessionAuth->getSession(sessionId)->userName);
  return user->createGraph();

}

bool NtgXmlRpcServer::deleteGraph(QString sessionId, QString /*serial*/, int graphId)
{
  NtgUser * user = _sessionAuth->getUser(_sessionAuth->getSession(sessionId)->userName);
  return user->deleteGraph(graphId);
}

void NtgXmlRpcServer::setGraphEnableCache(QString sessionId, QString serial, int graphId, bool cacheEnable)
{
    Q_UNUSED(serial);
    NtgUser * user = _sessionAuth->getUser(_sessionAuth->getSession(sessionId)->userName);
    user->setGrahEnableCache(graphId,cacheEnable);
}

void NtgXmlRpcServer::clearCache(QString sessionId, QString serial)
{
    Q_UNUSED(sessionId);
    Q_UNUSED(serial);
    NtgTransformCache::getOrCreate()->clearAll();
}

QHash<QString,QString> NtgXmlRpcServer::_xmlRpcStructToStringHash(const QVariantMap & structMap)
{
  QHash<QString,QString> hash;
  foreach(const QString & key, structMap.keys())
    hash.insert(key, structMap.value(key).toString());
  return hash;
}



NtgEntity NtgXmlRpcServer::_xmlRpcStructToEntity(QVariantMap structMap)
{
  NtgEntity entity;
  entity.type = structMap.take("entity_type").toString();
  entity.values = _xmlRpcStructToStringHash(structMap);
  return entity;
}


QVariantMap NtgXmlRpcServer::_stringHashToXmlRpcStruct(const QHash<QString,QString> & hash)
{
  QVariantMap structMap;
  foreach(const QString & key, hash.keys())
    structMap.insert(key, hash.value(key));
  return structMap;
}


QVariantMap NtgXmlRpcServer::_entityToXmlRpcStruct(const NtgEntity & entity)
{
  QVariantMap structMap;
  structMap.insert("entity_type", entity.type);
  structMap.unite(_stringHashToXmlRpcStruct(entity.values));
  return structMap;
}


void NtgXmlRpcServer::_reportError(QString sessionId, QString error)
{
  NtgSession * session = _sessionAuth->getSession(sessionId);
  if( ! session ) return;
  session->lastError = error;
}
