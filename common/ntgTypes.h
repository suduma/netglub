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

#ifndef TRANSFORM_TYPES_H
#define TRANSFORM_TYPES_H

#include <QString>
#include <QStringList>
#include <QHash>
#include <QMetaType>
#include <QDebug>
#include <QVariant>
#include <QMap>


namespace Ntg
{
  void initMetatypes();

  enum TransformStatus
  {
    NotStarted = 0,
    Running = 1,
    Finished = 2,
    Failed = 3
  };

  QString transformStatusName(TransformStatus status);

  enum TransformError
  {
    NoError = 0,
    CustomError = 1,
    CancelledError = 2,
    InvalidInputError = 3,
    InvalidParamsError = 4,
    InvalidResultsError = 5,
    InvalidScriptError = 6,
  };

  QString transformErrorName(TransformError error);
}

Q_DECLARE_METATYPE(Ntg::TransformError)
Q_DECLARE_METATYPE(Ntg::TransformStatus)
QDataStream & operator<< ( QDataStream & out, const Ntg::TransformStatus & s);
QDataStream & operator<< ( QDataStream & out, const Ntg::TransformError & e);
QDataStream & operator>> ( QDataStream & in, Ntg::TransformStatus & s);
QDataStream & operator>> ( QDataStream & in, Ntg::TransformError & e);

struct NtgTransformParam
{
  QString name;
  QString longName;
  QString description;
  QString defaultValue;
  bool isOptional;
  QString level;
  QString format;
  QString ioMode;
  QList<QMap<QString,QString> > formatParam;
};
Q_DECLARE_METATYPE(NtgTransformParam)

QDebug operator<<(QDebug dbg, const NtgTransformParam & tp);
QDataStream & operator<< ( QDataStream & out, const NtgTransformParam & tp);
QDataStream & operator>> ( QDataStream & in, NtgTransformParam & tp);
QMap<QString,QVariant> ParamToMap(NtgTransformParam tp, QString modelName);
NtgTransformParam mapToParam(QMap<QString,QVariant> paramMap);

typedef QHash<QString, NtgTransformParam > HashStringTParam;

struct NtgTransformModel
{
  NtgTransformModel() {}

  QString name;
  QString longName;
  bool enable;
  QString pluginName;
  QString description;
  QString category;
  QStringList inputTypes;
  QStringList outputTypes;
  QHash<QString, NtgTransformParam> params;
};

Q_DECLARE_METATYPE(NtgTransformModel)
Q_DECLARE_METATYPE(HashStringTParam)
Q_DECLARE_METATYPE(QList<NtgTransformModel>)

QDebug operator<<(QDebug dbg, const NtgTransformModel & tm);
QDataStream & operator<< ( QDataStream & out, const NtgTransformModel & tm);
QDataStream & operator>> ( QDataStream & in, NtgTransformModel & tm);
bool operator== (const NtgTransformModel tm_1, const NtgTransformModel tm_2);
QMap<QString,QVariant> transformToMap(NtgTransformModel tm);
NtgTransformModel mapToTransformModel(QMap<QString,QVariant> transformMap);
QHash<QString,NtgTransformModel> mapToTransformModelHash(QMap<QString,QVariant> transformMap);

struct NtgEntityModel
{
  NtgEntityModel() {}

  QString name;
  bool browsable;
  QString longName;
  QString color;
  QString description;
  QString category;
  QString parent;
  QByteArray imagePNG;
  QByteArray imageSVG;
  QHash<QString, NtgTransformParam> params;
};

Q_DECLARE_METATYPE(NtgEntityModel)
Q_DECLARE_METATYPE(QList<NtgEntityModel>)

QDebug operator<<(QDebug dbg, const NtgEntityModel & em);
QDataStream & operator<< ( QDataStream & out, const NtgEntityModel & em);
QDataStream & operator>> ( QDataStream & in, NtgEntityModel & em);
QMap<QString,QVariant> entityToMap(NtgEntityModel em);
NtgEntityModel mapToEntityModel(QMap<QString,QVariant> entityMap);
QHash<QString,NtgEntityModel> mapToEntityModelHash(QMap<QString,QVariant> entityMap);

typedef QHash<QString, QString> HashStringString;
struct NtgEntity
{
  NtgEntity() : id(0) {}
  NtgEntity(const QString & type_, const QHash<QString,QString> & values_)
    : type(type_) , values(values_), id(0) {}

  QString type;
  QHash<QString, QString> values;
  quint64 id;
};
Q_DECLARE_METATYPE(NtgEntity)
Q_DECLARE_METATYPE(HashStringString)

QDebug operator<<(QDebug dbg, const NtgEntity & e);
QDataStream & operator<< ( QDataStream & out, const NtgEntity & e);
QDataStream & operator>> ( QDataStream & in, NtgEntity & e);
bool operator== ( const NtgEntity  e_1,const NtgEntity  e_2);


Q_DECLARE_METATYPE(QList<NtgEntity>)





#endif // TRANSFORM_TYPES_H
