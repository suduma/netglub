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

#include "callermanager.h"
#include "mainwindow.h"


CallerManager * CallerManager::_callerManager = NULL;

CallerManager::CallerManager(const QString & URL, const QString & serial)
{
    _failed = new FailedConectionDialog("Failed to connect to master !");
    reloadConnexion(URL,serial);
}



CallerManager::~CallerManager()
{
  if (_rpc)
    delete _rpc;
  if (_caller)
    delete _caller;
  delete _failed;
}



CallerManager * CallerManager::create(const QString & URL, const QString & serial)
{
    _callerManager = new CallerManager(URL, serial);
    return _callerManager;
}


QString CallerManager::connectToMaster()
{
    _connectionError.clear();
    try
    {
        _sessionId = _caller->call("netglub.createSession", _serial).toString();
        if( _sessionId.isEmpty())
        {
            _connectionError = "Serial not valid !";
        }
    }
    catch(NtgXmlRpcCall * failedCaller)
    {
        _connectionError = "Failed to connect: [" + QString::number(failedCaller->error(),10)+
                          "] " +  failedCaller->errorString() + " (Compromised URL)";
    }
    return _connectionError;
}


void CallerManager::reloadConnexion(const QString & URL, const QString & serial)
{
    _rpc = new MaiaXmlRpcClient(QUrl(URL));
    _caller = new NtgXmlRpcCall(_rpc, true);
    _serial = serial;
    _URL = URL;
    connectToMaster();
    _offLineWork = false;
    //TODO Optimize Local transforms
    //LocalTransformManager disabled
    //_localTransformManager = new LocalTransformManager();
}

bool CallerManager::retryConnexion()
{
    reloadConnexion(_URL,_serial);
    return _connectionError.isEmpty();

}

CallerManager * CallerManager::get()
{
    return _callerManager;
}

QString CallerManager::getConnectionError()
{
    return _connectionError;
}

bool CallerManager::isOffLineWork()
{
    return _offLineWork;
}

void CallerManager::setOffLineWork(bool offLineWork)
{
    _offLineWork = offLineWork;
}


QHash<QString,NtgEntityModel> CallerManager::getAllEntities()
{
    QString _connectionError;
    try
    {
        if (!_offLineWork)
            return mapToEntityModelHash(_caller->call("netglub.getEntities").toMap());
    }
    catch (NtgXmlRpcCall * failedCaller)
    {
        _failed->exec();
        if (_failed->continueCall())
        {
            return getAllEntities();
        }
        else
        {
            this->deleteLater();
            qApp->exit(0);
        }
    }

    return QHash<QString,NtgEntityModel>();
}

QStringList CallerManager::getAvailableEntities()
{
    try
    {
        if (!_offLineWork)
            return _caller->call("netglub.getUserAvailableEntities").toStringList();
    }
    catch (NtgXmlRpcCall * failedCaller)
    {
        _failed->exec();
        if (_failed->continueCall())
        {
            return getAvailableEntities();
        }
        else
        {
            this->deleteLater();
            qApp->exit(0);
        }
    }
    return QStringList();
}

QHash<QString,NtgTransformModel> CallerManager::getMasterAvailableTransforms()
{
    try
    {
        if (!_offLineWork)
        {
            return mapToTransformModelHash(_caller->call("netglub.availableTransforms").toMap());
        }
    }
    catch (NtgXmlRpcCall * failedCaller)
    {
        _failed->exec();
        if (_failed->continueCall())
        {
            return getMasterAvailableTransforms();
        }
        else
        {
            this->deleteLater();
            qApp->exit(0);
        }
    }
    return QHash<QString,NtgTransformModel>();

}



//TODO : Optimize LocalTransforms
//Function disabled
//QHash<QString,NtgTransformModel> CallerManager::getLocalAvailableTransforms()
//{
//            return mapToTransformModelHash(_localTransformManager->transformList());
//}


int CallerManager::getNewGraphId()
{
    try
    {
        if (!_offLineWork)
            return _caller->call("netglub.createGraph").toInt();
        return 0;
    }
    catch (NtgXmlRpcCall * failedCaller)
    {
        _failed->exec();
        if (_failed->continueCall())
        {
            return getNewGraphId();
        }
        else
        {
            this->deleteLater();
            qApp->exit(0);
        }
    }
    return 0;
}

bool CallerManager::deleteGraph(int graphId)
{
    try
    {
        if (!_offLineWork)
            return _caller->call("netglub.deleteGraph",graphId).toInt();
        return false;
    }
    catch (NtgXmlRpcCall * failedCaller)
    {
        _failed->exec();
        if (_failed->continueCall())
        {
            return getNewGraphId();
        }
        else
        {
            this->deleteLater();
            qApp->exit(0);
        }
    }
    return false;
}

void CallerManager::setGraphEnableCache(int graphId, bool enableCache)
{
    try
    {
        if (!_offLineWork)
            _caller->call("netglub.setGraphEnableCache", graphId, enableCache);
    }
    catch (NtgXmlRpcCall * failedCaller)
    {
        _failed->exec();
        if (_failed->continueCall())
        {
            return setGraphEnableCache(graphId,enableCache);
        }
        else
        {
            this->deleteLater();
            qApp->exit(0);
        }
    }
}


void CallerManager::clearCache()
{
    try
    {
        if (!_offLineWork)
            _caller->call("netglub.clearCache");
    }
    catch (NtgXmlRpcCall * failedCaller)
    {
        _failed->exec();
        if (_failed->continueCall())
        {
            clearCache();
        }
        else
        {
            this->deleteLater();
            qApp->exit(0);
        }
    }
}

QString CallerManager::lastError()
{
    try
    {
        if (!_offLineWork)
            return _caller->call("netglub.lastError").toString();
        return QString();
    }
    catch (NtgXmlRpcCall * failedCaller)
    {
        _failed->exec();
        if (_failed->continueCall())
        {
            return lastError();
        }
        else
        {
            this->deleteLater();
            qApp->exit(0);
        }
    }
    return QString();
}


QString CallerManager::closeSession()
{
    try
    {
        if (!_offLineWork)
            return _caller->call("netglub.closeSession").toString();
        return QString();
    }
    catch (NtgXmlRpcCall * failedCaller)
    {
        _failed->exec();
        if (_failed->continueCall())
        {
            return closeSession();
        }
        else
        {
            this->deleteLater();
            qApp->exit(0);
        }
    }
    return QString();
}

//manage transforms :

int CallerManager::createTransform(int graphId, QString type)
{
    try
    {
        //TODO : Optimize LocalTransforms
        //Function disabled
//        if (_localTransformManager->isTransformType(type))
//            return _localTransformManager->createTransform(type);
//        else
            if (!_offLineWork)
                return _caller->call("netglub.createTransform", graphId, type).toInt();
    }
    catch (NtgXmlRpcCall * failedCaller)
    {
        _failed->exec();
        if (_failed->continueCall())
        {
            return createTransform(graphId, type);
        }
        else
        {
            this->deleteLater();
            qApp->exit(0);
        }
    }
    return 0;
}

bool CallerManager::destroyTransform(int graphId, int transformId)
{
    try
    {
        //TODO : Optimize LocalTransforms
        //Function disabled
//        if (_localTransformManager->isTransformId(transformId))
//            return _localTransformManager->destroyTransform(transformId);
//        else
            if (!_offLineWork)
                return _caller->call("netglub.destroyTransform", graphId, transformId).toBool() ;
    }
    catch (NtgXmlRpcCall * failedCaller)
    {
        _failed->exec();
        if (_failed->continueCall())
        {
            return destroyTransform(graphId, transformId);
        }
        else
        {
            this->deleteLater();
            qApp->exit(0);
        }
    }
    return false;
}

bool CallerManager::execTransform(int graphId, int transformId, QVariantMap inputEntity, QVariantMap inputParams)
{
    NtgEntity input = _xmlRpcStructToEntity(inputEntity);
    HashStringString params = _xmlRpcStructToStringHash(inputParams);

    try
    {
        //TODO : Optimize LocalTransforms
        //Function disabled
//        if (_localTransformManager->isTransformId(transformId))
//            return _localTransformManager->execTransform(transformId,input,params);
//        else
            if (!_offLineWork)
                return _caller->call("netglub.execTransform", graphId, transformId, inputEntity, inputParams).toBool() ;
    }
    catch (NtgXmlRpcCall * failedCaller)
    {
        _failed->exec();
        if (_failed->continueCall())
        {
            return execTransform(graphId, transformId, inputEntity, inputParams);
        }
        else
        {
            this->deleteLater();
            qApp->exit(0);
        }
    }
    return false;
}

bool CallerManager::cancelTransform(int graphId, int transformId)
{
    try
    {
        //TODO : Optimize LocalTransforms
        //Function disabled
//        if (_localTransformManager->isTransformId(transformId))
//            return _localTransformManager->cancelTransform(transformId);
//        else
            if (!_offLineWork)
                return _caller->call("netglub.cancelTransform", graphId, transformId).toBool() ;
    }
    catch (NtgXmlRpcCall * failedCaller)
    {
        _failed->exec();
        if (_failed->continueCall())
        {
            return cancelTransform(graphId, transformId);
        }
        else
        {
            this->deleteLater();
            qApp->exit(0);
        }
    }
    return false;
}

QVariantMap CallerManager::infoOfTransform(int graphId, int transformId)
{
    try
    {

//        qDebug() << "CallerManager::infoOfTransform ";
        //TODO : Optimize LocalTransforms
        //Function disabled
//        if (_localTransformManager->isTransformId(transformId))
//            return _localTransformManager->infoOfTransform(transformId);

        if (!_offLineWork)
            return _caller->call("netglub.infoOfTransform", graphId, transformId).toMap();
    }
    catch (NtgXmlRpcCall * failedCaller)
    {
        _failed->exec();
        if (_failed->continueCall())
        {
            return infoOfTransform(graphId, transformId);
        }
        else
        {
            this->deleteLater();
            qApp->exit(0);
        }
    }
    return QVariantMap();
}


QVariantList CallerManager::resultsOfTransform(int graphId, int transformId, int startIndex, int toIndex)
{
    try
    {


//        qDebug() << "CallerManager::resultsOfTransform";
        //TODO : Optimize LocalTransforms
        //Function disabled
//        if (_localTransformManager->isTransformId(transformId))
//            return _localTransformManager->resultsOfTransform(transformId,startIndex,toIndex);
//        else

            if (!_offLineWork)
                return _caller->call("netglub.resultsOfTransform", graphId, transformId, startIndex, toIndex).toList();
    }
    catch (NtgXmlRpcCall * failedCaller)
    {
        _failed->exec();
        if (_failed->continueCall())
        {
            return resultsOfTransform(graphId, transformId, startIndex, toIndex);
        }
        else
        {
            this->deleteLater();
            qApp->exit(0);
        }
    }
    return QVariantList();
}

QHash<QString,QString> CallerManager::_xmlRpcStructToStringHash(const QVariantMap & structMap)
{
    QHash<QString,QString> hash;
    foreach(const QString & key, structMap.keys())
        hash.insert(key, structMap.value(key).toString());
    return hash;
}



NtgEntity CallerManager::_xmlRpcStructToEntity(QVariantMap structMap)
{
    NtgEntity entity;
    entity.type = structMap.take("entity_type").toString();
    entity.values = _xmlRpcStructToStringHash(structMap);
    return entity;
}
//
//int CallerManager::createTransform(int graphId, QString type)
//{
//
//    try
//    {
//        if (!_offLineWork)
//            return _caller->call("netglub.createTransform", graphId, type).toInt();
//    }
//    catch (NtgXmlRpcCall * failedCaller)
//    {
//        _failed->exec();
//        if (_failed->continueCall())
//        {
//            return createTransform(graphId, type);
//        }
//        else
//        {
//            this->deleteLater();
//            qApp->exit(0);
//        }
//    }
//    return 0;
//}
//
//bool CallerManager::destroyTransform(int graphId, int transformId)
//{
//    try
//    {
//        if (!_offLineWork)
//            return _caller->call("netglub.destroyTransform", graphId, transformId).toBool() ;
//    }
//    catch (NtgXmlRpcCall * failedCaller)
//    {
//        _failed->exec();
//        if (_failed->continueCall())
//        {
//            return destroyTransform(graphId, transformId);
//        }
//        else
//        {
//            this->deleteLater();
//            qApp->exit(0);
//        }
//    }
//    return false;
//}
//
//bool CallerManager::execTransform(int graphId, int transformId, QVariantMap inputEntity, QVariantMap inputParams)
//{
//    try
//    {
//        if (!_offLineWork)
//            return _caller->call("netglub.execTransform", graphId, transformId, inputEntity, inputParams).toBool() ;
//    }
//    catch (NtgXmlRpcCall * failedCaller)
//    {
//        _failed->exec();
//        if (_failed->continueCall())
//        {
//            return execTransform(graphId, transformId, inputEntity, inputParams);
//        }
//        else
//        {
//            this->deleteLater();
//            qApp->exit(0);
//        }
//    }
//    return false;
//}
//
//bool CallerManager::cancelTransform(int graphId, int transformId)
//{
//    try
//    {
//        if (!_offLineWork)
//            return _caller->call("netglub.cancelTransform", graphId, transformId).toBool() ;
//    }
//    catch (NtgXmlRpcCall * failedCaller)
//    {
//        _failed->exec();
//        if (_failed->continueCall())
//        {
//            return cancelTransform(graphId, transformId);
//        }
//        else
//        {
//            this->deleteLater();
//            qApp->exit(0);
//        }
//    }
//    return false;
//}
//
//QVariantMap CallerManager::infoOfTransform(int graphId, int transformId)
//{
//    try
//    {
//        if (!_offLineWork)
//            return _caller->call("netglub.infoOfTransform", graphId, transformId).toMap();
//    }
//    catch (NtgXmlRpcCall * failedCaller)
//    {
//        _failed->exec();
//        if (_failed->continueCall())
//        {
//            return infoOfTransform(graphId, transformId);
//        }
//        else
//        {
//            this->deleteLater();
//            qApp->exit(0);
//        }
//    }
//    return QVariantMap();
//}
//
//QVariantList CallerManager::resultsOfTransform(int graphId, int transformId, int startIndex, int toIndex)
//{
//    try
//    {
//        if (!_offLineWork)
//            return _caller->call("netglub.resultsOfTransform", graphId, transformId, startIndex, toIndex).toList();
//    }
//    catch (NtgXmlRpcCall * failedCaller)
//    {
//        _failed->exec();
//        if (_failed->continueCall())
//        {
//            return resultsOfTransform(graphId, transformId, startIndex, toIndex);
//        }
//        else
//        {
//            this->deleteLater();
//            qApp->exit(0);
//        }
//    }
//    return QVariantList();
//}


