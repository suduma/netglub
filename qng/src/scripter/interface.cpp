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

#include "interface.h"
#include "graph.h"

#include <QtGui>
#include "graphview.h"
#include "mainwindow.h"
#include "graphview.h"
#include "edgeitem.h"
#include <unistd.h>


GraphPrototype::GraphPrototype(QObject *parent) : QObject(parent)
{
}

QObject* GraphPrototype::newNode(QString entityType)
{
    Graph * graph= qscriptvalue_cast<Graph*>(thisObject());
    if(graph)
    {
        Node * node = graph->newNode();
        NtgEntityModel model = ModelsManager::getOrCreate()->getEntityModel(entityType);
        node->setEntityModel(model);
        NtgEntity entity;
        entity.type = model.name;
        foreach(NtgTransformParam param, model.params)
        {
            entity.values.insert(param.name,param.defaultValue);
        }
        node->setEntity(entity);

        if(graph->getView()!=NULL)
        {
            NtgNodeItem * item = node->newNtgItem();
            graph->getView()->scene()->addItem(item);
            graph->getView()->redrawItem(item);
        }
        return qobject_cast<Node*>(node);
    }
    return NULL;
}

void GraphPrototype::stats()
{
    Graph * graph= qscriptvalue_cast<Graph*>(thisObject());

    if(graph)
    {
        QString str = "Graph : "+graph->getName();
        str += "\n  "+QString::number(graph->getNodeList().size())+" nodes; "+QString::number(graph->getEdgeSet().size())+" edges.";
        MainWindow::get()->addInfoLogMessage(str);
    }
}

void GraphPrototype::setLayout(QString view, QString layout)
{
    Graph * graph= qscriptvalue_cast<Graph*>(thisObject());
    if(graph)
    {
        if(graph->getView())
        {
            int id = MiningBlockLayoutId;
            if(view == "mining" && layout == "block")
                id = MiningBlockLayoutId;
            else if (view == "mining" && layout == "hierarchical")
                id = MiningHierarchyLayoutId;
            else if (view == "mining" && layout == "circular")
                id = MiningCircularLayoutId;
            else if (view == "mining" && layout == "molecular")
                id = MiningMolecularLayoutId;
            else if(view == "centrality" && layout == "block")
                id = CentralityBlockLayoutId;
            else if (view == "centrality" && layout == "hierarchical")
                id = CentralityHierarchyLayoutId;
            else if (view == "centrality" && layout == "circular")
                id = CentralityCircularLayoutId;
            else if (view == "centrality" && layout == "molecular")
                id = CentralityMolecularLayoutId;
            else if(view == "edgeWeighted" && layout == "block")
                id = EdgeWeightedBlockLayoutId;
            else if (view == "edgeWeighted" && layout == "hierarchical")
                id = EdgeWeightedHierarchyLayoutId;
            else if (view == "edgeWeighted" && layout == "circular")
                id = EdgeWeightedCircularLayoutId;
            else if (view == "edgeWeighted" && layout == "molecular")
                id = EdgeWeightedMolecularLayoutId;
            else
                qCritical() << "args in {mining,centrality,edgeWeighted}-{block,hierarchical,circular,molecular}";

            graph->getView()->setLayoutId(id);
            MainWindow::get()->refreshLayoutActions();
        }
    }
}

void GraphPrototype::startLayout()
{
    Graph * graph= qscriptvalue_cast<Graph*>(thisObject());
    if(graph)
    {
        graph->startLayout();
    }
}

void GraphPrototype::bestFit()
{
    Graph * graph= qscriptvalue_cast<Graph*>(thisObject());
    if(graph)
    {
        if(graph->getView())
            graph->getView()->bestFit();
    }
}

QObjectList GraphPrototype::nodeList()
{
    QObjectList objLlist;
    Graph * graph= qscriptvalue_cast<Graph*>(thisObject());
    if(graph)
    {
        foreach(Node * pNode, graph->getNodeList())
        {
            QObject * object = qobject_cast<Node*>(pNode);
            objLlist.append(object);
        }
    }
    return objLlist;
}

bool GraphPrototype::transformsFinished()
{
    bool ret = true;
    Graph * graph= qscriptvalue_cast<Graph*>(thisObject());
    if(graph)
    {
        ret = !graph->getTransformTimer()->activeTransform(graph);
    }
    return ret;
}

void GraphPrototype::waitForTransformsFinished()
{
    while(!transformsFinished())
    {
        usleep(10000);
        qApp->processEvents();
    }
}

QObject* GraphPrototype::createTransform(QString name)
{
    Transform * transform = new Transform(this,name);
    return qobject_cast<Transform*>(transform);
}

void GraphPrototype::deleteNode(Node * node)
{
    Graph * graph= qscriptvalue_cast<Graph*>(thisObject());
    if(graph)
    {
        graph->deleteNode(node);
    }
}

void GraphPrototype::deleteEdge(Node* fromNode, Node* toNode)
{
    Graph * graph= qscriptvalue_cast<Graph*>(thisObject());
    if(graph)
    {
        foreach(Edge* pEdge, fromNode->getOutEdgeSet())
        {
            if(pEdge->getHeadNode()==toNode)
                graph->deleteEdge(pEdge);
        }
    }
}

void GraphPrototype::addEdge(Node* fromNode, Node* toNode)
{
    Graph * graph= qscriptvalue_cast<Graph*>(thisObject());
    if(graph)
    {
        Edge * edge = graph->newEdge(fromNode,toNode);
        if (edge != NULL)
        {
            EdgeItem * edgeItem = edge->newItem();
            graph->getView()->scene()->addItem(edgeItem);
        }
    }
}



/*
 *  Node Prototype
 */

NodePrototype::NodePrototype(QObject *parent) : QObject(parent)
{
}

void NodePrototype::display()
{
    Node * node= qscriptvalue_cast<Node*>(thisObject());
    if(node)
    {
        QString  str;
        NtgEntityModel model = node->getEntityModel();
        NtgEntity entity = node->getEntity();
        str += "Entity : " + model.name;
        foreach(NtgTransformParam param, model.params)
        {
            str += "\n  " + param.name + " : ";
            if(entity.values.contains(param.name))
                str += entity.values.value(param.name);
            else
                str += "undefined";
        }
        MainWindow::get()->addInfoLogMessage(str);
    }
}

void NodePrototype::setParam(QString name,QString value)
{
    Node * node= qscriptvalue_cast<Node*>(thisObject());
    if(node)
    {
        node->addToEntity(name,value);
    }
}

QString NodePrototype::param(QString name)
{
    Node * node= qscriptvalue_cast<Node*>(thisObject());
    if(node)
    {
        NtgEntity entity = node->getEntity();
        if(entity.values.contains(name))
            return entity.values.value(name);
    }
    return QString("");
}

QString NodePrototype::type()
{
    Node * node= qscriptvalue_cast<Node*>(thisObject());
    if(node)
    {
        return node->getEntityModel().name;
    }
    return "";
}

QStringList NodePrototype::availableTransforms()
{
    QStringList objectList;
    Node * node= qscriptvalue_cast<Node*>(thisObject());
    if(node)
    {
        QString str;
        QMultiMap<QString,NtgTransformModel> transformMap = ModelsManager::getOrCreate()->getAvailableTransformsbyCategories(node->getEntityModel().name);
        foreach(NtgTransformModel m, transformMap.values())
        {
            objectList.append(m.name);
        }
    }
    return objectList;
}

void NodePrototype::transform(QString transform)
{
    Node * node= qscriptvalue_cast<Node*>(thisObject());
    if(node)
    {
        Graph * graph = node->getGraph();
        QVariantMap parameters;
        QList<Node*> nodelist;
        nodelist.append(node);
        graph->getTransformTimer()->addTransform(transform,graph,nodelist,parameters);
    }
}

void NodePrototype::advTransform(Transform* transform)
{
    Node * node= qscriptvalue_cast<Node*>(thisObject());
    if(node)
    {
        Graph * graph = node->getGraph();
        QVariantMap parameters = transform->getParams();
        QList<Node*> nodelist;
        nodelist.append(node);
        QString name = transform->getName();
        graph->getTransformTimer()->addTransform(name,graph,nodelist,parameters);
    }
}





















/*
 *  Transform
 */
Transform::Transform(QObject *parent, QString name) : QObject(parent), _name(name)
{

}

Transform::Transform(const Transform& t) : QObject(t.parent())
{
    _name = t._name;
    _params = t._params;
}

QVariantMap Transform::getParams()
{
    return _params;
}

QString Transform::getName()
{
    return _name;
}

/*
 *  TransformInterface
 */
TransformInterface::TransformInterface(QObject *parent) : QObject(parent)
{

}

void TransformInterface::setParam(QString name, QString value)
{
    Transform * transform = qscriptvalue_cast<Transform*>(thisObject());

    if(transform)
    {
        transform->_params.insert(name,QVariant(value));
    }
}

QString TransformInterface::param(QString name)
{
    Transform * transform = qscriptvalue_cast<Transform*>(thisObject());
    if(transform && transform->_params.contains(name))
        return transform->_params.value(name).toString();
    return "";
}

void TransformInterface::display()
{
    Transform * transform = qscriptvalue_cast<Transform*>(thisObject());
    if(transform)
    {
        MainWindow::get()->addInfoLogMessage("Transform :" + transform->_name);
    }
}





/* print functions */
QScriptValue print_debug(QScriptContext *context, QScriptEngine *)
{
    QScriptValue nothing;
    if (context->argumentCount() == 1)
        qDebug() << context->argument(0).toString();
    return nothing;
}

QScriptValue print_info(QScriptContext *context, QScriptEngine *)
{
    QScriptValue nothing;
    if (context->argumentCount() == 1)
        MainWindow::get()->addInfoLogMessage(context->argument(0).toString());
    return nothing;
}

QScriptValue print_warning(QScriptContext *context, QScriptEngine *)
{
    QScriptValue nothing;
    if (context->argumentCount() == 1)
        qWarning() << context->argument(0).toString();
    return nothing;
}

QScriptValue print_critical(QScriptContext *context, QScriptEngine *)
{
    QScriptValue nothing;
    if (context->argumentCount() == 1)
        qCritical() << context->argument(0).toString();
    return nothing;
}

