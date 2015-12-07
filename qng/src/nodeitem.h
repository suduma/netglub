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

#ifndef NODEITEM_H
#define NODEITEM_H

#include <QGraphicsEllipseItem>
#include <QObject>

class Node;
class EdgeItem;

class NodeItem : public QGraphicsEllipseItem
{

public:
    NodeItem(Node * n);
    virtual ~NodeItem();

    void addEdge(EdgeItem * edge);
    QList<EdgeItem *> edges() const;

    enum { Type = UserType + 1 };
    int type() const { return Type; }


    QRectF boundingRect() const;
    QPainterPath shape() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    QList<EdgeItem *> _edgeList;

    void setCurrentValue(QString value);

    virtual void keyPressEvent(QKeyEvent * event );

    Node * getNode();


protected:
  QVariant itemChange(GraphicsItemChange change, const QVariant &value);
  void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
  void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);

  QColor * _color;
  QImage * _image;

  QString * _curentValue;


private:
  Node * _node;



};

#endif // NODEITEM_H
