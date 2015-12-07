/******************************************************************************
**
** Copyright (C) 2009-2010 DIATEAM. All rights reserved.
**
** This file is part of the "Netglub" program.
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

#include "ntgTypes.h"

void Ntg::initMetatypes()
{
  qRegisterMetaType<NtgTransformModel>();
  qRegisterMetaTypeStreamOperators<NtgTransformModel>("NtgTransformModel");

  qRegisterMetaType<NtgTransformParam>();
  qRegisterMetaTypeStreamOperators<NtgTransformParam>("NtgTransformParam");

  qRegisterMetaType<HashStringTParam>();
  qRegisterMetaTypeStreamOperators<HashStringTParam>("HashStringTParam");

  qRegisterMetaType<HashStringString>();
  qRegisterMetaTypeStreamOperators<HashStringString>("HashStringString");

  qRegisterMetaType<QList<NtgTransformModel> >();
  qRegisterMetaTypeStreamOperators<QList<NtgTransformModel> >("QList<NtgTransformModel>");

  qRegisterMetaType<NtgEntity>();
  qRegisterMetaTypeStreamOperators<NtgEntity>("NtgEntity");

  qRegisterMetaType<QList<NtgEntity> >();
  qRegisterMetaTypeStreamOperators<QList<NtgEntity> >("QList<NtgEntity>");

  qRegisterMetaType<Ntg::TransformStatus>();
  qRegisterMetaTypeStreamOperators<Ntg::TransformStatus>("Ntg::TransformStatus");

  qRegisterMetaType<Ntg::TransformError>();
  qRegisterMetaTypeStreamOperators<Ntg::TransformError>("Ntg::TransformError");

  qRegisterMetaType<NtgEntityModel>();
  qRegisterMetaTypeStreamOperators<Ntg::TransformError>("NtgEntityModel");

  qRegisterMetaType<QList<NtgEntityModel> >();
  qRegisterMetaTypeStreamOperators<Ntg::TransformError>("QList<NtgEntityModel>");
}


typedef QMap<int,QString> IntStringMap;

Q_GLOBAL_STATIC_WITH_INITIALIZER(IntStringMap, _transformStatus, {
  x->insert((int)Ntg::NotStarted, "Not Started");
  x->insert((int)Ntg::Running, "Running");
  x->insert((int)Ntg::Finished, "Finished");
  x->insert((int)Ntg::Failed, "Failed");
});

QString Ntg::transformStatusName(TransformStatus status)
{
  return _transformStatus()->value((int)status);
}

Q_GLOBAL_STATIC_WITH_INITIALIZER(IntStringMap, _transformError, {
  x->insert((int)Ntg::NoError, "No Error");
  x->insert((int)Ntg::CustomError, "Custom Error");
  x->insert((int)Ntg::CancelledError, "Cancelled Error");
  x->insert((int)Ntg::InvalidInputError, "Invalid Input Error");
  x->insert((int)Ntg::InvalidParamsError, "Invalid Params Error");
  x->insert((int)Ntg::InvalidResultsError, "Invalid Results Error");
  x->insert((int)Ntg::InvalidScriptError, "Invalid Script Error");
});

QString Ntg::transformErrorName(TransformError error)
{
  return _transformError()->value((int)error);
}

QDataStream & operator<< ( QDataStream & out, const Ntg::TransformStatus & s)
{
  return out << (int)s;
}

QDataStream & operator<< ( QDataStream & out, const Ntg::TransformError & e)
{
  return out << (int)e;
}

QDataStream & operator>> ( QDataStream & in, Ntg::TransformStatus & s)
{
  int i;
  in >> i;
  s = (Ntg::TransformStatus)i;
  return in;
}

QDataStream & operator>> ( QDataStream & in, Ntg::TransformError & e)
{
  int i;
  in >> i;
  e = (Ntg::TransformError)i;
  return in;
}

//----------------------- TransformParam --------------------------------------

QDebug operator<<(QDebug dbg, const NtgTransformParam & tp)
{
  dbg << "\n    Name:" << tp.name << "\n"
      << "\tLong Name:" << tp.longName << "\n"
      << "\tOptional:" << tp.isOptional << "\n"
      << "\tioMode:" << tp.ioMode << "\n"
      << "\tDescription:" << tp.description << "\n"
      << "\tDefault value:" << tp.defaultValue << "\n"
      << "\tLevel:" << tp.level << "\n"
      << "\tFormat:" << tp.format << "\n"
      << "\t\tparameter(s):" << tp.formatParam << "\n";
  return dbg.nospace();
}

QDataStream & operator<< ( QDataStream & out, const NtgTransformParam & tp)
{
  out << tp.name << tp.longName << tp.isOptional << tp.description << tp.defaultValue << tp.level
      << tp.format << tp.ioMode << tp.formatParam;
  return out;
}


QDataStream & operator>> ( QDataStream & in, NtgTransformParam & tp)
{
  in >> tp.name >> tp.longName >> tp.isOptional >> tp.description >> tp.defaultValue >> tp.level
     >> tp.format >> tp.ioMode >> tp.formatParam;
  return in;
}

QMap<QString,QVariant> ParamToMap(NtgTransformParam tp, QString modelName)
{
  QMap<QString,QVariant> paramMap;

  paramMap.insert(modelName + (QString("|param|"))
                  .append(tp.name) + ("|longName"),QVariant(tp.longName));
    paramMap.insert(modelName + (QString("|param|"))
                    .append(tp.name) + ("|isOptional"),QVariant(tp.isOptional));
    paramMap.insert(modelName + (QString("|param|"))
                    .append(tp.name) + ("|description"),QVariant(tp.description));
    paramMap.insert(modelName + (QString("|param|"))
                    .append(tp.name) + ("|ioMode"),QVariant(tp.ioMode));
    paramMap.insert(modelName + (QString("|param|"))
                    .append(tp.name) + ("|defaultValue"),QVariant(tp.defaultValue));
    paramMap.insert(modelName + (QString("|param|"))
                    .append(tp.name) + ("|level"),QVariant(tp.level));


    QString paramFormatStr;
    for (int ind=0;ind < tp.formatParam.count();ind++)
    {
      foreach (QString key, tp.formatParam.at(ind).keys())
      {
        paramFormatStr += key + "|" + tp.formatParam.at(ind).value(key) + "!";
      }
      paramFormatStr.chop(1);
      paramMap.insert(modelName + QString("|param|").append(tp.name) + "|format|"
                           + tp.format + "|" +QString::number(ind),QVariant(paramFormatStr));
      paramFormatStr.clear();
    }
  //qDebug () << mapToParam(paramMap);
  return paramMap;
}

 NtgTransformParam mapToParam(QMap<QString,QVariant> paramMap)
{
   // here the map would be like : DDToIp|param|timeOut|description
  NtgTransformParam tp;
  QStringList paramKey;
  QList<QMap<QString,QString> > formatList;
  //qDebug() << paramMap;
  foreach (QString key , paramMap.keys())
  {
    paramKey = key.split("|");
    if (paramKey.length() < 3)
    {
      qWarning() << "Error parameter with key :" << key ;
      continue;
    }
    tp.name = paramKey.at(2);
    if (!tp.name.isNull())
    {
      if (paramKey.at(3) == "longName")
      {
        tp.longName = paramMap.value(key).toString();
      }
      else if (paramKey.at(3) == "isOptional")
      {
        tp.isOptional = paramMap.value(key).toBool();
      }
      else if (paramKey.at(3) == "description")
      {
        tp.description = paramMap.value(key).toString();
      }
      else if (paramKey.at(3) == "defaultValue")
      {
        tp.defaultValue = paramMap.value(key).toString();
      }
      else if (paramKey.at(3) == "ioMode")
      {
        tp.ioMode = paramMap.value(key).toString();
      }
      else if (paramKey.at(3) == "level")
      {
        tp.level = paramMap.value(key).toString();
      }
      else if (paramKey.at(3) == "format")
      {
        if (paramKey.count() < 4)
        {
          qWarning() << "Error parameter with key :" << key ;
          continue;
        }
        tp.format = paramKey.at(4);

        QStringList formatParamList = paramMap.value(key).toString().split("!");

        QMap<QString,QString> formatParamMap;

        foreach (QString formatParamStr, formatParamList)
        {
          formatParamMap.insert(formatParamStr.split("|").at(0),
                                formatParamStr.split("|").at(1));
        }

        formatList.append(formatParamMap);
      }

    }
  }
  //qDebug() << formatList;
  tp.formatParam = formatList;
  return tp;
}


//----------------------- TransformModel --------------------------------------

QDebug operator<<(QDebug dbg, const NtgTransformModel & tm)
{
  dbg << "\nName:" << tm.name << "\n"
      << "Long Name:" << tm.longName << "\n"
      << "Enabled:" << QString(tm.enable?"true":"false") << "\n"
      << "Classname:" << tm.pluginName << "\n"
      << "Description:" << tm.description << "\n"
      << "Category:" << tm.category << "\n"
      << "Input Types:" << tm.inputTypes << "\n"
      << "Output Types:" << tm.outputTypes << "\n"
      << "Params:" << tm.params << "\n";
  return dbg.nospace();
}


QDataStream & operator<< ( QDataStream & out, const NtgTransformModel & tm)
{
  out << tm.name  << tm.longName << tm.pluginName << tm.description << tm.inputTypes
      << tm.outputTypes << tm.params;// << tm.category;
  return out;
}


QDataStream & operator>> ( QDataStream & in, NtgTransformModel & tm)
{
  in >> tm.name >> tm.longName >> tm.pluginName >> tm.description >> tm.inputTypes
     >> tm.outputTypes >> tm.params;// >> tm.category;
  return in;
}


bool operator== (const NtgTransformModel tm_1, const NtgTransformModel tm_2)
{
  return tm_1.name==tm_2.name
      && tm_1.inputTypes==tm_2.inputTypes
      && tm_1.outputTypes==tm_2.outputTypes;
}


QMap<QString,QVariant> transformToMap(NtgTransformModel tm)
{
  QMap<QString,QVariant> entityMap;

    entityMap.insert(tm.name + ("|pluginName"),QVariant(tm.pluginName));
    entityMap.insert(tm.name + ("|longName"),QVariant(tm.longName));
    entityMap.insert(tm.name + ("|enable"),QVariant(tm.enable));
    entityMap.insert(tm.name + ("|description"),QVariant(tm.description));
    entityMap.insert(tm.name + ("|category"),QVariant(tm.category));
    QString typesString("");
    foreach (QString type,tm.inputTypes )
    {
      typesString.append(type+ "|");
    }
    entityMap.insert(tm.name + ("|inputTypes"),QVariant(typesString));

    typesString.clear();
    foreach (QString type,tm.outputTypes )
    {
      typesString.append(type+ "|");
    }
    entityMap.insert(tm.name + ("|outputTypes"),QVariant(typesString));
    foreach (NtgTransformParam param, tm.params)
    {
      entityMap.unite(ParamToMap(param,tm.name));
    }

  return entityMap;
}

NtgTransformModel mapToTransformModel(QMap<QString,QVariant> transformMap)
{
   // here the map would be like : DDToIp|description
  NtgTransformModel tm;
  QStringList transformKey;
  QString str = transformMap.keys().at(0).split('|').at(0);
  QMap<QString,QVariant>  paramMap;
  QHash<QString,QMap<QString,QVariant> > listOfParameters;
  QString paramName("");
  foreach (QString key , transformMap.keys())
  {
   // qDebug() << key;
    transformKey = key.split("|");
    tm.name = transformKey.at(0);
    if (tm.name == str)
    {
      if (transformKey.at(1) == "pluginName")
      {
        tm.pluginName = transformMap.value(key).toString();
      }
      else if (transformKey.at(1) == "longName")
      {
        tm.longName = transformMap.value(key).toString();
      }
      else if (transformKey.at(1) == "enable")
      {
        tm.enable = transformMap.value(key).toBool();
      }
      else if (transformKey.at(1) == "description")
      {
        tm.description = transformMap.value(key).toString();
      }
      else if (transformKey.at(1) == "category")
      {
        tm.category = transformMap.value(key).toString();
      }
      else if (transformKey.at(1) == "inputTypes")
      {
        QStringList typesList;
        if (transformKey.length() >1)
        {
          typesList = transformMap.value(key).toString().split("|",QString::SkipEmptyParts);
        }
        tm.inputTypes = typesList;
      }
      else if (transformKey.at(1) == "outputTypes")
      {
        QStringList typesList;
        if (transformKey.length() >1)
        {
          typesList = transformMap.value(key).toString().split("|",QString::SkipEmptyParts);
        }
        tm.outputTypes = typesList;
      }
      else if (transformKey.at(1) == "param")
      {
        if (transformKey.length() >2)
        {
          paramName = transformKey.at(2);
          if (!listOfParameters.contains(paramName))
          {
            paramMap.clear();
            listOfParameters.insert(paramName,paramMap);
          }

          paramMap = listOfParameters.value(paramName);
          paramMap.insert(key,transformMap.value(key));
          listOfParameters.insert(paramName, paramMap);
        }
        else
        {
          qWarning()<< "A parameters seems to be out of range. key :" << key;
        }
      }
    }
  }

  foreach (QString stt, listOfParameters.keys())
  {
    tm.params.insert(stt,mapToParam(listOfParameters.value(stt)));
  }
  return tm;
}

QHash<QString,NtgTransformModel> mapToTransformModelHash(QMap<QString,QVariant> transformMap)
{
  QHash<QString,NtgTransformModel> transformHash;
  QVariantMap tmp;
  QString temp("");
  while (!transformMap.isEmpty())
  {
    temp = transformMap.keys().at(0).split("|").at(0);
    foreach (QString key , transformMap.keys())
    {
      if (key.split("|").at(0) == temp)
      {
        tmp.insert(key,transformMap.take(key));
      }
    }
    transformHash.insert(temp,mapToTransformModel(tmp));
    tmp.clear();
  }
  return transformHash;
}


//----------------------- EntityModel --------------------------------------

QDebug operator<<(QDebug dbg, const NtgEntityModel & em)
{
  QString test = "false";

  dbg << "\nName:" << em.name << "\n"
      << "Long Name:" << em.longName << "\n"
      << "Color (Format #RRGGBB) :" << em.color << "\n"
      << "Browsable:" << QString(em.browsable?"true":"false") << "\n"
      << "ImagePNG:" << QString(em.imagePNG.isNull() || em.imagePNG.isEmpty()
                             || em.imagePNG == ""?"no image available":"image loaded") << "\n"
      << "ImageSVG:" << QString(em.imageSVG.isNull() || em.imageSVG.isEmpty()
                             || em.imageSVG == ""?"no image available":"image loaded") << "\n"
      << "Description:" << em.description << "\n"
      << "Category:" << em.category << "\n"
      << "Parent:" << em.parent << "\n"
      << "Params:" << em.params << "\n";
  return dbg.nospace();
}


QDataStream & operator<< ( QDataStream & out, const NtgEntityModel & em)
{

  out << em.name << em.browsable << em.longName << em.color << em.imagePNG
      << em.imageSVG << em.description << em.category << em.parent << em.params;

  return out;
}


QDataStream & operator>> ( QDataStream & in, NtgEntityModel & em)
{
  in >> em.name >> em.browsable >> em.longName >> em.color >> em.imagePNG
     >> em.imageSVG >> em.description >> em.category >> em.parent >> em.params;
  return in;
}


QMap<QString,QVariant> entityToMap(NtgEntityModel em)
{
  QMap<QString,QVariant> entityMap;
    entityMap.insert(em.name + ("|browsable"),QVariant(em.browsable));
    entityMap.insert(em.name + ("|longName"),QVariant(em.longName));
    entityMap.insert(em.name + ("|color"),QVariant(em.color));
    entityMap.insert(em.name + ("|imagePNG"),QVariant(em.imagePNG));
    entityMap.insert(em.name + ("|imageSVG"),QVariant(em.imageSVG));
    entityMap.insert(em.name + ("|description"),QVariant(em.description));
    entityMap.insert(em.name + ("|category"),QVariant(em.category));
    entityMap.insert(em.name + ("|parent"),QVariant(em.parent));
    foreach (NtgTransformParam param, em.params)
    {
      entityMap.unite(ParamToMap(param,em.name));
    }

  return entityMap;
}


NtgEntityModel mapToEntityModel(QMap<QString,QVariant> entityMap)
{
   // here the map would be like : DDToIp|description
  NtgEntityModel em;
  QStringList entityKey;
  QString str = entityMap.keys().at(0).split('|').at(0);
  QMap<QString,QVariant>  paramMap;
  QHash<QString,QMap<QString,QVariant> > listOfParameters;
  QString paramName("");
  foreach (QString key , entityMap.keys())
  {
    entityKey = key.split("|");
    em.name = entityKey.at(0);
    if (em.name == str)
    {
      if (entityKey.at(1) == "longName")
      {
        em.longName = entityMap.value(key).toString();
      }
      else if (entityKey.at(1) == "color")
      {
        em.color = entityMap.value(key).toString();
      }
      else if (entityKey.at(1) == "imagePNG")
      {
        em.imagePNG = entityMap.value(key).toByteArray();
      }
      else if (entityKey.at(1) == "imageSVG")
      {
        em.imageSVG = entityMap.value(key).toByteArray();
      }
      else if (entityKey.at(1) == "description")
      {
        em.description = entityMap.value(key).toString();
      }
      else if (entityKey.at(1) == "browsable")
      {
        em.browsable = entityMap.value(key).toBool();
      }
      else if (entityKey.at(1) == "category")
      {
        em.category = entityMap.value(key).toString();
      }
      else if (entityKey.at(1) == "parent")
      {
        em.parent = entityMap.value(key).toString();
      }
      else if (entityKey.at(1) == "param")
      {
        if (entityKey.length() >2)
        {
          paramName = entityKey.at(2);
          if (!listOfParameters.contains(paramName))
          {
            paramMap.clear();
            listOfParameters.insert(paramName,paramMap);
          }

          paramMap = listOfParameters.value(paramName);
          paramMap.insert(key,entityMap.value(key));
          listOfParameters.insert(paramName, paramMap);

        }
        else
        {
          qWarning()<< "A parameters seems to be out of range. key :" << key;
        }
      }
    }
  }
  foreach (QString stt, listOfParameters.keys())
  {
    em.params.insert(stt,mapToParam(listOfParameters.value(stt)));
  }
  return em;
}


QHash<QString,NtgEntityModel> mapToEntityModelHash(QMap<QString,QVariant> entityMap)
{
  QHash<QString,NtgEntityModel> entityHash;
  QVariantMap tmp;
  QString temp("");
  while (!entityMap.isEmpty())
  {
    temp = entityMap.keys().at(0).split("|").at(0);
    foreach (QString key , entityMap.keys())
    {
      if (key.split("|").at(0) == temp)
      {
        tmp.insert(key,entityMap.take(key));
      }
    }
    entityHash.insert(temp,mapToEntityModel(tmp));
    tmp.clear();
  }
  return entityHash;
}

//----------------------------- Entity ----------------------------------------

QDebug operator<<(QDebug dbg, const NtgEntity & e)
{
  dbg << "\nType:" << e.type << "\n"
      << "Values:" << e.values << "\n"
      << "Hash:" << e.id << "\n";
  return dbg.nospace();
}


QDataStream & operator<< ( QDataStream & out, const NtgEntity & e)
{
  out << e.type << e.values << e.id;
  return out;
}


QDataStream & operator>> ( QDataStream & in, NtgEntity & e)
{
  in >> e.type >> e.values >> e.id;
  return in;
}

bool operator== ( const NtgEntity  e_1,const NtgEntity  e_2)
{
    return e_1.type == e_2.type && e_1.values == e_2.values;
}

