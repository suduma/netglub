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

#ifndef ENTITYPOPWINDOW_H
#define ENTITYPOPWINDOW_H

#include "graph.h"
#include "ntgTypes.h"
#include "managetransformdialog.h"
#include <QWidget>
#include <QDialog>
#include <QGridLayout>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QVariantMap>
#include <QVariant>
#include <QGroupBox>
#include <QHBoxLayout>

class FieldWidget;
class TransformWidget;

class EntityPopWindow : public QDialog
{
Q_OBJECT
public:
    explicit EntityPopWindow(Node * node, QWidget *parent = 0);
    virtual ~EntityPopWindow();

signals:

public slots:
    void updateNode();
    void changeSigle(bool toggled);

protected:
    Node * _node;
    QVBoxLayout * _entityLayout;
    QPushButton * _buttonValidateEntity;
    QPushButton * _buttonCancelEntity;
    QList<FieldWidget *> _widgetList;
    QList<FieldWidget *> _widgetOptinalList;

    QPushButton * _moreButton;
    QWidget * _moreWidget;


};

/****************************************************************************
**  class TransformPopWindow
****************************************************************************/

class TransformPopWindow : public QDialog
{
Q_OBJECT
public:
    explicit TransformPopWindow(NtgTransformModel model, QWidget *parent = 0);
    virtual ~TransformPopWindow();

    QVariantMap getParams();

    bool hasToDoTransform();
signals:

public slots:
    void defaultSettings();
    void updateSettings();
    void cancelTransform();

protected:
    NtgTransformModel _model;
    QPushButton * _buttonDefaultTransform;
    QPushButton * _buttonValidateTransform;
    QPushButton * _buttonCancelTransform;
    QVBoxLayout * _transformLayout;
    QList<FieldWidget *> _widgetList;

    QCheckBox * _rememberCheckBox;
    QVariantMap _parameters;

    bool doTransform;



};



/****************************************************************************
**  class FieldWidget
****************************************************************************/

class FieldWidget : public QWidget
{
  Q_OBJECT
  public:
      FieldWidget(QWidget *parent = 0);
      virtual ~FieldWidget();
      virtual QString getValue() const = 0;
      virtual QString getName() const;
      virtual void setValue(QString value) const = 0;

  protected:
      QString _nameId;
      QLabel * _label;
      QGridLayout * _layout;


};

/****************************************************************************
**  class LineFieldWidget
****************************************************************************/

class LineFieldWidget : public FieldWidget
{
  Q_OBJECT
  public:
      LineFieldWidget(NtgTransformParam param, QWidget *parent = 0);
      virtual ~LineFieldWidget();
      virtual QString getValue() const ;
      virtual void setValue(QString value) const;
      void setFormat(NtgTransformParam param);

  protected:
    QLineEdit * _lineEdit;

};

/****************************************************************************
**  class ListFieldWidget
****************************************************************************/

class ListFieldWidget : public FieldWidget
{
  Q_OBJECT
  public:
      ListFieldWidget(NtgTransformParam param, QWidget *parent = 0);
      virtual ~ListFieldWidget();
      virtual QString getValue() const;
      virtual void setValue(QString value) const;

  protected:
      QComboBox * _comboBox;
};
#endif // ENTITYPOPWINDOW_H
