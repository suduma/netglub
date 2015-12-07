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

#include <QtGui>
#include "qngsettings.h"
#include "modelsmanager.h"

QngSettings::QngSettings()
{
    contentsWidget = new QListWidget;
    contentsWidget->setViewMode(QListView::IconMode);
    contentsWidget->setIconSize(QSize(48, 42));
    contentsWidget->setMovement(QListView::Static);
    contentsWidget->setMaximumWidth(80);
    contentsWidget->setSpacing(12);

    pagesWidget = new QStackedWidget;
    ConfigurationPage * config = new ConfigurationPage(this);
    pagesWidget->addWidget(config);
    pagesWidget->addWidget(new UpdatePage);
    pagesWidget->addWidget(new PersonPage);

    connect(config->connection,SIGNAL(offLinepush()),this,SLOT(close()));
    connect(config->connection,SIGNAL(validatePush(bool)),this,SLOT(closeOrNot(bool)));
//    connect(config,SIGNAL(),this,SLOT(close()));

    QPushButton *closeButton = new QPushButton(tr("Close"));

    createIcons();
    contentsWidget->setCurrentRow(0);

    connect(closeButton, SIGNAL(clicked()), this, SLOT(close()));

    QHBoxLayout *horizontalLayout = new QHBoxLayout;
    horizontalLayout->addWidget(contentsWidget);
    horizontalLayout->addWidget(pagesWidget, 1);

    QHBoxLayout *buttonsLayout = new QHBoxLayout;
    buttonsLayout->addStretch(1);
    buttonsLayout->addWidget(closeButton);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(horizontalLayout);
    mainLayout->addStretch(1);
    mainLayout->addSpacing(12);
    mainLayout->addLayout(buttonsLayout);
    setLayout(mainLayout);

    setWindowTitle(tr("Settings"));
}

void QngSettings::closeOrNot(bool quit)
{
    if (quit)
    {
        emit reloadSession();
        this->close();
    }
}

void QngSettings::createIcons()
{
    QListWidgetItem *configButton = new QListWidgetItem(contentsWidget,1);
    configButton->setIcon(QIcon(":/images/configure.png"));
    configButton->setText(tr("Config"));
    configButton->setTextAlignment(Qt::AlignHCenter);
    configButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    QListWidgetItem *updateButton = new QListWidgetItem(contentsWidget);
    updateButton->setIcon(QIcon(":/images/update.png"));
    updateButton->setText(tr("Update"));
    updateButton->setTextAlignment(Qt::AlignHCenter);
    updateButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    QListWidgetItem *queryButton = new QListWidgetItem(contentsWidget);
    queryButton->setIcon(QIcon(":/images/person.png"));
    queryButton->setText(tr("Setting"));
    queryButton->setTextAlignment(Qt::AlignHCenter);
    queryButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    connect(contentsWidget,
            SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)),
            this, SLOT(changePage(QListWidgetItem*,QListWidgetItem*)));
}

void QngSettings::changePage(QListWidgetItem *current, QListWidgetItem *previous)
{
    if (!current)
        current = previous;

    pagesWidget->setCurrentIndex(contentsWidget->row(current));
}

ConfigurationPage::ConfigurationPage(QWidget *parent)
    : QWidget(parent)
{
    connection = new ConnectionWidget(false,this);
    QVBoxLayout * layout = new QVBoxLayout(this);
    layout->addWidget(connection,1,Qt::AlignTop);
    setLayout(layout);
//    connect(CW,SIGNAL(hidden()),this,SLOT(close()));
    QSettings settings;
    if (settings.contains("URL") && settings.contains("serial"))
    {
        connection->setURL(settings.value("URL").toString());
        connection->setSerial(settings.value("serial").toString());
    }


}

UpdatePage::UpdatePage(QWidget *parent)
    : QWidget(parent)
{



    QLabel * label = new QLabel("state :",this);
    _checkButton = new QPushButton("&Check For Updates",this);
    _stateLabel = new QLabel("unknown",this);
    _stateLabel->setAlignment(Qt::AlignCenter);
    _stateLabel->setWordWrap(true);
    _stateLabel->setFrameStyle(6);
    _updateButton = new QPushButton("&Update Files");

    QGridLayout * layout = new QGridLayout(this);
    layout->addWidget(label,0,0,1,1,Qt::AlignLeft);
    layout->addWidget(_checkButton,0,1,1,1,Qt::AlignRight);
    layout->addWidget(_stateLabel,1,0,1,2);
    layout->addWidget(_updateButton,2,0,1,2);

    QGroupBox *updateGroup = new QGroupBox(tr("Update :"),this);
    updateGroup->setLayout(layout);
    updateGroup->setEnabled(false);

    QVBoxLayout * l = new QVBoxLayout(this);
    l->addWidget(updateGroup,1,Qt::AlignTop);
    setLayout(l);




}

PersonPage::PersonPage(QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle("My Settings :");
    QLineEdit * _firstNameLineEdit = new QLineEdit(this);
    QLineEdit * _lastNameLineEdit = new QLineEdit(this);
    QLineEdit * _emailLineEdit = new QLineEdit(this);
    QLineEdit * _passwordLineEdit = new QLineEdit(this);
    _passwordLineEdit->setEchoMode(QLineEdit::Password);

    QPushButton * _validateButton = new QPushButton("validate entries",this);

    QGroupBox * groupBox = new QGroupBox("My settings :",this);
    groupBox->setEnabled(false);

    QFormLayout * layout = new QFormLayout(this);
    layout->setSpacing(12);

    layout->addRow("&First Name",_firstNameLineEdit);
    layout->addRow("&Last Name",_lastNameLineEdit);
    layout->addRow("&Email",_emailLineEdit);
    layout->addRow("&Password",_passwordLineEdit);

    layout->addWidget(_validateButton);
    groupBox->setLayout(layout);
    QVBoxLayout * l = new QVBoxLayout(this);
    l->addWidget(groupBox,1,Qt::AlignTop);
    setLayout(l);






}
