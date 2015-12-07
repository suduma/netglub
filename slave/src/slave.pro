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
# Project created by QtCreator 2010-01-21T16:18:41
# -------------------------------------------------
QT += network \
    xml
QT += gui
TARGET = slave
CONFIG += console \
    debug
CONFIG -= app_bundle
TEMPLATE = app

OBJECTS_DIR = ../build
MOC_DIR = ../build
DESTDIR = ../

SOURCES += main.cpp \
    ntgTransform.cpp \
    ntgTransformManager.cpp \
    ntgSlave.cpp \
    ntgSafeProcess.cpp
HEADERS += ntgTransform.h \
    ntgTransformManager.h \
    ntgSlave.h \
    ntgSafeProcess.h
include("../../common/common.pri")

# Export every symbols from the app so the plugins can access it all.
# Eventually, only export the desired symbols using -Wl,--dynamic-list <list>
QMAKE_LFLAGS += -rdynamic
