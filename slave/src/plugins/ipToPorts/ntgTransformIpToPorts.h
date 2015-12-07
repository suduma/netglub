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

#ifndef TRANSFORMIPTOPORTS_H
#define TRANSFORMIPTOPORTS_H

#include <QMetaType>

#include "ntgSafeProcess.h"
#include "ntgTransform.h"
#include "ntgTransformPlugin.h"

class NtgTransformIpToPorts : public NtgTransform, public NtgTransformPlugin
{
  Q_OBJECT
  Q_INTERFACES(NtgTransformPlugin)

  public:
    virtual QString transformType() const;
    virtual NtgTransform * createTransform() const;

    NtgTransformIpToPorts();
    virtual ~NtgTransformIpToPorts();

  protected slots:
    virtual void _exec(const NtgEntity & input, QHash<QString, QString> params);
    virtual void _cancel();

    void _parseOutput();
    void _finished( int exitCode, QProcess::ExitStatus exitStatus);
    void _error(QProcess::ProcessError error);

  protected:
    NtgSafeProcess _process;
    QByteArray _output;
};

#endif // TRANSFORMIPTOMACHINE_H
