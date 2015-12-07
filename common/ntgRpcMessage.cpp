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

#include "ntgRpcMessage.h"
#include <QtEndian>

#define TIMEOUT 5000


NtgRpcMessage::NtgRpcMessage()
  : _messageType(NtgRpcMessage::LAST_MESSAGE_TYPE)
  , _isError(false)
{
}


NtgRpcMessage::~NtgRpcMessage()
{
}


bool NtgRpcMessage::writeToSocket(QAbstractSocket * socket) const
{
  if(!socket)
    return false;

  QByteArray block;
  if(!this->_serialize(block))
    return false;

  return _writeToSocket(socket, block);
}


NtgRpcMessage* NtgRpcMessage::readFromSocket(QAbstractSocket * socket)
{
  if( !socket)
    return NULL;

  QByteArray block;
  if(!_readFromSocket(socket, block))
    return NULL;

  NtgRpcMessage* m = new NtgRpcMessage();
  if(!m->_unserialize(block))
  {
    qCritical() << "NtgRpcMessage: error during unmarshalling of message.";
    delete m;
    return NULL;
  }

  return m;
}


bool NtgRpcMessage::_writeToSocket(QAbstractSocket * socket, const QByteArray & data)
{
  const char* dataToWrite = data.data();
  qint64 bytesToWrite = data.size();
  qint64 bytesWritten;
  while(bytesToWrite != 0 &&
       (bytesWritten = socket->write(dataToWrite, bytesToWrite)) != -1)
  {
    if(bytesWritten == -1)
      return false;

    bytesToWrite -= bytesWritten;
    dataToWrite += bytesWritten;
  }
  return true;
}


static bool readMessageChunk(QAbstractSocket * socket, QByteArray & data,
                             quint32 messageSize, quint32 bytesAvailable)
{
  quint32 remainingSize = messageSize - data.size();
  quint32 bytesToRead = qMin(remainingSize, bytesAvailable);
  int currentSize = data.size();
  data.resize(data.size() + bytesToRead);
  char * dataPtr = data.data() + currentSize;

  qint64 bytesRead = 0;
  while(bytesToRead != 0 && (bytesRead = socket->read(dataPtr, bytesToRead)) != -1)
  {
    bytesToRead -= bytesRead;
    dataPtr += bytesRead;
  }

  if( bytesRead == -1 )
  {
    qWarning("NtgRpcMessage: error while reading");
    return false;
  }
  return true;
}


bool NtgRpcMessage::_readFromSocket(QAbstractSocket * socket, QByteArray & data)
{
  const int minimumHeaderSize = (int)sizeof(quint32);  // size
  quint32 messageSize = 0;
  data.clear();

  while(data.isEmpty() || data.size() != (int)messageSize)
  {
    quint32 bytesAvailable = (quint32)socket->bytesAvailable();
    if(bytesAvailable == 0 && ! socket->waitForReadyRead(TIMEOUT))
    {
      qCritical("NtgRpcMessage: Could not retrieve all data from socket");
      return false;
    }

    // still need message size
    if( messageSize == 0)
    {
      if( ! readMessageChunk(socket, data, minimumHeaderSize, bytesAvailable))
        return false;

      if( data.size() == minimumHeaderSize )
      {
        // we don't use datastreams here as we can't assume their representation
        // in a datastream fits in 4 bytes.
        messageSize = qFromBigEndian(*reinterpret_cast<const quint32*>(data.constData()));
        if (messageSize <= (quint32)minimumHeaderSize)
        {
          qWarning("NtgRpcMessage::_readFromSocket: message size invalid %u", messageSize);
          data.clear();
          return false;
        }
      }
    }
    else if( ! readMessageChunk(socket, data, messageSize, bytesAvailable))
    {
      return false;
    }
  }
  return true;
}


bool NtgRpcMessage::_serialize(QByteArray & block) const
{
  QDataStream in(&block,QIODevice::WriteOnly | QIODevice::Truncate);
  in.setVersion(Ntg::dataStreamVersion);
  quint32 size = 0;
  in.writeRawData((char*)&size,sizeof(quint32));
  in << Ntg::version << _messageType;

  switch(_messageType)
  {
    case NtgRpcMessage::INVOKE_MESSAGE:
    {
      in << _objName << _symbolName << _argList;
      break;
    }
    case NtgRpcMessage::SIGNAL_MESSAGE:
    {
      in << _objName << _symbolName << _argList;
      break;
    }
    case NtgRpcMessage::RESPONSE_MESSAGE:
    {
      in << _isError << _returnValue;
      break;
    }
    default:
    {
      qCritical("NtgRpcMessage: Unknown message type!");
      return false;
    }
  }
  in.device()->seek(0);
  size = qToBigEndian((quint32)block.size());
  in.writeRawData((char*)&size,sizeof(quint32));
  return _checkStream(in);
}


bool NtgRpcMessage::_unserialize(const QByteArray & block)
{
  QDataStream out(block);
  out.setVersion(Ntg::dataStreamVersion);

  quint32 size;
  quint16 version;
  out.readRawData((char*)&size, sizeof(quint32));
  size = qFromBigEndian(size);
  out >> version;
  if(size != (quint32)block.size() || version != Ntg::version)
    return false;

  out >> _messageType;

  switch(_messageType)
  {
    case NtgRpcMessage::INVOKE_MESSAGE:
    {
      out >> _objName >> _symbolName >> _argList;
      break;
    }
    case NtgRpcMessage::SIGNAL_MESSAGE:
    {
      out >> _objName >> _symbolName >> _argList;
      break;
    }
    case NtgRpcMessage::RESPONSE_MESSAGE:
    {
      out  >> _isError >> _returnValue;
      break;
    }
    default:
      return false;
  }
  return _checkStream(out);
}


bool NtgRpcMessage::_checkStream(const QDataStream & stream)
{
  return (stream.status() == QDataStream::Ok);
}


bool NtgRpcMessage::_checkMessageType(int messageType)
{
  if(messageType < 0 || messageType >= NtgRpcMessage::LAST_MESSAGE_TYPE)
  {
    qCritical("NtgRpcMessage: Invalid messageType!");
    return false;
  }
  return true;
}


bool NtgRpcMessage::_isMessageFull(const QByteArray & message)
{
  if(message.size() < (int)sizeof(quint32))
    return false;
  quint32 size = 0;
  QDataStream out(message);
  out.setVersion(Ntg::dataStreamVersion);
  out >> size;
  if(size != (quint32)message.size() || !_checkStream(out))
    return false;
  return true;
}


NtgRpcMessage* NtgRpcMessage::makeInvokeMessage(const QString & objName,
                                                const QByteArray & methodName,
                                                const QVariantList & argList)
{
  NtgRpcMessage* m = new NtgRpcMessage;
  if(m->_setMessageType(NtgRpcMessage::INVOKE_MESSAGE) &&
     m->setObjName(objName) &&
     m->setSymbolName(methodName) &&
     m->setArgList(argList)
     )
    return m;

  delete m;
  return NULL;
}


NtgRpcMessage* NtgRpcMessage::makeSignalMessage(const QString & objName,
                                                const QByteArray & signalName,
                                                const QVariantList & argList)
{
  NtgRpcMessage* m = new NtgRpcMessage;
  if(m->_setMessageType(NtgRpcMessage::SIGNAL_MESSAGE) &&
     m->setObjName(objName) &&
     m->setSymbolName(signalName) &&
     m->setArgList(argList))
    return m;

  delete m;
  return NULL;
}


NtgRpcMessage* NtgRpcMessage::makeValueResponseMessage(const QVariant & returnValue)
{
  NtgRpcMessage* m = new NtgRpcMessage;
  if(m->_setMessageType(NtgRpcMessage::RESPONSE_MESSAGE) &&
     m->setReturnValue(returnValue))
    return m;

  delete m;
  return NULL;
}


NtgRpcMessage* NtgRpcMessage::makeErrorResponseMessage(const QString & errorString)
{
  NtgRpcMessage* m = new NtgRpcMessage;
  if(m->_setMessageType(NtgRpcMessage::RESPONSE_MESSAGE) &&
     m->setErrorString(errorString))
    return m;

  delete m;
  return NULL;
}

//----- getters and setters ------

int NtgRpcMessage::messageType() const {return _messageType;}
bool NtgRpcMessage::_setMessageType(int messageType)
{
  if(!_checkMessageType(messageType))
  {
    qCritical("NtgRpcMessage: Invalid messageType!");
    return false;
  }
  _messageType = messageType;
  return true;
}


QString NtgRpcMessage::objName() const { return _objName;}
bool NtgRpcMessage::setObjName(const QString & name)
{
  if(name.isEmpty())
  {
    qCritical("NtgRpcMessage: Empty object name!");
    return false;
  }
  _objName = name;
  return true;
}


QByteArray NtgRpcMessage::symbolName() const {return _symbolName;}
bool NtgRpcMessage::setSymbolName(const QByteArray & name)
{
  if(name.isEmpty())
  {
    qCritical("NtgRpcMessage: Empty symbol name!");
    return false;
  }
  _symbolName = name;
  return true;
}


QVariantList NtgRpcMessage::argList() const {return _argList;}
bool NtgRpcMessage::setArgList(const QVariantList & argList)
{
  _argList = argList;
  return true;
}


QVariant NtgRpcMessage::returnValue() const {return (_isError ? QVariant() : _returnValue);}
bool NtgRpcMessage::setReturnValue(const QVariant & value)
{
  _isError = false;
  _returnValue = value;
  return true;
}


QString NtgRpcMessage::errorString() const {return (_isError ? _returnValue.toString() : QString());}
bool NtgRpcMessage::setErrorString(const QString & errorString)
{
  _isError = true;
  _returnValue = QVariant(errorString);
  return true;
}


bool NtgRpcMessage::isError() const {return _isError;}
