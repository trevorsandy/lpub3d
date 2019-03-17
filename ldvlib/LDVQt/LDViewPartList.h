#ifndef __LDVIEWPARTLIST_H__
#define __LDVIEWPARTLIST_H__

#include "LDVHtmlInventory.h"
#include "ui_LDVPartList.h"

class LDVWidget;
class LDVPreferences;
class LDVHtmlInventory;

class LDVPartList : public QDialog , Ui::LDVPartListPanel
{
	Q_OBJECT
public:
    LDVPartList(LDVWidget *modelWidget, LDVHtmlInventory *htmlInventory);
    ~LDVPartList(void);
    void populateColumnList();
//    QCheckListItem *description;
    int exec();

public slots:
	void doOk();
	void doCancel();
	void doUp();
	void doDown();
	void doHighlighted(QListWidgetItem * current, QListWidgetItem * previous );
	void doShowModel();
    void doShowPreferences();
protected:
	void doMoveColumn(int distance);
    LDrawModelViewer *m_modelViewer;
    LDVWidget        *m_modelWidget;
    LDVHtmlInventory *m_htmlInventory;
};
#endif //__LDVIEWPARTLIST_H__
