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

#ifndef NTGSESSIONAUTHENTICATOR_H
#define NTGSESSIONAUTHENTICATOR_H

#include <QObject>
#include <QHash>
#include <QTimer>
#include <QDateTime>
#include <QSignalMapper>
#include <QHostAddress>
#include <QMutex>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QMultiMap>

#include "ntgTransformProxy.h"
#include "ntgTypes.h"

namespace Ntg {
  enum AuthLevel
  {
    AUTH_NONE = 0, AUTH_USER = 1, AUTH_ADMIN = 2
  };

  AuthLevel stringToAuthLevel(const QString & str);
}

struct NtgGraph
{
    NtgGraph(){};

    int graphId;
    bool cacheEnable;
    QList<int> transformList;
};


class NtgUser;


/****************************************************************************
**  class NtgGroup
****************************************************************************/
class NtgGroup : public QObject
{
    Q_OBJECT
public:
    NtgGroup();
    virtual ~NtgGroup();
    QHash<QString,NtgTransformModel *> getTransformHash();
    QHash<QString,NtgUser *> getGroupUserHash();

    QString groupName();
    QString description();

    void setProperties(QString groupName, QString description);

    void addTransform(NtgTransformModel * transform);
    void addUser(NtgUser * user);

protected:
    QHash<QString,NtgTransformModel *> _transformHash;

    QHash<QString,NtgUser *> _groupUserHash;
    QString _groupName;
    QString _description;
};


/****************************************************************************
**  class NtgUser
****************************************************************************/

class NtgUser : public QObject
{
    Q_OBJECT
public:
    NtgUser(QSqlDatabase * db);
    virtual ~NtgUser();
    QHash<QString,NtgTransformModel *> getUserTransformHash();
    QHash<QString,NtgGroup *> getUserGroupHash();
    bool addGroup(NtgGroup * group);


    // graph management
    int createGraph();
    bool addGraphId(int graphId);
    void addTransformToGraph(int graphId, int transformId);
    bool removeTransform(int graphId, int transformId);
    bool deleteGraph(int graphId);
    bool transformExistInGraph(int graphId, int transformId);
    bool hasGraph(int graphId);
    void setGrahEnableCache(int graphId,bool cacheEnable);
    bool getGrahEnableCache(int graphId);
    bool reachedLimitTransformsForCommunity(QString ip);

    void setProperties(QString userName,
                       Ntg::AuthLevel authLevel,
                       bool enable,
                       QString password,
                       QString serial,
                       int nextGraphId,
                       QString firstName,
                       QString lastName,
                       QString emailAddress,
                       QDateTime registrationDateTime,
                       QDateTime lastModificationDateTime);

    QString userName();
    static int limitOftransforms;
protected:
    QString _userName;
    Ntg::AuthLevel _authLevel;
    bool _enable;
    QString _password;
    QString _serial;
    int _nextGraphId;
    QString _firstName;
    QString _lastName;
    QString _emailAddress;
    QDateTime _registrationDateTime;
    QDateTime _lastModificationDateTime;


    QSqlDatabase * _db;
    QSqlQuery * _updateUserGraphIdQuery;
    QMap<int,NtgGraph* > _graphMap;
//    QMultiMap<int,int> graphIdMultiMap  ;
//    QList<int> availableGraphList;

    QHash<QString,NtgGroup *> _userGroupHash;


};


/****************************************************************************
**  class NtgSession
****************************************************************************/
class NtgSession : public QObject
{
  Q_OBJECT
  public:
    NtgSession();
    NtgSession(QString id_, QString userName_, Ntg::AuthLevel level_);
    virtual ~NtgSession();

    bool isValid() const {return ! id.isEmpty();}

  public slots:
    void _sessionTimeout(void);

  signals:
    void timeout(const QString & id);

  public:
    QString id;
    QString userName;
    QString serial;
    Ntg::AuthLevel level;
    QHostAddress ip;
    QTimer sessionTimeout;
    QString lastError;
    QDateTime registrationDateTime;
    QDateTime lastModificationDateTime;
};


class NtgTransformManager;

/****************************************************************************
**  class NtgSessionManager
****************************************************************************/
class NtgSessionManager : public QObject
{
  Q_OBJECT
  public:
    NtgSessionManager(QSqlDatabase * db, NtgTransformManager * transformManager, int sessionDuration = 600 /*seconds*/);
    virtual ~NtgSessionManager();

    NtgSession * createSession(const QString & sessionId, const QString & serial, const QHostAddress & ip);
    bool activateSession(const QString & sessionId, const QString & serial);
    bool updateSession(const QString & sessionId, const QString & serial);
    void closeSession(const QString & sessionId);
    NtgSession * getSession(const QString & sessionId) const;
    NtgUser* getUser(const QString & userName) const;
    bool updateOrCreateSessionSql(const NtgSession * session);
    QHash<QString,NtgTransformModel*> getUserTransformsModels(const QString & sessionId);
    QHash<QString,NtgEntityModel *> getAllEntitiesModels();
    QStringList getUserEntitiesNames(const QString & sessionId);
    bool isAvailableTransformForUser(const QString & sessionId, const QString & name);

    int sessionDuration() const; // in seconds

  protected slots:
    void _sessionTimeout(const QString & id);

  protected:
    int _sessionDuration;
    QHash<QString,NtgSession*> _sessionHash;
    QHash<QString,NtgUser*> _userHash;
    QHash<QString,NtgGroup*> _groupHash;

    QSignalMapper _timeoutMapper;
    QSqlDatabase * _db;
    QSqlQuery * _getUserQuery;
    NtgTransformManager * _transformManager;

    mutable QMutex _mutex;
};

#endif // NTGSESSIONAUTHENTICATOR_H
