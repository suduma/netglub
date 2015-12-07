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

#ifndef NTGRPCHANDLER_H
#define NTGRPCHANDLER_H

#include <QObject>
#include <QVariant>

class NtgRpcTransport;
class NtgRpcMessage;

class NtgRpcHandler : public QObject
{
  Q_OBJECT
  public:
    NtgRpcHandler();
    virtual ~NtgRpcHandler();

    bool registerInvokableObject(QObject * obj);
    bool registerSignalProxy(QObject * obj,
                             const QString & proxiedObjName);
    bool unRegisterSignalProxy(QObject * obj,
                               const QString & proxiedObjName);

    bool emitSignal(QObject * obj,
                    const QString & signalName,
                    const QVariantList & argList);

    bool invokeMethod(const QString & objName,
                      const QString & methodName,
                      const QVariantList & argList,
                      QVariant * returnValue = NULL);

    QString lastError() const;
    bool failed() const;

    void setTransport(NtgRpcTransport * transport);
    NtgRpcMessage * processMessage(NtgRpcMessage * msg);

  private:
    QHash<QString, QObject*> _objects;
    QMultiHash<QString, QObject*> _signalProxies;
    NtgRpcTransport * _transport;
    QString _lastError;
};

#endif // NTGRPCHANDLER_H
