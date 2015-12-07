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

#include "ntgTransform.h"
#include "ntgRpcHandler.h"

quint32 NtgTransform::_instanceCounter = 0;

NtgTransform::NtgTransform()
  : _handler(NULL)
{
  setObjectName(QString("NtgTransform.%1").arg(_instanceCounter++));
  _status = Ntg::NotStarted;
  _progressPercent = 0;
  _progressText = "Not running";
  _error = Ntg::NoError;
}


NtgTransform::~NtgTransform()
{
  cancel();
}


void NtgTransform::exec(const NtgEntity & input, HashStringString params)
{
  if( _status == Ntg::Running)
  {
    qCritical("NtgTransform %s already running ! Not doing anything.", qPrintable(objectName()));
    return;
  }
  _reset();

  if( ! _model.inputTypes.contains(input.type) )
  {
    QString err("Received input type \"%1\" is not handled by transform \"%2\"!");
    _setError(Ntg::InvalidInputError, err.arg(input.type).arg(_model.name));
    return;
  }

  foreach(const NtgTransformParam & param, _model.params)
  {
    if( ! params.contains(param.name))
    {
      if( param.isOptional)
      {
        if( ! param.defaultValue.isEmpty())
          params[param.name] = param.defaultValue;
      }
      else
      {
        QString err("Missing mandatory param \"%1\" for transform \"%2\"!");
        _setError(Ntg::InvalidParamsError, err.arg(param.name).arg(_model.name));
        return;
      }
    }
  }

  foreach(const QString & param, params.keys())
  {
    if( ! _model.params.contains(param))
    {
      QString err("Unknown param \"%1\" for transform \"%2\"!");
      _setError(Ntg::InvalidParamsError, err.arg(param).arg(_model.name));
      return;
    }
  }

  _inputEntity = input;
  _exec(input, params);
}


void NtgTransform::cancel()
{
  if(_status == Ntg::Running )
    _cancel();
}


Ntg::TransformStatus NtgTransform::status() const
{
  return _status;
}


double NtgTransform::progressValue () const
{
  return _progressPercent;
}


QString NtgTransform::progressText () const
{
  return _progressText;
}


Ntg::TransformError NtgTransform::error() const
{
  return _error;
}


QString NtgTransform::errorString() const
{
  return _errorString;
}


int NtgTransform::latestResultIndex() const
{
  return (_resultList.size() - 1);
}


QList<NtgEntity> NtgTransform::results(int startIndex, int toIndex) const
{
  if( startIndex >= _resultList.size() || (toIndex > 0 && toIndex < startIndex))
    return QList<NtgEntity>();
  if( toIndex >= _resultList.size() || toIndex < 0)
    return _resultList.mid(startIndex);
  return _resultList.mid(startIndex, (toIndex - startIndex) + 1);
}


const NtgTransformModel & NtgTransform::model() const
{
  return _model;
}


QString NtgTransform::type() const
{
  return _model.name;
}


const NtgEntity & NtgTransform::inputEntity() const
{
  return _inputEntity;
}


QString NtgTransform::workDirPath() const
{
  return _workDirPath;
}


void NtgTransform::setHandler(NtgRpcHandler * handler)
{
  _handler = handler;
  if(_handler)
    _handler->registerInvokableObject(this);
}


QHash<QString,QString> NtgTransform::configValues() const
{
  return _configValues;
}


bool NtgTransform::configValueAsBool(const QString & name, bool * value) const
{
  if( ! value ) return false;
  QString boolStr = _configValues.value(name).toLower();
  if(boolStr == "true" || boolStr == "yes" || boolStr == "1")
    *value = true;
  else if(boolStr == "false" || boolStr == "no" || boolStr == "0")
    *value = false;
  else
    return false;
  return true;
}


bool NtgTransform::configValueAsInt(const QString & name, int * value) const
{
  if( ! value ) return false;
  QString intStr = _configValues.value(name);
  bool ok;
  int valueTmp = intStr.toInt(&ok, 0);
  if( ! ok ) return false;
  *value = valueTmp;
  return true;
}


bool NtgTransform::configValueAsDouble(const QString & name, double * value) const
{
  if( ! value ) return false;
  QString doubleStr = _configValues.value(name);
  bool ok;
  double valueTmp = doubleStr.toDouble(&ok);
  if( ! ok ) return false;
  *value = valueTmp;
  return true;
}


bool NtgTransform::configValueAsString(const QString & name, QString * value) const
{
  if( ! value ) return false;
  QString str = _configValues.value(name);
  if(str.isEmpty())
    return false;
  *value = str;
  return true;
}


void NtgTransform::_setStatus(Ntg::TransformStatus status)
{
  if(status == _status)
    return;
  _status = status;
  emit statusChanged(status);
  if(_handler)
  {
    QVariantList args;
    args << QVariant::fromValue(_status);
    _handler->emitSignal(this, "_statusChanged", args);
  }
}


void NtgTransform::_setError(Ntg::TransformError error, QString msg)
{
  _error = error;
  _errorString = msg;
  _setStatus(Ntg::Failed);
}


void NtgTransform::_setProgress(double progressPercent, QString progressStr)
{
  if( progressPercent > 100.0)
    _progressPercent = 100.0;
  else
    _progressPercent = progressPercent;
  if( ! progressStr.isNull())
    _progressText = progressStr;
  emit progressChanged(_progressPercent, _progressText);
  if(_handler)
  {
    QVariantList args;
    args << _progressPercent << _progressText;
    _handler->emitSignal(this, "_progressChanged", args);
  }
}


void NtgTransform::_addResult(NtgEntity entity)
{
  if( ! _model.outputTypes.contains(entity.type) )
  {
    QString err("Invalid result type \"%1\" for transform \"%2\"!");
    _setError(Ntg::InvalidResultsError, err.arg(entity.type).arg(_model.name));
    return;
  }
  _resultList.append(entity);
  emit resultReadyAt(_resultList.size()-2, _resultList.size()-1);
  if(_handler)
  {
    QVariantList args;
    args << (_resultList.size()-2) << (_resultList.size()-1);
    _handler->emitSignal(this, "_resultReadyAt", args);
  }
}


void NtgTransform::_addResults(QList<NtgEntity> entities)
{
  if(entities.isEmpty())
    return;

  foreach(const NtgEntity & e , entities)
  {
    if( ! _model.outputTypes.contains(e.type) )
    {
      QString err("Invalid result type \"%1\" for transform \"%2\"!");
      _setError(Ntg::InvalidResultsError, err.arg(e.type).arg(_model.name));
      return;
    }
  }
  _resultList.append(entities);
  emit resultReadyAt(_resultList.size() - entities.size(), _resultList.size()-1);
  if(_handler)
  {
    QVariantList args;
    args  << (_resultList.size() - entities.size()) << (_resultList.size()-1);
    _handler->emitSignal(this, "_resultReadyAt", args);
  }
}


void NtgTransform::_reset()
{
  _status = Ntg::NotStarted;
  _error = Ntg::NoError;
  _errorString.clear();
  _progressPercent = 0;
  _progressText.clear();
  _resultList.clear();
  _inputEntity = NtgEntity();
}
