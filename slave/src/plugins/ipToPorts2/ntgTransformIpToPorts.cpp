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

#include <QDebug>

#include "ntgTransformIpToPorts.h"
#include <QtPlugin>
#include <QDebug>

Q_EXPORT_PLUGIN2(ipToPorts2, NtgTransformIpToPorts)

QString NtgTransformIpToPorts::transformType() const
{
  return QString("ipToPorts2");
}


NtgTransform * NtgTransformIpToPorts::createTransform() const
{
  return new NtgTransformIpToPorts();
}



NtgTransformIpToPorts::NtgTransformIpToPorts()
  : NtgTransform()
{
}


NtgTransformIpToPorts::~NtgTransformIpToPorts()
{
}


void NtgTransformIpToPorts::_exec(const NtgEntity & input, QHash<QString, QString> params)
{
  if( (input.type != "ip-address" || ! input.values.contains("value")) &&
      (input.type != "ip-block" || ! input.values.contains("value")))
  {
    _setError(Ntg::InvalidInputError, "Type of input NtgEntity is neither \"ip-address\" nor \"ip-block\", or missing key\"value\".");
    return;
  }

  QStringList arguments;

  arguments << "-n" << "-oG" << "-";

  if(params.contains("agressiveness"))
    arguments << QString("-T%1").arg(params.value("agressiveness").toInt());

  arguments << input.values["value"];

  _process.setReadChannel(QProcess::StandardOutput);
  connect(&_process, SIGNAL(readyReadStandardOutput()), this, SLOT(_parseOutput()));
  connect(&_process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(_finished(int, QProcess::ExitStatus)));
  connect(&_process, SIGNAL(error(QProcess::ProcessError)), this, SLOT(_error(QProcess::ProcessError)));
  _process.start("nmap", arguments, QIODevice::ReadOnly);
  _setStatus(Ntg::Running);
}


void NtgTransformIpToPorts::_cancel()
{
  _process.kill();
}


static void ungetLine(QIODevice * dev, QByteArray line)
{
  for(int i = line.size()-1; i >= 0 ; i--)
    dev->ungetChar(line.at(i));
}


void NtgTransformIpToPorts::_parseOutput()
{
  _setProgress(50.0, "Parsing output...");
  QList<NtgEntity> results;
  QByteArray line;
  while( ! _process.atEnd())
  {
    if( ! _process.canReadLine())
      break;

    line = _process.readLine();
    if(line.startsWith("Host:"))
    {

      QRegExp re("Host: (\\d{1,3}(?:\\.\\d{1,3}){3})\\s+\\(.*\\)\\s+Ports:\\s+(.+)\\s+Ignored State:.+");
      if( ! re.exactMatch(QString(line)))
      {
        ungetLine(&_process, line);
        break;
      }
      QString host = re.cap(1);
      QStringList ports = re.cap(2).split(',');

      foreach(const QString & port, ports)
      {
        QStringList parts = port.split('/');
        QHash<QString,QString> values;
        values["value"] = QString::number(parts.at(0).toInt());
        values["protocol"] = parts.at(2);
        values["service"] = parts.at(4);
        values["ip-address"] = host;
        NtgEntity e("port", values );
        results.append(e);
      }
    }
  }
  _addResults(results);
}


void NtgTransformIpToPorts::_finished( int exitCode, QProcess::ExitStatus exitStatus)
{
  if( exitStatus == QProcess::NormalExit && exitCode == 0)
    _setStatus(Ntg::Finished);
  else
  {
    QString str = _process.readAllStandardError();
    _setError(Ntg::CustomError, str);
  }
}


void NtgTransformIpToPorts::_error(QProcess::ProcessError error)
{
  _setError(Ntg::CustomError, _process.errorString() );
}
