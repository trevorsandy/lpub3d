/****************************************************************************
**
** Copyright (C) 2007-2009 Kevin Clague. All rights reserved.
** Copyright (C) 2015 - 2020 Trevor SANDY. All rights reserved.
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

/****************************************************************************
 *
 * This class encapsulates the external renderers.  For now, this means
 * only ldglite.
 *
 * Please see lpub.h for an overall description of how the files in LPub
 * make up the LPub program.
 *
 ***************************************************************************/

#ifndef RENDER_H
#define RENDER_H

#include <QString>
#include <QStringList>
#include "options.h"

class QString;
class QStringList;
class Meta;
class AssemMeta;
class LPubMeta;
class UnitsMeta;
class RotStepMeta;
class FloatPairMeta;
class NativeOptions;
class ViewerOptions;
class Project;

class Render
{
public:
  Render() {}
  virtual ~Render() {}
  static QString const   getRenderer();
  static bool            useLDViewSCall();
  static bool            useLDViewSList();
  static int             rendererTimeout();
  static void            setRenderer(QString const &);
  static bool            clipImage(QString const &);
  static QString const   getRotstepMeta(RotStepMeta &, bool isKey = false);
  static QString const   getPovrayRenderQuality(int quality = -1);
  static int             executeLDViewProcess(QStringList &, Options::Mt);
  static QString const   fixupDirname(const QString &);
  static QString const   getRenderImageFile(int);
  static QString const   getRenderModelFile(int);
  static float           getPovrayRenderCameraDistance(const QString &cdKeys);
  static void            showLdvExportSettings(int mode);
  static void            showLdvLDrawPreferences(int mode);
  static bool            RenderNativeImage(const NativeOptions *);
  static bool            NativeExport(const NativeOptions *);
  static float           ViewerCameraDistance(Meta &meta, float);
  static bool            ExecuteViewer(const NativeOptions *, bool Export = false);
  static bool            LoadViewer(const ViewerOptions *);
  static QStringList const getImageAttributes(const QString &);
  static bool            compareImageAttributes(const QStringList &,
                                                const QString &,
                                                bool pare = true);
  static bool            createSnapshotsList(const QStringList &,
                                            const QString &);
  static void            addArgument(QStringList &arguments,
                                     const QString &arg,
                                     const QString &argChk = QString(),
                                     const int povGenerator = 0,/*0=POV-Ray,1=POVGenerator*/
                                     const int additionalArgs = 1);
  static bool            doLDVCommand(const QStringList &args,
                                    int = -1 /*EXPORT_NONE*/,
                                    int = 6 /*NumIniFiles*/);
  static bool            LoadStepProject(Project *,
                                     const QString &);
  static int             createNativeModelFile(QStringList &csiParts,
                                     bool doFadeStep,
                                     bool doHighlightStep);
  static int             mergeNativeCSISubModels(QStringList &subModels,
                                     QStringList &subModelParts,
                                     bool doFadeStep,
                                     bool doHighlightStep);
  static void            setNativeHeaderAndNoFileMeta(QStringList &parts,
                                     const QString &modelName,
                                     bool pliPart,
                                     bool displayOnly);
  static int             rotateParts(const QString &addLine,
                                     RotStepMeta &rotStep,
                                     const QStringList &parts,
                                     QString &ldrName,
                                     const QString &modelName,
                                     FloatPairMeta &ca,
                                     bool ldv = false);
  static int             rotateParts(const QStringList &parts,
                                     QString &ldrName,
                                     const QString &rs,
                                     QString &ca);
  static int             rotateParts(const QString &addLine,
                                     RotStepMeta &rotStep,
                                     QStringList &parts,
                                     FloatPairMeta &ca,
                                     bool  applyCA = true);
  static int             splitIMParts(const QStringList &rotatedParts,
                                      const QString &rotsComment,
                                      const QString &ldrName,
                                      const QString &csiKey);
  virtual int               renderCsi(const QString &,
                                      const QStringList &,
                                      const QStringList &,
                                      const QString &,
                                      Meta &,
                                      int = 0) = 0;
  virtual int               renderPli(const QStringList &,
                                      const QString &,
                                      Meta &,
                                      int,
                                      int) = 0;

protected:
  virtual float        cameraDistance(Meta &meta, float) = 0;

  friend class Project;
  friend class Step;
};

class POVRay : public Render
{
public:
  POVRay() {}
  virtual ~POVRay() {}
  virtual int renderCsi(const QString &,  const QStringList &, const QStringList &, const QString &, Meta &, int = 0);
  virtual int renderPli(                  const QStringList &, const QString &, Meta &, int, int);
  virtual float cameraDistance(Meta &meta, float);
};

class LDGLite : public Render
{
public:
  LDGLite() {}
  virtual ~LDGLite() {}
  virtual int renderCsi(const QString &,  const QStringList &, const QStringList &,const QString &, Meta &, int = 0);
  virtual int renderPli(                  const QStringList &, const QString &, Meta &, int, int);
  virtual float cameraDistance(Meta &meta, float);
};

class LDView : public Render
{
public:
  LDView() {}
  virtual ~LDView() {}
  virtual int renderCsi(const QString &,  const QStringList &, const QStringList &, const QString &, Meta &, int = 0);
  virtual int renderPli(                  const QStringList &, const QString &, Meta &, int, int);
  virtual float cameraDistance(Meta &meta, float);
};

class Native : public Render
{
public:
  Native() {}
  virtual ~Native() {}
  virtual int renderCsi(const QString &,  const QStringList &, const QStringList &, const QString &, Meta &, int = 0);
  virtual int renderPli(                  const QStringList &, const QString &, Meta &, int, int);
  virtual float cameraDistance(Meta &meta, float);
};

inline void removeEmptyStrings(QStringList &l)
{
  l.removeAll({});
}

extern Render *renderer;
extern LDGLite ldglite;
extern LDView  ldview;
extern POVRay  povray;
extern Native  native;

#endif
