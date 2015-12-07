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

#ifndef ENTITYTREEWIDGET_H
#define ENTITYTREEWIDGET_H

#include <QTreeWidget>
#include "ntgTypes.h"
#include "modelsmanager.h"

class QMouseEvent;
class EntityTreeWidgetItem;

class EntityTreeWidget : public QTreeWidget
{
  public:
    EntityTreeWidget ( QWidget * parent = 0 );
    virtual ~EntityTreeWidget();
    virtual EntityTreeWidgetItem * dragEntity() const { return _dragEntity; };
    virtual void update();

  protected:
    virtual void mousePressEvent ( QMouseEvent * event );
    virtual void mouseMoveEvent ( QMouseEvent * event );

    QHash<QString,NtgEntityModel> _entityModelHash;
    EntityTreeWidgetItem * _dragEntity;
    QPoint _startDragPos;

};

#endif // ENTITYTREEWIDGET_H
