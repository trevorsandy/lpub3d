/****************************************************************************
**
** Copyright (C) 2015 - 2025 Trevor SANDY. All rights reserved.
**
** This file may be used under the terms of the
** GNU General Public Liceense (GPL) version 3.0
** which accompanies this distribution, and is
** available at http://www.gnu.org/licenses/gpl.html
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

/****************************************************************************
 *
 * This file implements a dialog that allows the user to change the page
 * size and/or orientation.
 *
 * Please see lpub.h for an overall description of how the files in LPub3D
 * make up the LPub3D program.
 *
 ***************************************************************************/

#ifndef SIZEANDORIENTATIONDIALOG_H
#define SIZEANDORIENTATIONDIALOG_H

#include <QDialog>
#include "meta.h"

class SizeAndOrientationGui;
class SizeAndOrientationDialog : public QDialog
{
  Q_OBJECT
public:

  SizeAndOrientationDialog(
    PageSizeData    &sgoods,
    OrientationEnc  &ogoods,
    QString          _name,
    QWidget         *parent);

  virtual ~SizeAndOrientationDialog();

  static bool getSizeAndOrientation(
    PageSizeData   &sgoods,
    OrientationEnc &ogoods,
    QString         name,
    QWidget        *parent = 0);

    PageSizeMeta        metaS;
    PageOrientationMeta metaO;

public slots:
  void accept() override;
  void cancel();

private:
  SizeAndOrientationGui *sizeAndOrientation;
};


#endif // SIZEANDORIENTATIONDIALOG_H
