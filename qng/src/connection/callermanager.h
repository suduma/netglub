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

#ifndef CALLERMANAGER_H
#define CALLERMANAGER_H

#include <QObject>

#include "maiaXmlRpcClient.h"
#include "ntgXmlRpcCall.h"
#include "ntgImageHandler.h"
#include "ntgTypes.h"
#include "connectionwidgets.h"
//#include "localtransformmanager.h"

class ConnectionWidget;
class ConnectionDialog;
class FailedConectionDialog;

class CallerManager : public QObject {
    Q_OBJECT
public:
    static CallerManager * create(const QString & URL, const QString & serial);
    static CallerManager * get();
    //CallerManager(const QString & URL, const QString & serial);
    virtual ~CallerManager();

    QString connectToMaster();

    void reloadConnexion(const QString & URL, const QString & serial);
    bool retryConnexion();
    QHash<QString,NtgEntityModel> getAllEntities();
    QStringList getAvailableEntities();
    QHash<QString,NtgTransformModel> getAvailableTransforms();
    int getNewGraphId();
    bool deleteGraph(int graphId);
    void setGraphEnableCache(int graphId, bool enableCache);
    void clearCache(); // WARNING : clear all the cache, only for Debug version
    QString lastError();
    QString closeSession();

    bool isOffLineWork();
    void setOffLineWork(bool offLineWork);


    // manage transforms
//    int createTransform(int graphId, QString type);
//    bool destroyTransform(int graphId, int transformId);
//    bool execTransform(int graphId, int transformId, QVariantMap inputEntity, QVariantMap inputParams);
//    bool cancelTransform(int graphId, int transformId);
//    QVariantMap infoOfTransform(int graphId, int transformId);
//    QVariantList resultsOfTransform(int graphId, int transformId, int startIndex, int toIndex);
//
//    QString getConnectionError();
    int createTransform(int graphId, QString type);
    bool destroyTransform(int graphId, int transformId);
    bool execTransform(int graphId, int transformId, QVariantMap inputEntity, QVariantMap inputParams);
    bool cancelTransform(int graphId, int transformId);
    QVariantMap infoOfTransform(int graphId, int transformId);
    QVariantList resultsOfTransform(int graphId, int transformId, int startIndex, int toIndex);
    //TODO : Optimize LocalTransforms
    //Function disabled
    //QHash<QString,NtgTransformModel> getLocalAvailableTransforms();
    QHash<QString,NtgTransformModel> getMasterAvailableTransforms();
   // QHash<QString,NtgTransformModel> getAvailableTransforms();
    QHash<QString,QString> _xmlRpcStructToStringHash(const QVariantMap & structMap);
    NtgEntity _xmlRpcStructToEntity(QVariantMap structMap);

    QString getConnectionError();

protected:
    QString _serial;
    QString _URL;
    QString _sessionId;

private:
    CallerManager(const QString & URL, const QString & serial);
    static CallerManager * _callerManager;
    MaiaXmlRpcClient * _rpc;
    NtgXmlRpcCall * _caller;
    QString _connectionError;

    //TODO : Optimize LocalTransformManager
    //LocalTransform Manager Disabled
    //LocalTransformManager * _localTransformManager;

    bool _offLineWork;
    FailedConectionDialog * _failed;


};
#endif // CALLERMANAGER_H
