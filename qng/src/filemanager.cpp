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

#include "filemanager.h"
#include "quazip/quazip.h"
#include "quazip/quazipfile.h"
#include "quazip/quazipfileinfo.h"
#include <QSettings>
#include <QFileInfo>
#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>
#include <QGridLayout>
#include <QPair>
#include <QMessageBox>




FileManager * FileManager::_fileManager = NULL;

FileManager::FileManager(QObject *parent) :
    QObject(parent)
{
    QSettings settings;
    QFileInfo settingsPath = QFileInfo(settings.fileName());
    QDir settingsDir = settingsPath.absoluteDir();

    if (!settingsDir.exists("tmp"))
        settingsDir.mkdir("tmp");
    else
    {
        _tmpDir = QDir(settingsDir);
        _tmpDir.cd("tmp");
        deleteAllDir();
    }

    _tmpDir = QDir(settingsDir);//.cd("tmp");
    _tmpDir.cd("tmp");


}


FileManager::~FileManager()
{

}

FileManager * FileManager::getOrCreate()
{
    if (!_fileManager || _fileManager == NULL)
    {
        _fileManager = new FileManager();
    }
    return _fileManager;
}



NtgFile FileManager::addFile(NtgFile fileStruct, Node* node)
{
    QDir tmp(_tmpDir);
    Graph * graph = node->getGraph();
    if (!tmp.cd(QString::number(graph->getGraphId(),10)))
    {
        if (!reloadTmpDirFromScratch(graph))
            return fileStruct;
        tmp.cd(QString::number(graph->getGraphId(),10));
    }
    QFileInfo srcFile = QFileInfo(fileStruct.srcPath);
    QFile file(fileStruct.srcPath);
    QFileInfo cpFile= QFileInfo(tmp,QString::number(node->getId(),10)+"-"+QString::number(fileStruct.id,10)+"-"+fileStruct.shortName);
    QString newFilePath = cpFile.absoluteFilePath();
    file.copy(newFilePath);
    fileStruct.cpPath = newFilePath;
    return fileStruct;
}

bool FileManager::removeFile(NtgFile fileStruct, Node * node)
{
    QDir tmp(_tmpDir);
    Graph * graph = node->getGraph();
    if (!tmp.cd(QString::number(graph->getGraphId(),10)))
    {
        if (!reloadTmpDirFromScratch(graph))
            return false;
        tmp.cd(QString::number(graph->getGraphId(),10));
    }

    return tmp.remove(fileStruct.cpPath);

}

bool FileManager::createTmpFile(Graph* graph)
{
    return _tmpDir.mkdir(QString::number(graph->getGraphId(),10));
}

bool FileManager::destroyTmpFile(Graph* graph)
{
    QDir tmp(_tmpDir);
    if (tmp.cd(QString::number(graph->getGraphId(),10)))
    {
        QFileInfoList files=tmp.entryInfoList();
        foreach(QFileInfo file, files)
        {
            tmp.remove(file.fileName());
        }
        return _tmpDir.rmdir(QString::number(graph->getGraphId(),10));
    }
    return true;
}



bool FileManager::saveTmpDir(QString fileName, Graph* graph, QDomDocument doc)
{
    QDir tmp(_tmpDir);
    if (!tmp.cd(QString::number(graph->getGraphId(),10)))
    {
        if (!reloadTmpDirFromScratch(graph))
            return false;
        tmp.cd(QString::number(graph->getGraphId(),10));
    }

    QFileInfo cpFile= QFileInfo(tmp,"graph.ntgxml");

    QFile file(cpFile.absoluteFilePath());
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
            return false;
    QTextStream out(&file);
    out << doc.toString() << endl;

    checkFileInCopyPath(graph);

    QuaZip zip(fileName);
    if(!zip.open(QuaZip::mdCreate)) {
      qWarning("testCreate(): zip.open(): %d", zip.getZipError());
      return false;
    }

    QFileInfoList files=QDir(tmp).entryInfoList();

    QFile inFile;
    QuaZipFile outFile(&zip);
    char c;
    foreach(QFileInfo file, files)
    {
      if(!file.isFile()||file.fileName()==fileName) continue;
      inFile.setFileName(file.absoluteFilePath());
      if(!inFile.open(QIODevice::ReadOnly)) {
        qWarning("testCreate(): inFile.open(): %s", inFile.errorString().toLocal8Bit().constData());
        return false;
      }
      if(!outFile.open(QIODevice::WriteOnly, QuaZipNewInfo(inFile.fileName().split("/").last(), inFile.fileName()))) {
        qWarning("testCreate(): outFile.open(): %d", outFile.getZipError());
        return false;
      }
      while(inFile.getChar(&c)&&outFile.putChar(c)){};
      if(outFile.getZipError()!=UNZ_OK) {
        qWarning("testCreate(): outFile.putChar(): %d", outFile.getZipError());
        return false;
      }
      outFile.close();
      if(outFile.getZipError()!=UNZ_OK) {
        qWarning("testCreate(): outFile.close(): %d", outFile.getZipError());
        return false;
      }
      inFile.close();
    }
    zip.close();
    if(zip.getZipError()!=0) {
      qWarning("testCreate(): zip.close(): %d", zip.getZipError());
      return false;
    }
    return true;
}

QString FileManager::createTmpDirFromNtg(Graph* graph, QString fileName)
{


    QuaZip zip(fileName);
    if(!zip.open(QuaZip::mdUnzip)) {
      qWarning("testRead(): zip.open(): %d", zip.getZipError());
      return "";
    }
    zip.setFileNameCodec("IBM866");
    QuaZipFileInfo info;
    QuaZipFile file(&zip);
    QFile out;
    QString name;

    QDir tmp(_tmpDir);

    if (!tmp.cd(QString::number(graph->getGraphId(),10)))
    {
        if (!createTmpFile(graph))
            return false;
        tmp.cd(QString::number(graph->getGraphId(),10));
    }


    QString fileResult ="";
    char c;
    for(bool more=zip.goToFirstFile(); more; more=zip.goToNextFile()) {
        if(!zip.getCurrentFileInfo(&info)) {
            qWarning("testRead(): getCurrentFileInfo(): %d\n", zip.getZipError());
            return "";
        }
        if(!file.open(QIODevice::ReadOnly)) {
            qWarning("testRead(): file.open(): %d", file.getZipError());
            return "";
        }
        name=file.getActualFileName();
//        qDebug() << name;
        if(file.getZipError()!=UNZ_OK) {
            qWarning("testRead(): file.getFileName(): %d", file.getZipError());
            return "";
        }

        QFileInfo filetry = QFileInfo(tmp,name);

        if (filetry.fileName() == "graph.ntgxml")
        {
            fileResult = filetry.absoluteFilePath();
        }
        out.setFileName(filetry.absoluteFilePath());
        out.open(QIODevice::WriteOnly);
        while(file.getChar(&c)) out.putChar(c);
        out.close();
        if(file.getZipError()!=UNZ_OK) {
            qWarning("testRead(): file.getFileName(): %d", file.getZipError());
            return "";
        }
        if(!file.atEnd()) {
            qWarning("testRead(): read all but not EOF");
            return "";
        }
        file.close();
        if(file.getZipError()!=UNZ_OK) {
            qWarning("testRead(): file.close(): %d", file.getZipError());
            return "";
        }
    }
    zip.close();
    if(zip.getZipError()!=UNZ_OK) {
        qWarning("testRead(): zip.close(): %d", zip.getZipError());
        return "";
    }
    return fileResult;

}

bool FileManager::reloadTmpDirFromScratch(Graph *graph)
{
    //nearly the same as reload file but for each file.
    bool noToAll = false;
    createTmpFile(graph);
    foreach(Node * node, graph->getNodeList())
    {
        foreach(NtgFile fileStruct, node->getFileMap())
        {
            if (noToAll)
            {
                node->removeFileFromMap(fileStruct.id);
            }
            else
            {
                if (fileStruct.srcPath.isEmpty())
                {

                    QMessageBox::StandardButton reply;
                    reply = QMessageBox::question(0, tr("File not found"),
                                                    "The file \"" + fileStruct.shortName +"\" was not found.\nDo you want to reload it?",
                                                    QMessageBox::Yes | QMessageBox::No | QMessageBox::NoAll);
                    node->removeFileFromMap(fileStruct.id);


                    if (reply == QMessageBox::Yes)
                    {
                        QFileDialog::Options options;
                        QString selectedFilter;
                        QString fileName2 = QFileDialog::getOpenFileName(0,
                                                    tr("Change file path"),
                                                    "hello world",
                                                    tr("All Files (*)"),
                                                    &selectedFilter,
                                                    options);

                        if (!fileName2.isEmpty())
                        {
                            node->addFile(fileName2);
                        }
                    }
                    else if (reply == QMessageBox::NoAll)
                        noToAll = true;
                }
                else
                {
                    QFileInfo fileInfo = QFileInfo(fileStruct.srcPath);
                    if (fileInfo.isFile())
                    {
                        if (addFile(fileStruct,node).shortName.isEmpty())
                            return false;
                    }
                    else
                    {
                        QMessageBox::StandardButton reply;
                        reply = QMessageBox::question(0, tr("File not found"),
                                                        "The file \"" + fileStruct.shortName +"\" was not found.\nDo you want to choose a new path?",
                                                        QMessageBox::Yes | QMessageBox::No | QMessageBox::NoAll);
                        node->removeFileFromMap(fileStruct.id);
                        if (reply == QMessageBox::Yes)
                        {
                            QFileDialog::Options options;
                            QString selectedFilter;
                            QString fileName2 = QFileDialog::getOpenFileName(0,
                                                        tr("Change file path"),
                                                        "hello world",
                                                        tr("All Files (*)"),
                                                        &selectedFilter,
                                                        options);

                            if (!fileName2.isEmpty())
                                node->addFile(fileName2);
                        }
                        else if (reply == QMessageBox::NoAll)
                            noToAll = true;
                    }
                }
            }
        }
        return true;
    }



    return true;
}

bool FileManager::reloadFile(NtgFile fileStruct, Node * node)
{
    if (fileStruct.srcPath.isEmpty())
    {

        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(0, tr("File not found"),
                                        "The file \"" + fileStruct.shortName +"\" was not found.\nDo you want to reload it?",
                                        QMessageBox::Yes | QMessageBox::No );

        node->removeFileFromMap(fileStruct.id);


        if (reply == QMessageBox::Yes)
        {
            QFileDialog::Options options;
            QString selectedFilter;
            QString fileName2 = QFileDialog::getOpenFileName(0,
                                        tr("Change file path"),
                                        "hello world",
                                        tr("All Files (*)"),
                                        &selectedFilter,
                                        options);

            if (!fileName2.isEmpty())
            {
                node->addFile(fileName2);
                return true;
            }
            else
                return false;
        }
        return true;


        //try to unzip from the zip file, not working
    }
    else
    {
        QFileInfo fileInfo = QFileInfo(fileStruct.srcPath);
        if (fileInfo.isFile())
        {
            if (addFile(fileStruct,node).shortName.isEmpty())
                return false;
        }
        else
        {
            QMessageBox::StandardButton reply;
            reply = QMessageBox::question(0, tr("File not found"),
                                            "The file \"" + fileStruct.shortName +"\" was not found.\nDo you want to choose a new path?",
                                            QMessageBox::Yes | QMessageBox::No);
            node->removeFileFromMap(fileStruct.id);
            if (reply == QMessageBox::Yes)
            {
                QFileDialog::Options options;
                QString selectedFilter;
                QString fileName2 = QFileDialog::getOpenFileName(0,
                                            tr("Change file path"),
                                            "hello world",
                                            tr("All Files (*)"),
                                            &selectedFilter,
                                            options);

                if (!fileName2.isEmpty())
                    node->addFile(fileName2);
                else
                    return false;
            }
            else
                return false;
        }
        return true;

    }

}

bool FileManager::deleteAllDir()
{
    QStringList donePath;
    return deleteDir(_tmpDir.absolutePath(),donePath);

}

bool FileManager::deleteDir(QString Path, QStringList donePath)
{
    QDir dir(Path);
    QFileInfoList files=dir.entryInfoList();
    foreach(QFileInfo file, files)
    {
        if (!donePath.contains(file.absoluteFilePath()) && file.fileName() != "." && file.fileName() != "..")
        {
            donePath.append(file.absoluteFilePath());
            if (file.isDir())
            {

                deleteDir(file.absoluteFilePath(), donePath);
                dir.rmdir(file.absoluteFilePath());
            }
            else
                dir.remove(file.absoluteFilePath());
        }
    }
    return true;
}

void FileManager::linkNodeAndFiles(Node* node)
{
    QDir tmp(_tmpDir);
    Graph * graph = node->getGraph();
    if (!tmp.cd(QString::number(graph->getGraphId(),10)))
    {
        if (!reloadTmpDirFromScratch(graph))
            return;
        tmp.cd(QString::number(graph->getGraphId(),10));
    }
    QFileInfoList files=QDir(tmp).entryInfoList();



    foreach(NtgFile fileStruct, node->getFileMap())
    {
        QFileInfo fileInfo = QFileInfo(tmp,QString::number(node->getId(),10)+"-"+QString::number(fileStruct.id,10)+"-"+fileStruct.shortName);
        if (fileInfo.isFile())
        {
            fileStruct.cpPath = fileInfo.absoluteFilePath();
            qreal t = fileInfo.size();
            fileStruct.size = QString::number(t/1000);
            node->addFileOnlyToMap(fileStruct);
        }
        else
        {
             qCritical() << "error while loading file :\""+fileStruct.shortName+"\"";
        }
    }


}

bool FileManager::checkFileInCopyPath(Graph* graph)
{
    foreach(Node * node, graph->getNodeList())
    {
        foreach (NtgFile fileStructCheck, node->getFileMap())
        {
            QFileInfo fileInfo = QFileInfo(fileStructCheck.cpPath);
            if (!fileInfo.isFile())
                reloadFile(fileStructCheck,node);
        }
    }

    return true;
}



/*-------------------------------------------------------------------
  ------ class FilePopWindow
  ------------------------------------------------------------------*/

FilePopWindow::FilePopWindow(Node * node, QWidget *parent) : QDialog(parent)
{
    _node = node;

    _buttonAdd = new QPushButton(QIcon(":/images/add.png"),"",this);
    connect(_buttonAdd,SIGNAL(clicked()),this,SLOT(addFile()));

    _buttonRemove = new QPushButton(QIcon(":/images/remove.png"),"",this);
    connect(_buttonRemove,SIGNAL(clicked()),this,SLOT(removeFile()));

    _quit = new QPushButton(QIcon(":/images/exit.png"),"quit",this);
    connect(_quit,SIGNAL(clicked()),this,SLOT(close()));

    _view = new QTableWidget(this);
//    _view->verticalHeader()->verticalHeader()->setVisible(false);
//    _view->setSelectionMode(s);
    _view->setSelectionBehavior(QAbstractItemView::SelectRows);
    _view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    updateList();

    QGridLayout * _layout = new QGridLayout(this);
    _layout->addWidget(_view,0,0,6,2);
    _layout->addWidget(_buttonAdd,0,2,1,1,Qt::AlignTop);
    _layout->addWidget(_buttonRemove,1,2,1,1,Qt::AlignTop);
    _layout->addWidget(_quit,5,2,1,1,Qt::AlignBottom);
    _layout->setColumnStretch(1,1);
    _layout->setColumnStretch(0,1);
    setLayout(_layout);
}

FilePopWindow::~FilePopWindow()
{

}

void FilePopWindow::addFile()
{
    QFileDialog::Options options;
    QString selectedFilter;
    QString fileName = QFileDialog::getOpenFileName(this,
                                tr("Add file to nodes"),
                                "hello world",
                                tr("All Files (*)"),
                                &selectedFilter,
                                options);

    if (!fileName.isEmpty())
    {
         _node->addFile(fileName);
         updateList();
    }
}

void FilePopWindow::removeFile()
{
    if(_view->currentRow()>-1)
    {
        _node->removeFileFromMap(_view->item(_view->currentRow(),2)->data(Qt::DisplayRole).toInt());
    }
    updateList();
}


void FilePopWindow::updateList()
{
    _view->setColumnCount(3);
    _view->setRowCount(_node->getFileMap().count());
    _view->setHorizontalHeaderLabels(QStringList() << tr("File")
                                                   << tr("Source Path")
                                                   << tr("id"));
    _view->hideColumn(2);
    int i =0;
    foreach(NtgFile file, _node->getFileMap())
    {
        QTableWidgetItem * fileItem = new QTableWidgetItem(file.shortName);
        QTableWidgetItem * srcFileItem = new QTableWidgetItem(file.srcPath);
        QTableWidgetItem * idItem = new QTableWidgetItem(QString::number(file.id,10));
        _view->setItem(i,0,fileItem);
        _view->setItem(i,1,srcFileItem);
        _view->setItem(i++,2,idItem);
    }

}








