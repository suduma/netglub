/*
 * libMaia - maiaXmlRpcServer.h
 * Copyright (c) 2007 Sebastian Wiedenroth <wiedi@frubar.net>
 *                and Karl Glatz
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef MAIAXMLRPCSERVER_H
#define MAIAXMLRPCSERVER_H

#include <QtCore>
#include <QtXml>
#include <QtNetwork>

#include "maiaObject.h"
#include "maiaXmlRpcServerConnection.h"
#include "ntgMethodProvider.h"
#include "ntgSessionManager.h"

class MaiaXmlRpcServer : private QTcpServer
{
  Q_OBJECT

  public:
    MaiaXmlRpcServer(const QHostAddress & address = QHostAddress::Any,
                     quint16 port = 8080, quint16 connectionTimeOut = 180,
                     QObject* parent = 0);
    MaiaXmlRpcServer(quint16 port = 8080, quint16 connectionTimeOut = 180,
                     QObject* parent = 0);

    ~MaiaXmlRpcServer();

    QHostAddress getServerAddress();

    void setSslConfig(const QSslConfiguration & conf);
    void setMethodProvider(NtgMethodProvider * methodProvider);

  protected slots:
    void startServer();
    void handleSslErrors(QList<QSslError> errors);

    void stopServer();
    void deleteClientThread();

  protected:
    virtual void incomingConnection(int handle);

  private:
    QSslConfiguration sslConf;
    NtgMethodProvider * _methodProvider;
    quint16 _connectionTimeOut;
    QHostAddress _serverAddress;
    quint16 _serverPort;
    QThread * _serverThread;
    QMutex _mutex;
};

#endif
