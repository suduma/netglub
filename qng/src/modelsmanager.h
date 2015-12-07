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

#ifndef MODELSMANAGER_H
#define MODELSMANAGER_H

#include <QObject>
#include <QMultiMap>

#include "callermanager.h"
#include "ntgTypes.h"
#include "ntgImageHandler.h"

#include "mtgentity.h"

struct MtgEntityAssociator
{
    QString mtgType;
    QString ntgType;
    QHash<QString, QString> mtgToNtgParam;
    QHash<QString, QString> ntgToMtgParam;
};

class ModelsManager : public QObject
{
Q_OBJECT
public:
    static ModelsManager * getOrCreate();
    virtual ~ModelsManager();
    void updateModels();
    QHash<QString,NtgEntityModel> getEntityModelHash();
    QHash<QString,NtgEntityModel> getAvailableEntities();
    NtgEntityModel getEntityModel(QString nameId);
    QMultiMap<QString,NtgTransformModel> getAvailableTransformsbyCategories(QString input);
    QList<QString> getCategoryModel(QString category, QString input);
    QHash<QString,NtgTransformModel> getAvailableTransforms();

    QString getTransformLongName(QString transformId);
    NtgTransformModel getTransform(QString transformId);

    NtgEntity convertMtgEntityToNtgEntity(MtgEntity);
    MtgEntity convertNtgEntityToMtgEntity(NtgEntity);

protected:
    QHash<QString,NtgEntityModel> _entityModelHash;
    QStringList _availableEntities;

    QHash<QString,NtgTransformModel> _masterAvailableTransformsHash;
    QHash<QString,NtgTransformModel> _localAvailableTransformsHash;

    QHash<QString,MtgEntityAssociator> _mtgConverter;
    QHash<QString,MtgEntityAssociator> _ntgConverter;

private:
    explicit ModelsManager(QObject *parent = 0);


    static ModelsManager * _modelsManager;
    static bool isInit;


signals:

public slots:

};

#endif // MODELSMANAGER_H
