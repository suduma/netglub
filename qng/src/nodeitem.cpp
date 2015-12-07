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

#include "nodeitem.h"
#include "edgeitem.h"
#include "graph.h"
#include <QtGui>
#include "graphview.h"
//#include "settings/entitypopwindow.h"
#include "mainwindow.h"

NodeItem::NodeItem(Node * node)
    : QGraphicsEllipseItem(), _node(node)
{
    //qDebug("NodeItem::NodeItem()");
    setFlags(ItemIsMovable | ItemIsSelectable | ItemIsFocusable);
    setFlag(ItemSendsGeometryChanges);
    setCacheMode(DeviceCoordinateCache);
    int size = _node->getSize();
    setRect(-size/2,-size/2,size,size);
    _color = _node->getGraph()->getView()->getItemColor(_node->getEntityModel().name);
    _image = _node->getGraph()->getView()->getItemImage(_node->getName());
    QString ModelName = node->getEntityModel().name;
    if (!ModelName.isEmpty())
    {
        if (node->getEntity().values.contains("value"))
            setCurrentValue(node->getEntity().values.value("value"));
        else
            setCurrentValue("no value setted yet");
    }
}

NodeItem::~NodeItem()
{

}

void NodeItem::addEdge(EdgeItem * edge)
{
    _edgeList << edge;
    //edge->adjust();
}

QList<EdgeItem *> NodeItem::edges() const
{
    return _edgeList;
}

QRectF NodeItem::boundingRect() const
{
    qreal adjust = 2;
    QRectF rect = this->rect();
    return QRectF(rect.left() - adjust, rect.top() - adjust,
        rect.width() + 3 + adjust, rect.height() + 3 + adjust);
}

QPainterPath NodeItem::shape() const
{
    QPainterPath path;
    QRectF rect = this->rect();
    path.addEllipse(rect.left(), rect.top(), rect.width(), rect.height());
    return path;
}

void NodeItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *)
{
    painter->setPen(Qt::NoPen);
    painter->setBrush(Qt::darkGray);
    QRectF rect = this->rect();

    if(MainWindow::get()->isBubbleViewMode())
    {
        painter->drawEllipse(rect.left()+3, rect.top()+3, rect.width(), rect.height());

        QRadialGradient gradient(rect.left()+rect.width()*0.30, rect.top()+rect.height()*0.30, rect.width()/2);
        //if (option->state & QStyle::State_Sunken) {
        if (isSelected()) {
          gradient.setCenter(3, 3);
          gradient.setFocalPoint(3, 3);
          gradient.setColorAt(1, QColor(Qt::yellow).light(120));
          gradient.setColorAt(0, QColor(Qt::darkYellow).light(120));
        } else {
          gradient.setColorAt(0, *_color);//Qt::yellow);
          gradient.setColorAt(1, _color->darker(200));
        }
        painter->setBrush(gradient);
        painter->setPen(QPen(Qt::black, 0));
        painter->drawEllipse(rect);
    }




    if(MainWindow::get()->isIconViewMode() && !_image->isNull())
    {
        painter->drawImage(rect.topLeft(),_image->scaled(rect.width(),rect.height()));

        painter->setPen(QPen(Qt::black, 0));
        QFont f = painter->font();
        f.setPointSize(rect.width()/20);
        painter->setFont(f);
        painter->drawText(rect,Qt::AlignBottom|Qt::AlignHCenter,_node->getValue());
    }



}

QVariant NodeItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
  switch (change) {
  case ItemPositionHasChanged:
    foreach (EdgeItem *edge, _edgeList)
        edge->adjust();
    //graph->itemMoved();
    break;
  default:
    break;
  };
  update();
  return QGraphicsEllipseItem::itemChange(change, value);
}


void NodeItem::setCurrentValue(QString value)
{
    QString ModelName = _node->getEntityModel().name;

    if (!ModelName.isEmpty())
    {
        QString tooltip = "<table border=\"0\" cellspacing=\"0\" cellpadding=\"5\" >";
        tooltip += "<tr><td rowspan=\"2\" align=\"center\">";
        tooltip += "<img src=\"images/entities/" + ModelName + "/image.png\" border=\"0\" align=\"center\"/>";
        tooltip += "</td><td>";
        tooltip += _node->getEntityModel().longName;
        tooltip += "</td></tr><tr><td>";
        tooltip += value;
        tooltip += "</td></tr></table>";
        setToolTip(tooltip);
    }
}

void NodeItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{

    setSelected(true);
    _node->getGraph()->getContextMenu()->exec(event->screenPos());
  //node->contextMenu->exec(event->screenPos());
}

void NodeItem::keyPressEvent ( QKeyEvent * event )
{
//    qDebug("NodeItem::keyPressEvent");
    if(event->key() == Qt::Key_Delete)
        qDebug("del");
        
}

void NodeItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
  Q_UNUSED(event);

//  qDebug() << "NodeItem::mouseDoubleClickEvent ";

  qDebug() << "NodeItem::mouseDoubleClickEvent ";
  /*

  EntityPopWindow * entityWindow = new EntityPopWindow(_node);
  entityWindow->exec();
  */
  _node->update();
}

Node * NodeItem::getNode()
{
    return _node;
}


