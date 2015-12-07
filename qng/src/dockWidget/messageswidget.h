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

#ifndef MESSAGESWIDGET_H
#define MESSAGESWIDGET_H

#include <QWidget>
#include <QTextEdit>
#include <QGridLayout>
#include <QPushButton>
#include <QRegExp>
#include <QDebug>
#include <QListView>
#include "QProcess"
#include <QStandardItemModel>
#include <QButtonGroup>
#include <QToolBar>
#include <QAction>

class MessagesWidget : public QWidget
{
Q_OBJECT

Q_ENUMS(InfoMsg)

public:
    enum InfoMsg{Infomsg=5};
    explicit MessagesWidget(QWidget *parent = 0);
    ~MessagesWidget();

    void appendMessage(QtMsgType type, const QString & msg);
protected slots:
    void _showSlot(bool);
    void _showAllSlot(bool);
    void _clearSlot(bool);


protected:
    QToolBar * _toolbar;


    QListView * _listview;
    QStandardItemModel * _listmodel;
    QList<QPair<QtMsgType,QString> > _messagesList;

    QAction * _infoAction;
    QAction * _debugAction;
    QAction * _warningAction;
    QAction * _criticalAction;
    QAction * _allAction;
    QAction * _clearAction;

    QProcess * process;

};

#endif // MESSAGESWIDGET_H
