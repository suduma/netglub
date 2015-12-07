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


#ifndef TRANSFORMGENERIC_H
#define TRANSFORMGENERIC_H

#include <QProcess>
#include <QMetaType>

#include "localtransformplugin.h"
#include "localtransformmanager.h"

class NtgTransformGeneric : public LocalTransform, public LocalTransformPlugin
{
  Q_OBJECT
  Q_INTERFACES(LocalTransformPlugin)

  public:
    virtual QString transformType() const;
    virtual LocalTransform * createTransform() const;

    NtgTransformGeneric();
    virtual ~NtgTransformGeneric();

  protected:
    void _writeInput(QProcess * process, const NtgEntity & input,
                     const QHash<QString,QString> & params);

    virtual void _exec(const NtgEntity & input, QHash<QString, QString> params);
    virtual void _cancel();

  protected slots:
    void _parseOutput();
    void _finished( int exitCode, QProcess::ExitStatus exitStatus);
    void _error(QProcess::ProcessError error);

  protected:
    LocalSafeProcess _process;
    QByteArray _output;
};

#endif // TRANSFORMGENERIC_H
