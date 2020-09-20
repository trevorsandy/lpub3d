#include "lc_global.h"
#include "lc_qpropertiestree.h"
#include "lc_qcolorpicker.h"
#include "lc_application.h"
#include "lc_model.h"
#include "lc_mainwindow.h"
#include "object.h"
#include "piece.h"
#include "camera.h"
#include "light.h"
#include "pieceinf.h"
#include "lc_library.h"
#include "lc_qutils.h"
/*** LPub3D Mod - Camera Globe ***/
#include "project.h"
/*** LPub3D Mod end ***/

// Draw an icon indicating opened/closing branches
static QIcon drawIndicatorIcon(const QPalette &palette, QStyle *style)
{
	QPixmap pix(14, 14);
	pix.fill(Qt::transparent);
	QStyleOption branchOption;
	branchOption.rect = QRect(2, 2, 9, 9); // ### hardcoded in qcommonstyle.cpp
	branchOption.palette = palette;
	branchOption.state = QStyle::State_Children;

	QPainter p;
	// Draw closed state
	p.begin(&pix);
	style->drawPrimitive(QStyle::PE_IndicatorBranch, &branchOption, &p);
	p.end();
	QIcon rc = pix;
	rc.addPixmap(pix, QIcon::Selected, QIcon::Off);
	// Draw opened state
	branchOption.state |= QStyle::State_Open;
	pix.fill(Qt::transparent);
	p.begin(&pix);
	style->drawPrimitive(QStyle::PE_IndicatorBranch, &branchOption, &p);
	p.end();

	rc.addPixmap(pix, QIcon::Normal, QIcon::On);
	rc.addPixmap(pix, QIcon::Selected, QIcon::On);
	return rc;
}

static QIcon drawCheckBox(bool value)
{
	QStyleOptionButton opt;
	opt.state |= value ? QStyle::State_On : QStyle::State_Off;
	opt.state |= QStyle::State_Enabled;
	const QStyle *style = QApplication::style();
	// Figure out size of an indicator and make sure it is not scaled down in a list view item
	// by making the pixmap as big as a list view icon and centering the indicator in it.
	// (if it is smaller, it can't be helped)
	const int indicatorWidth = style->pixelMetric(QStyle::PM_IndicatorWidth, &opt);
	const int indicatorHeight = style->pixelMetric(QStyle::PM_IndicatorHeight, &opt);
	const int listViewIconSize = indicatorWidth;
	const int pixmapWidth = indicatorWidth;
	const int pixmapHeight = qMax(indicatorHeight, listViewIconSize);

	opt.rect = QRect(0, 0, indicatorWidth, indicatorHeight);
	QPixmap pixmap = QPixmap(pixmapWidth, pixmapHeight);
	pixmap.fill(Qt::transparent);
	{
		// Center?
		const int xoff = (pixmapWidth  > indicatorWidth)  ? (pixmapWidth  - indicatorWidth)  / 2 : 0;
		const int yoff = (pixmapHeight > indicatorHeight) ? (pixmapHeight - indicatorHeight) / 2 : 0;
		QPainter painter(&pixmap);
		painter.translate(xoff, yoff);
		style->drawPrimitive(QStyle::PE_IndicatorCheckBox, &opt, &painter);
	}
	return QIcon(pixmap);
}

int lcQPropertiesTreeDelegate::indentation(const QModelIndex &index) const
{
	if (!m_treeWidget)
		return 0;

	QTreeWidgetItem *item = m_treeWidget->indexToItem(index);
	int indent = 0;
	while (item->parent())
	{
		item = item->parent();
		++indent;
	}

	if (m_treeWidget->rootIsDecorated())
		++indent;

	return indent * m_treeWidget->indentation();
}

void lcQPropertiesTreeDelegate::slotEditorDestroyed(QObject *object)
{
	if (m_editedWidget == object)
	{
		m_editedWidget = nullptr;
		m_editedItem = nullptr;
	}
}

QWidget *lcQPropertiesTreeDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &style, const QModelIndex &index) const
{
	Q_UNUSED(style);

	if (index.column() == 1 && m_treeWidget)
	{
		QTreeWidgetItem *item = m_treeWidget->indexToItem(index);

		if (item && (item->flags() & Qt::ItemIsEnabled))
		{
			QWidget *editor = m_treeWidget->createEditor(parent, item);
			if (editor)
			{
				editor->setAutoFillBackground(true);
				editor->installEventFilter(const_cast<lcQPropertiesTreeDelegate *>(this));

				m_editedItem = item;
				m_editedWidget = editor;

				connect(editor, SIGNAL(destroyed(QObject *)), this, SLOT(slotEditorDestroyed(QObject *)));

				return editor;
			}
		}
	}

	return nullptr;
}

void lcQPropertiesTreeDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	Q_UNUSED(index)
	editor->setGeometry(option.rect.adjusted(0, 0, 0, -1));
}

void lcQPropertiesTreeDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	bool hasValue = true;
	if (m_treeWidget)
		hasValue = m_treeWidget->indexToItem(index)->data(0, lcQPropertiesTree::PropertyTypeRole).toInt() != lcQPropertiesTree::PropertyGroup;

	QStyleOptionViewItem opt = option;

	opt.state &= ~QStyle::State_HasFocus;

	if (index.column() == 1 && m_treeWidget)
	{
		QTreeWidgetItem *item = m_treeWidget->indexToItem(index);
		if (m_editedItem && m_editedItem == item)
			m_disablePainting = true;
	}

	QItemDelegate::paint(painter, opt, index);
	if (option.type)
		m_disablePainting = false;

	opt.palette.setCurrentColorGroup(QPalette::Active);
	QColor color = static_cast<QRgb>(QApplication::style()->styleHint(QStyle::SH_Table_GridLineColor, &opt));
	painter->save();
	painter->setPen(QPen(color));

	if (!m_treeWidget || (!m_treeWidget->lastColumn(index.column()) && hasValue))
	{
		int right = (option.direction == Qt::LeftToRight) ? option.rect.right() : option.rect.left();
		painter->drawLine(right, option.rect.y(), right, option.rect.bottom());
	}

	painter->restore();
}

void lcQPropertiesTreeDelegate::drawDecoration(QPainter *painter, const QStyleOptionViewItem &option, const QRect &rect, const QPixmap &pixmap) const
{
	if (m_disablePainting)
		return;

	QItemDelegate::drawDecoration(painter, option, rect, pixmap);
}

void lcQPropertiesTreeDelegate::drawDisplay(QPainter *painter, const QStyleOptionViewItem &option, const QRect &rect, const QString &text) const
{
	if (m_disablePainting)
		return;

	QItemDelegate::drawDisplay(painter, option, rect, text);
}

QSize lcQPropertiesTreeDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	return QItemDelegate::sizeHint(option, index) + QSize(3, 4);
}

bool lcQPropertiesTreeDelegate::eventFilter(QObject *object, QEvent *event)
{
	if (event->type() == QEvent::FocusOut)
	{
		QFocusEvent *fe = static_cast<QFocusEvent *>(event);
		if (fe->reason() == Qt::ActiveWindowFocusReason)
			return false;
	}
	return QItemDelegate::eventFilter(object, event);
}

lcQPropertiesTree::lcQPropertiesTree(QWidget *parent) :
	QTreeWidget(parent)
{
	setIconSize(QSize(18, 18));
	setColumnCount(2);
	QStringList labels;
	labels.append(tr("Property"));
	labels.append(tr("Value"));
	setHeaderLabels(labels);
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
	header()->setSectionsMovable(false);
	header()->setSectionResizeMode(QHeaderView::ResizeToContents);
#else
	header()->setMovable(false);
	header()->setResizeMode(QHeaderView::ResizeToContents);
#endif
	header()->setVisible(false);
	setAlternatingRowColors(true);
	setRootIsDecorated(false);
	setEditTriggers(QAbstractItemView::EditKeyPressed);

	m_expandIcon = drawIndicatorIcon(palette(), style());
	m_checkedIcon = drawCheckBox(true);
	m_uncheckedIcon = drawCheckBox(false);

	m_delegate = new lcQPropertiesTreeDelegate(parent);
	m_delegate->setTreeWidget(this);
	setItemDelegate(m_delegate);

	SetEmpty();

	connect(header(), SIGNAL(sectionDoubleClicked(int)), this, SLOT(resizeColumnToContents(int)));
}

QSize lcQPropertiesTree::sizeHint() const
{
	return QSize(200, -1);
}

void lcQPropertiesTree::drawRow(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	QStyleOptionViewItem opt = option;

	QTreeWidgetItem *item = itemFromIndex(index);

	if (item->data(0, lcQPropertiesTree::PropertyTypeRole).toInt() == lcQPropertiesTree::PropertyGroup)
	{
		const QColor c = option.palette.color(QPalette::Dark);
		painter->fillRect(option.rect, c);
		opt.palette.setColor(QPalette::AlternateBase, c);
	}

	QTreeWidget::drawRow(painter, opt, index);
	QColor color = static_cast<QRgb>(QApplication::style()->styleHint(QStyle::SH_Table_GridLineColor, &opt));
	painter->save();
	painter->setPen(QPen(color));
	painter->drawLine(opt.rect.x(), opt.rect.bottom(), opt.rect.right(), opt.rect.bottom());
	painter->restore();
}

void lcQPropertiesTree::keyPressEvent(QKeyEvent *event)
{
	switch (event->key())
	{
	case Qt::Key_Return:
	case Qt::Key_Enter:
	case Qt::Key_Space: // Trigger Edit
		if (!m_delegate->editedItem())
		{
			if (const QTreeWidgetItem *item = currentItem())
			{
				if (item->columnCount() >= 2 && ((item->flags() & (Qt::ItemIsEditable | Qt::ItemIsEnabled)) == (Qt::ItemIsEditable | Qt::ItemIsEnabled)))
				{
					event->accept();
					// If the current position is at column 0, move to 1.
					QModelIndex index = currentIndex();
					if (index.column() == 0)
					{
						index = index.sibling(index.row(), 1);
						setCurrentIndex(index);
					}
					edit(index);
					return;
				}
			}
		}
		break;

	default:
		break;
	}

	QTreeWidget::keyPressEvent(event);
}

void lcQPropertiesTree::mousePressEvent(QMouseEvent *event)
{
	QTreeWidget::mousePressEvent(event);
	QTreeWidgetItem *item = itemAt(event->pos());

	if (item)
	{
		if ((item != m_delegate->editedItem()) && (event->button() == Qt::LeftButton) && (header()->logicalIndexAt(event->pos().x()) == 1) &&
			((item->flags() & (Qt::ItemIsEditable | Qt::ItemIsEnabled)) == (Qt::ItemIsEditable | Qt::ItemIsEnabled)))
			editItem(item, 1);
	}
}

void lcQPropertiesTree::Update(const lcArray<lcObject*>& Selection, lcObject* Focus)
{
	lcPropertyWidgetMode Mode = LC_PROPERTY_WIDGET_EMPTY;

	if (Focus)
	{
		switch (Focus->GetType())
		{
		case LC_OBJECT_PIECE:
			Mode = LC_PROPERTY_WIDGET_PIECE;
			break;

		case LC_OBJECT_CAMERA:
			Mode = LC_PROPERTY_WIDGET_CAMERA;
			break;

		case LC_OBJECT_LIGHT:
			Mode = LC_PROPERTY_WIDGET_LIGHT;
			break;
		}
	}
	else
	{
		for (int ObjectIdx = 0; ObjectIdx < Selection.GetSize(); ObjectIdx++)
		{
			switch (Selection[ObjectIdx]->GetType())
			{
			case LC_OBJECT_PIECE:
				if (Mode == LC_PROPERTY_WIDGET_EMPTY)
				{
					Mode = LC_PROPERTY_WIDGET_PIECE;
/*** LPub3D Mod - set the focus on the first piece ***/
					Focus = (lcObject*)Selection[ObjectIdx];
/*** LPub3D Mod end ***/
				}
				else if (Mode != LC_PROPERTY_WIDGET_PIECE)
				{
					Mode = LC_PROPERTY_WIDGET_MULTIPLE;
					ObjectIdx = Selection.GetSize();
				}
				break;

			case LC_OBJECT_CAMERA:
				if (Mode != LC_PROPERTY_WIDGET_EMPTY)
				{
					Mode = LC_PROPERTY_WIDGET_MULTIPLE;
					ObjectIdx = Selection.GetSize();
				}
				else
				{
					Mode = LC_PROPERTY_WIDGET_CAMERA;
					Focus = Selection[ObjectIdx];
				}
				break;

			case LC_OBJECT_LIGHT:
				if (Mode != LC_PROPERTY_WIDGET_EMPTY)
				{
					Mode = LC_PROPERTY_WIDGET_MULTIPLE;
					ObjectIdx = Selection.GetSize();
				}
				else
				{
					Mode = LC_PROPERTY_WIDGET_LIGHT;
					Focus = Selection[ObjectIdx];
				}
				break;
			}
		}
	}

	switch (Mode)
	{
	case LC_PROPERTY_WIDGET_EMPTY:
		SetEmpty();
		break;

	case LC_PROPERTY_WIDGET_PIECE:
		SetPiece(Selection, Focus);
		break;

	case LC_PROPERTY_WIDGET_CAMERA:
		SetCamera(Focus);
		break;

	case LC_PROPERTY_WIDGET_LIGHT:
		SetLight(Focus);
		break;

	case LC_PROPERTY_WIDGET_MULTIPLE:
		SetMultiple();
		break;
	}
}

class lcStepValidator : public QIntValidator
{
public:
	lcStepValidator(lcStep Min, lcStep Max, bool AllowEmpty)
		: QIntValidator(1, INT_MAX), mMin(Min), mMax(Max), mAllowEmpty(AllowEmpty)
	{
	}

	QValidator::State validate(QString& Input, int& Pos) const
	{
		if (mAllowEmpty && Input.isEmpty())
			return Acceptable;

		bool Ok;
		lcStep Step = Input.toUInt(&Ok);

		if (Ok)
			return (Step >= mMin && Step <= mMax) ? Acceptable : Invalid;

		return QIntValidator::validate(Input, Pos);
	}

protected:
	lcStep mMin;
	lcStep mMax;
	bool mAllowEmpty;
};

QWidget *lcQPropertiesTree::createEditor(QWidget *parent, QTreeWidgetItem *item) const
{
	PropertyType propertyType = (PropertyType)item->data(0, lcQPropertiesTree::PropertyTypeRole).toInt();

	switch (propertyType)
	{
	case PropertyGroup:
		return nullptr;

	case PropertyBool:
		{
			QCheckBox *editor = new QCheckBox(parent);
			bool value = item->data(0, PropertyValueRole).toBool();

			editor->setChecked(value);

			connect(editor, SIGNAL(toggled(bool)), this, SLOT(slotToggled(bool)));

			return editor;
		}

	case PropertyFloat:
		{
			QLineEdit *editor = new QLineEdit(parent);
			float value = item->data(0, PropertyValueRole).toFloat();

			editor->setValidator(new QDoubleValidator(editor));
			editor->setText(lcFormatValueLocalized(value));

			connect(editor, SIGNAL(returnPressed()), this, SLOT(slotReturnPressed()));

			return editor;
		}
/*** LPub3D Mod - enable lights ***/
	case PropertyFloatLightSpotSize:
	{
		QLineEdit *editor = new QLineEdit(parent);
		float value = item->data(0, PropertyValueRole).toFloat();

		editor->setValidator(new QDoubleValidator(1.0, 180.0,1, editor));
		editor->setText(lcFormatValueLocalized(value));
		editor->setToolTip(tr("Angle of the spotlight beam."));

		connect(editor, SIGNAL(returnPressed()), this, SLOT(slotReturnPressed()));
		return editor;
	}
/*** LPub3D Mod end ***/
/*** LPub3D Mod - Camera Globe, Custom properties ***/
	case PropertyFloatCameraAngle:
		{
			QLineEdit *editor = new QLineEdit(parent);
			float value = item->data(0, PropertyValueRole).toFloat();

			QRegExp vrx("^(?:[+\\-]{0,1}[\\d]+[,|.]{0,1}[\\d]*){1}\\s{0,1}(?:[+\\-]{0,1}[\\d]+[,|.]{0,1}[\\d]*){0,1}$");
			editor->setValidator(new QRegExpValidator(vrx,parent));
			editor->setText(lcFormatValueLocalized(value));
			editor->setToolTip("Eneter respective angle or both angles. Use single space delimeter");

			connect(editor, SIGNAL(returnPressed()), this, SLOT(slotReturnPressed()));

			return editor;
		}
	case PropertyFloatTarget:
		{
			QLineEdit *editor = new QLineEdit(parent);
			float value = item->data(0, PropertyValueRole).toFloat();

			QRegExp vrx("^(?:[+\\-]{0,1}[\\d]+[,|.]{0,1}[\\d]*){1}\\s{0,1}(?:[+\\-]{0,1}[\\d]+[,|.]{0,1}[\\d]*){0,1}\\s{0,1}(?:[+\\-]{0,1}[\\d]+[,|.]{0,1}[\\d]*){0,1}$");
			editor->setValidator(new QRegExpValidator(vrx,parent));
			editor->setText(lcFormatValueLocalized(value));
			editor->setToolTip("Enter respective axis or all axes. Use single space delimeter");

			connect(editor, SIGNAL(returnPressed()), this, SLOT(slotReturnPressed()));

			return editor;
		}
/*** LPub3D Mod end ***/
/*** LPub3D Mod - LPub3D properties ***/
	case PropertyFloatReadOnly:
		{
			QLineEdit *editor = new QLineEdit(parent);
			float value = item->data(0, PropertyValueRole).toFloat();

			editor->setText(lcFormatValueLocalized(value));
			editor->setReadOnly(true);
			editor->setToolTip("Property is read only");

			return editor;
		}
/*** LPub3D Mod end ***/

	case PropertyStep:
		{
			QLineEdit* Editor = new QLineEdit(parent);

			lcStep Value = item->data(0, PropertyValueRole).toUInt();
			lcStep Show = partShow->data(0, PropertyValueRole).toUInt();
			lcStep Hide = partHide->data(0, PropertyValueRole).toUInt();

			if (Show && Hide)
			{
				if (item == partShow)
					Editor->setValidator(new lcStepValidator(1, Hide - 1, false));
				else
					Editor->setValidator(new lcStepValidator(Show + 1, LC_STEP_MAX, true));
			}
			else
				Editor->setValidator(new lcStepValidator(1, LC_STEP_MAX, item == partHide));

			if (item != partHide || Value != LC_STEP_MAX)
				Editor->setText(QString::number(Value));

			connect(Editor, SIGNAL(returnPressed()), this, SLOT(slotReturnPressed()));

			return Editor;
		}
/*** LPub3D Mod - LPub3D properties ***/
	case PropertyIntReadOnly:
		{
			QLineEdit *editor = new QLineEdit(parent);
			quint32 value = item->data(0, PropertyValueRole).toUInt();

			editor->setText(QString::number(value));
			editor->setReadOnly(true);
			editor->setToolTip("Property is read only");

			return editor;
		}
/*** LPub3D Mod end ***/

	case PropertyString:
		{
			QLineEdit *editor = new QLineEdit(parent);
			const char *value = (const char*)item->data(0, PropertyValueRole).value<void*>();

			editor->setText(value);

			connect(editor, SIGNAL(returnPressed()), this, SLOT(slotReturnPressed()));

			return editor;
		}
/*** LPub3D Mod - enable lights ***/
	case PropertyStringReadOnly:
		{
			QLineEdit *editor = new QLineEdit(parent);
			const char *value = (const char*)item->data(0, PropertyValueRole).value<void*>();

			editor->setText(value);
			editor->setReadOnly(true);
			editor->setToolTip("Property is read only");

			return editor;
		}
	case PropertyLightShape:
		{
			QComboBox *editor = new QComboBox(parent);

			editor->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
			editor->setMinimumContentsLength(1);

			QStringList shapes = { "Square", "Disk", "Rectangle",  "Ellipse"};
			for (int i = 0; i < shapes.size(); i++)
				editor->addItem(shapes.at(i), qVariantFromValue(i));

			int value = item->data(0, PropertyValueRole).toInt();
			editor->setCurrentIndex(value);

			connect(editor, SIGNAL(currentIndexChanged(int)), this, SLOT(slotSetValue(int)));

			return editor;
		}
	case PropertyLightColor:
	{
		QPushButton *editor = new QPushButton(parent);
		QColor value = item->data(0, PropertyValueRole).value<QColor>();

		updateLightColorEditor(editor, value);

		connect(editor, SIGNAL(clicked()), this, SLOT(slotColorButtonClicked()));

		return editor;
	}
/*** LPub3D Mod end ***/
	case PropertyColor:
		{
			QPushButton *editor = new QPushButton(parent);
			int value = item->data(0, PropertyValueRole).toInt();

			updateColorEditor(editor, value);

			connect(editor, SIGNAL(clicked()), this, SLOT(slotColorButtonClicked()));

			return editor;
		}

	case PropertyPart:
		{
			QComboBox *editor = new QComboBox(parent);

			editor->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
			editor->setMinimumContentsLength(1);

			lcPiecesLibrary* Library = lcGetPiecesLibrary();
			std::vector<PieceInfo*> SortedPieces;
			SortedPieces.reserve(Library->mPieces.size());

			for (const auto& PartIt : Library->mPieces)
				SortedPieces.push_back(PartIt.second);

			auto PieceCompare = [](PieceInfo* Info1, PieceInfo* Info2)
			{
				return strcmp(Info1->m_strDescription, Info2->m_strDescription) < 0;
			};

			std::sort(SortedPieces.begin(), SortedPieces.end(), PieceCompare);

			for (PieceInfo* Info : SortedPieces)
				editor->addItem(Info->m_strDescription, qVariantFromValue((void*)Info));

			PieceInfo *info = (PieceInfo*)item->data(0, PropertyValueRole).value<void*>();
			editor->setCurrentIndex(editor->findData(qVariantFromValue((void*)info)));

			connect(editor, SIGNAL(currentIndexChanged(int)), this, SLOT(slotSetValue(int)));

			return editor;
		}
	}

	return nullptr;
}

void lcQPropertiesTree::updateColorEditor(QPushButton *editor, int value) const
{
	QImage img(12, 12, QImage::Format_ARGB32);
	img.fill(0);

	lcColor* color = &gColorList[value];
	QPainter painter(&img);
	painter.setCompositionMode(QPainter::CompositionMode_Source);
	painter.setPen(Qt::darkGray);
	painter.setBrush(QColor::fromRgbF(qreal(color->Value[0]), qreal(color->Value[1]), qreal(color->Value[2])));
	painter.drawRect(0, 0, img.width() - 1, img.height() - 1);
	painter.end();

	editor->setStyleSheet("Text-align:left");
	editor->setIcon(QPixmap::fromImage(img));
	editor->setText(color->Name);
}

/*** LPub3D Mod - enable lights ***/
void lcQPropertiesTree::updateLightColorEditor(QPushButton *editor, QColor qcolor) const
{
	QImage img(12, 12, QImage::Format_ARGB32);
	img.fill(0);

	QString description = QString("Hex: %1").arg(qcolor.name());

	QPainter painter(&img);
	painter.setCompositionMode(QPainter::CompositionMode_Source);
	painter.setPen(Qt::darkGray);
	painter.setBrush(qcolor);
	painter.drawRect(0, 0, img.width() - 1, img.height() - 1);
	painter.end();

	editor->setStyleSheet("Text-align:left");
	editor->setIcon(QPixmap::fromImage(img));
	editor->setText(description);
}
/*** LPub3D Mod end ***/

void lcQPropertiesTree::slotToggled(bool Value)
{
	QTreeWidgetItem* Item = m_delegate->editedItem();
	lcModel* Model = gMainWindow->GetActiveModel();

/*** LPub3D Mod - enable lights ***/
	lcObject* Focus = Model->GetFocusObject();

	if (mWidgetMode == LC_PROPERTY_WIDGET_CAMERA)
	{
/*** LPub3D Mod end ***/
		if (Focus && Focus->IsCamera())
		{
			lcCamera* Camera = (lcCamera*)Focus;

			if (Item == cameraOrtho)
			{
				Model->SetCameraOrthographic(Camera, Value);
			}
		}
	}
/*** LPub3D Mod - enable lights ***/
	else if (mWidgetMode == LC_PROPERTY_WIDGET_LIGHT)
	{
		lcLight* Light = (Focus && Focus->IsLight()) ? (lcLight*)Focus : nullptr;

		if (Light && Item == lightEnableCutoff)
		{
			lcLightProps Props = Light->GetLightProps();
			Props.mEnableCutoff = Value;
			Model->UpdateLight(Light, Props, LC_LIGHT_USE_CUTOFF);
		}
	}
/*** LPub3D Mod end ***/
}

void lcQPropertiesTree::slotReturnPressed()
{
	QLineEdit* Editor = (QLineEdit*)sender();
	QTreeWidgetItem* Item = m_delegate->editedItem();
	lcModel* Model = gMainWindow->GetActiveModel();

	if (mWidgetMode == LC_PROPERTY_WIDGET_PIECE)
	{
		lcPiece* Piece = (mFocus && mFocus->IsPiece()) ? (lcPiece*)mFocus : nullptr;

		if (Item == partPositionX || Item == partPositionY || Item == partPositionZ)
		{
			lcVector3 Center;
			lcMatrix33 RelativeRotation;
			Model->GetMoveRotateTransform(Center, RelativeRotation);
			lcVector3 Position = Center;
			float Value = lcParseValueLocalized(Editor->text());

/*** LPub3D Mod - Switch Y and Z axis ***/
			if (Item == partPositionX)
				Position[0] = Value;
			else if (Item == partPositionY)
				Position[2] = Value;
			else if (Item == partPositionZ)
				Position[1] = Value;
/*** LPub3D Mod end ***/

			lcVector3 Distance = Position - Center;

			Model->MoveSelectedObjects(Distance, Distance, false, false, true, true);
		}
		else if (Item == partRotationX || Item == partRotationY || Item == partRotationZ)
		{
			lcVector3 InitialRotation;
			if (Piece)
				InitialRotation = lcMatrix44ToEulerAngles(Piece->mModelWorld) * LC_RTOD;
			else
				InitialRotation = lcVector3(0.0f, 0.0f, 0.0f);
			lcVector3 Rotation = InitialRotation;

			float Value = lcParseValueLocalized(Editor->text());

/*** LPub3D Mod - Switch Y and Z axis ***/
			if (Item == partRotationX)
				Rotation[0] = Value;
			else if (Item == partRotationY)
				Rotation[2] = Value;
			else if (Item == partRotationZ)
				Rotation[1] = Value;
/*** LPub3D Mod end ***/

			Model->RotateSelectedPieces(Rotation - InitialRotation, true, false, true, true);
		}
		else if (Item == partShow)
		{
			bool Ok = false;
			lcStep Step = Editor->text().toUInt(&Ok);

			if (Ok)
				Model->SetSelectedPiecesStepShow(Step);
		}
		else if (Item == partHide)
		{
			QString Text = Editor->text();

			if (Text.isEmpty())
				Model->SetSelectedPiecesStepHide(LC_STEP_MAX);
			else
			{
				bool Ok = false;
				lcStep Step = Text.toUInt(&Ok);

				if (Ok)
					Model->SetSelectedPiecesStepHide(Step);
			}
		}
	}
	else if (mWidgetMode == LC_PROPERTY_WIDGET_CAMERA)
	{
		lcCamera* Camera = (mFocus && mFocus->IsCamera()) ? (lcCamera*)mFocus : nullptr;

		if (Camera)
		{
/*** LPub3D Mod - Camera Globe, camera name ***/
			const char* Name = "";
			Name = Camera->GetName();
			bool isDefault = Name && !Name[0];
/*** LPub3D Mod end ***/

			if (Item == cameraPositionX || Item == cameraPositionY || Item == cameraPositionZ)
			{
				lcVector3 Center = Camera->mPosition;
				lcVector3 Position = Center;
				float Value = lcParseValueLocalized(Editor->text());

/*** LPub3D Mod - Camera Globe, Switch Y and Z axis ***/
				if (Item == cameraPositionX)
					Position[0] = Value;
				else if (Item == cameraPositionY)
					Position[2] = Value;
				else if (Item == cameraPositionZ)
					Position[1] = Value;
/*** LPub3D Mod end ***/

				lcVector3 Distance = Position - Center;

/*** LPub3D Mod - Camera Globe  ***/
				if (isDefault) {
					Camera->mPosition = Position;
					Model->MoveDefaultCamera(Camera, Distance);
				} else
/*** LPub3D Mod end ***/
					Model->MoveSelectedObjects(Distance, Distance, false, false, true, true);
			}
			else if (Item == cameraTargetX || Item == cameraTargetY || Item == cameraTargetZ)
			{
				lcVector3 Center = Camera->mTargetPosition;
				lcVector3 Position = Center;

/*** LPub3D Mod - Camera Globe, Switch Y and Z axis ***/
				QStringList ValueList = QString(Editor->text()).trimmed().split(" ");

				float Value       = 0.0f;
				float SecondValue = 0.0f;
				float ThirdValue  = 0.0f;

				switch (ValueList.size()){
				case 1:
					Value       = lcParseValueLocalized(ValueList.first());
					break;
				case 2:
					Value       = lcParseValueLocalized(ValueList.first());
					SecondValue = lcParseValueLocalized(ValueList.last());
					break;
				case 3:
					Value       = lcParseValueLocalized(ValueList.at(0));
					SecondValue = lcParseValueLocalized(ValueList.at(1));
					ThirdValue  = lcParseValueLocalized(ValueList.at(2));
					break;
				}

				bool hasSingleVal = ValueList.size() == 2;
				bool hasOneOther  = ValueList.size() == 2;
				bool hasTwoOther  = ValueList.size() == 3;

				// Switch Y and Z axis
				if (Item == cameraTargetX) {
					Position[0] = Value;
					if (hasOneOther) {
						Position[2] = SecondValue;
					}
					if (hasTwoOther) {
						Position[2] = SecondValue;
						Position[1] = ThirdValue;
					}
				} else if (Item == cameraTargetY) {
					if (hasSingleVal) {
						Position[2] = Value;
					}
					if (hasOneOther) {
						Position[0] = SecondValue;
						Position[2] = Value;
					}
					if (hasTwoOther) {
						Position[0] = Value ;
						Position[2] = SecondValue;
						Position[1] = ThirdValue;
					}
				} else if (Item == cameraTargetZ) {
					if (hasSingleVal) {
						Position[1] = Value;
					}
					if (hasOneOther) {
						Position[2] = Value;
						Position[1] = SecondValue;
					}
					if (hasTwoOther) {
						Position[0] = Value ;
						Position[2] = SecondValue;
						Position[1] = ThirdValue;
					}
				}
/*** LPub3D Mod end ***/

				lcVector3 Distance = Position - Center;

/*** LPub3D Mod - Camera Globe  ***/
				if (isDefault) {
					Camera->mTargetPosition = Position;
					Model->MoveDefaultCamera(Camera, Distance);
				} else
/*** LPub3D Mod end ***/
					Model->MoveSelectedObjects(Distance, Distance, false, false, true, true);
			}
			else if (Item == cameraUpX || Item == cameraUpY || Item == cameraUpZ)
			{
				lcVector3 Center = Camera->mUpVector;
				lcVector3 Position = Center;
				float Value = lcParseValueLocalized(Editor->text());

/*** LPub3D Mod - Camera Globe, Switch Y and Z axis ***/
				if (Item == cameraUpX)
					Position[0] = Value;
				else if (Item == cameraUpY)
					Position[2] = Value;
				else if (Item == cameraUpZ)
					Position[1] = Value;
/*** LPub3D Mod end ***/

				lcVector3 Distance = Position - Center;

/*** LPub3D Mod - Camera Globe  ***/
				if (isDefault) {
					Camera->mUpVector = Position;
					Model->MoveDefaultCamera(Camera, Distance);
				} else
/*** LPub3D Mod end ***/
					Model->MoveSelectedObjects(Distance, Distance, false, false, true, true);
			}
/*** LPub3D Mod - Camera Globe ***/
			else if (Item == cameraGlobeLatitude || Item == cameraGlobeLongitude /*|| Item == cameraGlobeDistance*/)
			{
				QStringList ValueList = QString(Editor->text()).trimmed().split(" ",QString::SkipEmptyParts);
				bool hasOther = ValueList.size() == 2;

				float Value = lcParseValueLocalized(ValueList.first());
				float OtherValue = hasOther ? lcParseValueLocalized(ValueList.last()) : 0.0f;

				float Latitude, Longitude, Distance;
				Camera->GetAngles(Latitude,Longitude,Distance);
				if (Item == cameraGlobeLatitude) {
					Latitude = Value;
					if (hasOther)
					   Longitude = OtherValue;
				} else if (Item == cameraGlobeLongitude) {
					if (hasOther)
						Latitude = OtherValue;
					Longitude = Value;
				}

				Model->SetCameraGlobe(Camera, Latitude, Longitude, Distance);
			}
			else if (Item == pictureImageSizeWidth || Item == pictureImageSizeHeight)
			{
				float Value = lcParseValueLocalized(Editor->text());

				if (Item == pictureImageSizeWidth) {
					lcGetActiveProject()->SetImageSize(
								int(Value),
								lcGetActiveProject()->GetImageHeight());
				} else if (Item == pictureImageSizeHeight) {
					lcGetActiveProject()->SetImageSize(
								lcGetActiveProject()->GetImageWidth(),
								int(Value));
				}
			}
/*** LPub3D Mod end ***/
			else if (Item == cameraFOV)
			{
				float Value = lcParseValueLocalized(Editor->text());

				Model->SetCameraFOV(Camera, Value);
			}
			else if (Item == cameraNear)
			{
				float Value = lcParseValueLocalized(Editor->text());

				Model->SetCameraZNear(Camera, Value);
			}
			else if (Item == cameraFar)
			{
				float Value = lcParseValueLocalized(Editor->text());

				Model->SetCameraZFar(Camera, Value);
			}
			else if (Item == cameraName)
			{
				QString Value = Editor->text();

				Model->SetCameraName(Camera, Value.toLocal8Bit().data());
			}
/*** LPub3D Mod - Camera Globe ***/
			lcArray<lcObject*> Selection;
			Update(Selection, Camera);
/*** LPub3D Mod end ***/
		}
	}
/*** LPub3D Mod - enable lights ***/
	else if (mWidgetMode == LC_PROPERTY_WIDGET_LIGHT)
	{
		lcLight* Light  = (mFocus && mFocus->IsLight()) ? (lcLight*)mFocus : nullptr;

		if (Light)
		{
			lcLightProps Props = Light->GetLightProps();

			const char* Name = "";
			Name = Light->GetName();

			if (Item == lightPositionX || Item == lightPositionY || Item == lightPositionZ)
			{
				lcVector3 Center = Light->mPosition;
				lcVector3 Position = Center;
				float Value = lcParseValueLocalized(Editor->text());
				// Switch Y and Z axis
				if (Item == lightPositionX)
					Position[0] = Value;
				else if (Item == lightPositionY)
					Position[2] = Value;
				else if (Item == lightPositionZ)
					Position[1] = Value;

				lcVector3 Distance = Position - Center;

				Model->MoveSelectedObjects(Distance, Distance, false, false, true, true);
			}
			else if (Item == lightTargetX || Item == lightTargetY || Item == lightTargetZ)
			{
				lcVector3 Center = Light->mTargetPosition;
				lcVector3 Position = Center;
				float Value = lcParseValueLocalized(Editor->text());
				// Switch Y and Z axis
				if (Item == lightTargetX)
					Position[0] = Value;
				else if (Item == lightTargetY)
					Position[2] = Value;
				else if (Item == lightTargetZ)
					Position[1] = Value;

				lcVector3 Distance = Position - Center;

				Model->MoveSelectedObjects(Distance, Distance, false, false, true, true);
			}
			else if (Item == lightColorR || Item == lightColorG || Item == lightColorB)
			{
				float Value = lcParseValueLocalized(Editor->text());
				if (Item == lightColorR)
					Props.mLightColor[0] = Value;
				else if (Item == lightColorG)
					Props.mLightColor[1] = Value;
				else if (Item == lightColorB)
					Props.mLightColor[2] = Value;

				Model->UpdateLight(Light, Props, LC_LIGHT_COLOR);
			}
			else if (Item == lightFactorA || Item == lightFactorB)
			{
				float Value = lcParseValueLocalized(Editor->text());
				if (Item == lightFactorA)
					Props.mLightFactor[0] = Value;
				else if (Item == lightFactorB)
					Props.mLightFactor[1] = Value;

				Model->UpdateLight(Light, Props, LC_LIGHT_FACTOR);
			}
			else if (Item == lightSpecular)
			{
				 Props.mLightSpecular = lcParseValueLocalized(Editor->text());

				Model->UpdateLight(Light, Props, LC_LIGHT_SPECULAR);
			}
			else if (Item == lightExponent)
			{
				Props.mSpotExponent = lcParseValueLocalized(Editor->text());

				Model->UpdateLight(Light, Props, LC_LIGHT_EXPONENT);
			}
			else if (Item == lightCutoff)
			{
				Props.mSpotCutoff = lcParseValueLocalized(Editor->text());

				Model->UpdateLight(Light, Props, LC_LIGHT_CUTOFF);
			}
			else if (Item == lightSpotSize)
			{
				Props.mSpotSize = lcParseValueLocalized(Editor->text());

				Model->UpdateLight(Light, Props, LC_LIGHT_SPOT_SIZE);
			}
			else if (Item == lightName)
			{
				QString Value = Editor->text();

				Model->SetLightName(Light, Value.toLocal8Bit().data());
			}
		}
	}
/*** LPub3D Mod end ***/
}

void lcQPropertiesTree::slotSetValue(int Value)
{
	QTreeWidgetItem* Item = m_delegate->editedItem();
	lcModel* Model = gMainWindow->GetActiveModel();

	if (mWidgetMode == LC_PROPERTY_WIDGET_PIECE)
	{
		if (Item == partColor)
		{
			Model->SetSelectedPiecesColorIndex(Value);

			QPushButton *editor = (QPushButton*)m_delegate->editor();
			updateColorEditor(editor, Value);
		}
		else if (Item == partID)
		{
			QComboBox *editor = (QComboBox*)sender();

			Model->SetSelectedPiecesPieceInfo((PieceInfo*)editor->itemData(Value).value<void*>());
		}
	}
	else if (mWidgetMode == LC_PROPERTY_WIDGET_LIGHT)
	{
		lcObject* Focus = Model->GetFocusObject();

		lcLight* Light = (Focus && Focus->IsLight()) ? (lcLight*)Focus : nullptr;

		if (Light && Item == lightShape)
		{
			lcLightProps Props = Light->GetLightProps();
			Props.mLightShape = Value;
			Model->UpdateLight(Light, Props, LC_LIGHT_SHAPE);
		}
	}
}

/*** LPub3D Mod - enable lights ***/
void lcQPropertiesTree::slotSetColorValue(QColor Value)
{
	lcModel*  Model = gMainWindow->GetActiveModel();
	lcObject* Focus = Model->GetFocusObject();
	lcLight*  Light = (Focus && Focus->IsLight()) ? (lcLight*)Focus : nullptr;
	if (Light)
	{
		float r = Value.red();
		float g = Value.green();
		float b = Value.blue();
		lcVector3 Color(r/255, g/255, b/255);

		lcLightProps Props = Light->GetLightProps();
		Props.mLightColor = Color;
		Model->UpdateLight(Light, Props, LC_LIGHT_COLOR);
	}
}
/*** LPub3D Mod end ***/

void lcQPropertiesTree::slotColorButtonClicked()
{
	lcModel* Model  = gMainWindow->GetActiveModel();
	lcObject* Focus = Model->GetFocusObject();
	QWidget *parent = (QWidget*)sender();
	QWidget *popup  = nullptr;

	if (mWidgetMode == LC_PROPERTY_WIDGET_PIECE)
	{
		int ColorIndex = gDefaultColor;
		if (Focus && Focus->IsPiece())
			ColorIndex = ((lcPiece*)Focus)->mColorIndex;

		popup  = new lcQColorPickerPopup(parent, ColorIndex);
		connect(popup, SIGNAL(selected(int)), SLOT(slotSetValue(int)));

		popup->setMinimumSize(300, 200);

	}
	else if (mWidgetMode == LC_PROPERTY_WIDGET_LIGHT)
	{
		QColor qcolor;
		lcVector3 Color(0.0f, 0.0f, 0.0f); // Black
		if (Focus && Focus->IsLight())
			Color = ((lcLight*)Focus)->mLightColor;

		qcolor = QColor::fromRgbF(qreal(Color[0]), qreal(Color[1]), qreal(Color[2]));
		popup  = new QColorDialog(qcolor, parent);
		connect(popup, SIGNAL(colorSelected(QColor)), SLOT(slotSetColorValue(QColor)));
	}

	if (popup && parent) {
		const QRect desktop = QApplication::desktop()->geometry();

		QPoint pos = parent->mapToGlobal(parent->rect().bottomLeft());
		if (pos.x() < desktop.left())
			pos.setX(desktop.left());
		if (pos.y() < desktop.top())
			pos.setY(desktop.top());

		if ((pos.x() + popup->width()) > desktop.width())
			pos.setX(desktop.width() - popup->width());
		if ((pos.y() + popup->height()) > desktop.bottom())
			pos.setY(desktop.bottom() - popup->height());
		popup->move(pos);

		popup->setFocus();
		popup->show();
	}
}
/*** LPub3D Mod end ***/

QTreeWidgetItem *lcQPropertiesTree::addProperty(QTreeWidgetItem *parent, const QString& label, PropertyType propertyType)
{
	QTreeWidgetItem *newItem;

	if (parent)
		newItem = new QTreeWidgetItem(parent, QStringList(label));
	else
		newItem = new QTreeWidgetItem(this, QStringList(label));

	newItem->setData(0, PropertyTypeRole, QVariant(propertyType));
	newItem->setFlags(newItem->flags() | Qt::ItemIsEditable);
	setItemExpanded(newItem, true);

	if (propertyType == PropertyGroup)
	{
		newItem->setFirstColumnSpanned(true);
		newItem->setIcon(0, m_expandIcon);
	}

	return newItem;
}

void lcQPropertiesTree::SetEmpty()
{
	clear();

	partPosition = nullptr;
	partPositionX = nullptr;
	partPositionY = nullptr;
	partPositionZ = nullptr;
	partRotation = nullptr;
	partRotationX = nullptr;
	partRotationY = nullptr;
	partRotationZ = nullptr;
/*** LPub3D Mod - part properties ***/
	partAppearance = nullptr;
	partID = nullptr;
	partColor = nullptr;
	partFileName = nullptr;
	partModel = nullptr;
	partType = nullptr;
	partIsSubmodel = nullptr;
	partVisibility = nullptr;
	partShow = nullptr;
	partHide = nullptr;
/*** LPub3D Mod end ***/
/*** LPub3D Mod - Camera Globe ***/
	cameraGlobe = nullptr;
	cameraGlobeLatitude = nullptr;
	cameraGlobeLongitude = nullptr;
	cameraGlobeDistance = nullptr;
/*** LPub3D Mod end ***/
	cameraPosition = nullptr;
	cameraPositionX = nullptr;
	cameraPositionY = nullptr;
	cameraPositionZ = nullptr;
	cameraTarget = nullptr;
	cameraTargetX = nullptr;
	cameraTargetY = nullptr;
	cameraTargetZ = nullptr;
	cameraUp = nullptr;
	cameraUpX = nullptr;
	cameraUpY = nullptr;
	cameraUpZ = nullptr;
	cameraSettings = nullptr;
	cameraOrtho = nullptr;
	cameraFOV = nullptr;
	cameraNear = nullptr;
	cameraFar = nullptr;
	cameraName = nullptr;
/*** LPub3D Mod - Camera Globe ***/
	picture = nullptr;
	pictureModelScale = nullptr;
	pictureResolution = nullptr;
	picturePageSizeWidth = nullptr;
	picturePageSizeHeight = nullptr;
	pictureImageSizeWidth = nullptr;
	pictureImageSizeHeight = nullptr;
/*** LPub3D Mod end ***/

/*** LPub3D Mod - enable lights ***/
	lightPosition = nullptr;
	lightPositionX = nullptr;
	lightPositionY = nullptr;
	lightPositionZ = nullptr;
	lightTarget = nullptr;
	lightTargetX = nullptr;
	lightTargetY = nullptr;
	lightTargetZ = nullptr;
	lightColor = nullptr;
	lightColorIcon = nullptr;
	lightColorR = nullptr;
	lightColorG = nullptr;
	lightColorB = nullptr;
	lightProperties = nullptr;
	lightSpecular = nullptr;
	lightCutoff = nullptr;
	lightEnableCutoff = nullptr;
	lightExponent = nullptr;
	lightType = nullptr;
	lightFactorA = nullptr;
	lightFactorB = nullptr;
	lightName = nullptr;
	lightSpotSize = nullptr;
	lightShape = nullptr;
/*** LPub3D Mod end ***/

	mWidgetMode = LC_PROPERTY_WIDGET_EMPTY;
	mFocus = nullptr;
}

void lcQPropertiesTree::SetPiece(const lcArray<lcObject*>& Selection, lcObject* Focus)
{
	if (mWidgetMode != LC_PROPERTY_WIDGET_PIECE)
	{
		SetEmpty();

/*** LPub3D Mod - Switch Rotation and Position attributes ***/
		partRotation = addProperty(nullptr, tr("Rotation"), PropertyGroup);
		partRotationX = addProperty(partRotation, tr("X"), PropertyFloat);
		partRotationY = addProperty(partRotation, tr("Y"), PropertyFloat);
		partRotationZ = addProperty(partRotation, tr("Z"), PropertyFloat);

		partPosition = addProperty(nullptr, tr("Position"), PropertyGroup);
		partPositionX = addProperty(partPosition, tr("X"), PropertyFloat);
		partPositionY = addProperty(partPosition, tr("Y"), PropertyFloat);
		partPositionZ = addProperty(partPosition, tr("Z"), PropertyFloat);
/*** LPub3D Mod end ***/


/*** LPub3D Mod - Add LPub3D attributes ***/
		partAppearance = addProperty(nullptr, tr("Appearance"), PropertyGroup);
		partID = addProperty(partAppearance, tr("Part"), PropertyPart);
		partColor = addProperty(partAppearance, tr("Color"), PropertyColor);
		partFileName = addProperty(partAppearance, tr("File ID"), PropertyPart);
		partModel = addProperty(partAppearance, tr("Model"), PropertyPart);
		partType = addProperty(partAppearance, tr("Type"), PropertyPart);
		partIsSubmodel = addProperty(partAppearance, tr("Submodel"), PropertyPart);
/*** LPub3D Mod end ***/

		partVisibility = addProperty(nullptr, tr("Visible Steps"), PropertyGroup);
		partShow = addProperty(partVisibility, tr("Show"), PropertyStep);
		partHide = addProperty(partVisibility, tr("Hide"), PropertyStep);

		mWidgetMode = LC_PROPERTY_WIDGET_PIECE;
	}

	lcModel* Model = gMainWindow->GetActiveModel();
	lcPiece* Piece = (Focus && Focus->IsPiece()) ? (lcPiece*)Focus : nullptr;
	mFocus = Piece;

	lcVector3 Position;
	lcMatrix33 RelativeRotation;
	Model->GetMoveRotateTransform(Position, RelativeRotation);
/*** LPub3D Mod - Switch Y and Z axis ***/
	partPositionX->setText(1, lcFormatValueLocalized(Position[0]));
	partPositionX->setData(0, PropertyValueRole, Position[0]);
	partPositionY->setText(1, lcFormatValueLocalized(Position[2]));
	partPositionY->setData(0, PropertyValueRole, Position[2]);
	partPositionZ->setText(1, lcFormatValueLocalized(Position[1]));
	partPositionZ->setData(0, PropertyValueRole, Position[1]);
/*** LPub3D Mod end ***/

	lcVector3 Rotation;
	if (Piece)
		Rotation = lcMatrix44ToEulerAngles(Piece->mModelWorld) * LC_RTOD;
	else
		Rotation = lcVector3(0.0f, 0.0f, 0.0f);
/*** LPub3D Mod - Switch Y and Z axis ***/
	partRotationX->setText(1, lcFormatValueLocalized(Rotation[0]));
	partRotationX->setData(0, PropertyValueRole, Rotation[0]);
	partRotationY->setText(1, lcFormatValueLocalized(Rotation[2]));
	partRotationY->setData(0, PropertyValueRole, Rotation[2]);
	partRotationZ->setText(1, lcFormatValueLocalized(Rotation[1]));
	partRotationZ->setData(0, PropertyValueRole, Rotation[1]);
/*** LPub3D Mod end ***/

	lcStep Show = 0;
	lcStep Hide = 0;
	int ColorIndex = gDefaultColor;
	PieceInfo* Info = nullptr;

/*** LPub3D Mod - Add Model Properties ***/
	const lcModelProperties& ModelInfo = Model->GetProperties();
/*** LPub3D Mod end ***/

	if (Piece)
	{
		Show = Piece->GetStepShow();
		Hide = Piece->GetStepHide();
		ColorIndex = Piece->mColorIndex;
		Info = Piece->mPieceInfo;
	}
	else
	{
		bool FirstPiece = true;

		for (int ObjectIdx = 0; ObjectIdx < Selection.GetSize(); ObjectIdx++)
		{
			lcObject* Object = Selection[ObjectIdx];

			if (Object->GetType() != LC_OBJECT_PIECE)
				continue;

			lcPiece* SelectedPiece = (lcPiece*)Object;

			if (FirstPiece)
			{
				Show = SelectedPiece->GetStepShow();
				Hide = SelectedPiece->GetStepHide();
				ColorIndex = SelectedPiece->mColorIndex;
				Info = SelectedPiece->mPieceInfo;

				FirstPiece = false;
			}
			else
			{
				if (SelectedPiece->GetStepShow() != Show)
					Show = 0;

				if (SelectedPiece->GetStepHide() != Hide)
					Hide = 0;

				if (SelectedPiece->mColorIndex != ColorIndex)
					ColorIndex = gDefaultColor;

				if (SelectedPiece->mPieceInfo != Info)
					Info = nullptr;
			}
		}
	}

	QString text = Info ? Info->m_strDescription : QString();
	partID->setText(1, text);
	partID->setToolTip(1, text);
	partID->setData(0, PropertyValueRole, qVariantFromValue((void*)Info));

	QImage img(16, 16, QImage::Format_ARGB32);
	img.fill(0);

	lcColor* color = &gColorList[ColorIndex];
	QPainter painter(&img);
	painter.setCompositionMode(QPainter::CompositionMode_Source);
	painter.setPen(Qt::darkGray);
	painter.setBrush(QColor::fromRgbF(color->Value[0], color->Value[1], color->Value[2]));
	painter.drawRect(0, 0, img.width() - 1, img.height() - 1);
	painter.end();

	partColor->setIcon(1, QIcon(QPixmap::fromImage(img)));
	partColor->setText(1, color->Name);
	partColor->setData(0, PropertyValueRole, ColorIndex);

	partFileName->setText(1, Info ? Info->mFileName : QString());
	partFileName->setData(0, PropertyValueRole, qVariantFromValue((void*)Info));

	partModel->setText(1, ModelInfo.mName);
	partModel->setData(0, PropertyValueRole, qVariantFromValue((void*)Info));

	partType->setText(1, Info ? Info->mZipFileType == LC_ZIPFILE_OFFICIAL ? QString("Official Part") : QString("Unofficial Part") : QString());
	partType->setData(0, PropertyValueRole, qVariantFromValue((void*)Info));

	partIsSubmodel->setText(1, Info ? Info->IsModel() ? QString("Yes") : QString("No") : QString());
	partIsSubmodel->setData(0, PropertyValueRole, qVariantFromValue((void*)Info));

	partShow->setText(1, QString::number(Show));
	partShow->setData(0, PropertyValueRole, Show);
	partHide->setText(1, Hide == LC_STEP_MAX ? QString() : QString::number(Hide));
	partHide->setData(0, PropertyValueRole, Hide);

	FirstHit = false;
}

void lcQPropertiesTree::SetCamera(lcObject* Focus)
{
	if (mWidgetMode != LC_PROPERTY_WIDGET_CAMERA)
	{
		SetEmpty();

/*** LPub3D Mod - Camera Globe ***/
		cameraGlobe = addProperty(nullptr, tr("Camera Globe"), PropertyGroup);
		cameraGlobeLatitude = addProperty(cameraGlobe, tr("Latitude"), PropertyFloatCameraAngle);
		cameraGlobeLongitude = addProperty(cameraGlobe, tr("Longitude"), PropertyFloatCameraAngle);
		cameraGlobeDistance = addProperty(cameraGlobe, tr("Distance"), PropertyIntReadOnly);
/*** LPub3D Mod end ***/

		cameraPosition = addProperty(nullptr, tr("Position"), PropertyGroup);
		cameraPositionX = addProperty(cameraPosition, tr("X"), PropertyFloat);
		cameraPositionY = addProperty(cameraPosition, tr("Y"), PropertyFloat);
		cameraPositionZ = addProperty(cameraPosition, tr("Z"), PropertyFloat);

		cameraTarget = addProperty(nullptr, tr("Target"), PropertyGroup);
		cameraTargetX = addProperty(cameraTarget, tr("X"), PropertyFloatTarget);
		cameraTargetY = addProperty(cameraTarget, tr("Y"), PropertyFloatTarget);
		cameraTargetZ = addProperty(cameraTarget, tr("Z"), PropertyFloatTarget);

		cameraUp = addProperty(nullptr, tr("Up"), PropertyGroup);
		cameraUpX = addProperty(cameraUp, tr("X"), PropertyFloat);
		cameraUpY = addProperty(cameraUp, tr("Y"), PropertyFloat);
		cameraUpZ = addProperty(cameraUp, tr("Z"), PropertyFloat);

/*** LPub3D Mod - Rename Camera Settings ***/
		cameraSettings = addProperty(nullptr, tr("Viewer Settings"), PropertyGroup);
/*** LPub3D Mod end ***/
		cameraOrtho = addProperty(cameraSettings, tr("Orthographic"), PropertyBool);
		cameraFOV = addProperty(cameraSettings, tr("Field of View"), PropertyFloat);
		cameraNear = addProperty(cameraSettings, tr("Near"), PropertyFloat);
		cameraFar = addProperty(cameraSettings, tr("Far"), PropertyFloat);
		cameraName = addProperty(cameraSettings, tr("Name"), PropertyString);

/*** LPub3D Mod - Camera Globe ***/
		picture = addProperty(nullptr, tr("Picture"), PropertyGroup);
		pictureModelScale = addProperty(picture, tr("Scale"), PropertyFloatReadOnly);
		pictureResolution = addProperty(picture, tr("Resolution"), PropertyFloatReadOnly);
		picturePageSizeWidth = addProperty(picture, tr("Page Width"), PropertyFloatReadOnly);
		picturePageSizeHeight = addProperty(picture, tr("Page Height"), PropertyFloatReadOnly);
		pictureImageSizeWidth = addProperty(picture, tr("Image Width"), PropertyFloat);
		pictureImageSizeHeight = addProperty(picture, tr("Image Height"), PropertyFloat);
/*** LPub3D Mod end ***/

		mWidgetMode = LC_PROPERTY_WIDGET_CAMERA;
	}

	lcCamera* Camera = (Focus && Focus->IsCamera()) ? (lcCamera*)Focus : nullptr;
	mFocus = Camera;

	lcVector3 Position(0.0f, 0.0f, 0.0f);
	lcVector3 Target(0.0f, 0.0f, 0.0f);
	lcVector3 UpVector(0.0f, 0.0f, 0.0f);
/*** LPub3D Mod - Camera Globe ***/
	float Latitude      = 0.0f;
	float Longitude     = 0.0f;
	float ModelScale    = 0.0f;
	float Distance      = 0.0f;
	float Resolution    = 0.0f;
	int PageSizeWidth   = 0;
	int PageSizeHeight  = 0;
	int ImageSizeWidth  = 0;
	int ImageSizeHeight = 0;
/*** LPub3D Mod end ***/
	bool Ortho = false;
	float FoV = 60.0f;
	float ZNear = 1.0f;
	float ZFar = 100.0f;
	const char* Name = "";
/*** LPub3D Mod - Camera Globe ***/
	float LPub3D_FoV   = FoV;
	float LPub3D_ZNear = ZNear;
	float LPub3D_ZFar  = ZFar;
/*** LPub3D Mod end ***/
	if (Camera)
	{
/*** LPub3D Mod - Camera Globe ***/
		Camera->GetAngles(Latitude,Longitude,Distance);

		Position = normalizeDegrees(Camera->mPosition);
		Target   = normalizeDegrees(Camera->mTargetPosition);
		UpVector = normalizeDegrees(Camera->mUpVector);
/*** LPub3D Mod end ***/

		Ortho = Camera->IsOrtho();
		FoV = Camera->m_fovy;
		ZNear = Camera->m_zNear;
		ZFar = Camera->m_zFar;
		Name = Camera->GetName();
/*** LPub3D Mod - Camera Globe ***/
		if (Name && !Name[0])
			Name = "Default";
		ModelScale      = Camera->GetScale();
		Resolution      = lcGetActiveProject()->GetResolution();
		PageSizeWidth   = lcGetActiveProject()->GetPageWidth();
		PageSizeHeight  = lcGetActiveProject()->GetPageHeight();
		ImageSizeWidth  = lcGetActiveProject()->GetImageWidth();
		ImageSizeHeight = lcGetActiveProject()->GetImageHeight();
		LPub3D_FoV      = FoV   + CAMERA_FOV_DEFAULT   - CAMERA_FOV_NATIVE_DEFAULT;
		LPub3D_ZNear    = ZNear + CAMERA_ZNEAR_DEFAULT - CAMERA_ZNEAR_NATIVE_DEFAULT;
		LPub3D_ZFar     = ZFar  + CAMERA_ZFAR_DEFAULT  - CAMERA_ZFAR_NATIVE_DEFAULT;
/*** LPub3D Mod end ***/
	}

/*** LPub3D Mod - Camera Globe ***/
	cameraGlobeLatitude->setText(1, lcFormatValueLocalized(Latitude));
	cameraGlobeLatitude->setData(0, PropertyValueRole, Latitude);
	cameraGlobeLongitude->setText(1, lcFormatValueLocalized(Longitude));
	cameraGlobeLongitude->setData(0, PropertyValueRole, Longitude);
	cameraGlobeDistance->setText(1, lcFormatValueLocalized(qRound(Distance)));
	cameraGlobeDistance->setData(0, PropertyValueRole, qRound(Distance));
/*** LPub3D Mod end ***/

/*** LPub3D Mod - Camera Globe, Switch Y and Z axis ***/
	cameraPositionX->setText(1, lcFormatValueLocalized(Position[0]));
	cameraPositionX->setData(0, PropertyValueRole, Position[0]);
	cameraPositionY->setText(1, lcFormatValueLocalized(Position[2]));
	cameraPositionY->setData(0, PropertyValueRole, Position[2]);
	cameraPositionZ->setText(1, lcFormatValueLocalized(Position[1]));
	cameraPositionZ->setData(0, PropertyValueRole, Position[1]);
/*** LPub3D Mod end ***/

/*** LPub3D Mod - Camera Globe, Switch Y and Z axis ***/
	cameraTargetX->setText(1, lcFormatValueLocalized(Target[0]));
	cameraTargetX->setData(0, PropertyValueRole, Target[0]);
	cameraTargetY->setText(1, lcFormatValueLocalized(Target[2]));
	cameraTargetY->setData(0, PropertyValueRole, Target[2]);
	cameraTargetZ->setText(1, lcFormatValueLocalized(Target[1]));
	cameraTargetZ->setData(0, PropertyValueRole, Target[1]);
/*** LPub3D Mod end ***/

/*** LPub3D Mod - Camera Globe, Switch Y and Z axis ***/
	cameraUpX->setText(1, lcFormatValueLocalized(UpVector[0]));
	cameraUpX->setData(0, PropertyValueRole, UpVector[0]);
	cameraUpY->setText(1, lcFormatValueLocalized(UpVector[2]));
	cameraUpY->setData(0, PropertyValueRole, UpVector[2]);
	cameraUpZ->setText(1, lcFormatValueLocalized(UpVector[1]));
	cameraUpZ->setData(0, PropertyValueRole, UpVector[1]);
/*** LPub3D Mod end ***/

	cameraOrtho->setText(1, Ortho ? "True" : "False");
	cameraOrtho->setData(0, PropertyValueRole, Ortho);
	cameraFOV->setText(1, lcFormatValueLocalized(FoV));
/*** LPub3D Mod - Camera Globe ***/
	cameraFOV->setToolTip(1,QString("LPub3D Setting: %1").arg(double(LPub3D_FoV)));
/*** LPub3D Mod end ***/
	cameraFOV->setData(0, PropertyValueRole, FoV);
	cameraNear->setText(1, lcFormatValueLocalized(ZNear));
/*** LPub3D Mod - Camera Globe ***/
	cameraNear->setToolTip(1,QString("LPub3D Setting: %1").arg(int(LPub3D_ZNear)));
/*** LPub3D Mod end ***/
	cameraNear->setData(0, PropertyValueRole, ZNear);
	cameraFar->setText(1, lcFormatValueLocalized(ZFar));
/*** LPub3D Mod - Camera Globe ***/
	cameraFar->setToolTip(1,QString("LPub3D Setting: %1").arg(int(LPub3D_ZFar)));
/*** LPub3D Mod end ***/
	cameraFar->setData(0, PropertyValueRole, ZFar);

	cameraName->setText(1, Name);
	cameraName->setData(0, PropertyValueRole, qVariantFromValue((void*)Name));

/*** LPub3D Mod - Camera Globe ***/
	pictureModelScale->setText(1, lcFormatValueLocalized(ModelScale));
	pictureModelScale->setData(0, PropertyValueRole, ModelScale);
	pictureResolution->setText(1, lcFormatValueLocalized(Resolution));
	pictureResolution->setData(0, PropertyValueRole, Resolution);
	picturePageSizeWidth->setText(1, lcFormatValueLocalized(PageSizeWidth));
	picturePageSizeWidth->setData(0, PropertyValueRole, PageSizeWidth);
	picturePageSizeHeight->setText(1, lcFormatValueLocalized(PageSizeHeight));
	picturePageSizeHeight->setData(0, PropertyValueRole, PageSizeHeight);
	pictureImageSizeWidth->setText(1, lcFormatValueLocalized(ImageSizeWidth));
	pictureImageSizeWidth->setData(0, PropertyValueRole, ImageSizeWidth);
	pictureImageSizeHeight->setText(1, lcFormatValueLocalized(ImageSizeHeight));
	pictureImageSizeHeight->setData(0, PropertyValueRole, ImageSizeHeight);
/*** LPub3D Mod end ***/

}

void lcQPropertiesTree::SetLight(lcObject* Focus)
{
/*** LPub3D Mod - enable lights ***/
	lcLight* Light = (Focus && Focus->IsLight()) ? (lcLight*)Focus : nullptr;

	const char* Name            = "Light";
	const char* FactorALabel    = "FactorA";
	const char* ExponentLabel   = "Exponent";
	QString Type                = "";
	QString Shape               = "";
	QString FactorAToolTip      = "";
	QString FactorBToolTip      = "";
	int   LightIndex            = LC_UNDEFINED_LIGHT;
	int   ShapeIndex            = LC_UNDEFINED_SHAPE;
	float SpotSize              = 0.0f;
	float Specular              = 0.0f;
	float Cutoff                = 0.0f;
	float Exponent              = 0.0f;
	bool EnableCutoff           = false;
	PropertyType TargetProperty = PropertyFloat;
	lcVector3 Position(0.0f, 0.0f, 0.0f);
	lcVector3 Target(0.0f, 0.0f, 0.0f);
	lcVector3 Color(0.0f, 0.0f, 0.0f);
	lcVector2 Factor(0.0f, 0.0f);

	if (Light)
	{
		Name = Light->GetName();

		Position       = normalizeDegrees(Light->mPosition);
		Target         = normalizeDegrees(Light->mTargetPosition);
		Color          = Light->mLightColor;
		Factor         = Light->mLightFactor;
		LightIndex     = Light->mLightType;
		switch(LightIndex){
		case LC_POINTLIGHT:
			Type = "Point";
			FactorALabel = "Radius (m)";
			FactorAToolTip = "Shadow soft size - Light size for shadow sampling";
			break;
		case LC_SUNLIGHT:
			Type = "Sun";
			FactorALabel = "Angle (°)";
			FactorAToolTip = "Angular diamater of the sun as seen from the Earth";
			break;
		case LC_SPOTLIGHT:
			Type = "Spot";
			FactorALabel = "Radius (m)";
			FactorAToolTip = "Shadow soft size - Light size for shadow sampling";
			FactorBToolTip = "Shadow blend - The softness of the spotlight edge";
			break;
		case LC_AREALIGHT:
			Type = "Area";
			FactorALabel = "Width (X)";
			FactorAToolTip = "Size of the area of the area light. X direction size for rectangular shapes";
			FactorBToolTip = "Size of the area of the area light. Y direction size for rectangular shapes";
			break;
		default:
			Type = "Undefined";
			FactorALabel = "FactorA";
			break;
		}
		ShapeIndex     = Light->mLightShape;
		switch(ShapeIndex){
		case LC_LIGHT_SHAPE_SQUARE:
			Shape = "Square";
			break;
		case LC_LIGHT_SHAPE_DISK:
			Shape = "Disk";
			break;
		case LC_LIGHT_SHAPE_RECTANGLE:
			Shape = "Rectangle";
			break;
		case LC_LIGHT_SHAPE_ELLIPSE:
			Shape = "Ellipse";
			break;
		default:
			break;
		}
		Specular       = Light->mLightSpecular;
		Exponent       = Light->mSpotExponent;
		ExponentLabel  = LightIndex ? LightIndex == LC_SUNLIGHT ? "Strength" : "Power" : "Exponent";
		Cutoff         = Light->mSpotCutoff;
		EnableCutoff   = Light->mEnableCutoff;
		TargetProperty = Light->mLightType > LC_POINTLIGHT ? PropertyFloat : PropertyFloatReadOnly;
		SpotSize       = Light->mSpotSize;
	}

	if (mWidgetMode != LC_PROPERTY_WIDGET_LIGHT ||
		mLightType  != LightIndex ||
		mLightShape != ShapeIndex)
	{
		SetEmpty();
		// Position
		lightPosition  = addProperty(nullptr, tr("Position"), PropertyGroup);
		lightPositionX = addProperty(lightPosition, tr("X"), PropertyFloat);
		lightPositionY = addProperty(lightPosition, tr("Y"), PropertyFloat);
		lightPositionZ = addProperty(lightPosition, tr("Z"), PropertyFloat);
		// Target Position
		if (LightIndex != LC_POINTLIGHT) {
			lightTarget  = addProperty(nullptr, tr("Target"), PropertyGroup);
			lightTargetX = addProperty(lightTarget, tr("X"), TargetProperty);
			lightTargetY = addProperty(lightTarget, tr("Y"), TargetProperty);
			lightTargetZ = addProperty(lightTarget, tr("Z"), TargetProperty);
		}
		// Ambient Colour
		lightColor  = addProperty(nullptr, tr("Color"), PropertyGroup);
		lightColorIcon = addProperty(lightColor, tr("Color"), PropertyLightColor);
		lightColorR = addProperty(lightColor, tr("Red"), PropertyFloat);
		lightColorG = addProperty(lightColor, tr("Green"), PropertyFloat);
		lightColorB = addProperty(lightColor, tr("Blue"), PropertyFloat);
		// Properties
		lightProperties   = addProperty(nullptr, tr("Properties"), PropertyGroup);
		lightType         = addProperty(lightProperties, tr("Type"), PropertyStringReadOnly);
		lightExponent     = addProperty(lightProperties, tr(ExponentLabel), PropertyFloat);
		lightFactorA      = addProperty(lightProperties, tr(FactorALabel), PropertyFloat);
		if (LightIndex == LC_AREALIGHT) {
			if (ShapeIndex == LC_LIGHT_SHAPE_RECTANGLE || ShapeIndex == LC_LIGHT_SHAPE_ELLIPSE)
				lightFactorB   = addProperty(lightProperties, tr("Height (Y)"), PropertyFloat);
			lightShape         = addProperty(lightProperties, tr("Shape"), PropertyLightShape);
		} else if (LightIndex == LC_SPOTLIGHT) {
			lightFactorB   = addProperty(lightProperties, tr("Spot Blend"), PropertyFloat);
			lightSpotSize      = addProperty(lightProperties, tr("Spot Size (°)"), PropertyFloatLightSpotSize);
		}
		if (LightIndex != LC_SUNLIGHT) {
			lightEnableCutoff = addProperty(lightProperties, tr("Cutoff"), PropertyBool);
			lightCutoff       = addProperty(lightProperties, tr("Cutoff Distance"), PropertyFloat);
		}
		lightSpecular     = addProperty(lightProperties, tr("Specular"), PropertyFloat);
		lightName         = addProperty(lightProperties, tr("Name"), PropertyString);

		mWidgetMode = LC_PROPERTY_WIDGET_LIGHT;
		mLightType  = LightIndex;
		mLightShape = ShapeIndex;
	}

	mFocus = Light;

	lightPositionX->setText(1, lcFormatValueLocalized(Position[0]));
	lightPositionX->setData(0, PropertyValueRole, Position[0]);
	lightPositionY->setText(1, lcFormatValueLocalized(Position[2]));
	lightPositionY->setData(0, PropertyValueRole, Position[2]);
	lightPositionZ->setText(1, lcFormatValueLocalized(Position[1]));
	lightPositionZ->setData(0, PropertyValueRole, Position[1]);

	if (LightIndex != LC_POINTLIGHT) {
		lightTargetX->setText(1, lcFormatValueLocalized(Target[0]));
		lightTargetX->setData(0, PropertyValueRole, Target[0]);
		lightTargetY->setText(1, lcFormatValueLocalized(Target[2]));
		lightTargetY->setData(0, PropertyValueRole, Target[2]);
		lightTargetZ->setText(1, lcFormatValueLocalized(Target[1]));
		lightTargetZ->setData(0, PropertyValueRole, Target[1]);
	}

	QImage img(16, 16, QImage::Format_ARGB32);
	img.fill(0);

	lcVector3 _Color(Color[0]*255, Color[1]*255, Color[2]*255);
	QColor qcolor = QColor::fromRgb(int(_Color[0]), int(_Color[1]), int(_Color[2]));
	QString description = QString("Hex: %1").arg(qcolor.name());

	QPainter painter(&img);
	painter.setCompositionMode(QPainter::CompositionMode_Source);
	painter.setPen(Qt::darkGray);
	painter.setBrush(qcolor);
	painter.drawRect(0, 0, img.width() - 1, img.height() - 1);
	painter.end();

	lightColorIcon->setIcon(1, QIcon(QPixmap::fromImage(img)));
	lightColorIcon->setText(1, description);
	lightColorIcon->setData(0, PropertyValueRole, qcolor);

	lightColorR->setText(1, lcFormatValueLocalized(Color[0]));
	lightColorR->setData(0, PropertyValueRole, Color[0]);
	lightColorR->setToolTip(1, tr("Red color component - use .0 - 1 format"));
	lightColorG->setText(1, lcFormatValueLocalized(Color[1]));
	lightColorG->setData(0, PropertyValueRole, Color[1]);
	lightColorG->setToolTip(1, tr("Green color component - use .0 - 1 format"));
	lightColorB->setText(1, lcFormatValueLocalized(Color[2]));
	lightColorB->setData(0, PropertyValueRole, Color[2]);
	lightColorB->setToolTip(1, tr("Blue color component - use  format"));

	lightType->setText(1, Type);
	lightType->setData(0, PropertyValueRole, Type);

	lightExponent->setText(1, lcFormatValueLocalized(Exponent));
	lightExponent->setData(0, PropertyValueRole, Exponent);
	lightExponent->setToolTip(1, tr("Intensity of the light in Watts."));

	lightFactorA->setText(1, lcFormatValueLocalized(Factor[0]));
	lightFactorA->setData(0, PropertyValueRole, Factor[0]);
	lightFactorA->setToolTip(1, tr(FactorAToolTip.toLatin1()));

	if (LightIndex == LC_AREALIGHT) {
		if (ShapeIndex == LC_LIGHT_SHAPE_RECTANGLE || ShapeIndex == LC_LIGHT_SHAPE_ELLIPSE) {
			lightFactorB->setText(1, lcFormatValueLocalized(Factor[1]));
			lightFactorB->setData(0, PropertyValueRole, Factor[1]);
			lightFactorB->setToolTip(1, tr(FactorBToolTip.toLatin1()));
		}

		lightShape->setText(1, Shape);
		lightShape->setData(0, PropertyValueRole, ShapeIndex);
		lightShape->setToolTip(1, tr("Shape of the arealight."));

	} else if (LightIndex == LC_SPOTLIGHT) {
		lightFactorB->setText(1, lcFormatValueLocalized(Factor[1]));
		lightFactorB->setData(0, PropertyValueRole, Factor[1]);
		lightFactorB->setToolTip(1, tr(FactorBToolTip.toLatin1()));

		lightSpotSize->setText(1, lcFormatValueLocalized(SpotSize));
		lightSpotSize->setData(0, PropertyValueRole, SpotSize);
		lightSpotSize->setToolTip(1, tr("Angle of the spotlight beam."));
	}

	if (LightIndex != LC_SUNLIGHT) {
		lightEnableCutoff->setText(1, EnableCutoff ? "True" : "False");
		lightEnableCutoff->setData(0, PropertyValueRole, EnableCutoff);

		lightCutoff->setText(1, lcFormatValueLocalized(Cutoff));
		lightCutoff->setData(0, PropertyValueRole, Cutoff);
		lightCutoff->setToolTip(1, tr("Distance at which the light influence will be set to 0."));
	}

	lightSpecular->setText(1, lcFormatValueLocalized(Specular));
	lightSpecular->setData(0, PropertyValueRole, Specular);
	lightSpecular->setToolTip(1, tr("Specular reflection multiplier factor."));

	lightName->setText(1, Name);
	lightName->setData(0, PropertyValueRole, qVariantFromValue((void*)Name));
/*** LPub3D Mod end ***/
}

void lcQPropertiesTree::SetMultiple()
{
	if (mWidgetMode != LC_PROPERTY_WIDGET_MULTIPLE)
	{
		SetEmpty();

		addProperty(nullptr, tr("Multiple Objects Selected"), PropertyGroup);

		mWidgetMode = LC_PROPERTY_WIDGET_MULTIPLE;
	}

	mFocus = nullptr;
}

bool lcQPropertiesTree::lastColumn(int column) const
{
	return header()->visualIndex(column) == columnCount() - 1;
}
