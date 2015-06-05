#ifndef _ABOUTDIALOG_H_
#define _ABOUTDIALOG_H_

#include <QDialog>
#include <windows.h>


namespace Ui {
class AboutDialog;
}

typedef enum { Win_64, Win_32, OsError, OSother } OsType;
//enum OsType {Win_64, Win_32, OsError, OsOther};
typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);

class AboutDialog : public QDialog
{
	Q_OBJECT

public:
	explicit AboutDialog(QWidget *parent, void *data);
    QString osName();
	~AboutDialog();

#ifdef Q_WS_WIN
    //int checkOS();
    OsType checkOS();
#endif

private:
	Ui::AboutDialog *ui;

};


#endif // _ABOUTDIALOG_H_
