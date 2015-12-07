/******************************************************************************
**
** Copyright (C) 2009-2010 DIATEAM. All rights reserved.
**
** This file is part of the Netglub GUI program.
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

#include "transformtimer.h"
#include "mainwindow.h"


QHash<QString,QString> xmlRpcStructToStringHash(const QVariantMap & structMap)
{
  QHash<QString,QString> hash;
  foreach(const QString & key, structMap.keys())
    hash.insert(key, structMap.value(key).toString());
  return hash;
}




TransformTimer::TransformTimer(QObject *parent) :
    QObject(parent),
    _mutex(QMutex::Recursive)
{
//  qDebug() << "TransformTimer::TransformTimer";
  _timer = new QTimer(parent);

  _caller = CallerManager::get();
  connect(_timer,SIGNAL(timeout()),this,SLOT(doTransforms()));
}


TransformTimer::~TransformTimer()
{
  qDebug() << "TransformTimer::~TransformTimer";
  if (!_transformStructHash.isEmpty())
  {
    qDeleteAll(_transformStructHash);
  }
  delete _timer;
}

void TransformTimer::doTransforms()
{
    QMutexLocker locker(&_mutex);
  bool redraw = false;
//  qDebug() << "TransformTimer::doTransforms";
  if (_transformStructHash.isEmpty())
    _timer->stop();
  else
  {
//    QList<Graph *> graphToRedraw;
//    qDebug() << "TransformTimer::doTransforms when finding some transforms";
    QVariantList results;
    foreach(QString key,_transformStructHash.keys())
    {
//      qDebug() << "doTransform :" << key;
      QVariantMap infos;
      //int lastResultIndex = -1;

      _graphToRedraw.append(_transformStructHash.value(key)->graph);
      infos = _caller->infoOfTransform(_graphToRedraw.last()->getGraphId(),_transformStructHash.value(key)->transformId);
      if( infos.isEmpty() )
      {
        qWarning() << "get Infos from master failed:" << _caller->lastError();
        _transformStructHash.remove(key);
        emit transformFinished();
        return;
      }
      int tmp = infos.value("latest result index").toInt();

      if( tmp != _transformStructHash.value(key)->lastResultIndex)
      {
          results.clear();
        results = _caller->resultsOfTransform(_transformStructHash.value(key)->graph->getGraphId(),
                                              _transformStructHash.value(key)->transformId,
                                              _transformStructHash.value(key)->lastResultIndex+1,
                                              tmp);
        if( results.isEmpty() )
        {
          qDebug() << "resultsOfTransform failed:" << _caller->lastError();
          _transformStructHash.remove(key);
          emit transformFinished();
          return;
        }
        redraw = true;
//        endingNumber += results.count();
        foreach(const QVariant & res, results)
        {
          NtgEntity entity;
          QVariantMap resMap = res.toMap();
          entity.type = resMap.take("entity_type").toString();
          entity.values = xmlRpcStructToStringHash(resMap);
//          qDebug() << entity.values;
          bool errorInGrphDesign = false;
          if (entity.values.contains("id"))
          {
              if (!entity.values.contains("parent-id"))
                  errorInGrphDesign = true;
              QString nId = entity.values.take("parent-id");
//              QList<int> intIdList;
              if (nId.isEmpty())
              {
                  QString tmp = entity.values.take("id");
                  Node * pnode;
                  pnode = _transformStructHash.value(key)->graph->addNodeFromTransform(ModelsManager::getOrCreate()->getTransformLongName(_transformStructHash.value(key)->name),
                                                                           0,
                                                                           entity,
                                                                           ModelsManager::getOrCreate()->getEntityModel(entity.type));
                  _transformStructHash.value(key)->localNodeIdMap.insert(tmp,pnode);
              }
              else
              {
                  QStringList idList = nId.split(",");
                  foreach(QString id , idList)
                  {
                      Node * pnode;
                      if (_transformStructHash.value(key)->localNodeIdMap.contains(id))
                      {
                          QString tmp = entity.values.take("id");
                          pnode = _transformStructHash.value(key)->graph->addNodeFromTransform(ModelsManager::getOrCreate()->getTransformLongName(_transformStructHash.value(key)->name),
                                                                                   _transformStructHash.value(key)->localNodeIdMap.value(id),
                                                                                   entity,
                                                                                   ModelsManager::getOrCreate()->getEntityModel(entity.type));
                          _transformStructHash.value(key)->localNodeIdMap.insert(tmp,pnode);
                       }
                       else
                           errorInGrphDesign = true;

                  }
              }

//              qDebug() << intIdList;
          }
          else
              errorInGrphDesign = true;
          if (errorInGrphDesign)
          {
            _transformStructHash.value(key)->graph->addNodeFromTransform(ModelsManager::getOrCreate()->getTransformLongName(_transformStructHash.value(key)->name),
                                                                         _transformStructHash.value(key)->node,
                                                                         entity,
                                                                         ModelsManager::getOrCreate()->getEntityModel(entity.type));
          }
          _transformStructHash.value(key)->countEntity++;
        }
        _transformStructHash.value(key)->lastResultIndex = tmp;
      }


      bool transformEnded = false;
      if( infos.value("status") == "Finished")
      {
          QString text = "*** Transform from "
                         + _transformStructHash.value(key)->node->getEntity().values.value("value") +" "
                         + ModelsManager::getOrCreate()->getTransformLongName(_transformStructHash.value(key)->name)
                         + " finished with "+ QString::number( _transformStructHash.value(key)->countEntity) +" outputs ***";
          MainWindow::get()->addInfoLogMessage(text);
//        qWarning() << text;
        transformEnded =true;
        emit transformFinished();
      }

      if( infos.value("status") == "Failed")
      {
          QString text = "*** Transform from "
                         + _transformStructHash.value(key)->node->getEntity().values.value("value") +" "
                         + ModelsManager::getOrCreate()->getTransformLongName(_transformStructHash.value(key)->name) + " aborted ***";
//          MainWindow::get()->addInfoLogMessage(text);
          qWarning() << text;
        transformEnded =true;
        emit transformFinished();
      }

      if (transformEnded)
      {
          if( ! _caller->destroyTransform( _transformStructHash.value(key)->graph->getGraphId(),
                                           _transformStructHash.value(key)->transformId))
          {

              qWarning("detroyTransform failed: %s", qPrintable(_caller->lastError()));
              return;
          }
          _transformStructHash.value(key)->node->update();
          _transformStructHash.remove(key);
//          qDebug("*** Transform destroyed ***");
      }


    }
    if (redraw)
    {
        foreach(Graph * g, _graphToRedraw)
        {
            if (g)
                g->startLayout();
        }
        _graphToRedraw.clear();
    }
  }

}

void TransformTimer::addTransform(QString transformName, Graph * graph, QList<Node *> nodeList, QVariantMap param)
{
    if (_timer->isActive())
        _timer->stop();
    QMutexLocker locker(&_mutex);
    foreach(Node * node, nodeList)
    {

        //    qDebug() << "TransformTimer::addTransform";
        TransformStruct * transform = new TransformStruct();
        transform->graph= graph;
        transform->node = node;
        transform->param = param;
        transform->lastResultIndex = -1;
        transform->name = transformName;
        transform->countEntity =0;
        transform->localNodeIdMap.insert("0",node);

        transform->transformId = _caller->createTransform(graph->getGraphId(),transformName);
        connect(node,SIGNAL(destroyedNode(Node*)),this,SLOT(stopTransform(Node*)));
        if( transform->transformId == 0 )
        {
            qCritical() << _caller->lastError();
          return;
        }
        _transformStructHash.insert(transformName + QString::number(transform->transformId),transform);
    //    qDebug() << "-------------------------------------------------------\n" <<_transformStructHash << "\n-------------------------------------------------------\n";

        QVariantMap entityMap;
        NtgEntity ent = node->getEntity();
        entityMap.insert("entity_type",ent.type);
        foreach(QString key , ent.values.keys())
        {
          entityMap.insert(key,ent.values.value(key));
        }


        if( ! _caller->execTransform(graph->getGraphId(), transform->transformId, entityMap, transform->param) )
        {
          qDebug() << "execTransform failed:" << _caller->lastError();
          return;
        }
        QString text = "*** Transform from \"" + node->getEntity().values.value("value") +"\" " + ModelsManager::getOrCreate()->getTransformLongName(transformName) + " started ***";
        MainWindow::get()->addInfoLogMessage(text);
    //    qDebug("*** Transform started ***");
        emit beginTransform();
    }

    _timer->start(5*1000);
}


void TransformTimer::removeGraph(Graph *graph)
{
    foreach(QString key,_transformStructHash.keys())
    {
        if(_transformStructHash.value(key)->graph/*->getGraphId()*/ == graph/*->getGraphId()*/)
        {
            _transformStructHash.remove(key);
            emit transformFinished();
        }
    }
    _graphToRedraw.removeAll(graph);
}

void TransformTimer::stopTransforms()
{
    QMutexLocker locker(&_mutex);
    if (_timer->isActive())
    {
        _timer->stop();
        foreach(QString key,_transformStructHash.keys())
        {
            _caller->cancelTransform(_transformStructHash.value(key)->graph->getGraphId(),_transformStructHash.value(key)->transformId);
            QString text = "*** Transform from "
                           + _transformStructHash.value(key)->node->getEntityModel().longName +" "
                           + ModelsManager::getOrCreate()->getTransformLongName(_transformStructHash.value(key)->name) + " canceled ***";
            MainWindow::get()->addInfoLogMessage(text);
        }
        _transformStructHash.clear();
    }
}

void TransformTimer::stopTransform(Node * node)
{
    QMutexLocker locker(&_mutex);
    foreach(QString key,_transformStructHash.keys())
    {
        if(_transformStructHash.value(key)->graph->getGraphId() == node->getGraph()->getGraphId() &&
           _transformStructHash.value(key)->node->getId() == node->getId())
        {
            _caller->destroyTransform(_transformStructHash.value(key)->graph->getGraphId(),_transformStructHash.value(key)->transformId);
            _transformStructHash.remove(key);
            emit transformFinished();
        }
    }
}

bool TransformTimer::activeTransform(Graph * graph)
{
    QMutexLocker locker(&_mutex);
    foreach(TransformStruct * transform,_transformStructHash.values())
    {
        if(transform->graph == graph)
        {
            return true;
        }
    }
    return false;
}
