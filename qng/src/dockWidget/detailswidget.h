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

#ifndef DETAILSWIDGET_H
#define DETAILSWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QGridLayout>
#include "graph.h"

class DetailsWidget : public QWidget
{
Q_OBJECT
public:
    explicit DetailsWidget(QWidget *parent = 0);
    virtual  ~DetailsWidget();

    void setNodeList(QList<Node *> nodeList);
    void setEdgeSet(QSet<Edge*>&);

    QString getFormatedString(QString text, int size);

protected:
    virtual void mouseDoubleClickEvent(QMouseEvent *);

    QLabel * _titleLabel;
    QLabel * _imgLabel;
    QLabel * _detailsLabel;

    QLabel * _label;
    Node * _node;

};


#endif // DETAILSWIDGET_H
