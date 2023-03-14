/****************************************************************************
**
** Copyright (C) 2022 - 2023 Trevor SANDY. All rights reserved.
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

#include <QMessageBox>

#include "lpub_object.h"

#include "application.h"
#include "lpub_preferences.h"
#include "step.h"
#include "lpub.h"
#include "messageboxresizable.h"
#include "qsimpleupdater.h"
#include "texteditdialog.h"
#include "preferencesdialog.h"
#include "commonmenus.h"

#include "lc_qpreferencesdialog.h"
#include "lc_application.h"
#include "lc_mainwindow.h"
#include "lc_library.h"
#include "lc_profile.h"
#include "lc_context.h"
#include "lc_view.h"
#include "lc_http.h"
#include "project.h"
#include "camera.h"

#include <jsonfile.h>
#include <commands/snippets/jsonsnippettranslatorfactory.h>
#include <commands/snippets/snippetcollection.h>
#include <commands/jsoncommandtranslatorfactory.h>
#include <commands/commandcollection.h>
#include <commands/commandstextedit.h>
#include <commands/commandsdialog.h>

LPub *lpub;

QString    LPub::commandlineFile;
QString    LPub::viewerStepKey;
QString    LPub::m_versionInfo;
QString    LPub::m_releaseNotesContent;
QString    LPub::DEFS_URL = VER_UPDATE_CHECK_JSON_URL;
bool       LPub::m_updaterCancelled;
bool       LPub::m_setReleaseNotesAsText;
int        LPub::ViewpointsComboSaveIndex = -1;

const QString shortcutParentNames[] =
{
  "Root",                // 0 - NO_ACTION,
  "MainWindow",          // 1 - MAIN_WINDOW_ACTION,
  "CommandEditor",       // 2 - COMMAND_EDITOR_ACTION,
  "TextEditor",          // 3 - TEXT_EDITOR_ACTION,
  "VisualEditor",        // 4 - VISUAL_EDITOR_ACTION,
  "ParameterEditor",     // 5 - PARAMS_EDITOR_ACTION,
  "CommandsDialog",      // 6 - COMMANDS_DIALOG_ACTION,
  "CommandsTextEditor",  // 7 - COMMANDS_TEXT_EDIT_ACTION,
  "Other"                // 8 - OTHER_ACTION
};

LPub::LPub()
{
  lpub = this;
}

LPub::~LPub()
{
  lpub = nullptr;
}

void LPub::loadDialogs()
{
  textEdit        = new TextEditDialog(gui);
  commandsDialog  = new CommandsDialog(nullptr);
  commandTextEdit = new CommandsTextEdit(nullptr);
  snippetTextEdit = new CommandsTextEdit(nullptr);
}

void LPub::loadPreferencesDialog()
{
  emit messageSig(LOG_INFO_STATUS,tr("Loading preferences dialogs..."));
  preferencesDialog = new PreferencesDialog(gui);
  visualEditorPreferencesDialog = new lcQPreferencesDialog(nullptr);
}

/*********************************************
 *
 * Command line support calls
 *
 ********************************************/

int LPub::Process3DViewerCommandLine()
{
    return gApplication->Process3DViewerCommandLine();
}

/********************************************************************
 *
 * Manage visual editor stud style
 *
 * *****************************************************************/

int LPub::GetStudStyle()
{
    if (gApplication)
        return static_cast<int>(lcGetPiecesLibrary()->GetStudStyle());
    else
        return lcGetProfileInt(LC_PROFILE_STUD_STYLE);
}

bool LPub::GetAutomateEdgeColor()
{
    if (gApplication)
        return lcGetPreferences().mAutomateEdgeColor;
    else
        return lcGetProfileInt(LC_PROFILE_AUTOMATE_EDGE_COLOR);
}

quint32 LPub::GetStudCylinderColor()
{
    if (gApplication)
        return lcGetPreferences().mStudCylinderColor;
    else
        return lcGetProfileInt(LC_PROFILE_STUD_CYLINDER_COLOR);
}

quint32 LPub::GetPartEdgeColor()
{
    if (gApplication)
        return lcGetPreferences().mPartEdgeColor;
    else
        return lcGetProfileInt(LC_PROFILE_PART_EDGE_COLOR);
}

quint32 LPub::GetBlackEdgeColor()
{
    if (gApplication)
        return lcGetPreferences().mBlackEdgeColor;
    else
        return lcGetProfileInt(LC_PROFILE_BLACK_EDGE_COLOR);
}

quint32 LPub::GetDarkEdgeColor()
{
    if (gApplication)
        return lcGetPreferences().mDarkEdgeColor;
    else
        return lcGetProfileInt(LC_PROFILE_DARK_EDGE_COLOR);
}

float LPub::GetPartEdgeContrast()
{
    if (gApplication)
        return lcGetPreferences().mPartEdgeContrast;
    else
        return lcGetProfileFloat(LC_PROFILE_PART_EDGE_CONTRAST);
}

float LPub::GetPartColorLightDarkIndex()
{
    if (gApplication)
        return lcGetPreferences().mPartColorValueLDIndex;
    else
        return lcGetProfileFloat(LC_PROFILE_PART_COLOR_VALUE_LD_INDEX);
}

void LPub::SetStudStyle(const NativeOptions* Options, bool value)
{
    float  PartColorValueLDIndex = lcGetProfileFloat(LC_PROFILE_PART_COLOR_VALUE_LD_INDEX);
    quint32 StudCylinderColor    = lcGetProfileInt(LC_PROFILE_STUD_CYLINDER_COLOR);
    quint32 PartEdgeColor        = lcGetProfileInt(LC_PROFILE_PART_EDGE_COLOR);
    quint32 BlackEdgeColor       = lcGetProfileInt(LC_PROFILE_BLACK_EDGE_COLOR);
    quint32 DarkEdgeColor        = lcGetProfileInt(LC_PROFILE_DARK_EDGE_COLOR);
    int StudStyle                = lcGetProfileInt(LC_PROFILE_STUD_STYLE);

    if (Options) {
        PartColorValueLDIndex = Options->LightDarkIndex;
        StudCylinderColor     = Options->StudCylinderColor;
        PartEdgeColor         = Options->PartEdgeColor;
        BlackEdgeColor        = Options->BlackEdgeColor;
        DarkEdgeColor         = Options->DarkEdgeColor;
        StudStyle             = Options->StudStyle;
    }

    lcPreferences& Preferences = lcGetPreferences();

    bool Change = GetStudStyle()                      != StudStyle;
         Change |= Preferences.mPartColorValueLDIndex != PartColorValueLDIndex;
         Change |= Preferences.mStudCylinderColor     != StudCylinderColor;
         Change |= Preferences.mPartEdgeColor         != PartEdgeColor;
         Change |= Preferences.mBlackEdgeColor        != BlackEdgeColor;
         Change |= Preferences.mDarkEdgeColor         != DarkEdgeColor;

    if (Change) {
        Preferences.mPartColorValueLDIndex = PartColorValueLDIndex;
        Preferences.mStudCylinderColor     = StudCylinderColor;
        Preferences.mPartEdgeColor         = PartEdgeColor;
        Preferences.mBlackEdgeColor        = BlackEdgeColor;
        Preferences.mDarkEdgeColor         = DarkEdgeColor;

        lcGetPiecesLibrary()->SetStudStyle(static_cast<lcStudStyle>(StudStyle), value);

        lcGetPiecesLibrary()->LoadColors();
    }
}

void LPub::SetAutomateEdgeColor(const NativeOptions* Options)
{
    bool  AutomateEdgeColor     = lcGetProfileInt(LC_PROFILE_AUTOMATE_EDGE_COLOR);
    float PartEdgeContrast      = lcGetProfileFloat(LC_PROFILE_PART_EDGE_CONTRAST);
    float PartColorValueLDIndex = lcGetProfileFloat(LC_PROFILE_PART_COLOR_VALUE_LD_INDEX);

    if (Options) {
        AutomateEdgeColor     = Options->AutoEdgeColor;
        PartEdgeContrast      = Options->EdgeContrast;
        PartColorValueLDIndex = Options->EdgeSaturation;
    }

    lcPreferences& Preferences = lcGetPreferences();

    bool Changed  = Preferences.mAutomateEdgeColor     != AutomateEdgeColor;
         Changed |= Preferences.mPartEdgeContrast      != PartEdgeContrast;
         Changed |= Preferences.mPartColorValueLDIndex != PartColorValueLDIndex;

    if (Changed) {
        Preferences.mAutomateEdgeColor     = AutomateEdgeColor;
        Preferences.mPartEdgeContrast      = PartEdgeContrast;
        Preferences.mPartColorValueLDIndex = PartColorValueLDIndex;

        lcGetPiecesLibrary()->LoadColors();
    }
}

/********************************************************************
 * Open visual editor project
 * *****************************************************************/

bool LPub::OpenProject(const NativeOptions* Options, int Type/*NATIVE_VIEW*/, bool UseFile/*false*/)
{
    bool Loaded = false;

    Project* Loader = nullptr;

    if (Type != NATIVE_VIEW) // NATIVE_IMAGE or NATIVE_EXPORT
    {
        if (Type == NATIVE_IMAGE)
            Loader = new Project(false/*IsPreview*/, NATIVE_IMAGE);
        else
            Loader = new Project();

        QString FileName, StepKey;
        if (UseFile && !Options->InputFileName.isEmpty())
           FileName = Options->InputFileName;
        else
           StepKey = Options->ViewerStepKey;

        Loaded = Loader->Load(FileName, StepKey, Options->ImageType, false/*ShowErrors*/);
        if (Loaded)
        {
            gApplication->SetProject(Loader);
            lcView::UpdateProjectViews(Loader);
        }
        else
            delete Loader;
    }
    else if (gMainWindow) // NATIVE_VIEW
    {
        if (UseFile && !Options->InputFileName.isEmpty()) {
            Loaded = gMainWindow->OpenProject(Options->InputFileName);
        } else {
            Loader = new Project();
            Loaded = Loader->Load(QString()/*FileName*/, Options->ViewerStepKey, Options->ImageType, true/*ShowErrors*/);
            if (Loaded)
            {
                gApplication->SetProject(Loader);
                lcView::UpdateProjectViews(Loader);
            }
            else
                delete Loader;
        }

        if (Loaded) {
            if (Options->IsReset) {
                gMainWindow->SetRelativeTransform(mRelativeTransform);
                gMainWindow->SetSeparateTransform(mSeparateTransform);
                gui->SetVisualEditorRotateTransform(mRotateTransform);
                gui->restoreLightAndViewpointDefaults();
                gui->enableVisualBuildModification();
                gui->enableVisualBuildModActions();
                if (Options->ImageType == Options::CSI) {
                    QVector<TypeLine> LineTypeIndexes;
                    PartSource Selection = VIEWER_CLR;
                    lcModel* ActiveModel = lcGetActiveProject()->GetMainModel();
                    if (ActiveModel)
                    {
                        emit gMainWindow->SelectedPartLinesSig(LineTypeIndexes, Selection);
                        ActiveModel->ResetModAction();
                    }
                }
            }
        }
        gui->RaiseVisualEditDockWindow();
    }

    if (Loaded && Type != NATIVE_EXPORT)
        Loaded = Render::RenderNativeView(Options, Type == NATIVE_IMAGE);

    if (gMainWindow && Options->IsReset && Loaded)
        gMainWindow->UpdateDefaultCameraProperties();

    return Loaded;
}

/********************************************************************
 *  Visual Editor viewpoint latitude longitude
 * *****************************************************************/

int LPub::SetViewpointLatLonDialog(bool SetCamera)
{
    if (!gMainWindow)
        return static_cast<int>(QDialog::Accepted);

    lcView* ActiveView = gui->GetActiveView();

    mLatitude       = 0.0f;
    mLongitude      = 0.0f;
    mDistanceFactor = qAbs(ActiveView->GetCamera()->GetDDF());
    float Distance  = 1.0f;

    ActiveView->GetCamera()->GetAngles(mLatitude, mLongitude, Distance);

    QPalette ReadOnlyPalette = QApplication::palette();
    if (Preferences::displayTheme == THEME_DARK)
        ReadOnlyPalette.setColor(QPalette::Base,QColor(Preferences::themeColors[THEME_DARK_PALETTE_MIDLIGHT]));
    else
        ReadOnlyPalette.setColor(QPalette::Base,QColor(Preferences::themeColors[THEME_DEFAULT_PALETTE_LIGHT]));
    ReadOnlyPalette.setColor(QPalette::Text,QColor(LPUB3D_DISABLED_TEXT_COLOUR));

    QDialog * LatLonDialog = new QDialog(gMainWindow);

    LatLonDialog->setWindowTitle(tr("Camera Latitude/Longitude"));

    LatLonDialog->setWhatsThis(lpubWT(WT_DIALOG_CAMERA_LAT_LON, LatLonDialog->windowTitle()));

    QFormLayout *LatLonForm = new QFormLayout(LatLonDialog);

    QGroupBox *LatLonGrpBox = new QGroupBox();

    LatLonForm->addWidget(LatLonGrpBox);

    QGridLayout *LatLonLayout = new QGridLayout(LatLonGrpBox);

    QLabel *LatLabel = new QLabel(tr("Latitude:"),LatLonDialog);
    LatLonLayout->addWidget(LatLabel,0,0);

    mLatSpin = new QSpinBox(LatLonDialog);
    mLatSpin->setRange(-360,360);
    mLatSpin->setSingleStep(1);
    mLatSpin->setValue(mLatitude);
    LatLonLayout->addWidget(mLatSpin,0,1);

    mLatResetButton = new QPushButton(LatLonDialog);
    mLatResetButton->setIcon(QIcon(":/resources/resetaction.png"));
    mLatResetButton->setIconSize(QSize(16,16));
    mLatResetButton->setStyleSheet("QPushButton { background-color: rgba(255,255,255,0);border: 0px; }");
    mLatResetButton->setToolTip(tr("Reset"));
    mLatResetButton->setEnabled(false);
    LatLonLayout->addWidget(mLatResetButton,0,2);
    connect(mLatSpin, SIGNAL(valueChanged(int)), this, SLOT(latResetEnabled(int)));
    connect(mLatResetButton, SIGNAL(clicked(bool)), this, SLOT(latReset(bool)));

    QLabel *LonLabel = new QLabel(tr("Longitude:"), LatLonDialog);
    LatLonLayout->addWidget(LonLabel,1,0);

    mLonSpin = new QSpinBox(LatLonDialog);
    mLonSpin->setRange(-360,360);
    mLonSpin->setSingleStep(1);
    mLonSpin->setValue(mLongitude);
    LatLonLayout->addWidget(mLonSpin,1,1);

    mLonResetButton = new QPushButton(LatLonDialog);
    mLonResetButton->setIcon(QIcon(":/resources/resetaction.png"));
    mLonResetButton->setIconSize(QSize(16,16));
    mLonResetButton->setStyleSheet("QPushButton { background-color: rgba(255,255,255,0);border: 0px; }");
    mLonResetButton->setToolTip(tr("Reset"));
    mLonResetButton->setEnabled(false);
    LatLonLayout->addWidget(mLonResetButton,1,2);
    connect(mLonSpin, SIGNAL(valueChanged(int)), this, SLOT(lonResetEnabled(int)));
    connect(mLonResetButton, SIGNAL(clicked(bool)), this, SLOT(lonReset(bool)));

    QCheckBox *DistanceCheck = new QCheckBox(tr("Distance Factor:"), LatLonDialog);
    LatLonLayout->addWidget(DistanceCheck,2,0);

    mDDFSpin = new QDoubleSpinBox(LatLonDialog);
    mDDFSpin->setEnabled(false);
    mDDFSpin->setRange(0.0f,100.0f);
    mDDFSpin->setSingleStep(0.01);
    mDDFSpin->setDecimals(2);
    mDDFSpin->setValue(mDistanceFactor);
    LatLonLayout->addWidget(mDDFSpin,2,1);

    mDDFResetButton = new QPushButton(LatLonDialog);
    mDDFResetButton->setIcon(QIcon(":/resources/resetaction.png"));
    mDDFResetButton->setIconSize(QSize(16,16));
    mDDFResetButton->setStyleSheet("QPushButton { background-color: rgba(255,255,255,0);border: 0px; }");
    mDDFResetButton->setToolTip(tr("Reset"));
    mDDFResetButton->setEnabled(false);
    LatLonLayout->addWidget(mDDFResetButton,2,2);
    connect(DistanceCheck, SIGNAL(clicked(bool)), this, SLOT(ddfSpinEnabled(bool)));
    connect(mDDFSpin, SIGNAL(valueChanged(double)), this, SLOT(ddfResetEnabled(double)));
    connect(mDDFResetButton, SIGNAL(clicked(bool)), this, SLOT(ddfReset(bool)));

    QLabel *DistanceLabel = new QLabel(tr("Disance:"), LatLonDialog);
    LatLonLayout->addWidget(DistanceLabel,2,3);
    QLineEdit *DistanceEdit = new QLineEdit(LatLonDialog);
    DistanceEdit->setText(QString::number(int(Distance)));
    DistanceEdit->setPalette(ReadOnlyPalette);
    DistanceEdit->setReadOnly(true);
    LatLonLayout->addWidget(DistanceEdit,2,4);

    // options - button box
    QDialogButtonBox *LatLonButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                                                             Qt::Horizontal, LatLonDialog);
    LatLonForm->addRow(LatLonButtonBox);
    QObject::connect(LatLonButtonBox, SIGNAL(accepted()), LatLonDialog, SLOT(accept()));
    QObject::connect(LatLonButtonBox, SIGNAL(rejected()), LatLonDialog, SLOT(reject()));

    QDialog::DialogCode DialogCode = static_cast<QDialog::DialogCode>(LatLonDialog->exec());
    if (DialogCode == QDialog::Accepted)
    {
         bool ApplyZoomExtents = true;
         float Latitude  = static_cast<float>(mLatSpin->value());
         float Longitude = static_cast<float>(mLonSpin->value());

         lcPreferences& Preferences = lcGetPreferences();

         if(DistanceCheck->isChecked())
         {
             ApplyZoomExtents = false;
             Preferences.mDDF = static_cast<float>(mDDFSpin->value());
         }

         ActiveView->SetViewpointLatLon(Latitude, Longitude, Distance, ApplyZoomExtents, SetCamera);

         if (Preferences.mDDF != mDistanceFactor)
             Preferences.mDDF  = mDistanceFactor;
    }
    else
    {
        // hack the lat/lon action CommandId to reset ViewpointGroup action to Home on cancel
        gMainWindow->mActions[LC_VIEW_VIEWPOINT_LAT_LON]->setProperty("CommandId", QVariant(LC_VIEW_VIEWPOINT_HOME));

        delete LatLonDialog;
    }

    return static_cast<int>(DialogCode);
}

void LPub::latResetEnabled(int i)
{
    mLatResetButton->setEnabled(i != static_cast<int>(mLatitude));
}

void LPub::latReset(bool)
{
    mLatResetButton->setEnabled(false);
    if(mLatSpin) {
        mLatSpin->setValue(mLatitude);
        mLatSpin->setFocus();
    }
}

void LPub::lonResetEnabled(int i)
{
    mLonResetButton->setEnabled(i != static_cast<int>(mLongitude));
}

void LPub::lonReset(bool)
{
    mLonResetButton->setEnabled(false);
    if(mLonSpin) {
        mLonSpin->setValue(mLongitude);
        mLonSpin->setFocus();
    }
}

void LPub::ddfResetEnabled(double d)
{
    mDDFResetButton->setEnabled(d != static_cast<double>(mDistanceFactor));
}

void LPub::ddfReset(bool)
{
    mDDFResetButton->setEnabled(false);
    if(mDDFSpin) {
        mDDFSpin->setValue(mDistanceFactor);
        mDDFSpin->setFocus();
    }
}

void LPub::ddfSpinEnabled(bool b)
{
    mDDFSpin->setEnabled(b);
}

/********************************************************************
 *  Fade previous steps from command
 * *****************************************************************/

bool LPub::setFadeStepsFromCommand()
{
  QString result;
  Where top(ldrawFile.topLevelFile(),0,0);
  Where topLevelModel(top);
  QRegExp fadeRx = QRegExp("FADE_STEPS ENABLED\\s*(GLOBAL)?\\s*TRUE");
  if (!Preferences::enableFadeSteps) {
    Preferences::enableFadeSteps = Gui::stepContains(topLevelModel,fadeRx,result,1);
    if (Preferences::enableFadeSteps) {
      if (result != QLatin1String("GLOBAL")) {
        emit lpub->messageSig(LOG_ERROR,tr("Top level FADE_STEPS set to ENABLED command must have GLOBAL qualifier."));
        Preferences::enableFadeSteps = false;
      } else if (!Preferences::finalModelEnabled) {
        result.clear();
        topLevelModel = top;
        fadeRx.setPattern("FINAL_MODEL_ENABLED\\s*(GLOBAL)?\\s*(?:TRUE|FALSE)");
        Preferences::finalModelEnabled = Gui::stepContains(topLevelModel,fadeRx,result,1);
        if (Preferences::finalModelEnabled) {
          emit lpub->messageSig(LOG_INFO, tr("Display Final Model is Enabled"));
        }
        if (result != QLatin1String("GLOBAL")) {
          emit lpub->messageSig(LOG_NOTICE,tr("FINAL_MODEL_ENABLED command should have GLOBAL qualifier."));
        }
      }
    }
  }

  bool setupFadeSteps = false;
  result.clear();
  topLevelModel = top;
  fadeRx.setPattern("FADE_STEPS SETUP\\s*(GLOBAL)?\\s*TRUE");
  setupFadeSteps = Gui::stepContains(topLevelModel,fadeRx,result,1);
  if (setupFadeSteps && result != QLatin1String("GLOBAL")) {
    emit lpub->messageSig(LOG_ERROR,tr("Top level FADE_STEPS SETUP command must have GLOBAL qualifier."));
    setupFadeSteps = false;
  }
  if (setupFadeSteps) {
    if (Preferences::enableFadeSteps) {
      emit lpub->messageSig(LOG_WARNING,tr("Enable fade previous steps setup local encountered but global fade previous steps is enabled.<br>"
                                           "Fade previous steps setup local will be ignored."),true/*Show Dialog*/);
      setupFadeSteps = false;
    }
  }

  if (Preferences::enableFadeSteps != Preferences::initEnableFadeSteps)
    emit lpub->messageSig(LOG_INFO_STATUS,tr("Fade Previous Steps is %1 - Set from meta command.")
                                            .arg(Preferences::enableFadeSteps ? "ON" : "OFF"));
  if (setupFadeSteps)
     emit lpub->messageSig(LOG_INFO_STATUS,tr("Fade Previous Steps Setup is ENABLED."));

  if (!Preferences::enableFadeSteps && !setupFadeSteps)
    return false;

  result.clear();
  topLevelModel = top;
  fadeRx.setPattern("FADE_STEPS OPACITY\\s*(?:GLOBAL)?\\s*(\\d+)");
  Gui::stepContains(topLevelModel,fadeRx,result,1);
  if (!result.isEmpty()) {
    bool ok = result.toInt(&ok);
    int fadeStepsOpacityCompare = Preferences::fadeStepsOpacity;
    Preferences::fadeStepsOpacity = ok ? result.toInt() : FADE_OPACITY_DEFAULT;
    bool fadeStepsOpacityChanged = Preferences::fadeStepsOpacity != fadeStepsOpacityCompare;
    if (fadeStepsOpacityChanged)
      emit lpub->messageSig(LOG_INFO,tr("Fade Steps Transparency changed from %1 to %2 percent - Set from meta command")
                                       .arg(fadeStepsOpacityCompare)
                                       .arg(Preferences::fadeStepsOpacity));
  }

  result.clear();
  topLevelModel = top;
  fadeRx.setPattern("FADE_STEPS COLOR\\s*(?:GLOBAL)?\\s*\"(\\w+)\"");
  Gui::stepContains(topLevelModel,fadeRx,result,1);
  if (!result.isEmpty()) {
    QColor ParsedColor = LDrawColor::color(result);
    bool fadeStepsUseColorCompare = Preferences::fadeStepsUseColour;
    Preferences::fadeStepsUseColour = ParsedColor.isValid();
    if (Preferences::fadeStepsUseColour != fadeStepsUseColorCompare)
      emit lpub->messageSig(LOG_INFO,tr("Use Fade Color is %1 - Set from meta command")
                                       .arg(Preferences::fadeStepsUseColour ? "ON" : "OFF"));
    QString fadeStepsColourCompare = Preferences::validFadeStepsColour;
    Preferences::validFadeStepsColour = ParsedColor.isValid() ? result : Preferences::validFadeStepsColour;
    if (QString(Preferences::validFadeStepsColour).toLower() != fadeStepsColourCompare.toLower())
      emit lpub->messageSig(LOG_INFO,tr("Fade Steps Color preference changed from %1 to %2 - Set from meta command")
                                       .arg(fadeStepsColourCompare.replace("_"," "))
                                       .arg(QString(Preferences::validFadeStepsColour).replace("_"," ")));
  }

  return setupFadeSteps;
}

bool LPub::setHighlightStepFromCommand()
{
  QString result;
  Where top(ldrawFile.topLevelFile(),0,0);
  Where topLevelModel(top);
  QRegExp highlightRx = QRegExp("HIGHLIGHT_STEP ENABLED\\s*(GLOBAL)?\\s*TRUE");
  if (!Preferences::enableHighlightStep) {
    Preferences::enableHighlightStep = Gui::stepContains(topLevelModel,highlightRx,result,1);
    if (Preferences::enableHighlightStep) {
      if (result != QLatin1String("GLOBAL")) {
        emit lpub->messageSig(LOG_ERROR,tr("Top level HIGHLIGHT_STEP ENABLED command must have GLOBAL qualifier."));
        Preferences::enableHighlightStep = false;
      } else if (!Preferences::finalModelEnabled) {
        result.clear();
        topLevelModel = top;
        highlightRx.setPattern("FINAL_MODEL_ENABLED\\s*(GLOBAL)?\\s*(?:TRUE|FALSE)");
        Preferences::finalModelEnabled = Gui::stepContains(topLevelModel,highlightRx,result,1);
        if (Preferences::finalModelEnabled) {
          emit lpub->messageSig(LOG_INFO,tr("Display Final Model is Enabled"));
        }
        if (result != QLatin1String("GLOBAL")) {
          emit lpub->messageSig(LOG_NOTICE,tr("FINAL_MODEL_ENABLED command should have GLOBAL qualifier."));
        }
      }
    }
  }

  bool setupHighlightStep = false;
  result.clear();
  topLevelModel = top;
  highlightRx.setPattern("HIGHLIGHT_STEP SETUP\\s*(GLOBAL)?\\s*TRUE");
  setupHighlightStep = Gui::stepContains(topLevelModel,highlightRx,result,1);
  if (setupHighlightStep && result != QLatin1String("GLOBAL")) {
    emit lpub->messageSig(LOG_ERROR,tr("Top level HIGHLIGHT_STEP SETUP command must have GLOBAL qualifier."));
    setupHighlightStep = false;
  }
  if (setupHighlightStep) {
    if (Preferences::enableHighlightStep) {
      emit lpub->messageSig(LOG_WARNING,tr("Enable highlight current step setup local encountered but global highlight current step is enabled.<br>"
                                           "Highlight current step setup local will be ignored."),true/*Show Dialog*/);
      setupHighlightStep = false;
    }
  }

  if (Preferences::enableHighlightStep != Preferences::initEnableHighlightStep)
    emit lpub->messageSig(LOG_INFO_STATUS,tr("Highlight Current Step is %1 - Set from meta command.")
                                            .arg(Preferences::enableHighlightStep ? "ON" : "OFF"));
  if (setupHighlightStep)
     emit lpub->messageSig(LOG_INFO_STATUS,tr("Highlight Current Step Setup is ENABLED."));

  if (!Preferences::enableHighlightStep && !setupHighlightStep)
    return false;

  result.clear();
  topLevelModel = top;
  highlightRx.setPattern("HIGHLIGHT_STEP COLOR\\s*(?:GLOBAL)?\\s*\"(0x|#)([\\da-fA-F]+)\"");
  if (Gui::stepContains(topLevelModel,highlightRx,result)) {
    result = QString("%1%2").arg(highlightRx.cap(1),highlightRx.cap(2));
    QColor ParsedColor = QColor(result);
    QString highlightStepColourCompare = Preferences::highlightStepColour;
    Preferences::highlightStepColour = ParsedColor.isValid() ? result : Preferences::validFadeStepsColour;
    bool highlightStepColorChanged = QString(Preferences::highlightStepColour).toLower() != highlightStepColourCompare.toLower();
    if (highlightStepColorChanged)
      emit lpub->messageSig(LOG_INFO,tr("Highlight Step Color preference changed from %1 to %2 - Set from meta command")
                                  .arg(highlightStepColourCompare)
                                  .arg(Preferences::highlightStepColour));
  }

  return setupHighlightStep;
}

bool LPub::setPreferredRendererFromCommand(const QString &preferredRenderer)
{
  if (preferredRenderer.isEmpty())
      return false;

  int renderer = Preferences::preferredRenderer;
  bool useLDVSingleCall = false;
  bool useLDVSnapShotList = false;
  bool useNativeGenerator = true;
  bool rendererChanged = false;
  bool renderFlagChanged = false;

  QString message;
  QString command = preferredRenderer.toLower();
  if (command == "native") {
    renderer = RENDERER_NATIVE;
  } else if (command == "ldview") {
    renderer = RENDERER_LDVIEW;
  } else if ((useLDVSingleCall = command == "ldview-sc")) {
    renderer = RENDERER_LDVIEW;
  } else if ((useLDVSnapShotList = command == "ldview-scsl")) {
    renderer = RENDERER_LDVIEW;
  } else if (command == "ldglite") {
    renderer = RENDERER_LDGLITE;
  } else if (command == "povray") {
    renderer = RENDERER_POVRAY;
  } else if (command == "povray-ldv") {
    renderer = RENDERER_POVRAY;
    useNativeGenerator = false;
  } else {
    emit lpub->messageSig(LOG_ERROR,tr("Invalid renderer console command option specified: '%1'.").arg(renderer));
    return rendererChanged;
  }

  if (renderer == RENDERER_LDVIEW) {
    if (Preferences::enableLDViewSingleCall != useLDVSingleCall) {
      message = tr("Renderer preference use LDView Single Call changed from %1 to %2.")
                        .arg(Preferences::enableLDViewSingleCall ? "Yes" : "No")
                        .arg(useLDVSingleCall ? "Yes" : "No");
      emit lpub->messageSig(LOG_INFO,message);
      Preferences::enableLDViewSingleCall = useLDVSingleCall;
      renderFlagChanged = true;
    }
    if (Preferences::enableLDViewSnaphsotList != useLDVSnapShotList) {
      message = tr("Renderer preference use LDView Snapshot List changed from %1 to %2.")
                        .arg(Preferences::enableLDViewSnaphsotList ? "Yes" : "No")
                        .arg(useLDVSingleCall ? "Yes" : "No");
      emit lpub->messageSig(LOG_INFO,message);
      Preferences::enableLDViewSnaphsotList = useLDVSnapShotList;
      if (useLDVSnapShotList)
          Preferences::enableLDViewSingleCall = true;
      if (!renderFlagChanged)
        renderFlagChanged = true;
    }
  } else if (renderer == RENDERER_POVRAY) {
    if (Preferences::useNativePovGenerator != useNativeGenerator) {
      message = tr("Renderer preference POV file generator changed from %1 to %2.")
                        .arg(Preferences::useNativePovGenerator ? rendererNames[RENDERER_NATIVE] : rendererNames[RENDERER_LDVIEW])
                        .arg(useNativeGenerator ? rendererNames[RENDERER_NATIVE] : rendererNames[RENDERER_LDVIEW]);
      emit lpub->messageSig(LOG_INFO,message);
      Preferences::useNativePovGenerator = useNativeGenerator;
      if (!renderFlagChanged)
        renderFlagChanged = true;
    }
  }

  rendererChanged = renderer != Preferences::preferredRenderer;

  if (rendererChanged || renderFlagChanged) {
    message = QString("Renderer preference will change from %1 to %2%3.")
                      .arg(rendererNames[Preferences::preferredRenderer])
                      .arg(rendererNames[renderer])
                      .arg(renderer == RENDERER_POVRAY ? tr(" (POV file generator is %1)")
                                                                 .arg(Preferences::useNativePovGenerator ? rendererNames[RENDERER_NATIVE] : rendererNames[RENDERER_LDVIEW]) :
                           renderer == RENDERER_LDVIEW ? useLDVSingleCall ?
                                                         useLDVSnapShotList ? tr(" (Single Call using Export File List)") :
                                                                              tr(" (Single Call)") :
                                                                              QString() : QString());
    emit lpub->messageSig(LOG_INFO,message);
  }

  if (rendererChanged) {
    Preferences::preferredRenderer = renderer;
    Render::setRenderer(Preferences::preferredRenderer);
    Preferences::preferredRendererPreferences(true/*global*/);
    Preferences::updatePOVRayConfigFiles();
  }

  return rendererChanged;
}

/********************************************************************
 *
 *
 *
 * *****************************************************************/

// flip orientation for landscape
int LPub::pageSize(PageMeta &meta, int which){
  int _which;
  if (meta.orientation.value() == Landscape){
      which == 0 ? _which = 1 : _which = 0;
    } else {
      _which = which;
    }
  return meta.size.valuePixels(_which);
}

/*********************************************
 *
 * split viewer step keys
 *
 ********************************************/

QStringList LPub::getViewerStepKeys(bool modelName, bool pliPart, const QString &key)
{
    // viewerStepKey - 3 elements:
    // CSI: 0=modelNameIndex, 1=lineNumber,   2=stepNumber [_dm (displayModel)]
    // SMI: 0=modelNameIndex, 1=lineNumber,   2=stepNumber [_Preview (Submodel Preview)]
    // PLI: 0=partNameString, 1=colourNumber, 2=stepNumber
    QStringList keys = key.isEmpty() ? viewerStepKey.split(";") : key.split(";");
    // confirm keys has at least 3 elements
    if (keys.size() < 3) {
#ifdef QT_DEBUG_MODE
        emit lpub->messageSig(LOG_DEBUG, tr("Parse stepKey [%1] failed").arg(viewerStepKey));
#endif
        return QStringList();
    } else if (keys.at(2).count("_")) {
        QStringList displayStepKeys = keys.at(2).split("_");
        keys.removeLast();
        keys.append(displayStepKeys);
    }

    if (!pliPart) {
        bool ok;
        int modelNameIndex = keys[0].toInt(&ok);
        if (!ok) {
#ifdef QT_DEBUG_MODE
            emit lpub->messageSig(LOG_DEBUG, tr("Parse stepKey failed. Expected model name index integer got [%1]").arg(keys[0]));
#endif
            return QStringList();
        }

        if (modelName)
            keys.replace(0,lpub->ldrawFile.getSubmodelName(modelNameIndex));
    }

    return keys;
}

/*********************************************
 *
 * extract stepKey - callled for CSI and SMI only
 *
 ********************************************/

  bool LPub::extractStepKey(Where &here, int &stepNumber, const QString &key)
  {
      // viewerStepKey elements CSI: 0=modelName, 1=lineNumber, 2=stepNumber [,3=_dm (displayModel)]
      QStringList keyArgs = getViewerStepKeys(true/*modelName*/, false/*pliPart*/, key);

      if (!keyArgs.size())
          return false;

      QString modelName  = keyArgs[0]; // Converted to modelName in getViewerStepKeys()

      bool ok[2];
      int lineNumber = keyArgs[1].toInt(&ok[0]);
      if (modelName.isEmpty()) {
          emit lpub->messageSig(LOG_ERROR,tr("Model name was not found for index [%1]").arg(keyArgs[1]));
          return false;
      } else if (!ok[0]) {
          emit lpub->messageSig(LOG_ERROR,tr("Line number is not an integer [%1]").arg(keyArgs[1]));
          return false;
      } else {
          here = Where(modelName,lpub->ldrawFile.getSubmodelIndex(modelName),lineNumber);
      }

      stepNumber = 0;
      if (lpub->page.modelDisplayOnlyStep || lpub->page.subModel.viewerSubmodel) {
          stepNumber = QStringList(keyArgs[2].split("_")).first().toInt(&ok[1]);
      } else {
          stepNumber = keyArgs[2].toInt(&ok[1]);
      }
      if (!ok[1]) {
          emit lpub->messageSig(LOG_WARNING,tr("Step number is not an integer [%1]. Using 0").arg(keyArgs[2]), true);
          stepNumber = 0;
      }

//      if (Preferences::debugLogging) {
//          QString messsage = QString("Step Key parse OK, modelName: %1, lineNumber: %2, stepNumber: %3")
//                                     .arg(modelName).arg(lineNumber).arg(stepNumber);
//          if (!stepNumber && page.pli.tsize() && page.pli.bom)
//              messsage = QString("Step Key parse OK but this is a BOM page, step pageNumber: %1")
//                                 .arg(stepPageNum);
//          emit lpub->messageSig(LOG_DEBUG, messsage);
//      }

      return true;
  }

/*********************************************
 *
 * current step - called for CSI
 *
 ********************************************/

void LPub::setCurrentStep(Step *step, Where &here, int stepNumber, int stepType)
{
    bool stepMatch = false;
    auto checkMatchOrIsCalledOutStep = [&] (Step* step, int &stepType, bool &stepMatch)
    {
        if (!(stepMatch = step->stepNumber.number == stepNumber || step->topOfStep() == here )) {
            for (int k = 0; k < step->list.size() && !stepMatch; k++) {
                if (step->list[k]->relativeType == CalloutType) {
                    Callout *callout = dynamic_cast<Callout *>(step->list[k]);
                    if (callout) {
                        stepType = BM_CALLOUT_STEP;
                        for (int l = 0; l < callout->list.size() && !stepMatch; l++) {
                            Range *range = dynamic_cast<Range *>(callout->list[l]);
                            if (range) {
                                for (int m = 0; m < range->list.size() && !stepMatch; m++) {
                                    if (range->relativeType == RangeType) {
                                        Step *step = dynamic_cast<Step *>(range->list[m]);
                                        if (step && step->relativeType == StepType) {
                                            setCurrentStep(step, here, stepNumber, stepType);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    };

    if (stepType == BM_MULTI_STEP) {
        for (int i = 0; i < page.list.size() && !stepMatch; i++) {
            Range *range = dynamic_cast<Range *>(page.list[i]);
            if (range) {
                for (int j = 0; j < range->list.size() && !stepMatch; j++) {
                    if (range->relativeType == RangeType) {
                        if ((step = dynamic_cast<Step *>(range->list[j]))) {
                            checkMatchOrIsCalledOutStep(step, stepType, stepMatch);
                            if (stepMatch)
                                break;
                        }
                    }
                }
            }
        }
    } else if (stepType == BM_SINGLE_STEP) {
        Range *range = dynamic_cast<Range *>(page.list[0]);
        if (range) {
            if (range->relativeType == RangeType) {
                if ((step = dynamic_cast<Step *>(range->list[0]))) {
                    checkMatchOrIsCalledOutStep(step, stepType, stepMatch);
                }
            }
        }
    } else if (step) {
        checkMatchOrIsCalledOutStep(step, stepType, stepMatch);
    }

    if (!stepMatch)
        step = nullptr;
    else
        setCurrentStep(step);

    if (Preferences::debugLogging && !stepMatch)
        emit lpub->messageSig(LOG_DEBUG, tr("%1 Step number %2 for %3 - modelName [%4] topOfStep [%5]")
                                           .arg(stepMatch ? "Match!" : "Oh oh!")
                                           .arg(tr("%1 %2").arg(stepNumber).arg(stepMatch ? "found" : "not found"))
                                           .arg(stepType == BM_MULTI_STEP  ? "multi step"  :
                                                stepType == BM_CALLOUT_STEP   ? "called out"  :
                                                stepType == BM_SINGLE_STEP ? "single step" : "gStep")
                                           .arg(here.modelName).arg(here.lineNumber));
}

bool LPub::setCurrentStep(const QString &key)
{
    currentStep    = nullptr;
    Step *step     = gStep;
    Where here     = Where();
    int stepType   = BM_NO_STEP;
    int stepNumber = 0;

    extractStepKey(here, stepNumber, key);

    QString stepNumberSpecified;
    if (!stepNumber)
        stepNumberSpecified = tr("not specified");
    else
        stepNumberSpecified = QString::number(stepNumber);

    if (page.relativeType == StepGroupType)
        stepType = BM_MULTI_STEP;
    else if (page.relativeType == SingleStepType && page.list.size())
        stepType = BM_SINGLE_STEP;

    if ((here != Where() || stepNumber) && (stepType || step)) {
        setCurrentStep(step, here, stepNumber, stepType);
    } else {
        emit lpub->messageSig(LOG_ERROR, tr("Could not determine step for '%1' at step number '%2'.")
                                           .arg(here.modelName).arg(stepNumberSpecified));
    }

#ifdef QT_DEBUG_MODE
    if (currentStep)
        emit lpub->messageSig(LOG_TRACE,tr("Step %1 loaded from key: %2")
                        .arg(currentStep->stepNumber.number).arg(key));
#endif

    return currentStep;
}

void LPub::setCurrentStep(Step *step)
{
    if (step) {
        if (step->viewerStepKey.isEmpty())
            step->viewerStepKey = QString("%1;%2;%3%4")
                                          .arg(step->top.modelIndex)
                                          .arg(step->top.lineNumber)
                                          .arg(step->stepNumber.number)
                                          .arg(lpub->mi.viewerStepKeySuffix(step->top, step));
        currentStep = step;
        viewerStepKey = currentStep->viewerStepKey;
#ifdef QT_DEBUG_MODE
        emit lpub->messageSig(LOG_DEBUG,tr("Set current step %1, with key '%2'.")
                                          .arg(currentStep->stepNumber.number).arg(currentStep->viewerStepKey));

    } else {
        emit lpub->messageSig(LOG_WARNING,tr("Specified step is NULL, cannot set current step."));
#endif
    }
}

/****************************************************************************
 *
 * Get current path
 *
 ***************************************************************************/

QString LPub::getFilePath(const QString &fileName)
{
    QFileInfo fileInfo(fileName);

    // relative path
    if (fileInfo.exists() && fileInfo.isFile()) {
        return fileInfo.absoluteFilePath();
    }

    // current path
    bool hasDot = false;
    const QString currentPath = QDir::currentPath();
    const QString filePath(QDir::toNativeSeparators(fileName));
    const QString dot(     QDir::toNativeSeparators("./"));
    const QString dotDot(  QDir::toNativeSeparators("../"));
    if ((hasDot = filePath.startsWith(dot)) || filePath.startsWith(dotDot)) {
        const QString file(hasDot ? QString(filePath).replace(dot,"") : filePath);
        if (!currentPath.isEmpty())
            fileInfo.setFile(QDir::toNativeSeparators(currentPath + QDir::separator() + file));
    }

    // absolute path
    if (fileInfo.exists() && fileInfo.isFile()) {
        return fileInfo.absoluteFilePath();
    }

    QString message = QString("Failed to resolve file %1 at path:<br>[%2]<br>")
                              .arg(fileInfo.fileName()).arg(currentPath);
    if (fileInfo.absoluteFilePath().contains("Google Drive",Qt::CaseInsensitive))
        message.append(QString("<br>Replace '...\\Google Drive\\' absolute path with mapped path 'G:\\My Drive\\'."
                               "<br>Use your specified drive letter (versus 'G:\\') accordingly."));

    emit lpub->messageSig(LOG_ERROR,message);

    return QString();
}

/****************************************************************************
 *
 * LPub snippet collection load
 *
 ***************************************************************************/

void LPub::loadSnippetCollection()
{
    QElapsedTimer timer;
    timer.start();

    snippetCollection = new SnippetCollection(this);

    JsonFile<Snippet>::load(":/resources/builtinsnippets.json", snippetCollection);
    int builtinSnippetCount = snippetCollection->count();

    SnippetCollection userSnippetCollection;
    const QString userSnippets = QString("%1/extras/user-snippets.json").arg(Preferences::lpubDataPath);
    JsonFile<Snippet>::load(QDir::toNativeSeparators(userSnippets), &userSnippetCollection);
    for (int i = 0; i < userSnippetCollection.count(); ++i) {
        Snippet userSnippet = userSnippetCollection.at(i);
        if (!snippetCollection->contains(userSnippet.trigger))
           snippetCollection->insert(userSnippet);
    }

    emit messageSig(LOG_INFO, tr("Snippet collection loaded %1 (builit-in %2 user %3) of %4 snippets. %5")
                    .arg(snippetCollection->count())
                    .arg(builtinSnippetCount)
                    .arg(userSnippetCollection.count())
                    .arg(builtinSnippetCount + userSnippetCollection.count())
                    .arg(elapsedTime(timer.elapsed())));
}

/****************************************************************************
 *
 * LPub command collection load
 *
 ***************************************************************************/

void LPub::loadCommandCollection()
{
    QElapsedTimer timer;
    timer.start();

    commandCollection = new CommandCollection(this);

    const QString userCommandDescriptions = QString("%1/extras/user-command-descriptions.json").arg(Preferences::lpubDataPath);
    JsonFile<Command>::load(QDir::toNativeSeparators(userCommandDescriptions), commandCollection);
    int userCommandCount = commandCollection->count();

    CommandCollection builtinCommandCollection;
    JsonFile<Command>::load(":/resources/builtincommanddescriptions.json", &builtinCommandCollection);
    int builtinCommandCount = builtinCommandCollection.count();

    QStringList commands;
    meta.doc(commands);

    QRegExp rx("^([^\\[\\(<\\\n]*)");
    Q_FOREACH(QString command, commands) {
        QString preamble;
        if (command.contains(rx))
            preamble = rx.cap(1).trimmed();
        else {
            logTrace() << QString("Preamble mis-match for command [%1]").arg(command);
            continue;
        }

        if (preamble.endsWith(" FONT"))
            command = preamble + " <\"font attributes\">";
        else
        if (preamble.endsWith(" FONT_COLOR"))
            command = preamble + " <\"color name|#RRGGBB\">";
        else
        if (preamble.endsWith(" PRIMARY") || preamble.endsWith(" SECONDARY") || preamble.endsWith(" TERTIARY"))
            command = preamble + " <\"Part Color\"|\"Part Category\"|\"Part Size\"|\"Part Element\"|\"No Sort\">";
        else
        if (preamble.endsWith(" PRIMARY_DIRECTION") || preamble.endsWith(" SECONDARY_DIRECTION") || preamble.endsWith(" TERTIARY_DIRECTION"))
            command = preamble + " <\"Ascending\"|\"Descending\">";
        else
        if (preamble.endsWith(" SUBMODEL_BACKGROUND_COLOR") || preamble.endsWith(" SUBMODEL_FONT_COLOR"))
            command = preamble + " <\"color name|#RRGGBB\"> [<\"color name|#RRGGBB\"> <\"color name|#RRGGBB\"> <\"color name|#RRGGBB\">]";
        else
        if (preamble.endsWith(" SUBMODEL_FONT"))
            command = preamble + " <\"font attributes\"> [<\"font attributes\"> <\"font attributes\"> <\"font attributes\">]";
        else
        if (preamble.endsWith(" COLOR") && !preamble.contains(" FADE_STEPS "))
            command = preamble + " <\"color name|#RRGGBB\">";
        else
        if (preamble.endsWith(" FILE"))
            command = preamble + " <\"file path\">";
        else
        if (preamble.endsWith(" LIGHT SHAPE"))
            command = preamble + " <\"Undefined\"|\"Square\"|\"Disk\"|\"Rectangle\"|\"Ellipse\">";
        else
        if (preamble.endsWith(" LIGHT TYPE"))
            command = preamble + " <\"Point\"|\"Sun\"|\"Spot\"|\"Area\">";
        else
        if (preamble.endsWith(" REMOVE NAME"))
            command = preamble + " <LDraw part name>";
        else
        if (preamble.endsWith(" REMOVE PART"))
            command = preamble + " <LDraw part name>";
        else
        if (preamble.endsWith(" STYLE"))
            command = preamble + " <annotation style integer 0-4>";

        Command _command;
        if (commandCollection->contains(preamble)) {
            _command = commandCollection->command(preamble);
            _command.command         = command;
            _command.modified        = Command::True;
        } else if (builtinCommandCollection.contains(preamble)) {
            _command = builtinCommandCollection.command(preamble);
            _command.command         = command;
            if (_command.modified   == Command::False)
                _command.description = command;
        } else {
            _command.preamble        = preamble;
            _command.command         = command;
            _command.description     = command;
        }
        commandCollection->insert(_command);
    }

    emit messageSig(LOG_INFO, tr("Command collection loaded %1 (default %2, builit-in %3, user %4) of %5 commands. %6")
                    .arg(commandCollection->count())
                    .arg(commands.count() - (builtinCommandCount + userCommandCount))
                    .arg(builtinCommandCount)
                    .arg(userCommandCount)
                    .arg(commands.count())
                    .arg(elapsedTime(timer.elapsed())));
}

/****************************************************************************
 *
 * Export LPub commands
 *
 ***************************************************************************/

bool LPub::exportMetaCommands(const QString &fileName, QString &result, bool descriptons)
{
    if (!commandCollection) {
        result = tr("The command collection is null.");
        emit messageSig(LOG_ERROR, result);
        return false;
    }

    static const QLatin1String fmtDateTime("yyyy-MM-dd hh:mm:ss");

    static const QLatin1String documentTitle("LPub Meta Commands");

    int n = 0;
    enum { FirstRec };
    QStringList doc, rec;
    for (int i = 0; i < commandCollection->count(); ++i) {
        Command command = commandCollection->at(i);
        rec = command.command.split("\n");
        for (int g = 0; g < rec.size(); g++) {
            if (g == FirstRec)
                doc.append(QString("%1. %2").arg(++n,3,10,QChar('0')).arg(rec[g]));
            else
                doc.append(QString("     %1").arg(rec[g]));
        }
        if (descriptons && command.modified == Command::True) {
            rec = command.description.split("\n");
            doc.append(QString("     0 // DESCRIPTION:"));
            for (int h = 0; h < rec.size(); h++)
                doc.append(QString("     %1").arg(rec[h]));
        }
    }

    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        emit messageSig(LOG_ERROR, tr("Cannot write file %1:\n%2.")
                        .arg(fileName)
                        .arg(file.errorString()));
        return false;
    }

    QTextStream out(&file);

    const QString generated = tr("%1 %2 - Generated on %3")
            .arg(VER_PRODUCTNAME_STR)
            .arg(documentTitle)
            .arg(QDateTime::currentDateTime().toString(fmtDateTime));

    const QString exported = QString("%1 of %2")
            .arg(n)
            .arg(commandCollection->count());

    // header
    doc.prepend(tr("Meta Commands:"));
    doc.prepend(QString());
    doc.prepend(QString());
    doc.prepend(tr("   |  Items bisected by a pipe (or) indicate multiple options are available; however, only one option per command can be specified."));
    doc.prepend(tr("  \" \" Items within double quotes are \"string\" values. Strings containing space require quotes. Numeric values are not quoted."));
    doc.prepend(tr("  [ ] Items within square brackets indicate optional meta command(s) and or value(s)."));
    doc.prepend(tr("  < > Items within chevron (greater,less than) indicate meta command value options required to complete the command."));
    doc.prepend(tr("  ( ) Items within curly brackets (parentheses) are built-in meta command options."));
    doc.prepend(QString());
    doc.prepend(tr("Meta Command Symbols:"));
    doc.prepend(QString());
#ifdef Q_OS_WIN
    doc.prepend(tr("---------------------------------------------------------------------------------"));
    doc.prepend(QString());
    doc.prepend(tr("  LPub UDL: %1assets/resources/LPub3D_Npp_UDL.xml.zip").arg(VER_HOMEPAGE_GITHUB_STR));
    doc.prepend(tr("  selected to enable LPub syntax highlighting."));
    doc.prepend(tr("  Install LPub3D_Npp_UDL.xml and open this file in Notepad++ with 'LPUB3D' UDL"));
    doc.prepend(tr("  available in the 'extras' folder or at the %1 homepage.").arg(VER_PRODUCTNAME_STR));
    doc.prepend(tr("  %1 has an LPub User Defined Language (UDL) configuration file for Notepad++").arg(VER_PRODUCTNAME_STR));
    doc.prepend(tr("  Best viewed on Windows with Notepad++ <https://notepad-plus-plus.org>."));
    doc.prepend(QString());
#endif
    doc.prepend(tr("---------------------------------------------------------------------------------"));
    doc.prepend(QString());
    doc.prepend(tr("  Copyright © 2022 by %1").arg(VER_PUBLISHER_STR));
    doc.prepend(tr("  License.....: GPLv3 - see %1").arg(VER_LICENSE_INFO_STR));
    doc.prepend(tr("  Homepage....: %1").arg(VER_HOMEPAGE_GITHUB_STR));
    doc.prepend(tr("  Last Update.: %1").arg(VER_COMPILE_DATE_STR));
    doc.prepend(tr("  Version.....: %1.%2").arg(VER_PRODUCTVERSION_STR).arg(VER_COMMIT_STR));
    doc.prepend(tr("  Author......: %1").arg(VER_PUBLISHER_STR));
    doc.prepend(tr("  Name........: %1, LPub Meta Commands").arg(VER_FILEDESCRIPTION_STR));
    doc.prepend(QString());
    doc.prepend(generated);

    // footer
    doc.append(QString());
    doc.append(tr("Meta Command Notes:"));
    doc.append(tr("-    The <\"page size id\"> meta value captures paper size, e.g. A4, B4, Letter, Custom, etc..."));
    doc.append(tr("     For custom page size use  <decimal width> <decimal height> \"Custom\""));
    doc.append(tr("-    The SUBMODEL metas below enable font and background settings for nested submodels and callouts."));
    doc.append(tr("-    The SUBMODEL_FONT meta is supported for up to four levels."));
    doc.append(tr("-    The SUBMODEL_FONT_COLOR meta is supported for up to four levels."));
    doc.append(tr("-    The SUBMODEL_BACKGROUND_COLOR meta is supported for up to four levels."));
    doc.append(tr("     Four level colours #FFFFFF, #FFFFCC, #FFCCCC, and #CCCCFF are predefined."));
    doc.append(tr("-    The <stud style integer 0-7> meta value captures the 7 stud style types."));
    doc.append(tr("     0 None"));
    doc.append(tr("     1 Thin line logo"));
    doc.append(tr("     2 Outline logo"));
    doc.append(tr("     3 Sharp top logo"));
    doc.append(tr("     4 Rounded top logo"));
    doc.append(tr("     5 Flattened logo"));
    doc.append(tr("     6 High contrast without logo"));
    doc.append(tr("     7 High contrast with logo"));
    doc.append(tr("-    The <annotation style integer 0-4> meta value captures the 4 annotation icon style types."));
    doc.append(tr("     0 None"));
    doc.append(tr("     1 Circle"));
    doc.append(tr("     2 Square"));
    doc.append(tr("     3 Rectangle"));
    doc.append(tr("     4 LEGO element"));
    doc.append(tr("-    The <line integer 0-5> meta value captures the 5 border line types."));
    doc.append(tr("     0 None"));
    doc.append(tr("     1 Solid        ----"));
    doc.append(tr("     2 Dash         - - "));
    doc.append(tr("     3 Dot          ...."));
    doc.append(tr("     4 Dash dot     -.-."));
    doc.append(tr("     5 Dash dot dot -..-"));
    doc.append(tr("-    The <\"font attributes\"> meta value is a comma-delimited <\"string\"> of 10 attributes."));
    doc.append(tr("     Example font attributes <\" Arial, 64, -1, 255, 75, 0, 0, 0, 0, 0 \">"));
    doc.append(tr("     1  FamilyName - \"Arial\""));
    doc.append(tr("     2  PointSizeF - 64 size of font, -1 if using PixelSize"));
    doc.append(tr("     3  PixelSize  - -1 size of font, -1 if using PointSizeF"));
    doc.append(tr("     4  StyleHint  - 255 = no style hint set, 5 = any style, 4 = system font, 0 = Helvetica, etc..."));
    doc.append(tr("     5  Weight     - 75 = bold, 50 = normal, etc..."));
    doc.append(tr("     6  Underline  - 0 = disabled, 1 = enabled"));
    doc.append(tr("     7  Strikeout  - 0 = disabled, 1 = enabled"));
    doc.append(tr("     8  StrikeOut  - 0 = disabled, 1 = enabled"));
    doc.append(tr("     9  FixedPitch - 0 = disabled, 1 = enabled"));
    doc.append(tr("     10 RawMode    - 0 obsolete, use default value"));
    doc.append(QString());
    doc.append(tr("End of file."));

    // export content list
    for (int i = 0; i < doc.size(); i++) {
        out << doc[i] << lpub_endl;
    }

    file.close();

    result = tr("Export %1 processed %2 commands.").arg(documentTitle).arg(exported);

    emit messageSig(LOG_INFO, QString(result).replace(".", QString(" to %1.")
                                                                   .arg(QDir::toNativeSeparators(fileName))));

    return true;
}

/****************************************************************************
 *
 * Remove LPub formatting from document, submodel, page, step, callout and BOM
 *
 ***************************************************************************/

void LPub::removeLPubFormatting(int option)
{
  QMessageBoxResizable box;
  box.setWindowIcon(QIcon());
  box.setIconPixmap (QPixmap(":/icons/lpub96.png"));
  box.setTextFormat (Qt::RichText);
  box.setStandardButtons (QMessageBox::Yes| QMessageBox::No);
  box.setDefaultButton   (QMessageBox::Yes);
  box.setWindowFlags (Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
  box.setWindowTitle(tr ("Remove LPub Formatting"));

  Where top;
  Where bottom;
  bool stepFound = false;

  switch (option)
  {
  case RLPF_DOCUMENT:
      top = Where(ldrawFile.topLevelFile(), 0, 1);
      break;
  case RLPF_SUBMODEL:
      top.modelIndex = currentStep ? currentStep->top.modelIndex : page.top.modelIndex;
      if (!top.modelIndex)
      {
          bool preserveGlobalHeaders = false;
          top = Where(ldrawFile.topLevelFile(), 0, 1);
          box.setStandardButtons (QMessageBox::Yes| QMessageBox::No |QMessageBox::Cancel);
          box.setDefaultButton(QMessageBox::No);
          QCheckBox *cb1 = new QCheckBox(tr ("Preserve GLOBAL header lines ?"));
          box.setCheckBox(cb1);
          QObject::connect(cb1, &QCheckBox::stateChanged, [&](int state) {
              if (static_cast<Qt::CheckState>(state) == Qt::CheckState::Checked)
                  preserveGlobalHeaders = true;
          });

          box.setText (tr("<b> Remove format from top level model - %1. </b>").arg(top.modelName));
          box.setInformativeText (tr ("This action will remove formatting for the top level model only.<br>"
                                      "Did you want to remove formatting from the entire document ?"));
          box.adjustSize();
          int result = box.exec();
          if (result == QMessageBox::Cancel) {
              return;
          } else if (result == QMessageBox::Yes) {
              option = RLPF_DOCUMENT;
              Gui::displayPageNum = 1 + Gui::pa;
              top = Where(ldrawFile.topLevelFile(), 0, 1);
          }

          if (preserveGlobalHeaders)
              mi.scanPastGlobal(top);
      }
      break;
  case RLPF_PAGE:
      if (page.relativeType == StepGroupType)
      {
          /*
          for Step Group we want page.top [Steps::top]
          which is set at MULTI_STEP BEGIN/END.
          Like this, we start the first step after the header.
          */
          top = page.top;
          bottom = page.bottom;
      } else {
          /*
          for Single Step and No Step we want topOf/bottomOf Steps
          [Steps::list(ranges)::list(ranges_elements)::top or
          Steps::top for non-step pages] which is set at the
          first encounter of type 1-5 part - so we should start
          the first step after the header.
          */
          top = page.topOfSteps();
          bottom = page.bottomOfSteps();
      }
      break;
  case RLPF_STEP:
      if (currentStep) {
          if (currentStep->multiStep) {
              int selectedStep  = 0;
              bool stepSelected = false;
              Steps *steps = dynamic_cast<Steps *>(&page);
              if (steps && page.relativeType == StepGroupType) {
                  selectedStep = QInputDialog::getInt(gui,tr("Steps"),tr("Which Step"),1,1,steps->list.size(),1,&stepSelected);
                  if (stepSelected) {
                      for (int i = 0; i < steps->list.size() && !stepFound; i++) {
                          if (steps->list[i]->relativeType == RangeType) {
                             Range *range = dynamic_cast<Range *>(steps->list[i]);
                             if (range) {
                                 for (int j = 0; j < range->list.size(); j++) {
                                    Step *step = dynamic_cast<Step *>(range->list[j]);
                                    if ((stepFound = step && step->stepNumber.number == selectedStep)) {
                                        top = step->topOfStep();
                                        bottom = step->bottomOfStep();
                                        break;
                                    }
                                 }
                             }
                          }
                      }

                      if (!stepFound)
                          box.setText (tr("<b> The selected step %1 was not found. </b>").arg(selectedStep));
                  } else {
                      box.setText (tr("<b> Step selection was canceled. </b>"));
                  }
              }
          } else {
              stepFound = true;
              top = currentStep->top;
              bottom = currentStep->bottom;
          }
      } else {
          box.setText (tr("<b> The current step is null. </b>"));
      }

      if (!stepFound) {
          box.setInformativeText (tr ("Do you want to remove formatting for the current page ?"));
          if (box.exec() == QMessageBox::Yes) {
              top = page.top;
              bottom = page.bottom;
              option = RLPF_PAGE;
          } else
              return;
      }
      break;
  case RLPF_BOM:
         //start at the bottom of the page's last step
         bottom = Gui::topOfPages[Gui::displayPageNum];
         top = Where(bottom.modelName, bottom.modelIndex, 1);
      break;
  default:
      break;
  }

  mi.removeLPubFormatting(option, top, bottom);
}

/****************************************************************************
 *
 * Download with progress monotor
 *
 ***************************************************************************/

void LPub::downloadFile(QString URL, QString title, bool promptRedirect, bool showProgress)
{
    mTitle = title;
    mShowProgress = showProgress;
    mPromptRedirect = promptRedirect;

    if (mShowProgress) {
        mProgressDialog = new QProgressDialog(nullptr);
        connect(mProgressDialog, SIGNAL(canceled()), this, SLOT(cancelDownload()));

        mProgressDialog->setWindowTitle(tr("Downloading"));
        mProgressDialog->setLabelText(tr("Downloading %1").arg(mTitle));
        mProgressDialog->show();
    }

    mHttpManager   = new QNetworkAccessManager(this);

    mUrl = URL;

    mHttpRequestAborted = false;

    startRequest(mUrl);

    while (mHttpReply)
        QApplication::processEvents();
}

void LPub::updateDownloadProgress(qint64 bytesRead, qint64 totalBytes)
{
    if (mHttpRequestAborted)
        return;
    if (mShowProgress) {
        mProgressDialog->setMaximum(int(totalBytes));
        mProgressDialog->setValue(int(bytesRead));
    }
}

void LPub::startRequest(QUrl url)
{
    QNetworkRequest request(url);
#if (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0))
    if (!mPromptRedirect)
        request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
#endif
    mHttpReply = mHttpManager->get(request);

    connect(mHttpReply, SIGNAL(downloadProgress(qint64,qint64)),
            this, SLOT(updateDownloadProgress(qint64,qint64)));

    connect(mHttpReply, SIGNAL(finished()),
            this, SLOT(httpDownloadFinished()));
}

void LPub::cancelDownload()
{
    mHttpRequestAborted = true;
    mHttpReply->abort();
}

void LPub::httpDownloadFinished()
{
    if (mHttpRequestAborted) {
        mByteArray.clear();
        mHttpReply->deleteLater();
        mHttpReply = nullptr;
        if (mShowProgress)
            mProgressDialog->close();
        return;
    }

    QString message;
    QString const messageTitle = tr("%1 Download").arg(VER_PRODUCTNAME_STR);
    QVariant redirectionTarget = mHttpReply->attribute(QNetworkRequest::RedirectionTargetAttribute);
    if (mHttpReply->error()) {
        mByteArray.clear();
        message = tr("%1 download failed: %2.")
                     .arg(mTitle).arg(mHttpReply->errorString());
        if (message.endsWith("not found."))
            message.append(tr("<br>Your internet connection may be interrupted."));
        if (Preferences::modeGUI){
            QMessageBox::warning(nullptr, messageTitle, message);
        } else {
            logError() << message;
        }
    } else if (!redirectionTarget.isNull()) {
        // This block should only trigger for redirects when Qt
        // is less than 5.6.0 or if prompt redirect set to True
        QUrl newUrl = mUrl.resolved(redirectionTarget.toUrl());
        bool proceedToRedirect = true;
        if (mPromptRedirect && Preferences::modeGUI) {
            proceedToRedirect = QMessageBox::question(nullptr, messageTitle,
                                              tr("Download redirect to %1 ?").arg(newUrl.toString()),
                                              QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes;
        } else {
            logNotice() << tr("Download redirect to %1 ?").arg(newUrl.toString());
        }
        if (proceedToRedirect) {
            mUrl = newUrl;
            mHttpReply->deleteLater();
            mByteArray.resize(0);
            startRequest(mUrl);
            return;
        }
    } else {
        mByteArray = mHttpReply->readAll();
    }

    if (mShowProgress)
        mProgressDialog->close();

    mHttpReply->deleteLater();
    mHttpReply = nullptr;
    mHttpManager = nullptr;
}

QString LPub::elapsedTime(const qint64 &duration) {

    qint64 elapsed = duration;
    int milliseconds = int(elapsed % 1000);
    elapsed /= 1000;
    int seconds = int(elapsed % 60);
    elapsed /= 60;
    int minutes = int(elapsed % 60);
    elapsed /= 60;
    int hours = int(elapsed % 24);

    return tr("Elapsed time: %1%2%3")
                   .arg(hours   >   0 ?
                                  QString("%1 %2 ")
                                          .arg(hours)
                                          .arg(hours   > 1 ? tr("hours")   : tr("hour")) :
                                  QString())
                   .arg(minutes > 0 ?
                                  QString("%1 %2 ")
                                          .arg(minutes)
                                          .arg(minutes > 1 ? tr("minutes") : tr("minute")) :
                                  QString())
                   .arg(QString("%1.%2 %3")
                                .arg(seconds)
                                .arg(milliseconds,3,10,QLatin1Char('0'))
                                .arg(seconds > 1 ? tr("seconds") : tr("second")));

}

void LPub::SetShadingMode(lcShadingMode ShadingMode)
{
    if (gMainWindow)
        gMainWindow->SetShadingMode(ShadingMode);
}

PartWorker& LPub::partWorkerLDSearchDirs()
{
    return gui->partWorkerLDSearchDirs;
}

// the next four calls are used when setting Visual Editor preferences
void LPub::clearAndReloadModelFile(bool fileReload, bool savePrompt)
{
    emit gui->clearAndReloadModelFileSig(fileReload, savePrompt);
}

void LPub::reloadCurrentPage(bool savePrompt)
{
    emit gui->reloadCurrentPageSig(savePrompt);
}

void LPub::restartApplication(bool changeLibrary, bool prompt)
{
    emit gui->restartApplicationSig(changeLibrary, prompt);
}

void LPub::setShortcutKeywords()
{
    QElapsedTimer timer;
    timer.start();

    // setup shortcut keywords for filter completer
    QStringList kwList = QStringList()
        << shortcutParentNames[NO_ACTION]
        << shortcutParentNames[MAIN_WINDOW_ACTION]
        << shortcutParentNames[COMMAND_EDITOR_ACTION]
        << shortcutParentNames[TEXT_EDITOR_ACTION]
        << shortcutParentNames[VISUAL_EDITOR_ACTION]
        << shortcutParentNames[PARAMS_EDITOR_ACTION]
        << shortcutParentNames[COMMANDS_DIALOG_ACTION]
        << shortcutParentNames[COMMANDS_TEXT_EDIT_ACTION]
        << shortcutParentNames[OTHER_ACTION]
           ;
    foreach (Action act, actions) {
        if (act.action) {
            QStringList sl = act.id.split(".");
            kwList << QString(sl.takeLast()).split(" ");
            foreach (QString string, sl)
                kwList << string.replace(" ", "");
            foreach (const QString &word, kwList)
                if (word.size() < 2)
                    kwList.removeAll(word);
        }
    }
    kwList.removeDuplicates();

    auto lt = [] (const QString &w1, const QString &w2) { return w1 < w2; };
    std::sort(kwList.begin(), kwList.end(),lt);

    shortcutIdKeywords = kwList;

#ifdef QT_DEBUG_MODE
    emit gui->messageSig(LOG_NOTICE, tr("Loaded %1 shortcut keywords.%2")
                                        .arg(shortcutIdKeywords.size())
                                        .arg(elapsedTime(timer.elapsed())));
#endif
}

void LPub::setDefaultKeyboardShortcuts()
{
    QMap<QString, Action>::iterator it = actions.begin();
    while (it != actions.end()) {
        const QKeySequence shortcut(it.value().action->property("defaultshortcut").value<QKeySequence>());
        it.value().action->setShortcut(shortcut);
        ++it;
    }
}

void LPub::setKeyboardShortcuts()
{
    QMap<QString, Action>::const_iterator it = actions.constBegin();
    while (it != actions.constEnd())
    {
        if (it.value().action) {
            setKeyboardShortcut(it.value().action);
        }
        ++it;
    }
    foreach (QAction *action, textEdit->actions()) {
        setKeyboardShortcut(action);
    }
    foreach (QAction *action, commandsDialog->actions()) {
        setKeyboardShortcut(action);
    }
    foreach (QAction *action, commandTextEdit->actions()) {
        setKeyboardShortcut(action);
    }
    foreach (QAction *action, snippetTextEdit->actions()) {
        setKeyboardShortcut(action);
    }
}

void LPub::setKeyboardShortcut(QAction *action)
{
    bool idOk = true;
    bool userDefined = false;
    int parentID = action->objectName().split(".").last().toInt(&idOk);
    if ((userDefined = Preferences::hasKeyboardShortcut(action->objectName()))) {
        action->setShortcut(Preferences::keyboardShortcut(action->objectName()));
    }
    if (idOk && (parentID == TEXT_EDITOR_ACTION || userDefined) && !action->shortcut().isEmpty()) {
        const int position = action->toolTip().lastIndexOf("-");
        const QString toolTip = action->toolTip().left(position).trimmed();
        const QString shortcutText = action->shortcut().toString(QKeySequence::NativeText);
        action->setToolTip(QString("%1 - %2").arg(toolTip).arg(shortcutText));
    }
}

void LPub::setKeyboardShortcut(QMenu *menu)
{
    // setKeyboardShortcut(ui.menuEdit);

    foreach (QAction *action, menu->actions()) {
        if (action->menu()) {
            // recurse into submenu
            setKeyboardShortcut(action->menu());
        } else {
            setKeyboardShortcut(action);
        }
    }
}

QAction *LPub::getAct(const QString &objectName)
{
    if (actions.contains(objectName))
        return actions.value(objectName).action;
    emit messageSig(LOG_TRACE, tr("Action was not found or is null [%1]").arg(objectName));
    return nullptr;
}

void LPub::loadBanner(const int &type)
{
    if (!Preferences::modeGUI)
        return;

    QTemporaryDir tempDir;
    if (tempDir.isValid())
        Gui::loadBanner(type, tempDir.path());
}

void LPub::setupChangeLogUpdate()
{
    // QSimpleUpdater start
    m_updaterCancelled = false;
    m_updater = QSimpleUpdater::getInstance();
    connect (m_updater, SIGNAL(checkingFinished (QString)),
             this,      SLOT(  updateChangelog (QString)));

    connect (m_updater, SIGNAL(cancel()),
             this,      SLOT(  updaterCancelled ()));

    // set release notes
    if (Preferences::autoUpdateChangeLog) {
        //populate release notes from the web
        m_updater->retrieveChangeLog(LPub::DEFS_URL);
    } else {
        // populate release notes from local file
        updateChangelog(QString());
    }
}

void LPub::updaterCancelled()
{
  m_updaterCancelled = true;
}

void LPub::updateChangelog (const QString &url)
{
    auto processRequest = [&] ()
    {
        if (m_updater->getUpdateAvailable(url) || m_updater->getChangelogOnly(url)) {
            if (!m_updaterCancelled) {
#if defined LP3D_CONTINUOUS_BUILD || defined LP3D_DEVOPS_BUILD || defined LP3D_NEXT_BUILD
#ifdef QT_DEBUG_MODE
                m_versionInfo = tr("Change Log for version %1 revision %2 (%3)")
                                 .arg(qApp->applicationVersion(), QString::fromLatin1(VER_REVISION_STR), QString::fromLatin1(VER_BUILD_TYPE_STR));
#else
                m_versionInfo = tr("Change Log for version %1 revision %2 (%3)")
                                 .arg(m_updater->getLatestVersion(url), m_updater->getLatestRevision(DEFS_URL), QString::fromLatin1(VER_BUILD_TYPE_STR));
#endif
#else
                int revisionNumber = m_updater->getLatestRevision(DEFS_URL).toInt();
                m_versionInfo = tr("Change Log for version %1%2")
                                 .arg(m_updater->getLatestVersion(url), revisionNumber ? QString(" revision %1").arg(m_updater->getLatestRevision(DEFS_URL)) : "");
#endif
                m_setReleaseNotesAsText = m_updater->compareVersionStr(url, m_updater->getLatestVersion(url), PLAINTEXT_CHANGE_LOG_CUTOFF_VERSION);
                m_releaseNotesContent = m_updater->getChangelog(url);
            }
        }
        emit checkForUpdatesFinished();
    };

    if (url == DEFS_URL)
        processRequest();
    else {
#if defined LP3D_CONTINUOUS_BUILD || defined LP3D_DEVOPS_BUILD || defined LP3D_NEXT_BUILD
        m_versionInfo = tr("Change Log for version %1 revision %2 (%3)")
                .arg(qApp->applicationVersion(), QString::fromLatin1(VER_REVISION_STR), QString::fromLatin1(VER_BUILD_TYPE_STR));
#else
        m_versionInfo = tr("Change Log for version %1%2")
                .arg(qApp->applicationVersion(), QString::fromLatin1(VER_REVISION_STR));
#endif
        //populate releaseNotes
        QString releaseNotesFile = QString("%1/%2/RELEASE_NOTES.html").arg(Preferences::lpub3dPath).arg(Preferences::lpub3dDocsResourcePath);

        QFile file(releaseNotesFile);
        if (! file.open(QFile::ReadOnly | QFile::Text)) {
            m_setReleaseNotesAsText = true;
            m_releaseNotesContent = tr("Failed to open Release Notes file: \n%1:\n%2")
                                             .arg(releaseNotesFile)
                                             .arg(file.errorString());
        } else {
            m_releaseNotesContent = QString::fromStdString(file.readAll().toStdString());
        }
    }

    if (m_versionInfo.isEmpty())
        m_versionInfo = tr("Undefined");
}

void LPub::saveVisualEditorTransformSettings()
{
    mRotateTransform   = gui->VisualEditorRotateTransform();
    mRelativeTransform = gMainWindow->GetRelativeTransform();
    mSeparateTransform = gMainWindow->GetSeparateTransform();
}
