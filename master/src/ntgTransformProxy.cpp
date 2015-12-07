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

#include "ntgTransformProxy.h"

/****************************************************************************
**  class NtgTransformCache
****************************************************************************/

NtgTransformCache * NtgTransformCache::_transformCache = NULL;

NtgTransformCache * NtgTransformCache::getOrCreate()
{
    if (!_transformCache)
    {
        _transformCache = new NtgTransformCache();
    }
    return _transformCache;
}


NtgTransformCache::NtgTransformCache(): _mutex(QMutex::Recursive)
{

}

NtgTransformCache::~NtgTransformCache()
{

}

void NtgTransformCache::addTocache(QString transformName,
                NtgEntity inputEntity,
                QList<NtgEntity> ouputEntitiesList)
{
    QMutexLocker locker(&_mutex);
    int index = contains(transformName,inputEntity);
    if (index == -1)
    {
        NtgOutputCache * outputcache = new NtgOutputCache;
        outputcache->transformName= transformName;
        outputcache->inputEntity = inputEntity;
        _cache.append(outputcache);
        index = _cache.count() - 1;
    }
    _cache.at(index)->ouputEntitiesList.append(ouputEntitiesList);
}

int NtgTransformCache::contains(QString transformName, NtgEntity inputEntity)
{
    QMutexLocker locker(&_mutex);
    int index = 0;
    foreach (NtgOutputCache * outputCache, _cache)
    {
        if (outputCache->inputEntity == inputEntity && outputCache->transformName == transformName)
        {
            return index;
        }
        index++;
    }
    return -1;
}

QList<NtgEntity> NtgTransformCache::getFromCache(QString transformName, NtgEntity inputEntity)
{
    QMutexLocker locker(&_mutex);
    qDebug() << "from cache";
    foreach (NtgOutputCache * outputCache, _cache)
    {
        if (outputCache->inputEntity == inputEntity && outputCache->transformName == transformName)
        {
            return outputCache->ouputEntitiesList;
        }
    }
    return QList<NtgEntity>();
}

void NtgTransformCache::clearAll()
{
    _cache.clear();
}

/****************************************************************************
**  class NtgTransformProxy
****************************************************************************/

typedef QHash<int,NtgTransformProxy*> TransformHashType;
Q_GLOBAL_STATIC(TransformHashType, transformHash)

int NtgTransformProxy::_idCount = 0;

NtgTransformProxy::NtgTransformProxy(NtgSlaveProxy * slaveProxy,
                                     NtgRpcHandler * handler,
                                     const QString & objName,
                                     const NtgTransformModel & model,
                                     const QString & sessionId,
                                     bool enableCache)
  : QObject()
  , _id(++_idCount)
  , _sessionId(sessionId)
  , _handler(handler)
  , _objName(objName)
  , _model(model)
  ,_slaveProxy(slaveProxy)

{
  _takeFromCache = false;
  _reset();
  _handler->registerSignalProxy(this, _objName);
  transformHash()->insert(_id, this);
  _enableCache=enableCache;

}


NtgTransformProxy::~NtgTransformProxy()
{
  transformHash()->remove(_id);
}


int NtgTransformProxy::id() const
{
  return _id;
}


QString NtgTransformProxy::sessionId() const
{
  return _sessionId;
}


void NtgTransformProxy::exec(const NtgEntity & input, QHash<QString, QString> params)
{
    _inputEntityFixed = input;
    if(_enableCache)
    {
        if (NtgTransformCache::getOrCreate()->contains(_model.name,input) != -1)
        {
            _resultList = NtgTransformCache::getOrCreate()->getFromCache(_model.name,input);
            _status = Ntg::Finished;
            _takeFromCache = true;
        }
    }


    if(!_takeFromCache)
    {
        _reset();
        QVariantList args;
        args << QVariant::fromValue(input) << QVariant::fromValue(params);
        if( ! _handler->invokeMethod(_objName, "exec", args))
        {
        qCritical("Invoke %s failed : %s", __FUNCTION__, qPrintable(_handler->lastError()));
        }
    }
}


void NtgTransformProxy::cancel()
{
  QVariantList args;
  if( ! _handler->invokeMethod(_objName, "cancel", args))
    qCritical("Invoke %s failed : %s", __FUNCTION__, qPrintable(_handler->lastError()));
}


Ntg::TransformStatus NtgTransformProxy::status() const
{
    return _status;
}


double NtgTransformProxy::progressValue () const
{
  return _progressPercent;
}


QString NtgTransformProxy::progressText () const
{
  return _progressText;
}


Ntg::TransformError NtgTransformProxy::error() const
{
  return _error;
}


QString NtgTransformProxy::errorString() const
{
  return _errorString;
}


int NtgTransformProxy::latestResultIndex() const
{
    return _resultList.size()-1;
}


QList<NtgEntity> NtgTransformProxy::results(int startIndex, int toIndex) const
{
    QList<NtgEntity> result;
    if( startIndex >= _resultList.size() || (toIndex > 0 && toIndex < startIndex))
        return result;

    if( toIndex >= _resultList.size() || toIndex < 0)
        result = _resultList.mid(startIndex);
    else
        result = _resultList.mid(startIndex, (toIndex - startIndex) + 1);
    if (!_takeFromCache && _enableCache)
        NtgTransformCache::getOrCreate()->addTocache(_model.name,_inputEntityFixed,result);
    return result;
}


const NtgTransformModel & NtgTransformProxy::model() const
{
  return _model;
}


QString NtgTransformProxy::name() const
{
  return _model.name;
}


const NtgEntity & NtgTransformProxy::inputEntity() const
{
  if(_inputEntity.type.isEmpty()) // not in cache yet ??
  {
    QVariantList args;
    QVariant retVal;
    if( ! _handler->invokeMethod(_objName, "inputEntity", args, &retVal))
      qCritical("Invoke 'inputEntity' failed : %s", qPrintable(_handler->lastError()));
    _inputEntity = retVal.value<NtgEntity>();
  }
  return _inputEntity;
}


QList<NtgTransformProxy*> NtgTransformProxy::transformsInSession(QString sessionId)
{
  QList<NtgTransformProxy*> list;
  foreach(NtgTransformProxy * tProxy, *transformHash())
  {
    if( tProxy->sessionId() == sessionId)
      list.append(tProxy);
  }
  return list;
}


NtgTransformProxy * NtgTransformProxy::getTransform(int id)
{
  if( ! transformHash()->contains(id) )
    return NULL;
  return transformHash()->value(id);
}


void NtgTransformProxy::_statusChanged(Ntg::TransformStatus status)
{
  _status = status;
  if(_status == Ntg::Failed)
  {
    QVariantList args;
    QVariant retValError;
    if( ! _handler->invokeMethod(_objName, "error", args, &retValError))
      qCritical("Invoke 'error' failed : %s", qPrintable(_handler->lastError()));
    QVariant retValErrorString;
    if( ! _handler->invokeMethod(_objName, "errorString", args, &retValErrorString))
      qCritical("Invoke 'error' failed : %s", qPrintable(_handler->lastError()));
    _error = retValError.value<Ntg::TransformError>();
    _errorString = retValErrorString.value<QString>();
  }
  emit statusChanged(_status);
}


void NtgTransformProxy::_progressChanged(double progressPercent, const QString & progressText)
{
  _progressPercent = progressPercent;
  _progressText = progressText;
  emit progressChanged(_progressPercent, _progressText);
}


void NtgTransformProxy::_resultReadyAt(int fromIndex, int toIndex)
{
  if(toIndex >= _resultList.size())
  {
    QVariantList args;
    args << _resultList.size() << toIndex;
    QVariant retVal;
    if( ! _handler->invokeMethod(_objName, "results", args, &retVal))
      qCritical("Invoke 'results' failed : %s", qPrintable(_handler->lastError()));
    _resultList.append(retVal.value<QList<NtgEntity> >());
  }
  emit resultReadyAt(fromIndex, toIndex);
}


void NtgTransformProxy::_reset()
{
  _status = Ntg::NotStarted;
  _error = Ntg::NoError;
  _errorString.clear();
  _progressPercent = 0;
  _progressText.clear();
  _resultList.clear();
  _inputEntity = NtgEntity();
}

NtgSlaveProxy * NtgTransformProxy::slaveProxy()
{
    return _slaveProxy;
}

QString NtgTransformProxy::objName()
{
    return _objName;
}
