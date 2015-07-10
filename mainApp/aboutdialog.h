#ifndef _ABOUTDIALOG_H_
#define _ABOUTDIALOG_H_

#include <QDialog>
#include <windows.h>


namespace Ui {
class AboutDialog;
}

typedef enum { Win_64, Win_32, OsError, OSother } OsType;
typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);

class AboutDialog : public QDialog
{
	Q_OBJECT

private slots:
    void showReadmeDetails(bool);
    void showCreditDetails(bool);

public:
	explicit AboutDialog(QWidget *parent, void *data);
    QString osName();
    QPushButton *detailsButton;
    QPushButton *creditsButton;
    QString content;
	~AboutDialog();

#ifdef Q_WS_WIN
    OsType checkOS();
#endif

private:
	Ui::AboutDialog *ui;

};


#endif // _ABOUTDIALOG_H_
