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

#ifndef TRANSFORMTIMER_H
#define TRANSFORMTIMER_H

#include <QObject>
#include <QTimer>
#include <QVariantMap>
#include <QMutex>
#include "graph.h"
#include "callermanager.h"
#include "modelsmanager.h"

class Graph;
class Node;

struct TransformStruct
{
  TransformStruct(){};

    Graph * graph;
    Node * node;
    int transformId;
    QVariantMap param;
    int lastResultIndex;
    QString name;
    int countEntity;
    QMap<QString,Node*> localNodeIdMap;
};

class TransformTimer : public QObject
{
Q_OBJECT
public:
    explicit TransformTimer(QObject *parent = 0);
    virtual ~TransformTimer();
    void addTransform(QString transformName, Graph * graph, QList<Node *> node, QVariantMap param);

    void removeGraph(Graph * graph);
    bool activeTransform(Graph*);


signals:
    void transformFinished();
    void beginTransform();


public slots:
    void doTransforms();
    void stopTransforms();
    void stopTransform(Node * node);

protected:
    QTimer * _timer;
    QHash<QString,TransformStruct*>  _transformStructHash;
    CallerManager * _caller;
    QList<Graph *> _graphToRedraw;

    mutable QMutex _mutex;

};

#endif // TRANSFORMTIMER_H
