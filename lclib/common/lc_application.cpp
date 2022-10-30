#include "lc_global.h"
#include <stdio.h>
#include "lc_application.h"
#include "lc_library.h"
#include "lc_profile.h"
#include "project.h"
#include "lc_mainwindow.h"
#include "lc_qpreferencesdialog.h"
#include "lc_partselectionwidget.h"
#include "lc_shortcuts.h"
#include "lc_view.h"
#include "camera.h"
#include "lc_previewwidget.h"
/*** LPub3D Mod - moved to application.cpp ***/
/***
#ifdef Q_OS_WIN
#include <QtPlatformHeaders\QWindowsWindowFunctions>
#endif
***/
/*** LPub3D Mod end ***/

/*** LPub3D Mod - includes ***/
#include "name.h"
#include "color.h"
#include "application.h"
#include "threadworkers.h"
#include "lpubalert.h"
/*** LPub3D Mod end ***/

lcApplication* gApplication;

void lcPreferences::LoadDefaults()
{
	mFixedAxes = lcGetProfileInt(LC_PROFILE_FIXED_AXES);
	mMouseSensitivity = lcGetProfileInt(LC_PROFILE_MOUSE_SENSITIVITY);
	mShadingMode = static_cast<lcShadingMode>(lcGetProfileInt(LC_PROFILE_SHADING_MODE));
	mBackgroundGradient = lcGetProfileInt(LC_PROFILE_BACKGROUND_GRADIENT);
	mBackgroundSolidColor = lcGetProfileInt(LC_PROFILE_BACKGROUND_COLOR);
	mBackgroundGradientColorTop = lcGetProfileInt(LC_PROFILE_GRADIENT_COLOR_TOP);
	mBackgroundGradientColorBottom = lcGetProfileInt(LC_PROFILE_GRADIENT_COLOR_BOTTOM);
	mDrawAxes = lcGetProfileInt(LC_PROFILE_DRAW_AXES);
	mAxisIconLocation = static_cast<lcAxisIconLocation>(lcGetProfileInt(LC_PROFILE_DRAW_AXES_LOCATION));
	mAxesColor = lcGetProfileInt(LC_PROFILE_AXES_COLOR);
	mTextColor = lcGetProfileInt(LC_PROFILE_TEXT_COLOR);
	mMarqueeBorderColor = lcGetProfileInt(LC_PROFILE_MARQUEE_BORDER_COLOR);
	mMarqueeFillColor = lcGetProfileInt(LC_PROFILE_MARQUEE_FILL_COLOR);
	mOverlayColor = lcGetProfileInt(LC_PROFILE_OVERLAY_COLOR);
	mActiveViewColor = lcGetProfileInt(LC_PROFILE_ACTIVE_VIEW_COLOR);
	mInactiveViewColor = lcGetProfileInt(LC_PROFILE_INACTIVE_VIEW_COLOR);
	mDrawEdgeLines = lcGetProfileInt(LC_PROFILE_DRAW_EDGE_LINES);
	mDrawConditionalLines = lcGetProfileInt(LC_PROFILE_DRAW_CONDITIONAL_LINES);
	mLineWidth = lcGetProfileFloat(LC_PROFILE_LINE_WIDTH);
	mAllowLOD = lcGetProfileInt(LC_PROFILE_ALLOW_LOD);
	mMeshLODDistance = lcGetProfileFloat(LC_PROFILE_LOD_DISTANCE);
	mFadeSteps = lcGetProfileInt(LC_PROFILE_FADE_STEPS);
	mFadeStepsColor = lcGetProfileInt(LC_PROFILE_FADE_STEPS_COLOR);
	mHighlightNewParts = lcGetProfileInt(LC_PROFILE_HIGHLIGHT_NEW_PARTS);
	mHighlightNewPartsColor = lcGetProfileInt(LC_PROFILE_HIGHLIGHT_NEW_PARTS_COLOR);
	mDrawGridStuds = lcGetProfileInt(LC_PROFILE_GRID_STUDS);
	mGridStudColor = lcGetProfileInt(LC_PROFILE_GRID_STUD_COLOR);
	mDrawGridLines = lcGetProfileInt(LC_PROFILE_GRID_LINES);
	mGridLineSpacing = lcGetProfileInt(LC_PROFILE_GRID_LINE_SPACING);
	mGridLineColor = lcGetProfileInt(LC_PROFILE_GRID_LINE_COLOR);
	mDrawGridOrigin = lcGetProfileInt(LC_PROFILE_GRID_ORIGIN);
	mViewSphereEnabled = lcGetProfileInt(LC_PROFILE_VIEW_SPHERE_ENABLED);
	mViewSphereLocation = static_cast<lcViewSphereLocation>(lcGetProfileInt(LC_PROFILE_VIEW_SPHERE_LOCATION));
	mViewSphereSize = lcGetProfileInt(LC_PROFILE_VIEW_SPHERE_SIZE);
	mViewSphereColor = lcGetProfileInt(LC_PROFILE_VIEW_SPHERE_COLOR);
	mViewSphereTextColor = lcGetProfileInt(LC_PROFILE_VIEW_SPHERE_TEXT_COLOR);
	mViewSphereHighlightColor = lcGetProfileInt(LC_PROFILE_VIEW_SPHERE_HIGHLIGHT_COLOR);
	mAutoLoadMostRecent = lcGetProfileInt(LC_PROFILE_AUTOLOAD_MOSTRECENT);
	mRestoreTabLayout = lcGetProfileInt(LC_PROFILE_RESTORE_TAB_LAYOUT);
	mColorTheme = static_cast<lcColorTheme>(lcGetProfileInt(LC_PROFILE_COLOR_THEME));
	mPreviewViewSphereEnabled = lcGetProfileInt(LC_PROFILE_PREVIEW_VIEW_SPHERE_ENABLED);
	mPreviewViewSphereSize = lcGetProfileInt(LC_PROFILE_PREVIEW_VIEW_SPHERE_SIZE);
	mPreviewViewSphereLocation = static_cast<lcViewSphereLocation>(lcGetProfileInt(LC_PROFILE_PREVIEW_VIEW_SPHERE_LOCATION));
	mDrawPreviewAxis = lcGetProfileInt(LC_PROFILE_PREVIEW_DRAW_AXES);
	mStudCylinderColor = lcGetProfileInt(LC_PROFILE_STUD_CYLINDER_COLOR);
	mPartEdgeColor = lcGetProfileInt(LC_PROFILE_PART_EDGE_COLOR);
	mBlackEdgeColor = lcGetProfileInt(LC_PROFILE_BLACK_EDGE_COLOR);
	mDarkEdgeColor = lcGetProfileInt(LC_PROFILE_DARK_EDGE_COLOR);
	mPartEdgeContrast = lcGetProfileFloat(LC_PROFILE_PART_EDGE_CONTRAST);
	mPartColorValueLDIndex = lcGetProfileFloat(LC_PROFILE_PART_COLOR_VALUE_LD_INDEX);
	mAutomateEdgeColor = lcGetProfileInt(LC_PROFILE_AUTOMATE_EDGE_COLOR);
	mObjectSelectedColor = lcGetProfileUInt(LC_PROFILE_OBJECT_SELECTED_COLOR);
	mObjectFocusedColor = lcGetProfileUInt(LC_PROFILE_OBJECT_FOCUSED_COLOR);
	mCameraColor = lcGetProfileUInt(LC_PROFILE_CAMERA_COLOR);
	mLightColor = lcGetProfileUInt(LC_PROFILE_LIGHT_COLOR);
	mControlPointColor = lcGetProfileUInt(LC_PROFILE_CONTROL_POINT_COLOR);
	mControlPointFocusedColor = lcGetProfileUInt(LC_PROFILE_CONTROL_POINT_FOCUSED_COLOR);

/*** LPub3D Mod - Build mod object selected colour ***/	
	mBMObjectSelectedColor = lcGetProfileUInt(LC_PROFILE_BM_OBJECT_SELECTED_COLOR);
/*** LPub3D Mod end ***/	

/*** LPub3D Mod - Zoom extents ***/
	mZoomExtents = lcGetProfileInt(LC_PROFILE_ZOOM_EXTENTS);
/*** LPub3D Mod - ***/

/*** LPub3D Mod - parts load order ***/
	mPreferOfficialParts = lcGetProfileInt(LC_PROFILE_PREFER_OFFICIAL_PARTS);
/*** LPub3D Mod - ***/

/*** LPub3D Mod - preview widget for LPub3D ***/
	mPreviewEnabled  = lcGetProfileInt(LC_PROFILE_PREVIEW_ENABLED);
	mPreviewSize     = lcGetProfileInt(LC_PROFILE_PREVIEW_SIZE);
	mPreviewLocation = static_cast<lcPreviewLocation>(lcGetProfileInt(LC_PROFILE_PREVIEW_LOCATION));
	mPreviewPosition = static_cast<lcPreviewPosition>(lcGetProfileInt(LC_PROFILE_PREVIEW_POSITION));
	mPreviewLoadPath = lcGetProfileString(LC_PROFILE_PREVIEW_LOAD_PATH);
/*** LPub3D Mod end ***/

/*** LPub3D Mod - Update Default Camera ***/
	mDefaultCameraProperties = lcGetProfileInt(LC_PROFILE_DEFAULT_CAMERA_PROPERTIES);
	mDDF = lcGetProfileFloat(LC_PROFILE_DEFAULT_DISTANCE_FACTOR);
	mCDP = lcGetProfileFloat(LC_PROFILE_CAMERA_DEFAULT_POSITION);
	mCFoV = lcGetProfileFloat(LC_PROFILE_CAMERA_FOV);
	mCNear = lcGetProfileFloat(LC_PROFILE_CAMERA_NEAR_PLANE);
	mCFar = lcGetProfileFloat(LC_PROFILE_CAMERA_FAR_PLANE);
/*** LPub3D Mod end ***/

/*** LPub3D Mod - Native Renderer settings ***/
	mNativeViewpoint = lcGetProfileInt(LC_PROFILE_NATIVE_VIEWPOINT);
	mNativeProjection = lcGetProfileInt(LC_PROFILE_NATIVE_PROJECTION);
/*** LPub3D Mod end ***/

/*** LPub3D Mod - Timeline part icons ***/
	mViewPieceIcons = lcGetProfileInt(LC_PROFILE_VIEW_PIECE_ICONS);
/*** LPub3D Mod end ***/

/*** LPub3D Mod - true fade ***/
	mLPubTrueFade     = lcGetProfileInt(LC_PROFILE_LPUB_TRUE_FADE);
/*** LPub3D Mod end ***/

/*** LPub3D Mod - Selected Parts ***/
	mBuildModificationEnabled = lcGetProfileInt(LC_PROFILE_BUILD_MODIFICATION);
/*** LPub3D Mod end ***/
}

void lcPreferences::SaveDefaults()
{
	lcSetProfileInt(LC_PROFILE_FIXED_AXES, mFixedAxes);
	lcSetProfileInt(LC_PROFILE_MOUSE_SENSITIVITY, mMouseSensitivity);
	lcSetProfileInt(LC_PROFILE_SHADING_MODE, static_cast<int>(mShadingMode));
	lcSetProfileInt(LC_PROFILE_DRAW_AXES, mDrawAxes);
	lcSetProfileInt(LC_PROFILE_DRAW_AXES_LOCATION, static_cast<int>(mAxisIconLocation));
	lcSetProfileInt(LC_PROFILE_AXES_COLOR, mAxesColor);
	lcSetProfileInt(LC_PROFILE_TEXT_COLOR, mTextColor);
	lcSetProfileInt(LC_PROFILE_BACKGROUND_GRADIENT, mBackgroundGradient);
	lcSetProfileInt(LC_PROFILE_BACKGROUND_COLOR, mBackgroundSolidColor);
	lcSetProfileInt(LC_PROFILE_GRADIENT_COLOR_TOP, mBackgroundGradientColorTop);
	lcSetProfileInt(LC_PROFILE_GRADIENT_COLOR_BOTTOM, mBackgroundGradientColorBottom);
	lcSetProfileInt(LC_PROFILE_MARQUEE_BORDER_COLOR, mMarqueeBorderColor);
	lcSetProfileInt(LC_PROFILE_MARQUEE_FILL_COLOR, mMarqueeFillColor);
	lcSetProfileInt(LC_PROFILE_OVERLAY_COLOR, mOverlayColor);
	lcSetProfileInt(LC_PROFILE_ACTIVE_VIEW_COLOR, mActiveViewColor);
	lcSetProfileInt(LC_PROFILE_INACTIVE_VIEW_COLOR, mInactiveViewColor);
	lcSetProfileInt(LC_PROFILE_DRAW_EDGE_LINES, mDrawEdgeLines);
	lcSetProfileInt(LC_PROFILE_DRAW_CONDITIONAL_LINES, mDrawConditionalLines);
	lcSetProfileFloat(LC_PROFILE_LINE_WIDTH, mLineWidth);
	lcSetProfileInt(LC_PROFILE_ALLOW_LOD, mAllowLOD);
	lcSetProfileFloat(LC_PROFILE_LOD_DISTANCE, mMeshLODDistance);
	lcSetProfileInt(LC_PROFILE_FADE_STEPS, mFadeSteps);
	lcSetProfileInt(LC_PROFILE_FADE_STEPS_COLOR, mFadeStepsColor);
	lcSetProfileInt(LC_PROFILE_HIGHLIGHT_NEW_PARTS, mHighlightNewParts);
	lcSetProfileInt(LC_PROFILE_HIGHLIGHT_NEW_PARTS_COLOR, mHighlightNewPartsColor);
	lcSetProfileInt(LC_PROFILE_GRID_STUDS, mDrawGridStuds);
	lcSetProfileInt(LC_PROFILE_GRID_STUD_COLOR, mGridStudColor);
	lcSetProfileInt(LC_PROFILE_GRID_LINES, mDrawGridLines);
	lcSetProfileInt(LC_PROFILE_GRID_LINE_SPACING, mGridLineSpacing);
	lcSetProfileInt(LC_PROFILE_GRID_LINE_COLOR, mGridLineColor);
	lcSetProfileInt(LC_PROFILE_GRID_ORIGIN, mDrawGridOrigin);
	lcSetProfileInt(LC_PROFILE_VIEW_SPHERE_ENABLED, mViewSphereSize ? 1 : 0);
	lcSetProfileInt(LC_PROFILE_VIEW_SPHERE_LOCATION, static_cast<int>(mViewSphereLocation));
	lcSetProfileInt(LC_PROFILE_VIEW_SPHERE_SIZE, mViewSphereSize);
	lcSetProfileInt(LC_PROFILE_VIEW_SPHERE_COLOR, mViewSphereColor);
	lcSetProfileInt(LC_PROFILE_VIEW_SPHERE_TEXT_COLOR, mViewSphereTextColor);
	lcSetProfileInt(LC_PROFILE_VIEW_SPHERE_HIGHLIGHT_COLOR, mViewSphereHighlightColor);
	lcSetProfileInt(LC_PROFILE_AUTOLOAD_MOSTRECENT, mAutoLoadMostRecent);
	lcSetProfileInt(LC_PROFILE_RESTORE_TAB_LAYOUT, mRestoreTabLayout);
	lcSetProfileInt(LC_PROFILE_COLOR_THEME, static_cast<int>(mColorTheme));
	lcSetProfileInt(LC_PROFILE_PREVIEW_VIEW_SPHERE_SIZE, mPreviewViewSphereSize);
	lcSetProfileInt(LC_PROFILE_PREVIEW_VIEW_SPHERE_LOCATION, static_cast<int>(mPreviewViewSphereLocation));
	lcSetProfileInt(LC_PROFILE_PREVIEW_DRAW_AXES, mDrawPreviewAxis);
	lcSetProfileInt(LC_PROFILE_STUD_CYLINDER_COLOR, mStudCylinderColor);
	lcSetProfileInt(LC_PROFILE_PART_EDGE_COLOR, mPartEdgeColor);
	lcSetProfileInt(LC_PROFILE_BLACK_EDGE_COLOR, mBlackEdgeColor);
	lcSetProfileInt(LC_PROFILE_DARK_EDGE_COLOR, mDarkEdgeColor);
	lcSetProfileFloat(LC_PROFILE_PART_EDGE_CONTRAST, mPartEdgeContrast);
	lcSetProfileFloat(LC_PROFILE_PART_COLOR_VALUE_LD_INDEX, mPartColorValueLDIndex);
	lcSetProfileInt(LC_PROFILE_AUTOMATE_EDGE_COLOR, mAutomateEdgeColor);
	lcSetProfileUInt(LC_PROFILE_OBJECT_SELECTED_COLOR, mObjectSelectedColor);
	lcSetProfileUInt(LC_PROFILE_OBJECT_FOCUSED_COLOR, mObjectFocusedColor);
	lcSetProfileUInt(LC_PROFILE_CAMERA_COLOR, mCameraColor);
	lcSetProfileUInt(LC_PROFILE_LIGHT_COLOR, mLightColor);
	lcSetProfileUInt(LC_PROFILE_CONTROL_POINT_COLOR, mControlPointColor);
	lcSetProfileUInt(LC_PROFILE_CONTROL_POINT_FOCUSED_COLOR, mControlPointFocusedColor);

/*** LPub3D Mod - Build mod object selected colour ***/	
	lcSetProfileUInt(LC_PROFILE_BM_OBJECT_SELECTED_COLOR, mBMObjectSelectedColor);
/*** LPub3D Mod - ***/
	
/*** LPub3D Mod - Zoom extents ***/
	lcSetProfileInt(LC_PROFILE_ZOOM_EXTENTS, mZoomExtents);
/*** LPub3D Mod - ***/

/*** LPub3D Mod - parts load order ***/
	lcSetProfileInt(LC_PROFILE_PREFER_OFFICIAL_PARTS, mPreferOfficialParts);
/*** LPub3D Mod - ***/

/*** LPub3D Mod - preview widget for LPub3D ***/
	lcSetProfileInt(LC_PROFILE_PREVIEW_ENABLED, mPreviewViewSphereEnabled);
	lcSetProfileInt(LC_PROFILE_PREVIEW_ENABLED, mPreviewEnabled);
	lcSetProfileInt(LC_PROFILE_PREVIEW_SIZE, mPreviewSize);
	lcSetProfileInt(LC_PROFILE_PREVIEW_LOCATION, static_cast<int>(mPreviewLocation));
	lcSetProfileInt(LC_PROFILE_PREVIEW_POSITION, static_cast<int>(mPreviewPosition));
	lcSetProfileString(LC_PROFILE_PREVIEW_LOAD_PATH, mPreviewLoadPath);
/*** LPub3D Mod end ***/

/*** LPub3D Mod - Update Default Camera ***/
	lcSetProfileInt(LC_PROFILE_DEFAULT_CAMERA_PROPERTIES, mDefaultCameraProperties);
	lcSetProfileFloat(LC_PROFILE_DEFAULT_DISTANCE_FACTOR, mDDF);
	lcSetProfileFloat(LC_PROFILE_CAMERA_DEFAULT_POSITION, mCDP);
	lcSetProfileFloat(LC_PROFILE_CAMERA_FOV, mCFoV);
	lcSetProfileFloat(LC_PROFILE_CAMERA_NEAR_PLANE, mCNear);
	lcSetProfileFloat(LC_PROFILE_CAMERA_FAR_PLANE, mCFar);
/*** LPub3D Mod end ***/

/*** LPub3D Mod - Native Renderer settings ***/
	lcSetProfileInt(LC_PROFILE_NATIVE_VIEWPOINT, mNativeViewpoint);
	lcSetProfileInt(LC_PROFILE_NATIVE_PROJECTION, mNativeProjection);
/*** LPub3D Mod end ***/

/*** LPub3D Mod - Timeline part icons ***/
	lcSetProfileInt(LC_PROFILE_VIEW_PIECE_ICONS, mViewPieceIcons);
/*** LPub3D Mod end ***/

/*** LPub3D Mod - true fade ***/
	lcSetProfileInt(LC_PROFILE_LPUB_TRUE_FADE, mLPubTrueFade);
/*** LPub3D Mod end ***/

/*** LPub3D Mod - Selected Parts ***/
	lcSetProfileInt(LC_PROFILE_BUILD_MODIFICATION, mBuildModificationEnabled);
/*** LPub3D Mod end ***/
}

void lcPreferences::SetInterfaceColors(lcColorTheme ColorTheme)
{
	if (ColorTheme == lcColorTheme::Dark)
	{
		mAxesColor = LC_RGBA(160, 160, 160, 255);
		mTextColor = LC_RGBA(160, 160, 160, 255);
		mBackgroundSolidColor = LC_RGB(49, 52, 55);
		mBackgroundGradientColorTop = LC_RGB(0, 0, 191);
		mBackgroundGradientColorBottom = LC_RGB(255, 255, 255);
/*** LPub3D Mod - preview widget for LPub3D ***/
		mOverlayColor = LC_RGBA(224, 224, 224, 255);
		mMarqueeBorderColor = LC_RGBA(64, 64, 255, 255);
		mMarqueeFillColor = LC_RGBA(64, 64, 255, 64);
		mInactiveViewColor = LC_RGBA(69, 69, 69, 255);
/*** LPub3D Mod end ***/
		mActiveViewColor = LC_RGBA(41, 128, 185, 255);
		mGridStudColor = LC_RGBA(24, 24, 24, 192);
		mGridLineColor = LC_RGBA(24, 24, 24, 255);
		mViewSphereColor = LC_RGBA(35, 38, 41, 255);
		mViewSphereTextColor = LC_RGBA(224, 224, 224, 255);
		mViewSphereHighlightColor = LC_RGBA(41, 128, 185, 255);
/*** LPub3D Mod - Object selected colour ***/
		mObjectSelectedColor = LC_RGBA(229, 76, 102, 255);
		mObjectFocusedColor = LC_RGBA(102, 76, 229, 255);
		mCameraColor = LC_RGBA(128, 204, 128, 255);
		mLightColor = LC_RGBA(128, 204, 128, 255);
		mControlPointColor = LC_RGBA(128, 204, 128, 128);
		mControlPointFocusedColor = LC_RGBA(102, 76, 229, 128);
/*** LPub3D Mod end ***/
/*** LPub3D Mod - Build mod object selected colour ***/
		mBMObjectSelectedColor = LC_RGBA(121, 216, 121, 255);
/*** LPub3D Mod end ***/
	}
	else
	{
		mAxesColor = LC_RGBA(0, 0, 0, 255);
		mTextColor = LC_RGBA(0, 0, 0, 255);
		mBackgroundSolidColor = LC_RGB(255, 255, 255);
		mBackgroundGradientColorTop = LC_RGB(54, 72, 95);
		mBackgroundGradientColorBottom = LC_RGB(49, 52, 55);
		mOverlayColor = LC_RGBA(0, 0, 0, 255);
/*** LPub3D Mod - preview widget for LPub3D ***/
		mMarqueeBorderColor = LC_RGBA(64, 64, 255, 255);
		mMarqueeFillColor = LC_RGBA(64, 64, 255, 64);
		mInactiveViewColor = LC_RGBA(69, 69, 69, 255);
/*** LPub3D Mod end ***/
		mActiveViewColor = LC_RGBA(255, 0, 0, 255);
		mGridStudColor = LC_RGBA(64, 64, 64, 192);
		mGridLineColor = LC_RGBA(0, 0, 0, 255);
		mViewSphereColor = LC_RGBA(255, 255, 255, 255);
		mViewSphereTextColor = LC_RGBA(0, 0, 0, 255);
		mViewSphereHighlightColor = LC_RGBA(255, 0, 0, 255);
/*** LPub3D Mod - Object selected colour ***/
		mObjectSelectedColor = LC_RGBA(229, 76, 102, 255);
		mObjectFocusedColor = LC_RGBA(102, 76, 229, 255);
		mCameraColor = LC_RGBA(128, 204, 128, 255);
		mLightColor = LC_RGBA(128, 204, 128, 255);
		mControlPointColor = LC_RGBA(128, 204, 128, 128);
		mControlPointFocusedColor = LC_RGBA(102, 76, 229, 128);
/*** LPub3D Mod end ***/
/*** LPub3D Mod - Build mod object selected colour ***/
		mBMObjectSelectedColor = LC_RGBA(121, 216, 121, 255);
/*** LPub3D Mod end ***/
	}
}

/*** LPub3D Mod - Relocate Argc and Argv ***/
lcApplication::lcApplication(const lcCommandLineOptions *Options)
	:mOptions(*Options)
/*** LPub3D Mod end ***/
{
/*** LPub3D Mod - disable leoCAD application vars ***/
/***
	setApplicationDisplayName(QLatin1String("LeoCAD"));
***/
/*** LPub3D Mod end ***/

	gApplication = this;
/*** LPub3D Mod - Initialize default style, not used ***/
/***
	mDefaultStyle = style()->objectName();
***/
/*** LPub3D Mod end ***/
	mPreferences.LoadDefaults();

/*** LPub3D Mod - global options ***/
	mPreferences.mShadingMode = Options->ShadingMode;
	mPreferences.mDrawConditionalLines = Options->DrawConditionalLines;
/*** LPub3D Mod end ***/
}

/*** LPub3D Mod - true fade ***/
bool lcApplication::LPubFadeSteps(){
	return Preferences::enableFadeSteps;
}

bool lcApplication::UseLPubFadeColour()
{
	return Preferences::fadeStepsUseColour;
}

QString lcApplication::LPubFadeColour()
{
	return LDrawColor::ldColorCode(Preferences::validFadeStepsColour);
}
/*** LPub3D Mod end ***/

lcApplication::~lcApplication()
{
	gApplication = nullptr;
}

void lcApplication::UpdateStyle()
{
/*** LPub3D Mod - behaviour moved to LPub3D application ***/
	return;
/*** LPub3D Mod end ***/

	if (mPreferences.mColorTheme == lcColorTheme::Dark)
	{
		if (!QApplication::setStyle("fusion"))
			return;

		QPalette Palette = QApplication::palette();

		Palette.setColor(QPalette::Window, QColor(49, 52, 55));
		Palette.setColor(QPalette::WindowText, QColor(240, 240, 240));
		Palette.setColor(QPalette::Base, QColor(35, 38, 41));
		Palette.setColor(QPalette::AlternateBase, QColor(44, 47, 50));
		Palette.setColor(QPalette::ToolTipBase, QColor(224, 224, 244));
		Palette.setColor(QPalette::ToolTipText, QColor(58, 58, 58));
#if (QT_VERSION >= QT_VERSION_CHECK(5, 12, 0))
		Palette.setColor(QPalette::PlaceholderText, QColor(100, 100, 100));
#endif
		Palette.setColor(QPalette::Text, QColor(224, 224, 224));
		Palette.setColor(QPalette::Button, QColor(45, 48, 51));
		Palette.setColor(QPalette::ButtonText, QColor(224, 224, 244));
		Palette.setColor(QPalette::Light, QColor(65, 65, 65));
		Palette.setColor(QPalette::Midlight, QColor(62, 62, 62));
		Palette.setColor(QPalette::Dark, QColor(35, 35, 35));
		Palette.setColor(QPalette::Mid, QColor(50, 50, 50));
		Palette.setColor(QPalette::Shadow, QColor(20, 20, 20));
//		Palette.setColor(QPalette::Highlight, QColor(46, 108, 219));
		Palette.setColor(QPalette::Highlight, QColor(41, 128, 185));
		Palette.setColor(QPalette::HighlightedText, QColor(232, 232, 232));
		Palette.setColor(QPalette::Link, QColor(41, 128, 185));

		Palette.setColor(QPalette::Disabled, QPalette::Text, QColor(128, 128, 128));
		Palette.setColor(QPalette::Disabled, QPalette::WindowText, QColor(128, 128, 128));
		Palette.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(128, 128, 128));

		QApplication::setPalette(Palette);

		QFile StylesheetFile(QLatin1String(":/stylesheet/stylesheet.qss"));

		if (StylesheetFile.open(QIODevice::ReadOnly))
		{
			QString Stylesheet = QString::fromLatin1(StylesheetFile.readAll());
			qApp->setStyleSheet(Stylesheet);
		}
	}
	else
	{
		QApplication::setStyle(mDefaultStyle);
		QApplication::setPalette(qApp->style()->standardPalette());
		qApp->setStyleSheet(QString());
	}
}

void lcApplication::SaveTabLayout() const
{
/*** LPub3D Mod - Fix crash in CLI mode - Render Image ***/
	if (!gMainWindow || !mProject || mProject->GetFileName().isEmpty() || mProject->IsRenderImage())
		return;
/*** LPub3D Mod - ***/

	QSettings Settings;
	QByteArray TabLayout = gMainWindow->GetTabLayout();

	Settings.setValue(GetTabLayoutKey(), TabLayout);
}

QString lcApplication::GetTabLayoutKey() const
{
/*** LPub3D Mod - Render Image ***/
	if (mProject && !mProject->IsRenderImage())
/*** LPub3D Mod end ***/
	{
		QString FileName = mProject->GetFileName();
		if (!FileName.isEmpty())
		{
			FileName.replace('\\', '?');
			FileName.replace('/', '?');
			return QString("TabLayouts/%1").arg(FileName);
		}
	}

	return QString();
}

void lcApplication::SetProject(Project* Project)
{
	SaveTabLayout();

/*** LPub3D Mod - Render Image ***/
	if (gMainWindow && !Project->IsRenderImage())
/*** LPub3D Mod end ***/
	{
		gMainWindow->RemoveAllModelTabs();

		if (gMainWindow->GetPreviewWidget())
			gMainWindow->GetPreviewWidget()->ClearPreview();
	}

	delete mProject;
	mProject = Project;

	Project->SetActiveModel(0);
	lcGetPiecesLibrary()->RemoveTemporaryPieces();

/*** LPub3D Mod - Render Image ***/
	if (mProject && !mProject->GetFileName().isEmpty() && !Project->IsRenderImage() && mPreferences.mRestoreTabLayout)
/*** LPub3D Mod end ***/
	{
		QSettings Settings;
		QByteArray TabLayout = Settings.value(GetTabLayoutKey()).toByteArray();

		if (gMainWindow)
			gMainWindow->RestoreTabLayout(TabLayout);
	}
}

void lcApplication::SetClipboard(const QByteArray& Clipboard)
{
	mClipboard = Clipboard;
	gMainWindow->UpdatePaste(!mClipboard.isEmpty());
}

void lcApplication::ExportClipboard(const QByteArray& Clipboard)
{
	QMimeData* MimeData = new QMimeData();

	MimeData->setData("application/vnd.leocad-clipboard", Clipboard);
	QApplication::clipboard()->setMimeData(MimeData);

	SetClipboard(Clipboard);
}

bool lcApplication::LoadPartsLibrary(const QList<QPair<QString, bool>>& LibraryPaths, bool OnlyUsePaths)
{
/*** LPub3D Mod - run search directories ***/
	Q_UNUSED(LibraryPaths);

	PartWorker partWorker;

	// load search directories
	partWorker.ldsearchDirPreferences();

	// process search directories to update library archive
	if (Preferences::archivePartsOnLaunch) {
		partWorker.processLDSearchDirParts();
	} else {
		emit Application::instance()->splashMsgSig("70% - Skip parts archive per application preference...");

		// time delay to display archive message
		QTime dt = QTime::currentTime().addSecs(3);
		while (QTime::currentTime() < dt)
			QCoreApplication::processEvents(QEventLoop::AllEvents, 100);

	}

	emit Application::instance()->splashMsgSig("75% - Archive libraries loading...");
/*** LPub3D Mod end ***/

	if (mLibrary == nullptr)
		mLibrary = new lcPiecesLibrary();

	const bool ShowProgress = gMainWindow != nullptr;

	if (!OnlyUsePaths)
	{
/*** LPub3D Mod - disable LEOCAD_LIB env var  ***/
/***
		char* EnvPath = getenv("LEOCAD_LIB");

		if (EnvPath && EnvPath[0])
			return mLibrary->Load(EnvPath, ShowProgress);
***/
/*** LPub3D Mod end ***/

		QString CustomPath = lcGetProfileString(LC_PROFILE_PARTS_LIBRARY);

		if (!CustomPath.isEmpty())
			return mLibrary->Load(CustomPath, ShowProgress);
	}

/*** LPub3D Mod - disable LibraryPaths load	 ***/
/***
	for (const QPair<QString, bool>& LibraryPathEntry : LibraryPaths)
	{
		if (mLibrary->Load(LibraryPathEntry.first, ShowProgress))
		{
			if (LibraryPathEntry.second)
				mLibrary->SetOfficialPieces();

			return true;
		}
	}
***/
/*** LPub3D Mod end ***/
	return false;
}

lcCommandLineOptions lcApplication::ParseCommandLineOptions()
{
	lcPreferences Preferences;
	Preferences.LoadDefaults();

	lcCommandLineOptions Options;

	Options.ParseOK = true;
	Options.Exit = false;
	Options.SaveImage = false;
	Options.SaveWavefront = false;
	Options.Save3DS = false;
	Options.SaveCOLLADA = false;
	Options.SaveHTML = false;
	Options.SetCameraAngles = false;
	Options.SetCameraPosition = false;
/*** LPub3D Mod - DrawConditionalLines ***/
	Options.DrawConditionalLines = false;
/*** LPub3D Mod end ***/
	Options.Orthographic = false;
	Options.SetFoV = false;
	Options.SetZPlanes = false;
	Options.SetFadeStepsColor = false;
	Options.SetHighlightColor = false;
	Options.FadeSteps = Preferences.mFadeSteps;
	Options.ImageHighlight = Preferences.mHighlightNewParts;
	Options.ImageWidth = lcGetProfileInt(LC_PROFILE_IMAGE_WIDTH);
	Options.ImageHeight = lcGetProfileInt(LC_PROFILE_IMAGE_HEIGHT);
	Options.ShadingMode = Preferences.mShadingMode;
	Options.LineWidth = Preferences.mLineWidth;
	Options.AASamples = lcGetProfileInt(LC_PROFILE_ANTIALIASING_SAMPLES);
	Options.StudStyle = static_cast<lcStudStyle>(lcGetProfileInt(LC_PROFILE_STUD_STYLE));
	Options.ImageStart = 0;
	Options.ImageEnd = 0;
	Options.CameraPosition[0] = lcVector3(0.0f, 0.0f, 0.0f);
	Options.CameraPosition[1] = lcVector3(0.0f, 0.0f, 0.0f);
	Options.CameraPosition[2] = lcVector3(0.0f, 0.0f, 0.0f);
	Options.CameraLatLon = lcVector2(0.0f, 0.0f);
	Options.FoV = 0.0f;
	Options.ZPlanes = lcVector2(0.0f, 0.0f);
	Options.Viewpoint = lcViewpoint::Count;
	Options.FadeStepsColor = Preferences.mFadeStepsColor;
	Options.HighlightColor = Preferences.mHighlightNewPartsColor;
	Options.StudCylinderColor = Preferences.mStudCylinderColor;
	Options.PartEdgeColor = Preferences.mPartEdgeColor;
	Options.BlackEdgeColor = Preferences.mBlackEdgeColor;
	Options.DarkEdgeColor = Preferences.mDarkEdgeColor;
	Options.PartEdgeContrast = Preferences.mPartEdgeContrast;
	Options.PartColorValueLDIndex = Preferences.mPartColorValueLDIndex;
	Options.AutomateEdgeColor = Preferences.mAutomateEdgeColor;

/*** LPub3D Mod - process command line ***/
	QStringList Arguments = Application::instance()->arguments();
/*** LPub3D Mod end ***/

	if (Arguments.isEmpty())
		return Options;

	Arguments.removeFirst();

	while (!Arguments.isEmpty())
	{
		QString Option = Arguments.takeFirst();

		if (Option.isEmpty())
			continue;

/*** LPub3D Mod - process command line ***/
		if (!Arguments.isEmpty() && Option[0] != '-')
			continue;
/*** LPub3D Mod end ***/

		auto ParseString = [&Option, &Arguments, &Options](QString& Value, bool Required)
		{
			if (!Arguments.isEmpty() && Arguments.front()[0] != '-')
			{
				QString Parameter = Arguments.takeFirst();
				Value = Parameter;
			}
			else if (Required)
			{
				Options.StdErr += tr("Not enough parameters for the '%1' option.\n").arg(Option);
				Options.ParseOK = false;
				return false;
			}

			return true;
		};

		auto ParseInteger = [&Option, &Arguments, &Options](int& Value, int Min, int Max)
		{
			if (!Arguments.isEmpty() && Arguments.front()[0] != '-')
			{
				bool Ok = false;
				QString Parameter = Arguments.takeFirst();
				int NewValue = Parameter.toInt(&Ok);

				if (Ok && NewValue >= Min && NewValue <= Max)
				{
					Value = NewValue;
					return true;
				}
				else
					Options.StdErr += tr("Invalid parameter value specified for the '%1' option: '%2'.\n").arg(Option, Parameter);
			}
			else
				Options.StdErr += tr("Not enough parameters for the '%1' option.\n").arg(Option);

			Options.ParseOK = false;
			return false;
		};

		auto ParseUnsigned = [&Option, &Arguments, &Options](uint& Value, uint Min, uint Max)
		{
			if (!Arguments.isEmpty() && Arguments.front()[0] != '-')
			{
				bool Ok = false;
				QString Parameter = Arguments.takeFirst();
				uint NewValue = Parameter.toUInt(&Ok);

				if (Ok && NewValue >= Min && NewValue <= Max)
				{
					Value = NewValue;
					return true;
				}
				else
					Options.StdErr += tr("Invalid parameter value specified for the '%1' option: '%2'.\n").arg(Option, Parameter);
			}
			else
				Options.StdErr += tr("Not enough parameters for the '%1' option.\n").arg(Option);

			Options.ParseOK = false;
			return false;
		};

		auto ParseFloat = [&Option, &Arguments, &Options](float& Value, float Min, float Max)
		{
			if (!Arguments.isEmpty() && Arguments.front()[0] != '-')
			{
				bool Ok = false;
				QString Parameter = Arguments.takeFirst();
				float NewValue = Parameter.toFloat(&Ok);

				if (Ok && NewValue >= Min && NewValue <= Max)
				{
					Value = NewValue;
					return true;
				}
				else
					Options.StdErr += tr("Invalid parameter value specified for the '%1' option: '%2'.\n").arg(Option, Parameter);
			}
			else
				Options.StdErr += tr("Not enough parameters for the '%1' option.\n").arg(Option);

			Options.ParseOK = false;
			return false;
		};

		auto ParseFloatArray = [&Option, &Arguments, &Options](int Count, float* ValueArray, bool NegativesValid)
		{
			if (Arguments.size() < Count)
			{
				Options.StdErr += tr("Not enough parameters for the '%1' option.\n").arg(Option);
				Arguments.clear();
				Options.ParseOK = false;
				return false;
			}

			for (int ParseIndex = 0; ParseIndex < Count; ParseIndex++)
			{
				if (NegativesValid || Arguments.front()[0] != '-')
				{
					bool Ok = false;
					QString Parameter = Arguments.takeFirst();
					float NewValue = Parameter.toFloat(&Ok);

					if (Ok)
					{
						*(ValueArray++) = NewValue;
						continue;
					}

					Options.StdErr += tr("Invalid parameter value specified for the '%1' option: '%2'.\n").arg(Option, Parameter);
				}
				else
					Options.StdErr += tr("Not enough parameters for the '%1' option.\n").arg(Option);

				Options.ParseOK = false;
				return false;
			}

			return true;
		};

		auto ParseColor = [&Option, &Arguments, &Options](quint32& Color)
		{
			if (!Arguments.isEmpty() && Arguments.front()[0] != '-')
			{
				QString Parameter = Arguments.takeFirst();
				QColor ParsedColor = QColor(Parameter);

				if (ParsedColor.isValid())
				{
					Color = LC_RGBA(ParsedColor.red(), ParsedColor.green(), ParsedColor.blue(), ParsedColor.alpha());
					return true;
				}
				else
					Options.StdErr += tr("Invalid parameter value specified for the '%1' option: '%2'.\n").arg(Option, Parameter);
			}
			else
				Options.StdErr += tr("Not enough parameters for the '%1' option.\n").arg(Option);

			Options.ParseOK = false;
			return false;
		};

		if (Option[0] != '-')
		{
			if (QFileInfo(Option).isReadable())
				Options.ProjectName = Option;
			else
				Options.StdErr += tr("The file '%1' is not readable.\n").arg(Option);

			continue;
		}
/*** LPub3D Mod - process command line ***/
		/***
		if (Option == QLatin1String("-l") || Option == QLatin1String("--libpath"))
		{
			QString LibPath;

			if (ParseString(LibPath, true))
				Options.LibraryPaths += qMakePair<QString, bool>(LibPath, false);
		}
		else
		***/
/*** LPub3D Mod end ***/
		if (Option == QLatin1String("-i") || Option == QLatin1String("--image"))
		{
			Options.SaveImage = true;
			ParseString(Options.ImageName, false);
		}
		else if (Option == QLatin1String("-w") || Option == QLatin1String("--width"))
			ParseInteger(Options.ImageWidth, 1, INT_MAX);
		else if (Option == QLatin1String("-h") || Option == QLatin1String("--height"))
			ParseInteger(Options.ImageHeight, 1, INT_MAX);
		else if (Option == QLatin1String("-f") || Option == QLatin1String("--from"))
			ParseUnsigned(Options.ImageStart, 1, LC_STEP_MAX);
		else if (Option == QLatin1String("-t") || Option == QLatin1String("--to"))
			ParseUnsigned(Options.ImageEnd, 1, LC_STEP_MAX);
		else if (Option == QLatin1String("-s") || Option == QLatin1String("--submodel"))
			ParseString(Options.ModelName, true);
		else if (Option == QLatin1String("-c") || Option == QLatin1String("--camera"))
			ParseString(Options.CameraName, true);
		else if (Option == QLatin1String("--viewpoint"))
		{
			QString ViewpointName;

			if (ParseString(ViewpointName, true))
			{
				Options.Viewpoint = lcCamera::GetViewpoint(ViewpointName);

				if (Options.Viewpoint == lcViewpoint::Count)
				{
					Options.StdErr += tr("Invalid parameter value specified for the '%1' option: '%2'.\n").arg(Option, ViewpointName);
					Options.ParseOK = false;
				}
			}
		}
		else if (Option == QLatin1String("--camera-angles"))
		{
			if ((Options.SetCameraAngles = ParseFloatArray(2, Options.CameraLatLon, true)) && (fabsf(Options.CameraLatLon[0]) > 360.0f || fabsf(Options.CameraLatLon[1]) > 360.0f))
			{
				Options.StdErr += tr("Invalid parameter value(s) specified for the '%1' option: limits are +/- 360.\n").arg(Option);
				Options.ParseOK = false;
			}
		}
		else if (Option == QLatin1String("--camera-position") || Option == QLatin1String("--camera-position-ldraw"))
		{
			if ((Options.SetCameraPosition = ParseFloatArray(9, Options.CameraPosition[0], true)))
			{
				lcVector3 Front = Options.CameraPosition[1] - Options.CameraPosition[0];

				if (Front.LengthSquared() < 1.0f || Options.CameraPosition[2].LengthSquared() < 1.0f || fabsf(lcDot(lcNormalize(Front), lcNormalize(Options.CameraPosition[2]))) > 0.99f)
				{
					Options.StdErr += tr("Invalid parameter value(s) specified for the '%1' option.\n").arg(Option);
					Options.ParseOK = false;
				}
				else if (Option == QLatin1String("--camera-position-ldraw"))
				{
					Options.CameraPosition[0] = lcVector3LDrawToLeoCAD(Options.CameraPosition[0]);
					Options.CameraPosition[1] = lcVector3LDrawToLeoCAD(Options.CameraPosition[1]);
					Options.CameraPosition[2] = lcVector3LDrawToLeoCAD(Options.CameraPosition[2]);
				}
			}
		}
/*** LPub3D Mod - DrawConditionalLines ***/
		else if (Option == QLatin1String("-cl") || Option == QLatin1String("--draw-conditional-lines"))
			Options.DrawConditionalLines = true;
/*** LPub3D Mod end ***/
		else if (Option == QLatin1String("--orthographic"))
			Options.Orthographic = true;
		else if (Option == QLatin1String("--fov"))
			Options.SetFoV = ParseFloat(Options.FoV, 1.0f, 180.0f);
		else if (Option == QLatin1String("--zplanes"))
		{
			if ((Options.SetZPlanes = ParseFloatArray(2, Options.ZPlanes, false)) && (Options.ZPlanes[0] < 1.0 || Options.ZPlanes[0] >= Options.ZPlanes[1]))
			{
				Options.StdErr += tr("Invalid parameter value(s) specified for the '%1' option: requirements are: 1 <= <near> < <far>.\n").arg(Option);
				Options.ParseOK = false;
			}
		}
		else if (Option == QLatin1String("-scc") || Option == QLatin1String("--stud-cylinder-color"))
		{
			if (ParseColor(Options.StudCylinderColor))
			{
				if (!lcIsHighContrast(Options.StudStyle))
				{
					Options.StdErr += tr("High contrast stud style is required for the '%1' option but is not enabled.\n").arg(Option);
					Options.ParseOK = false;
				}
			}
		}
		else if (Option == QLatin1String("-ec") || Option == QLatin1String("--edge-color"))
		{
			if (ParseColor(Options.PartEdgeColor))
			{
				if (!lcIsHighContrast(Options.StudStyle))
				{
					Options.StdErr += tr("High contrast stud style is required for the '%1' option but is not enabled.\n").arg(Option);
					Options.ParseOK = false;
				}
			}
		}
		else if (Option == QLatin1String("-bec") || Option == QLatin1String("--black-edge-color"))
		{
			if (ParseColor(Options.BlackEdgeColor))
			{
				if (!lcIsHighContrast(Options.StudStyle))
				{
					Options.StdErr += tr("High contrast stud style is required for the '%1' option but is not enabled.\n").arg(Option);
					Options.ParseOK = false;
				}
			}
		}
		else if (Option == QLatin1String("-dec") || Option == QLatin1String("--dark-edge-color"))
		{
			if (ParseColor(Options.DarkEdgeColor))
			{
				if (!lcIsHighContrast(Options.StudStyle))
				{
					Options.StdErr += tr("High contrast stud style is required for the '%1' option but is not enabled.\n").arg(Option);
					Options.ParseOK = false;
				}
			}
		}
		else if (Option == QLatin1String("-aec") || Option == QLatin1String("--automate-edge-color"))
		{
			Options.AutomateEdgeColor = true;
		}
		else if (Option == QLatin1String("-cc") || Option == QLatin1String("--color-contrast"))
		{
			if (ParseFloat(Options.PartEdgeContrast, 0.0f, 1.0f))
			{
				if (!Options.AutomateEdgeColor)
				{
					Options.StdErr += tr("Automate edge color is required for the '%1' option but is not enabled.\n").arg(Option);
					Options.ParseOK = false;
				}
			}
		}
		else if (Option == QLatin1String("-ldv") || Option == QLatin1String("--light-dark-value"))
		{
			if (ParseFloat(Options.PartColorValueLDIndex, 0.0f, 1.0f))
			{
				if (!Options.AutomateEdgeColor)
				{
					Options.StdErr += tr("Automate edge color is required for the '%1' option but is not enabled.\n").arg(Option);
					Options.ParseOK = false;
				}
			}
		}
		else if (Option == QLatin1String("--fade-steps"))
			Options.FadeSteps = true;
		else if (Option == QLatin1String("--no-fade-steps"))
			Options.FadeSteps = false;
		else if (Option == QLatin1String("--fade-steps-color"))
		{
			if (ParseColor(Options.FadeStepsColor))
			{
				Options.SetFadeStepsColor = true;
				Options.FadeSteps = true;
			}
		}
		else if (Option == QLatin1String("--highlight"))
			Options.ImageHighlight = true;
		else if (Option == QLatin1String("--no-highlight"))
			Options.ImageHighlight = false;
		else if (Option == QLatin1String("--highlight-color"))
		{
			if (ParseColor(Options.HighlightColor))
			{
				Options.SetHighlightColor = true;
				Options.ImageHighlight = true;
			}
		}
		else if (Option == QLatin1String("--shading"))
		{
			QString ShadingString;

			if (ParseString(ShadingString, true))
			{
				if (ShadingString == QLatin1String("wireframe"))
					Options.ShadingMode = lcShadingMode::Wireframe;
				else if (ShadingString == QLatin1String("flat"))
					Options.ShadingMode = lcShadingMode::Flat;
				else if (ShadingString == QLatin1String("default"))
					Options.ShadingMode = lcShadingMode::DefaultLights;
				else if (ShadingString == QLatin1String("full"))
					Options.ShadingMode = lcShadingMode::Full;
				else
				{
					Options.StdErr += tr("Invalid parameter value specified for the '%1' option: '%2'.\n").arg(Option, ShadingString);
					Options.ParseOK = false;
				}
			}
		}
		else if (Option == QLatin1String("--line-width"))
			ParseFloat(Options.LineWidth, 0.0f, 10.0f);
		else if (Option == QLatin1String("--aa-samples"))
		{
			if (ParseInteger(Options.AASamples, 1, 8) && Options.AASamples != 1 && Options.AASamples != 2 && Options.AASamples != 4 && Options.AASamples != 8)
			{
				Options.StdErr += tr("Invalid parameter value specified for the '%1' option: '%2'.\n").arg(Option, QString::number(Options.AASamples));
				Options.ParseOK = false;
			}
		}
		else if (Option == QLatin1String("-ss") || Option == QLatin1String("--stud-style"))
		{
			int StudStyle;

			if (ParseInteger(StudStyle, 0, static_cast<int>(lcStudStyle::Count) - 1))
				Options.StudStyle = static_cast<lcStudStyle>(StudStyle);
		}
		else if (Option == QLatin1String("-obj") || Option == QLatin1String("--export-wavefront"))
		{
			Options.SaveWavefront = true;
			ParseString(Options.SaveWavefrontName, false);
		}
		else if (Option == QLatin1String("-3ds") || Option == QLatin1String("--export-3ds"))
		{
			Options.Save3DS = true;
			ParseString(Options.Save3DSName, false);
		}
		else if (Option == QLatin1String("-dae") || Option == QLatin1String("--export-collada"))
		{
			Options.SaveCOLLADA = true;
			ParseString(Options.SaveCOLLADAName, false);
		}
		else if (Option == QLatin1String("-html") || Option == QLatin1String("--export-html"))
		{
			Options.SaveHTML = true;
			ParseString(Options.SaveHTMLName, false);
		}
/*** LPub3D Mod - process command line ***/
		/***
		else if (Option == QLatin1String("-v") || Option == QLatin1String("--version"))
		{
#ifdef LC_CONTINUOUS_BUILD
			Options.StdOut += tr("LeoCAD Continuous Build %1\n").arg(QT_STRINGIFY(LC_CONTINUOUS_BUILD));
#else
			Options.StdOut += tr("LeoCAD Version %1\n").arg(LC_VERSION_TEXT);
#endif
			Options.StdOut += tr("Compiled %1\n").arg(__DATE__);
			Options.Exit = true;
		}
		else if (Option == QLatin1String("-?") || Option == QLatin1String("--help"))
		{
			Options.StdOut += tr("Usage: leocad [options] [file]\n");
			Options.StdOut += tr("  [options] can be:\n");
			Options.StdOut += tr("  -l, --libpath <path>: Set the Parts Library location to path.\n");
			Options.StdOut += tr("  -i, --image <outfile.ext>: Save a picture in the format specified by ext and exit.\n");
			Options.StdOut += tr("  -w, --width <width>: Set the picture width.\n");
			Options.StdOut += tr("  -h, --height <height>: Set the picture height.\n");
			Options.StdOut += tr("  -f, --from <step>: Set the first step to save pictures.\n");
			Options.StdOut += tr("  -t, --to <step>: Set the last step to save pictures.\n");
			Options.StdOut += tr("  -s, --submodel <submodel>: Set the active submodel.\n");
			Options.StdOut += tr("  -c, --camera <camera>: Set the active camera.\n");
			Options.StdOut += tr("  -ss, --stud-style <id>: Set the stud style 0=No style, 1=LDraw single wire, 2=LDraw double wire, 3=LDraw raised floating, 4=LDraw raised rounded, 5=LDraw subtle rounded, 6=LEGO no logo, 7=LEGO single wire.\n");
			Options.StdOut += tr("  --viewpoint <front|back|left|right|top|bottom|home>: Set the viewpoint.\n");
			Options.StdOut += tr("  --camera-angles <latitude> <longitude>: Set the camera angles in degrees around the model.\n");
			Options.StdOut += tr("  --camera-position <x> <y> <z> <tx> <ty> <tz> <ux> <uy> <uz>: Set the camera position, target and up vector.\n");
			Options.StdOut += tr("  --camera-position-ldraw <x> <y> <z> <tx> <ty> <tz> <ux> <uy> <uz>: Set the camera position, target and up vector using LDraw coordinates.\n");
			Options.StdOut += tr("  --orthographic: Render images using an orthographic projection.\n");
			Options.StdOut += tr("  --fov <degrees>: Set the vertical field of view used to render images (< 180).\n");
			Options.StdOut += tr("  --zplanes <near> <far>: Set the near and far clipping planes used to render images (1 <= <near> < <far>).\n");
			Options.StdOut += tr("  --fade-steps: Render parts from prior steps faded.\n");
			Options.StdOut += tr("  --no-fade-steps: Do not render parts from prior steps faded.\n");
			Options.StdOut += tr("  --fade-steps-color <rgba>: Renderinng color for prior step parts (#AARRGGBB).\n");
			Options.StdOut += tr("  --highlight: Highlight parts in the steps they appear.\n");
			Options.StdOut += tr("  --no-highlight: Do not highlight parts in the steps they appear.\n");
			Options.StdOut += tr("  --highlight-color: Renderinng color for highlighted parts (#AARRGGBB).\n");
			Options.StdOut += tr("  --shading <wireframe|flat|default|full>: Select shading mode for rendering.\n");
			Options.StdOut += tr("  --line-width <width>: Set the width of the edge lines.\n");
			Options.StdOut += tr("  --aa-samples <count>: AntiAliasing sample size (1, 2, 4, or 8).\n");
			Options.StdOut += tr("  -scc, --stud-cylinder-color <#AARRGGBB>: High contrast stud cylinder color.\n");
			Options.StdOut += tr("  -ec, --edge-color <#AARRGGBB>: High contrast edge color.\n");
			Options.StdOut += tr("  -bec, --black-edge-color <#AARRGGBB>: High contrast edge color for black parts.\n");
			Options.StdOut += tr("  -dec, --dark-edge-color <#AARRGGBB>: High contrast edge color for dark color parts.\n");
			Options.StdOut += tr("  -aec, --automate-edge-color: Enable automatically adjusted edge colors.\n");
			Options.StdOut += tr("  -cc, --color-contrast <float>: Color contrast value between 0.0 and 1.0.\n");
			Options.StdOut += tr("  -ldv, --light-dark-value <float>: Light/Dark color value between 0.0 and 1.0.\n");
			Options.StdOut += tr("  -obj, --export-wavefront <outfile.obj>: Export the model to Wavefront OBJ format.\n");
			Options.StdOut += tr("  -3ds, --export-3ds <outfile.3ds>: Export the model to 3D Studio 3DS format.\n");
			Options.StdOut += tr("  -dae, --export-collada <outfile.dae>: Export the model to COLLADA DAE format.\n");
			Options.StdOut += tr("  -html, --export-html <folder>: Create an HTML page for the model.\n");
			Options.StdOut += tr("  -v, --version: Output version information and exit.\n");
			Options.StdOut += tr("  -?, --help: Display this help message and exit.\n");
			Options.StdOut += QLatin1String("\n");
			Options.Exit = true;
		}
		else
		{
			Options.StdErr += tr("Unknown option: '%1'.\n").arg(Option);
			Options.ParseOK = false;
		}
		***/
/*** LPub3D Mod end ***/

	}

/*** LPub3D Mod - process command line ***/
		/***
	if (Options.AutomateEdgeColor && lcIsHighContrast(Options.StudStyle))
	{
		Options.StdErr += tr("High contrast stud and edge color settings are ignored when -aec or --automate-edge-color is set.\n");
	}
		***/
/*** LPub3D Mod end ***/

	if (!Options.CameraName.isEmpty())
	{
		if (Options.Viewpoint != lcViewpoint::Count)
			Options.StdErr += tr("--viewpoint is ignored when --camera is set.\n");

		if (Options.Orthographic)
			Options.StdErr += tr("--orthographic is ignored when --camera is set.\n");

		if (Options.SetCameraAngles)
			Options.StdErr += tr("--camera-angles is ignored when --camera is set.\n");

		if (Options.SetCameraPosition)
			Options.StdErr += tr("--camera-position is ignored when --camera is set.\n");
	}
	else if (Options.Viewpoint != lcViewpoint::Count)
	{
		if (Options.SetCameraAngles)
			Options.StdErr += tr("--camera-angles is ignored when --viewpoint is set.\n");

		if (Options.SetCameraPosition)
			Options.StdErr += tr("--camera-position is ignored when --viewpoint is set.\n");
	}
	else if (Options.SetCameraAngles)
	{
		if (Options.SetCameraPosition)
			Options.StdErr += tr("--camera-position is ignored when --camera-angles is set.\n");
	}

	const bool SaveAndExit = (Options.SaveImage || Options.SaveWavefront || Options.Save3DS || Options.SaveCOLLADA || Options.SaveHTML);

	if (SaveAndExit && Options.ProjectName.isEmpty())
	{
		Options.StdErr += tr("No file name specified.\n");
		Options.ParseOK = false;
	}

	return Options;
}

/*** LPub3D Mod - process command line ***/
int lcApplication::Process3DViewerCommandLine()
{
	QTextStream StdErr(stderr, QIODevice::WriteOnly);
	QTextStream StdOut(stdout, QIODevice::WriteOnly);

	const bool SaveAndExit = (mOptions.SaveImage || mOptions.SaveWavefront || mOptions.Save3DS || mOptions.SaveCOLLADA || mOptions.SaveHTML);

	if (!SaveAndExit)
		return 0;

	/***
	if (!SaveAndExit && mOptions.ProjectName.isEmpty() && lcGetProfileInt(LC_PROFILE_AUTOLOAD_MOSTRECENT))
		mOptions.ProjectName = lcGetProfileString(LC_PROFILE_RECENT_FILE1);
	***/

	bool ProjectLoaded = false;

	if (!mOptions.ProjectName.isEmpty())
	{
		if (gMainWindow)
			gMainWindow->OpenProject(mOptions.ProjectName);
		else
		{
			Project* LoadedProject = new Project();

			if (LoadedProject->Load(mOptions.ProjectName, false))
			{
				SetProject(LoadedProject);
				ProjectLoaded = true;
			}
			else
			{
				delete LoadedProject;
			}
		}
	}

	if (ProjectLoaded)
	{
		if (!mOptions.ModelName.isEmpty())
			mProject->SetActiveModel(mOptions.ModelName);

		std::unique_ptr<lcView> ActiveView;

		if (mOptions.SaveImage)
		{
			lcModel* Model;

			if (!mOptions.ModelName.isEmpty())
			{
				Model = mProject->GetModel(mOptions.ModelName);

				if (!Model)
				{
					StdErr << tr("Error: model '%1' does not exist.\n").arg(mOptions.ModelName);
					StdErr.flush();
					return -1;
				}
			}
			else
				Model = mProject->GetMainModel();

			ActiveView = std::unique_ptr<lcView>(new lcView(lcViewType::View, Model));

			ActiveView->SetOffscreenContext();
			ActiveView->MakeCurrent();
		}

		if (mOptions.SaveImage)
			ActiveView->SetSize(mOptions.ImageWidth, mOptions.ImageHeight);

		if (ActiveView)
		{
			if (!mOptions.CameraName.isEmpty())
				ActiveView->SetCamera(mOptions.CameraName);
			else
			{
				ActiveView->SetProjection(mOptions.Orthographic);

				if (mOptions.SetFoV)
					ActiveView->GetCamera()->m_fovy = mOptions.FoV;

				if (mOptions.SetZPlanes)
				{
					lcCamera* Camera = ActiveView->GetCamera();

					Camera->m_zNear = mOptions.ZPlanes[0];
					Camera->m_zFar = mOptions.ZPlanes[1];
				}

				if (mOptions.Viewpoint != lcViewpoint::Count)
					ActiveView->SetViewpoint(mOptions.Viewpoint);
				else if (mOptions.SetCameraAngles)
					ActiveView->SetCameraAngles(mOptions.CameraLatLon[0], mOptions.CameraLatLon[1]);
				else if (mOptions.SetCameraPosition)
					ActiveView->SetViewpoint(mOptions.CameraPosition[0], mOptions.CameraPosition[1], mOptions.CameraPosition[2]);
			}
		}

		if (mOptions.SaveImage)
		{
			lcModel* ActiveModel = ActiveView->GetModel();

			if (mOptions.ImageName.isEmpty())
				mOptions.ImageName = mProject->GetImageFileName(true);

			if (mOptions.ImageEnd < mOptions.ImageStart)
				mOptions.ImageEnd = mOptions.ImageStart;
			else if (mOptions.ImageStart > mOptions.ImageEnd)
				mOptions.ImageStart = mOptions.ImageEnd;

			if ((mOptions.ImageStart == 0) && (mOptions.ImageEnd == 0))
				mOptions.ImageStart = mOptions.ImageEnd = ActiveModel->GetCurrentStep();
			else if ((mOptions.ImageStart == 0) && (mOptions.ImageEnd != 0))
				mOptions.ImageStart = mOptions.ImageEnd;
			else if ((mOptions.ImageStart != 0) && (mOptions.ImageEnd == 0))
				mOptions.ImageEnd = mOptions.ImageStart;

			if (mOptions.ImageStart > 255)
				mOptions.ImageStart = 255;

			if (mOptions.ImageEnd > 255)
				mOptions.ImageEnd = 255;

			QString Frame;

			if (mOptions.ImageStart != mOptions.ImageEnd)
			{
				QString Extension = QFileInfo(mOptions.ImageName).suffix();
				Frame = mOptions.ImageName.left(mOptions.ImageName.length() - Extension.length() - 1) + QLatin1String("%1.") + Extension;
			}
			else
				Frame = mOptions.ImageName;

			mPreferences.mFadeSteps = mOptions.FadeSteps;
			if (mOptions.SetFadeStepsColor)
				mPreferences.mFadeStepsColor = mOptions.FadeStepsColor;
			mPreferences.mHighlightNewParts = mOptions.ImageHighlight;
			if (mOptions.SetHighlightColor)
				mPreferences.mHighlightNewPartsColor = mOptions.HighlightColor;

			if (mOptions.CameraName.isEmpty() && !mOptions.SetCameraPosition)
				ActiveView->ZoomExtents();

			auto ProgressCallback = [&StdOut](const QString& FileName)
			{
				StdOut << tr("Saved '%1'.\n").arg(FileName);
			};

			ActiveView->SaveStepImages(Frame, mOptions.ImageStart != mOptions.ImageEnd, mOptions.ImageStart, mOptions.ImageEnd, ProgressCallback);
		}

		if (mOptions.SaveWavefront)
		{
			QString FileName;

			if (!mOptions.SaveWavefrontName.isEmpty())
				FileName = mOptions.SaveWavefrontName;
			else
				FileName = mOptions.ProjectName;

			QString Extension = QFileInfo(FileName).suffix().toLower();

			if (Extension.isEmpty())
			{
				FileName += ".obj";
			}
			else if (Extension != "obj")
			{
				FileName = FileName.left(FileName.length() - Extension.length() - 1);
				FileName += ".obj";
			}

			if (mProject->ExportWavefront(FileName))
				StdOut << tr("Saved '%1'.\n").arg(FileName);
		}

		if (mOptions.Save3DS)
		{
			QString FileName;

			if (!mOptions.Save3DSName.isEmpty())
				FileName = mOptions.Save3DSName;
			else
				FileName = mOptions.ProjectName;

			QString Extension = QFileInfo(FileName).suffix().toLower();

			if (Extension.isEmpty())
			{
				FileName += ".3ds";
			}
			else if (Extension != "3ds")
			{
				FileName = FileName.left(FileName.length() - Extension.length() - 1);
				FileName += ".3ds";
			}

			if (mProject->Export3DStudio(FileName))
				StdOut << tr("Saved '%1'.\n").arg(FileName);
		}

		if (mOptions.SaveCOLLADA)
		{
			QString FileName;

			if (!mOptions.SaveCOLLADAName.isEmpty())
				FileName = mOptions.SaveCOLLADAName;
			else
				FileName = mOptions.ProjectName;

			QString Extension = QFileInfo(FileName).suffix().toLower();

			if (Extension.isEmpty())
			{
				FileName += ".dae";
			}
			else if (Extension != "dae")
			{
				FileName = FileName.left(FileName.length() - Extension.length() - 1);
				FileName += ".dae";
			}

			if (mProject->ExportCOLLADA(FileName))
				StdOut << tr("Saved '%1'.\n").arg(FileName);
		}

		if (mOptions.SaveHTML)
		{
			lcHTMLExportOptions HTMLOptions(mProject);

			if (!mOptions.SaveHTMLName.isEmpty())
				HTMLOptions.PathName = mOptions.SaveHTMLName;

			mProject->ExportHTML(HTMLOptions);
		}
	}

	return 1;
}
/*** LPub3D Mod end ***/

/*** LPub3D Mod - initialize with LPub3D as parent ***/
lcStartupMode lcApplication::Initialize(const QList<QPair<QString, bool>>& LibraryPaths, QMainWindow *parent)
{
	bool SaveAndExit = !Application::instance()->modeGUI();

	bool OnlyUsePaths = false;

	emit Application::instance()->splashMsgSig("45% - Visual Editor widgets loading...");

	QTextStream StdOut(stdout, QIODevice::WriteOnly);
	QTextStream StdErr(stderr, QIODevice::WriteOnly);

	/***
	if (!mOptions.StdErr.isEmpty())
	{
		StdErr << mOptions.StdErr;
		StdErr.flush();
	}
	***/
	if (!mOptions.StdOut.isEmpty())
	{
		StdOut << mOptions.StdOut;
		StdOut.flush();
	}
	/***
	if (!mOptions.ParseOK)
		return lcStartupMode::Error;

	if (mOptions.Exit)
		return lcStartupMode::Success;

	if (!lcContext::InitializeRenderer())
	{
		StdErr << tr("Visual Editor error creating OpenGL context.\n");
		return lcStartupMode::Error;
	}

	const bool SaveAndExit = (mOptions.SaveImage || mOptions.SaveWavefront || mOptions.Save3DS || mOptions.SaveCOLLADA || mOptions.SaveHTML);
	***/

	if (!SaveAndExit)
	{
		UpdateStyle();

		gMainWindow = new lcMainWindow(parent);

		lcLoadDefaultKeyboardShortcuts();
		lcLoadDefaultMouseShortcuts();
	}

	if (!LoadPartsLibrary(LibraryPaths, OnlyUsePaths))
	{
		QString Message;

		if (mLibrary->LoadBuiltinPieces())
			Message = tr("Visual Editor could not find a compatible Parts Library so only a small number of parts will be available.\n");
		else
			Message = tr("Visual Editor could not load Parts Library.\n");

		if (gMainWindow)
			QMessageBox::information(gMainWindow, tr("Visual Editor"), Message);
		else
		{
			StdErr << Message << "\n";
			StdErr.flush();
		}
	}

	mPreferences.mDrawConditionalLines = mOptions.DrawConditionalLines;
	mPreferences.mShadingMode = mOptions.ShadingMode;
	mPreferences.mLineWidth = mOptions.LineWidth;
	mPreferences.mStudCylinderColor = mOptions.StudCylinderColor;
	mPreferences.mPartEdgeColor = mOptions.PartEdgeColor;
	mPreferences.mBlackEdgeColor = mOptions.BlackEdgeColor;
	mPreferences.mDarkEdgeColor = mOptions.DarkEdgeColor;
	mPreferences.mPartEdgeContrast = mOptions.PartEdgeContrast;
	mPreferences.mPartColorValueLDIndex = mOptions.PartColorValueLDIndex;
	mPreferences.mAutomateEdgeColor = mOptions.AutomateEdgeColor;

	lcGetPiecesLibrary()->SetStudStyle(mOptions.StudStyle, false);

	if (!SaveAndExit)
		gMainWindow->CreateWidgets();

	Project* NewProject = new Project();
	SetProject(NewProject);

	if (!SaveAndExit)
	{
		gMainWindow->SetColorIndex(lcGetColorIndex(7));
		/***
		gMainWindow->GetPartSelectionWidget()->SetDefaultPart();
		gMainWindow->UpdateRecentFiles();
		gMainWindow->show();

#ifdef Q_OS_WIN
		QWindowsWindowFunctions::setHasBorderInFullScreen(gMainWindow->windowHandle(), true);
#endif
		***/
	}

	return SaveAndExit ? lcStartupMode::Success : lcStartupMode::ShowWindow;
}
/*** LPub3D Mod end ***/

void lcApplication::Shutdown()
{
	delete gMainWindow;
	gMainWindow = nullptr;

	delete mProject;
	mProject = nullptr;

	delete mLibrary;
	mLibrary = nullptr;

	lcContext::ShutdownRenderer();
}

void lcApplication::ShowPreferencesDialog()
{
	lcPreferencesDialogOptions Options;
	int CurrentAASamples = lcGetProfileInt(LC_PROFILE_ANTIALIASING_SAMPLES);
	lcStudStyle CurrentStudStyle = lcGetPiecesLibrary()->GetStudStyle();

	Options.Preferences = mPreferences;

	Options.LibraryPath = lcGetProfileString(LC_PROFILE_PARTS_LIBRARY);
	Options.MinifigSettingsPath = lcGetProfileString(LC_PROFILE_MINIFIG_SETTINGS);
	Options.ColorConfigPath = lcGetProfileString(LC_PROFILE_COLOR_CONFIG);
	Options.POVRayPath = lcGetProfileString(LC_PROFILE_POVRAY_PATH);
	Options.LGEOPath = lcGetProfileString(LC_PROFILE_POVRAY_LGEO_PATH);
	Options.DefaultAuthor = lcGetProfileString(LC_PROFILE_DEFAULT_AUTHOR_NAME);
	Options.Language = lcGetProfileString(LC_PROFILE_LANGUAGE);
	Options.CheckForUpdates = lcGetProfileInt(LC_PROFILE_CHECK_UPDATES);

	Options.AASamples = CurrentAASamples;
	Options.StudStyle = CurrentStudStyle;

	Options.Categories = gCategories;
	Options.CategoriesModified = false;
	Options.CategoriesDefault = false;

	Options.KeyboardShortcuts = gKeyboardShortcuts;
	Options.KeyboardShortcutsModified = false;
	Options.KeyboardShortcutsDefault = false;
	Options.MouseShortcuts = gMouseShortcuts;
	Options.MouseShortcutsModified = false;
	Options.MouseShortcutsDefault = false;

/*** LPub3D Mod - parts load order ***/
	Options.HasUnofficialParts = mLibrary->HasUnofficialParts();
/*** LPub3D Mod - ***/

/*** LPub3D Mod - preview widget for LPub3D ***/
	lcPreviewPosition PreviewDockable = Options.Preferences.mPreviewPosition;
/*** LPub3D Mod end ***/

/*** LPub3D Mod - preference refresh ***/
	Options.Preferences.mShadingMode   = (lcShadingMode)lcGetProfileInt(LC_PROFILE_SHADING_MODE);
	Options.Preferences.mDrawEdgeLines = lcGetProfileInt(LC_PROFILE_DRAW_EDGE_LINES);
	Options.Preferences.mLineWidth	   = lcGetProfileFloat(LC_PROFILE_LINE_WIDTH);
/*** LPub3D Mod end ***/

/*** LPub3D Mod - Native Renderer settings ***/
	Options.Preferences.mNativeViewpoint = lcGetProfileInt(LC_PROFILE_NATIVE_VIEWPOINT);
	Options.Preferences.mNativeProjection = lcGetProfileInt(LC_PROFILE_NATIVE_PROJECTION);
/*** LPub3D Mod end ***/

/*** LPub3D Mod - Timeline part icons ***/
	Options.Preferences.mViewPieceIcons = lcGetProfileInt(LC_PROFILE_VIEW_PIECE_ICONS);
/*** LPub3D Mod end ***/

/*** LPub3D Mod - true fade ***/
	Options.Preferences.mLPubTrueFade = lcGetProfileInt(LC_PROFILE_LPUB_TRUE_FADE);
	Options.Preferences.mDrawConditionalLines = lcGetProfileInt(LC_PROFILE_DRAW_CONDITIONAL_LINES);
/*** LPub3D Mod end ***/

	lcQPreferencesDialog Dialog(gMainWindow, &Options);
	if (Dialog.exec() != QDialog::Accepted)
		return;

/*** LPub3D Mod - preferences save message ***/
	lpubAlert->messageSig(LOG_STATUS, QString("Saving Visual Editor preferences. Please wait..."));
/*** LPub3D Mod end ***/

	bool LanguageChanged = Options.Language != lcGetProfileString(LC_PROFILE_LANGUAGE);
	bool LibraryChanged = Options.LibraryPath != lcGetProfileString(LC_PROFILE_PARTS_LIBRARY);
	bool ColorsChanged = Options.ColorConfigPath != lcGetProfileString(LC_PROFILE_COLOR_CONFIG);
	bool AAChanged = CurrentAASamples != Options.AASamples;
	bool StudStyleChanged = CurrentStudStyle != Options.StudStyle;
	bool AutomateEdgeColorChanged = Options.Preferences.mAutomateEdgeColor != mPreferences.mAutomateEdgeColor;
	AutomateEdgeColorChanged |= Options.Preferences.mStudCylinderColor != mPreferences.mStudCylinderColor;
	AutomateEdgeColorChanged |= Options.Preferences.mPartEdgeColor != mPreferences.mPartEdgeColor;
	AutomateEdgeColorChanged |= Options.Preferences.mBlackEdgeColor != mPreferences.mBlackEdgeColor;
	AutomateEdgeColorChanged |= Options.Preferences.mDarkEdgeColor != mPreferences.mDarkEdgeColor;
	AutomateEdgeColorChanged |= Options.Preferences.mPartEdgeContrast != mPreferences.mPartEdgeContrast;
	AutomateEdgeColorChanged |= Options.Preferences.mPartColorValueLDIndex != mPreferences.mPartColorValueLDIndex;

/*** LPub3D Mod - parts load order ***/
	bool PreferOfficialPartsChanged = Options.Preferences.mPreferOfficialParts != mPreferences.mPreferOfficialParts;
/*** LPub3D Mod - ***/

/*** LPub3D Mod - preference refresh ***/
	bool drawEdgeLinesChanged = Options.Preferences.mDrawEdgeLines != mPreferences.mDrawEdgeLines;
	bool shadingModeChanged = Options.Preferences.mShadingMode != mPreferences.mShadingMode;
	bool lineWidthChanged = Options.Preferences.mLineWidth != mPreferences.mLineWidth;
/*** LPub3D Mod end ***/

/*** LPub3D Mod - Native Renderer settings ***/
	bool NativeViewpointChanged = Options.Preferences.mNativeViewpoint != mPreferences.mNativeViewpoint;
	bool NativeProjectionChanged = Options.Preferences.mNativeProjection != mPreferences.mNativeProjection;
/*** LPub3D Mod end ***/

/*** LPub3D Mod - Timeline part icons ***/
	bool ViewPieceIconsChangd = Options.Preferences.mViewPieceIcons != mPreferences.mViewPieceIcons;
/*** LPub3D Mod end ***/

/*** LPub3D Mod - true fade ***/
	bool LPubTrueFadeChanged = Options.Preferences.mLPubTrueFade != mPreferences.mLPubTrueFade;
	bool DrawConditionalLinesChanged = Options.Preferences.mDrawConditionalLines != mPreferences.mDrawConditionalLines;
/*** LPub3D Mod end ***/

/*** LPub3D Mod - Update Default Camera ***/
	bool DefaultCameraChanged = Options.Preferences.mDDF != mPreferences.mDDF;
	DefaultCameraChanged |= Options.Preferences.mCDP != mPreferences.mCDP;
	DefaultCameraChanged |= Options.Preferences.mCFoV != mPreferences.mCFoV;
	DefaultCameraChanged |= Options.Preferences.mCNear != mPreferences.mCNear;
	DefaultCameraChanged |= Options.Preferences.mCFar != mPreferences.mCFar;
/*** LPub3D Mod end ***/

	mPreferences = Options.Preferences;

	mPreferences.SaveDefaults();
	UpdateStyle();

	lcSetProfileString(LC_PROFILE_DEFAULT_AUTHOR_NAME, Options.DefaultAuthor);
	lcSetProfileString(LC_PROFILE_PARTS_LIBRARY, Options.LibraryPath);
	lcSetProfileString(LC_PROFILE_MINIFIG_SETTINGS, Options.MinifigSettingsPath);
	lcSetProfileString(LC_PROFILE_POVRAY_PATH, Options.POVRayPath);
	lcSetProfileString(LC_PROFILE_POVRAY_LGEO_PATH, Options.LGEOPath);
	lcSetProfileString(LC_PROFILE_LANGUAGE, Options.Language);
	lcSetProfileInt(LC_PROFILE_CHECK_UPDATES, Options.CheckForUpdates);
	lcSetProfileInt(LC_PROFILE_ANTIALIASING_SAMPLES, Options.AASamples);
	lcSetProfileInt(LC_PROFILE_STUD_STYLE, static_cast<int>(Options.StudStyle));

/*** LPub3D Mod - parts load order ***/
	lcSetProfileInt(LC_PROFILE_UPDATE_CACHE_INDEX, static_cast<int>(PreferOfficialPartsChanged));
/*** LPub3D Mod - ***/

/*** LPub3D Mod - preview widget for LPub3D ***/
	lcPreviewPosition Dockable = Options.Preferences.mPreviewPosition;
	if (PreviewDockable != Dockable)
		emit gMainWindow->TogglePreviewWidgetSig(
			Dockable == lcPreviewPosition::Dockable);

	Preferences::updateViewerInterfaceColors();

	gMainWindow->GetPreviewWidget()->UpdatePreview();
/*** LPub3D Mod end ***/

/*** LPub3D Mod - preference refresh ***/
	bool restartApp = false;
	bool reloadPage = false;
	bool reloadFile = false;

	QMessageBox box;
	box.setMinimumSize(40,20);
	box.setIcon (QMessageBox::Question);
	box.setDefaultButton   (QMessageBox::Ok);
	box.setStandardButtons (QMessageBox::Ok | QMessageBox::Cancel);

	if (LanguageChanged || LibraryChanged || AAChanged || PreferOfficialPartsChanged) {
		QString thisChange = LanguageChanged ? tr("Language") :
							 LibraryChanged  ? tr("Library") :
							 AAChanged ?  tr("Anti-aliasing") :
										  tr("Prefer Official Parts");
		box.setText (tr("You must close and restart %1 to enable %2 change.")
					 .arg(QLatin1String(VER_PRODUCTNAME_STR))
					 .arg(thisChange));
		box.setInformativeText (tr("Click \"OK\" to close and restart %1 or \"Cancel\" to continue.\n\n")
								.arg(QLatin1String(VER_PRODUCTNAME_STR)));
		if (box.exec() == QMessageBox::Ok) {
			restartApp = true;
		}
	}

	if ((ViewPieceIconsChangd ||
		 LPubTrueFadeChanged  ||
		 DefaultCameraChanged ||
		 DrawConditionalLinesChanged) && !restartApp && !reloadFile)
		reloadPage = true;

	if (Preferences::preferredRenderer == RENDERER_NATIVE && !restartApp)
	{
		if (shadingModeChanged     ||
			drawEdgeLinesChanged   ||
			lineWidthChanged       ||
			NativeViewpointChanged ||
			NativeProjectionChanged)
		{
			reloadFile = true;

			QString oldShadingMode, newShadingMode;
			switch (int(Options.Preferences.mShadingMode))
			{
			case int(lcShadingMode::Flat):
				newShadingMode = "flat";
				break;
			case int(lcShadingMode::DefaultLights):
				newShadingMode = "default lights";
				break;
			case int(lcShadingMode::Full):
				newShadingMode = "full";
				break;
			case int(lcShadingMode::Wireframe):
				newShadingMode = "wire frame";
				break;
			default:
				newShadingMode = "unknown";
			}

			switch (lcGetProfileInt(LC_PROFILE_SHADING_MODE))
			{
			case int(lcShadingMode::Flat):
				oldShadingMode = "flat";
				break;
			case int(lcShadingMode::DefaultLights):
				oldShadingMode = "default lights";
				break;
			case int(lcShadingMode::Full):
				oldShadingMode = "full";
				break;
			case int(lcShadingMode::Wireframe):
				oldShadingMode = "wire frame";
				break;
			default:
				oldShadingMode = "unknown";
			}

			if (shadingModeChanged)
				logInfo() << QString("Shading mode changed from %1 to %2.")
							 .arg(oldShadingMode)
							 .arg(newShadingMode);
			if (lineWidthChanged)
				logInfo() << QString("Edge line width changed from %1 to %2.")
							 .arg(double(lcGetProfileFloat(LC_PROFILE_LINE_WIDTH)))
							 .arg(double(Options.Preferences.mLineWidth));
			if (drawEdgeLinesChanged)
				logInfo() << QString("Draw edge lines is %1.").arg(Options.Preferences.mDrawEdgeLines ? "ON" : "OFF");

			if (NativeViewpointChanged) {
				QString Viewpoint;
				switch (lcGetProfileInt(LC_PROFILE_NATIVE_VIEWPOINT))
				{
				case 0:
					Viewpoint = "Front";
					break;
				case 1:
					Viewpoint = "Back";
					break;
				case 2:
					Viewpoint = "Top";
					break;
				case 3:
					Viewpoint = "Bottom";
					break;
				case 4:
					Viewpoint = "Left";
					break;
				case 5:
					Viewpoint = "Right";
					break;
				case 6:
					Viewpoint = "Home";
					break;
				default:
					Viewpoint = "Front";
				}
				logInfo() << QString("Native Viewport changed to '%1'.").arg(Viewpoint.toUpper());

			}

			if (NativeProjectionChanged) {
				QVariant uValue(true);
				QString Projection;
				switch (lcGetProfileInt(LC_PROFILE_NATIVE_PROJECTION))
				{
				case 0:
					Projection = "Perscpective";
					break;
				case 1:
					Projection = "Ortographic";
					uValue = false;
					break;
				default:
					Projection = "Perscpective";
					break;
				}
				logInfo() << QString("Native Projection changed to '%1'.").arg(Projection.toUpper());
				QSettings Settings;
				Settings.setValue(QString("%1/%2").arg(SETTINGS,"PerspectiveProjection"),uValue);
			}
		}
	}
/*** LPub3D Mod end ***/
	if (Options.CategoriesModified)
	{
		if (Options.CategoriesDefault)
			lcResetDefaultCategories();
		else
		{
			gCategories = Options.Categories;
			lcSaveDefaultCategories();
		}

		gMainWindow->UpdateCategories();
	}

	if (Options.KeyboardShortcutsModified)
	{
		if (Options.KeyboardShortcutsDefault)
			lcResetDefaultKeyboardShortcuts();
		else
		{
			gKeyboardShortcuts = Options.KeyboardShortcuts;
			lcSaveDefaultKeyboardShortcuts();
		}

		gMainWindow->UpdateShortcuts();
	}

	if (Options.MouseShortcutsModified)
	{
		if (Options.MouseShortcutsDefault)
			lcResetDefaultMouseShortcuts();
		else
		{
			gMouseShortcuts = Options.MouseShortcuts;
			lcSaveDefaultMouseShortcuts();
		}
	}

	if (StudStyleChanged)
	{
		lcSetProfileInt(LC_PROFILE_STUD_STYLE, static_cast<int>(Options.StudStyle));
		lcGetPiecesLibrary()->SetStudStyle(Options.StudStyle, true);
	}
	else if (ColorsChanged || AutomateEdgeColorChanged)
	{
		if (ColorsChanged)
			lcSetProfileString(LC_PROFILE_COLOR_CONFIG, Options.ColorConfigPath);
		lcGetPiecesLibrary()->LoadColors();
	}

	gMainWindow->SetShadingMode(Options.Preferences.mShadingMode);
	lcView::UpdateAllViews();

/*** LPub3D Mod restart and reload***/
	lpubAlert->messageSig(LOG_STATUS, QString("Visual Editor preferences saved"));
	if (restartApp) {
		restartApplication();
	}
	else
	if (reloadFile) {
		clearAndReloadModelFile();
	}
	else
	if (reloadPage) {
		reloadCurrentPage();
	}
/*** LPub3D Mod end ***/
}
