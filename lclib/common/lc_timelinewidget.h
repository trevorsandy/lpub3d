#pragma once

class lcTimelineWidget : public QTreeWidget
{
	Q_OBJECT

public:
	lcTimelineWidget(QWidget* Parent);
	virtual ~lcTimelineWidget();

	void Update(bool Clear, bool UpdateItems);
	void UpdateSelection();

	void InsertStep();
	void RemoveStep();
	void MoveSelection();
	void SetCurrentStep();

public slots:
	void ItemSelectionChanged();
	void CustomMenuRequested(QPoint Pos);

protected:
	virtual void dropEvent(QDropEvent* Event);
	virtual void mousePressEvent(QMouseEvent* Event);
	void UpdateModel();
/*** LPub3D Mod - Timeline part icons ***/
    void GetIcon(int Size, int ColorIndex, bool IsModel);
    bool GetPieceIcon(int Size, QString IconUID);

    QMap<QString, QIcon> mPieceIcons;
/*** LPub3D Mod end ***/
	QMap<int, QIcon> mIcons;
	QMap<lcPiece*, QTreeWidgetItem*> mItems;
	bool mIgnoreUpdates;
};

