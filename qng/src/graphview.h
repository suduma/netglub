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

#ifndef GRAPHVIEW_H
#define GRAPHVIEW_H

#include <QGraphicsView>
#include <QMenu>

#include "graph.h"
#include "abstractlayout.h"

#include "QMutex"


class QKeyEvent;
class QWheelEvent;
class QMouseEvent;

class Graph;
class QProgressDialog;
class QProgressBar;


enum LayoutId {
    MiningBlockLayoutId=0,
    MiningHierarchyLayoutId,
    MiningCircularLayoutId,
    MiningMolecularLayoutId,
    CentralityBlockLayoutId,
    CentralityHierarchyLayoutId,
    CentralityCircularLayoutId,
    CentralityMolecularLayoutId,
    EdgeWeightedBlockLayoutId,
    EdgeWeightedHierarchyLayoutId,
    EdgeWeightedCircularLayoutId,
    EdgeWeightedMolecularLayoutId
};

class GraphView : public QGraphicsView
{
    Q_OBJECT
    Q_PROPERTY(DrawMode drawMod READ getDrawMod WRITE setDrawMod)
    Q_ENUMS(DrawMode)
public:
    enum DrawMode{Default,DrawEdge};
    GraphView(Graph * graph, QWidget * parent = 0);
    virtual ~GraphView();

    virtual Graph * getGraph() const;
    virtual void setLayoutId(int id);
    virtual int getLayoutId() const;
    virtual bool scaleView(qreal scaleFactor);
    virtual void scrollView(QPoint dp);
    virtual void refreshView();

    virtual void setFitInView(bool fitInview);

    void addGraphItems();
    virtual void redrawItem(NtgNodeItem * item);

    QColor * getItemColor(QString);
    QImage * getItemImage(QString);

    void deleteAllNodes();
    bool deleteAllSelectedNodes();
    void selectAllNodes();
    void deleteAllSelectedEdges();
    bool isThereSelectedNtgItem();

    void startLayout();

    void setFreezed(bool);
    bool isFreezed();

    QRectF allNodesBoundingBox();
    QRectF selectedNodesBoundingBox();
    void resetScene(QRectF&);


    DrawMode getDrawMod();
    void setDrawMod(DrawMode);
    
    void execContextMenu(QPoint);
public slots:
    void redrawAnItem(NtgNodeItem * ntgItem);

    void bestFit();


protected slots:
    virtual void _layoutFinishedSlot();
    void doTransform(QAction * action);
    void doAllTransforms();
    void openUrlSlot();
    void addFileSlot();
    void defaultSlot();



protected:
    virtual void wheelEvent(QWheelEvent *event);
    virtual void mousePressEvent(QMouseEvent *mouseEvent);
    virtual void mouseReleaseEvent(QMouseEvent *mouseEvent);
    virtual void mouseMoveEvent(QMouseEvent *mouseEvent);

//    void contextMenuEvent(QContextMenuEvent *event);

    /* Drag & Drop support */
    virtual void dragEnterEvent(QDragEnterEvent *event);
    virtual void dragMoveEvent(QDragMoveEvent *event);
    virtual void dropEvent(QDropEvent *event);
    virtual void dragLeaveEvent(QDragLeaveEvent *event);

    virtual void keyPressEvent(QKeyEvent * event );
    virtual void keyReleaseEvent(QKeyEvent * event );

    void startDrag();



    //void contextMenuEvent ( QContextMenuEvent * event );

    QPoint lastMousePos;

    Graph * _graph;
    AbstractLayout * _layout;
    QGraphicsLineItem * _currentLine;
    bool _ctrlIsPressed;    
    QMenu * _contextMenu;

    int _currentLayoutId;

    QMap<QString,QColor*> _itemColorMap;
    QMap<QString,QImage*> _itemImageMap;


    QAction * _viewLayoutAct;
    QAction * _layoutAct;
    bool _startLayoutAgain;
    bool _fitInView;
    bool _freezed;

    DrawMode _drawMode;

    QHash<QString, QString> _transformNameHash;


    qreal _switchingFactor;
};

#endif // GRAPHVIEW_H
