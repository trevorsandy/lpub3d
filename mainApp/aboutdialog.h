/****************************************************************************
**
** Copyright (C) 2015 - 2018 Trevor SANDY. All rights reserved.
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

#ifndef _ABOUTDIALOG_H_
#define _ABOUTDIALOG_H_

#include <QDialog>
#ifdef Q_OS_WIN
#include <windows.h>
#endif


namespace Ui {
class AboutDialog;
}

typedef enum { Win_64, Win_32, OsError, OsOther } OsType;
#ifdef Q_OS_WIN
typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);
#endif

class AboutDialog : public QDialog
{
	Q_OBJECT

private slots:
    void showReadmeDetails(bool);
    void showCreditDetails(bool);

public:
        explicit AboutDialog(QWidget *parent);
    QString osName();
    QPushButton *detailsButton;
    QPushButton *creditsButton;
	~AboutDialog();

#ifdef Q_OS_WIN
    OsType checkOS();
#endif

private:
	Ui::AboutDialog *ui;

};


#endif // _ABOUTDIALOG_H_
