#include "lc_global.h"
#include "lc_qgroupdialog.h"
#include "ui_lc_qgroupdialog.h"
#include "group.h"

lcQGroupDialog::lcQGroupDialog(QWidget *parent, const QString& Name) :
	QDialog(parent),
	ui(new Ui::lcQGroupDialog)
{
	ui->setupUi(this);

	ui->name->setText(Name);
}

lcQGroupDialog::~lcQGroupDialog()
{
	delete ui;
}

void lcQGroupDialog::accept()
{
	QString Name = ui->name->text();

	if (Name.isEmpty())
	{
/*** LPub3D Mod - set Visual Editor label ***/
		QMessageBox::information(this, "Visual Editor", tr("Name cannot be empty."));
/*** LPub3D Mod end ***/
		return;
	}

	mName = Name;

	QDialog::accept();
}
