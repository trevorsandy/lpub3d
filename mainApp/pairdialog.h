/****************************************************************************
**
** Copyright (C) 2007-2009 Kevin Clague. All rights reserved.
** Copyright (C) 2015 - 2023 Trevor SANDY. All rights reserved.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://www.trolltech.com/products/qt/opensource.html
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

/****************************************************************************
 *
 * This file implements a few dialogs that contain textually editable
 * versions of number that describe configuration of LDraw files.
 *
 * Please see lpub.h for an overall description of how the files in LPub
 * make up the LPub program.
 *
 ***************************************************************************/

#ifndef PAIRDIALOG
#define PAIRDIALOG

#include <QtGlobal>
#include <QDialog>
#include <QWidget>
#include <QString>
#include <QLineEdit>
#include <QLabel>
#include <QGridLayout>
#include <QDialogButtonBox>
#include <QRadioButton>
#include "meta.h"

class MetaGui;
class FloatsGui;
class FloatPairDialog : public QDialog
{

  Q_OBJECT

public:
  FloatPairDialog(
    QString       title,
    QString       heading1,
    QString       heading2,
    FloatPairMeta *leaf,
    QWidget *parent);

  virtual ~FloatPairDialog();
  static bool getFloatPair(
    QString  title,
    QString  heading1,
    QString  heading2,
    FloatPairMeta *leaf,
    float    values[],
    QWidget *parent);

  FloatPairMeta *meta;

public slots:
  void accept() override;
  void cancel();

private:

  FloatsGui *floats;

};

class DoubleSpinDialog : public QDialog

{

  Q_OBJECT

public:
  DoubleSpinDialog(
    QString       title,
    QString       heading,
    FloatMeta    *floatMeta,
    float         step,
    DoubleSpinEnc spinGui,
    QWidget       *parent);

  virtual ~DoubleSpinDialog()
  {
  }

  static bool getFloat(
    QString        title,
    QString        heading,
     FloatMeta     *floatMeta,
    float         &value,
    float          step,
    DoubleSpinEnc  spinGui,
    QWidget       *parent);

  FloatMeta *meta;

private:
  MetaGui   *spin;

public slots:
  void accept() override;
  void cancel();
};

class CameraFOVDialog : public QDialog

{

  Q_OBJECT

public:
  CameraFOVDialog(
    QString    title,
    QString    heading,
    FloatMeta *fovMeta,
    FloatMeta *zNearMeta,
    FloatMeta *zFarMeta,
    QWidget   *parent);

  virtual ~CameraFOVDialog()
  {
  }

  static bool getCameraFOV(
    QString    title,
    QString    heading,
    FloatMeta *fovMeta,
    FloatMeta *zNearMeta,
    FloatMeta *zFarMeta,
    float     &fovData,
    float     &zNearData,
    float     &zFarData,
    QWidget   *parent);

  FloatMeta *fovmeta;
  FloatMeta *znearmeta;
  FloatMeta *zfarmeta;

private:
  MetaGui   *cameraFOV;
  MetaGui   *cameraZNear;
  MetaGui   *cameraZFar;

public slots:
  void accept() override;
  void cancel();
};


class CameraAnglesDialog : public QDialog
{

  Q_OBJECT

public:
  CameraAnglesDialog(
    QString          title,
    CameraAnglesData &goods,
    QWidget          *parent);

  virtual ~CameraAnglesDialog();
  static bool getCameraAngles(
    QString           _name,
    CameraAnglesData &goods,
    QWidget          *parent);

  CameraAnglesMeta meta;

public slots:
  void accept() override;
  void cancel();

private:

  MetaGui *cameraAngles;
};

class FloatDialog : public QDialog
{
  Q_OBJECT
public:
  FloatDialog(
              QString title, 
              QString label0,
              FloatMeta *leaf);
 ~FloatDialog();

  static bool getFloat(
                QString type, 
                QString label0, 
                FloatMeta *leaf,
                float  &try0);
private:
  QLabel    *label0;
  QLineEdit *float0;
  QGridLayout *grid;
  QDialogButtonBox *buttonBox;
};

class LocalDialog : public QDialog
{

  Q_OBJECT

public:
  LocalDialog(
              QString  title, 
              QString  question,
              QWidget *parent);
 ~LocalDialog();
  static bool getLocal(
                QString  title, 
                QString  question,
                QWidget *parent);
private:
};

class OptionDialog : public QDialog
{

  Q_OBJECT

public:
  OptionDialog(
              QString  titles,
              QString  options,
              QWidget *parent);
 ~OptionDialog();
  static int getOption(
                QString  titles,
                QString  options,
                QWidget *parent);
private:
    QList<QRadioButton*> buttonList;
};
#endif
