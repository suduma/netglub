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

#ifndef NTGXMLRPCCALL_H
#define NTGXMLRPCCALL_H

#include <QObject>
#include "maiaXmlRpcClient.h"

class NtgXmlRpcCall : public QObject
{
  Q_OBJECT
  public:
    NtgXmlRpcCall(MaiaXmlRpcClient * client, bool throwExceptions = false);

    QVariant call(QString method, QVariantList args);

    QVariant call(QString method);
    QVariant call(QString method, QVariant arg0);
    QVariant call(QString method, QVariant arg0, QVariant arg1);
    QVariant call(QString method, QVariant arg0, QVariant arg1, QVariant arg2);
    QVariant call(QString method, QVariant arg0, QVariant arg1, QVariant arg2,
                  QVariant arg3);
    QVariant call(QString method, QVariant arg0, QVariant arg1, QVariant arg2,
                  QVariant arg3, QVariant arg4);
    QVariant call(QString method, QVariant arg0, QVariant arg1, QVariant arg2,
                  QVariant arg3, QVariant arg4, QVariant arg5);
    QVariant call(QString method, QVariant arg0, QVariant arg1, QVariant arg2,
                  QVariant arg3, QVariant arg4, QVariant arg5, QVariant arg6);
    QVariant call(QString method, QVariant arg0, QVariant arg1, QVariant arg2,
                  QVariant arg3, QVariant arg4, QVariant arg5, QVariant arg6,
                  QVariant arg7);
    QVariant call(QString method, QVariant arg0, QVariant arg1, QVariant arg2,
                  QVariant arg3, QVariant arg4, QVariant arg5, QVariant arg6,
                  QVariant arg7, QVariant arg8);
    QVariant call(QString method, QVariant arg0, QVariant arg1, QVariant arg2,
                  QVariant arg3, QVariant arg4, QVariant arg5, QVariant arg6,
                  QVariant arg7, QVariant arg8, QVariant arg9);

    bool failed() const;

    int error() const;
    QString errorString() const;

  private slots:
    void _resultSlot(QVariant & retVal);
    void _faultSlot(int error, const QString & message);

  private:
    MaiaXmlRpcClient * _client;
    QEventLoop _loop;
    bool _throwExceptions;
    QVariant _result;
    int _error;
    QString _errorString;
};

#endif // NTGXMLRPCCALL_H
