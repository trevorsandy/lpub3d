#include "lc_global.h"
#include "lc_qpreferencesdialog.h"
#include "ui_lc_qpreferencesdialog.h"
#include "lc_qutils.h"
#include "lc_categorydialog.h"
#include "lc_library.h"
#include "lc_application.h"
#include "lc_qutils.h"
#include "lc_glextensions.h"
#include "pieceinf.h"
#include "lc_edgecolordialog.h"
#include "lc_blenderpreferences.h"

/*** LPub3D Mod - Native Renderer settings ***/
#include "camera.h"
/*** LPub3D Mod end ***/

/*** LPub3D Mod - Common menus help ***/
#include "lpub_object.h"
#include "commonmenus.h"
/*** LPub3D Mod end ***/

static const char* gLanguageLocales[] =
{
	"", "cs_CZ", "de_DE", "en_US", "fr_FR", "pt_PT", "es_ES"
};

/*** LPub3D Mod - Load Preferences Dialog ***/
lcQPreferencesDialog::lcQPreferencesDialog(QWidget* Parent)
	: QDialog(Parent), ui(new Ui::lcQPreferencesDialog)
{
	ui->setupUi(this);

/*** LPub3D Mod - Common menus help ***/
	setWhatsThis(lpubWT(WT_LPUB3D_PREFERENCES, tr("Visual Editor Preferences")));

	ui->tabCategories->setWhatsThis(lpubWT(              WT_VISUAL_PREFERENCES_CATEGORIES, tr("Categories")));
	ui->tabColors->setWhatsThis(lpubWT(                  WT_VISUAL_PREFERENCES_COLORS, tr("Colors")));
	ui->tabGeneral->setWhatsThis(lpubWT(                 WT_VISUAL_PREFERENCES_GENERAL, tr("General")));
	ui->tabKeyboard->setWhatsThis(lpubWT(                WT_VISUAL_PREFERENCES_KEYBOARD_SHORTCUTS, tr("Keyboard Shortcuts")));
	ui->tabMouse->setWhatsThis(lpubWT(                   WT_VISUAL_PREFERENCES_MOUSE, tr("Mouse Shortcuts")));
	ui->tabRendering->setWhatsThis(lpubWT(               WT_VISUAL_PREFERENCES_RENDERING, tr("Rendering")));
	ui->tabUserInterface->setWhatsThis(lpubWT(           WT_VISUAL_PREFERENCES_USER_INTERFACE, tr("Interface, Base Grid, View Sphere, Preview")));
	ui->tabViews->setWhatsThis(lpubWT(                   WT_VISUAL_PREFERENCES_VIEWS, tr("Views")));

	ui->ApplicationGroup->setWhatsThis(lpubWT(           WT_CONTROL_VISUAL_PREFERENCES_APPLICATION,ui->ApplicationGroup->title()));
	ui->AuthorWidget->setWhatsThis(lpubWT(               WT_CONTROL_VISUAL_PREFERENCES_AUTHOR,tr("Default Author")));
	ui->BackgroundColorGroup->setWhatsThis(lpubWT(       WT_CONTROL_VISUAL_PREFERENCES_BACKGROUND_COLOR,ui->BackgroundColorGroup->title()));
	ui->BaseGridColorGroup->setWhatsThis(lpubWT(         WT_CONTROL_VISUAL_PREFERENCES_BASE_GRID,ui->BaseGridColorGroup->title()));
	ui->BaseGridGroup->setWhatsThis(lpubWT(              WT_CONTROL_VISUAL_PREFERENCES_BASE_GRID_COLOR,ui->BaseGridGroup->title()));
	ui->CameraGroup->setWhatsThis(lpubWT(                WT_CONTROL_LPUB3D_PREFERENCES_CAMERA_DEFAULTS,ui->CameraGroup->title()));
	ui->CategoryEditGroup->setWhatsThis(lpubWT(          WT_CONTROL_VISUAL_PREFERENCES_CATEGORY,ui->CategoryEditGroup->title()));
	ui->CategoryGroup->setWhatsThis(lpubWT(              WT_CONTROL_VISUAL_PREFERENCES_CATEGORY_EDIT,ui->CategoryGroup->title()));
	ui->CustomPathsGroup->setWhatsThis(lpubWT(           WT_CONTROL_VISUAL_PREFERENCES_PATHS,ui->CustomPathsGroup->title()));
	ui->InterfaceColorGroup->setWhatsThis(lpubWT(        WT_CONTROL_VISUAL_PREFERENCES_INTERFACE,ui->InterfaceColorGroup->title()));
	ui->InterfaceGroup->setWhatsThis(lpubWT(             WT_CONTROL_VISUAL_PREFERENCES_INTERFACE_COLOR,ui->InterfaceGroup->title()));
	ui->KeyboardShortcutsTableGroup->setWhatsThis(lpubWT(WT_CONTROL_VISUAL_PREFERENCES_KEYBOARD_SHORTCUTS_TABLE,ui->KeyboardShortcutsTableGroup->title()));
	ui->ModelViewGroup->setWhatsThis(lpubWT(             WT_CONTROL_VISUAL_PREFERENCES_VIEW_SPHERE,ui->ModelViewGroup->title()));
	ui->MouseShortcutGroup->setWhatsThis(lpubWT(         WT_CONTROL_VISUAL_PREFERENCES_MOUSE_SHORTCUTS,ui->MouseShortcutGroup->title()));
	ui->MouseShortcutsTableGroup->setWhatsThis(lpubWT(   WT_CONTROL_VISUAL_PREFERENCES_MOUSE_SHORTCUTS_TABLE,ui->MouseShortcutsTableGroup->title()));
	ui->MouseWidget->setWhatsThis(lpubWT(                WT_CONTROL_VISUAL_PREFERENCES_MOUSE,tr("Mouse Sensitivity")));
	ui->ObjectsColorGroup->setWhatsThis(lpubWT(          WT_CONTROL_VISUAL_PREFERENCES_OBJECTS_COLOR,ui->ObjectsColorGroup->title()));
	ui->PartPreviewGroup->setWhatsThis(lpubWT(           WT_CONTROL_VISUAL_PREFERENCES_PART_PREVIEW,ui->PartPreviewGroup->title()));
	ui->RenderingWidget->setWhatsThis(lpubWT(            WT_CONTROL_LPUB3D_PREFERENCES_RENDERING,tr("Rendering")));
	ui->SettingsWidget->setWhatsThis(lpubWT(             WT_CONTROL_VISUAL_PREFERENCES_GENERAL_SETTINGS,tr("General Settings")));
	ui->shortcutGroup->setWhatsThis(lpubWT(              WT_CONTROL_VISUAL_PREFERENCES_SHORTCUT,ui->shortcutGroup->title()));
	ui->TimelineWidget->setWhatsThis(lpubWT(             WT_CONTROL_VISUAL_PREFERENCES_TIMELINE,tr("Timeline")));
	ui->ViewSphereGroup->setWhatsThis(lpubWT(            WT_CONTROL_VISUAL_PREFERENCES_VIEW_SPHERE,ui->ViewSphereGroup->title()));
	ui->ViewPointAndProjectionGroup->setWhatsThis(lpubWT(WT_CONTROL_LPUB3D_PREFERENCES_VIEWPOINT_AND_PROJECTION, ui->ViewPointAndProjectionGroup->title()));
/*** LPub3D Mod end ***/

/*** LPub3D Mod - suppress Win/macOS preferences dialog settings ***/
/***
#if defined(Q_OS_WIN) || defined(Q_OS_MACOS)
	ui->povrayLabel->hide();
	ui->povrayExecutable->hide();
	ui->povrayExecutableBrowse->hide();
	delete ui->povrayLabel;
	delete ui->povrayLayout;
#endif
***/
/*** LPub3D Mod end ***/

	connect(ui->BackgroundSolidColorButton, &QToolButton::clicked, this, &lcQPreferencesDialog::ColorButtonClicked);
	connect(ui->BackgroundGradient1ColorButton, &QToolButton::clicked, this, &lcQPreferencesDialog::ColorButtonClicked);
	connect(ui->BackgroundGradient2ColorButton, &QToolButton::clicked, this, &lcQPreferencesDialog::ColorButtonClicked);
	connect(ui->ActiveViewColorButton, &QToolButton::clicked, this, &lcQPreferencesDialog::ColorButtonClicked);
	connect(ui->InactiveViewColorButton, &QToolButton::clicked, this, &lcQPreferencesDialog::ColorButtonClicked);
	connect(ui->AxesColorButton, &QToolButton::clicked, this, &lcQPreferencesDialog::ColorButtonClicked);
	connect(ui->TextColorButton, &QToolButton::clicked, this, &lcQPreferencesDialog::ColorButtonClicked);
	connect(ui->MarqueeBorderColorButton, &QToolButton::clicked, this, &lcQPreferencesDialog::ColorButtonClicked);
	connect(ui->MarqueeFillColorButton, &QToolButton::clicked, this, &lcQPreferencesDialog::ColorButtonClicked);
	connect(ui->OverlayColorButton, &QToolButton::clicked, this, &lcQPreferencesDialog::ColorButtonClicked);
	connect(ui->FadeStepsColor, &QToolButton::clicked, this, &lcQPreferencesDialog::ColorButtonClicked);
	connect(ui->HighlightNewPartsColor, &QToolButton::clicked, this, &lcQPreferencesDialog::ColorButtonClicked);
	connect(ui->gridStudColor, &QToolButton::clicked, this, &lcQPreferencesDialog::ColorButtonClicked);
	connect(ui->gridLineColor, &QToolButton::clicked, this, &lcQPreferencesDialog::ColorButtonClicked);
	connect(ui->ViewSphereColorButton, &QToolButton::clicked, this, &lcQPreferencesDialog::ColorButtonClicked);
	connect(ui->ViewSphereTextColorButton, &QToolButton::clicked, this, &lcQPreferencesDialog::ColorButtonClicked);
	connect(ui->ViewSphereHighlightColorButton, &QToolButton::clicked, this, &lcQPreferencesDialog::ColorButtonClicked);
	connect(ui->ObjectSelectedColorButton, &QToolButton::clicked, this, &lcQPreferencesDialog::ColorButtonClicked);
	connect(ui->ObjectFocusedColorButton, &QToolButton::clicked, this, &lcQPreferencesDialog::ColorButtonClicked);
	connect(ui->CameraColorButton, &QToolButton::clicked, this, &lcQPreferencesDialog::ColorButtonClicked);
	connect(ui->LightColorButton, &QToolButton::clicked, this, &lcQPreferencesDialog::ColorButtonClicked);
	connect(ui->ControlPointColorButton, &QToolButton::clicked, this, &lcQPreferencesDialog::ColorButtonClicked);
	connect(ui->ControlPointFocusedColorButton, &QToolButton::clicked, this, &lcQPreferencesDialog::ColorButtonClicked);
/*** LPub3D Mod - Build mod object selected colour ***/
	connect(ui->BMObjectSelectedColorButton, &QToolButton::clicked, this, &lcQPreferencesDialog::ColorButtonClicked);
/*** LPub3D Mod end ***/
	connect(ui->categoriesTree, SIGNAL(itemSelectionChanged()), this, SLOT(updateParts()));
	ui->shortcutEdit->installEventFilter(this);
	connect(ui->commandList, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)), this, SLOT(commandChanged(QTreeWidgetItem*)));
	connect(ui->mouseTree, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)), this, SLOT(MouseTreeItemChanged(QTreeWidgetItem*)));
	connect(ui->HighContrastButton, SIGNAL(clicked()), this, SLOT(AutomateEdgeColor()));
	connect(ui->AutomateEdgeColorButton, SIGNAL(clicked()), this, SLOT(AutomateEdgeColor()));
/*** LPub3D Mod - Reset fade/highlight default colours ***/
	connect(ui->ResetFadeStepsButton, SIGNAL(clicked()), this, SLOT(ResetFadeHighlightColor()));
	connect(ui->ResetHighlightNewPartsButton, SIGNAL(clicked()), this, SLOT(ResetFadeHighlightColor()));
/*** LPub3D Mod end ***/
/*** LPub3D Mod - line width max granularity ***/
	connect(ui->LineWidthMaxGranularityButton, SIGNAL(clicked()), this, SLOT(LineWidthMaxGranularity()));
/*** LPub3D Mod end ***/

/*** LPub3D Mod - set preferences dialog properties ***/
	ui->MinifigSettingsEdit->hide();
	ui->MinifigSettingsBrowseButton->hide();
	ui->MinifigSettingsLabel->hide();
	ui->autoLoadMostRecent->hide();
	ui->lgeoPathBrowse->hide();
	ui->Language->hide();
	ui->checkForUpdates->hide();
	ui->RestoreTabLayout->hide();
	ui->ColorTheme->setDisabled(true);
	ui->label_29->hide();					//label Language
	ui->label_10->hide();					//label check for updates
	ui->fixedDirectionKeys->hide();
	ui->tabWidget->removeTab(4);			//hide tabKeyboard
	ui->tabWidget->removeTab(4);			//hide mouse
/*** LPub3D Mod end ***/

}

/*** LPub3D Mod - Load Preferences Dialog ***/
void lcQPreferencesDialog::setOptions(lcPreferencesDialogOptions* Options)
{
	mSetOptions = true;

	mOptions = Options;

	ui->partsLibrary->setText(mOptions->LibraryPath);
	ui->ColorConfigEdit->setText(mOptions->ColorConfigPath);
	ui->MinifigSettingsEdit->setText(mOptions->MinifigSettingsPath);
	ui->povrayExecutable->setText(mOptions->POVRayPath);
	ui->lgeoPath->setText(mOptions->LGEOPath);
	ui->authorName->setText(mOptions->DefaultAuthor);
	ui->mouseSensitivity->setValue(mOptions->Preferences.mMouseSensitivity);
	const bool ColorThemeBlocked = ui->ColorTheme->blockSignals(true);
	ui->ColorTheme->setCurrentIndex(static_cast<int>(mOptions->Preferences.mColorTheme));
	ui->ColorTheme->blockSignals(ColorThemeBlocked);
	for (unsigned int LanguageIdx = 0; LanguageIdx < LC_ARRAY_COUNT(gLanguageLocales); LanguageIdx++)
	{
		if (mOptions->Language == gLanguageLocales[LanguageIdx])
		{
			ui->Language->setCurrentIndex(LanguageIdx);
			break;
		}
	}
	ui->checkForUpdates->setCurrentIndex(mOptions->CheckForUpdates);
	ui->fixedDirectionKeys->setChecked(mOptions->Preferences.mFixedAxes);
	ui->autoLoadMostRecent->setChecked(mOptions->Preferences.mAutoLoadMostRecent);
	ui->RestoreTabLayout->setChecked(mOptions->Preferences.mRestoreTabLayout);
	ui->AutomateEdgeColor->setChecked(mOptions->Preferences.mAutomateEdgeColor);

	ui->antiAliasing->setChecked(mOptions->AASamples != 1);
	if (mOptions->AASamples == 8)
		ui->antiAliasingSamples->setCurrentIndex(2);
	else if (mOptions->AASamples == 4)
		ui->antiAliasingSamples->setCurrentIndex(1);
	else
		ui->antiAliasingSamples->setCurrentIndex(0);
	ui->edgeLines->setChecked(mOptions->Preferences.mDrawEdgeLines);
	ui->ConditionalLinesCheckBox->setChecked(mOptions->Preferences.mDrawConditionalLines);
/*** LPub3D Mod - parts load order ***/
	ui->PreferOfficialParts->setChecked(mOptions->Preferences.mPreferOfficialParts);
	ui->PreferOfficialParts->setEnabled(mOptions->HasUnofficialParts);
/*** LPub3D Mod - ***/

/*** LPub3D Mod - Zoom extents ***/
	ui->ZoomExtentCombo->setCurrentIndex(mOptions->Preferences.mZoomExtents);
/*** LPub3D Mod - ***/

	if (!gSupportsShaderObjects)
	{
		ui->ConditionalLinesCheckBox->setChecked(false);
		ui->ConditionalLinesCheckBox->setEnabled(false);
	}

/*** LPub3D Mod - line width max granularity ***/
	float Max = mOptions->Preferences.mLineWidthMaxGranularity;
/*** LPub3D Mod - ***/
#ifndef LC_OPENGLES
	if (QSurfaceFormat::defaultFormat().samples() > 1)
	{
		glGetFloatv(GL_SMOOTH_LINE_WIDTH_RANGE, mLineWidthRange);
		glGetFloatv(GL_SMOOTH_LINE_WIDTH_GRANULARITY, &mLineWidthGranularity);
	}
	else
#endif
	{
		glGetFloatv(GL_ALIASED_LINE_WIDTH_RANGE, mLineWidthRange);
/*** LPub3D Mod - line width max granularity ***/
		mLineWidthGranularity = Max;
/*** LPub3D Mod - ***/
	}

/*** LPub3D Mod - line width max granularity ***/
	ui->LineWidthSlider->setRange(0, (mLineWidthRange[1] - mLineWidthRange[0]) / qMax(Max, mLineWidthGranularity));
/*** LPub3D Mod - ***/
	ui->LineWidthSlider->setValue((mOptions->Preferences.mLineWidth - mLineWidthRange[0]) / mLineWidthGranularity);

	ui->MeshLOD->setChecked(mOptions->Preferences.mAllowLOD);

	ui->MeshLODSlider->setRange(0, 1500.0f / mMeshLODMultiplier);
	ui->MeshLODSlider->setValue(mOptions->Preferences.mMeshLODDistance / mMeshLODMultiplier);

	ui->FadeSteps->setChecked(mOptions->Preferences.mFadeSteps);
	ui->HighlightNewParts->setChecked(mOptions->Preferences.mHighlightNewParts);
	ui->gridStuds->setChecked(mOptions->Preferences.mDrawGridStuds);
	ui->gridLines->setChecked(mOptions->Preferences.mDrawGridLines);
	ui->gridLineSpacing->setText(QString::number(mOptions->Preferences.mGridLineSpacing));
	ui->GridOriginCheckBox->setChecked(mOptions->Preferences.mDrawGridOrigin);
	ui->AxisIconCheckBox->setChecked(mOptions->Preferences.mDrawAxes);

	ui->AxisIconLocationCombo->setCurrentIndex((int)mOptions->Preferences.mAxisIconLocation);

	if (!mOptions->Preferences.mBackgroundGradient)
		ui->BackgroundSolidRadio->setChecked(true);
	else
		ui->BackgroundGradientRadio->setChecked(true);

	ui->ViewSphereLocationCombo->setCurrentIndex((int)mOptions->Preferences.mViewSphereLocation);

	if (mOptions->Preferences.mViewSphereEnabled)
	{
		switch (mOptions->Preferences.mViewSphereSize)
		{
		case 200:
			ui->ViewSphereSizeCombo->setCurrentIndex(3);
			break;
		case 100:
			ui->ViewSphereSizeCombo->setCurrentIndex(2);
			break;
		case 50:
			ui->ViewSphereSizeCombo->setCurrentIndex(1);
			break;
		default:
			ui->ViewSphereSizeCombo->setCurrentIndex(0);
			break;
		}
	}
	else
		ui->ViewSphereSizeCombo->setCurrentIndex(0);

	ui->PreviewAxisIconCheckBox->setChecked(mOptions->Preferences.mDrawPreviewAxis);

/*** LPub3D Mod - preview widget for LPub3D ***/
	ui->PreviewLocationCombo->setCurrentIndex((int)mOptions->Preferences.mPreviewLocation);

	ui->PreviewPositionCombo->setCurrentIndex((int)mOptions->Preferences.mPreviewPosition);

	if (mOptions->Preferences.mPreviewEnabled)
	{
		switch (mOptions->Preferences.mPreviewSize)
		{
		case 400:
			ui->PreviewSizeCombo->setCurrentIndex(2);
			break;
		case 300:
			ui->PreviewSizeCombo->setCurrentIndex(1);
			break;
		default: /*Disabled/Docked*/
			ui->PreviewSizeCombo->setCurrentIndex(0);
			break;
		}
		ui->PreviewAxisIconCheckBox->setEnabled(true);
	}
	else
		ui->PreviewSizeCombo->setCurrentIndex(0);
/*** LPub3D Mod end ***/

	ui->PreviewViewSphereLocationCombo->setCurrentIndex((int)mOptions->Preferences.mPreviewViewSphereLocation);

	if (mOptions->Preferences.mPreviewViewSphereEnabled)
	{
		switch (mOptions->Preferences.mPreviewViewSphereSize)
		{
		case 100:
			ui->PreviewViewSphereSizeCombo->setCurrentIndex(3);
			break;
		case 75:
			ui->PreviewViewSphereSizeCombo->setCurrentIndex(2);
			break;
		case 50:
			ui->PreviewViewSphereSizeCombo->setCurrentIndex(1);
			break;
		default:
			ui->PreviewViewSphereSizeCombo->setCurrentIndex(0);
			break;
		}
	}
	else
		ui->PreviewViewSphereSizeCombo->setCurrentIndex(0);

	if (!lcGetPiecesLibrary()->SupportsStudStyle())
		ui->studStyleCombo->setEnabled(false);

	ui->studStyleCombo->setCurrentIndex(static_cast<int>(mOptions->StudStyle));

	if (!gSupportsShaderObjects)
		ui->ShadingMode->removeItem(static_cast<int>(lcShadingMode::DefaultLights));
	ui->ShadingMode->setCurrentIndex(static_cast<int>(mOptions->Preferences.mShadingMode));

	auto SetButtonPixmap = [](quint32 Color, QToolButton* Button)
	{
		QPixmap Pixmap(12, 12);

		Pixmap.fill(QColor(LC_RGBA_RED(Color), LC_RGBA_GREEN(Color), LC_RGBA_BLUE(Color)));
		Button->setIcon(Pixmap);
	};

	SetButtonPixmap(mOptions->Preferences.mBackgroundSolidColor, ui->BackgroundSolidColorButton);
	SetButtonPixmap(mOptions->Preferences.mBackgroundGradientColorTop, ui->BackgroundGradient1ColorButton);
	SetButtonPixmap(mOptions->Preferences.mBackgroundGradientColorBottom, ui->BackgroundGradient2ColorButton);
	SetButtonPixmap(mOptions->Preferences.mAxesColor, ui->AxesColorButton);
	SetButtonPixmap(mOptions->Preferences.mTextColor, ui->TextColorButton);
	SetButtonPixmap(mOptions->Preferences.mMarqueeBorderColor, ui->MarqueeBorderColorButton);
	SetButtonPixmap(mOptions->Preferences.mMarqueeFillColor, ui->MarqueeFillColorButton);
	SetButtonPixmap(mOptions->Preferences.mOverlayColor, ui->OverlayColorButton);
	SetButtonPixmap(mOptions->Preferences.mActiveViewColor, ui->ActiveViewColorButton);
	SetButtonPixmap(mOptions->Preferences.mInactiveViewColor, ui->InactiveViewColorButton);
	SetButtonPixmap(mOptions->Preferences.mFadeStepsColor, ui->FadeStepsColor);
	SetButtonPixmap(mOptions->Preferences.mHighlightNewPartsColor, ui->HighlightNewPartsColor);
	SetButtonPixmap(mOptions->Preferences.mGridStudColor, ui->gridStudColor);
	SetButtonPixmap(mOptions->Preferences.mGridLineColor, ui->gridLineColor);
	SetButtonPixmap(mOptions->Preferences.mViewSphereColor, ui->ViewSphereColorButton);
	SetButtonPixmap(mOptions->Preferences.mViewSphereTextColor, ui->ViewSphereTextColorButton);
	SetButtonPixmap(mOptions->Preferences.mViewSphereHighlightColor, ui->ViewSphereHighlightColorButton);
	SetButtonPixmap(mOptions->Preferences.mObjectSelectedColor, ui->ObjectSelectedColorButton);
	SetButtonPixmap(mOptions->Preferences.mObjectFocusedColor, ui->ObjectFocusedColorButton);
	SetButtonPixmap(mOptions->Preferences.mCameraColor, ui->CameraColorButton);
	SetButtonPixmap(mOptions->Preferences.mLightColor, ui->LightColorButton);
	SetButtonPixmap(mOptions->Preferences.mControlPointColor, ui->ControlPointColorButton);
	SetButtonPixmap(mOptions->Preferences.mControlPointFocusedColor, ui->ControlPointFocusedColorButton);
/*** LPub3D Mod - Build mod object selected colour ***/
	SetButtonPixmap(mOptions->Preferences.mBMObjectSelectedColor, ui->BMObjectSelectedColorButton);
/*** LPub3D Mod end ***/

/*** LPub3D Mod - Native Renderer settings ***/
	ui->ProjectionCombo->setCurrentIndex(mOptions->Preferences.mNativeProjection);
	ui->ViewpointsCombo->setCurrentIndex(mOptions->Preferences.mNativeViewpoint);
	LPub::ViewpointsComboSaveIndex = mOptions->Preferences.mNativeViewpoint;
/*** LPub3D Mod end ***/

	on_studStyleCombo_currentIndexChanged(ui->studStyleCombo->currentIndex());
	on_antiAliasing_toggled();
	on_AutomateEdgeColor_toggled();
	on_edgeLines_toggled();
	on_LineWidthSlider_valueChanged();
	on_MeshLODSlider_valueChanged();
	on_FadeSteps_toggled();
	on_HighlightNewParts_toggled();
	on_gridStuds_toggled();
	on_gridLines_toggled();
/*** LPub3D Mod - Enable background colour buttons ***/
	on_BackgroundSolidRadio_toggled(ui->BackgroundSolidRadio->isChecked());
	on_BackgroundGradientRadio_toggled(ui->BackgroundGradientRadio->isChecked());
/*** LPub3D Mod end ***/
	on_ViewSphereSizeCombo_currentIndexChanged(ui->ViewSphereSizeCombo->currentIndex());

	on_PreviewViewSphereSizeCombo_currentIndexChanged(ui->PreviewViewSphereSizeCombo->currentIndex());
/*** LPub3D Mod - preview widget for LPub3D ***/
	on_PreviewSizeCombo_currentIndexChanged(ui->PreviewSizeCombo->currentIndex());
	on_PreviewPositionCombo_currentIndexChanged(ui->PreviewPositionCombo->currentIndex());
	ui->PreviewLocationCombo->setEnabled(
				ui->PreviewSizeCombo->currentIndex() != 0 &&
				ui->PreviewPositionCombo->currentIndex() != 0);
/*** LPub3D Mod end ***/

	updateCategories();
	ui->categoriesTree->setCurrentItem(ui->categoriesTree->topLevelItem(0));

	updateCommandList();
	new lcQTreeWidgetColumnStretcher(ui->commandList, 0);
	commandChanged(nullptr);

	UpdateMouseTree();
	ui->mouseTree->header()->setSectionResizeMode(0, QHeaderView::Stretch);
	ui->mouseTree->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
	ui->mouseTree->header()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
	MouseTreeItemChanged(nullptr);

/*** LPub3D Mod - Update Default Camera ***/
	QPalette readOnlyPalette = QApplication::palette();
	if (mOptions->Preferences.mColorTheme == lcColorTheme::Dark)
		readOnlyPalette.setColor(QPalette::Base,QColor("#3E3E3E")); // THEME_DARK_PALETTE_MIDLIGHT
	else
		readOnlyPalette.setColor(QPalette::Base,QColor("#AEADAC")); // THEME_DEFAULT_PALETTE_LIGHT
	readOnlyPalette.setColor(QPalette::Text,QColor("#808080"));		// LPUB3D_DISABLED_TEXT_COLOUR

	ui->defaultCameraProperties->setChecked(mOptions->Preferences.mDefaultCameraProperties);
	ui->cameraDefaultDistanceFactor->setValue(qreal(mOptions->Preferences.mDDF));
	ui->cameraDefaultPosition->setValue(qreal(mOptions->Preferences.mCDP));
	ui->cameraFoV->setValue(qreal(mOptions->Preferences.mCFoV));
	ui->cameraNearPlane->setValue(qreal(mOptions->Preferences.mCNear));
	ui->cameraFarPlane->setValue(qreal(mOptions->Preferences.mCFar));
	ui->cameraDistanceFactor->setText(QString::number(qreal(mOptions->Preferences.mCDP) /
													  -qreal(mOptions->Preferences.mDDF)));

	connect(ui->resetDefaultDistanceFactor, SIGNAL(clicked()), this, SLOT(cameraPropertyReset()));
	connect(ui->resetDefaultPosition, SIGNAL(clicked()), this, SLOT(cameraPropertyReset()));
	connect(ui->resetFoV, SIGNAL(clicked()), this, SLOT(cameraPropertyReset()));
	connect(ui->resetNearPlane, SIGNAL(clicked()), this, SLOT(cameraPropertyReset()));
	connect(ui->resetFarPlane, SIGNAL(clicked()), this, SLOT(cameraPropertyReset()));
/*** LPub3D Mod end ***/

/*** LPub3D Mod - Timeline part icons ***/
	bool preferredIcons = mOptions->Preferences.mViewPieceIcons;
	ui->viewPieceIconsRadio->setChecked(preferredIcons);
	ui->viewColorIconsRadio->setChecked(!preferredIcons);
/*** LPub3D Mod end ***/

/*** LPub3D Mod - lpub fade highlight ***/
	ui->LPubFadeHighlight->setChecked(mOptions->Preferences.mLPubFadeHighlight);
/*** LPub3D Mod end ***/

/*** LPub3D Mod - set preferences dialog properties ***/
	ui->authorName->setReadOnly(true);
	ui->authorName->setPalette(readOnlyPalette);
	ui->partsLibrary->setReadOnly(true);
	ui->partsLibrary->setPalette(readOnlyPalette);
	ui->partsLibraryBrowse->hide();
	ui->partsArchiveBrowse->hide();
	ui->cameraDistanceFactor->setReadOnly(true);
	ui->cameraDistanceFactor->setPalette(readOnlyPalette);
	ui->ColorConfigEdit->setReadOnly(true);
	ui->ColorConfigEdit->setPalette(readOnlyPalette);
	ui->ColorConfigBrowseButton->hide();
	ui->povrayExecutable->setReadOnly(true);
	ui->povrayExecutable->setPalette(readOnlyPalette);
	ui->povrayExecutableBrowse->hide();
	ui->lgeoPath->setReadOnly(true);
	ui->lgeoPath->setPalette(readOnlyPalette);
/*** LPub3D Mod end ***/

	mSetOptions = false;
}
/*** LPub3D Mod end ***/

lcQPreferencesDialog::~lcQPreferencesDialog()
{
	delete ui;
}

void lcQPreferencesDialog::accept()
{
	int gridLineSpacing = ui->gridLineSpacing->text().toInt();
	if (gridLineSpacing < 1)
	{
/*** LPub3D Mod - set Visual Editor label ***/
		QMessageBox::information(this, "Visual Editor", tr("Grid spacing must be greater than 0."));
/*** LPub3D Mod end ***/
		return;
	}

	mOptions->LibraryPath = ui->partsLibrary->text();
	mOptions->MinifigSettingsPath = ui->MinifigSettingsEdit->text();
	mOptions->ColorConfigPath = ui->ColorConfigEdit->text();
	mOptions->POVRayPath = ui->povrayExecutable->text();
	mOptions->LGEOPath = ui->lgeoPath->text();
	mOptions->DefaultAuthor = ui->authorName->text();
	mOptions->Preferences.mMouseSensitivity = ui->mouseSensitivity->value();
	mOptions->Preferences.mColorTheme = static_cast<lcColorTheme>(ui->ColorTheme->currentIndex());
	mOptions->Preferences.mAutomateEdgeColor = ui->AutomateEdgeColor->isChecked();
/*** LPub3D Mod - parts load order ***/
	mOptions->Preferences.mPreferOfficialParts = ui->PreferOfficialParts->isChecked();
/*** LPub3D Mod - ***/

/*** LPub3D Mod - Zoom extents ***/
	mOptions->Preferences.mZoomExtents = ui->ZoomExtentCombo->currentIndex();
/*** LPub3D Mod - ***/

	int Language = ui->Language->currentIndex();
	if (Language < 0 || Language > static_cast<int>(LC_ARRAY_COUNT(gLanguageLocales)))
		Language = 0;
	mOptions->Language = gLanguageLocales[Language];

	mOptions->CheckForUpdates = ui->checkForUpdates->currentIndex();
	mOptions->Preferences.mFixedAxes = ui->fixedDirectionKeys->isChecked();
	mOptions->Preferences.mAutoLoadMostRecent = ui->autoLoadMostRecent->isChecked();
	mOptions->Preferences.mRestoreTabLayout = ui->RestoreTabLayout->isChecked();

	if (!ui->antiAliasing->isChecked())
		mOptions->AASamples = 1;
	else if (ui->antiAliasingSamples->currentIndex() == 2)
		mOptions->AASamples = 8;
	else if (ui->antiAliasingSamples->currentIndex() == 1)
		mOptions->AASamples = 4;
	else
		mOptions->AASamples = 2;

	mOptions->Preferences.mDrawEdgeLines = ui->edgeLines->isChecked();
	mOptions->Preferences.mDrawConditionalLines = ui->ConditionalLinesCheckBox->isChecked();
	mOptions->Preferences.mLineWidth = mLineWidthRange[0] + static_cast<float>(ui->LineWidthSlider->value()) * mLineWidthGranularity;
	mOptions->Preferences.mAllowLOD = ui->MeshLOD->isChecked();
	mOptions->Preferences.mMeshLODDistance = ui->MeshLODSlider->value() * mMeshLODMultiplier;
	mOptions->Preferences.mFadeSteps = ui->FadeSteps->isChecked();
	mOptions->Preferences.mHighlightNewParts = ui->HighlightNewParts->isChecked();

	mOptions->Preferences.mDrawGridStuds = ui->gridStuds->isChecked();
	mOptions->Preferences.mDrawGridLines = ui->gridLines->isChecked();
	mOptions->Preferences.mGridLineSpacing = gridLineSpacing;
	mOptions->Preferences.mDrawGridOrigin = ui->GridOriginCheckBox->isChecked();

	mOptions->Preferences.mBackgroundGradient = ui->BackgroundGradientRadio->isChecked();
	mOptions->Preferences.mDrawAxes = ui->AxisIconCheckBox->isChecked();
	mOptions->Preferences.mAxisIconLocation = (lcAxisIconLocation)ui->AxisIconLocationCombo->currentIndex();
	mOptions->Preferences.mViewSphereEnabled = ui->ViewSphereSizeCombo->currentIndex() > 0;
	mOptions->Preferences.mViewSphereLocation = (lcViewSphereLocation)ui->ViewSphereLocationCombo->currentIndex();

	switch (ui->ViewSphereSizeCombo->currentIndex())
	{
	case 3:
		mOptions->Preferences.mViewSphereSize = 200;
		break;
	case 2:
		mOptions->Preferences.mViewSphereSize = 100;
		break;
	case 1:
		mOptions->Preferences.mViewSphereSize = 50;
		break;
	}

	mOptions->Preferences.mShadingMode = (lcShadingMode)ui->ShadingMode->currentIndex();

	mOptions->StudStyle = static_cast<lcStudStyle>(ui->studStyleCombo->currentIndex());

	mOptions->Preferences.mDrawPreviewAxis = ui->PreviewAxisIconCheckBox->isChecked();
	mOptions->Preferences.mPreviewViewSphereEnabled = ui->PreviewViewSphereSizeCombo->currentIndex() > 0;
	mOptions->Preferences.mPreviewViewSphereLocation = (lcViewSphereLocation)ui->PreviewViewSphereLocationCombo->currentIndex();

/*** LPub3D Mod - preview widget for LPub3D ***/
	mOptions->Preferences.mPreviewEnabled = ui->PreviewSizeCombo->currentIndex() > 0;

	mOptions->Preferences.mPreviewLocation = (lcPreviewLocation)ui->PreviewLocationCombo->currentIndex();

	mOptions->Preferences.mPreviewPosition = (lcPreviewPosition)ui->PreviewPositionCombo->currentIndex();

	switch (ui->PreviewSizeCombo->currentIndex())
	{
	case 2:
		mOptions->Preferences.mPreviewSize = 400;
		break;
	case 1:
		mOptions->Preferences.mPreviewSize = 300;
		break;
	}
/*** LPub3D Mod end ***/

	switch (ui->PreviewViewSphereSizeCombo->currentIndex())
	{
	case 3:
		mOptions->Preferences.mPreviewViewSphereSize = 100;
		break;
	case 2:
		mOptions->Preferences.mPreviewViewSphereSize = 75;
		break;
	case 1:
		mOptions->Preferences.mPreviewViewSphereSize = 50;
		break;
	}

/*** LPub3D Mod - Update Default Camera ***/
	mOptions->Preferences.mDefaultCameraProperties = ui->defaultCameraProperties->isChecked();
	mOptions->Preferences.mDDF = float(ui->cameraDefaultDistanceFactor->value());
	mOptions->Preferences.mCDP = float(ui->cameraDefaultPosition->value());
	mOptions->Preferences.mCFoV = float(ui->cameraFoV->value());
	mOptions->Preferences.mCNear = float(ui->cameraNearPlane->value());
	mOptions->Preferences.mCFar = float(ui->cameraFarPlane->value());
/*** LPub3D Mod end ***/

/*** LPub3D Mod - Native Renderer settings ***/
	mOptions->Preferences.mNativeViewpoint = ui->ViewpointsCombo->currentIndex();
	mOptions->Preferences.mNativeProjection = ui->ProjectionCombo->currentIndex();
/*** LPub3D Mod end ***/

/*** LPub3D Mod - Timeline part icons ***/
	mOptions->Preferences.mViewPieceIcons = ui->viewPieceIconsRadio->isChecked();
/*** LPub3D Mod end ***/

/*** LPub3D Mod - lpub fade highlight ***/
	mOptions->Preferences.mLPubFadeHighlight = ui->LPubFadeHighlight->isChecked();
/*** LPub3D Mod end ***/

	QDialog::accept();
}

void lcQPreferencesDialog::on_partsLibraryBrowse_clicked()
{
	QString result = QFileDialog::getExistingDirectory(this, tr("Select Parts Library Folder"), ui->partsLibrary->text());

	if (!result.isEmpty())
		ui->partsLibrary->setText(QDir::toNativeSeparators(result));
}

void lcQPreferencesDialog::on_partsArchiveBrowse_clicked()
{
	QString result = QFileDialog::getOpenFileName(this, tr("Select Parts Library Archive"), ui->partsLibrary->text(), tr("Supported Archives (*.zip *.bin);;All Files (*.*)"));

	if (!result.isEmpty())
		ui->partsLibrary->setText(QDir::toNativeSeparators(result));
}

/*** LPub3D Mod - disabled, there is not BlenderAddonSettingsButton ***/
/*
void lcQPreferencesDialog::on_BlenderAddonSettingsButton_clicked()
{
	int Width = 1280;
	int Height = 720;
	double Scale = 1.0f;

	lcBlenderPreferencesDialog::GetBlenderPreferences(
		Width,
		Height,
		Scale,
		this);

	Q_UNUSED(Width)
	Q_UNUSED(Height)
	Q_UNUSED(Scale)
}
*/
/*** LPub3D Mod end ***/

void lcQPreferencesDialog::on_ColorConfigBrowseButton_clicked()
{
	QString Result = QFileDialog::getOpenFileName(this, tr("Select Color Configuration File"), ui->ColorConfigEdit->text(), tr("Settings Files (*.ldr);;All Files (*.*)"));

	if (!Result.isEmpty())
		ui->ColorConfigEdit->setText(QDir::toNativeSeparators(Result));
}

void lcQPreferencesDialog::on_MinifigSettingsBrowseButton_clicked()
{
	QString Result = QFileDialog::getOpenFileName(this, tr("Select Minifig Settings File"), ui->MinifigSettingsEdit->text(), tr("Settings Files (*.ini);;All Files (*.*)"));

	if (!Result.isEmpty())
		ui->MinifigSettingsEdit->setText(QDir::toNativeSeparators(Result));
}

void lcQPreferencesDialog::on_povrayExecutableBrowse_clicked()
{
#ifdef Q_OS_WIN
	QString filter(tr("Executable Files (*.exe);;All Files (*.*)"));
#else
	QString filter(tr("All Files (*.*)"));
#endif

	QString result = QFileDialog::getOpenFileName(this, tr("Select POV-Ray Executable"), ui->povrayExecutable->text(), filter);

	if (!result.isEmpty())
		ui->povrayExecutable->setText(QDir::toNativeSeparators(result));
}

void lcQPreferencesDialog::on_lgeoPathBrowse_clicked()
{
	QString result = QFileDialog::getExistingDirectory(this, tr("Open LGEO Folder"), ui->lgeoPath->text());

	if (!result.isEmpty())
		ui->lgeoPath->setText(QDir::toNativeSeparators(result));
}

void lcQPreferencesDialog::on_ColorTheme_currentIndexChanged(int Index)
{
	Q_UNUSED(Index);

	if (QMessageBox::question(this, tr("Reset Colors"), tr("Would you like to also reset the interface colors to match the color theme?")) == QMessageBox::Yes)
		mOptions->Preferences.SetInterfaceColors(static_cast<lcColorTheme>(ui->ColorTheme->currentIndex()));
}

void lcQPreferencesDialog::ColorButtonClicked()
{
	QObject* Button = sender();
	QString Title;
	quint32* Color = nullptr;
	QColorDialog::ColorDialogOptions DialogOptions;

	if (Button == ui->BackgroundSolidColorButton)
	{
		Color = &mOptions->Preferences.mBackgroundSolidColor;
		Title = tr("Select Background Color");
	}
	else if (Button == ui->BackgroundGradient1ColorButton)
	{
		Color = &mOptions->Preferences.mBackgroundGradientColorTop;
		Title = tr("Select Gradient Top Color");
	}
	else if (Button == ui->BackgroundGradient2ColorButton)
	{
		Color = &mOptions->Preferences.mBackgroundGradientColorBottom;
		Title = tr("Select Gradient Bottom Color");
	}
	else if (Button == ui->AxesColorButton)
	{
		Color = &mOptions->Preferences.mAxesColor;
		Title = tr("Select Axes Color");
	}
	else if (Button == ui->TextColorButton)
	{
		Color = &mOptions->Preferences.mTextColor;
		Title = tr("Select Text Color");
	}
	else if (Button == ui->MarqueeBorderColorButton)
	{
		Color = &mOptions->Preferences.mMarqueeBorderColor;
		Title = tr("Select Marquee Border Color");
	}
	else if (Button == ui->MarqueeFillColorButton)
	{
		Color = &mOptions->Preferences.mMarqueeFillColor;
		Title = tr("Select Marquee Fill Color");
		DialogOptions = QColorDialog::ShowAlphaChannel;
	}
	else if (Button == ui->OverlayColorButton)
	{
		Color = &mOptions->Preferences.mOverlayColor;
		Title = tr("Select Overlay Color");
	}
	else if (Button == ui->ActiveViewColorButton)
	{
		Color = &mOptions->Preferences.mActiveViewColor;
		Title = tr("Select Active View Color");
	}
	else if (Button == ui->InactiveViewColorButton)
	{
		Color = &mOptions->Preferences.mInactiveViewColor;
		Title = tr("Select Inactive View Color");
	}
	else if (Button == ui->FadeStepsColor)
	{
		Color = &mOptions->Preferences.mFadeStepsColor;
		Title = tr("Select Fade Color");
		DialogOptions = QColorDialog::ShowAlphaChannel;
	}
	else if (Button == ui->HighlightNewPartsColor)
	{
		Color = &mOptions->Preferences.mHighlightNewPartsColor;
		Title = tr("Select Highlight Color");
		DialogOptions = QColorDialog::ShowAlphaChannel;
	}
	else if (Button == ui->gridStudColor)
	{
		Color = &mOptions->Preferences.mGridStudColor;
		Title = tr("Select Grid Stud Color");
		DialogOptions = QColorDialog::ShowAlphaChannel;
	}
	else if (Button == ui->gridLineColor)
	{
		Color = &mOptions->Preferences.mGridLineColor;
		Title = tr("Select Grid Line Color");
	}
	else if (Button == ui->ViewSphereColorButton)
	{
		Color = &mOptions->Preferences.mViewSphereColor;
		Title = tr("Select View Sphere Color");
	}
	else if (Button == ui->ViewSphereTextColorButton)
	{
		Color = &mOptions->Preferences.mViewSphereTextColor;
		Title = tr("Select View Sphere Text Color");
	}
	else if (Button == ui->ViewSphereHighlightColorButton)
	{
		Color = &mOptions->Preferences.mViewSphereHighlightColor;
		Title = tr("Select View Sphere Highlight Color");
	}
	else if (Button == ui->ObjectSelectedColorButton)
	{
		Color = &mOptions->Preferences.mObjectSelectedColor;
		Title = tr("Select Object Selected Color");
	}
	else if (Button == ui->ObjectFocusedColorButton)
	{
		Color = &mOptions->Preferences.mObjectFocusedColor;
		Title = tr("Select Object Focused Color");
	}
	else if (Button == ui->CameraColorButton)
	{
		Color = &mOptions->Preferences.mCameraColor;
		Title = tr("Select Camera Color");
	}
	else if (Button == ui->LightColorButton)
	{
		Color = &mOptions->Preferences.mLightColor;
		Title = tr("Select Light Color");
	}
	else if (Button == ui->ControlPointColorButton)
	{
		Color = &mOptions->Preferences.mControlPointColor;
		Title = tr("Select Control Point Color");
		DialogOptions = QColorDialog::ShowAlphaChannel;
	}
	else if (Button == ui->ControlPointFocusedColorButton)
	{
		Color = &mOptions->Preferences.mControlPointFocusedColor;
		Title = tr("Select Control Point Focused Color");
		DialogOptions = QColorDialog::ShowAlphaChannel;
	}
/*** LPub3D Mod - Build mod object selected colour ***/
	else if (Button == ui->BMObjectSelectedColorButton)
	{
		Color = &mOptions->Preferences.mBMObjectSelectedColor;
		Title = tr("Select Build Mod Object Selected Color");
		DialogOptions = QColorDialog::ShowAlphaChannel;
	}
/*** LPub3D Mod end ***/
	else
		return;

	QColor oldColor = QColor(LC_RGBA_RED(*Color), LC_RGBA_GREEN(*Color), LC_RGBA_BLUE(*Color), LC_RGBA_ALPHA(*Color));
	QColor newColor = QColorDialog::getColor(oldColor, this, Title, DialogOptions);

	if (newColor == oldColor || !newColor.isValid())
		return;

	*Color = LC_RGBA(newColor.red(), newColor.green(), newColor.blue(), newColor.alpha());

	QPixmap pix(12, 12);

	newColor.setAlpha(255);
	pix.fill(newColor);
	((QToolButton*)Button)->setIcon(pix);
}

/*** LPub3D Mod - Reset fade/highlight default colours ***/
void lcQPreferencesDialog::ResetFadeHighlightColor()
{
	quint32* Color = nullptr;
	QPixmap pix(12, 12);
	QColor resetColor;

	if (sender() == ui->ResetFadeStepsButton)
	{
		Color = &mOptions->Preferences.mFadeStepsColor;
	   *Color = LC_RGBA(128, 128, 128, 128);
		resetColor = QColor(LC_RGBA_RED(*Color), LC_RGBA_GREEN(*Color), LC_RGBA_BLUE(*Color), LC_RGBA_ALPHA(*Color));
		pix.fill(resetColor);
		ui->FadeStepsColor->setIcon(pix);
	}
	else if (sender() == ui->ResetHighlightNewPartsButton)
	{
		Color = &mOptions->Preferences.mHighlightNewPartsColor;
	   *Color = LC_RGBA(255, 242, 0, 192);
		resetColor = QColor(LC_RGBA_RED(*Color), LC_RGBA_GREEN(*Color), LC_RGBA_BLUE(*Color), LC_RGBA_ALPHA(*Color));
		pix.fill(resetColor);
		ui->HighlightNewPartsColor->setIcon(pix);
	}
}
/*** LPub3D Mod end ***/

/*** LPub3D Mod - Enable background colour buttons ***/
void lcQPreferencesDialog::on_BackgroundSolidRadio_toggled(bool checked)
{
	ui->BackgroundSolidColorButton->setEnabled(checked);
	ui->BackgroundGradient1ColorButton->setEnabled(!checked);
	ui->BackgroundGradient2ColorButton->setEnabled(!checked);
}

void lcQPreferencesDialog::on_BackgroundGradientRadio_toggled(bool checked)
{
	ui->BackgroundGradient1ColorButton->setEnabled(checked);
	ui->BackgroundGradient2ColorButton->setEnabled(checked);
	ui->BackgroundSolidColorButton->setEnabled(!checked);
}
/*** LPub3D Mod end ***/

void lcQPreferencesDialog::on_antiAliasing_toggled()
{
	ui->antiAliasingSamples->setEnabled(ui->antiAliasing->isChecked());
}

void lcQPreferencesDialog::on_edgeLines_toggled()
{
	const bool Enable = ui->edgeLines->isChecked() || ui->ConditionalLinesCheckBox->isChecked();

	ui->LineWidthSlider->setEnabled(Enable);
	ui->LineWidthLabel->setEnabled(Enable);
}

void lcQPreferencesDialog::on_ConditionalLinesCheckBox_toggled()
{
	const bool Enable = ui->edgeLines->isChecked() || ui->ConditionalLinesCheckBox->isChecked();

	ui->LineWidthSlider->setEnabled(Enable);
	ui->LineWidthLabel->setEnabled(Enable);
}

void lcQPreferencesDialog::on_LineWidthSlider_valueChanged()
{
	float Value = mLineWidthRange[0] + static_cast<float>(ui->LineWidthSlider->value()) * mLineWidthGranularity;
	ui->LineWidthLabel->setText(QString::number(Value, 'f', 5));
}

void lcQPreferencesDialog::on_MeshLODSlider_valueChanged()
{
	float Value = ui->MeshLODSlider->value() * mMeshLODMultiplier;
	ui->MeshLODLabel->setText(QString::number(static_cast<int>(Value)));
}

/*** LPub3D Mod - lpub fade highlight ***/
void lcQPreferencesDialog::on_LPubFadeHighlight_toggled()
{
	if (ui->LPubFadeHighlight->isChecked())
	{
		if (ui->FadeSteps->isChecked())
			ui->FadeSteps->setChecked(false);
		if (ui->HighlightNewParts->isChecked())
			ui->HighlightNewParts->setChecked(false);
	}
}
/*** LPub3D Mod end ***/

void lcQPreferencesDialog::on_FadeSteps_toggled()
{
	ui->FadeStepsColor->setEnabled(ui->FadeSteps->isChecked());
/*** LPub3D Mod - Reset fade default colours ***/
	ui->ResetFadeStepsButton->setEnabled(ui->FadeSteps->isChecked());
/*** LPub3D Mod end ***/
/*** LPub3D Mod - lpub fade highlight ***/
	if (ui->FadeSteps->isChecked() && ui->LPubFadeHighlight->isChecked())
		ui->LPubFadeHighlight->setChecked(false);
/*** LPub3D Mod end ***/
}

void lcQPreferencesDialog::on_HighlightNewParts_toggled()
{
	ui->HighlightNewPartsColor->setEnabled(ui->HighlightNewParts->isChecked());
/*** LPub3D Mod - Reset highlight default colours ***/
	ui->ResetHighlightNewPartsButton->setEnabled(ui->HighlightNewParts->isChecked());
/*** LPub3D Mod end ***/
/*** LPub3D Mod - lpub fade highlight ***/
	if (ui->HighlightNewParts->isChecked() && ui->LPubFadeHighlight->isChecked())
		ui->LPubFadeHighlight->setChecked(false);
/*** LPub3D Mod end ***/
}

void lcQPreferencesDialog::on_gridStuds_toggled()
{
	ui->gridStudColor->setEnabled(ui->gridStuds->isChecked());
}

void lcQPreferencesDialog::on_gridLines_toggled()
{
	ui->gridLineColor->setEnabled(ui->gridLines->isChecked());
	ui->gridLineSpacing->setEnabled(ui->gridLines->isChecked());
}

void lcQPreferencesDialog::on_PreviewViewSphereSizeCombo_currentIndexChanged(int Index)
{
	ui->PreviewViewSphereLocationCombo->setEnabled(Index != 0);
}

/*** LPub3D Mod - preview widget for LPub3D ***/
void lcQPreferencesDialog::on_ViewSphereSizeCombo_currentIndexChanged(int Index)
{
	bool Enabled = Index != 0;

	ui->ViewSphereLocationCombo->setEnabled(Enabled);
	ui->ViewSphereColorButton->setEnabled(Enabled);
	ui->ViewSphereTextColorButton->setEnabled(Enabled);
	ui->ViewSphereHighlightColorButton->setEnabled(Enabled);
}
/*** LPub3D Mod end ***/

void lcQPreferencesDialog::on_AutomateEdgeColor_toggled()
{
	ui->AutomateEdgeColorButton->setEnabled(ui->AutomateEdgeColor->isChecked());
}

void lcQPreferencesDialog::on_studStyleCombo_currentIndexChanged(int index)
{
	ui->HighContrastButton->setEnabled(lcIsHighContrast(static_cast<lcStudStyle>(index)));
}

void lcQPreferencesDialog::AutomateEdgeColor()
{
	lcAutomateEdgeColorDialog Dialog(this, sender() == ui->HighContrastButton);
	if (Dialog.exec() == QDialog::Accepted)
	{
		mOptions->Preferences.mStudCylinderColorEnabled = Dialog.mStudCylinderColorEnabled;
		mOptions->Preferences.mStudCylinderColor = Dialog.mStudCylinderColor;
		mOptions->Preferences.mPartEdgeColorEnabled = Dialog.mPartEdgeColorEnabled;
		mOptions->Preferences.mPartEdgeColor = Dialog.mPartEdgeColor;
		mOptions->Preferences.mBlackEdgeColorEnabled = Dialog.mBlackEdgeColorEnabled;
		mOptions->Preferences.mBlackEdgeColor = Dialog.mBlackEdgeColor;
		mOptions->Preferences.mDarkEdgeColorEnabled = Dialog.mDarkEdgeColorEnabled;
		mOptions->Preferences.mDarkEdgeColor = Dialog.mDarkEdgeColor;
		mOptions->Preferences.mPartEdgeContrast = Dialog.mPartEdgeContrast;
		mOptions->Preferences.mPartColorValueLDIndex = Dialog.mPartColorValueLDIndex;
	}
}

/*** LPub3D Mod - line width max granularity ***/
void lcQPreferencesDialog::LineWidthMaxGranularity()
{
	float Max = mOptions->Preferences.mLineWidthMaxGranularity;
	QString const Header = tr("%1 Smoothing Granularity");
	QInputDialog Dialog(this);
	Dialog.setInputMode(QInputDialog::DoubleInput);
	Dialog.setWindowTitle(tr("Edge Line Width"));
	Dialog.setWhatsThis(lpubWT(WT_GUI_LINE_WIDTH_MAX_GRANULARITY, Header.arg(Dialog.windowTitle())));
	Dialog.setLabelText(tr("Smoothing Granularity:"));
	Dialog.setDoubleValue(Max);
	Dialog.setDoubleRange(0.0f, 1.0f);
	Dialog.setDoubleDecimals(4);
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
	Dialog.setDoubleStep(0.001);
#endif
	if (Dialog.exec() == QDialog::Accepted)
	{
		if (Dialog.doubleValue() != Max)
		{
			Max = Dialog.doubleValue();
			mOptions->Preferences.mLineWidthMaxGranularity = Max;
			ui->LineWidthSlider->setRange(0, (mLineWidthRange[1] - mLineWidthRange[0]) / qMax(Max, mLineWidthGranularity));
		}
	}
}
/*** LPub3D Mod end ***/

/*** LPub3D Mod - preview widget for LPub3D ***/
void lcQPreferencesDialog::on_PreviewSizeCombo_currentIndexChanged(int Index)
{
	ui->PreviewLocationCombo->setEnabled(Index != 0);
	if (ui->PreviewPositionCombo->currentIndex() != 0)
		ui->PreviewPositionCombo->setEnabled(Index != 0);
}
/*** LPub3D Mod end ***/

/*** LPub3D Mod - preview widget for LPub3D ***/
void lcQPreferencesDialog::on_PreviewPositionCombo_currentIndexChanged(int Index)
{
	ui->PreviewSizeCombo->setEnabled(Index != 0);
	ui->PreviewLocationCombo->setEnabled(Index != 0);
}
/*** LPub3D Mod end ***/

void lcQPreferencesDialog::updateCategories()
{
	QTreeWidgetItem* CategoryItem;
	QTreeWidget* CategoriesTree = ui->categoriesTree;

	CategoriesTree->clear();

	for (int CategoryIndex = 0; CategoryIndex < static_cast<int>(mOptions->Categories.size()); CategoryIndex++)
	{
		CategoryItem = new QTreeWidgetItem(CategoriesTree, QStringList(mOptions->Categories[CategoryIndex].Name));
		CategoryItem->setData(0, CategoryRole, QVariant(CategoryIndex));
	}

	CategoryItem = new QTreeWidgetItem(CategoriesTree, QStringList(tr("Unassigned")));
	CategoryItem->setData(0, CategoryRole, QVariant(-1));
}

void lcQPreferencesDialog::updateParts()
{
	lcPiecesLibrary *Library = lcGetPiecesLibrary();
	QTreeWidget *tree = ui->partsTree;

	tree->clear();

	QList<QTreeWidgetItem*> selectedItems = ui->categoriesTree->selectedItems();

	if (selectedItems.empty())
		return;

	QTreeWidgetItem *categoryItem = selectedItems.first();
	int categoryIndex = categoryItem->data(0, CategoryRole).toInt();

	if (categoryIndex != -1)
	{
		std::vector<PieceInfo*> SingleParts, GroupedParts;

		Library->GetCategoryEntries(mOptions->Categories[categoryIndex].Keywords.constData(), false, SingleParts, GroupedParts);

		for (PieceInfo* Info : SingleParts)
		{
			QStringList rowList(Info->m_strDescription);
			rowList.append(Info->mFileName);

			new QTreeWidgetItem(tree, rowList);
		}
	}
	else
	{
		for (const auto& PartIt : Library->mPieces)
		{
			PieceInfo* Info = PartIt.second;

			for (categoryIndex = 0; categoryIndex < static_cast<int>(mOptions->Categories.size()); categoryIndex++)
			{
				if (Library->PieceInCategory(Info, mOptions->Categories[categoryIndex].Keywords.constData()))
					break;
			}

			if (categoryIndex == static_cast<int>(mOptions->Categories.size()))
			{
				QStringList rowList(Info->m_strDescription);
				rowList.append(Info->mFileName);

				new QTreeWidgetItem(tree, rowList);
			}
		}
	}

	tree->resizeColumnToContents(0);
	tree->resizeColumnToContents(1);
}

void lcQPreferencesDialog::on_newCategory_clicked()
{
	lcLibraryCategory category;

	lcCategoryDialog dialog(this, &category);
/*** LPub3D Mod - Common menus help ***/
	dialog.setWhatsThis(lpubWT(WT_DIALOG_VISUAL_CATEGORY_NEW, dialog.windowTitle()));
/*** LPub3D Mod end ***/
	if (dialog.exec() != QDialog::Accepted)
		return;

	mOptions->CategoriesModified = true;
	mOptions->CategoriesDefault = false;
	mOptions->Categories.emplace_back(std::move(category));

	updateCategories();
	ui->categoriesTree->setCurrentItem(ui->categoriesTree->topLevelItem(static_cast<int>(mOptions->Categories.size()) - 1));
}

void lcQPreferencesDialog::on_editCategory_clicked()
{
	QList<QTreeWidgetItem*> selectedItems = ui->categoriesTree->selectedItems();

	if (selectedItems.empty())
		return;

	QTreeWidgetItem *categoryItem = selectedItems.first();
	int categoryIndex = categoryItem->data(0, CategoryRole).toInt();

	if (categoryIndex == -1)
		return;

	lcCategoryDialog dialog(this, &mOptions->Categories[categoryIndex]);
/*** LPub3D Mod - Common menus help ***/
	dialog.setWhatsThis(lpubWT(WT_DIALOG_VISUAL_CATEGORY_EDIT, dialog.windowTitle()));
/*** LPub3D Mod end ***/
	if (dialog.exec() != QDialog::Accepted)
		return;

	mOptions->CategoriesModified = true;
	mOptions->CategoriesDefault = false;

	updateCategories();
	ui->categoriesTree->setCurrentItem(ui->categoriesTree->topLevelItem(categoryIndex));
}

void lcQPreferencesDialog::on_deleteCategory_clicked()
{
	QList<QTreeWidgetItem*> selectedItems = ui->categoriesTree->selectedItems();

	if (selectedItems.empty())
		return;

	QTreeWidgetItem *categoryItem = selectedItems.first();
	int categoryIndex = categoryItem->data(0, CategoryRole).toInt();

	if (categoryIndex == -1)
		return;

	QString question = tr("Are you sure you want to delete the category '%1'?").arg(mOptions->Categories[categoryIndex].Name);
/*** LPub3D Mod - set Visual Editor label ***/
	if (QMessageBox::question(this, "Visual Editor", question, QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
		return;
/*** LPub3D Mod end ***/

	mOptions->CategoriesModified = true;
	mOptions->CategoriesDefault = false;
	mOptions->Categories.erase(mOptions->Categories.begin() + categoryIndex);

	updateCategories();
}

void lcQPreferencesDialog::on_importCategories_clicked()
{
	QString FileName = QFileDialog::getOpenFileName(this, tr("Import Categories"), "", tr("Text Files (*.txt);;All Files (*.*)"));

	if (FileName.isEmpty())
		return;

	std::vector<lcLibraryCategory> Categories;
	if (!lcLoadCategories(FileName, Categories))
	{
/*** LPub3D Mod - set Visual Editor label ***/
		QMessageBox::warning(this, "Visual Editor", tr("Error loading categories file."));
/*** LPub3D Mod end ***/
		return;
	}

	mOptions->Categories = Categories;
	mOptions->CategoriesModified = true;
	mOptions->CategoriesDefault = false;
}

void lcQPreferencesDialog::on_exportCategories_clicked()
{
	QString FileName = QFileDialog::getSaveFileName(this, tr("Export Categories"), "", tr("Text Files (*.txt);;All Files (*.*)"));

	if (FileName.isEmpty())
		return;

	if (!lcSaveCategories(FileName, mOptions->Categories))
	{
/*** LPub3D Mod - set Visual Editor label ***/
		QMessageBox::warning(this, "Visual Editor", tr("Error saving categories file."));
/*** LPub3D Mod end ***/
		return;
	}
}

void lcQPreferencesDialog::on_resetCategories_clicked()
{
/*** LPub3D Mod - set Visual Editor label ***/
	if (QMessageBox::question(this, "Visual Editor", tr("Are you sure you want to load the default categories?"), QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
		return;
/*** LPub3D Mod end ***/

	lcResetCategories(mOptions->Categories);

	mOptions->CategoriesModified = true;
	mOptions->CategoriesDefault = true;

	updateCategories();
}

bool lcQPreferencesDialog::eventFilter(QObject *object, QEvent *event)
{
	Q_UNUSED(object);

	if (event->type() == QEvent::KeyPress)
	{
		QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);

		int nextKey = keyEvent->key();
		if (nextKey == Qt::Key_Control || nextKey == Qt::Key_Shift || nextKey == Qt::Key_Meta || nextKey == Qt::Key_Alt)
			return true;

		Qt::KeyboardModifiers state = keyEvent->modifiers();
		QString text = QKeySequence(nextKey).toString();
		if ((state & Qt::ShiftModifier) && (text.isEmpty() || !text.at(0).isPrint() || text.at(0).isLetter() || text.at(0).isSpace()))
			nextKey |= Qt::SHIFT;
		if (state & Qt::ControlModifier)
			nextKey |= Qt::CTRL;
		if (state & Qt::MetaModifier)
			nextKey |= Qt::META;
		if (state & Qt::AltModifier)
			nextKey |= Qt::ALT;

		QKeySequence ks(nextKey);
		ui->shortcutEdit->setText(ks.toString(QKeySequence::NativeText));
		keyEvent->accept();

		return true;
	}

	if (event->type() == QEvent::Shortcut || event->type() == QEvent::KeyRelease || event->type() == QEvent::ShortcutOverride)
	{
		event->accept();
		return true;
	}

	return QDialog::eventFilter(object, event);
}

void lcQPreferencesDialog::updateCommandList()
{
	ui->commandList->clear();
	QMap<QString, QTreeWidgetItem*> sections;

	for (unsigned int actionIdx = 0; actionIdx < LC_NUM_COMMANDS; actionIdx++)
	{
		if (!gCommands[actionIdx].ID[0])
			continue;

		const QString identifier = tr(gCommands[actionIdx].ID);

		int pos = identifier.indexOf(QLatin1Char('.'));
		int subPos = identifier.indexOf(QLatin1Char('.'), pos + 1);
		if (subPos == -1)
			subPos = pos;

		const QString parentSection = identifier.left(pos);

		if (subPos != pos)
		{
			if (!sections.contains(parentSection))
			{
				QTreeWidgetItem *categoryItem = new QTreeWidgetItem(ui->commandList, QStringList(parentSection));
				QFont f = categoryItem->font(0);
				f.setBold(true);
				categoryItem->setFont(0, f);
				sections.insert(parentSection, categoryItem);
				ui->commandList->expandItem(categoryItem);
			}
		}

		const QString section = identifier.left(subPos);
		const QString subId = identifier.mid(subPos + 1);

		if (!sections.contains(section))
		{
			QTreeWidgetItem *parent = sections[parentSection];
			QTreeWidgetItem *categoryItem;
			QString subSection;

			if (pos != subPos)
				subSection = identifier.mid(pos + 1, subPos - pos - 1);
			else
				subSection = section;

			if (parent)
				categoryItem = new QTreeWidgetItem(parent, QStringList(subSection));
			else
				categoryItem = new QTreeWidgetItem(ui->commandList, QStringList(subSection));

			QFont f = categoryItem->font(0);
			f.setBold(true);
			categoryItem->setFont(0, f);
			sections.insert(section, categoryItem);
			ui->commandList->expandItem(categoryItem);
		}

		QTreeWidgetItem *item = new QTreeWidgetItem;
		QKeySequence sequence(mOptions->KeyboardShortcuts.mShortcuts[actionIdx]);
		item->setText(0, qApp->translate("Menu", gCommands[actionIdx].MenuName).remove('&').remove(QLatin1String("...")));
		item->setText(1, sequence.toString(QKeySequence::NativeText));
		item->setData(0, Qt::UserRole, QVariant::fromValue(actionIdx));

		if (mOptions->KeyboardShortcuts.mShortcuts[actionIdx] != gCommands[actionIdx].DefaultShortcut)
			setShortcutModified(item, true);

		sections[section]->addChild(item);
	}
}

void lcQPreferencesDialog::setShortcutModified(QTreeWidgetItem *treeItem, bool modified)
{
	QFont font = treeItem->font(0);
	font.setItalic(modified);
	treeItem->setFont(0, font);
	font.setBold(modified);
	treeItem->setFont(1, font);
}

void lcQPreferencesDialog::commandChanged(QTreeWidgetItem *current)
{
	if (!current || !current->data(0, Qt::UserRole).isValid())
	{
		ui->shortcutEdit->setText(QString());
		ui->shortcutGroup->setEnabled(false);
		return;
	}

	ui->shortcutGroup->setEnabled(true);

	int shortcutIndex = qvariant_cast<int>(current->data(0, Qt::UserRole));
	QKeySequence key(mOptions->KeyboardShortcuts.mShortcuts[shortcutIndex]);
	ui->shortcutEdit->setText(key.toString(QKeySequence::NativeText));
}

void lcQPreferencesDialog::on_KeyboardFilterEdit_textEdited(const QString& Text)
{
	if (Text.isEmpty())
	{
		std::function<void(QTreeWidgetItem*)> ShowItems = [&ShowItems](QTreeWidgetItem* ParentItem)
		{
			for (int ChildIdx = 0; ChildIdx < ParentItem->childCount(); ChildIdx++)
				ShowItems(ParentItem->child(ChildIdx));

			ParentItem->setHidden(false);
		};

		ShowItems(ui->commandList->invisibleRootItem());
	}
	else
	{
		std::function<bool(QTreeWidgetItem*,bool)> ShowItems = [&ShowItems, &Text](QTreeWidgetItem* ParentItem, bool ForceVisible)
		{
			ForceVisible |= (bool)ParentItem->text(0).contains(Text, Qt::CaseInsensitive) | (bool)ParentItem->text(1).contains(Text, Qt::CaseInsensitive);
			bool Visible = ForceVisible;

			for (int ChildIdx = 0; ChildIdx < ParentItem->childCount(); ChildIdx++)
				Visible |= ShowItems(ParentItem->child(ChildIdx), ForceVisible);

			ParentItem->setHidden(!Visible);

			return Visible;
		};

		ShowItems(ui->commandList->invisibleRootItem(), false);
	}
}

void lcQPreferencesDialog::on_shortcutAssign_clicked()
{
	QTreeWidgetItem* CurrentItem = ui->commandList->currentItem();

	if (!CurrentItem || !CurrentItem->data(0, Qt::UserRole).isValid())
		return;

	uint ShortcutIndex = CurrentItem->data(0, Qt::UserRole).toUInt();
	QString (&Shortcuts)[LC_NUM_COMMANDS] = mOptions->KeyboardShortcuts.mShortcuts;

	if (ShortcutIndex >= LC_ARRAY_COUNT(Shortcuts))
		return;

	QString NewShortcut = ui->shortcutEdit->text();

	if (!NewShortcut.isEmpty())
	{
		for (uint ExistingIndex = 0; ExistingIndex < LC_ARRAY_COUNT(Shortcuts); ExistingIndex++)
		{
			if (NewShortcut == Shortcuts[ExistingIndex] && ExistingIndex != ShortcutIndex)
			{
				QString ActionText = qApp->translate("Menu", gCommands[ExistingIndex].MenuName).remove('&').remove(QLatin1String("..."));
				QString QuestionText = tr("The shortcut '%1' is already assigned to '%2'. Do you want to replace it?").arg(NewShortcut, ActionText);

				if (QMessageBox::question(this, tr("Override Shortcut"), QuestionText, QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
					return;

				mOptions->KeyboardShortcuts.mShortcuts[ExistingIndex].clear();

				std::function<QTreeWidgetItem* (QTreeWidgetItem*)> FindItem = [&FindItem, ExistingIndex](QTreeWidgetItem* ParentItem) -> QTreeWidgetItem*
				{
					for (int ChildIdx = 0; ChildIdx < ParentItem->childCount(); ChildIdx++)
					{
						QTreeWidgetItem* ChildItem = ParentItem->child(ChildIdx);
						uint ChildIndex = ChildItem->data(0, Qt::UserRole).toUInt();

						if (ChildIndex == ExistingIndex)
							return ChildItem;

						QTreeWidgetItem* ExistingItem = FindItem(ChildItem);

						if (ExistingItem)
							return ExistingItem;
					}

					return nullptr;
				};

				QTreeWidgetItem* ExistingItem = FindItem(ui->commandList->invisibleRootItem());

				if (ExistingItem)
				{
					ExistingItem->setText(1, QString());
					setShortcutModified(ExistingItem, gCommands[ShortcutIndex].DefaultShortcut[0] != 0);
				}
			}
		}
	}

	mOptions->KeyboardShortcuts.mShortcuts[ShortcutIndex] = NewShortcut;
	CurrentItem->setText(1, NewShortcut);

	setShortcutModified(CurrentItem, mOptions->KeyboardShortcuts.mShortcuts[ShortcutIndex] != gCommands[ShortcutIndex].DefaultShortcut);

	mOptions->KeyboardShortcutsModified = true;
	mOptions->KeyboardShortcutsDefault = false;
}

void lcQPreferencesDialog::on_shortcutRemove_clicked()
{
	ui->shortcutEdit->setText(QString());

	on_shortcutAssign_clicked();
}

void lcQPreferencesDialog::on_shortcutsImport_clicked()
{
	QString FileName = QFileDialog::getOpenFileName(this, tr("Import shortcuts"), "", tr("Text Files (*.txt);;All Files (*.*)"));

	if (FileName.isEmpty())
		return;

	lcKeyboardShortcuts Shortcuts;
	if (!Shortcuts.Load(FileName))
	{
/*** LPub3D Mod - set Visual Editor label ***/
		QMessageBox::warning(this, "Visual Editor", tr("Error loading keyboard shortcuts file."));
/*** LPub3D Mod end ***/
		return;
	}

	mOptions->KeyboardShortcuts = Shortcuts;

	mOptions->KeyboardShortcutsModified = true;
	mOptions->KeyboardShortcutsDefault = false;
}

void lcQPreferencesDialog::on_shortcutsExport_clicked()
{
	QString FileName = QFileDialog::getSaveFileName(this, tr("Export shortcuts"), "", tr("Text Files (*.txt);;All Files (*.*)"));

	if (FileName.isEmpty())
		return;

	if (!mOptions->KeyboardShortcuts.Save(FileName))
	{
/*** LPub3D Mod - set Visual Editor label ***/
		QMessageBox::warning(this, "Visual Editor", tr("Error saving keyboard shortcuts file."));
/*** LPub3D Mod end ***/
		return;
	}
}

void lcQPreferencesDialog::on_shortcutsReset_clicked()
{
/*** LPub3D Mod - set Visual Editor label ***/
	if (QMessageBox::question(this, "Visual Editor", tr("Are you sure you want to load the default keyboard shortcuts?"), QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
		return;
/*** LPub3D Mod end ***/

	mOptions->KeyboardShortcuts.Reset();
	updateCommandList();

	mOptions->KeyboardShortcutsModified = true;
	mOptions->KeyboardShortcutsDefault = true;
}

void lcQPreferencesDialog::UpdateMouseTree()
{
	ui->mouseTree->clear();

	for (int ToolIdx = 0; ToolIdx < static_cast<int>(lcTool::Count); ToolIdx++)
		UpdateMouseTreeItem(ToolIdx);
}

void lcQPreferencesDialog::UpdateMouseTreeItem(int ItemIndex)
{
	auto GetShortcutText = [](Qt::MouseButton Button, Qt::KeyboardModifiers Modifiers)
	{
		QString Shortcut = QKeySequence(Modifiers).toString(QKeySequence::NativeText);

		switch (Button)
		{
		case Qt::LeftButton:
			Shortcut += tr("Left Button");
			break;

		case Qt::MiddleButton:
			Shortcut += tr("Middle Button");
			break;

		case Qt::RightButton:
			Shortcut += tr("Right Button");
			break;

		default:
			Shortcut.clear();
		}
		return Shortcut;
	};

	QString Shortcut1 = GetShortcutText(mOptions->MouseShortcuts.mShortcuts[ItemIndex].Button1, mOptions->MouseShortcuts.mShortcuts[ItemIndex].Modifiers1);
	QString Shortcut2 = GetShortcutText(mOptions->MouseShortcuts.mShortcuts[ItemIndex].Button2, mOptions->MouseShortcuts.mShortcuts[ItemIndex].Modifiers2);

	QTreeWidgetItem* Item = ui->mouseTree->topLevelItem(ItemIndex);

	if (Item)
	{
		Item->setText(1, Shortcut1);
		Item->setText(2, Shortcut2);
	}
	else
		new QTreeWidgetItem(ui->mouseTree, QStringList() << tr(gToolNames[ItemIndex]) << Shortcut1 << Shortcut2);
}

void lcQPreferencesDialog::on_mouseAssign_clicked()
{
	QTreeWidgetItem* Current = ui->mouseTree->currentItem();

	if (!Current)
		return;

	int ButtonIndex = ui->mouseButton->currentIndex();
	Qt::MouseButton Button = Qt::NoButton;
	Qt::KeyboardModifiers Modifiers = Qt::NoModifier;

	if (ButtonIndex)
	{
		switch (ButtonIndex)
		{
		case 1:
			Button = Qt::LeftButton;
			break;

		case 2:
			Button = Qt::MiddleButton;
			break;

		case 3:
			Button = Qt::RightButton;
			break;
		}

		if (ui->mouseControl->isChecked())
			Modifiers |= Qt::ControlModifier;

		if (ui->mouseShift->isChecked())
			Modifiers |= Qt::ShiftModifier;

		if (ui->mouseAlt->isChecked())
			Modifiers |= Qt::AltModifier;

		for (int ToolIdx = 0; ToolIdx < static_cast<int>(lcTool::Count); ToolIdx++)
		{
			if (ToolIdx == ButtonIndex)
				continue;

			if (mOptions->MouseShortcuts.mShortcuts[ToolIdx].Button2 == Button && mOptions->MouseShortcuts.mShortcuts[ToolIdx].Modifiers2 == Modifiers)
			{
				if (QMessageBox::question(this, tr("Override Shortcut"), tr("This shortcut is already assigned to '%1', do you want to replace it?").arg(tr(gToolNames[ToolIdx])), QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
					return;

				mOptions->MouseShortcuts.mShortcuts[ToolIdx].Button2 = Qt::NoButton;
				mOptions->MouseShortcuts.mShortcuts[ToolIdx].Modifiers2 = Qt::NoModifier;
			}

			if (mOptions->MouseShortcuts.mShortcuts[ToolIdx].Button1 == Button && mOptions->MouseShortcuts.mShortcuts[ToolIdx].Modifiers1 == Modifiers)
			{
				if (QMessageBox::question(this, tr("Override Shortcut"), tr("This shortcut is already assigned to '%1', do you want to replace it?").arg(tr(gToolNames[ToolIdx])), QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
					return;

				mOptions->MouseShortcuts.mShortcuts[ToolIdx].Button1 = mOptions->MouseShortcuts.mShortcuts[ToolIdx].Button2;
				mOptions->MouseShortcuts.mShortcuts[ToolIdx].Modifiers1 = mOptions->MouseShortcuts.mShortcuts[ToolIdx].Modifiers2;
				mOptions->MouseShortcuts.mShortcuts[ToolIdx].Button2 = Qt::NoButton;
				mOptions->MouseShortcuts.mShortcuts[ToolIdx].Modifiers2 = Qt::NoModifier;

				UpdateMouseTreeItem(ToolIdx);
			}
		}
	}

	int ItemIndex = ui->mouseTree->indexOfTopLevelItem(Current);
	mOptions->MouseShortcuts.mShortcuts[ItemIndex].Button2 = mOptions->MouseShortcuts.mShortcuts[ItemIndex].Button1;
	mOptions->MouseShortcuts.mShortcuts[ItemIndex].Modifiers2 = mOptions->MouseShortcuts.mShortcuts[ItemIndex].Modifiers1;
	mOptions->MouseShortcuts.mShortcuts[ItemIndex].Button1 = Button;
	mOptions->MouseShortcuts.mShortcuts[ItemIndex].Modifiers1 = Modifiers;

	mOptions->MouseShortcutsModified = true;
	mOptions->MouseShortcutsDefault = false;

	UpdateMouseTreeItem(ItemIndex);
}

void lcQPreferencesDialog::on_mouseRemove_clicked()
{
	QTreeWidgetItem* Current = ui->mouseTree->currentItem();

	if (!Current)
		return;

	int ItemIndex = ui->mouseTree->indexOfTopLevelItem(Current);
	mOptions->MouseShortcuts.mShortcuts[ItemIndex].Button1 = mOptions->MouseShortcuts.mShortcuts[ItemIndex].Button2;
	mOptions->MouseShortcuts.mShortcuts[ItemIndex].Modifiers1 = mOptions->MouseShortcuts.mShortcuts[ItemIndex].Modifiers2;
	mOptions->MouseShortcuts.mShortcuts[ItemIndex].Button2 = Qt::NoButton;
	mOptions->MouseShortcuts.mShortcuts[ItemIndex].Modifiers2 = Qt::NoModifier;

	mOptions->MouseShortcutsModified = true;
	mOptions->MouseShortcutsDefault = false;

	UpdateMouseTreeItem(ItemIndex);
	MouseTreeItemChanged(Current);
}

void lcQPreferencesDialog::on_MouseImportButton_clicked()
{
	QString FileName = QFileDialog::getOpenFileName(this, tr("Import Shortcuts"), "", tr("Text Files (*.txt);;All Files (*.*)"));

	if (FileName.isEmpty())
		return;

	lcMouseShortcuts Shortcuts;
	if (!Shortcuts.Load(FileName))
	{
/*** LPub3D Mod - set Visual Editor label ***/
		QMessageBox::warning(this, "Visual Editor", tr("Error loading mouse shortcuts file."));
		return;
/*** LPub3D Mod end ***/
	}

	mOptions->MouseShortcuts = Shortcuts;
	UpdateMouseTree();

	mOptions->MouseShortcutsModified = true;
	mOptions->MouseShortcutsDefault = false;
}

void lcQPreferencesDialog::on_MouseExportButton_clicked()
{
	QString FileName = QFileDialog::getSaveFileName(this, tr("Export Shortcuts"), "", tr("Text Files (*.txt);;All Files (*.*)"));

	if (FileName.isEmpty())
		return;

	if (!mOptions->MouseShortcuts.Save(FileName))
		QMessageBox::warning(this, "LeoCAD", tr("Error saving mouse shortcuts file."));
}

void lcQPreferencesDialog::on_mouseReset_clicked()
{
/*** LPub3D Mod - set Visual Editor label ***/
	if (QMessageBox::question(this, "Visual Editor", tr("Are you sure you want to load the default mouse shortcuts?"), QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
		return;
/*** LPub3D Mod end ***/

	mOptions->MouseShortcuts.Reset();
	UpdateMouseTree();

	mOptions->MouseShortcutsModified = true;
	mOptions->MouseShortcutsDefault = true;
}

void lcQPreferencesDialog::MouseTreeItemChanged(QTreeWidgetItem* Current)
{
	if (!Current)
	{
		ui->MouseShortcutGroup->setEnabled(false);
		return;
	}

	ui->MouseShortcutGroup->setEnabled(true);

	int ToolIndex = ui->mouseTree->indexOfTopLevelItem(Current);

	Qt::MouseButton Button = mOptions->MouseShortcuts.mShortcuts[ToolIndex].Button1;

	switch (Button)
	{
	case Qt::LeftButton:
		ui->mouseButton->setCurrentIndex(1);
		break;

	case Qt::MiddleButton:
		ui->mouseButton->setCurrentIndex(2);
		break;

	case Qt::RightButton:
		ui->mouseButton->setCurrentIndex(3);
		break;

	default:
		ui->mouseButton->setCurrentIndex(0);
		break;
	}

	Qt::KeyboardModifiers Modifiers = mOptions->MouseShortcuts.mShortcuts[ToolIndex].Modifiers1;
	ui->mouseControl->setChecked((Modifiers & Qt::ControlModifier) != 0);
	ui->mouseShift->setChecked((Modifiers & Qt::ShiftModifier) != 0);
	ui->mouseAlt->setChecked((Modifiers & Qt::AltModifier) != 0);
}

/*** LPub3D Mod - Native Renderer settings ***/
void lcQPreferencesDialog::on_ViewpointsCombo_currentIndexChanged(int index)
{
	int ProjectionComboSaveIndex = ui->ProjectionCombo->currentIndex();

	if (index < static_cast<int>(lcViewpoint::Count))
		ui->ProjectionCombo->setCurrentIndex(2/*Default*/);

	QDialog::DialogCode DialogCode = QDialog::Accepted;

	if (index == static_cast<int>(lcViewpoint::LatLon))
	{
		DialogCode = static_cast<QDialog::DialogCode>(lpub->SetViewpointLatLonDialog(true/*SetCamera*/));
		if (DialogCode != QDialog::Accepted)
		{
			ui->ViewpointsCombo->setCurrentIndex(LPub::ViewpointsComboSaveIndex);
			ui->ProjectionCombo->setCurrentIndex(ProjectionComboSaveIndex);
		}
	}

	if (DialogCode == QDialog::Accepted)
		LPub::ViewpointsComboSaveIndex = index;
}

void lcQPreferencesDialog::on_ProjectionCombo_currentIndexChanged(int index)
{
	if (mSetOptions)
		return;

	if (index < 2/*Default*/)
		ui->ViewpointsCombo->setCurrentIndex(static_cast<int>(lcViewpoint::Count));
}
/*** LPub3D Mod end ***/

/*** LPub3D Mod - Reset theme colors ***/
void lcQPreferencesDialog::on_ResetColorsButton_clicked()
{
	QString question = tr("Are you sure you want to reset colors to Theme %1 ?").arg(
	   mOptions->Preferences.mColorTheme == lcColorTheme::Dark ? "Dark" : "Default");

	if (QMessageBox::question(this, "Visual Editor", question, QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
		return;

	auto SetButtonPixmap = [](quint32 Color, QToolButton* Button)
	{
		QPixmap Pixmap(12, 12);

		Pixmap.fill(QColor(LC_RGBA_RED(Color), LC_RGBA_GREEN(Color), LC_RGBA_BLUE(Color)));
		Button->setIcon(Pixmap);
	};

	mOptions->Preferences.SetInterfaceColors(mOptions->Preferences.mColorTheme);

	SetButtonPixmap(mOptions->Preferences.mAxesColor, ui->AxesColorButton);
	SetButtonPixmap(mOptions->Preferences.mTextColor, ui->TextColorButton);
	SetButtonPixmap(mOptions->Preferences.mBackgroundSolidColor, ui->BackgroundSolidColorButton);
	SetButtonPixmap(mOptions->Preferences.mBackgroundGradientColorTop, ui->BackgroundGradient1ColorButton);
	SetButtonPixmap(mOptions->Preferences.mBackgroundGradientColorBottom, ui->BackgroundGradient2ColorButton);
	SetButtonPixmap(mOptions->Preferences.mOverlayColor, ui->OverlayColorButton);
	SetButtonPixmap(mOptions->Preferences.mMarqueeBorderColor, ui->MarqueeBorderColorButton);
	SetButtonPixmap(mOptions->Preferences.mMarqueeFillColor, ui->MarqueeFillColorButton);
	SetButtonPixmap(mOptions->Preferences.mActiveViewColor, ui->ActiveViewColorButton);
	SetButtonPixmap(mOptions->Preferences.mInactiveViewColor, ui->InactiveViewColorButton);
	SetButtonPixmap(mOptions->Preferences.mGridStudColor, ui->gridStudColor);
	SetButtonPixmap(mOptions->Preferences.mGridLineColor, ui->gridLineColor);
	SetButtonPixmap(mOptions->Preferences.mViewSphereColor, ui->ViewSphereColorButton);
	SetButtonPixmap(mOptions->Preferences.mViewSphereTextColor, ui->ViewSphereTextColorButton);
	SetButtonPixmap(mOptions->Preferences.mViewSphereHighlightColor, ui->ViewSphereHighlightColorButton);
	SetButtonPixmap(mOptions->Preferences.mObjectSelectedColor, ui->ObjectSelectedColorButton);
	SetButtonPixmap(mOptions->Preferences.mObjectFocusedColor, ui->ObjectFocusedColorButton);
	SetButtonPixmap(mOptions->Preferences.mCameraColor, ui->CameraColorButton);
	SetButtonPixmap(mOptions->Preferences.mLightColor, ui->LightColorButton);
	SetButtonPixmap(mOptions->Preferences.mControlPointColor, ui->ControlPointColorButton);
	SetButtonPixmap(mOptions->Preferences.mControlPointFocusedColor, ui->ControlPointFocusedColorButton);
	SetButtonPixmap(mOptions->Preferences.mBMObjectSelectedColor, ui->BMObjectSelectedColorButton);
}
/*** LPub3D Mod end ***/

/*** LPub3D Mod - Update Default Camera ***/
void lcQPreferencesDialog::on_cameraDefaultDistanceFactor_valueChanged(double value)
{
	qreal cdp = ui->cameraDefaultPosition->value();
	ui->cameraDistanceFactor->setText(QString::number(cdp / -value));
}

void lcQPreferencesDialog::on_cameraDefaultPosition_valueChanged(double value)
{
	qreal ddf = ui->cameraDefaultDistanceFactor->value();
	ui->cameraDistanceFactor->setText(QString::number(value / -ddf));
}

void lcQPreferencesDialog::cameraPropertyReset()
{
	if (sender() == ui->resetDefaultDistanceFactor) {
		ui->cameraDefaultDistanceFactor->setValue(qreal(8.0));
	} else if (sender() == ui->resetDefaultPosition) {
		ui->cameraDefaultPosition->setValue(qreal(1250.0));
	} else if (sender() == ui->resetFoV) {
		ui->cameraFoV->setValue(qreal(30.0));
	} else if (sender() == ui->resetNearPlane) {
		ui->cameraNearPlane->setValue(qreal(25));
	} else if (sender() == ui->resetFarPlane) {
		ui->cameraFarPlane->setValue(qreal(50000.0));
	}
}
/*** LPub3D Mod end ***/
