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

#ifndef TRANSFORMMANAGER_H
#define TRANSFORMMANAGER_H

#include <QObject>
#include <QStringList>
#include <QSharedPointer>
#include "ntgTransform.h"
#include "ntgRpcHandler.h"

class NtgTransformPlugin;

struct NtgTransformInfo
{
  NtgTransformPlugin * plugin;
  QString workingDirPath;
  NtgTransformModel model;
  QHash<QString, QString> values;
};


class NtgTransformManager : public QObject
{
  Q_OBJECT
  public:
    NtgTransformManager(QString xmlDirPath, QString pluginPath, NtgRpcHandler * handler);
    virtual ~NtgTransformManager();

    QStringList transformList() const;
    NtgTransform * createTransformFromType(QString type);

  public slots:
    QString createTransform(QString transformType);
    bool destroyTransform(QString objName);
    QStringList currentTransforms(QString transformType = "");
    QList<NtgTransformModel> models();

  protected slots:
    void _newStatus(Ntg::TransformStatus status);
    void _newProgress(double progressPercent, const QString & progressText);
    void _newResults(int fromIndex, int toIndex);

  protected:
    void _loadPlugins(QString path);
    void _createModelsFromXml(QString path);

  protected:
    NtgRpcHandler * _handler;
    QHash<QString, NtgTransformInfo> _transformInfos;
    QHash<QString, NtgTransform*> _currentTransforms;
    QHash<QString, NtgTransformPlugin*> _plugins;
};

#endif // TRANSFORMMANAGER_H
