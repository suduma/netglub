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

#ifndef NTGSSLTRANSPORT_H
#define NTGSSLTRANSPORT_H

#include <QObject>
#include <QHostAddress>
#include <QSslSocket>
#include <QSslConfiguration>

#include "ntgRpcTransport.h"

class NtgSlaveCreds;

struct NtgSlaveCreds
{
  QString serial;
  QHostAddress ip;
};

class NtgRpcSslTransport : public QObject, public NtgRpcTransport
{
  Q_OBJECT
  public:
    NtgRpcSslTransport(const QSslConfiguration & tlsConf, NtgRpcHandler * handler);
    virtual ~NtgRpcSslTransport();

    virtual NtgRpcMessage * transmitMessage(NtgRpcMessage * msg);

    bool connectToMaster(QString masterLocation);
    bool connectToSlave(int socketDescriptor, NtgSlaveCreds * creds);
    bool authorizeSlave();
    bool refuseSlave();

  signals:
    void transportClosed();

  protected slots:
    void _incomingData();
    void _socketDisconnected();
    void _socketError(QAbstractSocket::SocketError socketError);
#ifdef TRANSPORT_WITH_SSL
    void _sslErrors(const QList<QSslError> & errors);
#endif

  protected:
    void _setupSignals();
    void _closeTransport();

#ifdef TRANSPORT_WITH_SSL
    QSslSocket * _socket;
#else
    QTcpSocket * _socket;
#endif

    QSslConfiguration _tlsConf;

    enum {MODE_NONE, MODE_MASTER,MODE_SLAVE,MODE_SLAVE_AUTHORIZED} _mode;
    bool _waitingForResponse;
    NtgRpcMessage * _response;
    QList<NtgRpcMessage*> _queuedMessages;
};

#endif // NTGSSLTRANSPORT_H
