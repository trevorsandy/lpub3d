
/****************************************************************************
**
** Copyright (C) 2007-2009 Kevin Clague. All rights reserved.
** Copyright (C) 2015 - 2018 Trevor SANDY. All rights reserved.
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
class NativePov;
class lcVector3;
class Project;

class Render
{
public:
  Render() {}
  enum Mt { PLI, CSI };
  virtual ~Render() {}
  static QString const   getRenderer();
  static void            setRenderer(QString const &);
  static QString const   getRotstepMeta(RotStepMeta &);
  static int             executeLDViewProcess(QStringList &, Mt);
  static bool            useLDViewSCall();
  static bool            useLDViewSList();
  bool                   RenderNativeImage(const NativeOptions &);
  bool                   LoadViewer(const ViewerOptions &);
  bool                   LoadStepProject(Project *,
                                  const QString &);
  int                    rotateParts(const QString &addLine,
                                     RotStepMeta &rotStep,
                                     const QStringList &parts,
                                     QString &ldrName,
                                     const QString &modelName,
                                     FloatPairMeta &ca);
  static int             rotateParts(const QString &addLine,
                                     RotStepMeta &rotStep,
                                     QStringList &parts,
                                     FloatPairMeta &ca,
                                     bool  applyCA = true);
  static int             splitIMParts(const QStringList &rotatedParts,
                                      const QString &rotsComment,
                                      const QString &ldrName,
                                      const QString &csiKey);
  virtual int 		 renderCsi(const QString &,
                                      const QStringList &,
                                      const QStringList &,
                                      const QString &,
                                      Meta &) = 0;
  virtual int 		 renderPli(const QStringList &,
                                      const QString &,
                                      Meta &,
                                      bool bom) = 0;

protected:
  virtual float          cameraDistance(Meta &meta, float) = 0;

  friend class Project;
  friend class Step;
};

extern Render *renderer;

class POVRay : public Render
{
public:
  POVRay() {}
  virtual ~POVRay() {}
  virtual int renderCsi(const QString &,  const QStringList &, const QStringList &, const QString &, Meta &);
  virtual int renderPli(                  const QStringList &, const QString &, Meta &, bool bom);
  virtual float cameraDistance(Meta &meta, float);
};

class LDGLite : public Render
{
public:
  LDGLite() {}
  virtual ~LDGLite() {}
  virtual int renderCsi(const QString &,  const QStringList &, const QStringList &,const QString &, Meta &);
  virtual int renderPli(                  const QStringList &, const QString &, Meta &, bool bom);
  virtual float cameraDistance(Meta &meta, float);
};

class LDView : public Render
{
public:
  LDView() {}
  virtual ~LDView() {}
  virtual int renderCsi(const QString &,  const QStringList &, const QStringList &, const QString &, Meta &);
  virtual int renderPli(                  const QStringList &, const QString &, Meta &, bool bom);
  virtual float cameraDistance(Meta &meta, float);
};

class Native : public Render
{
public:
  Native() {}
  virtual ~Native() {}
  virtual int renderCsi(const QString &,  const QStringList &, const QStringList &, const QString &, Meta &);
  virtual int renderPli(                  const QStringList &, const QString &, Meta &, bool bom);
  virtual float cameraDistance(Meta &meta, float);
};

class ViewerOptions
{
public:
  ViewerOptions()
  {
    ImageType       = Render::CSI;
  }
  QString ViewerCsiKey;
  Render::Mt ImageType;
  float CameraDistance;
  float FoV;
  float Latitude;
  float Longitude;
};

class NativeOptions
{
public:
  NativeOptions()
  {
    ImageType         = Render::CSI;
    TransBackground   = true;
    HighlightNewParts = false;
    LineWidth         = 1.0;    
  }
  QString InputFileName;
  QString OutputFileName;
  Render::Mt ImageType;
  float ImageWidth;
  float ImageHeight;
  float FoV;
  float Latitude;
  float Longitude;
  float CameraDistance;
  float LineWidth;
  bool HighlightNewParts;
  bool TransBackground;
};
#endif
