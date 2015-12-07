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

#include "ntgRpcHandler.h"
#include "ntgRpcMessage.h"
#include "ntgRpcTransport.h"

#include <QMetaMethod>

NtgRpcHandler::NtgRpcHandler()
  : QObject()
  , _transport(NULL)
{
}


NtgRpcHandler::~NtgRpcHandler()
{
//    qDebug() << "NtgRpcHandler::~NtgRpcHandler";
  _objects.clear();
  _signalProxies.clear();
}


bool NtgRpcHandler::registerInvokableObject(QObject * obj)
{
  if( ! obj || obj->objectName().isEmpty() )
    return false;
  _objects.insert(obj->objectName(), obj);
  return true;
}


bool NtgRpcHandler::registerSignalProxy(QObject * obj, const QString & proxiedObjName)
{
  if( ! obj || proxiedObjName.isEmpty())
    return false;
  _signalProxies.insertMulti(proxiedObjName, obj);
  return true;
}

bool NtgRpcHandler::unRegisterSignalProxy(QObject * obj, const QString & proxiedObjName)
{
    if( ! obj || proxiedObjName.isEmpty())
      return false;
    _signalProxies.remove(proxiedObjName, obj);
    return true;
}


bool NtgRpcHandler::emitSignal(QObject * obj, const QString & signalName, const QVariantList & argList)
{
    qDebug() << signalName << argList << "-------------------------------------------------------------------";
  _lastError.clear();
  if(!_transport)
  {
    _lastError = "No transport set for handler";
    return false;
  }

  if( ! obj)
    return false;
  QString objName = obj->objectName();
  NtgRpcMessage * msg = NtgRpcMessage::makeSignalMessage(objName,signalName.toAscii(),argList);
  if( ! msg )
  {
    _lastError = "Could not make signal message";
    return false;
  }
  if( _transport->transmitMessage(msg) != NULL)
  {
    qWarning("Transport tried to respond to a emitted message !");
    delete msg;
    return false;
  }

  delete msg;
  return true;
}


bool NtgRpcHandler::invokeMethod(const QString & objName,
                                 const QString & methodName,
                                 const QVariantList & argList,
                                 QVariant * returnValue)
{
  _lastError.clear();
  if(!_transport)
  {
    _lastError = "No transport set for handler";
    return false;
  }

  NtgRpcMessage * msg = NtgRpcMessage::makeInvokeMessage(objName,methodName.toAscii(),argList);
  if( ! msg )
  {
    _lastError = "Could not make invoke message";
    return false;
  }
  NtgRpcMessage * res = _transport->transmitMessage(msg);

  delete msg;
  if( ! res || res->messageType() != NtgRpcMessage::RESPONSE_MESSAGE)
  {
    _lastError = "Transport did not return a response message";
    delete res;
    return false;
  }
  if( res->isError() )
  {
    _lastError = res->errorString();
    delete res;
    return false;
  }
  if(returnValue)
    *returnValue = res->returnValue();
  delete res;

  return true;
}


static QByteArray getReturnType(const QMetaObject *obj,
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


static bool invokeMethodWithVariants(QObject * obj,
                                     const QByteArray & method,
                                     const QVariantList &args,
                                     QVariant * ret = NULL,
                                     Qt::ConnectionType type = Qt::AutoConnection)
{


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
  else  /* QVariant */
  {
    retarg = QGenericReturnArgument("QVariant", &retval);
  }

  if(retTypeName.isEmpty()) /* void */
  {
    if(!QMetaObject::invokeMethod(obj, method.data(), type,
                                  arg[0], arg[1], arg[2], arg[3], arg[4],
                                  arg[5], arg[6], arg[7], arg[8], arg[9]))
      return false;
  }
  else
  {
    if(!QMetaObject::invokeMethod(obj, method.data(), type, retarg,
                                  arg[0], arg[1], arg[2], arg[3], arg[4],
                                  arg[5], arg[6], arg[7], arg[8], arg[9]))
      return false;
  }

  if(retval.isValid() && ret)
    *ret = retval;

  return true;
}


QString NtgRpcHandler::lastError() const
{
  return _lastError;
}


bool NtgRpcHandler::failed() const
{
  return (!_lastError.isEmpty());
}


void NtgRpcHandler::setTransport(NtgRpcTransport * transport)
{
  _transport = transport;
}


NtgRpcMessage * NtgRpcHandler::processMessage(NtgRpcMessage * msg)
{
  if(! msg)
    return NULL;
  switch(msg->messageType())
  {
    case NtgRpcMessage::INVOKE_MESSAGE:
    {
      if( ! _objects.contains(msg->objName()) )
        return NtgRpcMessage::makeErrorResponseMessage("No such objects !");
      QObject * obj = _objects[msg->objName()];
      QVariant retVal;
      if( ! invokeMethodWithVariants(obj, msg->symbolName(), msg->argList(), &retVal))
      {
        QString err("Invoking %1 on %2 failed !");
        qDebug() << msg->argList();
        err = err.arg(QString(msg->symbolName())).arg(obj->objectName());
        return NtgRpcMessage::makeErrorResponseMessage(err);
      }
      return NtgRpcMessage::makeValueResponseMessage(retVal);
    }
    case NtgRpcMessage::SIGNAL_MESSAGE:
    {
      if( _signalProxies.contains(msg->objName()) )
      {
        QList<QObject *> objects = _signalProxies.values(msg->objName());
        foreach(QObject * obj, objects)
        {
          if( ! invokeMethodWithVariants(obj, msg->symbolName(), msg->argList(), NULL, Qt::QueuedConnection))
          {
            qCritical() << "Invoking failed !" <<  msg->symbolName() << msg->argList();
            qDebug("Objname: %s", qPrintable(obj->objectName()));
            const QMetaObject * mo = obj->metaObject();
            for(int i = 0; i < mo->methodCount(); i++)
            {
              QMetaMethod mm = mo->method(i);
              qDebug("-> %s %s", mm.typeName(), mm.signature());
            }
          }
        }
      }
      return NULL;
    }
    case NtgRpcMessage::RESPONSE_MESSAGE:
    {
      qCritical("Error: Handler was asked to process a RESPONSE...");
      return NULL;
    }
  }
  return NULL;
}
