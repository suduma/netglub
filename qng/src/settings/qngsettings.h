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

#ifndef QNGSETTINGS_H
#define QNGSETTINGS_H

#include <QDialog>
#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>

#include "connection/connectionwidgets.h"

QT_BEGIN_NAMESPACE
class QListWidget;
class QListWidgetItem;
class QStackedWidget;
QT_END_NAMESPACE

class ConfigurationPage : public QWidget
{
public:
    ConfigurationPage(QWidget *parent = 0);
    ConnectionWidget * connection;
};

class PersonPage : public QWidget
{
public:
    PersonPage(QWidget *parent = 0);

private:
    QLineEdit * _firstNameLineEdit;
    QLineEdit * _lastNameLineEdit;
    QLineEdit * _emailLineEdit;
    QLineEdit * _passwordLineEdit;

    QPushButton * _validateButton;

};

class UpdatePage : public QWidget
{
public:
    UpdatePage(QWidget *parent = 0);

private:
    QPushButton * _checkButton;
    QLabel * _stateLabel;
    QPushButton * _updateButton;
};


class QngSettings : public QDialog
{
    Q_OBJECT

public:
    QngSettings();

signals:
    void reloadSession();

public slots:
    void changePage(QListWidgetItem *current, QListWidgetItem *previous);
    void closeOrNot(bool quit);

private:
    void createIcons();

    QListWidget *contentsWidget;
    QStackedWidget *pagesWidget;

};




#endif // QNGSETTINGS_H
