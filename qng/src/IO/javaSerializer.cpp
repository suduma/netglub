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

#include "javaSerializer.h"

JavaSerializer::JavaSerializer(QDataStream& out) : _stream(out)
{
	_cptByte = 0;
}

void JavaSerializer::writeJavaSerilizationV05()
{
    unsigned int tmp = 0xaced0005;
    _stream << tmp;
}

void JavaSerializer::writeByte(char byte)
{
	_arrayByte.append(byte);
}

void JavaSerializer::writeShort(unsigned short value)
{
	_arrayByte.append((value>>8)&0xff);
	_arrayByte.append((value)&0xff);
}

void JavaSerializer::writeInt(unsigned int value)
{
	_arrayByte.append((value>>24)&0x0ff);
	_arrayByte.append((value>>16)&0x0ff);
	_arrayByte.append((value>>8)&0x0ff);
	_arrayByte.append((value)&0x0ff);
}

void JavaSerializer::writeDouble(double d)
{
    quint64 tmp = *(quint64*)(&d);
	writeInt((tmp>>32)&0xffffffff);
	writeInt(tmp&0xffffffff);
}

void JavaSerializer::flushArrayByte()
{
	if(_arrayByte.size() == 0)
		return;
	else if(_arrayByte.size() <= 0xff)
    {
        _stream << (unsigned char) INTEGER;
        _stream << (unsigned char) _arrayByte.size();
        _stream.writeRawData(_arrayByte.data(),_arrayByte.size());
        _arrayByte.clear();
	}
	else if(_arrayByte.size() <= 0x7fffffff)
    {
        _stream << (unsigned char) BLOCKDATALONG;
        _stream << (quint32) _arrayByte.size();
        _stream.writeRawData(_arrayByte.data(),_arrayByte.size());
        _arrayByte.clear();
	}
}

void JavaSerializer::writeString(string str)
{
	flushArrayByte();

	if(str.size() <= 0xffff)
    {
        _stream << (unsigned char) STRING;
        _stream << (quint16) str.size();
        _stream.writeRawData(str.data(),str.size());
    }
	else if(str.size() <= 0xffffffff)
    {
        _stream << (unsigned char) LONGSTRING;
        _stream << (quint32) 0;
        _stream << (quint32) str.size();
        _stream.writeRawData(str.data(),str.size());
    }

}




