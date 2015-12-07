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

#include "edgeitem.h"
#include "graph.h"
#include "nodeitem.h"
#include "ntgnodeitem.h"
#include <QtGui>
#include <math.h>

static const double Pi = 3.14159265358979323846264338327950288419717;
static double TwoPi = 2.0 * Pi;

EdgeItem::EdgeItem(Edge * edge)
   : _edge(edge), arrowSize(10)
{
    setFlags(ItemIsSelectable);
    setZValue(-1);
    setAcceptedMouseButtons(0);
    _tailItem = edge->getTailNode()->getNtgItem();
    _headItem = edge->getHeadNode()->getNtgItem();
    _tailItem->addEdge(this);
    _headItem->addEdge(this);
    adjust();
}

EdgeItem::~EdgeItem()
{

}

NtgNodeItem * EdgeItem::getTailItem() const
{
   return _tailItem;
}

void EdgeItem::setTailItem(NtgNodeItem * item)
{
   _tailItem = item;
   adjust();
}

NtgNodeItem * EdgeItem::getHeadItem() const
{
   return _headItem;
}

void EdgeItem::setHeadItem(NtgNodeItem * item)
{
   _headItem = item;
   adjust();
}

void EdgeItem::adjust()
{
    if (!_tailItem || !_headItem)
        return;

    QLineF line(mapFromItem(_tailItem, 0, 0), mapFromItem(_headItem, 0, 0));
    qreal length = line.length();

    prepareGeometryChange();

    if (length > qreal(20.)) {
        QPointF edgeOffsetSource((line.dx() * _tailItem->boundingRect().width()/2) / length, (line.dy() * _tailItem->boundingRect().height()/2) / length);
        QPointF edgeOffsetDest((line.dx() * _headItem->boundingRect().width()/2) / length, (line.dy() * _headItem->boundingRect().height()/2) / length);
        sourcePoint = line.p1() + edgeOffsetSource;
        destPoint = line.p2() - edgeOffsetDest;
    } else {
        sourcePoint = destPoint = line.p1();
    }
}

QRectF EdgeItem::boundingRect() const
{
    if (!_tailItem || !_headItem)
        return QRectF();

    qreal penWidth = 1;
    qreal extra = (penWidth + arrowSize) / 2.0;

    return QRectF(sourcePoint, QSizeF(destPoint.x() - sourcePoint.x(),
        destPoint.y() - sourcePoint.y()))
        .normalized().adjusted(-extra, -extra, extra, extra);
}

void EdgeItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{

    if (!_tailItem || !_headItem)
        return;

    QLineF line(sourcePoint, destPoint);
    if (qFuzzyCompare(line.length(), qreal(0.)))
       return;

    // Draw the line itself
    if(isSelected())
        painter->setPen(QPen(Qt::black, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    else
        painter->setPen(QPen(Qt::black, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter->drawLine(line);

    // Draw the arrows
    double angle = ::acos(line.dx() / line.length());
    if (line.dy() >= 0)
       angle = TwoPi - angle;

    //QPointF sourceArrowP1 = sourcePoint + QPointF(sin(angle + Pi / 3) * arrowSize,cos(angle + Pi / 3) * arrowSize);
    //QPointF sourceArrowP2 = sourcePoint + QPointF(sin(angle + Pi - Pi / 3) * arrowSize,cos(angle + Pi - Pi / 3) * arrowSize);
    _destArrowP1 = destPoint + QPointF(sin(angle - Pi / 3) * arrowSize,
                                             cos(angle - Pi / 3) * arrowSize);
    _destArrowP2 = destPoint + QPointF(sin(angle - Pi + Pi / 3) * arrowSize,
                                             cos(angle - Pi + Pi / 3) * arrowSize);

    painter->setBrush(Qt::black);
    //painter->drawPolygon(QPolygonF() << line.p1() << sourceArrowP1 << sourceArrowP2);
    painter->drawPolygon(QPolygonF() << line.p2() << _destArrowP1 << _destArrowP2);
}

QPainterPath EdgeItem::shape() const
{
    QPainterPath path;
    path.addPolygon(QPolygonF() << sourcePoint<<_destArrowP1<<destPoint<<_destArrowP2);
    return path;
}
