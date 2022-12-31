/****************************************************************************
**
** Copyright (C) 2007-2009 Kevin Clague. All rights reserved.
** Copyright (C) 2015 - 2022 Trevor SANDY. All rights reserved.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://www.trolltech.com/products/qt/opensource.html
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QDesktopServices>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>

#include <QItemEditorFactory>
#include <QKeySequence>
#include <QKeySequenceEdit>
#include <QStyledItemDelegate>
#include <QTableWidgetItem>

#include "ui_preferences.h"
#include "preferencesdialog.h"
#include "qsimpleupdater.h"
#include "messageboxresizable.h"
#include "lpub_preferences.h"
#include "application.h"
#include "updatecheck.h"
#include "commonmenus.h"

#include <LDVQt/LDVWidget.h>

#include "color.h"
#include "meta.h"
#include "lpub.h"
#include "version.h"
#include "lc_profile.h"
#include "lc_application.h"
#include "lc_qutils.h"

// Resize all columns to content except for one stretching column. (adapted from LeoCAD implementation)
CommandListColumnStretcher::CommandListColumnStretcher(QTreeWidget *treeWidget, int columnToStretch)
    : QObject(treeWidget->header()), m_columnToStretch(columnToStretch),m_interactiveResize(false), m_stretchWidth(0)
{
    parent()->installEventFilter(this);
    connect(treeWidget->header(), SIGNAL(sectionResized(int, int, int)), SLOT(sectionResized(int, int, int)));
    QHideEvent stretch;
    CommandListColumnStretcher::eventFilter(parent(), &stretch);
}

void CommandListColumnStretcher::sectionResized(int LogicalIndex, int OldSize, int NewSize)
{
    Q_UNUSED(OldSize)

    if (LogicalIndex == m_columnToStretch)
    {
        QHeaderView* HeaderView = qobject_cast<QHeaderView*>(parent());

        if (HeaderView->isVisible())
            m_interactiveResize = true;

        m_stretchWidth = NewSize;
    }
}

bool CommandListColumnStretcher::eventFilter(QObject* Object, QEvent* Event)
{
    if (Object == parent())
    {
        QHeaderView* HeaderView = qobject_cast<QHeaderView*>(Object);

        if (Event->type() == QEvent::Show)
        {
            for (int i = 0; i < HeaderView->count(); ++i)
                HeaderView->setSectionResizeMode(i, QHeaderView::Interactive);

            m_stretchWidth = HeaderView->sectionSize(m_columnToStretch);

        }
        else if (Event->type() == QEvent::Hide)
        {
            if (!m_interactiveResize)
                for (int i = 0; i < HeaderView->count(); ++i)
                    HeaderView->setSectionResizeMode(i, i == m_columnToStretch ? QHeaderView::Stretch : QHeaderView::ResizeToContents);
        }
        else if (Event->type() == QEvent::Resize)
        {
            if (HeaderView->sectionResizeMode(m_columnToStretch) == QHeaderView::Interactive) {

                const int StretchWidth = HeaderView->isVisible() ? m_stretchWidth : 32;

                HeaderView->resizeSection(m_columnToStretch, StretchWidth);
            }
        }
    }
    return false;
}

PreferencesDialog::PreferencesDialog(QWidget* _parent) :
    QDialog(_parent)
{
  ui.setupUi(this);

  PreferencesSizeWidget sw(QSize(500,620), QSize(500,600));
  QVBoxLayout layout(ui.preferencesTabWidget);
  layout.addWidget(&sw);

  setWhatsThis(lpubWT(WT_LPUB3D_PREFERENCES, tr("Preferences")));

  ui.tabGeneral->setWhatsThis(lpubWT(                WT_LPUB3D_PREFERENCES_GENERAL, tr("General")));
  ui.tabRenderers->setWhatsThis(lpubWT(              WT_LPUB3D_PREFERENCES_RENDERERS, tr("Renderers")));
  ui.LDGLiteTab->setWhatsThis(lpubWT(                WT_LPUB3D_PREFERENCES_LDGLITE, tr("LDGLite")));
  ui.LDViewTab->setWhatsThis(lpubWT(                 WT_LPUB3D_PREFERENCES_LDVIEW, tr("LDView")));
  ui.NativeTab->setWhatsThis(lpubWT(                 WT_LPUB3D_PREFERENCES_NATIVE, tr("Native")));
  ui.POVRayTab->setWhatsThis(lpubWT(                 WT_LPUB3D_PREFERENCES_POVRAY, tr("POV-Ray")));
  ui.tabPublishing->setWhatsThis(lpubWT(             WT_LPUB3D_PREFERENCES_PUBLISHING, tr("Publishing")));
  ui.tabLogging->setWhatsThis(lpubWT(                WT_LPUB3D_PREFERENCES_THEME_AND_LOGGING, tr("Theme / Logging")));
  ui.tabShortcuts->setWhatsThis(lpubWT(              WT_LPUB3D_PREFERENCES_KEYBOARD_SHORTCUTS, tr("Keyboard Shortcuts")));
  ui.tabOther->setWhatsThis(lpubWT(                  WT_LPUB3D_PREFERENCES_OTHER, tr("Other")));
  ui.tabUpdates->setWhatsThis(lpubWT(                WT_LPUB3D_PREFERENCES_UPDATES, tr("Software Updates")));
  ui.tabDirectories->setWhatsThis(lpubWT(            WT_LPUB3D_PREFERENCES_SEARCH_DIRECTORIES, tr("Search Directories")));

  ui.altLDConfigGrpBox->setWhatsThis(lpubWT(         WT_CONTROL_LPUB3D_PREFERENCES_ALTERNATE_LDCONFIG, ui.altLDConfigGrpBox->title()));
  ui.applyCameraAnglesGrpBox->setWhatsThis(lpubWT(   WT_CONTROL_LPUB3D_PREFERENCES_APPLY_CAMERA_ANGLES, ui.applyCameraAnglesGrpBox->title()));
  ui.attributesGrpBox->setWhatsThis(lpubWT(          WT_CONTROL_LPUB3D_PREFERENCES_ATTRIBUTES, tr("Document Attributes")));
  ui.cameraGrpBox->setWhatsThis(lpubWT(              WT_CONTROL_LPUB3D_PREFERENCES_CAMERA_DEFAULTS, ui.cameraGrpBox->title())); //LcLib
  ui.changeLogGrpBox->setWhatsThis(lpubWT(           WT_CONTROL_LPUB3D_PREFERENCES_CHANGE_LOG, ui.changeLogGrpBox->title()));
  ui.defaultsGrpBox->setWhatsThis(lpubWT(            WT_CONTROL_LPUB3D_PREFERENCES_DEFAULTS, ui.defaultsGrpBox->title()));
  ui.editorGrpBox->setWhatsThis(lpubWT(              WT_CONTROL_LPUB3D_PREFERENCES_EDITOR, tr("Editor")));
  ui.fadeStepGrpBox->setWhatsThis(lpubWT(            WT_CONTROL_LPUB3D_PREFERENCES_FADE_STEP, ui.fadeStepGrpBox->title()));
  ui.generalSettingsGrpBox->setWhatsThis(lpubWT(     WT_CONTROL_LPUB3D_PREFERENCES_GENERAL_SETTINGS, tr("General Settings")));
  ui.highlightStepGrpBox->setWhatsThis(lpubWT(       WT_CONTROL_LPUB3D_PREFERENCES_HIGHLIGHT_STEP, ui.highlightStepGrpBox->title()));
  ui.imageMatteGrpBox->setWhatsThis(lpubWT(          WT_CONTROL_LPUB3D_PREFERENCES_IMAGE_MATTE, ui.imageMatteGrpBox->title()));
  ui.KeyboardShortcutGroup->setWhatsThis(lpubWT(     WT_CONTROL_LPUB3D_PREFERENCES_KEYBOARD_SHORTCUT, ui.KeyboardShortcutGroup->title()));
  ui.KeyboardShortcutTableGroup->setWhatsThis(lpubWT(WT_CONTROL_LPUB3D_PREFERENCES_KEYBOARD_SHORTCUT_TABLE, ui.KeyboardShortcutTableGroup->title()));
  ui.ldgliteGrpBox->setWhatsThis(lpubWT(             WT_CONTROL_LPUB3D_PREFERENCES_LDGLITE, ui.ldgliteGrpBox->title()));
  ui.ldgliteInstallGrpBox->setWhatsThis(lpubWT(      WT_CONTROL_LPUB3D_PREFERENCES_LDGLITE_INSTALL, ui.ldgliteInstallGrpBox->title()));
  ui.ldrawLibPathGrpBox->setWhatsThis(lpubWT(        WT_CONTROL_LPUB3D_PREFERENCES_LDRAW_LIB_PATH, ui.ldrawLibPathGrpBox->title()));
  ui.ldviewGrpBox->setWhatsThis(lpubWT(              WT_CONTROL_LPUB3D_PREFERENCES_LDVIEW, ui.ldviewGrpBox->title()));
  ui.ldviewInstallGrpBox->setWhatsThis(lpubWT(       WT_CONTROL_LPUB3D_PREFERENCES_LDVIEW_INSTALL, ui.ldviewInstallGrpBox->title()));
  ui.ldvPOVSettingsGrpBox->setWhatsThis(lpubWT(      WT_CONTROL_LPUB3D_PREFERENCES_POVRAY_LDVIEW_POV_GENERATION, ui.ldvPOVSettingsGrpBox->title()));
  ui.ldvSettingsGrpBox->setWhatsThis(lpubWT(         WT_CONTROL_LPUB3D_PREFERENCES_LDVIEW_SETTINGS, ui.ldvSettingsGrpBox->title()));
  ui.lgeoGrpBox->setWhatsThis(lpubWT(                WT_CONTROL_LPUB3D_PREFERENCES_POVRAY_LGEO, ui.lgeoGrpBox->title()));
  ui.loggingGrpBox->setWhatsThis(lpubWT(             WT_CONTROL_LPUB3D_PREFERENCES_LOGGING, ui.loggingGrpBox->title()));
  ui.includesGrpBox->setWhatsThis(lpubWT(            WT_CONTROL_LPUB3D_PREFERENCES_LOG_LINE_ATTRIBUTES, ui.includesGrpBox->title()));
  ui.logLevelGrpBox->setWhatsThis(lpubWT(            WT_CONTROL_LPUB3D_PREFERENCES_LOG_LEVEL, ui.logLevelGrpBox->title()));
  ui.logLevelsGrpBox->setWhatsThis(lpubWT(           WT_CONTROL_LPUB3D_PREFERENCES_LOG_LEVEL_MESSAGES, ui.logLevelsGrpBox->title()));
  ui.logPathGrpBox->setWhatsThis(lpubWT(             WT_CONTROL_LPUB3D_PREFERENCES_LOG_PATH, tr("Log Path")));
  ui.lsynthPartsGrpBox->setWhatsThis(lpubWT(         WT_CONTROL_LPUB3D_PREFERENCES_RENDERERS_LSYNTH_PARTS, ui.lsynthPartsGrpBox->title()));
  ui.renderMessageLbl->setWhatsThis(lpubWT(          WT_CONTROL_LPUB3D_PREFERENCES_RENDERERS_MESSAGE, tr("Renderer Message")));
  ui.messagesGrpBox->setWhatsThis(lpubWT(            WT_CONTROL_LPUB3D_PREFERENCES_MESSAGES, ui.messagesGrpBox->title()));
  ui.pageProcessingContinuousGrpBox->setWhatsThis(lpubWT(WT_CONTROL_LPUB3D_PREFERENCES_PAGE_PROCESSING_CONTINUOUS, ui.pageProcessingContinuousGrpBox->title()));
  ui.pliControlGrpBox->setWhatsThis(lpubWT(          WT_CONTROL_LPUB3D_PREFERENCES_PLI_CONTROL, ui.pliControlGrpBox->title()));
  ui.povNativeGenGrpBox->setWhatsThis(lpubWT(        WT_CONTROL_LPUB3D_PREFERENCES_POVRAY_NATIVE_POV_GENERATION, ui.povNativeGenGrpBox->title()));
  ui.povrayGrpBox->setWhatsThis(lpubWT(              WT_CONTROL_LPUB3D_PREFERENCES_POVRAY, ui.povrayGrpBox->title()));
  ui.povrayInstallGrpBox->setWhatsThis(lpubWT(       WT_CONTROL_LPUB3D_PREFERENCES_POVRAY_INSTALL, ui.povrayInstallGrpBox->title()));
  ui.preferredRendererGrpBox->setWhatsThis(lpubWT(   WT_CONTROL_LPUB3D_PREFERENCES_RENDERERS_AND_PROJECTION, tr("Preferred Renderer")));
  ui.publishDescriptionGrpBox->setWhatsThis(lpubWT(  WT_CONTROL_LPUB3D_PREFERENCES_PUBLISH_DESCRIPTION, ui.publishDescriptionGrpBox->title()));
  ui.publishLogoGrpBox->setWhatsThis(lpubWT(         WT_CONTROL_LPUB3D_PREFERENCES_PUBLISH_LOGO, ui.publishLogoGrpBox->title()));
  ui.renderingWidget->setWhatsThis(lpubWT(           WT_CONTROL_LPUB3D_PREFERENCES_RENDERING,tr("Native Rendering"))); //LcLib
  ui.searchDirsGrpBox->setWhatsThis(lpubWT(          WT_CONTROL_LPUB3D_PREFERENCES_SEARCH_DIRECTORIES, ui.searchDirsGrpBox->title()));
  ui.themeSceneColorsGrpBox->setWhatsThis(lpubWT(    WT_CONTROL_LPUB3D_PREFERENCES_THEME_SCENE_COLORS, ui.themeSceneColorsGrpBox->title()));
  ui.themeSelectAndColorsGrpBox->setWhatsThis(lpubWT(WT_CONTROL_LPUB3D_PREFERENCES_THEME_SELECT_AND_COLORS, tr("Theme And Colors")));
  ui.unitsGrpBox->setWhatsThis(lpubWT(               WT_CONTROL_LPUB3D_PREFERENCES_UNITS, ui.unitsGrpBox->title()));
  ui.updatesGrpBox->setWhatsThis(lpubWT(             WT_CONTROL_LPUB3D_PREFERENCES_UPDATES, ui.updatesGrpBox->title()));
  ui.viewpointAndProjectionGrpBox->setWhatsThis(lpubWT(WT_CONTROL_LPUB3D_PREFERENCES_VIEWPOINT_AND_PROJECTION, ui.viewpointAndProjectionGrpBox->title())); //LcLib

  // hide 3rd party application browse buttons
  ui.browseLDGLite->hide();
  ui.browseLDView->hide();
  ui.browsePOVRAY->hide();

  /* Themes */
  ui.themeCombo->addItem(THEME_DEFAULT);
  ui.themeCombo->addItem(THEME_DARK);

  resetSceneColorsFlag = false;

  /* Logging */
  QStringList logLevels = tr(VER_LOGGING_LEVELS_STR).split(",");
  ui.logLevelCombo->addItems(logLevels);

  /* Fade Steps Colors*/
  ui.fadeStepsColoursCombo->addItems(LDrawColor::names());

  // shortcuts
  QCompleter *completer = new QCompleter(ui.KeyboardFilterEdit);
  completer->setModel(new QStringListModel(lpub->shortcutIdKeywords, completer));
  completer->setModelSorting(QCompleter::CaseInsensitivelySortedModel);
  completer->setCaseSensitivity(Qt::CaseInsensitive);
  completer->setWrapAround(false);
  ui.KeyboardFilterEdit->setCompleter(completer);
  ui.KeyboardFilterEdit->setClearButtonEnabled(true);

  ui.shortcutEdit->installEventFilter(this);
  QAction *setShortcutEditResetAct = ui.shortcutEdit->addAction(QIcon(":/resources/resetaction.png"), QLineEdit::TrailingPosition);
  setShortcutEditResetAct->setText(tr("Shortcut Reset"));
  setShortcutEditResetAct->setObjectName("setShortcutEditResetAct.8");
  lpub->actions.insert(setShortcutEditResetAct->objectName(), Action(QStringLiteral("Edit.ShortcutReset"), setShortcutEditResetAct));
  connect(ui.shortcutEdit,         SIGNAL(textChanged(const QString&)),
          this,                    SLOT(enableShortcutEditReset(const QString&)));
  connect(setShortcutEditResetAct, SIGNAL(triggered()),
          this,                    SLOT(shortcutEditReset()));
  connect(ui.commandList,          SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)),
          this,                    SLOT( commandChanged(QTreeWidgetItem*)));
  connect(completer,               SIGNAL(activated(const QString&)),
          this,                    SLOT(on_KeyboardFilterEdit_textEdited(const QString&)));

  updateCommandList();
  new CommandListColumnStretcher(ui.commandList, 0);
  commandChanged(nullptr);
  // end shortcuts

  // populate model file defaults from settings (versus Preferences)
  QSettings Settings;
  if (Settings.contains(QString("%1/%2").arg(DEFAULTS,"Author"))) {
    ui.authorName_Edit->setText(                 Settings.value(QString("%1/%2").arg(DEFAULTS,"Author")).toString());
  }
  if (Settings.contains(QString("%1/%2").arg(DEFAULTS,"URL"))) {
    ui.publishURL_Edit->setText(                 Settings.value(QString("%1/%2").arg(DEFAULTS,"URL")).toString());
  }
  if (Settings.contains(QString("%1/%2").arg(DEFAULTS,"Email"))) {
    ui.publishEmail_Edit->setText(               Settings.value(QString("%1/%2").arg(DEFAULTS,"Email")).toString());
  }
  if (Settings.contains(QString("%1/%2").arg(DEFAULTS,"DocumentLogoFile"))) {
    ui.publishLogoPath->setText(                 Settings.value(QString("%1/%2").arg(DEFAULTS,"DocumentLogoFile")).toString());
    QFileInfo fileInfo(ui.publishLogoPath->text());
    if (!fileInfo.exists()) {
      Settings.remove(QString("%1/%2").arg(DEFAULTS,"DocumentLogoFile"));
      ui.publishLogoPath->clear();
    }
  }
  if (Settings.contains(QString("%1/%2").arg(DEFAULTS,"PublishDescription"))) {
    ui.publishDescriptionEdit->setText(          Settings.value(QString("%1/%2").arg(DEFAULTS,"PublishDescription")).toString());
  }
  ui.publishLogoGrpBox->setChecked(!ui.publishLogoPath->text().isEmpty());

  // set 3rd party application dialogues to read-only
  ui.ldglitePath->setReadOnly(true);
  ui.ldglitePath->setPalette(readOnlyPalette);
  ui.ldviewPath->setReadOnly(true);
  ui.ldviewPath->setPalette(readOnlyPalette);
  ui.povrayPath->setReadOnly(true);
  ui.povrayPath->setPalette(readOnlyPalette);

  // set log path to read only
  ui.logPathEdit->setReadOnly(true);
  ui.logPathEdit->setPalette(readOnlyPalette);

  // [WIP] - Experimental
  ui.imageMatteGrpBox->setEnabled(false);
  ui.imageMattingChk->setEnabled(false);

  //search directories
  ui.lineEditIniFile->setReadOnly(true);
  ui.textEditSearchDirs->setWordWrapMode(QTextOption::WordWrap);
  ui.textEditSearchDirs->setLineWrapMode(QTextEdit::FixedColumnWidth);
  ui.textEditSearchDirs->setLineWrapColumnOrWidth(LINE_WRAP_WIDTH);
  ui.textEditSearchDirs->setReadOnly(true);
  ui.textEditSearchDirs->setToolTip(tr("Read only list of LDraw.ini search directories."));

  // change log
  ui.changeLog_txbr->setWordWrapMode(QTextOption::WordWrap);
  ui.changeLog_txbr->setLineWrapMode(QTextEdit::FixedColumnWidth);
  ui.changeLog_txbr->setLineWrapColumnOrWidth(LINE_WRAP_WIDTH);
  ui.changeLog_txbr->setOpenExternalLinks(true);

  if (ui.povGenNativeRadio->isChecked()) {
      ui.ldvPOVSettingsGrpBox->setTitle(tr("Native POV File Generation Settings"));
      ui.ldvPoVFileGenOptBtn->setToolTip(tr("Open LDView POV generation dialogue"));
      ui.ldvPoVFileGenPrefBtn->setToolTip(tr("Open LDView preferences dialogue"));
  } else {
      ui.ldvPoVFileGenOptBtn->setToolTip(tr("Open LDView POV generation dialogue"));
      ui.ldvPoVFileGenPrefBtn->setToolTip(tr("Open LDView preferences dialogue"));
      ui.ldvPOVSettingsGrpBox->setTitle(tr("LDView POV File Generation Settings"));
  }

  ui.preferencesTabWidget->setCurrentIndex(0);

  // lcLib
  connect(ui.FadeStepsColor,                &QToolButton::clicked,
          this,                             &PreferencesDialog::ColorButtonClicked);
  connect(ui.HighlightNewPartsColor,        &QToolButton::clicked,
          this,                             &PreferencesDialog::ColorButtonClicked);
  connect(ui.HighContrastButton,            SIGNAL(clicked()),
          this,                             SLOT(AutomateEdgeColor()));
  connect(ui.AutomateEdgeColorButton,       SIGNAL(clicked()),
          this,                             SLOT(AutomateEdgeColor()));
  connect(ui.ResetFadeStepsButton,          SIGNAL(clicked()),
          this,                             SLOT(ResetFadeHighlightColor()));
  connect(ui.ResetHighlightNewPartsButton,  SIGNAL(clicked()),
          this,                             SLOT(ResetFadeHighlightColor()));

  connect(ui.resetDefaultDistanceFactor,    SIGNAL(clicked()),
          this,                             SLOT(cameraPropertyReset()));
  connect(ui.resetDefaultPosition,          SIGNAL(clicked()),
          this,                             SLOT(cameraPropertyReset()));
  connect(ui.resetFoV,                      SIGNAL(clicked()),
          this,                             SLOT(cameraPropertyReset()));
  connect(ui.resetNearPlane,                SIGNAL(clicked()),
          this,                             SLOT(cameraPropertyReset()));
  connect(ui.resetFarPlane,                 SIGNAL(clicked()),
          this,                             SLOT(cameraPropertyReset()));
  // lcLib

  connect(ui.ldvPoVFileGenOptBtn,           SIGNAL(clicked()),
          this,                             SLOT(ldvPoVFileGenOptBtn_clicked()));
  connect(ui.ldvPoVFileGenPrefBtn,          SIGNAL(clicked()),
          this,                             SLOT(ldvPoVFileGenPrefBtn_clicked()));
  connect(ui.sceneBackgroundColorButton,    SIGNAL(clicked()),
          this,                             SLOT(sceneColorButtonClicked()));
  connect(ui.sceneGridColorButton,          SIGNAL(clicked()),
          this,                             SLOT(sceneColorButtonClicked()));
  connect(ui.sceneRulerTickColorButton,     SIGNAL(clicked()),
          this,                             SLOT(sceneColorButtonClicked()));
  connect(ui.sceneRulerTrackingColorButton, SIGNAL(clicked()),
          this,                             SLOT(sceneColorButtonClicked()));
  connect(ui.sceneGuideColorButton,         SIGNAL(clicked()),
          this,                             SLOT(sceneColorButtonClicked()));
  connect(lpub,                             SIGNAL(checkForUpdatesFinished ()),
          this,                             SLOT(updateChangelog ()));

  setSizeGripEnabled(true);
}

PreferencesDialog::~PreferencesDialog()
{}

void PreferencesDialog::setPreferences()
{

  readOnlyPalette = QApplication::palette();

  if (Preferences::displayTheme == THEME_DARK)
      readOnlyPalette.setColor(QPalette::Base,QColor(Preferences::themeColors[THEME_DARK_PALETTE_MIDLIGHT]));
  else
      readOnlyPalette.setColor(QPalette::Base,QColor(Preferences::themeColors[THEME_DEFAULT_PALETTE_LIGHT]));
  readOnlyPalette.setColor(QPalette::Text,QColor(LPUB3D_DISABLED_TEXT_COLOUR));

  mLDrawLibPath = Preferences::ldrawLibPath.isEmpty() ? "." : Preferences::ldrawLibPath;
  ui.ldrawLibPathEdit->setText(mLDrawLibPath);

  ldrawLibPathTitle            = tr("LDraw Library Path For %1").arg(Preferences::validLDrawPartsLibrary);
  ui.ldrawLibPathGrpBox->setTitle(ldrawLibPathTitle);

  QString fadeStepsColorTitle  = tr("Use %1 Global Fade Color").arg(Preferences::validLDrawPartsLibrary);
  QString ldrawSearchDirsTitle = tr("LDraw Content Search Directories for %1").arg(Preferences::validLDrawPartsLibrary);
  bool useLDViewSCall          = (Preferences::enableLDViewSingleCall && Preferences::preferredRenderer == RENDERER_LDVIEW);

  showSaveOnRedrawFlag         = Preferences::showSaveOnRedraw;
  showSaveOnUpdateFlag         = Preferences::showSaveOnUpdate;

  ui.autoUpdateChangeLogBox->setChecked(         Preferences::autoUpdateChangeLog);
  ui.updateChangeLogBtn->setEnabled(            !Preferences::autoUpdateChangeLog);

  // preferred renderer
  ui.ldviewPath->setText(                        Preferences::ldviewExe);
  ui.ldviewGrpBox->setVisible(                   Preferences::ldviewInstalled);
  ui.ldviewInstallGrpBox->setVisible(           !Preferences::ldviewInstalled);

  ui.ldviewSingleCall_Chk->setChecked(           useLDViewSCall);
  ui.ldviewSnaphsotsList_Chk->setChecked(        Preferences::enableLDViewSnaphsotList && useLDViewSCall);
  ui.ldviewSnaphsotsList_Chk->setEnabled(        useLDViewSCall);

  ui.ldglitePath->setText(                       Preferences::ldgliteExe);
  ui.ldgliteGrpBox->setVisible(                  Preferences::ldgliteInstalled);
  ui.ldgliteInstallGrpBox->setVisible(          !Preferences::ldgliteInstalled);

  ui.povrayPath->setText(                        Preferences::povrayExe);
  ui.povrayGrpBox->setVisible(                   Preferences::povRayInstalled);
  ui.povrayInstallGrpBox->setVisible(           !Preferences::povRayInstalled);

  ui.povrayDisplay_Chk->setChecked(              Preferences::povrayDisplay);
  ui.povrayAutoCropBox->setChecked(              Preferences::povrayAutoCrop);
  ui.povrayRenderQualityCombo->setCurrentIndex(  Preferences::povrayRenderQuality);

  ui.lgeoGrpBox->setEnabled(                     Preferences::usingDefaultLibrary);
  ui.lgeoPath->setText(                          Preferences::lgeoPath);
  ui.lgeoGrpBox->setChecked(                     Preferences::lgeoPath != "");
  ui.lgeoStlLibLbl->setText(                     Preferences::lgeoStlLib ? DURAT_LGEO_STL_LIB_INFO : "");

  setRenderers();

  // end preferred renderer

  ui.fadeStepsUseColourGrpBox->setTitle(            fadeStepsColorTitle);
  ui.pliControlEdit->setText(                    Preferences::pliControlFile);
  ui.altLDConfigPath->setText(                   Preferences::altLDConfigPath);
  ui.altLDConfigGrpBox->setChecked(              Preferences::altLDConfigPath != "");
  ui.pliControlGrpBox->setChecked(               Preferences::pliControlFile != "");

  ui.displayAllAttributes_Chk->setChecked(       Preferences::displayAllAttributes);
  ui.generateCoverPages_Chk->setChecked(         Preferences::generateCoverPages);
  ui.publishTOC_Chk->setChecked(                 Preferences::printDocumentTOC);
  ui.doNotShowPageProcessDlgChk->setChecked(     Preferences::doNotShowPageProcessDlg);
  ui.enableDownloader_Chk->setChecked(           Preferences::enableDownloader);
  ui.showDownloadRedirects_Chk->setChecked(      Preferences::showDownloadRedirects);
  ui.addLSynthSearchDirBox->setEnabled(          Preferences::archiveLSynthParts);
  ui.addLSynthSearchDirBox->setChecked(          Preferences::addLSynthSearchDir);
  ui.archiveLSynthPartsBox->setChecked(          Preferences::archiveLSynthParts);
  ui.showUpdateNotifications_Chk->setChecked(    Preferences::showUpdateNotifications);
  ui.showAllNotificstions_Chk->setChecked(       Preferences::showAllNotifications);
  ui.checkUpdateFrequency_Combo->setCurrentIndex(Preferences::checkUpdateFrequency);
  ui.rendererTimeout->setValue(                  Preferences::rendererTimeout);
  ui.pageDisplayPauseSpin->setValue(             Preferences::pageDisplayPause);

  ui.loadLastOpenedFileCheck->setChecked(        Preferences::loadLastOpenedFile);
  ui.extendedSubfileSearchCheck->setChecked(     Preferences::extendedSubfileSearch);
  ui.ldrawFilesLoadMsgsCombo->setCurrentIndex(   Preferences::ldrawFilesLoadMsgs);
  ui.projectionCombo->setCurrentIndex(           Preferences::perspectiveProjection ? 0 : 1);

  ui.saveOnRedrawChkBox->setChecked(             Preferences::saveOnRedraw);
  ui.saveOnUpdateChkBox->setChecked(             Preferences::saveOnUpdate);

  ui.loggingGrpBox->setChecked(                  Preferences::logging);
  ui.logPathEdit->setEnabled(                    Preferences::logging);
  ui.logPathEdit->setText(                       Preferences::logPath);

  ui.includeLogLevelBox->setChecked(             Preferences::includeLogLevel);
  ui.includeTimestampBox->setChecked(            Preferences::includeTimestamp);
  ui.includeLineNumberBox->setChecked(           Preferences::includeLineNumber);
  ui.includeFileNameBox->setChecked(             Preferences::includeFileName);
  ui.includeFunctionBox->setChecked(             Preferences::includeFunction);

  ui.logLevelGrpBox->setChecked(                 Preferences::logLevel);
  ui.logLevelsGrpBox->setChecked(                Preferences::logLevels);

  ui.fadeStepGrpBox->setChecked(                 Preferences::enableFadeSteps);
  ui.fadeStepsUseColourGrpBox->setEnabled(       Preferences::enableFadeSteps);
  ui.fadeStepsUseColourGrpBox->setChecked(       Preferences::fadeStepsUseColour);
  ui.fadeStepsColoursCombo->setEnabled(          Preferences::enableFadeSteps && Preferences::fadeStepsUseColour);
  ui.fadeStepsOpacityGrpBox->setEnabled(         Preferences::enableFadeSteps);
  ui.fadeStepsOpacitySlider->setEnabled(         Preferences::enableFadeSteps);
  ui.fadeStepsOpacitySlider->setValue(           Preferences::fadeStepsOpacity);

  ui.fadeStepsColoursCombo->setCurrentIndex(int(ui.fadeStepsColoursCombo->findText(Preferences::validFadeStepsColour)));
  QColor fadeColor = LDrawColor::color(Preferences::validFadeStepsColour);
  if(fadeColor.isValid() ) {
      ui.fadeStepsColourLabel->setAutoFillBackground(true);
      QString styleSheet =
              QString("QLabel { background-color: rgb(%1, %2, %3); }")
              .arg(fadeColor.red())
              .arg(fadeColor.green())
              .arg(fadeColor.blue());
      ui.fadeStepsColourLabel->setStyleSheet(styleSheet);
  }

  ui.highlightStepGrpBox->setChecked(            Preferences::enableHighlightStep);
  ui.highlightStepBtn->setEnabled(               Preferences::enableHighlightStep);
  ui.highlightStepLabel->setEnabled(             Preferences::enableHighlightStep);
  ui.highlightFirstStepBox->setChecked(          Preferences::highlightFirstStep);

  QColor highlightColour = QColor(               Preferences::highlightStepColour);
  if(highlightColour.isValid() ) {
    ui.highlightStepColorLabel->setAutoFillBackground(true);
    QString styleSheet =
        QString("QLabel { background-color: rgb(%1, %2, %3); }")
            .arg(highlightColour.red())
            .arg(highlightColour.green())
            .arg(highlightColour.blue());
    ui.highlightStepColorLabel->setStyleSheet(styleSheet);
  }

  ui.logLevelCombo->setCurrentIndex(int(ui.logLevelCombo->findText(Preferences::loggingLevel)));

  ui.debugLevelBox->setChecked(                  Preferences::debugLevel);
  ui.traceLevelBox->setChecked(                  Preferences::traceLevel);
  ui.noticeLevelBox->setChecked(                 Preferences::noticeLevel);
  ui.infoLevelBox->setChecked(                   Preferences::infoLevel);
  ui.statusLevelBox->setChecked(                 Preferences::statusLevel);
  ui.errorLevelBox->setChecked(                  Preferences::errorLevel);
  ui.fatalLevelBox->setChecked(                  Preferences::fatalLevel);

  ui.includeAllLogAttribBox->setChecked(         Preferences::includeAllLogAttributes);
  ui.allLogLevelsBox->setChecked(                Preferences::allLogLevels);

  // search directories
  ui.lineEditIniFile->setPalette(readOnlyPalette);
  if (Preferences::ldrawiniFound) {
      ui.lineEditIniFile->setText(tr("Using LDraw.ini File: %1").arg(Preferences::ldrawiniFile));
      ui.lineEditIniFile->setToolTip(tr("LDraw.ini file"));
      ui.textEditSearchDirs->setPalette(readOnlyPalette);
      ui.textEditSearchDirs->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
  } else {
      ui.lineEditIniFile->setText(Preferences::ldSearchDirs.size() == 0 ?
                                  tr("Using default search. No search directories detected.") :
                                  tr("Using default %1 search.").arg(VER_PRODUCTNAME_STR));
      ui.lineEditIniFile->setToolTip(tr("Default search"));
  }

  ui.searchDirsGrpBox->setTitle(ldrawSearchDirsTitle);
  if (Preferences::ldSearchDirs.size() > 0) {
      ui.textEditSearchDirs->clear();
      Q_FOREACH (QString searchDir, Preferences::ldSearchDirs)
        ui.textEditSearchDirs->append(searchDir);
  }
  // end search directories

  bool centimeters = Preferences::preferCentimeters;
  ui.Centimeters->setChecked(centimeters);
  ui.Inches->setChecked(!centimeters);

  bool applyCALocally = Preferences::applyCALocally;
  ui.applyCALocallyRadio->setChecked(applyCALocally);
  ui.applyCARendererRadio->setChecked(! applyCALocally);

  bool renderPOVRay = Preferences::preferredRenderer == RENDERER_POVRAY;
  ui.povNativeGenGrpBox->setEnabled(renderPOVRay);
  ui.ldvPOVSettingsGrpBox->setEnabled(renderPOVRay);
  ui.ldvPoVFileGenOptBtn->setEnabled(renderPOVRay);
  ui.ldvPoVFileGenPrefBtn->setEnabled(renderPOVRay);
  ui.povGenNativeRadio->setChecked(Preferences::useNativePovGenerator);
  ui.povGenLDViewRadio->setChecked(!Preferences::useNativePovGenerator);

  ui.ldvPreferencesBtn->setEnabled(Preferences::preferredRenderer == RENDERER_LDVIEW);

  ui.themeCombo->setCurrentText(Preferences::displayTheme);

  QPixmap colorPix(12, 12);

  sceneBackgroundColorStr = Preferences::sceneBackgroundColor;
  colorPix.fill(QColor(sceneBackgroundColorStr));
  ui.sceneBackgroundColorButton->setIcon(colorPix);

  sceneGridColorStr = Preferences::sceneGridColor;
  colorPix.fill(QColor(sceneGridColorStr));
  ui.sceneGridColorButton->setIcon(colorPix);

  sceneRulerTickColorStr = Preferences::sceneRulerTickColor;
  colorPix.fill(QColor(sceneRulerTickColorStr));
  ui.sceneRulerTickColorButton->setIcon(colorPix);

  sceneRulerTrackingColorStr = Preferences::sceneRulerTrackingColor;
  colorPix.fill(QColor(sceneRulerTrackingColorStr));
  ui.sceneRulerTrackingColorButton->setIcon(colorPix);

  sceneGuideColorStr = Preferences::sceneGuideColor;
  colorPix.fill(QColor(sceneGuideColorStr));
  ui.sceneGuideColorButton->setIcon(colorPix);

  /* [Experimental] LDView Image Matting */
  ui.imageMattingChk->setChecked(                Preferences::enableImageMatting);
  ui.imageMattingChk->setEnabled((Preferences::preferredRenderer == RENDERER_LDVIEW) && Preferences::enableFadeSteps);

  QString version = qApp->applicationVersion();
  QString revision = QString::fromLatin1(VER_REVISION_STR);
  QStringList updatableVersions = Preferences::availableVersions.split(",",SkipEmptyParts);
#ifdef QT_DEBUG_MODE
  updatableVersions.prepend(version);
#endif
  updatableVersions.removeDuplicates();
  ui.moduleVersion_Combo->clear();
  ui.moduleVersion_Combo->addItems(updatableVersions);
  ui.moduleVersion_Combo->setCurrentIndex(int(ui.moduleVersion_Combo->findText(version)));

  QString versionInfo;
#if defined LP3D_CONTINUOUS_BUILD || defined LP3D_DEVOPS_BUILD || defined LP3D_NEXT_BUILD
  versionInfo = tr("Change Log for version %1 revision %2 (%3)").arg(version, revision, QString::fromLatin1(VER_BUILD_TYPE_STR));
#else
  int revisionNumber = revision.toInt();
  versionInfo = tr("Change Log for version %1%2").arg(version, revisionNumber ? QString(" revision %1").arg(revision) : "");
#endif
  ui.changeLogGrpBox->setTitle(versionInfo);

  updateChangelog();

  // show message options
  mShowLineParseErrors    = Preferences::lineParseErrors;
  mShowInsertErrors       = Preferences::showInsertErrors;
  mShowBuildModErrors     = Preferences::showBuildModErrors;
  mShowIncludeFileErrors  = Preferences::showIncludeFileErrors;
  mShowAnnotationErrors   = Preferences::showAnnotationErrors;
}

void PreferencesDialog::setOptions(lcLibRenderOptions* Options)
{
    mSetOptions = true;
    mOptions = Options;

    // LPub3D Preferences
    setPreferences();

    // LcLib Preferences
    lcQPreferencesInit();
    mSetOptions = false;
}

void PreferencesDialog::setRenderers()
{
    bool ldgliteExists = false;
    bool povRayExists = false;
    bool ldviewExists = false;

    int povRayIndex = -1;
    int ldgliteIndex = -1;
    int ldviewIndex = -1;
    int nativeIndex = -1;

    QFileInfo fileInfo;

    ui.preferredRenderer->setMaxCount(0);
    ui.preferredRenderer->setMaxCount(4);

    if (Preferences::povRayInstalled) {
      fileInfo.setFile(Preferences::povrayExe);
      povRayIndex = ui.preferredRenderer->count();
      povRayExists = fileInfo.exists();
      povRayExists &= fileInfo.exists();
      if (povRayExists) {
          ui.preferredRenderer->addItem(rendererNames[RENDERER_POVRAY]);
        }
    } else {
      connect(ui.povrayInstall, SIGNAL(clicked()), this, SLOT(installRenderer()));
    }

    if (Preferences::ldgliteInstalled) {
      fileInfo.setFile(Preferences::ldgliteExe);
      ldgliteIndex = ui.preferredRenderer->count();
      ldgliteExists = fileInfo.exists();
      if (ldgliteExists) {
        ui.preferredRenderer->addItem(rendererNames[RENDERER_LDGLITE]);
      }
    } else {
      connect(ui.ldgliteInstall, SIGNAL(clicked()), this, SLOT(installRenderer()));
    }

    if (Preferences::ldviewInstalled) {
      fileInfo.setFile(Preferences::ldviewExe);
      ldviewIndex = ui.preferredRenderer->count();
      ldviewExists = fileInfo.exists();
      if (ldviewExists) {
        ui.preferredRenderer->addItem(rendererNames[RENDERER_LDVIEW]);
      }
    } else {
      connect(ui.ldviewInstall, SIGNAL(clicked()), this, SLOT(installRenderer()));
    }

    nativeIndex = ui.preferredRenderer->count();
    ui.preferredRenderer->addItem(rendererNames[RENDERER_NATIVE]);

    if (Preferences::preferredRenderer == RENDERER_LDVIEW && ldviewExists) {
      ui.preferredRenderer->setCurrentIndex(ldviewIndex);
      ui.preferredRenderer->setEnabled(true);
      ui.renderersTabWidget->setCurrentWidget(ui.LDViewTab);
    } else if (Preferences::preferredRenderer == RENDERER_LDGLITE && ldgliteExists) {
      ui.preferredRenderer->setCurrentIndex(ldgliteIndex);
      ui.preferredRenderer->setEnabled(true);
      ui.renderersTabWidget->setCurrentWidget(ui.LDGLiteTab);
      ui.highlightStepLineWidthSpin->setVisible(true);
      ui.highlightStepLabel->setVisible(true);
      //ui.highlightStepSpacer->setVisible(true);
      ui.highlightStepLineWidthSpin->setEnabled(Preferences::enableHighlightStep);
      ui.highlightStepLineWidthSpin->setValue(Preferences::highlightStepLineWidth);
    }  else if (Preferences::preferredRenderer == RENDERER_POVRAY && povRayExists) {
      ui.preferredRenderer->setCurrentIndex(povRayIndex);
      ui.preferredRenderer->setEnabled(true);
      ui.renderersTabWidget->setCurrentWidget(ui.POVRayTab);
    } else {
        ui.renderersTabWidget->setCurrentWidget(ui.NativeTab);
        if (Preferences::preferredRenderer == RENDERER_NATIVE) {
          ui.preferredRenderer->setCurrentIndex(nativeIndex);
          ui.preferredRenderer->setEnabled(true);
        } else {
          ui.preferredRenderer->setEnabled(false);
        }
    }

    if (Preferences::preferredRenderer != RENDERER_LDGLITE) {
        ui.highlightStepLineWidthSpin->setVisible(false);
        ui.highlightStepLabel->setVisible(false);
        //ui.highlightStepSpacer->setVisible(false);
    }
}

void PreferencesDialog::installRenderer()
{
    ui.renderMessageLbl->setText("");

    int which = -1;
    if (sender() == ui.ldgliteInstall)
        which = RENDERER_LDGLITE;
    else if (sender() == ui.ldviewInstall)
        which = RENDERER_LDVIEW;
    else if (sender() == ui.povrayInstall)
        which = RENDERER_POVRAY;

    if (gui->installRenderer(which)) {
        setRenderers();
        switch (which)
        {
        case RENDERER_LDVIEW:
            ui.ldviewPath->setText(              Preferences::ldviewExe);
            ui.ldviewGrpBox->setVisible(         Preferences::ldviewInstalled);
            ui.ldviewInstallGrpBox->setVisible( !Preferences::ldviewInstalled);
            break;
        case RENDERER_LDGLITE:
            ui.ldglitePath->setText(             Preferences::ldgliteExe);
            ui.ldgliteGrpBox->setVisible(        Preferences::ldgliteInstalled);
            ui.ldgliteInstallGrpBox->setVisible(!Preferences::ldgliteInstalled);
            break;
        case RENDERER_POVRAY:
            ui.povrayPath->setText(              Preferences::povrayExe);
            ui.povrayGrpBox->setVisible(         Preferences::povRayInstalled);
            ui.povrayInstallGrpBox->setVisible( !Preferences::povRayInstalled);
            break;
        default:
            break;
        }
        ui.renderMessageLbl->setText(tr("%1 renderer installed successfully.").arg(rendererNames[which]));
    }
}

void PreferencesDialog::sceneColorButtonClicked()
{
    QObject *button = sender();
    QString title;
    QColor oldColor;
    QColorDialog::ColorDialogOptions dialogOptions;

    if (button == ui.sceneBackgroundColorButton)
    {
        oldColor = QColor(sceneBackgroundColorStr);
        title = tr("Select Scene Background Color");
        dialogOptions = nullptr;
    }
    else if (button == ui.sceneGridColorButton)
    {
        oldColor = QColor(sceneGridColorStr);
        title = tr("Select Scene Grid Color");
        dialogOptions = nullptr;
    }
    else if (button == ui.sceneRulerTickColorButton)
    {
        oldColor = QColor(sceneRulerTickColorStr);
        title = tr("Select Scene Ruler Tick Color");
        dialogOptions = nullptr;
    }
    else if (button == ui.sceneRulerTrackingColorButton)
    {
        oldColor = QColor(sceneRulerTrackingColorStr);
        title = tr("Select Ruler Tracking Color");
        dialogOptions = nullptr;
    }
    else if (button == ui.sceneGuideColorButton)
    {
        oldColor = QColor(sceneGuideColorStr);
        title = tr("Select Scene Guide Color");
        dialogOptions = nullptr;
    }
    else
        return;

    QColor newColor = QColorDialog::getColor(oldColor, this, title, dialogOptions);

    if (newColor == oldColor || !newColor.isValid())
        return;

    if (button == ui.sceneBackgroundColorButton)
    {
        sceneBackgroundColorStr = newColor.name();
    }
    else if (button == ui.sceneGridColorButton)
    {
        sceneGridColorStr = newColor.name();
    }
    else if (button == ui.sceneRulerTickColorButton)
    {
        sceneRulerTickColorStr = newColor.name();
    }
    else if (button == ui.sceneRulerTrackingColorButton)
    {
        sceneRulerTrackingColorStr = newColor.name();
    }
    else if (button == ui.sceneGuideColorButton)
    {
        sceneGuideColorStr = newColor.name();
    }

    QPixmap pix(12, 12);

    pix.fill(newColor);
    ((QToolButton*)button)->setIcon(pix);
}

void PreferencesDialog::on_ldrawLibPathEdit_editingFinished()
{
    QString newPath = QDir::toNativeSeparators(ui.ldrawLibPathEdit->text().trimmed().toLower());
    QString oldPath = QDir::toNativeSeparators(mLDrawLibPath.toLower());
    if (newPath != oldPath) {
        QMessageBox box;
        if (!Preferences::checkLDrawLibrary(ui.ldrawLibPathEdit->text())) {
            box.setIcon (QMessageBox::Warning);
            box.setWindowTitle(tr ("Suspicious LDraw Directory!"));
            box.setDefaultButton   (QMessageBox::Ok);
            box.setStandardButtons (QMessageBox::Ok);
            box.setText (tr("The selected path [%1] does not "
                            "appear to be a valid LDraw Library.")
                            .arg(ui.ldrawLibPathEdit->text()));
            box.exec();
        } else {
            if (Preferences::validLDrawLibrary != Preferences::validLDrawLibraryChange) {
                ui.lgeoGrpBox->setEnabled(Preferences::validLDrawLibraryChange == LEGO_LIBRARY);
                QString ldrawTitle = tr("LDraw Library Path For %1® Parts")
                                        .arg(Preferences::validLDrawLibraryChange);
                ui.ldrawLibPathGrpBox->setTitle(ldrawTitle);
                ui.ldrawLibPathGrpBox->setStyleSheet("QGroupBox::title { color : red; }");
            }
        }
    } else {
        ui.ldrawLibPathGrpBox->setTitle(ldrawLibPathTitle);
        ui.ldrawLibPathGrpBox->setStyleSheet("");
    }
}

void PreferencesDialog::on_browseLDraw_clicked()
{
    Preferences::ldrawPreferences(true);
    ui.ldrawLibPathEdit->setText(Preferences::ldrawLibPath);
    if (Preferences::validLDrawLibrary != Preferences::validLDrawLibraryChange) {
        ui.lgeoGrpBox->setEnabled(Preferences::validLDrawLibraryChange == LEGO_LIBRARY);
        QString ldrawTitle = tr("LDraw Library Path For %1® Parts")
                                .arg(Preferences::validLDrawLibraryChange);
        ui.ldrawLibPathGrpBox->setTitle(ldrawTitle);
        ui.ldrawLibPathGrpBox->setStyleSheet("QGroupBox::title { color : red; }");
    } else {
        ui.ldrawLibPathGrpBox->setTitle(ldrawLibPathTitle);
        ui.ldrawLibPathGrpBox->setStyleSheet("");
    }
}

void PreferencesDialog::on_browseAltLDConfig_clicked()
{
#ifdef Q_OS_WIN
    QString filter(tr("LDraw Files (*.ldr);;All Files (*.*)"));
#else
    QString filter(tr("All Files (*.*)"));
#endif

    QString result = QFileDialog::getOpenFileName(this, tr("Select LDraw LDConfig File"),
                                                  ui.altLDConfigPath->text().isEmpty() ? Preferences::ldrawLibPath : ui.altLDConfigPath->text(),
                                                  filter);

    if (!result.isEmpty()) {
        result = QDir::toNativeSeparators(result);
        ui.altLDConfigPath->setText(result);
        ui.altLDConfigGrpBox->setChecked(true);
    }
}

void PreferencesDialog::on_browseLGEO_clicked()
{

    QString result = QFileDialog::getExistingDirectory(this, tr("Locate LGEO Directory"),
                                                       ui.lgeoPath->text().isEmpty() ? Preferences::ldrawLibPath : ui.lgeoPath->text(),
                                                       QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if (!result.isEmpty()) {
            ui.lgeoPath->setText(QDir::toNativeSeparators(result));
            ui.lgeoGrpBox->setChecked(true);
        }
}

void PreferencesDialog::on_browsePli_clicked()
{
#ifdef Q_OS_WIN
    QString filter(tr("LDraw Files (*.mpd;*.dat;*.ldr);;All Files (*.*)"));
#else
    QString filter(tr("All Files (*.*)"));
#endif
    QString filePath = QFileDialog::getOpenFileName(
                this, tr("Locate Parts List orientation/size file"),
                ui.pliControlEdit->text().isEmpty() ? Preferences::lpubDataPath + "/extras" : ui.pliControlEdit->text(),
                filter);
    if (!filePath.isEmpty()) {
      QString cwd = QDir::currentPath();
      if (filePath.startsWith(cwd))
        filePath = filePath.replace(cwd,".");
      ui.pliControlEdit->setText(QDir::toNativeSeparators(filePath));
      ui.pliControlGrpBox->setChecked(true);
    }
}

void PreferencesDialog::on_browsePublishLogo_clicked()
{
#ifdef Q_OS_WIN
    QString filter(tr("Image Files (*.png);;All Files (*.*)"));
#else
    QString filter(tr("All Files (*.*)"));
#endif
    QString filePath = QFileDialog::getOpenFileName(
                this, tr("Select Default Document Logo"),
                ui.publishLogoPath->text().isEmpty() ? Preferences::lpubDataPath + "/extras" : ui.publishLogoPath->text(),
                filter);
    if (!filePath.isEmpty()) {
      QString cwd = QDir::currentPath();
      if (filePath.startsWith(cwd))
        filePath = filePath.replace(cwd,".");
      filePath = QDir::toNativeSeparators(filePath);
      ui.publishLogoPath->setText(filePath);
      ui.publishLogoGrpBox->setChecked(true);
    }
}

void PreferencesDialog::on_ldviewSingleCall_Chk_clicked(bool checked)
{
    ui.ldviewSnaphsotsList_Chk->setEnabled(checked);
}

void PreferencesDialog::on_includeAllLogAttribBox_clicked(bool checked)
{
  ui.includeLogLevelBox->setChecked(checked);
  ui.includeTimestampBox->setChecked(checked);
  ui.includeLineNumberBox->setChecked(checked);
  ui.includeFileNameBox->setChecked(checked);
  ui.includeFunctionBox->setChecked(checked);
}

void PreferencesDialog::on_allLogLevelsBox_clicked(bool checked)
{
  ui.debugLevelBox->setChecked(checked);
  ui.traceLevelBox->setChecked(checked);
  ui.noticeLevelBox->setChecked(checked);
  ui.infoLevelBox->setChecked(checked);
  ui.statusLevelBox->setChecked(checked);
  ui.errorLevelBox->setChecked(checked);
  ui.fatalLevelBox->setChecked(checked);
}

void PreferencesDialog::on_logLevelsGrpBox_clicked(bool checked)
{
  ui.loggingMessageLbl->hide();
  ui.statusLevelBox->setChecked(checked);
  ui.logLevelGrpBox->setChecked(!checked);
}

void PreferencesDialog::on_logLevelGrpBox_clicked(bool checked)
{
  ui.loggingMessageLbl->hide();
  ui.logLevelsGrpBox->setChecked(!checked);
}

void PreferencesDialog::on_ldviewGrpBox_clicked(bool checked)
{
  if (! checked) {
      QMessageBox box;
      box.setIcon (QMessageBox::Information);
      box.setStandardButtons (QMessageBox::Ok);
      box.setWindowTitle(tr ("LDView Settings?"));
      box.setText (tr ("The 'is installed' LDView setting is automatically set at application startup.\n"
                       "Changes will be reset at next application start? "));
      emit gui->messageSig(LOG_STATUS,box.text());
      box.exec();
    }
}

void PreferencesDialog::on_ldgliteGrpBox_clicked(bool checked)
{
    if (! checked) {
        QMessageBox box;
        box.setIcon (QMessageBox::Information);
        box.setStandardButtons (QMessageBox::Ok);
        box.setWindowTitle(tr ("LDGLite Settings?"));
        box.setText (tr ("The 'is installed' LDGLite setting is automatically set at application startup.\n"
                         "Changes will be reset at next application start? "));
        emit gui->messageSig(LOG_STATUS,box.text());
        box.exec();
    }
}

void PreferencesDialog::on_povrayGrpBox_clicked(bool checked)
{
  if (! checked) {
      QMessageBox box;
      box.setIcon (QMessageBox::Information);
      box.setStandardButtons (QMessageBox::Ok);
      box.setWindowTitle(tr ("Raytracer (POV-Ray) Settings?"));
      box.setText (tr ("The 'is installed' Raytracer (POV-Ray) setting is automatically set at application startup.\n"
                       "Changes will be reset at next application start? "));
      emit gui->messageSig(LOG_STATUS,box.text());
      box.exec();
  }
}

void PreferencesDialog::on_altLDConfigGrpBox_clicked(bool checked)
{
  if (! checked && ! ui.altLDConfigPath->text().isEmpty()) {
    QMessageBox box;
    box.setIcon (QMessageBox::Warning);
    box.setStandardButtons (QMessageBox::Yes|QMessageBox::Cancel);
    box.setWindowTitle(tr ("Alternate LDConfig File"));
    box.setText (tr ("This action will remove %1 from your settings.\n"
                     "Are you sure you want to continue? ")
                     .arg(ui.altLDConfigPath->text()));
    emit gui->messageSig(LOG_STATUS,box.text());
    if (box.exec() == QMessageBox::Yes) {
      ui.altLDConfigPath->clear();
      ui.altLDConfigGrpBox->setChecked(false);
    }
  }
}

void PreferencesDialog::on_archiveLSynthPartsBox_clicked(bool checked)
{
    ui.addLSynthSearchDirBox->setEnabled(checked);
}

void PreferencesDialog::on_fadeStepsColoursCombo_currentIndexChanged(const QString &colorName)
{
  QColor newFadeColor = LDrawColor::color(colorName);
  if(newFadeColor.isValid() ) {
      ui.fadeStepsColourLabel->setAutoFillBackground(true);
      QString styleSheet =
          QString("QLabel { background-color: rgb(%1, %2, %3); }")
          .arg(newFadeColor.red())
          .arg(newFadeColor.green())
          .arg(newFadeColor.blue());
      ui.fadeStepsColourLabel->setStyleSheet(styleSheet);
    }
}

void PreferencesDialog::on_highlightStepBtn_clicked()
{
  QColor highlightColour = QColorDialog::getColor(ui.highlightStepColorLabel->palette().background().color(), this );
  if(highlightColour.isValid()) {
    ui.highlightStepColorLabel->setAutoFillBackground(true);
    QString styleSheet =
        QString("QLabel { background-color: rgb(%1, %2, %3); }")
        .arg(highlightColour.red())
        .arg(highlightColour.green())
        .arg(highlightColour.blue());
    ui.highlightStepColorLabel->setStyleSheet(styleSheet);
  }
}

void PreferencesDialog::on_fadeStepGrpBox_clicked(bool checked)
{
  ui.fadeStepsUseColourGrpBox->setEnabled(checked);
  ui.fadeStepsColoursCombo->setEnabled(checked);
  ui.fadeStepsOpacityGrpBox->setEnabled(checked);
  ui.fadeStepsOpacitySlider->setEnabled(checked);

  /* [Experimental] LDView Image Matting */
  ui.imageMattingChk->setEnabled((Preferences::preferredRenderer == RENDERER_LDVIEW) && checked);
}

void PreferencesDialog::on_fadeStepsUseColourGrpBox_clicked(bool checked)
{
  ui.fadeStepsColoursCombo->setEnabled(checked);
}

void PreferencesDialog::on_highlightStepGrpBox_clicked(bool checked)
{
  ui.highlightStepBtn->setEnabled(checked);
  ui.highlightStepLabel->setEnabled(checked);
  // Only enabled for LDGLite
  if (ui.preferredRenderer->currentText() == rendererNames[RENDERER_LDGLITE])
    ui.highlightStepLineWidthSpin->setEnabled(checked);
  else
    ui.highlightStepLineWidthSpin->setEnabled(false);
}

void PreferencesDialog::on_preferredRenderer_currentIndexChanged(const QString &currentText)
{
  bool ldviewEnabled  = (currentText == rendererNames[RENDERER_LDVIEW]);
  bool povrayEnabled  = (currentText == rendererNames[RENDERER_POVRAY]);
  bool ldgliteEnabled = (currentText == rendererNames[RENDERER_LDGLITE]);
  bool nativeEnabled  = (currentText == rendererNames[RENDERER_NATIVE]);
  ui.povNativeGenGrpBox->setEnabled(povrayEnabled);
  ui.ldvPOVSettingsGrpBox->setEnabled(povrayEnabled);
  ui.ldvPreferencesBtn->setEnabled(ldviewEnabled);
  ui.ldvPoVFileGenOptBtn->setEnabled(povrayEnabled);
  ui.ldvPoVFileGenPrefBtn->setEnabled(povrayEnabled);
  if (ui.povGenNativeRadio->isChecked())
    ui.ldvPOVSettingsGrpBox->setTitle(tr("Native POV File Generation Settings"));
  else
  if (ui.povGenLDViewRadio->isChecked())
    ui.ldvPOVSettingsGrpBox->setTitle(tr("LDView POV File Generation Settings"));

  if (ldviewEnabled)
    ui.renderersTabWidget->setCurrentWidget(ui.LDViewTab);
  else if (povrayEnabled)
    ui.renderersTabWidget->setCurrentWidget(ui.POVRayTab);
  else if (ldgliteEnabled)
    ui.renderersTabWidget->setCurrentWidget(ui.LDGLiteTab);
  else if (nativeEnabled)
    ui.renderersTabWidget->setCurrentWidget(ui.NativeTab);

  if (ldgliteEnabled) {
    ui.highlightStepLineWidthSpin->setVisible(true);
    ui.highlightStepLabel->setVisible(true);
    //ui.highlightStepSpacer->setVisible(true);
    ui.highlightStepLineWidthSpin->setEnabled(Preferences::enableHighlightStep);
  } else {
    ui.highlightStepLineWidthSpin->setVisible(false);
    ui.highlightStepLabel->setVisible(false);
    //ui.highlightStepSpacer->setVisible(false);
  }

  bool applyCARenderer = ldviewEnabled && ui.projectionCombo->currentText() == QLatin1String("Perspective");
  ui.applyCALocallyRadio->setChecked(! applyCARenderer);
  ui.applyCARendererRadio->setChecked(applyCARenderer);

  /* [Experimental] LDView Image Matting */
  ui.imageMattingChk->setEnabled(ldviewEnabled && Preferences::enableFadeSteps);
}

void PreferencesDialog::on_projectionCombo_currentIndexChanged(const QString &currentText)
{
    if (mSetOptions)
        return;

    bool applyCARenderer = ui.preferredRenderer->currentText() == rendererNames[RENDERER_LDVIEW] &&
                           currentText == QLatin1String("Perspective");
    ui.applyCALocallyRadio->setChecked(! applyCARenderer);
    ui.applyCARendererRadio->setChecked(applyCARenderer);

    ui.ProjectionCombo->setCurrentIndex(ui.projectionCombo->currentIndex());
}

void PreferencesDialog::on_applyCALocallyRadio_clicked(bool checked)
{
    bool applyCARenderer = ui.preferredRenderer->currentText() == rendererNames[RENDERER_LDVIEW] &&
                           ui.projectionCombo->currentText() == QLatin1String("Perspective");
    if (checked && applyCARenderer) {
      QMessageBox box;
      box.setIcon (QMessageBox::Warning);
      box.setStandardButtons (QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel);
      box.setWindowTitle(tr ("Perspective Projection"));
      box.setText (tr ("The preferred renderer is set to %1 and projection is Perspective.<br>"
                       "This configureaiton requires camera angles to be set by %1.<br>"
                       "Are you sure you want to change the camera angles settings ?")
                       .arg(rendererNames[RENDERER_LDVIEW]));
      box.setDefaultButton(QMessageBox::No);
      if (box.exec() == QMessageBox::No) {
          ui.applyCALocallyRadio->setChecked(!checked);
          ui.applyCARendererRadio->setChecked(checked);
      } else {
          ui.applyCALocallyRadio->setChecked(checked);
          ui.applyCARendererRadio->setChecked(!checked);
      }
    }
}

void PreferencesDialog::on_ldvPreferencesBtn_clicked()
{
    ldvWidget = new LDVWidget(this);
    ldvWidget->showLDVPreferences();
}

void PreferencesDialog::on_povGenNativeRadio_clicked(bool checked)
{
    if (checked) {
        ui.ldvPOVSettingsGrpBox->setTitle(tr("Native POV File generation Settings"));
        ui.ldvPoVFileGenOptBtn->setToolTip(tr("Open Native POV generation dialogue"));
        ui.ldvPoVFileGenPrefBtn->setToolTip(tr("Open Native preferences dialogue"));
    }
}

void PreferencesDialog::on_povGenLDViewRadio_clicked(bool checked)
{
    if (checked) {
        ui.ldvPOVSettingsGrpBox->setTitle(tr("LDView POV File Generation Settings"));
        ui.ldvPoVFileGenOptBtn->setToolTip(tr("Open LDView POV generation dialogue"));
        ui.ldvPoVFileGenPrefBtn->setToolTip(tr("Open LDView preferences dialogue"));
    }
}

void PreferencesDialog::on_saveOnRedrawChkBox_clicked(bool checked)
{
  Q_UNUSED(checked)
  showSaveOnUpdateFlag = true;
}

void PreferencesDialog::on_saveOnUpdateChkBox_clicked(bool checked)
{
  Q_UNUSED(checked)
  showSaveOnUpdateFlag = true;
}

void PreferencesDialog::on_resetSceneColorsButton_clicked(bool checked)
{
    resetSceneColorsFlag = checked;

    if (Preferences::displayTheme == THEME_DARK) {
        sceneBackgroundColorStr    = Preferences::themeColors[THEME_DARK_SCENE_BACKGROUND_COLOR];
        sceneGridColorStr          = Preferences::themeColors[THEME_DARK_GRID_PEN];
        sceneRulerTickColorStr     = Preferences::themeColors[THEME_DARK_RULER_TICK_PEN];
        sceneRulerTrackingColorStr = Preferences::themeColors[THEME_DARK_RULER_TRACK_PEN];
        sceneGuideColorStr         = Preferences::themeColors[THEME_DARK_GUIDE_PEN];
    } else {
        sceneBackgroundColorStr    = Preferences::themeColors[THEME_DEFAULT_SCENE_BACKGROUND_COLOR];
        sceneGridColorStr          = Preferences::themeColors[THEME_DEFAULT_GRID_PEN];
        sceneRulerTickColorStr     = Preferences::themeColors[THEME_DEFAULT_RULER_TICK_PEN];
        sceneRulerTrackingColorStr = Preferences::themeColors[THEME_DEFAULT_RULER_TRACK_PEN];
        sceneGuideColorStr         = Preferences::themeColors[THEME_DEFAULT_GUIDE_PEN];
    }

    QPixmap pix(12, 12);

    pix.fill(QColor(sceneBackgroundColorStr));
    ui.sceneBackgroundColorButton->setIcon(pix);

    pix.fill(QColor(sceneGridColorStr));
    ui.sceneGridColorButton->setIcon(pix);

    pix.fill(QColor(sceneRulerTickColorStr));
    ui.sceneRulerTickColorButton->setIcon(pix);

    pix.fill(QColor(sceneRulerTrackingColorStr));
    ui.sceneRulerTrackingColorButton->setIcon(pix);

    pix.fill(QColor(sceneGuideColorStr));
    ui.sceneGuideColorButton->setIcon(pix);
}

void PreferencesDialog::on_themeColorsButton_clicked()
{
    ThemeColorsDialog *themeColorsDialog = new ThemeColorsDialog();
    editedThemeColors = themeColorsDialog->getEditedThemeColors();
}

void PreferencesDialog::ldvPoVFileGenOptBtn_clicked()
{
    if (ui.povGenNativeRadio->isChecked())
        ldvWidget = new LDVWidget(this,NativePOVIni,true);
    else
        ldvWidget = new LDVWidget(this,LDViewPOVIni,true);

    ldvWidget->showLDVExportOptions();
}

void PreferencesDialog::ldvPoVFileGenPrefBtn_clicked()
{
    if (ui.povGenNativeRadio->isChecked())
        ldvWidget = new LDVWidget(this,NativePOVIni,true);
    else
        ldvWidget = new LDVWidget(this,LDViewPOVIni,true);

    ldvWidget->showLDVPreferences();
}

void PreferencesDialog::on_loggingGrpBox_clicked(bool checked)
{
      ui.logPathEdit->setEnabled(checked);
}

void PreferencesDialog::on_optionsButton_clicked(bool checked)
{
    Q_UNUSED(checked)

    // options dialogue
    messageDialog = new QDialog();

    messageDialog->setWindowTitle(tr("File Parse Messages"));

    messageDialog->setWhatsThis(lpubWT(WT_LPUB3D_PREFERENCES_MESSAGES,messageDialog->windowTitle()));

    QFormLayout *form = new QFormLayout(messageDialog);

    // options - parse errors
    QGroupBox *parseErrorGrpBox = new QGroupBox(tr("Message Categories"));
    parseErrorGrpBox->setWhatsThis(lpubWT(WT_CONTROL_LPUB3D_PREFERENCES_PARSE_ERROR,parseErrorGrpBox->title()));
    form->addWidget(parseErrorGrpBox);
    QGridLayout *parseErrorLayout = new QGridLayout(parseErrorGrpBox);

    QCheckBox* parseErrorChkBox = new QCheckBox(tr("Show model line parse errors"), messageDialog);
    parseErrorChkBox->setChecked(Preferences::lineParseErrors);
    parseErrorLayout->addWidget(parseErrorChkBox,0,0,1,2);
    parseErrorTBtn = new QToolButton(messageDialog);
    parseErrorLayout->addWidget(parseErrorTBtn,1,0);
    parseErrorLbl = new QLabel("", messageDialog);
    parseErrorLayout->addWidget(parseErrorLbl,1,1);
    QFrame* separator = new QFrame();
    separator->setFrameShape(QFrame::HLine);
    parseErrorLayout->addWidget(separator,2,0,1,2);
    QObject::connect(parseErrorTBtn, SIGNAL(clicked()), this, SLOT(messageManagement()));

    // options - insert errors
    QCheckBox * insertErrorChkBox = new QCheckBox(tr("Show model insert errors"), messageDialog);
    insertErrorChkBox ->setChecked(Preferences::showInsertErrors);
    parseErrorLayout->addWidget(insertErrorChkBox,3,0,1,2);
    insertErrorTBtn = new QToolButton(messageDialog);
    parseErrorLayout->addWidget(insertErrorTBtn,4,0);
    insertErrorLbl = new QLabel("", messageDialog);
    parseErrorLayout->addWidget(insertErrorLbl,4,1);
    separator = new QFrame();
    separator->setFrameShape(QFrame::HLine);
    parseErrorLayout->addWidget(separator,5,0,1,2);
    QObject::connect(insertErrorTBtn, SIGNAL(clicked()), this, SLOT(messageManagement()));

    // options - build modification errors
    QCheckBox * buildModErrorChkBox = new QCheckBox(tr("Show build modification errors"), messageDialog);
    buildModErrorChkBox->setChecked(Preferences::showBuildModErrors);
    parseErrorLayout->addWidget(buildModErrorChkBox,6,0,1,2);
    buildModErrorTBtn = new QToolButton(messageDialog);
    parseErrorLayout->addWidget(buildModErrorTBtn,7,0);
    buildModErrorLbl = new QLabel("", messageDialog);
    parseErrorLayout->addWidget(buildModErrorLbl,7,1);
    separator = new QFrame();
    separator->setFrameShape(QFrame::HLine);
    parseErrorLayout->addWidget(separator,8,0,1,2);
    QObject::connect(buildModErrorTBtn, SIGNAL(clicked()), this, SLOT(messageManagement()));

    // options - include file errors
    QCheckBox * includeFileErrorChkBox = new QCheckBox(tr("Show include file errors"), messageDialog);
    includeFileErrorChkBox->setChecked(Preferences::showIncludeFileErrors);
    parseErrorLayout->addWidget(includeFileErrorChkBox,9,0,1,2);
    includeErrorTBtn = new QToolButton(messageDialog);
    parseErrorLayout->addWidget(includeErrorTBtn,10,0);
    includeErrorLbl = new QLabel("", messageDialog);
    parseErrorLayout->addWidget(includeErrorLbl,10,1);
    separator = new QFrame();
    separator->setFrameShape(QFrame::HLine);
    parseErrorLayout->addWidget(separator,11,0,1,2);
    QObject::connect(includeErrorTBtn, SIGNAL(clicked()), this, SLOT(messageManagement()));

    // options - annotation errors
    QCheckBox * annotationErrorChkBox = new QCheckBox(tr("Show annotation errors"), messageDialog);
    annotationErrorChkBox->setChecked(Preferences::showAnnotationErrors);
    parseErrorLayout->addWidget(annotationErrorChkBox,12,0,1,2);
    annotationErrorTBtn = new QToolButton(messageDialog);
    parseErrorLayout->addWidget(annotationErrorTBtn,13,0);
    annotationErrorLbl = new QLabel("", messageDialog);
    parseErrorLayout->addWidget(annotationErrorLbl,13,1);
    QObject::connect(annotationErrorTBtn, SIGNAL(clicked()), this, SLOT(messageManagement()));
    separator = new QFrame();
    separator->setFrameShape(QFrame::HLine);
    parseErrorLayout->addWidget(separator,14,0,1,2);

    // options - clear all detail messages
    clearDetailErrorsTBtn = new QToolButton(messageDialog);
    parseErrorLayout->addWidget(clearDetailErrorsTBtn,15,0);
    clearDetailErrorsLbl = new QLabel("", messageDialog);
    parseErrorLayout->addWidget(clearDetailErrorsLbl,15,1);
    QObject::connect(clearDetailErrorsTBtn, SIGNAL(clicked()), this, SLOT(messageManagement()));

    // options - button box
    messageButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                                                             Qt::Horizontal, messageDialog);
    form->addRow(messageButtonBox);
    QObject::connect(messageButtonBox, SIGNAL(accepted()), messageDialog, SLOT(accept()));
    QObject::connect(messageButtonBox, SIGNAL(rejected()), messageDialog, SLOT(reject()));

    messageDialog->setMinimumHeight(parseErrorGrpBox->sizeHint().height());

    // load message counts
    messageManagement();

    if (messageDialog->exec() == QDialog::Accepted) {
        mShowLineParseErrors   = parseErrorChkBox->isChecked();
        mShowInsertErrors      = insertErrorChkBox ->isChecked();
        mShowBuildModErrors    = buildModErrorChkBox->isChecked();
        mShowIncludeFileErrors = includeFileErrorChkBox->isChecked();
        mShowAnnotationErrors  = annotationErrorChkBox->isChecked();
    }
}

void PreferencesDialog::on_checkForUpdates_btn_clicked()
{
    checkForUpdates();
}

void PreferencesDialog::on_autoUpdateChangeLogBox_clicked(bool checked)
{
   ui.updateChangeLogBtn->setEnabled(!checked);
}

void PreferencesDialog::on_updateChangeLogBtn_clicked()
{
    //populate release notes from the web
    QElapsedTimer timer;
    timer.start();
    m_updateFinished = false;
    lpub->m_updater->retrieveChangeLog( LPub::DEFS_URL);
    while (!m_updateFinished)
        QApplication::processEvents();
    emit gui->messageSig(LOG_NOTICE, tr("Download release notes completed. %1")
                                        .arg(lpub->elapsedTime(timer.elapsed())));
}

void PreferencesDialog::checkForUpdates ()
{
    auto processRequest = [&] ()
    {
        const QString htmlNotes = ui.changeLog_txbr->toHtml();

        ui.changeLog_txbr->setHtml(tr("<p><span style=\"color: #0000ff;\">Updating change log, please wait...</span></p>"));

        /* Get settings from the UI */
        const QString moduleVersion = ui.moduleVersion_Combo->currentText();
        const QString moduleRevision = QString::fromLatin1(VER_REVISION_STR);
        const bool showRedirects = ui.showDownloadRedirects_Chk->isChecked();
        const bool enableDownloader = ui.enableDownloader_Chk->isChecked();
        const bool showAllNotifications = ui.showAllNotificstions_Chk->isChecked();
        const bool showUpdateNotifications = ui.showUpdateNotifications_Chk->isChecked();

        /* Apply the settings */
        if (lpub->m_updater->getModuleVersion(LPub::DEFS_URL) != moduleVersion)
            lpub->m_updater->setModuleVersion(LPub::DEFS_URL, moduleVersion);
        if (lpub->m_updater->getModuleRevision(LPub::DEFS_URL) != moduleRevision)
            lpub->m_updater->setModuleRevision(LPub::DEFS_URL, moduleRevision);
        lpub->m_updater->setShowRedirects(LPub::DEFS_URL, showRedirects);
        lpub->m_updater->setDownloaderEnabled(LPub::DEFS_URL, enableDownloader);
        lpub->m_updater->setNotifyOnFinish(LPub::DEFS_URL, showAllNotifications);
        lpub->m_updater->setNotifyOnUpdate (LPub::DEFS_URL, showUpdateNotifications);

        /* Check for updates */
        lpub->m_updater->checkForUpdates (LPub::DEFS_URL);

        if (!LPub::m_updaterCancelled) {
            QSettings Settings;
            Settings.setValue(QString("%1/%2").arg(UPDATES,"LastCheck"), QDateTime::currentDateTimeUtc());
        }

        if (ui.changeLog_txbr->document()->isEmpty())
            ui.changeLog_txbr->setHtml(htmlNotes);
    };

    QElapsedTimer timer;
    timer.start();
    m_updateFinished = false;
    processRequest();
    while (!m_updateFinished)
        QApplication::processEvents();
    emit gui->messageSig(LOG_NOTICE, tr("Check for updates completed. %1")
                                        .arg(lpub->elapsedTime(timer.elapsed())));
}

void PreferencesDialog::messageManagement()
{
    auto countErrors = [this] ()
    {
        int lineParseErrorCount   = 0;
        int insertErrorCount      = 0;
        int buildModErrorCount    = 0;
        int includeFileErrorCount = 0;
        int annotationErrorCount  = 0;

        if (Preferences::messagesNotShown.size()) {
            bool ok;
            Q_FOREACH (QString message,Preferences::messagesNotShown) {
                int key = QString(message.at(0)).toInt(&ok);
                if (ok) {
                    Preferences::MsgKey msgKey = Preferences::MsgKey(key);
                    switch(msgKey)
                    {
                    case Preferences::ParseErrors:
                        lineParseErrorCount++; break;
                    case Preferences::InsertErrors:
                        insertErrorCount++; break;
                    case Preferences::BuildModErrors:
                        buildModErrorCount++; break;
                    case Preferences::IncludeFileErrors:
                        includeFileErrorCount++; break;
                    case Preferences::AnnotationErrors:
                        annotationErrorCount++; break;
                    default:
                        break;
                    }
                }
            }
        }

        parseErrorLbl->setText(tr("Clear %1 model line parse errors").arg(lineParseErrorCount));
        parseErrorTBtn->setEnabled(lineParseErrorCount);
        buildModErrorLbl->setText(tr("Clear %1 build modification errors").arg(buildModErrorCount));
        buildModErrorTBtn->setEnabled(buildModErrorCount);
        insertErrorLbl->setText(tr("Clear %1 model insert errors").arg(insertErrorCount));
        insertErrorTBtn->setEnabled(insertErrorCount);
        includeErrorLbl->setText(tr("Clear %1 include file errors") .arg(includeFileErrorCount));
        includeErrorTBtn->setEnabled(includeFileErrorCount);
        annotationErrorLbl->setText(tr("Clear %1 annotation errors").arg(annotationErrorCount));
        annotationErrorTBtn->setEnabled(annotationErrorCount);
    };

    auto clearErrors = [](Preferences::MsgKey msgKey)
    {
        int counter = 0;
        if (msgKey == Preferences::NumKeys) {
            counter = Preferences::messagesNotShown.size();
            Preferences::messagesNotShown.clear();
            return counter;
        }
        Preferences::messagesNotShown.erase(std::remove_if(
            Preferences::messagesNotShown.begin(), Preferences::messagesNotShown.end(),
            [&counter, &msgKey](const QString &message) {
                bool ok;
                int keyInt = QString(message.at(0)).toInt(&ok);
                if (ok) {
                    counter++;
                    return msgKey == Preferences::MsgKey(keyInt);
                }
                return false;
            }), Preferences::messagesNotShown.end());
        return counter;
    };

    int cleared = 0;
    QString clearDetailText;
    const QString style = QString("QLabel { color : %1; }")
            .arg(Preferences::displayTheme == THEME_DARK ? "#ff9999" : "#ff0000");
    if (sender() == parseErrorTBtn) {
        cleared = clearErrors(Preferences::ParseErrors);
        parseErrorLbl->setStyleSheet(style);
        parseErrorLbl->setText(QString("Cleared %1 model line parse errors").arg(cleared));
    } else if (sender() == insertErrorTBtn) {
        cleared = clearErrors(Preferences::InsertErrors);
        insertErrorLbl->setStyleSheet(style);
        insertErrorLbl->setText(tr("Cleared %1 model insert errors").arg(cleared));
    } else if (sender() == buildModErrorTBtn) {
        cleared = clearErrors(Preferences::BuildModErrors);
        buildModErrorLbl->setStyleSheet(style);
        buildModErrorLbl->setText(tr("Cleared %1 build modification errorss").arg(cleared));
    } else if (sender() == includeErrorTBtn) {
        cleared = clearErrors(Preferences::IncludeFileErrors);
        includeErrorLbl->setStyleSheet(style);
        includeErrorLbl->setText(tr("Cleared %1 include file error").arg(cleared));
    } else if (sender() == annotationErrorTBtn) {
        cleared = clearErrors(Preferences::AnnotationErrors);
        annotationErrorLbl->setStyleSheet(style);
        annotationErrorLbl->setText(tr("Cleared %1 annotation errors").arg(cleared));
    } else if (sender() == clearDetailErrorsTBtn) {
        cleared = clearErrors(Preferences::NumKeys);
        clearDetailErrorsLbl->setStyleSheet(style);
        clearDetailText = tr("Cleared");
        countErrors();
    } else {
        countErrors();
        clearDetailText = QString("Clear");
    }
    clearDetailErrorsLbl->setText(tr("%1 all %2 errors")
                                  .arg(clearDetailText)
                                  .arg(sender() == clearDetailErrorsTBtn ? cleared :
                                                                           Preferences::messagesNotShown.size()));
    clearDetailErrorsTBtn->setEnabled(Preferences::messagesNotShown.size());
    messageButtonBox->button(QDialogButtonBox::Cancel)->setEnabled(!cleared);
}

QString const PreferencesDialog::moduleVersion()
{
   return ui.moduleVersion_Combo->currentText();
}

QString const PreferencesDialog::ldrawLibPath()
{
  return ui.ldrawLibPathEdit->displayText();
}

QString const PreferencesDialog::altLDConfigPath()
{
  if (ui.altLDConfigGrpBox->isChecked()) {
    return ui.altLDConfigPath->displayText();
  }
  return "";
}

QString const PreferencesDialog::pliControlFile()
{
  if (ui.pliControlGrpBox->isChecked()) {
    return ui.pliControlEdit->displayText();
  }
  return "";
}

QString const PreferencesDialog::lgeoPath()
{
    if (Preferences::povRayInstalled && ui.lgeoGrpBox->isChecked()){
        return ui.lgeoPath->displayText();
    }
    return "";
}

QString const PreferencesDialog::ldviewExe()
{
  if (Preferences::ldviewInstalled) {
    return ui.ldviewPath->displayText();
  }
  return "";
}

QString const PreferencesDialog::ldgliteExe()
{
  if (Preferences::ldgliteInstalled) {
    return ui.ldglitePath->displayText();
  }
    return "";
}

QString const PreferencesDialog::povrayExe()
{
    if (Preferences::povRayInstalled) {
        return ui.povrayPath->displayText();
    }
    return "";
}

int PreferencesDialog::preferredRenderer()
{
  if (ui.preferredRenderer->isEnabled()) {
    return rendererMap[ui.preferredRenderer->currentText()];
  }
  return RENDERER_INVALID;
}

bool PreferencesDialog::useNativePovGenerator()
{
    return ui.povGenNativeRadio->isChecked();
}

bool PreferencesDialog::autoUpdateChangeLog()
{
    return ui.autoUpdateChangeLogBox->isChecked();
}

bool PreferencesDialog::perspectiveProjection()
{
   return ui.projectionCombo->currentIndex() == 0 ? true : false;
}

bool PreferencesDialog::saveOnRedraw()
{
       return ui.saveOnRedrawChkBox->isChecked();
}

bool PreferencesDialog::saveOnUpdate()
{
       return ui.saveOnUpdateChkBox->isChecked();
}

bool PreferencesDialog::povrayDisplay()
{
    return ui.povrayDisplay_Chk->isChecked();
}

int PreferencesDialog::povrayRenderQuality()
{
    return ui.povrayRenderQualityCombo->currentIndex();
}

bool PreferencesDialog::povrayAutoCrop()
{
    return ui.povrayAutoCropBox->isChecked();
}

bool PreferencesDialog::loadLastOpenedFile()
{
    return ui.loadLastOpenedFileCheck->isChecked();
}

bool PreferencesDialog::extendedSubfileSearch()
{
    return ui.extendedSubfileSearchCheck->isChecked();
}

int PreferencesDialog::ldrawFilesLoadMsgs()
{
    return ui.ldrawFilesLoadMsgsCombo->currentIndex();
}

QString const PreferencesDialog::fadeStepsColour()
{
    return ui.fadeStepsColoursCombo->currentText();
}

bool PreferencesDialog::showSaveOnRedraw()
{
 return showSaveOnRedrawFlag;
}

bool PreferencesDialog::showSaveOnUpdate()
{
 return showSaveOnUpdateFlag;
}

QString const PreferencesDialog::highlightStepColour()
{
    return ui.highlightStepColorLabel->palette().background().color().name();
}

QString const PreferencesDialog::documentLogoFile()
{
    if (ui.publishLogoGrpBox->isChecked()){
        return ui.publishLogoPath->displayText();
    }
    return QString();
}

int PreferencesDialog::fadeStepsOpacity()
{
  return ui.fadeStepsOpacitySlider->value();
}

int PreferencesDialog::highlightStepLineWidth()
{
  return ui.highlightStepLineWidthSpin->value();
}

bool PreferencesDialog::highlightFirstStep()
{
       return ui.highlightFirstStepBox->isChecked();
}

bool PreferencesDialog::enableImageMatting()
{
       return ui.imageMattingChk->isChecked();
}

bool PreferencesDialog::centimeters()
{
  return ui.Centimeters->isChecked();
}

bool PreferencesDialog::applyCALocally()
{
  return ui.applyCALocallyRadio->isChecked();
}

bool  PreferencesDialog::enableFadeSteps()
{
  return ui.fadeStepGrpBox->isChecked();
}

bool  PreferencesDialog::enableHighlightStep()
{
  return ui.highlightStepGrpBox->isChecked();
}

bool PreferencesDialog::enableDocumentLogo()
{
  return ui.publishLogoGrpBox->isChecked();
}

bool PreferencesDialog::fadeStepsUseColour()
{
    return ui.fadeStepsUseColourGrpBox->isChecked();
}

bool PreferencesDialog::enableLDViewSingleCall()
{
  return ui.ldviewSingleCall_Chk->isChecked();
}

bool PreferencesDialog::enableLDViewSnaphsotList()
{
       return ui.ldviewSnaphsotsList_Chk->isChecked();
}

bool  PreferencesDialog::displayAllAttributes()
{
  return ui.displayAllAttributes_Chk->isChecked();
}

bool  PreferencesDialog::generateCoverPages()
{
  return ui.generateCoverPages_Chk->isChecked();
}

bool  PreferencesDialog::printDocumentTOC()
{
  return ui.publishTOC_Chk->isChecked();
}

bool  PreferencesDialog::doNotShowPageProcessDlg()
{
  return ui.doNotShowPageProcessDlgChk->isChecked();
}

QString const PreferencesDialog::displayTheme()
{
  return ui.themeCombo->currentText();
}

QMap<int, QString> const PreferencesDialog::themeColours()
{
    return editedThemeColors;
}

QString const PreferencesDialog::sceneBackgroundColor()
{
  return sceneBackgroundColorStr;
}

QString const PreferencesDialog::sceneGridColor()
{
  return sceneGridColorStr;
}

QString const PreferencesDialog::sceneRulerTickColor()
{
  return sceneRulerTickColorStr;
}

QString const PreferencesDialog::sceneRulerTrackingColor()
{
  return sceneRulerTrackingColorStr;
}

QString const PreferencesDialog::sceneGuideColor()
{
  return sceneGuideColorStr;
}

bool PreferencesDialog::resetSceneColors()
{
  return resetSceneColorsFlag;
}

QString const PreferencesDialog::defaultURL()
{
  return ui.publishURL_Edit->displayText();
}

QString const PreferencesDialog::defaultEmail()
{
  return ui.publishEmail_Edit->displayText();
}

QString const PreferencesDialog::defaultAuthor()
{
  return ui.authorName_Edit->displayText();
}

QString const PreferencesDialog::publishDescription()
{
  return ui.publishDescriptionEdit->toPlainText();
}

bool PreferencesDialog::addLSynthSearchDir()
{
  return ui.addLSynthSearchDirBox->isChecked();
}

bool PreferencesDialog::archiveLSynthParts()
{
  return ui.archiveLSynthPartsBox->isChecked();
}

bool PreferencesDialog::showUpdateNotifications()
{
  return ui.showUpdateNotifications_Chk->isChecked();
}

bool PreferencesDialog::enableDownloader()
{
  return ui.enableDownloader_Chk->isChecked();
}

bool PreferencesDialog::showDownloadRedirects()
{
  return ui.showDownloadRedirects_Chk->isChecked();
}

bool PreferencesDialog::showAllNotifications()
{
  return ui.showAllNotificstions_Chk->isChecked();
}

int PreferencesDialog::checkUpdateFrequency()
{
  return ui.checkUpdateFrequency_Combo->currentIndex();
}

int PreferencesDialog::rendererTimeout()
{
  return ui.rendererTimeout->value();
}

int PreferencesDialog::pageDisplayPause()
{
  return ui.pageDisplayPauseSpin->value();
}

bool PreferencesDialog::showLineParseErrors()
{
  return mShowLineParseErrors;
}

bool PreferencesDialog::showInsertErrors()
{
  return mShowInsertErrors ;
}

bool PreferencesDialog::showBuildModErrors()
{
  return mShowBuildModErrors;
}

bool PreferencesDialog::showIncludeFileErrors()
{
  return mShowIncludeFileErrors;
}

bool PreferencesDialog::inlineNativeContent()
{
  QSettings Settings;
  if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,"InlineNativeContent")))
    return Preferences::inlineNativeContent;
  return Settings.value(QString("%1/%2").arg(SETTINGS,"InlineNativeContent")).toBool();

  // return ui.inlineNativeContentBox->isChecked(); // future implementation
}

bool PreferencesDialog::showAnnotationErrors()
{
  return mShowAnnotationErrors;
}

bool PreferencesDialog::includeLogLevel()
{
  return ui.includeLogLevelBox->isChecked();
}
bool PreferencesDialog::includeTimestamp()
{
  return ui.includeTimestampBox->isChecked();
}
bool PreferencesDialog::includeLineNumber()
{
  return ui.includeLineNumberBox->isChecked();
}
bool PreferencesDialog::includeFileName()
{
  return ui.includeFileNameBox->isChecked();
}
bool PreferencesDialog::includeFunction()
{
  return ui.includeFunctionBox->isChecked();
}
bool PreferencesDialog::includeAllLogAttrib()
{
  return ui.includeAllLogAttribBox->isChecked();
}
bool PreferencesDialog::loggingGrpBox()
{
  return ui.loggingGrpBox->isChecked(); //off-level
}
bool PreferencesDialog::logLevelGrpBox()
{
  return ui.logLevelGrpBox->isChecked();
}
const QString PreferencesDialog::logLevelCombo()
{
  return ui.logLevelCombo->currentText();
}
bool PreferencesDialog::logLevelsGrpBox()
{
  return ui.logLevelsGrpBox->isChecked();
}
bool PreferencesDialog::debugLevel()
{
  return ui.debugLevelBox->isChecked();
}
bool PreferencesDialog::traceLevel()
{
  return ui.traceLevelBox->isChecked();
}
bool PreferencesDialog::noticeLevel()
{
  return ui.noticeLevelBox->isChecked();
}
bool PreferencesDialog::infoLevel()
{
  return ui.infoLevelBox->isChecked();
}
bool PreferencesDialog::statusLevel()
{
  return ui.statusLevelBox->isChecked();
}
bool PreferencesDialog::errorLevel()
{
  return ui.errorLevelBox->isChecked();
}
bool PreferencesDialog::fatalLevel()
{
  return ui.fatalLevelBox->isChecked();
}
bool PreferencesDialog::allLogLevels()
{
  return ui.allLogLevelsBox->isChecked();
}

void PreferencesDialog::updateChangelog()
{
    ui.changeLogGrpBox->setTitle(LPub::m_versionInfo);

    if (LPub::m_setReleaseNotesAsText)
        ui.changeLog_txbr->setPlainText(LPub::m_releaseNotesContent);
    else
        ui.changeLog_txbr->setHtml(LPub::m_releaseNotesContent);

    if (ui.updateChangeLogBtn->isEnabled())
        ui.updateChangeLogBtn->setEnabled(false);

    m_updateFinished = true;
}

void PreferencesDialog::accept()
{
    bool missingParms = false;
    QFileInfo fileInfo;

    if (resetSceneColorsFlag)
        emit gui->messageSig(LOG_INFO,tr("Scene Colors have been reset."));

    if (!ui.povrayPath->text().isEmpty() && (ui.povrayPath->text() != Preferences::povrayExe)){
        fileInfo.setFile(ui.povrayPath->text());
        bool povRayExists = fileInfo.exists();
        if (povRayExists)
        {
            Preferences::povrayExe = ui.povrayPath->text();
            ui.preferredRenderer->addItem(rendererNames[RENDERER_POVRAY]);
        } else {
            emit gui->messageSig(LOG_ERROR,tr("POV-Ray path entered is not valid: %1").arg(ui.povrayPath->text()));
        }
    }
    if (!ui.ldglitePath->text().isEmpty() && (ui.ldglitePath->text() != Preferences::ldgliteExe)) {
        fileInfo.setFile(ui.ldglitePath->text());
        bool ldgliteExists = fileInfo.exists();
        if (ldgliteExists)
        {
            Preferences::ldgliteExe = ui.ldglitePath->text();
            ui.preferredRenderer->addItem(rendererNames[RENDERER_LDGLITE]);
        } else {
            emit gui->messageSig(LOG_ERROR,tr("LDGLite path entered is not valid: %1").arg(ui.ldglitePath->text()));
        }
    }
    if (!ui.ldviewPath->text().isEmpty() && (ui.ldviewPath->text() != Preferences::ldviewExe)) {
        QString ldviewPath = ui.ldviewPath->text();
#ifndef Q_OS_LINUX
        fileInfo.setFile(ldviewPath);
#else
        // force use command line-only "ldview" (not RENDERER_LDVIEW) if not using Windows
        QFileInfo info(ldviewPath);
        fileInfo.setFile(QString("%1/%2").arg(info.absolutePath()).arg(info.fileName().toLower()));
#endif
        bool ldviewExists = fileInfo.exists();
        if (ldviewExists)
        {
            Preferences::ldviewExe = ldviewPath;
            ui.preferredRenderer->addItem(rendererNames[RENDERER_LDVIEW]);
        } else {
            emit gui->messageSig(LOG_ERROR,tr("LDView path entered is not valid: %1").arg(ui.ldviewPath->text()));
        }
    }

    // LcLib Preferences
    lcQPreferencesAccept();

    if(ui.preferredRenderer->count() == 0 || ui.ldrawLibPathEdit->text().isEmpty())
    {
        missingParms = true;
        if (ui.preferredRenderer->count() == 0){
            ui.ldglitePath->setPlaceholderText(tr("At lease one renderer must be defined"));
            ui.ldviewPath->setPlaceholderText(tr("At lease one renderer must be defined"));
            ui.povrayPath->setPlaceholderText(tr("At lease one renderer must be defined"));
            ui.ldrawLibPathEdit->setPlaceholderText(tr("LDraw path must be defined"));
        }
    }

    if (ui.includesGrpBox->isChecked() &&
        ! ui.includeLogLevelBox->isChecked() &&
        ! ui.includeTimestampBox->isChecked() &&
        ! ui.includeLineNumberBox->isChecked() &&
        ! ui.includeFileNameBox->isChecked() &&
        ! ui.includeFunctionBox->isChecked())
    {
        missingParms = true;
        ui.loggingMessageLbl->show();
        ui.loggingMessageLbl->setText(tr("At lease one attribute must be included."));
        ui.loggingMessageLbl->setStyleSheet("QLabel { background-color : red; color : white; }");
    }

    if (ui.logLevelsGrpBox->isChecked() &&
        ! ui.fatalLevelBox->isChecked() &&
        ! ui.errorLevelBox->isChecked() &&
        ! ui.statusLevelBox->isChecked() &&
        ! ui.infoLevelBox->isChecked() &&
        ! ui.noticeLevelBox->isChecked() &&
        ! ui.traceLevelBox->isChecked() &&
        ! ui.debugLevelBox->isChecked())
    {
        missingParms = true;
        ui.loggingMessageLbl->show();
        ui.loggingMessageLbl->setText(tr("At lease one logging level must be selected."));
        ui.loggingMessageLbl->setStyleSheet("QLabel { background-color : red; color : white; }");
    }

    if (missingParms)
    {
        if (QMessageBox::Yes == QMessageBox::question(this, tr("Close Dialogue?"),
                                 tr("Required settings are missing.\n Are you sure you want to exit?"),
                                 QMessageBox::Yes|QMessageBox::No)){
            QDialog::reject(); //keep open
        }
    } else {
        QDialog::accept();
    }
}

void PreferencesDialog::cancel()
{
    QDialog::reject();
}

void PreferencesDialog::closeEvent(QCloseEvent *event)
{
    if (maybeSave())
        accept();
    else
        cancel();
    QDialog::closeEvent(event);
}

bool PreferencesDialog::maybeSave()
{
  bool rc = false;
  bool changed = false;
  QString changeMessage;

  if (mOptions->KeyboardShortcutsModified) {
      changeMessage = tr("There are modified keyboard shortcuts.");
      changed = true;
  }

  if (changed) {

    QPixmap _icon = QPixmap(":/icons/lpub96.png");

    QMessageBoxResizable box;
    box.setWindowIcon(QIcon());
    box.setIconPixmap (_icon);
    box.setTextFormat (Qt::RichText);
    box.setWindowTitle(tr ("%1 Preferences").arg(VER_PRODUCTNAME_STR));
    box.setWindowFlags (Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
    QString title = "<b>" + tr ("Preference changes detected") + "</b>";
    QString text = tr("%1<br>Do you want to save your changes?").arg(changeMessage);
    box.setText (title);
    box.setInformativeText (text);
    box.setStandardButtons (QMessageBox::No | QMessageBox::Yes);
    box.setDefaultButton   (QMessageBox::Yes);

    if (box.exec() == QMessageBox::Yes)
      rc = true;
  }
  return rc;
}

/***********************************************************************
 *
 * Theme Colors
 *
 **********************************************************************/

QMap<int, QString> ThemeColorsDialog::getEditedThemeColors()
{
    dialog = new QDialog(nullptr);
    dialog->setWindowTitle(tr("Edit Theme Colors"));

    dialog->setWhatsThis(lpubWT(WT_LPUB3D_PREFERENCES_THEME_COLORS,dialog->windowTitle()));

    // Default Theme Tab
    tabs = new QTabWidget(dialog);
    tabs->setMovable(false);
    QVBoxLayout *layout = new QVBoxLayout();

    dialog->setLayout(layout);
    layout->addWidget(tabs);

    QWidget *container = new QWidget(tabs);
    container->setObjectName(tr("Default Theme"));
    container->setWhatsThis(lpubWT(WT_LPUB3D_PREFERENCES_DEFAULT_THEME_COLORS,container->objectName()));
    QGridLayout *gridLayout = new QGridLayout(container);
    gridLayout->setSizeConstraint(QLayout::SetFixedSize);
    container->setLayout(gridLayout);

    QScrollArea *scrollArea = new QScrollArea(tabs);
    scrollArea->setWidget(container);

    auto setControls = [this, &gridLayout, &container] (const int begin, const int end)
    {
        for (int i = begin; i < end; i++) {
            mThemeColors[i] = Preferences::themeColors[i];
            // description label
            QLabel *label = new QLabel(Preferences::defaultThemeColors[i].label, container);
            gridLayout->addWidget(label,i,0);

            // color button
            QToolButton *colorButton = new QToolButton(container);
            QPixmap pix(12, 12);
            pix.fill(getColor(i, mThemeColors[i]));
            colorButton->setIcon(pix);
            colorButton->setProperty("index", i);
            colorButton->setProperty("color", mThemeColors[i]);
            colorButton->setToolTip(tr("Edit Color"));
            gridLayout->addWidget(colorButton,i,1);
            connect(colorButton, SIGNAL(clicked()), this, SLOT(setThemeColor()));
            colorButtonList << colorButton;

            // reset button
            QToolButton *resetButton = new QToolButton(container);
            resetButton->setText(tr("..."));
            resetButton->setProperty("index", i);
            QString toolTipText = QString(tr("Reset"));
            if (mThemeColors[i] == Preferences::defaultThemeColors[i].color) {
                resetButton->setEnabled(false);
            } else {
                // prepare color icon
                QImage image(16, 16, QImage::Format_RGB888);
                image.fill(getColor(i, Preferences::defaultThemeColors[i].color));
                QPainter painter(&image);
                painter.setPen(Qt::darkGray);
                painter.drawRect(0, 0, image.width() - 1, image.height() - 1);
                painter.end();
                QByteArray ba;
                QBuffer buffer(&ba);
                buffer.open(QIODevice::WriteOnly);
                image.save(&buffer, "PNG");
                buffer.close();
                const char*  format = "<table><tr><td>Reset To %1</td><td style=\"vertical-align:middle\"><img src=\"data:image/png;base64,%2\"/></td></tr></table>";
                toolTipText = QString(format).arg(Preferences::defaultThemeColors[i].color.toUpper(), QString(buffer.data().toBase64()));
            }
            resetButton->setToolTip(toolTipText);
            gridLayout->addWidget(resetButton,i,2);
            connect(resetButton, SIGNAL(clicked()), this, SLOT(resetThemeColor()));
            resetButtonList << resetButton;
        }
    };

    setControls(THEME_DEFAULT_SCENE_BACKGROUND_COLOR, THEME_DARK_SCENE_BACKGROUND_COLOR);

    scrollArea->setMinimumWidth(
                container->sizeHint().width() +
                2 * scrollArea->frameWidth() +
                scrollArea->verticalScrollBar()->sizeHint().width());

    tabs->addTab(scrollArea,container->objectName());

    // Dark Theme Tab
    container = new QWidget(tabs);
    container->setObjectName(tr("Dark Theme"));
    container->setWhatsThis(lpubWT(WT_LPUB3D_PREFERENCES_DARK_THEME_COLORS,container->objectName()));
    gridLayout = new QGridLayout(container);
    gridLayout->setSizeConstraint(QLayout::SetFixedSize);
    container->setLayout(gridLayout);

    scrollArea = new QScrollArea(tabs);
    scrollArea->setWidget(container);

    setControls(THEME_DARK_SCENE_BACKGROUND_COLOR, THEME_NUM_COLORS);

    scrollArea->setMinimumWidth(
                container->sizeHint().width() +
                2 * scrollArea->frameWidth() +
                scrollArea->verticalScrollBar()->sizeHint().width());

    tabs->addTab(scrollArea,container->objectName());

    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                               Qt::Horizontal, dialog);
    dialog->layout()->addWidget(&buttonBox);
    QObject::connect(&buttonBox, SIGNAL(accepted()), dialog, SLOT(accept()));
    QObject::connect(&buttonBox, SIGNAL(rejected()), dialog, SLOT(reject()));

    QPushButton *button = new QPushButton(tr("Reset"));
    button->setToolTip(tr("Reset theme colors to defaults"));
    buttonBox.addButton(button,QDialogButtonBox::ActionRole);
    QObject::connect(button,SIGNAL(clicked()), this, SLOT(resetThemeColors()));

    button = new QPushButton(tr("Defaults"));
    button->setToolTip(tr("Toggle 'Defaults' tab"));
    buttonBox.addButton(button,QDialogButtonBox::ActionRole);
    QObject::connect(button,SIGNAL(clicked()), this, SLOT(toggleDefaultsTab()));

    bool themeDark = Preferences::displayTheme == THEME_DARK ;

    QMap<int, QString> editedColors;

    dialog->setMinimumSize(200,400);

    tabs->setCurrentIndex(themeDark ? 1 : 0);

    if (dialog->exec() == QDialog::Accepted) {
        for (int i = 0; i < THEME_NUM_COLORS; i++) {
            if (mThemeColors[i] != Preferences::themeColors[i]) {
                editedColors.insert(i, mThemeColors[i].toUpper());
                LC_PROFILE_KEY key = LC_PROFILE_KEY(-1);
                switch (i)
                {
                case THEME_DARK_VIEWER_BACKGROUND_COLOR:
                case THEME_DEFAULT_VIEWER_BACKGROUND_COLOR:
                    key = LC_PROFILE_BACKGROUND_COLOR;
                    break;
                case THEME_DARK_VIEWER_GRADIENT_COLOR_TOP:
                case THEME_DEFAULT_VIEWER_GRADIENT_COLOR_TOP:
                    key = LC_PROFILE_GRADIENT_COLOR_TOP;
                    break;
                case THEME_DARK_VIEWER_GRADIENT_COLOR_BOTTOM:
                case THEME_DEFAULT_VIEWER_GRADIENT_COLOR_BOTTOM:
                    key = LC_PROFILE_GRADIENT_COLOR_BOTTOM;
                    break;
                case THEME_DARK_AXES_COLOR:
                case THEME_DEFAULT_AXES_COLOR:
                    key = LC_PROFILE_AXES_COLOR;
                    break;
                case THEME_DARK_OVERLAY_COLOR:
                case THEME_DEFAULT_OVERLAY_COLOR:
                    key = LC_PROFILE_OVERLAY_COLOR;
                    break;
                case THEME_DARK_MARQUEE_BORDER_COLOR:
                case THEME_DEFAULT_MARQUEE_BORDER_COLOR:
                    key = LC_PROFILE_MARQUEE_BORDER_COLOR;
                    break;
                case THEME_DARK_MARQUEE_FILL_COLOR:
                case THEME_DEFAULT_MARQUEE_FILL_COLOR:
                    key = LC_PROFILE_MARQUEE_FILL_COLOR;
                    break;
                case THEME_DARK_INACTIVE_VIEW_COLOR:
                case THEME_DEFAULT_INACTIVE_VIEW_COLOR:
                    key = LC_PROFILE_INACTIVE_VIEW_COLOR;
                    break;
                case THEME_DARK_ACTIVE_VIEW_COLOR:
                case THEME_DEFAULT_ACTIVE_VIEW_COLOR:
                    key = LC_PROFILE_ACTIVE_VIEW_COLOR;
                    break;
                case THEME_DARK_GRID_STUD_COLOR:
                case THEME_DEFAULT_GRID_STUD_COLOR:
                    key = LC_PROFILE_GRID_STUD_COLOR;
                    break;
                case THEME_DARK_GRID_LINE_COLOR:
                case THEME_DEFAULT_GRID_LINE_COLOR:
                    key = LC_PROFILE_GRID_LINE_COLOR;
                    break;
                case THEME_DARK_TEXT_COLOR:
                case THEME_DEFAULT_TEXT_COLOR:
                    key = LC_PROFILE_TEXT_COLOR;
                    break;
                case THEME_DARK_VIEW_SPHERE_COLOR:
                case THEME_DEFAULT_VIEW_SPHERE_COLOR:
                    key = LC_PROFILE_VIEW_SPHERE_COLOR;
                    break;
                case THEME_DARK_VIEW_SPHERE_TEXT_COLOR:
                case THEME_DEFAULT_VIEW_SPHERE_TEXT_COLOR:
                    key = LC_PROFILE_VIEW_SPHERE_TEXT_COLOR;
                    break;
                case THEME_DARK_VIEW_SPHERE_HLIGHT_COLOR:
                case THEME_DEFAULT_VIEW_SPHERE_HLIGHT_COLOR:
                    key = LC_PROFILE_VIEW_SPHERE_HIGHLIGHT_COLOR;
                    break;
                case THEME_DARK_OBJECT_SELECTED_COLOR:
                case THEME_DEFAULT_OBJECT_SELECTED_COLOR:
                    key = LC_PROFILE_OBJECT_SELECTED_COLOR;
                    break;
                case THEME_DARK_OBJECT_FOCUSED_COLOR:
                case THEME_DEFAULT_OBJECT_FOCUSED_COLOR:
                    key = LC_PROFILE_OBJECT_FOCUSED_COLOR;
                    break;
                case THEME_DARK_CAMERA_COLOR:
                case THEME_DEFAULT_CAMERA_COLOR:
                    key = LC_PROFILE_CAMERA_COLOR;
                    break;
                case THEME_DARK_LIGHT_COLOR:
                case THEME_DEFAULT_LIGHT_COLOR:
                    key = LC_PROFILE_LIGHT_COLOR;
                    break;
                case THEME_DARK_CONTROL_POINT_COLOR:
                case THEME_DEFAULT_CONTROL_POINT_COLOR:
                    key = LC_PROFILE_CONTROL_POINT_COLOR;
                    break;
                case THEME_DARK_CONTROL_POINT_FOCUSED_COLOR:
                case THEME_DEFAULT_CONTROL_POINT_FOCUSED_COLOR:
                    key = LC_PROFILE_CONTROL_POINT_FOCUSED_COLOR;
                    break;
                case THEME_DARK_BM_OBJECT_SELECTED_COLOR:
                case THEME_DEFAULT_BM_OBJECT_SELECTED_COLOR:
                    key = LC_PROFILE_BM_OBJECT_SELECTED_COLOR;
                    break;
                default:
                    break;
                }

                if (key > LC_PROFILE_KEY(-1)) {
                    int alpha = 255;
                    switch (key)
                    {
                    case LC_PROFILE_GRID_STUD_COLOR:
                        alpha = 192;
                        break;
                    case LC_PROFILE_MARQUEE_FILL_COLOR:
                        alpha = 64;
                        break;
                    case LC_PROFILE_CONTROL_POINT_COLOR:
                    case LC_PROFILE_CONTROL_POINT_FOCUSED_COLOR:
                        alpha = 128;
                    default:
                        break;
                    }

                    QColor tc     = QColor(mThemeColors[i]);
                    quint32 color = qint32(LC_RGBA(tc.red(), tc.green(), tc.blue(), alpha));
                    switch (key)
                    {
                    case LC_PROFILE_OBJECT_SELECTED_COLOR:
                    case LC_PROFILE_OBJECT_FOCUSED_COLOR:
                    case LC_PROFILE_CAMERA_COLOR:
                    case LC_PROFILE_LIGHT_COLOR:
                    case LC_PROFILE_CONTROL_POINT_COLOR:
                    case LC_PROFILE_CONTROL_POINT_FOCUSED_COLOR:
                    case LC_PROFILE_BM_OBJECT_SELECTED_COLOR:
                        lcSetProfileUInt(key, color);
                        break;
                    default:
                        lcSetProfileInt(key, color);
                    }

#ifdef QT_DEBUG_MODE
                    emit gui->messageSig(LOG_DEBUG, QString("Display theme color set LC_PROFILE_KEY(%1): %2").arg(key).arg(color));
#endif
                }
            }
        }
    }

    return editedColors;
}

void ThemeColorsDialog::resetThemeColors()
{
    if (QMessageBox::Yes == QMessageBox::question(dialog, tr("Reset Theme Colors ?"),
                          tr("Reset all Theme colors to defaults.<br>Are you sure ?"),
                          QMessageBox::Yes|QMessageBox::No)) {
        for (int i = 0; i < THEME_NUM_COLORS; i++) {
            if (mThemeColors[i] == Preferences::defaultThemeColors[i].color)
                continue;
            else
                mThemeColors[i] = Preferences::defaultThemeColors[i].color;
            QPixmap pix(12, 12);
            pix.fill(getColor(i, mThemeColors[i]));
            colorButtonList[i]->setIcon(pix);
        }
    }
}

void ThemeColorsDialog::resetThemeColor()
{
    QObject* button = sender();
    int index = button->property("index").toInt();
    if (mThemeColors[index] == Preferences::themeColors[index])
       return;

    mThemeColors[index] = Preferences::themeColors[index];

    QPixmap pix(12, 12);
    pix.fill(getColor(index, (mThemeColors[index])));
    colorButtonList[index]->setIcon(pix);

    if (mThemeColors[index] == Preferences::defaultThemeColors[index].color) {
        resetButtonList[index]->setToolTip(tr("Reset"));
        resetButtonList[index]->setEnabled(false);
    } else {
        QImage image(16, 16, QImage::Format_RGB888);
        image.fill(getColor(index, Preferences::defaultThemeColors[index].color));
        QPainter painter(&image);
        painter.setPen(Qt::darkGray);
        painter.drawRect(0, 0, image.width() - 1, image.height() - 1);
        painter.end();
        QByteArray ba;
        QBuffer buffer(&ba);
        buffer.open(QIODevice::WriteOnly);
        image.save(&buffer, "PNG");
        buffer.close();
        const char* format = "<table><tr><td>Reset To %1 </td><td style=\"vertical-align:middle\"><img src=\"data:image/png;base64,%2\"/></td></tr></table>";
        QString text = QString(format).arg(Preferences::defaultThemeColors[index].color.toUpper(), QString(buffer.data().toBase64()));
        resetButtonList[index]->setToolTip(text);
        resetButtonList[index]->setEnabled(true);
    }
}

void ThemeColorsDialog::setThemeColor()
{
    QObject* button = sender();
    QString title = tr("Select Theme Color");

    int index = button->property("index").toInt();

    QColorDialog::ColorDialogOptions dialogOptions = QColorDialog::ShowAlphaChannel;
    QColor oldColor  = getColor(index, button->property("color").toString());
    QColor newColor  = QColorDialog::getColor(oldColor, dialog, title, dialogOptions);

    if (newColor.isValid())
        newColor = getColor(index, newColor.name());
    else
        return;

    if (newColor == oldColor)
        return;

    mThemeColors[index] = newColor.name().toUpper();

    QPixmap pix(12, 12);
    pix.fill(newColor);
    ((QToolButton*)button)->setIcon(pix);

    QImage image(16, 16, QImage::Format_RGB888);
    image.fill(oldColor);
    QPainter painter(&image);
    painter.setPen(Qt::darkGray);
    painter.drawRect(0, 0, image.width() - 1, image.height() - 1);
    painter.end();
    QByteArray ba;
    QBuffer buffer(&ba);
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, "PNG");
    buffer.close();
    const char* format = "<table><tr><td>Reset To %1</td><td style=\"vertical-align:middle\"><img src=\"data:image/png;base64,%2\"/></td></tr></table>";
    QString text = QString(format).arg(mThemeColors[index].toUpper(), QString(buffer.data().toBase64()));
    resetButtonList[index]->setToolTip(text);
    resetButtonList[index]->setEnabled(true);
}

QColor ThemeColorsDialog::getColor(const int index, const QString &name)
{
    // treat obaque colours
    QColor color(name);
    switch (index)
    {
    case THEME_DARK_GRID_STUD_COLOR:
    case THEME_DEFAULT_GRID_STUD_COLOR:
        color.setAlpha(192);
        break;
    case THEME_DEFAULT_LINE_SELECT:
        color.setAlpha(30);
        break;
    case THEME_DARK_LINE_HIGHLIGHT_EDITOR_SELECT:
    case THEME_DEFAULT_LINE_HIGHLIGHT_EDITOR_SELECT:
    case THEME_DARK_LINE_HIGHLIGHT_VIEWER_SELECT:
    case THEME_DEFAULT_LINE_HIGHLIGHT_VIEWER_SELECT:
        color = color.lighter(180);
        break;
    case THEME_DARK_MARQUEE_FILL_COLOR:
    case THEME_DEFAULT_MARQUEE_FILL_COLOR:
        color.setAlpha(64);
        break;
    case THEME_DARK_CONTROL_POINT_COLOR:
    case THEME_DEFAULT_CONTROL_POINT_COLOR:
    case THEME_DARK_CONTROL_POINT_FOCUSED_COLOR:
    case THEME_DEFAULT_CONTROL_POINT_FOCUSED_COLOR:
        color.setAlpha(128);
    default:
        break;
    }
    if (Preferences::displayTheme == THEME_DARK && index == THEME_DARK_LINE_ERROR) {
        color = color.lighter(180);
    }
    return color;
}

void ThemeColorsDialog::toggleDefaultsTab()
{
    if (tabs->count() < 3) {

        QWidget *container = new QWidget(tabs);
        QGridLayout *gridLayout = new QGridLayout(container);
        gridLayout->setSizeConstraint(QLayout::SetFixedSize);
        container->setLayout(gridLayout);

        QScrollArea *scrollArea = new QScrollArea(tabs);
        scrollArea->setWidget(container);

        QPalette readOnlyPalette = QApplication::palette();
        if (Preferences::displayTheme == THEME_DARK)
            readOnlyPalette.setColor(QPalette::Base,QColor(Preferences::themeColors[THEME_DARK_PALETTE_MIDLIGHT]));
        else
            readOnlyPalette.setColor(QPalette::Base,QColor(Preferences::themeColors[THEME_DEFAULT_PALETTE_LIGHT]));
        readOnlyPalette.setColor(QPalette::Text,QColor(LPUB3D_DISABLED_TEXT_COLOUR));
        for (int i = THEME_DEFAULT_SCENE_BACKGROUND_COLOR; i < THEME_NUM_COLORS; i++) {
            QString labelText = Preferences::defaultThemeColors[i].label;
            if (i < THEME_DARK_SCENE_BACKGROUND_COLOR)
                labelText.prepend("Default ");
            else
                labelText.prepend("Dark ");
            // description label
            QLabel *label = new QLabel(labelText, container);
            gridLayout->addWidget(label,i,0);

            // default color label
            label = new QLabel(container);
            label->setFixedWidth(16);
            label->setFrameStyle(QFrame::Sunken|QFrame::Panel);
            QColor color = getColor(i, Preferences::defaultThemeColors[i].color);
            QString styleSheet =
                    QString("QLabel { background-color: %1; }")
                    .arg(!color.isValid() ? "transparent" :
                         QString("rgba(%1, %2, %3, %4)")
                         .arg(color.red()).arg(color.green()).arg(color.blue()).arg(color.alpha()));
            label->setAutoFillBackground(true);
            label->setStyleSheet(styleSheet);
            gridLayout->addWidget(label,i,1);

            // default color hex (readOnly
            QLineEdit *readOnlyColor = new QLineEdit(container);
            readOnlyColor->setPalette(readOnlyPalette);
            readOnlyColor->setReadOnly(true);
            readOnlyColor->setText(Preferences::defaultThemeColors[i].color.toUpper());
            gridLayout->addWidget(readOnlyColor,i,2);
        }

        scrollArea->setMinimumWidth(
                    container->sizeHint().width() +
                    2 * scrollArea->frameWidth() +
                    scrollArea->verticalScrollBar()->sizeHint().width());

        tabs->addTab(scrollArea,"Defaults");
        tabs->setCurrentIndex(2);
    } else {
        tabs->removeTab(2);
        tabs->setCurrentIndex(Preferences::displayTheme == THEME_DARK ? 1 : 0);
    }
    dialog->adjustSize();
}

/***********************************
 * SHORTCUTS
 ***********************************/

bool PreferencesDialog::eventFilter(QObject *object, QEvent *event)
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
        ui.shortcutEdit->setText(ks.toString(QKeySequence::NativeText));
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

void PreferencesDialog::on_shortcutAssign_clicked()
{
    QTreeWidgetItem* CurrentItem = ui.commandList->currentItem();

    if (!CurrentItem || !CurrentItem->data(0, Qt::UserRole).isValid())
        return;

    const QString objectName = qvariant_cast<QString>(CurrentItem->data(0, Qt::UserRole));

    if (!lpub->actions.contains(objectName))
        return;

    QString NewShortcut = ui.shortcutEdit->text();

    if (!isValidKeyboardShortcut(objectName, NewShortcut))
        return;

    CurrentItem->setText(1, NewShortcut);
    CurrentItem->setData(1, Qt::UserRole, QVariant::fromValue(QKeySequence(NewShortcut)));

    mOptions->KeyboardShortcuts.insert(objectName, QKeySequence(NewShortcut));

    setShortcutModified(CurrentItem, QKeySequence(NewShortcut) != lpub->actions[objectName].action->shortcut());

    mOptions->KeyboardShortcutsModified = true;
    mOptions->KeyboardShortcutsDefault = false;
}

void PreferencesDialog::on_shortcutRemove_clicked()
{
    ui.shortcutEdit->setText(QString());

    on_shortcutAssign_clicked();
}

void PreferencesDialog::on_shortcutsImport_clicked()
{
    static const QString defaultFileName(QDir::currentPath() + QDir::separator() + VER_PRODUCTNAME_STR + "_Shortcuts.txt");

    QString FileName = QFileDialog::getOpenFileName(this, tr("Import shortcuts"), defaultFileName, tr("Text Files (*.txt);;All Files (*.*)"));

    if (FileName.isEmpty())
        return;

    if (!LoadKeyboardShortcuts(FileName)) {
        emit lpub->messageSig(LOG_ERROR, tr("Error loading keyboard shortcuts file."));
        return;
    }

    mOptions->KeyboardShortcutsModified = true;
    mOptions->KeyboardShortcutsDefault = false;
}

void PreferencesDialog::on_shortcutsExport_clicked()
{
    static const QString defaultFileName(QDir::currentPath() + QDir::separator() + VER_PRODUCTNAME_STR + "_Shortcuts.txt");

    QString FileName = QFileDialog::getSaveFileName(this, tr("Export shortcuts"), defaultFileName, tr("Text Files (*.txt);;All Files (*.*)"));

    if (FileName.isEmpty())
        return;

    int Count = 0;
    if (!SaveKeyboardShortcuts(FileName, Count)) {
        emit lpub->messageSig(LOG_ERROR, tr("Error saving keyboard shortcuts file."));
        return;
    }

    //display completion message
    QPixmap _icon = QPixmap(":/icons/lpub96.png");
    QMessageBoxResizable box;
    box.setWindowIcon(QIcon());
    box.setIconPixmap (_icon);
    box.setTextFormat (Qt::RichText);
    box.setStandardButtons (QMessageBox::Yes| QMessageBox::No);
    box.setDefaultButton   (QMessageBox::Yes);
    box.setWindowFlags (Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
    box.setWindowTitle(tr ("Export Keyboard Shortcuts"));
    box.setText (tr ("<b> %1 Shortcuts Exported </b>").arg(Count));
    box.setInformativeText (tr ("Do you want to open this document ?\n\n%1")
                                .arg(QDir::toNativeSeparators(FileName)));

    if (box.exec() == QMessageBox::Yes) {
        QString CommandPath = FileName;
        QProcess *Process = new QProcess(this);
        Process->setWorkingDirectory(QFileInfo(FileName).absolutePath() + QDir::separator());

#ifdef Q_OS_WIN
        Process->setNativeArguments(CommandPath);
        QDesktopServices::openUrl((QUrl("file:///"+CommandPath, QUrl::TolerantMode)));
#else
        Process->execute(CommandPath);
        Process->waitForFinished();

        QProcess::ExitStatus Status = Process->exitStatus();

        if (Status != 0) {  // look for error
            QErrorMessage *m = new QErrorMessage(this);
            m->showMessage(QString("%1\n%2").arg("Failed to launch PDF document!").arg(CommandPath));
        }
#endif
    }
}

void PreferencesDialog::on_shortcutsReset_clicked()
{
    if (QMessageBox::question(this, QLatin1Literal(VER_PRODUCTNAME_STR), tr("Are you sure you want to load the default keyboard shortcuts?"), QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
        return;

    updateCommandList(true/*Default shortucts*/);

    mOptions->KeyboardShortcutsModified = true;
    mOptions->KeyboardShortcutsDefault = true;
}

void PreferencesDialog::on_KeyboardFilterEdit_textEdited(const QString& Text)
{
    if (Text.isEmpty()) {
        std::function<void(QTreeWidgetItem*)> ShowItems = [&ShowItems](QTreeWidgetItem* ParentItem)
        {
            for (int ChildIdx = 0; ChildIdx < ParentItem->childCount(); ChildIdx++)
                ShowItems(ParentItem->child(ChildIdx));

            ParentItem->setHidden(false);
        };

        ShowItems(ui.commandList->invisibleRootItem());
    } else {
        std::function<bool(QTreeWidgetItem*,bool)> ShowItems = [&ShowItems, &Text](QTreeWidgetItem* ParentItem, bool ForceVisible)
        {
            ForceVisible |= (bool)ParentItem->text(0).contains(Text, Qt::CaseInsensitive) | (bool)ParentItem->text(1).contains(Text, Qt::CaseInsensitive);
            bool Visible = ForceVisible;

            for (int ChildIdx = 0; ChildIdx < ParentItem->childCount(); ChildIdx++)
                Visible |= ShowItems(ParentItem->child(ChildIdx), ForceVisible);

            ParentItem->setHidden(!Visible);

            return Visible;
        };

        ShowItems(ui.commandList->invisibleRootItem(), false);
    }
}

void PreferencesDialog::updateCommandList(bool loadDefaultShortcuts)
{
    QElapsedTimer timer;
    timer.start();

    int count = 0;

    ui.commandList->clear();
    QMap<QString, QTreeWidgetItem*> sections;

    QMap<QString, Action>::const_iterator it = lpub->actions.constBegin();
    while (it != lpub->actions.constEnd()) {
        if (it.value().id.isEmpty()) {
            ++it;
            continue;
        }

        bool idOk = true;
        int parentID = it.key().split(".").last().toInt(&idOk);

        if (!idOk)
            parentID = 0;

        const QString parentSection = shortcutParentNames[parentID];
        const QString mID = QString(".%1").arg(parentID);

        if (!sections.contains(parentSection)) {
            QTreeWidgetItem *parentItem = new QTreeWidgetItem(ui.commandList, QStringList(parentSection));
            QFont f = parentItem->font(0);
            f.setBold(true);
            parentItem->setFont(0, f);
            sections.insert(parentSection, parentItem);
            ui.commandList->expandItem(parentItem);
        }

        const QString identifier = QString(it.value().id).replace(" ", "");

        int pos = identifier.indexOf(QLatin1Char('.'));
        int subPos = identifier.indexOf(QLatin1Char('.'), pos + 1);
        int subSubPos = identifier.indexOf(QLatin1Char('.'), subPos + 1);
        bool noSubSub = false;
        if (subPos == -1)
            subPos = pos;
        if ((noSubSub = subSubPos == -1))
            subSubPos = subPos;

        const QString childSection = identifier.left(pos);

        if (subPos != pos) {
            if (!sections.contains(childSection+mID))
            {
                QTreeWidgetItem *parent = sections[parentSection];
                QTreeWidgetItem *childItem;
                if (parent)
                    childItem = new QTreeWidgetItem(parent, QStringList(childSection));
                else
                    childItem = new QTreeWidgetItem(ui.commandList, QStringList(childSection));

                QFont f = childItem->font(0);
                f.setBold(true);
                childItem->setFont(0, f);
                sections.insert(childSection+mID, childItem);
                ui.commandList->expandItem(childItem);
            }
        }

        const QString grandchildSection = identifier.left(subPos);

        if (subSubPos != subPos) {
            if (!sections.contains(grandchildSection+mID)) {
                QTreeWidgetItem *child = sections[childSection+mID];
                QTreeWidgetItem *parent = sections[parentSection];
                QTreeWidgetItem *grandchildItem;

                if (child)
                    grandchildItem = new QTreeWidgetItem(child, QStringList(grandchildSection));
                else if (parent)
                    grandchildItem = new QTreeWidgetItem(parent, QStringList(grandchildSection));
                else
                    grandchildItem = new QTreeWidgetItem(ui.commandList, QStringList(grandchildSection));

                QFont f = grandchildItem->font(0);
                f.setBold(true);
                grandchildItem->setFont(0, f);
                sections.insert(grandchildSection+mID, grandchildItem);
                ui.commandList->expandItem(grandchildItem);
            }
        }

        const QString section = identifier.left(subSubPos);

        if (!sections.contains(section+mID)) {
            QString categorySection;
            const int begin = noSubSub ? pos : subPos;
            const int end   = noSubSub ? subPos : subSubPos;
            if (begin != end)
                categorySection = identifier.mid(begin + 1, end - begin - 1);
            else
                categorySection = section;

            QTreeWidgetItem *grandchild  = sections[grandchildSection+mID];
            QTreeWidgetItem *child = sections[childSection+mID];
            QTreeWidgetItem *parent = sections[parentSection];
            QTreeWidgetItem *categoryItem;
            if (grandchild)
                categoryItem = new QTreeWidgetItem(grandchild, QStringList(categorySection));
            else if (child)
                categoryItem = new QTreeWidgetItem(child, QStringList(categorySection));
            else if (parent)
                categoryItem = new QTreeWidgetItem(parent, QStringList(categorySection));
            else
                categoryItem = new QTreeWidgetItem(ui.commandList, QStringList(categorySection));

            QFont f = categoryItem->font(0);
            f.setBold(true);
            categoryItem->setFont(0, f);
            sections.insert(section+mID, categoryItem);
            ui.commandList->expandItem(categoryItem);
        }

        QStringList headerLabels;
        headerLabels.append(tr("Action"));
        headerLabels.append(tr("Shortcut"));
        headerLabels.append(tr("Default"));
        headerLabels.append(tr("Icon"));
//#ifdef QT_DEBUG_MODE
//        headerLabels.append(tr("Key"));
//        headerLabels.append(tr("ID"));
//#endif
        ui.commandList->setColumnCount(headerLabels.count());
        ui.commandList->setHeaderLabels(headerLabels);

        QKeySequence shortcutDefault(it.value().action->property("defaultshortcut").value<QKeySequence>());
        QKeySequence shortcut(loadDefaultShortcuts ? shortcutDefault : it.value().action->shortcut());

        QTreeWidgetItem *item = new QTreeWidgetItem;
        item->setToolTip(0, qApp->translate("Menu", it.value().action->statusTip().toLatin1().constData()));
        item->setData(   0, Qt::UserRole, QVariant::fromValue(it.key()));
        item->setText(   0, qApp->translate("Menu", it.value().action->text().toLatin1().constData()).remove('&').remove(QLatin1String("...")));
        item->setData(   1, Qt::UserRole, QVariant::fromValue(shortcut));
        item->setText(   1, shortcut.toString(QKeySequence::NativeText));
        item->setText(   2, shortcutDefault.toString(QKeySequence::NativeText));
        item->setIcon(   3, it.value().action->icon());
//#ifdef QT_DEBUG_MODE
//        item->setText(4, it.key());
//        item->setText(5, it.value().id);
//#endif

        QColor textColor = QColor(Preferences::themeColors[THEME_DARK_PALETTE_MIDLIGHT]);
        if (Preferences::displayTheme == THEME_DARK)
            textColor = QColor(Preferences::themeColors[THEME_DEFAULT_PALETTE_LIGHT]);
        QBrush b (textColor);
        item->setForeground(0, b);
        item->setForeground(2, b);
//#ifdef QT_DEBUG_MODE
//        item->setForeground(4, b);
//        item->setForeground(5, b);
//#endif

        if (!loadDefaultShortcuts) {
            const bool isModified = shortcut != shortcutDefault;

            QString result;
//#ifdef QT_DEBUG_MODE
//            //superfluous, and expensive, call to enable permanently, but can be useful during development
//            if (!shortcut.isEmpty() && !isValidKeyboardShortcut(it.key(), shortcut.toString(QKeySequence::NativeText), result, true/*loading*/))
//                item->setToolTip(1, result);
//#endif
            const bool isInvalid = !result.isEmpty();

            if (isModified || isInvalid)
                setShortcutModified(item, true, isInvalid);
        }

        sections[section+mID]->addChild(item);
        ++it;
        ++count;
    }
#ifdef QT_DEBUG_MODE
    emit gui->messageSig(LOG_NOTICE, QString("Loaded %1 %2. %3")
                                             .arg(count).arg(loadDefaultShortcuts ? tr("default shortcuts") : tr("shortcuts"))
                                             .arg(lpub->elapsedTime(timer.elapsed())));
#endif
}

void PreferencesDialog::setShortcutModified(QTreeWidgetItem *treeItem, bool modified, bool invalid)
{
    QFont font = treeItem->font(0);
    font.setItalic(modified);
    treeItem->setFont(0, font);
    font.setBold(modified);
    treeItem->setFont(1, font);
    if (invalid) {
        QBrush b (Qt::red);
        treeItem->setForeground(1, b);
        treeItem->setFont(1, font);
    }
}

void PreferencesDialog::shortcutEditReset()
{
    if (ui.shortcutEdit) {
        QTreeWidgetItem* currentItem = ui.commandList->currentItem();
        if (!currentItem || !currentItem->data(1, Qt::UserRole).isValid()) {
            ui.shortcutEdit->setText(QString());
            ui.KeyboardShortcutGroup->setEnabled(false);
            lpub->getAct("setShortcutEditResetAct.8")->setEnabled(false);
            return;
        }

        QKeySequence shortcut(qvariant_cast<QKeySequence>(currentItem->data(1, Qt::UserRole)));
        ui.shortcutEdit->setText(shortcut.toString(QKeySequence::NativeText));
        lpub->getAct("setShortcutEditResetAct.8")->setEnabled(false);
    }
}

void PreferencesDialog::enableShortcutEditReset(const QString &displayText)
{
    if (ui.shortcutEdit) {
        QTreeWidgetItem* currentItem = ui.commandList->currentItem();
        if (!currentItem || !currentItem->data(1, Qt::UserRole).isValid()) {
            lpub->getAct("setShortcutEditResetAct.8")->setEnabled(false);
            return;
        }

        QKeySequence shortcut(qvariant_cast<QKeySequence>(currentItem->data(1, Qt::UserRole)));
        lpub->getAct("setShortcutEditResetAct.8")->setEnabled(displayText != shortcut.toString(QKeySequence::NativeText));
    }
}

void PreferencesDialog::commandChanged(QTreeWidgetItem *current)
{
    if (!current || !current->data(1, Qt::UserRole).isValid()) {
        ui.shortcutEdit->setText(QString());
        ui.KeyboardShortcutGroup->setEnabled(false);
        lpub->getAct("setShortcutEditResetAct.8")->setEnabled(false);
        return;
    }

    ui.KeyboardShortcutGroup->setEnabled(true);

    QKeySequence shortcut(qvariant_cast<QKeySequence>(current->data(1, Qt::UserRole)));
    ui.shortcutEdit->setText(shortcut.toString(QKeySequence::NativeText));
    lpub->getAct("setShortcutEditResetAct.8")->setEnabled(false);
}

bool PreferencesDialog::isValidKeyboardShortcut(const QString &ObjectName, const QString &NewShortcut)
{
   QString NotUsed;
   return isValidKeyboardShortcut(ObjectName, NewShortcut, NotUsed, false /*loading*/);
}

bool PreferencesDialog::isValidKeyboardShortcut(const QString &ObjectName, const QString &NewShortcut, QString &ResultText, bool Loading)
{
    bool idOk = true;
    int newPID = ObjectName.split(".").last().toInt(&idOk);

    if (!idOk)
        newPID = 0;

    if (!NewShortcut.isEmpty()) {
        uint ExistingIndex = 0;

        QMap<QString, Action>::iterator it = lpub->actions.begin();
        while (it != lpub->actions.end()) {

            bool idOk = true;
            int curPID = it.key().split(".").last().toInt(&idOk);

            if (!idOk)
                curPID = 0;

            if (NewShortcut == it.value().action->shortcut().toString(QKeySequence::NativeText) && it.key() != ObjectName && newPID == curPID) {
                QString ActionText = qApp->translate("Menu", it.value().action->text().toLatin1().constData()).remove('&').remove(QLatin1String("..."));
                ResultText = tr("The shortcut '%1' is already assigned to '%2'.").arg(NewShortcut, ActionText);
                QString QuestionText = tr("%1 Do you want to replace it?").arg(ResultText).arg(NewShortcut, ActionText);

                if (Loading || QMessageBox::question(this, tr("Override Shortcut"), QuestionText, QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
                    return false;

                mOptions->KeyboardShortcuts.insert(ObjectName, QKeySequence(NewShortcut));

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

                QTreeWidgetItem* ExistingItem = FindItem(ui.commandList->invisibleRootItem());

                if (ExistingItem)
                {
                    ExistingItem->setText(1, QString());
                    const QKeySequence &defaultKeySeq = it.value().action->property("defaultshortcut").value<QKeySequence>();
                    setShortcutModified(ExistingItem, defaultKeySeq.toString(QKeySequence::NativeText)[0] != 0);
                }
            }
            ++it;
            ++ExistingIndex;
        }
    }

    return true;
}

bool PreferencesDialog::SaveKeyboardShortcuts(const QString& FileName, int &Count)
{
    static const QLatin1String FmtDateTime("yyyy-MM-dd hh:mm:ss");

    static const QLatin1String DocumentTitle("Keyboard Shortcuts");

    const QString GeneratedOn = tr("%1 %2 - Generated on %3")
            .arg(VER_PRODUCTNAME_STR)
            .arg(DocumentTitle)
            .arg(QDateTime::currentDateTime().toString(FmtDateTime));

    QFile File(FileName);

    if (!File.open(QIODevice::WriteOnly))
        return false;

    QTextStream Stream(&File);

    Stream << GeneratedOn << QLatin1String("\n\n");

    return SaveKeyboardShortcuts(Stream, Count);
}

bool PreferencesDialog::SaveKeyboardShortcuts(QTextStream& Stream, int &Count)
{
    if (!mOptions->KeyboardShortcuts.isEmpty()) {
        if (QMessageBox::question(this, QLatin1Literal(VER_PRODUCTNAME_STR), tr("Do you want to save un-committed keyboard shortcuts?"), QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
            QMap<QString, QKeySequence>::const_iterator it = mOptions->KeyboardShortcuts.constBegin();
            while (it != mOptions->KeyboardShortcuts.constEnd()) {
                if (lpub->actions.contains(it.key()))
                    lpub->actions[it.key()].action->setShortcut(it.value());
                ++it;
            }
        }
    }

    QMap<QString, Action>::const_iterator it = lpub->actions.constBegin();
    while (it != lpub->actions.constEnd()) {
        if (it.value().action->shortcut().isEmpty()) {
            ++it;
            continue;
        }

        bool idOk = true;
        int parentID = it.key().split(".").last().toInt(&idOk);

        if (!idOk)
            parentID = 0;

        const QString parentName = shortcutParentNames[parentID];
        const QString shortcutName = QString("%1.%2").arg(it.key().split(".").first()).arg(parentName);
        const QString shortcutKey = it.value().action->shortcut().toString(QKeySequence::NativeText);

        Stream << shortcutName << QLatin1String("=") << shortcutKey << QLatin1String("\n");
        ++it;
        ++Count;
    }

    Stream << QLatin1String("\n") << QString("End of file.") << QLatin1String("\n");

    Stream.flush();

    return true;
}

bool PreferencesDialog::LoadKeyboardShortcuts(const QString& FileName)
{
    QFile File(FileName);

    if (!File.open(QIODevice::ReadOnly))
        return false;

    QTextStream Stream(&File);

    return LoadKeyboardShortcuts(Stream);
}

bool PreferencesDialog::LoadKeyboardShortcuts(QTextStream& Stream)
{
    QHash<QString, int> ParentNameMap;
    if (ParentNameMap.size() == 0) {
        ParentNameMap[shortcutParentNames[NO_ACTION]]                 = NO_ACTION;                 //  0
        ParentNameMap[shortcutParentNames[MAIN_WINDOW_ACTION]]        = MAIN_WINDOW_ACTION;        //  1
        ParentNameMap[shortcutParentNames[COMMAND_EDITOR_ACTION]]     = COMMAND_EDITOR_ACTION;     //  2
        ParentNameMap[shortcutParentNames[TEXT_EDITOR_ACTION]]        = TEXT_EDITOR_ACTION;        //  3
        ParentNameMap[shortcutParentNames[VISUAL_EDITOR_ACTION]]      = VISUAL_EDITOR_ACTION;      //  4
        ParentNameMap[shortcutParentNames[PARAMS_EDITOR_ACTION]]      = PARAMS_EDITOR_ACTION;      //  5
        ParentNameMap[shortcutParentNames[COMMANDS_DIALOG_ACTION]]    = COMMANDS_DIALOG_ACTION;    //  6
        ParentNameMap[shortcutParentNames[COMMANDS_TEXT_EDIT_ACTION]] = COMMANDS_TEXT_EDIT_ACTION; //  7
        ParentNameMap[shortcutParentNames[OTHER_ACTION]]              = OTHER_ACTION;              //  8
    }

    for (QString Line = Stream.readLine(); !Line.isNull(); Line = Stream.readLine()) {
        int Equals = Line.indexOf('=');

        if (Equals == -1)
            continue;

        const QStringList KS = Line.left(Equals).split(".");

        const QString Key = QString("%1.%2").arg(KS.first()).arg(ParentNameMap[KS.last()]);

        const QKeySequence Shortcut(Line.mid(Equals + 1));

        if (lpub->actions.contains(Key)) {

            if (!isValidKeyboardShortcut(Key, Shortcut.toString(QKeySequence::NativeText)))
                continue;

            mOptions->KeyboardShortcuts.insert(Key, Shortcut);
        } else {
            emit lpub->messageSig(LOG_ERROR, tr("The action %1 for this keyboard shortcut %2 was not found.")
                                  .arg(Key).arg(Shortcut.toString(QKeySequence::NativeText)));
        }
    }

    return true;
}
