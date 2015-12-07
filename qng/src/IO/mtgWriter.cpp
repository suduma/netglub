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

#include "mtgWriter.h"


MtgWriter::MtgWriter(string mtgout) : _out(mtgout)
{
	_serializer = NULL;
}

MtgWriter::~MtgWriter()
{
	if(_serializer != NULL)
		delete _serializer;
}

void MtgWriter::GZip(string inf, string outf)
{
    ifstream in(inf.c_str(),std::ios::binary | std::ios::in);
    ogzstream test_out(outf.c_str());
	char c;
	while(in.get(c))
		test_out <<  c;	
	in.close();
    	test_out.close();
}

void MtgWriter::copyFromTmp(ofstream& out, const char * inf)
{
	ifstream tmp_in(inf);
	char c;
    	while(tmp_in.get(c))
		out << c;
	tmp_in.close();
}

void MtgWriter::writeMtgMagic()
{
	_serializer->writeJavaSerilizationV05();
	_serializer->writeByte(3);
	_serializer->flushArrayByte();
}

void MtgWriter::write(QList<MtgEntity>& entityList, QList<QPair<int,int> >& edgeList)
{
    //write data
    QTemporaryFile ungzfileTmp;
    if(!ungzfileTmp.open())
        return;

    QDataStream ungzstream(&ungzfileTmp);
    _serializer = new JavaSerializer(ungzstream);
    writeDataSection(entityList,edgeList);
    delete _serializer;

    ungzfileTmp.flush();

    //compress
    QTemporaryFile gzfileTmp;
    if(!gzfileTmp.open())
        return;

    GZip(ungzfileTmp.fileName().toStdString(),gzfileTmp.fileName().toStdString());

    gzfileTmp.flush();
    ungzfileTmp.close();

    //write header + compressed data
    QFile file(QString::fromStdString(_out));
    if(!file.open(QIODevice::WriteOnly))
        return;

    QDataStream fileStream(&file);
    _serializer = new JavaSerializer(fileStream);
    writeMtgMagic();

    QByteArray array = gzfileTmp.readAll();
    fileStream.writeRawData(array,array.size());

    gzfileTmp.close();
    file.close();

    qDebug("[+] done");
}

void MtgWriter::writeDataSection(QList<MtgEntity>& entityList, QList<QPair<int,int> >& edgeList)
{
    //Writing header
    _serializer->writeJavaSerilizationV05();
    _serializer->writeByte(2);
    _serializer->writeByte(0);
    _serializer->writeByte(0); //No transient Obfuscation Byte

    //Writing Entities
    unsigned int entityNumber = entityList.size();
    qDebug("[+] nbr entity : %d",entityNumber);
    _serializer->writeInt(entityNumber);
    for(int i(0); i<entityList.size(); ++i)
    {
        MtgEntity mtgEntity = entityList.at(i);
        writeEntity(mtgEntity);
    }

    //Writing Edges
    unsigned int edgeNumber = edgeList.size();
    qDebug("[+] nbr edge : %d",edgeNumber);
    _serializer->writeInt(edgeNumber);
    for(int i(0); i<edgeList.length(); ++i)
    {
        QPair<int,int> pair = edgeList.at(i);
        writeEdge(pair);
    }

    _serializer->flushArrayByte();
}

void MtgWriter::writeEntity(MtgEntity& entity)
{
    _serializer->writeByte(_majorVersion);
    _serializer->writeByte(_minorVersion);
    _serializer->writeString(entity._type.toStdString());
    _serializer->writeString(entity._value.toStdString());
    _serializer->writeInt(entity._weight);
    _serializer->writeString(entity._additionalSearchTerm.toStdString());


    _serializer->writeInt(entity._propertyHash.size());
    QHashIterator<QString,MtgProperty> itrProperty(entity._propertyHash);
    while(itrProperty.hasNext())
    {
        itrProperty.next();
        MtgProperty mtgProperty = itrProperty.value();
        writeProperty(mtgProperty);
    }

    _serializer->writeInt(0);
    /*
    _serializer->writeInt(entity._generatorList.size());
    for(int i(0); i<entity._generatorList.size(); ++i)
    {
        MtgGenerator mtgGenerator = entity._generatorList.at(i);
        writeGenerator(mtgGenerator);
    }
    */

    _serializer->writeByte(2);
    _serializer->writeByte(1);
    _serializer->writeByte(0);

    double dx,dy;
    dx = entity._posx;
    dy = entity._posy;
    _serializer->writeDouble(dx);
    _serializer->writeDouble(dy);
}

void MtgWriter::writeProperty(MtgProperty& mtgProperty)
{
    _serializer->writeString(mtgProperty.name.toStdString());
    if(mtgProperty.type == "string")
        _serializer->writeString(mtgProperty.value.toStdString());
    else if(mtgProperty.type == "int")
        _serializer->writeInt(mtgProperty.value.toInt());
    else if(mtgProperty.type == "short")
        _serializer->writeInt(mtgProperty.value.toShort());
    else if(mtgProperty.type == "byte")
        _serializer->writeInt((char)mtgProperty.value.toShort());
    _serializer->writeString(mtgProperty.displayName.toStdString());
    _serializer->writeString(mtgProperty.description.toStdString());
    _serializer->writeByte((int)mtgProperty.matchingRule);
}

void MtgWriter::writeGenerator(MtgGenerator& mtgGenerator)
{
    _serializer->writeString(mtgGenerator.name.toStdString());
    _serializer->writeString(mtgGenerator.value.toStdString());
}

void MtgWriter::writeEdge(QPair<int,int>& pair)
{
    _serializer->writeInt(pair.first);
    _serializer->writeInt(pair.second);
    _serializer->writeByte(1);
    _serializer->writeByte(0);
    _serializer->writeByte(1);
}


