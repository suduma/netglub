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

#ifndef CONNECTIONWIDGETS_H
#define CONNECTIONWIDGETS_H

#include <QWidget>
#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QGridLayout>
#include <QCheckBox>

#include "callermanager.h"

class CallerManager;

class FailedConectionDialog : public QDialog
{
Q_OBJECT
public:
    explicit FailedConectionDialog(QString text, QWidget *parent = 0);
    virtual ~FailedConectionDialog();

    void setText(QString text);
    bool continueCall();

public slots:
    void quitSlt();
    void retrySlt();
    void connectSlt();
    void offLineSlt();

private:
    QLabel * _errorLabel;
    QPushButton * _retryButton;
    QPushButton * _connectionButton;
    QPushButton * _offLineButtonButton;
    QPushButton * _quitButton;

    bool _continueCaller;

};


class ConnectionWidget : public QWidget
{
Q_OBJECT
public:
    explicit ConnectionWidget(bool quitIsActive, QWidget *parent = 0);
    virtual ~ConnectionWidget();

    bool tryConection();
    void create(bool quitIsActive);

    bool continuToIHM();
    QString getSerial();
    QString getURL();

    void setSerial(QString serial);
    void setURL(QString URL);

signals:
    void hidden();
    void validatePush(bool quit);
    void quitPush();
    void offLinepush();

public slots:
    void quitIHM();
    void validate();
    void contactUs();
    void offLine();
    void echoModeSlot(int);

private:
    QLabel * _serialLabel;
    QLabel * _serialExampleLabel;
    QLineEdit * _serialLineEdit;

    QLabel * _URLLabel;
    QLabel * _URLExampleLabel;
    QLineEdit * _URLLineEdit;

    QCheckBox * _rememberCheckBox;
    QCheckBox * _echoModeCheckBox;

    QPushButton * _validateButton;
    QPushButton * _cancelPushButton;

    bool _continueToIHM;
    QString _URL;
    QString _serial;

    QWidget * _errorWidget;
    QLabel * _errorLabel;
    QPushButton * _offLineWorkButton;
    QPushButton * _supportSubmitButton;

    //FailedConectionDialog * _failed;
};

class ConnectionDialog : public QDialog
{
    Q_OBJECT
    public:
        explicit ConnectionDialog(QWidget *parent = 0);
        virtual ~ConnectionDialog();

        bool continuToIHM();

    private:
        ConnectionWidget * CW;
};



#endif // CONNECTIONWIDGETS_H
