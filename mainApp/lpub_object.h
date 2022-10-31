/****************************************************************************
**
** Copyright (C) 2018 - 2022 Trevor SANDY. All rights reserved.
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

#include "name.h"
#include "ldrawfiles.h"
#include "ranges.h"
#include "meta.h"

class Step;
class NativeOptions;
class lcHttpManager;
class QProgressDialog;
class QNetworkReply;
class QNetworkAccessManager;

class CommandCollection;
class SnippetCollection;

enum class lcShadingMode;

class LPub : public QObject
{
    Q_OBJECT

public:
  LPub();
  ~LPub();
  static QString elapsedTime(const qint64 &duration);

  /// Open project to enable native visual editor or image render
  bool OpenProject(const NativeOptions*, int = NATIVE_VIEW, bool = false);

  /// Flip page size per orientation and return size in pixels
  static int pageSize(PageMeta  &, int);

  /// Process viewer key to return model, line number and step number
  static QStringList getViewerStepKeys(bool = true, bool = false, const QString & = "");

  /// Stud sytle and automated edge color setting calls
  int     GetStudStyle();
  float   GetPartEdgeContrast();
  float   GetPartColorLightDarkIndex();
  bool    GetAutomateEdgeColor();
  quint32 GetStudCylinderColor();
  quint32 GetPartEdgeColor();
  quint32 GetBlackEdgeColor();
  quint32 GetDarkEdgeColor();

  void    SetStudStyle(const NativeOptions*, bool);
  void    SetAutomateEdgeColor(const NativeOptions*);
  void    SetShadingMode(lcShadingMode);

  /// Run command line execution
  int processCommandLine();

  /// Run visual editor command line execution
  int Process3DViewerCommandLine();

  bool setPreferredRendererFromCommand(const QString &);

  /// Fade and highlight settings from command line calls
  bool setFadeStepsFromCommand();
  bool setHighlightStepFromCommand();

  /// Current step management calls
  static bool extractStepKey(Where &, int &, const QString & = "");
  bool setCurrentStep(const QString & = "");
  void setCurrentStep(Step *, Where &, int, int = BM_SINGLE_STEP);
  void setCurrentStep(Step *);

  /// Visual Editor restart and reload calls
  void clearAndReloadModelFile(bool);
  void reloadCurrentPage();
  void restartApplication();

  /// LPub commands collection load call
  void loadCommandCollection();

  /// Export LPub meta commands to file
  bool exportMetaCommands(const QString &, QString &, bool = false);

  /// Download management calls
  void downloadFile(QString URL, QString, bool promptRedirect = false);
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

  /// meta command container
  Meta meta;

  /// meta command keywords;
  QStringList metaKeywords;

  /// Abstract version of page contents
  Page page;

  /// Current step as loaded in the Visual Editor
  Step *currentStep;

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

  /// Download management calls
  QProgressDialog *mProgressDialog;
  bool mPromptRedirect;
  bool mHttpRequestAborted;
  QUrl mUrl;

public slots:
  /// Download management calls
  void httpDownloadFinished();
  void cancelDownload();
  void updateDownloadProgress(qint64, qint64);
  void fileLoaded(bool b)
  {
      mFileLoaded = true;
      mFileLoadFail = !b;
  }

signals:
  void messageSig(LogType, QString);
  void loadFileSig(QString, bool);
  void consoleCommandSig(int, int*);

protected:
    /// Download management members
    QNetworkAccessManager* mHttpManager;
    QNetworkReply*         mHttpReply;
    QByteArray             mByteArray;
    QString                mTitle;

private:
    bool                   mFileLoaded;
    bool                   mFileLoadFail;
    static QString         commandlineFile;
};

extern class LPub *lpub;

inline Meta& getMetaRef()
{
    return lpub->meta;
}
#endif // LPUB_OBJECT_H
