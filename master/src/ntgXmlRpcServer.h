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

#ifndef NTGXMLRPCSERVER_H
#define NTGXMLRPCSERVER_H

#include <QObject>
#include "maiaXmlRpcServer.h"
#include "ntgTypes.h"

class NtgSlaveManager;
class NtgXmlRpcServer : public QObject
{
  Q_OBJECT

  public:
    NtgXmlRpcServer(NtgSlaveManager * slaveManager, NtgSessionManager * sessionAuth,
                    const QHostAddress & ip, quint16 port, const QSslConfiguration & conf,
                    quint16 connectionTimeOut);

  public slots:

    // Standard capability API

    QVariantMap getCapabilities(QString sessionId, QString serial);

    // standard introspection API
    // http://xmlrpc-c.sourceforge.net/introspection.html

    QVariantList listMethods(QString sessionId, QString serial);
    QVariantList methodSignature(QString sessionId, QString serial, QString method);
    QString methodHelp(QString sessionId, QString serial, QString method);

    // Error reporting

    QString lastError(QString sessionId, QString serial);

    // Session management

    QString createSession(QString sessionId, QString serial);
    void closeSession(QString sessionId, QString serial);

    // Transform management

    QVariantMap availableTransforms(QString sessionId, QString serial);
    int createTransform(QString sessionId, QString serial, int graphId, QString type);
    bool destroyTransform(QString sessionId, QString serial, int graphId, int transformId);
    bool execTransform(QString sessionId, QString serial, int graphId, int transformId, QVariantMap inputEntity, QVariantMap inputParams);
    bool cancelTransform(QString sessionId, QString serial, int graphId, int transformId);
    QVariantMap infoOfTransform(QString sessionId, QString serial, int graphId, int transformId);
    QVariantList resultsOfTransform(QString sessionId, QString serial, int graphId, int transformId, int startIndex, int toIndex = -1);

    //entity management
    QVariantMap getEntities(QString sessionId, QString serial);
    QVariantList getUserAvailableEntities(QString sessionId, QString serial);

    //graph management
    int createGraph(QString sessionId, QString serial);
    bool deleteGraph(QString sessionId, QString serial, int graphId);
    void setGraphEnableCache(QString sessionId, QString serial, int graphId, bool cacheEnable);
    void clearCache(QString sessionId, QString serial);


  private:

    static QHash<QString,QString> _xmlRpcStructToStringHash(const QVariantMap & structMap);
    static NtgEntity _xmlRpcStructToEntity(QVariantMap structMap);

    static QVariantMap _stringHashToXmlRpcStruct(const QHash<QString,QString> & structMap);
    static QVariantMap _entityToXmlRpcStruct(const NtgEntity & entity);

    void _reportError(QString sessionId, QString error);

    NtgSlaveManager * _slaveManager;
    NtgSessionManager * _sessionAuth;
    MaiaXmlRpcServer _xmlRpcServer;
    NtgMethodProvider _methodProvider;
};

#endif // NTGXMLRPCSERVER_H
