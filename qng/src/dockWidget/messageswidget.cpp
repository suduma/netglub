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

#include "messageswidget.h"
#include <QStandardItemModel>
#include <QDateTime>


MessagesWidget::MessagesWidget(QWidget *parent) :
    QWidget(parent)
{
    QVBoxLayout * mainLayout = new QVBoxLayout;
    setLayout(mainLayout);

    _toolbar = new QToolBar;
    _toolbar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    mainLayout->addWidget(_toolbar);



    _allAction = _toolbar->addAction(QIcon(":/images/messages/all.png"), tr("All"),this, SLOT(_showAllSlot(bool)));

    _infoAction = _toolbar->addAction(QIcon(":/images/messages/info.png"), tr("Info"),this, SLOT(_showSlot(bool)));
    _infoAction->setCheckable(true);
    _infoAction->setChecked(true);

    _debugAction = _toolbar->addAction(QIcon(":/images/messages/debug.png"), tr("Debug"),this, SLOT(_showSlot(bool)));
    _debugAction->setCheckable(true);
    _debugAction->setChecked(false);

    _warningAction = _toolbar->addAction(QIcon(":/images/messages/warning.png"), tr("Warning"),this, SLOT(_showSlot(bool)));
    _warningAction->setCheckable(true);
    _warningAction->setChecked(true);

    _criticalAction = _toolbar->addAction(QIcon(":/images/messages/error.png"), tr("Critical"),this, SLOT(_showSlot(bool)));
    _criticalAction->setCheckable(true);
    _criticalAction->setChecked(true);

    _clearAction = _toolbar->addAction(QIcon(":/images/clear.png"), tr("Clear"),this, SLOT(_clearSlot(bool)));

    _listview = new QListView(this);
    _listmodel = new QStandardItemModel;
    _listview->setModel(_listmodel);

    mainLayout->addWidget(_listview, 1);


}

void MessagesWidget::appendMessage(QtMsgType type, const QString & msg)
{
    //fprintf(stderr,"append item");
    //QString message = QDateTime::currentDateTime().toString() + " : " + msg;
    QString message = QTime::currentTime().toString() + " : " + msg;

    QPair<QtMsgType,QString> pair;
    pair.first = type;
    pair.second = message;
    _messagesList.prepend(pair);

    if(type==QtDebugMsg&& _debugAction->isChecked() )
    {
        QStandardItem * item = new QStandardItem(QIcon(":/images/messages/debug.png"),message);
        item->setEditable(false);
        _listmodel->insertRow(0,item);
    }
    else if(type==QtWarningMsg && _warningAction->isChecked())
    {
        QStandardItem * item = new QStandardItem(QIcon(":/images/messages/warning.png"),message);
        item->setEditable(false);
        _listmodel->insertRow(0,item);
    }
    else if(type==QtCriticalMsg && _criticalAction->isChecked())
    {
        QStandardItem * item = new QStandardItem(QIcon(":/images/messages/error.png"),message);
        item->setEditable(false);
        _listmodel->insertRow(0,item);
    }
    else if((int)type==(int)MessagesWidget::Infomsg && _infoAction->isChecked() )
    {
        QStandardItem * item = new QStandardItem(QIcon(":/images/messages/info.png"),message);
        item->setEditable(false);
        _listmodel->insertRow(0,item);
    }

}

MessagesWidget::~MessagesWidget()
{

}

void MessagesWidget::_showSlot(bool)
{
    _listmodel->clear();

    QListIterator<QPair<QtMsgType,QString> > itrMessages(_messagesList);
    while(itrMessages.hasNext())
    {
        QPair<QtMsgType,QString> pair = itrMessages.next();
        if(pair.first==QtDebugMsg && _debugAction->isChecked())
        {
            QStandardItem * item = new QStandardItem(QIcon(":/images/messages/debug.png"),pair.second);
            item->setEditable(false);
            _listmodel->appendRow(item);
        }
        else if(pair.first==QtWarningMsg && _warningAction->isChecked())
        {
            QStandardItem * item = new QStandardItem(QIcon(":/images/messages/warning.png"),pair.second);
            item->setEditable(false);
            _listmodel->appendRow(item);
        }
        else if(pair.first==QtCriticalMsg && _criticalAction->isChecked())
        {
            QStandardItem * item = new QStandardItem(QIcon(":/images/messages/error.png"),pair.second);
            item->setEditable(false);
            _listmodel->appendRow(item);
        }
        else if((int)(pair.first)==(int)(MessagesWidget::Infomsg) && _infoAction->isChecked() )
        {
            QStandardItem * item = new QStandardItem(QIcon(":/images/messages/info.png"),pair.second);
            item->setEditable(false);
            _listmodel->appendRow(item);
        }
    }
}

void MessagesWidget::_showAllSlot(bool)
{
    _debugAction->setChecked(true);
    _warningAction->setChecked(true);
    _criticalAction->setChecked(true);
    _infoAction->setChecked(true);

    _showSlot(false);
}

void MessagesWidget::_clearSlot(bool)
{
    _messagesList.clear();
    _listmodel->clear();
}
