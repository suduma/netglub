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

#include "scripter.h"
#include "mainwindow.h"


Scripter * Scripter::_scripter = NULL;

Scripter::Scripter() : _scriptEngine(new QScriptEngine), _protoGraph(NULL)
{
    createSymbol(MainWindow::get(),"main");

    _protoGraph = new GraphPrototype;
    _protoNode = new NodePrototype;
    _protoTransform = new TransformInterface;

    _scriptEngine->setDefaultPrototype(qMetaTypeId<Graph*>(),_scriptEngine->newQObject(_protoGraph));
    _scriptEngine->setDefaultPrototype(qMetaTypeId<Node*>(),_scriptEngine->newQObject(_protoNode));
    _scriptEngine->setDefaultPrototype(qMetaTypeId<Transform*>(),_scriptEngine->newQObject(_protoTransform));

    qScriptRegisterSequenceMetaType<QVector<Node*> >(_scriptEngine);

    /* print functions */
    QScriptValue scriptValue =  _scriptEngine->newObject();
    _scriptEngine->globalObject().setProperty("print_debug",_scriptEngine->newFunction(print_debug,scriptValue));
    scriptValue =  _scriptEngine->newObject();
    _scriptEngine->globalObject().setProperty("print_info",_scriptEngine->newFunction(print_info,scriptValue));
    scriptValue =  _scriptEngine->newObject();
    _scriptEngine->globalObject().setProperty("print_warning",_scriptEngine->newFunction(print_warning,scriptValue));
    scriptValue =  _scriptEngine->newObject();
    _scriptEngine->globalObject().setProperty("print_critical",_scriptEngine->newFunction(print_critical,scriptValue));
}

Scripter::~Scripter()
{
    if(_scripter) delete _scripter;
}

Scripter * Scripter::get()
{
    if(Scripter::_scripter == NULL)
        Scripter::_scripter = new Scripter();

    return Scripter::_scripter;
}

void Scripter::process(QString cmd)
{
    try {
   QScriptValue result = _scriptEngine->evaluate(cmd);
   if (_scriptEngine->hasUncaughtException())
   {
       int line = _scriptEngine->uncaughtExceptionLineNumber();
       qWarning() << "_uncaught exception at line" << line << ":" << result.toString();
   }
}
    catch(...)
    {
        qDebug() << "fatal";
    }
}

QScriptEngine * Scripter::engine()
{
    return _scriptEngine;
}

void Scripter::createSymbol(QObject * object, QString name)
{
    QScriptValue scriptValue = _scriptEngine->newQObject(object);
    _scriptEngine->globalObject().setProperty(name,scriptValue);
}

