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

#include "molecularlayout.h"

MolecularLayout::MolecularLayout(Graph* graph,AbstractLayout::ViewMod viewMod,  AbstractLayout::GroupMod groupMod, AbstractLayout::SpanMod spanMod) : AbstractLayout(graph,viewMod,groupMod,spanMod)
{
}

QRectF MolecularLayout::layout()
{
    ConnectedComponent * pConnectedComponent = new ConnectedComponent();
    pConnectedComponent->_nodeList = _graph->getNodeList();

    pConnectedComponent->createSuperNodes(_groupMods);
    pConnectedComponent->createAgnodes(_viewMod);
    pConnectedComponent->createAgedges();

    agsafeset(pConnectedComponent->_gvG,(char*)"nodesep",(char*)"2",(char*)"5");
    agsafeset(pConnectedComponent->_gvG,(char*)"ranksep",(char*)"2",(char*)"5");
    agsafeset(pConnectedComponent->_gvG,(char*)"charset",(char*)"latin1",(char*)"UTF-8");
    agsafeset(pConnectedComponent->_gvG,(char*)"splines",(char*)"false",(char*)"true");
    agsafeset(pConnectedComponent->_gvG,(char*)"overlap",(char*)"scale",(char*)"true"); //{scale, false}

    if(_spanMod==AbstractLayout::SpanOn)
    {
        agsafeset(pConnectedComponent->_gvG,(char*)"pack",(char*)"true",(char*)"false");
        agsafeset(pConnectedComponent->_gvG,(char*)"packmode",(char*)"graph",(char*)"node");
    }

    gvLayout(pConnectedComponent->_gvC,pConnectedComponent->_gvG,(char*)"neato");

    pConnectedComponent->positionNodes();

    gvFreeLayout(pConnectedComponent->_gvC,pConnectedComponent->_gvG);
    agclose(pConnectedComponent->_gvG);
    gvFreeContext(pConnectedComponent->_gvC);

    return centerBoundingBox();
}
