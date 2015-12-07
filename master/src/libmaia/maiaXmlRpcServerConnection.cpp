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

#include "maiaXmlRpcServerConnection.h"
#include "maiaXmlRpcServer.h"
#include <QStringList>
#include <QString>
#include <QCoreApplication>

static bool invokeMethodWithVariants(QObject *obj, const QByteArray &method,
                                     const QVariantList &args = QVariantList(),
                                     QVariant *ret = NULL,
                                     Qt::ConnectionType connectionType = Qt::AutoConnection);
static QByteArray getReturnType(const QMetaObject *obj,
                                const QByteArray &method,
                                const QList<QByteArray> argTypes);


MaiaXmlRpcServerConnection::MaiaXmlRpcServerConnection(NtgMethodProvider * provider,
                                                       QTcpSocket * connection,
                                                       quint16 connectionTimeOut)
  : QThread(), _methodProvider(provider)
{
  moveToThread(this);

  _sendCookie = true;
  _header = NULL;
  _clientConnection = connection;
  _keepAliveTimer.moveToThread(this);
  _connectionTimeOut = connectionTimeOut;
}


MaiaXmlRpcServerConnection::~MaiaXmlRpcServerConnection()
{
  qDebug() << "MaiaXmlRpcServerConnection::~MaiaXmlRpcServerConnection()";
  _keepAliveTimer.stop();
  qDebug() << "MaiaXmlRpcServerConnection::~MaiaXmlRpcServerConnection() ending";
  delete _header;
}

void MaiaXmlRpcServerConnection::run()
{

  connect(_clientConnection, SIGNAL(readyRead()), this, SLOT(readFromSocket()));
  connect(_clientConnection, SIGNAL(disconnected()), this, SLOT(quit()));
  _keepAlive = false;
  _keepAliveTimer.setInterval(_connectionTimeOut*1000);
  _keepAliveTimer.setSingleShot(true);
  connect(&_keepAliveTimer, SIGNAL(timeout()), this, SLOT(closeConnection()));

  exec();

}

void MaiaXmlRpcServerConnection::readFromSocket()
{
  QString lastLine;
  if( _keepAliveTimer.isActive())
    _keepAliveTimer.start();

  while(_clientConnection->canReadLine() && !_header)
  {
    lastLine = _clientConnection->readLine();
    _headerString += lastLine;
    if(lastLine == "\r\n")
    { /* http header end */
      _header = new QHttpRequestHeader(_headerString);
      if(!_header->isValid())
      {
        /* return http error */
        qDebug() << "Invalid Header";
        return;
      }
      else if(_header->method() != "POST")
      {
        /* return http error */
        qDebug() << "No Post!";
        return;
      }
      else if(!_header->contentLength())
      {
        /* return fault */
        qDebug() << "No Content Length";
        return;
      }
    }
  }

  if(_header)
  {
    if(_header->contentLength() <= _clientConnection->bytesAvailable())
    {
      _sessionId.clear();
      _serial.clear();

      if( _header->hasKey("Cookie"))
      {
        QString cookiesStr = _header->value("Cookie");
        QStringList cookies = cookiesStr.split(';');
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

      /* if asked explicitly or if HTTP/1.1, Keep-Alive */
      if((_header->hasKey("Connection") && _header->value("Connection").toLower() == "keep-alive")
        || (_header->majorVersion() == 1 && _header->minorVersion() == 1 &&
            _header->value("Connection").toLower() != "close"))
        _keepAlive = true;

      /* all data complete */
      parseCall(_clientConnection->read(_header->contentLength()));
      delete _header;
      _header = NULL;
      _headerString.clear();
    }
  }
}


void MaiaXmlRpcServerConnection::closeConnection()
{
  qDebug() << "MaiaXmlRpcServerConnection::closeConnection()";
  quit();
}


void MaiaXmlRpcServerConnection::sendResponse(QString content)
{
  QHttpResponseHeader header(200, "Ok");
  QByteArray block,dataBlock;
  header.setValue("Server", "MaiaXmlRpc/0.1");
  header.setValue("Content-Type", "text/xml");
  if( _keepAlive )
  {
    _keepAliveTimer.start();
    header.setValue("Connection","Keep-Alive");
    header.setValue("Keep-Alive",QString("timeout=%1").arg(_keepAliveTimer.interval()));
  }
  else
  {
	header.setValue("Connection","close");
  }
  if( _sendCookie)
  {
    QString cookieVal("SessionID=%1; Serial=%2; expires=%3;");
    cookieVal = cookieVal.arg(_sessionId).arg(_serial);
    int validLen = _methodProvider->sessionManager()->sessionDuration();
    QDateTime expiryDate = QDateTime::currentDateTime().addSecs(validLen).toUTC();
    QLocale englishLocale(QLocale::English);
    cookieVal = cookieVal.arg(englishLocale.toString(expiryDate, "ddd, dd-MMM-yyyy hh:mm:ss' GMT'"));
    header.setValue("Set-Cookie", cookieVal);
    //_sendCookie = false;
  }

  dataBlock.append(content.toUtf8());
  header.setContentLength(dataBlock.size());
  block.append(header.toString().toUtf8());
  block.append(dataBlock);
  _clientConnection->write(block);

  if( ! _keepAlive )
    _clientConnection->disconnectFromHost();
}


void MaiaXmlRpcServerConnection::parseCall(QString call)
{
  QDomDocument doc;
  QVariant ret;
  QString response;
  QObject *responseObject;
  const char *responseSlot;

  if(!doc.setContent(call))
  { /* received invalid xml */
    MaiaFault fault(-32700, "parse error: not well formed");
    sendResponse(fault.toString());
    return;
  }

  QDomElement methodNameElement = doc.documentElement().firstChildElement("methodName");
  QDomElement params = doc.documentElement().firstChildElement("params");
  if(methodNameElement.isNull())
  { /* invalid call */
    MaiaFault fault(-32600, "server error: invalid xml-rpc. not conforming to spec");
    sendResponse(fault.toString());
    return;
  }

  QString methodName = methodNameElement.text();
  bool hasSessionArg = methodName.startsWith("s_");

  QList<QVariant> args;
  QDomNode paramNode = params.firstChild();
  while(!paramNode.isNull())
  {
    args << MaiaObject::fromXml( paramNode.firstChild().toElement());
    paramNode = paramNode.nextSibling();
  }

  if( hasSessionArg )
  {
    methodName = methodName.mid(2);
    if (args.size() < 2 )
    {
      MaiaFault fault(1001, "server error: missing needed arguments (sessionId and/or serial)");
      sendResponse(fault.toString());
      return;
    }
    _sessionId = args.at(0).toString();
    _serial = args.at(1).toString();
  }

  //_sendCookie = false;

  if( _sessionId.isEmpty() || ! _methodProvider->sessionManager()->getSession(_sessionId))
  {
    qDebug() << "-------- sessionId is empty for " << _clientConnection->peerAddress().toString() << "  ! or doesn't exist as an object ! ----";
    _sessionId = _methodProvider->sessionManager()->createSession(_sessionId, _serial, _clientConnection->peerAddress())->id;
    //_sendCookie = true;
  }
  else
  {
    _methodProvider->sessionManager()->updateSession(_sessionId, _serial);
  }

  Ntg::MethodAccess access = _methodProvider->getMethod(_sessionId, methodName, &responseObject, &responseSlot);

  switch((int)access)
  {
    case Ntg::METHOD_UNKNOWN:
    {
      MaiaFault fault(-32601, QString("server error: requested method \"%1\" not found").arg(methodName));
      sendResponse(fault.toString());
      return;
    }
    case Ntg::METHOD_UNAUTHORIZED:
    {
      MaiaFault fault(1000, "server error: no access to requested method");
      sendResponse(fault.toString());
      return;
    }
    case Ntg::METHOD_VALID:
    {
      break;
    }
    default:
    {
      MaiaFault fault(-32603, "server error: unknown error");
      sendResponse(fault.toString());
      return;
    }
  }

  if( ! hasSessionArg )
  {
    if (methodName!="netglub.createSession") args.push_front(QVariant(_serial));
    args.push_front(QVariant(_sessionId));
  }
  
  qDebug() << methodName << args;

  bool fault = MainLoopCall::call(responseObject, responseSlot, args, &ret);

  NtgSession * session =  _methodProvider->sessionManager()->getSession(_sessionId);

  if (_serial.isEmpty() &&  session)
  {
    _serial = session->serial;
  }

  if( !fault  )
  { /* error invoking... */
    MaiaFault fault(-32602, "server error: invalid method parameters");
    sendResponse(fault.toString());
    return;
  }

  if(ret.canConvert<MaiaFault>())
  {
    response = ret.value<MaiaFault>().toString();
  }
  else
  {
    response = MaiaObject::prepareResponse(ret);
  }

  sendResponse(response);
}


bool MainLoopCall::call(QObject * obj, const QByteArray & slot, QVariantList args, QVariant * retVal)
{
  MainLoopCall mCall(obj, slot, args, retVal);
  return mCall._success;
}


void MainLoopCall::_call()
{
  _success = invokeMethodWithVariants(_obj, _slot, _args, _retVal);
}


MainLoopCall::MainLoopCall(QObject * obj, const QByteArray & slot, QVariantList args, QVariant * retVal)
  : _obj(obj), _slot(slot), _args(args), _retVal(retVal), _success(false)
{
  moveToThread(QCoreApplication::instance()->thread());
  if( this->thread() == QThread::currentThread() )
    invokeMethodWithVariants(this, "_call");
  else
    invokeMethodWithVariants(this, "_call", QVariantList(), NULL, Qt::BlockingQueuedConnection);
}



// ------------------------- utility functions --------------------------------

/*	taken from http://delta.affinix.com/2006/08/14/invokemethodwithvariants/
	thanks to Justin Karneges once again :) */
bool invokeMethodWithVariants(QObject *obj,
                              const QByteArray &method, const QVariantList &args,
                              QVariant *ret, Qt::ConnectionType connectionType)
{

  // QMetaObject::invokeMethod() has a 10 argument maximum
  if(args.count() > 10)
    return false;

  QList<QByteArray> argTypes;
  for(int n = 0; n < args.count(); ++n)
    argTypes += args[n].typeName();

  // get return type
  int metatype = 0;
  QByteArray retTypeName = getReturnType(obj->metaObject(), method, argTypes);
  if(!retTypeName.isEmpty()  && retTypeName != "QVariant")
  {
    metatype = QMetaType::type(retTypeName.data());
    if(metatype == 0) // lookup failed
      return false;
  }

  QGenericArgument arg[10];
  for(int n = 0; n < args.count(); ++n)
    arg[n] = QGenericArgument(args[n].typeName(), args[n].constData());

  QGenericReturnArgument retarg;
  QVariant retval;
  if(metatype != 0)
  {
    retval = QVariant(metatype, (const void *)0);
    retarg = QGenericReturnArgument(retval.typeName(), retval.data());
  }
  else
  { /* QVariant */
    retarg = QGenericReturnArgument("QVariant", &retval);
  }

  if(retTypeName.isEmpty())
  { /* void */
    if(!QMetaObject::invokeMethod(obj, method.data(), connectionType,
                                  arg[0], arg[1], arg[2], arg[3], arg[4],
                                  arg[5], arg[6], arg[7], arg[8], arg[9]))
      return false;
  }
  else
  {
    if(!QMetaObject::invokeMethod(obj, method.data(), connectionType, retarg,
                                  arg[0], arg[1], arg[2], arg[3], arg[4],
                                  arg[5], arg[6], arg[7], arg[8], arg[9]))
      return false;
  }

  if(retval.isValid() && ret)
    *ret = retval;
  return true;
}


QByteArray getReturnType(const QMetaObject *obj,
                         const QByteArray &method,
                         const QList<QByteArray> argTypes)
{
  for(int n = 0; n < obj->methodCount(); ++n)
  {
    QMetaMethod m = obj->method(n);
    QByteArray sig = m.signature();

    int offset = sig.indexOf('(');
    if(offset == -1)
      continue;
    QByteArray name = sig.mid(0, offset);
    if(name != method)
      continue;

    if(m.parameterTypes() != argTypes)
      continue;

    return m.typeName();
  }
  return QByteArray();
}
