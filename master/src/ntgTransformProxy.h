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

#ifndef NTGTRANSFORMPROXY_H
#define NTGTRANSFORMPROXY_H

#include <QObject>
#include <QMutex>

#include "ntgTypes.h"
#include "ntgRpcHandler.h"
#include "ntgSlaveProxy.h"

/****************************************************************************
**  class NtgTransformCache
****************************************************************************/

struct NtgOutputCache
{
    NtgOutputCache(){};

    QString transformName;
    NtgEntity inputEntity;
    QList<NtgEntity> ouputEntitiesList;
};


class NtgTransformCache : public QObject
{
public:
    static NtgTransformCache * getOrCreate();
    void addTocache(QString transformName,
                    NtgEntity inputEntity,
                    QList<NtgEntity> ouputEntitiesList);

    int contains(QString transformName, NtgEntity inputEntity);
    QList<NtgEntity> getFromCache(QString transformName, NtgEntity inputEntity);
    void clearAll();

private:
    NtgTransformCache();
    virtual ~NtgTransformCache();

    QList<NtgOutputCache*> _cache;
    mutable QMutex _mutex;

    static NtgTransformCache * _transformCache;
};

/****************************************************************************
**  class NtgTransformProxy
****************************************************************************/

class NtgTransformProxy : public QObject
{
  Q_OBJECT
  public:
    NtgTransformProxy(NtgSlaveProxy * slaveProxy, NtgRpcHandler * handler, const QString & objName,
                      const NtgTransformModel & model, const QString & sessionId,
                      bool enableCache);
    ~NtgTransformProxy();

  public slots:
    int id() const;
    QString sessionId() const;

    void exec(const NtgEntity & input, QHash<QString, QString> params);
    void cancel();

    Ntg::TransformStatus status() const;

    double progressValue () const;
    QString progressText () const;

    Ntg::TransformError error() const;
    QString errorString() const;

    int latestResultIndex() const;
    QList<NtgEntity> results(int startIndex, int toIndex = -1) const;

    const NtgTransformModel & model() const;
    QString name() const;
    const NtgEntity & inputEntity() const;

  public:
    static QList<NtgTransformProxy*> transformsInSession(QString sessionId);
    static NtgTransformProxy * getTransform(int id);
    NtgSlaveProxy * slaveProxy();
    QString objName();

  signals:
    void statusChanged(Ntg::TransformStatus status);
    void progressChanged(double progressPercent, const QString & progressText);
    void resultReadyAt(int fromIndex, int toIndex);

  protected slots:
    void _statusChanged(Ntg::TransformStatus status);
    void _progressChanged(double progressPercent, const QString & progressText);
    void _resultReadyAt(int fromIndex, int toIndex);

  private:
    void _reset();

    int _id;
    QString _sessionId;
    NtgRpcHandler * _handler;
    QString _objName;
    NtgTransformModel _model;
    bool _enableCache;
    bool _takeFromCache;

    Ntg::TransformStatus _status;
    Ntg::TransformError _error;
    QString _errorString;
    double _progressPercent;
    QString _progressText;
    QList<NtgEntity> _resultList;
    mutable NtgEntity _inputEntity;
    NtgEntity _inputEntityFixed;

    static int _idCount;
    NtgSlaveProxy * _slaveProxy;
};

#endif // NTGTRANSFORMPROXY_H
