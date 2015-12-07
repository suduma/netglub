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

#include "ntgTransformDomainNameToIp.h"

#include <QtPlugin>
#include <QTextStream>
#include <QDebug>

Q_EXPORT_PLUGIN2(domainToIp, NtgTransformDomainNameToIp)


QString NtgTransformDomainNameToIp::transformType() const
{
  return QString("domainToIp");
}


NtgTransform * NtgTransformDomainNameToIp::createTransform() const
{
  return new NtgTransformDomainNameToIp();
}


NtgTransformDomainNameToIp::NtgTransformDomainNameToIp()
  : NtgTransform()
{
  _process.dropPrivileges(true);
}


NtgTransformDomainNameToIp::~NtgTransformDomainNameToIp()
{
}


void NtgTransformDomainNameToIp::_exec(const NtgEntity & input, QHash<QString, QString> params)
{
  if( input.type != "domain-name" || ! input.values.contains("value"))
  {
    _setError(Ntg::InvalidInputError, "Type of input Entity is not\"domain-name\", or missing key \"value\".");
    return;
  }

  QStringList arguments;
  if(params.contains("server port"))
    arguments << QString("-port=%1").arg(params.value("server port"));
  if(params.contains("timeout"))
    arguments << QString("-timeout=%1").arg(params.value("timeout"));

  arguments << input.values["value"];

  if(params.contains("server"))
    arguments << params.value("server");

  _process.setReadChannel(QProcess::StandardOutput);
  connect(&_process, SIGNAL(readyReadStandardOutput()), this, SLOT(_parseOutput()));
  connect(&_process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(_finished(int, QProcess::ExitStatus)));
  connect(&_process, SIGNAL(error(QProcess::ProcessError)), this, SLOT(_error(QProcess::ProcessError)));
  _process.start("nslookup", arguments, QIODevice::ReadOnly);
  _setStatus(Ntg::Running);
}


void NtgTransformDomainNameToIp::_cancel()
{
  _process.kill();
}


static void ungetLine(QIODevice * dev, QByteArray line)
{
  for(int i = line.size()-1; i >= 0 ; i--)
    dev->ungetChar(line.at(i));
}


void NtgTransformDomainNameToIp::_parseOutput()
{
  _setProgress(50.0, "Parsing output...");
  QList<NtgEntity> results;
  QByteArray line;
  while( ! _process.atEnd())
  {
    if(_process.canReadLine())
    {
      line = _process.readLine();
      if(line.startsWith("Name:"))
      {
        if(_process.canReadLine())
        {
          line = _process.readLine();
          line = line.mid(9);line.chop(1);
          qDebug() << "IP: " << line;
          QHash<QString,QString> values;
          values["value"] = line;
          NtgEntity e("ip-address", values );
          results.append(e);
        }
        else
        {
          // incomplete IP line, unget Name line
          ungetLine(&_process, line);
          break;
        }
      }
    }
  }
  _addResults(results);
  _setProgress(100.0, "Finished");
}


void NtgTransformDomainNameToIp::_finished( int exitCode, QProcess::ExitStatus exitStatus)
{
  if( exitStatus == QProcess::NormalExit && exitCode == 0)
    _setStatus(Ntg::Finished);
  else
  {
    QString str = _process.readAllStandardError();
    _setError(Ntg::CustomError, str);
  }
}


void NtgTransformDomainNameToIp::_error(QProcess::ProcessError error)
{
  _setError(Ntg::CustomError, _process.errorString() );
}
