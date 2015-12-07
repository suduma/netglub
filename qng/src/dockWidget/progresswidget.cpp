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

#include "progresswidget.h"
#include <QGridLayout>
#include <QFont>


ProgressWidget::ProgressWidget(QWidget *parent) :
    QWidget(parent)
{
    QFont  blankFont;// = new QFont();
    blankFont.setPixelSize(1);
    blankFont.setPointSize(1);
    blankFont.setWeight(0);

    _inProcess = 0;
    _global = 0;
    _transformLabel = new QLabel("transform 0/0",this);
    _transformLabel->setAlignment(Qt::AlignCenter);
    _progressBar = new QProgressBar(this);
    _progressBar->setMaximum(0);
    _progressBar->setMinimum(0);
    _progressBar->setValue(0);
    _progressBar->setMaximum(1000);
    _progressBar->setMaximumHeight(15);
    _progressBar->setFont(blankFont);

    stopButton = new QPushButton(QIcon(":/images/stop.png"),"",this);
    stopButton->setEnabled(false);

    connect(stopButton,SIGNAL(clicked()),this,SLOT(stopAlltransforms()));

    QGridLayout * layout = new QGridLayout(this);
    layout->addWidget(_transformLabel,0,0,1,4,Qt::AlignBottom);
    layout->addWidget(_progressBar,1,0,1,5,Qt::AlignBottom);
    layout->addWidget(stopButton,0,4,1,1,Qt::AlignRight);
    setLayout(layout);

    setMaximumHeight(75);
}


ProgressWidget::~ProgressWidget()
{

}

void ProgressWidget::addTransform()
{
    stopButton->setEnabled(true);
    _inProcess++;
    _global++;
    _transformLabel->setText("transform "+ QString::number(_inProcess,10) +"/" + QString::number(_global,10));
    if (_inProcess == 1)
    {
        _progressBar->setMaximum(0);
    }

}

void ProgressWidget::endTransform()
{
    if (_inProcess>0)
        _inProcess--;
    _transformLabel->setText("transform "+ QString::number(_inProcess,10) +"/" + QString::number(_global,10));
    if (_inProcess == 0)
    {
        stopButton->setEnabled(false);
        _progressBar->setMaximum(1);
        _progressBar->repaint();
    }
}


void ProgressWidget::stopAlltransforms()
{
    stopButton->setEnabled(false);
    while(_inProcess > 0)
    {
        endTransform();
    }
}

