/******************************************************************************
**
** Copyright (C) 2009-2010 DIATEAM. All rights reserved.
**
** This file is part of the Netglub GUI program.
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

#include "ntgnodeitem.h"
#include "edgeitem.h"
#include "graph.h"
#include "settings/entitypopwindow.h"
#include <QPainter>
#include <QGraphicsSceneContextMenuEvent>
#include <QKeyEvent>
#include <QGraphicsSimpleTextItem>
#include <QSvgGenerator>
#include <QSvgRenderer>

NtgNodeItem::NtgNodeItem(Node * node)
    : QGraphicsObject(), _node(node), _ntgBubbleItem(NULL), _ntgSvgItem(NULL),_ntgCircleItem(NULL),_ntgValueTextItem(NULL), _svgAttachedItem(NULL)
{
    setFlags(ItemIsMovable | ItemIsSelectable | ItemIsFocusable);
    setFlag(ItemSendsGeometryChanges);
    setCacheMode(DeviceCoordinateCache);

    QColor backgroundSelectionColor = QColor(Qt::gray);
    QColor borderSelectionColor = backgroundSelectionColor.darker();
    _pen = QPen(borderSelectionColor);
    _pen.setCapStyle(Qt::RoundCap);
    _pen.setJoinStyle(Qt::RoundJoin);
    _pen.setStyle(Qt::DashLine);
    _pen.setCosmetic(true);
    _pen.setWidth(2);
    backgroundSelectionColor.setAlphaF(0.4);
    _brush = QBrush(backgroundSelectionColor);

    _svgId = -1;

    setAcceptDrops(true);
    init();
}

NtgNodeItem::~NtgNodeItem()
{
}

void NtgNodeItem::init()
{
    //STEP 1 : Bubble
   if(!_ntgBubbleItem)
        _ntgBubbleItem = new NtgBubbleItem(this);
    int size = _node->getSize();
    _ntgBubbleItem->setRect(-size/2,-size/2,size,size);
    _ntgBubbleItem->_color = _node->getEntityModel().color;
    _ntgBubbleItem->hide();

    //STEP 2 : Circle

    if(!_ntgCircleItem)
        _ntgCircleItem = new QGraphicsEllipseItem(this);
    _ntgCircleItem->setRect(-size/2,-size/2,size,size);

    _ntgCircleItem->setFlags(ItemStacksBehindParent);
    _ntgCircleItem->setCacheMode(DeviceCoordinateCache);

    QColor ntgCircleItemColor = QColor(_node->getEntityModel().color);
    ntgCircleItemColor.setAlphaF(0.8); // 20% alpha transparency
    _ntgCircleItem->setBrush(QBrush(ntgCircleItemColor));
    QPen ntgCircleItemPen = QPen(ntgCircleItemColor.darker());
    ntgCircleItemPen.setWidth(10);
    _ntgCircleItem->setPen(ntgCircleItemPen);

    if(!_ntgValueTextItem)
        _ntgValueTextItem = new QGraphicsSimpleTextItem(this);
    setText(_node->getEntity().values.value("value"));

    if (_node->getEntity().values.contains("value"))
        setCurrentToolTip(_node->getEntity().values.value("value"));
    else
        setCurrentToolTip("no value setted yet");

    //STEP 3 : Svg Icon


    setSvg(_node->getSvgId());
}

void NtgNodeItem::addEdge(EdgeItem * edge)
{
    _edgeList << edge;
}

QList<EdgeItem *> NtgNodeItem::edges() const
{
    return _edgeList;
}

void NtgNodeItem::setText(const QString & text)
{
  _ntgValueTextItem->setText(text);
  QRectF rect = _ntgValueTextItem->boundingRect();
  _ntgValueTextItem->setPos(-rect.width()/2.0,boundingRect().height()/2.0);
}

QRectF NtgNodeItem::boundingRect() const
{
    return _ntgCircleItem->boundingRect();
}

QPainterPath NtgNodeItem::shape() const
{
    return _ntgCircleItem->shape();
}

void NtgNodeItem::paint(QPainter *painter,
                   const QStyleOptionGraphicsItem *option,
                   QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
    if (isSelected())
    {
        painter->setPen(_pen);
        //painter->setBrush(_brush);
        painter->drawRoundedRect(boundingRect(), 10, 10);

    }
}

void NtgNodeItem::setIconMode()
{
    _ntgBubbleItem->hide();
    _ntgSvgItem->show();
    _ntgCircleItem->hide();
    if (_node->getFileMap().count() > 0)
        _svgAttachedItem->show();
    else
        _svgAttachedItem->hide();
    QRectF rect = _ntgValueTextItem->boundingRect();
    _ntgValueTextItem->setPos(-rect.width()/2.0,boundingRect().height()/2.0);
}

void NtgNodeItem::setBubbleMode()
{
    _ntgBubbleItem->show();
    _ntgSvgItem->hide();
    _ntgCircleItem->hide();
    _svgAttachedItem->hide();
    QRectF rect = _ntgValueTextItem->boundingRect();
    _ntgValueTextItem->setPos(-rect.width()/2.0,boundingRect().height()/2.0);
}

void NtgNodeItem::setCircleMode()
{
    _ntgBubbleItem->hide();
    _ntgSvgItem->hide();
    _ntgCircleItem->show();
    _svgAttachedItem->hide();
    QRectF rect = _ntgValueTextItem->boundingRect();
    _ntgValueTextItem->setPos(-rect.width()/2.0,boundingRect().height()/2.0);
}

void NtgNodeItem::setRect(qreal x, qreal y, qreal width, qreal height)
{
    _ntgCircleItem->setRect(x,y,width, height);
    _ntgBubbleItem->setRect(x,y,width, height);

}

QVariant NtgNodeItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
  switch (change) {
  case ItemPositionHasChanged:
    foreach (EdgeItem *edge, _edgeList)
        edge->adjust();
    //graph->itemMoved();
    break;
/*
  case ItemSelectedHasChanged:
    _ntgCircleItem->setSelected(value.toBool());
    _ntgSvgItem->setSelected(value.toBool());
    break;
*/
  default:
    break;
  };
  //update();
  return QGraphicsObject::itemChange(change, value);
}

void NtgNodeItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    setSelected(true);
    _node->getGraph()->execContextMenu(event->screenPos());
}

Node * NtgNodeItem::getNode()
{
	return _node;
}

void NtgNodeItem::keyPressEvent ( QKeyEvent * event )
{
//    qDebug("NodeItem::keyPressEvent");
    if(event->key() == Qt::Key_Delete)
        qDebug("del");

}
void NtgNodeItem::setSelected ( bool selected )
{
    QGraphicsObject::setSelected(selected);
}

void NtgNodeItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
  Q_UNUSED(event);
  EntityPopWindow entityWindow(_node);
  entityWindow.exec();
  _node->update();
}

void NtgNodeItem::setCurrentToolTip(QString value)
{
    QString ModelName = _node->getEntityModel().name;
    QSettings settings;
    QFileInfo settingsPath = QFileInfo(settings.fileName());
    QString pngFileName = settingsPath.absolutePath()+"/data/entities/"+ModelName+"/image.png";
    if (!ModelName.isEmpty())
    {
        QString text = "<table border=\"0\" cellspacing=\"0\" cellpadding=\"5\" >";
        text += "<tr><td rowspan=\"2\" align=\"center\">";
        text += "<img src=\""+pngFileName+"\" border=\"0\" align=\"center\"/>";
        text += "</td><td>";
        text += _node->getEntityModel().longName;
        text += "</td></tr><tr><td>";
        text += value;
        text += "</td></tr></table>";
        setToolTip(text);
    }
}


void NtgNodeItem::setSvg(int id)
{
    QSettings settings;
    QFileInfo settingsPath = QFileInfo(settings.fileName());
    QDir settingsDir = settingsPath.absoluteDir();

    QString svgFileName;
    if (id == -1)
    {
        svgFileName = settingsPath.absolutePath()+"/data/entities/"+_node->getEntityModel().name+"/image.svg";
        _node->setSvgId(-1);
    }
    else
    {
        if( _node->getFileMap().contains(id))
        {
            svgFileName = _node->getFileMap().value(id).cpPath;
            _node->setSvgId(id);
        }
        else
        {
            svgFileName = settingsPath.absolutePath()+"/data/entities/"+_node->getEntityModel().name+"/image.svg";
            _node->setSvgId(-1);
        }

    }

    if (QFileInfo(svgFileName).isReadable())
    {
        if(_ntgSvgItem)
            delete _ntgSvgItem;

        if (svgFileName.endsWith(".svg"))
            _ntgSvgItem = new QGraphicsSvgItem(svgFileName, this);
        else
        {

            QImage image(svgFileName);
            if (image == QImage())
                return;

            QPainter painter;
            painter.drawImage(0,0,image,1,1,1,1);
/*
//            QSvgGenerator generator;
//            generator.setFileName(svgFileName+".svg");
//            generator.setSize(image.size());
//            generator.setViewBox(QRect(0, 0, image.width(), image.height()));
//            generator.setTitle(tr("SVG Generator Example Drawing"));
//            generator.setDescription(tr("An SVG drawing created by the SVG Generator "
//                                        "Example provided with Qt."));
//
//            painter.begin(&generator);
//            painter.setRenderHint(QPainter::Antialiasing);
//
//            painter.setClipRect(QRect(0, 0, image.width(), image.height()));
//            painter.setPen(Qt::NoPen);
//            painter.drawImage(0,0,image,0,0, image.width(), image.height());
//            painter.end();
//
//            _ntgSvgItem = new QGraphicsSvgItem(svgFileName+".svg", this);
//            QDir().remove(svgFileName+".svg");
  */

            QByteArray byteArray;
            QDataStream dataStream(&byteArray, QIODevice::WriteOnly);
            QSvgGenerator generator;
            generator.setOutputDevice(dataStream.device());

            generator.setSize(image.size());
            generator.setViewBox(QRect(0, 0, image.width(), image.height()));
            generator.setTitle(tr("SVG Generator Example Drawing"));
            generator.setDescription(tr("An SVG drawing created by the SVG Generator "
                                        "Example provided with Qt."));


            painter.begin(&generator);
            painter.setRenderHint(QPainter::Antialiasing);

            painter.setClipRect(QRect(0, 0, image.width(), image.height()));
            painter.setPen(Qt::NoPen);
            painter.drawImage(0,0,image,0,0, image.width(), image.height());
            painter.end();

            svgRenderer = new QSvgRenderer(byteArray);
            _ntgSvgItem = new QGraphicsSvgItem(this);
            _ntgSvgItem->setSharedRenderer(svgRenderer);

        }
//        qDebug() << _ntgSvgItem->boundingRect().size();
        QRectF svgBBox = _ntgSvgItem->boundingRect();
        if (svgBBox.width() > svgBBox.height())
            _ntgSvgItem->scale(64.0/svgBBox.width(), 64.0/svgBBox.width());
        else
            _ntgSvgItem->scale(64.0/svgBBox.height(), 64.0/svgBBox.height());


        svgBBox = _ntgSvgItem->boundingRect();
        _ntgSvgItem->translate(-svgBBox.width()/2,-svgBBox.height()/2);

        if(_svgAttachedItem)
            delete _svgAttachedItem;
        _svgAttachedItem = new QGraphicsSvgItem(":/images/attached.svg", this);
        QRectF svgAttachedBBox = _svgAttachedItem->boundingRect();
        _svgAttachedItem->scale(54.0/boundingRect().width(), 54.0/boundingRect().height());
        svgAttachedBBox = _svgAttachedItem->boundingRect();
        _svgAttachedItem->translate(boundingRect().width()*0.25,-boundingRect().height()*0.75);

        _svgAttachedItem->setFlags(ItemStacksBehindParent);
        _svgAttachedItem->setCacheMode(DeviceCoordinateCache);

        _svgAttachedItem->hide();



        _ntgSvgItem->setFlags(ItemStacksBehindParent);
        _ntgSvgItem->setCacheMode(DeviceCoordinateCache);

    }
    _ntgSvgItem->hide();
}

void NtgNodeItem::dragEnterEvent(QGraphicsSceneDragDropEvent *event)
{
    //qDebug("NtgNodeItem::dragEnterEvent()");
    //qDebug() << event->mimeData()->formats();
    if (event->mimeData()->hasFormat("text/uri-list") ||
        event->mimeData()->hasFormat("text/plain"))
    {
        //qDebug("text/uri-list || text/plain");
        event->setDropAction(event->proposedAction());
        event->accept();
    }
    else event->ignore();

}

void NtgNodeItem::dropEvent(QGraphicsSceneDragDropEvent *event)
{
    //qDebug("NtgNodeItem::dropEvent()");
    //qDebug() << event->mimeData()->formats();
    if (event->mimeData()->hasFormat("text/uri-list") ||
        event->mimeData()->hasFormat("text/plain"))
    {
        //qDebug("text/uri-list || text/plain");


        QUrl url(event->mimeData()->data("text/uri-list").simplified());
        qDebug("%s", qPrintable(url.toLocalFile()));

        int id = _node->addFile(url.toLocalFile());
        if (url.path().endsWith(".svg") || url.path().endsWith(".png") || url.path().endsWith(".jpg") || url.path().endsWith(".gif"))
        {
            event->setDropAction(event->proposedAction());
            event->accept();
            setSvg(id);
            emit redrawItem(this);
        }
    }

}



NtgBubbleItem::NtgBubbleItem(NtgNodeItem * ntgItem)
    : QGraphicsEllipseItem(ntgItem)
{
    //setFlags(ItemStacksBehindParent | ItemIsMovable | ItemIsSelectable | ItemIsFocusable);
    setFlags(ItemStacksBehindParent);
    //setFlag(ItemSendsGeometryChanges);
    setCacheMode(DeviceCoordinateCache);
}

NtgBubbleItem::~NtgBubbleItem()
{

}

QRectF NtgBubbleItem::boundingRect() const
{
    qreal adjust = 2;
    QRectF rect = this->rect();
    return QRectF(rect.left() - adjust, rect.top() - adjust,
        rect.width() + 3 + adjust, rect.height() + 3 + adjust);
}

QPainterPath NtgBubbleItem::shape() const
{
    QPainterPath path;
    QRectF rect = this->rect();
    path.addEllipse(rect.left(), rect.top(), rect.width(), rect.height());
    return path;
}

void NtgBubbleItem::paint(QPainter *painter,
                          const QStyleOptionGraphicsItem *option,
                          QWidget *)
{
    Q_UNUSED(option);
    painter->setPen(Qt::NoPen);
    painter->setBrush(Qt::darkGray);
    QRectF rect = this->rect();

    painter->drawEllipse(rect.left()+3, rect.top()+3, rect.width(), rect.height());

    QRadialGradient gradient(rect.left()+rect.width()*0.30, rect.top()+rect.height()*0.30, rect.width()/2);
    if (isSelected()) {
      gradient.setCenter(3, 3);
      gradient.setFocalPoint(3, 3);
      gradient.setColorAt(1, QColor(Qt::yellow).light(120));
      gradient.setColorAt(0, QColor(Qt::darkYellow).light(120));
    } else {
      gradient.setColorAt(0, _color);//Qt::yellow);
      gradient.setColorAt(1, _color.darker(200));
    }
    painter->setBrush(gradient);
    painter->setPen(QPen(Qt::black, 0));
    painter->drawEllipse(rect);
}

