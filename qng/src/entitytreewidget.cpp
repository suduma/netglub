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

#include "entitytreewidget.h"
#include "entitytreewidgetitem.h"

#include <QApplication>
#include <QMouseEvent>
#include <QHeaderView>

EntityTreeWidget::EntityTreeWidget(QWidget * parent)
  : QTreeWidget(parent), _dragEntity(NULL)
{
    QStringList headerList;
    headerList << "name" << "type";
    setHeaderLabels(headerList);
    setHeaderHidden (true);
    hideColumn(1);
    header()->setResizeMode(0, QHeaderView::Stretch);
    setMouseTracking(true);
    setIconSize(QSize(24,24));
}


EntityTreeWidget::~EntityTreeWidget()
{
}


void EntityTreeWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    _startDragPos = event->pos();
    QTreeWidget::mousePressEvent(event);
}

void EntityTreeWidget::mouseMoveEvent(QMouseEvent *event)
{
  if (event->buttons() == Qt::LeftButton
      && (event->pos() - _startDragPos).manhattanLength() >= QApplication::startDragDistance())
  {
    EntityTreeWidgetItem * item = dynamic_cast<EntityTreeWidgetItem *>(currentItem());
    if (item&&!item->isDisabled())
    {

      _dragEntity = item;
      QByteArray itemData;
      QDataStream dataStream(&itemData, QIODevice::WriteOnly);
      NtgEntityModel entityModel = _entityModelHash.value(item->text(1));
      QPixmap pixmap = item->icon(0).pixmap(QSize(24,24));
      dataStream << entityModel;
      QMimeData * mimeData = new QMimeData;
      mimeData->setData("netglub/x-node-model", itemData);
      QDrag *drag = new QDrag(this);
      drag->setMimeData(mimeData);
      drag->setPixmap(pixmap);
      Qt::DropAction dropAction = drag->exec(Qt::CopyAction);
      Q_UNUSED(dropAction);
      _dragEntity = NULL;
    }
  }
}

void EntityTreeWidget::update()
{
    this->clear();
    QMultiMap<QString, QString> entityCatalogue;
    _entityModelHash = ModelsManager::getOrCreate()->getEntityModelHash();
    QStringList availableEntities = ModelsManager::getOrCreate()->getAvailableEntities().keys();

    foreach (NtgEntityModel entityModel, _entityModelHash)
    {

        if (entityModel.browsable && availableEntities.contains(entityModel.name))
        {
          entityCatalogue.insert(entityModel.category,entityModel.name);
        }
    }

    QTreeWidgetItem * categoryItem = NULL;
    EntityTreeWidgetItem * entityItem = NULL;
    QList<QTreeWidgetItem *> treeWidgetList;
    QMultiMap<QString, QString>::iterator i = entityCatalogue.begin();
    while (i != entityCatalogue.end())
    {
    QString categoryName = i.key();
    QString entityName = i.value();
    treeWidgetList = findItems(categoryName,Qt::MatchExactly,0);
    if (treeWidgetList.size() == 0)
    {
      categoryItem = new QTreeWidgetItem(this);
      categoryItem->setText(0,categoryName);
    }
    else
    {
      categoryItem = treeWidgetList.at(0);
    }
    entityItem = new EntityTreeWidgetItem(categoryItem);


    QString uuidName = entityName.toLower().simplified().replace(" ","");
    entityItem->setText(0, _entityModelHash.value(entityName).longName);
    entityItem->setText(1, entityName);
    QPixmap pixmap;
    pixmap.loadFromData(_entityModelHash.value(entityName).imagePNG,"PNG");
    entityItem->setIcon(0, QIcon(pixmap));

    ++i;
    }
    sortItems (0, Qt::AscendingOrder);
    expandAll();
}
