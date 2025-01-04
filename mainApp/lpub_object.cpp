/****************************************************************************
**
** Copyright (C) 2022 - 2025 Trevor SANDY. All rights reserved.
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
  visualEditorPreferencesDialog = new lcQPreferencesDialog(gui);
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

bool LPub::GetStudCylinderColorEnabled()
{
    if (gApplication)
        return lcGetPreferences().mStudCylinderColorEnabled;
    else
        return lcGetProfileInt(LC_PROFILE_STUD_CYLINDER_COLOR_ENABLED);
}

quint32 LPub::GetStudCylinderColor()
{
    if (gApplication)
        return lcGetPreferences().mStudCylinderColor;
    else
        return lcGetProfileInt(LC_PROFILE_STUD_CYLINDER_COLOR);
}

bool LPub::GetPartEdgeColorEnabled()
{
    if (gApplication)
        return lcGetPreferences().mPartEdgeColorEnabled;
    else
        return lcGetProfileInt(LC_PROFILE_PART_EDGE_COLOR_ENABLED);
}

quint32 LPub::GetPartEdgeColor()
{
    if (gApplication)
        return lcGetPreferences().mPartEdgeColor;
    else
        return lcGetProfileInt(LC_PROFILE_PART_EDGE_COLOR);
}

bool LPub::GetBlackEdgeColorEnabled()
{
    if (gApplication)
        return lcGetPreferences().mBlackEdgeColorEnabled;
    else
        return lcGetProfileInt(LC_PROFILE_BLACK_EDGE_COLOR_ENABLED);
}

bool LPub::GetDarkEdgeColorEnabled()
{
    if (gApplication)
        return lcGetPreferences().mDarkEdgeColorEnabled;
    else
        return lcGetProfileInt(LC_PROFILE_DARK_EDGE_COLOR_ENABLED);
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

bool LPub::GetAutomateEdgeColor()
{
    if (gApplication)
        return lcGetPreferences().mAutomateEdgeColor;
    else
        return lcGetProfileInt(LC_PROFILE_AUTOMATE_EDGE_COLOR);
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

void LPub::SetStudStyle(const NativeOptions* Options, bool Reload, bool Changed)
{
    bool Update                   = Changed;
    int StudStyle                 = lcGetProfileInt(  LC_PROFILE_STUD_STYLE);
    float LightDarkIndex          = lcGetProfileFloat(LC_PROFILE_PART_COLOR_VALUE_LD_INDEX);
    quint32 StudCylinderColor     = lcGetProfileInt(  LC_PROFILE_STUD_CYLINDER_COLOR);
    quint32 PartEdgeColor         = lcGetProfileInt(  LC_PROFILE_PART_EDGE_COLOR);
    quint32 BlackEdgeColor        = lcGetProfileInt(  LC_PROFILE_BLACK_EDGE_COLOR);
    quint32 DarkEdgeColor         = lcGetProfileInt(  LC_PROFILE_DARK_EDGE_COLOR);
    bool StudCylinderColorEnabled = lcGetProfileInt(  LC_PROFILE_STUD_CYLINDER_COLOR_ENABLED);
    bool PartEdgeColorEnabled     = lcGetProfileInt(  LC_PROFILE_PART_EDGE_COLOR_ENABLED);
    bool BlackEdgeColorEnabled    = lcGetProfileInt(  LC_PROFILE_BLACK_EDGE_COLOR_ENABLED);
    bool DarkEdgeColorEnabled     = lcGetProfileInt(  LC_PROFILE_DARK_EDGE_COLOR_ENABLED);
    lcPreferences& Preferences    = lcGetPreferences();

    if (!Update) {
        if (Options) {
            LightDarkIndex           = Options->LightDarkIndex;
            StudCylinderColorEnabled = Options->StudCylinderColorEnabled;
            StudCylinderColor        = Options->StudCylinderColor;
            PartEdgeColorEnabled     = Options->PartEdgeColorEnabled;
            PartEdgeColor            = Options->PartEdgeColor;
            BlackEdgeColorEnabled    = Options->BlackEdgeColorEnabled;
            BlackEdgeColor           = Options->BlackEdgeColor;
            DarkEdgeColorEnabled     = Options->DarkEdgeColorEnabled;
            DarkEdgeColor            = Options->DarkEdgeColor;
            StudStyle                = Options->StudStyle;
        }
        Update  = GetStudStyle()                        != StudStyle;
        Update |= Preferences.mPartColorValueLDIndex    != LightDarkIndex;
        Update |= Preferences.mStudCylinderColorEnabled != StudCylinderColorEnabled;
        Update |= Preferences.mStudCylinderColor        != StudCylinderColor;
        Update |= Preferences.mPartEdgeColorEnabled     != PartEdgeColorEnabled;
        Update |= Preferences.mPartEdgeColor            != PartEdgeColor;
        Update |= Preferences.mBlackEdgeColorEnabled    != BlackEdgeColorEnabled;
        Update |= Preferences.mBlackEdgeColor           != BlackEdgeColor;
        Update |= Preferences.mDarkEdgeColorEnabled     != DarkEdgeColorEnabled;
        Update |= Preferences.mDarkEdgeColor            != DarkEdgeColor;
    }

    if (Update) {
        if (Options) {
            StudStyle = Options->StudStyle;
            Preferences.mPartColorValueLDIndex    = Options->LightDarkIndex;
            Preferences.mStudCylinderColorEnabled = Options->StudCylinderColorEnabled;
            Preferences.mStudCylinderColor        = Options->StudCylinderColor;
            Preferences.mPartEdgeColorEnabled     = Options->PartEdgeColorEnabled;
            Preferences.mPartEdgeColor            = Options->PartEdgeColor;
            Preferences.mBlackEdgeColorEnabled    = Options->BlackEdgeColorEnabled;
            Preferences.mBlackEdgeColor           = Options->BlackEdgeColor;
            Preferences.mDarkEdgeColorEnabled     = Options->DarkEdgeColorEnabled;
            Preferences.mDarkEdgeColor            = Options->DarkEdgeColor;
        } else {
            Preferences.mPartColorValueLDIndex    = LightDarkIndex;
            Preferences.mStudCylinderColorEnabled = StudCylinderColorEnabled;
            Preferences.mStudCylinderColor        = StudCylinderColor;
            Preferences.mPartEdgeColorEnabled     = PartEdgeColorEnabled;
            Preferences.mPartEdgeColor            = PartEdgeColor;
            Preferences.mBlackEdgeColorEnabled    = BlackEdgeColorEnabled;
            Preferences.mBlackEdgeColor           = BlackEdgeColor;
            Preferences.mDarkEdgeColorEnabled     = DarkEdgeColorEnabled;
            Preferences.mDarkEdgeColor            = DarkEdgeColor;
        }

        lcGetPiecesLibrary()->SetStudStyle(static_cast<lcStudStyle>(StudStyle), Reload, Preferences.mStudCylinderColorEnabled);

        lcGetPiecesLibrary()->LoadColors();
    }
}

void LPub::SetAutomateEdgeColor(const NativeOptions* Options, bool Changed)
{
    bool  Update         = Changed;
    bool  AutoEdgeColor  = lcGetProfileInt(  LC_PROFILE_AUTOMATE_EDGE_COLOR);
    float EdgeContrast   = lcGetProfileFloat(LC_PROFILE_PART_EDGE_CONTRAST);
    float LightDarkIndex = lcGetProfileFloat(LC_PROFILE_PART_COLOR_VALUE_LD_INDEX);
    lcPreferences& Preferences = lcGetPreferences();

    if (!Update) {
        if (Options) {
            AutoEdgeColor  = Options->AutoEdgeColor;
            EdgeContrast   = Options->EdgeContrast;
            LightDarkIndex = Options->LightDarkIndex;
        }
        Update  = Preferences.mAutomateEdgeColor     != AutoEdgeColor;
        Update |= Preferences.mPartEdgeContrast      != EdgeContrast;
        Update |= Preferences.mPartColorValueLDIndex != LightDarkIndex;
    }

    if (Update) {
        if (Options) {
            Preferences.mAutomateEdgeColor     = Options->AutoEdgeColor;
            Preferences.mPartEdgeContrast      = Options->EdgeContrast;
            Preferences.mPartColorValueLDIndex = Options->LightDarkIndex;
        } else {
            Preferences.mAutomateEdgeColor     = AutoEdgeColor;
            Preferences.mPartEdgeContrast      = EdgeContrast;
            Preferences.mPartColorValueLDIndex = LightDarkIndex;
        }

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

    lcPreferences& Preferences = lcGetPreferences();

    Preferences.mLPubFadeHighlight = Options->LPubFadeHighlight;
    Preferences.mFadeStepsColor = lcGetProfileInt(LC_PROFILE_FADE_STEPS_COLOR);
    Preferences.mHighlightNewPartsColor = lcGetProfileInt(LC_PROFILE_HIGHLIGHT_NEW_PARTS_COLOR);

    if (Options->LPubFadeHighlight)
    {
        Preferences.mFadeSteps = lcGetProfileInt(LC_PROFILE_FADE_STEPS);
        Preferences.mHighlightNewParts = lcGetProfileInt(LC_PROFILE_HIGHLIGHT_NEW_PARTS);
    }
    else
    {
        Preferences.mFadeSteps = Options->FadeParts;
        Preferences.mHighlightNewParts = Options->HighlightParts;

        if (Options->FadeParts)
        {
            QColor FC;
            if (Preferences::fadeStepsUseColour)
            {
                FC = LDrawColor::color(Preferences::validFadeStepsColour);
                if (FC.isValid())
                    Preferences.mFadeStepsColor = LC_RGBA(FC.red(), FC.green(), FC.blue(), FC.alpha());
            }

            quint32* C = &Preferences.mFadeStepsColor;
            FC = QColor(LC_RGBA_RED(*C), LC_RGBA_GREEN(*C), LC_RGBA_BLUE(*C), LC_RGBA_ALPHA(*C));

            if (FC.isValid()) {
                FC.setAlpha(LPUB3D_OPACITY_TO_ALPHA(Preferences::fadeStepsOpacity, 255));
                Preferences.mFadeStepsColor = LC_RGBA(FC.red(), FC.green(), FC.blue(), FC.alpha());
            }
        }

        if (Options->HighlightParts)
        {
            QColor HC = QColor(Preferences::highlightStepColour);
            if (HC.isValid())
                Preferences.mHighlightNewPartsColor = LC_RGBA(HC.red(), HC.green(), HC.blue(), HC.alpha());

            if (Preferences.mLineWidth != Preferences::highlightStepLineWidth)
                Preferences.mLineWidth = Preferences::highlightStepLineWidth;
        }
    }

    if (Type != NATIVE_VIEW) // NATIVE_IMAGE or NATIVE_EXPORT
    {
        if (Type == NATIVE_IMAGE)
            Loader = new Project(false/*IsPreview*/, NATIVE_IMAGE);
        else
            Loader = new Project();

        if (Options->LPubFadeHighlight)
            Loader->SetLPubFadeHighlightParts(
                Options->FadeParts,
                Options->HighlightParts);

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
        Loader = new Project();

        if (Options->LPubFadeHighlight)
            Loader->SetLPubFadeHighlightParts(
                Options->FadeParts,
                Options->HighlightParts);

        if (UseFile && !Options->InputFileName.isEmpty())
            Loaded = Loader->Load(Options->InputFileName, true/*ShowErrors*/, Options->ViewerStepKey);
        else
            Loaded = Loader->Load(QString()/*FileName*/, Options->ViewerStepKey, Options->ImageType, true/*ShowErrors*/);

        if (Loaded)
        {
            gApplication->SetProject(Loader);
            lcView::UpdateProjectViews(Loader);

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
            gui->RaiseVisualEditDockWindow();
        }
        else
            delete Loader;
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
    if (Preferences::darkTheme)
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
  QRegExp fadeRx = QRegExp("FADE_STEPS LPUB_FADE\\s*(GLOBAL)?\\s*TRUE");
  bool nativeRenderer = Preferences::preferredRenderer == RENDERER_NATIVE;
  bool lpubFade = Gui::stepContains(topLevelModel,fadeRx,result,1);
  if (lpubFade) {
    if (result != QLatin1String("GLOBAL"))
      emit lpub->messageSig(LOG_WARNING,tr("Top level LPUB_FADE command missing GLOBAL qualifier."),LOG_SHOW_DIALOG);
    if (!nativeRenderer) {
      emit lpub->messageSig(LOG_WARNING,tr("LPUB_FADE command IGNORED.<br>"
                                           "LPUB_FADE command requires preferred render RENDERER_NATIVE.<br>"
                                           "PREFERRED_RENDERER command must precede LPUB_FADE."),LOG_SHOW_DIALOG);
      lpubFade = false;
    }
  }

  bool configureFadeSteps = lpubFade || (!lpubFade && !nativeRenderer);

  if (!Preferences::enableFadeSteps) {
    result.clear();
    fadeRx.setPattern("FADE_STEPS ENABLED\\s*(GLOBAL)?\\s*TRUE");
    Preferences::enableFadeSteps = Gui::stepContains(topLevelModel,fadeRx,result,1);
    if (Preferences::enableFadeSteps) {
      if (result != QLatin1String("GLOBAL")) {
        emit lpub->messageSig(LOG_WARNING,tr("Top level FADE_STEPS set to ENABLED but missing GLOBAL qualifier."),LOG_SHOW_DIALOG);
      } else if (configureFadeSteps && !Preferences::finalModelEnabled) {
        result.clear();
        topLevelModel = top;
        fadeRx.setPattern("FINAL_MODEL_ENABLED\\s*(GLOBAL)?\\s*(?:TRUE|FALSE)");
        Preferences::finalModelEnabled = Gui::stepContains(topLevelModel,fadeRx,result,1);
        if (Preferences::finalModelEnabled) {
          emit lpub->messageSig(LOG_INFO, tr("Display Final Model is Enabled"));
        }
        if (result != QLatin1String("GLOBAL")) {
          emit lpub->messageSig(LOG_WARNING,tr("FINAL_MODEL_ENABLED command should have GLOBAL qualifier."),LOG_SHOW_DIALOG);
        }
      }
    }
  }

  result.clear();
  topLevelModel = top;
  bool setupFadeSteps = false;
  if (configureFadeSteps) {
    fadeRx.setPattern("FADE_STEPS SETUP\\s*(GLOBAL)?\\s*TRUE");
    setupFadeSteps = Gui::stepContains(topLevelModel,fadeRx,result,1);
    if (setupFadeSteps && result != QLatin1String("GLOBAL"))
      emit lpub->messageSig(LOG_WARNING,tr("Top level FADE_STEPS SETUP command missing GLOBAL qualifier."),LOG_SHOW_DIALOG);
    if (setupFadeSteps && Preferences::enableFadeSteps) {
      emit lpub->messageSig(LOG_WARNING,tr("Command FADE_STEPS SETUP %1 encountered but<br>FADE_STEPS %1 is ENABLED.<br><br>"
                                           "Command FADE_STEPS SETUP %1 will be IGNORED.").arg(result),LOG_SHOW_DIALOG);
      setupFadeSteps = false;

    }
  }

  if (!Preferences::enableFadeSteps && !setupFadeSteps)
    return false;

  if (Preferences::enableFadeSteps != Preferences::initEnableFadeSteps)
    emit lpub->messageSig(LOG_INFO_STATUS,tr("Fade Steps is %1 - Set from meta command.")
                                            .arg(Preferences::enableFadeSteps ? "ON" : "OFF"));
  if (setupFadeSteps)
    emit lpub->messageSig(LOG_INFO_STATUS,tr("Fade Steps Setup is ENABLED."));

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
  fadeRx.setPattern("FADE_STEPS COLOR\\s*(?:GLOBAL)?\\s*(\")?(\\w+)(?(1)\1|)[^\n]*");
  Gui::stepContains(topLevelModel,fadeRx,result,2);
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

  return true /*setupFadeSteps || Preferences::enableFadeSteps*/;
}

bool LPub::setHighlightStepFromCommand()
{
  QString result;
  Where top(ldrawFile.topLevelFile(),0,0);
  Where topLevelModel(top);
  QRegExp highlightRx = QRegExp("HIGHLIGHT_STEP LPUB_HIGHLIGHT\\s*(GLOBAL)?\\s*TRUE");
  bool nativeRenderer = Preferences::preferredRenderer == RENDERER_NATIVE;
  bool lpubHighlight = Gui::stepContains(topLevelModel,highlightRx,result,1);
  if (lpubHighlight) {
    if (result != QLatin1String("GLOBAL"))
      emit lpub->messageSig(LOG_WARNING,tr("Top level LPUB_FADE command missing GLOBAL qualifier."),LOG_SHOW_DIALOG);
    if (!nativeRenderer) {
      emit lpub->messageSig(LOG_WARNING,tr("LPUB_HIGHLIGHT command IGNORED.<br>"
                                           "LPUB_HIGHLIGHT command requires preferred render RENDERER_NATIVE.<br>"
                                           "PREFERRED_RENDERER command must precede LPUB_HIGHLIGHT."),LOG_SHOW_DIALOG);
      lpubHighlight = false;
    }
  }

  bool configureHighlightStep = lpubHighlight || (!lpubHighlight && !nativeRenderer);

  if (!Preferences::enableHighlightStep) {
    result.clear();
    highlightRx.setPattern("HIGHLIGHT_STEP ENABLED\\s*(GLOBAL)?\\s*TRUE");
    Preferences::enableHighlightStep = Gui::stepContains(topLevelModel,highlightRx,result,1);
    if (Preferences::enableHighlightStep) {
      if (result != QLatin1String("GLOBAL")) {
        emit lpub->messageSig(LOG_WARNING,tr("Top level HIGHLIGHT_STEP set to ENABLED but missing GLOBAL qualifier."),LOG_SHOW_DIALOG);
      } else if (configureHighlightStep && !Preferences::finalModelEnabled) {
        result.clear();
        topLevelModel = top;
        highlightRx.setPattern("FINAL_MODEL_ENABLED\\s*(GLOBAL)?\\s*(?:TRUE|FALSE)");
        Preferences::finalModelEnabled = Gui::stepContains(topLevelModel,highlightRx,result,1);
        if (Preferences::finalModelEnabled) {
          emit lpub->messageSig(LOG_INFO,tr("Display Final Model is Enabled"));
        }
        if (result != QLatin1String("GLOBAL")) {
          emit lpub->messageSig(LOG_WARNING,tr("FINAL_MODEL_ENABLED command should have GLOBAL qualifier."),LOG_SHOW_DIALOG);
        }
      }
    }
  }

  result.clear();
  topLevelModel = top;
  bool setupHighlightStep = false;
  if (configureHighlightStep) {
    highlightRx.setPattern("HIGHLIGHT_STEP SETUP\\s*(GLOBAL)?\\s*TRUE");
    setupHighlightStep = Gui::stepContains(topLevelModel,highlightRx,result,1);
    if (setupHighlightStep && result != QLatin1String("GLOBAL")) {
      emit lpub->messageSig(LOG_WARNING,tr("Top level HIGHLIGHT_STEP SETUP command missing GLOBAL qualifier."),LOG_SHOW_DIALOG);
    }
    if (setupHighlightStep) {
      if (Preferences::enableHighlightStep) {
        emit lpub->messageSig(LOG_WARNING,tr("Command HIGHLIGHT_STEP SETUP %1 encountered but<br>HIGHLIGHT_STEP %1 is ENABLED.<br><br>"
                                             "Command HIGHLIGHT_STEP SETUP %1 will be IGNORED.").arg(result),LOG_SHOW_DIALOG);
        setupHighlightStep = false;
      }
    }
  }

  if (!Preferences::enableHighlightStep && !setupHighlightStep)
    return false;

  if (Preferences::enableHighlightStep != Preferences::initEnableHighlightStep)
    emit lpub->messageSig(LOG_INFO_STATUS,tr("Highlight Step is %1 - Set from meta command.")
                                             .arg(Preferences::enableHighlightStep ? "ON" : "OFF"));
  if (setupHighlightStep)
    emit lpub->messageSig(LOG_INFO_STATUS,tr("Highlight Step Setup is ENABLED."));

  result.clear();
  topLevelModel = top;
  highlightRx.setPattern("HIGHLIGHT_STEP COLOR\\s*(?:GLOBAL)?\\s*(\")?(0x|#)([\\da-fA-F]+)(?(1)\1|)[^\n]*");
  if (Gui::stepContains(topLevelModel,highlightRx,result)) {
    result = QString("%1%2").arg(highlightRx.cap(1),highlightRx.cap(3));
    QColor ParsedColor = QColor(result);
    QString highlightStepColourCompare = Preferences::highlightStepColour;
    Preferences::highlightStepColour = ParsedColor.isValid() ? result : Preferences::validFadeStepsColour;
    bool highlightStepColorChanged = QString(Preferences::highlightStepColour).toLower() != highlightStepColourCompare.toLower();
    if (highlightStepColorChanged)
      emit lpub->messageSig(LOG_INFO,tr("Highlight Step Color preference changed from %1 to %2 - Set from meta command")
                                        .arg(highlightStepColourCompare)
                                        .arg(Preferences::highlightStepColour));
  }

  return true /*setupHighlightStep || Preferences::enableHighlightStep*/;
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
    if (Preferences::enableLDViewSnaphsotList != useLDVSnapShotList) {
      message = tr("Renderer preference use LDView Snapshot List changed from %1 to %2.")
                    .arg(Preferences::enableLDViewSnaphsotList ? "Yes" : "No")
                    .arg(useLDVSnapShotList ? "Yes" : "No");
      emit lpub->messageSig(LOG_INFO,message);
      Preferences::enableLDViewSnaphsotList = useLDVSnapShotList;
      if (useLDVSnapShotList)
        useLDVSingleCall = true;
      renderFlagChanged = true;
    }
    if (Preferences::enableLDViewSingleCall != useLDVSingleCall) {
      message = tr("Renderer preference use LDView Single Call changed from %1 to %2.")
                        .arg(Preferences::enableLDViewSingleCall ? "Yes" : "No")
                        .arg(useLDVSingleCall ? "Yes" : "No");
      emit lpub->messageSig(LOG_INFO,message);
      Preferences::enableLDViewSingleCall = useLDVSingleCall;
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
                                                         useLDVSnapShotList ? tr(" (Single Call using Snapshot File List)") :
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
int LPub::pageSize(PageMeta &meta, int which) {
  int _which;
  if (meta.orientation.value() == Landscape) {
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
      if (lpub->page.displayPage >= DT_MODEL_DEFAULT || lpub->page.subModel.viewerSubmodel) {
          stepNumber = QStringList(keyArgs[2].split("_")).first().toInt(&ok[1]);
      } else {
          stepNumber = keyArgs[2].toInt(&ok[1]);
      }
      if (!ok[1]) {
          emit lpub->messageSig(LOG_WARNING,tr("Step number is not an integer [%1]. Using 0").arg(keyArgs[2]), LOG_SHOW_DIALOG);
          stepNumber = 0;
      }
/*
#ifdef QT_DEBUG_MODE
          QString messsage = QString("Step Key parse OK, modelName: %1, lineNumber: %2, stepNumber: %3")
                                     .arg(modelName).arg(lineNumber).arg(stepNumber);
          if (!stepNumber && page.pli.tsize() && page.pli.bom)
              messsage = QString("Step Key parse OK but this is a BOM page, step pageNumber: %1")
                                 .arg(Gui::stepPageNum);
          emit lpub->messageSig(LOG_DEBUG, messsage);
      }
#endif
//*/
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

    emit lpub->messageSig(LOG_WARNING,message);

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
                    .arg(LPub::elapsedTime(timer.elapsed())));
}

/****************************************************************************
 *
 * Command collection load
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
            command = preamble + " <\"font attributes...\">";
        else
        if (preamble.endsWith(" FONT_COLOR"))
            command = preamble + " <\"color name | #RRGGBB\">";
        else
        if (preamble.endsWith(" PRIMARY") || preamble.endsWith(" SECONDARY") || preamble.endsWith(" TERTIARY"))
            command = preamble + " <\"Part Color\" | \"Part Category\" | \"Part Size\" | \"Part Element\" | \"No Sort\">";
        else
        if (preamble.endsWith(" PRIMARY_DIRECTION") || preamble.endsWith(" SECONDARY_DIRECTION") || preamble.endsWith(" TERTIARY_DIRECTION"))
            command = preamble + " <\"Ascending\" | \"Descending\">";
        else
        if (preamble.endsWith(" SUBMODEL_BACKGROUND_COLOR") || preamble.endsWith(" SUBMODEL_FONT_COLOR"))
            command = preamble + " <\"color name | #RRGGBB\"> [<\"color name | #RRGGBB\"> <\"color name | #RRGGBB\"> <\"color name | #RRGGBB\">]";
        else
        if (preamble.endsWith(" SUBMODEL_FONT"))
            command = preamble + " <\"font attributes...\"> [<\"font attributes...\"> <\"font attributes...\"> <\"font attributes...\">]";
        else
        if (preamble.endsWith(" COLOR") && !preamble.contains(" FADE_STEPS "))
            command = preamble + " <\"color name|#RRGGBB\">";
        else
        if (preamble.endsWith(" FILE"))
            command = preamble + " <\"file path\">";
        else
        if (preamble.endsWith(" LIGHT AREA_SHAPE"))
            command = preamble + " <\"UNDEFINED\" | \"SQUARE\" | \"DISK\" | \"RECTANGLE\" | \"ELLIPSE\">";
        else
        if (preamble.endsWith(" LIGHT TYPE"))
            command = preamble + " <\"POINT\" | \"SUN\" | \"SPOT\" | \"AREA\">";
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
                    .arg(LPub::elapsedTime(timer.elapsed())));
}

/****************************************************************************
 *
 * Export LPUB commands
 *
 ***************************************************************************/

bool LPub::exportMetaCommands(const QString &fileName, QString &result, bool plainText, bool descriptons)
{
    if (!commandCollection) {
        result = tr("The command collection is null.");
        emit messageSig(LOG_ERROR, result);
        return false;
    }

    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        emit messageSig(LOG_ERROR, tr("Cannot write file %1:<br>%2.")
                        .arg(fileName)
                        .arg(file.errorString()));
        return false;
    }

    int n = 0;
    enum { FirstRec };
    QStringList doc, rec;
    for (int i = 0; i < commandCollection->count(); ++i) {
        Command command = commandCollection->at(i);
        QString const cleanCommand = command.command;
        rec = cleanCommand.split("\n");
        for (int g = 0; g < rec.size(); g++) {
            if (g == FirstRec) {
                if (plainText)
                    doc.append(QString("%1. %2").arg(++n,3,10,QChar('0')).arg(rec[g]));
                else {
                    bool singleRec = rec.size() == 1;
                    doc.append(QString("<li><code>%1%2").arg(rec[g]).arg(singleRec ? "</code></li>" : "<br>"));
                    ++n;
                }
            } else {
                if (plainText)
                    doc.append(QString("     %1").arg(rec[g]));
                else {
                    bool lastRec = g+1 == rec.size() && !descriptons;
                    doc.append(QString("          %1%2").arg(rec[g]).arg(lastRec ? "</code></li>" : "<br>"));
                }
            }
        }
        if (descriptons && command.modified == Command::True) {
            QString const cleanDescription = command.description;
            rec = cleanDescription.split("\n");
            doc.append(QString("%1").arg(plainText ? "     0 // DESCRIPTION:" : "          0 // DESCRIPTION:<br>"));
            for (int h = 0; h < rec.size(); h++) {
                if (plainText)
                    doc.append(QString("     %1").arg(rec[h]));
                else {
                    bool lastRec = h+1 == rec.size();
                    doc.append(QString("          %1%2").arg(rec[h]).arg(lastRec ? "</code></li>" : "<br>"));
                }
            }
        }
    }

    static const QLatin1String fmtDateTime("yyyy-MM-dd hh:mm:ss");

    static const QLatin1String documentTitle("LPUB Meta Commands");

    const QString generated = tr("%1 %2 - Generated on %3")
            .arg(VER_PRODUCTNAME_STR)
            .arg(documentTitle)
            .arg(QDateTime::currentDateTime().toString(fmtDateTime));

    if (plainText) {
        // header - lines are from bottom to top
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
        doc.prepend(tr("  LPUB UDL: %1assets/resources/%2.xml.zip").arg(VER_HOMEPAGE_GITHUB_STR).arg(VER_PRODUCTNAME_STR));
        doc.prepend(tr("  selected to enable LPUB syntax highlighting."));
        doc.prepend(tr("  Install %1_Npp_UDL.xml and open this file in Notepad++ with '%1' UDL").arg(VER_PRODUCTNAME_STR));
        doc.prepend(tr("  available in the 'extras' folder or at the %1 homepage.").arg(VER_PRODUCTNAME_STR));
        doc.prepend(tr("  %1 has an LPUB User Defined Language (UDL) configuration file for Notepad++").arg(VER_PRODUCTNAME_STR));
        doc.prepend(tr("  Best viewed on Windows with Notepad++ <https://notepad-plus-plus.org>."));
        doc.prepend(QString());
#endif
        doc.prepend(tr("---------------------------------------------------------------------------------"));
        doc.prepend(QString());
        doc.prepend(tr("  Copyright © 2016 - %1 by %2").arg(QDate::currentDate().year()).arg(VER_PUBLISHER_STR));
        doc.prepend(tr("  License.....: GPLv3 - see %1").arg(VER_LICENSE_INFO_STR));
        doc.prepend(tr("  Homepage....: %1").arg(VER_HOMEPAGE_GITHUB_STR));
        doc.prepend(tr("  Last Update.: %1").arg(VER_COMPILE_DATE_STR));
        doc.prepend(tr("  Version.....: %1.%2").arg(VER_PRODUCTVERSION_STR).arg(VER_COMMIT_STR));
        doc.prepend(tr("  Author......: %1").arg(VER_PUBLISHER_STR));
        doc.prepend(tr("  Name........: %1, LPUB Meta Commands").arg(VER_FILEDESCRIPTION_STR));
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
        doc.append(tr("-    The <\"font attributes...\"> meta value is a comma-delimited <\"string\"> of 10 attributes."));
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
        doc.append(tr("     Example font attributes <\" Arial, 64, -1, 255, 75, 0, 0, 0, 0, 0 \">"));
        doc.append(QString());
        doc.append(tr("End of file."));
    } else {
        // header - lines are from bottom to top
        doc.prepend("<ol id=\"commandOL\">");
        doc.prepend("</div>");
        doc.prepend("<i class=\"fa fa-filter fa-lg\"></i>");
        doc.prepend(QString("<input type=\"text\" id=\"commandInput\" onkeyup=\"commandFunction()\" placeholder=\"%1\" title=\"%2\">").arg("Meta command filter").arg(tr("Type command key words to filter list")));
        doc.prepend(tr("<label><b>Meta Commands:</b></label>"));
        doc.prepend("<div class=\"commandInputDiv\">");
        doc.prepend(tr("  <code>|</code> Items bisected by a pipe (or) indicate multiple options are available; however, only one option per command can be specified.</li></p>"));
        doc.prepend(tr("  <code>\" \"</code> Items within double quotes are <code>\"string\"</code> values. Strings containing space require quotes. Numeric values are not quoted.<br>"));
        doc.prepend(tr("  <code>[ ]</code> Items within square brackets indicate optional meta command(s) and or value(s).<br>"));
        doc.prepend(tr("  <code>< ></code> Items within chevron (greater,less than) indicate meta command value options required to complete the command.<br>"));
        doc.prepend(tr("<li><code>( )</code> Items within curly brackets (parentheses) are built-in meta command options.<br>"));
        doc.prepend(tr("<p><b>Meta Command Symbols:</b><br>"));
#ifdef Q_OS_WIN
        doc.prepend("<p>---------------------------------------------------------------------------------</p>");
        doc.prepend(tr("  <i class=\"fa fa-download fa-lg\"></i> %1_LPub_Meta_Commands.txt.zip</button></p>").arg(VER_PRODUCTNAME_STR));
        doc.prepend(tr("  <button onclick=\"location.href='%1assets/resources/%2_LPub_Meta_Commands.txt.zip'\" title=\"%1assets/resources/%2_LPub_Meta_Commands.txt.zip\">").arg(VER_HOMEPAGE_GITHUB_STR).arg(VER_PRODUCTNAME_STR));
        doc.prepend(tr("  <i class=\"fa fa-download fa-lg\"></i> %1_Npp_UDL.xml.zip </button>").arg(VER_PRODUCTNAME_STR));
        doc.prepend(tr("  <button onclick=\"location.href='%1assets/resources/%2_Npp_UDL.xml.zip'\" title=\"%1assets/resources/%2_Npp_UDL.xml.zip\">").arg(VER_HOMEPAGE_GITHUB_STR).arg(VER_PRODUCTNAME_STR));
        doc.prepend(tr("  selected to enable LPUB syntax highlighting.<br>"));
        doc.prepend(tr("  Install %1_Npp_UDL.xml and open the .txt instance of this file in Notepad++ with '%1' UDL").arg(VER_PRODUCTNAME_STR));
        doc.prepend(tr("  available in the 'extras' folder or at the <a href=\"%1\" target=\"_blank\" rel=\"noopener noreferrer\">%2 homepage</a>.<br>").arg(VER_HOMEPAGE_GITHUB_STR).arg(VER_PRODUCTNAME_STR));
        doc.prepend(tr("  %1 has an LPUB User Defined Language (UDL) configuration file for Notepad++").arg(VER_PRODUCTNAME_STR));
        doc.prepend(tr("  The text instance of this file is best viewed on Windows with Notepad++ <a href=\"%1\" target=\"_blank\" rel=\"noopener noreferrer\">%1</a>.<br>").arg("https://notepad-plus-plus.org"));
        doc.prepend("<p>");
#endif
        doc.prepend("<p>---------------------------------------------------------------------------------</p>");
        doc.prepend(tr("  Copyright &copy; 2016 - %1 by %2</p>").arg(QDate::currentDate().year()).arg(VER_PUBLISHER_STR));
        doc.prepend(tr("  License.....: GPLv3 - see <a href=\"%1\" target=\"_blank\" rel=\"noopener noreferrer\">%1</a><br>").arg(VER_LICENSE_INFO_STR));
        doc.prepend(tr("  Homepage....: <a href=\"%1\" target=\"_blank\" rel=\"noopener noreferrer\">%1</a><br>").arg(VER_HOMEPAGE_GITHUB_STR));
        doc.prepend(tr("  Last Update.: %1<br>").arg(VER_COMPILE_DATE_STR));
        doc.prepend(tr("  Version.....: %1.%2<br>").arg(VER_PRODUCTVERSION_STR).arg(VER_COMMIT_STR));
        doc.prepend(tr("  Author......: %1<br>").arg(VER_PUBLISHER_STR));
        doc.prepend(tr("  Name........: %1, LPUB Meta Commands<br>").arg(VER_FILEDESCRIPTION_STR));
        doc.prepend("<p>");
        doc.prepend("</div>");
        doc.prepend("<p>---------------------------------------------------------------------------------</p>");
        doc.prepend(QString("<h2>%1</h2><br>").arg(generated));
        doc.prepend("<img alt=\"LPub3D Logo\" src=\"data:image/png;base64,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\" />");
        doc.prepend("<div class=\"header\">");
        doc.prepend("<ul id=\"commandUL\">");
        doc.prepend("<body>");
        doc.prepend("</head>");
        doc.prepend("</style>");
        doc.prepend("#commandUL li code { font-size: 15px; color: #c7254e; }");
        doc.prepend("#commandOL li code:hover:not(.header) { background-color: #eee; }");
        doc.prepend("#commandOL li code { border: 1px solid #ddd; margin-top: -1px; /* Prevent double borders */ background-color: #f9f2f4; padding: 8px; text-decoration: none; font-size: 15px; color: #c7254e; display: block }");
        doc.prepend("#commandOL { padding: 0;margin: 0; }");
        doc.prepend("#commandInput { width: 100%; padding: 12px 40px; margin: 8px 0; display: inline-block; border: 1px solid #ccc; box-sizing: border-box; }");
        doc.prepend(".commandInputDiv i { position: absolute; left: 15px; top: 40px; color: gray; }");
        doc.prepend(".commandInputDiv { position: relative; }");
        doc.prepend("button:hover { opacity: 0.7; }");
        doc.prepend("button { background-color: #c7254e; border: none; color: white; padding: 12px 30px; cursor: pointer; font-size: 15px; }");
        doc.prepend("p { text-align: left; }");
        doc.prepend("body { font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; }");
        doc.prepend(".header h2 { font-weight: 100; position: relative; top: 18px; left: 10px; }");
        doc.prepend(".header img { float: left; width: 64px; height: 64px; }");
        doc.prepend("@charset \"UTF-8\";");
        doc.prepend("<style>");
        doc.prepend("<link rel=\"icon\" type=\"image/x-icon\" href=\"data:image/png;base64,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\">");
        doc.prepend("<link rel=\"stylesheet\" href=\"https://cdnjs.cloudflare.com/ajax/libs/font-awesome/4.7.0/css/font-awesome.min.css\">");
        doc.prepend(tr("<title>%1 LPUB Metacommands</title>").arg(VER_PRODUCTNAME_STR));
        doc.prepend("<meta charset=\"utf-8\">");
        doc.prepend("<head>");
        doc.prepend("<html>");
        doc.prepend("<!DOCTYPE html>");

        // footer
        doc.append("</ol>");
        doc.append(tr("<p><b>Meta Command Notes:</b><br>"));
        doc.append(tr("<li>The <code><\"page size id\"></code> meta value captures paper size, e.g. <code>A4, B4</code>, <code>Letter</code>, <code>Custom</code>, etc...<br>"));
        doc.append(tr(" For custom page size use <code><decimal width> <decimal height> \"Custom\"</code>.</li><br>"));
        doc.append(tr("<li>These <code>SUBMODEL</code> metas enable font and background settings for nested submodels and callouts.<br>"));
        doc.append(tr(" - <code>SUBMODEL_FONT</code> meta is supported for up to four levels.<br>"));
        doc.append(tr(" - <code>SUBMODEL_FONT_COLOR</code> meta is supported for up to four levels.<br>"));
        doc.append(tr(" - <code>SUBMODEL_BACKGROUND_COLOR</code> meta is supported for up to four levels.<br>"));
        doc.append(tr(" There are 4 predefined submodel level colors <span style=\"background-color:#FFFFFF\"><code>#FFFFFF</code></span>, <span style=\"background-color:#FFFFCC\"><code>#FFFFCC</code></span>, <span style=\"background-color:#FFCCCC\"><code>#FFCCCC</code></span>, and <span style=\"background-color:#CCCCFF\"><code>#CCCCFF</code></span>.</li><br>"));
        doc.append(tr("<li>The <code>&lt;stud style integer 0-7&gt;</code> meta value captures the 7 stud style types.<br>"));
        doc.append("<code>");
        doc.append(tr(" 0 None<br>"));
        doc.append(tr(" 1 Thin line logo<br>"));
        doc.append(tr(" 2 Outline logo<br>"));
        doc.append(tr(" 3 Sharp top logo<br>"));
        doc.append(tr(" 4 Rounded top logo<br>"));
        doc.append(tr(" 5 Flattened logo<br>"));
        doc.append(tr(" 6 High contrast without logo<br>"));
        doc.append(tr(" 7 High contrast with logo</code></li><br>"));
        doc.append(tr("<li>The <code>&lt;annotation style integer 0-4&gt;</code> meta value captures the 4 annotation icon style types.<br>"));
        doc.append("<code>");
        doc.append(tr(" 0 None<br>"));
        doc.append(tr(" 1 Circle<br>"));
        doc.append(tr(" 2 Square<br>"));
        doc.append(tr(" 3 Rectangle<br>"));
        doc.append(tr(" 4 LEGO element</code></li></br>"));
        doc.append(tr("<li>The <code>&lt;line integer 0-5&gt;</code> meta value captures the 5 border line types.<br>"));
        doc.append("<code>");
        doc.append(tr(" 0 None<br>"));
        doc.append(tr(" 1 Solid        ----<br>"));
        doc.append(tr(" 2 Dash         - - <br>"));
        doc.append(tr(" 3 Dot          ....<br>"));
        doc.append(tr(" 4 Dash dot     -.-.<br>"));
        doc.append(tr(" 5 Dash dot dot -..-</code></li></br>"));
        doc.append(tr("<li>The <code><\"font attributes...\"></code> meta value is a comma-delimited <code><\"string\"></code> of 10 attributes.<br>"));
        doc.append(tr("<code>1  FamilyName</code> - \"Arial\"<br>"));
        doc.append(tr("<code>2  PointSizeF</code> - <code>64</code> size of font, <code>-1</code> if using PixelSize<br>"));
        doc.append(tr("<code>3  PixelSize</code>  - <code>-1</code> size of font, <code>-1</code> if using PointSizeF<br>"));
        doc.append(tr("<code>4  StyleHint</code>  - <code>255</code> = no style hint set, <code>5</code> = any style, <code>4</code> = system font, <code>0</code> = Helvetica, etc...<br>"));
        doc.append(tr("<code>5  Weight</code>     - <code>75</code> = bold, <code>50</code> = normal, etc...<br>"));
        doc.append(tr("<code>6  Underline</code>  - <code>0</code> = disabled, <code>1</code> = enabled<br>"));
        doc.append(tr("<code>7  Strikeout</code>  - <code>0</code> = disabled, <code>1</code> = enabled<br>"));
        doc.append(tr("<code>8  StrikeOut</code>  - <code>0</code> = disabled, <code>1</code> = enabled<br>"));
        doc.append(tr("<code>9  FixedPitch</code> - <code>0</code> = disabled, <code>1</code> = enabled<br>"));
        doc.append(tr("<code>10 RawMode</code>    - <code>0</code> obsolete, use default value<br>"));
        doc.append(tr(" Example font attributes <code><\" Arial, 64, -1, 255, 75, 0, 0, 0, 0, 0 \"></code></li>"));
        doc.append("</p>");
        doc.append("</ul>");
        doc.append(tr("<p>End of file.</p>"));
        doc.append("<script>");
        doc.append("function commandFunction() {");
        doc.append("    var input, filter, ol, li, code, i, txtValue;");
        doc.append("    input = document.getElementById(\"commandInput\");");
        doc.append("    filter = input.value.toUpperCase();");
        doc.append("    ol = document.getElementById(\"commandOL\");");
        doc.append("    li = ol.getElementsByTagName(\"li\");");
        doc.append("    for (i = 0; i < li.length; i++) {");
        doc.append("        code = li[i].getElementsByTagName(\"code\")[0];");
        doc.append("        txtValue = code.textContent || code.innerText;");
        doc.append("        if (txtValue.toUpperCase().indexOf(filter) > -1) {");
        doc.append("            li[i].style.display = \"\";");
        doc.append("        } else {");
        doc.append("            li[i].style.display = \"none\";");
        doc.append("        }");
        doc.append("    }");
        doc.append("}");
        doc.append("</script>");
        doc.append("</body>");
        doc.append("</html>");
    }

    // export content list
    QTextStream out(&file);
    for (int i = 0; i < doc.size(); i++) {
        out << doc[i] << lpub_endl;
    }

    file.close();

    const QString exported = QString("%1 of %2")
            .arg(n)
            .arg(commandCollection->count());
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
  box.setIconPixmap (QPixmap(LPUB3D_MESSAGE_ICON));
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
        if (Preferences::modeGUI) {
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

QString LPub::elapsedTime(const qint64 &duration, bool pretty)
{
    qint64 elapsed = duration;
    int milliseconds = int(elapsed % 1000);
    elapsed /= 1000;
    int seconds = int(elapsed % 60);
    elapsed /= 60;
    int minutes = int(elapsed % 60);
    elapsed /= 60;
    int hours = int(elapsed % 24);

    return tr("%1%2%3%4")
              .arg(pretty        ?
                             tr("Elapsed time: ") : QString())
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

bool LPub::CurrentStepIsDisplayModel()
{
    if (currentStep)
        return currentStep->displayStep;
    return false;
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
void LPub::clearAndReloadModelFile(bool fileReload, bool savePrompt, bool keepWork)
{
    emit gui->clearAndReloadModelFileSig(fileReload, savePrompt, keepWork);
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
                                        .arg(LPub::elapsedTime(timer.elapsed())));
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
#ifdef QT_DEBUG_MODE
    emit gui->messageSig(LOG_ERROR, QString("Action was not found or is null [%1]").arg(objectName));
#endif
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
    LPub::m_updaterCancelled = false;
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
  LPub::m_updaterCancelled = true;
}

void LPub::updateChangelog (const QString &url)
{
    auto processRequest = [&] ()
    {
        if (m_updater->getUpdateAvailable(url) || m_updater->getChangelogOnly(url)) {
            if (!LPub::m_updaterCancelled) {
                int REV = m_updater->getLatestRevision(LPub::DEFS_URL).toInt(); Q_UNUSED(REV)
#if defined LP3D_CONTINUOUS_BUILD || defined LP3D_DEVOPS_BUILD || defined LP3D_NEXT_BUILD
#ifdef QT_DEBUG_MODE
                LPub::m_versionInfo = tr("Change Log for Version %1%2 (%3)")
                                 .arg(qApp->applicationVersion(), QString::fromLatin1(VER_REVISION_STR).toInt() ? tr(" Revision %1").arg(QString::fromLatin1(VER_REVISION_STR)) : QString(), QString::fromLatin1(VER_BUILD_TYPE_STR));
#else
                LPub::LPub::m_versionInfo = tr("Change Log for Version %1%2 (%3)")
                                 .arg(m_updater->getLatestVersion(url), REV ? QString(" Revision %1").arg(VER_REVISION_STR) : QString(), QString::fromLatin1(VER_BUILD_TYPE_STR));
#endif
#else
                LPub::m_versionInfo = tr("Change Log for Version %1%2")
                                 .arg(m_updater->getLatestVersion(url), REV ? QString(" Revision %1").arg(VER_REVISION_STR) : QString());
#endif
                LPub::m_setReleaseNotesAsText = m_updater->compareVersionStr(url, m_updater->getLatestVersion(url), PLAINTEXT_CHANGE_LOG_CUTOFF_VERSION);
                LPub::m_releaseNotesContent = m_updater->getChangelog(url);
            }
        }
        emit checkForUpdatesFinished();
    };

    if (url == LPub::DEFS_URL)
        processRequest();
    else {

#if defined LP3D_CONTINUOUS_BUILD || defined LP3D_DEVOPS_BUILD || defined LP3D_NEXT_BUILD
        LPub::m_versionInfo = tr("Change Log for Version %1%2 (%3)")
                           .arg(qApp->applicationVersion(), QString::fromLatin1(VER_REVISION_STR).toInt() ? tr(" Revision %1").arg(QString::fromLatin1(VER_REVISION_STR)) : QString(), QString::fromLatin1(VER_BUILD_TYPE_STR));
#else
        int REV = m_updater->getLatestRevision(LPub::DEFS_URL).toInt();
        LPub::m_versionInfo = tr("Change Log for Version %1%2")
                           .arg(m_updater->getLatestVersion(url), REV ? QString(" Revision %1").arg(VER_REVISION_STR) : QString());
#endif
        //populate releaseNotes
        QString releaseNotesFile = QString("%1/%2/RELEASE_NOTES.html").arg(Preferences::lpub3dPath).arg(Preferences::lpub3dDocsResourcePath);

        QFile file(releaseNotesFile);
        if (! file.open(QFile::ReadOnly | QFile::Text)) {
            LPub::m_setReleaseNotesAsText = true;
            LPub::m_releaseNotesContent = tr("Failed to open Release Notes file: \n%1:\n%2")
                                             .arg(releaseNotesFile)
                                             .arg(file.errorString());
        } else {
            LPub::m_releaseNotesContent = QString::fromStdString(file.readAll().toStdString());
        }
    }

    if (LPub::m_versionInfo.isEmpty())
        LPub::m_versionInfo = tr("Undefined");
}

void LPub::saveVisualEditorTransformSettings()
{
    mRotateTransform   = gui->VisualEditorRotateTransform();
    mRelativeTransform = gMainWindow->GetRelativeTransform();
    mSeparateTransform = gMainWindow->GetSeparateTransform();
}

void LPub::raiseVisualEditDockWindow()
{
    gui->RaiseVisualEditDockWindow();
}

void LPub::raisePreviewDockWindow()
{
    gui->RaisePreviewDockWindow();
}


