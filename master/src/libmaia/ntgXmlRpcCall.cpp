/******************************************************************************
**
** Copyright (C) 2009-2010 DIATEAM. All rights reserved.
**
** This file is part of the "Netglub" program.
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

#include "ntgXmlRpcCall.h"

NtgXmlRpcCall::NtgXmlRpcCall(MaiaXmlRpcClient * client, bool throwExceptions)
  : _client(client)
  , _throwExceptions(throwExceptions)
  , _error(0)
{
}


QVariant NtgXmlRpcCall::call(QString method, QVariantList args)
{
  _result.clear();
  _error = 0;
  _errorString.clear();
  _client->call(method, args, this, SLOT(_resultSlot(QVariant&)), this, SLOT(_faultSlot(int,QString)));
  _loop.exec(QEventLoop::ExcludeUserInputEvents);
  if( failed() && _throwExceptions)
    throw this;
  return _result;
}


QVariant NtgXmlRpcCall::call(QString method)
{
 QVariantList args;
 return call(method, args);
}


QVariant NtgXmlRpcCall::call(QString method, QVariant arg0)
{
 QVariantList args;
 args << arg0;
 return call(method, args);
}


QVariant NtgXmlRpcCall::call(QString method, QVariant arg0, QVariant arg1)
{
 QVariantList args;
 args << arg0 << arg1;
 return call(method, args);
}


QVariant NtgXmlRpcCall::call(QString method, QVariant arg0, QVariant arg1, QVariant arg2)
{
 QVariantList args;
 args << arg0 << arg1 << arg2;
 return call(method, args);
}


QVariant NtgXmlRpcCall::call(QString method, QVariant arg0, QVariant arg1, QVariant arg2,
              QVariant arg3)
{
 QVariantList args;
 args << arg0 << arg1 << arg2 << arg3;
 return call(method, args);
}


QVariant NtgXmlRpcCall::call(QString method, QVariant arg0, QVariant arg1, QVariant arg2,
              QVariant arg3, QVariant arg4)
{
 QVariantList args;
 args << arg0 << arg1 << arg2 << arg3 << arg4;
 return call(method, args);
}


QVariant NtgXmlRpcCall::call(QString method, QVariant arg0, QVariant arg1, QVariant arg2,
              QVariant arg3, QVariant arg4, QVariant arg5)
{
 QVariantList args;
 args << arg0 << arg1 << arg2 << arg3 << arg4 << arg5;
 return call(method, args);
}


QVariant NtgXmlRpcCall::call(QString method, QVariant arg0, QVariant arg1, QVariant arg2,
              QVariant arg3, QVariant arg4, QVariant arg5, QVariant arg6)
{
 QVariantList args;
 args << arg0 << arg1 << arg2 << arg3 << arg4 << arg5 << arg6;
 return call(method, args);
}


QVariant NtgXmlRpcCall::call(QString method, QVariant arg0, QVariant arg1, QVariant arg2,
              QVariant arg3, QVariant arg4, QVariant arg5, QVariant arg6,
              QVariant arg7)
{
 QVariantList args;
 args << arg0 << arg1 << arg2 << arg3 << arg4 << arg5 << arg6 << arg7;
 return call(method, args);
}


QVariant NtgXmlRpcCall::call(QString method, QVariant arg0, QVariant arg1, QVariant arg2,
              QVariant arg3, QVariant arg4, QVariant arg5, QVariant arg6,
              QVariant arg7, QVariant arg8)
{
 QVariantList args;
 args << arg0 << arg1 << arg2 << arg3 << arg4 << arg5 << arg6 << arg7 << arg8;
 return call(method, args);
}


QVariant NtgXmlRpcCall::call(QString method, QVariant arg0, QVariant arg1,
                             QVariant arg2, QVariant arg3, QVariant arg4,
                             QVariant arg5, QVariant arg6, QVariant arg7,
                             QVariant arg8, QVariant arg9)
{
 QVariantList args;
 args << arg0 << arg1 << arg2 << arg3 << arg4 << arg5 << arg6 << arg7 << arg8 << arg9;
 return call(method, args);
}


bool NtgXmlRpcCall::failed() const
{
  return (_error != 0);
}


int NtgXmlRpcCall::error() const
{
  return _error;
}


QString NtgXmlRpcCall::errorString() const
{
  return _errorString;
}


void NtgXmlRpcCall::_resultSlot(QVariant & retVal)
{
  _result = retVal;
  _loop.quit();
}


void NtgXmlRpcCall::_faultSlot(int error, const QString & message)
{
  _error = error;
  _errorString = message;
  _loop.quit();
}
