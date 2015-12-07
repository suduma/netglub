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

#ifndef MTGENTITY_H
#define MTGENTITY_H

#include <QString>
#include <QHash>

struct MtgProperty
{
    QString displayName;
    bool matchingRule;
    QString description;
    QString name;
    QString type;
    QString value;
};

struct MtgGenerator
{
    QString name;
    QString value;
};


struct MtgEntity
{
    QString _type;
    QString _value;
    QString _id;
    int _weight;
    double _posx;
    double _posy;
    QString _version;
    QString _additionalSearchTerm;

    QHash<QString,MtgProperty> _propertyHash;
    QList<MtgGenerator> _generatorList;
};

#endif // MTGENTITY_H
