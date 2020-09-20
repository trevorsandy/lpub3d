/****************************************************************************
**
** Copyright (C) 2018 - 2020 Trevor SANDY. All rights reserved.
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
* These dialogs provide graphical user interfaces for the Native Renderer
* CameraDistanceFactor
*
* Please see lpub.h for an overall description of how the files in LPub
* make up the LPub program.
*
***************************************************************************/

#ifndef CAMERADISTFACTORDIALOG_H
#define CAMERADISTFACTORDIALOG_H


#include <QDialog>
#include "meta.h"

class CameraDistFactorGui;
class CameraDistFactorDialog : public QDialog
{
  Q_OBJECT
public:

  CameraDistFactorDialog(
    QString   title,
    QString   heading,
    int      &goods,
    QWidget  *parent);

  virtual ~CameraDistFactorDialog();

  static bool getCameraDistFactor(
    QString   title,
    QString   heading,
    int      &goods,
    QWidget  *parent);

    NativeCDMeta meta;

public slots:
  void accept();
  void cancel();

private:
  CameraDistFactorGui *cameraDistFactor;
};

#endif // CAMERADISTFACTORDIALOG_H
