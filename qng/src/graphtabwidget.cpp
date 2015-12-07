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

#include "graphtabwidget.h"
#include "graphview.h"
#include "graphframe.h"

GraphTabWidget::GraphTabWidget(QWidget * parent) :
        QTabWidget(parent)
{
    setTabsClosable(true);
    setMovable(true);
}

GraphTabWidget::~GraphTabWidget()
{
    clear();
}

bool GraphTabWidget::removeTab(int index)
{
    Q_UNUSED(index);
    return true;
}

GraphView * GraphTabWidget::getGraphViewAt(int index)
{
    GraphView * graphView = NULL;
    if(widget(index))
        graphView = dynamic_cast<GraphView *>(widget(index));
    return graphView;
}
