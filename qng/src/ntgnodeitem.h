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

#ifndef NTGNODEITEM_H
#define NTGNODEITEM_H

#include <QGraphicsObject>
#include <QColor>
#include <QGraphicsEllipseItem>
#include <QGraphicsSvgItem>
#include <QPen>
#include <QBrush>
#include <QDropEvent>

class Node;
class NtgBubbleItem;
class EdgeItem;
class QKeyEvent;
class QGraphicsSimpleTextItem ;

/****************************************************************************
**  class NtgNodeItem
****************************************************************************/

class NtgNodeItem : public  QGraphicsObject
{
    Q_OBJECT
public:
    NtgNodeItem(Node * n);
    virtual ~NtgNodeItem();    
    void init();

    virtual void addEdge(EdgeItem * edge);
    virtual QList<EdgeItem *> edges() const;
    virtual void setText(const QString & text);

    enum { Type = UserType + 100 };
    int type() const { return Type; }

    virtual QRectF boundingRect() const;
    virtual QPainterPath shape() const;

    virtual void paint(QPainter *painter,
                       const QStyleOptionGraphicsItem *option,
                       QWidget *widget);

    virtual void setRect ( qreal x, qreal y, qreal width, qreal height );
    virtual void setIconMode();
    virtual void setBubbleMode();
    virtual void setCircleMode();

    virtual void setSvg(int id);
    int getSvgId();

    virtual void keyPressEvent(QKeyEvent * event );
    virtual void setSelected ( bool selected );

    void setCurrentToolTip(QString value);

	Node * getNode();

    void dragEnterEvent(QGraphicsSceneDragDropEvent *event);
    void dropEvent(QGraphicsSceneDragDropEvent *event);

signals:
    void redrawItem(NtgNodeItem * item);

protected:
  QVariant itemChange(GraphicsItemChange change, const QVariant &value);
  void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
  void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);

protected:
    Node * _node;
    NtgBubbleItem * _ntgBubbleItem;
    QGraphicsSvgItem * _ntgSvgItem;
    QGraphicsSvgItem * _svgAttachedItem;
    QGraphicsEllipseItem * _ntgCircleItem;
    QGraphicsSimpleTextItem * _ntgValueTextItem;
    QList<EdgeItem *> _edgeList;
    QPen _pen;
    QBrush _brush;

    int _svgId;

    QSvgRenderer * svgRenderer;

};

/****************************************************************************
**  class NtgBubbleItem
****************************************************************************/

class NtgBubbleItem : public QGraphicsEllipseItem
{

public:
    NtgBubbleItem(NtgNodeItem * ntgNodeItem);
    virtual ~NtgBubbleItem();

    enum { Type = UserType + 101 };
    int type() const { return Type; }


    virtual QRectF boundingRect() const;
    virtual QPainterPath shape() const;
    virtual void paint(QPainter *painter,
                       const QStyleOptionGraphicsItem * option,
                       QWidget *widget);
    QColor _color;

};
#endif // NTGNODEITEM_H
