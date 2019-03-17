#include <TCFoundation/mystring.h>
#include "LDViewPartList.h"
#include "LDVWidget.h"
#include <LDVPreferences.h>
#include <LDVWidgetDefaultKeys.h>

LDVPartList::LDVPartList(LDVWidget *modelWidget, LDVHtmlInventory *htmlInventory)
        : QDialog(qobject_cast<QWidget*>(modelWidget)),
         LDVPartListPanel(),
         m_modelViewer(modelWidget->getModelViewer() ? ((LDrawModelViewer*)modelWidget->getModelViewer()->retain()) : nullptr),
         m_modelWidget(modelWidget),
         m_htmlInventory(htmlInventory)
{
	setupUi(this);
    connect( upButton, SIGNAL( clicked() ), this, SLOT( doUp() ) );
    connect( downButton, SIGNAL( clicked() ), this, SLOT( doDown() ) );
    connect( okButton, SIGNAL( clicked() ), this, SLOT( doOk() ) );
    connect( cancelButton, SIGNAL( clicked() ), this, SLOT( doCancel() ) );
    connect( showModelButton, SIGNAL( clicked() ), this, SLOT( doShowModel() ) );
    connect( fieldOrderView, SIGNAL( currentItemChanged(QListWidgetItem *, QListWidgetItem *) ), this, SLOT( doHighlighted(QListWidgetItem *, QListWidgetItem *) ) );
    connect( preferencesButton, SIGNAL( clicked() ), this, SLOT( doShowPreferences() ) );

    if (m_modelWidget &&
            m_htmlInventory->getLookupSite() ==
            LDVHtmlInventory::LookUp::Bricklink)
        emit m_modelWidget->loadBLElementsSig();

//	fieldOrderView->header()->hide();
//	fieldOrderView->setSorting(-1);
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
	}
    m_htmlInventory->setShowFileFlag(openDocument = showWebPageButton->isChecked());
    if (openDocument){
         m_htmlInventory->setGeneratePdfFlag(false); // not implemented
    }

    m_htmlInventory->setLookupSiteFlag(partLookupSiteCombo->currentIndex());
    m_htmlInventory->setElementSourceFlag(elementSourceCombo->currentIndex());

    for (item = fieldOrderView->item(j = 0) ; item && (j < (fieldOrderView->count()));
		 item = fieldOrderView->item(++j))
	{
            QListWidgetItem *item2 =  (QListWidgetItem*)item;
        for (i = LDVPLCFirst; i <= LDVPLCLast; i++)
		{
            LDVPartListColumn column = LDVPartListColumn(i);
            const char *name = LDVHtmlInventory::getColumnName(column);
            if (strcmp(name,item->text().toLatin1().constData())==0)
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

	doShowModel();
	return QDialog::exec();
}

void LDVPartList::doShowModel()
{
	overwriteExistingButton->setEnabled(showModelButton->isChecked());
}

void LDVPartList::doShowPreferences()
{
    if (m_modelWidget->getLDVPreferences()) {
        LDVPreferences *preferences = m_modelWidget->getLDVPreferences();
        if (preferences->exec() == QDialog::Accepted)
            preferences->doOk();
        else
            preferences->doCancel();
    }
}
