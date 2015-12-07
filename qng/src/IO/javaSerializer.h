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

#ifndef JAVASERIALIZER_H
#define JAVASERIALIZER_H

#include <iostream>
#include <fstream>
#include <QByteArray>
#include <QDataStream>

#define INTEGER 0x77
#define BLOCKDATALONG 0x7a
#define STRING 0x74
#define LONGSTRING 0x7c
#define REFERENCE 0x71

using namespace std;

class JavaSerializer
{
	public :
    JavaSerializer(QDataStream&);

	void writeByte(char);
	void writeShort(unsigned short);
	void writeInt(unsigned int);
	void writeDouble(double);
	void writeString(string);

	void writeJavaSerilizationV05();

	void flushArrayByte();

	

	private :
    QDataStream & _stream;

	unsigned int _cptByte;
	QByteArray _arrayByte;




	



};

#endif

