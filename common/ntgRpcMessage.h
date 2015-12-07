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

#ifndef NTGMESSAGE_H
#define NTGMESSAGE_H

#include <QString>
#include <QDataStream>
#include <QAbstractSocket>
#include <QVariant>

namespace Ntg
{
  const quint16 version = 0;
  const QDataStream::Version dataStreamVersion = QDataStream::Qt_4_5;
}


class NtgRpcMessage
{
  public:
    enum
    {
      INVOKE_MESSAGE    = 0,
      SIGNAL_MESSAGE    = 1,
      RESPONSE_MESSAGE  = 2,

      LAST_MESSAGE_TYPE = 3
    };

    virtual ~NtgRpcMessage();

    static NtgRpcMessage* readFromSocket(QAbstractSocket * socket);
    bool writeToSocket(QAbstractSocket * socket) const;

    static NtgRpcMessage* makeInvokeMessage(const QString & objName,
                                         const QByteArray & methodName,
                                         const QVariantList & argList);

    static NtgRpcMessage* makeSignalMessage(const QString & objName,
                                         const QByteArray & signalName,
                                         const QVariantList & argList);

    static NtgRpcMessage* makeValueResponseMessage(const QVariant & returnValue = QVariant());

    static NtgRpcMessage* makeErrorResponseMessage(const QString & errorString);


    int messageType() const;

    QString objName() const;
    bool setObjName(const QString & name);

    QByteArray symbolName() const;
    bool setSymbolName(const QByteArray & name);

    QVariantList argList() const;
    bool setArgList(const QVariantList & argList);

    QVariant returnValue() const;
    bool setReturnValue(const QVariant & value);

    QString errorString() const;
    bool setErrorString(const QString & errorString);

    bool isError() const;

  protected:
    NtgRpcMessage();

    bool _setMessageType(int messageType);

    bool _serialize(QByteArray & block) const;
    bool _unserialize(const QByteArray & block);

    static bool _checkStream(const QDataStream & stream);
    static bool _checkMessageType(int messageType);
    static bool _isMessageFull(const QByteArray & message);

    static bool _readFromSocket(QAbstractSocket * socket, QByteArray & data);
    static bool _writeToSocket(QAbstractSocket * socket, const QByteArray & data);

  protected:
    int _messageType;

    QString _objName;
    QByteArray _symbolName; // either method or signal name
    QVariantList _argList;

    bool _isError;
    QVariant _returnValue; // or errorString if isError == true
};

#endif // NTGMESSAGE_H
