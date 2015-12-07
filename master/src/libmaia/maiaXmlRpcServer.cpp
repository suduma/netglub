/*
 * libMaia - maiaXmlRpcServer.cpp
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

#include "maiaXmlRpcServer.h"
#include "maiaFault.h"


MaiaXmlRpcServer::MaiaXmlRpcServer(const QHostAddress &address, quint16 port, quint16 connectionTimeOut, QObject* parent)
  : QTcpServer(parent)
  , _methodProvider(NULL)
  , _serverAddress(address)
  , _serverPort(port)
  , _mutex(QMutex::Recursive)
{
  _serverThread = new QThread();
  moveToThread(_serverThread);
  connect(_serverThread, SIGNAL(started()), this, SLOT(startServer()));
  connect(_serverThread, SIGNAL(finished()), this, SLOT(stopServer()));
  _serverThread->start();
  _connectionTimeOut = connectionTimeOut;
}


MaiaXmlRpcServer::MaiaXmlRpcServer(quint16 port, quint16 connectionTimeOut, QObject* parent)
  : QTcpServer(parent)
  , _methodProvider(NULL)
  , _serverAddress(QHostAddress::Any)
  , _serverPort(port)
  , _mutex(QMutex::Recursive)
{
  _serverThread = new QThread();
  moveToThread(_serverThread);
  connect(_serverThread, SIGNAL(finished()), _serverThread, SLOT(deleteLater()));
  _serverThread->start();
  _connectionTimeOut = connectionTimeOut;
}


MaiaXmlRpcServer::~MaiaXmlRpcServer()
{
  QMetaObject::invokeMethod(this, "stopServer", Qt::BlockingQueuedConnection);
  _serverThread->deleteLater();
}


void MaiaXmlRpcServer::startServer()
{
  this->listen(_serverAddress, _serverPort);
}


void MaiaXmlRpcServer::handleSslErrors(QList<QSslError> errors)
{
  QSslSocket * sslSocket = qobject_cast<QSslSocket*>(sender());
  if( ! sslSocket )
  {
    qCritical("Entered slot MaiaXmlRpcServer::handleSslErrors from non-SSL socket !");
    return;
  }
  foreach(const QSslError & error, errors)
    qWarning("SSL Error : %s from %s:%d", qPrintable(error.errorString()),
             qPrintable(sslSocket->peerAddress().toString()),
             sslSocket->peerPort());
}

void MaiaXmlRpcServer::deleteClientThread()
{
  QMutexLocker locker(&_mutex);
  MaiaXmlRpcServerConnection * client = qobject_cast<MaiaXmlRpcServerConnection*>(sender());
  if (client) {
    QTcpSocket * socket =  client->tcpSocket();
    socket->disconnectFromHost();
    socket->deleteLater();
    delete client;
  }
}

void MaiaXmlRpcServer::stopServer()
{
  this->close();
  _serverThread->quit();
}


void MaiaXmlRpcServer::incomingConnection(int handle)
{
    //qDebug() << "MaiaXmlRpcServer::incomingConnection()" << QThread::currentThreadId();

    QTcpSocket * tcpSocket = NULL;
#ifdef XMLRPC_WITH_SSL
  if( sslConf.isNull())
  {
#endif
    tcpSocket = new QTcpSocket();
    tcpSocket->setSocketDescriptor(handle);
#ifdef XMLRPC_WITH_SSL
  }
  else
  {
    QSslSocket * sslSocket = new QSslSocket();
    sslSocket->setSocketDescriptor(handle);
    sslSocket->setSslConfiguration(sslConf);
    connect(sslSocket,SIGNAL(sslErrors(QList<QSslError>)),
            this, SLOT(handleSslErrors(QList<QSslError>)));
    sslSocket->startServerEncryption();
    if( ! sslSocket->waitForEncrypted() )
    {
      qCritical("Encryption of client socket failed, dropping connection.");
      sslSocket->abort();
      sslSocket->deleteLater();
      return;
    }
    tcpSocket = sslSocket;
  }
#endif
  MaiaXmlRpcServerConnection * clientThread = new MaiaXmlRpcServerConnection(_methodProvider,tcpSocket,_connectionTimeOut);
  connect(clientThread, SIGNAL(finished()), this, SLOT(deleteClientThread()));
  clientThread->start();
}

QHostAddress MaiaXmlRpcServer::getServerAddress()
{
  QMutexLocker locker(&_mutex);
  return _serverAddress;
}


void MaiaXmlRpcServer::setSslConfig(const QSslConfiguration & conf)
{
  QMutexLocker locker(&_mutex);
  sslConf = conf;
}


void MaiaXmlRpcServer::setMethodProvider(NtgMethodProvider * methodProvider)
{
  QMutexLocker locker(&_mutex);
  _methodProvider = methodProvider;
}



