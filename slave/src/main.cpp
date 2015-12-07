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

#include <QMetaType>
#include <QDir>
#include <QSslConfiguration>
#include <QSslKey>
#include <getopt.h>
#include <errno.h>
#include "ntgTransformManager.h"

#include "ntgDaemon.h"
#include "ntgSlave.h"
#include "ntgLogger.h"

struct option long_options[] = {
 { "transforms-dir", required_argument, NULL, 't' },
 { "plugins-dir", required_argument, NULL, 'u' },
 { "master", required_argument, NULL, 'm' },
 { "tls-creds-dir", required_argument, NULL, 's' },
 { "logfile", required_argument, NULL, 'l' },
 { "pidfile", required_argument, NULL, 'p' },
 { "daemonize", no_argument, NULL, 'd' },
 { "help", no_argument, NULL, 'h' },
 { "event-log", required_argument, NULL, 'e' },
 { "reconnection-time-out", required_argument, NULL, 'c' },
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

void usage()
{
  QString help = QString("Usage: slave [-t <transforms-dir>] [-u <plugins-dir>] [-m master] [-c reconnectionTimeOut]");
  help +=QString("[-s <tls-creds-dir>] [-l <logfile>] [-p <pidfile>] [-d] [-h] [-e <event-log>]\n");
  help +=QString("OPTIONS:\n");
  help +=QString("  -t:    the transforms directory.\n");
  help +=QString("  -u:    the plugins directory.\n");
  help +=QString("  -m:    the ip address and the port of the master.\n");
  help +=QString("  -s:    the directory of all the usable credential.\n");
  help +=QString("  -l:    the file to log the deamon.\n");
  help +=QString("  -p:    the file containing the process id of this master.\n");
  help +=QString("  -d:    to daemonize or not (when ommited it thas not deamonize).\n");
  help +=QString("  -h:    show this help.\n");
  help +=QString("  -e:    the path to the event log file.\n");
  help +=QString("  -c:    the time to wait after a failed connection to the master to retry the connection.\n");
  help +=QString("EXAMPLE:\n");
  help +=QString("  sudo ./slave -t data/transforms/ -u data/plugins/ -m 127.0.0.1:2010 -s ./tools/tls_credentials/ -c 5 -l ");
  help +=QString("/var/log/netglub_slave.log -p /var/run/netglub_slave.pid -d -e /var/log/netglub_slave_event.log\n");
  printf("%s",qPrintable(help));
}


int main(int argc, char *argv[])
{
  NtgDaemon daemon(argc, argv);

  int option_index = 0;
  int c = 0;

  bool daemonize = false;
  QString transformDirPath = "data/transforms/";
  QString pluginsDirPath = "data/plugins/";
  QString masterLocation = "127.0.0.1:2010";
  QString tlsCredsDirPath = "./tools/tls_credentials/";
  QString logPath = "/var/log/netglub_slave.log";
  QString pidPath = "/var/run/netglub_slave.pid";
  QString eventlogPath = "/var/log/netglub_slave_event.log";
  quint16 reconnectionTimeOut = 5;
  while ((c=getopt_long(argc, argv, "t:u:m:s:l:p:de:hc:", long_options, &option_index)) != -1)
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
      case 'm':
      {
        masterLocation = optarg;
        break;
      }
      case 's':
      {
        tlsCredsDirPath = optarg;
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
      case 'h':
      {
        usage();
        return 0;
      }
      case 'c':
      {
        reconnectionTimeOut = QString(optarg).toUShort();
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

    NtgLogger eventLogger(eventlogPath);

    QDir tlsDir(tlsCredsDirPath);
    if( ! tlsDir.exists())
      qFatal("No such directory: \"%s\"", qPrintable(tlsCredsDirPath));

    QSslCertificate caCert;
    if( ! getCACert(tlsDir, caCert))
      qFatal("Could not load CA certificate. Dir :\"%s\"", qPrintable(tlsCredsDirPath));

    QSslCertificate clientCert;
    QSslKey clientKey;
    if( ! getCertKeyPair("client", tlsDir, clientCert, clientKey))
      qFatal("Could not load client certificate. Dir :\"%s\"", qPrintable(tlsCredsDirPath));

    QSslConfiguration tlsConf;
    tlsConf.setCaCertificates(QList<QSslCertificate>() << caCert);
    tlsConf.setLocalCertificate(clientCert);
    tlsConf.setPrivateKey(clientKey);
    tlsConf.setPeerVerifyMode(QSslSocket::VerifyPeer);
    tlsConf.setProtocol(QSsl::TlsV1);

    qDebug("Starting on : %s", qPrintable(masterLocation));

    NtgSlave slave(masterLocation, transformDirPath, pluginsDirPath, tlsConf,reconnectionTimeOut);
    return daemon.exec();
  }
  // never reached
  return 1;
}
