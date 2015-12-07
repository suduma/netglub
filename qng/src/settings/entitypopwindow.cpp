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

#include "entitypopwindow.h"

EntityPopWindow::EntityPopWindow(Node * node, QWidget *parent)
{
 Q_UNUSED(parent);
  _node = node;
  this->setWindowTitle("Properties of "+node->getEntityModel().longName);

  _entityLayout = new QVBoxLayout();

_entityLayout->addStretch(0);
  QHash<QString, NtgTransformParam> paramHash = node->getEntityModel().params;
  if (paramHash.contains("value"))
      _widgetList.append(new LineFieldWidget(paramHash.value("value"),this));
  else
      return;

  FieldWidget * w ;
  foreach(NtgTransformParam param,node->getEntityModel().params)
  {
        if(param.ioMode=="input")
        {
            if (param.format == "enum" || param.format == "bool")
            {
                w = new ListFieldWidget(param,this);
                w->setToolTip(param.description);
                if (param.isOptional)
                    _widgetOptinalList.append(w);
                else
                    _widgetList.append(w);
            }
            else if (param.name != "value")
            {
                w = new LineFieldWidget(param,this);
                w->setToolTip(param.description);
                if (param.isOptional)
                    _widgetOptinalList.append(w);
                else
                    _widgetList.append(w);
            }
        }
  }

  foreach(FieldWidget * f, _widgetList)
  {
    if (node->getEntity().values.contains(f->getName()))
      {
        f->setValue(node->getEntity().values.value(f->getName()));
      }
    _entityLayout->addWidget(f,3);
  }



  QGridLayout * buttonLayout = new QGridLayout(this);
  _buttonValidateEntity = new QPushButton("&validate entity",this);
  connect(_buttonValidateEntity,SIGNAL(clicked()),this,SLOT(updateNode()));

  _buttonCancelEntity = new QPushButton("&cancel",this);
  connect(_buttonCancelEntity,SIGNAL(clicked()),this,SLOT(reject()));

  if(!_widgetOptinalList.isEmpty())
  {
      QVBoxLayout * extensionLayout = new QVBoxLayout;
      extensionLayout->setMargin(0);

      foreach(FieldWidget * f, _widgetOptinalList)
      {
        if (node->getEntity().values.contains(f->getName()))
          {
            f->setValue(node->getEntity().values.value(f->getName()));
          }
        extensionLayout->addWidget(f,3);
      }

      _moreWidget = new QWidget(this);
      _moreWidget->setLayout(extensionLayout);



      _moreButton = new QPushButton(QIcon(":/images/add.png"),"",this);
      _moreButton->setCheckable(true);
      _moreButton->setAutoDefault(false);
      connect(_moreButton,SIGNAL(toggled(bool)),_moreWidget,SLOT(setVisible(bool)));
      connect(_moreButton,SIGNAL(toggled(bool)),this,SLOT(changeSigle(bool)));

      buttonLayout->addWidget(_moreButton,0,0,1,1,Qt::AlignLeft);

      _entityLayout->addWidget(_moreWidget);
      _moreWidget->hide();
  }

  buttonLayout->addWidget(_buttonValidateEntity,0,3,1,4,Qt::AlignRight);
  buttonLayout->addWidget(_buttonCancelEntity,0,2,1,1,Qt::AlignRight);

  QWidget * buttonWidget= new QWidget(this);
  buttonWidget->setLayout(buttonLayout);
  _entityLayout->addWidget(buttonWidget);

setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Preferred);
_entityLayout->setSizeConstraint(QLayout::SetMinimumSize);
  setLayout(_entityLayout);

}

EntityPopWindow::~EntityPopWindow()
{

}

void EntityPopWindow::updateNode()
{
  foreach(FieldWidget * f, _widgetList)
  {
    if (!f->getValue().isEmpty())
      _node->addToEntity(f->getName(),f->getValue());
  }
  foreach(FieldWidget * f, _widgetOptinalList)
  {
    if (!f->getValue().isEmpty())
      _node->addToEntity(f->getName(),f->getValue());
  }
  accept();
  //delete this;
}


void EntityPopWindow::changeSigle(bool toggled)
{
    if (toggled)
    {
        _moreButton->setIcon(QIcon(":/images/remove.png"));
        _entityLayout->setSizeConstraint(QLayout::SetMinimumSize);
    }
    else
    {
        _moreButton->setIcon(QIcon(":/images/add.png"));
        _entityLayout->setSizeConstraint(QLayout::SetMaximumSize);
        _entityLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);


    }

}

/****************************************************************************
**  class FieldWidget
****************************************************************************/

FieldWidget::FieldWidget(QWidget *parent) : QWidget(parent) {}

FieldWidget::~FieldWidget()
{

}

QString FieldWidget::getName() const
{
  return _nameId;
}

/****************************************************************************
**  class LineFieldWidget
****************************************************************************/

LineFieldWidget::LineFieldWidget(NtgTransformParam param, QWidget *parent) :
    FieldWidget(parent)
{
  _nameId = param.name;

  _layout = new QGridLayout(this);
  _lineEdit = new QLineEdit(this);
  _lineEdit->setText(param.defaultValue);
  setFormat(param);
  _label = new QLabel(param.longName,this);
  _label->setBuddy(_lineEdit);
  _layout->addWidget(_label,0,0,1,1,Qt::AlignLeft);
  _layout->addWidget(_lineEdit,0,1,1,2);
  _layout->setColumnStretch(0,0);
  _layout->setColumnStretch(1,1);
  this->setLayout(_layout);
//  if (param.ioMode.isEmpty() == "output")
//      _lineEdit->setEnabled(false);
}

LineFieldWidget::~LineFieldWidget()
{

}

QString LineFieldWidget::getValue() const
{
  return _lineEdit->text();
}

void LineFieldWidget::setValue(QString value) const
{
  _lineEdit->setText(value);
}

void LineFieldWidget::setFormat(NtgTransformParam param)
{
    if (param.format == "int")
    {
        for(int i=0; i<param.formatParam.count();i++)
        {
            QIntValidator * v = new QIntValidator(this);
            foreach(QString key, param.formatParam.at(i).keys())
            {
                if (key == "min")
                {
                    v->setBottom(param.formatParam.at(i).value(key).toInt());
                }
                if (key == "max")
                {
                    v->setTop(param.formatParam.at(i).value(key).toInt());
                }
                _lineEdit->setValidator(v);
            }
        }
    }
    else if (param.format == "string")
    {
        for(int i=0; i<param.formatParam.count();i++)
        {
            foreach(QString key, param.formatParam.at(i).keys())
            {
                if (key == "regex")
                {
                   _lineEdit->setValidator(new QRegExpValidator(QRegExp(param.formatParam.at(i).value(key)),this));
                }
//                if (key == "inputMask")
//                {
//                    _lineEdit->setInputMask(param.formatParam.at(i).value(key));
//                }
            }
        }
    }
}

/****************************************************************************
**  class ListFieldWidget
****************************************************************************/

ListFieldWidget::ListFieldWidget(NtgTransformParam param, QWidget *parent) :
    FieldWidget(parent)
{
  _nameId = param.name;

  QStringList enumList;
  for (int i =0; i<param.formatParam.count();i++)
  {
    enumList.append(param.formatParam.at(i).value("label"));
  }
  _layout = new QGridLayout(this);
  _comboBox = new QComboBox(this);
  _comboBox->addItems(enumList);
  _comboBox->setCurrentIndex(_comboBox->findText(param.defaultValue));
  _label = new QLabel(param.longName,this);
  _label->setBuddy(_comboBox);
  _layout->addWidget(_label,0,0,1,1,Qt::AlignLeft);
  _layout->addWidget(_comboBox,0,1);
  this->setLayout(_layout);
//  if (param.ioMode.isEmpty() == "output")
//      _comboBox->setEnabled(false);

}

ListFieldWidget::~ListFieldWidget()
{

}

QString ListFieldWidget::getValue() const
{
  return _comboBox->currentText();
}

void ListFieldWidget::setValue(QString value) const
{
  if (_comboBox->findText(value)> -1)
  {
    _comboBox->setCurrentIndex(_comboBox->findText(value));
  }
}

/****************************************************************************
**  class TransformPopWindow
****************************************************************************/

TransformPopWindow::TransformPopWindow(NtgTransformModel model, QWidget *parent)
    : QDialog(parent)
{
    doTransform= false;
  _model = model;

  this->setWindowTitle("Properties of "+model.longName +" transform");
  _transformLayout = new QVBoxLayout();

  _transformLayout->addStretch(0);
  QHash<QString, NtgTransformParam> paramHash = model.params;

  FieldWidget * w ;
  foreach(NtgTransformParam param,paramHash)
  {
        if (param.format == "enum" || param.format == "bool")
        {
            w = new ListFieldWidget(param,this);
            w->setValue(param.defaultValue);
            w->setToolTip(param.description);
            _widgetList.append(w);
        }
        else if (param.name != "value")
        {
            w = new LineFieldWidget(param,this);
            w->setValue(param.defaultValue);
            w->setToolTip(param.description);
            _widgetList.append(w);
        }
  }

  foreach(FieldWidget * f, _widgetList)
  {
    _transformLayout->addWidget(f,3);
  }

  _rememberCheckBox = new QCheckBox("&Remember settings",this);
  _rememberCheckBox->setChecked(true);
  _transformLayout->addWidget(_rememberCheckBox,1,Qt::AlignLeft);

   QHBoxLayout * tmpLayout = new QHBoxLayout(this);

   _buttonCancelTransform = new QPushButton("&Cancel",this);
   connect(_buttonCancelTransform,SIGNAL(clicked()),this,SLOT(cancelTransform()));
   tmpLayout->addWidget(_buttonCancelTransform,0,Qt::AlignLeft);

  _buttonValidateTransform = new QPushButton("&validate transform",this);
  connect(_buttonValidateTransform,SIGNAL(clicked()),this,SLOT(updateSettings()));
  tmpLayout->addWidget(_buttonValidateTransform,0,Qt::AlignCenter);

  _buttonDefaultTransform = new QPushButton("&Default",this);
  connect(_buttonDefaultTransform,SIGNAL(clicked()),this,SLOT(defaultSettings()));
  tmpLayout->addWidget(_buttonDefaultTransform,0,Qt::AlignRight);

  QWidget * tmpWidget = new QWidget(this);
  tmpWidget->setLayout(tmpLayout);
  _transformLayout->addWidget(tmpWidget);


  setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Preferred);
  _transformLayout->setSizeConstraint(QLayout::SetMinimumSize);
  setLayout(_transformLayout);
  _buttonValidateTransform->setFocus();

}

TransformPopWindow::~TransformPopWindow()
{

}

void TransformPopWindow::updateSettings()
{
    doTransform = true;
    foreach(FieldWidget * f, _widgetList)
    {
        if (!f->getValue().isEmpty())
          _parameters.insert(f->getName(),QVariant(f->getValue()));
    }
    ManageXmlFile * xmlFile  = new ManageXmlFile();

    if (_rememberCheckBox->isChecked())
    {
        QMap<QString,QString> parameterListString;
        foreach(QString key, _parameters.keys())
        {
            parameterListString.insert(key, _parameters.value(key).toString());
        }
        xmlFile->saveTransformToXmlFile(_model.name,parameterListString);
    }
    delete xmlFile;

    this->close();
}


void TransformPopWindow::defaultSettings()
{
    ManageXmlFile * xmlFile  = new ManageXmlFile();
    if (_rememberCheckBox->isChecked())
    {
        QMap<QString,QString> parameterListString;
        xmlFile->saveTransformToXmlFile(_model.name,parameterListString);
    }
    doTransform = true;
    delete xmlFile;
    this->close();
}

void TransformPopWindow::cancelTransform()
{
    this->close();
}

QVariantMap TransformPopWindow::getParams()
{
    return _parameters;
}

bool TransformPopWindow::hasToDoTransform()
{
    return doTransform;
}
