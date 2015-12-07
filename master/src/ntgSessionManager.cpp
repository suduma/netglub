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

#include "ntgSessionManager.h"
#include "ntgTransformManager.h"

#include <QCryptographicHash>
#include <QCoreApplication>
#include <QVariant>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlError>
#include <QTime>
#include <QCoreApplication>

Ntg::AuthLevel Ntg::stringToAuthLevel(const QString & str)
{
  if( str == QLatin1String("NONE"))
    return Ntg::AUTH_NONE;
  else if( str == QLatin1String("USER"))
    return Ntg::AUTH_USER;
  else if( str == QLatin1String("ADMIN"))
    return Ntg::AUTH_ADMIN;
  else
    return Ntg::AUTH_NONE;
}


/****************************************************************************
**  class NtgUser
****************************************************************************/

int NtgUser::limitOftransforms = 50;


NtgUser::NtgUser(QSqlDatabase * db):_db(db){}

NtgUser::~NtgUser()
{
//  qDeleteAll(_userGraphHash);
}

bool NtgUser::addGroup(NtgGroup * group)
{
  if (_userGroupHash.contains(group->groupName()))
  {
    qWarning() << "trying to create 2 groups with the same name ID : " << group->groupName();
    return false;
  }
  _userGroupHash.insert(group->groupName(),group);
  return true;
}

QHash<QString,NtgGroup *> NtgUser::getUserGroupHash()
{
  return _userGroupHash;
}

void NtgUser::setProperties( QString userName,
                             Ntg::AuthLevel authLevel,
                             bool enable,
                             QString password,
                             QString serial,
                             int nextGraphId,
                             QString firstName,
                             QString lastName,
                             QString emailAddress,
                             QDateTime registrationDateTime,
                             QDateTime lastModificationDateTime)
{
  _userName = userName;
  _authLevel = authLevel;
  _enable = enable;
  _password = password;
  _serial = serial;
  _nextGraphId = nextGraphId;
  _firstName = firstName;
  _lastName = lastName;
  _emailAddress = emailAddress;
  _registrationDateTime = registrationDateTime;
  _lastModificationDateTime = lastModificationDateTime;
}

QString NtgUser::userName()
{
  return _userName;
}



//TODO : how to get a user
QHash<QString,NtgTransformModel*> NtgUser::getUserTransformHash()
    {
      QHash<QString,NtgTransformModel*> userTransformModelList;

       foreach (NtgGroup *  group, _userGroupHash)
       {
         userTransformModelList.unite(group->getTransformHash());
       }

        return userTransformModelList;
    }


// graph management
int NtgUser::createGraph()
{
    QSqlQuery * selectCurrentGraphId = new QSqlQuery(*_db);
    if( ! selectCurrentGraphId->prepare("SELECT `next_graph_id` FROM `users` WHERE `username` = :username"))
    {
      qCritical("Query invalide : %s", qPrintable(selectCurrentGraphId->lastError().text()));
      delete selectCurrentGraphId;
      return 0;
    }

    selectCurrentGraphId->bindValue(":username", _userName);
    if( ! selectCurrentGraphId->exec() )
    {
      qWarning( "NtgUser::SetUserGraphId : selectCurrentGraphId %s", qPrintable(selectCurrentGraphId->lastError().text()));
      delete selectCurrentGraphId;
      return 0;
    }
    selectCurrentGraphId->next();
    int currentGraphId = selectCurrentGraphId->value(selectCurrentGraphId->record().indexOf("next_graph_id")).toInt();

    delete selectCurrentGraphId;

    QSqlQuery * updateUserGraphIdQuery = new QSqlQuery(*_db);
    if( ! updateUserGraphIdQuery->prepare("UPDATE `users` SET `next_graph_id` =`next_graph_id`+1 WHERE `username` = :username"))
    {
      qCritical("Query invalide : %s", qPrintable(updateUserGraphIdQuery->lastError().text()));
      delete updateUserGraphIdQuery;
      return 0;
    }
    updateUserGraphIdQuery->bindValue(":username", _userName);
    if( ! updateUserGraphIdQuery->exec() )
    {
      qWarning( "NtgUser::SetUserGraphId : updateUserGraphIdQuery %s", qPrintable(updateUserGraphIdQuery->lastError().text()));
      delete updateUserGraphIdQuery;
      return 0;
    }

    NtgGraph * graph = new NtgGraph();
    graph->cacheEnable = false;
    graph->graphId = currentGraphId;
    _graphMap.insert(currentGraphId,graph);

    delete updateUserGraphIdQuery;

    return currentGraphId;
}

bool NtgUser::addGraphId(int graphId)
{
    QSqlQuery * selectCurrentGraphId = new QSqlQuery(*_db);
    if( ! selectCurrentGraphId->prepare("SELECT `next_graph_id` FROM `users` WHERE `username` = :username"))
    {
      qCritical("Query invalide : %s", qPrintable(selectCurrentGraphId->lastError().text()));
      delete selectCurrentGraphId;
      return 0;
    }

    selectCurrentGraphId->bindValue(":username", _userName);
    if( ! selectCurrentGraphId->exec() )
    {
      qWarning( "NtgUser::SetUserGraphId : selectCurrentGraphId %s", qPrintable(selectCurrentGraphId->lastError().text()));
      delete selectCurrentGraphId;
      return 0;
    }
    selectCurrentGraphId->next();
    int currentGraphId = selectCurrentGraphId->value(selectCurrentGraphId->record().indexOf("next_graph_id")).toInt();

    delete selectCurrentGraphId;

    if (graphId < currentGraphId)
    {
        NtgGraph * graph = new NtgGraph();
        graph->cacheEnable = false;
        graph->graphId = graphId;
        _graphMap.insert(graphId,graph);
        return true;
    }
    return false;
}

void NtgUser::addTransformToGraph(int graphId, int transformId)
{
    _graphMap.value(graphId)->transformList.append(transformId);

  //graphIdMultiMap.insert(graphId,transformId);
}

bool NtgUser::removeTransform(int graphId, int transformId)
{
    return _graphMap.value(graphId)->transformList.removeOne(transformId);
    //return graphIdMultiMap.remove(graphId, transformId);
}

bool NtgUser::deleteGraph(int graphId)
{
    return _graphMap.remove(graphId);
//  if (availableGraphList.removeAll(graphId))
//  {
//    graphIdMultiMap.remove(graphId);
//    return true;
//  }
//  else
//    return false;
}

bool NtgUser::transformExistInGraph(int graphId, int transformId)
{
    return _graphMap.value(graphId)->transformList.contains(transformId);
//  return graphIdMultiMap.contains(graphId, transformId);
}

bool NtgUser::hasGraph(int graphId)
{
    return _graphMap.contains(graphId);
//  return availableGraphList.contains(graphId);
}

void NtgUser::setGrahEnableCache(int graphId,bool cacheEnable)
{
    _graphMap.value(graphId)->cacheEnable=cacheEnable;
}

bool NtgUser::getGrahEnableCache(int graphId)
{
    if (!hasGraph(graphId))
        return false;

    return _graphMap.value(graphId)->cacheEnable;
}


bool NtgUser::reachedLimitTransformsForCommunity(QString ip)
{
    if (_userGroupHash.count() > 1 || !_userGroupHash.contains("community"))
    {
        return false;
    }

    QSqlQuery * selectCurrentUserLimit = new QSqlQuery(*_db);
    if( ! selectCurrentUserLimit->prepare("SELECT * FROM `transforms_limit` WHERE `serial` = :serial AND `ip_address` = :ipAdress"))
    {
      qCritical("Query invalide : %s", qPrintable(selectCurrentUserLimit->lastError().text()));
      delete selectCurrentUserLimit;
      return true;
    }

    selectCurrentUserLimit->bindValue(":serial", _serial);
    selectCurrentUserLimit->bindValue(":ipAdress", ip);
    if( ! selectCurrentUserLimit->exec() )
    {
      qWarning( "NtgUser::reachedLimitTransformsForCommunity : selectCurrentUserLimit %s", qPrintable(selectCurrentUserLimit->lastError().text()));
      delete selectCurrentUserLimit;
      return true;
    }
    if (selectCurrentUserLimit->size() == 0)
    {
        delete selectCurrentUserLimit;
        QSqlQuery * insertNewSerialIp = new QSqlQuery(*_db);
        if( ! insertNewSerialIp->prepare("INSERT INTO `netglub`.`transforms_limit` (`serial` ,`ip_address` ,`date` ,`transforms_number`)VALUES (:serial, :ip, CURRENT_DATE(), '1');"))
        {
          qCritical("Query invalide : %s", qPrintable(insertNewSerialIp->lastError().text()));
          delete insertNewSerialIp;
          return true;
        }

        insertNewSerialIp->bindValue(":serial", _serial);
        insertNewSerialIp->bindValue(":ip", ip);
        if( ! insertNewSerialIp->exec() )
        {
          qWarning( "NtgUser::reachedLimitTransformsForCommunity : insertNewSerialIp %s", qPrintable(insertNewSerialIp->lastError().text()));
          delete insertNewSerialIp;
          return true;
        }
        delete insertNewSerialIp;
        return false;
    }
    else
    {
        selectCurrentUserLimit->next();
        QDate date = selectCurrentUserLimit->value(selectCurrentUserLimit->record().indexOf("date")).toDate();
        int index = selectCurrentUserLimit->value(selectCurrentUserLimit->record().indexOf("transforms_number")).toInt();
        delete selectCurrentUserLimit;

        QSqlQuery * updateSerialIp = new QSqlQuery(*_db);
        if( ! updateSerialIp->prepare("UPDATE `netglub`.`transforms_limit` SET `date` = :date , `transforms_number` = :index WHERE `serial` = :serial AND `ip_address` = :ip;"))
        {
          qCritical("Query invalide : %s", qPrintable(updateSerialIp->lastError().text()));
          delete updateSerialIp;
          return true;
        }
        updateSerialIp->bindValue(":serial", _serial);
        updateSerialIp->bindValue(":ip", ip);


        if (QDate::currentDate() > date)
        {
            date = QDate::currentDate();
            index = 1;
        }
        else if (QDate::currentDate() == date)
        {
            if (index >= limitOftransforms)
                return true;
            index++;
        }
        else
            return true;

        updateSerialIp->bindValue(":date", date);
        updateSerialIp->bindValue(":index", index);
        if( ! updateSerialIp->exec() )
        {
          qWarning( "NtgUser::reachedLimitTransformsForCommunity : updateSerialIp %s", qPrintable(updateSerialIp->lastError().text()));
          delete updateSerialIp;
          return true;
        }
        delete updateSerialIp;
        return false;
    }

//    selectCurrentUserLimit->next();
//    qDebug() << selectCurrentUserLimit->value(0);
//    int currentGraphId = selectCurrentGraphId->value(selectCurrentGraphId->record().indexOf("next_graph_id")).toInt();

//    delete selectCurrentUserLimit;
    return true;
}


/****************************************************************************
**  class NtgGroup
****************************************************************************/

NtgGroup::NtgGroup(){}

NtgGroup::~NtgGroup(){}

QHash<QString,NtgTransformModel *> NtgGroup::getTransformHash()
{
  return _transformHash;
}

QHash<QString,NtgUser *> NtgGroup::getGroupUserHash()
{
  return _groupUserHash;
}

QString NtgGroup::groupName()
{
  return _groupName;
}

QString NtgGroup::description()
{
  return _description;
}

void NtgGroup::setProperties(QString groupName, QString description)
{
  _groupName = groupName;
  _description = description;
}

void NtgGroup::addTransform(NtgTransformModel * transform)
{
  _transformHash.insert(transform->name,transform);
}

void NtgGroup::addUser(NtgUser * user)
{
  _groupUserHash.insert(user->userName(),user);
}

/****************************************************************************
**  class NtgSession
****************************************************************************/

NtgSession::NtgSession()
    : level(Ntg::AUTH_NONE)
{
    moveToThread(QCoreApplication::instance()->thread());
    sessionTimeout.moveToThread(QCoreApplication::instance()->thread());
    connect(&(sessionTimeout), SIGNAL(timeout()), this, SLOT(_sessionTimeout()));
}

NtgSession::NtgSession(QString id_, QString userName_, Ntg::AuthLevel level_)
    : id(id_), userName(userName_), level(level_)
{
    moveToThread(QCoreApplication::instance()->thread());
    sessionTimeout.moveToThread(QCoreApplication::instance()->thread());
    connect(&(sessionTimeout), SIGNAL(timeout()), this, SLOT(_sessionTimeout()));
}

NtgSession::~NtgSession()
{
    qDebug() << "NtgSession::~NtgSession()";
  sessionTimeout.stop();
}

void NtgSession::_sessionTimeout()
{
  emit timeout(id);
}


/****************************************************************************
**  class NtgSessionManager
****************************************************************************/

NtgSessionManager::NtgSessionManager(QSqlDatabase * db, NtgTransformManager * transformManager, int sessionDuration)
  : _sessionDuration(sessionDuration)
  , _db(db)
  , _getUserQuery(NULL)
  , _transformManager(transformManager)
  , _mutex(QMutex::Recursive)
{
  _getUserQuery = new QSqlQuery(*_db);
  if( ! _getUserQuery->prepare("SELECT `username`,`auth_level`, `enable`, `serial` FROM `users` WHERE `serial` = :serial AND `enable` = TRUE"))
  {
    qCritical("Query invalide : %s", qPrintable(_getUserQuery->lastError().text()));
  }



  QSqlQuery getAllGroupQuery(*_db);

  if( ! getAllGroupQuery.prepare("SELECT G.*, J.transform_name FROM `groups` AS G LEFT JOIN `join_groups_transforms` AS J ON G.`groupname` = J.`groupname`"))
  {
    qCritical("Query invalide : %s", qPrintable(getAllGroupQuery.lastError().text()));
  }

  if( ! getAllGroupQuery.exec() )
  {
    qCritical("NtgSessionManager::NtgSessionManager : getAllGroupQuery %s", qPrintable(getAllGroupQuery.lastError().text()));
  }
  NtgGroup * group = NULL;
  QString groupName;
  QString groupDescription;
  QString transformName;  QHash<QString,NtgTransformModel*> ntgGetUserTransformList();
  NtgTransformModel * transformModel = NULL;



  while (getAllGroupQuery.next())
  {
    groupName = getAllGroupQuery.value(getAllGroupQuery.record().indexOf("groupname")).toString();
    group = _groupHash.value(groupName);
    if (!group)
    {
      group = new NtgGroup();
      group->setProperties(getAllGroupQuery.value(getAllGroupQuery.record().indexOf("groupname")).toString(),
                           getAllGroupQuery.value(getAllGroupQuery.record().indexOf("description")).toString());
      _groupHash.insert(group->groupName(), group);
    }
    transformName = getAllGroupQuery.value(getAllGroupQuery.record().indexOf("transform_name")).toString();
    if (!transformName.isEmpty())
    {
      transformModel = _transformManager->getTransformModel(transformName);
      if (!transformModel)
      {
        qCritical("NtgSessionManager::NtgSessionManager : Invalid transformName \"%s\" in join_groups_transform ", qPrintable(transformName));
      }
      else
      {
        group->addTransform(transformModel);
      }
    }
  }

  QSqlQuery getAllUserQuery(*_db);

  if( ! getAllUserQuery.prepare("SELECT U.*, J.`groupname` FROM `users` AS U LEFT JOIN `join_groups_users` AS J ON U.`username` = J.`username`"))
  //if( ! getAllUserQuery.prepare("SELECT U.`username`, G.`groupname` FROM `users` AS U JOIN `join_groups_users` AS J ON U.`username` = J.`username` JOIN `groups` AS G ON G.`groupname` = J.`groupname`"))
  {  QHash<QString,NtgTransformModel*> ntgGetUserTransformList();
    qCritical("Query invalide : %s", qPrintable(getAllUserQuery.lastError().text()));
  }

  if( ! getAllUserQuery.exec() )
  {
    qCritical("NtgSessionManager::NtgSessionManager : getAllUserQuery %s", qPrintable(getAllUserQuery.lastError().text()));
  }

  QString userName;
  Ntg::AuthLevel authLevel = Ntg::AUTH_NONE;
  Q_UNUSED(authLevel);
  bool enable = false;
  Q_UNUSED(enable);
  QString password;
  QString serial;
  int nextGraphId = 1;
  Q_UNUSED(nextGraphId);
  QString firstName;
  QString lastName;
  QString emailAddress;
  QDateTime registrationDateTime;
  QDateTime lastModificationDateTime;

  NtgUser * user = NULL;

  while (getAllUserQuery.next())
  {
    userName = getAllUserQuery.value(getAllUserQuery.record().indexOf("username")).toString();
    user = _userHash.value(userName);
    if (!user)
    {
      user = new NtgUser(_db);

      user->setProperties(userName,
                          Ntg::stringToAuthLevel(getAllUserQuery.value(getAllUserQuery.record().indexOf("auth_level")).toString()),
                          getAllUserQuery.value(getAllUserQuery.record().indexOf("enable")).toBool(),
                          getAllUserQuery.value(getAllUserQuery.record().indexOf("password")).toString(),
                          getAllUserQuery.value(getAllUserQuery.record().indexOf("serial")).toString(),
                          getAllUserQuery.value(getAllUserQuery.record().indexOf("next_graph_id")).toInt(),
                          getAllUserQuery.value(getAllUserQuery.record().indexOf("firstname")).toString(),
                          getAllUserQuery.value(getAllUserQuery.record().indexOf("lastname")).toString(),
                          getAllUserQuery.value(getAllUserQuery.record().indexOf("email")).toString(),
                          getAllUserQuery.value(getAllUserQuery.record().indexOf("registration_datetime")).toDateTime(),
                          getAllUserQuery.value(getAllUserQuery.record().indexOf("last_modification_datetime")).toDateTime());

      _userHash.insert(userName, user);
    }
    groupName = getAllUserQuery.value(getAllUserQuery.record().indexOf("groupname")).toString();
    if (!groupName.isEmpty())
    {
      group = _groupHash.value(groupName);
      if (!group)
      {
        qCritical("NtgSessionManager::NtgSessionManager : unknow group name %s", qPrintable(groupName));
      }
      else
      {
        user->addGroup(_groupHash.value(groupName));
        //user->_userGroupHash.insert(groupName, _groupHash.value(groupName));
        group->addUser(user);
      }
    }
  }


  // DEBUG
//  foreach (NtgGroup *  group, _groupHash)
//  {
//    qDebug("[+] GROUPNAME = %s", qPrintable(group->groupName()));
//    foreach (NtgUser *  user, group->getGroupUserHash())
//    {
//      qDebug("\t[-] USERNAME = %s ", qPrintable(user->userName()));
//    }
//    foreach (const NtgTransformModel * transform, group->getTransformHash())
//    {
//      qDebug("\t[-] TRANFORM NAME = %s ", qPrintable(transform->name));
//    }
//  }
//
//
//  foreach (NtgUser *  user, _userHash)
//  {
//    qDebug("[+] USERNAME %s", qPrintable(user->userName()));
//    foreach (NtgGroup *  group, user->getUserGroupHash())
//    {
//      qDebug("\t[-] GROUPNAME %s", qPrintable(group->groupName()));
//    }
//  }


}

NtgSessionManager::~NtgSessionManager()
{
    qDebug() << "NtgSessionManager::~NtgSessionManager()";
  delete _getUserQuery;
  qDeleteAll(_sessionHash);
  qDeleteAll(_userHash);
  qDeleteAll(_groupHash);
  _sessionHash.clear();
  _userHash.clear();
  _groupHash.clear();
}

NtgSession* NtgSessionManager::createSession(const QString & sessionId, const QString & serial, const QHostAddress & ip)
{
  QMutexLocker locker(&_mutex);
  qsrand(QTime(0,0,0).secsTo(QTime::currentTime()));

  if (!sessionId.isEmpty()&&!serial.isEmpty()) 
  {
    NtgSession * session = new NtgSession();
    session->id = sessionId;
    session->ip = ip;
    session->registrationDateTime = QDateTime::currentDateTime(); 
    session->lastModificationDateTime = QDateTime::currentDateTime(); 

    _sessionHash.insert(sessionId, session);

    if(activateSession(sessionId, serial) )
    {
 //   QHash<QString,NtgTransformModel*> ntgGetUserTransformList();

      QMetaObject::invokeMethod(&(session->sessionTimeout), "start", Q_ARG(int,_sessionDuration));
    }
    return session;
  }

  QString id;
  QCryptographicHash hash(QCryptographicHash::Sha1);
  do {
    hash.reset();
    for(int i = 0; i < 10; i++)
    {
      int r = qrand();
      hash.addData((char*)&r, sizeof(int));
    }
    id = hash.result().toHex().mid(0,20);
  } while(_sessionHash.contains(sessionId));

  NtgSession * session = new NtgSession();
  session->id = id;
  session->ip = ip;
  session->registrationDateTime = QDateTime::currentDateTime(); 
  session->lastModificationDateTime = QDateTime::currentDateTime(); 


//  session->sessionTimeout.moveToThread(this->thread());
  connect(session, SIGNAL(timeout(const QString &)), this, SLOT(_sessionTimeout(const QString &)));

  QMetaObject::invokeMethod(&(session->sessionTimeout), "start", Q_ARG(int,_sessionDuration));

  _sessionHash.insert(id, session);
  return session;
}

bool NtgSessionManager::activateSession(const QString & sessionId, const QString & serial)
{
  QMutexLocker locker(&_mutex);
  qDebug() << "NtgSessionManager::activateSession(" << sessionId << ")";

  NtgSession * session = _sessionHash.value(sessionId);
  if( ! session )
  {
    qWarning("NtgSessionManager::activateSession : no such session %s", qPrintable(sessionId));
    return false;
  }

  _getUserQuery->bindValue(":serial", serial);
  if( ! _getUserQuery->exec() )
  {
    qWarning("NtgSessionManager::activateSession : %s", qPrintable(_getUserQuery->lastError().text()));
    return false;
  }

  if( ! _getUserQuery->next())
  {
    qWarning("NtgSessionManager::activateSession : No user with serial %s", qPrintable(serial));
    return false;
  }

  QString userName = _getUserQuery->value(_getUserQuery->record().indexOf("username")).toString();
  Ntg::AuthLevel authLevel = Ntg::stringToAuthLevel(_getUserQuery->value(_getUserQuery->record().indexOf("auth_level")).toString());
  session->userName = userName;
  session->serial = serial;
  session->level = authLevel;
  session->lastModificationDateTime = QDateTime::currentDateTime(); 

  //Create "session" in Db

  // reset timer in the right thread
  QMetaObject::invokeMethod(&(session->sessionTimeout), "start", Q_ARG(int,_sessionDuration));
  return true;
}

bool NtgSessionManager::updateSession(const QString & sessionId, const QString & serial)
{
  QMutexLocker locker(&_mutex);
  qDebug() << "NtgSessionManager::updateSession(" << sessionId << ")";

  NtgSession * session = _sessionHash.value(sessionId);
  if( ! session || session->serial != serial)
  {
    qWarning("NtgSessionManager::updateSession : no such session %s", qPrintable(sessionId));
    return false;
  }

  session->lastModificationDateTime = QDateTime::currentDateTime(); 



  // reset timer in the right thread
  QMetaObject::invokeMethod(&(session->sessionTimeout), "start", Q_ARG(int,_sessionDuration));


  return true;
}

void NtgSessionManager::closeSession(const QString & sessionId)
{
  QMutexLocker locker(&_mutex);
  qDebug() << "NtgSessionManager::closeSession(" << sessionId << ")";

  NtgSession * session = _sessionHash.value(sessionId);
  if( ! session )
  {
    qWarning("NtgSessionManager::closeSession : no such session %s", qPrintable(sessionId));
    return;
  }

  delete _sessionHash.take(sessionId);
}

NtgSession * NtgSessionManager::getSession(const QString & sessionId) const
{
  QMutexLocker locker(&_mutex);
  NtgSession * session = _sessionHash.value(sessionId);
  if( session )
  {
    // reset timer in the right thread
    QMetaObject::invokeMethod(&(session->sessionTimeout), "start", Q_ARG(int,_sessionDuration));
  }
  return session;
}

NtgUser* NtgSessionManager::getUser(const QString & userName) const
{
  return _userHash.value(userName);
}

int NtgSessionManager::sessionDuration() const
{
  QMutexLocker locker(&_mutex);
  return _sessionDuration;
}

void NtgSessionManager::_sessionTimeout(const QString & id)
{
  QMutexLocker locker(&_mutex);
  //qDebug() << "NtgSessionManager::_sessionTimeout(" << id << ")";
  closeSession(id);
  qDebug() << "NtgSessionManager::_sessionTimeout() ending";
}

QHash<QString,NtgTransformModel*> NtgSessionManager::getUserTransformsModels(const QString & sessionId)
{
  QMutexLocker locker(&_mutex);
  QHash<QString,NtgTransformModel*> userTransformsModels;
  NtgSession * session = _sessionHash.value(sessionId);
  if (! session)
  {
    qWarning() << "inexistant requested session (ID :" << sessionId << ")";
    return userTransformsModels;
  }

  NtgUser * user = _userHash.value(session->userName);
  if (! user)
  {
    qWarning() << "inexistant requested user (sessionID :" << sessionId << ")";
    return userTransformsModels;
  }

  QHash<QString,NtgTransformModel*> possibleTransformsModels = user->getUserTransformHash();
  QHash<QString,NtgTransformModel*> availableTransformsModels = _transformManager->transformsModels();

  foreach(NtgTransformModel * userTransform, possibleTransformsModels)
  {
    if (availableTransformsModels.contains(userTransform->name))
    {
      userTransformsModels.insert(userTransform->name,userTransform);
    }
//    else
//    {
//      //TODO cette transform devra etre grisée dans l'IHM
//      qDebug() << "no slave has this transforms :" << userTransform->name;
//    }
  }


  return userTransformsModels;

}

QHash<QString,NtgEntityModel *> NtgSessionManager::getAllEntitiesModels()
{
  QMutexLocker locker(&_mutex);
  return _transformManager->entitiesModels();
}

QStringList NtgSessionManager::getUserEntitiesNames(const QString & sessionId)
  {
    QMutexLocker locker(&_mutex);
    QStringList entitiesList;
    QHash<QString,NtgTransformModel*> availableUserTransforms = getUserTransformsModels(sessionId);
    foreach (NtgTransformModel* transformModel, availableUserTransforms)
    {
      entitiesList.append(transformModel->inputTypes);
    }
    entitiesList.removeDuplicates();
    return entitiesList;
  }

bool NtgSessionManager::isAvailableTransformForUser(const QString & sessionId, const QString & name)
{
  return getUserTransformsModels(sessionId).contains(name);
}
