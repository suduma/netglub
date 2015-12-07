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

#include "ntgLogger.h"

#include <QDateTime>

NtgLogger * NtgLogger::_instance = NULL;

NtgLogger::NtgLogger(QString logfile)
{
  if( _instance )
    qFatal("Only one logger allowed");

  _logfile = new QFile(logfile);
  if( ! _logfile->open(QIODevice::WriteOnly | QIODevice::Append ) )
    qFatal("NtgLogger: Could not open log file %s : %s", qPrintable(logfile), qPrintable(_logfile->errorString()));
  QByteArray sep(78, '-');
  _logfile->write(sep);
  _logfile->write("\n");

  _instance = this;
}


NtgLogger::~NtgLogger()
{
  if( _logfile )
    _logfile->close();
  delete _logfile;
  _instance = NULL;
}


NtgLogger * NtgLogger::instance()
{
  return _instance;
}


void NtgLogger::log(const QString & category, const QString & msg) const
{
  QString dateStr = QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss.zzz");
  QString catStr = ( ! category.isEmpty() ) ? category : "No Category";
  QString log("%1 [%2] %3\n");
  QByteArray data = log.arg(dateStr).arg(catStr).arg(msg).toLocal8Bit();
  if( ! this ) // very ugly, but it's safe. I think.
  {
    fprintf(stderr, "%s", data.constData());
  }
  else
  {
    if( _logfile->write(data) != data.size() )
      qCritical("Could not write log line to log file %s", qPrintable(_logfile->fileName()));
    _logfile->flush();
  }
}
