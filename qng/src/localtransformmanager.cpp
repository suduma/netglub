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


#include "localtransformmanager.h"
#include "localtransformplugin.h"

#include <QDebug>
#include <QDir>
#include <QMetaType>
#include <QDomDocument>
#include <QPluginLoader>
#include <QLibrary>
#include <QCoreApplication>

#include <stdlib.h>



/****************************************************************************
**  class LocalTransform
****************************************************************************/

quint32 LocalTransform::_instanceCounter = 0;
int LocalTransform::_idCount = 0;

LocalTransform::LocalTransform()
    : QObject()
    ,_id(++_idCount)
{
    setObjectName(QString("LocalTransform.%1").arg(_instanceCounter++));

    _status = Ntg::NotStarted;
    _progressPercent = 0;
    _progressText = "Not running";
    _error = Ntg::NoError;

}


LocalTransform::~LocalTransform()
{
    cancel();
}


int LocalTransform::id() const
{
    return _id;
}


bool LocalTransform::exec(const NtgEntity & input, HashStringString params)
{
    qDebug()<<"LocalTransform::exec";
    if( _status == Ntg::Running)
    {
        qCritical("LocalTransform%s already running ! Not doing anything.", qPrintable(objectName()));
        return 0;
    }
    _reset();

    if( ! _model.inputTypes.contains(input.type) )
    {
        QString err("Received input type \"%1\" is not handled by transform \"%2\"!");
        _setError(Ntg::InvalidInputError, err.arg(input.type).arg(_model.name));
        return 0;
    }

    foreach(const NtgTransformParam & param, _model.params)
    {
        if( ! params.contains(param.name))
        {
            if( param.isOptional)
            {
                if( ! param.defaultValue.isEmpty())
                    params[param.name] = param.defaultValue;
            }
            else
            {
                QString err("Missing mandatory param \"%1\" for transform \"%2\"!");
                _setError(Ntg::InvalidParamsError, err.arg(param.name).arg(_model.name));
                return 0;
            }
        }
    }

    foreach(const QString & param, params.keys())
    {
        if( ! _model.params.contains(param))
        {
            QString err("Unknown param \"%1\" for transform \"%2\"!");
            _setError(Ntg::InvalidParamsError, err.arg(param).arg(_model.name));
            return 0;
        }
    }

    _inputEntity = input;

    _exec(input, params);
    return true;
}


void LocalTransform::cancel()
{
    if(_status == Ntg::Running )
        _cancel();
}


Ntg::TransformStatus LocalTransform::status() const
{
    return _status;
}


double LocalTransform::progressValue () const
{
    return _progressPercent;
}


QString LocalTransform::progressText () const
{
    return _progressText;
}


Ntg::TransformError LocalTransform::error() const
{
    return _error;
}


QString LocalTransform::errorString() const
{
    return _errorString;
}


int LocalTransform::latestResultIndex() const
{
    return (_resultList.size() - 1);
}


QList<NtgEntity> LocalTransform::results(int startIndex, int toIndex) const
{
    qDebug()<<"LocalTransform::results";
    if( startIndex >= _resultList.size() || (toIndex > 0 && toIndex < startIndex))
        return QList<NtgEntity>();
    if( toIndex >= _resultList.size() || toIndex < 0)
        return _resultList.mid(startIndex);
    return _resultList.mid(startIndex, (toIndex - startIndex) + 1);
}


const NtgTransformModel & LocalTransform::model() const
{
    return _model;
}


QString LocalTransform::type() const
{
    return _model.name;
}


const NtgEntity & LocalTransform::inputEntity() const
{
    return _inputEntity;
}


QString LocalTransform::workDirPath() const
{
    return _workDirPath;
}




QHash<QString,QString> LocalTransform::configValues() const
{
    return _configValues;
}


bool LocalTransform::configValueAsBool(const QString & name, bool * value) const
{
    if( ! value ) return false;
    QString boolStr = _configValues.value(name).toLower();
    if(boolStr == "true" || boolStr == "yes" || boolStr == "1")
        *value = true;
    else if(boolStr == "false" || boolStr == "no" || boolStr == "0")
        *value = false;
    else
        return false;
    return true;
}


bool LocalTransform::configValueAsInt(const QString & name, int * value) const
{
    if( ! value ) return false;
    QString intStr = _configValues.value(name);
    bool ok;
    int valueTmp = intStr.toInt(&ok, 0);
    if( ! ok ) return false;
    *value = valueTmp;
    return true;
}


bool LocalTransform::configValueAsDouble(const QString & name, double * value) const
{
    if( ! value ) return false;
    QString doubleStr = _configValues.value(name);
    bool ok;
    double valueTmp = doubleStr.toDouble(&ok);
    if( ! ok ) return false;
    *value = valueTmp;
    return true;
}


bool LocalTransform::configValueAsString(const QString & name, QString * value) const
{
    if( ! value ) return false;
    QString str = _configValues.value(name);
    if(str.isEmpty())
        return false;
    *value = str;
    return true;
}


void LocalTransform::_setStatus(Ntg::TransformStatus status)
{
    if(status == _status)
        return;
    _status = status;
    emit statusChanged(status);

}


void LocalTransform::_setError(Ntg::TransformError error, QString msg)
{
    _error = error;
    _errorString = msg;
    _setStatus(Ntg::Failed);
}


void LocalTransform::_setProgress(double progressPercent, QString progressStr)
{
    if( progressPercent > 100.0)
        _progressPercent = 100.0;
    else
        _progressPercent = progressPercent;
    if( ! progressStr.isNull())
        _progressText = progressStr;
    emit progressChanged(_progressPercent, _progressText);

}


void LocalTransform::_addResult(NtgEntity entity)
{
     qDebug()<<"LocalTransform::_addResult";
    if( ! _model.outputTypes.contains(entity.type) )
    {
        QString err("Invalid result type \"%1\" for transform \"%2\"!");
        _setError(Ntg::InvalidResultsError, err.arg(entity.type).arg(_model.name));
        return;
    }
    _resultList.append(entity);
    emit resultReadyAt(_resultList.size()-2, _resultList.size()-1);

}

void LocalTransform::_addResults(QList<NtgEntity> entities)
{
    if(entities.isEmpty())
        return;

    foreach(const NtgEntity & e , entities)
    {
        if( ! _model.outputTypes.contains(e.type) )
        {
            QString err("Invalid result type \"%1\" for transform \"%2\"!");
            _setError(Ntg::InvalidResultsError, err.arg(e.type).arg(_model.name));
            return;
        }
    }
    _resultList.append(entities);_id++;
    emit resultReadyAt(_resultList.size() - entities.size(), _resultList.size()-1);

}


void LocalTransform::_reset()
{
    _status = Ntg::NotStarted;
    _error = Ntg::NoError;
    _errorString.clear();
    _progressPercent = 0;
    _progressText.clear();
    _resultList.clear();
    _inputEntity = NtgEntity();
}

/****************************************************************************
**  class LocalTransformManager
****************************************************************************/

//TODO transform into variables these paths
QString _xmlDirPath="./transforms";
QString _pluginPath="./plugins";

LocalTransformManager::LocalTransformManager()
{
    setObjectName("TLManager");
    _loadPlugins(_pluginPath);
    _createModelsFromXml(_xmlDirPath);

}


LocalTransformManager::~LocalTransformManager()
{
    _localTransformInfos.clear();
    _currentLocalTransforms.clear();
    _localPlugins.clear();
}


QVariantMap LocalTransformManager::transformList() const
{
    QVariantMap modelsVariantMap;

    foreach(const LocalTransformInfo _localTransform, _localTransformInfos)
    {
        modelsVariantMap.unite(transformToMap(_localTransform.model));
    }
    return modelsVariantMap;
}


LocalTransform* LocalTransformManager::createTransformFromType(QString type)
{
    if( ! _localTransformInfos.contains(type))
        return NULL;

    LocalTransformInfo info = _localTransformInfos.value(type);
    LocalTransform* t = info.plugin->createTransform();
    if( ! t ) return NULL;
    t->_model = info.model;
    t->_workDirPath = info.workingDirPath;
    t->_configValues = info.values;

    connect(t, SIGNAL(progressChanged(double,QString)), this, SLOT(_newProgress(double,QString)));
    connect(t, SIGNAL(resultReadyAt(int,int)), this, SLOT(_newResults(int,int)));
    connect(t, SIGNAL(statusChanged(Ntg::TransformStatus)), this, SLOT(_newStatus(Ntg::TransformStatus)));

    return t;
}

int LocalTransformManager::createTransform(QString transformType)
{
    if( ! _localTransformInfos.contains(transformType))
        return 0;
    LocalTransform* t = createTransformFromType(transformType);
    if( ! t)
        return 0;

    _currentLocalTransforms.insert(t->id(), t);
    qDebug() << "LocalManagerTransform::createTransform " << t->id();
    return t->id();
}

bool LocalTransformManager::execTransform(int transformId, const NtgEntity & input, HashStringString params)
{
    qDebug() << "LocalManagerTransform::execTransform" << transformId <<"  " << isTransformId(transformId);


    if (isTransformId(transformId))
    {
        return getTransform(transformId)->exec(input,params);
    }
    return false;
}

bool LocalTransformManager::destroyTransform(int transformId)
{
    if( ! _currentLocalTransforms.contains(transformId))
        return false;
    delete getTransform(transformId);
    return true;
}

bool LocalTransformManager::cancelTransform(int transformId)
{
    if( ! _currentLocalTransforms.contains(transformId))
        return false;
    getTransform(transformId)->cancel();
    return true;
}

LocalTransform * LocalTransformManager::getTransform(int transformId)
{
    return _currentLocalTransforms.value(transformId);
}

bool LocalTransformManager::isTransformType(QString type)
{
    if( ! _localTransformInfos.contains(type))
        return false;
    return true;
}

bool LocalTransformManager::isTransformId(int transformId)
{
    if (_currentLocalTransforms.contains(transformId))
        return true;
    else
        return false;
}
QList<NtgTransformModel> LocalTransformManager::models()
{
    QList<NtgTransformModel> models;
    foreach(const LocalTransformInfo & info, _localTransformInfos)
        models.append(info.model);
    return models;
}


void LocalTransformManager::_newStatus(Ntg::TransformStatus status)
{
    QStringList labels;
    labels << "NotStarted" << "Running" << "Finished" << "Failed";
    qDebug() << "Status: " << labels.at(status);

    LocalTransform* t = qobject_cast<LocalTransform*>(sender());
    if( ! t) qFatal("WTF");
    QStringList errors;
    errors << "NoError" << "CustomError" << "CancelledError"<< "InvalidInputError" << "InvalidParamsError" << "InvalidResultsError";
    if(t->status() == Ntg::Failed)
        qDebug() << "Error: " << errors.at(t->error()) << t->errorString();
}


void LocalTransformManager::_newProgress(double progressPercent, const QString & progressText)
{
    LocalTransform* t = qobject_cast<LocalTransform*>(sender());
    if( ! t) qFatal("WTF");
    qDebug("Progress : %.2lf%% %s", progressPercent, qPrintable(progressText));
}


void LocalTransformManager::_newResults(int fromIndex, int toIndex)
{
    LocalTransform* t = qobject_cast<LocalTransform*>(sender());
    if( ! t) qFatal("WTF");

    qDebug("*** Results from %d to %d", fromIndex, toIndex);

    foreach(const NtgEntity & e, t->results(fromIndex,toIndex))
    {
        qDebug() << "Result:" << e.type;
        qDebug() << "       " << e.values;
    }
}


void LocalTransformManager::_loadPlugins(QString path)
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
                LocalTransformPlugin * transformPlugin = qobject_cast<LocalTransformPlugin *>(plugin);
                if (transformPlugin)
                {
                    _localPlugins.insert(transformPlugin->transformType(), transformPlugin);
                    qDebug("Loaded plugin: %s", qPrintable(transformPlugin->transformType()));
                }
                else
                {
                    qWarning("Plugin \"%s\" doesn't implement LocalTransformPlugin interface",
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

QVariantMap LocalTransformManager::infoOfTransform(int transformId)
{
    qDebug() << "localTransformManager::infoOfTransform transformId :"<<transformId ;
    QVariantMap infos;
    LocalTransform * t = getTransform(transformId);
    if( ! t)
    {
        //_reportError(sessionId, "No transform with that id");
        return infos;
    }

    Ntg::TransformStatus status = t->status();
    infos.insert("status", Ntg::transformStatusName(status));
    if( status == Ntg::Failed )
    {
        infos.insert("error", Ntg::transformErrorName(t->error()));
        infos.insert("error msg", t->errorString());
    }
    infos.insert("latest result index", t->latestResultIndex());
    infos.insert("progress value", t->progressValue());
    infos.insert("progress msg", t->progressText());

    return infos;
}


#define XML_FAIL(format,...) {qCritical("XML Parse Error: " format, ##__VA_ARGS__);continue;}
#define XML_WARN(format,...) {qWarning( "XML Parse Warning: " format, ##__VA_ARGS__);}


QVariantList LocalTransformManager::resultsOfTransform(int transformId, int startIndex, int toIndex)
{
    qDebug() << "localTransformManager::resultsOfTransform";

    QVariantList results;


    if(isTransformId(transformId))
    {

        LocalTransform * t= getTransform(transformId);

        if( ! t )
        {
            //_reportError(sessionId, "No transform with that id");
            return results;
        }

        foreach(const NtgEntity & entity, t->results(startIndex, toIndex))
            results.append(_entityToXmlRpcStruct(entity));
        return results;
    }
       return results;
    
}

QVariantMap LocalTransformManager::_entityToXmlRpcStruct(const NtgEntity & entity)
{
  QVariantMap structMap;
  structMap.insert("entity_type", entity.type);
  structMap.unite(_stringHashToXmlRpcStruct(entity.values));
  return structMap;
}

QVariantMap LocalTransformManager::_stringHashToXmlRpcStruct(const QHash<QString,QString> & hash)
{
  QVariantMap structMap;
  foreach(const QString & key, hash.keys())
    structMap.insert(key, hash.value(key));
  return structMap;
}

void LocalTransformManager::_createModelsFromXml(QString path)
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


            QFile xmlConf(fileInfo.absoluteFilePath());
            if( ! xmlConf.open(QIODevice::ReadOnly))
                XML_FAIL("Could not open conf file %s", qPrintable(fileInfo.absoluteFilePath()));

            QDomDocument doc;
            doc.setContent(&xmlConf);
            if( doc.doctype().name() != "TransformSchema")
                XML_FAIL("Wrong DOCTYPE, is %s",qPrintable(doc.doctype().name()));

            NtgTransformModel model;
            model.category="Local";
            QDomElement rootElem = doc.documentElement();
            if(rootElem.tagName() != "transform")
                XML_FAIL("Root node should be <transform>, but is <%s>", qPrintable(rootElem.tagName()));

            model.name = rootElem.attribute("name")+" local";
            if(model.name.isEmpty())
                XML_FAIL("Empty name in <transform>");

            model.longName = rootElem.attribute("longName")+" local";
            if(model.longName.isEmpty())
              XML_FAIL("Empty long name in <transform>");

            model.pluginName = rootElem.attribute("type");
            LocalTransformPlugin * plugin = _localPlugins.value(model.pluginName);
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

            if( _localTransformInfos.contains(model.name))
            {
                XML_FAIL("Conflicting transform name '%s', used in both: \"%s\" and \"%s\"",
                         qPrintable(model.name), qPrintable(fileInfo.absolutePath()),
                         qPrintable(_localTransformInfos[model.name].workingDirPath));
            }

            model.category = "Local";
            _localTransformInfos[model.name].plugin = plugin;
            _localTransformInfos[model.name].model = model;
            _localTransformInfos[model.name].workingDirPath = fileInfo.absolutePath();
            _localTransformInfos[model.name].values = values;
        }
    }
}
//TODO Optimize class below
//Used to compile local plugin
///****************************************************************************
//**  class LocalSafeProcess
//****************************************************************************/
//
//LocalSafeProcess::LocalSafeProcess(QObject * parent)
//  : QProcess(parent)
//{
//}
//
//
//void LocalSafeProcess::dropPrivileges(bool drop)
//{
//  _dropPrivileges = drop;
//}
//
//
//void LocalSafeProcess::setupChildProcess()
//{
//  LocalDaemon * daemon = qobject_cast<LocalDaemon*>(qApp);
//  if( _dropPrivileges && ! daemon->dropPrivilegesPermanently())
//  {
//    emit finished(1, QProcess::CrashExit);
//    ::exit(1);
//   }
//}


