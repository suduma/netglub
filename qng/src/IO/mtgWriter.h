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

#ifndef MTGWRITER_H
#define MTGWRITER_H

#include <iostream>
#include <fstream>
#include <QtXml>
#include "javaSerializer.h"
#include "gzstream.h"
#include "mtgentity.h"


using namespace std;

class MtgWriter
{
	public :
    MtgWriter(string);
    ~MtgWriter();

    void write(QList<MtgEntity>&, QList<QPair<int,int> >&);
    void writeDataSection(QList<MtgEntity>&, QList<QPair<int,int> >&);
    void writeEntity(MtgEntity&);
    void writeProperty(MtgProperty&);
    void writeGenerator(MtgGenerator&);
    void writeEdge(QPair<int,int>&);

    void GZip(string, string);
	void writeMtgMagic();
	void copyFromTmp(ofstream&, const char *);
	
    private :
    string _out;
	JavaSerializer* _serializer;

	static const char _majorVersion = 1;
	static const char _minorVersion = 0;
};

#endif

