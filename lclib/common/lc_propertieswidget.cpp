#include "lc_global.h"
#include "lc_propertieswidget.h"
#include "lc_keyframewidget.h"
#include "object.h"
#include "piece.h"
#include "camera.h"
#include "light.h"
#include "pieceinf.h"
#include "lc_mainwindow.h"
#include "lc_collapsiblewidget.h"
#include "lc_colorpicker.h"
#include "lc_qutils.h"

/*** LPub3D Mod - Camera Globe ***/
#include "lpub_qtcompat.h"
#include "project.h"
/*** LPub3D Mod end ***/

/*** LPub3D Mod - Axis overlay ***/
enum axis
{
	X = 0,
	Y = 1,
	Z = 2
};
/*** LPub3D Mod end ***/

lcPropertiesWidget::lcPropertiesWidget(QWidget* Parent)
	: QWidget(Parent)
{
	CreateWidgets();
	SetLayoutMode(LayoutMode::Empty);
}

lcObjectPropertyId lcPropertiesWidget::GetEditorWidgetPropertyId(QWidget* Widget) const
{
	if (!Widget)
		return lcObjectPropertyId::Count;

	for (size_t Index = 0; Index < mPropertyWidgets.size(); Index++)
		if (mPropertyWidgets[Index].Editor == Widget)
			return static_cast<lcObjectPropertyId>(Index);

	return lcObjectPropertyId::Count;
}

lcObjectPropertyId lcPropertiesWidget::GetKeyFrameWidgetPropertyId(QWidget* Widget) const
{
	if (!Widget)
		return lcObjectPropertyId::Count;

	for (size_t Index = 0; Index < mPropertyWidgets.size(); Index++)
		if (mPropertyWidgets[Index].KeyFrame == Widget)
			return static_cast<lcObjectPropertyId>(Index);

	return lcObjectPropertyId::Count;
}

void lcPropertiesWidget::CategoryStateChanged(bool Expanded)
{
	QObject* Button = sender();

	for (CategoryWidgets& Category : mCategoryWidgets)
	{
		if (Category.Button == Button)
		{
			SetCategoryWidgetsVisible(Category, Expanded);
			break;
		}
	}
}

void lcPropertiesWidget::AddCategory(CategoryIndex Index, const QString& Title)
{
	mCurrentCategory = &mCategoryWidgets[static_cast<int>(Index)];
	mCurrentCategory->Category = Index;

	lcCollapsibleWidgetButton* CategoryButton = new lcCollapsibleWidgetButton(Title);

	mLayout->addWidget(CategoryButton, mLayoutRow, 0, 1, -1);
	mCurrentCategory->Button = CategoryButton;

	connect(CategoryButton, &lcCollapsibleWidgetButton::StateChanged, this, &lcPropertiesWidget::CategoryStateChanged);

	mLayoutRow++;
}

void lcPropertiesWidget::AddSpacing()
{
	mLayout->setRowMinimumHeight(mLayoutRow, 5);
	mCurrentCategory->SpacingRows.push_back(mLayoutRow);
	mLayoutRow++;
}

void lcPropertiesWidget::AddLabel(lcObjectPropertyId PropertyId, const QString& Text, const QString& ToolTip)
{
	QLabel* Label = new QLabel(Text, this);
	Label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	Label->setToolTip(ToolTip);

	mLayout->addWidget(Label, mLayoutRow, 1);

	mPropertyWidgets[static_cast<int>(PropertyId)].Label = Label;
}

void lcPropertiesWidget::KeyFrameChanged()
{
	QCheckBox* Widget = qobject_cast<QCheckBox*>(sender());
	lcObjectPropertyId PropertyId = GetKeyFrameWidgetPropertyId(Widget);

	if (PropertyId == lcObjectPropertyId::Count)
		return;

	lcModel* Model = gMainWindow->GetActiveModel();

	if (!Model)
		return;

	if (mFocusObject)
		Model->SetObjectsKeyFrame({ mFocusObject }, PropertyId, Widget->isChecked());
	else
		Model->SetObjectsKeyFrame(mSelection, PropertyId, Widget->isChecked());
}

void lcPropertiesWidget::UpdateKeyFrameWidget(lcObjectPropertyId PropertyId)
{
	lcKeyFrameWidget* Widget = mPropertyWidgets[static_cast<int>(PropertyId)].KeyFrame;

	if (Widget)
	{
		QSignalBlocker Blocker(Widget);
		lcModel* Model = gMainWindow->GetActiveModel();

		if (Model)
		{
			const lcStep Step = Model->GetCurrentStep();

			if (mFocusObject)
				Widget->setChecked(mFocusObject->HasKeyFrame(PropertyId, Step));
			else
			{
				int KeyFrameCount = 0, NonKeyFrameCount = 0;

				for (const lcObject* Object : mSelection)
				{
					if (Object->HasKeyFrame(PropertyId, Step))
						KeyFrameCount++;
					else
						NonKeyFrameCount++;
				}

				if (KeyFrameCount && NonKeyFrameCount)
					Widget->setCheckState(Qt::PartiallyChecked);
				else
					Widget->setCheckState(KeyFrameCount != 0 ? Qt::Checked : Qt::Unchecked);
			}
		}
	}
}

void lcPropertiesWidget::AddKeyFrameWidget(lcObjectPropertyId PropertyId)
{
	lcKeyFrameWidget* Widget = new lcKeyFrameWidget(this);
	Widget->setToolTip(tr("Toggle Key Frame"));

	connect(Widget, &QCheckBox::stateChanged, this, &lcPropertiesWidget::KeyFrameChanged);

	mLayout->addWidget(Widget, mLayoutRow, 3);

	mPropertyWidgets[static_cast<int>(PropertyId)].KeyFrame = Widget;
}

std::pair<QVariant, bool> lcPropertiesWidget::GetUpdateValue(lcObjectPropertyId PropertyId)
{
	QVariant Value;
	bool Partial = false;

	if (mFocusObject)
		Value = mFocusObject->GetPropertyValue(PropertyId);
	else
	{
		bool First = true;

		for (const lcObject* Object : mSelection)
		{
			const QVariant ObjectValue = Object->GetPropertyValue(PropertyId);

			if (First)
			{
				Value = ObjectValue;
				First = false;
			}
			else if (Value != ObjectValue)
			{
				Partial = true;
				break;
			}
		}
	}

	return { Value, Partial };
}

void lcPropertiesWidget::BoolChanged()
{
	QCheckBox* Widget = qobject_cast<QCheckBox*>(sender());
	lcObjectPropertyId PropertyId = GetEditorWidgetPropertyId(Widget);

	if (PropertyId == lcObjectPropertyId::Count)
		return;

	lcModel* Model = gMainWindow->GetActiveModel();

	if (!Model)
		return;

	const bool Value = Widget->isChecked();
	Model->SetObjectsProperty(mFocusObject ? std::vector<lcObject*>{ mFocusObject } : mSelection, PropertyId, Value);
}

void lcPropertiesWidget::UpdateBool(lcObjectPropertyId PropertyId)
{
	QCheckBox* CheckBox = qobject_cast<QCheckBox*>(mPropertyWidgets[static_cast<int>(PropertyId)].Editor);

	if (!CheckBox)
		return;

	QSignalBlocker Blocker(CheckBox);
	QVariant Value;
	bool Partial;

	std::tie(Value, Partial) = GetUpdateValue(PropertyId);

	if (Partial)
		CheckBox->setCheckState(Qt::PartiallyChecked);
	else
		CheckBox->setCheckState(Value.toBool() ? Qt::Checked : Qt::Unchecked);

	UpdateKeyFrameWidget(PropertyId);
}

void lcPropertiesWidget::AddBoolProperty(lcObjectPropertyId PropertyId, const QString& Text, const QString& ToolTip, bool SupportsKeyFrames)
{
	AddLabel(PropertyId, Text, ToolTip);

	QCheckBox* Widget = new QCheckBox(this);
	Widget->setToolTip(ToolTip);

	connect(Widget, &QCheckBox::stateChanged, this, &lcPropertiesWidget::BoolChanged);

	mLayout->addWidget(Widget, mLayoutRow, 2);

	mCurrentCategory->Properties.push_back(PropertyId);
	mPropertyWidgets[static_cast<int>(PropertyId)].Category = mCurrentCategory->Category;
	mPropertyWidgets[static_cast<int>(PropertyId)].Editor = Widget;

	if (SupportsKeyFrames)
		AddKeyFrameWidget(PropertyId);

	mLayoutRow++;
}

void lcPropertiesWidget::FloatChanged()
{
	QLineEdit* Widget = qobject_cast<QLineEdit*>(sender());
	lcObjectPropertyId PropertyId = GetEditorWidgetPropertyId(Widget);

	if (PropertyId == lcObjectPropertyId::Count)
		return;

	lcModel* Model = gMainWindow->GetActiveModel();

	if (!Model)
		return;

	lcPiece* Piece = dynamic_cast<lcPiece*>(mFocusObject);
	lcCamera* Camera = dynamic_cast<lcCamera*>(mFocusObject);
	lcLight* Light = dynamic_cast<lcLight*>(mFocusObject);
	float Value = lcParseValueLocalized(Widget->text());

	// todo: mouse drag

	if (PropertyId == lcObjectPropertyId::ObjectPositionX || PropertyId == lcObjectPropertyId::ObjectPositionY || PropertyId == lcObjectPropertyId::ObjectPositionZ)
	{
		lcVector3 Center;
		lcMatrix33 RelativeRotation;
		Model->GetMoveRotateTransform(Center, RelativeRotation);
		lcVector3 Position = Center;

/*** LPub3D Mod - Camera Globe, Switch Y and Z axis with -Y(LC -Z) in the up direction ***/
		if (PropertyId == lcObjectPropertyId::ObjectPositionX)
			Position[X] = Value;
		else if (PropertyId == lcObjectPropertyId::ObjectPositionY)
			Position[Z] = -Value;
		else if (PropertyId == lcObjectPropertyId::ObjectPositionZ)
			Position[Y] = Value;
/*** LPub3D Mod end ***/

		lcVector3 Distance = Position - Center;

		Model->MoveSelectedObjects(Distance, Distance, false, true, true, true);
	}
	else if (PropertyId == lcObjectPropertyId::ObjectRotationX || PropertyId == lcObjectPropertyId::ObjectRotationY || PropertyId == lcObjectPropertyId::ObjectRotationZ)
	{
		lcVector3 InitialRotation(0.0f, 0.0f, 0.0f);

		if (Piece)
			InitialRotation = lcMatrix44ToEulerAngles(Piece->mModelWorld) * LC_RTOD;
		else if (Light)
			InitialRotation = lcMatrix44ToEulerAngles(Light->GetWorldMatrix()) * LC_RTOD;

		lcVector3 Rotation = InitialRotation;

/*** LPub3D Mod - Camera Globe, Switch Y and Z axis with -Y(LC -Z) in the up direction ***/
		if (PropertyId == lcObjectPropertyId::ObjectRotationX)
			Rotation[X] = Value;
		else if (PropertyId == lcObjectPropertyId::ObjectRotationY)
			Rotation[Z] = -Value;
		else if (PropertyId == lcObjectPropertyId::ObjectRotationZ)
			Rotation[Y] = Value;
/*** LPub3D Mod end ***/

		Model->RotateSelectedObjects(Rotation - InitialRotation, true, false, true, true);
	}
	else if ( Piece || Light )
	{
		Model->SetObjectsProperty(mFocusObject ? std::vector<lcObject*>{ mFocusObject } : mSelection, PropertyId, Value);
	}

	if (PropertyId == lcObjectPropertyId::CameraPositionX || PropertyId == lcObjectPropertyId::CameraPositionY || PropertyId == lcObjectPropertyId::CameraPositionZ)
	{
		lcVector3 Center = Camera->mPosition;
		lcVector3 Position = Center;

/*** LPub3D Mod - Camera Globe, Switch Y and Z axis with -Y(LC -Z) in the up direction ***/
		if (PropertyId == lcObjectPropertyId::CameraPositionX)
			Position[X] = Value;
		else if (PropertyId == lcObjectPropertyId::CameraPositionY)
			Position[Z] = -Value;
		else if (PropertyId == lcObjectPropertyId::CameraPositionZ)
			Position[Y] = Value;
/*** LPub3D Mod end ***/

		lcVector3 Distance = Position - Center;

/*** LPub3D Mod - Camera Globe  ***/
		if (Camera->GetName().isEmpty())
			Model->MoveDefaultCamera(Camera, Distance);
		else
			Model->MoveSelectedObjects(Distance, Distance, false, false, true, true);
/*** LPub3D Mod end ***/
	}
	else if (PropertyId == lcObjectPropertyId::CameraTargetX || PropertyId == lcObjectPropertyId::CameraTargetY || PropertyId == lcObjectPropertyId::CameraTargetZ)
	{
		lcVector3 Center = Camera->mTargetPosition;
		lcVector3 Position = Center;

/*** LPub3D Mod - Camera Globe, Switch Y and Z axis with -Y(LC -Z) in the up direction ***/
		if (PropertyId == lcObjectPropertyId::CameraTargetX)
			Position[X] = Value;
		else if (PropertyId == lcObjectPropertyId::CameraTargetY)
			Position[Z] = -Value;
		else if (PropertyId == lcObjectPropertyId::CameraTargetZ)
			Position[Y] = Value;
/*** LPub3D Mod end ***/

		lcVector3 Distance = Position - Center;

/*** LPub3D Mod - Camera Globe, camera name ***/
		if (Camera->GetName().isEmpty())
			Model->MoveDefaultCamera(Camera, Distance);
		else
			Model->MoveSelectedObjects(Distance, Distance, false, false, true, true);
/*** LPub3D Mod end ***/
	}
	else if (PropertyId == lcObjectPropertyId::CameraUpX || PropertyId == lcObjectPropertyId::CameraUpY || PropertyId == lcObjectPropertyId::CameraUpZ)
	{
		lcVector3 Center = Camera->mUpVector;
		lcVector3 Position = Center;

/*** LPub3D Mod - Camera Globe, Switch Y and Z axis with -Y(LC -Z) in the up direction ***/
		if (PropertyId == lcObjectPropertyId::CameraUpX)
			Position[X] = Value;
		else if (PropertyId == lcObjectPropertyId::CameraUpY)
			Position[Z] = -Value;
		else if (PropertyId == lcObjectPropertyId::CameraUpZ)
			Position[Y] = Value;
/*** LPub3D Mod end ***/

		lcVector3 Distance = Position - Center;

/*** LPub3D Mod - Camera Globe  ***/
		if (Camera->GetName().isEmpty())
			Model->MoveDefaultCamera(Camera, Distance);
		else
			Model->MoveSelectedObjects(Distance, Distance, false, false, true, true);
	}
	else if (PropertyId == lcObjectPropertyId::CameraLatitude || PropertyId == lcObjectPropertyId::CameraLongitude /*|| PropertyId == lcObjectPropertyId::CameraDistance*/)
	{
		QStringList ValueList = QString(Widget->text()).trimmed().split(" ",SkipEmptyParts);
		bool hasOther = ValueList.size() == 2;

		float Value = lcParseValueLocalized(ValueList.first());
		float OtherValue = hasOther ? lcParseValueLocalized(ValueList.last()) : 0.0f;

		float Latitude, Longitude, Distance;
		Camera->GetAngles(Latitude,Longitude,Distance);
		if (PropertyId == lcObjectPropertyId::CameraLatitude) {
			Latitude = Value;
			if (hasOther)
				Longitude = OtherValue;
		} else if (PropertyId == lcObjectPropertyId::CameraLongitude) {
			if (hasOther)
				Latitude = OtherValue;
			Longitude = Value;
		}

		Model->SetCameraGlobe(Camera, Latitude, Longitude, Distance);
/*** LPub3D Mod end ***/
	}
	else if (Camera)
	{
		if (PropertyId == lcObjectPropertyId::CameraFOV)
		{
			Model->SetCameraFOV(Camera, Value);
		}
		else if (PropertyId == lcObjectPropertyId::CameraNear)
		{
			Model->SetCameraZNear(Camera, Value);
		}
		else if (PropertyId == lcObjectPropertyId::CameraFar)
		{
			Model->SetCameraZFar(Camera, Value);
		}
	}
}

void lcPropertiesWidget::UpdateFloat(lcObjectPropertyId PropertyId, float Value)
{
	QLineEdit* Widget = qobject_cast<QLineEdit*>(mPropertyWidgets[static_cast<int>(PropertyId)].Editor);
		
	if (Widget)
	{
		QSignalBlocker Blocker(Widget);

		Widget->setText(lcFormatValueLocalized(Value));
	}

	UpdateKeyFrameWidget(PropertyId);
}

void lcPropertiesWidget::AddFloatProperty(lcObjectPropertyId PropertyId, const QString& Text, const QString& ToolTip, bool SupportsKeyFrames, float Min, float Max)
{
	AddLabel(PropertyId, Text, ToolTip);

	QLineEdit* Widget = new QLineEdit(this);
	Widget->setToolTip(ToolTip);

	Widget->setValidator(new QDoubleValidator(Min, Max, 1, Widget));

	connect(Widget, &QLineEdit::editingFinished, this, &lcPropertiesWidget::FloatChanged);

	mLayout->addWidget(Widget, mLayoutRow, 2);

	mCurrentCategory->Properties.push_back(PropertyId);
	mPropertyWidgets[static_cast<int>(PropertyId)].Editor = Widget;
	mPropertyWidgets[static_cast<int>(PropertyId)].Category = mCurrentCategory->Category;

	if (SupportsKeyFrames)
		AddKeyFrameWidget(PropertyId);

	mLayoutRow++;
}

void lcPropertiesWidget::IntegerChanged()
{
	// todo: switch to spinner and support mouse drag
	QLineEdit* LineEdit = qobject_cast<QLineEdit*>(sender());
	lcObjectPropertyId PropertyId = GetEditorWidgetPropertyId(LineEdit);

	if (PropertyId == lcObjectPropertyId::Count)
		return;

	lcModel* Model = gMainWindow->GetActiveModel();

	if (!Model)
		return;

	const int Value = LineEdit->text().toInt();
	Model->SetObjectsProperty(mFocusObject ? std::vector<lcObject*>{ mFocusObject } : mSelection, PropertyId, Value);
}

void lcPropertiesWidget::UpdateInteger(lcObjectPropertyId PropertyId)
{
	QLineEdit* LineEdit = qobject_cast<QLineEdit*>(mPropertyWidgets[static_cast<int>(PropertyId)].Editor);

	if (!LineEdit)
		return;

	QSignalBlocker Blocker(LineEdit);
	QVariant Value;
	bool Partial;

	std::tie(Value, Partial) = GetUpdateValue(PropertyId);

	if (Partial)
	{
		LineEdit->clear();
		LineEdit->setPlaceholderText(tr("Multiple Values"));
	}
	else
	{
		LineEdit->setText(QString::number(Value.toInt()));
		LineEdit->setPlaceholderText(QString());
	}

	UpdateKeyFrameWidget(PropertyId);
}

void lcPropertiesWidget::AddIntegerProperty(lcObjectPropertyId PropertyId, const QString& Text, const QString& ToolTip, bool SupportsKeyFrames, int Min, int Max)
{
	AddLabel(PropertyId, Text, ToolTip);

	QLineEdit* Widget = new QLineEdit(this);
	Widget->setToolTip(ToolTip);

	Widget->setValidator(new QIntValidator(Min, Max, Widget));

	connect(Widget, &QLineEdit::editingFinished, this, &lcPropertiesWidget::IntegerChanged);

	mLayout->addWidget(Widget, mLayoutRow, 2);

	mCurrentCategory->Properties.push_back(PropertyId);
	mPropertyWidgets[static_cast<int>(PropertyId)].Editor = Widget;
	mPropertyWidgets[static_cast<int>(PropertyId)].Category = mCurrentCategory->Category;

	if (SupportsKeyFrames)
		AddKeyFrameWidget(PropertyId);

	mLayoutRow++;
}

void lcPropertiesWidget::StepNumberChanged()
{
	QLineEdit* Widget = qobject_cast<QLineEdit*>(sender());
	lcObjectPropertyId PropertyId = GetEditorWidgetPropertyId(Widget);

	if (PropertyId == lcObjectPropertyId::Count)
		return;

	lcModel* Model = gMainWindow->GetActiveModel();

	if (!Model)
		return;

	bool Ok = true;
	QString Text = Widget->text();
	lcStep Step = Text.isEmpty() && PropertyId == lcObjectPropertyId::PieceStepHide ? LC_STEP_MAX : Text.toUInt(&Ok);

	if (!Ok)
		return;

	if (PropertyId == lcObjectPropertyId::PieceStepShow)
	{
		Model->SetSelectedPiecesStepShow(Step);
	}
	else if (PropertyId == lcObjectPropertyId::PieceStepHide)
	{
		Model->SetSelectedPiecesStepHide(Step);
	}
}

void lcPropertiesWidget::UpdateStepNumber(lcObjectPropertyId PropertyId, lcStep Step, lcStep Min, lcStep Max)
{
	QLineEdit* Widget = qobject_cast<QLineEdit*>(mPropertyWidgets[static_cast<int>(PropertyId)].Editor);

	if (Widget)
	{
		QSignalBlocker Blocker(Widget);

		Widget->setValidator(new lcStepValidator(Min, Max, PropertyId == lcObjectPropertyId::PieceStepHide, Widget));
		Widget->setText(Step == LC_STEP_MAX ? QString() : QString::number(Step));
	}

	UpdateKeyFrameWidget(PropertyId);
}

void lcPropertiesWidget::AddStepNumberProperty(lcObjectPropertyId PropertyId, const QString& Text, const QString& ToolTip, bool SupportsKeyFrames)
{
	AddLabel(PropertyId, Text, ToolTip);

	QLineEdit* Widget = new QLineEdit(this);
	Widget->setToolTip(ToolTip);

	connect(Widget, &QLineEdit::editingFinished, this, &lcPropertiesWidget::StepNumberChanged);

	mLayout->addWidget(Widget, mLayoutRow, 2);

	mCurrentCategory->Properties.push_back(PropertyId);
	mPropertyWidgets[static_cast<int>(PropertyId)].Editor = Widget;
	mPropertyWidgets[static_cast<int>(PropertyId)].Category = mCurrentCategory->Category;

	if (SupportsKeyFrames)
		AddKeyFrameWidget(PropertyId);

	mLayoutRow++;
}

void lcPropertiesWidget::StringChanged()
{
	QLineEdit* LineEdit = qobject_cast<QLineEdit*>(sender());
	lcObjectPropertyId PropertyId = GetEditorWidgetPropertyId(LineEdit);

	if (PropertyId == lcObjectPropertyId::Count)
		return;

	lcModel* Model = gMainWindow->GetActiveModel();

	if (!Model)
		return;

	QString Value = LineEdit->text();
	Model->SetObjectsProperty(mFocusObject ? std::vector<lcObject*>{ mFocusObject } : mSelection, PropertyId, Value);
}

void lcPropertiesWidget::UpdateString(lcObjectPropertyId PropertyId)
{
	QLineEdit* LineEdit = qobject_cast<QLineEdit*>(mPropertyWidgets[static_cast<int>(PropertyId)].Editor);

	if (!LineEdit)
		return;

	QSignalBlocker Blocker(LineEdit);
	QVariant Value;
	bool Partial;

	std::tie(Value, Partial) = GetUpdateValue(PropertyId);

	if (Partial)
	{
		LineEdit->clear();
		LineEdit->setPlaceholderText(tr("Multiple Values"));
	}
	else
	{
/*** LPub3D Mod - Update Default Camera ***/
		if (PropertyId == lcObjectPropertyId::CameraName && Value.toString().isEmpty())
		{
			Value = tr("Default");
			LineEdit->setDisabled(true);
		}
/*** LPub3D Mod end ***/
		LineEdit->setText(Value.toString());
		LineEdit->setPlaceholderText(QString());
	}

	UpdateKeyFrameWidget(PropertyId);
}

void lcPropertiesWidget::AddStringProperty(lcObjectPropertyId PropertyId, const QString& Text, const QString& ToolTip, bool SupportsKeyFrames)
{
	AddLabel(PropertyId, Text, ToolTip);

	QLineEdit* Widget = new QLineEdit(this);
	Widget->setToolTip(ToolTip);

	connect(Widget, &QLineEdit::editingFinished, this, &lcPropertiesWidget::StringChanged);

	mLayout->addWidget(Widget, mLayoutRow, 2);

	mCurrentCategory->Properties.push_back(PropertyId);
	mPropertyWidgets[static_cast<int>(PropertyId)].Editor = Widget;
	mPropertyWidgets[static_cast<int>(PropertyId)].Category = mCurrentCategory->Category;

	if (SupportsKeyFrames)
		AddKeyFrameWidget(PropertyId);

	mLayoutRow++;
}

void lcPropertiesWidget::StringListChanged(int Value)
{
	QComboBox* ComboBox = qobject_cast<QComboBox*>(sender());
	lcObjectPropertyId PropertyId = GetEditorWidgetPropertyId(ComboBox);

	if (PropertyId == lcObjectPropertyId::Count)
		return;

	lcModel* Model = gMainWindow->GetActiveModel();

	if (!Model)
		return;

	Model->SetObjectsProperty(mFocusObject ? std::vector<lcObject*>{ mFocusObject } : mSelection, PropertyId, Value);
}

void lcPropertiesWidget::UpdateStringList(lcObjectPropertyId PropertyId)
{
	QComboBox* ComboBox = qobject_cast<QComboBox*>(mPropertyWidgets[static_cast<int>(PropertyId)].Editor);

	if (!ComboBox)
		return;

	QSignalBlocker Blocker(ComboBox);
	QVariant Value;
	bool Partial;

	std::tie(Value, Partial) = GetUpdateValue(PropertyId);
	bool HasMultiple = (ComboBox->itemText(ComboBox->count() - 1) == tr("Multiple Values"));

	if (Partial)
	{
		if (!HasMultiple)
			ComboBox->addItem(tr("Multiple Values"));

		ComboBox->setCurrentIndex(ComboBox->count() - 1);
	}
	else
	{
		if (HasMultiple)
			ComboBox->removeItem(ComboBox->count() - 1);

		ComboBox->setCurrentIndex(Value.toInt());
	}

	UpdateKeyFrameWidget(PropertyId);
}

void lcPropertiesWidget::AddStringListProperty(lcObjectPropertyId PropertyId, const QString& Text, const QString& ToolTip, bool SupportsKeyFrames, const QStringList& Strings)
{
	AddLabel(PropertyId, Text, ToolTip);

	QComboBox* Widget = new QComboBox(this);
	Widget->setToolTip(ToolTip);
	Widget->addItems(Strings);

	connect(Widget, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &lcPropertiesWidget::StringListChanged);

	mLayout->addWidget(Widget, mLayoutRow, 2);

	mCurrentCategory->Properties.push_back(PropertyId);
	mPropertyWidgets[static_cast<int>(PropertyId)].Editor = Widget;
	mPropertyWidgets[static_cast<int>(PropertyId)].Category = mCurrentCategory->Category;

	if (SupportsKeyFrames)
		AddKeyFrameWidget(PropertyId);

	mLayoutRow++;
}

void lcPropertiesWidget::ColorButtonClicked()
{
	QToolButton* ColorButton = qobject_cast<QToolButton*>(sender());
	lcObjectPropertyId PropertyId = GetEditorWidgetPropertyId(ColorButton);

	if (PropertyId == lcObjectPropertyId::Count)
		return;

	QVariant Value;
	bool Partial;

	std::tie(Value, Partial) = GetUpdateValue(PropertyId);

	QColor InitialColor = Partial ? QColor(128, 128, 128) : lcQColorFromVector3(Value.value<lcVector3>());

	QMenu* Menu = new QMenu(ColorButton);

	QWidgetAction* Action = new QWidgetAction(Menu);
	lcColorDialogPopup *Popup = new lcColorDialogPopup(InitialColor, Menu);
	Action->setDefaultWidget(Popup);
	Menu->addAction(Action);

	connect(Popup, &lcColorDialogPopup::ColorSelected, this, &lcPropertiesWidget::ColorChanged);

	Menu->exec(ColorButton->mapToGlobal(ColorButton->rect().bottomLeft()));

	delete Menu;
}

void lcPropertiesWidget::ColorChanged(QColor Color)
{
	lcColorDialogPopup* Popup = qobject_cast<lcColorDialogPopup*>(sender());
	QMenu* Menu = qobject_cast<QMenu*>(Popup->parent());
	QToolButton* ColorButton = qobject_cast<QToolButton*>(Menu->parent());
	lcObjectPropertyId PropertyId = GetEditorWidgetPropertyId(ColorButton);

	if (!Color.isValid())
		return;

	lcModel* Model = gMainWindow->GetActiveModel();

	if (!Model)
		return;

	const lcVector3 FloatColor = lcVector3FromQColor(Color);
	Model->SetObjectsProperty(mFocusObject ? std::vector<lcObject*>{ mFocusObject } : mSelection, PropertyId, QVariant::fromValue<lcVector3>(FloatColor));
}

void lcPropertiesWidget::UpdateColor(lcObjectPropertyId PropertyId)
{
	QToolButton* ColorButton = qobject_cast<QToolButton*>(mPropertyWidgets[static_cast<int>(PropertyId)].Editor);

	if (!ColorButton)
		return;

	QSignalBlocker Blocker(ColorButton);
	QVariant Value;
	bool Partial;

	std::tie(Value, Partial) = GetUpdateValue(PropertyId);

	QColor Color = Partial ? QColor(128, 128, 128) : lcQColorFromVector3(Value.value<lcVector3>());
	QPixmap Pixmap(14, 14);
	Pixmap.fill(Color);

	ColorButton->setIcon(Pixmap);
	ColorButton->setText(Partial ? tr(" Multiple Colors") : QString("  ") + Color.name());

	UpdateKeyFrameWidget(PropertyId);
}

void lcPropertiesWidget::AddColorProperty(lcObjectPropertyId PropertyId, const QString& Text, const QString& ToolTip, bool SupportsKeyFrames)
{
	AddLabel(PropertyId, Text, ToolTip);

	QToolButton* Widget = new QToolButton(this);
	Widget->setToolTip(ToolTip);
	Widget->setAutoRaise(true);
	Widget->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
	Widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

	connect(Widget, &QToolButton::clicked, this, &lcPropertiesWidget::ColorButtonClicked);

	mLayout->addWidget(Widget, mLayoutRow, 2);

	mCurrentCategory->Properties.push_back(PropertyId);
	mPropertyWidgets[static_cast<int>(PropertyId)].Editor = Widget;
	mPropertyWidgets[static_cast<int>(PropertyId)].Category = mCurrentCategory->Category;

	if (SupportsKeyFrames)
		AddKeyFrameWidget(PropertyId);

	mLayoutRow++;
}

void lcPropertiesWidget::PieceColorChanged(int ColorIndex)
{
	lcColorPickerPopup* Popup = qobject_cast<lcColorPickerPopup*>(sender());
	QMenu* Menu = qobject_cast<QMenu*>(Popup->parent());
	QToolButton* ColorButton = qobject_cast<QToolButton*>(Menu->parent());
	lcObjectPropertyId PropertyId = GetEditorWidgetPropertyId(ColorButton);

	Menu->close();

	if (PropertyId == lcObjectPropertyId::Count)
		return;

	lcModel* Model = gMainWindow->GetActiveModel();

	if (!Model)
		return;

	Model->SetObjectsProperty(mFocusObject ? std::vector<lcObject*>{ mFocusObject } : mSelection, PropertyId, ColorIndex);
}

void lcPropertiesWidget::PieceColorButtonClicked()
{
	QToolButton* ColorButton = qobject_cast<QToolButton*>(sender());
	lcObjectPropertyId PropertyId = GetEditorWidgetPropertyId(ColorButton);

	if (!ColorButton)
		return;

	QVariant Value;
	bool Partial;

	std::tie(Value, Partial) = GetUpdateValue(PropertyId);

	int ColorIndex = Partial ? gDefaultColor : Value.toInt();

	QMenu* Menu = new QMenu(ColorButton);

	QWidgetAction* Action = new QWidgetAction(Menu);
	lcColorPickerPopup* Popup = new lcColorPickerPopup(Menu, ColorIndex);
	Action->setDefaultWidget(Popup);
	Menu->addAction(Action);

	connect(Popup, &lcColorPickerPopup::Selected, this, &lcPropertiesWidget::PieceColorChanged);

	Menu->exec(ColorButton->mapToGlobal(ColorButton->rect().bottomLeft()));

	delete Menu;
}

void lcPropertiesWidget::UpdatePieceColor(lcObjectPropertyId PropertyId)
{
	QToolButton* ColorButton = qobject_cast<QToolButton*>(mPropertyWidgets[static_cast<int>(PropertyId)].Editor);

	if (!ColorButton)
		return;

	QSignalBlocker Blocker(ColorButton);
	QVariant Value;
	bool Partial;

	std::tie(Value, Partial) = GetUpdateValue(PropertyId);

	const int ColorIndex = Value.toInt();
	QColor Color = Partial ? QColor(128, 128, 128) : QColor::fromRgbF(gColorList[ColorIndex].Value[0], gColorList[ColorIndex].Value[1], gColorList[ColorIndex].Value[2]);
	QPixmap Pixmap(14, 14);
	Pixmap.fill(Color);

	ColorButton->setIcon(Pixmap);
	ColorButton->setText(Partial ? tr(" Multiple Colors") : QString("  ") + gColorList[ColorIndex].Name);

	UpdateKeyFrameWidget(PropertyId);
}

void lcPropertiesWidget::AddPieceColorProperty(lcObjectPropertyId PropertyId, const QString& Text, const QString& ToolTip, bool SupportsKeyFrames)
{
	AddLabel(PropertyId, Text, ToolTip);

	QToolButton* Widget = new QToolButton(this);
	Widget->setToolTip(ToolTip);
	Widget->setAutoRaise(true);
	Widget->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
	Widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

	connect(Widget, &QToolButton::clicked, this, &lcPropertiesWidget::PieceColorButtonClicked);

	mLayout->addWidget(Widget, mLayoutRow, 2);

	mCurrentCategory->Properties.push_back(PropertyId);
	mPropertyWidgets[static_cast<int>(PropertyId)].Editor = Widget;
	mPropertyWidgets[static_cast<int>(PropertyId)].Category = mCurrentCategory->Category;

	if (SupportsKeyFrames)
		AddKeyFrameWidget(PropertyId);

	mLayoutRow++;
}

void lcPropertiesWidget::UpdatePieceId(lcObjectPropertyId PropertyId)
{
	lcElidableToolButton* PieceIdButton = qobject_cast<lcElidableToolButton*>(mPropertyWidgets[static_cast<int>(PropertyId)].Editor);

	if (!PieceIdButton)
		return;

	QSignalBlocker Blocker(PieceIdButton);
	QVariant Value;
	bool Partial;

	std::tie(Value, Partial) = GetUpdateValue(PropertyId);

	PieceInfo* Info = static_cast<PieceInfo*>(Value.value<void*>());

	if (Partial)
		PieceIdButton->setText(tr("Multiple Pieces"));
	else if (Info)
		PieceIdButton->setText(Info->m_strDescription);

	UpdateKeyFrameWidget(PropertyId);
}

void lcPropertiesWidget::PieceIdButtonClicked()
{
	QToolButton* PieceIdButton = qobject_cast<QToolButton*>(sender());
	lcObjectPropertyId PropertyId = GetEditorWidgetPropertyId(PieceIdButton);

	if (!PieceIdButton)
		return;

	QVariant Value;
	bool Partial;

	std::tie(Value, Partial) = GetUpdateValue(PropertyId);

	PieceInfo* Info = static_cast<PieceInfo*>(Value.value<void*>());

	QMenu* Menu = new QMenu(PieceIdButton);

	QWidgetAction* Action = new QWidgetAction(Menu);
	lcPieceIdPickerPopup* Popup = new lcPieceIdPickerPopup(Partial ? nullptr : Info, Menu);
	Action->setDefaultWidget(Popup);
	Menu->addAction(Action);

	connect(Popup, &lcPieceIdPickerPopup::PieceIdSelected, this, &lcPropertiesWidget::PieceIdChanged);

	Menu->exec(PieceIdButton->mapToGlobal(PieceIdButton->rect().bottomLeft()));

	delete Menu;
}

void lcPropertiesWidget::PieceIdChanged(PieceInfo* Info)
{
	lcPieceIdPickerPopup* Popup = qobject_cast<lcPieceIdPickerPopup*>(sender());
	QMenu* Menu = qobject_cast<QMenu*>(Popup->parent());
	QToolButton* PieceIdButton = qobject_cast<QToolButton*>(Menu->parent());
	lcObjectPropertyId PropertyId = GetEditorWidgetPropertyId(PieceIdButton);

	lcModel* Model = gMainWindow->GetActiveModel();

	if (!Model || !Info)
		return;

	Model->SetObjectsProperty(mFocusObject ? std::vector<lcObject*>{ mFocusObject } : mSelection, PropertyId, QVariant::fromValue<void*>(Info));
}

void lcPropertiesWidget::AddPieceIdProperty(lcObjectPropertyId PropertyId, const QString& Text, const QString& ToolTip, bool SupportsKeyFrames)
{
	AddLabel(PropertyId, Text, ToolTip);

	lcElidableToolButton* Widget = new lcElidableToolButton(this);
	Widget->setToolTip(ToolTip);
	Widget->setAutoRaise(true);
	Widget->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
	Widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

	QPixmap Pixmap(1, 1);
	Pixmap.fill(QColor::fromRgba64(0, 0, 0, 0));
	Widget->setIcon(Pixmap);

	connect(Widget, &QToolButton::clicked, this, &lcPropertiesWidget::PieceIdButtonClicked);

	mLayout->addWidget(Widget, mLayoutRow, 2);

	mCurrentCategory->Properties.push_back(PropertyId);
	mPropertyWidgets[static_cast<int>(PropertyId)].Editor = Widget;
	mPropertyWidgets[static_cast<int>(PropertyId)].Category = mCurrentCategory->Category;

	if (SupportsKeyFrames)
		AddKeyFrameWidget(PropertyId);

	mLayoutRow++;
}

void lcPropertiesWidget::CreateWidgets()
{
	mLayout = new QGridLayout(this);
	mLayout->setVerticalSpacing(1);

	AddCategory(CategoryIndex::Piece, tr("Piece"));

	AddPieceIdProperty(lcObjectPropertyId::PieceId, tr("Part"), tr("Part Id"), false);
	AddPieceColorProperty(lcObjectPropertyId::PieceColor, tr("Color"), tr("Piece color"), false);
/*** LPub3D Mod - LPUB meta properties ***/
	AddStringListProperty(lcObjectPropertyId::PieceType, tr("Type"), tr("Piece type"), false, lcPiece::GetPieceTypeStrings());
	AddStringProperty(lcObjectPropertyId::PieceFileID, tr("File"), tr("File name"), false);
	AddStringProperty(lcObjectPropertyId::PieceModel, tr("Model"), tr("Model name"), false);
	AddBoolProperty(lcObjectPropertyId::PieceIsSubmodel, tr("Submodel"), tr("Piece is submodel"), false),
/*** LPub3D Mod end ***/

	AddSpacing();

	AddStepNumberProperty(lcObjectPropertyId::PieceStepShow, tr("Show"), tr("Step when piece is added to the model"), false);
	AddStepNumberProperty(lcObjectPropertyId::PieceStepHide, tr("Hide"), tr("Step when piece is hidden"), false);

	AddSpacing();

	AddCategory(CategoryIndex::Camera, tr("Camera"));

	AddStringProperty(lcObjectPropertyId::CameraName, tr("Name"), tr("Camera name"), false);
	AddStringListProperty(lcObjectPropertyId::CameraType, tr("Type"), tr("Camera type"), false, lcCamera::GetCameraTypeStrings());

	AddSpacing();

	AddFloatProperty(lcObjectPropertyId::CameraFOV, tr("FOV"), tr("Field of view in degrees"), false, 0.1f, 179.9f);
	AddFloatProperty(lcObjectPropertyId::CameraNear, tr("Near"), tr("Near clipping distance"), false, 0.001f, FLT_MAX);
	AddFloatProperty(lcObjectPropertyId::CameraFar, tr("Far"), tr("Far clipping distance"), false, 0.001f, FLT_MAX);
/*** LPub3D Mod - LPUB meta properties ***/
	AddIntegerProperty(lcObjectPropertyId::CameraLatitude, tr("Latitude"), tr("Camera globe latitude in degrees"), true, -360, 360);
	AddIntegerProperty(lcObjectPropertyId::CameraLongitude, tr("Longitude"), tr("Camera globe longitude in degrees"), true, -360, 360);
	AddIntegerProperty(lcObjectPropertyId::CameraDistance, tr("Distance"), tr("Camera distance using LDraw units"), true, 1, INT_MAX);
/*** LPub3D Mod end ***/

	AddSpacing();

	AddCategory(CategoryIndex::CameraTransform, tr("Transform"));

	AddFloatProperty(lcObjectPropertyId::CameraPositionX, tr("Position X"), tr("Camera position"), true, -FLT_MAX, FLT_MAX);
	AddFloatProperty(lcObjectPropertyId::CameraPositionY, tr("Y"), tr("Camera position"), true, -FLT_MAX, FLT_MAX);
	AddFloatProperty(lcObjectPropertyId::CameraPositionZ, tr("Z"), tr("Camera position"), true, -FLT_MAX, FLT_MAX);

	AddSpacing();

	AddFloatProperty(lcObjectPropertyId::CameraTargetX, tr("Target X"), tr("Camera target position"), true, -FLT_MAX, FLT_MAX);
	AddFloatProperty(lcObjectPropertyId::CameraTargetY, tr("Y"), tr("Camera target position"), true, -FLT_MAX, FLT_MAX);
	AddFloatProperty(lcObjectPropertyId::CameraTargetZ, tr("Z"), tr("Camera target position"), true, -FLT_MAX, FLT_MAX);

	AddSpacing();

	AddFloatProperty(lcObjectPropertyId::CameraUpX, tr("Up X"), tr("Camera up direction"), true, -FLT_MAX, FLT_MAX);
	AddFloatProperty(lcObjectPropertyId::CameraUpY, tr("Y"), tr("Camera up direction"), true, -FLT_MAX, FLT_MAX);
	AddFloatProperty(lcObjectPropertyId::CameraUpZ, tr("Z"), tr("Camera up direction"), true, -FLT_MAX, FLT_MAX);

	AddSpacing();

/*** LPub3D Mod - LPUB meta properties ***/
	AddCategory(CategoryIndex::CameraImage, tr("Image"));

	AddFloatProperty(lcObjectPropertyId::CameraImageScale, tr("Scale"), tr("Image scale"), false, 1.0f, FLT_MAX);
	AddIntegerProperty(lcObjectPropertyId::CameraImageResolution, tr("Resolution"), tr("Image resolution"), false, 1, INT_MAX);
	AddIntegerProperty(lcObjectPropertyId::CameraImagePageWidth, tr("Page Width"), tr("Page width"), false, 1, INT_MAX);
	AddIntegerProperty(lcObjectPropertyId::CameraImagePageHeight, tr("Page Height"), tr("Page height"), false, 1, INT_MAX);
	AddIntegerProperty(lcObjectPropertyId::CameraImageWidth, tr("Width"), tr("Image width"), false, 1, INT_MAX);
	AddIntegerProperty(lcObjectPropertyId::CameraImageHeight, tr("Height"), tr("Image height"), false, 1, INT_MAX);

	AddSpacing();
/*** LPub3D Mod end ***/

	AddCategory(CategoryIndex::Light, tr("Light"));

	AddStringProperty(lcObjectPropertyId::LightName, tr("Name"), tr("Light name"), false);
	AddStringListProperty(lcObjectPropertyId::LightType, tr("Type"), tr("Light type"), false, lcLight::GetLightTypeStrings());
/*** LPub3D Mod - LPUB meta properties ***/
	AddStringListProperty(lcObjectPropertyId::LightFormat, tr("Format"), tr("Light meta command format"), false, lcLight::GetLightFormatStrings());
/*** LPub3D Mod end ***/

	AddSpacing();

	AddColorProperty(lcObjectPropertyId::LightColor, tr("Color"), tr("Light color"), true);
	AddBoolProperty(lcObjectPropertyId::LightCastShadow, tr("Cast Shadow"), tr("Cast a shadow from this light"), false);

	AddSpacing();

	AddFloatProperty(lcObjectPropertyId::LightSpotConeAngle, tr("Cone Angle"), tr("Angle in degrees of the spot light's beam"), true, 0.0f, 179.9f);
	AddFloatProperty(lcObjectPropertyId::LightSpotPenumbraAngle, tr("Penumbra Angle"), tr("Angle in degrees over which the intensity of the spot light falls off to zero"), true, 0.0f, 179.9f);
	AddStringListProperty(lcObjectPropertyId::LightAreaShape, tr("Area Shape"), tr("The shape of the area light"), false, lcLight::GetAreaShapeStrings());
	AddFloatProperty(lcObjectPropertyId::LightAreaSizeX, tr("Size X"), tr("The width of the area light"), true, 0.0f, FLT_MAX);
	AddFloatProperty(lcObjectPropertyId::LightAreaSizeY, tr("Y"), tr("The height of the area light"), true, 0.0f, FLT_MAX);

	AddSpacing();

	AddCategory(CategoryIndex::LightBlender, tr("Blender Settings"));

	AddFloatProperty(lcObjectPropertyId::LightBlenderPower, tr("Power"), tr("Power of the light in Watts"), true, 0.0f, FLT_MAX);
/*** LPub3D Mod - LPUB meta properties ***/
	AddIntegerProperty(lcObjectPropertyId::LightBlenderCutoffDistance, tr("Cutoff Distance"), tr("Distance at which the light influence will be set to 0"), true, 1, INT_MAX);
	AddIntegerProperty(lcObjectPropertyId::LightBlenderDiffuse, tr("Diffuse"), tr("Diffuse factor"), true, 1, INT_MAX);
	AddIntegerProperty(lcObjectPropertyId::LightBlenderSpecular, tr("Specular"), tr("Specular factor"), true, 1, INT_MAX);
/*** LPub3D Mod end ***/
	AddFloatProperty(lcObjectPropertyId::LightPointBlenderRadius, tr("Radius"), tr("Shadow soft size"), true, 0.0f, FLT_MAX);
	AddFloatProperty(lcObjectPropertyId::LightSpotBlenderRadius, tr("Radius"), tr("Shadow soft size"), true, 0.0f, FLT_MAX);
	AddFloatProperty(lcObjectPropertyId::LightDirectionalBlenderAngle, tr("Angle"), tr("Angular diameter of the light"), true, 0.0f, 180.0f);

	AddSpacing();

	AddCategory(CategoryIndex::LightPOVRay, tr("POV-Ray Settings"));

	AddFloatProperty(lcObjectPropertyId::LightPOVRayPower, tr("Power"), tr("Power of the light (multiplicative factor)"), true, 0.0f, FLT_MAX);
	AddFloatProperty(lcObjectPropertyId::LightPOVRayFadeDistance, tr("Fade Distance"), tr("The distance at which the full light intensity arrives"), true, 0.0f, FLT_MAX);
	AddFloatProperty(lcObjectPropertyId::LightPOVRayFadePower, tr("Fade Power"), tr("Light falloff rate"), true, 0.0f, FLT_MAX);
	AddFloatProperty(lcObjectPropertyId::LightSpotPOVRayTightness, tr("Tightness"), tr("Additional exponential spot light edge softening"), true, 0.0f, FLT_MAX);
	AddIntegerProperty(lcObjectPropertyId::LightAreaPOVRayGridX, tr("Grid X"), tr("Number of point sources along the X axis"), true, 1, INT_MAX);
	AddIntegerProperty(lcObjectPropertyId::LightAreaPOVRayGridY, tr("Y"), tr("Number of point sources along the Y axis"), true, 1, INT_MAX);

	AddSpacing();

	AddCategory(CategoryIndex::ObjectTransform, tr("Transform"));

	AddFloatProperty(lcObjectPropertyId::ObjectPositionX, tr("Position X"), tr("Position of the object"), true, -FLT_MAX, FLT_MAX);
	AddFloatProperty(lcObjectPropertyId::ObjectPositionY, tr("Y"), tr("Position of the object"), true, -FLT_MAX, FLT_MAX);
	AddFloatProperty(lcObjectPropertyId::ObjectPositionZ, tr("Z"), tr("Position of the object"), true, -FLT_MAX, FLT_MAX);

	AddSpacing();

	AddFloatProperty(lcObjectPropertyId::ObjectRotationX, tr("Rotation X"), tr("Rotation of the object in degrees"), true, -FLT_MAX, FLT_MAX);
	AddFloatProperty(lcObjectPropertyId::ObjectRotationY, tr("Y"), tr("Rotation of the object in degrees"), true, -FLT_MAX, FLT_MAX);
	AddFloatProperty(lcObjectPropertyId::ObjectRotationZ, tr("Z"), tr("Rotation of the object in degrees"), true, -FLT_MAX, FLT_MAX);

	AddSpacing();

	mLayout->setRowStretch(mLayout->rowCount(), 1);
}

void lcPropertiesWidget::SetLayoutMode(LayoutMode Mode)
{
	if (mLayoutMode == Mode)
		return;

	mLayoutMode = Mode;
	const bool IsPiece = (mLayoutMode == LayoutMode::Piece);
	const bool IsCamera = (mLayoutMode == LayoutMode::Camera);
	const bool IsLight = (mLayoutMode == LayoutMode::Light);

	SetCategoryVisible(CategoryIndex::Piece, IsPiece);
	SetCategoryVisible(CategoryIndex::Camera, IsCamera);
	SetCategoryVisible(CategoryIndex::CameraTransform, IsCamera);
/*** LPub3D Mod - LPUB meta properties ***/
	SetCategoryVisible(CategoryIndex::CameraImage, IsCamera);
/*** LPub3D Mod end ***/
	SetCategoryVisible(CategoryIndex::Light, IsLight);
	SetCategoryVisible(CategoryIndex::LightBlender, IsLight);
	SetCategoryVisible(CategoryIndex::LightPOVRay, IsLight);
	SetCategoryVisible(CategoryIndex::ObjectTransform, IsPiece || IsLight);
}

void lcPropertiesWidget::SetCategoryWidgetsVisible(CategoryWidgets& Category, bool Visible)
{
	for (lcObjectPropertyId PropertyId : Category.Properties)
		SetPropertyWidgetsVisible(PropertyId, Visible && mPropertyWidgets[static_cast<int>(PropertyId)].Visible);

	for (int Row : Category.SpacingRows)
		mLayout->setRowMinimumHeight(Row, Visible ? 5 : 0);
}

void lcPropertiesWidget::SetPropertyVisible(lcObjectPropertyId PropertyId, bool Visible)
{
	PropertyWidgets& Property = mPropertyWidgets[static_cast<int>(PropertyId)];
	Property.Visible = Visible;

	SetPropertyWidgetsVisible(PropertyId, Visible && mCategoryWidgets[static_cast<int>(Property.Category)].Button->IsExpanded());
}

void lcPropertiesWidget::SetPropertyWidgetsVisible(lcObjectPropertyId PropertyId, bool Visible)
{
	PropertyWidgets& Property = mPropertyWidgets[static_cast<int>(PropertyId)];

	if (Property.Label)
		Property.Label->setVisible(Visible);

	if (Property.Editor)
		Property.Editor->setVisible(Visible);

	if (Property.KeyFrame)
		Property.KeyFrame->setVisible(Visible);
}

/*** LPub3D Mod - LPUB meta properties ***/
void lcPropertiesWidget::SetPropertyReadOnly(lcObjectPropertyId PropertyId, bool ReadOnly)
{
	PropertyWidgets& Property = mPropertyWidgets[static_cast<int>(PropertyId)];
	Property.ReadOnly = ReadOnly;

	SetPropertyWidgetsReadOnly(PropertyId, ReadOnly && mCategoryWidgets[static_cast<int>(Property.Category)].Button->IsExpanded());
}

void lcPropertiesWidget::SetPropertyWidgetsReadOnly(lcObjectPropertyId PropertyId, bool ReadOnly)
{
	PropertyWidgets& Property = mPropertyWidgets[static_cast<int>(PropertyId)];

	if (Property.Label)
		Property.Label->setDisabled(ReadOnly);

	if (Property.Editor)
		Property.Editor->setDisabled(ReadOnly);

	if (Property.KeyFrame)
		Property.KeyFrame->setDisabled(ReadOnly);
}
/*** LPub3D Mod end ***/

void lcPropertiesWidget::SetCategoryVisible(CategoryIndex Index, bool Visible)
{
	CategoryWidgets& Category = mCategoryWidgets[static_cast<int>(Index)];

	Category.Button->setVisible(Visible);

	SetCategoryWidgetsVisible(Category, Visible && Category.Button->IsExpanded());
}

void lcPropertiesWidget::SetEmpty()
{
	SetLayoutMode(LayoutMode::Empty);

	mFocusObject = nullptr;
	mSelection.clear();
}

void lcPropertiesWidget::SetPiece(const std::vector<lcObject*>& Selection, lcObject* Focus)
{
	SetLayoutMode(LayoutMode::Piece);

	lcPiece* Piece = dynamic_cast<lcPiece*>(Focus);
	mSelection = Selection;
	mFocusObject = Piece;

/*** LPub3D Mod - Camera Globe, Switch Y and Z axis with -Y(LC -Z) in the up direction ***/
	lcVector3 Position, SwitchPosition;
	lcMatrix33 RelativeRotation;
	lcModel* Model = gMainWindow->GetActiveModel();

	if (Model)
/*** LPub3D Mod - LPUB meta properties ***/
	{
		Model->GetMoveRotateTransform(SwitchPosition, RelativeRotation);
		if (Piece)
		{
			Piece->SetModelName(Model->GetProperties().mModelName);
			Piece->SetIsSubmodel(Piece->mPieceInfo->IsModel());
			mFocusObject = Piece;
		}

	}
/*** LPub3D Mod end ***/

	Position = lcVector3LeoCADToLDraw(SwitchPosition);
/*** LPub3D Mod end ***/

/*** LPub3D Mod - preview widget for LPub3D ***/
	lcPreferences& Preferences = lcGetPreferences();
	int ColorIndex = gDefaultColor;
	PieceInfo* Info = nullptr;
/*** LPub3D Mod end ***/

	UpdateFloat(lcObjectPropertyId::ObjectPositionX, Position[0]);
	UpdateFloat(lcObjectPropertyId::ObjectPositionY, Position[1]);
	UpdateFloat(lcObjectPropertyId::ObjectPositionZ, Position[2]);

/*** LPub3D Mod - Camera Globe, Switch Y and Z axis with -Y(LC -Z) in the up direction ***/
	lcVector3 Rotation, SwitchRotation;

	if (Piece)
		SwitchRotation = lcMatrix44ToEulerAngles(Piece->mModelWorld) * LC_RTOD;
	else
		SwitchRotation = lcVector3(0.0f, 0.0f, 0.0f);

	Rotation = lcVector3LeoCADToLDraw(SwitchRotation);
/*** LPub3D Mod end ***/

	UpdateFloat(lcObjectPropertyId::ObjectRotationX, Rotation[0]);
	UpdateFloat(lcObjectPropertyId::ObjectRotationY, Rotation[1]);
	UpdateFloat(lcObjectPropertyId::ObjectRotationZ, Rotation[2]);

	lcStep StepShow = 1;
	lcStep StepHide = LC_STEP_MAX;

	if (Piece)
	{
		StepShow = Piece->GetStepShow();
		StepHide = Piece->GetStepHide();
/*** LPub3D Mod - preview widget for LPub3D ***/
		Info = Piece->mPieceInfo;
		ColorIndex = Info->IsModel() ? lcGetColorIndex(LDRAW_MATERIAL_COLOUR) : Piece->GetColorIndex();
		if (Preferences.mPreviewEnabled && Preferences.mPreviewPosition == lcPreviewPosition::Dockable)
		{
			quint32 ColorCode = lcGetColorCode(ColorIndex);
			gMainWindow->PreviewPiece(Info->mFileName, ColorCode, false);
		}
/*** LPub3D Mod end ***/
	}
	else
	{
		bool FirstPiece = true;

		for (lcObject* Object : Selection)
		{
			if (!Object->IsPiece())
				continue;

			lcPiece* SelectedPiece = (lcPiece*)Object;

			if (FirstPiece)
			{
				StepShow = SelectedPiece->GetStepShow();
				StepHide = SelectedPiece->GetStepHide();
/*** LPub3D Mod - preview widget for LPub3D ***/
				ColorIndex = SelectedPiece->GetColorIndex();
				Info = SelectedPiece->mPieceInfo;
				if (Preferences.mPreviewEnabled && Preferences.mPreviewPosition == lcPreviewPosition::Dockable)
				{
					quint32 ColorCode = lcGetColorCode(ColorIndex);
					gMainWindow->PreviewPiece(Info->mFileName, ColorCode, false);
				}
/*** LPub3D Mod end ***/
				FirstPiece = false;
			}
			else
			{
				if (SelectedPiece->GetStepShow() != StepShow)
					StepShow = 0;

				if (SelectedPiece->GetStepHide() != StepHide)
					StepHide = 0;
/*** LPub3D Mod - preview widget for LPub3D ***/
				if (SelectedPiece->GetColorIndex() != ColorIndex)
					ColorIndex = gDefaultColor;

				if (SelectedPiece->mPieceInfo != Info)
					Info = nullptr;
/*** LPub3D Mod end ***/
			}
		}
	}

	UpdatePieceId(lcObjectPropertyId::PieceId);
	UpdatePieceColor(lcObjectPropertyId::PieceColor);
/*** LPub3D Mod - LPUB meta properties ***/
	UpdateBool(lcObjectPropertyId::PieceIsSubmodel);
	UpdateStringList(lcObjectPropertyId::PieceType);
	UpdateString(lcObjectPropertyId::PieceFileID);
	UpdateString(lcObjectPropertyId::PieceModel);

	SetPropertyReadOnly(lcObjectPropertyId::PieceIsSubmodel, true);
	SetPropertyReadOnly(lcObjectPropertyId::PieceType, true);
	SetPropertyReadOnly(lcObjectPropertyId::PieceFileID, true);
	SetPropertyReadOnly(lcObjectPropertyId::PieceModel, true);
/*** LPub3D Mod end ***/
	UpdateStepNumber(lcObjectPropertyId::PieceStepShow, StepShow ? StepShow : LC_STEP_MAX, 1, StepHide - 1);
	UpdateStepNumber(lcObjectPropertyId::PieceStepHide, StepHide ? StepHide : LC_STEP_MAX, StepShow + 1, LC_STEP_MAX);
}

void lcPropertiesWidget::SetCamera(const std::vector<lcObject*>& Selection, lcObject* Focus)
{
	SetLayoutMode(LayoutMode::Camera);

	lcCamera* Camera = dynamic_cast<lcCamera*>(Focus);
	mSelection = Selection;
	mFocusObject = Camera;

	lcVector3 Position(0.0f, 0.0f, 0.0f);
	lcVector3 Target(0.0f, 0.0f, 0.0f);
	lcVector3 UpVector(0.0f, 0.0f, 0.0f);
	float FoV = 60.0f;
	float ZNear = 1.0f;
	float ZFar = 100.0f;

/*** LPub3D Mod - Camera Globe ***/
	float Latitude = 0.0f;
	float Longitude = 0.0f;
	float Distance = 0.0f;
	float ImageScale = 0.0f;
	int Resolution = 0;
	int ImageWidth = 0;
	int ImageHeight = 0;
	int ImagePageWidth = 0;
	int ImagePageHeight = 0;
/*** LPub3D Mod end ***/

	if (Camera)
	{
/*** LPub3D Mod - Camera Globe, Switch Y and Z axis with -Y(LC -Z) in the up direction ***/
		Position = lcVector3LeoCADToLDraw(Camera->mPosition);
		Target = lcVector3LeoCADToLDraw(Camera->mTargetPosition);
		UpVector = lcVector3LeoCADToLDraw(Camera->mUpVector);
/*** LPub3D Mod end ***/
		FoV = Camera->m_fovy;
		ZNear = Camera->m_zNear;
		ZFar = Camera->m_zFar;
/*** LPub3D Mod - Camera Globe ***/
		Camera->GetAngles(Latitude,Longitude,Distance);
		ImageScale      = Camera->GetScale();
		Resolution      = lcGetActiveProject()->GetResolution();
		ImagePageWidth  = lcGetActiveProject()->GetPageWidth();
		ImagePageHeight = lcGetActiveProject()->GetPageHeight();
		ImageWidth      = lcGetActiveProject()->GetImageWidth();
		ImageHeight     = lcGetActiveProject()->GetImageHeight();
 /*** LPub3D Mod end ***/
	}

	UpdateString(lcObjectPropertyId::CameraName);
	UpdateStringList(lcObjectPropertyId::CameraType);

	UpdateFloat(lcObjectPropertyId::CameraFOV, FoV);
	UpdateFloat(lcObjectPropertyId::CameraNear, ZNear);
	UpdateFloat(lcObjectPropertyId::CameraFar, ZFar);

/*** LPub3D Mod - LPUB meta properties ***/
	UpdateFloat(lcObjectPropertyId::CameraLatitude, Latitude);
	UpdateFloat(lcObjectPropertyId::CameraLongitude, Longitude);
	UpdateFloat(lcObjectPropertyId::CameraDistance, Distance);
/*** LPub3D Mod end ***/

	UpdateFloat(lcObjectPropertyId::CameraPositionX, Position[0]);
	UpdateFloat(lcObjectPropertyId::CameraPositionY, Position[1]);
	UpdateFloat(lcObjectPropertyId::CameraPositionZ, Position[2]);

	UpdateFloat(lcObjectPropertyId::CameraTargetX, Target[0]);
	UpdateFloat(lcObjectPropertyId::CameraTargetY, Target[1]);
	UpdateFloat(lcObjectPropertyId::CameraTargetZ, Target[2]);

	UpdateFloat(lcObjectPropertyId::CameraUpX, UpVector[0]);
	UpdateFloat(lcObjectPropertyId::CameraUpY, UpVector[1]);
	UpdateFloat(lcObjectPropertyId::CameraUpZ, UpVector[2]);

/*** LPub3D Mod - LPUB meta properties ***/
	UpdateFloat(lcObjectPropertyId::CameraImageScale, ImageScale);
	UpdateFloat(lcObjectPropertyId::CameraImageResolution, Resolution);
	UpdateFloat(lcObjectPropertyId::CameraImageWidth, ImageWidth);
	UpdateFloat(lcObjectPropertyId::CameraImageHeight, ImageHeight);
	UpdateFloat(lcObjectPropertyId::CameraImagePageWidth, ImagePageWidth);
	UpdateFloat(lcObjectPropertyId::CameraImagePageHeight, ImagePageHeight);

	SetPropertyReadOnly(lcObjectPropertyId::CameraImageScale, true);
	SetPropertyReadOnly(lcObjectPropertyId::CameraImageResolution, true);
	SetPropertyReadOnly(lcObjectPropertyId::CameraImagePageWidth, true);
	SetPropertyReadOnly(lcObjectPropertyId::CameraImagePageHeight, true);
/*** LPub3D Mod end ***/
}

void lcPropertiesWidget::SetLight(const std::vector<lcObject*>& Selection, lcObject* Focus)
{
	SetLayoutMode(LayoutMode::Light);

	lcLight* Light = dynamic_cast<lcLight*>(Focus);
	mSelection = Selection;
	mFocusObject = Light;

	lcLightType LightType = lcLightType::Count;
	lcLightAreaShape LightAreaShape = lcLightAreaShape::Count;
/*** LPub3D Mod - LPUB meta properties ***/
	lcLightFormat LightFormat = lcLightFormat::BlenderLight;
/*** LPub3D Mod end ***/
	float PointBlenderRadius = 0.0f, SpotBlenderRadius = 0.0f, DirectionalBlenderAngle = 0.0f;
	float AreaSizeX = 0.0f, AreaSizeY = 0.0f;
	float BlenderPower = 0.0f, POVRayPower = 0.0f;
	float POVRayFadeDistance = 0.0f;
	float POVRayFadePower = 0.0f;
	lcVector3 Position(0.0f, 0.0f, 0.0f);
	lcVector3 Rotation = lcVector3(0.0f, 0.0f, 0.0f);
	float SpotConeAngle = 0.0f, SpotPenumbraAngle = 0.0f, SpotTightness = 0.0f;

	if (Light)
	{
		LightType = Light->GetLightType();
		LightAreaShape = Light->GetAreaShape();
/*** LPub3D Mod - LPUB meta properties ***/
		LightFormat = Light->GetLightFormat();
/*** LPub3D Mod end ***/
/*** LPub3D Mod - Camera Globe, Switch Y and Z axis with -Y(LC -Z) in the up direction ***/
		Position = lcVector3LeoCADToLDraw(Light->GetPosition());
		Rotation = lcVector3LeoCADToLDraw(lcMatrix44ToEulerAngles(Light->GetWorldMatrix()) * LC_RTOD);
/*** LPub3D Mod end ***/
		BlenderPower = Light->GetBlenderPower();
		POVRayPower = Light->GetPOVRayPower();
		POVRayFadeDistance = Light->GetPOVRayFadeDistance();
		POVRayFadePower = Light->GetPOVRayFadePower();
		SpotConeAngle = Light->GetSpotConeAngle();
		SpotPenumbraAngle = Light->GetSpotPenumbraAngle();
		SpotTightness = Light->GetSpotPOVRayTightness();
		PointBlenderRadius = Light->GetPointBlenderRadius();
		SpotBlenderRadius = Light->GetSpotBlenderRadius();
		DirectionalBlenderAngle = Light->GetDirectionalBlenderAngle();
		AreaSizeX = Light->GetAreaSizeX();
		AreaSizeY = Light->GetAreaSizeX();
	}
	else
	{
		bool First = true;
/*** LPub3D Mod - LPUB meta properties ***/
		bool PartialLightType = false, PartialAreaShape = false, PartialLightFormat = false;
/*** LPub3D Mod end ***/

		for (const lcObject* Object : mSelection)
		{
			const lcLight* CurrentLight = dynamic_cast<const lcLight*>(Object);

			if (!CurrentLight)
				continue;

			if (First)
			{
				LightType = CurrentLight->GetLightType();
				LightAreaShape = CurrentLight->GetAreaShape();
/*** LPub3D Mod - LPUB meta properties ***/
				LightFormat = CurrentLight->GetLightFormat();
/*** LPub3D Mod end ***/
				First = false;
			}
			else
			{
				if (LightType != CurrentLight->GetLightType())
					PartialLightType = true;

				if (LightAreaShape != CurrentLight->GetAreaShape())
					PartialAreaShape = true;

/*** LPub3D Mod - LPUB meta properties ***/
				if (LightFormat != CurrentLight->GetLightFormat())
					PartialLightFormat = true;
/*** LPub3D Mod end ***/
			}
		}

		if (PartialLightType)
			LightType = lcLightType::Count;

		if (PartialAreaShape)
			LightAreaShape = lcLightAreaShape::Count;
/*** LPub3D Mod - LPUB meta properties ***/
		if (PartialLightFormat)
			LightFormat = lcLightFormat::Count;
/*** LPub3D Mod end ***/
	}

	UpdateString(lcObjectPropertyId::LightName);
	UpdateStringList(lcObjectPropertyId::LightType);
	UpdateColor(lcObjectPropertyId::LightColor);

	UpdateFloat(lcObjectPropertyId::LightBlenderPower, BlenderPower);
	UpdateFloat(lcObjectPropertyId::LightPOVRayPower, POVRayPower);
	UpdateBool(lcObjectPropertyId::LightCastShadow);

/*** LPub3D Mod - LPUB meta properties ***/
	UpdateStringList(lcObjectPropertyId::LightFormat);

	const bool IsBlenderLightFormat = (LightFormat == lcLightFormat::BlenderLight);
	SetPropertyVisible(lcObjectPropertyId::LightBlenderCutoffDistance, IsBlenderLightFormat);
	SetPropertyVisible(lcObjectPropertyId::LightBlenderDiffuse, IsBlenderLightFormat);
	SetPropertyVisible(lcObjectPropertyId::LightBlenderSpecular, IsBlenderLightFormat);

	if (IsBlenderLightFormat)
	{
		UpdateInteger(lcObjectPropertyId::LightBlenderCutoffDistance);
		UpdateInteger(lcObjectPropertyId::LightBlenderDiffuse);
		UpdateInteger(lcObjectPropertyId::LightBlenderSpecular);
	}
/*** LPub3D Mod end ***/

	UpdateFloat(lcObjectPropertyId::LightPOVRayFadeDistance, POVRayFadeDistance);
	UpdateFloat(lcObjectPropertyId::LightPOVRayFadePower, POVRayFadePower);

	const bool IsPointLight = (LightType == lcLightType::Point);
	SetPropertyVisible(lcObjectPropertyId::LightPointBlenderRadius, IsPointLight);

	if (IsPointLight)
		UpdateFloat(lcObjectPropertyId::LightPointBlenderRadius, PointBlenderRadius);

	const bool IsSpotLight = (LightType == lcLightType::Spot);
	SetPropertyVisible(lcObjectPropertyId::LightSpotBlenderRadius, IsSpotLight);
	SetPropertyVisible(lcObjectPropertyId::LightSpotConeAngle, IsSpotLight);
	SetPropertyVisible(lcObjectPropertyId::LightSpotPenumbraAngle, IsSpotLight);
	SetPropertyVisible(lcObjectPropertyId::LightSpotPOVRayTightness, IsSpotLight);

	if (IsSpotLight)
	{
		UpdateFloat(lcObjectPropertyId::LightSpotBlenderRadius, SpotBlenderRadius);
		UpdateFloat(lcObjectPropertyId::LightSpotConeAngle, SpotConeAngle);
		UpdateFloat(lcObjectPropertyId::LightSpotPenumbraAngle, SpotPenumbraAngle);
		UpdateFloat(lcObjectPropertyId::LightSpotPOVRayTightness, SpotTightness);
	}

	const bool IsDirectionalLight = (LightType == lcLightType::Directional);
	SetPropertyVisible(lcObjectPropertyId::LightDirectionalBlenderAngle, IsDirectionalLight);

	if (IsDirectionalLight)
		UpdateFloat(lcObjectPropertyId::LightDirectionalBlenderAngle, DirectionalBlenderAngle);

	const bool IsAreaLight = (LightType == lcLightType::Area);
	SetPropertyVisible(lcObjectPropertyId::LightAreaShape, IsAreaLight);

	const bool IsSquare = (LightAreaShape == lcLightAreaShape::Square || LightAreaShape == lcLightAreaShape::Disk);
	SetPropertyVisible(lcObjectPropertyId::LightAreaSizeX, IsAreaLight);
	SetPropertyVisible(lcObjectPropertyId::LightAreaSizeY, IsAreaLight && !IsSquare);

	SetPropertyVisible(lcObjectPropertyId::LightAreaPOVRayGridX, IsAreaLight);
	SetPropertyVisible(lcObjectPropertyId::LightAreaPOVRayGridY, IsAreaLight);

	if (IsAreaLight)
	{
		UpdateStringList(lcObjectPropertyId::LightAreaShape);
		UpdateFloat(lcObjectPropertyId::LightAreaSizeX, AreaSizeX);
		UpdateFloat(lcObjectPropertyId::LightAreaSizeY, AreaSizeY);
		UpdateInteger(lcObjectPropertyId::LightAreaPOVRayGridX);
		UpdateInteger(lcObjectPropertyId::LightAreaPOVRayGridY);
	}

	UpdateFloat(lcObjectPropertyId::ObjectPositionX, Position[0]);
	UpdateFloat(lcObjectPropertyId::ObjectPositionY, Position[1]);
	UpdateFloat(lcObjectPropertyId::ObjectPositionZ, Position[2]);

	UpdateFloat(lcObjectPropertyId::ObjectRotationX, Rotation[0]);
	UpdateFloat(lcObjectPropertyId::ObjectRotationY, Rotation[1]);
	UpdateFloat(lcObjectPropertyId::ObjectRotationZ, Rotation[2]);
}

void lcPropertiesWidget::Update(const std::vector<lcObject*>& Selection, lcObject* Focus)
{
	mFocusObject = nullptr;
	mSelection.clear();

	LayoutMode Mode = LayoutMode::Empty;

	if (Focus)
	{
		switch (Focus->GetType())
		{
		case lcObjectType::Piece:
			Mode = LayoutMode::Piece;
			break;

		case lcObjectType::Camera:
			Mode = LayoutMode::Camera;
			break;

		case lcObjectType::Light:
			Mode = LayoutMode::Light;
			break;
		}
	}
	else
	{
		for (size_t ObjectIdx = 0; ObjectIdx < Selection.size(); ObjectIdx++)
		{
			switch (Selection[ObjectIdx]->GetType())
			{
			case lcObjectType::Piece:
				if (Mode == LayoutMode::Empty)
					Mode = LayoutMode::Piece;
				else if (Mode != LayoutMode::Piece)
				{
					Mode = LayoutMode::Multiple;
					ObjectIdx = Selection.size();
				}
				break;

			case lcObjectType::Camera:
				if (Mode == LayoutMode::Empty)
					Mode = LayoutMode::Camera;
				else if (Mode != LayoutMode::Camera)
				{
					Mode = LayoutMode::Multiple;
					ObjectIdx = Selection.size();
				}
				break;

			case lcObjectType::Light:
				if (Mode == LayoutMode::Empty)
					Mode = LayoutMode::Light;
				else if (Mode != LayoutMode::Light)
				{
					Mode = LayoutMode::Multiple;
					ObjectIdx = Selection.size();
				}
				break;
			}
		}
	}

	switch (Mode)
	{
	case LayoutMode::Empty:
	case LayoutMode::Multiple:
	case LayoutMode::Count:
		SetEmpty();
		break;

	case LayoutMode::Piece:
		SetPiece(Selection, Focus);
		break;

	case LayoutMode::Camera:
		SetCamera(Selection, Focus);
		break;

	case LayoutMode::Light:
		SetLight(Selection, Focus);
		break;
	}
}
