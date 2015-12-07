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

#include "graphview.h"
#include "mainwindow.h"
#include "graph.h"
#include "graphscene.h"
#include "ntgnodeitem.h"
#include "edgeitem.h"
#include "filemanager.h"

#include <QtGui>
#include <math.h>
#include "molecularlayout.h"
#include "blocklayout.h"
#include "circularlayout.h"
#include "hierarchicallayout.h"
#include "settings/entitypopwindow.h"
#include "nodeitem.h"



GraphView::GraphView(Graph * graph, QWidget * parent) :
        QGraphicsView (parent), _graph(graph), _layout(NULL),_currentLine(NULL),_ctrlIsPressed(false),_contextMenu(NULL)
{
    GraphScene * graphScene = new GraphScene(this);
    graphScene->setSceneRect(-5000,-5000,10000,10000);
    setScene(graphScene);
    setAcceptDrops(true);

    //  FullViewportUpdate  // MinimalViewportUpdate //SmartViewportUpdate
    setInteractive(true);
    setRenderHint(QPainter::Antialiasing, true);
    setRenderHint(QPainter::TextAntialiasing, false);
    setRenderHint(QPainter::SmoothPixmapTransform, false);

    setDragMode(QGraphicsView::RubberBandDrag);
    //setOptimizationFlags(QGraphicsView::DontSavePainterState|QGraphicsView::DontAdjustForAntialiasing);
    setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);

     //QGraphicsView can cache pre-rendered content in a QPixmap, which is then drawn onto the viewport.
    setCacheMode(QGraphicsView::CacheNone);  //CacheBackground | CacheNone

    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setResizeAnchor(QGraphicsView::NoAnchor);
    setFocusPolicy(Qt::StrongFocus);
    setFocus();
    //connect(scene, SIGNAL(selectionChanged()), mainWindow, SLOT(itemSelectionChangeSlot()));

    //this->scene()->addRect(this->sceneRect());


    _currentLayoutId = MiningBlockLayoutId;
    _startLayoutAgain = false;
    _fitInView = false;
    _freezed = false;
    _drawMode = GraphView::Default;
    _switchingFactor = 0.25;


}

GraphView::~GraphView()
{
    if(_layout!=NULL)
        //_layout->wait();
        _layout->terminate();

    if(_contextMenu!=NULL)
        delete _contextMenu;

}

void GraphView::addGraphItems()
{
  if (_graph)
  {
    QListIterator<Node*> i(_graph->getNodeList());
    while(i.hasNext())
    {
        Node * pNode = i.next();
        NtgNodeItem * ntgNodeItem = pNode->newNtgItem();
        scene()->addItem(ntgNodeItem);
        ntgNodeItem->setPos(pNode->getPos());
    }

    QSetIterator<Edge*> ei(_graph->getEdgeSet());
    while(ei.hasNext())
    {
        Edge * pEdge = ei.next();
        pEdge->newItem();
        scene()->addItem(pEdge->getItem());
    }
  }
}

void GraphView::redrawItem(NtgNodeItem * item)
{
    qreal currentFactor = matrix().m11();

    if (_currentLayoutId<=MiningMolecularLayoutId)
    {
        if (currentFactor>_switchingFactor)
        {
            item->setIconMode();
            return;
        }
        else
        {
            item->setCircleMode();
            return;
        }
    }
    else
    {
        item->setBubbleMode();
        return;
    }
}

Graph * GraphView::getGraph() const
{
    return _graph;
}

void GraphView::setLayoutId(int id)
{
//    qDebug("GraphView::setLayoutId(%d)",id);
    _currentLayoutId = id;
}

int GraphView::getLayoutId() const
{
    return _currentLayoutId;
}

bool GraphView::scaleView(qreal scaleFactor)
{
    if(_currentLine!=NULL)
    {
        scene()->removeItem(_currentLine);
        delete _currentLine;
        _currentLine = NULL;
    }

    qreal currentFactor = matrix().m11();
    qreal newFactor = matrix().scale(scaleFactor, scaleFactor).mapRect(QRectF(0, 0, 1, 1)).width();
    if (newFactor < 0.001 || newFactor > 1000) return false;
    scale(scaleFactor, scaleFactor);
    if (currentFactor>_switchingFactor && newFactor <= _switchingFactor)
    {
//        qDebug("1");
        QList<QGraphicsItem *> itemsList = scene()->items();
        NtgNodeItem * ntgNodeItem = NULL;
        for (int i = 0; i < itemsList.size(); ++i)
        {
            ntgNodeItem = qgraphicsitem_cast<NtgNodeItem*>(itemsList.at(i));
            if (ntgNodeItem) {
//                qDebug("11");
                redrawItem(ntgNodeItem);
            }
        }
    }
    else if (currentFactor<_switchingFactor && newFactor >= _switchingFactor)
    {
//        qDebug("2");
        QList<QGraphicsItem *> itemsList = scene()->items();
        NtgNodeItem * ntgNodeItem = NULL;
        for (int i = 0; i < itemsList.size(); ++i)
        {
            ntgNodeItem = qgraphicsitem_cast<NtgNodeItem*>(itemsList.at(i));
            if (ntgNodeItem) {
                redrawItem(ntgNodeItem);
            }
        }
    }
    return true;
}

void GraphView::refreshView()
{
    QListIterator<Node*> itrNodeList(_graph->getNodeList());
    while(itrNodeList.hasNext())
    {
        Node * pNode = itrNodeList.next();
        NtgNodeItem * ntgNodeItem = pNode->getNtgItem();
        if(ntgNodeItem)
            redrawItem(ntgNodeItem);
    }

}

void GraphView::scrollView(QPoint dp)
{
    /*
    QScrollBar* hscroll = horizontalScrollBar();
    QScrollBar* vscroll = verticalScrollBar();
    */
    translate(-dp.x(), -dp.y());
}

void GraphView::setFitInView(bool fitInView)
{
    _fitInView = fitInView;
}

void GraphView::wheelEvent(QWheelEvent *event)
{
    QPoint vpos1 = event->pos();
    QPointF spos1 = mapToScene(vpos1);
    qreal s = pow((double)2, event->delta() / 360.0);
    if (!scaleView(s)) return;
    QPoint vpos2 = mapFromScene(spos1);
    QPoint dp = vpos2 - vpos1;
    scrollView(-dp);
}

void GraphView::mousePressEvent(QMouseEvent * event)
{
    if(event->buttons() == Qt::RightButton)
    {
        _drawMode=Default;
        if(_currentLine!=NULL)
        {
            scene()->removeItem(_currentLine);
            delete _currentLine;
            _currentLine = NULL;
        }
        MainWindow::get()->uncheckDrawEdgeAct();
    }
    else if(_drawMode==DrawEdge)
    {
        QList<QGraphicsItem *> startItems = scene()->items(mapToScene(event->pos()));
        if(startItems.size()>0)
        {
            NtgNodeItem * ntgNodeItem = qgraphicsitem_cast<NtgNodeItem *>(startItems.first());


            if(ntgNodeItem!=0)
            {
                QPointF center = ntgNodeItem->pos();
                _currentLine = new QGraphicsLineItem(QLineF(center,center));
                _currentLine->setPen(QPen(Qt::black, 2));
                _currentLine->setZValue(10000);
                scene()->addItem(_currentLine);
            }
        }
    }
    else
    {
        QGraphicsView::mousePressEvent(event);
        if(event->buttons() == Qt::LeftButton && _ctrlIsPressed==true)
        {
            startDrag();
            _ctrlIsPressed = false;
        }
    }
}

void GraphView::mouseMoveEvent(QMouseEvent * event)
{
    if (_drawMode==DrawEdge && _currentLine != 0)
    {
      QLineF newLine(_currentLine->line().p1(), mapToScene(event->pos()));
      _currentLine->setLine(newLine);
    }
    else
        QGraphicsView::mouseMoveEvent(event);
}

void GraphView::mouseReleaseEvent(QMouseEvent * event)
{
    if (_currentLine != 0 && _drawMode==DrawEdge)
    {
        QList<QGraphicsItem *> startItems = scene()->items(_currentLine->line().p1());
        if (startItems.count() && startItems.first() == _currentLine)
        startItems.removeFirst();
        QList<QGraphicsItem *> endItems = scene()->items(_currentLine->line().p2());
        if (endItems.count() && endItems.first() == _currentLine)
            endItems.removeFirst();

        if (startItems.count() > 0 && endItems.count() > 0 && startItems.first() != endItems.first())
        {
            QGraphicsItem * startItem = startItems.first();
            QGraphicsItem * endItem = endItems.first();
            NtgNodeItem * startGraphicsItem = qgraphicsitem_cast<NtgNodeItem *>(startItem);
            NtgNodeItem * endGraphicsItem = qgraphicsitem_cast<NtgNodeItem *>(endItem);
            if (startGraphicsItem&&endGraphicsItem)
            {
                if (startGraphicsItem!=endGraphicsItem)
                {
                    Edge * edge = _graph->newEdge(startGraphicsItem->getNode(),endGraphicsItem->getNode());
                    if(edge !=  NULL)
                    {
                        EdgeItem * edgeItem = edge->newItem();
                        scene()->addItem(edgeItem);
                        MainWindow::get()->addInfoLogMessage("add 1 edge");
                    }
                }
            }
        }
      scene()->removeItem(_currentLine);
      delete _currentLine;
    }
    _currentLine = 0;    
    QGraphicsView::mouseReleaseEvent(event);
}

/*  & Drop support */

void GraphView::dragEnterEvent(QDragEnterEvent *event)
{
    //qDebug("GraphView::dragEnterEvent()");
    if (event->mimeData()->hasFormat("netglub/x-node-model"))
    {
        event->accept();
    }
    else if(event->mimeData()->hasFormat("netglub/x-node-entity"))
    {
        event->ignore();
    }
    else
    {
        QGraphicsView::dragEnterEvent(event);
        event->setDropAction(event->proposedAction());
        event->accept();
    }
}

void GraphView::dragMoveEvent(QDragMoveEvent *event)
{
    //qDebug("GraphView::dragMoveEvent()");
    if (event->mimeData()->hasFormat("netglub/x-node-model"))
    {
        event->setDropAction(event->proposedAction());
        event->accept();
    }
    else if(event->mimeData()->hasFormat("netglub/x-node-entity"))
    {
        event->ignore();
    }
    else
    {
        QGraphicsView::dragMoveEvent(event);
        event->setDropAction(event->proposedAction());
        event->accept();
    }
}

void GraphView::dragLeaveEvent(QDragLeaveEvent *event)
{
    QGraphicsView::dragLeaveEvent(event);
    Q_UNUSED(event)
}

void GraphView::dropEvent(QDropEvent *event)
{
    //qDebug("GraphView::dropEvent()");
    if (event->mimeData()->hasFormat("netglub/x-node-model"))
    {
        QByteArray itemData = event->mimeData()->data("netglub/x-node-model");
        QDataStream dataStream(&itemData, QIODevice::ReadOnly);

        NtgEntityModel entityModel;
        dataStream >> entityModel;

        Node * node = _graph->newNode();
        node->setEntityModel(entityModel);

        NtgNodeItem * ntgItem = node->newNtgItem();
        scene()->addItem(ntgItem);
        redrawItem(ntgItem);

        QPointF pos = mapToScene(event->pos());
        ntgItem->setPos(pos);

        event->acceptProposedAction();
        QApplication::setOverrideCursor(QCursor(Qt::ArrowCursor));

        EntityPopWindow * entityWindow = new EntityPopWindow(node);
        int ret = entityWindow->exec();
        if(ret == QDialog::Rejected)
            _graph->deleteNode(node);


        MainWindow::get()->setSelectable(true);
        QApplication::restoreOverrideCursor();
    }
    else
    {
        QGraphicsView::dropEvent(event);
    }
}

QColor * GraphView::getItemColor(QString str)
{
    QString colorStr = ModelsManager::getOrCreate()->getEntityModel(str).color;

    QRegExp * regexp = new QRegExp("#[0-9a-fA-F]{6}");

    if (regexp->exactMatch(colorStr))
        return new QColor(colorStr);
    else
        return new QColor(rand()%255,rand()%255,rand()%255);
}

QImage * GraphView::getItemImage(QString str)
{
    if(!_itemImageMap.contains(str))
    {
        QString uuidName = str.toLower().simplified().replace(" ","");
        QSettings settings;
        QFileInfo settingsPath = QFileInfo(settings.fileName());
        QString fileName = settingsPath.absolutePath()+"/data/entities/"+uuidName+"/image.png";

        QImage * pImage = new QImage(fileName);
        _itemImageMap.insert(str,pImage);
    }

    return _itemImageMap.value(str);
}

void GraphView::startLayout()
{
    if(_freezed)
    {
//        qDebug("layout is freezed");
        return;
    }
    if (_layout)
    {
        _startLayoutAgain = true;
        return;
    }



    switch(_currentLayoutId)
    {
    case MiningBlockLayoutId :
        _layout = new BlockLayout(_graph,AbstractLayout::Mining,AbstractLayout::SameParentsAndChilds);
        break;
    case MiningHierarchyLayoutId :
        _layout = new HierarchicalLayout(_graph);
        break;
    case MiningCircularLayoutId :
        _layout = new CircularLayout(_graph);
        break;
    case MiningMolecularLayoutId :
        _layout = new MolecularLayout(_graph);
        break;

    case CentralityBlockLayoutId :
        _layout = new BlockLayout(_graph,AbstractLayout::Centrality,AbstractLayout::SameParentsAndChilds);
        break;
    case CentralityHierarchyLayoutId :
        _layout = new HierarchicalLayout(_graph,AbstractLayout::Centrality);
        break;
    case CentralityCircularLayoutId :
        _layout = new CircularLayout(_graph,AbstractLayout::Centrality);
        break;
    case CentralityMolecularLayoutId :
        _layout = new MolecularLayout(_graph,AbstractLayout::Centrality);
        break;

    case EdgeWeightedBlockLayoutId :
        _layout = new BlockLayout(_graph,AbstractLayout::EdgeWeighted,AbstractLayout::SameParentsAndChilds);
        break;
    case EdgeWeightedHierarchyLayoutId :
        _layout = new HierarchicalLayout(_graph,AbstractLayout::EdgeWeighted);
        break;
    case EdgeWeightedCircularLayoutId :
        _layout = new CircularLayout(_graph,AbstractLayout::EdgeWeighted);
        break;
    case EdgeWeightedMolecularLayoutId :
        _layout = new MolecularLayout(_graph,AbstractLayout::EdgeWeighted);
        break;

    default:
        qCritical("Unable to layout valid layoutId \"%d\"", _currentLayoutId);
        return;
    }

    connect(_layout,SIGNAL(finished()), this, SLOT(_layoutFinishedSlot()));

    _startLayoutAgain = false;

    _layout->start();
}

void GraphView::_layoutFinishedSlot()
{
//    qDebug("GraphView::_layoutFinishedSlot()");
    QRectF bbox = _layout->centerBoundingBox();
    delete _layout;
    _layout = NULL;

    _graph->redraw();

    QRectF sceneRect(bbox);
    sceneRect.adjust(-bbox.width()/8,-bbox.height()/8,bbox.width()/8,bbox.height()/8);
    scene()->setSceneRect(sceneRect);

    if (_fitInView) fitInView(sceneRect,Qt::KeepAspectRatio);

    QList<QGraphicsItem *> itemsList = scene()->items();
    NtgNodeItem * ntgNodeItem = NULL;
    for (int i = 0; i < itemsList.size(); ++i)
    {
        ntgNodeItem = qgraphicsitem_cast<NtgNodeItem*>(itemsList.at(i));
        if (ntgNodeItem) {
            redrawItem(ntgNodeItem);
        }
    }


    if (_startLayoutAgain)
    {
        startLayout();
    }
}

void GraphView::keyPressEvent ( QKeyEvent * event )
{
    //qDebug("QGraphicsView::keyPressEvent");
    if(event->key() == Qt::Key_Delete)
    {
        if(!deleteAllSelectedNodes())
            deleteAllSelectedEdges();
    }
    if(event->key() == Qt::Key_Control)
    {
        _ctrlIsPressed = true;
    }

}

void GraphView::keyReleaseEvent(  QKeyEvent * event)
{
    if(event->key() == Qt::Key_Control)
    {
        _ctrlIsPressed = false;
    }

}

bool GraphView::deleteAllSelectedNodes()
{
    bool ret = false;
    MainWindow::get()->setSelectable(false);
    QListIterator<Node*> itrNode(_graph->getNodeList());
    while(itrNode.hasNext())
    {
        Node * pNode = itrNode.next();
        if(pNode->getNtgItem()->isSelected())
        {
            _graph->deleteNode(pNode);
            ret = true;
        }
    }
    return ret;
}


void GraphView::deleteAllSelectedEdges()
{
    MainWindow::get()->setSelectable(false);
    QSetIterator<Edge*> itrEdge(_graph->getEdgeSet());
    while(itrEdge.hasNext())
    {
        Edge * pEdge = itrEdge.next();
        if(pEdge->getItem()->isSelected())
            _graph->deleteEdge(pEdge);
    }
}


void GraphView::deleteAllNodes()
{
    QListIterator<Node*> itrNode(_graph->getNodeList());
    while(itrNode.hasNext())
    {
        Node * pNode = itrNode.next();
        _graph->deleteNode(pNode);
    }
}

void GraphView::setFreezed(bool freezed)
{
    _freezed = freezed;
    if(freezed==false)
        startLayout();
}

bool GraphView::isFreezed()
{
    return _freezed;
}

GraphView::DrawMode GraphView::getDrawMod()
{
    return _drawMode;
}

void GraphView::setDrawMod(DrawMode drawmode)
{
    _drawMode = drawmode;
}

QRectF GraphView::allNodesBoundingBox()
{
    QPolygonF polygon;
    QListIterator<Node*> itrNodeList(getGraph()->getNodeList());
    while(itrNodeList.hasNext())
    {
        Node * pNode = itrNodeList.next();
        //QPointF pos = pNode->getNtgItem()->pos();
        QPointF pos = pNode->getPos();
        QPointF dec = QPointF(pNode->getNtgItem()->boundingRect().width()/2,pNode->getNtgItem()->boundingRect().width()/2);
        QPolygonF pol = QPolygonF(QRectF(pos-dec,pos+dec));
        polygon<<pol;
    }
    QRectF bbox = polygon.boundingRect();
    return bbox;
}

QRectF GraphView::selectedNodesBoundingBox()
{
    QPolygonF polygon;
    QListIterator<Node*> itrNodeList(getGraph()->getNodeList());
    while(itrNodeList.hasNext())
    {
        Node * pNode = itrNodeList.next();
        if(pNode->getNtgItem()->isSelected())
        {
            QPointF pos = pNode->getNtgItem()->pos();
            QPointF dec = QPointF(pNode->getNtgItem()->boundingRect().width()/2,pNode->getNtgItem()->boundingRect().width()/2);
            QPolygonF pol = QPolygonF(QRectF(pos-dec,pos+dec));
            polygon<<pol;
        }
    }
    QRectF bbox = polygon.boundingRect();
    return bbox;
}

void GraphView::resetScene(QRectF& bbox)
{
    QRectF sceneRect(bbox);
    sceneRect.adjust(-bbox.width()/8,-bbox.height()/8,bbox.width()/8,bbox.height()/8);
    scene()->setSceneRect(sceneRect);
}

bool GraphView::isThereSelectedNtgItem()
{
    bool ret = false;
    QList<QGraphicsItem *> selectedItemList = scene()->selectedItems();
    QListIterator<QGraphicsItem*> itrSelectedItem(selectedItemList);
    while(itrSelectedItem.hasNext())
    {
        NtgNodeItem * startGraphicsItem = qgraphicsitem_cast<NtgNodeItem *>(itrSelectedItem.next());
        if(startGraphicsItem && startGraphicsItem->isSelected())
        {
            ret = true;
            break;
        }
    }
    return ret;
}

void GraphView::execContextMenu(QPoint pos)
{

    QList<Node *> selectedNodes = _graph->getSelectedNodeList();
    QMultiMap<QString,NtgTransformModel> availableTransforms;
    availableTransforms = ModelsManager::getOrCreate()->getAvailableTransformsbyCategories(selectedNodes.first()->getEntityModel().name);
    QMultiMap<QString,NtgTransformModel> tmp;
    foreach(Node * pNode,selectedNodes)
    {
        tmp = ModelsManager::getOrCreate()->getAvailableTransformsbyCategories(pNode->getEntityModel().name);
        foreach(NtgTransformModel model, availableTransforms)
        {
            if(!tmp.contains(model.category,model))
                availableTransforms.remove(model.category,model);
        }
    }

    if(_contextMenu!=NULL)
        delete _contextMenu;

    QAction * transformAct;
    _contextMenu = new QMenu();
    _transformNameHash.clear();
    if (!availableTransforms.isEmpty())
    {
        QStringList keyList;

        foreach(QString key,availableTransforms.keys())
        {
          if(!keyList.contains(key))
          {
              QMenu * CategoryMenu = new QMenu(key,_contextMenu);
            _contextMenu->addMenu(CategoryMenu);
            QStringList transformList;
            foreach(NtgTransformModel transform, availableTransforms.values(key))
            {
                transformAct = new QAction(transform.longName,CategoryMenu);
                transformAct->setToolTip(transform.description);
                transformAct->setStatusTip(transform.description);
                _transformNameHash.insert(transform.longName,transform.name);
                transformList.append(transform.name);
                CategoryMenu->addAction(transform.longName);
            }
            CategoryMenu->addSeparator();
            QAction * allCategoryAct = new QAction("Apply All Transform(s) of "+ key,CategoryMenu);
            allCategoryAct->setData(QVariant(transformList));
            CategoryMenu->addAction(allCategoryAct);
            keyList.append(key);
          }
        }
        connect(_contextMenu,SIGNAL(triggered(QAction*)),this,SLOT(doTransform(QAction*)));

        _contextMenu->addSeparator();
        QAction * allAct = new QAction("Apply All Transforms",_contextMenu);
        connect(allAct,SIGNAL(triggered()),this,SLOT(doAllTransforms()));
        _contextMenu->addAction(allAct);


        _contextMenu->addSeparator();
        QAction * aboutAct = new QAction("About Selected Entity(ies)" ,_contextMenu);
    //    connect(allAct,SIGNAL(triggered()),this,)
        aboutAct->setEnabled(false);
        _contextMenu->addAction(aboutAct);


    }
    else
    {
        QAction * noAct = new QAction("No Available Transform" ,_contextMenu);
        noAct->setEnabled(false);
        _contextMenu->addAction(noAct);
    }


    //
    if(selectedNodes.size()!=0)
    {
        _contextMenu->addSeparator();
        bool url = true;
        bool diffSvg = false;
        foreach(Node * node, selectedNodes)
        {
            if (node->getEntityModel().name != "url" && node->getEntityModel().name != "website"
                && node->getEntityModel().name != "document" && node->getEntityModel().name != "email-address"
                && node->getEntityModel().name != "location")
                url = false;

            if(node->getSvgId() >-1)
                diffSvg = true;
        }

        if(url)
        {
            QAction * urlAct = new QAction("Open",_contextMenu);
            connect(urlAct,SIGNAL(triggered()),this,SLOT(openUrlSlot()));
            _contextMenu->addAction(urlAct);
        }

        QAction * addFileAct = new QAction("Add/Remove File(s)" ,_contextMenu);
        connect(addFileAct,SIGNAL(triggered()),this,SLOT(addFileSlot()));
        _contextMenu->addAction(addFileAct);

        if (diffSvg)
        {
            QAction * defaultAct = new QAction("Default Picture" ,_contextMenu);
            connect(defaultAct,SIGNAL(triggered()),this,SLOT(defaultSlot()));
            _contextMenu->addAction(defaultAct);
        }

    }
    _contextMenu->exec(pos);

}

void GraphView::doTransform(QAction * action)
{
    QList<Node *> selectedNodes = _graph->getSelectedNodeList();

    if (!action->data().toStringList().isEmpty())
    {

        QStringList transformList =action->data().toStringList();


        QString message = "you are about to call over " + QString::number(transformList.count(),10)
                          +" transforms.\n are you sure about it?";
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, tr("Multiple transforms call"),
                                        message,
                                        QMessageBox::Yes | QMessageBox::Cancel);
        if (reply == QMessageBox::Yes)
        {
            foreach(QString transformName, transformList)
            {
                _graph->launchTransform(selectedNodes,transformName);
            }
        }

    }
    else if (action->text() != "Apply All Transforms" && action->text() != "Open" && action->text() != "Add/Remove File(s)" && action->text() != "Default Picture")
    {
        if(selectedNodes.count()> 5)
        {
            QString message = "you are about to call the transforms in over " + QString::number(selectedNodes.count(),10)
                              +" entities.\n are you sure about that?";
            QMessageBox::StandardButton reply;
            reply = QMessageBox::question(this, tr("Multiple node transform call"),
                                            message,
                                            QMessageBox::Yes | QMessageBox::Cancel);
            if (reply == QMessageBox::Yes)
            {
                _graph->launchTransform(selectedNodes,_transformNameHash.value(action->text()));
            }
        }
        else
            _graph->launchTransform(selectedNodes,_transformNameHash.value(action->text()));
    }
}

void GraphView::doAllTransforms()
{
    QList<Node *> selectedNodes = _graph->getSelectedNodeList();
    QString message = "you are about to call over " + QString::number(_transformNameHash.count()*selectedNodes.count(),10)
                      +" transforms.\n are you sure about it?";
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, tr("All transform call"),
                                    message,
                                    QMessageBox::Yes | QMessageBox::Cancel);
    if (reply == QMessageBox::Yes)
    {
        foreach(QString nameId, _transformNameHash)
            _graph->launchTransform(selectedNodes,nameId);
    }

}


void GraphView::openUrlSlot()
{
    MainWindow::get()->openSelectedNodesUrl();
}

void GraphView::addFileSlot()
{
    if (_graph->getSelectedNodeList().count() ==1)
    {
        FilePopWindow * FPW = new FilePopWindow(_graph->getSelectedNodeList().first(),this);
        FPW->exec();
        delete FPW;
    }
    else
    {
        QFileDialog::Options options;
        QString selectedFilter;
        QString fileName = QFileDialog::getOpenFileName(this,
                                    tr("Add file to nodes"),
                                    "hello world",
                                    tr("All Files (*)"),
                                    &selectedFilter,
                                    options);

        if (!fileName.isEmpty())
        {
            foreach(Node * node, _graph->getSelectedNodeList())
            {
                node->addFile(fileName);
            }
        }
    }

}

void GraphView::defaultSlot()
{
    foreach(Node * node ,_graph->getSelectedNodeList())
    {
        node->getNtgItem()->setSvg(-1);
        redrawItem(node->getNtgItem());
    }
}


void GraphView::redrawAnItem(NtgNodeItem * ntgItem)
{
    redrawItem(ntgItem);
}


void GraphView::startDrag()
{
    QMimeData * mimeData = new QMimeData;
    QList<Node*> nodeList = _graph->getSelectedNodeList();
    if(nodeList.size()==0)
    {
        QList<QGraphicsItem*> itemsList = scene()->items(mapToScene(mapFromGlobal(QCursor::pos())));
        if(itemsList.size()!=0)
        {
            NtgNodeItem * ntgNodeItem = qgraphicsitem_cast<NtgNodeItem*>(itemsList.first());
            if (ntgNodeItem) {
                nodeList.append(ntgNodeItem->getNode());
            }
        }
    }

    if(nodeList.size()!=0)
    {
        QByteArray itemData;
        QDataStream stream(&itemData,QIODevice::WriteOnly);
        QString text;

        foreach(Node * pNode, nodeList)
        {
            NtgEntity entity = pNode->getEntity();
            if(entity.values.contains("value"))
            {
                text.append(entity.values.value(("value"))).append(" \n");
            }

            stream << entity;
        }
        QDrag * drag = new QDrag(this);

        mimeData->setText(text);
        mimeData->setData("netglub/x-node-entity",itemData);
        drag->setMimeData(mimeData);
        drag->exec(Qt::CopyAction,Qt::CopyAction);
    }
}

void GraphView::bestFit()
{
    if(isThereSelectedNtgItem()==false)
    {
        QRectF bbox = allNodesBoundingBox();
        resetScene(bbox);
        bbox.adjust(-bbox.width()/8,-bbox.height()/8,bbox.width()/8,bbox.height()/8);
        fitInView(bbox,Qt::KeepAspectRatio);
    }
    else
    {
        QRectF bbox = allNodesBoundingBox();
        resetScene(bbox);

        bbox = selectedNodesBoundingBox();
        bbox.adjust(-bbox.width()/4,-bbox.height()/4,bbox.width()/4,bbox.height()/4);
        fitInView(bbox,Qt::KeepAspectRatio);
    }
    refreshView();
}
