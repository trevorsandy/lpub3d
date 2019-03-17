#include <TCFoundation/mystring.h>
#include "LDViewPartList.h"
#include "LDVWidget.h"
#include <LDVPreferences.h>
#include <LDVWidgetDefaultKeys.h>

LDVPartList::LDVPartList(LDVWidget *modelWidget, LDHtmlInventory *htmlInventory)
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

    // TEMPORARY hide until I figure out how to enable preferences
    preferencesBox->hide();

//	fieldOrderView->header()->hide();
//	fieldOrderView->setSorting(-1);
}

LDVPartList::~LDVPartList(void)
{
}

void LDVPartList::populateColumnList(void)
{
	const LDPartListColumnVector &columnOrder =
        m_htmlInventory->getColumnOrder();
	int i;
	fieldOrderView->clear();
	for (i = 0; i < (int)columnOrder.size(); i++)
	{
		LDPartListColumn column = columnOrder[i];
		const char *name = LDHtmlInventory::getColumnName(column);
		QListWidgetItem *item = new QListWidgetItem(name, fieldOrderView);
		item->setCheckState(m_htmlInventory->isColumnEnabled(column) ? Qt::Checked : Qt::Unchecked);
		item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
	}
    for (i = LDPLCFirst; i <= LDPLCLast; i++)
    {
        LDPartListColumn column = (LDPartListColumn)i;
        if (!m_htmlInventory->isColumnEnabled(column))
        {
            const char *name = LDHtmlInventory::getColumnName(column);
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
	bool showmodel;
	LDPartListColumnVector columnOrder;
	m_htmlInventory->setExternalCssFlag(generateExternalSSButton->isChecked());
	m_htmlInventory->setPartImagesFlag(showPartImageButton->isChecked());
	m_htmlInventory->setShowModelFlag(showmodel = 
					  showModelButton->isChecked());
    m_htmlInventory->setShowFileFlag(showWebPageButton->isChecked());
	if (showmodel)
	{
		 m_htmlInventory->setOverwriteSnapshotFlag(
					overwriteExistingButton->isChecked());
	}
	for (item = fieldOrderView->item(j = 0) ; item && (j < (fieldOrderView->count())) ;
		 item = fieldOrderView->item(++j))
	{
            QListWidgetItem *item2 =  (QListWidgetItem*)item;
		for (i = LDPLCFirst; i <= LDPLCLast; i++)
		{
            LDPartListColumn column = LDPartListColumn(i);
            const char *name = LDHtmlInventory::getColumnName(column);
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
        if (m_modelWidget->getLDVPreferences()->exec() == QDialog::Accepted)
            m_modelWidget->getLDVPreferences()->doOk();
        else
            m_modelWidget->getLDVPreferences()->doCancel();
    }
}
