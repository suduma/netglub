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

#ifndef MANAGETRANSFORMDIALOG_H
#define MANAGETRANSFORMDIALOG_H

#include <QWidget>
#include <QDialog>
#include <QTabWidget>
#include <QTreeView>
#include <QPushButton>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QSortFilterProxyModel>
//#include "mainwindow.h"
#include "dockWidget/treemodel.h"

class ManageXmlFile : public QObject
{
    Q_OBJECT
    public:
        explicit ManageXmlFile();

        bool saveTransformToXmlFile(QString transformId, QMap<QString,QString> params);
        QMap<QString,QString> removeTransformFromXmlFile(QString transformId);
        QMap<QString,QMap<QString,QString> > loadTransforms();
        bool contains(QString transformId);
        QMap<QString,QString> getTransformFromXmlFile(QString transformId);
    protected:
        QFile * file;
};



class TransformBoxDelegate : public QItemDelegate
{
    Q_OBJECT

public:
    TransformBoxDelegate(TransformTreeModel * model, QObject *parent = 0);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const;

    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const;

    void updateEditorGeometry(QWidget *editor,
        const QStyleOptionViewItem &option, const QModelIndex &index) const;

public slots:
    void emitCommitData();

protected:
    QVariant _data;
    QString _type;
    NtgTransformParam _param;
    TransformTreeModel * _model;
};



class TransformTreeModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    TransformTreeModel(QObject *parent = 0);
    ~TransformTreeModel();


    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const;

    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &index) const;

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;

    Qt::ItemFlags flags(const QModelIndex &index) const;
    bool setData(const QModelIndex &index, const QVariant &value,
                 int role = Qt::EditRole);
    bool setHeaderData(int section, Qt::Orientation orientation,
                       const QVariant &value, int role = Qt::EditRole);

    bool insertRows(int position, int rows,
                    const QModelIndex &parent = QModelIndex());
    bool removeRows(int position, int rows,
                    const QModelIndex &parent = QModelIndex());

    void sort(int column, Qt::SortOrder order);

//    void changed(bool hasChanged);
//    bool hasChanged();

    QMap<QString, boxDelegate*> getDelegateMap();

public slots:
    void changeSettings(QModelIndex topLeft, QModelIndex bottomRight);

private:
    void setupModelData(TreeItem *parent);
    TreeItem *getItem(const QModelIndex &index) const;

    TreeItem *rootItem;
//    bool _changed;
//    bool _isAlone;

    QMap<QString, boxDelegate*> _delegateMap;

};


class ManageTransformDialog : public QDialog
{
Q_OBJECT
public:
    explicit ManageTransformDialog(QWidget *parent = 0);

signals:

public slots:

protected:
    QTabWidget * _tabWidget;
    QTreeView * _treeView;
    QPushButton * _closeButton;
    ManageXmlFile * _xmlFile;

};



#endif // MANAGETRANSFORMDIALOG_H
