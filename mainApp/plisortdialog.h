/****************************************************************************
**
** Copyright (C) 2016 Trevor SANDY. All rights reserved.
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
 * This class implements a simple dialog that lets the user specify sort
 * options for the parts list images.
 *
 * Please see lpub.h for an overall description of how the files in LPub
 * make up the LPub program.
 *
 ***************************************************************************/

#ifndef PLISORTDIALOG_H
#define PLISORTDIALOG_H

#include <QDialog>
#include "meta.h"

class PliSortGui;
class PliSortDialog : public QDialog
{
  Q_OBJECT
public:

  PliSortDialog(
    QString         &goods,
    QString          name,
    QWidget         *parent);

  virtual ~PliSortDialog();

  static bool getPliSortOption(
    QString   &goods,
    QString        name,
    QWidget       *parent);

    PliSortMeta    meta;

public slots:
  void accept();
  void cancel();

private:
  PliSortGui *pliSortBy;
};

#endif // PLISORTDIALOG_H
