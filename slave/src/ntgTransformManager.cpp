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

#include "ntgTransformPlugin.h"

#include <QDebug>
#include <QDir>
#include <QMetaType>
#include <QDomDocument>
#include <QPluginLoader>
#include <QLibrary>
#include <QCoreApplication>


NtgTransformManager::NtgTransformManager(QString xmlDirPath, QString pluginPath, NtgRpcHandler * handler)
  : QObject()
  , _handler(handler)
{
  setObjectName("TManager");
  if(_handler)
    _handler->registerInvokableObject(this);
  _loadPlugins(pluginPath);
  _createModelsFromXml(xmlDirPath);
}


NtgTransformManager::~NtgTransformManager()
{
  _transformInfos.clear();
  _currentTransforms.clear();
  _plugins.clear();
}


QStringList NtgTransformManager::transformList() const
{
  return _transformInfos.keys();
}


NtgTransform * NtgTransformManager::createTransformFromType(QString type)
{
  if( ! _transformInfos.contains(type))
    return NULL;

  NtgTransformInfo info = _transformInfos.value(type);
  NtgTransform * t = info.plugin->createTransform();
  if( ! t ) return NULL;
  t->_model = info.model;
  t->_workDirPath = info.workingDirPath;
  t->_configValues = info.values;
  t->setHandler(_handler);
  connect(t, SIGNAL(progressChanged(double,QString)), this, SLOT(_newProgress(double,QString)));
  connect(t, SIGNAL(resultReadyAt(int,int)), this, SLOT(_newResults(int,int)));
  connect(t, SIGNAL(statusChanged(Ntg::TransformStatus)), this, SLOT(_newStatus(Ntg::TransformStatus)));

  return t;
}


QString NtgTransformManager::createTransform(QString transformType)
{
  if( ! _transformInfos.contains(transformType))
    return QString();
  NtgTransform * t = createTransformFromType(transformType);
  if( ! t)
    return QString();
  QString name = t->objectName();
  _currentTransforms.insert(name, t);
  return name;
}


bool NtgTransformManager::destroyTransform(QString objName)
{
  if( ! _currentTransforms.contains(objName))
    return false;
  delete _currentTransforms.take(objName);
  return true;
}


QStringList NtgTransformManager::currentTransforms(QString transformType)
{
  if(transformType.isEmpty())
    return _currentTransforms.keys();
  QStringList names;
  foreach(const NtgTransform * t, _currentTransforms)
  {
    if(t->type() == transformType)
      names.append(t->objectName());
  }
  return names;
}


QList<NtgTransformModel> NtgTransformManager::models()
{
  QList<NtgTransformModel> models;
  foreach(const NtgTransformInfo & info, _transformInfos)
    models.append(info.model);
  return models;
}


void NtgTransformManager::_newStatus(Ntg::TransformStatus status)
{
  QStringList labels;
  labels << "NotStarted" << "Running" << "Finished" << "Failed";
  qDebug() << "Status: " << labels.at(status);

  NtgTransform * t = qobject_cast<NtgTransform*>(sender());
  if( ! t) qFatal("WTF");
  QStringList errors;
  errors << "NoError" << "CustomError" << "CancelledError"<< "InvalidInputError" << "InvalidParamsError" << "InvalidResultsError";
  if(t->status() == Ntg::Failed)
    qDebug() << "Error: " << errors.at(t->error()) << t->errorString();
}


void NtgTransformManager::_newProgress(double progressPercent, const QString & progressText)
{
  NtgTransform * t = qobject_cast<NtgTransform*>(sender());
  if( ! t) qFatal("WTF");
  qDebug("Progress : %.2lf%% %s", progressPercent, qPrintable(progressText));
}


void NtgTransformManager::_newResults(int fromIndex, int toIndex)
{
  NtgTransform * t = qobject_cast<NtgTransform*>(sender());
  if( ! t) qFatal("WTF");

  qDebug("*** Results from %d to %d", fromIndex, toIndex);

  foreach(const NtgEntity & e, t->results(fromIndex,toIndex))
  {
      qDebug() << "Result:" << e.type;
      qDebug() << "       " << e.values;
  }
}


void NtgTransformManager::_loadPlugins(QString path)
{
  QDir pluginsDir(path);
  qDebug("Loading plugins .... %s", qPrintable(pluginsDir.absolutePath()));
  QFileInfoList entryList = pluginsDir.entryInfoList(QDir::NoDotAndDotDot | QDir::Files |
                                                     QDir::AllDirs);
  foreach(const QFileInfo & fileInfo, entryList)
  {
    if(fileInfo.isDir())
    {
      _loadPlugins(fileInfo.absoluteFilePath());
      continue;
    }
    else if (QLibrary::isLibrary(fileInfo.absoluteFilePath()))
    {
      QPluginLoader pluginLoader(fileInfo.absoluteFilePath());
      QObject * plugin = pluginLoader.instance();
      if (plugin)
      {
        NtgTransformPlugin * transformPlugin = qobject_cast<NtgTransformPlugin*>(plugin);
        if (transformPlugin)
        {
          _plugins.insert(transformPlugin->transformType(), transformPlugin);
          qDebug("Loaded plugin: %s", qPrintable(transformPlugin->transformType()));
        }
        else
        {
          qWarning("Plugin \"%s\" doesn't implement NtgTransformPlugin interface",
                   qPrintable(fileInfo.fileName()));
        }
      }
      else
      {
        qWarning("Loading plugin \"%s\" failed: %s", qPrintable(fileInfo.fileName()),
                 qPrintable(pluginLoader.errorString()));
      }
    }
  }
}


#define XML_FAIL(format,...) {qCritical("XML Parse Error: " format, ##__VA_ARGS__);continue;}
#define XML_WARN(format,...) {qWarning( "XML Parse Warning: " format, ##__VA_ARGS__);}


void NtgTransformManager::_createModelsFromXml(QString path)
{
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
      _createModelsFromXml(fileInfo.absoluteFilePath());
      continue;
    }
    else
    {
      //qDebug("Parsing file %s", qPrintable(fileInfo.absoluteFilePath()));

      QFile xmlConf(fileInfo.absoluteFilePath());
      if( ! xmlConf.open(QIODevice::ReadOnly))
        XML_FAIL("Could not open conf file %s", qPrintable(fileInfo.absoluteFilePath()));

      QDomDocument doc;
      doc.setContent(&xmlConf);
      if( doc.doctype().name() != "TransformSchema")
        XML_FAIL("Wrong DOCTYPE, is %s",qPrintable(doc.doctype().name()));

      NtgTransformModel model;
      QDomElement rootElem = doc.documentElement();
      if(rootElem.tagName() != "transform")
        XML_FAIL("Root node should be <transform>, but is <%s>", qPrintable(rootElem.tagName()));

      model.name = rootElem.attribute("name");
      if(model.name.isEmpty())
        XML_FAIL("Empty name in <transform>");

      model.longName = rootElem.attribute("longName");
      if(model.longName.isEmpty())
        XML_FAIL("Empty long name in <transform>");

      model.pluginName = rootElem.attribute("type");
      NtgTransformPlugin * plugin = _plugins.value(model.pluginName);
      if( ! plugin )
        XML_FAIL("Unsupported transform type \"%s\"", qPrintable(model.pluginName));

      QDomElement descNode = rootElem.firstChildElement("description");
      if(descNode.isNull())
        XML_FAIL( "No <description> in the XML!");
      model.description = descNode.text();

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
        if(model.params.contains(param.name))
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
        model.params[param.name] = param;
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
        model.inputTypes << entityType;
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
        model.outputTypes << entityType;
      }

      if( _transformInfos.contains(model.name))
      {
        XML_FAIL("Conflicting transform name '%s', used in both: \"%s\" and \"%s\"",
                 qPrintable(model.name), qPrintable(fileInfo.absolutePath()),
                 qPrintable(_transformInfos[model.name].workingDirPath));
      }
      _transformInfos[model.name].plugin = plugin;
      _transformInfos[model.name].model = model;
      _transformInfos[model.name].workingDirPath = fileInfo.absolutePath();
      _transformInfos[model.name].values = values;
    }
  }
}


