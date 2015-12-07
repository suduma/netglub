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

#include "ntgTransformManager.h"
#include <QVariant>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlError>

#include <QDebug>
#include <QDir>
#include <QMetaType>
#include <QDomDocument>
#include <QPluginLoader>
#include <QLibrary>
#include <QCoreApplication>
#include <QBuffer>
#include <QFile>
#include <QtGui/QImageReader>
#include <QtGui/QImageWriter>
#include <QtGui/QPixmap>

#define XML_FAIL(format,...) {qCritical("XML Parse Error: " format, ##__VA_ARGS__);continue;}
#define XML_WARN(format,...) {qWarning( "XML Parse Warning: " format, ##__VA_ARGS__);}


NtgTransformManager::NtgTransformManager(QSqlDatabase * db, const QString & transformsPath, const QString & entitiesPath)
  : _db(db),
    _mutex(QMutex::Recursive)

{
  _masterTransformModelHash = new QHash<QString,NtgTransformModel *>();
  _masterEntityModelHash = new QHash<QString,NtgEntityModel *>();
  
  //crating the catalogues :
  _createTransformsModels(transformsPath);
  _createEntitiesModels(entitiesPath);

//DEBUG
//  foreach (NtgTransformModel *  transformModel, *_masterTransformModelHash)
//  {
//    qDebug("[+] TRANSFORM = %s ", qPrintable(transformModel->name));
//  }
}


QHash<QString,NtgTransformModel *> NtgTransformManager::transformsModels()
{
  return *_masterTransformModelHash;
}

QHash<QString,NtgEntityModel *> NtgTransformManager::entitiesModels()
{
  return *_masterEntityModelHash;
}

NtgTransformManager::~NtgTransformManager()
{
  delete _masterTransformModelHash;
  delete _masterEntityModelHash;
}

NtgTransformModel * NtgTransformManager::getTransformModel(const QString & name)
{
  QMutexLocker locker(&_mutex);
  return (_masterTransformModelHash->value(name));
}

void NtgTransformManager::_createTransformsModels(QString path)
{
  QMutexLocker locker(&_mutex);
  QSqlQuery getAllTransformModelQuery(*_db);

  if( ! getAllTransformModelQuery.prepare("SELECT T.*, C.`shortname` FROM `transforms` AS T JOIN `join_transforms_category_transforms` AS J ON T.`name` = J.`transform_name` JOIN `transforms_category` AS C ON J.category_name = C.name") )
  {
    qCritical("NtgTransformManager::_createTransformsModels : Query invalide : %s", qPrintable(getAllTransformModelQuery.lastError().text()));
  }
  if( ! getAllTransformModelQuery.exec() )
  {
    qCritical("NtgTransformManager::_createTransformsModels : getAllTransformModelQuery %s", qPrintable(getAllTransformModelQuery.lastError().text()));
  }
  QPair<bool,QString> pair;
  while (getAllTransformModelQuery.next())
  {
      QPair<bool,QString> pair;
      pair.first = getAllTransformModelQuery.value(getAllTransformModelQuery.record().indexOf("enable")).toBool();
      pair.second = getAllTransformModelQuery.value(getAllTransformModelQuery.record().indexOf("shortname")).toString();
      _transformsInfoFromDB.insert(getAllTransformModelQuery.value(getAllTransformModelQuery.record().indexOf("name")).toString(),pair);
  }
  _createTransformsModelsFromXmls(path);

  if (!_transformsInfoFromDB.isEmpty())
  {
    qCritical() << "there is no xml description for the following transforms : " << _transformsInfoFromDB.keys();
  }
}

void NtgTransformManager::_createTransformsModelsFromXmls(QString path)
{
  QMutexLocker locker(&_mutex);
  QDir dir(path);
  QStringList filters;
  filters << "conf.xml";
  QFileInfoList entryList = dir.entryInfoList(filters, QDir::NoDotAndDotDot |
                                                       QDir::Files |
                                                       QDir::AllDirs);
  foreach(const QFileInfo & fileInfo, entryList)
  {
    if(fileInfo.isDir())
    {
      _createTransformsModelsFromXmls(fileInfo.absoluteFilePath());
      continue;
    }
    else
    {
      QFile xmlConf(fileInfo.absoluteFilePath());
      if( ! xmlConf.open(QIODevice::ReadOnly))
        XML_FAIL("Could not open conf file %s", qPrintable(fileInfo.absoluteFilePath()));
      QDomDocument doc;
      doc.setContent(&xmlConf);
      if( doc.doctype().name() != "TransformSchema")
        XML_FAIL("Wrong DOCTYPE, is %s",qPrintable(doc.doctype().name()));

      NtgTransformModel* model = new NtgTransformModel();
      QDomElement rootElem = doc.documentElement();
      if(rootElem.tagName() != "transform")
        XML_FAIL("Root node should be <transform>, but is <%s>", qPrintable(rootElem.tagName()));

      model->name = rootElem.attribute("name");
      if(model->name.isEmpty())
        XML_FAIL("Empty name in <transform>");

      model->longName = rootElem.attribute("longName");
      if(model->longName.isEmpty())
        XML_FAIL("Empty long name in <transform>");

      model->pluginName = rootElem.attribute("type");

      QDomElement descNode = rootElem.firstChildElement("description");
      if(descNode.isNull())
        XML_FAIL( "No <description> in the XML!");
      model->description = descNode.text();

      QDomElement parametersNode = rootElem.firstChildElement("parameters");
      if(parametersNode.isNull())
        XML_WARN( "No <parameters> in the XML!");

      for(QDomElement paramNode = parametersNode.firstChildElement("param");
          ! paramNode.isNull();paramNode = paramNode.nextSiblingElement("param"))
      {
        NtgTransformParam param;
        param.name = paramNode.attribute("name");
        if(param.name.isEmpty())
          XML_FAIL("<param> has no name!");
        if(model->params.contains(param.name))
          XML_FAIL("Duplicate \"%s\" <param>!", qPrintable(param.name));
        param.longName = paramNode.attribute("longName");
        if (param.longName.isEmpty())
            XML_FAIL("<param> \"%s\" has no longName!", qPrintable(param.name));
        param.defaultValue = paramNode.attribute("default");
        param.description = paramNode.attribute("description");
        QString optionalStr = paramNode.attribute("optional").toLower();
        if( optionalStr == "true")
          param.isOptional = true;
        else if( optionalStr == "false")
          param.isOptional = false;
        else
          XML_FAIL("<param> \"%s\" has invalid optional field.", qPrintable(param.name));

        param.format = paramNode.attribute("format");
        param.level = paramNode.attribute("level");

        QList<QMap<QString,QString> > formatParamList;
        QMap<QString,QString> formatParamMap;
        for(QDomElement paramFormatNode = paramNode.firstChildElement(param.format);
          ! paramFormatNode.isNull();paramFormatNode = paramFormatNode.nextSiblingElement(param.format))
        {
          QDomNamedNodeMap formatNode = paramFormatNode.attributes();
          for (int ind = 0 ; ind<formatNode.count();ind++)
          {
            formatParamMap.insert(formatNode.item(ind).nodeName(),
                                  formatNode.item(ind).nodeValue());
          }
          formatParamList.append(formatParamMap);
          formatParamMap.clear();
        }
        param.formatParam= formatParamList;
        model->params[param.name] = param;
      }

      QHash<QString,QString> values;
      QDomElement configNode = rootElem.firstChildElement("config");
      for(QDomElement valNode = configNode.firstChildElement("value");
          ! valNode.isNull();valNode = valNode.nextSiblingElement("value"))
      {
        QString name = valNode.attribute("name");
        QString val = valNode.text();
        if(name.isEmpty())
          XML_FAIL("<value> has no name!");
        if(values.contains(name))
          XML_FAIL("Duplicate \"%s\" <value>!", qPrintable(name));
        values[name] = val;
      }

      QDomElement inputNode = rootElem.firstChildElement("input");
      if(inputNode.isNull())
        XML_FAIL( "No <input> in the XML!");

      for(QDomElement entityNode = inputNode.firstChildElement("entity");
          ! entityNode.isNull();entityNode = entityNode.nextSiblingElement("entity"))
      {
        QString entityType = entityNode.attribute("type");
        if(entityType.isEmpty())
          XML_FAIL("Input <entity> has empty type!");
        model->inputTypes << entityType;
      }

      QDomElement outputNode = rootElem.firstChildElement("output");
      if(outputNode.isNull())
        XML_FAIL( "No <output> in the XML!");

      for(QDomElement entityNode = outputNode.firstChildElement("entity");
          ! entityNode.isNull();entityNode = entityNode.nextSiblingElement("entity"))
      {
        QString entityType = entityNode.attribute("type");
        if(entityType.isEmpty())
          XML_FAIL("Output <entity> has empty type!");
        model->outputTypes << entityType;
      }

      //FEATURES include path of conflicting transforms
//      qDebug()<< *model;
      if ( _masterTransformModelHash->contains(model->name))
      {
          XML_FAIL("Conflicting transform name '%s' ", qPrintable(model->name));
          return;
      }
      if ( !_transformsInfoFromDB.contains(model->name) )
      {
        qCritical() << "No such xml transform description into database."
                    << "\n\tDetail:" << fileInfo.absoluteFilePath();
      }
      else
      {
          QPair<bool,QString> pair = _transformsInfoFromDB.take(model->name);
          model->enable = pair.first;
          model->category = pair.second;
          _masterTransformModelHash->insert(model->name,model);
      }
    }

  }
}

void NtgTransformManager::_createEntitiesModels(QString path)
{
  QMutexLocker locker(&_mutex);
  QSqlQuery getAllEntityModelQuery(*_db);

  if( ! getAllEntityModelQuery.prepare("SELECT E.*, D.`shortname` FROM `entities` AS E LEFT JOIN `join_entities_category_entities` AS J ON E.`name` = J.`entity_name`LEFT JOIN `entities_category` AS D ON J.`category_name`=D.`name`") )
  {
    qCritical("NtgTransformManager::_createEntitiesModels : Query invalide : %s", qPrintable(getAllEntityModelQuery.lastError().text()));
  }
  if( ! getAllEntityModelQuery.exec() )
  {
    qCritical("NtgTransformManager::_createEntitiesModels : getAllTransformModelQuery %s", qPrintable(getAllEntityModelQuery.lastError().text()));
  }
  while (getAllEntityModelQuery.next())
  {
      QPair<bool,QString> pair;
      pair.first = getAllEntityModelQuery.value(getAllEntityModelQuery.record().indexOf("browsable")).toBool();
      pair.second = getAllEntityModelQuery.value(getAllEntityModelQuery.record().indexOf("shortname")).toString();
      _entitiesInfoFromDB.insert(getAllEntityModelQuery.value(getAllEntityModelQuery.record().indexOf("name")).toString(),pair);

  }

  _createEntitiesModelsFromXmls(path);


  if (!_entitiesInfoFromDB.isEmpty())
  {
    qCritical() << "there is no xml description for the following entities : " << _entitiesInfoFromDB.keys();
  }
}

void NtgTransformManager::_createEntitiesModelsFromXmls(QString path)
{
  QMutexLocker locker(&_mutex);
  QDir dir(path);
  QStringList filters;
  filters << "desc.xml";
  QFileInfoList entryList = dir.entryInfoList(filters, QDir::NoDotAndDotDot |
                                                       QDir::Files |
                                                       QDir::AllDirs);

  foreach(const QFileInfo & fileInfo, entryList)
  {
    if(fileInfo.isDir())
    {
      _createEntitiesModelsFromXmls(fileInfo.absoluteFilePath());
      continue;
    }
    else
    {
      QFile xmlConf(fileInfo.absoluteFilePath());
      if( ! xmlConf.open(QIODevice::ReadOnly))
        XML_FAIL("Could not open conf file %s", qPrintable(fileInfo.absoluteFilePath()));
      QDomDocument doc;
      doc.setContent(&xmlConf);
      if( doc.doctype().name() != "EntitySchema")
        XML_FAIL("Wrong DOCTYPE, is %s",qPrintable(doc.doctype().name()));

      NtgEntityModel* model = new NtgEntityModel();
      QDomElement rootElem = doc.documentElement();
      if(rootElem.tagName() != "entity")
        XML_FAIL("Root node should be <transform>, but is <%s>", qPrintable(rootElem.tagName()));

      model->name = rootElem.attribute("name");
      if(model->name.isEmpty())
        XML_FAIL("Empty name in <entity>");

      model->longName = rootElem.attribute("longName");
      if(model->longName.isEmpty())
        XML_FAIL("Empty longName in <entity>");

      QString pngImage = rootElem.attribute("pngImage");
      if (pngImage.isEmpty())
          XML_FAIL( "No <PNG image> in the XML!");
      
      QFile pngFile(path+"/"+pngImage);
      if (pngFile.open(QIODevice::ReadOnly))
      {
        model->imagePNG = pngFile.readAll();
        pngFile.close();
      }
      
      QString svgImage = rootElem.attribute("svgImage");
      if (svgImage.isEmpty())
          XML_FAIL( "No <SVG image> in the XML!");

      QFile svgFile(path+"/"+svgImage);
      if (svgFile.open(QIODevice::ReadOnly))
      {
        model->imageSVG = svgFile.readAll();
        svgFile.close();
      }

      model->color = rootElem.attribute("color");
      model->parent = rootElem.attribute("parent");

      QDomElement descNode = rootElem.firstChildElement("description");
      if(descNode.isNull())
        XML_FAIL( "No <description> in the XML!");
      model->description = descNode.text();

      QDomElement parametersNode = rootElem.firstChildElement("fieldList");
      if(parametersNode.isNull())
        XML_WARN( "No <parameters> in the XML!");
      for(QDomElement paramNode = parametersNode.firstChildElement("field");
          ! paramNode.isNull();paramNode = paramNode.nextSiblingElement("field"))
      {
        NtgTransformParam param;
        param.name = paramNode.attribute("name");
        if(param.name.isEmpty())
          XML_FAIL("<field> has no name!");
        if(model->params.contains(param.name))
          XML_FAIL("Duplicate \"%s\" <field>!", qPrintable(param.name));
        param.longName = paramNode.attribute("longName");
        if (param.longName.isEmpty())
            XML_FAIL("<param> \"%s\" has no longName!", qPrintable(param.name));
        param.defaultValue = paramNode.attribute("default");
        param.ioMode = paramNode.attribute("ioMode");
        if (param.ioMode.isEmpty())
            XML_FAIL("<param> \"%s\" has no ioMode!", qPrintable(param.name));
        param.description = paramNode.attribute("description");
        QString optionalStr = paramNode.attribute("optional").toLower();
        if( optionalStr == "true")
          param.isOptional = true;
        else
          param.isOptional = false;

        param.format = paramNode.attribute("format");
        param.level = paramNode.attribute("level");
        QList<QMap<QString,QString> > formatParamList;
        QMap<QString,QString> formatParamMap;
        for(QDomElement paramFormatNode = paramNode.firstChildElement(param.format);
          ! paramFormatNode.isNull();paramFormatNode = paramFormatNode.nextSiblingElement(param.format))
        {
          QDomNamedNodeMap formatNode = paramFormatNode.attributes();
          for (int ind = 0 ; ind<formatNode.count();ind++)
          {
            formatParamMap.insert(formatNode.item(ind).nodeName(),
                                  formatNode.item(ind).nodeValue());
          }
          formatParamList.append(formatParamMap);
          formatParamMap.clear();
        }
        param.formatParam= formatParamList;
        model->params[param.name] = param;
      }

      //FEATURES include path of conflicting entities
      if ( _masterEntityModelHash->contains(model->name))
      {
          XML_FAIL("Conflicting entity name '%s' ", qPrintable(model->name));
          return;
      }
      if ( !_entitiesInfoFromDB.contains(model->name))
      {
        qCritical() << "the entity \"" << qPrintable(model->name) <<"\" doesn't exist in the database.";
      }
      else
      {
          QPair<bool,QString> pair = _entitiesInfoFromDB.take(model->name);

          model->browsable = pair.first;
          model->category = pair.second;
          _masterEntityModelHash->insert(model->name,model);
      }
    }

  }

}

