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

#ifndef GRAPH_H
#define GRAPH_H

#include <QObject>
#include <QGraphicsEllipseItem>
#include <QSet>
#include <QMenu>
#include <QTableWidget>
#include <QGraphicsItemAnimation>
#include "ntgnodeitem.h"
#include "ntgTypes.h"
#include "dockWidget/treemodel.h"
#include "transformtimer.h"
#include "filemanager.h"


class Node;
class Edge;

class NtgNodeItem;

class EdgeItem;
class GraphView;
class GraphFrame;
class TreeModel;
class TransformTimer;

/****************************************************************************
**  class Graph
****************************************************************************/
class FileManager;
struct NtgFile;

class Graph : public QObject
{
Q_OBJECT
    Q_PROPERTY(QString _name READ getName WRITE setName)

public:    
    virtual ~Graph();
    static Graph * newGraph(QObject * parent = 0);
    static Graph * loadGraph(const QString & fileName, QObject * parent = 0);
    static bool loadGraphFromMtg(Graph * graph,QString fileName);
    static bool loadGraphFromNtg(Graph * graph, QString fileName);
    static bool loadGraphFromZip(Graph * graph, QString fileName);

    virtual bool saveGraph(const QString & fileName);
    bool saveGraphToMtg(QString fileName);
    bool saveGraphToNtg(QString fileName);
    bool saveGraphToZip(QString fileName);
    static void destroyGraph(Graph * graph);

    void exportToCsv(QString);

    virtual QString getName(void) const;
    virtual void setName(const QString & name);
    virtual QString getFileName(void) const;
    virtual void setFileName(const QString & name);
    virtual QString getUuid(void) const;

    GraphFrame * getGraphFrame();
    void setGraphFrame(GraphFrame *);
    virtual GraphView * getView(void) const;
    virtual void setGraphView(GraphView * view);

    virtual Node * newNode();
    virtual Node * newNodeWithId(uint id);
    virtual Node * addNodeFromTransform(QString transformLongName, Node * parentNode, NtgEntity entity,NtgEntityModel model);
    virtual Edge * newEdge(Node *tail,Node *head);
    void deleteNode(Node*);
    void deleteEdge(Edge*);

    virtual QRectF getBoundingBox();
    virtual void redraw();

    virtual Node * similarNode(Node *);

    QList<Node *> getNodeList();
    QSet<Edge *> getEdgeSet();

    virtual int getGraphId();
    virtual void setGraphId(int graphId);

    virtual void launchTransform(QList<Node *> nodeList, QString transformNameId);

    void execContextMenu(QPoint);

    QList<Node *> getSelectedNodeList();
    QSet<Edge*> getSelectedEdgeSet();

    void startLayout();

    void setChanged(bool changed);
    bool hasChanged();

    Graph(QObject * parent = 0);
    Graph(const Graph&);

    TransformTimer* getTransformTimer();

protected:
    static uint _newGraphId;
    uint _nextNodeId;
    QString _name;
    QString _fileName;
    QString _uuid;
    GraphView * _view;
    GraphFrame * _graphFrame;
    QList<Node *> _nodeList;
    QSet<Edge *> _edgeSet;
    bool _changeSinceLastSave;

    int _graphId; //master need it
    TransformTimer * _transformTimer;

    bool _changed; //to save it






private:
    //Graph(QObject * parent = 0);



};

/****************************************************************************
**  class Node
****************************************************************************/

class Node : public QObject
{
    Q_OBJECT
    //Q_PROPERTY(NtgEntityModel _model READ getEntityModel WRITE setEntityModel)
    //Q_PROPERTY(NtgEntity _entityValues READ getEntity WRITE setEntity)
public:
    Node(Graph * graph, uint id);
    Node(const Node&);
    virtual ~Node();

    virtual Graph * getGraph() const;
    virtual uint getId() const;

    virtual NtgNodeItem * getNtgItem() const;
    virtual NtgNodeItem * newNtgItem();

    void setAttributes(QString name, QString sid, QString value="");

    virtual QString getName();
    virtual QString getValue();

    virtual void redraw();

    void setPos(QPointF pos);
    QPointF getPos();
    void setSize(int);
    int getSize();

    void addOutEdge(Edge*);
    void addInEdge(Edge*);

    QSet<Edge*>& getOutEdgeSet();
    QSet<Edge*>& getInEdgeSet();
    QSet<Node*> neighbors();
    QSet<Node*> parentsNodes();
    QSet<Node*> childsNodes();

    NtgEntityModel getEntityModel();
    void setEntityModel(NtgEntityModel entityModel);

    NtgEntity getEntity();
    void setEntity(NtgEntity entity);

//    QList<QString> getFromTransform();
//    void setFromTransform(QString fromTransform);

    virtual void addToEntity(QString type, QString value);


    TreeModel * getOrCreateTreeModel();

    void update();

    bool isSimilitarTo(NtgEntityModel,NtgEntity);
    void mergeWith(NtgEntityModel,NtgEntity);
    void mergeWith(Node * node);

    void addFileOnlyToMap(NtgFile fileStruct); //to use with linkFiletomap from filemanager.

    int addFile(QString path);
    void removeFileFromMap(int key);
    QMap<int,NtgFile> getFileMap();

    int getSvgId();
    void setSvgId(int svgId);

signals:
    void destroyedNode(Node *);

private:
    Graph * _graph;
    uint _id;
    NtgNodeItem * _ntgItem;

    QString _name;
    QString _sid;
    QString _value;
    QPointF _pos;
    int _size;

    QSet<Edge*> _outEdgeSet;
    QSet<Edge*> _inEdgeSet;


    NtgEntityModel _model;
    NtgEntity _entityValues;

    QTableWidget * _properties;
    TreeModel * _treeModel;

//    QList<QString> _fromTransform;

    QTimeLine *_timer;
    QGraphicsItemAnimation *_animation;

    int _nextFileId;
    QMap<int,NtgFile> _fileMap;

    int _svgId;



};

/****************************************************************************
**  class Edge
****************************************************************************/

class Edge : public QObject
{
public:
    Edge(Graph * graph, Node * tail, Node * head);
    virtual ~Edge();

    virtual Graph * getGraph() const;
    virtual uint getId() const;
    virtual EdgeItem * getItem() const;
    virtual EdgeItem * newItem();
    virtual Node * getTailNode() const;
    virtual Node * getHeadNode() const;

    QString getTransformName();
    void setTransformName(QString);

private:
    Graph * _graph;
    Node * _tailNode;
    Node * _headNode;
    uint _id;
    EdgeItem * _item;
    QString _transformName;

};


#endif // GRAPH_H
