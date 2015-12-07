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

#ifndef NTGINTERFACE_H
#define NTGINTERFACE_H

#include<QObject>
#include<QtScript>
#include "graph.h"
#include "ntgTypes.h"


class Transform;

QScriptValue print_debug(QScriptContext *context, QScriptEngine *engine);
QScriptValue print_info(QScriptContext *context, QScriptEngine *engine);
QScriptValue print_warning(QScriptContext *context, QScriptEngine *engine);
QScriptValue print_critical(QScriptContext *context, QScriptEngine *engine);


class GraphPrototype : public QObject, public QScriptable
{
    Q_OBJECT

public:
    GraphPrototype(QObject *parent=0);
public slots:
    QObject* newNode(QString entityType);
    QObjectList nodeList();
    bool transformsFinished();
    void waitForTransformsFinished();
    QObject* createTransform(QString name);

    void deleteNode(Node*);
    void deleteEdge(Node*, Node*);
    void addEdge(Node*, Node*);

    void stats();

    void setLayout(QString, QString);
    void startLayout();
    void bestFit();
private :

};

class NodePrototype : public QObject, public QScriptable
{
    Q_OBJECT

public:
    NodePrototype(QObject *parent=0);
public slots :
    void display();
    void setParam(QString name,QString value);
    QString param(QString name);
    QString type();
    QStringList availableTransforms();
    void transform(QString transform);
    void advTransform(Transform*);
private :
        QMap<QScriptValue,Node*> _remaininObjects;
};


class Transform : public QObject
{
    Q_OBJECT
    friend class TransformInterface;
public:
    Transform(QObject * parent = 0, QString name= "");
    Transform(const Transform&);
    QVariantMap getParams();
    QString getName();
private:
    QString _name;
    QVariantMap _params;
};

class TransformInterface : public QObject, public QScriptable
{
    Q_OBJECT
public:
    TransformInterface(QObject * parent=0);

public slots:
    void setParam(QString, QString);
    QString param(QString);
    void display();
private :
        QMap<QScriptValue,Transform*> _remaininObjects;

};

Q_DECLARE_METATYPE(Graph*)
Q_DECLARE_METATYPE(Node*)
Q_DECLARE_METATYPE(QVector<Node*>)
Q_DECLARE_METATYPE(Transform*)


#endif // INTERFACE_H
