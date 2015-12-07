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

#include "ntgMethodProvider.h"
#include <QMetaMethod>
#include <QStringList>


NtgMethodProvider::NtgMethodProvider(NtgSessionManager * sessionAuth)
  : _sessionAuth(sessionAuth)
  , _mutex(QMutex::Recursive)
{
}


bool NtgMethodProvider::registerMethod(const QString & name,
                                       Ntg::AuthLevel level,
                                       QObject * object,
                                       const char * slot,
                                       QString helpText)
{
  QMutexLocker locker(&_mutex);
  if( _methods.contains(name) )
    return false;
  MethodInfo info(name, level, object, QMetaObject::normalizedSignature(slot).constData(), helpText);
  _methods.insert(name, info);
  return true;
}


Ntg::MethodAccess NtgMethodProvider::getMethod(QString sessionId,
                                               QString method,
                                               QObject ** object,
                                               const char ** slot)
{
  QMutexLocker locker(&_mutex);
  if( ! _methods.contains(method) )
    return Ntg::METHOD_UNKNOWN;
  const MethodInfo & mInfo = _methods[method];

  NtgSession * session = _sessionAuth->getSession(sessionId);
  Ntg::AuthLevel currentLevel = Ntg::AUTH_NONE;
  if( session )
    currentLevel = session->level;
  if( currentLevel >= mInfo.level)
  {
    *object = mInfo.object;
    *slot = mInfo.method;
    return Ntg::METHOD_VALID;
  }
  return Ntg::METHOD_UNAUTHORIZED;
}


QStringList NtgMethodProvider::listMethods() const
{
  QMutexLocker locker(&_mutex);
  return _methods.keys();
}


QList<QList<QByteArray> > NtgMethodProvider::methodSignature(QString methodName) const
{
  QMutexLocker locker(&_mutex);
  QList<QList<QByteArray> > sigList;

  if( ! _methods.contains(methodName) )
    return sigList;

  QObject * obj = _methods.value(methodName).object;
  QByteArray method = _methods.value(methodName).method;

  const QMetaObject * mObj = obj->metaObject();
  for(int n = 0; n < mObj->methodCount(); ++n)
  {
    QMetaMethod m = mObj->method(n);
    QByteArray sig = m.signature();

    int offset = sig.indexOf('(');
    if(offset == -1)
      continue;
    QByteArray name = sig.mid(0, offset);
    if(name != method)
      continue;

    sigList.append(m.parameterTypes());
  }
  return sigList;
}


QString NtgMethodProvider::methodHelp(QString methodName) const
{
  QMutexLocker locker(&_mutex);
  if( ! _methods.contains(methodName) )
    return QString();
  return _methods.value(methodName).helpText;
}


NtgSessionManager * NtgMethodProvider::sessionManager() const
{
  QMutexLocker locker(&_mutex);
  return _sessionAuth;
}
