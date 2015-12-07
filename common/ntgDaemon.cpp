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

#include "ntgDaemon.h"

#include <QDateTime>
#include <QTextCodec>

#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <pwd.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/socket.h>

int NtgDaemon::_sighupFd[2] = {0,0};
bool NtgDaemon::_sigHandlerOnce = false;
bool NtgDaemon::_logToConsole = true;
QFile * NtgDaemon::_logfile = NULL;


typedef void (*SignalHandler)(int);

static bool installSignalHandler(int signum, SignalHandler handler)
{
  struct sigaction act, old;
  sigemptyset(&act.sa_mask);
  act.sa_handler = reinterpret_cast<void (*)(int)>(handler);
  act.sa_flags = 0;
  int r;
  do
  {
    r = sigaction(signum, &act, &old);
  } while(( r == -1 ) && ( errno == EINTR ));

  return ( r != -1 );
}


NtgDaemon::NtgDaemon(int& argc, char **& argv)
  : QCoreApplication(argc, argv)
  , _userId(0)
  , _socketNotifier(NULL)
  , _pidfile(NULL)
  , _foreground(true)
{
  QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));

  qInstallMsgHandler(&NtgDaemon::_qtMessageHandler);

  if(::geteuid() != 0)
    qFatal("This program must be run as root.");

  _userId = ::getuid();
  if( _userId == 0)
  {
    // UID == EUID == 0, we're run as sudo, we need to get _userId to non-root
    // try to get SUDO_UID
    bool ok = false;
    _userId = QString(::getenv("SUDO_UID")).toInt(&ok);
    if( ! ok || _userId == 0)
    {
      passwd * p= ::getpwnam("nobody");
      if( ! p )
        qFatal("Can't find user 'nobody' !");
      _userId = p->pw_uid;
    }
  }
}


NtgDaemon::~NtgDaemon()
{
  _cleanup();
}


void NtgDaemon::daemonize()
{
  if( ! _foreground)
  {    
    switch (::fork())
    {
      case 0:
        break;
      case -1:
        // Error
        qFatal("Error demonizing (fork)! %d - %s\n", errno, ::strerror(errno));
        break;
      default:
        ::_exit(0);
    }

    if (::setsid() < 0)
      qFatal("Error demonizing (setsid)! %d - %s\n", errno, ::strerror(errno));

    switch (::fork())
    {
      case 0:
        break;
      case -1:
        // Error
        qFatal("Error demonizing (second fork)! %d - %s\n", errno, ::strerror(errno));
        break;
      default:
        ::_exit(0);
    }
  }
  // TODO : chdir("/");

  if ( ! _pidfilePath.isEmpty())
  {
    _pidfile = new QFile(_pidfilePath);
    if( ! _pidfile->open(QIODevice::ReadWrite))
      qFatal("Cannot open pidfile %s",qPrintable(_pidfilePath));
    if ( ::lockf(_pidfile->handle(), F_TLOCK, 0) < 0)
      qFatal("Can't get a lock on %s. another instance is already running.",qPrintable(_pidfilePath));
    QByteArray pid = QByteArray::number(getpid());
    if( _pidfile->write(pid) != pid.size())
      qFatal("Can't write pid in pidfile %s.",qPrintable(_pidfilePath));
    _pidfile->flush();
  }

  if (::socketpair(AF_UNIX, SOCK_STREAM, 0, _sighupFd))
         qFatal("Couldn't create HUP socketpair");

  _socketNotifier = new QSocketNotifier(_sighupFd[1], QSocketNotifier::Read, this);
  connect(_socketNotifier, SIGNAL(activated(int)), this, SLOT(_handleSighup()));

  int sigs[] = { SIGINT, SIGQUIT, SIGILL, SIGTRAP, SIGABRT, SIGBUS,
                 SIGFPE, SIGUSR1, SIGSEGV, SIGUSR2, SIGPIPE, SIGALRM,
                 SIGTERM, SIGTSTP, SIGTTIN, SIGTTOU, SIGURG, SIGXCPU,
                 SIGXFSZ, SIGVTALRM, SIGPROF, SIGPOLL, SIGIO, SIGSYS};

  installSignalHandler(SIGHUP, NtgDaemon::_hupSignalHandler);

  for(int i = 0; i < 24; i++)
    installSignalHandler(sigs[i], NtgDaemon::_otherSignalHandler);

  if( ! _foreground)
  {
    _logToConsole = false;
    ::freopen( "/dev/null", "r", stdin);
    ::freopen( "/dev/null", "w", stdout);
    ::freopen( "/dev/null", "w", stderr);
  }
}


void NtgDaemon::execCommand(QString cmd)
{
  if ( _pidfilePath.isEmpty())
    qFatal("Can't locate running instance without a pidfile");

  QFile pidfile(_pidfilePath);

  bool status = (cmd == "status");
  if( status && ! pidfile.exists())
  {
    printf("Status : not running\n");
    return;
  }

  if( ! pidfile.open(QIODevice::ReadOnly))
    qFatal("Cannot open pidfile %s",qPrintable(_pidfilePath));
  QByteArray pidStr = pidfile.readAll();
  bool ok = false;
  int pid = pidStr.toInt(&ok);
  if( ! ok )
    qFatal("Pidfile %s does not contain a pid.",qPrintable(_pidfilePath));

  bool running = (::kill(pid, 0) == 0);

  if(status)
  {
    printf("Pid [%d] Status : %s\n", pid, (running ? "running" : "not running"));
    return;
  }
  if( ! running )
  {
    qWarning("No such process %d, removing pidfile.", pid);
    pidfile.remove();
    return;
  }

  if( cmd == "reload")
  {
    if( ::kill(pid, SIGHUP) < 0)
      qFatal("Reloading failed. pid: %d error: %s", pid, strerror(errno));
    printf("Reloading pid %d successful.\n", pid);
  }
  else if (cmd == "stop")
  {
    if( ::kill(pid, SIGINT) < 0)
      qFatal("Stopping failed. pid: %d error: %s", pid, strerror(errno));
    printf("Stopping pid %d successful.\n", pid);
  }
  else
  {
    qFatal("Unknown command: %s", qPrintable(cmd));
  }
}


void NtgDaemon::setForeground(bool f)
{
  _foreground = f;
}


bool NtgDaemon::setPidfile(QString path)
{
  if( _pidfile)
    return false; // TODO can change
  if( ! path.startsWith('/'))
    path.prepend("/var/run/");
  _pidfilePath = path;
  return true;
}


bool NtgDaemon::setLogfile(QString path)
{
  if( _logfile)
    return false; // TODO can change
  if( ! path.startsWith('/'))
    path.prepend("/var/log/");
  _logfile = new QFile(path);
  if( ! _logfile->open(QIODevice::WriteOnly | QIODevice::Append) )
  {
    delete _logfile;
    return false;
  }
  QByteArray sep(78, '-');
  _logfile->write(sep);
  _logfile->write("\n");
  return true;
}


bool NtgDaemon::setNonRootUser(QString name)
{
  passwd * p = ::getpwnam(qPrintable(name));
  if( ! p )
  {
    qWarning("Can't find user '%s'", qPrintable(name));
    return false;
  }
  _userId = p->pw_uid;
  return true;
}


bool NtgDaemon::dropPrivileges()
{
  if (::seteuid(_userId) != 0)
  {
    qCritical("Can't drop privileges !");
    return false;
  }
  return true;
}


bool NtgDaemon::dropPrivilegesPermanently()
{
  if (::geteuid() != 0)
  {
    if (::seteuid(0) != 0)
    {
      qCritical("Can't regain privileges temporarily to drop them permanently !");
      return false;
    }
  }
  if (::setuid(_userId) != 0)
  {
    qCritical("Can't drop privileges permanently !");
    return false;
  }

  if (::setuid(0) != -1)
  {
    qCritical("Privileges were not dropped permanently !");
    return false;
  }
  return true;
}


bool NtgDaemon::regainPrivileges()
{
  if (::seteuid(0) != 0)
  {
    qCritical("Can't regain privileges !");
    return false;
  }
  return true;
}


void NtgDaemon::_handleSighup()
{
  _socketNotifier->setEnabled(false);
  char tmp;
  ::read(_sighupFd[1], &tmp, sizeof(tmp));
  emit reload();
  _socketNotifier->setEnabled(true);
}


void NtgDaemon::_hupSignalHandler(int /* sig */)
{
  char a = 1;
  ::write(_sighupFd[0], &a, sizeof(a));
}


#define SIG_CASE(SUFFIX) case SIG ## SUFFIX : return ( #SUFFIX );

static QString signalToString(int signum)
{
  switch(signum)
  {
    SIG_CASE(HUP)
    SIG_CASE(INT);
    SIG_CASE(QUIT);
    SIG_CASE(ILL);
    SIG_CASE(TRAP);
    SIG_CASE(ABRT);
    SIG_CASE(BUS);
    SIG_CASE(FPE);
    SIG_CASE(KILL);
    SIG_CASE(USR1);
    SIG_CASE(SEGV);
    SIG_CASE(USR2);
    SIG_CASE(PIPE);
    SIG_CASE(ALRM);
    SIG_CASE(TERM);
    SIG_CASE(CHLD);
    SIG_CASE(CONT);
    SIG_CASE(STOP);
    SIG_CASE(TSTP);
    SIG_CASE(TTIN);
    SIG_CASE(TTOU);
    SIG_CASE(URG);
    SIG_CASE(XCPU);
    SIG_CASE(XFSZ);
    SIG_CASE(VTALRM);
    SIG_CASE(PROF);
    SIG_CASE(WINCH);
    SIG_CASE(POLL);
    SIG_CASE(SYS);
  }
  return(QString("SIG %1").arg(signum));
}


void NtgDaemon::_otherSignalHandler(int signum)
{
  if(_sigHandlerOnce)
  {
    qWarning("Sig handler called again [%s], exiting without cleaning up.\n", qPrintable(signalToString(signum)));
    ::_exit(NtgDaemon::FAILED);
  }

  //if signal is one of these, we catch it, log, and try to exit properly
  if(signum == SIGSYS || signum == SIGQUIT || signum == SIGILL ||
     signum == SIGABRT || signum == SIGBUS || signum == SIGFPE ||
     signum == SIGSEGV || signum == SIGPIPE || signum == SIGTSTP ||
     signum == SIGTTIN || signum == SIGTTOU || signum == SIGXFSZ )
  {
    _sigHandlerOnce = true;
    qWarning("We received fatal signal [%s], stopping.\n",  qPrintable(signalToString(signum)));
    if(qApp)
      qApp->exit(NtgDaemon::FAILED);
    else
    {
      qobject_cast<NtgDaemon*>(qApp)->_cleanup();
      ::exit(NtgDaemon::FAILED);
    }
  }
  else if(signum == SIGINT || signum == SIGTERM)
  {
    _sigHandlerOnce = true;
    qWarning("We received signal [%s], stopping.\n", qPrintable(signalToString(signum)));
    if(qApp)
      qApp->exit(NtgDaemon::STOPPED);
    else
    {
      qobject_cast<NtgDaemon*>(qApp)->_cleanup();
      ::exit(NtgDaemon::STOPPED);
    }
  }
  else
  {
    qWarning("Ignoring signal |%s] .\n",  qPrintable(signalToString(signum)));
  }
}


void NtgDaemon::_qtMessageHandler(QtMsgType type, const char * msg)
{
  QString dateStr = QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss.zzz");
  QString levelStr;
  switch(type)
  {
    case QtDebugMsg:    levelStr = "   Debug"; break;
    case QtWarningMsg:  levelStr = " Warning"; break;
    case QtCriticalMsg: levelStr = "Critical"; break;
    case QtFatalMsg:    levelStr = "   Fatal"; break;
    default:            levelStr = "Unknown"; break;
  }
  QString log = QString("%1 [%2] %3\n").arg(dateStr).arg(levelStr).arg(msg);

  if(_logToConsole)
  {
    ::fprintf(stderr, "%s", qPrintable(log));
    flush();
  }
  if(_logfile)
  {
    _logfile->write(log.toLocal8Bit());
    _logfile->flush();
  }
  if( type == QtFatalMsg )
    ::abort();
}


void NtgDaemon::_cleanup()
{
  delete _socketNotifier;
  close(_sighupFd[0]);
  close(_sighupFd[1]);

  if( ! _pidfilePath.isEmpty() && _pidfile && _pidfile->exists())
  {
    if( ! _pidfile->remove())
      qCritical("Can't remove pidfile %s", qPrintable(_pidfilePath));
  }
  delete _pidfile;
  if( _logfile )
    _logfile->close();
  delete _logfile;

  qInstallMsgHandler(NULL);
}
