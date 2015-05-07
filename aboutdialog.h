#ifndef _ABOUTDIALOG_H_
#define _ABOUTDIALOG_H_

#include <QDialog>

namespace Ui {
class AboutDialog;
}

class AboutDialog : public QDialog
{
	Q_OBJECT

public:
	explicit AboutDialog(QWidget *parent, void *data);
	~AboutDialog();

private:
	Ui::AboutDialog *ui;

};

#endif // _ABOUTDIALOG_H_
