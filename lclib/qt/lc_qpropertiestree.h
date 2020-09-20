#pragma once

#include "lc_array.h"

struct lcPartProperties;
class lcQPropertiesTreeDelegate;

enum lcPropertyWidgetMode
{
	LC_PROPERTY_WIDGET_EMPTY,
	LC_PROPERTY_WIDGET_PIECE,
	LC_PROPERTY_WIDGET_CAMERA,
	LC_PROPERTY_WIDGET_LIGHT,
	LC_PROPERTY_WIDGET_MULTIPLE
};

class lcQPropertiesTree : public QTreeWidget
{
	Q_OBJECT

public:
	lcQPropertiesTree(QWidget *parent = 0);

	QSize sizeHint() const;

	QTreeWidgetItem *indexToItem(const QModelIndex &index) const
	{
		return itemFromIndex(index);
	}

	void Update(const lcArray<lcObject*>& Selection, lcObject* Focus);

	QWidget *createEditor(QWidget *parent, QTreeWidgetItem *item) const;
	bool lastColumn(int column) const;

	enum
	{
		PropertyTypeRole = Qt::UserRole,
		PropertyValueRole
	};

	enum PropertyType
	{
		PropertyGroup,
		PropertyBool,
		PropertyFloat,
/*** LPub3D Mod - Camera Globe, Custom properties ***/
		PropertyFloatCameraAngle,
		PropertyFloatTarget,
/*** LPub3D Mod end ***/
		PropertyInt,
		PropertyString,
		PropertyColor,
		PropertyPart,
/*** LPub3D Mod - LPub3D properties ***/
		PropertyFloatReadOnly,
		PropertyIntReadOnly
/*** LPub3D Mod end ***/
	};

protected slots:
	void slotToggled(bool value);
	void slotReturnPressed();
	void slotSetValue(int value);
	void slotColorButtonClicked();

protected:
	void keyPressEvent(QKeyEvent *event);
	void mousePressEvent(QMouseEvent *event);
	void drawRow(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
	void updateColorEditor(QPushButton *editor, int value) const;

	QTreeWidgetItem *addProperty(QTreeWidgetItem *parent, const QString& label, PropertyType propertyType);

	void SetEmpty();
	void SetPiece(const lcArray<lcObject*>& Selection, lcObject* Focus);
	void SetCamera(lcObject* Focus);
	void SetLight(lcObject* Focus);
	void SetMultiple();

	void getPartProperties(lcPartProperties *properties);
/*** LPub3D Mod - LPub3D properties ***/
	bool FirstHit;
/*** LPub3D Mod end ***/
	lcPropertyWidgetMode mWidgetMode;
	lcObject* mFocus;

	lcQPropertiesTreeDelegate *m_delegate;
	QIcon m_expandIcon;
	QIcon m_checkedIcon;
	QIcon m_uncheckedIcon;

	QTreeWidgetItem *partPosition;
	QTreeWidgetItem *partPositionX;
	QTreeWidgetItem *partPositionY;
	QTreeWidgetItem *partPositionZ;
	QTreeWidgetItem *partRotation;
	QTreeWidgetItem *partRotationX;
	QTreeWidgetItem *partRotationY;
	QTreeWidgetItem *partRotationZ;
	QTreeWidgetItem *partVisibility;
	QTreeWidgetItem *partShow;
	QTreeWidgetItem *partHide;
	QTreeWidgetItem *partAppearance;
	QTreeWidgetItem *partColor;
	QTreeWidgetItem *partID;
/*** LPub3D Mod - LPub3D Properties ***/
	QTreeWidgetItem *partFileName;
	QTreeWidgetItem *partModel;
	QTreeWidgetItem *partType;
	QTreeWidgetItem *partIsSubmodel;
/*** LPub3D Mod end ***/

/*** LPub3D Mod - Camera Globe ***/
	QTreeWidgetItem *cameraGlobe;
	QTreeWidgetItem *cameraGlobeLatitude;
	QTreeWidgetItem *cameraGlobeLongitude;
	QTreeWidgetItem *cameraGlobeDistance;
	QTreeWidgetItem *picture;
	QTreeWidgetItem *pictureModelScale;
	QTreeWidgetItem *pictureResolution;
	QTreeWidgetItem *picturePageSizeWidth;
	QTreeWidgetItem *picturePageSizeHeight;
	QTreeWidgetItem *pictureImageSizeWidth;
	QTreeWidgetItem *pictureImageSizeHeight;
/*** LPub3D Mod end ***/
	QTreeWidgetItem *cameraPosition;
	QTreeWidgetItem *cameraPositionX;
	QTreeWidgetItem *cameraPositionY;
	QTreeWidgetItem *cameraPositionZ;
	QTreeWidgetItem *cameraTarget;
	QTreeWidgetItem *cameraTargetX;
	QTreeWidgetItem *cameraTargetY;
	QTreeWidgetItem *cameraTargetZ;
	QTreeWidgetItem *cameraUp;
	QTreeWidgetItem *cameraUpX;
	QTreeWidgetItem *cameraUpY;
	QTreeWidgetItem *cameraUpZ;
	QTreeWidgetItem *cameraSettings;
	QTreeWidgetItem *cameraOrtho;
	QTreeWidgetItem *cameraFOV;
	QTreeWidgetItem *cameraNear;
	QTreeWidgetItem *cameraFar;
	QTreeWidgetItem *cameraName;
};

class lcQPropertiesTreeDelegate : public QItemDelegate
{
	Q_OBJECT
public:
	lcQPropertiesTreeDelegate(QObject *parent = 0)
		: QItemDelegate(parent), m_treeWidget(0), m_editedItem(0), m_editedWidget(0), m_disablePainting(false)
	{}

	void setTreeWidget(lcQPropertiesTree *treeWidget)
	{
		m_treeWidget = treeWidget;
	}

	QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
	void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const;
	void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
	QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
	void setModelData(QWidget *, QAbstractItemModel *, const QModelIndex &) const {}
	void setEditorData(QWidget *, const QModelIndex &) const {}
	bool eventFilter(QObject *object, QEvent *event);

	QTreeWidgetItem *editedItem() const
	{
		return m_editedItem;
	}

	QWidget *editor() const
	{
		return m_editedWidget;
	}

protected:
	void drawDecoration(QPainter *painter, const QStyleOptionViewItem &option, const QRect &rect, const QPixmap &pixmap) const;
	void drawDisplay(QPainter *painter, const QStyleOptionViewItem &option, const QRect &rect, const QString &text) const;

private slots:
	void slotEditorDestroyed(QObject *object);

private:
	int indentation(const QModelIndex &index) const;

	lcQPropertiesTree *m_treeWidget;
	mutable QTreeWidgetItem *m_editedItem;
	mutable QWidget *m_editedWidget;
	mutable bool m_disablePainting;
};

