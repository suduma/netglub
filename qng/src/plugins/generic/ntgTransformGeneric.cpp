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

#include "ntgTransformGeneric.h"
#include <QtPlugin>
#include <QFileInfo>
#include <QDebug>

Q_EXPORT_PLUGIN2(generic, NtgTransformGeneric)

QString NtgTransformGeneric::transformType() const
{
  return QString("generic");
}


LocalTransform * NtgTransformGeneric::createTransform() const
{
  return new NtgTransformGeneric();
}


NtgTransformGeneric::NtgTransformGeneric()
  : LocalTransform()
{
}


NtgTransformGeneric::~NtgTransformGeneric()
{
}


static QByteArray dumpString(QString str)
{
  QByteArray data;
  data.append('"');
  data.append(str.replace('"', '\"').toLocal8Bit());
  data.append('"');
  return data;
}


static QByteArray dumpHash(const QHash<QString,QString> & hash)
{
  QByteArray data;
  QHash<QString,QString>::const_iterator it = hash.constBegin();
  for(;it != hash.constEnd(); ++it)
  {
    if(it != hash.constBegin())
      data.append(',');
    data.append(dumpString(it.key()));
    data.append(':');
    data.append(dumpString(it.value()));
  }
  return data;
}


void NtgTransformGeneric::_writeInput(QProcess * process, const NtgEntity & input,
                                   const QHash<QString,QString> & params)
{
  QByteArray data("Input ");
  data.append(dumpString(input.type));
  data.append(' ');
  data.append(dumpHash(input.values));
  data.append(' ');
  data.append(dumpHash(params));
  data.append('\n');

  if(process->write(data) != data.size())
  {
    QString err("Could not write input to transform \"\%1\"");
    _setError(Ntg::CustomError, err.arg(model().name));
  }
}


static QStringList dumpToArgs(const NtgEntity & e, const QHash<QString,QString> & params)
{
  QStringList args;
  args.append(e.type);
  QHash<QString,QString>::const_iterator it = e.values.constBegin();
  for(;it != e.values.constEnd(); ++it)
  {
    args.append(it.key());
    args.append(it.value());
  }
  args.append(QString());
  it = params.constBegin();
  for(;it != params.constEnd(); ++it)
  {
    args.append(it.key());
    args.append(it.value());
  }
  return args;
}


void NtgTransformGeneric::_exec(const NtgEntity & input, QHash<QString, QString> params)
{
  bool needRoot = false;
  configValueAsBool("need_root", &needRoot);
  _process.dropPrivileges(!needRoot);

  bool inputAsArgs = true;
  configValueAsBool("input_as_args", &inputAsArgs);

  foreach(const QString & key, configValues().keys())
  {
    if( key.startsWith("param_"))
    {
      QString realKey = key.mid(6);
      QString val;
      qDebug("key: %s", qPrintable(realKey));
      if( ! configValueAsString(key,&val))
        continue;
      params[realKey] = val;
    }
  }

  QFileInfo fInfo(workDirPath() + "/transform");
  if( ! fInfo.exists())
  {
    QString err("Missing 'transform' script in %1 for transform %2");
    _setError(Ntg::InvalidScriptError, err.arg(workDirPath(), model().name) );
    return;
  }

  if( ! fInfo.isReadable() || ! fInfo.isExecutable())
  {
    QString err("'transform' script in %1 for transform %2 has invalid privileges (missing 'execute' or 'read')");
    _setError(Ntg::InvalidScriptError, err.arg(workDirPath(), model().name) );
    return;
  }

  _process.setReadChannel(QProcess::StandardOutput);
  connect(&_process, SIGNAL(readyReadStandardOutput()), this, SLOT(_parseOutput()));
  connect(&_process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(_finished(int, QProcess::ExitStatus)));
  connect(&_process, SIGNAL(error(QProcess::ProcessError)), this, SLOT(_error(QProcess::ProcessError)));
  _process.setWorkingDirectory(workDirPath());

  QStringList args;
  if(inputAsArgs)
    args = dumpToArgs(input, params);

  _process.start("./transform", args, QIODevice::ReadWrite);
  _setStatus(Ntg::Running);

  if( ! inputAsArgs)
    _writeInput(&_process, input, params);
}


void NtgTransformGeneric::_cancel()
{
  _process.terminate();
  _process.waitForFinished(1000);
  _process.kill();
}


static bool removeWhitespace(QString & line)
{
  if(line.isEmpty())
  {
    qWarning("WS: Empty line !");
    return false;
  }
  int i = 0;
  while(line.at(i).isSpace())
  {
    i++;
    if(i >= line.size())
    {
      line.clear();
      return true;
    }
  }
  line = line.mid(i,-1);
  return true;
}


static bool readString(QString & line, QString & str)
{
  if( ! removeWhitespace(line) || line.isEmpty())
    return false;
  if(line.at(0) != '"')
  {
    qWarning("RS: Invalid char in line:\n%s", qPrintable(line));
    return false;
  }

  str = "";
  int i=1,i2=0;
  while( true )
  {
    i2 = line.indexOf('"', i2+1);
    if(i2 == -1)
    {
      qWarning("RS: No end \" for string from pos %d in line:\n%s", i, qPrintable(line));
      return false;
    }
    str = line.mid(i, i2-i);
    if(line.at(i2-1) == '\\')
      str.chop(1);
    else
      break;
  }
  line.remove(0,i2+1);
  return true;
}


static bool readHash(QString & line, QHash<QString,QString> & hash)
{
  while( true )
  {
    QString key, val;
    if( ! readString(line, key) )
      return false;
    if( ! removeWhitespace(line) || line.isEmpty())
      return false;
    if(line.at(0) != ':')
    {
      qWarning("RH: No ':' for hash in line:\n%s", qPrintable(line));
      return false;
    }
    line.remove(0,1);
    if( ! readString(line, val) )
      return false;
    hash.insert(key, val);
    if( ! removeWhitespace(line))
      return false;
    if( line.isEmpty() || line.at(0) != ',')
      break;
    line = line.remove(0,1);
  }
  return true;
}


static bool readNumber(QString & line, double & d)
{
  if( ! removeWhitespace(line) || line.isEmpty() )
    return false;
  int i = 0;
  int offset = 1;
  while( (i+offset) < line.size() && ! line.at(i+offset).isSpace())
    offset++;
  bool ok;
  QString numStr = line.mid(i,offset);
  d = numStr.toDouble(&ok);
  if( ! ok )
  {
    qWarning("RN: could not convert '%s' to number", qPrintable(numStr));
    return false;
  }
  line.remove(0,i+offset);
  return true;
}


void NtgTransformGeneric::_parseOutput()
{
  QList<NtgEntity> results;
  QByteArray line;
  QRegExp re("^\\s*(Result|Error|Progress)\\s+(.+)$", Qt::CaseInsensitive);
  while( ! _process.atEnd())
  {
    if( ! _process.canReadLine())
      break;

    line = _process.readLine();
    if( re.indexIn(QString::fromUtf8(line.constData(), line.size())) == -1)
    {
      qWarning("Invalid line:\n%s", line.constData());
      continue;
    }
    QString lineType = re.cap(1).toLower();
    QString lineData = re.cap(2);
    if(lineType == "result")
    {
      NtgEntity result;
      if( ! readString(lineData, result.type) ||
          ! readHash(lineData, result.values))
      {
        qCritical("Failed to parse Result line:\n%s", line.constData());
        continue;
      }
      results << result;
    }
    else if(lineType == "error")
    {
      QString errStr;
      if( ! readString(lineData, errStr))
      {
        qCritical("Failed to parse Error line:\n%s", line.constData());
        continue;
      }
      _setError(Ntg::CustomError, errStr);
      _process.close();
      break;
    }
    else if(lineType == "progress")
    {
      QString progressText;
      double progressValue;
      if( ! readNumber(lineData, progressValue) ||
          ! readString(lineData, progressText))
      {
        qCritical("Failed to parse Progress line:\n%s", line.constData());
        continue;
      }
      //flush results
      _addResults(results);
      results.clear();
      _setProgress(progressValue, progressText);
    }
  }
  _addResults(results);
}


void NtgTransformGeneric::_finished( int exitCode, QProcess::ExitStatus exitStatus)
{
  if( _process.bytesAvailable() != 0)
    _parseOutput();
  if( exitStatus == QProcess::NormalExit && exitCode == 0)
    _setStatus(Ntg::Finished);
  else
  {
    QString str = _process.readAllStandardError();
    _setError(Ntg::CustomError, str);
  }
}


void NtgTransformGeneric::_error(QProcess::ProcessError )
{
  if( _process.bytesAvailable() != 0)
    _parseOutput();
  _setError(Ntg::CustomError, _process.errorString() );
}
