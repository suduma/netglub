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

#include "graph.h"
#include "graphview.h"
#include "graphframe.h"
#include "mainwindow.h"
#include "ntgnodeitem.h"
#include "edgeitem.h"

#include "settings/entitypopwindow.h"

#include "ultMtgReader.h"
#include "mtgWriter.h"
#include "quazip/quazip.h"
#include "quazip/quazipfile.h"


#include <QtGui>

/****************************************************************************
**  class Graph
****************************************************************************/

uint Graph::_newGraphId = 1;

Graph::Graph(QObject * parent) :
        QObject(parent), _view(NULL), _changeSinceLastSave(false)
{
    _name = QString("Default Graph %1.ng").arg(_newGraphId++);
    _uuid = QUuid::createUuid().toString();
    _nextNodeId = 0;
    _transformTimer = MainWindow::get()->getTransformTimer();
    _graphId = CallerManager::get()->getNewGraphId();
    CallerManager::get()->setGraphEnableCache(_graphId,MainWindow::get()->usingCache());
//    FileManager::getOrCreate()->createTmpFile(this);
//    qDebug() << "Graph::Graph(QObject * parent)";
    _changed = false;

}

Graph::Graph(const Graph& g):
        QObject(g.parent())
{
    _name = g._name;
    _uuid = g._uuid;
    _nextNodeId = g._nextNodeId;
    _transformTimer = g._transformTimer;
    _graphId = g._graphId;
    _view = g._view;
    _changeSinceLastSave = g._changeSinceLastSave;
}

Graph::~Graph()
{
//    qDebug()<< "Graph::~Graph()";
    FileManager::getOrCreate()->destroyTmpFile(this);
//    CallerManager::get()->deleteGraph(_graphId);
    _transformTimer->removeGraph(this);
    qDeleteAll(_edgeSet);
    if (_nodeList.count()>0)
        qDeleteAll(_nodeList);
}

Graph * Graph::newGraph(QObject * parent)
{
//    qDebug("Graph::newGraph(QObject * parent)");
//    qDebug() << QString::number(CallerManager::get()->getNewGraphId(),10);
    Graph * graph = new Graph(parent);
    return graph;
}

Graph * Graph::loadGraph(const QString & fileName, QObject * parent)
{
//    qDebug("Graph::loadGraph(const QString & fileName, QObject * parent), fileName = \"%s\"", qPrintable(fileName));
    Graph * graph = NULL;
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(MainWindow::get(), tr("Read file error"),
                             tr("Cannot read file %1:\n%2.")
                             .arg(fileName)
                             .arg(file.errorString()));
        return graph;

    }
    graph = new Graph(parent);
    graph->setFileName(fileName);

    if(fileName.endsWith(".mtg",Qt::CaseInsensitive) || fileName.endsWith(".mtgxml",Qt::CaseInsensitive))
    {
        if(!loadGraphFromMtg(graph,fileName))
            return graph;
    }
    else if(fileName.endsWith(".ntgxml",Qt::CaseInsensitive))
    {
        if(!loadGraphFromNtg(graph,fileName))
            return graph;
    }
    else if(fileName.endsWith(".ntg",Qt::CaseInsensitive))
    {
        if(!loadGraphFromZip(graph,fileName))
            return graph;
    }
    else
        qWarning("bad file extension");

    MainWindow::get()->addInfoLogMessage("file successfully loaded");
    return graph;
}

bool Graph::loadGraphFromMtg(Graph * graph,QString fileName)
{
    QList<MtgEntity> entityList;
    QList<QPair<int,int> > edgeList;

    QDomDocument doc("MtgXml");
    if(fileName.endsWith(".mtg",Qt::CaseInsensitive))
    {
//        qDebug("load MTG");
        UltMtgReader reader(fileName.toStdString());

        if(!reader.proceed(entityList,edgeList))
            return false;

    }
    else if(fileName.endsWith(".mtgxml",Qt::CaseInsensitive))
    {
//        qDebug("load MTGXML");
        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly))
                return false;
        if (!doc.setContent(&file)) {
                file.close();
                return false;
        }
        file.close();

        QDomElement docElem = doc.documentElement();
        QDomElement xmlEntitySet = docElem.firstChildElement("EntitySet");
        QDomNodeList xmlEntityList = xmlEntitySet.childNodes();
        QList<Node*> tmpNodeList;
        for(unsigned int i(0); i<xmlEntityList.length(); ++i)
        {
            QDomElement xmlentity = xmlEntityList.at(i).toElement();
            MtgEntity mtgEntity;
            mtgEntity._type = xmlentity.attribute("type");
            mtgEntity._additionalSearchTerm = xmlentity.attribute("additionalSearchTerm");
            mtgEntity._version = xmlentity.attribute("version");
            mtgEntity._posx = xmlentity.attribute("posx").toDouble();
            mtgEntity._posy = xmlentity.attribute("posy").toDouble();
            mtgEntity._weight = xmlentity.attribute("weight").toInt();
            mtgEntity._id = i;
            mtgEntity._value = xmlentity.attribute("value");

            QDomElement xmlpropertySet = xmlentity.firstChildElement("PropertySet");
            QDomNodeList propertyList = xmlpropertySet.childNodes();
            for(int i(0); i<propertyList.size(); ++i)
            {
                QDomElement xmlproperty = propertyList.at(i).toElement();
                MtgProperty mtgProperty;
                mtgProperty.name = xmlproperty.attribute("name");
                mtgProperty.value = xmlproperty.text();
                mtgProperty.displayName = xmlproperty.attribute("displayName");
                mtgProperty.description = xmlproperty.attribute("description");
                mtgEntity._propertyHash.insert(mtgProperty.name,mtgProperty);
            }
            QDomElement xmlgeneratorSet = xmlentity.firstChildElement("GeneratorSet");
            QDomNodeList generatorList = xmlgeneratorSet.childNodes();
            for(int i(0); i<propertyList.size();++i)
            {
                QDomElement xmlgenerator = generatorList.at(i).toElement();
                MtgGenerator mtgGenerator;
                mtgGenerator.name = xmlgenerator.attribute("name");
                mtgGenerator.value = xmlgenerator.text();
                mtgEntity._generatorList.append(mtgGenerator);
            }
            entityList.append(mtgEntity);
        }

        QDomElement xmlEdgeSet = docElem.firstChildElement("EdgeSet");
        QDomNodeList xmlEdgeList = xmlEdgeSet.childNodes();
        for(unsigned int i(0); i<xmlEdgeList.length(); ++i)
        {
            QDomElement xmledge = xmlEdgeList.at(i).toElement();
            QPair<int,int> pair;
            pair.first = xmledge.attribute("from").toInt();
            pair.second = xmledge.attribute("to").toInt();
            edgeList.append(pair);
        }
    }

    QList<Node*> tmpnodeList;
    QListIterator<MtgEntity> itrEntityList(entityList);
    while(itrEntityList.hasNext())
    {
        MtgEntity mtgEntity = itrEntityList.next();
        Node * pNode = graph->newNode();
        tmpnodeList.append(pNode);
        NtgEntity ntgEntity = ModelsManager::getOrCreate()->convertMtgEntityToNtgEntity(mtgEntity);
        pNode->setEntity(ntgEntity);
        pNode->setEntityModel(ModelsManager::getOrCreate()->getEntityModel(ntgEntity.type));
    }

    QListIterator<QPair<int,int> > itrEdgeList(edgeList);
    while(itrEdgeList.hasNext())
    {
        QPair<int,int> pair = itrEdgeList.next();
        graph->newEdge(tmpnodeList.at(pair.first),tmpnodeList.at(pair.second));
    }

    return true;
}

bool Graph::loadGraphFromNtg(Graph * graph, QString fileName)
{
    QDomDocument doc("NtgXml");
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly))
        return false;
    if (!doc.setContent(&file)) {
        file.close();
        return false;
    }
    file.close();

    QDomElement docElem = doc.documentElement();
    QDomElement xmlEntitySet = docElem.firstChildElement("EntitySet");
    QDomNodeList entityList = xmlEntitySet.childNodes();
    for(int i(0); i<entityList.size();++i)
    {
        QDomElement xmlentity = entityList.at(i).toElement();
        Node * pNode = graph->newNode();
        NtgEntity entity;
        entity.type = xmlentity.attribute("type");

        QDomNodeList valueList = xmlentity.childNodes();
        for(int j(0); j<valueList.size(); ++j)
        {
            QDomElement xmlvalue = valueList.at(j).toElement();
            entity.values.insert(xmlvalue.attribute("key"),xmlvalue.attribute("value"));
        }
        pNode->setEntity(entity);
        pNode->setEntityModel(ModelsManager::getOrCreate()->getEntityModel(pNode->getEntity().type));
    }

    QList<Node*> nodeList = graph->getNodeList();

    QDomElement xmlEdgeSet = docElem.firstChildElement("EdgeSet");
    QDomNodeList edgeList = xmlEdgeSet.childNodes();
    for(int i(0); i<edgeList.size();++i)
    {
        QDomElement xmledge = edgeList.at(i).toElement();
        if(xmledge.attribute("from").toInt() <nodeList.size() && xmledge.attribute("to").toInt() <nodeList.size())
        {
            Edge * pEdge = graph->newEdge(nodeList.at(xmledge.attribute("from").toInt()),nodeList.at(xmledge.attribute("to").toInt()));
            if(pEdge) pEdge->setTransformName(xmledge.attribute("transform"));
        }
    }
    return true;
}

bool Graph::loadGraphFromZip(Graph * graph, QString fileName)
{
//    qDebug("Graph::loadGraphFromZip");
    QString xmlFile = FileManager::getOrCreate()->createTmpDirFromNtg(graph,fileName);
    if (!xmlFile.isEmpty())
    {
        QDomDocument doc("NtgXml");
        QFile file(xmlFile);
        if (!file.open(QIODevice::ReadOnly))
            return false;
        if (!doc.setContent(&file)) {
            file.close();
            return false;
        }
        file.close();



        QDomElement docElem = doc.documentElement();
        QDomElement xmlEntitySet = docElem.firstChildElement("EntitySet");
        QDomNodeList entityList = xmlEntitySet.childNodes();
        for(int i(0); i<entityList.size();++i)
        {
            QDomElement xmlentity = entityList.at(i).toElement();
            Node * pNode = graph->newNodeWithId(xmlentity.attribute("id").toUInt());
            NtgEntity entity;
            entity.type = xmlentity.attribute("type");
            if (xmlentity.hasAttribute("svgId"))
                pNode->setSvgId(xmlentity.attribute("svgId").toInt());


            QDomNodeList valueList = xmlentity.childNodes();
            for(int j(0); j<valueList.size(); ++j)
            {
                QDomElement xmlvalue = valueList.at(j).toElement();
                if (xmlvalue.nodeName() == "property")
                    entity.values.insert(xmlvalue.attribute("key"),xmlvalue.attribute("value"));
                else if(xmlvalue.nodeName() == "file")
                {
                    NtgFile fileStruct;
                    fileStruct.id = xmlvalue.attribute("id").toInt();
                    fileStruct.shortName = xmlvalue.attribute("shortName");
                    pNode->addFileOnlyToMap(fileStruct);
                }

            }
            FileManager::getOrCreate()->linkNodeAndFiles(pNode);

            pNode->setEntity(entity);
            pNode->setEntityModel(ModelsManager::getOrCreate()->getEntityModel(pNode->getEntity().type));
        }

//        FileManager::getOrCreate()->linkNodeAndFiles(graph);

        QList<Node*> nodeList = graph->getNodeList();

        QDomElement xmlEdgeSet = docElem.firstChildElement("EdgeSet");
        QDomNodeList edgeList = xmlEdgeSet.childNodes();
        for(int i(0); i<edgeList.size();++i)
        {
            QDomElement xmledge = edgeList.at(i).toElement();
            if(xmledge.attribute("from").toInt() <nodeList.size() && xmledge.attribute("to").toInt() <nodeList.size())
            {
                Edge * pEdge = graph->newEdge(nodeList.at(xmledge.attribute("from").toInt()),nodeList.at(xmledge.attribute("to").toInt()));
                if(pEdge) pEdge->setTransformName(xmledge.attribute("transform"));
            }
        }
        qDebug("Graph::loadGraphFromZip end");
        return true;
    }
    return false;
}



bool Graph::saveGraph(const QString & fileName)
{
//    qDebug("Graph::save(const QString & fileName), fileName = \"%s\"", qPrintable(fileName));

    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Text))
    {
        QMessageBox::warning(MainWindow::get(), tr("Recent Files"),
                             tr("Cannot write file %1:\n%2.")
                             .arg(fileName)
                             .arg(file.errorString()));
        return false;
    }

    if(fileName.endsWith(".mtg",Qt::CaseInsensitive) || fileName.endsWith(".mtgxml",Qt::CaseInsensitive))
    {
        if(!saveGraphToMtg(fileName))
            return false;
    }
    else if(fileName.endsWith(".ntgxml",Qt::CaseInsensitive))
    {
        if(!saveGraphToNtg(fileName))
            return false;
    }
    else if(fileName.endsWith(".ntg",Qt::CaseInsensitive))
    {
        if(!saveGraphToZip(fileName))
            return false;
    }
    else
    {
        qWarning("Bad file extension");
        return false;
    }

    setFileName(fileName);
    MainWindow::get()->addInfoLogMessage("file saved : "+fileName);
    return true;
}

bool Graph::saveGraphToMtg(QString fileName)
{
    QList<MtgEntity> entityList;
    QList<QPair<int,int> >edgeList;

    QListIterator<Node*> itrNodeList(_nodeList);
    while(itrNodeList.hasNext())
    {
        Node * pNode = itrNodeList.next();
        MtgEntity mtgEntity = ModelsManager::getOrCreate()->convertNtgEntityToMtgEntity(pNode->getEntity());
        entityList.append(mtgEntity);
    }
    QSetIterator<Edge*> itrEdgeSet(_edgeSet);
    while(itrEdgeSet.hasNext())
    {
        Edge * pEdge = itrEdgeSet.next();
        QPair<int,int> pair;
        pair.first = _nodeList.indexOf(pEdge->getTailNode());
        pair.second = _nodeList.indexOf(pEdge->getHeadNode());
        edgeList.append(pair);
    }

    if(fileName.endsWith(".mtg",Qt::CaseInsensitive))
    {
        MtgWriter mtgWriter(fileName.toStdString());
        mtgWriter.write(entityList,edgeList);
//        qDebug("MTG file saved");
    }
    else //temporary .mtgxml
    {
        QDomDocument doc("MtgXml");
        QDomNode xmlNode = doc.createProcessingInstruction("xml","version=\"1.0\" encoding=\"ISO-8859-1\"");
        doc.insertBefore(xmlNode, doc.firstChild());
        QDomElement root = doc.createElement("MtgXml");
        doc.appendChild(root);
        QDomElement xmlentities = doc.createElement("EntitySet");
        xmlentities.setAttribute("quantity",_nodeList.length());
        root.appendChild(xmlentities);
        //Entities
        for(int i(0); i<_nodeList.size(); ++i)
        {
            Node * pNode = _nodeList.at(i);
            MtgEntity mtgEntity = entityList.at(i);
            QDomElement xmlentity = doc.createElement("Entity");
            xmlentity.setAttribute("id",i);
            xmlentity.setAttribute("version","1.0");
            xmlentity.setAttribute("type",mtgEntity._type);
            xmlentity.setAttribute("value",mtgEntity._value);
            xmlentity.setAttribute("weight",mtgEntity._weight);
            xmlentity.setAttribute("additionalSearchTerm",mtgEntity._additionalSearchTerm);
            xmlentity.setAttribute("posx",pNode->getPos().x());
            xmlentity.setAttribute("posy",pNode->getPos().y());

            QDomElement xmlpropertyset = doc.createElement("PropertySet");
            int propertyCount = mtgEntity._propertyHash.size();
            xmlpropertyset.setAttribute("quantity", propertyCount);
            xmlentity.appendChild(xmlpropertyset);
            QHashIterator<QString,MtgProperty> itrProperty(mtgEntity._propertyHash);
            while(itrProperty.hasNext())
            {
                itrProperty.next();
                MtgProperty mtgProperty = itrProperty.value();
                QDomElement xmlproperty = doc.createElement("Property");
                xmlproperty.setAttribute("name",itrProperty.key());
                xmlproperty.setAttribute("type",mtgProperty.type);
                QDomCDATASection xmltext = doc.createCDATASection(mtgProperty.value);
                xmlproperty.appendChild(xmltext);
                xmlproperty.setAttribute("displayName",mtgProperty.displayName);
                xmlproperty.setAttribute("description",mtgProperty.description);
                xmlproperty.setAttribute("matchingRule",0);
                xmlpropertyset.appendChild(xmlproperty);
            }

            QDomElement xmlgeneratorset = doc.createElement("GeneratorSet");
            xmlgeneratorset.setAttribute("quantity",mtgEntity._propertyHash.size());
            xmlentity.appendChild(xmlgeneratorset);
            QListIterator<MtgGenerator> itrGenerator(mtgEntity._generatorList);
            while(itrGenerator.hasNext())
            {
                MtgGenerator mtgGenerator = itrGenerator.next();
                QDomElement xmlgenerator = doc.createElement("Generator");
                xmlgenerator.setAttribute("name",mtgGenerator.name);
                QDomCDATASection xmltext = doc.createCDATASection(mtgGenerator.value);
                xmlgenerator.appendChild(xmltext);
                xmlgeneratorset.appendChild(xmlgenerator);
            }

            double x,y;
            x = pNode->getPos().x();
            y = pNode->getPos().y();
            xmlentity.setAttribute("posx",x);
            xmlentity.setAttribute("posy",y);

            xmlentities.appendChild(xmlentity);
        }
        QDomElement xmllinks = doc.createElement("EdgeSet");
        xmllinks.setAttribute("quantity",_edgeSet.size());
        root.appendChild(xmllinks);

        QSetIterator<Edge*> itrEdgeSet(_edgeSet);
        uint i = 0;
        while(itrEdgeSet.hasNext())
        {
            Edge * pEdge = itrEdgeSet.next();
            QDomElement xmllink = doc.createElement("Edge");
            xmllink.setAttribute("id",i++);
            xmllink.setAttribute("from",_nodeList.lastIndexOf(pEdge->getTailNode()));
            xmllink.setAttribute("to",_nodeList.lastIndexOf(pEdge->getHeadNode()));
            xmllinks.appendChild(xmllink);
        }

        QFile file(fileName);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
                return false;
        QTextStream out(&file);
                out << doc.toString() << endl;
//        qDebug("MTGXML file saved");
    }
    return true;
}

bool Graph::saveGraphToNtg(QString fileName)
{
    QDomDocument doc("MtgXml");
    QDomElement root = doc.createElement("NtgXml");
    root.setAttribute("graphId",getGraphId());
    doc.appendChild(root);
    QDomElement xmlentities = doc.createElement("EntitySet");
    root.appendChild(xmlentities);
    QDomElement xmledges = doc.createElement("EdgeSet");
    root.appendChild(xmledges);

    for(int i = 0; i<_nodeList.size();++i)
    {
        Node * pNode = _nodeList.at(i);
        QDomElement xmlentity = doc.createElement("Entity");
        xmlentity.setAttribute("id",pNode->getId());
        xmlentity.setAttribute("type",pNode->getEntity().type);

//        qDebug() << pNode->getEntity().values;
        QHashIterator<QString,QString> itrValuesHash(pNode->getEntity().values);
        while(itrValuesHash.hasNext())
        {
            itrValuesHash.next();
            QDomElement xmlvalue = doc.createElement("property");
            xmlvalue.setAttribute("key",itrValuesHash.key());
            xmlvalue.setAttribute("value",itrValuesHash.value());
            xmlentity.appendChild(xmlvalue);
        }

        xmlentities.appendChild(xmlentity);

        QSetIterator<Edge*> itrEdge(pNode->getOutEdgeSet());
        while(itrEdge.hasNext())
        {
            Edge * pEdge = itrEdge.next();
            Node * tNode = pEdge->getHeadNode();
            QDomElement xmledge = doc.createElement("Edge");
            xmledge.setAttribute("from",_nodeList.indexOf(pNode));
            xmledge.setAttribute("to",_nodeList.indexOf(tNode));
            //xmledge.setAttribute("transform",pEdge->getTransformName());
            xmledges.appendChild(xmledge);
        }
    }

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
            return false;
    QTextStream out(&file);
            out << doc.toString() << endl;
//    qDebug("NTGXML file saved");
    return true;
}


bool Graph::saveGraphToZip(QString fileName)
{
    FileManager::getOrCreate()->checkFileInCopyPath(this);
    QDomDocument doc("NtgXml");
    QDomElement root = doc.createElement("NtgXml");
    root.setAttribute("graphId",getGraphId());
    doc.appendChild(root);
    QDomElement xmlentities = doc.createElement("EntitySet");
    root.appendChild(xmlentities);
    QDomElement xmledges = doc.createElement("EdgeSet");
    root.appendChild(xmledges);

    for(int i = 0; i<_nodeList.size();++i)
    {
        Node * pNode = _nodeList.at(i);
        QDomElement xmlentity = doc.createElement("Entity");
        xmlentity.setAttribute("id",pNode->getId());
        xmlentity.setAttribute("type",pNode->getEntity().type);
        xmlentity.setAttribute("svgId",pNode->getSvgId());

//        qDebug() << pNode->getEntity().values;
        QHashIterator<QString,QString> itrValuesHash(pNode->getEntity().values);
        while(itrValuesHash.hasNext())
        {
            itrValuesHash.next();
            QDomElement xmlvalue = doc.createElement("property");
            xmlvalue.setAttribute("key",itrValuesHash.key());
            xmlvalue.setAttribute("value",itrValuesHash.value());
            xmlentity.appendChild(xmlvalue);
        }

        foreach(NtgFile fileStruct, pNode->getFileMap())
        {
            QDomElement xmlvalue = doc.createElement("file");
            xmlvalue.setAttribute("id",fileStruct.id);
            xmlvalue.setAttribute("shortName",fileStruct.shortName);
            xmlentity.appendChild(xmlvalue);
        }

        xmlentities.appendChild(xmlentity);

        QSetIterator<Edge*> itrEdge(pNode->getOutEdgeSet());
        while(itrEdge.hasNext())
        {
            Edge * pEdge = itrEdge.next();
            Node * tNode = pEdge->getHeadNode();
            QDomElement xmledge = doc.createElement("Edge");
            xmledge.setAttribute("from",_nodeList.indexOf(pNode));
            xmledge.setAttribute("to",_nodeList.indexOf(tNode));
            //xmledge.setAttribute("transform",pEdge->getTransformName());
            xmledges.appendChild(xmledge);
        }
    }

    FileManager::getOrCreate()->saveTmpDir(fileName,this,doc);

    qDebug("NTG file saved");
    return true;
}


void Graph::exportToCsv(QString fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Text))
    {
        qWarning("exportToCsv fail");
    }

    QTextStream stream(&file);
    stream << "EntityType,parameterKey:parameterValue,parameterKey:parameterValue,..\n";

    foreach(Node * pNode, _nodeList)
    {
        NtgEntity entity = pNode->getEntity();
        QString line = entity.type + ",";

        QHashIterator<QString, QString> itrValues(entity.values);
        while(itrValues.hasNext())
        {
            itrValues.next();
            line += itrValues.key() + ":";

            QString value = itrValues.value();
            if(value.contains(",")||value.contains("\n"))
            {
                value.prepend("\"");
                value.append("\"");
            }
            line += value + ",";
        }        
        line.chop(1);

        stream << line << "\n";

    }
    file.close();

}

void Graph::destroyGraph(Graph * graph)
{
    delete graph;
}

QString Graph::getName(void) const
{
    return _name;
}

void Graph::setName(const QString & name)
{
    _name = name;
}

QString Graph::getFileName(void) const
{
    return _fileName;
}

void Graph::setFileName(const QString & fileName)
{
    QString name = QFileInfo(fileName).fileName();
    if (!name.isEmpty())
    {
      _fileName = fileName;
      _name = name;
    }
}

QString Graph::getUuid(void) const
{
    return _uuid;
}

GraphView * Graph::getView(void) const
{
    return _view;
}

void Graph::setGraphView(GraphView * view)
{
    delete _view;
    _view = view;
}

Node * Graph::newNode()
{
    Node * node = new Node(this,_nextNodeId++);
    _nodeList.append(node);
    _changed =true;
    return node;
}

Node * Graph::newNodeWithId(uint id)
{
    Node * node = new Node(this,id);
    if (id>=_nextNodeId)
        _nextNodeId= id +1;
    _nodeList.append(node);
    _changed =true;
    return node;
//    return newNode();
}

Edge * Graph::newEdge(Node *tail,Node *head)
{
    if(tail==head)
        return NULL;
    foreach(Edge * pEdge, _edgeSet)
    {
        if(pEdge->getTailNode()==tail && pEdge->getHeadNode()==head)
            return NULL;
    }

    Edge * edge = new Edge(this,tail,head);
    _edgeSet.insert(edge);

    _changed =true;
    tail->addOutEdge(edge);
    head->addInEdge(edge);
    return edge;
}

QList<Node *> Graph::getNodeList()
{
    return _nodeList;
}

void Graph::redraw()
{
    QListIterator<Node*> i(_nodeList);
    while(i.hasNext())
    {
        Node *pNode = i.next();
        pNode->redraw();
    }

}

QSet<Edge *> Graph::getEdgeSet()
{
    return _edgeSet;
}

GraphFrame * Graph::getGraphFrame()
{
    return _graphFrame;
}

void Graph::setGraphFrame(GraphFrame * frame)
{
    _graphFrame = frame;
}

QRectF Graph::getBoundingBox()
{
    QPolygonF p;
    QListIterator<Node*> i(_nodeList);
    while(i.hasNext())
    {
        p << i.next()->getPos();
    }
    QRectF bbox = p.boundingRect();
    i.toFront();
    while(i.hasNext())
    {
        Node *pNode = i.next();
        pNode->setPos(pNode->getPos()-bbox.center());
    }
    bbox.translate(-bbox.center());
    return bbox;
}

void Graph::deleteNode(Node* node)
{
    QGraphicsScene * pScene = this->getView()->scene();
    QSet<Edge* > edgeSet = node->getOutEdgeSet()+node->getInEdgeSet();
    QSetIterator<Edge*> itrEdgeSet(edgeSet);
    while(itrEdgeSet.hasNext())
    {
        Edge * pEdge = itrEdgeSet.next();
        _edgeSet.remove(pEdge);
        _changed =true;
        pScene->removeItem(pEdge->getItem());
        delete pEdge;
    }

    _nodeList.removeOne(node);
    _changed =true;
    pScene->removeItem(node->getNtgItem());

    foreach(NtgFile file, node->getFileMap())
    {
        node->removeFileFromMap(file.id);
    }
    delete node;
}

void Graph::deleteEdge(Edge* edge)
{
    QGraphicsScene * pScene = this->getView()->scene();
    _edgeSet.remove(edge);
    _changed =true;
    pScene->removeItem(edge->getItem());
    delete edge;
}

QList<Node *> Graph::getSelectedNodeList()
{
    QList<Node *> selectedNodes;
    QListIterator<Node*> itrNode(_nodeList);
    while(itrNode.hasNext())
    {
        Node * pNode = itrNode.next();
        if(pNode->getNtgItem()->isSelected())
            selectedNodes.append(pNode);
    }
    return selectedNodes;
}

QSet<Edge*> Graph::getSelectedEdgeSet()
{
    QSet<Edge*> selectedEdges;
    QSetIterator<Edge*> itrEdgeSet(_edgeSet);
    while(itrEdgeSet.hasNext())
    {
        Edge * pEdge = itrEdgeSet.next();
        if(pEdge->getItem() && pEdge->getItem()->isSelected())
            selectedEdges.insert(pEdge);
    }
    return selectedEdges;
}

void Graph::startLayout()
{
    _view->setFitInView(true);
    _view->startLayout();
}


Node * Graph::addNodeFromTransform(QString transformLongName, Node * parentNode, NtgEntity entity,NtgEntityModel model)
{
    Node * similarNode = NULL;
    foreach(Node * node,_nodeList)
    {
        if(node->isSimilitarTo(model,entity))
        {
            //qDebug("isSimilar");
            similarNode = node;
            break;
        }
    }

    Node * node;
    if (similarNode == NULL)
    {
        node = newNode();
        node->setEntity(entity);
        node->setEntityModel(model);
//        node->setFromTransform(transformLongName);

        NtgNodeItem * nodeItem = node->newNtgItem();
        if (parentNode!=0)
            nodeItem->setPos(parentNode->getNtgItem()->pos());
        _view->scene()->addItem(nodeItem);
        if (parentNode!=0)
        {
            Edge * edge = newEdge(parentNode,node);
            if (edge != NULL)
            {
                edge->setTransformName(transformLongName);
                EdgeItem * edgeItem = edge->newItem();
                _view->scene()->addItem(edgeItem);
            }
        }
    }
    else
    {
        node = similarNode;
        if (parentNode!=0)
        {
            Edge * edge = newEdge(parentNode,node);
            if(edge!=NULL)
            {
                edge->setTransformName(transformLongName);
                EdgeItem * edgeItem = edge->newItem();
                _view->scene()->addItem(edgeItem);
            }
        }
        else {}
        node->mergeWith(model,entity);
    }

    return node;
}

int Graph::getGraphId()
{
  return _graphId;
}

void Graph::setGraphId(int graphId)
{
  _graphId = graphId;
}

void Graph::launchTransform(QList<Node*> nodeList, QString transformNameId)
{
  NtgEntity entity;
  NtgTransformModel model = ModelsManager::getOrCreate()->getTransform(transformNameId);
  ManageXmlFile * xmlFile = new ManageXmlFile();
//  QSettings settings;
  QMap<QString,QString> parametersMapString;
  QVariantMap parameters;

  if (!model.params.isEmpty())
  {
    if (xmlFile->contains(transformNameId))
    {
        parametersMapString = xmlFile->getTransformFromXmlFile(transformNameId);
        foreach(QString key, parametersMapString.keys())
        {
            parameters.insert(key,QVariant(parametersMapString.value(key)));
        }

//        parameters = settings.value(transformNameId).toMap();
    }
    else
    {
        TransformPopWindow * transformDialog = new TransformPopWindow(model,MainWindow::get());
      transformDialog->exec();
      if (!transformDialog->hasToDoTransform())
          return;
      parameters=transformDialog->getParams();
      delete transformDialog;
    }
  }

  _transformTimer->addTransform(transformNameId,this,nodeList,parameters);

//  foreach(Node* node, _nodeList)
//  {
//    if (node->getId() == (unsigned int)nodeId)
//    {
////      entity = node->getEntity();
//      _transformTimer->addTransform(transformNameId,this,node,parameters);
//    }
//  }

//  qDebug() << "launch transform :" << transformNameId << "with params" << parameters << "in node with entity :" << entity;
    delete xmlFile;
}


void Graph::execContextMenu(QPoint pos)
{
    if(_view!=NULL)
        _view->execContextMenu(pos);
}

void Graph::setChanged(bool changed)
{
    _changed = changed;
}

bool Graph::hasChanged()
{
    return _changed;
}


Node * Graph::similarNode(Node * pNode)
{
    NtgEntityModel model = pNode->getEntityModel();
    NtgEntity entity = pNode->getEntity();
    Node * similarNode = NULL;
    foreach(Node * node,_nodeList)
    {
        if(node!=pNode && node->isSimilitarTo(model,entity))
        {
            //qDebug("isSimilar");
            similarNode = node;
            break;
        }
    }
    return similarNode;
}

TransformTimer * Graph::getTransformTimer()
{
    return _transformTimer;
}



/****************************************************************************
**  class Node
****************************************************************************/



Node::Node(Graph * graph, uint id)
    : QObject(graph), _graph(graph),_id(id),_ntgItem(NULL),_properties(NULL),_treeModel(NULL),_timer(NULL),_animation(NULL)
{
    _size = 72;
    _treeModel = new TreeModel(this);
    _nextFileId =0;

//    QStringList headers;
//    headers << tr("Name") << tr("Value");

    _timer = new QTimeLine(2000);
    _timer->setCurveShape(QTimeLine::EaseInOutCurve);
    _timer->setFrameRange(0, 100);

    _svgId= -1;
}

Node::Node(const Node & node) : QObject(node._graph), _graph(node._graph),_id(node._id),_ntgItem(node._ntgItem),_properties(node._properties),_treeModel(node._treeModel),_timer(node._timer),_animation(node._animation)
{
    _size = node._size;
}

Node::~Node()
{

    emit destroyedNode(this);
    if (_timer)
        delete _timer;
    delete _treeModel;
}

Graph * Node::getGraph() const
{
    return _graph;
}

uint Node::getId() const
{
    return _id;
}

NtgNodeItem * Node::getNtgItem() const
{
    return _ntgItem;
}


NtgNodeItem * Node::newNtgItem()
{
  NtgNodeItem * item = new NtgNodeItem(this);
  _ntgItem = item;
  connect(item,SIGNAL(redrawItem(NtgNodeItem*)),getGraph()->getView(),SLOT(redrawAnItem(NtgNodeItem *)));
  return _ntgItem;
}

void Node::setAttributes(QString name, QString sid, QString value)
{
    _name = name; _sid = sid; _value = value;
    if(_ntgItem!=NULL) _ntgItem->setToolTip(_name.append(" : ").append(_value));
}

QString Node::getName()
{
    return _name;
}

QString Node::getValue()
{
    return _value;
}

void Node::redraw()
{
/*
    if (_item)
    {
        _timer->stop();
        _item->setRect(-_size/2,-_size/2,_size,_size);


        _animation = new QGraphicsItemAnimation;
        _animation->setItem(_item);
        _animation->setTimeLine(_timer);

        _animation->setPosAt(0.0, _item->pos());

        QPoint p(0,0);
        _animation->setPosAt(1.0, getPos());

        _timer->start();
    }
*/
    if (_ntgItem)
    {
        _timer->stop();
        _ntgItem->setRect(-_size/2,-_size/2,_size,_size);

        if(!_animation)
            _animation = new QGraphicsItemAnimation(this);
        _animation->setItem(_ntgItem);
        _animation->setTimeLine(_timer);

        _animation->setPosAt(0.0, _ntgItem->pos());

        QPoint p(0,0);
        _animation->setPosAt(1.0, getPos());

        _timer->start();
    }
}

void Node::setPos(QPointF pos)
{
    _pos = pos;
}

QPointF Node::getPos()
{
    return _pos;
}

void Node::setSize(int s)
{
    _size = s;
}

int Node::getSize()
{
    return _size;
}

void Node::addOutEdge(Edge* e)
{
    _outEdgeSet.insert(e);
    _treeModel->changed(true);
}

void Node::addInEdge(Edge* e)
{
    _inEdgeSet.insert(e);
    _treeModel->changed(true);
}

QSet<Edge*>& Node::getOutEdgeSet()
{
    return _outEdgeSet;
}

QSet<Edge*>& Node::getInEdgeSet()
{
    return _inEdgeSet;
}


QSet<Node*> Node::neighbors()
{
    QSet<Node*> neighbors;
    QSetIterator<Edge*> itrInEdgeSet(_inEdgeSet);
    while(itrInEdgeSet.hasNext())
        neighbors.insert(itrInEdgeSet.next()->getTailNode());
    QSetIterator<Edge*> itrOutEdgeSet(_outEdgeSet);
    while(itrOutEdgeSet.hasNext())
        neighbors.insert(itrOutEdgeSet.next()->getHeadNode());
    return neighbors;
}

QSet<Node*> Node::parentsNodes()
{
    QSet<Node*> parents;
    QSetIterator<Edge*> itrEdge(_inEdgeSet);
    while(itrEdge.hasNext())
    {
           parents.insert(itrEdge.next()->getTailNode());
    }
    return parents;
}

QSet<Node*> Node::childsNodes()
{
    QSet<Node*> childs;
    QSetIterator<Edge*> itrEdge(_outEdgeSet);
    while(itrEdge.hasNext())
    {
           childs.insert(itrEdge.next()->getHeadNode());
    }
    return childs;
}

NtgEntityModel Node::getEntityModel()
{
    return _model;
}

void Node::setEntityModel(NtgEntityModel entityModel)
{
    _entityValues.type = entityModel.name;
    _model = entityModel;
}


NtgEntity Node::getEntity()
{
    return _entityValues;
}

void Node::setEntity(NtgEntity entity)
{
    _entityValues = entity;
    if(_ntgItem && entity.values.contains("value"))
    {
        _ntgItem->setText(entity.values.value("value"));
    }
}

void Node::addToEntity(QString type, QString value)
{
    _entityValues.values.insert(type,value);
    MainWindow::get()->updateDocks();
    _treeModel->changed(true);

    if (type == "value")
    {
        _ntgItem->setText(value);
        _ntgItem->setCurrentToolTip(value);
    }

    Node * similarNode = _graph->similarNode(this);
    if(similarNode!=NULL)
    {
//        qDebug("merge");
        mergeWith(similarNode);
    }


}


TreeModel * Node::getOrCreateTreeModel()
{
    if (_treeModel->hasChanged())
    {
        if(_treeModel!=NULL)
            delete _treeModel;
        _treeModel = new TreeModel(this);
        _treeModel->changed(false);
    }
    return _treeModel;
}

void Node::update()
{
    _treeModel->changed(true);
    MainWindow::get()->updateDocks();
//    _treeModel->hasChanged();
    //_treeModel = new TreeModel(this);
}

//QList<QString> Node::getFromTransform()
//{
////    return _fromTransform;
//}

//void Node::setFromTransform(QString fromTransform)
//{
////    _fromTransform.append(fromTransform);
//}

bool Node::isSimilitarTo(NtgEntityModel otherEntityModel, NtgEntity otherEntity)
{
    if(otherEntityModel.name == "unknown")
        return false;

    bool sameType = getEntityModel().name == otherEntityModel.name;
    bool parentType = getEntityModel().parent == otherEntityModel.name;
    bool childType = getEntityModel().name == otherEntityModel.parent;

    //qDebug() << getEntityModel().name << " :" << getEntityModel().parent ;

    NtgEntityModel sourceModel;

    if(sameType || childType)
    {
        sourceModel=otherEntityModel;
    }
    else if(parentType)
    {
        sourceModel=getEntityModel();
    }
    else
        return false;

    foreach(NtgTransformParam param, sourceModel.params)
    {
        if(param.isOptional==false)
        {
            bool sameParamName = getEntity().values.contains(param.name) && otherEntity.values.contains(param.name);
            bool sameParamValue = QString::compare(getEntity().values.value(param.name).simplified(),//.remove(QRegExp("[^\\w\\s\\.]")),
                                                   otherEntity.values.value(param.name).simplified(),//.remove(QRegExp("[^\\w\\s\\.]")),
                                                   Qt::CaseInsensitive) == 0;
            bool noSuchParamValue1 = !getEntity().values.contains(param.name) ||
                                    getEntity().values.value(param.name).simplified().isEmpty();
            bool noSuchParamValue2 = !otherEntity.values.contains(param.name) ||
                                    otherEntity.values.value(param.name).simplified().isEmpty();

            if ((sameParamName && sameParamValue) ||
                (noSuchParamValue1 && noSuchParamValue2))
            {
                //qDebug("==");
            }
            else
            {
                //qDebug("!=");
                return false;
            }
        }
    }

//    qDebug("similar!");
    return true;
}

void Node::mergeWith(NtgEntityModel otherEntityModel, NtgEntity otherEntity)
{
    if(otherEntityModel.name == "unknown")
        return ;

    bool sameType = getEntityModel().name == otherEntityModel.name;
    bool parentType = getEntityModel().parent == otherEntityModel.name;
    bool childType = getEntityModel().name == otherEntityModel.parent;

    NtgEntity srcEntity, destEntity;
    NtgEntityModel srcModel, destModel;

    if(sameType || childType)
    {        
        destEntity = otherEntity;
        destModel = otherEntityModel;
        srcEntity = getEntity();
        srcModel = getEntityModel();
    }
    else if(parentType)
    {
        destEntity = getEntity();
        destModel = getEntityModel();
        srcEntity = otherEntity;
        srcModel = otherEntityModel;
    }
    else
        return;

    foreach(NtgTransformParam param, srcModel.params)
    {
        if(param.isOptional==true)
        {
            if(srcEntity.values.contains(param.name) &&
               (!destEntity.values.contains(param.name)  || destEntity.values.value(param.name).simplified()=="")
                )
            {
                destEntity.values.insert(param.name,srcEntity.values.value(param.name));
            }
        }
    }

    setEntity(destEntity);
    setEntityModel(destModel);
}

void Node::mergeWith(Node * similarNode)
{
    mergeWith(similarNode->getEntityModel(),similarNode->getEntity());

    _ntgItem->init();
    _graph->getView()->redrawItem(_ntgItem);

    foreach(Edge* pEdge, similarNode->getInEdgeSet())
    {
        Edge * edge = _graph->newEdge(pEdge->getTailNode(),this);
        if(edge !=  NULL)
        {
            edge->setTransformName(pEdge->getTransformName());
            EdgeItem * edgeItem = edge->newItem();
            _graph->getView()->scene()->addItem(edgeItem);
        }
    }
    foreach(Edge* pEdge, similarNode->getOutEdgeSet())
    {
        Edge * edge = _graph->newEdge(this,pEdge->getHeadNode());
        if(edge !=  NULL)
        {
            edge->setTransformName(pEdge->getTransformName());
            EdgeItem * edgeItem = edge->newItem();
            _graph->getView()->scene()->addItem(edgeItem);
        }
    }

    _graph->deleteNode(similarNode);
}


void Node::addFileOnlyToMap(NtgFile fileStruct)
{
    _fileMap.insert(fileStruct.id,fileStruct);
    if (fileStruct.id>=_nextFileId)
        _nextFileId = fileStruct.id +1;
}

int Node::addFile(QString path)
{
    if (_entityValues.values.contains("value"))
    {
//        QString NodeValueId =_entityValues.values.value("value").simplified().replace(" ",".");
//        QString key =NodeValueId+"-|-"+path.split("/").last();
        QFileInfo fileinfo(path);
        NtgFile file;
        if (fileinfo.isFile())
        {
            file.id = _nextFileId++;
            file.srcPath = fileinfo.absoluteFilePath();
            file.shortName = fileinfo.fileName();
            qreal t = fileinfo.size();
            file.size = QString::number(t/1000);
            _fileMap.insert(file.id,FileManager::getOrCreate()->addFile(file,this));

        }
        else
            return -1;
        _treeModel->changed(true);
        MainWindow::get()->updateDocks();
        _ntgItem->setSvg(_svgId);
        getGraph()->getView()->redrawItem(_ntgItem);
        getGraph()->setChanged(true);
        return file.id;

    }
    else
    {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::information(0, tr("no value setted"), tr("you can't add files to an empty entity"));
        return -1;
    }
}

QMap<int,NtgFile> Node::getFileMap()
{
    return _fileMap;
}

void Node::removeFileFromMap(int key)
{
    FileManager::getOrCreate()->removeFile(_fileMap.take(key),this);
    _treeModel->changed(true);
    MainWindow::get()->updateDocks();
    _ntgItem->setSvg(_svgId);
    getGraph()->getView()->redrawItem(_ntgItem);
    getGraph()->setChanged(true);

}

int Node::getSvgId()
{
    return _svgId;
}

void Node::setSvgId(int svgId)
{
    _svgId = svgId;
}


/****************************************************************************
**  class Edge
****************************************************************************/

Edge::Edge(Graph * graph, Node * tail, Node * head)
    : QObject(graph), _graph(graph), _tailNode(tail), _headNode(head), _item(NULL)
{

}

Edge::~Edge()
{
    if(_tailNode)
    {
        _tailNode->getOutEdgeSet().remove(this);
        _tailNode->getOrCreateTreeModel()->changed(true);
    }
    if(_headNode)
    {
        _headNode->getInEdgeSet().remove(this);
        _headNode->getOrCreateTreeModel()->changed(true);
    }
}

Graph * Edge::getGraph() const
{
    return _graph;
}

uint Edge::getId() const
{
    return _id;
}

EdgeItem * Edge::getItem() const
{
    return _item;
}

Node * Edge::getTailNode() const
{
    return _tailNode;
}

Node * Edge::getHeadNode() const
{
    return _headNode;
}

EdgeItem * Edge::newItem()
{
  EdgeItem * item = new EdgeItem(this);
  _item = item;
  return _item;
}

QString Edge::getTransformName()
{
    return _transformName;
}

void Edge::setTransformName(QString name)
{
    _transformName = name;
}


//GREPTHIS
//Q_DECLARE_METATYPE(Graph*)


