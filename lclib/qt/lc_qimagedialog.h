#pragma once

#include <QDialog>

namespace Ui {
class lcQImageDialog;
}

class lcQImageDialog : public QDialog
{
	Q_OBJECT
	
public:
	lcQImageDialog(QWidget* Parent);
	~lcQImageDialog();

	QString mFileName;
	int mWidth;
	int mHeight;
	int mStart;
	int mEnd;

public slots:
	void accept();
	void on_fileNameBrowse_clicked();
/*** LPub3D Mod - lpub3d image file resize */
	void on_reset_clicked();
	void valueChanged(int value);
/*** LPub3D Mod end ***/

private:
	Ui::lcQImageDialog *ui;
};

