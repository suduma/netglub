/*
 * libMaia - maiaXmlRpcServerConnection.h
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

#ifndef MAIAXMLRPCSERVERCONNECTION_H
#define MAIAXMLRPCSERVERCONNECTION_H

#include <QtCore>
#include <QtXml>
#include <QtNetwork>
#include "maiaFault.h"
#include "ntgMethodProvider.h"

class MaiaXmlRpcServerConnection : public QThread
{
  Q_OBJECT

  public:
    MaiaXmlRpcServerConnection(NtgMethodProvider * provider, QTcpSocket *connection, quint16 connectionTimeOut);
    ~MaiaXmlRpcServerConnection();
    void run();

    QTcpSocket * tcpSocket() { return _clientConnection; };
  private slots:
    void readFromSocket();
    void closeConnection();

  private:
    void sendResponse(QString content);
    void parseCall(QString call);

    QTcpSocket * _clientConnection;
    QString _headerString;
    QHttpRequestHeader * _header;
    bool _keepAlive;
    QTimer _keepAliveTimer;
    quint16 _connectionTimeOut;
    QString _sessionId;
    QString _serial;
    bool _sendCookie;
    NtgMethodProvider * _methodProvider;
};


// ----------------------------------------------------------------------------
class MainLoopCall : public QObject
{
  Q_OBJECT
  public:
    static bool call(QObject * obj, const QByteArray & slot, QVariantList args, QVariant * retVal);

  private slots:
    void _call();

  private:
    MainLoopCall(QObject * obj, const QByteArray & slot, QVariantList args, QVariant * retVal);

    QObject * _obj;
    QByteArray _slot;
    QVariantList _args;
    QVariant * _retVal;
    bool _success;
};

#endif
