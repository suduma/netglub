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

#ifndef NTGMETHODPROVIDER_H
#define NTGMETHODPROVIDER_H

#include <QObject>
#include <QHash>
#include <QMutex>

#include "ntgSessionManager.h"

namespace Ntg {
  enum MethodAccess
  {
    METHOD_VALID = 0, METHOD_UNAUTHORIZED = 1, METHOD_UNKNOWN = 2,
  };
}


class NtgMethodProvider : public QObject
{
  Q_OBJECT
  public:
    NtgMethodProvider(NtgSessionManager * sessionAuth);

    bool registerMethod(const QString & name, Ntg::AuthLevel level,
                        QObject * object, const char * slot, QString helpText = "");

    Ntg::MethodAccess getMethod(QString sessionId, QString method,
                                QObject ** responseObject, const char ** responseSlot);

    QStringList listMethods() const;
    QList<QList<QByteArray> > methodSignature(QString methodName) const;
    QString methodHelp(QString methodName) const;

    NtgSessionManager * sessionManager() const;

  protected:
    struct MethodInfo
    {
      MethodInfo() : level(Ntg::AUTH_NONE), object(NULL), method(NULL) {}

      MethodInfo(QString name_, Ntg::AuthLevel level_, QObject * object_, const char * method_, QString helpText_)
        : name(name_), level(level_), object(object_), method(qstrdup(method_)), helpText(helpText_) {}

      MethodInfo(const MethodInfo & m)
        : name(m.name), level(m.level), object(m.object), method(qstrdup(m.method)), helpText(m.helpText) {}

      ~MethodInfo() {delete[] method;}

      QString name;
      Ntg::AuthLevel level;
      QObject * object;
      char * method;
      QString helpText;
    };

    QHash<QString, MethodInfo> _methods;
    NtgSessionManager * _sessionAuth;

    mutable QMutex _mutex;
};

#endif // NTGMETHODPROVIDER_H
