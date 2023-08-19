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

	QSize sizeHint() const override;

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
/*** LPub3D Mod - enable lights ***/
		PropertyFloatLightSpotSize,
		PropertyFloatLightSpotFalloff,
/*** LPub3D Mod end ***/
/*** LPub3D Mod - Camera Globe, Custom properties ***/
		PropertyBoolReadOnly,
		PropertyFloatCameraAngle,
		PropertyFloatTarget,
/*** LPub3D Mod end ***/
		PropertyStep,
		PropertyString,
/*** LPub3D Mod - enable lights ***/
		PropertyStringLightReadOnly,
		PropertyLightFormat,
		PropertyLightShape,
		PropertyLightColor,
/*** LPub3D Mod end ***/
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
/*** LPub3D Mod - enable lights ***/
	void slotSetColorValue(QColor Value);
/*** LPub3D Mod end ***/

protected:
	void keyPressEvent(QKeyEvent *event) override;
	void mousePressEvent(QMouseEvent *event) override;
	void drawRow(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
	void updateColorEditor(QPushButton *editor, int value) const;
/*** LPub3D Mod - enable lights ***/
	void updateLightColorEditor(QPushButton *editor, QColor color) const;
/*** LPub3D Mod end ***/

	QTreeWidgetItem *addProperty(QTreeWidgetItem *parent, const QString& label, PropertyType propertyType);

	void SetEmpty();
	void SetPiece(const lcArray<lcObject*>& Selection, lcObject* Focus);
	void SetCamera(lcObject* Focus);
	void SetLight(lcObject* Focus);
	void SetMultiple();

	void getPartProperties(lcPartProperties *properties);

/*** LPub3D Mod - enable lights ***/
	int mLightType;
	int mLightShape;
	bool mPOVRayLight;
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

/*** LPub3D Mod - enable lights ***/
	QTreeWidgetItem *lightConfiguration;
	QTreeWidgetItem *lightPosition;
	QTreeWidgetItem *lightPositionX;
	QTreeWidgetItem *lightPositionY;
	QTreeWidgetItem *lightPositionZ;
	QTreeWidgetItem *lightTarget;
	QTreeWidgetItem *lightTargetX;
	QTreeWidgetItem *lightTargetY;
	QTreeWidgetItem *lightTargetZ;
	QTreeWidgetItem *lightColor;
	QTreeWidgetItem *lightColorIcon;
	QTreeWidgetItem *lightColorR;
	QTreeWidgetItem *lightColorG;
	QTreeWidgetItem *lightColorB;
	QTreeWidgetItem *lightProperties;
	QTreeWidgetItem *lightDiffuse;
	QTreeWidgetItem *lightSpecular;
	QTreeWidgetItem *lightCutoff;
	QTreeWidgetItem *lightEnableCutoff;
	QTreeWidgetItem *lightExponent;
	QTreeWidgetItem *lightType;
	QTreeWidgetItem *lightSpotSize;
	QTreeWidgetItem *lightShape;
	QTreeWidgetItem *lightFactorA;
	QTreeWidgetItem *lightFactorB;
	QTreeWidgetItem *lightName;
	QTreeWidgetItem *lightFormat;
	QTreeWidgetItem *lightShadowless;
	QTreeWidgetItem *lightAreaGridRows;
	QTreeWidgetItem *lightAreaGridColumns;
	QTreeWidgetItem *lightSpotFalloff;
	QTreeWidgetItem *lightSpotTightness;
/*** LPub3D Mod end ***/
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

	QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
	void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
	void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
	QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
	void setModelData(QWidget *, QAbstractItemModel *, const QModelIndex &) const override {}
	void setEditorData(QWidget *, const QModelIndex &) const override {}
	bool eventFilter(QObject *object, QEvent *event) override;

	QTreeWidgetItem *editedItem() const
	{
		return m_editedItem;
	}

	QWidget *editor() const
	{
		return m_editedWidget;
	}

protected:
	void drawDecoration(QPainter *painter, const QStyleOptionViewItem &option, const QRect &rect, const QPixmap &pixmap) const override;
	void drawDisplay(QPainter *painter, const QStyleOptionViewItem &option, const QRect &rect, const QString &text) const override;

private slots:
	void slotEditorDestroyed(QObject *object);

private:
	int indentation(const QModelIndex &index) const;

	lcQPropertiesTree *m_treeWidget;
	mutable QTreeWidgetItem *m_editedItem;
	mutable QWidget *m_editedWidget;
	mutable bool m_disablePainting;
};
