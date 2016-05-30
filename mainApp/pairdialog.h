
 
/****************************************************************************

**

** Copyright (C) 2007-2009 Kevin Clague. All rights reserved.

** Copyright (C) 2016 Trevor SANDY. All rights reserved.

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
#include "meta.h"

class FloatsGui;

class FloatPairDialog : public QDialog

{

  Q_OBJECT

public:

    

  FloatPairDialog(

    float    value[],

    QString  title,

    QString  heading1,

    QString  heading2,

    QWidget *parent);



  virtual ~FloatPairDialog();



  static bool getFloatPair(

    float    value[],

    QString  title,

    QString  heading1,

    QString  heading2,

    QWidget *parent);



  FloatPairMeta meta;



public slots:

  void accept();

  void cancel();

  

private:

  FloatsGui *floats;

};





class DoubleSpinGui;

class DoubleSpinDialog : public QDialog

{

  Q_OBJECT

private:

      

public:

    

  DoubleSpinDialog(

    float   &value,

    float    min,

    float    max,

    float    step,

    QString  title,

    QString  heading,

    QWidget *parent);



  virtual ~DoubleSpinDialog()

  {

  }



  static bool getFloat(

    float   &value,

    float    min,

    float    max,

    float    step,

    QString  title,

    QString  heading,

    QWidget *parent);



  FloatMeta meta;



private:

  DoubleSpinGui *spin;



public slots:

  void accept();

  void cancel();

};



#include <QDialog>

#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
#include <QtWidgets/QWidget>
#else
#include <QWidget>
#endif

#include <QString>

#include <QLineEdit>

#include <QLabel>

#include <QGridLayout>

#include <QDialogButtonBox>

#include "meta.h"



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

#endif

