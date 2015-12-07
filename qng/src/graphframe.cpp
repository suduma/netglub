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

#include "graphframe.h"
#include "graph.h"


GraphFrame::GraphFrame(const QString &name, QWidget *parent, Graph * graph)
    : QFrame(parent)
{
    Q_UNUSED(name);
    if(graph==NULL) graph = Graph::newGraph(parent);
    _graph = graph;
    _graph->setGraphFrame(this);

    //view
    _graphView = new GraphView(graph);
    graph->setGraphView(_graphView);

}

GraphFrame::~GraphFrame()
{
    delete _graphView;
    delete _graph;
}

void GraphFrame::setGraphView(GraphView * gv)
{
    _graphView = gv;
}

GraphView * GraphFrame::getGraphView()
{
    return _graphView;
}

Graph * GraphFrame::getGraph()
{
    return _graph;
}

void GraphFrame::setGraph(Graph * graph)
{
    _graph = graph;
}

