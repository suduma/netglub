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

#include "ntgRpcSslTransport.h"
#include "ntgRpcMessage.h"
#include "ntgRpcHandler.h"
#include "ntgTypes.h"

#include <QSslSocket>
#include <QSslConfiguration>
#include <QStringList>
#include <QTimer>

#define TIMEOUT 20000

NtgRpcSslTransport::NtgRpcSslTransport(const QSslConfiguration & tlsConf, NtgRpcHandler * handler)
  : QObject()
{
  _mode = MODE_NONE;
  _response = NULL;
  _waitingForResponse = false;
  _handler = handler;
  _tlsConf = tlsConf;
#ifdef TRANSPORT_WITH_SSL
  _socket = new QSslSocket();
  _socket->setSslConfiguration(_tlsConf);
  connect(_socket, SIGNAL(sslErrors(QList<QSslError>)), this, SLOT(_sslErrors(QList<QSslError>)));
#else
  _socket = new QTcpSocket();
#endif
  connect(_socket, SIGNAL(disconnected()), this, SLOT(_socketDisconnected()));
  connect(_socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(_socketError(QAbstractSocket::SocketError)));
}


NtgRpcSslTransport::~NtgRpcSslTransport()
{
  _closeTransport();
  if( _socket )
  {
    if( _socket->state() != QAbstractSocket::UnconnectedState)
      _socket->abort();
    _socket->deleteLater();
    _socket = NULL;
  }
}


NtgRpcMessage * NtgRpcSslTransport::transmitMessage(NtgRpcMessage * msg)
{
  if( _socket->state() == QAbstractSocket::ConnectedState
      && (
           (_mode == MODE_MASTER && msg->messageType() == NtgRpcMessage::SIGNAL_MESSAGE)
        || (_mode == MODE_SLAVE_AUTHORIZED && msg->messageType() != NtgRpcMessage::RESPONSE_MESSAGE)
      )
    )
  {
    if(  ! msg->writeToSocket(_socket) || ! _socket->flush())
    {
      _closeTransport();
      return NULL;
    }

    // if it's a signal, we're done
    if( msg->messageType() == NtgRpcMessage::SIGNAL_MESSAGE)
      return NULL;

    _response = NULL;
    _waitingForResponse = true;

    // this will call _incomingData() if a packet arrives, thus setting _response
    while( ! _response )
    {
      // this will call _incomingData() if a packet arrives, thus setting _response
      if( ! _socket->waitForReadyRead(TIMEOUT))
      {
        _closeTransport();
        _waitingForResponse = false;
        return NULL;
      }
    }

    _waitingForResponse = false;
    return _response;
  }
  return NULL;
}


bool NtgRpcSslTransport::connectToMaster(QString masterLocation)
{
  if( _mode != MODE_NONE )
    return false;

  int idx = masterLocation.indexOf(':');
  QString location;
  quint16 port = 2010;
  if(idx < 0)
    location = masterLocation;
  else
  {
    location = masterLocation.split(':').first();
    port = masterLocation.split(':').at(1).toUShort();
  }

  qDebug("Connecting to : %s:%d", qPrintable(location), port);

#ifdef TRANSPORT_WITH_SSL
  _socket->connectToHostEncrypted(location,port);
  if( ! _socket->waitForEncrypted(TIMEOUT))
  {
    qCritical("Setting up socket failed");
    return false;
  }
#else
  _socket->connectToHost(location,port);
  if( ! _socket->waitForConnected(TIMEOUT))
  {
    qCritical("Setting up socket failed");
    return false;
  }
#endif

  _mode = MODE_MASTER;

  connect(_socket, SIGNAL(readyRead()), this, SLOT(_incomingData()));

  // we probably received data during waitForEncrypted, go check
  QTimer::singleShot(0, this, SLOT(_incomingData()));
  return true;
}


bool NtgRpcSslTransport::connectToSlave(int socketDescriptor, NtgSlaveCreds * creds)
{
  if( _mode != MODE_NONE )
    return false;

  _socket->setSocketDescriptor(socketDescriptor);

#ifdef TRANSPORT_WITH_SSL
  _socket->startServerEncryption();
  if( ! _socket->waitForEncrypted(TIMEOUT))
  {
    _closeTransport();
    qCritical("Setting up socket failed");
    return false;
  }
  if( creds)
  {
    creds->serial = _socket->peerCertificate().subjectInfo(QSslCertificate::CommonName);
    creds->ip = _socket->peerAddress();
  }
#else
  Q_UNUSED(creds);
#endif
  _mode = MODE_SLAVE;
  return true;
}


bool NtgRpcSslTransport::authorizeSlave()
{
  if( _mode != MODE_SLAVE )
    return false;
  connect(_socket, SIGNAL(readyRead()), this, SLOT(_incomingData()));
  _mode = MODE_SLAVE_AUTHORIZED;
  return true;
}


bool NtgRpcSslTransport::refuseSlave()
{
  if( _mode != MODE_SLAVE )
    return false;
  _closeTransport();
  return true;
}


void NtgRpcSslTransport::_incomingData()
{

  // unqueue signal messages
  if( ! _waitingForResponse)
  {
    foreach(NtgRpcMessage * msg, _queuedMessages)
    {
      if( _handler->processMessage(msg) != NULL )
        qWarning("Handler tried to respond to a signal");
      delete msg;
    }
    _queuedMessages.clear();
  }

  // process latest messages

  while(_socket->bytesAvailable() > 0)
  {
    NtgRpcMessage * msg = NtgRpcMessage::readFromSocket(_socket);
    if( ! msg)
    {
      qCritical("Reading message from socket failed, closing transport !");
      _closeTransport();
      return;
    }

    if( _waitingForResponse)
    {
      if(msg->messageType() == NtgRpcMessage::RESPONSE_MESSAGE )
      {
        _response = msg;
//        _eventLoop.quit();
      }
      else if(msg->messageType() == NtgRpcMessage::SIGNAL_MESSAGE )
      {
        if( _queuedMessages.isEmpty())
          QTimer::singleShot(0,this,SLOT(_incomingData()));
        _queuedMessages.append(msg);
      }
      else
      {
        qWarning("Message other than SIGNAL or RESPONSE are not allowed from a slave!");
        delete msg;
        _closeTransport();
      }
    }
    else
    {
      NtgRpcMessage * res = _handler->processMessage(msg);

      if( msg->messageType() == NtgRpcMessage::SIGNAL_MESSAGE )
      {
        if( res )
          qWarning("Handler tried to respond to a signal");
        delete msg;
        continue;
      }

      delete msg;

      if( ! res || res->messageType() != NtgRpcMessage::RESPONSE_MESSAGE)
      {
        qCritical("Received incorrect response from handler !");
        delete res;
        _closeTransport();
        return;
      }
      if( ! res->writeToSocket(_socket) || ! _socket->flush())
      {
        qCritical("Sending response failed !");
        delete res;
        _closeTransport();
        return;
      }
      delete res;
    }
  }
}


void NtgRpcSslTransport::_socketDisconnected()
{
  _closeTransport();
}


void NtgRpcSslTransport::_socketError(QAbstractSocket::SocketError socketError)
{
  QAbstractSocket * socket = qobject_cast<QAbstractSocket*>(sender());
  qWarning("Socket Error %d : %s", socketError, qPrintable(socket->errorString()));
  _closeTransport();
}

#ifdef TRANSPORT_WITH_SSL
void NtgRpcSslTransport::_sslErrors(const QList<QSslError> & errors)
{
  QSslSocket * socket = qobject_cast<QSslSocket*>(sender());
  if(errors.size() == 1 && errors.first().error() == QSslError::HostNameMismatch)
  {
    QSslCertificate cert = errors.first().certificate();
    qWarning("ignoring error %s ", qPrintable(cert.subjectInfo(QSslCertificate::CommonName)));
    socket->ignoreSslErrors();
    return;
  }

  foreach(const QSslError& err, errors)
  {
    qWarning("TLS Error %d : %s", err.error(), qPrintable(err.errorString()));
  }
  _closeTransport();
}
#endif


void NtgRpcSslTransport::_closeTransport()
{
  if(_socket)
    _socket->disconnectFromHost();

  int prevMode = _mode;
  _mode = MODE_NONE;
  _response = NULL;
  qDeleteAll(_queuedMessages);
  _queuedMessages.clear();

  if( prevMode != MODE_NONE )
    emit transportClosed();
}
