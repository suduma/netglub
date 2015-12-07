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

#ifndef NTGLOGGER_H
#define NTGLOGGER_H

#include <QString>
#include <QFile>

#define NTG_LOG(CAT,MSG) do {NtgLogger::instance()->log(CAT,MSG);} while(0);

class NtgLogger
{
  public:
    NtgLogger(QString logfile);
    virtual ~NtgLogger();

    static NtgLogger * instance();

    void log(const QString & category, const QString & msg) const;

  protected:
    static NtgLogger * _instance;

    QFile * _logfile;
};

#endif // NTGLOGGER_H
