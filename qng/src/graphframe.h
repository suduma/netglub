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

#ifndef GRAPHFRAME_H
#define GRAPHFRAME_H

#include <QtGui>
#include "graphview.h"
#include "graph.h"

class GraphFrame : public QFrame
{
    Q_OBJECT
public:
    GraphFrame(const QString &name, QWidget *parent = 0, Graph * g = NULL);
    virtual ~GraphFrame();

    void setGraphView(GraphView *);
    GraphView * getGraphView();
    void setGraph(Graph *);
    Graph * getGraph();


protected slots:
    //void slotSelectLayoutType(int);

private:
/*
    QGroupBox * _gbView;
    QButtonGroup * _bgViewType;
    QPushButton * _pbMiningView;
    QPushButton * _pbOutWeightView;
    QPushButton * _pbInWeightView;

    QMap<int,QGroupBox*> _gbMap;

    QButtonGroup * _bgMVLay;
    QPushButton * _pbMVLayPGroup;
    QPushButton * _pbMVLayPCGroup;
    QPushButton * _pbMVLayDot;
    QPushButton * _pbMVLayFdp;
    QPushButton * _pbMVLayNeato;
    QPushButton * _pbMVLayTwopi;
    QPushButton * _pbMVLayCirco;

    QButtonGroup * _bgOWLay;
    QPushButton * _pbOWLayDot;
    QPushButton * _pbOWLayCirco;
    QPushButton * _pbOWLayNeato;
    QPushButton * _pbOWLayFdp;

    QButtonGroup * _bgIWLay;
    QPushButton * _pbIWLayDot;
    QPushButton * _pbIWLayNeato;
    QPushButton * _pbIWLayFdp;
*/
    GraphView * _graphView;
    Graph * _graph;

};

#endif // GRAPHFRAME_H
