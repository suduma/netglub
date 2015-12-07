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

#ifndef OLAYOUT_H
#define OLAYOUT_H

#include "graph.h"
#include "graphviz/gvc.h"

class ConnectedComponent;

class BetweenessCentrality
{
public:
    int **_adjMatrix;
    QSet<int> ***_next2;
    double * _weight;
    unsigned int _nbrNode;
    QList<Node*> _nodeList;

    BetweenessCentrality(QList<Node*> nodeList);
    ~BetweenessCentrality();
    void init();
    double * proceedFloydWarshall();
    void path(int j, int i, int coeff);
    void normalize(int);
    double * proceedUpdating();
};

class SuperNode
{
public:
    QSet<Node*> _nodeSet;
    Agnode_t * _gvnode;
    QString _type;
    QSet<Node*> _parentSet;
    QSet<Node*> _childSet;

    bool hSameParentsAndSameChilds(const SuperNode* spn) const;
    bool hSameParentsAndNoChilds(const SuperNode* spn) const;
    bool hNoParentsAndNoChilds(const SuperNode* spn) const;
};

class AbstractLayout : public QThread
{
    Q_OBJECT
    Q_PROPERTY(ViewMod viewMod READ getViewMod WRITE setViewMod)
    Q_PROPERTY(SpanMod spanMod READ getSpanMod WRITE setSpanMod)
    Q_ENUMS(ViewMod)
    Q_ENUMS(SpanMod)
    //Q_PROPERTY(GroupMod groupMod READ getGroupMod WRITE setGroupMod)
    Q_FLAGS(GroupMod GroupMods)


public:
    enum ViewMod{Mining,Centrality,EdgeWeighted};
    enum GroupMod{SameParentsNoChilds=0x01,SameParentsAndChilds=0x02,NoParentsAndChilds=0x04,NoGroup=0x08};
    Q_DECLARE_FLAGS(GroupMods,GroupMod)
    enum SpanMod{SpanOn, SpanOff};

    static QMutex _mutex;

    AbstractLayout(Graph*,AbstractLayout::ViewMod viewMod = AbstractLayout::Mining, AbstractLayout::GroupMods groupMods = AbstractLayout::SameParentsAndChilds, AbstractLayout::SpanMod spanMod=AbstractLayout::SpanOff);

    virtual QRectF layout() = 0;
    QRectF centerBoundingBox();

    void setViewMod(ViewMod);
    ViewMod getViewMod();
    void setGroupMod(GroupMods);
    GroupMods getGroupMod();
    void setSpanMod(SpanMod);
    SpanMod getSpanMod();

protected:
    Graph * _graph;
    ViewMod _viewMod;
    GroupMods _groupMods;
    SpanMod _spanMod;

    QSet<ConnectedComponent*> retrieveConnectedComponents(QList<Node*>);
    void repack(QSet<ConnectedComponent*>&);
    virtual void run();

};

class ConnectedComponent
{
public:
    QList<Node*> _nodeList;
    QList<SuperNode*> _superNodeList;
    QMap<Node*,SuperNode*> _nodeMap;

    Agraph_t * _gvG;
    GVC_t * _gvC;
    QRectF _boundingBox;
    Agnode_t * _gvnode;

    ConnectedComponent();
    virtual ~ConnectedComponent();
    void createSuperNodes(AbstractLayout::GroupMods);
    void calcBetweenessCentrality(double*);
    void createAgnodes(AbstractLayout::ViewMod);
    void createAgedges();
    void positionNodes();


};


#endif // OLAYOUT_H
