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

#include "javaUnserializer.h"
#include <QDebug>

JavaUnserializer::JavaUnserializer(QDataStream & stream) : _stream(stream)
{
    _cptByteLeft = 0;
}

bool JavaUnserializer::readByte(unsigned char& ret)
{
    if(_cptByteLeft < 1)
		if(!checkIntTypeAndInitCpt())
			return false;

    _stream >> ret;
	_cptByteLeft--;
	return true;
}

bool JavaUnserializer::readShort(unsigned short& ret)
{
	ret = 0;
	for(int i(1); i>=0; --i)
	{
		unsigned char byte;
		if(!readByte(byte))
			return false;
		ret |= byte<< (i*8);
	}
	return true;
}

bool JavaUnserializer::readInt(unsigned int& ret)
{
	ret = 0;
	for(int i(3); i>=0; --i)
	{
		unsigned char byte;
		if(!readByte(byte))
			return false;
		ret |= byte<< (i*8);
	}
	return true;
}

bool JavaUnserializer::readDouble(double& d)
{
	unsigned int tab[]={0,0};
	readInt(tab[1]);
	readInt(tab[0]);	
	d = *(double*) tab;
	return true;
}

bool JavaUnserializer::readString(string& str)
{	
    unsigned char code;
    _stream >> code;
	if(code == STRING || code == LONGSTRING)
	{
		unsigned int size;
		if(code == STRING)
		{
            unsigned short tmp;
            _stream >> tmp;
            size = tmp;
		}
		else
		{
            unsigned int tmp;
            _stream >> tmp; //0s
            _stream >> size;
		}	
		ostringstream oss;	
        unsigned char tmp;
		for(unsigned int i(0); i<size; ++i)
		{
            _stream >> tmp;
            oss << tmp;
/*
            if (tmp==0x0D) {
                _stream >> tmp;
                oss << tmp;
            }
*/
        }
		str = oss.str();
		_stringTab.push_back(str);
        return true;
    }
	else if(code == REFERENCE)
	{
        unsigned char tmp;
        _stream >> tmp; _stream >> tmp; //don't care
        unsigned short index;
        _stream >> index;
        str = _stringTab[(int)index];
        return true;
    }
    return false;
}

bool inline JavaUnserializer::checkIntTypeAndInitCpt()
{
    unsigned char c;
    _stream >> c;
    if(c == INTEGER)
	{
        unsigned char t;
        _stream >> t;
        _cptByteLeft = t;
		return true;
	}
    else if(c == BLOCKDATALONG)
    {
        _stream >> _cptByteLeft;
        return true;
    }
	return false;
}

unsigned char JavaUnserializer::watchByte()
{
    return _stream.device()->peek(1).at(0);
}

bool JavaUnserializer::checkJavaSerialisationV05()
{
    unsigned char a,b,c,d;
    _stream >> a >> b >> c >> d;
    return (a==0xac && b==0xed && c==0x00 && d==0x05);
}

unsigned int JavaUnserializer::getCptByteLeft()
{
	return _cptByteLeft;
}


