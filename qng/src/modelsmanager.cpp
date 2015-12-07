/******************************************************************************
**
** Copyright (C) 2009-2010 DIATEAM. All rights reserved.
**
** This file is part of the Netglub GUI program.
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

#include "modelsmanager.h"
#include <unistd.h>

ModelsManager * ModelsManager::_modelsManager = NULL;
bool  ModelsManager::isInit = false;

ModelsManager::ModelsManager(QObject *parent) :
    QObject(parent)
{
    isInit = true;
    updateModels();

    /* temporaire - sur le serveur Ã  termes ? */
    MtgEntityAssociator associator;

    //main entities

    //EmailAddress
    associator.mtgType = "EmailAddress";
    associator.ntgType = "email-address";
    //associator.mtgToNtgParam.insert("","value");
    //associator.mtgToNtgParam.insert("","verified");
    _mtgConverter.insert("EmailAddress",associator);

    //Netblock
    associator.mtgType = "Netblock";
    associator.ntgType = "ip-block";
    //associator.mtgToNtgParam.insert("","value");
    //associator.mtgToNtgParam.insert("","netmask");
    //associator.mtgToNtgParam.insert("","cidr");
    associator.mtgToNtgParam.insert("startIP","start-ip");associator.ntgToMtgParam.insert("start-ip","startIP");
    associator.mtgToNtgParam.insert("endIP","end-ip");associator.ntgToMtgParam.insert("end-ip","end-ip");
    _mtgConverter.insert("Netblock",associator);

    //IPAddress
    associator.mtgType = "IPAddress";
    associator.ntgType = "ip-address";
    //associator.mtgToNtgParam.insert("","value");
    _mtgConverter.insert("IPAddress",associator);

    //Website
    associator.mtgType = "Website";
    associator.ntgType = "website";
    //associator.mtgToNtgParam.insert("","value");
    //associator.mtgToNtgParam.insert("","protocol");
    associator.mtgToNtgParam.insert("URLS","server");associator.ntgToMtgParam.insert("server","URLS");
    _mtgConverter.insert("Website",associator);

    //Location
    associator.mtgType = "Location";
    associator.ntgType = "location";
    associator.mtgToNtgParam.insert("country","value");associator.ntgToMtgParam.insert("value","country");
    associator.mtgToNtgParam.insert("city","city");associator.ntgToMtgParam.insert("city","city");
    //associator.mtgToNtgParam.insert("","latitude");
    //associator.mtgToNtgParam.insert("","longitude");
    associator.mtgToNtgParam.insert("area","other");associator.ntgToMtgParam.insert("other","area");
    _mtgConverter.insert("Location",associator);

    //Person
    associator.mtgType = "Person";
    associator.ntgType = "person";
    associator.mtgToNtgParam.insert("lastname","name");associator.ntgToMtgParam.insert("name","lastname");
    associator.mtgToNtgParam.insert("firstname","surname");associator.ntgToMtgParam.insert("surname","firstname");
    associator.mtgToNtgParam.insert("additional","nicknames");associator.ntgToMtgParam.insert("nicknames","additional");
    associator.mtgToNtgParam.insert("countrysc","search-word");associator.ntgToMtgParam.insert("search-word","countrysc");
    _mtgConverter.insert("Person",associator);

    //PhoneNumber
    associator.mtgType = "PhoneNumber";
    associator.ntgType = "phone-number";
    associator.mtgToNtgParam.insert("countrycode","country");associator.ntgToMtgParam.insert("country","countrycode");
    _mtgConverter.insert("PhoneNumber",associator);

    //DNSName
    associator.mtgType = "DNSName";
    associator.ntgType = "dns-name";
    _mtgConverter.insert("DNSName",associator);

    //Phrase
    associator.mtgType = "Phrase";
    associator.ntgType = "phrase";
    associator.mtgToNtgParam.insert("xdfg","search-field");associator.ntgToMtgParam.insert("search-field","xdfg");
    _mtgConverter.insert("Phrase",associator);

    //Port
    associator.mtgType = "Port";
    associator.ntgType = "port";
    associator.mtgToNtgParam.insert("xdfg","host");associator.ntgToMtgParam.insert("host","xdfg");
    associator.mtgToNtgParam.insert("xdfg","filter");associator.ntgToMtgParam.insert("filter","xdfg");
    associator.mtgToNtgParam.insert("xdfg","protocol");associator.ntgToMtgParam.insert("protocol","xdfg");
    _mtgConverter.insert("Port",associator);

    //ASNumber
    associator.mtgType = "ASNumber";
    associator.ntgType = "autonomous-system";
    _mtgConverter.insert("ASNumber",associator);

    //unknow -->

    //Webtitle

    //Webdir

    //Banner

    //Service

    //Vuln


    //derived entities

    //Domain
    associator.mtgType = "Domain";
    associator.ntgType = "domain-name";
    //associator.mtgToNtgParam.insert("","value");
    _mtgConverter.insert("Domain",associator);

    //AffiliationSpock

    //NSrecord
    associator.mtgType = "NSrecord";
    associator.ntgType = "ns-record";
    _mtgConverter.insert("NSrecord",associator);




    isInit = false;

}

ModelsManager::~ModelsManager()
{
//    qDebug() << "ModelsManager::~ModelsManager";
}

ModelsManager * ModelsManager::getOrCreate()
{
    while (true)
    {
        if (!isInit)
            break;
        usleep(2000);
    }
    if (_modelsManager == NULL)
    {
        _modelsManager = new ModelsManager();
    }

    return _modelsManager;
}


void ModelsManager::updateModels()
{
    _entityModelHash = CallerManager::get()->getAllEntities();

    QSettings settings;
    QFileInfo settingsPath = QFileInfo(settings.fileName());
    QDir settingsDir = settingsPath.absoluteDir();


    foreach (NtgEntityModel entityModel, _entityModelHash)
    {
        settingsDir.mkpath("data/entities/"+entityModel.name);
        QFile pngFile(settingsDir.absolutePath()+"/data/entities/"+entityModel.name+"/image.png");
        if (pngFile.open(QIODevice::WriteOnly))
        {
            pngFile.write(entityModel.imagePNG);
            pngFile.close();
        }

        QFile svgFile(settingsDir.absolutePath()+"/data/entities/"+entityModel.name+"/image.svg");
        if (svgFile.open(QIODevice::WriteOnly))
        {
            svgFile.write(entityModel.imageSVG);
            svgFile.close();
        }

/*
        NtgImageHandler imgPng(entityModel.imagePNG,true);
        imgPng.saveImage(settingsDir.absolutePath()+"/data/entities/"+entityModel.name+"/image.png");
        NtgImageHandler imgSvg(entityModel.imageSVG,true);
        imgSvg.saveImage(settingsDir.absolutePath()+"/data/entities/"+entityModel.name+"/image.svg");
*/

    }

    _availableEntities = CallerManager::get()->getAvailableEntities();
    _masterAvailableTransformsHash =  CallerManager::get()->getMasterAvailableTransforms();
    //TODO : Optimize LocalTransforms
    //Function disabled
    // _localAvailableTransformsHash =  CallerManager::get()->getLocalAvailableTransforms();

}


QHash<QString,NtgEntityModel> ModelsManager::getEntityModelHash()
{
  return _entityModelHash;
}

QHash<QString,NtgEntityModel> ModelsManager::getAvailableEntities()
{
  QHash<QString,NtgEntityModel> availableEntities;
  foreach (NtgEntityModel entity, _entityModelHash)
  {
    if (_availableEntities.contains(entity.name))
    {
      availableEntities.insert(entity.name,entity);
    }
  }
  return availableEntities;
}


NtgEntityModel ModelsManager::getEntityModel(QString nameId)
{
    if(_entityModelHash.contains(nameId))
        return _entityModelHash.value(nameId);
    else
    {
        qWarning()<< "unknow Entity model : " + nameId;
        return _entityModelHash.value("unknown");
    }
}


QMultiMap<QString,NtgTransformModel> ModelsManager::getAvailableTransformsbyCategories(QString input)
{
    QMultiMap<QString,NtgTransformModel> availableTransformMap;

    foreach(NtgTransformModel transform, _masterAvailableTransformsHash)
    {
        if (transform.inputTypes.contains(input))
            availableTransformMap.insert(transform.category,transform);


    }
//    foreach(NtgTransformModel transform, _localAvailableTransformsHash)
//    {
//        if (transform.inputTypes.contains(input))
//            availableTransformMap.insert(transform.category,transform);
//
//    }

    return availableTransformMap;
}
QHash<QString,NtgTransformModel> ModelsManager::getAvailableTransforms()
{
    QHash<QString,NtgTransformModel> availableTransformMap;
    availableTransformMap.unite(_masterAvailableTransformsHash);
    //availableTransformMap.unite(_localAvailableTransformsHash);

    return availableTransformMap;
}


QList<QString> ModelsManager::getCategoryModel(QString category, QString input)
{
    QList<QString> availableTransformList;

    foreach(NtgTransformModel transform, _masterAvailableTransformsHash)
    {
        if (transform.category == category && transform.inputTypes.contains(input))
            availableTransformList.append(transform.name);


    }
//    foreach(NtgTransformModel transform, _localAvailableTransformsHash)
//    {
//        if (transform.category == category && transform.inputTypes.contains(input))
//            availableTransformList.append(transform.name);
//    }

    return availableTransformList;
}


NtgEntity ModelsManager::convertMtgEntityToNtgEntity(MtgEntity mtgEntity)
{
    NtgEntity ntgEntity;
    ntgEntity.type = "unknow_";

    QHashIterator<QString,MtgEntityAssociator> itrAssociator(_mtgConverter);
    while(itrAssociator.hasNext())
    {
        itrAssociator.next();
        if(itrAssociator.key()==mtgEntity._type && itrAssociator.value().ntgType!="unknow")    //if mtgType match an associator
        {
            MtgEntityAssociator mtgEntityAssociator = itrAssociator.value();
            ntgEntity.type = mtgEntityAssociator.ntgType;
            ntgEntity.values.insert("value",mtgEntity._value);

            QHashIterator<QString,MtgProperty> itrProperties(mtgEntity._propertyHash); //for each mtg property
            while(itrProperties.hasNext())
            {
                itrProperties.next();
                if(mtgEntityAssociator.mtgToNtgParam.contains(itrProperties.key())) //if the property is convertible
                {
                    QString name = mtgEntityAssociator.mtgToNtgParam.value(itrProperties.key());
                    QString value = itrProperties.value().value;
                    ntgEntity.values.insert(name,value);
                }
                else
                    qDebug() << "mtgEntity " << mtgEntity._type << " has an unknow param : " << itrProperties.key();
            }
            return ntgEntity;
        }
    }

    qDebug() << "unknow mtg entity type : " << mtgEntity._type;

    return ntgEntity;
}

MtgEntity ModelsManager::convertNtgEntityToMtgEntity(NtgEntity ntgEntity)
{
    MtgEntity mtgEntity;

    QHashIterator<QString,MtgEntityAssociator> itrAssociator(_mtgConverter);
    while(itrAssociator.hasNext())
    {
        itrAssociator.next();
        if(itrAssociator.value().ntgType==ntgEntity.type)
        {
            MtgEntityAssociator mtgEntityAssociator = itrAssociator.value();
            mtgEntity._type = mtgEntityAssociator.mtgType;
            mtgEntity._value = ntgEntity.values.value("value");
            mtgEntity._weight = 100;
            mtgEntity._additionalSearchTerm = "ast";
            mtgEntity._posx = 100.0;
            mtgEntity._posy = 100.0;

            QHashIterator<QString,QString> itrParams(ntgEntity.values);
            while(itrParams.hasNext())
            {
                itrParams.next();
                if(mtgEntityAssociator.ntgToMtgParam.contains(itrParams.key()))
                {
                    QString name = mtgEntityAssociator.ntgToMtgParam.value(itrParams.key());
                    QString value = itrParams.value();
                    MtgProperty mtgProperty;
                    mtgProperty.name = name;
                    mtgProperty.value = value;
                    mtgProperty.type = "string";
                    //etc...
                    mtgEntity._propertyHash.insert(name,mtgProperty);
                }
            }
            return mtgEntity;
        }
    }

    qDebug() << "not associated ntg entity type : " << ntgEntity.type;

    return mtgEntity;
}


QString ModelsManager::getTransformLongName(QString transformId)
{
//   if (_localAvailableTransformsHash.contains(transformId))
//       return _localAvailableTransformsHash.value(transformId).longName;
//  else
      return _masterAvailableTransformsHash.value(transformId).longName;
}

NtgTransformModel ModelsManager::getTransform(QString transformId)
{
//    if (_localAvailableTransformsHash.contains(transformId))
//        return _localAvailableTransformsHash.value(transformId);
//    else
       return _masterAvailableTransformsHash.value(transformId);
}
