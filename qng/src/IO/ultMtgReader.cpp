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

#include "ultMtgReader.h"
#include <QtCore>

#include <QDataStream>


UltMtgReader::UltMtgReader(string in, bool display) : _in(in), _display(display)
{	
	_unserializer = NULL;
}

UltMtgReader::~UltMtgReader()
{

}

bool UltMtgReader::proceed(QList<MtgEntity>& entityList, QList<QPair<int,int> >& edgeList)
{
    //qDebug("UltMtgReader::proceed()");
    QFile file(QString::fromStdString(_in));
    if (!file.open(QIODevice::ReadOnly))
        return false;
    QDataStream in(&file);
    _unserializer = new JavaUnserializer(in);
    if(checkMtgMagic())
        qDebug("[+] file is a .mtg");
    else
    {
        qCritical("[X] file is not a .mtg");
        qCritical("[X] fail");
        file.close();
        return false;
    }
    delete _unserializer;

//Uncompress
    QByteArray inArray = in.device()->readAll();
    file.close();

    QTemporaryFile gzfileTmp;
    //QFile gzfileTmp("gzfileTemp.tmp");
    //if (!gzfileTmp.open(QIODevice::ReadWrite))
    if (!gzfileTmp.open())
        return false;
    QDataStream gzstream(&gzfileTmp);
    gzstream.writeRawData(inArray.data(),inArray.size());

    gzfileTmp.flush();

    QTemporaryFile ungzfileTmp;
    //QFile ungzfileTmp("ungzfileTemp.tmp");
    //if (!ungzfileTmp.open(QIODevice::ReadWrite))
    if (!ungzfileTmp.open())
        return false;

    unGZip(gzfileTmp.fileName().toStdString(),ungzfileTmp.fileName().toStdString());

    gzfileTmp.close();    
//End Uncompress

    QDataStream stream(&ungzfileTmp);

    _unserializer = new JavaUnserializer(stream);
    if(!readData(entityList,edgeList))
    {
        qCritical("\n[X] fail");
        delete _unserializer;
        return false;
    }
    return true;
}

bool UltMtgReader::readData(QList<MtgEntity>& entityList, QList<QPair<int,int> >& edgeList)
{
    if(!_unserializer->checkJavaSerialisationV05())
        return false;

    if(!readMtgDataHeader())
        return false;

    unsigned int entityNumber;
    if(!_unserializer->readInt(entityNumber))
        return false;
    qDebug("[+] nbr entity : %d ",entityNumber);

    //Entities
    for(unsigned int i(0); i<entityNumber; ++i)
    {
        MtgEntity mtgEntity;
        mtgEntity._id = i;
        if(!readEntity(mtgEntity))
        {
            qCritical("[X]readEntity failed : %d",i);
            return false;
        }
        entityList.append(mtgEntity);
    }

    //Links
    unsigned int linksNumber;
    if(!_unserializer->readInt(linksNumber))
        return false;

    qDebug("[+] nbr edge : %d", linksNumber);
    for(unsigned int i(0); i<linksNumber; ++i)
    {
        QPair<int,int> edge;
        if(!readLink(edge))
        {
            qCritical("[X] readLink failed : %d",i);
            return false;
        }
        edgeList.append(edge);
    }

    return true;
}

void UltMtgReader::copyToTmp(ifstream& in, const char* tmpfile)
{
	ofstream out(tmpfile);
	char c;
    	while(in.get(c))
		out << c;
	out.close();
}

void UltMtgReader::unGZip(string inf, string outf)
{
    igzstream in(inf.c_str());
    ofstream test_out(outf.c_str(), std::ios::binary | std::ios::out);
	char c;
    while(in.get(c)) {
        test_out << c;
    }
    in.close();
    test_out.close();
}

bool UltMtgReader::readMtgDataHeader()
{
	unsigned char byte;
	_unserializer->readByte(byte);
	_unserializer->readByte(byte);
	_unserializer->readByte(byte);
	_obfuscationByte = (byte == 1) ? true : false;
	return true;
}

bool UltMtgReader::checkMtgMagic()
{	
    if(!_unserializer->checkJavaSerialisationV05())
        return false;
    unsigned char byte;
	if(!_unserializer->readByte(byte) || byte !=3)
		return false;

	return true;
}

bool UltMtgReader::readGenerator(MtgGenerator& generator)
{
    string str;
    _unserializer->readString(str);
    generator.name = QString::fromStdString(str);
    _unserializer->readString(str);
    generator.value = QString::fromStdString(str);
    return true;
}

bool UltMtgReader::readProperty(MtgProperty& property)
{
    //qDebug("UltMtgReader::readProperty() --- START ---");
    string propertyName;

    string propertyDisplayName;
    string propertyDescriptionName;

    if(!_unserializer->readString(propertyName))
        return false;
        
    property.name = QString::fromStdString(propertyName);
    //qDebug("propertyName = %s", qPrintable(property.name));

    unsigned char c = _unserializer->watchByte();
    if(c == INTEGER) //possible in theory.
    {
        unsigned int nbbytes = _unserializer->getCptByteLeft();
        QString value;
        switch(nbbytes)
        {
            case 1:
                unsigned char vbyte;
                _unserializer->readByte(vbyte);
                property.type = "byte";
                value = QString::number(vbyte);
                break;
            case 2:
                unsigned short vshort;
                _unserializer->readShort(vshort);
                property.type = "short";
                value = QString::number(vshort);
                break;
            case 4:
                unsigned int vint;
                _unserializer->readInt(vint);
                property.type = "int";
                value = QString::number(vint);
                break;
            default:
                ostringstream oss;
                unsigned char rbyte;
                for(unsigned int i(0);i<nbbytes;++i)
                {
                    _unserializer->readByte(rbyte);
                    oss << rbyte << "|";
                }
                string str = oss.str();
                 property.type = "raw";
                value = QString::fromStdString(str);
        }
        property.value = value;
    }
    else if(c == STRING || c == REFERENCE)
    {
        string str;
        if(!_unserializer->readString(str))
        {
            qCritical("[readProperty error string]");
            return false;
        }
        property.type = "string";
        property.value = QString::fromStdString(str);
    }
    else
    {
        qCritical("[readProperty error] property type error c=%d", c);
        return false;
    }

    if(!_unserializer->readString(propertyDisplayName)) {
        //qDebug("unable to read display name");
        return false;
    }
    property.displayName= QString::fromStdString(propertyDisplayName);

    if(!_unserializer->readString(propertyDescriptionName)) {
        //qDebug("unable to read description name");
        return false;
    }
    property.description = QString::fromStdString(propertyDescriptionName);

    unsigned char byte;
    if(!_unserializer->readByte(byte)) {
        //qDebug("unable to read matching rule byte");
        return false;
    }
    property.matchingRule = byte;

    return true;
}

bool UltMtgReader::readEntity(MtgEntity& entity)
{
    string entityType;
    string entityValue;
    unsigned int entityWeight;
    string entityAdditionalSearchTerm;
    unsigned int propertyCount;
    unsigned int generatorCount;

    unsigned char majorv;
    unsigned char minorv;

    if(!_unserializer->readByte(majorv))
        return false;
    if(!_unserializer->readByte(minorv))
        return false;
    if(majorv != 1 || minorv != 0)
        return false;


    if(!_unserializer->readString(entityType))
        return false;
    if(!_unserializer->readString(entityValue))
        return false;


    if(!_unserializer->readInt(entityWeight))
        return false;

    if(!_unserializer->readString(entityAdditionalSearchTerm))
        return false;

    entity._version = "1.0";
    entity._type = QString::fromStdString(entityType);
    entity._value = QString::fromStdString(entityValue);
    entity._weight = entityWeight;
    entity._additionalSearchTerm = QString::fromStdString(entityAdditionalSearchTerm);

    //Properties
    if(!_unserializer->readInt(propertyCount))
        return false;

    for(unsigned int i(0); i<propertyCount; ++i)
    {
        MtgProperty property;
        if(!readProperty(property))
        {
            qCritical("[x] readProperty failed");
            return false;
        }
        entity._propertyHash.insert(property.name,property);
    }

    //Generators
    if(!_unserializer->readInt(generatorCount))
        return false;

    for(unsigned int i(0); i<generatorCount; ++i)
    {
        MtgGenerator generator;
        if(!readGenerator(generator))
        {
            qCritical("[x] readGenerator failed");
            return false;
        }
        entity._generatorList.append(generator);
    }

    unsigned char byte;
    _unserializer->readByte(byte);
    if(byte != 2) // bourrage
    {
        qCritical("pronostic failed %x",byte);
        return false;
    }

    unsigned char majorversion;
    _unserializer->readByte(majorversion);
    unsigned char minorversion;
    _unserializer->readByte(minorversion);
    if(majorversion != 1 && minorversion != 0) //major & minor version ?
        return false;


    double x,y;
    _unserializer->readDouble(x);
    _unserializer->readDouble(y);
    entity._posx = x;
    entity._posy = y;

    if(_obfuscationByte)
    {
        if(!_unserializer->readByte(byte))
            return false;
    }

    return true;
}

bool UltMtgReader::readLink(QPair<int,int>& pair)
{
    unsigned int parentId;
    unsigned int childId;
    unsigned char c1,c2,c3;

    if(!_unserializer->readInt(parentId))
    {
        return false;
    }

    if(!_unserializer->readInt(childId))
    {
        return false;
    }

    pair.first = parentId;
    pair.second = childId;

    _unserializer->readByte(c1);//1
    _unserializer->readByte(c2);//0
    _unserializer->readByte(c3);//1

    if(c1 !=1 || c2!=0 || c3 !=1)
    {
        return false;
    }

    unsigned char byte;
    if(_obfuscationByte)
    {
        if(!_unserializer->readByte(byte))
        {
        return false;
        }
    }

    return true;
}
