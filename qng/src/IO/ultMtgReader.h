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

#ifndef ULTMTGREADER_H
#define ULTMTGREADER_H

#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <gzstream.h>

#include "javaUnserializer.h"
#include "mtgentity.h"

#include <QtXml>

using namespace std;

class UltMtgReader
{
	public:
        UltMtgReader(string, bool display=false);
		~UltMtgReader();
		void copyToTmp(ifstream& in, const char* tmpfile);
        void unGZip(string,string);

		bool checkMtgMagic();
		
        bool readMtgDataHeader();

        bool proceed(QList<MtgEntity>& entityList, QList<QPair<int,int> >& edgeList);
        bool readData(QList<MtgEntity>& entityList, QList<QPair<int,int> >& edgeList);
        bool readEntity(MtgEntity&);
        bool readProperty(MtgProperty&);
        bool readGenerator(MtgGenerator&);
        bool readLink(QPair<int,int>&);

		

	private:
        string _in;

		JavaUnserializer* _unserializer;

		bool _display;
		bool _obfuscationByte;


};

#endif

