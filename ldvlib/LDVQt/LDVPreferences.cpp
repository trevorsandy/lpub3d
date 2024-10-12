/****************************************************************************
**
** Copyright (C) 2018 - 2023 Trevor SANDY. All rights reserved.
**
** This file may be used under the terms of the
** GNU General Public Liceense (GPL) version 3.0
** which accompanies this distribution, and is
** available at http://www.gnu.org/licenses/gpl.html
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

// On the Mac, when using Qt, glextmacosqt.h has to be included prior to anything
// else that might include GL/gl.h, to override and force-load the needed extensions,
// otherwise things don't compile. This is annoying, but it doesn't appear to hurt anything.
#ifdef __APPLE__
#include <GL/glextmacosqt.h>
#endif // __APPLE__
#include <TRE/TREGLExtensions.h>
#include <LDLoader/LDLModel.h>
#include <LDLoader/LDLPalette.h>
#include <TCFoundation/TCUserDefaults.h>
#include <TCFoundation/mystring.h>
#include <LDLib/LDUserDefaultsKeys.h>
#include <TCFoundation/TCMacros.h>
//#include <TCFoundation/TCWebClient.h>  // Not Used
#include <QColor>
#include <QInputDialog>
#include <QFileDialog>
#include <QMessageBox>
#include <QColorDialog>
#include <TCFoundation/TCLocalStrings.h>
#include <LDLib/LDPreferences.h>
#include <LDLib/LDrawModelViewer.h>
#include <LDVWidgetDefaultKeys.h>
#ifdef WIN32
#include <TCFoundation/TCTypedValueArray.h>
#include <LDVExtensionsSetup.h>
#endif // WIN32

#include "LDVPreferences.h"
#include "LDVWidget.h"

#include "lc_edgecolordialog.h"
#include "commonmenus.h"

#define DEFAULT_PREF_SET TCLocalStrings::get("DefaultPrefSet")
#define	MAX_EXTRA_DIR	10

TCStringArray* LDVPreferences::extraSearchDirs = nullptr;

LDVPreferences::LDVPreferences(LDVWidget* modelWidget, QWidget *parent)
	:QDialog(parent),
	  LDVPreferencesPanel(),
	  modelViewer(modelWidget->getModelViewer() ? ((LDrawModelViewer*)modelWidget->getModelViewer()->retain()) : nullptr),
	  ldPrefs(new LDPreferences(modelViewer)),
	  checkAbandon(true)
{
	setupUi(this);

	setWhatsThis(lpubWT(WT_LDVIEW_PREFERENCES,windowTitle()));

	generalTab->setWhatsThis(lpubWT(WT_LDVIEW_PREFERENCES_GENERAL, tr("General")));
	ldrawTab->setWhatsThis(lpubWT(WT_LDVIEW_PREFERENCES_LDRAW, tr("LDraw")));
	geometryTab->setWhatsThis(lpubWT(WT_LDVIEW_PREFERENCES_GEOMETRY, tr("Geometry")));
	effectsTab->setWhatsThis(lpubWT(WT_LDVIEW_PREFERENCES_EFFECTS, tr("Effects")));
	primitivesTab->setWhatsThis(lpubWT(WT_LDVIEW_PREFERENCES_PRIMITIVES, tr("Primitives")));
	updatesTab->setWhatsThis(lpubWT(WT_LDVIEW_PREFERENCES_UPDATES, tr("Updates")));
	preferencsSetTab->setWhatsThis(lpubWT(WT_LDVIEW_PREFERENCES_PREFERENCES_SET, tr("Preferences Set")));

	fsaaBox->setWhatsThis(lpubWT(                    WT_CONTROL_LDVIEW_PREFERENCES_GENERAL_ANTIALIASING,fsaaBox->title().replace("&","")));
	generalColorBox->setWhatsThis(lpubWT(            WT_CONTROL_LDVIEW_PREFERENCES_GENERAL_COLORS,generalColorBox->title().replace("&","")));
	generalMiscBox->setWhatsThis(lpubWT(             WT_CONTROL_LDVIEW_PREFERENCES_GENERAL_MISC,generalMiscBox->title().replace("&","")));
	generalSnapshotSaveBox->setWhatsThis(lpubWT(     WT_CONTROL_LDVIEW_PREFERENCES_GENERAL_SAVE_DIRECTORIES,generalSnapshotSaveBox->title().replace("&","")));
	customConfigBox->setWhatsThis(lpubWT(            WT_CONTROL_LDVIEW_PREFERENCES_GENERAL_CUSTOM_LDCONFIG, customConfigBox->title().replace("&","")));
	iniBox->setWhatsThis(lpubWT(                     WT_CONTROL_LDVIEW_PREFERENCES_GENERAL_INI_FILE,iniBox->title().replace("&","")));
	geometryModelBox->setWhatsThis(lpubWT(           WT_CONTROL_LDVIEW_PREFERENCES_GEOMETRY_MODEL,geometryModelBox->title().replace("&","")));
	wireframeButton->setWhatsThis(lpubWT(            WT_CONTROL_LDVIEW_PREFERENCES_GEOMETRY_WIREFRAME,wireframeButton->title().replace("&","")));
	enableBFCButton->setWhatsThis(lpubWT(            WT_CONTROL_LDVIEW_PREFERENCES_GEOMETRY_BFC,enableBFCButton->title().replace("&","")));
	edgeLinesButton->setWhatsThis(lpubWT(            WT_CONTROL_LDVIEW_PREFERENCES_GEOMETRY_EDGE_LINES,edgeLinesButton->title().replace("&","")));
	lightingButton->setWhatsThis(lpubWT(             WT_CONTROL_LDVIEW_PREFERENCES_EFFECTS_LIGHTING,lightingButton->title().replace("&","")));
	stereoButton->setWhatsThis(lpubWT(               WT_CONTROL_LDVIEW_PREFERENCES_EFFECTS_STEREO,stereoButton->title().replace("&","")));
	wireframeCutawayButton->setWhatsThis(lpubWT(     WT_CONTROL_LDVIEW_PREFERENCES_EFFECTS_WIREFRAME_CUTAWAY,wireframeCutawayButton->title().replace("&","")));
	effectsTransparencyBox->setWhatsThis(lpubWT(     WT_CONTROL_LDVIEW_PREFERENCES_EFFECTS_TRANSPARENCY,effectsTransparencyBox->title().replace("&","")));
	effectsMiscBox->setWhatsThis(lpubWT(             WT_CONTROL_LDVIEW_PREFERENCES_EFFECTS_MISC,effectsMiscBox->title().replace("&","")));
	primitiveSubstitutionButton->setWhatsThis(lpubWT(WT_CONTROL_LDVIEW_PREFERENCES_PRIMITIVE_SUBSTITUTION,primitiveSubstitutionButton->title().replace("&","")));
	TexturesGroupBox->setWhatsThis(lpubWT(           WT_CONTROL_LDVIEW_PREFERENCES_PRIMITIVE_TEXTURES,TexturesGroupBox->title().replace("&","")));
	primitivesMiscBox->setWhatsThis(lpubWT(          WT_CONTROL_LDVIEW_PREFERENCES_PRIMITIVE_MISC,primitivesMiscBox->title().replace("&","")));
	updatesProxyBox->setWhatsThis(lpubWT(            WT_CONTROL_LDVIEW_PREFERENCES_UPDATES_PROXY,updatesProxyBox->title().replace("&","")));
	updatesMissingpartBox->setWhatsThis(lpubWT(      WT_CONTROL_LDVIEW_PREFERENCES_UPDATES_MISSING_PART,updatesMissingpartBox->title().replace("&","")));
	ExtraDirListView->setWhatsThis(lpubWT(           WT_CONTROL_LDVIEW_PREFERENCES_UPDATES_LDRAW_SEARCH_DIRECTORIES, tr("Extra Search Directories")));
	updateLDrawLibraryBox->setWhatsThis(lpubWT(      WT_CONTROL_LDVIEW_PREFERENCES_UPDATES_LDRAW_LIBRARY, updateLDrawLibraryBox->title().replace("&","")));
	preferenceSetList->setWhatsThis(lpubWT(          WT_CONTROL_LDVIEW_PREFERENCES_PREFERENCE_SET,tr("Preferences Set List")));

	connect( aaLinesButton, SIGNAL( stateChanged(int) ), this, SLOT( enableApply() ) );
	connect( applyButton, SIGNAL( pressed() ), this, SLOT( doApply() ) );
	connect( okButton, SIGNAL( clicked() ), this, SLOT( doOk() ) );
	connect( cancelButton, SIGNAL( clicked() ), this, SLOT( doCancel() ) );
	connect( primitvesResetButton, SIGNAL( clicked() ), this, SLOT( doResetPrimitives() ) );
	connect( seamWidthButton, SIGNAL( clicked() ), this, SLOT( enableApply() ) );
	connect( seamWidthDoubleSpin, SIGNAL( valueChanged(double) ), this, SLOT( enableApply() ) );
	connect( fieldOfViewDoubleSpin, SIGNAL( valueChanged(double) ), this, SLOT( enableApply() ) );
	connect( memoryUsageBox, SIGNAL( activated(int) ), this, SLOT( enableApply() ) );
	connect( snapshotSaveDirBox, SIGNAL( activated(int) ), this, SLOT( snapshotSaveDirBoxChanged() ) );
	connect( partsListsSaveDirBox, SIGNAL( activated(int) ), this, SLOT( partsListsSaveDirBoxChanged() ) );
	connect( exportsListsSaveDirBox, SIGNAL( activated(int) ), this, SLOT( exportsListsSaveDirBoxChanged() ) );
	connect( snapshotSaveDirButton, SIGNAL( clicked() ), this, SLOT( snapshotSaveDirBrowse() ) );
	connect( partsListsSaveDirButton, SIGNAL( clicked() ), this, SLOT( partsListsSaveDirBrowse() ) );
	connect( exportsSaveDirButton, SIGNAL( clicked() ), this, SLOT( exportsSaveDirBrowse() ) );
	connect( customConfigButton, SIGNAL( clicked() ), this, SLOT( customConfigBrowse() ) );
	connect( transparentButton, SIGNAL( stateChanged(int) ), this, SLOT( enableApply() ) );
	connect( wireframeThicknessSlider, SIGNAL( valueChanged(int) ), this, SLOT( enableApply() ) );
	connect( edgesOnlyButton, SIGNAL( stateChanged(int) ), this, SLOT( enableApply() ) );
	connect( conditionalLinesButton, SIGNAL( stateChanged(int) ), this, SLOT( enableApply() ) );
	connect( conditionalLinesButton, SIGNAL( toggled(bool) ), this, SLOT( doConditionalShow(bool) ) );
	connect( conditionalShowAllButton, SIGNAL( stateChanged(int) ), this, SLOT( enableApply() ) );
	connect( conditionalShowControlPtsButton, SIGNAL( stateChanged(int) ), this, SLOT( enableApply() ) );
	connect( wireframeFogButton, SIGNAL( stateChanged(int) ), this, SLOT( enableApply() ) );
	connect( wireframeRemoveHiddenLineButton, SIGNAL( stateChanged(int) ), this, SLOT( enableApply() ) );
	connect( highQualityLinesButton, SIGNAL( stateChanged(int) ), this, SLOT( enableApply() ) );
	connect( alwaysBlackLinesButton, SIGNAL( stateChanged(int) ), this, SLOT( enableApply() ) );
	connect( edgeThicknessSlider, SIGNAL( valueChanged(int) ), this, SLOT( enableApply() ) );
	connect( processLdconfigLdrButton, SIGNAL( stateChanged(int) ), this, SLOT( enableApply() ) );
	connect( randomColorsButton, SIGNAL( stateChanged(int) ), this, SLOT( enableApply() ) );
	connect( partBoundingBoxOnlyBox, SIGNAL( stateChanged(int) ), this, SLOT( enableApply() ) );
	connect( qualityLightingButton, SIGNAL( stateChanged(int) ), this, SLOT( enableApply() ) );
	connect( subduedLightingButton, SIGNAL( stateChanged(int) ), this, SLOT( enableApply() ) );
	connect( specularLightingButton, SIGNAL( stateChanged(int) ), this, SLOT( enableApply() ) );
	connect( alternateLightingButton, SIGNAL( stateChanged(int) ), this, SLOT( enableApply() ) );
	connect( crossEyedStereoButton, SIGNAL( toggled(bool) ), this, SLOT( enableApply() ) );
	connect( parallelStereoButton, SIGNAL( toggled(bool) ), this, SLOT( enableApply() ) );
	connect( stereoAmountSlider, SIGNAL( valueChanged(int) ), this, SLOT( enableApply() ) );
	connect( colorCutawayButton, SIGNAL( toggled(bool) ), this, SLOT( enableApply() ) );
	connect( monochromeCutawayButton, SIGNAL( toggled(bool) ), this, SLOT( enableApply() ) );
	connect( sortTransparencyButton, SIGNAL( toggled(bool) ), this, SLOT( doSortTransparency(bool) ) );
	connect( cutawayOpacitySlider, SIGNAL( valueChanged(int) ), this, SLOT( enableApply() ) );
	connect( cutawayThicknessSlider, SIGNAL( valueChanged(int) ), this, SLOT( enableApply() ) );
	connect( stippleTransparencyButton, SIGNAL( toggled(bool) ), this, SLOT( doStippleTransparency(bool) ) );
	connect( flatShadingButton, SIGNAL( stateChanged(int) ), this, SLOT( enableApply() ) );
	connect( smoothCurvesButton, SIGNAL( stateChanged(int) ), this, SLOT( enableApply() ) );
	connect( nearestFilteringButton, SIGNAL( toggled(bool) ), this, SLOT( enableApply() ) );
	connect( bilinearFilteringButton, SIGNAL( toggled(bool) ), this, SLOT( enableApply() ) );
	connect( trilinearFilteringButton, SIGNAL( toggled(bool) ), this, SLOT( enableApply() ) );
	connect( anisotropicFilteringButton, SIGNAL( toggled(bool) ), this, SLOT( enableApply() ) );
	connect( anisotropicFilteringButton, SIGNAL( toggled(bool) ), this, SLOT( doAnisotropic() ) );
	connect( anisotropicFilteringSlider, SIGNAL( valueChanged(int) ), this, SLOT( doAnisotropicSlider(int) ) );
	connect( anisotropicFilteringSlider, SIGNAL( valueChanged(int) ), this, SLOT( enableApply() ) );
	connect( curveQualitySlider, SIGNAL( valueChanged(int) ), this, SLOT( enableApply() ) );
	connect( lowQualityStudsButton, SIGNAL( stateChanged(int) ), this, SLOT( enableApply() ) );
	connect( hiresPrimitivesButton, SIGNAL( stateChanged(int) ), this, SLOT( enableApply() ) );
	connect( generalResetButton, SIGNAL( clicked() ), this, SLOT( doResetGeneral() ) );
	connect( geometryResetButton, SIGNAL( clicked() ), this, SLOT( doResetGeometry() ) );
	connect( effectsResetButton, SIGNAL( clicked() ), this, SLOT( doResetEffects() ) );
	connect( updatesResetButton, SIGNAL( clicked() ), this, SLOT( doResetUpdates() ) );
	connect( updatesResetTimesButton, SIGNAL( clicked () ), this, SLOT (doResetTimesUpdates() ) );
	connect( wireframeButton, SIGNAL( toggled(bool) ), this, SLOT( doWireframe(bool) ) );
	connect( enableBFCButton, SIGNAL( toggled(bool) ), this, SLOT( doBFC(bool) ) );
	connect( enableBFCButton, SIGNAL( toggled(bool) ), this, SLOT( enableApply() ) );
	connect( bfcRedBackFaceButton, SIGNAL( toggled(bool) ), this, SLOT( enableApply() ) );
	connect( bfcGreenFrontFaceButton, SIGNAL( toggled(bool) ), this, SLOT( enableApply() ) );
	connect( bfcBlueNeutralFaceButton, SIGNAL( toggled(bool) ), this, SLOT( enableApply() ) );
	connect( wireframeButton, SIGNAL( toggled(bool) ), this, SLOT( enableApply() ) );
	connect( edgeLinesButton, SIGNAL( toggled(bool) ), this, SLOT( enableApply() ) );
	connect( edgeLinesButton, SIGNAL( toggled(bool) ), this, SLOT( doEdgeLines(bool) ) );
	connect( lightingButton, SIGNAL( toggled(bool) ), this, SLOT( enableApply() ) );
	connect( lightingButton, SIGNAL( toggled(bool) ), this, SLOT( doLighting(bool) ) );
	connect( lightingDir11, SIGNAL( toggled(bool) ), this, SLOT( enableApply() ) );
	connect( lightingDir12, SIGNAL( toggled(bool) ), this, SLOT( enableApply() ) );
	connect( lightingDir13, SIGNAL( toggled(bool) ), this, SLOT( enableApply() ) );
	connect( lightingDir21, SIGNAL( toggled(bool) ), this, SLOT( enableApply() ) );
	connect( lightingDir22, SIGNAL( toggled(bool) ), this, SLOT( enableApply() ) );
	connect( lightingDir23, SIGNAL( toggled(bool) ), this, SLOT( enableApply() ) );
	connect( lightingDir31, SIGNAL( toggled(bool) ), this, SLOT( enableApply() ) );
	connect( lightingDir32, SIGNAL( toggled(bool) ), this, SLOT( enableApply() ) );
	connect( lightingDir33, SIGNAL( toggled(bool) ), this, SLOT( enableApply() ) );
	connect( effectsUseLIGHTDATButton, SIGNAL( toggled(bool) ), this, SLOT( enableApply() ) );
	connect( effectsUseLIGHTDATButton, SIGNAL( toggled(bool) ), this, SLOT( doDrawLightDats() ) );
	connect( effectsReplaceStandarLightButton, SIGNAL( toggled(bool) ), this, SLOT( enableApply() ) );
	connect( effectsHideLIGHTButton, SIGNAL( toggled(bool) ), this, SLOT( enableApply() ) );
	connect( stereoButton, SIGNAL( toggled(bool) ), this, SLOT( doStereo(bool) ) );
	connect( stereoButton, SIGNAL( toggled(bool) ), this, SLOT( enableApply() ) );
	connect( wireframeCutawayButton, SIGNAL( toggled(bool) ), this, SLOT( doWireframeCutaway(bool) ) );
	connect( wireframeCutawayButton, SIGNAL( toggled(bool) ), this, SLOT( enableApply() ) );
	connect( primitiveSubstitutionButton, SIGNAL( toggled(bool) ), this, SLOT( enableApply() ) );
	connect( primitiveSubstitutionButton, SIGNAL( toggled(bool) ), this, SLOT( doPrimitiveSubstitution(bool) ) );
	connect( textureStudsButton, SIGNAL( stateChanged(int) ), this, SLOT( enableApply() ) );
	connect( textureStudsButton, SIGNAL( toggled(bool) ), this, SLOT( doTextureStuds(bool) ) );
	connect( useTextureMapsButton, SIGNAL( stateChanged(int) ), this, SLOT( enableApply() ) );
	connect( useTextureMapsButton, SIGNAL( toggled(bool) ), this, SLOT( doTextureStuds(bool) ) );
	connect( newPreferenceSetButton, SIGNAL( clicked() ), this, SLOT( doNewPreferenceSet() ) );
	connect( delPreferenceSetButton, SIGNAL( clicked() ), this, SLOT( doDelPreferenceSet() ) );
	connect( hotkeyPreferenceSetButton, SIGNAL( clicked() ), this, SLOT( doHotkeyPreferenceSet() ) );
	connect( preferenceSetList, SIGNAL( currentItemChanged(QListWidgetItem *,QListWidgetItem *) ), this, SLOT( doPrefSetSelected(QListWidgetItem *,QListWidgetItem *) ) );
	connect( backgroundColorButton, SIGNAL( clicked() ), this, SLOT( doBackgroundColor() ) );
	connect( defaultColorButton, SIGNAL( clicked() ), this, SLOT( doDefaultColor() ) );
	connect( updatesNoproxyButton, SIGNAL( toggled(bool) ), this, SLOT( disableProxy() ) );
	connect( updatesWindowsproxyButton, SIGNAL( toggled(bool) ), this, SLOT( enableProxy() ) );
	connect( updatesProxyButton, SIGNAL( toggled(bool) ), this, SLOT( enableProxy() ) );
	connect( updatesMissingpartsButton, SIGNAL( toggled(bool) ), this, SLOT( doUpdateMissingparts(bool) ) );
	connect( updatesMissingpartsButton, SIGNAL( toggled(bool) ), this, SLOT( enableApply() ) );
	connect( transparentOffsetSlider, SIGNAL( valueChanged(int) ), this, SLOT( enableApply() ) );

	connect( addExtraDirButton, SIGNAL( clicked() ), this, SLOT( doAddExtraDir() ) );
	connect( delExtraDirButton, SIGNAL( clicked() ), this, SLOT( doDelExtraDir() ) );
	connect( upExtraDirButton, SIGNAL( clicked() ), this, SLOT( doUpExtraDir() ) );
	connect( downExtraDirButton, SIGNAL( clicked() ), this, SLOT( doDownExtraDir() ) );
	connect( ExtraDirListView, SIGNAL( currentItemChanged ( QListWidgetItem * , QListWidgetItem * ) ), this, SLOT( doExtraDirSelected(QListWidgetItem *,QListWidgetItem *) ) );
	connect( ldrawDirButton, SIGNAL(clicked() ), this, SLOT( doLDrawDir() ) );
	connect( ldrawZipButton, SIGNAL(clicked() ), this, SLOT( doLDrawZip() ) );
	if (!extraSearchDirs)
	{
		extraSearchDirs = new TCStringArray;
		populateExtraSearchDirs();
	}

	connect( automateEdgeColorBox, SIGNAL( toggled(bool) ), this, SLOT( enableApply() ) );
	connect( automateEdgeColorBox, SIGNAL(toggled(bool)), this, SLOT(enableAutomateEdgeColorButton()) );
	connect( automateEdgeColorButton, SIGNAL(clicked()), this, SLOT(automateEdgeColor()) );
	connect( studStyleCombo, SIGNAL( currentIndexChanged(int) ), this, SLOT( enableApply() ) );
	connect( studStyleCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(enableHighContrastButton(int)) );
	connect( highContrastButton, SIGNAL(clicked()), this, SLOT(automateEdgeColor()) );

	loadSettings();
#ifdef WIN32
	setupAntialiasing();
	connect( fsaaModeBox, SIGNAL( currentIndexChanged(int) ), this, SLOT( enableApply() ) );
	connect( fsaaModeBox, SIGNAL( currentIndexChanged(const QString &) ), this, SLOT( fsaaModeBoxChanged(const QString &) ) );
	connect( havePixelBufferButton, SIGNAL( clicked() ), this, SLOT( enableApply() ) );
	connect( updateLDrawLibraryButton, SIGNAL( clicked () ), this, SLOT(doLibraryCheckForUpdates() ) );
#else
	fsaaModeBox->setDisabled(true);
	fsaaModeBox->hide();
	fsaaModeLabel->hide();
	havePixelBufferButton->hide();
	updatesWindowsproxyButton->hide();
#endif // WIN32

	ldPrefs->applySettings();

	reflectSettings();

	QStyle *style = defaultColorButton->style();
	if (style != nullptr)
	{
		QString styleName = style->metaObject()->className();
		if (styleName == "QGtkStyle")
		{
			// QGtkStyle uses an image for the background, and doesn't show
			// the background color at all, so update the color buttons to use
			// the QWindowsStyle instead.
			// In Qt5 no need for changing the style, background color can be
			// set
			defaultColorButton->setStyle(QStyleFactory::create("Windows"));
			backgroundColorButton->setStyle(QStyleFactory::create("Windows"));
		}
	}

	ldvModelWidget = modelWidget;

	this->setWindowTitle(ldvModelWidget->getIniTitle().append(" Preferences"));

	QPalette readOnlyPalette = QApplication::palette();
	if (ldvModelWidget->getDarkTheme())
		readOnlyPalette.setColor(QPalette::Base,QColor("#3E3E3E")); // THEME_DARK_PALETTE_MIDLIGHT
	else
		readOnlyPalette.setColor(QPalette::Base,QColor("#AEADAC")); // THEME_DEFAULT_PALETTE_LIGHT
	readOnlyPalette.setColor(QPalette::Text,QColor("#808080"));     // LPUB3D_DISABLED_TEXT_COLOUR

	QString iniFileMessage;
	if (TCUserDefaults::isIniFileSet())
	{
		QString prefSet = TCUserDefaults::getSessionName();
		iniFileMessage = QString("%1").arg(ldvModelWidget->getIniFile());
		iniBox->setTitle(QString("INI file using '%1' preference set")
						 .arg(prefSet.isEmpty() ? "Default" : prefSet));
	} else {
		iniFileMessage = QString("INI file not specified. Using built-in default settings.");
		iniFileEdit->setStyleSheet("QLineEdit { background-color : red; color : white; }");
	}

	iniFileEdit->setReadOnly(true);
	iniFileEdit->setPalette(readOnlyPalette);
	iniFileEdit->setText(iniFileMessage);

	IniFlag iniFlag = ldvModelWidget->getIniFlag();
	if (iniFlag == LDViewIni) {
		usingLDView = true;
		defaultPartlistDirLabel->hide();
		partsListsSaveDirBox->hide();
		partsListsSaveDirEdit->hide();
		partsListsSaveDirButton->hide();
	}
	else
	{
		usingLDView = iniFlag == POVRayRender ? true : false;
		if (iniFlag == POVRayRender ||
			iniFlag == NativePOVIni ||
			iniFlag == NativeSTLIni ||
			iniFlag == Native3DSIni) {
			// Snapshots Save Directory
			defaultSnapshotDirLabel->hide();
			snapshotSaveDirBox->hide();
			snapshotSaveDirEdit->hide();
			snapshotSaveDirButton->hide();
			// Part List Save Directory
			defaultPartlistDirLabel->hide();
			partsListsSaveDirBox->hide();
			partsListsSaveDirEdit->hide();
			partsListsSaveDirButton->hide();
		} else {
			// iniFlag = NativePartList (HTML)
			// Snapshots Save Directory
			defaultSnapshotDirLabel->hide();
			snapshotSaveDirBox->hide();
			snapshotSaveDirEdit->hide();
			snapshotSaveDirButton->hide();
			// Exports Save Directory
			exportsDirLabel->hide();
			exportsListsSaveDirBox->hide();
			exportsSaveDirEdit->hide();
			exportsSaveDirButton->hide();
		}

		// Remove Updates Tab if not using LDView renderder
		if (!usingLDView)
			tabs->removeTab(tabs->indexOf(updatesTab));
	}

	applyButton->setEnabled(false);

#ifdef DEBUG
	setDebugLevel((int)TCUserDefaults::longForKey(DEBUG_LEVEL_KEY, 1, false));
#else // DEBUG
	setDebugLevel((int)TCUserDefaults::longForKey(DEBUG_LEVEL_KEY, 0, false));
#endif // DEBUG

	setMinimumSize(50,50);
}

LDVPreferences::~LDVPreferences(void)
{
}

void LDVPreferences::show(void)
{
	applyButton->setEnabled(false);
	QDialog::show();
	raise();
	//setActiveWindow();
}

void LDVPreferences::doPrefSetsApply(void)
{
	TCStringArray *oldPrefSetNames = TCUserDefaults::getAllSessionNames();
	int i;
	int b;
	int count = oldPrefSetNames->getCount();
//	char *prefSetName = nullptr;
	const char *sessionName = TCUserDefaults::getSessionName();
	bool changed = false;

	for (i = 0; i < count; i++)
	{
		char *oldPrefSetName = oldPrefSetNames->stringAtIndex(i);
		int index=-1;
		for(b = 0; b < preferenceSetList->count(); b++)
		{
			if (strcmp(oldPrefSetNames->stringAtIndex(i),
						preferenceSetList->item(b)->text().toUtf8().constData()) == 0)
			{
				index = b;
			}
		}
		if (index == -1)
		{
			TCUserDefaults::removeSession(oldPrefSetName);
		}
	}
	count=preferenceSetList->count();
	for(i = 1; i < count; i++)
	{
		if (oldPrefSetNames->indexOfString(getPrefSet(i)) < 0)
		{
			TCUserDefaults::setSessionName(getPrefSet(i), PREFERENCE_SET_KEY);
		}
	}
	oldPrefSetNames->release();
	if (getSelectedPrefSet() &&
		(strcmp(getSelectedPrefSet(), DEFAULT_PREF_SET) == 0))
	{
		if (sessionName && sessionName[0])
		{
			TCUserDefaults::setSessionName(nullptr, PREFERENCE_SET_KEY);
			changed = true;
		}
	}
	else
	{
		const char *selectedPrefSet = getSelectedPrefSet();
		if (!sessionName || !selectedPrefSet ||
			strcmp(sessionName, selectedPrefSet) != 0)
		{
			TCUserDefaults::setSessionName(selectedPrefSet, PREFERENCE_SET_KEY);
			changed = true;
		}
	}
	if (changed)
	{
		loadSettings();
		reflectSettings();

		doGeneralApply();
		doLDrawApply();
		doGeometryApply();
		doEffectsApply();
		doPrimitivesApply();
		applyButton->setEnabled(false);
		if (modelViewer)
		{
			setupDefaultRotationMatrix();
		}
		checkAbandon = true;
	}
}

void LDVPreferences::doGeneralApply(void)
{
	int r, g, b;

	ldPrefs->setLineSmoothing(aaLinesButton->checkState());
	ldPrefs->setProcessLdConfig(processLdconfigLdrButton->checkState());
	ldPrefs->setRandomColors(randomColorsButton->checkState());

	ldPrefs->getBackgroundColor(r, g, b);
	if (backgroundColor.isValid() && backgroundColor != QColor(r,g,b)){
	   backgroundColor.getRgb(&r, &g, &b);
	   ldPrefs->setDefaultColor(r, g, b);
	}

	ldPrefs->getDefaultColor(r, g, b);
	if (defaultColor.isValid() && defaultColor != QColor(r,g,b)){
	   defaultColor.getRgb(&r, &g, &b);
	   ldPrefs->setDefaultColor(r, g, b);
	}

	ldPrefs->setFov(fieldOfViewDoubleSpin->value());
	ldPrefs->setMemoryUsage(memoryUsageBox->currentIndex());
	ldPrefs->setTransDefaultColor(transparentButton->checkState());
	LDPreferences::DefaultDirMode snapshotDirMode, partsListDirMode, exportDirMode;
	ldPrefs->setSnapshotsDirMode(snapshotDirMode =
				(LDPreferences::DefaultDirMode)snapshotSaveDirBox->currentIndex());
	if (snapshotDirMode == LDPreferences::DDMSpecificDir)
	{
		snapshotDir = snapshotSaveDirEdit->text();
		if(snapshotDir.length()>0)
		{
			ldPrefs->setSnapshotsDir(snapshotDir.toUtf8().constData());
		}
		else
		{
			ldPrefs->setSnapshotsDirMode(LDPreferences::DDMLastDir);
		}
	}
	ldPrefs->setPartsListsDirMode(partsListDirMode =
			(LDPreferences::DefaultDirMode)partsListsSaveDirBox->currentIndex());
	if (partsListDirMode == LDPreferences::DDMSpecificDir)
	{
		partsListDir = partsListsSaveDirEdit->text();
		if (partsListDir.length() > 0)
		{
			ldPrefs->setPartsListsDir(partsListDir.toUtf8().constData());
		}
		else
		{
			ldPrefs->setPartsListsDirMode(LDPreferences::DDMLastDir);
		}
	}
	ldPrefs->setSaveDirMode(LDPreferences::SOExport, exportDirMode =
			(LDPreferences::DefaultDirMode)exportsListsSaveDirBox->currentIndex());
	if (exportDirMode == LDPreferences::DDMSpecificDir)
	{
		exportDir = exportsSaveDirEdit->text();
		if (exportDir.length() > 0)
		{
			ldPrefs->setSaveDir(LDPreferences::SOExport, exportDir.toUtf8().constData());
		}
		else
		{
			ldPrefs->setSaveDirMode(LDPreferences::SOExport,
					LDPreferences::DDMLastDir);
		}
	}
	// Other General Settings
	TCUserDefaults::setBoolForKey(autoCropButton->isChecked(),
								  AUTO_CROP_KEY, false);
	TCUserDefaults::setBoolForKey(transparentBackgroundButton->isChecked(),
								  SAVE_ALPHA_KEY, false);
#ifdef WIN32
	TCUserDefaults::setLongForKey(havePixelBufferButton->isChecked() ? 1 : 0,
								  IGNORE_PBUFFER_KEY, false);

#endif

	ldPrefs->setCustomConfigPath(customConfigEdit->text().toUtf8().constData());
	ldPrefs->applyGeneralSettings();
	ldPrefs->commitGeneralSettings();
}

void LDVPreferences::doLDrawApply(void)
{
	const char *lDrawDir = copyString(ldrawDirEdit->text().toUtf8().constData());
	ldPrefs->setLDrawDir(lDrawDir);
	if (lDrawDir)
		LDVPreferences::setLDrawDir(lDrawDir);
	const char *ldrawZip = copyString(ldrawZipEdit->text().toUtf8().constData());
	ldPrefs->setLDrawZipPath(ldrawZip);
	if (ldrawZip)
		LDVPreferences::setLDrawZipPath(ldrawZip);
	recordExtraSearchDirs();
	ldPrefs->applyLDrawSettings();
	ldPrefs->commitLDrawSettings();
}

void LDVPreferences::doGeometryApply(void)
{
	ldPrefs->setUseSeams(seamWidthButton->isChecked());
	ldPrefs->setBoundingBoxesOnly(partBoundingBoxOnlyBox->isChecked());
	ldPrefs->setSeamWidth(int(seamWidthDoubleSpin->value() * 100.0f));
	ldPrefs->setDrawWireframe(wireframeButton->isChecked());
	if (ldPrefs->getDrawWireframe())
	{
		ldPrefs->setUseWireframeFog(wireframeFogButton->isChecked());
		ldPrefs->setRemoveHiddenLines(
			wireframeRemoveHiddenLineButton->isChecked());
	}
	ldPrefs->setBfc(enableBFCButton->isChecked());
	if (ldPrefs->getBfc())
	{
		ldPrefs->setRedBackFaces(bfcRedBackFaceButton->isChecked());
		ldPrefs->setGreenFrontFaces(bfcGreenFrontFaceButton->isChecked());
		ldPrefs->setBlueNeutralFaces(bfcBlueNeutralFaceButton->isChecked());
	}
	ldPrefs->setWireframeThickness(wireframeThicknessSlider->value());
	ldPrefs->setShowHighlightLines(edgeLinesButton->isChecked());
	if (ldPrefs->getShowHighlightLines())
	{
		ldPrefs->setEdgesOnly(edgesOnlyButton->isChecked());
		ldPrefs->setDrawConditionalHighlights(
			conditionalLinesButton->isChecked());
		if (ldPrefs->getDrawConditionalHighlights())
		{
			ldPrefs->setShowAllConditionalLines(
				conditionalShowAllButton->isChecked());
			ldPrefs->setShowConditionalControlPoints(
				conditionalShowControlPtsButton->isChecked());
		}
		ldPrefs->setUsePolygonOffset(highQualityLinesButton->isChecked());
		ldPrefs->setBlackHighlights(alwaysBlackLinesButton->isChecked());
		ldPrefs->setEdgeThickness(edgeThicknessSlider->value());
	}
	ldPrefs->applyGeometrySettings();
	ldPrefs->commitGeometrySettings();
}

LDPreferences::LightDirection LDVPreferences::getSelectedLightDirection(void)
{
	LDPreferences::LightDirection lightDirection =
		LDPreferences::CustomDirection;
	if(lightingDir11->isChecked())
	{
		lightDirection = LDPreferences::UpperLeft;
	}
	else if (lightingDir12->isChecked())
	{
	lightDirection = LDPreferences::UpperMiddle;
	}
	else if (lightingDir13->isChecked())
	{
		lightDirection = LDPreferences::UpperRight;
	}
	else if (lightingDir21->isChecked())
	{
		lightDirection = LDPreferences::MiddleLeft;
	}
	else if (lightingDir22->isChecked())
	{
		lightDirection = LDPreferences::MiddleMiddle;
	}
	else if (lightingDir23->isChecked())
	{
		lightDirection = LDPreferences::MiddleRight;
	}
	else if (lightingDir31->isChecked())
	{
		lightDirection = LDPreferences::LowerLeft;
	}
	else if (lightingDir32->isChecked())
	{
		lightDirection = LDPreferences::LowerMiddle;
	}
	else if (lightingDir33->isChecked())
	{
		lightDirection = LDPreferences::LowerRight;
	}
	return lightDirection;
}

void LDVPreferences::doEffectsApply(void)
{
	LDVStereoMode smTemp = LDVStereoNone;
	LDVCutawayMode cmTemp = LDVCutawayNormal;

	ldPrefs->setUseLighting(lightingButton->isChecked());
	if (ldPrefs->getUseLighting())
	{
		LDPreferences::LightDirection lightDirection =
			getSelectedLightDirection();
		ldPrefs->setQualityLighting(qualityLightingButton->isChecked());
		ldPrefs->setSubduedLighting(subduedLightingButton->isChecked());
		ldPrefs->setUseSpecular(specularLightingButton->isChecked());
		ldPrefs->setOneLight(alternateLightingButton->isChecked());
		if (lightDirection != LDPreferences::CustomDirection)
		{
			ldPrefs->setLightDirection(lightDirection);
		}
		if (effectsUseLIGHTDATButton->isChecked())
		{
			ldPrefs->setDrawLightDats(true);
			ldPrefs->setOptionalStandardLight(
				effectsReplaceStandarLightButton->isChecked());
		}
		else
		{
			ldPrefs->setDrawLightDats(false);
		}
	}
	ldPrefs->setNoLightGeom(effectsHideLIGHTButton->isChecked());
	if (!stereoButton->isChecked())
	{
		smTemp = LDVStereoNone;
	}
	else if (crossEyedStereoButton->isChecked())
	{
		smTemp = LDVStereoCrossEyed;
	}
	else if (parallelStereoButton->isChecked())
	{
		smTemp = LDVStereoParallel;
	}
	ldPrefs->setStereoMode(smTemp);
	ldPrefs->setStereoEyeSpacing(stereoAmountSlider->value());
	if (!wireframeCutawayButton->isChecked())
	{
		cmTemp = LDVCutawayNormal;
	}
	else if (colorCutawayButton->isChecked())
	{
		cmTemp = LDVCutawayWireframe;
	}
	else if (monochromeCutawayButton->isChecked())
	{
		cmTemp = LDVCutawayStencil;
	}
	ldPrefs->setCutawayMode(cmTemp);
	ldPrefs->setCutawayAlpha(cutawayOpacitySlider->value());
	ldPrefs->setCutawayThickness(cutawayThicknessSlider->value());
	ldPrefs->setSortTransparent(sortTransparencyButton->isChecked());
	ldPrefs->setUseStipple(stippleTransparencyButton->isChecked());
	ldPrefs->setUseFlatShading(flatShadingButton->isChecked());
	ldPrefs->setPerformSmoothing(smoothCurvesButton->isChecked());
	ldPrefs->applyEffectsSettings();
	ldPrefs->commitEffectsSettings();
}

void LDVPreferences::setAniso(int value)
{
	QString s;
	int intLevel = 1 << value;
	s = s.setNum(intLevel);
	s+="x";
	if (value < 1 ) s = "";
	anisotropicLabel->setText(s);
	anisotropicFilteringSlider->setValue(value);
	ldPrefs->setAnisoLevel(intLevel);
}

void LDVPreferences::doPrimitivesApply(void)
{
	int aniso = 0;
	bool aps,texmaps;
	ldPrefs->setTexmaps(texmaps = useTextureMapsButton->isChecked());
	ldPrefs->setAllowPrimitiveSubstitution(aps =
		primitiveSubstitutionButton->isChecked());
	if (ldPrefs->getAllowPrimitiveSubstitution())
	{
		ldPrefs->setTextureStuds(textureStudsButton->isChecked());
		ldPrefs->setCurveQuality(curveQualitySlider->value());
	}
	if (texmaps)
	{
		ldPrefs->setTextureOffsetFactor(transparentOffsetSlider->value()/10.0f);
	}
	if ((aps && ldPrefs->getTextureStuds()) || texmaps)
	{
		int iTemp = GL_NEAREST_MIPMAP_NEAREST;

		if (nearestFilteringButton->isChecked())
		{
			iTemp = GL_NEAREST_MIPMAP_NEAREST;
		}
		else if (bilinearFilteringButton->isChecked())
		{
			iTemp = GL_LINEAR_MIPMAP_NEAREST;
		}
		else if (trilinearFilteringButton->isChecked())
		{
			iTemp = GL_LINEAR_MIPMAP_LINEAR;
		}
		else if (anisotropicFilteringButton->isChecked())
		{
			iTemp = GL_LINEAR_MIPMAP_LINEAR;
			aniso = anisotropicFilteringSlider->value();
		}
		ldPrefs->setTextureFilterType(iTemp);
		setAniso(aniso);
	}
	ldPrefs->setQualityStuds(!lowQualityStudsButton->isChecked());
	ldPrefs->setHiResPrimitives(hiresPrimitivesButton->isChecked());
	ldPrefs->setAutomateEdgeColor(automateEdgeColorBox->isChecked());
	ldPrefs->setStudStyle(studStyleCombo->currentIndex());
	ldPrefs->applyPrimitivesSettings();
	ldPrefs->commitPrimitivesSettings();
}

void LDVPreferences::doUpdatesApply()
{
	int  iTemp;
	bool ok;

	ldPrefs->setCheckPartTracker(updatesMissingpartsButton->isChecked());
#ifdef WIN32
	if (updatesWindowsproxyButton->isChecked())
	{
		ldPrefs->setProxyType(1);
	} else
#endif // WIN32
	if (updatesProxyButton->isChecked())
	{
		ldPrefs->setProxyType(2);
	}
	else
	{
		ldPrefs->setProxyType(0);
	}
	iTemp = portEdit->text().toInt(&ok);
	if (ok)
	{
		ldPrefs->setProxyPort(iTemp);
	}
	iTemp = daymissingpartcheckText->text().toInt(&ok);
	if (ok)
	{
		ldPrefs->setMissingPartWait(iTemp);
	}
	iTemp = dayupdatedpartcheckText->text().toInt(&ok);
	if (ok)
	{
		ldPrefs->setUpdatedPartWait(iTemp);
	}
	ldPrefs->setProxyServer(proxyEdit->text().toUtf8().constData());
	ldPrefs->applyUpdatesSettings();
	ldPrefs->commitUpdatesSettings();
}

void LDVPreferences::doBackgroundColor()
{
/*** LPub3D Mod - use button icon image ***/
	int r,g,b;
	QPixmap pix(12, 12);
	QString title("Select Background Color");
	QColorDialog::ColorDialogOptions dialogOptions = QColorDialog::ShowAlphaChannel;

	ldPrefs->getBackgroundColor(r, g, b);
	QColor oldColor = QColor(r,g,b);
	QColor newColor = QColorDialog::getColor(oldColor, this, title, dialogOptions);
	if(newColor.isValid() && newColor != oldColor )
	{
		 backgroundColor = newColor;
		 pix.fill(newColor);
		 backgroundColorButton->setIcon(pix);
		 applyButton->setEnabled(true);
	}
/*** LPub3D Mod end ***/
}

void LDVPreferences::doDefaultColor()
{
/*** LPub3D Mod - use button icon image ***/
	int r, g, b, a, i;
	QRgb old[16];
	QPixmap pix(12, 12);
	QString title("Select Default Color");
	QColorDialog::ColorDialogOptions dialogOptions = QColorDialog::ShowAlphaChannel;

	for (i = 0 ; i < 16; i++)
	{
		old[i] = QColorDialog::customColor(i).rgb();
		LDLPalette::getDefaultRGBA(i, r, g, b, a);
		QColorDialog::setCustomColor(i, qRgb(r, g, b));
	}
	ldPrefs->getDefaultColor(r, g, b);
	QColor oldColor = QColor(r,g,b);
	QColor newColor = QColorDialog::getColor(oldColor, this, title, dialogOptions);
	if(newColor.isValid() && newColor != oldColor )
	{
		defaultColor = newColor;
		pix.fill(newColor);
		defaultColorButton->setIcon(pix);
		applyButton->setEnabled(true);
	}
/*** LPub3D Mod end ***/
	for (i = 0 ; i <16 ; i++)
		QColorDialog::setCustomColor(i, old[i]);
}

void LDVPreferences::doApply(void)
{
	doGeneralApply();
	doLDrawApply();
	doGeometryApply();
	doEffectsApply();
	doPrimitivesApply();
	if (usingLDView)
		doUpdatesApply();
	doPrefSetsApply();
	applyButton->setEnabled(false);
	if (modelViewer)
	   setupDefaultRotationMatrix();
	checkAbandon = true;
}

void LDVPreferences::doOk(void)
{
	doApply();
	QDialog::accept();
	QDialog::close();
}

void LDVPreferences::doCancel(void)
{
	QDialog::reject();
	QDialog::close();
}

void LDVPreferences::doAnisotropic(void)
{
	bool x = anisotropicFilteringButton->isChecked();
	anisotropicFilteringSlider->setEnabled(x);
	anisotropicLabel->setEnabled(x);
	doAnisotropicSlider(anisotropicFilteringSlider->value());
}

void LDVPreferences::doAnisotropicSlider(int i)
{
	QString s;
	s = s.setNum(1 << i);
	s+="x";
	anisotropicLabel->setText(s);
}

bool LDVPreferences::getAllowPrimitiveSubstitution(void)
{
	return ldPrefs->getAllowPrimitiveSubstitution();
}

void LDVPreferences::setButtonState(QCheckBox *button, bool state)
{
	bool buttonState = button->isChecked();

	if (state != buttonState)
	{
		button->toggle();
	}
}

void LDVPreferences::setButtonState(QRadioButton *button, bool state)
{
	bool buttonState = button->isChecked();

	if (state != buttonState)
	{
		button->toggle();
	}
}

void LDVPreferences::setDoubleRangeValue(QDoubleSpinBox *rangeControl, float value)
{
	float rangeValue = rangeControl->value();

	if (value != rangeValue)
	{
		rangeControl->setValue(value);
	}
}

void LDVPreferences::setRangeValue(QSpinBox *rangeControl, int value)
{
	int rangeValue = rangeControl->value();

	if (value != rangeValue)
	{
		rangeControl->setValue(value);
	}
}

void LDVPreferences::setRangeValue(QSlider *rangeControl, int value)
{
	int rangeValue = rangeControl->value();

	if (value != rangeValue)
	{
		rangeControl->setValue(value);
	}
}

void LDVPreferences::loadSettings(void)
{
	ldPrefs->loadSettings();
}

void LDVPreferences::setDrawWireframe(bool value)
{
	if (value != ldPrefs->getDrawWireframe())
	{
		ldPrefs->setDrawWireframe(value, true, true);
		reflectWireframeSettings();
	}
}

void LDVPreferences::setUseWireframeFog(bool value)
{
	if (value != ldPrefs->getUseWireframeFog())
	{
		ldPrefs->setUseWireframeFog(value, true, true);
		reflectWireframeSettings();
	}
}

void LDVPreferences::setRemoveHiddenLines(bool value)
{
	if (value != ldPrefs->getRemoveHiddenLines())
	{
		ldPrefs->setRemoveHiddenLines(value, true, true);
		reflectWireframeSettings();
	}
}

void LDVPreferences::setTextureStud(bool value)
{
	if (value != ldPrefs->getTextureStuds())
	{
		ldPrefs->setTextureStuds(value, true, true);
		reflectPrimitivesSettings();
	}
}

void LDVPreferences::setEdgeOnly(bool value)
{
	if (value != ldPrefs->getEdgesOnly())
	{
		ldPrefs->setEdgesOnly(value, true, true);
		reflectGeometrySettings();
	}
}

void LDVPreferences::setConditionalLine(bool value)
{
	if (value != ldPrefs->getDrawConditionalHighlights())
	{
		ldPrefs->setDrawConditionalHighlights(value, true, true);
		reflectGeometrySettings();
	}
}

void LDVPreferences::setHighQuality(bool value)
{
	if (value != ldPrefs->getUsePolygonOffset())
	{
		ldPrefs->setUsePolygonOffset(value, true, true);
		reflectGeometrySettings();
	}
}

void LDVPreferences::setAlwaysBlack(bool value)
{
	if (value != ldPrefs->getBlackHighlights())
	{
		ldPrefs->setBlackHighlights(value, true, true);
		reflectGeometrySettings();
	}
}

void LDVPreferences::setRedBackFaces(bool value)
{
	if (value != ldPrefs->getRedBackFaces())
	{
		ldPrefs->setRedBackFaces(value, true, true);
		reflectGeometrySettings();
	}
}

void LDVPreferences::setGreenFrontFaces(bool value)
{
	if (value != ldPrefs->getGreenFrontFaces())
	{
		ldPrefs->setGreenFrontFaces(value, true, true);
		reflectGeometrySettings();
	}
}

void LDVPreferences::setBlueNeutralFaces(bool value)
{
	if (value != ldPrefs->getBlueNeutralFaces())
	{
		ldPrefs->setBlueNeutralFaces(value, true, true);
		reflectGeometrySettings();
	}
}

void LDVPreferences::setShowsHighlightLines(bool value)
{
	if (value != ldPrefs->getShowHighlightLines())
	{
		ldPrefs->setShowHighlightLines(value, true, true);
		reflectGeometrySettings();
	}
}

void LDVPreferences::setAllowPrimitiveSubstitution(bool value)
{
	if (value != ldPrefs->getAllowPrimitiveSubstitution())
	{
		ldPrefs->setAllowPrimitiveSubstitution(value, true, true);
		reflectPrimitivesSettings();
	}
}

void LDVPreferences::setUseLighting(bool value)
{
	if (value != ldPrefs->getUseLighting())
	{
		ldPrefs->setUseLighting(value, true, true);
		reflectEffectsSettings();
	}
}

void LDVPreferences::setUseBFC(bool value)
{
	if (value != ldPrefs->getBfc())
	{
		ldPrefs->setBfc(value, true, true);
		reflectGeometrySettings();
	}
}

void LDVPreferences::setUseSeams(bool value)
{
	if (value != ldPrefs->getUseSeams())
	{
		ldPrefs->setUseSeams(value, true, true);
		reflectGeometrySettings();
	}
}

void LDVPreferences::reflectSettings(void)
{
	reflectGeneralSettings();
	reflectLDrawSettings();
	reflectGeometrySettings();
	reflectEffectsSettings();
	reflectPrimitivesSettings();
	if (usingLDView)
		reflectUpdatesSettings();
	setupPrefSetsList();
}

void LDVPreferences::reflectGeneralSettings(void)
{
	int r, g, b;
	QPixmap pix(12, 12);

	setButtonState(aaLinesButton, ldPrefs->getLineSmoothing());
	setButtonState(processLdconfigLdrButton,
		ldPrefs->getProcessLdConfig());
	setButtonState(randomColorsButton,ldPrefs->getRandomColors());

	ldPrefs->getBackgroundColor(r, g, b);
	pix.fill(QColor(r, g, b));
	backgroundColorButton->setIcon(pix);

	ldPrefs->getDefaultColor(r, g, b);
	pix.fill(QColor(r, g, b));
	defaultColorButton->setIcon(pix);

	setDoubleRangeValue(fieldOfViewDoubleSpin, (float)ldPrefs->getFov());
	setButtonState(transparentButton, ldPrefs->getTransDefaultColor());
	memoryUsageBox->setCurrentIndex(ldPrefs->getMemoryUsage());

	// Other General Settings
	autoCropButton->setChecked(
			TCUserDefaults::boolForKey(AUTO_CROP_KEY, false, false));
	transparentBackgroundButton->setChecked(
			TCUserDefaults::longForKey(SAVE_ALPHA_KEY, 0, false) != 0);
#ifdef WIN32
	havePixelBufferButton->setChecked(
			TCUserDefaults::longForKey(IGNORE_PBUFFER_KEY, 0, false) != 0);
#endif

	setupSaveDirs();
	customConfigEdit->setText(ldPrefs->getCustomConfigPath().c_str());
}

void LDVPreferences::reflectLDrawSettings(void)
{
	populateExtraSearchDirs();
	populateExtraDirsListBox();
	ldrawZipEdit->setText(ldPrefs->getLDrawZipPath());
	ldrawDirEdit->setText(ldPrefs->getLDrawDir());
}

void LDVPreferences::reflectGeometrySettings(void)
{
	setButtonState(seamWidthButton, ldPrefs->getUseSeams());
	setDoubleRangeValue(seamWidthDoubleSpin, ldPrefs->getSeamWidth() / 100.0f);
	partBoundingBoxOnlyBox->setChecked(ldPrefs->getBoundingBoxesOnly());
	reflectWireframeSettings();
	reflectBFCSettings();
//	wireframeThicknessSlider->setValue(wireframeThickness);
	edgeLinesButton->setChecked(ldPrefs->getShowHighlightLines());
	if (ldPrefs->getShowHighlightLines())
	{
		enableEdgeLines();
	}
	else
	{
		disableEdgeLines();
	}
	setRangeValue(edgeThicknessSlider, ldPrefs->getEdgeThickness());
}

void LDVPreferences::reflectWireframeSettings(void)
{
	wireframeButton->setChecked(ldPrefs->getDrawWireframe());
	setRangeValue(wireframeThicknessSlider,
		ldPrefs->getWireframeThickness());
	if (ldPrefs->getDrawWireframe())
	{
		enableWireframe();
	}
	else
	{
		disableWireframe();
	}
}

void LDVPreferences::reflectBFCSettings()
{
	enableBFCButton->setChecked(ldPrefs->getBfc());
	if (ldPrefs->getBfc())
	{
		enableBFC();
	}
	else
	{
		disableBFC();
	}
}

void LDVPreferences::reflectEffectsSettings(void)
{
	lightingButton->setChecked(ldPrefs->getUseLighting());
	if (ldPrefs->getUseLighting())
	{
		enableLighting();
	}
	else
	{
		disableLighting();
	}
	setButtonState(effectsHideLIGHTButton, ldPrefs->getNoLightGeom());
	if (ldPrefs->getStereoMode() != LDVStereoNone)
	{
		stereoButton->setChecked(true);
		enableStereo();
	}
	else
	{
		stereoButton->setChecked(false);
		disableStereo();
	}
	stereoAmountSlider->setValue(ldPrefs->getStereoEyeSpacing());
	if (ldPrefs->getCutawayMode() != LDVCutawayNormal)
	{
		wireframeCutawayButton->setChecked(true);
		enableWireframeCutaway();
	}
	else
	{
		wireframeCutawayButton->setChecked(false);
		disableWireframeCutaway();
	}
	cutawayOpacitySlider->setValue(ldPrefs->getCutawayAlpha());
	cutawayThicknessSlider->setValue(ldPrefs->getCutawayThickness());
	setButtonState(sortTransparencyButton,
		ldPrefs->getSortTransparent());
	setButtonState(stippleTransparencyButton, ldPrefs->getUseStipple());
	setButtonState(flatShadingButton, ldPrefs->getUseFlatShading());
	setButtonState(smoothCurvesButton, ldPrefs->getPerformSmoothing());
}

void LDVPreferences::reflectPrimitivesSettings(void)
{
	primitiveSubstitutionButton->setChecked(
		ldPrefs->getAllowPrimitiveSubstitution());
	if (ldPrefs->getAllowPrimitiveSubstitution())
	{
		enablePrimitiveSubstitution();
	}
	else
	{
		disablePrimitiveSubstitution();
	}
	curveQualitySlider->setValue(ldPrefs->getCurveQuality());
	setButtonState(lowQualityStudsButton, !ldPrefs->getQualityStuds());
	setButtonState(hiresPrimitivesButton, ldPrefs->getHiResPrimitives());
	useTextureMapsButton->setChecked(ldPrefs->getTexmaps());
	if (ldPrefs->getTexmaps())
	{
		transparentOffsetSlider->setValue(int(ldPrefs->getTextureOffsetFactor()*10));
	}
	automateEdgeColorBox->setChecked(ldPrefs->getAutomateEdgeColor());
	enableStudStyleCombo();
	enableAutomateEdgeColorButton();
	enableHighContrastButton(studStyleCombo->currentIndex());
}

void LDVPreferences::reflectUpdatesSettings(void)
{
	enum { PROXY_NONE, PROXY_WINDOWS, PROXY_MANUAL };
	int proxyType = ldPrefs->getProxyType();
	if (proxyType)
	{
		enableProxyServer();
		if (proxyType == PROXY_WINDOWS) {
			updatesWindowsproxyButton->setChecked(true);
		} else /*PROXY_MANUAL*/{
			updatesProxyButton->setChecked(true);
		}
	}
	else  /*PROXY_NONE*/
	{
		disableProxyServer();
		updatesNoproxyButton->setChecked(true);
	}
	setButtonState(updatesMissingpartsButton,
		ldPrefs->getCheckPartTracker());
	proxyEdit->setText(ldPrefs->getProxyServer());
	portEdit->setText(QString::number(ldPrefs->getProxyPort()));
	daymissingpartcheckText->setValue(ldPrefs->getMissingPartWait());
	dayupdatedpartcheckText->setValue(ldPrefs->getUpdatedPartWait());
	doUpdateMissingparts(ldPrefs->getCheckPartTracker());
}

void LDVPreferences::doResetGeneral(void)
{
	ldPrefs->loadDefaultGeneralSettings(false);
	reflectGeneralSettings();
}

void LDVPreferences::doResetLDraw(void)
{
	ldPrefs->loadDefaultLDrawSettings(false);
	reflectLDrawSettings();
}

void LDVPreferences::doResetGeometry(void)
{
	ldPrefs->loadDefaultGeometrySettings(false);
	reflectGeometrySettings();
}

void LDVPreferences::doResetEffects(void)
{
	ldPrefs->loadDefaultEffectsSettings(false);
	reflectEffectsSettings();
}

void LDVPreferences::doResetPrimitives(void)
{
	ldPrefs->loadDefaultPrimitivesSettings(false);
	reflectPrimitivesSettings();
}

void LDVPreferences::doResetUpdates(void)
{
	ldPrefs->loadDefaultUpdatesSettings(false);
	reflectUpdatesSettings();
}

void LDVPreferences::doResetTimesUpdates(void)
{
	LDrawModelViewer::resetUnofficialDownloadTimes();
}

void LDVPreferences::getRGBA(int color, int &r, int &g, int &b, int &a)
{
	r = color & 0xFF;
	g = (color >> 8) & 0xFF;
	b = (color >> 16) & 0xFF;
	a = (color >> 24) & 0xFF;
}

void LDVPreferences::getRGB(int color, int &r, int &g, int &b)
{
	int dummy = 255;
	getRGBA(color, r, g, b, dummy);
}

char *LDVPreferences::getLastOpenPath(char *pathKey)
{
	char *path;
	const char *constPathKey;

	if (pathKey)
	{
		constPathKey = pathKey;
	}
	else
	{
		constPathKey = LAST_OPEN_PATH_KEY;
	}
	path = TCUserDefaults::stringForKey(constPathKey, nullptr, false);
	if (!path)
	{
		path = copyString("/dos/c/ldraw");
	}
	return path;
}

void LDVPreferences::setLastOpenPath(const char *path, char *pathKey)
{
	const char *constPathKey;

	if (pathKey)
	{
		constPathKey = pathKey;
	}
	else
	{
		constPathKey = LAST_OPEN_PATH_KEY;
	}
	TCUserDefaults::setStringForKey(path, constPathKey, false);
}

void LDVPreferences::doLibraryCheckForUpdates()
{
	ldvModelWidget->checkForLibraryUpdates();
}

char *LDVPreferences::getLDrawDir(void)
{
	return TCUserDefaults::stringForKey(LDRAWDIR_KEY, nullptr, false);;
}

void LDVPreferences::setLDrawDir(const char *path)
{
	TCUserDefaults::setStringForKey(path, LDRAWDIR_KEY, false);
}

char *LDVPreferences::getLDrawZipPath(void)
{
	return TCUserDefaults::stringForKey(LDRAWZIP_KEY, nullptr, false);;
}

void LDVPreferences::setLDrawZipPath(const char *path)
{
	TCUserDefaults::setStringForKey(path, LDRAWZIP_KEY, false);
}

LDVPollMode LDVPreferences::getPollMode(void)
{
	return (LDVPollMode)TCUserDefaults::longForKey(POLL_KEY, 0, false);
}

void LDVPreferences::setPollMode(LDVPollMode value)
{
	TCUserDefaults::setLongForKey(value, POLL_KEY, false);
}

LDInputHandler::ViewMode LDVPreferences::getViewMode(void)
{
	return (LDInputHandler::ViewMode)TCUserDefaults::longForKey(VIEW_MODE_KEY, 0 , false);
}

void LDVPreferences::setViewMode(LDInputHandler::ViewMode value)
{
	TCUserDefaults::setLongForKey(value, VIEW_MODE_KEY, false);
}

void LDVPreferences::setCheckPartTracker(bool value)
{
	TCUserDefaults::setLongForKey(value, CHECK_PART_TRACKER_KEY, false);
}

bool LDVPreferences::getCheckPartTracker(void)
{
	return TCUserDefaults::longForKey(CHECK_PART_TRACKER_KEY, long(true), false);
}

bool LDVPreferences::getLatLongMode(void)
{
	return TCUserDefaults::longForKey(EXAMINE_MODE_KEY, LDrawModelViewer::EMFree ,false);
}

void LDVPreferences::setLatLongMode(bool value)
{
	TCUserDefaults::setLongForKey(value, EXAMINE_MODE_KEY, false);
}

bool LDVPreferences::getKeepRightSideUp(void)
{
	return TCUserDefaults::longForKey(KEEP_RIGHT_SIDE_UP_KEY, false, false);
}

void LDVPreferences::setKeepRightSideUp(bool value)
{
	TCUserDefaults::setLongForKey(value, KEEP_RIGHT_SIDE_UP_KEY, false);
}

bool LDVPreferences::getPovAspectRatio(void)
{
	return TCUserDefaults::boolForKey(POV_CAMERA_ASPECT_KEY, false, false);
}

void LDVPreferences::setWindowSize(int width, int height)
{
	if (width != windowWidth)
	{
		windowWidth = width;
		TCUserDefaults::setLongForKey(width, WINDOW_WIDTH_KEY, false);
	}
	if (height != windowHeight)
	{
		windowHeight = height;
		TCUserDefaults::setLongForKey(height, WINDOW_HEIGHT_KEY, false);
	}
}

int LDVPreferences::getWindowWidth(void)
{
	return windowWidth;
}

int LDVPreferences::getWindowHeight(void)
{
	return windowHeight;
}

void LDVPreferences::doWireframeCutaway(bool value)
{
	if (value)
	{
		enableWireframeCutaway();
	}
	else
	{
		disableWireframeCutaway();
	}
}

void LDVPreferences::doProxyServer(bool value)
{
	if (value)
		enableProxyServer();
	else
		disableProxyServer();
}

void LDVPreferences::doUpdateMissingparts(bool value)
{
	daymissingpartcheckText->setEnabled(value);
	daymissingpartcheckLabel->setEnabled(value);
	dayupdatedpartcheckLabel->setEnabled(value);
	dayupdatedpartcheckText->setEnabled(value);
}

void LDVPreferences::doLighting(bool value)
{
	if (value)
	{
		enableLighting();
	}
	else
	{
		disableLighting();
	}
}

void LDVPreferences::doDrawLightDats()
{
	bool checked = effectsUseLIGHTDATButton->isChecked();
	effectsReplaceStandarLightButton->setEnabled(checked);
	bool enabled = ldPrefs->getOptionalStandardLight() ? true : false;
	if (!checked) enabled = false;
	setButtonState(effectsReplaceStandarLightButton , enabled);
}

void LDVPreferences::doSaveDefaultViewAngle()
{
	ldPrefs->saveDefaultView();
}

void LDVPreferences::doStereo(bool value)
{
	if (value)
	{
		enableStereo();
	}
	else
	{
		disableStereo();
	}
}

void LDVPreferences::doSortTransparency(bool value)
{
	if(value)
	{
		setButtonState(stippleTransparencyButton,false);
	}
	applyButton->setEnabled(true);
}

void LDVPreferences::doStippleTransparency(bool value)
{
	if(value)
	{
		setButtonState(sortTransparencyButton,false);
	}
	applyButton->setEnabled(true);
}

void LDVPreferences::doWireframe(bool value)
{
	if (value)
	{
		enableWireframe();
	}
	else
	{
		disableWireframe();
	}
}

void LDVPreferences::doBFC(bool value)
{
	if (value)
	{
		enableBFC();
	}
	else
	{
		disableBFC();
	}
}

void LDVPreferences::doEdgeLines(bool value)
{
	if (value)
	{
		enableEdgeLines();
	}
	else
	{
		disableEdgeLines();
	}
}

void LDVPreferences::doConditionalShow(bool value)
{
	if (value)
	{
		enableConditionalShow();
	}
	else
	{
		disableConditionalShow();
	}
}

void LDVPreferences::doPrimitiveSubstitution(bool value)
{
	if (value)
	{
		enablePrimitiveSubstitution();
	}
	else
	{
		disablePrimitiveSubstitution();
	}
}

void LDVPreferences::doTextureStuds(bool value)
{
	if (value || useTextureMapsButton->isChecked())
	{
		enableTextureStuds();
	}
	else
	{
		disableTextureStuds();
	}
	updateTexmapsEnabled();
}

void LDVPreferences::doNewPreferenceSet()
{
	bool ok;
	QString name = QInputDialog::getText(this,QString("Native POV New Preference Set"),
				   QString("Enter name of the new PreferenceSet"), QLineEdit::Normal,QString(),
					&ok);
	if (ok && !name.isEmpty())
	{
		for(int i = 0; i < preferenceSetList->count(); i++)
		{
			if (getPrefSet(i) && strcmp(getPrefSet(i), name.toUtf8().constData())==0)
			{
				QMessageBox::warning(this,
					QString::fromWCharArray(TCLocalStrings::get(L"PrefSetAlreadyExists")),
					QString::fromWCharArray(TCLocalStrings::get(L"DuplicateName")),
					QMessageBox::Ok,0);
				return;
			}
		}
		if (name.indexOf('/')!=-1)
		{
			QMessageBox::warning(this,
				QString::fromWCharArray(TCLocalStrings::get(L"PrefSetNameBadChars")),
				QString::fromWCharArray(TCLocalStrings::get(L"InvalidName")),
				QMessageBox::Ok,0);
				return;
		}
		new QListWidgetItem(name,preferenceSetList);
		selectPrefSet(name.toUtf8().constData());
		return;
	}
	if (name.isEmpty() && ok)
	{
		QMessageBox::warning(this,
			QString::fromWCharArray(TCLocalStrings::get(L"PrefSetNameRequired")),
			QString::fromWCharArray(TCLocalStrings::get(L"EmptyName")),
			QMessageBox::Ok,0);
		return;
	}
}

void LDVPreferences::doDelPreferenceSet()
{
	const char *selectedPrefSet = getSelectedPrefSet();
	if (selectedPrefSet)
	{
		int selectedIndex = preferenceSetList->currentRow();
		if (checkAbandon && applyButton->isEnabled())
		{
			if(QMessageBox::warning(this,
				QString::fromWCharArray(TCLocalStrings::get(L"AbandonChanges")),
				QString::fromWCharArray(TCLocalStrings::get(L"PrefSetAbandonConfirm")),
				 QMessageBox::Yes, QMessageBox::No)== QMessageBox::Yes)
			{
				abandonChanges();
			}
			else
			{
				return;
			}
		}
		checkAbandon = false;
		delete preferenceSetList->currentItem();
		selectedIndex = preferenceSetList->currentRow();
		selectedPrefSet = getPrefSet(selectedIndex);
		selectPrefSet(selectedPrefSet, true);
	}
}

void LDVPreferences::doHotkeyPreferenceSet()
{
	hotKeyIndex = getCurrentHotKey();

	if (hotKeyIndex == -1)
	{
		hotKeyIndex++;
	}
	else if (hotKeyIndex == 0)
	{
		hotKeyIndex = 10;
	}
	QStringList lst;
	lst << TCLocalStrings::get("<None>") << "1" << "2" << "3" <<
	"4" << "5" << "6" << "7" << "8" << "9" << "0";
	bool ok;
	QString res = QInputDialog::getItem(this,getSelectedPrefSet(),
			"Select a hot key to automatically select this Preference Set:\nAlt + ",
			lst, hotKeyIndex, false, &ok);
	if (ok)
	{
		hotKeyIndex = lst.indexOf(res);
		if(hotKeyIndex != -1)
		{
			saveCurrentHotKey();
		}
	}

}

char *LDVPreferences::getHotKey(int index)
{
	char key[128];

	sprintf(key, "%s/Key%d", HOT_KEYS_KEY, index);
	return TCUserDefaults::stringForKey(key, nullptr, false);
}

int LDVPreferences::getHotKey(const char *currentPrefSetName)
{
	int i;
	int retValue = -1;

	for (i = 0; i < 10 && retValue == -1; i++)
	{
		char *prefSetName = getHotKey(i);

		if (prefSetName)
		{
			if (strcmp(prefSetName, currentPrefSetName) == 0)
			{
				retValue = i;
			}
			delete prefSetName;
		}
	}
	return retValue;
}

void LDVPreferences::performHotKey(int hotKeyIndex)
{
	char *hotKeyPrefSetName = getHotKey(hotKeyIndex);
	bool changed = false;
	if (hotKeyPrefSetName)
	{
		const char *currentSessionName = TCUserDefaults::getSessionName();
		bool hotKeyIsDefault = strcmp(hotKeyPrefSetName, DEFAULT_PREF_SET) == 0;

		if (currentSessionName)
		{
			if (strcmp(currentSessionName, hotKeyPrefSetName) == 0)
			{
				changed = true;
			}
		}
		else if (hotKeyIsDefault)
		{
			changed = true;
		}
		if (!changed)
		{
			if (hotKeyIsDefault)
			{
				TCUserDefaults::setSessionName(nullptr, PREFERENCE_SET_KEY);
				changed = true;
			}
			else
			{
				TCStringArray *sessionNames =
					TCUserDefaults::getAllSessionNames();

				if (sessionNames->indexOfString(hotKeyPrefSetName) != -1)
				{
					TCUserDefaults::setSessionName(hotKeyPrefSetName,
						PREFERENCE_SET_KEY);
					{
						changed = true;
					}
				}
				sessionNames->release();
			}
			if (changed)
			{
				loadSettings();
				reflectSettings();
				doApply();
			}
		}
	}
	delete hotKeyPrefSetName;
}


int LDVPreferences::getCurrentHotKey(void)
{
	int retValue = -1;

	if (getSelectedPrefSet())
	{
		return getHotKey(getSelectedPrefSet());
	}
	return retValue;
}

void LDVPreferences::saveCurrentHotKey(void)
{
	int currentHotKey = getCurrentHotKey();

	if (currentHotKey >= 0)
	{
		char key[128];

		sprintf(key, "%s/Key%d", HOT_KEYS_KEY, currentHotKey);
		TCUserDefaults::removeValue(key, false);
	}
	if (hotKeyIndex > 0)
	{
		char key[128];

		sprintf(key, "%s/Key%d", HOT_KEYS_KEY, hotKeyIndex % 10);
		TCUserDefaults::setStringForKey(getSelectedPrefSet(), key, false);
	}
}

void LDVPreferences::abandonChanges(void)
{
	applyButton->setEnabled(false);
	loadSettings();
	reflectSettings();
}

const char *LDVPreferences::getPrefSet(int index)
{
	return copyString(preferenceSetList->item(index)->text().toUtf8().constData());
}

const char *LDVPreferences::getSelectedPrefSet(void)
{
	int selectedIndex = preferenceSetList->currentRow();
	if (selectedIndex!=-1)
	{
		return copyString(preferenceSetList->currentItem()->text().toUtf8().constData());
	}
	return nullptr;
}
bool LDVPreferences::doPrefSetSelected(bool force)
{
	const char *selectedPrefSet = getSelectedPrefSet();
	bool needToReselect = false;

	if (checkAbandon && applyButton->isEnabled() && !force)
	{
		char *savedSession =
			TCUserDefaults::getSavedSessionNameFromKey(PREFERENCE_SET_KEY);

		if (!savedSession || !savedSession[0])
		{
			delete savedSession;
			savedSession = copyString(DEFAULT_PREF_SET);
		}
		if (selectedPrefSet && (strcmp(savedSession, selectedPrefSet) != 0))
		{
			needToReselect = true;
			selectPrefSet(nullptr, true);
			QMessageBox::warning(this,QString::fromWCharArray(TCLocalStrings::get(L"Error")),
				"You have made changes to the current preference set.  You must either apply those changes or abandon them before you can select a new preference set.");
		}
		delete savedSession;
	}
	if (selectedPrefSet)
	{
		bool enabled = true;

		if (needToReselect)
		{
			selectPrefSet(selectedPrefSet);
		}
		if (strcmp(selectedPrefSet, DEFAULT_PREF_SET) == 0)
		{
			enabled = false;
		}
		delPreferenceSetButton->setEnabled(enabled);
		delete selectedPrefSet;
	}
	applyButton->setEnabled(true);
	checkAbandon = false;
	return false;
}

void LDVPreferences::selectPrefSet(const char *prefSet, bool force)
{
	if (prefSet)
	{
		for (int i=0;i<preferenceSetList->count();i++)
		{
			if (strcmp(prefSet,preferenceSetList->item(i)->text().toUtf8().constData())==0)
			{
				preferenceSetList->setCurrentRow(i);
			}
		}
		doPrefSetSelected(force);
	}
	else
	{
		char *savedSession =
			TCUserDefaults::getSavedSessionNameFromKey(PREFERENCE_SET_KEY);

		if (savedSession && savedSession[0])
		{
			selectPrefSet(savedSession, force);
		}
		else
		{
			selectPrefSet(DEFAULT_PREF_SET, force);
		}
		delete savedSession;
	}
}

void LDVPreferences::setupPrefSetsList(void)
{
	TCStringArray *sessionNames = TCUserDefaults::getAllSessionNames();
	int i;
	int count = sessionNames->getCount();
	preferenceSetList->clear();
	new QListWidgetItem(QString(DEFAULT_PREF_SET),preferenceSetList);
	for (i = 0; i < count; i++)
	{
		new QListWidgetItem(sessionNames->stringAtIndex(i),preferenceSetList);
	}
	selectPrefSet();
	sessionNames->release();
}

void LDVPreferences::enableWireframeCutaway(void)
{
	colorCutawayButton->setEnabled(true);
	monochromeCutawayButton->setEnabled(true);
	cutawayOpacitySlider->setEnabled(true);
	cutawayThicknessSlider->setEnabled(true);
	cutawayOpacityLabel->setEnabled(true);
	cutawayThicknessLabel->setEnabled(true);
	setButtonState(colorCutawayButton, false);
	setButtonState(monochromeCutawayButton, false);
	switch (ldPrefs->getCutawayMode())
	{
	case LDVCutawayNormal:
	case LDVCutawayWireframe:
		colorCutawayButton->toggle();
		break;
	case LDVCutawayStencil:
		monochromeCutawayButton->toggle();
		break;
	}
}

void LDVPreferences::selectLightDirection(LDPreferences::LightDirection ld)
{
	switch (ld)
	{
	case LDPreferences::UpperLeft:
		lightingDir11->setChecked(true);
		break;
	case LDPreferences::UpperMiddle:
		lightingDir12->setChecked(true);
		break;
	case LDPreferences::UpperRight:
		lightingDir13->setChecked(true);
		break;
	case LDPreferences::MiddleLeft:
		lightingDir21->setChecked(true);
		break;
	case LDPreferences::MiddleMiddle:
		lightingDir22->setChecked(true);
		break;
	case LDPreferences::MiddleRight:
		lightingDir23->setChecked(true);
		break;
	case LDPreferences::LowerLeft:
		lightingDir31->setChecked(true);
		break;
	case LDPreferences::LowerMiddle:
		lightingDir32->setChecked(true);
		break;
	case LDPreferences::LowerRight:
		lightingDir33->setChecked(true);
		break;
	case LDPreferences::CustomDirection:
		break;
	}
}

void LDVPreferences::enableLighting(void)
{
	qualityLightingButton->setEnabled(true);
	subduedLightingButton->setEnabled(true);
	specularLightingButton->setEnabled(true);
	alternateLightingButton->setEnabled(true);
	effectsUseLIGHTDATButton->setEnabled(true);
	lightingDir11->setEnabled(true);
	lightingDir12->setEnabled(true);
	lightingDir13->setEnabled(true);
	lightingDir21->setEnabled(true);
	lightingDir22->setEnabled(true);
	lightingDir23->setEnabled(true);
	lightingDir31->setEnabled(true);
	lightingDir32->setEnabled(true);
	lightingDir33->setEnabled(true);
	setButtonState(qualityLightingButton, ldPrefs->getQualityLighting());
	setButtonState(subduedLightingButton, ldPrefs->getSubduedLighting());
	setButtonState(specularLightingButton, ldPrefs->getUseSpecular());
	setButtonState(alternateLightingButton, ldPrefs->getOneLight());
	setButtonState(effectsUseLIGHTDATButton, ldPrefs->getDrawLightDats());
	selectLightDirection(ldPrefs->getLightDirection());
	doDrawLightDats();
}

void LDVPreferences::enableStereo(void)
{
	crossEyedStereoButton->setEnabled(true);
	parallelStereoButton->setEnabled(true);
	stereoAmountSlider->setEnabled(true);
	stereoAmountLabel->setEnabled(true);
	setButtonState(crossEyedStereoButton, false);
	setButtonState(parallelStereoButton, false);
	switch (ldPrefs->getStereoMode())
	{
	case LDVStereoNone:
	case LDVStereoCrossEyed:
		crossEyedStereoButton->toggle();
		break;
	case LDVStereoParallel:
		parallelStereoButton->toggle();
		break;
	default:
		break;
	}
}

void LDVPreferences::enableWireframe(void)
{
	wireframeFogButton->setEnabled(true);
	wireframeRemoveHiddenLineButton->setEnabled(true);
	wireframeThicknessSlider->setEnabled(true);
	wireframeThicknessLabel->setEnabled(true);
	setButtonState(wireframeFogButton, ldPrefs->getUseWireframeFog());
	setButtonState(wireframeRemoveHiddenLineButton,
		ldPrefs->getRemoveHiddenLines());
}

void LDVPreferences::enableBFC(void)
{
	bfcRedBackFaceButton->setEnabled(true);
		bfcGreenFrontFaceButton->setEnabled(true);
		bfcBlueNeutralFaceButton->setEnabled(true);
	setButtonState(bfcRedBackFaceButton, ldPrefs->getRedBackFaces());
	setButtonState(bfcGreenFrontFaceButton,
		ldPrefs->getGreenFrontFaces());
	setButtonState(bfcBlueNeutralFaceButton,
		ldPrefs->getBlueNeutralFaces());
}

void LDVPreferences::enableEdgeLines(void)
{
	conditionalLinesButton->setEnabled(true);
	edgesOnlyButton->setEnabled(true);
	highQualityLinesButton->setEnabled(true);
	alwaysBlackLinesButton->setEnabled(true);
	edgeThicknessLabel->setEnabled(true);
	edgeThicknessSlider->setEnabled(true);
	setButtonState(conditionalLinesButton,
		ldPrefs->getDrawConditionalHighlights());
	setButtonState(edgesOnlyButton, ldPrefs->getEdgesOnly());
	setButtonState(highQualityLinesButton,
		ldPrefs->getUsePolygonOffset());
	setButtonState(alwaysBlackLinesButton,
		ldPrefs->getBlackHighlights());
	if (ldPrefs->getDrawConditionalHighlights())
	{
		enableConditionalShow();
	}
	else
	{
		disableConditionalShow();
	}
}

void LDVPreferences::enableConditionalShow(void)
{
	conditionalShowAllButton->setEnabled(true);
	conditionalShowControlPtsButton->setEnabled(true);
	setButtonState(conditionalShowAllButton,
		ldPrefs->getShowAllConditionalLines());
	setButtonState(conditionalShowControlPtsButton,
		ldPrefs->getShowConditionalControlPoints());
}


void LDVPreferences::enablePrimitiveSubstitution(void)
{
	textureStudsButton->setEnabled(true);
	curveQualityLabel->setEnabled(true);
	curveQualitySlider->setEnabled(true);
	setButtonState(textureStudsButton, ldPrefs->getTextureStuds());
	if (ldPrefs->getTextureStuds() || ldPrefs->getTexmaps())
	{
		enableTextureStuds();
	}
	else
	{
		disableTextureStuds();
	}
}

void LDVPreferences::enableTextureStuds(void)
{
	GLfloat maxAniso = TREGLExtensions::getMaxAnisoLevel();
	TCFloat32 anisoLevel = ldPrefs->getAnisoLevel();
	short numAnisoLevels = (short)(log(maxAniso)/log(2.0) + 0.5f);
	if (numAnisoLevels)
		anisotropicFilteringSlider->setMaximum(numAnisoLevels);
	nearestFilteringButton->setEnabled(true);
	bilinearFilteringButton->setEnabled(true);
	trilinearFilteringButton->setEnabled(true);
	anisotropicFilteringButton->setEnabled(
		TREGLExtensions::haveAnisoExtension());
	setButtonState(nearestFilteringButton, false);
	setButtonState(bilinearFilteringButton, false);
	setButtonState(trilinearFilteringButton, false);
	setButtonState(anisotropicFilteringButton, false);
	switch (ldPrefs->getTextureFilterType())
	{
	case GL_NEAREST_MIPMAP_NEAREST:
		nearestFilteringButton->toggle();
		break;
	case GL_LINEAR_MIPMAP_NEAREST:
		bilinearFilteringButton->toggle();
		break;
	case GL_LINEAR_MIPMAP_LINEAR:
		trilinearFilteringButton->toggle();
		break;
	}
	if (anisoLevel > 1.0)
	{
		anisotropicFilteringButton->toggle();
		setAniso((int)(log(anisoLevel)/log(2.0)+0.5f));
	}
	if (anisoLevel > maxAniso)
	{
		anisoLevel = (TCFloat32)maxAniso;
	}
	if (anisoLevel < 2.0f)
	{
		anisoLevel = 2.0f;
	}
	doAnisotropic();
}

void LDVPreferences::enableTexmaps(void)
{
	transparentTextureOffsetLabel->setEnabled(true);
	transparentOffsetSlider->setEnabled(true);
	transparentOffsetSlider->setValue(ldPrefs->getTexturesAfterTransparent()*10.0);

}


void LDVPreferences::enableProxyServer(void)
{
	proxyLabel->setEnabled(true);
	proxyEdit->setEnabled(true);
	portLabel->setEnabled(true);
	portEdit->setEnabled(true);
	proxyEdit->setText(ldPrefs->getProxyServer());
}

void LDVPreferences::disableWireframeCutaway(void)
{
	colorCutawayButton->setEnabled(false);
	monochromeCutawayButton->setEnabled(false);
	cutawayOpacitySlider->setEnabled(false);
	cutawayThicknessSlider->setEnabled(false);
	cutawayOpacityLabel->setEnabled(false);
	cutawayThicknessLabel->setEnabled(false);
	setButtonState(colorCutawayButton, false);
	setButtonState(monochromeCutawayButton, false);
}

void LDVPreferences::uncheckLightDirections(void)
{
	lightingDir11->setChecked(false);
	lightingDir12->setChecked(false);
	lightingDir13->setChecked(false);
	lightingDir21->setChecked(false);
	lightingDir22->setChecked(false);
	lightingDir23->setChecked(false);
	lightingDir31->setChecked(false);
	lightingDir32->setChecked(false);
	lightingDir33->setChecked(false);
}

void LDVPreferences::disableLighting(void)
{
	qualityLightingButton->setEnabled(false);
	subduedLightingButton->setEnabled(false);
	specularLightingButton->setEnabled(false);
	alternateLightingButton->setEnabled(false);
	effectsUseLIGHTDATButton->setEnabled(false);
	effectsHideLIGHTButton->setEnabled(true);
	lightingDir11->setEnabled(false);
	lightingDir12->setEnabled(false);
	lightingDir13->setEnabled(false);
	lightingDir21->setEnabled(false);
	lightingDir22->setEnabled(false);
	lightingDir23->setEnabled(false);
	lightingDir31->setEnabled(false);
	lightingDir32->setEnabled(false);
	lightingDir33->setEnabled(false);
	setButtonState(qualityLightingButton, false);
	setButtonState(subduedLightingButton, false);
	setButtonState(specularLightingButton, false);
	setButtonState(alternateLightingButton, false);
	setButtonState(effectsUseLIGHTDATButton, false);

	uncheckLightDirections();
}

void LDVPreferences::disableStereo(void)
{
	crossEyedStereoButton->setEnabled(false);
	parallelStereoButton->setEnabled(false);
	stereoAmountSlider->setEnabled(false);
	stereoAmountLabel->setEnabled(false);
	setButtonState(crossEyedStereoButton, false);
	setButtonState(parallelStereoButton, false);
}

void LDVPreferences::disableWireframe(void)
{
	wireframeFogButton->setEnabled(false);
	wireframeRemoveHiddenLineButton->setEnabled(false);
	wireframeThicknessSlider->setEnabled(false);
	wireframeThicknessLabel->setEnabled(false);
	setButtonState(wireframeFogButton, false);
	setButtonState(wireframeRemoveHiddenLineButton, false);
}

void LDVPreferences::disableBFC(void)
{
	bfcRedBackFaceButton->setEnabled(false);
		bfcGreenFrontFaceButton->setEnabled(false);
	bfcBlueNeutralFaceButton->setEnabled(false);
	setButtonState(bfcRedBackFaceButton, false);
		setButtonState(bfcGreenFrontFaceButton, false);
		setButtonState(bfcBlueNeutralFaceButton, false);
}

void LDVPreferences::disableEdgeLines(void)
{
	conditionalLinesButton->setEnabled(false);
	conditionalShowAllButton->setEnabled(false);
	conditionalShowControlPtsButton->setEnabled(false);
	edgesOnlyButton->setEnabled(false);
	highQualityLinesButton->setEnabled(false);
	alwaysBlackLinesButton->setEnabled(false);
	edgeThicknessLabel->setEnabled(false);
	edgeThicknessSlider->setEnabled(false);
	setButtonState(conditionalLinesButton, false);
	setButtonState(conditionalShowAllButton, false);
	setButtonState(conditionalShowControlPtsButton, false);
	setButtonState(edgesOnlyButton, false);
	setButtonState(highQualityLinesButton, false);
	setButtonState(alwaysBlackLinesButton, false);
}

void LDVPreferences::disableConditionalShow(void)
{
	conditionalShowAllButton->setEnabled(false);
	conditionalShowControlPtsButton->setEnabled(false);
	setButtonState(conditionalShowAllButton, false);
	setButtonState(conditionalShowControlPtsButton, false);
}

void LDVPreferences::disablePrimitiveSubstitution(void)
{
	textureStudsButton->setEnabled(false);
	curveQualityLabel->setEnabled(false);
	curveQualitySlider->setEnabled(false);
	setButtonState(textureStudsButton, false);
	if (!useTextureMapsButton->isChecked()) disableTextureStuds();
}

void LDVPreferences::disableTextureStuds(void)
{
	nearestFilteringButton->setEnabled(false);
	bilinearFilteringButton->setEnabled(false);
	trilinearFilteringButton->setEnabled(false);
	anisotropicFilteringButton->setEnabled(false);
	anisotropicFilteringSlider->setEnabled(false);
	anisotropicLabel->setEnabled(false);
	setButtonState(nearestFilteringButton, false);
	setButtonState(bilinearFilteringButton, false);
	setButtonState(trilinearFilteringButton, false);
	setButtonState(anisotropicFilteringButton, false);
}

void LDVPreferences::disableTexmaps(void)
{
	transparentTextureOffsetLabel->setEnabled(false);
	transparentOffsetSlider->setEnabled(false);
}

void LDVPreferences::updateTexmapsEnabled(void)
{
	if (useTextureMapsButton->isChecked())
	{
		enableTexmaps();
	} else
	{
		disableTexmaps();
	}
}

void LDVPreferences::disableProxyServer(void)
{
	proxyLabel->setEnabled(false);
	proxyEdit->setEnabled(false);
	portLabel->setEnabled(false);
	portEdit->setEnabled(false);
}

#ifdef WIN32

int LDVPreferences::getFSAAFactor(void)
{
	int fsaaMode = ldPrefs->getFsaaMode();

	if (fsaaMode && LDVExtensionsSetup::haveMultisampleExtension())
	{
		if (fsaaMode <= 5)
		{
			return fsaaMode & 0x6; // Mask off bottom bit
		}
		else
		{
			return fsaaMode >> 3;
		}
	}
	else
	{
		return 0;
	}
}

bool LDVPreferences::getUseNvMultisampleFilter(void)
{
	int fsaaMode = ldPrefs->getFsaaMode();

	if ((fsaaMode & 0x1) &&
		TREGLExtensions::haveNvMultisampleFilterHintExtension())
	{
		return true;
	}
	else
	{
		return false;
	}
}

void LDVPreferences::setupAntialiasing(void)
{
	HWND hWnd = (HWND)this->winId();
	HINSTANCE hInstance = (HINSTANCE)this->winId();
	LDVExtensionsSetup::setup(hWnd, hInstance);
	TCIntArray *fsaaModes = LDVExtensionsSetup::getFSAAModes();

	UCCHAR modeString[1024];

	// Remove all items from FSAA combo box list.
	fsaaModeBox->clear();
	// Add "None" to FSAA combo box list as only item.
	fsaaModeBox->addItem(QString::fromWCharArray(TCObject::ls(_UC("FsaaNone"))));
	// Select "None", just in case something else doesn't get selected later.
	fsaaModeBox->setCurrentIndex(0);
	// The following array should always exist, even if it is empty, but check
	// just to be sure.
	if (fsaaModes)
	{
		int i;
		int count = fsaaModes->getCount();

		// Note that fsaaModes contains a sorted array of unique FSAA factors.
		for (i = 0; i < count; i++)
		{
			int value = (*fsaaModes)[i];

			sucprintf(modeString, COUNT_OF(modeString),
					  TCObject::ls(_UC("FsaaNx")), value);
			fsaaModeBox->addItem(QString::fromWCharArray(modeString));
			// nVidia hardware supports Quincunx and 9-box pattern, so add an
			// "Enhanced" item to the list if the extension is supported and
			// the current factor is 2 or 4.
			if ((value == 2 || value == 4) &&
					TREGLExtensions::haveNvMultisampleFilterHintExtension())
			{
				sucprintf(modeString, COUNT_OF(modeString),
						  TCObject::ls(_UC("FsaaNx")), value);
				ucstrcat(modeString, _UC(" "));
				ucstrcat(modeString, TCObject::ls(_UC("FsaaEnhanced")));
				fsaaModeBox->addItem(QString::fromWCharArray(modeString));
			}
		}
	}
	if (ldPrefs->getFsaaMode())
	{
		sucprintf(modeString, COUNT_OF(modeString),
				  TCObject::ls(_UC("FsaaNx")), getFSAAFactor());
		if (getUseNvMultisampleFilter())
		{
			ucstrcat(modeString, _UC(" "));
			ucstrcat(modeString, TCObject::ls(_UC("FsaaEnhanced")));
		}

		int vv;
		if ((vv = fsaaModeBox->findText(QString::fromWCharArray(modeString))) == -1) {
			vv = 0;
			ldPrefs->setFsaaMode(vv);
		}
		fsaaModeBox->setCurrentIndex(vv);
	}
}

void LDVPreferences::fsaaModeBoxChanged(const QString &controlString)
{
	ucstring selectedString = controlString.toStdWString().c_str();
	int fsaaMode;

	if (selectedString == TCObject::ls(_UC("FsaaNone")))
	{
		fsaaMode = 0;
	}
	else
	{
		sucscanf(selectedString.c_str(), _UC("%d"), &fsaaMode);
		if (fsaaMode > 4)
		{
			fsaaMode = fsaaMode << 3;
		}
		else if (selectedString.find(TCObject::ls(_UC("FsaaEnhanced"))) <
			selectedString.size())
		{
			fsaaMode |= 1;
		}
	}
	ldPrefs->setFsaaMode(fsaaMode);
	enableApply();
}

#endif // WIN32

void LDVPreferences::setupDefaultRotationMatrix(void)
{
	char *value = TCUserDefaults::stringForKey(DEFAULT_LAT_LONG_KEY);

	if (value)
	{
		TCFloat latitude;
		TCFloat longitude;

		if (sscanf(value, "%f,%f", &latitude, &longitude) == 2)
		{
			TCFloat leftMatrix[16];
			TCFloat rightMatrix[16];
			TCFloat resultMatrix[16];
			TCFloat cosTheta;
			TCFloat sinTheta;

			TCVector::initIdentityMatrix(leftMatrix);
			TCVector::initIdentityMatrix(rightMatrix);
			latitude = (TCFloat)deg2rad(latitude);
			longitude = (TCFloat)deg2rad(longitude);

			// First, apply latitude by rotating around X.
			cosTheta = (TCFloat)cos(latitude);
			sinTheta = (TCFloat)sin(latitude);
			rightMatrix[5] = cosTheta;
			rightMatrix[6] = sinTheta;
			rightMatrix[9] = -sinTheta;
			rightMatrix[10] = cosTheta;
			TCVector::multMatrix(leftMatrix, rightMatrix, resultMatrix);

			memcpy(leftMatrix, resultMatrix, sizeof(leftMatrix));
			TCVector::initIdentityMatrix(rightMatrix);

			// Next, apply longitude by rotating around Y.
			cosTheta = (TCFloat)cos(longitude);
			sinTheta = (TCFloat)sin(longitude);
			rightMatrix[0] = cosTheta;
			rightMatrix[2] = -sinTheta;
			rightMatrix[8] = sinTheta;
			rightMatrix[10] = cosTheta;
			TCVector::multMatrix(leftMatrix, rightMatrix, resultMatrix);

			modelViewer->setDefaultRotationMatrix(resultMatrix);
		}
		delete value;
	}
	else
	{
		value = TCUserDefaults::stringForKey(DEFAULT_MATRIX_KEY);
		if (value)
		{
			TCFloat matrix[16];

			memset(matrix, 0, sizeof(matrix));
			matrix[15] = 1.0f;
			if (sscanf(value, "%f,%f,%f,%f,%f,%f,%f,%f,%f",
				&matrix[0], &matrix[4], &matrix[8],
				&matrix[1], &matrix[5], &matrix[9],
				&matrix[2], &matrix[6], &matrix[10]) == 9)
			{
				modelViewer->setDefaultRotationMatrix(matrix);
			}
			delete value;
		}
	}
}

void LDVPreferences::getBackgroundColor(int &r, int &g, int &b)
{
	ldPrefs->getBackgroundColor(r, g, b);
}

bool LDVPreferences::getDrawWireframe(void)
{
	return ldPrefs->getDrawWireframe();
}

bool LDVPreferences::getUseWireframeFog(void)
{
	return ldPrefs->getUseWireframeFog();
}

bool LDVPreferences::getRemoveHiddenLines(void)
{
	return ldPrefs->getRemoveHiddenLines();
}

bool LDVPreferences::getTextureStud(void)
{
	return ldPrefs->getTextureStuds();
}

bool LDVPreferences::getShowsHighlightLines(void)
{
	return ldPrefs->getShowHighlightLines();
}

bool LDVPreferences::getUseLighting(void)
{
	return ldPrefs->getUseLighting();
}

bool LDVPreferences::getUseBFC(void)
{
	return ldPrefs->getBfc();
}

bool LDVPreferences::getUseSeams(void)
{
	return ldPrefs->getUseSeams();
}

void LDVPreferences::userDefaultChangedAlertCallback(TCAlert *alert)  // Not Used
{
	const char *key = alert->getMessage();

	if (key)
	{
		if (strcmp(key, CHECK_PART_TRACKER_KEY) == 0)
		{
			if (ldPrefs)
			{
				reflectUpdatesSettings();
			}
		}
	}
}

void LDVPreferences::checkLightVector(void)
{
	LDPreferences::LightDirection selectedDirection =
		getSelectedLightDirection();
	LDPreferences::LightDirection lightDirection =
		ldPrefs->getLightDirection();
	if (selectedDirection != lightDirection)
	{
		uncheckLightDirections();
		selectLightDirection(lightDirection);
	}
}

void LDVPreferences::updateSaveDir(QLineEdit *textField, QPushButton *button,
								LDPreferences::DefaultDirMode dirMode,
								QString &filename)
{
	bool enable = false;

	if (dirMode == LDPreferences::DDMSpecificDir)
	{
		textField->setText(filename);
		enable = true;
	}
	else
	{
		textField->setText("");
	}
	button->setEnabled(enable);
	textField->setEnabled(enable);
}

void LDVPreferences::setupSaveDir(QComboBox *comboBox, QLineEdit *textField,
							   QPushButton *button,
							   LDPreferences::DefaultDirMode dirMode,
							   QString &filename)
{
	comboBox->setCurrentIndex(dirMode);
	updateSaveDir(textField, button, dirMode, filename);
}

void LDVPreferences::setupSaveDirs()
{
	snapshotDir = ldPrefs->getSaveDir(LDPreferences::SOSnapshot).c_str();
	setupSaveDir(snapshotSaveDirBox, snapshotSaveDirEdit,
				 snapshotSaveDirButton,
				 ldPrefs->getSaveDirMode(LDPreferences::SOSnapshot),
				 snapshotDir);
	partsListDir =ldPrefs->getSaveDir(LDPreferences::SOPartsList).c_str();
	setupSaveDir(partsListsSaveDirBox, partsListsSaveDirEdit,
				 partsListsSaveDirButton,
				 ldPrefs->getSaveDirMode(LDPreferences::SOPartsList),
				 partsListDir);
	exportDir = ldPrefs->getSaveDir(LDPreferences::SOExport).c_str();
	setupSaveDir(exportsListsSaveDirBox, exportsSaveDirEdit,
				 exportsSaveDirButton,
				 ldPrefs->getSaveDirMode(LDPreferences::SOExport),
				 exportDir);
}

void LDVPreferences::snapshotSaveDirBoxChanged()
{
	applyButton->setEnabled(true);
	updateSaveDir(snapshotSaveDirEdit,snapshotSaveDirButton,
		(LDPreferences::DefaultDirMode)snapshotSaveDirBox->currentIndex(),
		snapshotDir);
}

void LDVPreferences::partsListsSaveDirBoxChanged()
{
	updateSaveDir(partsListsSaveDirEdit,partsListsSaveDirButton,
		(LDPreferences::DefaultDirMode)partsListsSaveDirBox->currentIndex(),
		partsListDir);
	applyButton->setEnabled(true);
}

void LDVPreferences::exportsListsSaveDirBoxChanged()
{
	updateSaveDir(exportsSaveDirEdit,exportsSaveDirButton,
			(LDPreferences::DefaultDirMode)exportsListsSaveDirBox->currentIndex(),
			exportDir);
	applyButton->setEnabled(true);
}

void LDVPreferences::snapshotSaveDirBrowse()
{
	browseForDir(QString::fromWCharArray(TCLocalStrings::get(L"BrowseForSnapshotDir")),
		snapshotSaveDirEdit, snapshotDir);
}

void LDVPreferences::partsListsSaveDirBrowse()
{
	browseForDir(QString::fromWCharArray(TCLocalStrings::get(L"BrowseForPartsListDir")),
		partsListsSaveDirEdit, partsListDir);
}

void LDVPreferences::exportsSaveDirBrowse()
{
	browseForDir(QString::fromWCharArray(TCLocalStrings::get(L"BrowseForExportListDir")),
		exportsSaveDirEdit, exportDir);
}

void LDVPreferences::customConfigBrowse()
{
	QString selectedfile=QFileDialog::getOpenFileName(this,"Browse for LDraw file","","LDraw file (*.mpd *.ldr *.dat)");
	if (!selectedfile.isEmpty())
	{
		customConfigEdit->setText(selectedfile);
		applyButton->setEnabled(true);
	}
}

void LDVPreferences::browseForDir(QString prompt, QLineEdit *textField, QString &dir)
{
	QString selectedfile=QFileDialog::getExistingDirectory(this,prompt,dir);
	if (!selectedfile.isEmpty())
	{
		textField->setText(dir = selectedfile);
		applyButton->setEnabled(true);
	}
}

void LDVPreferences::enableStudStyleCombo()
{
	studStyleCombo->addItem(QString::fromStdWString(TCObject::ls(_UC("Plain"))));
	studStyleCombo->addItem(QString::fromStdWString(TCObject::ls(_UC("ThinLineLogo"))));
	studStyleCombo->addItem(QString::fromStdWString(TCObject::ls(_UC("OutlineLogo"))));
	studStyleCombo->addItem(QString::fromStdWString(TCObject::ls(_UC("SharpTopLogo"))));
	studStyleCombo->addItem(QString::fromStdWString(TCObject::ls(_UC("RoundedTopLogo"))));
	studStyleCombo->addItem(QString::fromStdWString(TCObject::ls(_UC("FlattenedLogo"))));
	studStyleCombo->addItem(QString::fromStdWString(TCObject::ls(_UC("HighContrast"))));
	studStyleCombo->addItem(QString::fromStdWString(TCObject::ls(_UC("HighContrastWithLogo"))));
	studStyleCombo->setCurrentIndex(ldPrefs->getStudStyle());
}

void LDVPreferences::enableAutomateEdgeColorButton()
{
	automateEdgeColorButton->setEnabled(automateEdgeColorBox->isChecked());
}

void LDVPreferences::enableHighContrastButton(int index)
{
	highContrastButton->setEnabled(index > 5);
}

void LDVPreferences::automateEdgeColor()
{
	lcAutomateEdgeColorDialog Dialog(this, sender() == highContrastButton);
	if (Dialog.exec() == QDialog::Accepted)
	{
		int r, g, b, a;
		getRGBA(Dialog.mStudCylinderColor, r, g, b, a);
		ldPrefs->setStudCylinderColor(r, g, b, a);
		getRGBA(Dialog.mPartEdgeColor, r, g, b, a);
		ldPrefs->setPartEdgeColor(r, g, b, a);
		getRGBA(Dialog.mBlackEdgeColor, r, g, b, a);
		ldPrefs->setBlackEdgeColor(r, g, b, a);
		getRGBA(Dialog.mDarkEdgeColor, r, g, b, a);
		ldPrefs->setDarkEdgeColor(r, g, b, a);

		ldPrefs->setPartEdgeContrast(Dialog.mPartEdgeContrast);
		ldPrefs->setPartColorValueLDIndex(Dialog.mPartColorValueLDIndex);
	}
}

void LDVPreferences::enableApply(void)
{
	applyButton->setEnabled(true);
}

void LDVPreferences::enableProxy(void)
{
	doProxyServer(true);
	enableApply();
}

void LDVPreferences::disableProxy(void)
{
	doProxyServer(false);
	enableApply();
}


void LDVPreferences::doAddExtraDir(void)
{
	int count=ExtraDirListView->count();
	if (count>=MAX_EXTRA_DIR) { return;}
	QString selectedfile = QFileDialog::getExistingDirectory(this,"Choose a Directory",".");
	if (!selectedfile.isEmpty())
	{
		new QListWidgetItem(selectedfile,ExtraDirListView);
		extraSearchDirs->addString(selectedfile.toUtf8().constData());
		delExtraDirButton->setEnabled(true);
		if (count==MAX_EXTRA_DIR-1)
		{
			addExtraDirButton->setEnabled(false);
		}
	}
	doExtraDirSelected();
}

void LDVPreferences::doDelExtraDir(void)
{
	int index=ExtraDirListView->currentRow(),
		count=ExtraDirListView->count();
	if (index!=-1)
	{
		extraSearchDirs->removeStringAtIndex(index);
		delete ExtraDirListView->currentItem();
		if (count==1)
		{
			delExtraDirButton->setEnabled(false);
		}
		if (count==MAX_EXTRA_DIR)
		{
			addExtraDirButton->setEnabled(true);
		}
	}
}
void LDVPreferences::doExtraDirSelected(void)
{
	int index=ExtraDirListView->currentRow(),
		count=ExtraDirListView->count();
	upExtraDirButton->setEnabled(index>0 ? true : false);
	downExtraDirButton->setEnabled(((index == count-1) && (count > 0)) ? false : true);
}

void LDVPreferences::doUpExtraDir(void)
{
	int index=ExtraDirListView->currentRow(),
		count=ExtraDirListView->count();
	char *extraDir;

	if (index>0 && count >1)
	{
		QString tmp=ExtraDirListView->currentItem()->text();
		delete ExtraDirListView->currentItem();
		QListWidgetItem *newitem = new QListWidgetItem(tmp);
		ExtraDirListView->insertItem(index-1,newitem);
		ExtraDirListView->setCurrentItem(ExtraDirListView->item(index-1));
		extraDir=copyString(extraSearchDirs->stringAtIndex(index));
		extraSearchDirs->removeStringAtIndex(index);
		extraSearchDirs->insertString(extraDir,index-1);
		delete extraDir;
	}
}

void LDVPreferences::doDownExtraDir(void)
{
	int index=ExtraDirListView->currentRow(),
		count=ExtraDirListView->count();
	char *extraDir;
	if (index<count-1 && count>0 && index!=-1)
	{
		QString tmp=ExtraDirListView->currentItem()->text();
		delete ExtraDirListView->currentItem();
		ExtraDirListView->insertItem(index+1,new QListWidgetItem(tmp));
		ExtraDirListView->setCurrentItem(ExtraDirListView->item(index+1));
		extraDir=copyString(extraSearchDirs->stringAtIndex(index));
		extraSearchDirs->removeStringAtIndex(index);
		extraSearchDirs->insertString(extraDir,index+1);
		delete extraDir;
	}
}

void LDVPreferences::populateExtraDirsListBox(void)
{
	int i;
	int count=ExtraDirListView->count();
	char *dir;
	for (i=0;i<count;i++) { delete ExtraDirListView->item(0); }
	count = extraSearchDirs->getCount();
	for (i=0;i<count;i++)
	{
		dir=extraSearchDirs->stringAtIndex(i);
		if (dir && dir[0])
		{
			new QListWidgetItem(extraSearchDirs->stringAtIndex(i),ExtraDirListView);
		}
	}
	if (count==MAX_EXTRA_DIR)
	{
		addExtraDirButton->setEnabled(false);
	}
	delExtraDirButton->setEnabled(count>0 ? true : false);
}

void LDVPreferences::recordExtraSearchDirs(void)
{
	int i;
	int count = extraSearchDirs->getCount();

	for (i = 0; i <= count; i++)
	{
		char key[128];
		char *extraDir;

		snprintf(key, sizeof(key), "%s/Dir%03d", EXTRA_SEARCH_DIRS_KEY, i + 1);
		extraDir = extraSearchDirs->stringAtIndex(i);
		if (extraDir)
		{
			TCUserDefaults::setStringForKey(extraDir, key, false);
		}
		else
		{
			TCUserDefaults::removeValue(key, false);
		}
	}
	for (i=count; i<MAX_EXTRA_DIR;i++)
	{
		char key[128];
		char extraDir[]="";


		snprintf(key, sizeof(key), "%s/Dir%03d", EXTRA_SEARCH_DIRS_KEY, i + 1);
		TCUserDefaults::setStringForKey(extraDir, key, false);
	}
	if (modelViewer)
	{
		modelViewer->setExtraSearchDirs(extraSearchDirs);
	}
}

void LDVPreferences::populateExtraSearchDirs(void)
{
	int i;

	extraSearchDirs->removeAll();
	for (i = 1; true; i++)
	{
		char key[128];
		char *extraSearchDir;

		snprintf(key, sizeof(key), "%s/Dir%03d", EXTRA_SEARCH_DIRS_KEY, i);
		extraSearchDir = TCUserDefaults::stringForKey(key, NULL, false);
		if (extraSearchDir && extraSearchDir[0])
		{
			extraSearchDirs->addString(extraSearchDir);
			delete extraSearchDir;
		}
		else
		{
			break;
		}
	}
}

void LDVPreferences::doLDrawDir(void)
{
	QString dir = QFileDialog::getExistingDirectory(this,"Please select the directory in which you installed LDraw",ldrawDirEdit->text());
	if (dir.isEmpty()) {return;}
	ldrawDirEdit->setText(dir);
	applyButton->setEnabled(true);
}

void LDVPreferences::doLDrawZip(void)
{
	QString file = QFileDialog::getOpenFileName(this,"Select the LDraw Part Library Zip","","ZIP file (*.zip)");
	if (file.isEmpty()) {return;}
	if (LDLModel::checkLDrawZipPath(file.toUtf8().constData()))
	{
		ldrawZipEdit->setText(file);
	}
	else
	{
		QMessageBox::warning(this,
			"Error",
			QString::fromWCharArray(TCLocalStrings::get(L"InvalidZip")),
			QMessageBox::Ok);

	}
}
