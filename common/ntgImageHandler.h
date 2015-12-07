/******************************************************************************
**
** Copyright (C) 2009-2010 DIATEAM. All rights reserved.
**
** This file is part of the "NetGlub" GUI program.
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

#ifndef NTGIMAGEHANDLER_H
#define NTGIMAGEHANDLER_H

#include <QString>
#include <QtGui/QImage>
#include <QBuffer>
#include <QFile>
#include <QDebug>

class NtgImageHandler : public QObject
{
Q_OBJECT
public:
    NtgImageHandler(const QString & path);
    NtgImageHandler(QByteArray bytes,bool b);
    virtual ~NtgImageHandler();
    QByteArray getByteArray();
    void saveImage(const QString & path);

protected:
    QImage *_img;
};



#endif // NTGIMAGEHANDLER_H
