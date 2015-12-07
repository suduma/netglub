/******************************************************************************
**
** Copyright (C) 2009-2010 DIATEAM. All rights reserved.
**
** This file is part of the Netglub GUI program.
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

#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include <QObject>
#include <QDomDocument>
#include <QHash>
#include <QDir>
#include <QTableWidget>
#include <QPushButton>
#include <QDialog>

#include "graph.h"

class Graph;

struct NtgFile
{
    QString shortName;
    QString srcPath;
    QString cpPath;
    QString size;
    int id;
};


class FileManager : public QObject
{
Q_OBJECT
public:
    static FileManager * getOrCreate();
    virtual ~FileManager();

    NtgFile addFile(NtgFile fileStruct, Node* node);
    bool removeFile(NtgFile fileStruct,  Node* node);
    bool createTmpFile(Graph* graph);
    bool destroyTmpFile(Graph* graph);
    bool saveTmpDir(QString fileName, Graph* graph, QDomDocument doc);
    QString createTmpDirFromNtg(Graph* graph, QString fileName);
    void linkNodeAndFiles(Node* node);
    bool reloadTmpDirFromScratch(Graph* graph);
    bool reloadFile(NtgFile fileStruct, Node* node);

    bool checkFileInCopyPath(Graph* graph);

    bool deleteAllDir();

    bool deleteDir(QString Path, QStringList donePath);


signals:

public slots:

private:
    explicit FileManager(QObject *parent = 0);
    static FileManager * _fileManager;

    QDir _tmpDir;

};


class FilePopWindow : public QDialog
{
Q_OBJECT
public:
    explicit FilePopWindow(Node * node, QWidget *parent = 0);
    virtual ~FilePopWindow();

    void updateList();

signals:

public slots:
    void addFile();
    void removeFile();

protected:
    QPushButton * _buttonAdd;
    QPushButton * _buttonRemove;
    QPushButton * _quit;
    QTableWidget * _view;

    Node * _node;
};



#endif // FILEMANAGER_H
