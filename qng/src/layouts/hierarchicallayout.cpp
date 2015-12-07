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
#include "QTime"
#include "QTextStream"

HierarchicalLayout::HierarchicalLayout(Graph* graph,AbstractLayout::ViewMod viewMod,  AbstractLayout::GroupMod groupMod) : AbstractLayout(graph,viewMod,groupMod)
{
}

QRectF HierarchicalLayout::layout()
{
    QTime t;
    t.start();
    QSet<ConnectedComponent*> connectedComponents = retrieveConnectedComponents(_graph->getNodeList());

    QSetIterator<ConnectedComponent*> itrConnectedComponents(connectedComponents);
    while(itrConnectedComponents.hasNext())
    {
        ConnectedComponent * pConnectedComponent = itrConnectedComponents.next();
        pConnectedComponent->createSuperNodes(_groupMods);
        pConnectedComponent->createAgnodes(_viewMod);
        pConnectedComponent->createAgedges();

        agsafeset(pConnectedComponent->_gvG,(char*)"nodesep",(char*)"2",(char*)"5");
        agsafeset(pConnectedComponent->_gvG,(char*)"ranksep",(char*)"2",(char*)"5");
        agsafeset(pConnectedComponent->_gvG,(char*)"charset",(char*)"latin1",(char*)"UTF-8");
        agsafeset(pConnectedComponent->_gvG,(char*)"splines",(char*)"false",(char*)"true");
        agsafeset(pConnectedComponent->_gvG,(char*)"rankdir",(char*)"LR",(char*)"TB");
        gvLayout(pConnectedComponent->_gvC,pConnectedComponent->_gvG,(char*)"dot");

        pConnectedComponent->positionNodes();
    }

    repack(connectedComponents);
    qDeleteAll(connectedComponents);
    return centerBoundingBox();
}
