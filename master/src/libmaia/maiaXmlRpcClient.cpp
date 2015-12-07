/*
 * libMaia - maiaXmlRpcServerConnection.cpp
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

#include "maiaXmlRpcClient.h"
#include "maiaFault.h"


MaiaXmlRpcClient::MaiaXmlRpcClient(QObject* parent)
  : QObject(parent)
{
  _http = new QHttp(this);
  _socket = new QSslSocket();
  _http->setSocket(_socket);
  connect(_socket, SIGNAL(encrypted()), this, SLOT(sslCheck()));
}


MaiaXmlRpcClient::MaiaXmlRpcClient(QUrl url, QObject* parent)
  : QObject(parent)
{
  _http = new QHttp(this);
  _socket = new QSslSocket();
  _http->setSocket(_socket);
  connect(_socket, SIGNAL(encrypted()), this, SLOT(sslCheck()));
  setUrl(url);
}


MaiaXmlRpcClient::~MaiaXmlRpcClient()
{
    delete _http;
    delete _socket;
}


void MaiaXmlRpcClient::setUrl(QUrl url)
{
  if(!url.isValid())
    return;

  _url = url;
  QHttp::ConnectionMode connMode = (_url.scheme() == "https") ? QHttp::ConnectionModeHttps : QHttp::ConnectionModeHttp;
  _http->setHost(_url.host(), connMode, _url.port() != -1 ? _url.port() : 80);
  if (!_url.userName().isEmpty())
    _http->setUser(_url.userName(), _url.password());
}


void MaiaXmlRpcClient::call(QString method, QList<QVariant> args,
							QObject* responseObject, const char* responseSlot,
                            QObject* faultObject, const char* faultSlot)
{
  int callid = 0;
  MaiaObject* call = new MaiaObject(this);
  connect(call, SIGNAL(aresponse(QVariant &)), responseObject, responseSlot);
  connect(call, SIGNAL(fault(int, const QString &)), faultObject, faultSlot);

  connect(_http, SIGNAL(requestFinished(int, bool)), this, SLOT(httpRequestDone(int, bool)));
  connect(_http, SIGNAL(responseHeaderReceived(QHttpResponseHeader)), this, SLOT(responseHeaderReceived(QHttpResponseHeader)));
  connect(_http, SIGNAL(sslErrors(QList<QSslError>)), this, SLOT(handleSslErrors(QList<QSslError>)));

  QHttpRequestHeader header("POST", _url.path());
  header.setValue("Host", _url.host());
  header.setValue("User-Agent", "libmaia 0.1");
  header.setValue("Content-type", "text/xml");
  if (!_sessionId.isEmpty() || _serial.isEmpty())
  {
    QString cookieVal("SessionID=%1; Serial=%2;");
    cookieVal = cookieVal.arg(_sessionId).arg(_serial);
    header.setValue("Cookie", cookieVal);
  }

  callid = _http->request(header, call->prepareCall(method, args).toUtf8());
  _callmap[callid] = call;
}


void MaiaXmlRpcClient::httpRequestDone(int id, bool error)
{
  QString response;
  if(!_callmap.contains(id))
    return;
  if(error) {
    MaiaFault fault(-32300, _http->errorString());
    response = fault.toString();
  } else {
    response = QString::fromUtf8(_http->readAll());
  }
  _callmap[id]->parseResponse(response);
  _callmap.remove(id);
}


void MaiaXmlRpcClient::responseHeaderReceived(QHttpResponseHeader header)
{
  if (header.keys().contains("Set-Cookie")) {
    QString cookieStr = header.value("Set-Cookie");
    QStringList cookies = cookieStr.split(';');
    foreach(const QString & cookieStr, cookies)
    {
      int valueOffset = cookieStr.indexOf('=');
      if( valueOffset == -1)
        continue;
      QString name = cookieStr.mid(0, valueOffset).simplified();
      QString value = cookieStr.mid(valueOffset+1, -1).simplified();
      if( name == "SessionID")
      {
        _sessionId = value;
      }
      else if( name == "Serial")
      {
        _serial = value;
      }
    }
  }
}


void MaiaXmlRpcClient::handleSslErrors(QList<QSslError> /*errors*/)
{
   /*
   foreach(const QSslError & error, errors)
    qWarning("SSL Error : %s", qPrintable(error.errorString()));
  */
  _http->ignoreSslErrors();
}


void MaiaXmlRpcClient::sslCheck()
{
  qDebug() << _socket->peerCertificate();
}
