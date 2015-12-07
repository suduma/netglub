# #####################################################################
# 
#  Copyright (C) 2009-2010 DIATEAM. All rights reserved.
# 
#  This file is part of the "NetGlub" program.
# 
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
# 
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
# 
#  You should have received a copy of the GNU General Public License
#  along with this program.  If not, see <http://www.gnu.org/licenses/>.
# 
# #####################################################################

# -------------------------------------------------
# Project created by QtCreator 2010-02-08T10:43:20
# -------------------------------------------------


QT += network xml sql

TARGET = master
CONFIG += console
CONFIG += debug
CONFIG -= app_bundle
TEMPLATE = app

DEFINES+=XMLRPC_WITH_SSL
#DEFINES+=TRANSPORT_WITH_SSL

OBJECTS_DIR = ../build
MOC_DIR = ../build
DESTDIR = ../

SOURCES += main.cpp \
    ntgSlaveManager.cpp \
    ntgSlaveProxy.cpp \
    ntgMaster.cpp \
    ntgTransformProxy.cpp \
    ntgXmlRpcServer.cpp \
    ntgSessionManager.cpp \
    ntgMethodProvider.cpp \
    ntgTransformManager.cpp
HEADERS += ntgSlaveManager.h \
    ntgSlaveProxy.h \
    ntgMaster.h \
    ntgTransformProxy.h \
    ntgXmlRpcServer.h \
    ntgSessionManager.h \
    ntgMethodProvider.h \
    ntgTransformManager.h


# LibMaia
DEPENDPATH += libmaia
INCLUDEPATH += libmaia
HEADERS += maiaFault.h maiaObject.h maiaXmlRpcServerConnection.h maiaXmlRpcServer.h
SOURCES += maiaFault.cpp maiaObject.cpp maiaXmlRpcServerConnection.cpp maiaXmlRpcServer.cpp

include(../../common/common.pri)
