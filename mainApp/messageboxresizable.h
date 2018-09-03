/****************************************************************************
**
** Copyright (C) 2018 Trevor SANDY. All rights reserved.
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

#ifndef MESSAGEBOXRESIZABLE_H
#define MESSAGEBOXRESIZABLE_H

// Resizable QMessageBox Hack
#include <QMessageBox>
#include <QTextEdit>
#include <QEvent>

class QMessageBoxResizable: public QMessageBox
{
Q_OBJECT
public:
   QMessageBoxResizable() {
     setMouseTracking(true);
     setSizeGripEnabled(true);
   }
private:
   virtual bool event(QEvent *e) {
     bool res = QMessageBox::event(e);
     switch (e->type()) {
     case QEvent::MouseMove:
     case QEvent::MouseButtonPress:
       setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
       if (QWidget *textEdit = findChild<QTextEdit *>()) {
         textEdit->setMaximumHeight(QWIDGETSIZE_MAX);
       }
     default:
       break;
     }
     return res;
   }
};

#endif // MESSAGEBOXRESIZABLE_H
