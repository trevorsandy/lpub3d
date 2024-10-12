#include <QFileDialog>
#include <TCFoundation/mystring.h>
#include "LDViewPartList.h"
#include "LDVWidget.h"
#include <LDVPreferences.h>
#include <LDVWidgetDefaultKeys.h>

#include "commonmenus.h"

LDVPartList::LDVPartList(LDVWidget *modelWidget, LDVHtmlInventory *htmlInventory)
		: QDialog(qobject_cast<QWidget*>(modelWidget)),
		 LDVPartListPanel(),
		 m_modelViewer(modelWidget->getModelViewer() ? ((LDrawModelViewer*)modelWidget->getModelViewer()->retain()) : nullptr),
		 m_modelWidget(modelWidget),
		 m_htmlInventory(htmlInventory)
{
	setupUi(this);

	setWindowTitle(tr("LDView HTML Part List Export"));

	setWhatsThis(lpubWT(WT_DIALOG_LDVIEW_HTML_PART_LIST_EXPORT,windowTitle()));

	settingsBox->setWhatsThis(lpubWT(WT_CONTROL_LDVIEW_HTML_PART_LIST_EXPORT_DISPLAY_OPTIONS,settingsBox->title()));
	partReferenceBox->setWhatsThis(lpubWT(WT_CONTROL_LDVIEW_HTML_PART_LIST_EXPORT_PART_REFERENCE,partReferenceBox->title()));
	columnOrderBox->setWhatsThis(lpubWT(WT_CONTROL_LDVIEW_HTML_PART_LIST_EXPORT_COLUMN_ORDER,columnOrderBox->title()));

	connect( upButton, SIGNAL( clicked() ), this, SLOT( doUp() ) );
	connect( downButton, SIGNAL( clicked() ), this, SLOT( doDown() ) );
	connect( okButton, SIGNAL( clicked() ), this, SLOT( doOk() ) );
	connect( cancelButton, SIGNAL( clicked() ), this, SLOT( doCancel() ) );
	connect( showModelButton, SIGNAL( clicked() ), this, SLOT( doShowModel() ) );
	connect( fieldOrderView, SIGNAL( currentItemChanged(QListWidgetItem *, QListWidgetItem *) ), this, SLOT( doHighlighted(QListWidgetItem *, QListWidgetItem *) ) );
	connect( preferencesButton, SIGNAL( clicked() ), this, SLOT( doShowPreferences() ) );
	connect( snapshotButton, SIGNAL( clicked() ), this, SLOT( doBrowseSnapshotFile() ) );
	connect( snapshotEdit, SIGNAL( editingFinished() ), this, SLOT(doUpdateSnapshotEdit() ) );
	connect( rebrickableKeyEdit, SIGNAL( editingFinished() ), this, SLOT(doUpdateRebrickableKey() ) );
	connect( partLookupSiteCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(doEnableRebrickableKeyEdit(int)));

	if (m_modelWidget) {
		if (m_htmlInventory->getLookupSite() ==
			LDVHtmlInventory::LookUp::Bricklink)
			emit m_modelWidget->loadBLCodesSig();
	}

	QValidator *validator = new QIntValidator(10, 5000, this);
	snapshotWidthEdit->setValidator(validator);
	snapshotHeightEdit->setValidator(validator);
	snapshotEdit->setValidator(new QRegExpValidator(
		   QRegExp("^.*\\.png$",Qt::CaseInsensitive), this));
	rebrickableKeyEdit->setValidator(new QRegExpValidator(
		   QRegExp("^[\\p{L}0-9]*$",Qt::CaseInsensitive), this));

//	fieldOrderView->header()->hide();
//	fieldOrderView->setSorting(-1);
	setMinimumSize(50,50);
}

LDVPartList::~LDVPartList(void)
{
}

void LDVPartList::populateColumnList(void)
{
	const LDVPartListColumnVector &columnOrder =
		m_htmlInventory->getColumnOrder();
	int i;
	fieldOrderView->clear();
	for (i = 0; i < (int)columnOrder.size(); i++)
	{
		LDVPartListColumn column = columnOrder[i];
		const char *name = LDVHtmlInventory::getColumnName(column);
		QListWidgetItem *item = new QListWidgetItem(name, fieldOrderView);
		item->setCheckState(m_htmlInventory->isColumnEnabled(column) ? Qt::Checked : Qt::Unchecked);
		item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
	}
	for (i = LDVPLCFirst; i <= LDVPLCLast; i++)
	{
		LDVPartListColumn column = (LDVPartListColumn)i;
		if (!m_htmlInventory->isColumnEnabled(column))
		{
			const char *name = LDVHtmlInventory::getColumnName(column);
			QListWidgetItem *item = new QListWidgetItem(name, fieldOrderView);
			item->setCheckState(m_htmlInventory->isColumnEnabled(column) ? Qt::Checked : Qt::Unchecked);
			item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
		}
	}

}

void LDVPartList::doUp()
{
	doMoveColumn(-1);
}

void LDVPartList::doDown()
{
	doMoveColumn(1);
}

void LDVPartList::doOk()
{
	int i,j;
	QListWidgetItem *item;
	bool showmodel,openDocument;
	LDVPartListColumnVector columnOrder;
	m_htmlInventory->setExternalCssFlag(generateExternalSSButton->isChecked());
	m_htmlInventory->setPartImagesFlag(showPartImageButton->isChecked());
	m_htmlInventory->setShowModelFlag(showmodel = showModelButton->isChecked());
	if (showmodel)
	{
		 m_htmlInventory->setOverwriteSnapshotFlag(
					overwriteExistingButton->isChecked());
		 m_htmlInventory->setSnapshotWidthFlag(
					 snapshotWidthEdit->text().toInt());
		 m_htmlInventory->setSnapshotHeightFlag(
					 snapshotHeightEdit->text().toInt());
		 if (overwriteExistingButton->isChecked() && !(snapshotEdit->text().isEmpty()))
			 m_htmlInventory->setUserDefinedSnapshot(snapshotEdit->text().toUtf8().constData());
	}
	m_htmlInventory->setShowFileFlag(openDocument = showWebPageButton->isChecked());
	if (openDocument) {
		 m_htmlInventory->setGeneratePdfFlag(false); // not implemented
	}

	m_htmlInventory->setLookupSiteFlag(partLookupSiteCombo->currentIndex());
	m_htmlInventory->setElementSourceFlag(elementSourceCombo->currentIndex());

	doUpdateRebrickableKey();

	for (item = fieldOrderView->item(j = 0) ; item && (j < (fieldOrderView->count()));
		 item = fieldOrderView->item(++j))
	{
			QListWidgetItem *item2 =  (QListWidgetItem*)item;
		for (i = LDVPLCFirst; i <= LDVPLCLast; i++)
		{
			LDVPartListColumn column = LDVPartListColumn(i);
			const char *name = LDVHtmlInventory::getColumnName(column);
			if (strcmp(name,item->text().toUtf8().constData())==0)
			{
				if (item2->checkState() == Qt::Checked)
				{
					columnOrder.push_back(column);
				}
			}
		}
	}
	m_htmlInventory->setColumnOrder(columnOrder);
	accept();
}

void LDVPartList::doCancel()
{
	reject();
}

void LDVPartList::doMoveColumn(int distance)
{
	QListWidgetItem *item = fieldOrderView->currentItem();
	QListWidgetItem *newitem = ( fieldOrderView->item(fieldOrderView->currentRow() + (distance == 1 ? 1 : -1)));
	if (!newitem) return;
	QString ttt=newitem->text();
	Qt::CheckState s = ((QListWidgetItem*)newitem)->checkState();
	newitem->setText(item->text());
	((QListWidgetItem*)newitem)->setCheckState(((QListWidgetItem*)item)->checkState());
	item->setText(ttt);
	((QListWidgetItem*)item)->setCheckState(s);
	fieldOrderView->setCurrentItem(newitem);
	doHighlighted(newitem,newitem);
}

void LDVPartList::doHighlighted(QListWidgetItem * current, QListWidgetItem * /* previous */)
{
	int item = fieldOrderView->row(current);
	upButton->setEnabled(item > 0);
	downButton->setEnabled(item < fieldOrderView->count()-1);
}

int LDVPartList::exec()
{
	bool showmodel;
	populateColumnList();
	generateExternalSSButton->setChecked(
		m_htmlInventory->getExternalCssFlag());
	showPartImageButton->setChecked(
		m_htmlInventory->getPartImagesFlag());
	showModelButton->setChecked(showmodel =
		m_htmlInventory->getShowModelFlag());
	overwriteExistingButton->setChecked(showmodel ?
		m_htmlInventory->getOverwriteSnapshotFlag() : false);
	showWebPageButton->setChecked(m_htmlInventory->getShowFileFlag());
	partLookupSiteCombo->setCurrentIndex(
		m_htmlInventory->getLookupSiteFlag());
	elementSourceCombo->setCurrentIndex(
		m_htmlInventory->getElementSourceFlag());
	snapshotWidthEdit->setText(
		QString::number(m_htmlInventory->getSnapshotWidthFlag()));
	snapshotHeightEdit->setText(
		QString::number(m_htmlInventory->getSnapshotHeightFlag()));
	rebrickableKeyEdit->setEnabled(
		m_htmlInventory->getLookupSiteFlag() == LDVHtmlInventory::LookUp::Rebrickable);
	rebrickableKeyEdit->setText(
		QString::fromStdString(m_htmlInventory->getRebrickableKeyFlag()));

	doShowModel();

	return QDialog::exec();
}

void LDVPartList::doShowModel()
{
	bool show = showModelButton->isChecked();

	overwriteExistingButton->setEnabled(show);
	snapshotWidthEdit->setEnabled(show);
	snapshotHeightEdit->setEnabled(show);
	widthLabel->setEnabled(show);
	heightLabel->setEnabled(show);
	snapshotButton->setEnabled(show);
	snapshotEdit->setEnabled(show);
}

void LDVPartList::doShowPreferences()
{
	 LDVPreferences *preferences = new LDVPreferences(m_modelWidget);
	 if (preferences->exec() == QDialog::Accepted)
		 preferences->doOk();
	 else
		 preferences->doCancel();
}

void LDVPartList::doBrowseSnapshotFile()
{
	QString Result = QFileDialog::getOpenFileName(this, tr("Select Snapshot File"), snapshotEdit->text(), tr("Supported Image File (*.png);;PNG Files (*.png)"));
	if (!Result.isEmpty()) {
		doSetSnapshotFile(Result);
	}
}

void LDVPartList::doUpdateSnapshotEdit()
{
	QString Result = snapshotEdit->text();
	if (!Result.isEmpty())
		doSetSnapshotFile(Result);
}

void LDVPartList::doSetSnapshotFile(QString &Result)
{
	preferencesButton->setEnabled(false);
	snapshotEdit->setText(QDir::toNativeSeparators(Result));

	QImage image(Result);
	if (snapshotWidthEdit->text().toInt() != image.width()) {
		m_htmlInventory->setSnapshotWidthFlag(image.width());
		snapshotWidthEdit->setText(QString::number(m_htmlInventory->getSnapshotWidthFlag()));
	}
	if (snapshotHeightEdit->text().toInt() != image.height()) {
		m_htmlInventory->setSnapshotHeightFlag(image.height());
		snapshotHeightEdit->setText(QString::number(m_htmlInventory->getSnapshotHeightFlag()));
	}
}

void LDVPartList::doUpdateRebrickableKey()
{
	QString Result = rebrickableKeyEdit->text();
	if (!Result.isEmpty() && Result != QString::fromStdString(m_htmlInventory->getRebrickableKeyFlag()))
		m_htmlInventory->setRebrickableKeyFlag(Result.toStdString());
}

void LDVPartList::doEnableRebrickableKeyEdit(int index)
{
	rebrickableKeyEdit->setEnabled(index == LDVHtmlInventory::LookUp::Rebrickable);
}
