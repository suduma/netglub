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

#include "managetransformdialog.h"
#include <QComboBox>
#include <QSpinBox>



ManageXmlFile::ManageXmlFile()
{
    QSettings settings;
    QFileInfo settingsPath = QFileInfo(settings.fileName());
    QDir settingsDir = settingsPath.absoluteDir();
    QString path = settingsDir.absolutePath()+"/data/transform.xml";

    file = new QFile(path);

    if (!QFile::exists(path))
    {
        if (!file->open(QIODevice::WriteOnly))
            return;
        file->close();
    }
}



bool ManageXmlFile::saveTransformToXmlFile(QString transformId, QMap<QString,QString> params)
{
    file->open(QIODevice::ReadOnly);
    QDomDocument doc;
    QDomElement root;

    bool noError = true;
    QFile xmlConf(file->fileName());
    if( ! xmlConf.open(QIODevice::ReadOnly))
      return false;
    doc.setContent(&xmlConf);
    if( doc.doctype().name() != "TransformParameters")
        noError = false;

    root =  doc.documentElement();
    if (root.tagName() != "transforms")
        noError = false;

    if (!noError)
    {
        file->close();

        file->open(QIODevice::WriteOnly);
        file->write("");
        file->close();

        file->open(QIODevice::ReadOnly);
        doc = QDomDocument("TransformParameters");
        root = doc.createElement("transforms");
        doc.appendChild(root);
    }
    else
    {
        QDomNode toRemoveNode;
        for(QDomElement paramNode = root.firstChildElement("transform");
            ! paramNode.isNull();paramNode = paramNode.nextSiblingElement("transform"))
        {
            if (paramNode.attribute("name") == transformId)
            {
                toRemoveNode = paramNode;
                break;
            }
        }
        root.removeChild(toRemoveNode);
    }

    QDomElement tag = doc.createElement("transform");
    tag.setAttribute("name",transformId);
    root.appendChild(tag);

    foreach(QString key , params.keys())
    {
        QDomElement param = doc.createElement("param");
        param.setAttribute("name",key);
        param.setAttribute("value",params.value(key));
        tag.appendChild(param);
    }



    xmlConf.close();
    file->close();

    file->open(QIODevice::WriteOnly);
    QString xml = doc.toString();

    file->write(xml.toUtf8()) ;
    file->close();

    return true;

}

QMap<QString,QString> ManageXmlFile::removeTransformFromXmlFile(QString transformId)
{
    file->open(QIODevice::ReadOnly);
    QMap<QString,QString> paramMap;
    QDomDocument doc;
    QDomElement root;

    bool noError = true;
    QFile xmlConf(file->fileName());
    if( ! xmlConf.open(QIODevice::ReadOnly))
      return paramMap;
    doc.setContent(&xmlConf);
    if( doc.doctype().name() != "TransformParameters")
        noError = false;

    root =  doc.documentElement();
    if (root.tagName() != "transforms")
        noError = false;

    if (noError)
    {
        QDomNode toRemoveNode;
        for(QDomElement transformNode = root.firstChildElement("transform");
            ! transformNode.isNull();transformNode = transformNode.nextSiblingElement("transform"))
        {
            if (transformNode.attribute("name") == transformId)
            {
                toRemoveNode = transformNode;
                for(QDomElement paramNode = transformNode.firstChildElement("param");
                  ! paramNode.isNull();paramNode = paramNode.nextSiblingElement("param"))
                {
                    paramMap.insert(paramNode.attribute("name"),paramNode.attribute("value"));
                }
                break;
            }
        }
        root.removeChild(toRemoveNode);
    }




    xmlConf.close();
    file->close();

    file->open(QIODevice::WriteOnly);
    QString xml = doc.toString();

    file->write(xml.toUtf8()) ;
    file->close();
    return paramMap;

}

QMap<QString,QMap<QString,QString> > ManageXmlFile::loadTransforms()
{
    file->open(QIODevice::ReadOnly);
    QMap<QString,QMap<QString,QString> > transformMap;
    QDomDocument doc;
    QDomElement root;


    bool noError = true;
    QFile xmlConf(file->fileName());
    if( ! xmlConf.open(QIODevice::ReadOnly))
      return transformMap;
    doc.setContent(&xmlConf);
    if( doc.doctype().name() != "TransformParameters")
        noError = false;

    root =  doc.documentElement();
    if (root.tagName() != "transforms")
        noError = false;

    if (noError)
    {
        for(QDomElement transformNode = root.firstChildElement("transform");
            ! transformNode.isNull();transformNode = transformNode.nextSiblingElement("transform"))
        {
            QString transformId = transformNode.attribute("name");
            QMap<QString,QString> paramMap;
            for(QDomElement paramNode = transformNode.firstChildElement("param");
              ! paramNode.isNull();paramNode = paramNode.nextSiblingElement("param"))
            {
                paramMap.insert(paramNode.attribute("name"),paramNode.attribute("value"));
            }
            transformMap.insert(transformId,paramMap);

        }
    }



    xmlConf.close();
    file->close();
    qDebug() << transformMap;
    return transformMap;
}

bool ManageXmlFile::contains(QString transformId)
{
    file->open(QIODevice::ReadOnly);
    QDomDocument doc;
    QDomElement root;


    QFile xmlConf(file->fileName());
    if( ! xmlConf.open(QIODevice::ReadOnly))
      return false;
    doc.setContent(&xmlConf);
    if( doc.doctype().name() != "TransformParameters")
        return false;

    root =  doc.documentElement();
    if (root.tagName() != "transforms")
        return false;

    for(QDomElement paramNode = root.firstChildElement("transform");
        ! paramNode.isNull();paramNode = paramNode.nextSiblingElement("transform"))
    {
        if (paramNode.attribute("name") == transformId)
        {
            xmlConf.close();
            file->close();
            return true;
        }
    }

    xmlConf.close();
    file->close();

    return false;
}


QMap<QString,QString> ManageXmlFile::getTransformFromXmlFile(QString transformId)
{
    file->open(QIODevice::ReadOnly);
    QMap<QString,QString> paramMap;
    QDomDocument doc;
    QDomElement root;

    bool noError = true;
    QFile xmlConf(file->fileName());
    if( ! xmlConf.open(QIODevice::ReadOnly))
      return paramMap;
    doc.setContent(&xmlConf);
    if( doc.doctype().name() != "TransformParameters")
        noError = false;

    root =  doc.documentElement();
    if (root.tagName() != "transforms")
        noError = false;

    if (noError)
    {
        for(QDomElement transformNode = root.firstChildElement("transform");
            ! transformNode.isNull();transformNode = transformNode.nextSiblingElement("transform"))
        {
            if (transformNode.attribute("name") == transformId)
            {
                for(QDomElement paramNode = transformNode.firstChildElement("param");
                  ! paramNode.isNull();paramNode = paramNode.nextSiblingElement("param"))
                {
                    paramMap.insert(paramNode.attribute("name"),paramNode.attribute("value"));
                }
                break;
            }
        }

    }

    xmlConf.close();
    file->close();
    return paramMap;

}


ManageTransformDialog::ManageTransformDialog(QWidget *parent) :
    QDialog(parent)
{
    setWindowTitle(tr("Manage Transforms"));

    _tabWidget = new QTabWidget(this);
    QWidget * masterTransformWidget =new QWidget(this);

    _treeView = new QTreeView(this);
    TransformTreeModel * model = new TransformTreeModel(this);
    _treeView->setModel(model);
    _treeView->setAlternatingRowColors(true);

    for (int column = 0; column < model->columnCount(); ++column)
        _treeView->resizeColumnToContents(column);

    _treeView->setEditTriggers(QAbstractItemView::AnyKeyPressed|
                               QAbstractItemView::DoubleClicked|
                               QAbstractItemView::CurrentChanged);



    _treeView->setItemDelegate(new TransformBoxDelegate(model));

    _treeView->hideColumn(4);




    QVBoxLayout * treeLayout = new QVBoxLayout(this);
    treeLayout->addWidget(_treeView);
    masterTransformWidget->setLayout(treeLayout);


    _tabWidget->addTab(masterTransformWidget, "Master Transform Manager");

    QWidget * localTransformWidget =new QWidget(this);

    QVBoxLayout * localLayout = new QVBoxLayout(this);
    localLayout->addWidget(new QLabel("Under Construction",localTransformWidget),0,Qt::AlignCenter);
    localTransformWidget->setLayout(localLayout);

    _tabWidget->addTab(localTransformWidget, "Local Transform Manager");

    _closeButton = new QPushButton("&Close", this);
    connect(_closeButton,SIGNAL(clicked()),this,SLOT(close()));

    QVBoxLayout * layout = new QVBoxLayout(this);
    layout->addWidget(_tabWidget,0);
    layout->addWidget(_closeButton,1,Qt::AlignRight);
    _treeView->setSortingEnabled(true);
    _treeView->sortByColumn(0,Qt::AscendingOrder);

    setMinimumWidth(900);
    setLayout(layout);

}


/*-------------------------------------------------
  class TransformTreeModel
-------------------------------------------------*/

TransformTreeModel::TransformTreeModel(QObject *parent)
    : QAbstractItemModel(parent)
{

    QVector<QVariant> rootData;
    rootData << "Transform Name" << "Input Entity" << "Output Entity" << "Remember Settings" << "Ids";
    rootItem = new TreeItem(rootData);
    setupModelData(rootItem);

    connect(this,SIGNAL(dataChanged(QModelIndex,QModelIndex)),this,SLOT(changeSettings(QModelIndex,QModelIndex)));

//    xmlFile = MainWindow::get()->getManageXmlFile();
}

TransformTreeModel::~TransformTreeModel()
{
    delete rootItem;
}

int TransformTreeModel::columnCount(const QModelIndex & /* parent */) const
{
    return rootItem->columnCount();
}

QVariant TransformTreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.column() == 3 && role == Qt::CheckStateRole && parent(index).row() == -1 && getItem(index)->childCount()>0)
    {

        TreeItem *item = getItem(index);
        if (item->data(index.column()).toBool())
            return Qt::Checked;
        else
            return Qt::Unchecked;



    }

    if (role != Qt::DisplayRole && role != Qt::EditRole )
        return QVariant();

    TreeItem *item = getItem(index);


    return item->data(index.column());
}


Qt::ItemFlags TransformTreeModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;


    if (getItem(index)->parent() == rootItem && index.column() !=4)
    {
        switch (index.column())
        {
            case 0:
            case 1:
            case 2:
                return Qt::ItemIsSelectable | Qt::ItemIsEnabled; // non-editable column
            case 3:
                if (getItem(index)->childCount()>0)
                    return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsUserCheckable; // editable column
                else
                    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
            default:
                return Qt::ItemIsSelectable | Qt::ItemIsEnabled; // non-editable column

        }
    }

    switch (index.column())
    {
        case 0:
            return Qt::ItemIsSelectable | Qt::ItemIsEnabled; // non-editable column
        case 1:
            return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable; // editable column
        default:
            return  Qt::ItemIsSelectable; // non-editable column

    }

}



TreeItem *TransformTreeModel::getItem(const QModelIndex &index) const
{
    if (index.isValid()) {
        TreeItem *item = static_cast<TreeItem*>(index.internalPointer());
        if (item) return item;
    }
    return rootItem;
}

QVariant TransformTreeModel::headerData(int section, Qt::Orientation orientation,
                               int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return rootItem->data(section);

    return QVariant();
}


QModelIndex TransformTreeModel::index(int row, int column, const QModelIndex &parent) const
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


QModelIndex TransformTreeModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    TreeItem *childItem = getItem(index);
    TreeItem *parentItem = childItem->parent();

    if (parentItem == rootItem)
        return QModelIndex();

    return createIndex(parentItem->childNumber(), 0, parentItem);
}



int TransformTreeModel::rowCount(const QModelIndex &parent) const
{
    TreeItem *parentItem = getItem(parent);

    return parentItem->childCount();
}


bool TransformTreeModel::setData(const QModelIndex &index, const QVariant &value,
                        int role)
{
    if (index.column() == 3 && role == Qt::CheckStateRole)
    {
        TreeItem *item = getItem(index);
        bool send =false;
        if (value.toInt() == Qt::Checked)
        {
            send =true;
        }

        bool result = item->setData(index.column(),QVariant(send));
        if (result)
            emit dataChanged(index, index);

        return result;
    }

    if (role != Qt::EditRole )
        return false;

    TreeItem *item = getItem(index);
    bool result = item->setData(index.column(), value);
    if (result)
        emit dataChanged(index, index);

    return result;
}

bool TransformTreeModel::setHeaderData(int section, Qt::Orientation orientation,
                              const QVariant &value, int role)
{
    if (role != Qt::EditRole || orientation != Qt::Horizontal)
        return false;

    bool result = rootItem->setData(section, value);

    if (result)
        emit headerDataChanged(orientation, section, section);

    return result;
}

void TransformTreeModel::setupModelData(TreeItem *parent)
{
//
//    parent->insertChildren()

    ManageXmlFile * xmlFile = new ManageXmlFile();
    QVector<QVariant> transformData;

    QHash<QString,NtgTransformModel> availableTransforms = ModelsManager::getOrCreate()->getAvailableTransforms();
//    qDebug() << availableTransforms;
    int lIndex = 0;
    bool inSettings = false;
    foreach (NtgTransformModel transform, availableTransforms)
    {
//        qDebug() << transform.name << transform.longName;
        transformData.clear();
        QStringList list;
        foreach (QString input ,transform.inputTypes)
        {
            list.append(ModelsManager::getOrCreate()->getEntityModel(input).longName);
        }
        QString inputs = list.join(", ");
        list.clear();
        foreach (QString output ,transform.outputTypes)
        {
            list.append(ModelsManager::getOrCreate()->getEntityModel(output).longName);
        }
        QString outputs = list.join(", ");
        transformData << transform.longName << inputs << outputs;
        if (transform.params.count()==0)
        {
            transformData << "";
            inSettings = false;
        }
        else if (xmlFile->contains(transform.name))
        {
            transformData << "true";
            inSettings = true;
        }
        else
        {
            transformData << "false";
            inSettings = false;
        }
        transformData << transform.name;
        parent->insertChildren(parent->childCount(), 1, rootItem->columnCount());
        for (int column = 0; column < transformData.size(); ++column)
            parent->child(parent->childCount() - 1)->setData(column, transformData[column]);

        TreeItem * child = parent->child(lIndex++);

        foreach(NtgTransformParam param, transform.params)
        {
            QVector<QVariant> valuesData;
            valuesData << param.longName;
            if (inSettings)
                valuesData << xmlFile->getTransformFromXmlFile(transform.name).value(param.name);
            else
                valuesData << param.defaultValue;
            valuesData << "" << "" << param.name;
//            qDebug() << param.format << "-----------------------------";
            _delegateMap.insert(param.longName,new boxDelegate(param));
            child->insertChildren(child->childCount(), 1, rootItem->columnCount());
            for (int column = 0; column < valuesData.size(); ++column)
                child->child(child->childCount() - 1)->setData(column, valuesData[column]);
        }
    }

    delete xmlFile;
}



void TransformTreeModel::changeSettings(QModelIndex topLeft, QModelIndex bottomRight)
{
    Q_UNUSED(bottomRight);
    ManageXmlFile *xmlFile = new ManageXmlFile();
    if (topLeft.column() == 3)
    {
//        qDebug() << "TransformTreeModel::changeSettings";
        NtgTransformModel actualTransform = ModelsManager::getOrCreate()->getTransform(data(topLeft.sibling(topLeft.row(),4),Qt::DisplayRole).toString());

        if (actualTransform.name.isEmpty())
            return;



        if (getItem(topLeft)->data(3).toBool())
        {
            QVariantMap map;
            QMap<QString,QString> paramMap;
            for (int i =0 ; i< getItem(topLeft)->childCount();i++)
            {

                NtgTransformParam actualParam = actualTransform.params.value(getItem(topLeft)->child(i)->data(4).toString());
                QString key = actualParam.name;
                if (key.isEmpty())
                    return;
                paramMap.insert(key, getItem(topLeft)->child(i)->data(1).toString());

            }
            xmlFile->saveTransformToXmlFile(actualTransform.name,paramMap);
        }
        else
        {
            xmlFile->removeTransformFromXmlFile(actualTransform.name);
        }
        return;
    }

    if (topLeft.parent().row() != -1)
        setData(createIndex(topLeft.parent().row(),3,getItem(topLeft)->parent()),QVariant(true),Qt::EditRole);

    delete xmlFile;
}


QMap<QString, boxDelegate*> TransformTreeModel::getDelegateMap()
{
    return _delegateMap;
}


bool TransformTreeModel::insertRows(int position, int rows, const QModelIndex &parent)
{
    TreeItem *parentItem = getItem(parent);
    bool success;

    beginInsertRows(parent, position, position + rows - 1);
    success = parentItem->insertChildren(position, rows, rootItem->columnCount());
    endInsertRows();

    return success;
}

bool TransformTreeModel::removeRows(int position, int rows, const QModelIndex &parent)
{
    TreeItem *parentItem = getItem(parent);
    bool success = true;

    beginRemoveRows(parent, position, position + rows - 1);
    success = parentItem->removeChildren(position, rows);
    endRemoveRows();

    return success;
}

void TransformTreeModel::sort(int column, Qt::SortOrder order)
{
//    QModelIndexList indexes = match(index(0,0),
//                                    Qt::DisplayRole,
//                                    "*",
//                                    -1,
//                                    Qt::MatchWildcard|Qt::MatchRecursive);
//
//    QModelIndex indexa;
//    int i=0;
//    foreach (indexa, indexes)
//    {
//        i++;
//        qDebug() << data(indexa.sibling(indexa.row(),column),Qt::DisplayRole);
//    }
//    QVector<QVariant> transformData;
//
//    while (i>=0)
//    {
//        this->removeRow(0,index(-1,0));
//        i--;
//    }
//
//    qDebug() << "column :" << column;
    Q_UNUSED(column);
    Q_UNUSED(order);
}


/*-------------------------------------------------
  class TransformBoxDelegate
-------------------------------------------------*/

TransformBoxDelegate::TransformBoxDelegate(TransformTreeModel * model, QObject *parent )
        : QItemDelegate(parent)
{
    _model = model;
}

QWidget * TransformBoxDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                      const QModelIndex &index) const
{
//    qDebug () << "TransformBoxDelegate::createEditor";

    NtgTransformModel actualTransform = ModelsManager::getOrCreate()->getTransform(_model->data(index.parent().sibling(index.parent().row(),4),Qt::DisplayRole).toString());
    if (actualTransform.name.isEmpty())
        return QItemDelegate::createEditor(parent,option,index);
    NtgTransformParam  actualParam = actualTransform.params.value(_model->data(index.sibling(index.row(),4),Qt::DisplayRole).toString());
    if (actualParam.name.isEmpty())
        return QItemDelegate::createEditor(parent,option,index);


//        qDebug() << "format ----------------->" <<actualParam.format;
    if (actualParam.format == "string")
    {

        QLineEdit * editor = new QLineEdit(parent);
        editor->setValidator(new QRegExpValidator(QRegExp(actualParam.formatParam.first().value("regex")),editor));
        return editor;
    }
    else if (actualParam.format == "enum" || actualParam.format == "bool")
    {
        QComboBox * editor = new QComboBox(parent);
        QStringList enumList;
        for(int i=0; i<actualParam.formatParam.count();i++)
            enumList.append(actualParam.formatParam.at(i).value("label"));
        editor->addItems(enumList);
        return editor;
    }
    else if (actualParam.format == "int")
    {
        QSpinBox *editor = new QSpinBox(parent);
        if (actualParam.formatParam.first().contains("min"))
            editor->setMinimum(actualParam.formatParam.first().value("min").toInt());
        if (actualParam.formatParam.first().contains("max"))
            editor->setMaximum(actualParam.formatParam.first().value("max").toInt());

        return editor;
    }
    else
    {
        return QItemDelegate::createEditor(parent,option,index);
    }


}

void TransformBoxDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{

    NtgTransformModel actualTransform = ModelsManager::getOrCreate()->getTransform(_model->data(index.parent().sibling(index.parent().row(),4),Qt::DisplayRole).toString());
    if (actualTransform.name.isEmpty())
        QItemDelegate::setEditorData(editor,index);
    NtgTransformParam  actualParam = actualTransform.params.value(_model->data(index.sibling(index.row(),4),Qt::DisplayRole).toString());
    if (actualParam.name.isEmpty())
        QItemDelegate::setEditorData(editor,index);


    if (actualParam.format == "string")
    {
        QString value = index.model()->data(index, Qt::EditRole).toString();

        QLineEdit *lineEdit = static_cast<QLineEdit*>(editor);
        lineEdit->setText(value);
    }
    else if (actualParam.format == "enum" || actualParam.format == "bool")
    {
        QString value = index.model()->data(index, Qt::EditRole).toString();

        QComboBox *comboBox = static_cast<QComboBox*>(editor);
        comboBox->setCurrentIndex(comboBox->findText(value));
    }
    else if (actualParam.format == "int")
    {
        int value = index.model()->data(index, Qt::EditRole).toInt();

        QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
        spinBox->setValue(value);
    }
    else
        QItemDelegate::setEditorData(editor,index);

}

void TransformBoxDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                  const QModelIndex &index) const
{
    Q_UNUSED(index);
    NtgTransformModel actualTransform = ModelsManager::getOrCreate()->getTransform(_model->data(index.parent().sibling(index.parent().row(),4),Qt::DisplayRole).toString());
    if (actualTransform.name.isEmpty())
        QItemDelegate::setModelData(editor,model,index);
    NtgTransformParam  actualParam = actualTransform.params.value(_model->data(index.sibling(index.row(),4),Qt::DisplayRole).toString());
    if (actualParam.name.isEmpty())
        QItemDelegate::setModelData(editor,model,index);

    if (actualParam.format == "string")
    {
        QLineEdit *lineEdit = static_cast<QLineEdit*>(editor);
        connect(lineEdit,SIGNAL(editingFinished()),this,SLOT(emitCommitData()));
        QString value = lineEdit->text();

        model->setData(index, value, Qt::EditRole);
    }
    else if (actualParam.format == "enum" || actualParam.format == "bool")
    {
        QComboBox *comboBox = static_cast<QComboBox*>(editor);
//                connect(comboBox->lineEdit(),SIGNAL(editingFinished()),this,SLOT(emitCommitData()));
        QString value = comboBox->currentText();
//                qDebug() << "test -------------------->" << comboBox->currentIndex();
        model->setData(index, value, Qt::EditRole);
    }
    else if (actualParam.format == "int")
    {
        QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
        spinBox->interpretText();
        int value = spinBox->value();

        model->setData(index, value, Qt::EditRole);
    }
    else
        QItemDelegate::setModelData(editor,model,index);


}


void TransformBoxDelegate::updateEditorGeometry(QWidget *editor,
    const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    editor->setGeometry(option.rect);
    Q_UNUSED(index);
}


void TransformBoxDelegate::emitCommitData()
{
    emit commitData(qobject_cast<QWidget *>(sender()));
}


