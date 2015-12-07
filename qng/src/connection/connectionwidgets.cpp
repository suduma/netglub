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

#include "connectionwidgets.h"
#include "mainwindow.h"

#include <QDebug>
#include <QRegExpValidator>
#include <QRegExp>
#include <QDesktopWidget>


ConnectionDialog::ConnectionDialog(QWidget *parent) :
        QDialog(parent)
{

    setWindowTitle(tr("Netglub master connection"));

    CW = new ConnectionWidget(true,this);
    QVBoxLayout * layout = new QVBoxLayout(this);
    layout->addWidget(CW);
    setLayout(layout);
    connect(CW,SIGNAL(hidden()),this,SLOT(hide()));
    QSettings settings;

    QDesktopWidget desktopWidget;
    QRect desktopRect(desktopWidget.availableGeometry(desktopWidget.primaryScreen()));
    move(desktopRect.center() - rect().center());


    if (settings.contains("URL") && settings.contains("serial"))
    {
        CW->setURL(settings.value("URL").toString());
        CW->setSerial(settings.value("serial").toString());
        if (!CW->tryConection())
            exec();
    }
    else exec();
}


ConnectionDialog::~ConnectionDialog()
{

}

bool ConnectionDialog::continuToIHM()
{
    return CW->continuToIHM();
}

ConnectionWidget::ConnectionWidget(bool quitIsActive, QWidget *parent) :
    QWidget(parent)
{
    _continueToIHM = false;
    create(quitIsActive);

}
ConnectionWidget::~ConnectionWidget()
{

}


void ConnectionWidget::create(bool quitIsActive)
{
    
    //TODO : VALIDATORS

    QFont  ExFont;// = new QFont();
    ExFont.setPointSize(7);

    _serialExampleLabel = new QLabel(tr("e.g. ####-####-####-####"),this);
    _serialExampleLabel->setFont(ExFont);
    _serialLineEdit = new QLineEdit("1111-4567-89ab-cdef",this);
    _serialLineEdit->setValidator(new QRegExpValidator(QRegExp("([0-9a-zA-Z#]{4}-){3}[0-9a-zA-Z#]{4}"),_serialLineEdit));
    _serialLineEdit->setInputMask("NNNN-NNNN-NNNN-NNNN;#");
    _serialLineEdit->setEchoMode(QLineEdit::Password);
    _serialLabel = new QLabel(tr("&Serial"),this);
    _serialLabel->setBuddy(_serialLineEdit);

    _URLExampleLabel = new QLabel(tr("e.g. https://www.netglub.org:8080/RPC2"),this);
    _URLExampleLabel->setFont(ExFont);
    _URLLineEdit = new QLineEdit("https://localhost:8080/RPC2",this);
    _URLLabel = new QLabel(tr("&Url"),this);
    _URLLabel->setBuddy(_URLLineEdit);

    _rememberCheckBox = new QCheckBox("&Remember settings");
    _rememberCheckBox->setCheckable(true);
    _rememberCheckBox->setChecked(true);

    _echoModeCheckBox = new QCheckBox("&Show / hide serial",this);
    _echoModeCheckBox->setCheckable(true);
    _echoModeCheckBox->setChecked(false);
    connect(_echoModeCheckBox,SIGNAL(stateChanged(int)),this,SLOT(echoModeSlot(int)));



    _validateButton = new QPushButton("&Ok",this);
     connect(_validateButton,SIGNAL(clicked()),this,SLOT(validate()));

    _cancelPushButton = new QPushButton("&Quit",this);
    _cancelPushButton->setVisible(quitIsActive);
    connect(_cancelPushButton,SIGNAL(clicked()),this,SLOT(quitIHM()));


    _offLineWorkButton = new QPushButton("&Off Line Work",this);
    connect(_offLineWorkButton,SIGNAL(clicked()),this,SLOT(offLine()));
    _offLineWorkButton->setEnabled(false);

    _errorWidget = new QWidget(this);

    _errorLabel = new QLabel("no error",_errorWidget);
    _errorLabel->setAlignment(Qt::AlignCenter);
    _errorLabel->setWordWrap(true);
    _errorLabel->setFrameStyle(6);



    _supportSubmitButton = new QPushButton("&Contact Us",_errorWidget);
    _supportSubmitButton->setEnabled(false);
    connect(_supportSubmitButton,SIGNAL(clicked()),this,SLOT(contactUs()));

    QGridLayout * errorLayout = new QGridLayout();
    errorLayout->addWidget(new QLabel("An error was encountered :"),0,0,1,4);
    errorLayout->addWidget(_errorLabel,1,0,1,4);
    errorLayout->addWidget(new QLabel("Contact us for more details :"),2,0,1,3);
    errorLayout->addWidget(_supportSubmitButton,2,3,1,1);

    _errorWidget->setLayout(errorLayout);
    _errorWidget->setVisible(false);

    QGridLayout * layout = new QGridLayout();
    layout->setSizeConstraint(QLayout::SetFixedSize);

    layout->addWidget(_serialLabel,0,0,1,1);
    layout->addWidget(_serialLineEdit,0,1,1,3);
    layout->addWidget(_serialExampleLabel,1,1,1,3);

    layout->addWidget(_echoModeCheckBox,2,1,1,3);

    layout->addWidget(_URLLabel,3,0,1,1);
    layout->addWidget(_URLLineEdit,3,1,1,3);
    layout->addWidget(_URLExampleLabel,4,1,1,3);

    layout->addWidget(_rememberCheckBox,5,1,1,2);

    layout->addWidget(_offLineWorkButton,6,1,1,1);
    layout->addWidget(_validateButton,6,3,1,1);
    layout->addWidget(_cancelPushButton,6,2,1,1);

    layout->addWidget(_errorWidget,7,0,1,4);

    setLayout(layout);

}



bool ConnectionWidget::continuToIHM()
{
    return _continueToIHM;
}

QString ConnectionWidget::getSerial()
{
    return _serial;
}

QString ConnectionWidget::getURL()
{
    return _URL;
}

void ConnectionWidget::setSerial(QString serial)
{
    _serialLineEdit->setText(serial);
    _serial = serial;
}

void ConnectionWidget::setURL(QString URL)
{
    _URLLineEdit->setText(URL);
    _URL = URL;
}

bool ConnectionWidget::tryConection()
{
    //qDebug () << "ConnectionWidget::tryConection()";
    CallerManager::create(_URL,_serial);
    _continueToIHM = true;
    QString connectionError = CallerManager::get()->getConnectionError();
    //qDebug() << connectionError;
    if (!connectionError.isEmpty())
    {
        //qDebug() << "problem in connection";
        _validateButton->setText("&Retry");
        _continueToIHM = false;
        _errorLabel->setText(connectionError);
        _errorWidget->setVisible(true);
        return false;
    }
    else
    {
        QSettings settings;
        if (_rememberCheckBox->isChecked())
        {
            settings.setValue("URL",QVariant(_URL));
            settings.setValue("serial",QVariant(_serial));
        }
        else
        {
            if (settings.contains("URL") && settings.contains("serial"))
            {
                settings.remove("URL");
                settings.remove("serial");
            }

        }

        hide();
        emit hidden();
        return true;
    }
}

void ConnectionWidget::echoModeSlot(int echoMode)
{
    if(echoMode == Qt::Checked)
    {
        _serialLineEdit->setEchoMode(QLineEdit::Normal);
    }
    else
    {
        _serialLineEdit->setEchoMode(QLineEdit::Password);
    }
}


/*------------------------------------
               slots
--------------------------------------*/

void ConnectionWidget::validate()
{
    //qDebug () << "ConnectionWidget::validate()";
    _serial = _serialLineEdit->text();
    _URL = _URLLineEdit->text();
    emit validatePush(tryConection());
}


void ConnectionWidget::quitIHM()
{
    if (CallerManager::get())
        delete CallerManager::get();

    hide();
    emit hidden();
    emit quitPush();
}



void ConnectionWidget::contactUs()
{

}

void ConnectionWidget::offLine()
{
    _continueToIHM = true;
    CallerManager::create("","");
    CallerManager::get()->setOffLineWork(true);

    hide();
    emit hidden();
    emit offLinepush();
}





/*------------------------------------
Class FailedConectionDialog
--------------------------------------*/

FailedConectionDialog::FailedConectionDialog(QString text, QWidget *parent)
{
    Q_UNUSED(parent);

    QLabel * _errorLabel = new QLabel(text,this);
    _errorLabel->setFrameStyle(6);
    _errorLabel->setAlignment(Qt::AlignCenter);
    _errorLabel->setWordWrap(true);

    QPushButton * _retryButton = new QPushButton("&Retry");
    connect(_retryButton,SIGNAL(clicked()),this,SLOT(retrySlt()));

    QPushButton * _connectionButton = new QPushButton("&Connect");
    connect(_connectionButton,SIGNAL(clicked()),this,SLOT(connectSlt()));

    QPushButton * _offLineButtonButton = new QPushButton("&Off Line Work");
    connect(_offLineButtonButton,SIGNAL(clicked()),this,SLOT(offLineSlt()));
    _offLineButtonButton->setEnabled(false);

    QPushButton * _quitButton = new QPushButton("&Quit");
    connect(_quitButton,SIGNAL(clicked()),this,SLOT(quitSlt()));


    QGridLayout * layout = new QGridLayout(this);
    layout->addWidget(_errorLabel,0,0,1,4);
    layout->addWidget(_quitButton,1,0,1,1);
    layout->addWidget(_offLineButtonButton,1,1,1,1);
    layout->addWidget(_connectionButton,1,2,1,1);
    layout->addWidget(_retryButton,1,3,1,1);

    setLayout(layout);

    _continueCaller = false;
}

FailedConectionDialog::~FailedConectionDialog()
{
    //qDebug() << "FailedConectionDialog::~FailedConectionDialog";
}

void FailedConectionDialog::setText(QString text)
{
    _errorLabel->setText(text);
}

void FailedConectionDialog::quitSlt()
{
    MainWindow::get()->close();
    this->close();
}

void FailedConectionDialog::retrySlt()
{
    if(CallerManager::get()->retryConnexion())
    {
        //qDebug() << "FailedConectionDialog::retrySlt() mid";
        _continueCaller=true;
        hide();
    }
}

void FailedConectionDialog::connectSlt()
{
    ConnectionDialog * CD = new ConnectionDialog();
    bool continueToIHM = false;
    if (CD->continuToIHM())
        continueToIHM = true;
    delete CD;
    if (continueToIHM)
    {
        _continueCaller=true;
        hide();
    }
}

void FailedConectionDialog::offLineSlt()
{
    CallerManager::get()->setOffLineWork(true);
    hide();
}

bool FailedConectionDialog::continueCall()
{
    bool tmp = _continueCaller;
    _continueCaller = false;
    return tmp;
}

