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

#include "detailswidget.h"
#include <mainwindow.h>
#include <QDesktopServices>

DetailsWidget::DetailsWidget(QWidget *parent) :
    QWidget(parent)
{

    _titleLabel = new QLabel(this);
    _titleLabel->setMinimumHeight(10);
    _titleLabel->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Maximum);

    _imgLabel = new QLabel(this);
//    _imgLabel->setFixedSize(48,48);

    _detailsLabel = new QLabel(this);
    _detailsLabel->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Maximum);
    _detailsLabel->setMaximumHeight(30);
    _detailsLabel->setMaximumWidth(250);
    _detailsLabel->setWordWrap(true);


    _label = new QLabel(this);
    _label->setFrameStyle(6);
    _label->setAlignment(Qt::AlignCenter);
//    _label->setWordWrap(true);
    _label->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Maximum);
    _label->setVisible(false);

    QGridLayout * layout = new QGridLayout(this);
//    layout->addWidget(_label,Qt::AlignCenter);
    layout->addWidget(_imgLabel,0,0,2,1,Qt::AlignCenter);
    layout->setColumnStretch(0,0);
    layout->addWidget(_titleLabel,0,1,1,2);
    layout->addWidget(_detailsLabel,1,1,3,2);
    layout->setColumnStretch(1,1);
    setLayout(layout);
//    setMaximumWidth();

}

DetailsWidget::~DetailsWidget()
{

}


QString DetailsWidget::getFormatedString(QString text,int size)
{
    for (int i= 1; i< text.size()/size+1; i++)
        text.insert(size*i,QString("\n"));
    return text;
}

void DetailsWidget::setNodeList(QList<Node *> nodeList)
{
//    qDebug() << "DetailsWidget::setNodeList(QList<Node *> nodeList)";
    if (nodeList.count() == 0)
    {
        _titleLabel->clear();
        _imgLabel->setPixmap(NULL);
        _detailsLabel->clear();
    }
    else if (nodeList.count() == 1)
    {
        Node * node = nodeList.first();
        QString ModelName = node->getEntityModel().name;
        QSettings settings;
        QFileInfo settingsPath = QFileInfo(settings.fileName());
        QString pngFileName = settingsPath.absolutePath()+"/data/entities/"+ModelName+"/image.png";
        if (!ModelName.isEmpty())
        {
            QPixmap pixmap(pngFileName);
            _imgLabel->setPixmap(pixmap);
            _titleLabel->setText(node->getEntityModel().longName);
            if (node->getEntity().values.contains("value"))
                _detailsLabel->setText(getFormatedString(node->getEntity().values.value("value"),30));
            else
                _detailsLabel->setText("None");
        }

    }
    else if (nodeList.count() > 1)
    {
        QMap<QString,QString> comparingMap;
        Node * pN = nodeList.first();
        comparingMap.insert("type",pN->getEntity().type);
        foreach(QString key, pN->getEntity().values.keys())
        {
            comparingMap.insert(key,pN->getEntity().values.value(key));
        }

        foreach(Node * node, nodeList)
        {
            if(node->getEntity().type != comparingMap.value("type"))
            {
                comparingMap.clear();
            }
            else
            {
                foreach(QString key, node->getEntity().values.keys())
                    if (node->getEntity().values.value(key) != comparingMap.value(key))
                        comparingMap.remove(key);
            }
        }
        if (comparingMap.count() == 0)
        {
            _imgLabel->setPixmap(QPixmap(":/images/entity.png"));
            _titleLabel->setText("No common details.");
            _detailsLabel->setText(QString::number(nodeList.size()).append(" entities selected."));
//            text = QString("No common details. \n").append(QString::number(nodeList.size())).append(" entities selected.");
        }
        else if (comparingMap.count() >0 && comparingMap.contains("type"))
        {
////            qDebug() << comparingMap;
            QSettings settings;
            QFileInfo settingsPath = QFileInfo(settings.fileName());
            QString pngFileName = settingsPath.absolutePath()+"/data/entities/"+comparingMap.value("type")+"/image.png";
            QPixmap pixmap(pngFileName);
            _imgLabel->setPixmap(pixmap);
            _titleLabel->setText(pN->getEntityModel().longName + " : " + QString::number(nodeList.size()));
            comparingMap.remove("type") ;
            if (comparingMap.isEmpty())
            {
                _detailsLabel->setText(QString("No other Common details") );
            }
            else
            {
                QString text = "";
                foreach(QString key, comparingMap.keys())
                {
                     if(!comparingMap.value(key).isEmpty() && !pN->getEntityModel().params.value(key).longName.isEmpty())
                         text += getFormatedString(pN->getEntityModel().params.value(key).longName + " : " + comparingMap.value(key),30) + "\n";
                }
            }
        }
    }
    _label->update();
    _label->updateGeometry();
}

void DetailsWidget::setEdgeSet(QSet<Edge*>& edgeSet)
{
    _imgLabel->setPixmap(QPixmap(":/images/edge.png"));
    if(edgeSet.size()==1)
    {

        _titleLabel->setText(QString::number(edgeSet.size())+" edge selected");
    }
    else if(edgeSet.size()>1)
    {
        _titleLabel->setText(QString::number(edgeSet.size())+" edges selected");
    }
}

void DetailsWidget::mouseDoubleClickEvent(QMouseEvent * event)
{
    Q_UNUSED(event)
    MainWindow::get()->openSelectedNodesUrl();
}
