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

#include "ntgImageHandler.h"

NtgImageHandler::NtgImageHandler(const QString & path )
{
    QFile file(path);
    if (file.exists())
    {
        _img = new QImage(path);
    }
    else
    {
        if (path != "")
            qWarning() << "no such image : " << path ;
        _img = new QImage();
    }
}

NtgImageHandler::NtgImageHandler(QByteArray bytes,bool b)
{
  Q_UNUSED(b);
  _img = new QImage();
  _img->loadFromData(bytes,"PNG");
}

NtgImageHandler::~NtgImageHandler()
{
  delete _img;
}


QByteArray NtgImageHandler::getByteArray()
{
  if (_img)
  {
    QByteArray bytes;
    QBuffer buffer(&bytes);
    buffer.open(QIODevice::WriteOnly);
    _img->save(&buffer,"PNG");
    return bytes;
  }
  else
  {
    return "";
  }
}

void NtgImageHandler::saveImage(const QString & path)
{
    _img->save(path,"PNG");
}

