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

#include <QtGui>

#include "treemodel.h"

TreeModel::TreeModel(Node * node,
                     QObject *parent)
    : QAbstractItemModel(parent)
{
    _isAlone = true;
    _node = node;
    QVector<QVariant> rootData;
    rootData << "Name" << "value" << "Id";
    rootItem = new TreeItem(rootData);
    setupModelData(rootItem);
    _changed = true;

    connect(this,SIGNAL(dataChanged(QModelIndex,QModelIndex)),this,SLOT(changeEntity(QModelIndex,QModelIndex)));

}


TreeModel::TreeModel(QList<Node *> nodeList,
                     QObject *parent)
    : QAbstractItemModel(parent)
{
    _isAlone = false;
    _nodeList = nodeList;
    QVector<QVariant> rootData;
    rootData << "type" << "value" << "Id";
    rootItem = new TreeItem(rootData);
    setupModelDataFromNodes(rootItem);
    _changed = true;



}


TreeModel::~TreeModel()
{
    delete rootItem;
}

int TreeModel::columnCount(const QModelIndex & /* parent */) const
{
    return rootItem->columnCount();
}

QVariant TreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole && role != Qt::EditRole)
        return QVariant();

    TreeItem *item = getItem(index);

    return item->data(index.column());
}


Qt::ItemFlags TreeModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;


    if (getItem(index)->parent() == rootItem || parent(index).row()>0)
    {
        return  Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    }
    if (_isAlone)
    {
        switch (index.column())
        {
            case 0:
                return Qt::ItemIsSelectable | Qt::ItemIsEnabled; // non-editable column
            case 1:
                return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable; // editable column
            default:
                return Qt::ItemIsSelectable | Qt::ItemIsEnabled; // non-editable column

        }
    }
    else
    {
        return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    }
}



TreeItem *TreeModel::getItem(const QModelIndex &index) const
{
    if (index.isValid()) {
        TreeItem *item = static_cast<TreeItem*>(index.internalPointer());
        if (item) return item;
    }
    return rootItem;
}

QVariant TreeModel::headerData(int section, Qt::Orientation orientation,
                               int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return rootItem->data(section);

    return QVariant();
}


QModelIndex TreeModel::index(int row, int column, const QModelIndex &parent) const
{
    if (parent.isValid() && parent.column() != 0)
        return QModelIndex();



    TreeItem *parentItem = getItem(parent);

    TreeItem *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}


QModelIndex TreeModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    TreeItem *childItem = getItem(index);
    TreeItem *parentItem = childItem->parent();

    if (parentItem == rootItem)
        return QModelIndex();

    return createIndex(parentItem->childNumber(), 0, parentItem);
}



int TreeModel::rowCount(const QModelIndex &parent) const
{
    TreeItem *parentItem = getItem(parent);

    return parentItem->childCount();
}


bool TreeModel::setData(const QModelIndex &index, const QVariant &value,
                        int role)
{
    if (role != Qt::EditRole)
        return false;

    TreeItem *item = getItem(index);
    bool result = item->setData(index.column(), value);

    if (result)
        emit dataChanged(index, index);

    return result;
}

bool TreeModel::setHeaderData(int section, Qt::Orientation orientation,
                              const QVariant &value, int role)
{
    if (role != Qt::EditRole || orientation != Qt::Horizontal)
        return false;

    bool result = rootItem->setData(section, value);

    if (result)
        emit headerDataChanged(orientation, section, section);

    return result;
}

void TreeModel::setupModelData(TreeItem *parent)
{
//
//    parent->insertChildren()
    QVector<QVariant> categoryData;

    categoryData << "Entity Informations" << "";
    parent->insertChildren(parent->childCount(), 1, rootItem->columnCount());
    for (int column = 0; column < categoryData.size(); ++column)
        parent->child(parent->childCount() - 1)->setData(column, categoryData[column]);


    TreeItem * child = parent->child(0);



    foreach(NtgTransformParam param, _node->getEntityModel().params)
    {
        QVector<QVariant> valuesData;
        valuesData << param.longName;
        if (_node->getEntity().values.contains(param.name))
            valuesData << _node->getEntity().values.value(param.name);
        else
            valuesData << "";


        _delegateMap.insert(param.longName,new boxDelegate(param));
        child->insertChildren(child->childCount(), 1, rootItem->columnCount());
        for (int column = 0; column < valuesData.size(); ++column)
            child->child(child->childCount() - 1)->setData(column, valuesData[column]);   
    }

    QModelIndexList indexes = match(index(0,0),
                                           Qt::DisplayRole,
                                           "*",
                                           -1,
                                           Qt::MatchWildcard|Qt::MatchRecursive);


    QVector<QVariant> graphData;
    graphData << "Graph Informations" << "";
    parent->insertChildren(parent->childCount(), 1, rootItem->columnCount());
    for (int column = 0; column < graphData.size(); ++column)
        parent->child(parent->childCount() - 1)->setData(column, graphData[column]);


    TreeItem * childGraph = parent->child(1);

    QVector<QVariant> valuesData;
    valuesData << "Nb In Edges" << QString::number(_node->getInEdgeSet().count(),10);
    childGraph->insertChildren(childGraph->childCount(), 1, rootItem->columnCount());
    for (int column = 0; column < valuesData.size(); ++column)
        childGraph->child(childGraph->childCount() - 1)->setData(column, valuesData[column]);


    valuesData.clear();
    valuesData << "Nb Out Edges" << QString::number(_node->getOutEdgeSet().count(),10);
    childGraph->insertChildren(childGraph->childCount(), 1, rootItem->columnCount());
    for (int column = 0; column < valuesData.size(); ++column)
        childGraph->child(childGraph->childCount() - 1)->setData(column, valuesData[column]);


    if (_node->getFileMap().count() >0)
    {
        QVector<QVariant> fileData;
        fileData << "ressources" << "";
        parent->insertChildren(parent->childCount(), 1, rootItem->columnCount());
        for (int column = 0; column < fileData.size(); ++column)
            parent->child(parent->childCount() - 1)->setData(column, fileData[column]);

        TreeItem * childFile = parent->child(2);

        QVector<QVariant> fileValueData;

        foreach(int key, _node->getFileMap().keys())
        {
            fileValueData.clear();
            fileValueData  << _node->getFileMap().value(key).shortName << _node->getFileMap().value(key).size +" ko" << key;
            childFile->insertChildren(childFile->childCount(), 1, rootItem->columnCount());
            for (int column = 0; column < fileValueData.size(); ++column)
                childFile->child(childFile->childCount() - 1)->setData(column, fileValueData[column]);
        }
    }
}


void TreeModel::setupModelDataFromNodes(TreeItem *parent)
{
    if (_nodeList.count() > 1)
    {
        foreach(Node * node,_nodeList)
        {
            QVector<QVariant> nodeData;

            nodeData << node->getEntityModel().longName << node->getEntity().values.value("value") << node->getId();
            parent->insertChildren(parent->childCount(), 1, rootItem->columnCount());
            for (int column = 0; column < nodeData.size(); ++column)
                parent->child(parent->childCount() - 1)->setData(column, nodeData[column]);
        }
    }

}

void TreeModel::changed(bool hasChanged)
{
    _changed = hasChanged;
}

bool TreeModel::hasChanged()
{
    return _changed;
}

void TreeModel::changeEntity(QModelIndex topLeft, QModelIndex bottomRight)
{
    Q_UNUSED(bottomRight);
    QString valueToChange ="";
    foreach(NtgTransformParam param,_node->getEntityModel().params)
    {
        if (param.longName == getItem(topLeft)->data(0).toString())
        {
            valueToChange = param.name;
            break;
        }
    }

    if (!valueToChange.isEmpty())
        _node->addToEntity(valueToChange,getItem(topLeft)->data(1).toString());

//    qDebug()<<this->getItem(topLeft)->data(1).toString() << _node->getEntity();
}

QMap<QString, boxDelegate*> TreeModel::getDelegateMap()
{
    return _delegateMap;
}


/*-------------------------------------------------
  class TreeItem
-------------------------------------------------*/
TreeItem::TreeItem(const QVector<QVariant> &data, TreeItem *parent)
{
    parentItem = parent;
    itemData = data;
}

TreeItem::~TreeItem()
{

    qDeleteAll(childItems);
}



TreeItem *TreeItem::child(int number)
{
    return childItems.value(number);
}

int TreeItem::childCount() const
{
    return childItems.count();
}

int TreeItem::childNumber() const
{
    if (parentItem)
        return parentItem->childItems.indexOf(const_cast<TreeItem*>(this));

    return 0;
}

int TreeItem::columnCount() const
{
    return itemData.count();
}

QVariant TreeItem::data(int column) const
{
    return itemData.value(column);
}

bool TreeItem::insertChildren(int position, int count, int columns)
{
    if (position < 0 || position > childItems.size())
        return false;

    for (int row = 0; row < count; ++row) {
        QVector<QVariant> data(columns);
        TreeItem *item = new TreeItem(data, this);
        childItems.insert(position, item);
    }

    return true;
}



TreeItem *TreeItem::parent()
{
    return parentItem;
}


bool TreeItem::setData(int column, const QVariant &value)
{
    if (column < 0 || column >= itemData.size())
        return false;

    itemData[column] = value;
    return true;
}


bool TreeItem::removeChildren(int position, int count)
{
    if (position < 0 || position + count > childItems.size())
        return false;

    for (int row = 0; row < count; ++row)
        delete childItems.takeAt(position);

    return true;
}

/*-------------------------------------------------
  class boxDelegate
-------------------------------------------------*/

boxDelegate::boxDelegate(NtgTransformParam param, QObject *parent )
        : QItemDelegate(parent)
{
    _param = param;
    _type = param.format;


    if (param.format == "string")
        _data = QVariant(param.formatParam.first().value("regex"));
    else if (param.format == "enum" || param.format == "bool")
    {
        QStringList enumList;
        for(int i=0; i<param.formatParam.count();i++)
            enumList.append(param.formatParam.at(i).value("label"));
        _data = QVariant(enumList);
    }
    else if (param.format == "int")
    {
        QVariantMap minMax;
        if (param.formatParam.first().contains("min"))
            minMax.insert("min",QVariant(param.formatParam.first().value("min").toInt()));
        if (param.formatParam.first().contains("max"))
            minMax.insert("max",QVariant(param.formatParam.first().value("max").toInt()));
        _data = QVariant(minMax);
    }
}

QWidget * boxDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                      const QModelIndex &index) const
{
//    qDebug () << "boxDelegate::createEditor";
    if (_type == "string")
    {
        QLineEdit * editor = new QLineEdit(parent);
        editor->setValidator(new QRegExpValidator(QRegExp(_data.toString()),editor));
        return editor;
    }
    else if (_type == "enum" || _type == "bool")
    {
        QComboBox * editor = new QComboBox(parent);
        editor->addItems(_data.toStringList());
        return editor;
    }
    else if (_type == "int")
    {
        QSpinBox *editor = new QSpinBox(parent);
        if (_data.toMap().contains("min"))
            editor->setMinimum(_data.toMap().value("min").toInt());
        if (_data.toMap().contains("max"))
            editor->setMaximum(_data.toMap().value("max").toInt());

        return editor;
    }
    else
        return QItemDelegate::createEditor(parent,option,index);

}

void boxDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    if (_type == "string")
    {
        QString value = index.model()->data(index, Qt::EditRole).toString();

        QLineEdit *lineEdit = static_cast<QLineEdit*>(editor);
        lineEdit->setText(value);
    }
    else if (_type == "enum" || _type == "bool")
    {
        QString value = index.model()->data(index, Qt::EditRole).toString();

        QComboBox *comboBox = static_cast<QComboBox*>(editor);
        comboBox->setCurrentIndex(comboBox->findText(value));
    }
    else if (_type == "int")
    {
        int value = index.model()->data(index, Qt::EditRole).toInt();

        QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
        spinBox->setValue(value);
    }
    else
        QItemDelegate::setEditorData(editor,index);
}

void boxDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                  const QModelIndex &index) const
{
    if (_type == "string")
    {
        QLineEdit *lineEdit = static_cast<QLineEdit*>(editor);
        connect(lineEdit,SIGNAL(editingFinished()),this,SLOT(emitCommitData()));
        QString value = lineEdit->text();

        model->setData(index, value, Qt::EditRole);
    }
    else if (_type == "enum"  || _type == "bool")
    {
        QComboBox *comboBox = static_cast<QComboBox*>(editor);


        QString value = comboBox->currentText();

        model->setData(index, QVariant(value), Qt::EditRole);
    }
    else if (_type == "int")
    {
        QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
        spinBox->interpretText();
        int value = spinBox->value();

        model->setData(index, value, Qt::EditRole);
    }
    else
        QItemDelegate::setModelData(editor,model,index);
}

void boxDelegate::updateEditorGeometry(QWidget *editor,
    const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    editor->setGeometry(option.rect);
    Q_UNUSED(index);
}


void boxDelegate::emitCommitData()
{
    emit commitData(qobject_cast<QWidget *>(sender()));
}


