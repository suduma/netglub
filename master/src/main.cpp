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

#include <QDir>
#include <QSslConfiguration>
#include <QSslKey>
#include <QNetworkInterface>
#include <QSqlDatabase>
#include <QSqlError>
#include <QUrl>
#include <getopt.h>

#include "ntgMaster.h"
#include "ntgDaemon.h"
#include "ntgLogger.h"

struct option long_options[] = {
  { "ip", required_argument, NULL, 'i' },
  { "port", required_argument, NULL, 'o' },
  { "tls-creds-dir", required_argument, NULL, 's' },
  { "logfile", required_argument, NULL, 'l' },
  { "pidfile", required_argument, NULL, 'p' },
  { "daemonize", no_argument, NULL, 'd' },
  { "event-log", required_argument, NULL, 'e' },
  { "client-port", required_argument, NULL, 'e' },
  { "db-url", required_argument, NULL, 'b' },
  { "session-time-out", required_argument, NULL, 't' },
  { "connection-time-out", required_argument, NULL, 'k' },
  { "transforms-path", required_argument, NULL, 'a' },
  { "entities-path", required_argument, NULL, 'f' },
  { NULL, NULL, NULL, NULL }
};


static bool getCACert(QDir tlsDir, QSslCertificate & cert)
{
  QFile certFile(tlsDir.absoluteFilePath("ca.crt"));
  if( ! certFile.open(QIODevice::ReadOnly))
  {
    qWarning("Can't open CA certificate");
    return false;
  }

  cert = QSslCertificate(&certFile);
  if(cert.isNull())
  {
    qWarning("Loading CA certificate failed");
    certFile.close();
    return false;
  }
  certFile.close();
  return true;
}


static bool getCertKeyPair(QString basename, QDir tlsDir, QSslCertificate & cert, QSslKey & key)
{
  QFile certFile(tlsDir.absoluteFilePath(basename+".crt"));
  if( ! certFile.open(QIODevice::ReadOnly))
  {
    qWarning("Can't open %s certificate", qPrintable(basename));
    return false;
  }

  cert = QSslCertificate(&certFile);
  if(cert.isNull())
  {
    qWarning("Loading %s certificate failed", qPrintable(basename));
    certFile.close();
    return false;
  }
  certFile.close();

  QFile keyFile(tlsDir.absoluteFilePath(basename+".key"));
  if( ! keyFile.open(QIODevice::ReadOnly))
  {
    qWarning("Can't open %s key", qPrintable(basename));
    return false;
  }

  key = QSslKey(&keyFile, QSsl::Rsa);
  if(key.isNull())
  {
    qWarning("Loading %s key failed", qPrintable(basename));
    keyFile.close();
    return false;
  }
  keyFile.close();

  return true;
}


static QHostAddress ipFromString(QString str)
{
  QNetworkInterface bindIf = QNetworkInterface::interfaceFromName(str);
  if(bindIf.isValid() && ! bindIf.addressEntries().isEmpty())
    return bindIf.addressEntries().first().ip();
  return QHostAddress(str);
}

static void closeDb()
{
  QSqlDatabase::removeDatabase("netglub");
}

void usage()
{
  QString help = QString("Usage: master [-i ip-address] [-o port] [-s <tls-creds-dir>]");
  help +=QString(" [-l <logfile>] [-p <pidfile>] [-d] [-h] [-e <event-log>] [-b db-url] [-c client-port]");
  help +=QString(" [-t sessionTimeOut] [-k connectionTimeOut] [-a <transformsPath>] [-f <entitiesPath>]\n");
  help +=QString("OPTIONS:\n");
  help +=QString("  -i    --ip                  : the listening ip address.\n");
  help +=QString("  -o    --port                : the port used to listen on, waiting for slaves.\n");
  help +=QString("  -s    --tls-creds-dir       : the directory of all the usable credential.\n");
  help +=QString("  -l    --logfile             : the file to log the deamon.\n");
  help +=QString("  -p    --pidfile             : the file containing the process id of this master.\n");
  help +=QString("  -d    --daemonize           : to daemonize or not (when ommited it thas not deamonize).\n");
  help +=QString("  -h    --help                : show this help.\n");
  help +=QString("  -e    --event-log           : the path to the event log file.\n");
  help +=QString("  -b    --db-url              : the URL of the database to use.\n");
  help +=QString("  -c    --client-port         : the port used to listen on, waiting for the client (cli or gui).\n");
  help +=QString("  -t    --session-duration    : timeOut after inactivity to remove a session in seconds.\n");
  help +=QString("  -k    --connection-duration : timeout after inactivity to close the connection (socket) in seconds.\n");
  help +=QString("  -a    --transforms-path     : path to the transforms dirs.\n");
  help +=QString("  -f    --entities-path       : path to the entities dirs.\n");
  help +=QString("EXAMPLE:\n");
  help +=QString("  sudo ./master -i 127.0.0.1 -o 2010 -s ./tools/tls_credentials/ -l /var/log/netglub_master.log -p /var/run/netglub_master.pid");
  help +=QString(" -d -e /var/log/netglub_master_event.log -b mysql://netglub:netglub@localhost/netglub2 -c 8080 -t 20 -k 60 ");
  help +=QString("-a ./data/transforms/ -f ./data/entities/\n");
  printf("%s",qPrintable(help));
}


int main(int argc, char *argv[])
{
  NtgDaemon daemon(argc, argv);

  int option_index = 0;
  int c = 0;

  QHostAddress ip("127.0.0.1");
  quint16 port = 2010;
  quint16 clientPort = 8080;
  quint16 sessionTimeOut = 20;
  quint16 connectionTimeOut = 60;
  QString tlsCredsDirPath("./tools/tls_credentials/");
  QString logPath("/var/log/netglub_master.log");
  QString pidPath("/var/run/netglub_master.pid");
  QString eventlogPath("/var/log/netglub_master_event.log");
  QUrl dbUrl("mysql://netglub:netglub@localhost/netglub");
  QString transformsPath("./data/transforms/");
  QString entitiesPath("./data/entities/");
  bool daemonize = false;
  while ((c=getopt_long(argc, argv, "i:o:s:l:p:dhe:b:c:t:k:a:f:", long_options, &option_index)) != -1)
  {
    switch (c)
    {
      case 'o':
      {
        port = QString(optarg).toUShort();
        break;
      }
      case 'c':
      {
        clientPort = QString(optarg).toUShort();
        break;
      }
      case 's':
      {
        tlsCredsDirPath = optarg;
        break;
      }
      case 'i':
      {
        ip = ipFromString(optarg);
        if( ip.isNull() )
          qFatal("Invalid address: %s", optarg);
        break;
      }
      case 'l':
      {
        logPath = optarg;
        break;
      }
      case 'p':
      {
        pidPath = optarg;
        break;
      }
      case 'h':
      {
        usage();
        return 0;
      }
      case 'd':
      {
        daemonize = true;
        break;
      }
      case 'e':
      {
        eventlogPath = optarg;
        break;
      }
      case 'b':
      {
        dbUrl = QUrl(optarg);
        break;
      }
      case 't':
      {
        sessionTimeOut = QString(optarg).toUShort();
        break;
      }
      case 'k':
      {
        connectionTimeOut = QString(optarg).toUShort();
        break;
      }
      case 'a':
      {
        transformsPath = optarg;
        break;
      }
      case 'f':
      {
        entitiesPath = optarg;
        break;
      }
      default:
      {
        usage();
        return 0;
      }
    }
    option_index = 0;
  }

  daemon.setPidfile(pidPath);

  if( optind < argc)
  {
    daemon.execCommand(argv[optind]);
    return 0;
  }
  else
  {
    if(daemonize)
      daemon.setForeground(false);

    daemon.setLogfile(logPath);

    daemon.daemonize();

    qsrand(time(NULL) * getpid());

    NtgLogger eventLogger(eventlogPath);

    // try to open DB
    QString dbDriverName = "Q" + dbUrl.scheme().toUpper();
    if( ! QSqlDatabase::isDriverAvailable(dbDriverName) )
      qFatal("No SQL driver for scheme %s", qPrintable(dbUrl.scheme()));
    QSqlDatabase db = QSqlDatabase::addDatabase(dbDriverName, dbUrl.path().remove(0,1));
    qAddPostRoutine(&closeDb);
    if( ! dbUrl.userName().isEmpty() ) db.setUserName(dbUrl.userName());
    if( ! dbUrl.password().isEmpty() ) db.setPassword(dbUrl.password());
    if( ! dbUrl.host().isEmpty()) db.setHostName(dbUrl.host());
    if( dbUrl.port() != -1 ) db.setPort(dbUrl.port());
    if( ! dbUrl.path().isEmpty() ) db.setDatabaseName(dbUrl.path().mid(1));

    if ( ! db.open())
      qFatal("Could not open database : %s", qPrintable(db.lastError().text()));

    QDir tlsDir(tlsCredsDirPath);
    if( ! tlsDir.exists())
      qFatal("No such directory: \"%s\"", qPrintable(tlsCredsDirPath));

    QSslCertificate caCert;
    if( ! getCACert(tlsDir, caCert))
      qFatal("Could not load CA certificate. Dir :\"%s\"", qPrintable(tlsCredsDirPath));

    QSslCertificate serverCert;
    QSslKey serverKey;
    if( ! getCertKeyPair("server", tlsDir, serverCert, serverKey))
      qFatal("Could not load server certificate. Dir :\"%s\"", qPrintable(tlsCredsDirPath));

    QSslConfiguration tlsConf;
    tlsConf.setCaCertificates(QList<QSslCertificate>() << caCert);
    tlsConf.setLocalCertificate(serverCert);
    tlsConf.setPrivateKey(serverKey);
    tlsConf.setPeerVerifyMode(QSslSocket::VerifyPeer);
    tlsConf.setProtocol(QSsl::TlsV1);

    qDebug("Starting on : %s:%d", qPrintable(ip.toString()), port);

    NtgMaster master(ip, port, clientPort, tlsConf, &db, sessionTimeOut, connectionTimeOut,
                     transformsPath,entitiesPath);

    return daemon.exec();
  }
  // never reached
  return 1;
}
