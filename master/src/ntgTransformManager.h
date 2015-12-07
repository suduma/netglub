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

#ifndef NTGTRANSFORMMANAGER_H
#define NTGTRANSFORMMANAGER_H

#include <QObject>
#include "ntgTypes.h"
#include "ntgImageHandler.h"
#include <QMutex>
#include <QPair>



class QSqlDatabase;


class NtgTransformManager : public QObject
{
  Q_OBJECT
  public:
    NtgTransformManager(QSqlDatabase * db, const QString & transformsPath, const QString & entitiesPath);
    virtual ~NtgTransformManager();
    NtgTransformModel * getTransformModel(const QString & name);
    QHash<QString,NtgTransformModel *> transformsModels();
    QHash<QString,NtgEntityModel *> entitiesModels();

  protected:
    QHash<QString,NtgTransformModel *>* _masterTransformModelHash;
    QHash<QString,NtgEntityModel *>* _masterEntityModelHash;
    QSqlDatabase * _db;
    void _createTransformsModels(QString path);
    void _createTransformsModelsFromXmls(QString path);
    void _createEntitiesModels(QString path);
    void _createEntitiesModelsFromXmls(QString path);

    QMap<QString, QPair<bool,QString> > _transformsInfoFromDB;
    QMap<QString, QPair<bool,QString> > _entitiesInfoFromDB;

    mutable QMutex _mutex;
};

#endif // NTGTRANSFORMMANAGER_H
