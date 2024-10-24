/****************************************************************************
**
** Copyright (C) 2007-2009 Kevin Clague. All rights reserved.
** Copyright (C) 2015 - 2024 Trevor SANDY. All rights reserved.
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
class Project;
class StudStyleMeta;
class AutoEdgeColorMeta;
class HighContrastColorMeta;

class Render
{
public:
  Render(){}
  virtual ~Render() {}
  static int             getRenderer();
  static bool            useLDViewSCall();
  static bool            useLDViewSList();
  static int             rendererTimeout();
  static int             getDistanceRendererIndex();
  static void            setRenderer(int);
  static bool            clipImage(QString const &);
  static QString const   getRotstepMeta(RotStepMeta &, bool isKey = false);
  static QString const   getPovrayRenderQuality(int quality = -1);
  static int             executeLDViewProcess(QStringList &, QStringList &, Options::Mt);
  static QString const   fixupDirname(const QString &);
  static QString const   getRenderImageFile(int);
  static QString const   getRenderModelFile(int, bool = true);
  static float           getPovrayRenderCameraDistance(const QString &cdKeys);
  static void            showLdvExportSettings(int mode);
  static void            showLdvLDrawPreferences(int mode);
  static bool            RenderNativeImage(const NativeOptions *);
  static bool            NativeExport(const NativeOptions *);
  static float           ViewerCameraDistance(Meta &meta, float);
  static bool            RenderNativeView(const NativeOptions *, bool RenderImage = false);
  static bool            LoadViewer(const NativeOptions *);
  static QStringList const getImageAttributes(const QString &);
  static QStringList splitParms(const QString &);
  static bool            compareImageAttributes(const QStringList &, const QString &, bool pare = true);
  static void            getStudStyleAndAutoEdgeSettings(StudStyleMeta *ssm,
                                      HighContrastColorMeta *hccm, AutoEdgeColorMeta *acm,
                                      QString &ss, QString &ae, QString &ac, QString &ai,
                                      QString &hs, QString &hp, QString &hb, QString &hd);

  static bool            createSnapshotsList(const QStringList &, const QString &);
  static void            addArgument(QStringList &_arguments,
                                     const QString &_newArg,
                                     const QString &_argChk = QString(),
                                     const int povGenerator = 0,/*0=POV-Ray,1=POVGenerator*/
                                     const int additionalArgs = 1,
                                     const bool quoteNewArg = false);
  static bool            doLDVCommand(const QStringList &args,
                                      int exportMode = -1 /*EXPORT_NONE*/,
                                      int = 6 /*NumIniFiles*/);
  static bool            isSingleSubfile(const QStringList &partLines);
  static int             createNativeModelFile(QStringList &rotatedParts,
                                     bool doFadeStep,
                                     bool doHighlightStep,
                                     int imageType = 0,
                                     bool singleSubfile = false);
  static bool            pruneNativeParts(QStringList &rotatedParts);
  static int             mergeNativeSubfiles(QStringList &subFiles,
                                     QStringList &subFileParts,
                                     bool doFadeStep,
                                     bool doHighlightStep,
                                     int imageType = 0);
  static int            mergeSubmodelContent(QStringList &);
  static int            setLDrawHeaderAndFooterMeta(QStringList &lines,
                                     const QString &modelName,
                                     int imageType,
                                     int displayType = 0);       /*DT_DEFAULT*/
  static int            rotatePartsRD(const QStringList &parts,   // RotateParts #1 - 5 parms - used exclusively by RenderDialog
                                     QString &ldrName,
                                     const QString &rs,
                                     QString &ca,
                                     int imageType);
  static int             rotateParts(const QString &addLine,     // RotateParts #2 - 8 parms
                                     RotStepMeta &rotStep,
                                     const QStringList &parts,
                                     QString &ldrName,
                                     const QString &modelName,
                                     FloatPairMeta &ca,
                                     int option,
                                     int imageType);
  static int             rotateParts(const QString &addLine,     // RotateParts #3 - 5 parms
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
