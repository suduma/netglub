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

#include "scriptwidget.h"

#include "scripter/scripter.h"
#include "mainwindow.h"

ScriptWidget::ScriptWidget(QWidget *parent) : QWidget(parent)
{
    _scriptTextEdit = new QTextEdit;

    QVBoxLayout * layout = new QVBoxLayout(this);

    _toolbar = new QToolBar;
    _toolbar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

    _openAct = _toolbar->addAction(QIcon(":/images/openscript.png"), tr("Open Scenario"),this, SLOT(openScriptSlot()));
    _openAct = _toolbar->addAction(QIcon(":/images/fileexport.png"), tr("Save Scenario"),this, SLOT(saveScriptSlot()));
    _processAct = _toolbar->addAction(QIcon(":/images/run.png"), tr("Run"),this, SLOT(processSlot()));
    _resetAct = _toolbar->addAction(QIcon(":/images/editclear.png"), tr("Reset"),this, SLOT(resetSlot()));

    layout->addWidget(_toolbar);

    layout->addWidget(_scriptTextEdit);

    setLayout(layout);


}

void ScriptWidget::processSlot()
{
    Scripter::get()->process(_scriptTextEdit->toPlainText());
}

void ScriptWidget::resetSlot()
{
    _scriptTextEdit->setPlainText("");
    _filename = "";
    this->parentWidget()->setWindowTitle("Scenario");
}

void ScriptWidget::openScriptSlot()
{
    QSettings settings;
    QString scriptpath = settings.value("scriptpath").toString();
    QString fileName =
            QFileDialog::getOpenFileName(this,
                                         tr("Open Scenario"),
                                         scriptpath,
                                         tr("Script ( *.ntgsc "));
    if (!fileName.isEmpty())
    {
        loadScript(fileName);
        settings.setValue("scriptpath",QFileInfo(fileName).absolutePath());
    }
}

void ScriptWidget::loadScript(QString fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(MainWindow::get(), tr("Read file error"),
                             tr("Cannot read file %1:\n%2.")
                             .arg(fileName)
                             .arg(file.errorString()));
        return;
    }

    QTextStream stream(&file);
    QString script = stream.readAll();
    _scriptTextEdit->setPlainText(script);
    _filename = fileName;
    this->parentWidget()->setWindowTitle("Scenario "+QFileInfo(fileName).fileName());
}

void ScriptWidget::saveScriptSlot()
{
    if(_filename.isEmpty())
    {
        QSettings settings;
        QString scriptpath = settings.value("scriptpath").toString();
        QString fileName = QFileDialog::getSaveFileName(this, tr("Save Script"),
                                                scriptpath+"/untitled.ntgsc",
                                                tr("File (*.ntgsc)"));
        if (!fileName.isEmpty())
        {
            saveScript(fileName);
            settings.setValue("scriptpath",QFileInfo(fileName).absolutePath());
        }
    }
    else
    {
        saveScript(_filename);
    }
}

void ScriptWidget::saveScript(QString fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox::warning(MainWindow::get(), tr("Write file error"),
                             tr("Cannot write file %1:\n%2.")
                             .arg(fileName)
                             .arg(file.errorString()));
        return;
    }
    QTextStream stream(&file);
    stream << _scriptTextEdit->toPlainText();
    _filename = fileName;
    this->parentWidget()->setWindowTitle("Scenario "+QFileInfo(fileName).fileName());
}
