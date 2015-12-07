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

#ifndef TREEMODEL_H
#define TREEMODEL_H

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>
#include <QStringList>
#include <QStyledItemDelegate>
#include <QItemDelegate>


#include "graph.h"

class TreeModel;
class TransformTreeModel;

class TreeItem
{
public:
    TreeItem(const QVector<QVariant> &data, TreeItem *parent = 0);
    ~TreeItem();

    TreeItem *child(int number);
    int childCount() const;
    int columnCount() const;
    QVariant data(int column) const;
    bool insertChildren(int position, int count, int columns);
    TreeItem *parent();
    int childNumber() const;
    bool setData(int column, const QVariant &value);
    bool removeChildren(int position, int count);

private:
    QList<TreeItem*> childItems;
    QVector<QVariant> itemData;
    TreeItem *parentItem;
};

class boxDelegate : public QItemDelegate
{
    Q_OBJECT

public:
    boxDelegate(NtgTransformParam param, QObject *parent = 0);

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
};



class TreeModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    TreeModel(Node * node,
              QObject *parent = 0);
    TreeModel(QList<Node *> nodeList,
              QObject *parent = 0);
    ~TreeModel();


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


    void changed(bool hasChanged);
    bool hasChanged();

    QMap<QString, boxDelegate*> getDelegateMap();


public slots:
    void changeEntity(QModelIndex topLeft, QModelIndex bottomRight);

private:
    void setupModelData(TreeItem *parent);
    void setupModelDataFromNodes(TreeItem *parent);
    TreeItem *getItem(const QModelIndex &index) const;

    TreeItem *rootItem;
    Node * _node;
    QList<Node *> _nodeList;
    bool _changed;
    bool _isAlone;

    QMap<QString, boxDelegate*> _delegateMap;



};


#endif
