
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
* This file describes a dialogue that the users can use to change attributes
* associated with pointers.  Pages, Dividers, Assemblies and Callouts have pointers.
*
* Please see lpub.h for an overall description of how the files in LPub
* make up the LPub program.
*
***************************************************************************/

#ifndef POINTERATTRIBDIALOG_H
#define POINTERATTRIBDIALOG_H

#include <QDialog>
#include "meta.h"

class PointerAttribGui;
class PointerAttribDialog : public QDialog
{
 Q_OBJECT
public:

 PointerAttribDialog(PointerAttribData  &goods,
   QString name,
   bool    isCallout);

 virtual ~PointerAttribDialog()
 {
 }

 static bool getPointerAttrib(
   PointerAttribData  &goods,
   QString             name,
   bool                isCallout);

 PointerAttribMeta   meta;

public slots:
 void accept();
 void cancel();

private:
 PointerAttribGui *pointerAttrib;
};

#endif // POINTERATTRIBDIALOG_H
