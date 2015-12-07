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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QHash>
#include <QSet>
#include <QTreeView>
#include <QComboBox>

#include "callermanager.h"
#include "modelsmanager.h"
#include "transformtimer.h"
#include "dockWidget/progresswidget.h"
#include "dockWidget/detailswidget.h"
#include "dockWidget/messageswidget.h"
#include "dockWidget/scriptwidget.h"
#include "settings/qngsettings.h"
#include "settings/managetransformdialog.h"

class QMenu;
class QToolbar;
class QAction;

class GraphTabWidget;
class Graph;
class GraphView;
class QTreeWidget;
class EntityTreeWidget;

/*
struct NtgEntityModelQNG
{
  QString name;
  QString type;
  QString iconFileName;
  QSet<QString> categorySet;
  QHash<QString, QString> parameterHash;
};
*/


/****************************************************************************
**  class MainWindow
****************************************************************************/

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    static MainWindow * create(QWidget * parent = 0);
    static MainWindow * get();


    bool isLayoutWeightedMod();
    bool isLayoutGroupChildMod();

    virtual TransformTimer * getTransformTimer();

    void setSelectable(bool bo);

    static void messageHandler(QtMsgType type, const char *msg);
    void addLogMessage(QtMsgType type, const QString & msg);
    void addInfoLogMessage(const QString & msg);

    void openSelectedNodesUrl();

    bool usingCache();

    void uncheckDrawEdgeAct();

    void refreshLayoutActions();

    void updateDetailsDock();

    void updateDocks();

public slots:
    QObject* newGraph();
    QObject* loadGraph(const QString & fileName);
    void saveGraph(Graph* graph,QString fileName);

    void useCache(bool useCache);
    void clearCache();

    /* Scripts-only slots */
    void selectGraph(Graph*);
    void closeGraph(Graph*);


protected slots:
    virtual void _selectionChangedSlot();

    void _openGraphSlot();
    void _saveGraphSlot();
    void _saveAsGraphSlot();
    void _searchSlot();
    void _goToSearchSlot();
    void _exportToCsv();
    void _exportScreenShotSlot();
    void _openRecentFileSlot();
    void _printSlot();
    void _openScriptSlot();
    void _saveScriptSlot();
    void _aboutSlot();
    void _settingsSlot();
    virtual void _graphSelectSlot(int index);
    virtual void _graphCloseSlot(int index);
    void _windowsMenuSlot();
    void _manageTransformSlot();

    virtual void _layoutActSlot(QAction * action);
    virtual void _selectActSlot(QAction * action);
    virtual void _zoomActSlot(QAction * action);
    virtual void _miscActLayoutSlot(QAction * action);
    virtual void _freezedActSlot(bool);
    virtual void _drawModeSlot(bool);



    virtual void _copySlot();
    virtual void _pasteSlot();
    virtual void _cutSlot();
    virtual void _deleteSlot();
    void _copy();


    void _reloadSlot();

    void selectNode(QModelIndex index);

    void closeEvent(QCloseEvent *event);


    virtual void keyPressEvent(QKeyEvent *event);
protected:
    virtual void _createActions();
    virtual void _createMenus();
    virtual void _createToolBars();
    virtual void _createStatusBar();
    virtual void _createDockWindows();


    virtual void _updateRecentFile(const QString &fileName);
    virtual void _updateRecentFileActions();
    virtual QString _strippedName(const QString &fullFileName);
    void setViewModel(QList<Node*>& selectedNodes);
    void setViewModel(QSet<Edge*> selectedEdges);

    int _dockNumber;
    QAction * _windowSeparatorAct;

//    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);

    static QtMsgHandler _oldMsgHandler;

    GraphView * _currentGraphView;
    GraphTabWidget * _tabWidget;
    EntityTreeWidget * _entityTreeWidget;

    QMap<int, QPair<QAction *, QAction *> > _layoutActMap;
    QHash<QString, NtgEntityModel> _entityModelHash;

    QList<QPair<QPointF, NtgEntity> > _copyEntityList;
    QList<QPair<int,int> > _copyEdgeList;


//    EntityPropertiesWidget * _entityPropertiesWidget;

    QTreeView * _view;
    TransformTimer * _transformTimer;


    DetailsWidget * _entityDetailsWidget;
    MessagesWidget * _messagesWidget;
    ProgressWidget * _progressWidget;
    ScriptWidget * _scriptWidget;

    QMenu * _fileMenu;
    QMenu * _editMenu;
    QMenu * _toolsMenu;
    QMenu * _layoutsMenu;
    //QMenu * _layoutWeightedMenu;
    QMenu * _viewMenu; //TODO ?
    QMenu * _windowsMenu;
    QMenu * _helpMenu;



    QToolBar * _fileToolBar;
    QToolBar * _editToolBar;
    QToolBar * _selectToolBar;
    QToolBar * _layoutsToolBar;
    QToolBar * _zoomsToolBar;
    QToolBar * _searchToolBar;
    //TODO findToolBar;

    //FILE ACTIONS
    QAction * _newAct;
    QAction * _openAct;
    QAction * _saveAct;
    QAction * _saveAsAct;
    QAction * _exportToCsvAct;
    QAction * _exportScreenShotAct;
    QAction * _printAct;
    QAction * _quitAct;
    QAction * _separatorAct;
    QAction *_openScriptAct;
    QAction * _saveScriptAct;

    //HELP ACTIONS
    QAction * _aboutAct;
    QAction * _aboutQtAct;


    //EDIT ACTIONS
    QAction * _cutAct;
    QAction * _copyAct;
    QAction * _pasteAct;
    QAction * _deleteAct;
    QAction * _selectAllAct;
    QAction * _inverseSelectionAct;

    QAction * _selectParentsAct;
    QAction * _addParentsAct;
    QAction * _selectChildrenAct;
    QAction * _addChildrenAct;
    QAction * _selectNeighborsAct;
    QAction * _addNeighborsAct;
    QActionGroup * _selectActGroup;

    QAction * _settingsAct;

    //LAYOUTS ACTIONS

    QAction * _miningViewLayoutAct;
    QAction * _centralityViewLayoutAct;
    QAction * _edgeWeightedViewLayoutAct;
    QActionGroup * _viewLayoutActGroup;

    QAction * _blockLayoutAct;
    QAction * _hierarchyLayoutAct;
    QAction * _circularLayoutAct;
    QAction * _molecularLayoutAct;
    QActionGroup * _layoutActGroup;


    QAction * _layoutFreezeAct;
    QAction * _layoutRedrawAct;
    QAction * _layoutClearAllAct;
    QActionGroup * _miscLayoutActGroup;

    QAction * _aLayoutWeightedMod;
    QAction * _aLayoutGroupChildMod;

    QActionGroup * _agLayoutTypeGraphOption;
    QAction * _aLayoutDirectionOption;


    //TOOLS ACTIONS

    QAction * _manageTransformAct;
    QAction * _reloadAct;
    QAction * _drawEdgeAct;

    enum { MaxRecentFiles = 5 };
    QAction * _recentFileActs[MaxRecentFiles];

    QAction * _useCacheAct;
    QAction * _clearCache;

    //ZOOMS ACTIONS
    QAction * _zoomInAct;
    QAction * _zoomOutAct;
    QAction * _zoomFitBestAct;
    QActionGroup * _zoomActGroup;



    //FIND ACTIONS
    QAction * _searchAction;
    QAction * _goToSearchSlotAct;

    //FIND WIDGET
    QLineEdit * _searchLineEdit;
    QComboBox * _asComboBox;



private:
    MainWindow(QWidget *parent = 0);
    virtual ~MainWindow();

    //CallerManager * _caller;

    static MainWindow * _mainWindow;

    bool _changingSelection;
    
    bool _usingCache;


};

#endif // MAINWINDOW_H
