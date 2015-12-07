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

#include <QtCore/QCoreApplication>
#include <QFileInfo>
#include <QDebug>

#include <getopt.h>
#include <errno.h>

#include "ntgTypes.h"
#include "ntgDaemon.h"
#include "ntgTransformManager.h"

struct option long_options[] = {
 { "transforms-dir", required_argument, NULL, 't' },
 { "plugins-dir", required_argument, NULL, 'u' },
 { NULL, NULL, NULL, NULL }
};

int main(int argc, char *argv[])
{
  NtgDaemon daemon(argc, argv);

  int option_index = 0;
  int c = 0;

  QString transformDirPath = "./transforms/";
  QString pluginsDirPath = "./plugins/";
  while ((c=getopt_long(argc, argv, "t:u:", long_options, &option_index)) != -1)
  {
    switch (c)
    {
      case 't':
      {
        transformDirPath = optarg;
        break;
      }
      case 'u':
      {
        pluginsDirPath = optarg;
        break;
      }
    }
    option_index = 0;
  }

  if( optind+3 >= argc )
  {
    QString appName = QFileInfo(argv[0]).completeBaseName();
    fprintf(stderr,"Usage: %s [-u <plugins dir> |-t <xml dir> ] <transform> "
            "<input: <type> (<name> <val>)+ > [ \"\" <params...>]\n",
            qPrintable(appName));
    exit(1);
  }

  QString transformType = QString::fromLocal8Bit(argv[optind]);
  NtgEntity entity;
  entity.type = argv[optind+1];
  int offset = optind + 2;
  for(; offset + 1 < argc && qstrlen(argv[offset]) != 0; offset += 2)
    entity.values[QString::fromLocal8Bit(argv[offset])] = QString::fromLocal8Bit(argv[offset+1]);

  QHash<QString,QString> params;
  if( qstrlen(argv[offset]) == 0 )
  {
    offset++;
    for(; offset + 1 < argc; offset += 2)
      params[QString::fromLocal8Bit(argv[offset])] = QString::fromLocal8Bit(argv[offset+1]);
  }

  qDebug() << entity;
  qDebug() << params;

  daemon.daemonize();
  NtgTransformManager manager(transformDirPath, pluginsDirPath, NULL);
  NtgTransform * t = manager.createTransformFromType(transformType);
  if( ! t )
    qFatal("No such transform: %s", qPrintable(transformType));

  QObject::connect(t, SIGNAL(statusChanged(Ntg::TransformStatus)), &daemon, SLOT(quit()));
  t->exec(entity, params);

  return daemon.exec();
}
