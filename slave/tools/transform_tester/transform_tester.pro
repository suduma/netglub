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
# Project created by QtCreator 2010-02-19T11:30:06
# -------------------------------------------------
QT -= gui
QT += xml network
TARGET = transform_tester
CONFIG += console
CONFIG -= app_bundle
TEMPLATE = app

OBJECTS_DIR = build
MOC_DIR = build

INCLUDEPATH += ../../src/ ../../../common
DEPENDPATH += ../../src/ ../../../common

SOURCES += main.cpp ntgDaemon.cpp ntgRpcHandler.cpp ntgTransformManager.cpp \
           ntgTransform.cpp ntgRpcMessage.cpp ntgSafeProcess.cpp ntgTypes.cpp
HEADERS += ntgDaemon.h ntgRpcHandler.h ntgTransformManager.h ntgTransform.h \
           ntgSafeProcess.h

QMAKE_LFLAGS += -rdynamic
