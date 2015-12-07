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

#include <QtGui/QApplication>
#include "mainwindow.h"
#include "connectionwidgets.h"
#include <QDebug>
#include <QPixmap>
#include <QSplashScreen>
#include <unistd.h>

#define WAIT_SPLASHSCREEN

int main(int argc, char *argv[])
{

    QApplication app(argc, argv);
    app.setOrganizationName("Diateam");
    app.setApplicationName("qng");
    app.addLibraryPath("lib");

    QPixmap step0(":/images/splash/splash.png");
    QPixmap step1(":/images/splash/splash-1.png");
    QPixmap step2(":/images/splash/splash-2.png");
    QPixmap step3(":/images/splash/splash-3.png");
    QPixmap step4(":/images/splash/splash-4.png");
    QSplashScreen splash(step0, Qt::WindowStaysOnTopHint);
    splash.show();

    splash.showMessage("STEP 1 (20%) - Loading interface's ressources",
    Qt::AlignRight | Qt::AlignBottom,Qt::white);

    int waitForMs = 250;

#ifdef WAIT_SPLASHSCREEN
    usleep(waitForMs);
#endif
    splash.setPixmap(step1);
#ifdef WAIT_SPLASHSCREEN
    usleep(waitForMs);
#endif
    app.processEvents();
    splash.showMessage("STEP 2 (35%) - Loading plugins",
      Qt::AlignRight | Qt::AlignBottom,Qt::white);
    splash.setPixmap(step2);
#ifdef WAIT_SPLASHSCREEN
    usleep(waitForMs);
#endif
    QPointF center = splash.geometry().center();

    ConnectionDialog * CD = new ConnectionDialog();
//    CD->move(splash.geometry().center() - CD->rect().center());
    bool continueToIHM = false;
    if (CD->continuToIHM())
        continueToIHM = true;
    delete CD;
    app.processEvents();  
    splash.showMessage("STEP 3 (75%) - Connecting to master",
      Qt::AlignRight | Qt::AlignBottom,Qt::white);
    splash.setPixmap(step3);

    if (continueToIHM)
    {
        MainWindow * mainWindow = MainWindow::create();
        app.processEvents();  
        splash.showMessage("STEP 4 (100%) - Retrieving data",
          Qt::AlignRight | Qt::AlignBottom,Qt::white);
        splash.setPixmap(step4);
        mainWindow->show();
        app.processEvents();
#ifdef WAIT_SPLASHSCREEN
//        QTest::qWait (10*waitForMs);
#endif
        splash.close();
        return app.exec();
    }
    else
    {
       splash.close();
       return 0;
    }

}
