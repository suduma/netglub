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

#ifndef TRANSFORM_H
#define TRANSFORM_H

#include <QObject>
#include <QHash>
#include <QStringList>

#include "ntgTypes.h"

class NtgTransformManager;
class NtgRpcHandler;

class NtgTransform : public QObject
{
  friend class NtgTransformManager;
  Q_OBJECT

  public slots:

    void exec(const NtgEntity & input, HashStringString params);
    void cancel();

    Ntg::TransformStatus status() const;

    double progressValue () const;
    QString progressText () const;

    Ntg::TransformError error() const;
    QString errorString() const;

    int latestResultIndex() const;
    QList<NtgEntity> results(int startIndex, int toIndex = -1) const;

    const NtgTransformModel & model() const;
    QString type() const;
    const NtgEntity & inputEntity() const;

  public:
    QString workDirPath() const;
    void setHandler(NtgRpcHandler * handler);

    QHash<QString,QString> configValues() const;
    bool configValueAsBool(const QString & name, bool * value) const;
    // numbers beginning with '0x' are read as hex, '0' as octal, else as decimal.
    bool configValueAsInt(const QString & name, int * value) const;
    bool configValueAsDouble(const QString & name, double * value) const;
    bool configValueAsString(const QString & name, QString * value) const;

  signals:

    void statusChanged(Ntg::TransformStatus status);
    void progressChanged(double progressPercent, const QString & progressText);
    void resultReadyAt(int fromIndex, int toIndex);

  protected:
    NtgTransform();
    virtual ~NtgTransform();

    virtual void _exec(const NtgEntity & input, QHash<QString, QString> params) = 0;
    virtual void _cancel() = 0;

    void _setStatus(Ntg::TransformStatus status);
    void _setError(Ntg::TransformError error, QString msg);
    void _setProgressMax(int max);
    void _setProgress(double progress, QString progressStr = QString());
    void _addResult(NtgEntity entity);
    void _addResults(QList<NtgEntity> entities);

  private:
    void _reset();

    NtgRpcHandler * _handler;
    NtgTransformModel _model;
    QString _workDirPath;
    QHash<QString,QString> _configValues;

    Ntg::TransformStatus _status;
    Ntg::TransformError _error;
    QString _errorString;
    double _progressPercent;
    QString _progressText;
    QList<NtgEntity> _resultList;
    NtgEntity _inputEntity;

    static quint32 _instanceCounter;
};
#endif // TRANSFORM_H
