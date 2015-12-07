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

#ifndef JAVAUNSERIALIZER_H
#define JAVAUNSERIALIZER_H

#include <stdlib.h>
#include <vector>
#include <sstream>
#include <QDataStream>

#define INTEGER 0x77
#define BLOCKDATALONG 0x7a
#define STRING 0x74
#define LONGSTRING 0x7c
#define REFERENCE 0x71

using namespace std;

enum ObjectType{Integer,String};


class JavaUnserializer
{
    public:
    JavaUnserializer(QDataStream & stream);

	unsigned char watchByte();

	bool readByte(unsigned char&);
	bool readShort(unsigned short&);
	bool readInt(unsigned int&);
	bool readDouble(double&);
	

	bool readString(string& str);
	bool checkJavaSerialisationV05();

    unsigned int getCptByteLeft();


	private:
	bool inline checkIntTypeAndInitCpt();

    QDataStream & _stream;

	unsigned int _cptByteLeft;
	vector<string> _stringTab;
	

};

#endif

