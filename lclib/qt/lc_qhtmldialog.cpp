#include "lc_global.h"
#include "lc_qhtmldialog.h"
#include "ui_lc_qhtmldialog.h"
#include "project.h"

lcQHTMLDialog::lcQHTMLDialog(QWidget* Parent, lcHTMLExportOptions* Options)
	: QDialog(Parent), ui(new Ui::lcQHTMLDialog)
{
	ui->setupUi(this);

	ui->stepWidth->setValidator(new QIntValidator(0, 2048, ui->stepWidth));
	ui->stepHeight->setValidator(new QIntValidator(0, 2048, ui->stepHeight));

	mOptions = Options;

	ui->outputFolder->setText(QDir::toNativeSeparators(mOptions->PathName));

	if (mOptions->CurrentOnly)
		ui->currentModelOnly->setChecked(true);
	else if (mOptions->SubModels)
		ui->currentModelSubmodels->setChecked(true);
	else
		ui->allModels->setChecked(true);

	ui->transparentImages->setChecked(mOptions->TransparentImages);
	ui->singlePage->setChecked(mOptions->SinglePage);
	ui->oneStepPerPage->setChecked(!mOptions->SinglePage);
	ui->indexPage->setChecked(mOptions->SinglePage);
	ui->stepWidth->setText(QString::number(mOptions->StepImagesWidth));
	ui->stepHeight->setText(QString::number(mOptions->StepImagesHeight));
/*** LPub3D Mod - HTML Highlight New Parts ***/
	ui->highlightNewParts->setChecked(mOptions->HtmlHighlightNewParts);
/*** LPub3D Mod end ***/
	ui->partsAfterEachStep->setChecked(mOptions->PartsListStep);
	ui->partsAtTheEnd->setChecked(mOptions->PartsListEnd);
/*** LPub3D Mod - HTML Fade Steps ***/
	ui->FadeSteps->setChecked(mOptions->HtmlFadeSteps);
/*** LPub3D Mod end ***/
}

lcQHTMLDialog::~lcQHTMLDialog()
{
	delete ui;
}

void lcQHTMLDialog::accept()
{
	QString pathName = ui->outputFolder->text();

	if (pathName.isEmpty())
	{
/*** LPub3D Mod - set Visual Editor label ***/
		QMessageBox::information(this, "Visual Editor", tr("Output folder cannot be empty."));
		return;
/*** LPub3D Mod end ***/
	}

	mOptions->PathName = pathName;
	mOptions->SubModels = ui->currentModelSubmodels->isChecked();
	mOptions->CurrentOnly = ui->currentModelOnly->isChecked();
	mOptions->TransparentImages = ui->transparentImages->isChecked();
	mOptions->SinglePage = ui->singlePage->isChecked();
	mOptions->IndexPage = ui->indexPage->isChecked();
	mOptions->StepImagesWidth = ui->stepWidth->text().toInt();
	mOptions->StepImagesHeight = ui->stepHeight->text().toInt();
/*** LPub3D Mod - HTML Highlight New Parts ***/
	mOptions->HtmlHighlightNewParts = ui->highlightNewParts->isChecked();
/*** LPub3D Mod end ***/
	mOptions->PartsListStep = ui->partsAfterEachStep->isChecked();
	mOptions->PartsListEnd = ui->partsAtTheEnd->isChecked();
/*** LPub3D Mod - HTML Fade Steps ***/
	mOptions->HtmlFadeSteps = ui->FadeSteps->isChecked();
/*** LPub3D Mod end ***/

	QDialog::accept();
}

void lcQHTMLDialog::on_outputFolderBrowse_clicked()
{
	QString result = QFileDialog::getExistingDirectory(this, tr("Select Output Folder"), ui->outputFolder->text());

	if (!result.isEmpty())
		ui->outputFolder->setText(QDir::toNativeSeparators(result));
}
