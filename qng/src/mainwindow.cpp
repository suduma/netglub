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

#include "mainwindow.h"
#include "graphtabwidget.h"

#include "graph.h"
#include "graphview.h"
#include "graphscene.h"
#include "entitytreewidget.h"
#include "entitytreewidgetitem.h"
#include "graphframe.h"
#include "filemanager.h"

#include <QtGui>
#include <QDomDocument>
#include <QFileInfo>
#include <QPair>
#include <QStandardItemModel>
#include <QStandardItem>
#include "edgeitem.h"


/****************************************************************************
**  class MainWindow
****************************************************************************/

MainWindow * MainWindow::_mainWindow = NULL;
QtMsgHandler MainWindow::_oldMsgHandler = NULL;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent), _currentGraphView(NULL), _tabWidget(NULL)
{

    _transformTimer = new TransformTimer();

//    ModelsManager::getOrCreate();

    setAttribute(Qt::WA_DeleteOnClose);

    _createActions();
    _createMenus();
    _createToolBars();
    _createStatusBar();

    _createDockWindows();


    //TODO ?
    setWindowTitle(tr("Netglub GUI version 1.0"));



    _tabWidget = new GraphTabWidget(this);
    connect(_tabWidget, SIGNAL(currentChanged(int)), this, SLOT(_graphSelectSlot(int)));
    connect(_tabWidget, SIGNAL(tabCloseRequested(int)), this, SLOT(_graphCloseSlot(int)));

    _changingSelection = true;

    setCentralWidget(_tabWidget);

    resize(1024, 800);

    MainWindow::_oldMsgHandler = qInstallMsgHandler(&MainWindow::messageHandler);

    _usingCache= false;


}

MainWindow::~MainWindow()
{    
    qInstallMsgHandler(MainWindow::_oldMsgHandler);
    delete ModelsManager::getOrCreate();
    CallerManager::get()->deleteLater();
    delete _tabWidget;
    _transformTimer->deleteLater();
    CallerManager::get()->closeSession();
}

void MainWindow::messageHandler(QtMsgType type, const char *msg)
{
  if(MainWindow::_oldMsgHandler) _oldMsgHandler(type, msg);
  MainWindow::get()->addLogMessage(type, msg);
}

void MainWindow::addLogMessage(QtMsgType type, const QString & msg)
{
    _messagesWidget->appendMessage(type, msg);
}

void MainWindow::addInfoLogMessage(const QString & msg)
{
    _messagesWidget->appendMessage((QtMsgType)MessagesWidget::Infomsg, msg);
}

MainWindow * MainWindow::create(QWidget *parent)
{
    if (!_mainWindow)
        _mainWindow = new MainWindow(parent);
    return _mainWindow;
}

MainWindow * MainWindow::get()
{
    return _mainWindow;
}

TransformTimer * MainWindow::getTransformTimer()
{
  return _transformTimer;
}

bool MainWindow::isLayoutWeightedMod()
{
    return _aLayoutWeightedMod->isChecked();
}

bool MainWindow::isLayoutGroupChildMod()
{
    return _aLayoutGroupChildMod->isChecked();
}

void MainWindow::updateDocks()
{
    _entityDetailsWidget->setNodeList(_currentGraphView->getGraph()->getSelectedNodeList());
    QList<Node *> selectedNodes = _tabWidget->getGraphViewAt(_tabWidget->currentIndex())->getGraph()->getSelectedNodeList();
    setViewModel(selectedNodes);
}

/****************************************************************************
Protected Slots
****************************************************************************/

QObject* MainWindow::newGraph()
{
    addInfoLogMessage("Creation of a new graph");
    Graph * graph = Graph::newGraph(this);
    GraphView * view = new GraphView(graph,this);
    graph->setGraphView(view);
    int newIndex = _tabWidget->addTab(view, graph->getName());
    _tabWidget->setCurrentIndex(newIndex);
    connect(view->scene(), SIGNAL(selectionChanged()), this, SLOT(_selectionChangedSlot()));
    graph->setChanged(false);
    return qobject_cast<Graph*>(graph);
}

void MainWindow::_openGraphSlot()
{
    QSettings settings;
    QString path = settings.value("graphdir").toString();
    if(path.isEmpty()) path = "/home";
    QString fileName =
            QFileDialog::getOpenFileName(this,
                                         tr("Open File"),
                                         path,
                                         tr("File ( *.ntgxml *.mtg *.mtgxml *.ntg"));

    if (!fileName.isEmpty())
    {
        loadGraph(fileName);
        settings.setValue("graphdir", QFileInfo(fileName).absolutePath());
    }
}

void MainWindow::_saveGraphSlot()
{
    if (!_currentGraphView)
    {
        qCritical("No current GraphView to save");
        return;
    }

    Graph * graph = _currentGraphView->getGraph();
    if (!graph)
    {
        qCritical("Unable to found valid graph to save into current GraphView");
        return;
    }

    QString fileName = graph->getFileName();
    if (fileName.isEmpty())
        _saveAsGraphSlot();
    else
        saveGraph(graph,fileName);

}

void MainWindow::saveGraph(Graph* graph,QString fileName)
{
    QApplication::setOverrideCursor(Qt::WaitCursor);
    bool saved = graph->saveGraph(fileName);
    QApplication::restoreOverrideCursor();

    if (saved)
    {
        graph->setName(fileName.split("/").last());
        _updateRecentFile(fileName);
        _tabWidget->setTabText(_tabWidget->indexOf(graph->getView()), graph->getName());
        statusBar()->showMessage(tr("Graph saved"), 2000);
        graph->setChanged(false);
    }
    else
    {
        qCritical("Unable to save graph \"%s\"",qPrintable(fileName));
        statusBar()->showMessage(tr("Unable to save Graph !"), 2000);
    }
}

void MainWindow::_saveAsGraphSlot()
{
    if (!_currentGraphView)
    {
        qCritical("No current GraphView to save");
        return;
    }

    Graph * graph = _currentGraphView->getGraph();
    if (!graph)
    {
        qCritical("Unable to found valid graph to save into current GraphView");
        return;
    }

//    QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"),
//                                                    "/home/untitled.ntg",
//                                                    tr("File (*.mtg *.mtgxml *.ntgxml *.ntg);;NetGlub format [.ntg] (*.ntgxml);;NetGlub XML format [.ntgxml] (*.ntgxml);;Maltego file format [.mtg] (*.mtg);;Maltego file format translated in XML [.mtgxml] (*.mtgxml)"));

//    if (fileName.isEmpty())
    QSettings settings;
    QString path = settings.value("graphdir").toString();
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save Graph"),
                                                    path+"/untitled.ntg",
                                                    tr("File (*.mtg *.mtgxml *.ntgxml *.ntg);;NetGlub format [.ntg] (*.ntg);;NetGlub XML format [.ntgxml] (*.ntgxml);;Maltego file format [.mtg] (*.mtg);;Maltego file format translated in XML [.mtgxml] (*.mtgxml)"));
    if (!fileName.isEmpty())
    {
        settings.setValue("graphdir",QFileInfo(fileName).absolutePath());
        graph->setFileName(fileName);
        saveGraph(graph,fileName);
    }
}

void MainWindow::_openRecentFileSlot()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (action)
    {
        QString fileName = action->data().toString();
        loadGraph(fileName);
    }
}

void MainWindow::_aboutSlot()
{
    const char *htmlText =
    "<HTML>"
    "<p><b>NetGlub 1.0</b></p>"
    "<p>NetGlub is a really Open Source Information Gathering software</p>"
    "<p>This program is licensed under GPL v3 </p>"
    "<p> <a href=\"http://www.netglub.org\">http://www.netglub.org/</a> </p>"
    "<p>To draw graphs, this software uses  <a href=\"http://www.graphviz.org\">Graphviz</a> </p>"
    "<p>Copyright(c) 2010 Diateam</p>"
    "</HTML>";

    QMessageBox aboutNetGlub(QMessageBox::NoIcon,tr("About NetGlub"), htmlText,QMessageBox::Close,this);
    aboutNetGlub.setIconPixmap(QPixmap(":/images/logo.png"));
    aboutNetGlub.exec();
}

void MainWindow::_graphSelectSlot(int index)
{
    GraphView * view = _tabWidget->getGraphViewAt(index);
    if (!view)
    {
        _currentGraphView = NULL;
        QMapIterator<int, QPair<QAction *, QAction *> > i(_layoutActMap);
        while (i.hasNext())
        {
             i.next();
             i.value().first->setEnabled(false);
             i.value().first->setChecked(false);
             i.value().second->setEnabled(false);
             i.value().second->setChecked(false);
        }
        _saveAct->setEnabled(false);
        _saveAsAct->setEnabled(false);
        _exportToCsvAct->setEnabled(false);
        _exportScreenShotAct->setEnabled(false);
        _printAct->setEnabled(false);
        _selectAllAct->setEnabled(false);
        _inverseSelectionAct->setEnabled(false);

        _selectParentsAct->setEnabled(false);
        _addParentsAct->setEnabled(false);
        _selectChildrenAct->setEnabled(false);
        _addChildrenAct->setEnabled(false);
        _selectNeighborsAct->setEnabled(false);
        _addNeighborsAct->setEnabled(false);

        _zoomInAct->setEnabled(false);
        _zoomOutAct->setEnabled(false);
        _zoomFitBestAct->setEnabled(false);

        _layoutFreezeAct->setEnabled(false);
        _layoutRedrawAct->setEnabled(false);
        _layoutClearAllAct->setEnabled(false);

        _pasteAct->setEnabled(false);

        _drawEdgeAct->setEnabled(false);

        _searchLineEdit->setEnabled(false);
        _searchAction->setEnabled(false);
        _goToSearchSlotAct->setEnabled(false);
        _asComboBox->setEnabled(false);

        if (index!=-1) qCritical("Unable to find valid GraphView by index");
        return;
    }
    _currentGraphView = view;
    _saveAct->setEnabled(true);
    _saveAsAct->setEnabled(true);
    _exportToCsvAct->setEnabled(true);
    _exportScreenShotAct->setEnabled(true);
    _printAct->setEnabled(true);
    _selectAllAct->setEnabled(true);

    _zoomInAct->setEnabled(true);
    _zoomOutAct->setEnabled(true);
    _zoomFitBestAct->setEnabled(true);

    _layoutFreezeAct->setEnabled(true);
    _layoutFreezeAct->setChecked(_currentGraphView->isFreezed());


    _layoutRedrawAct->setEnabled(true);
    _layoutClearAllAct->setEnabled(true);

    if(_copyEntityList.size()!=0)
        _pasteAct->setEnabled(true);

    _drawEdgeAct->setEnabled(true);
    if(_currentGraphView->getDrawMod()==GraphView::DrawEdge)
        _drawEdgeAct->setChecked(true);
    else
        _drawEdgeAct->setChecked(false);



    _searchLineEdit->setEnabled(true);
    _goToSearchSlotAct->setEnabled(true);
    _searchAction->setEnabled(true);
    _asComboBox->setEnabled(true);

    refreshLayoutActions();

    QGraphicsScene * scene = _currentGraphView->scene();
    if (scene)
    {
        bool actEnable = _currentGraphView->isThereSelectedNtgItem();
        _inverseSelectionAct->setEnabled(actEnable);

        _selectParentsAct->setEnabled(actEnable);
        _addParentsAct->setEnabled(actEnable);
        _selectChildrenAct->setEnabled(actEnable);
        _addChildrenAct->setEnabled(actEnable);
        _selectNeighborsAct->setEnabled(actEnable);
        _addNeighborsAct->setEnabled(actEnable);
    }

    _selectionChangedSlot();

}

void MainWindow::_graphCloseSlot(int index)
{
    GraphView * view = _tabWidget->getGraphViewAt(index);
    if (!view)
    {
        qCritical("Unable to find valid GraphView by index");
        return;
    }
    Graph * graph = view->getGraph();
    if (graph)
    {
        if (graph->hasChanged())
        {
            QMessageBox::StandardButton reply;
            reply = QMessageBox::question(this, tr("Close Tab"),
                                            "you are about to close a non saved graph\n Are you sure you want to quit?",
                                            QMessageBox::Yes | QMessageBox::Save | QMessageBox::Cancel);

            if (reply == QMessageBox::Save)
                _saveGraphSlot();
            if (reply == QMessageBox::Yes || reply == QMessageBox::Save)
            {
                _tabWidget->removeTab(index);
                delete view;
                delete graph;
                _currentGraphView = NULL;
                _graphSelectSlot(_tabWidget->currentIndex ());
            }
        }
        else
        {
            _tabWidget->removeTab(index);
            delete view;
            delete graph;
            _currentGraphView = NULL;
            _graphSelectSlot(_tabWidget->currentIndex ());
        }

    }
}

void MainWindow::_selectionChangedSlot(void)
{
    bool actEnable = false;
    if (_currentGraphView)
        actEnable = _currentGraphView->isThereSelectedNtgItem();

    _inverseSelectionAct->setEnabled(actEnable);

    _selectParentsAct->setEnabled(actEnable);
    _addParentsAct->setEnabled(actEnable);
    _selectChildrenAct->setEnabled(actEnable);
    _addChildrenAct->setEnabled(actEnable);
    _selectNeighborsAct->setEnabled(actEnable);
    _addNeighborsAct->setEnabled(actEnable);

    _copyAct->setEnabled(actEnable);
    _cutAct->setEnabled(actEnable);
    _deleteAct->setEnabled(actEnable);

    if (!_tabWidget)
        return;

    if (_tabWidget->count() == 0)
        return;

    if(!_tabWidget->getGraphViewAt(_tabWidget->currentIndex()))
    {
        return;
    }

    if (_changingSelection)
    {

        QList<Node *> selectedNodes = _tabWidget->getGraphViewAt(_tabWidget->currentIndex())->getGraph()->getSelectedNodeList();


        setViewModel(selectedNodes);

    }
    else
    {
        _view->setModel(0);
        _entityDetailsWidget->setNodeList(QList<Node *>());
    }
    _changingSelection = true;




    if(actEnable == false && _currentGraphView != NULL)
    {

        QSet<Edge*> selectedEdges = _currentGraphView->getGraph()->getSelectedEdgeSet();
        if(selectedEdges.size()!=0)
            setViewModel(selectedEdges);
    }

}

void MainWindow::setViewModel(QList<Node*>& selectedNodes)
{
    if (selectedNodes.count() == 0)
    {
        _view->setModel(0);
    }
    else
    {
        TreeModel * model = NULL;
        if (selectedNodes.count() ==1)
        {
            model = selectedNodes.first()->getOrCreateTreeModel();
        }
        else if (selectedNodes.count() > 1)
        {
             model = new TreeModel(selectedNodes,this);
        }
        if (model)
            _view->setModel(model);
//        for (int column = 0; column < model->columnCount(); ++column)
//            _view->resizeColumnToContents(column);
        _view->resizeColumnToContents(0);

        QModelIndexList indexes = model->match(model->index(0,0),
                                               Qt::DisplayRole,
                                               "*",
                                               -1,
                                               Qt::MatchWildcard|Qt::MatchRecursive);
        _view->setEditTriggers(QAbstractItemView::AnyKeyPressed|
                              QAbstractItemView::CurrentChanged|
                              QAbstractItemView::SelectedClicked);
        foreach (QModelIndex index, indexes)
        {
            _view->expand(index);
            if (model->getDelegateMap().contains(index.data(Qt::DisplayRole).toString()))
                _view->setItemDelegateForRow(index.row(),model->getDelegateMap().value(index.data(Qt::DisplayRole).toString()));
//            if (index.data(Qt::DisplayRole))
//            _view->setItemDelegateForRow(index.row());;
        }
        _view->hideColumn(2);
    }


    _entityDetailsWidget->setNodeList(selectedNodes);
}

void MainWindow::setViewModel(QSet<Edge*> selectedEdges)
{
    QStandardItemModel * model = new QStandardItemModel();
    model->setHorizontalHeaderLabels(QStringList("Edges"));
    QStandardItem *parentItem = model->invisibleRootItem();

    QSetIterator<Edge*> itrSelecteedEges(selectedEdges);
    while(itrSelecteedEges.hasNext())
    {
        Edge * pEdge = itrSelecteedEges.next();
        QStandardItem * item0 = new QStandardItem("From : "+pEdge->getTailNode()->getEntityModel().longName + " To : "+pEdge->getHeadNode()->getEntityModel().longName);
        item0->setEditable(false);
        parentItem->appendRow(item0);

        if (!pEdge->getTransformName().isEmpty())
        {
            QStandardItem * item1 = new QStandardItem("Transform name : "+pEdge->getTransformName());
            item1->setEditable(false);
            item0->appendRow(item1);
        }
    }
    _view->setModel(model);
//    _view->expandAll();

    _entityDetailsWidget->setEdgeSet(selectedEdges);
}

void MainWindow::_layoutActSlot(QAction * action)
{
    Q_UNUSED(action);
    QAction * viewLayoutAct = _viewLayoutActGroup->checkedAction();
    QAction * layoutAct = _layoutActGroup->checkedAction();
    int layoutId = MiningBlockLayoutId;
    QMapIterator<int, QPair<QAction *, QAction *> > i(_layoutActMap);
    while (i.hasNext())
    {
         i.next();
         if (i.value().first == viewLayoutAct && i.value().second == layoutAct)
         {
            layoutId = i.key();
            break;
         }
    }
    if (_currentGraphView)
    {
        _currentGraphView->setLayoutId(layoutId);
        _currentGraphView->setFitInView(true);
        _currentGraphView->startLayout();
    }
}

void MainWindow::_selectActSlot(QAction * action)
{
    if(!_currentGraphView)
        return;

    disconnect(_currentGraphView->scene(), SIGNAL(selectionChanged()), 0, 0);

    QList<Node*> selectedNodes;
    QListIterator<Node*> itrNodeList(_currentGraphView->getGraph()->getNodeList());
    while(itrNodeList.hasNext())
    {
        Node * pNode = itrNodeList.next();
        if(pNode->getNtgItem()->isSelected())
            selectedNodes.append(pNode);
    }

    if(action==_selectParentsAct)
    {        
        QListIterator<Node*> itrSelectedNodes(selectedNodes);
        while(itrSelectedNodes.hasNext())
        {
            Node * pNode = itrSelectedNodes.next();
            pNode->getNtgItem()->setSelected(false);
            QSetIterator<Node*> itrNodes(pNode->parentsNodes());
            while(itrNodes.hasNext())
            {
                itrNodes.next()->getNtgItem()->setSelected(true);
            }
        }
    }
    else if(action==_addParentsAct)
    {
        QListIterator<Node*> itrSelectedNodes(selectedNodes);
        while(itrSelectedNodes.hasNext())
        {
            Node * pNode = itrSelectedNodes.next();
            QSetIterator<Node*> itrNodes(pNode->parentsNodes());
            while(itrNodes.hasNext())
            {
                itrNodes.next()->getNtgItem()->setSelected(true);
            }
        }
    }
    else if(action==_selectChildrenAct)
    {
        QListIterator<Node*> itrSelectedNodes(selectedNodes);
        while(itrSelectedNodes.hasNext())
        {
            Node * pNode = itrSelectedNodes.next();
            pNode->getNtgItem()->setSelected(false);
            QSetIterator<Node*> itrNodes(pNode->childsNodes());
            while(itrNodes.hasNext())
            {
                itrNodes.next()->getNtgItem()->setSelected(true);
            }
        }
    }
    else if(action==_addChildrenAct)
    {
        QListIterator<Node*> itrSelectedNodes(selectedNodes);
        while(itrSelectedNodes.hasNext())
        {
            Node * pNode = itrSelectedNodes.next();
            QSetIterator<Node*> itrNodes(pNode->childsNodes());
            while(itrNodes.hasNext())
            {
                itrNodes.next()->getNtgItem()->setSelected(true);
            }
        }
    }
    else if(action==_selectNeighborsAct)
    {
        QListIterator<Node*> itrSelectedNodes(selectedNodes);
        while(itrSelectedNodes.hasNext())
        {
            Node * pNode = itrSelectedNodes.next();
            pNode->getNtgItem()->setSelected(false);
            QSetIterator<Node*> itrNodes(pNode->childsNodes()+pNode->parentsNodes());
            while(itrNodes.hasNext())
            {
                itrNodes.next()->getNtgItem()->setSelected(true);
            }
        }
    }
    else if(action==_addNeighborsAct)
    {
        QListIterator<Node*> itrSelectedNodes(selectedNodes);
        while(itrSelectedNodes.hasNext())
        {
            Node * pNode = itrSelectedNodes.next();
            QSetIterator<Node*> itrNodes(pNode->childsNodes()+pNode->parentsNodes());
            while(itrNodes.hasNext())
            {
                itrNodes.next()->getNtgItem()->setSelected(true);
            }
        }
    }
    else if(action==_selectAllAct)
    {
        QListIterator<Node*> itrNodeList(_currentGraphView->getGraph()->getNodeList());
        while(itrNodeList.hasNext())
        {
            Node * pNode = itrNodeList.next();
            pNode->getNtgItem()->setSelected(true);
            _currentGraphView->setFocus();
        }

    }
    else if(action==_inverseSelectionAct)
    {
        QListIterator<Node*> itrNodeList(_currentGraphView->getGraph()->getNodeList());
        while(itrNodeList.hasNext())
        {
            Node * pNode = itrNodeList.next();
            if(pNode->getNtgItem()->isSelected())
                pNode->getNtgItem()->setSelected(false);
            else
                pNode->getNtgItem()->setSelected(true);
        }
    }

    connect(_currentGraphView->scene(), SIGNAL(selectionChanged()), this, SLOT(_selectionChangedSlot()));
    _selectionChangedSlot();
}


void MainWindow::_reloadSlot()
{
    ModelsManager::getOrCreate()->updateModels();
    _entityTreeWidget->update();
}

void MainWindow::selectNode(QModelIndex index)
{
    QList<Node *> selectedNodes = _tabWidget->getGraphViewAt(_tabWidget->currentIndex())->getGraph()->getSelectedNodeList();
    if (selectedNodes.count()>1)
    {
        foreach (Node * node, selectedNodes)
        {
            if (node->getId() !=_view->model()->data(index.sibling(index.row(),2),Qt::EditRole))
            {
                node->getNtgItem()->setSelected(false);

            }
        }
        QRectF bbox = _currentGraphView->allNodesBoundingBox();
        _currentGraphView->resetScene(bbox);

        bbox = _currentGraphView->selectedNodesBoundingBox();
        bbox.adjust(-bbox.width()*5,-bbox.height()*5,bbox.width()*5,bbox.height()*5);
        _currentGraphView->fitInView(bbox,Qt::KeepAspectRatio);
        _currentGraphView->setFocus();
    }
    else if(selectedNodes.count() == 1)
    {
        if (index.parent().row() == 2)
        {
            Node * node = selectedNodes.first();
            NtgFile file = node->getFileMap().value(_view->model()->data(index.sibling(index.row(),2)).toInt());

            if (file.shortName.endsWith(".svg") || file.shortName.endsWith(".png") || file.shortName.endsWith(".jpg") || file.shortName.endsWith(".gif"))
            {
                node->getNtgItem()->setSvg(file.id);
                _currentGraphView->redrawItem(node->getNtgItem());
            }
            else
            {
                if (!file.srcPath.isEmpty())
                    QDesktopServices::openUrl(QUrl::fromLocalFile(file.srcPath));
                else
                    QDesktopServices::openUrl(QUrl::fromLocalFile(file.cpPath));
            }
        }

    }
}

void MainWindow::_settingsSlot()
{
    QngSettings set;
    connect(&set,SIGNAL(reloadSession()),this,SLOT(_reloadSlot()));
    set.move(pos().x()+width()/2-(set.width()/2),
              pos().y()+height()/2-(set.height()/2));
    set.exec();
}


void MainWindow::useCache(bool useCache)
{
    _usingCache = useCache;
    for (int i =0 ; i< _tabWidget->count(); i++)
    {
        CallerManager::get()->setGraphEnableCache(_tabWidget->getGraphViewAt(i)->getGraph()->getGraphId(),useCache);
    }
}

void MainWindow::clearCache()
{
    CallerManager::get()->clearCache();
}

bool MainWindow::usingCache()
{
    return _usingCache;
}

void MainWindow::_windowsMenuSlot()
{
//    qWarning() << "_dockNumber" << _dockNumber << "_windowsMenu->actions().count()" << _windowsMenu->actions().count();
    for (int j = _dockNumber; j < _windowsMenu->actions().count(); j++)
    {
        _windowsMenu->removeAction(_windowsMenu->actions().at(j));
    }
    _windowSeparatorAct->setVisible(false);
    if (_tabWidget->count()> 0)
    {
        _windowSeparatorAct->setVisible(true);
        for (int i = 0; i< _tabWidget->count(); i++)
        {
//            _windowsMenu->addAction()
//            qWarning() << "yeeeaah!!!";
        }
    }
}

void MainWindow::_manageTransformSlot()
{
    ManageTransformDialog * manageTransform = new ManageTransformDialog(this);
    manageTransform->setAttribute(Qt::WA_DeleteOnClose,true);
    manageTransform->exec();
}

void MainWindow::_goToSearchSlot()
{
    _searchLineEdit->setFocus();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    bool hasToBeSaved = false;
    for(int i=0 ; i<_tabWidget->count(); i++)
    {
        if (_tabWidget->getGraphViewAt(i)->getGraph()->hasChanged())
        {
            hasToBeSaved =true;
            break;
        }
    }
    if (hasToBeSaved)
    {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, tr("Close Tab"),
                                        "you are about to close a non saved graph\n Are you sure you want to quit?",
                                        QMessageBox::Yes | QMessageBox::SaveAll | QMessageBox::Cancel);

        if (reply == QMessageBox::SaveAll)
        {
            for(int i=0 ; i<_tabWidget->count(); i++)
            {
                _currentGraphView = _tabWidget->getGraphViewAt(i);
                _saveGraphSlot();
            }
            hasToBeSaved = false;
            for(int i=0 ; i<_tabWidget->count(); i++)
            {
                if (_tabWidget->getGraphViewAt(i)->getGraph()->hasChanged())
                {
                    hasToBeSaved =true;
                    break;
                }
            }
            if (hasToBeSaved)
            {
                event->ignore();
            }
        }
        else if (reply == QMessageBox::Cancel)
            event->ignore();
    }

//    FileManager::getOrCreate()->deleteAllDir();
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{

    switch(event->key())
    {
    case Qt::Key_Delete:

        if(!_currentGraphView->deleteAllSelectedNodes())
            _currentGraphView->deleteAllSelectedEdges();
        break;
    }
//    QKeySequence
    QMainWindow::keyPressEvent(event);
}

/****************************************************************************
Protected
****************************************************************************/

#define XML_FAIL(format,...) {qCritical("XML Parse Error: " format, ##__VA_ARGS__);}
#define XML_WARN(format,...) {qWarning( "XML Parse Warning: " format, ##__VA_ARGS__);}

void MainWindow::_createActions()
{
    //FILE ACTIONS
    _newAct = new QAction(QIcon(":/images/new.png"), tr("&New"), this);
    _newAct->setShortcuts(QKeySequence::New);
    _newAct->setStatusTip(tr("New graph"));
    connect(_newAct, SIGNAL(triggered()), this, SLOT(newGraph()));

    _openAct = new QAction(QIcon(":/images/open.png"), tr("&Open..."), this);
    _openAct->setShortcuts(QKeySequence::Open);
    _openAct->setStatusTip(tr("Open an existing graph"));
    connect(_openAct, SIGNAL(triggered()), this, SLOT(_openGraphSlot()));

    _saveAct = new QAction(QIcon(":/images/save.png"), tr("&Save"), this);
    _saveAct->setShortcuts(QKeySequence::Save);
    _saveAct->setStatusTip(tr("Save the graph to disk"));
    _saveAct->setEnabled(false);
    connect(_saveAct, SIGNAL(triggered()), this, SLOT(_saveGraphSlot()));

    _saveAsAct = new QAction(QIcon(":/images/saveAs.png"), tr("Save &As..."), this);
    _saveAsAct->setShortcuts(QKeySequence::SaveAs);
    _saveAsAct->setStatusTip(tr("Save the graph under a new name"));
    _saveAsAct->setEnabled(false);
    connect(_saveAsAct, SIGNAL(triggered()), this, SLOT(_saveAsGraphSlot()));

    _exportToCsvAct = new QAction(tr("Export to &csv..."), this);
    _exportToCsvAct->setStatusTip(tr("Export the graph in a comma-separated values file"));
    _exportToCsvAct->setEnabled(false);
    connect(_exportToCsvAct, SIGNAL(triggered()), this, SLOT(_exportToCsv()));

    _exportScreenShotAct = new QAction(tr("&Export graph's screenshot as..."), this);
    _exportScreenShotAct->setStatusTip(tr("Export the observable graph part in a new image file"));
    _exportScreenShotAct->setEnabled(false);
    connect(_exportScreenShotAct, SIGNAL(triggered()), this, SLOT(_exportScreenShotSlot()));

    for (int i = 0; i < MaxRecentFiles; ++i) {
        _recentFileActs[i] = new QAction(this);
        _recentFileActs[i]->setVisible(false);
        connect(_recentFileActs[i], SIGNAL(triggered()), this, SLOT(_openRecentFileSlot()));
    }

    _printAct = new QAction(QIcon(":/images/print.png"), tr("&Print..."), this);
    _printAct->setShortcuts(QKeySequence::Print);
    _printAct->setStatusTip(tr("Print current graph"));
    _printAct->setEnabled(false);
    connect(_printAct,SIGNAL(triggered()),this,SLOT(_printSlot()));


    _quitAct = new QAction(QIcon(":/images/exit.png"), tr("&Quit"), this);
    _quitAct->setShortcuts(QKeySequence::Quit);
    _quitAct->setStatusTip(tr("Quit the application"));
    connect(_quitAct, SIGNAL(triggered()), qApp, SLOT(closeAllWindows()));

    _openScriptAct = new QAction(QIcon(":/images/openscript.png"), tr("O&pen Scenario"), this);
    _quitAct->setStatusTip(tr("Open scenario"));
    connect(_openScriptAct, SIGNAL(triggered()), this, SLOT(_openScriptSlot()));

    _saveScriptAct = new QAction(QIcon(":/images/fileexport.png"), tr("S&ave Scenario"), this);
    _quitAct->setStatusTip(tr("Save scenario"));
    connect(_saveScriptAct, SIGNAL(triggered()), this, SLOT(_saveScriptSlot()));

    //EDIT ACTIONS
    _cutAct = new QAction(QIcon(":/images/cut.png"), tr("C&ut"), this);
    _cutAct->setShortcuts(QKeySequence::Cut);
    _cutAct->setStatusTip(tr("Cut"));
    _cutAct->setEnabled(false);
    connect(_cutAct,SIGNAL(triggered()),this,SLOT(_cutSlot()));

    _copyAct = new QAction(QIcon(":/images/copy.png"), tr("Cop&y"), this);
    _copyAct->setShortcuts(QKeySequence::Copy);
    _copyAct->setStatusTip(tr("Copy"));
    _copyAct->setEnabled(false);
    connect(_copyAct,SIGNAL(triggered()),this,SLOT(_copySlot()));

    _pasteAct = new QAction(QIcon(":/images/paste.png"), tr("&Paste"), this);
    _pasteAct->setShortcuts(QKeySequence::Paste);
    _pasteAct->setStatusTip(tr("Paste"));
    _pasteAct->setEnabled(false);
    connect(_pasteAct,SIGNAL(triggered()),this,SLOT(_pasteSlot()));

    _deleteAct = new QAction(QIcon(":/images/delete.png"), tr("&Delete"), this);
    _deleteAct->setShortcuts(QKeySequence::Delete);
    _deleteAct->setStatusTip(tr("Delete"));
    _deleteAct->setEnabled(false);
    connect(_deleteAct,SIGNAL(triggered()),this,SLOT(_deleteSlot()));

    _selectAllAct = new QAction(QIcon(":/images/selectAll.png"), tr("&Select all"), this);
    _selectAllAct->setShortcuts(QKeySequence::SelectAll);
    _selectAllAct->setStatusTip(tr("Select all"));
    _selectAllAct->setEnabled(false);

    _inverseSelectionAct = new QAction(QIcon(":/images/inverseSelect.png"), tr("&Invert selection"), this);
    _inverseSelectionAct->setShortcuts(QKeySequence::Italic);
    _inverseSelectionAct->setStatusTip(tr("Invert current selection"));
    _inverseSelectionAct->setEnabled(false);

    _selectParentsAct = new QAction(QIcon(":/images/selectParents_48x48.png"),tr("Select &Parents"), this);
    _selectParentsAct->setStatusTip(tr("Select all parents"));
    _selectParentsAct->setEnabled(false);

    _addParentsAct = new QAction(QIcon(":/images/addParents_48x48.png"),tr("A&dd Parents"), this);
    _addParentsAct->setStatusTip(tr("Add all parents to the current selection"));
    _addParentsAct->setEnabled(false);

    _selectChildrenAct = new QAction(QIcon(":/images/selectChildren_48x48.png"),tr("Select &Children"), this);
    _selectChildrenAct->setStatusTip(tr("Select all children"));
    _selectChildrenAct->setEnabled(false);

    _addChildrenAct = new QAction(QIcon(":/images/addChildren_48x48.png"),tr("A&dd Children"), this);
    _addChildrenAct->setStatusTip(tr("Add all children to the current selection"));
    _addChildrenAct->setEnabled(false);

    _selectNeighborsAct = new QAction(QIcon(":/images/selectNeighbors_48x48.png"),tr("S&elect Neighbors"), this);
    _selectNeighborsAct->setStatusTip(tr("Select all neighbors"));
    _selectNeighborsAct->setEnabled(false);

    _addNeighborsAct = new QAction(QIcon(":/images/addNeighbors_48x48.png"),tr("&Add Neighbors"), this);
    _addNeighborsAct->setStatusTip(tr("Add all neighbors to the current selection"));
    _addNeighborsAct->setEnabled(false);

    _settingsAct = new QAction(tr("Setti&ngs..."), this);
    _settingsAct->setStatusTip(tr("Setings"));
    _settingsAct->setShortcuts(QKeySequence::Preferences);
    connect(_settingsAct,SIGNAL(triggered()),this,SLOT(_settingsSlot()));

    _selectActGroup = new QActionGroup(this);
    _selectActGroup->addAction(_selectParentsAct);
    _selectActGroup->addAction(_addParentsAct);
    _selectActGroup->addAction(_selectChildrenAct);
    _selectActGroup->addAction(_addChildrenAct);
    _selectActGroup->addAction(_selectNeighborsAct);
    _selectActGroup->addAction(_addNeighborsAct);
    _selectActGroup->addAction(_selectAllAct);
    _selectActGroup->addAction(_inverseSelectionAct);

    connect(_selectActGroup,SIGNAL(triggered(QAction*)),this,SLOT(_selectActSlot(QAction*)));

    _useCacheAct = new QAction(QIcon(":/images/cacheUse.png"),tr("Use Cache"), this);
    _useCacheAct->setStatusTip(tr("Use the Cache"));
    _useCacheAct->setToolTip("Use the Cache");
    _useCacheAct->setCheckable(true);
    _useCacheAct->setChecked(false);

    connect(_useCacheAct,SIGNAL(triggered(bool)),this,SLOT(useCache(bool)));

    _clearCache  = new QAction(QIcon(":/images/cacheClear.png"),tr("Clear Cache"), this);

    connect(_clearCache,SIGNAL(triggered()),this,SLOT(clearCache()));
    _clearCache->setEnabled(false);

    //LAYOUTS ACTIONS
    bool viewLayoutEnable = false;

    _miningViewLayoutAct = new QAction(tr("Mining view"), this);
    _miningViewLayoutAct->setCheckable(true);
    _miningViewLayoutAct->setEnabled(viewLayoutEnable);
    _centralityViewLayoutAct = new QAction(tr("Centrality view"), this);
    _centralityViewLayoutAct->setCheckable(true);
    _centralityViewLayoutAct->setEnabled(viewLayoutEnable);
    _edgeWeightedViewLayoutAct = new QAction(tr("Edge weighted view"), this);
    _edgeWeightedViewLayoutAct->setCheckable(true);
    _edgeWeightedViewLayoutAct->setEnabled(viewLayoutEnable);

    _viewLayoutActGroup = new QActionGroup(this);
    _viewLayoutActGroup->addAction(_miningViewLayoutAct);
    _viewLayoutActGroup->addAction(_centralityViewLayoutAct);
    _viewLayoutActGroup->addAction(_edgeWeightedViewLayoutAct);

    connect(_viewLayoutActGroup,SIGNAL(triggered(QAction*)),this,SLOT(_layoutActSlot(QAction*)));

    bool layoutEnable = false;

    _blockLayoutAct = new QAction(QIcon(":/images/topToBottom.png"),tr("&Block top to bottom"), this);
    _blockLayoutAct->setCheckable(true);
    _blockLayoutAct->setEnabled(layoutEnable);
    _hierarchyLayoutAct = new QAction(QIcon(":/images/leftToRight.png"),tr("&Hierarchy left to right"), this);
    _hierarchyLayoutAct->setCheckable(true);
    _hierarchyLayoutAct->setEnabled(layoutEnable);
    _circularLayoutAct = new QAction(QIcon(":/images/circular.png"),tr("&Circular"), this);
    _circularLayoutAct->setCheckable(true);
    _circularLayoutAct->setEnabled(layoutEnable);
    _molecularLayoutAct = new QAction(QIcon(":/images/molecular.png"),tr("&Molecular"), this);
    _molecularLayoutAct->setCheckable(true);
    _molecularLayoutAct->setEnabled(layoutEnable);

    _layoutActGroup = new QActionGroup(this);
    _layoutActGroup->addAction(_blockLayoutAct);
    _layoutActGroup->addAction(_hierarchyLayoutAct);
    _layoutActGroup->addAction(_circularLayoutAct);
    _layoutActGroup->addAction(_molecularLayoutAct);
    connect(_layoutActGroup,SIGNAL(triggered(QAction*)),this,SLOT(_layoutActSlot(QAction*)));

    _layoutActMap.insert(MiningBlockLayoutId,qMakePair(_miningViewLayoutAct,_blockLayoutAct));
    _layoutActMap.insert(MiningHierarchyLayoutId,qMakePair(_miningViewLayoutAct,_hierarchyLayoutAct));
    _layoutActMap.insert(MiningCircularLayoutId,qMakePair(_miningViewLayoutAct,_circularLayoutAct));
    _layoutActMap.insert(MiningMolecularLayoutId,qMakePair(_miningViewLayoutAct,_molecularLayoutAct));

    _layoutActMap.insert(CentralityBlockLayoutId,qMakePair(_centralityViewLayoutAct,_blockLayoutAct));
    _layoutActMap.insert(CentralityHierarchyLayoutId,qMakePair(_centralityViewLayoutAct,_hierarchyLayoutAct));
    _layoutActMap.insert(CentralityCircularLayoutId,qMakePair(_centralityViewLayoutAct,_circularLayoutAct));
    _layoutActMap.insert(CentralityMolecularLayoutId,qMakePair(_centralityViewLayoutAct,_molecularLayoutAct));

    _layoutActMap.insert(EdgeWeightedBlockLayoutId,qMakePair(_edgeWeightedViewLayoutAct,_blockLayoutAct));
    _layoutActMap.insert(EdgeWeightedHierarchyLayoutId,qMakePair(_edgeWeightedViewLayoutAct,_hierarchyLayoutAct));
    _layoutActMap.insert(EdgeWeightedCircularLayoutId,qMakePair(_edgeWeightedViewLayoutAct,_circularLayoutAct));
    _layoutActMap.insert(EdgeWeightedMolecularLayoutId,qMakePair(_edgeWeightedViewLayoutAct,_molecularLayoutAct));


    _layoutFreezeAct = new QAction(QIcon(":/images/freeze.png"),tr("&Freeze Layout"), this);
    _layoutFreezeAct->setStatusTip(tr("freeze movement of graph (no addition and no movement)"));
    _layoutFreezeAct->setEnabled(false);
    _layoutFreezeAct->setCheckable(true);
    connect(_layoutFreezeAct,SIGNAL(toggled(bool)),this,SLOT(_freezedActSlot(bool)));

    _layoutRedrawAct = new QAction(QIcon(":/images/redraw.png"),tr("&Redraw Layout"), this);
    _layoutRedrawAct->setStatusTip(tr("Redraw the layout using the current layout configuration"));
    _layoutRedrawAct->setEnabled(false);
    _layoutClearAllAct = new QAction(QIcon(":/images/clear.png"),tr("&Clear all"), this);
    _layoutClearAllAct->setStatusTip(tr("Clear all entities in this graph"));
    _layoutClearAllAct->setEnabled(false);
    _miscLayoutActGroup = new QActionGroup(this);
    _miscLayoutActGroup->addAction(_layoutRedrawAct);
    _miscLayoutActGroup->addAction(_layoutClearAllAct);
    connect(_miscLayoutActGroup,SIGNAL(triggered(QAction*)),this,SLOT(_miscActLayoutSlot(QAction*)));

    //HELP ACTIONS

    _aboutAct = new QAction(tr("&About"), this);
    _aboutAct->setStatusTip(tr("Show the application's About box"));
    connect(_aboutAct, SIGNAL(triggered()), this, SLOT(_aboutSlot()));

    _aboutQtAct = new QAction(tr("About &Qt"), this);
    _aboutQtAct->setStatusTip(tr("Show the Qt library's About box"));
    connect(_aboutQtAct, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

    _aLayoutWeightedMod = new QAction(tr("&Weighted mod"),this);
    _aLayoutWeightedMod->setCheckable(true);
    _aLayoutGroupChildMod = new QAction(tr("&Group childs mod"),this);
    _aLayoutGroupChildMod->setCheckable(true);

    _agLayoutTypeGraphOption = new QActionGroup(this);
    QAction * agraph = new QAction(tr("&AGRAPH"),_agLayoutTypeGraphOption);
    agraph->setCheckable(true);
    agraph = new QAction(tr("&AGRAPHSTRICT"),_agLayoutTypeGraphOption);
    agraph->setCheckable(true);
    agraph = new QAction(tr("&AGDIGRAPH"),_agLayoutTypeGraphOption);
    agraph->setCheckable(true);
    agraph->setChecked(true);
    agraph = new QAction(tr("&AGDIGRAPHSTRICT"),_agLayoutTypeGraphOption);
    agraph->setCheckable(true);

    _aLayoutDirectionOption = new QAction("left to right",this);
    _aLayoutDirectionOption->setCheckable(true);

    //TOOLS ACTIONS
    _manageTransformAct = new QAction(tr("&Manage Transforms..."), this);
    _manageTransformAct->setStatusTip(tr("Manage all transforms"));
    connect(_manageTransformAct,SIGNAL(triggered()),this,SLOT(_manageTransformSlot()));
//    _manageTransformAct->setEnabled(false);

    _drawEdgeAct = new QAction(QIcon(":/images/edge.png"),tr("&Edge"), this);
    _drawEdgeAct->setCheckable(true);
    _drawEdgeAct->setChecked(false);
    _drawEdgeAct->setEnabled(false);
    connect(_drawEdgeAct,SIGNAL(triggered(bool)),this,SLOT(_drawModeSlot(bool)));

    //ZOOMS ACTIONS
    _zoomInAct = new QAction(QIcon(":/images/zoomIn.png"),tr("Zoom &In"), this);
    _zoomInAct->setEnabled(false);
    _zoomOutAct = new QAction(QIcon(":/images/zoomOut.png"),tr("Zoom &Out"), this);
    _zoomOutAct->setEnabled(false);
    _zoomFitBestAct = new QAction(QIcon(":/images/zoomFitBest.png"),tr("Zoom &Fit Best"), this);
    _zoomFitBestAct->setEnabled(false);

    _zoomActGroup = new QActionGroup(this);
    _zoomActGroup->addAction(_zoomInAct);
    _zoomActGroup->addAction(_zoomOutAct);
    _zoomActGroup->addAction(_zoomFitBestAct);
    connect(_zoomActGroup,SIGNAL(triggered(QAction*)),this,SLOT(_zoomActSlot(QAction*)));

    _reloadAct = new QAction(tr("&Reload session"), this);
    connect(_reloadAct,SIGNAL(triggered()),this,SLOT(_reloadSlot()));

    _goToSearchSlotAct = new QAction(QIcon(":/images/find.png"),tr("Search"), this);
    _goToSearchSlotAct->setShortcut(QKeySequence::Find);
    _goToSearchSlotAct->setEnabled(false);
    connect(_goToSearchSlotAct,SIGNAL(triggered()),this,SLOT(_goToSearchSlot()));

    //FIND ACTIONS
    _searchAction = new QAction(QIcon(":/images/find.png"),tr("Search"), this);

    _searchAction->setEnabled(false);
    connect(_searchAction,SIGNAL(triggered()),this,SLOT(_searchSlot()));




}

void MainWindow::_createMenus()
{
    _fileMenu = menuBar()->addMenu(tr("&File"));
    _fileMenu->addAction(_newAct);
    _fileMenu->addAction(_openAct);
    _fileMenu->addAction(_saveAct);
    _fileMenu->addAction(_saveAsAct);

    _separatorAct = _fileMenu->addSeparator();
    _fileMenu->addAction(_exportToCsvAct);
    _fileMenu->addAction(_exportScreenShotAct);

    _fileMenu->addSeparator();
    _fileMenu->addAction(_openScriptAct);
    _fileMenu->addAction(_saveScriptAct);

    _separatorAct = _fileMenu->addSeparator();
    for (int i = 0; i < MaxRecentFiles; ++i)
        _fileMenu->addAction(_recentFileActs[i]);
    _fileMenu->addSeparator();
    _fileMenu->addAction(_printAct);
    _fileMenu->addSeparator();
    _fileMenu->addAction(_quitAct);

    _updateRecentFileActions();

    //menuBar()->addSeparator();


    _editMenu = menuBar()->addMenu(tr("&Edit"));
    _editMenu->addAction(_cutAct);
    _editMenu->addAction(_copyAct);
    _editMenu->addAction(_pasteAct);
    _editMenu->addAction(_deleteAct);
    _editMenu->addAction(_selectAllAct);
    _editMenu->addAction(_inverseSelectionAct);
    _editMenu->addSeparator();
    _editMenu->addAction(_selectParentsAct);
    _editMenu->addAction(_addParentsAct);
    _editMenu->addAction(_selectChildrenAct);
    _editMenu->addAction(_addChildrenAct);
    _editMenu->addAction(_selectNeighborsAct);
    _editMenu->addAction(_addNeighborsAct);
    _editMenu->addSeparator();
    _editMenu->addAction(_settingsAct);


    _toolsMenu = menuBar()->addMenu(tr("&Tools"));
    _toolsMenu->addAction(_manageTransformAct);
    _toolsMenu->addSeparator();
    _toolsMenu->addAction(_zoomInAct);
    _toolsMenu->addAction(_zoomOutAct);
    _toolsMenu->addAction(_zoomFitBestAct);
    _toolsMenu->addAction(_goToSearchSlotAct);
    _toolsMenu->addSeparator();
    _toolsMenu->addAction(_reloadAct);

    _layoutsMenu = menuBar()->addMenu(tr("&Layouts"));

    _layoutsMenu->addAction(_miningViewLayoutAct);
    _layoutsMenu->addAction(_centralityViewLayoutAct);
    _layoutsMenu->addAction(_edgeWeightedViewLayoutAct);

    _layoutsMenu->addSeparator();

    _layoutsMenu->addAction(_blockLayoutAct);
    _layoutsMenu->addAction(_hierarchyLayoutAct);
    _layoutsMenu->addAction(_circularLayoutAct);
    _layoutsMenu->addAction(_molecularLayoutAct);

    _layoutsMenu->addSeparator();
    _layoutsMenu->addAction(_layoutRedrawAct);
    _layoutsMenu->addAction(_layoutFreezeAct);
    _layoutsMenu->addAction(_layoutClearAllAct);

    menuBar()->addSeparator();

    _windowsMenu = menuBar()->addMenu(tr("&Windows"));
    connect(_windowsMenu,SIGNAL(aboutToShow()), this, SLOT(_windowsMenuSlot()));


    _helpMenu = menuBar()->addMenu(tr("&Help"));
    _helpMenu->addAction(_aboutAct);
    _helpMenu->addAction(_aboutQtAct);

}

void MainWindow::_createToolBars()
{
    _fileToolBar = addToolBar(tr("File"));
    _fileToolBar->addAction(_newAct);
    _fileToolBar->addAction(_openAct);
    _fileToolBar->addAction(_saveAct);
    _fileToolBar->addAction(_saveAsAct);
    _fileToolBar->addAction(_quitAct);

    _editToolBar = addToolBar(tr("Edit"));
    _editToolBar->addAction(_cutAct);
    _editToolBar->addAction(_copyAct);
    _editToolBar->addAction(_pasteAct);
    _editToolBar->addAction(_deleteAct);
    _editToolBar->addAction(_selectAllAct);

    _selectToolBar = addToolBar(tr("Select"));
    _selectToolBar->addAction(_selectParentsAct);
    _selectToolBar->addAction(_addParentsAct);
    _selectToolBar->addAction(_selectChildrenAct);
    _selectToolBar->addAction(_addChildrenAct);
    _selectToolBar->addAction(_selectNeighborsAct);
    _selectToolBar->addAction(_addNeighborsAct);

    _zoomsToolBar = addToolBar(tr("Zooms"));
    _zoomsToolBar->addAction(_zoomInAct);
    _zoomsToolBar->addAction(_zoomOutAct);
    _zoomsToolBar->addAction(_zoomFitBestAct);
    _zoomsToolBar->addSeparator();
    _zoomsToolBar->addAction(_useCacheAct);
    _zoomsToolBar->addAction(_clearCache);

    foreach(QToolButton* pButton, _zoomsToolBar->findChildren<QToolButton*>())
    {
        if (pButton->defaultAction() == _zoomInAct ||
            pButton->defaultAction() == _zoomOutAct)
        {
            pButton->setAutoRepeat(true);
            pButton->setAutoRepeatInterval(33);
            pButton->setAutoRepeatDelay(0);
        }
    }



    addToolBarBreak();

    _layoutsToolBar = addToolBar(tr("Layouts"));
    _layoutsToolBar->addAction(_miningViewLayoutAct);
    _layoutsToolBar->addAction(_centralityViewLayoutAct);
    _layoutsToolBar->addAction(_edgeWeightedViewLayoutAct);


    _layoutsToolBar->addAction(_blockLayoutAct);
    _layoutsToolBar->addAction(_hierarchyLayoutAct);
    _layoutsToolBar->addAction(_circularLayoutAct);
    _layoutsToolBar->addAction(_molecularLayoutAct);

    _layoutsToolBar->addSeparator();
    _layoutsToolBar->addAction(_layoutRedrawAct);
    _layoutsToolBar->addAction(_layoutFreezeAct);
    _layoutsToolBar->addAction(_layoutClearAllAct);
    _layoutsToolBar->addAction(_drawEdgeAct);

    addToolBarBreak();

    _searchToolBar = addToolBar(tr("Search"));
    _searchToolBar->addWidget(new QLabel("Search : ",_searchToolBar));

    _searchLineEdit = new QLineEdit(_searchToolBar);
    _searchLineEdit->setFixedSize(200,25);

//    _searchLineEdit->setShortcutEnabled();
    _searchLineEdit->setEnabled(false);
    _searchToolBar->addWidget(_searchLineEdit);
    _searchToolBar->addWidget(new QLabel(" As : ",_searchToolBar));
    _asComboBox = new QComboBox(_searchToolBar);
//    _asComboBox->addItem("All");

    connect(_searchLineEdit,SIGNAL(returnPressed()),this,SLOT(_searchSlot()));

    QHash<QString,NtgEntityModel> entityModelHash = ModelsManager::getOrCreate()->getEntityModelHash();//CallerManager::get()->getAllEntities();
    QStandardItemModel * model = new QStandardItemModel(this);
    QList<QStandardItem*> list;
    list << new QStandardItem("All") << new QStandardItem("all");
    model->appendRow(list);

    foreach (NtgEntityModel entity, entityModelHash)
    {
        list.clear();
        list << new QStandardItem(entity.longName) << new QStandardItem(entity.name);
        model->appendRow(list);
    }
    model->sort(0,Qt::AscendingOrder);
    _asComboBox->setEnabled(false);
    _asComboBox->setModel(model);
    _searchToolBar->addWidget(_asComboBox);
    _searchToolBar->addAction(_searchAction);
//    _searchToolBar->addAction(_goToSearchSlotAct);
}

void MainWindow::_createStatusBar()
{
    statusBar()->showMessage(tr("Ready"));
}

void MainWindow::_createDockWindows()
{
  QDockWidget * dock = NULL;
  _dockNumber = 0;
  /****************************************************************************
  STEP 1 : "Messages" dockable widget creation
  ****************************************************************************/

  dock = new QDockWidget("Messages",this);
  dock->setAllowedAreas(Qt::BottomDockWidgetArea);
  _messagesWidget = new MessagesWidget(this);
  dock->setWidget(_messagesWidget);
  addDockWidget(Qt::BottomDockWidgetArea,dock);
  _windowsMenu->addAction(dock->toggleViewAction());
  _dockNumber++;

  /****************************************************************************
  STEP 2 : "Palette" dockable widget creation
  ****************************************************************************/

  dock = new QDockWidget(tr("Palette"), this);
  dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
  _entityTreeWidget = new EntityTreeWidget(dock);
  dock->setWidget(_entityTreeWidget);
  addDockWidget(Qt::LeftDockWidgetArea, dock);
  _windowsMenu->addAction(dock->toggleViewAction());
  _dockNumber++;
  _entityTreeWidget->update();


  /****************************************************************************
  STEP 3 : "Details" dockable widget creation
  ****************************************************************************/

  dock = new QDockWidget("Details",this);
  dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
  _entityDetailsWidget = new DetailsWidget(this);
  _entityDetailsWidget->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Maximum);
  dock->setWidget(_entityDetailsWidget);
  addDockWidget(Qt::RightDockWidgetArea,dock);
  _windowsMenu->addAction(dock->toggleViewAction());
  _dockNumber++;

  /****************************************************************************
  STEP 4 : "Properties" dockable widget creation
  ****************************************************************************/

  dock = new QDockWidget("Properties",this);
  dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
  //_entityPropertiesWidget = new EntityPropertiesWidget(this);
  _view = new QTreeView(this);
  _view->setObjectName(QString::fromUtf8("_view"));
  _view->setAlternatingRowColors(true);
  _view->setSelectionBehavior(QAbstractItemView::SelectRows);
  _view->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
  _view->setAnimated(false);
  _view->setAllColumnsShowFocus(true);
//  _view->setItemDelegateForRow();;
  connect(_view,SIGNAL(doubleClicked(QModelIndex)),this,SLOT(selectNode(QModelIndex)));
  dock->setWidget(_view);
  addDockWidget(Qt::RightDockWidgetArea,dock);
  _windowsMenu->addAction(dock->toggleViewAction());
  _dockNumber++;


  /****************************************************************************
  STEP 5 : "Progress" dockable widget creation
  ****************************************************************************/

  dock = new QDockWidget("Progress",this);
  dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
  _progressWidget = new ProgressWidget(this);
  dock->setWidget(_progressWidget);
  _windowsMenu->addAction(dock->toggleViewAction());
  _dockNumber++;

  connect(_transformTimer,SIGNAL(beginTransform()),_progressWidget,SLOT(addTransform()));
  connect(_transformTimer,SIGNAL(transformFinished()),_progressWidget,SLOT(endTransform()));
  connect(_progressWidget->stopButton,SIGNAL(clicked()),_transformTimer,SLOT(stopTransforms()));

  dock->resize(_progressWidget->size());
  addDockWidget(Qt::RightDockWidgetArea,dock);

  /****************************************************************************
  STEP 6 : "Script" dockable widget creation
  ****************************************************************************/


  dock = new QDockWidget("Scenario",this);
  dock->setAllowedAreas(Qt::BottomDockWidgetArea|Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
  _scriptWidget = new ScriptWidget(this);
  dock->setWidget(_scriptWidget);
  _windowsMenu->addAction(dock->toggleViewAction());
  _dockNumber++;


  _windowSeparatorAct = _windowsMenu->addSeparator();

  addDockWidget(Qt::BottomDockWidgetArea,dock);


}

QObject* MainWindow::loadGraph(const QString & fileName)
{
    setSelectable(false);

    Graph * graph = NULL;
    GraphView * view = NULL;
    for (int i=0; i < _tabWidget->count(); i++)
    {
      view = _tabWidget->getGraphViewAt(i);
      if (view)
      {
          graph = view->getGraph();
          if ((graph) && (graph->getFileName() == fileName))
          {
              _tabWidget->setCurrentIndex(i);
              QApplication::restoreOverrideCursor();
              return qobject_cast<Graph*>(graph);
          }
      }
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);

    graph = Graph::loadGraph(fileName, this);

    QApplication::restoreOverrideCursor();

    if (graph)
    {
        _updateRecentFile(fileName);
        GraphView * view = new GraphView(graph,this);
        graph->setGraphView(view);

        int newIndex = _tabWidget->addTab(view, graph->getName());
        _tabWidget->setCurrentIndex(newIndex);

        view->addGraphItems();

        connect(view->scene(), SIGNAL(selectionChanged()), this, SLOT(_selectionChangedSlot()));
        statusBar()->showMessage(tr("Graph loaded"), 2000);
        view->setFitInView(true);
        view->startLayout();
        graph->setChanged(false);
    }
    else
    {
        qCritical("Unable to load graph \"%s\"",qPrintable(fileName));
        statusBar()->showMessage(tr("Unable to load Graph !"), 2000);
    }
    return qobject_cast<Graph*>(graph);
}

void MainWindow::_updateRecentFile(const QString &fileName)
{
    QSettings settings;
    QStringList files = settings.value("recentFileList").toStringList();
    files.removeAll(fileName);
    files.prepend(fileName);
    while (files.size() > MaxRecentFiles)
        files.removeLast();

    settings.setValue("recentFileList", files);
    _updateRecentFileActions();
}

void MainWindow::_updateRecentFileActions()
{
    QSettings settings;
    QStringList files = settings.value("recentFileList").toStringList();

    int numRecentFiles = qMin(files.size(), (int)MaxRecentFiles);

    for (int i = 0; i < numRecentFiles; ++i) {
        QString text = tr("&%1 %2").arg(i + 1).arg(_strippedName(files[i]));
        _recentFileActs[i]->setText(text);
        _recentFileActs[i]->setData(files[i]);
        _recentFileActs[i]->setVisible(true);
    }

    for (int j = numRecentFiles; j < MaxRecentFiles; ++j)
        _recentFileActs[j]->setVisible(false);

    _separatorAct->setVisible(numRecentFiles > 0);
}

QString MainWindow::_strippedName(const QString &fullFileName)
{
    return QFileInfo(fullFileName).fileName();
}

void MainWindow::_zoomActSlot(QAction * action)
{
    if(_currentGraphView)
    {
        if(action==_zoomInAct)
        {
            _currentGraphView->scaleView(1.1);
        }
        else if(action==_zoomOutAct)
        {
            _currentGraphView->scaleView(0.9);
        }
        else if(action==_zoomFitBestAct)
        {
            if(_currentGraphView->isThereSelectedNtgItem()==false)
            {
                QRectF bbox = _currentGraphView->allNodesBoundingBox();
                _currentGraphView->resetScene(bbox);
                bbox.adjust(-bbox.width()/8,-bbox.height()/8,bbox.width()/8,bbox.height()/8);
                _currentGraphView->fitInView(bbox,Qt::KeepAspectRatio);
            }
            else
            {
                QRectF bbox = _currentGraphView->allNodesBoundingBox();
                _currentGraphView->resetScene(bbox);

                bbox = _currentGraphView->selectedNodesBoundingBox();
                bbox.adjust(-bbox.width()/4,-bbox.height()/4,bbox.width()/4,bbox.height()/4);
                _currentGraphView->fitInView(bbox,Qt::KeepAspectRatio);
            }
            _currentGraphView->refreshView();
        }
    }
    else
    {
        qCritical("MainWindow::_zoomActSlot -> no currentGraphView");
    }
}

void MainWindow::_miscActLayoutSlot(QAction * action)
{
    if(action==_layoutRedrawAct)
    {
        _currentGraphView->setFitInView(true);
        _currentGraphView->startLayout();
    }
    else if(action==_layoutClearAllAct)
    {
        QMessageBox::StandardButton reply;

        if (!_currentGraphView->getGraph()->getNodeList().isEmpty())
        {
            reply = QMessageBox::question(this, tr("Clear All"),
                                            "you are about to delete all current nodes, are you sure about that?",
                                            QMessageBox::Yes | QMessageBox::Cancel);
            if (reply == QMessageBox::Yes)
                _currentGraphView->deleteAllNodes();
        }
    }
}

void MainWindow::_freezedActSlot(bool checked)
{
    _currentGraphView->setFreezed(checked);
}

void MainWindow::_copy()
{
    QList<Node*> tmpNodeList;
    QPolygonF polygon;
    QListIterator<Node*> itrNodeList(_currentGraphView->getGraph()->getNodeList());
    while(itrNodeList.hasNext())
    {
        Node * pNode = itrNodeList.next();
        if(pNode->getNtgItem()->isSelected())
        {
            tmpNodeList.append(pNode);
            QPointF pos = pNode->getNtgItem()->pos();
            polygon << pos;
        }
    }
    QRectF brect = polygon.boundingRect();
    QPointF topLeft = brect.topLeft();

    if(tmpNodeList.size()==0)
        return;

    _copyEntityList.clear();
    _copyEdgeList.clear();

    for(int i(0);i<tmpNodeList.size();++i)
    {
        Node * from = tmpNodeList.at(i);
        QPair<QPointF, NtgEntity> pair;
        QPointF pos = from->getNtgItem()->pos() - topLeft;
        pair.first=pos;

        pair.second = from->getEntity();
        _copyEntityList.append(pair);

        QSetIterator<Edge*> itrEdge(from->getOutEdgeSet());
        while(itrEdge.hasNext())
        {
            Node * to = itrEdge.next()->getHeadNode();
            if(to->getNtgItem()->isSelected())
            {
                QPair<int,int> pair;
                pair.first = tmpNodeList.indexOf(from);
                pair.second = tmpNodeList.indexOf(to);
                _copyEdgeList.append(pair);
            }
        }
    }
    _pasteAct->setEnabled(true);
}

void MainWindow::_copySlot()
{       
    if(_currentGraphView)
    {
        _copy();
    }
}

void MainWindow::_pasteSlot()
{
    if(_currentGraphView)
    {
        disconnect(_currentGraphView->scene(), SIGNAL(selectionChanged()), 0, 0);

        QPointF cursor = QCursor::pos();
        cursor = _currentGraphView->mapFromGlobal(QPoint(cursor.x(),cursor.y()));
        //if(!_currentGraphView->sceneRect().contains(_currentGraphView->mapToScene(QPoint(cursor.x(),cursor.y()))))
        QRectF rect = _currentGraphView->viewport()->rect();
        if(!rect.contains(cursor))
            cursor = QPointF(0,0);
        cursor = _currentGraphView->mapToScene(QPoint(cursor.x(),cursor.y()));

        int nbnodes = _currentGraphView->getGraph()->getNodeList().size();

        QListIterator<QPair<QPointF, NtgEntity> > itrNodeList(_copyEntityList);
        while(itrNodeList.hasNext())
        {
            QPair<QPointF,NtgEntity> pair = itrNodeList.next();
            Node * pNode = _currentGraphView->getGraph()->newNode();
            pNode->setEntity(pair.second);
            pNode->setEntityModel(ModelsManager::getOrCreate()->getEntityModel(pair.second.type));

            NtgNodeItem * nodeItem = pNode->newNtgItem();
            QPointF pos = pair.first;
            nodeItem->setPos(QPoint(cursor.x(),cursor.y())+QPoint(pos.x(),pos.y()));

            _currentGraphView->scene()->addItem(nodeItem);
            _currentGraphView->redrawItem(nodeItem);
            nodeItem->setSelected(true);
        }

        QList<Node*> nodeList = _currentGraphView->getGraph()->getNodeList();

        QListIterator<QPair<int,int> > itrEdgeList(_copyEdgeList);
        while(itrEdgeList.hasNext())
        {
            QPair<int,int> pair = itrEdgeList.next();
            Edge * edge = _currentGraphView->getGraph()->newEdge(nodeList.at(nbnodes+pair.first),nodeList.at(nbnodes+pair.second));
            if (edge != NULL)
            {
                EdgeItem * edgeItem = edge->newItem();
                _currentGraphView->scene()->addItem(edgeItem);
            }
        }

        connect(_currentGraphView->scene(), SIGNAL(selectionChanged()), this, SLOT(_selectionChangedSlot()));
        _selectionChangedSlot();
    }
}


void MainWindow::_cutSlot()
{
    if(_currentGraphView)
    {
        _copy();
        _currentGraphView->deleteAllSelectedNodes();
    }
}

void MainWindow::_deleteSlot()
{
    _currentGraphView->deleteAllSelectedNodes();
}

void MainWindow::setSelectable(bool bo)
{
    _changingSelection = bo;
}

void MainWindow::_drawModeSlot(bool checked)
{
    Q_UNUSED(checked);
    if(_currentGraphView)
    {
        if(_drawEdgeAct->isChecked())
        {
            _currentGraphView->setDrawMod(GraphView::DrawEdge);
        }
        else
        {
            _currentGraphView->setDrawMod(GraphView::Default);
        }
    }
}

void MainWindow::_printSlot()
{
    if(_currentGraphView!=NULL)
    {
        QPrinter printer;
        QPrintDialog printDialog(&printer,this);
        if(printDialog.exec() == QDialog::Accepted)
        {
            QPainter painter(&printer);
            _currentGraphView->render(&painter);
        }
    }
}

void MainWindow::_exportToCsv()
{
    if(_currentGraphView!=NULL)
    {
        QSettings settings;
        QString exportpath = settings.value("exportpath").toString();
        QString fileName = QFileDialog::getSaveFileName(this, tr("Export to csv"),
                                                        exportpath+"/untitled.csv",
                                                        tr("File (*.csv)"));
        if (fileName.isEmpty())
        {
            return;
        }
        _currentGraphView->getGraph()->exportToCsv(fileName);
        settings.setValue("exportpath",QFileInfo(fileName).absolutePath());
    }
}

void MainWindow::_exportScreenShotSlot()
{
    if(_currentGraphView!=NULL)
    {
        QSettings settings;
        QString exportpath = settings.value("exportpath").toString();
        QString fileName = QFileDialog::getSaveFileName(this, tr("Export to png"),
                                                        exportpath+"/untitled.png",
                                                        tr("File (*.png)"));
        if (fileName.isEmpty())
        {
            return;
        }

        if(fileName.endsWith("png",Qt::CaseInsensitive))
        {
            QImage image(_currentGraphView->viewport()->rect().width(),_currentGraphView->viewport()->rect().height(),QImage::Format_ARGB32);
            image.fill(0);
            QPainter painter(&image);
            _currentGraphView->viewport()->render(&painter);

            bool ret = image.save(fileName,"png",100);
            if(ret==false)
                qWarning("graph not exported");
            else
                MainWindow::get()->addInfoLogMessage("graph successfully exported ");
            settings.setValue("exportpath",QFileInfo(fileName).absolutePath());
        }
        else
        {
            qWarning("filetype not supported yet, try png");
        }

    }
}

void MainWindow::_searchSlot()
{
    if(_currentGraphView!=NULL)
    {
        QModelIndexList indexes = _asComboBox->model()->match(_asComboBox->model()->index(0,0),
                                               Qt::DisplayRole,
                                               "*",
                                               -1,
                                               Qt::MatchWildcard|Qt::MatchRecursive);
        QModelIndex currentIndex;
        foreach (QModelIndex index, indexes)
        {
            if (index.row() == _asComboBox->currentIndex())
            {
                currentIndex = index;
                break;
            }
        }
        if(currentIndex.row() <0)
            return;

        QList<Node*> mNodeList;

        QString str = _searchLineEdit->text();

        if(QString::compare(str.simplified()," ",Qt::CaseInsensitive)!=0)
        {

            QListIterator<Node*> itrNodeList(_currentGraphView->getGraph()->getNodeList());
            while(itrNodeList.hasNext())
            {
                Node * pNode = itrNodeList.next();
                NtgEntity entity = pNode->getEntity();
                if((_asComboBox->currentText() == "All" || _asComboBox->model()->data(currentIndex.sibling(currentIndex.row(),1),Qt::DisplayRole).toString() == entity.type)
                    && entity.values.contains("value")
                    && entity.values.value("value").simplified().indexOf(str.simplified(),0,Qt::CaseInsensitive)!=-1
                    )
                {
                    pNode->getNtgItem()->setSelected(true);
                    mNodeList.append(pNode);
                }
                else
                    pNode->getNtgItem()->setSelected(false);
            }
        }

        if(mNodeList.size()==1)
            _currentGraphView->centerOn(mNodeList.first()->getNtgItem());

        if(mNodeList.size()==0)
            MainWindow::get()->addInfoLogMessage("Your search \""+ str + "\" ["+ _asComboBox->currentText() +"] did not match any entity value");
        else
        {
            MainWindow::get()->addInfoLogMessage("Your search \""+ str + "\" ["+ _asComboBox->currentText() +"] returns " + QString::number(mNodeList.size()) + " results");
            _currentGraphView->setFocus();
        }

    }
}

void MainWindow::openSelectedNodesUrl()
{
    if(_currentGraphView==NULL) return;

    QList<Node*> selectedNodes = _currentGraphView->getGraph()->getSelectedNodeList();
    foreach(Node * pNode,selectedNodes)
    {
        NtgEntity entity = pNode->getEntity();
        if((entity.type=="url"||entity.type=="website")&&entity.values.contains("value"))
        {
            QString addr = entity.values.value("value");
            if(addr.startsWith("www."))
                addr.prepend("http://");
            QDesktopServices::openUrl(QUrl(addr.simplified()));
        }
        else if(entity.type=="document" && entity.values.contains("url"))
        {
            QDesktopServices::openUrl(QUrl(entity.values.value("url").simplified()));
        }
        else if(entity.type=="email-address")
        {
            QDesktopServices::openUrl(QUrl("mailto:"+entity.values.value("value").simplified()));
        }
        else if(entity.type=="location" && entity.values.contains("latitude") && entity.values.contains("longitude"))
        {


            QString addr = "http://maps.google.fr/maps?q="+entity.values.value("latitude")+","+entity.values.value("longitude");
            QDesktopServices::openUrl(QUrl(addr.simplified()));
        }

    }
}

void MainWindow::uncheckDrawEdgeAct()
{
    _drawEdgeAct->setChecked(false);
}

void MainWindow::_openScriptSlot()
{
    _scriptWidget->openScriptSlot();
}

void MainWindow::_saveScriptSlot()
{
    _scriptWidget->saveScriptSlot();
}

void MainWindow::refreshLayoutActions()
{
    if(!_currentGraphView) return;
    QMapIterator<int, QPair<QAction *, QAction *> > i(_layoutActMap);
    while (i.hasNext())
    {
         i.next();
         i.value().first->setEnabled(true);
         i.value().second->setEnabled(true);
     }

    int layoutId = _currentGraphView->getLayoutId();
    QPair<QAction *, QAction *> pairAct = _layoutActMap.value(layoutId);
    pairAct.first->setChecked(true);
    pairAct.second->setChecked(true);
}


/*
 *      Scripts-only slots
 */

void MainWindow::selectGraph(Graph* graph)
{
    if(graph && graph->getView())
        _tabWidget->setCurrentWidget(graph->getView());
}

void MainWindow::closeGraph(Graph * graph)
{
    if(graph&& graph->getView()){
        delete graph->getView();
        delete graph;
        _currentGraphView = NULL;
        _graphSelectSlot(_tabWidget->currentIndex());
    }
}
