/****************************************************************************
**
** Copyright (C) 2022 - 2024 Trevor SANDY. All rights reserved.
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

#ifndef LPUB_OBJECT_H
#define LPUB_OBJECT_H

#include <QObject>

#include "declarations.h"
#include "ldrawfiles.h"
#include "ranges.h"
#include "meta.h"

class Step;
class PartWorker;
class NativeOptions;
class TextEditDialog;
class lcHttpManager;
class QProgressDialog;
class QNetworkReply;
class QNetworkAccessManager;
class QSimpleUpdater;
class QSpinBox;
class QDoubleSpinBox;

class CommandsDialog;
class CommandsTextEdit;
class CommandCollection;
class SnippetCollection;

class PreferencesDialog;
class lcQPreferencesDialog;

enum class lcShadingMode;

class LPub : public QObject
{
  Q_OBJECT

public:
  LPub();
  ~LPub();
  static QString elapsedTime(const qint64 &duration, bool pretty = true);

  /// Open project to enable native visual editor or image render
  bool OpenProject(const NativeOptions*, int = NATIVE_VIEW, bool = false);

  /// Visual Editor viewpoint latitude longitude
  int SetViewpointLatLonDialog(bool SetCamera = false);

  /// Flip page size per orientation and return size in pixels
  static int pageSize(PageMeta  &, int);

  /// Process viewer key to return model, line number and step number
  static QStringList getViewerStepKeys(bool = true, bool = false, const QString & = "");
  static bool extractStepKey(Where &, int &, const QString & = "");

  /// Process specified path and return absolute path
  static QString getFilePath(const QString &fileName);

  /// Stud sytle and automated edge color setting calls
  int     GetStudStyle();
  float   GetPartEdgeContrast();
  float   GetPartColorLightDarkIndex();
  bool    GetAutomateEdgeColor();
  quint32 GetStudCylinderColor();
  quint32 GetPartEdgeColor();
  quint32 GetBlackEdgeColor();
  quint32 GetDarkEdgeColor();
  bool    GetStudCylinderColorEnabled();
  bool    GetPartEdgeColorEnabled();
  bool    GetBlackEdgeColorEnabled();
  bool    GetDarkEdgeColorEnabled();

  void    SetStudStyle(const NativeOptions*, bool, bool = false);
  void    SetAutomateEdgeColor(const NativeOptions*, bool = false);
  void    SetShadingMode(lcShadingMode);

  /// Load text preferences and command management dialogs
  void loadDialogs();
  void loadPreferencesDialog();

  /// Shortcuts
  void setShortcutKeywords();
  void setKeyboardShortcuts();
  void setDefaultKeyboardShortcuts();
  void setKeyboardShortcut(QMenu *menu);
  void setKeyboardShortcut(QAction *action);

  /// Action management
  QAction *getAct(const QString &objectName);

  /// Remove LPub formatting from document, submodel, page, step, callout and BOM
  void removeLPubFormatting(int option = 0/*Document*/);

  /// Run command line execution
  int processCommandLine();

  /// Run visual editor command line execution
  int Process3DViewerCommandLine();

  /// Preferred renderer from meta command
  bool setPreferredRendererFromCommand(const QString &);

  /// Fade and highlight settings from meta command
  bool setFadeStepsFromCommand();
  bool setHighlightStepFromCommand();

  /// Current step management calls
  bool setCurrentStep(const QString & = "");
  void setCurrentStep(Step *, Where &, int, int = BM_SINGLE_STEP);
  void setCurrentStep(Step *);

  /// determine if current step is display step
  bool CurrentStepIsDisplayModel();

  /// Visual Editor restart and reload calls
  void clearAndReloadModelFile(bool fileReload = false, bool savePrompt = false, bool keepWork = false);
  void reloadCurrentPage(bool savePrompt = false);
  void restartApplication(bool changeLibrary = false, bool prompt = false);

  /// LPub commands collection load
  void loadCommandCollection();

  /// LPub snippet collection load
  void loadSnippetCollection();

  /// Export LPub meta commands to file
  bool exportMetaCommands(const QString &, QString &, bool = false);

  /// Search directory thread worker
  PartWorker& partWorkerLDSearchDirs();

  /// Load banner
  static void loadBanner(const int &type);

  /// Download management public calls
  void downloadFile(QString URL,
                    QString,
                    bool promptRedirect = false,
                    bool showProgress = true);
  void startRequest(QUrl url);
  QByteArray getDownloadedFile() const
  {
    return mByteArray;
  }

  /// currently loaded model file
  static QString getCmdLineFile()
  {
    return commandlineFile;
  }

  /// Shortcut mangement
  QMap<QString, Action> actions;

  /// Shortcut id keywords
  QStringList shortcutIdKeywords;

  /// meta command container
  Meta meta;

  /// meta command keywords;
  QStringList metaKeywords;

  /// Abstract version of page contents
  Page page;

  /// Current step as loaded in the Visual Editor
  Step *currentStep = nullptr;

  /// utility functions for meta commands
  MetaItem mi;

  /// Contains MPD or all files used in model
  LDrawFile ldrawFile;

  /// User-modified command description collection
  CommandCollection *commandCollection = nullptr;

  /// Command snippets collection
  SnippetCollection *snippetCollection = nullptr;

  /// Currently loaded CSI in Visual Editor
  static QString viewerStepKey;

  /// Native export options
  NativeOptions *Options = nullptr;

  /// Download management public members
  QProgressDialog *mProgressDialog = nullptr;
  bool mPromptRedirect;
  bool mHttpRequestAborted;
  QUrl mUrl;

  /// Release notes and version updater
  static QString    DEFS_URL;
  static QString    m_versionInfo;
  QSimpleUpdater   *m_updater = nullptr;
  static bool       m_updaterCancelled;
  static bool       m_setReleaseNotesAsText;
  static QString    m_releaseNotesContent;

  /// Visual Editor viewpoint latitude longitude
  static int        ViewpointsComboSaveIndex;

  /// Text editor dialog
  TextEditDialog   *textEdit = nullptr;

  /// Commands dialog
  CommandsDialog   *commandsDialog = nullptr;

  /// Commands text editor
  CommandsTextEdit *commandTextEdit = nullptr;

  /// Snippet text editor
  CommandsTextEdit *snippetTextEdit = nullptr;

  /// LPub3D Preferences dialog
  PreferencesDialog *preferencesDialog = nullptr;

  /// Visual Editor Preferences dialog
  lcQPreferencesDialog *visualEditorPreferencesDialog = nullptr;

public slots:
  /// Visual editor transform
  void saveVisualEditorTransformSettings();

  /// Download management public slots
  void httpDownloadFinished();
  void cancelDownload();
  void updateDownloadProgress(qint64, qint64);
  void fileLoaded(bool b)
  {
    mFileLoaded = true;
    mFileLoadFail = !b;
  }
  
  /// Update change log call
  void setupChangeLogUpdate();
  void updaterCancelled();
  void updateChangelog(const QString &);

  /// Raise visual editor windows
  void raiseVisualEditDockWindow();
  void raisePreviewDockWindow();

  /// Visual Editor viewpoint latitude longitude
private slots:
  void latReset(bool);
  void lonReset(bool);
  void ddfReset(bool);
  void latResetEnabled(int);
  void lonResetEnabled(int);
  void ddfResetEnabled(double);
  void ddfSpinEnabled(bool);

signals:
  void messageSig(LogType, const QString &, int = 0);
  void loadFileSig(QString, bool);
  void consoleCommandSig(int, int*);
  void checkForUpdatesFinished();

protected:
  /// Download management protected members
  bool                   mShowProgress = true;
  QNetworkAccessManager* mHttpManager = nullptr;
  QNetworkReply*         mHttpReply = nullptr;
  QByteArray             mByteArray;
  QString                mTitle;

  /// Visual Editor viewpoint latitude longitude
  QSpinBox              *mLatSpin;
  QSpinBox              *mLonSpin;
  QDoubleSpinBox        *mDDFSpin;
  QPushButton           *mLatResetButton;
  QPushButton           *mLonResetButton;
  QPushButton           *mDDFResetButton;
  float                  mLatitude;
  float                  mLongitude;
  float                  mDistanceFactor;

  /// Visual editor transform
  bool                   mRotateTransform;
  bool                   mRelativeTransform;
  bool                   mSeparateTransform;

private:
  bool                   mFileLoaded;
  bool                   mFileLoadFail;
  static QString         commandlineFile;
};

extern const QString shortcutParentNames[];
extern class LPub *lpub;

inline Meta& getMetaRef()
{
  return lpub->meta;
}
#endif // LPUB_OBJECT_H
