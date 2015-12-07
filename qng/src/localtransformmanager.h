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

#ifndef LOCALTRANSFORMMANAGER_H
#define LOCALTRANSFORMMANAGER_H

#include <QObject>
#include <QStringList>
#include <QSharedPointer>
#include "ntgTypes.h"
#include <QProcess>
#include <QHash>
#include <QStringList>
#include <QMutex>


class LocalTransformPlugin;
class LocalTransformManager;

/****************************************************************************
**  class LocalTransform
****************************************************************************/

class LocalTransform: public QObject
{
    friend class LocalTransformManager;
    Q_OBJECT

public:
    LocalTransform();
    virtual ~LocalTransform();

public slots:

    bool exec(const NtgEntity & input, HashStringString params);
    void cancel();
    int id() const;

    Ntg::TransformStatus status() const;

    double progressValue () const;
    QString progressText () const;

    Ntg::TransformError error() const;
    QString errorString() const;

    int latestResultIndex() const;
    QList<NtgEntity> results(int startIndex, int toIndex = -1) const;

    const NtgTransformModel & model() const;
    QString type() const;
    const NtgEntity & inputEntity() const;

public:
    QString workDirPath() const;

    QHash<QString,QString> configValues() const;
    bool configValueAsBool(const QString & name, bool * value) const;
    // numbers beginning with '0x' are read as hex, '0' as octal, else as decimal.
    bool configValueAsInt(const QString & name, int * value) const;
    bool configValueAsDouble(const QString & name, double * value) const;
    bool configValueAsString(const QString & name, QString * value) const;


signals:

    void statusChanged(Ntg::TransformStatus status);
    void progressChanged(double progressPercent, const QString & progressText);
    void resultReadyAt(int fromIndex, int toIndex);

public:

    virtual void _exec(const NtgEntity & input, QHash<QString, QString> params) = 0;
    virtual void _cancel() = 0;

    void _setStatus(Ntg::TransformStatus status);
    void _setError(Ntg::TransformError error, QString msg);
    void _setProgressMax(int max);
    void _setProgress(double progress, QString progressStr = QString());
    void _addResult(NtgEntity entity);
    void _addResults(QList<NtgEntity> entities);


private:
    void _reset();


public:
    NtgTransformModel _model;
    QString _workDirPath;
    QHash<QString,QString> _configValues;
    int _id;

private:
    Ntg::TransformStatus _status;
    Ntg::TransformError _error;
    QString _errorString;
    double _progressPercent;
    QString _progressText;
    QList<NtgEntity> _resultList;
    NtgEntity _inputEntity;

    static quint32 _instanceCounter;
    static int _idCount;

};

/****************************************************************************
**  class LocalTransformManager
****************************************************************************/

struct LocalTransformInfo
{
    LocalTransformPlugin * plugin;
    QString workingDirPath;
    NtgTransformModel model;
    QHash<QString, QString> values;
};


class LocalTransformManager : public QObject
{
    Q_OBJECT
public:
    LocalTransformManager();
    virtual ~LocalTransformManager();

    QVariantMap transformList() const;
    LocalTransform* createTransformFromType(QString type);

public slots:
    int createTransform(QString transformType);
    bool execTransform(int transformId, const NtgEntity & input, HashStringString params);
    bool destroyTransform(int transformId);
    bool cancelTransform(int transformId);
    //  QStringList currentTransforms(QString transformType = "");
    QList<NtgTransformModel> models();
    QVariantMap infoOfTransform(int transformId);
    QVariantList resultsOfTransform(int transformId, int startIndex, int toIndex);
    LocalTransform * getTransform(int transformId);
    bool isTransformId(int transformId);
    bool isTransformType(QString type);


protected slots:
    void _newStatus(Ntg::TransformStatus status);
    void _newProgress(double progressPercent, const QString & progressText);
    void _newResults(int fromIndex, int toIndex);

protected:
    void _loadPlugins(QString path);
    void _createModelsFromXml(QString path);

protected:
    QHash<QString, LocalTransformInfo> _localTransformInfos;
    QHash<int, LocalTransform*> _currentLocalTransforms;
    QHash<QString, LocalTransformPlugin*> _localPlugins;

private:
    QVariantMap _entityToXmlRpcStruct(const NtgEntity & entity);
    QVariantMap _stringHashToXmlRpcStruct(const QHash<QString,QString> & hash);

};
//TODO : Optimize local plugins
//Declaration used to compile local plugin
///****************************************************************************
//**  class LocalSafeProcess
//****************************************************************************/
//
//class LocalSafeProcess : public QProcess
//{
//    Q_OBJECT
//public:
//    LocalSafeProcess(QObject *parent = 0);
//
//    void dropPrivileges(bool drop);
//
//protected:
//    virtual void setupChildProcess();
//
//    bool _dropPrivileges;
//};

#endif // LOCALTRANSFORMMANAGER_H
