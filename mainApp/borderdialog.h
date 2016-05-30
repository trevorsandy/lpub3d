 
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
 * This file describes a dialog that the users can use to change attributes
 * associated with borders.  Pages, Parts Lists and Callouts have borders.
 *
 * Please see lpub.h for an overall description of how the files in LPub
 * make up the LPub program.
 *
 ***************************************************************************/

#ifndef BORDERDIALOG_H
#define BORDERDIALOG_H

#include <QDialog>
#include "meta.h"

/*
 * Area
 * Square
 * Width  value
 * Height value
 * Cols   value
 */
class BorderGui;
class BorderDialog : public QDialog
{
  Q_OBJECT
public:
    
  BorderDialog(
    BorderData  &goods,
    QString      name,
    bool         rotateArrow = false);

  virtual ~BorderDialog()
  {
  }

  static bool getBorder(
    BorderData  &goods,
    QString      name,
    bool         rotateArrow = false);

  BorderMeta   meta;

public slots:
  void accept();
  void cancel();
  
private:
  BorderGui *border;
};
#endif
