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

#ifndef NTGSLAVEPROXY_H
#define NTGSLAVEPROXY_H

#include <QObject>

#include "ntgTypes.h"

class NtgRpcHandler;
class NtgTransformProxy;
class NtgSlaveCreds;

class NtgSlaveProxy : public QObject
{
  Q_OBJECT
  public:
    NtgSlaveProxy(NtgRpcHandler * handler);
    virtual ~NtgSlaveProxy();

    static NtgSlaveProxy * makeProxy(NtgRpcHandler * handler, QHash<QString, NtgTransformModel*> masterTransformsHash);

    void updateModels(QHash<QString, NtgTransformModel*> masterTransformsHash);
    QList<NtgTransformModel> models() const;
    bool hasTransform(QString type) const;
    QStringList availableTransforms() const;

    NtgTransformProxy * createTransform(QString sessionId, QString transformType, bool enableCache);
    bool destroyTransform(NtgTransformProxy * proxy);
    QList<NtgTransformProxy *> currentTransforms(QString transformType = "");

  protected:
    NtgRpcHandler * _handler;
    QHash<QString, NtgTransformModel> _models;
    QHash<int, NtgTransformProxy*> _transformProxies;

    static void _initTypes();
    static bool _typesInited;
    static int _instanceCounter;
};
#endif // NTGSLAVEPROXY_H
