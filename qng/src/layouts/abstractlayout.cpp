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

#include "hierarchicallayout.h"
#include "graph.h"
#include "QTime"
#include <QTextStream>
#include <cmath>


bool SuperNode::hSameParentsAndSameChilds(const SuperNode* spn) const
{
    return spn->_type==this->_type && spn->_childSet==this->_childSet && spn->_parentSet==this->_parentSet;
}
bool SuperNode::hSameParentsAndNoChilds(const SuperNode* spn) const
{
    return spn->_type==this->_type && spn->_childSet.size()==0 && this->_childSet.size()==0 && spn->_parentSet==this->_parentSet;
}
bool SuperNode::hNoParentsAndNoChilds(const SuperNode* spn) const
{
    return spn->_type==this->_type && spn->_childSet.size()==0 && this->_childSet.size()==0 && spn->_parentSet.size()==0 && this->_parentSet.size()==0;
}

// Connected Component

ConnectedComponent::ConnectedComponent()
{    
    _gvC = gvContext();
    ///*
    QString str = "mygraph" + QString::number(qrand());
    std::string s = str.toStdString();
    _gvG = agopen((char*)s.c_str(),AGDIGRAPH);//*/
    //_gvG = agopen((char*)"mygraph",AGDIGRAPH);
}

ConnectedComponent::~ConnectedComponent()
{    
    gvFreeLayout(_gvC,_gvG);
    agclose(_gvG);
    gvFreeContext(_gvC);
    qDeleteAll(_superNodeList);
}

void ConnectedComponent::createSuperNodes(AbstractLayout::GroupMods groupMod)
{
    QListIterator<Node*> itrNodeList(_nodeList);
    while(itrNodeList.hasNext())
    {
        Node * pNode = itrNodeList.next();
        SuperNode * superNode = new SuperNode();
        if(pNode->getEntity().type!="")
            superNode->_type = pNode->getEntity().type;
        else
            superNode->_type = pNode->getName();
        QSetIterator<Edge*> tmpItrEdgeIn(pNode->getInEdgeSet());
        while(tmpItrEdgeIn.hasNext())
            superNode->_parentSet.insert(tmpItrEdgeIn.next()->getTailNode());
        QSetIterator<Edge*> tmpItrEdgeOut(pNode->getOutEdgeSet());
        while(tmpItrEdgeOut.hasNext())
            superNode->_childSet.insert(tmpItrEdgeOut.next()->getHeadNode());
        superNode->_nodeSet.insert(pNode);


        bool match = false;
        SuperNode * pSuperNode = NULL;

        QListIterator<SuperNode*> itrSuperNodeList(_superNodeList);
        while(itrSuperNodeList.hasNext())
        {
            pSuperNode = itrSuperNodeList.next();
            if((groupMod & AbstractLayout::NoParentsAndChilds && pSuperNode->hNoParentsAndNoChilds(superNode))
                || (groupMod & AbstractLayout::SameParentsNoChilds&&pSuperNode->hSameParentsAndNoChilds(superNode))
                || (groupMod & AbstractLayout::SameParentsAndChilds&&pSuperNode->hSameParentsAndSameChilds(superNode)))
            {
                match = true;
                pSuperNode->_nodeSet.insert(pNode);
                delete superNode;
                superNode = pSuperNode;
                break;
            }
        }

        if(!match)
        {
            if(pNode->getInEdgeSet().size() == 0)
            {
                _superNodeList.insert(0,superNode);
            }
            else
            {
                _superNodeList.append(superNode);
            }
        }
    }
}

void ConnectedComponent::calcBetweenessCentrality(double * betweenessTab)
{
    unsigned int nbrNode = _nodeList.size();

    //adjacency list
    QList<int> adjacencyList[nbrNode];
    for(unsigned int i(0); i<nbrNode; ++i)
    {
        QSetIterator<Node*> itrNeighbors(_nodeList.at(i)->neighbors());
        while(itrNeighbors.hasNext())
        {
            adjacencyList[i].append(_nodeList.indexOf(itrNeighbors.next()));
        }
    }

    //global init
    for(unsigned int i(0);i<nbrNode;++i)
    {
        betweenessTab[i]=0;
    }

    //Updating Algorithm
    for(unsigned int s(0); s<nbrNode; ++s)
    {
        //init
        QList<int> predesc[nbrNode];
        int pathNumber[nbrNode];
        int depth[nbrNode];
        double centralityUpdate[nbrNode];
        for(unsigned int i(0); i<nbrNode; ++i)
        {
            pathNumber[i] = 0;
            depth[i] = -1;
            centralityUpdate[i] = 0;
        }
        depth[s] = 0; pathNumber[s] = 1;
        QList<int> openNodesQueue;
        QList<int> orderSeenStack;

        openNodesQueue.append(s);
        while(!openNodesQueue.isEmpty())
        {
            int v = openNodesQueue.first();
            openNodesQueue.removeFirst();

            orderSeenStack.append(v);
            QListIterator<int> itrNeighbors(adjacencyList[v]);
            while(itrNeighbors.hasNext())
            {
                int w = itrNeighbors.next();
                if(depth[w]<0) //found the first time ?
                {
                    openNodesQueue.append(w);
                    depth[w] = depth[v] + 1;
                }
                if(depth[w]==depth[v]+1)//shortest path to w via v ?
                {
                    pathNumber[w]+=pathNumber[v];
                    predesc[w].append(v);
                }
            }
        }

        //S returns vertices in order of non-increasing distance from s
        while(!orderSeenStack.isEmpty())
        {
            unsigned int w = orderSeenStack.last();
            orderSeenStack.removeLast();
            QListIterator<int> itrPredesc(predesc[w]);
            while(itrPredesc.hasNext())
            {
                int v = itrPredesc.next();
                centralityUpdate[v] += ((float)pathNumber[v]/(float)pathNumber[w])*(1.0+depth[w]);
            }
            if(w!=s) betweenessTab[w] += centralityUpdate[w];
        }
    }

    //scale
    double value = 200;
    double max = 0;
    for(unsigned int i(0); i<nbrNode; ++i)
    {
        if(betweenessTab[i]>max)
            max = betweenessTab[i];
    }
    if(max > value)
    {
        double factor = value/max;
        for(unsigned int i(0); i<nbrNode; ++i)
        {
            betweenessTab[i] *= factor;
        }
    }
}

void ConnectedComponent::createAgnodes(AbstractLayout::ViewMod viewMod)
{
    double bcWeight[_nodeList.size()];
    if(viewMod==AbstractLayout::Centrality)
    {
        QTime t; t.start();
        calcBetweenessCentrality(bcWeight);
        //qDebug("betweeness centrallity, time : %d ms",t.elapsed());
    }

    QListIterator<SuperNode*> itrSuperNodeList(_superNodeList);
    while(itrSuperNodeList.hasNext())
    {
        SuperNode * pSuperNode = itrSuperNodeList.next();
        QString id = QString("n").append(QString::number(pSuperNode->_nodeSet.values().first()->getId()));
        Agnode_t * gvnode = agnode(_gvG,id.toLatin1().data());
        pSuperNode->_gvnode = gvnode;

        int nbNode = pSuperNode->_nodeSet.size();
        int weightIn = 0;
        //int weightOut = 0;
        int cWeight = 0;
        QSetIterator<Node*> tmpItrNodeSet(pSuperNode->_nodeSet);
        while(tmpItrNodeSet.hasNext())
        {
            Node * pNode = tmpItrNodeSet.next();
            _nodeMap.insert(pNode,pSuperNode);
            if(viewMod==AbstractLayout::EdgeWeighted)
                weightIn += pNode->getInEdgeSet().size();
            //weightOut += pNode->getOutEdgeSet().size();
            else if(viewMod==AbstractLayout::Centrality)
                cWeight += bcWeight[_nodeList.indexOf(pNode)];
        }

        float f;
        if(viewMod==AbstractLayout::EdgeWeighted)
        {
            int weight = weightIn;
            f = (nbNode == 1)? weight+1.0:ceil(sqrt(nbNode))*(weight/nbNode+1.0) +1.0;
        }
        else if(viewMod==AbstractLayout::Centrality)
        {
          int weight = cWeight/10;
          f = (nbNode == 1)? weight+1.0:ceil(sqrt(nbNode))*(weight/nbNode+1.0) +1.0;
        }
        else // viewMod==AbstractLayout::Mining
        {
            f = (nbNode==1)?1.0:ceil(sqrt(nbNode)+1.0);
        }

        QString str;
        QTextStream qts(&str,QIODevice::ReadWrite);
        qts<< f;
        agsafeset(gvnode,(char*)"width",(char*)str.toLatin1().data(),(char*)"0.75");
        agsafeset(gvnode,(char*)"height",(char*)str.toLatin1().data(),(char*)"0.5");
        agsafeset(gvnode,(char*)"fixedsize",(char*)"true",(char*)"false");
    }
}

void ConnectedComponent::createAgedges()
{
    QSet<Edge*> alreadyCreated;
    QMapIterator<Node*,SuperNode*> itrSuperNodeMap(_nodeMap);
    while(itrSuperNodeMap.hasNext())
    {
        itrSuperNodeMap.next();
        QSet<Edge*> edgeSet = itrSuperNodeMap.key()->getInEdgeSet() + itrSuperNodeMap.key()->getOutEdgeSet();
        QSetIterator<Edge*> itrEdgeSet(edgeSet);
        while(itrEdgeSet.hasNext())
        {
            Edge * pEdge = itrEdgeSet.next();
            if(!alreadyCreated.contains(pEdge))
            {
                alreadyCreated.insert(pEdge);
                agedge(_gvG,_nodeMap.value(pEdge->getTailNode())->_gvnode,_nodeMap.value(pEdge->getHeadNode())->_gvnode);
            }
        }
    }
}

void ConnectedComponent::positionNodes()
{
    QPolygonF polygon;
    QListIterator<SuperNode*> itrSuperNode(_superNodeList);
    while(itrSuperNode.hasNext())
    {
        SuperNode * superNode = itrSuperNode.next();
        Agnode_t * gvnode = superNode->_gvnode;
        int nbnodes = superNode->_nodeSet.size();

        int width = ceil(sqrt(nbnodes));
        int height = ceil(nbnodes/(float)width);
        int size = ND_xsize(gvnode);
        if(width!=1) size = (ND_xsize(gvnode)-72) / width;

        QPointF topLeft(ND_coord(gvnode).x -ND_xsize(gvnode)/2, -ND_coord(gvnode).y -ND_ysize(gvnode)/2);
        int incrx = ND_xsize(gvnode)/(width);

        QPointF startingPoint;
        if(width>height)
            startingPoint = topLeft + QPointF(incrx/2,incrx);
        else
            startingPoint = topLeft + QPointF(incrx/2,incrx/2);

        int i=0,j=0;
        QSetIterator<Node*> tmpItrNode(superNode->_nodeSet);
        while(tmpItrNode.hasNext())
        {
            Node * tmpnode = tmpItrNode.next();
            QPointF pos = startingPoint+QPointF(i*incrx,j*incrx);

            tmpnode->setPos(pos);
            tmpnode->setSize(size);

            QPolygonF poly = QPolygonF(QRectF(pos.x()-size/2,pos.y()-size/2,size,size));
            polygon << poly;

            ++i;
            if(i%width == 0)
            {
                j++;i=0;
            }
        }
    }
    _boundingBox = polygon.boundingRect();
}


//AbstractLayout


AbstractLayout::AbstractLayout(Graph* graph,AbstractLayout::ViewMod viewMod,AbstractLayout::GroupMods groupMods,AbstractLayout::SpanMod spanMod):_graph(graph),_viewMod(viewMod),_groupMods(groupMods),_spanMod(spanMod)
{

}

QSet<ConnectedComponent*> AbstractLayout::retrieveConnectedComponents(QList<Node*> nodeList)
{
    QSet<ConnectedComponent*> connectedComponentSet;
    ConnectedComponent * soloNodesComponent = new ConnectedComponent;


    QList<Node*> openNodeList;

    while(!nodeList.isEmpty())
    {
        Node * pNode = nodeList.first();

        nodeList.removeOne(pNode);      //open node
        openNodeList.append(pNode);
        ConnectedComponent * connectedComponent = new ConnectedComponent;
        while(!openNodeList.isEmpty())
        {
            Node * nNode = openNodeList.first();
            QSet<Node*> neighbors;
            QSetIterator<Edge*> tmpItrEdgeIn(nNode->getInEdgeSet());
            while(tmpItrEdgeIn.hasNext())
                neighbors.insert(tmpItrEdgeIn.next()->getTailNode());
            QSetIterator<Edge*> tmpItrEdgeOut(nNode->getOutEdgeSet());
            while(tmpItrEdgeOut.hasNext())
                neighbors.insert(tmpItrEdgeOut.next()->getHeadNode());

            QSetIterator<Node*> itrNeighbors(neighbors);
            while(itrNeighbors.hasNext())
            {
                Node * mNode = itrNeighbors.next();
                if(nodeList.contains(mNode))
                {
                    nodeList.removeOne(mNode);  //open node
                    openNodeList.append(mNode);
                }
            }
            openNodeList.removeOne(nNode);  //close node
            connectedComponent->_nodeList.append(nNode);
        }
        if((_groupMods & AbstractLayout::NoGroup)==0 && connectedComponent->_nodeList.size()==1)
        {
            soloNodesComponent->_nodeList.append(connectedComponent->_nodeList.first());
            delete connectedComponent;
        }
        else
            connectedComponentSet.insert(connectedComponent);
    }

    if((_groupMods & AbstractLayout::NoGroup)==0 && soloNodesComponent->_nodeList.size()!=0)
        connectedComponentSet.insert(soloNodesComponent);
    else
        delete soloNodesComponent;

    return connectedComponentSet;
}

void AbstractLayout::repack(QSet<ConnectedComponent*>& connectedComponents)
{
    GVC_t * gvC = gvContext();
    Agraph_t * gvG = agopen((char*)"mygraph",AGDIGRAPH);

    QSetIterator<ConnectedComponent*> itrConnectedComponents(connectedComponents);
    while(itrConnectedComponents.hasNext())
    {
        ConnectedComponent * pConnectedComponent = itrConnectedComponents.next();
        QRectF brect = pConnectedComponent->_boundingBox;
        QString id = QString("c").append(QString::number(pConnectedComponent->_nodeList.first()->getId()));
        Agnode_t * gvnode = pConnectedComponent->_gvnode = agnode(gvG,(char*)id.toLatin1().data());
        float width = brect.width()/72;
        float height = brect.height()/72;
        QString strWidth;
        QTextStream qtsW(&strWidth,QIODevice::ReadWrite);
        qtsW<< width;
        QString strHeight;
        QTextStream qtsH(&strHeight,QIODevice::ReadWrite);
        qtsH<< height;
        agsafeset(gvnode,(char*)"width",(char*)strWidth.toLatin1().data(),(char*)"0.75");
        agsafeset(gvnode,(char*)"height",(char*)strHeight.toLatin1().data(),(char*)"0.5");
        agsafeset(gvnode,(char*)"fixedsize",(char*)"true",(char*)"false");
    }

    agsafeset(gvG,(char*)"nodesep",(char*)"2",(char*)"5");
    agsafeset(gvG,(char*)"ranksep",(char*)"2",(char*)"5");
    agsafeset(gvG,(char*)"charset",(char*)"latin1",(char*)"UTF-8");
    agsafeset(gvG,(char*)"overlap",(char*)"false",(char*)"true");
    gvLayout(gvC,gvG,(char*)"neato");

    itrConnectedComponents.toFront();
    while(itrConnectedComponents.hasNext())
    {
        ConnectedComponent * pConnectedComponent = itrConnectedComponents.next();
        Agnode_t * gvnode = pConnectedComponent->_gvnode;
        QPointF dec = QPointF(ND_coord(gvnode).x,ND_coord(gvnode).y);
        QRectF brect = pConnectedComponent->_boundingBox;
        QPointF adjust = QPointF(brect.bottomLeft());
        adjust += QPointF(brect.width()/2,-brect.height()/2);
        QListIterator<Node*> itrNodeList(pConnectedComponent->_nodeList);
        while(itrNodeList.hasNext())
        {
            Node * pNode = itrNodeList.next();
            QPointF pos = pNode->getPos() + dec - adjust;
            pNode->setPos(pos);
        }
    }

    gvFreeLayout(gvC,gvG);
    agclose(gvG);
    gvFreeContext(gvC);
}

QRectF AbstractLayout::centerBoundingBox()
{
    QPolygonF p;
    QListIterator<Node*> i(_graph->getNodeList());
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

AbstractLayout::ViewMod AbstractLayout::getViewMod()
{
    return _viewMod;
}

void AbstractLayout::setViewMod(ViewMod viewMod)
{
    _viewMod = viewMod;
}

void AbstractLayout::setGroupMod(GroupMods groupMods)
{
    _groupMods = groupMods;
}

AbstractLayout::GroupMods AbstractLayout::getGroupMod()
{
    return _groupMods;
}

void AbstractLayout::setSpanMod(SpanMod spanMod)
{
    _spanMod = spanMod;
}

AbstractLayout::SpanMod AbstractLayout::getSpanMod()
{
    return _spanMod;
}

void AbstractLayout::run()
{
    AbstractLayout::_mutex.lock();
    layout();
    AbstractLayout::_mutex.unlock();
}

QMutex AbstractLayout::_mutex;
