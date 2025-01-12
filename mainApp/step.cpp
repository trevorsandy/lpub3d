/****************************************************************************
**
** Copyright (C) 2007-2009 Kevin Clague. All rights reserved.
** Copyright (C) 2015 - 2025 Trevor SANDY. All rights reserved.
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
 * This class represents one step including a step number, and assembly
 * image, possibly a parts list image and zero or more callouts needed for
 * the step.
 *
 * Please see lpub.h for an overall description of how the files in LPub3D
 * make up the LPub3D program.
 *
 ***************************************************************************/

#include <QFileInfo>
#include <QDir>
#include <QFile>

#include "lpub.h"
#include "step.h"
#include "range.h"
#include "ranges.h"
#include "render.h"
#include "calloutbackgrounditem.h"
#include "csiannotation.h"
#include "pointer.h"
#include "numberitem.h"
#include "resolution.h"
#include "dependencies.h"
#include "paths.h"
#include "ldrawfiles.h"
#include "lc_application.h"
#include <LDVQt/LDVImageMatte.h>

/*********************************************************************
 *
 * Create a new step and remember the meta-command state at the time
 * that it was created.
 *
 ********************************************************************/
Step* gStep;
Step::Step(
    Where                  &topOfStep,
    AbstractStepsElement  *_parent,
    int                     number,         // step number as seen by the user
    Meta                  &_meta,           // the current state of the meta-commands
    bool                    calledOut,      // if we're a callout
    bool                    multiStep)
  : calledOut(calledOut)
  , multiStep(multiStep)
{
  top                       =  topOfStep;
  top.modelIndex            =  gui->getSubmodelIndex(topOfStep.modelName);
  parent                    = _parent;
  submodelLevel             = _meta.submodelStack.size();
  stepNumber.number         =  number;       // record step number
  csiItem                   = nullptr;
  adjustOnItemOffset        = false;

  bfxLoadStep               = false;
  buildModActionStep        = false;
  displayStep               = DT_DEFAULT;
  dividerType               = NoDivider;
  relativeType              = StepType;
  csiPlacement.relativeType = CsiType;
  stepNumber.relativeType   = StepNumberType;
  rotateIcon.relativeType   = RotateIconType;
  subModel.relativeType     = SubModelType;

  float pW, hH, fH;
  int which                   = _meta.LPub.page.orientation.value() == Landscape ? 1 : 0;
  pW                          = _meta.LPub.page.size.value(which);
  hH                          = _meta.LPub.page.pageHeader.size.valueInches(1);
  fH                          = _meta.LPub.page.pageFooter.size.valueInches(1);
  PageHeaderMeta headerMeta   = _meta.LPub.page.pageHeader;
  PageFooterMeta footerMeta   = _meta.LPub.page.pageFooter;
  headerMeta.size.setValuesInches(pW,hH);
  footerMeta.size.setValuesInches(pW,fH);
  plPageHeader.relativeType   = PageHeaderType;
  plPageHeader.placement      = headerMeta.placement;
  plPageHeader.size[XX]       = headerMeta.size.valuePixels(XX);
  plPageHeader.size[YY]       = headerMeta.size.valuePixels(YY);
  plPageFooter.relativeType   = PageFooterType;
  plPageFooter.placement      = footerMeta.placement;
  plPageFooter.size[XX]       = footerMeta.size.valuePixels(XX);
  plPageFooter.size[YY]       = footerMeta.size.valuePixels(YY);

  if (calledOut) {
      csiStepMeta.fadeSteps        = _meta.LPub.callout.csi.fadeSteps;
      csiStepMeta.highlightStep    = _meta.LPub.callout.csi.highlightStep;
      csiStepMeta.preferredRenderer= _meta.LPub.callout.csi.preferredRenderer;
      csiStepMeta.studStyle        = _meta.LPub.callout.csi.studStyle;
      csiPlacement.margin          = _meta.LPub.callout.csi.margin;
      csiPlacement.placement       = _meta.LPub.callout.csi.placement;
      pli.margin                   = _meta.LPub.callout.pli.margin;
      pli.placement                = _meta.LPub.callout.pli.placement;
      rotateIconMeta               = _meta.LPub.callout.rotateIcon;
      numberPlacemetMeta           = _meta.LPub.callout.stepNum;
      stepNumber.placement         = _meta.LPub.callout.stepNum.placement;
      stepNumber.font              = _meta.LPub.callout.stepNum.font.valueFoo();
      stepNumber.color             = _meta.LPub.callout.stepNum.color.value();
      stepNumber.margin            = _meta.LPub.callout.stepNum.margin;
      subModel.margin              = _meta.LPub.callout.subModel.margin;
      subModel.placement           = _meta.LPub.callout.subModel.placement;
      pliPerStep                   = _meta.LPub.callout.pli.perStep.value();
      csiStepMeta                  = _meta.LPub.callout.csi;
      justifyStep                  = _meta.LPub.callout.justifyStep;
    } else if (multiStep) {
      csiStepMeta.fadeSteps        = _meta.LPub.multiStep.csi.fadeSteps;
      csiStepMeta.highlightStep    = _meta.LPub.multiStep.csi.highlightStep;
      csiStepMeta.preferredRenderer= _meta.LPub.multiStep.csi.preferredRenderer;
      csiStepMeta.studStyle        = _meta.LPub.multiStep.csi.studStyle;
      csiPlacement.margin          = _meta.LPub.multiStep.csi.margin;
      csiPlacement.placement       = _meta.LPub.multiStep.csi.placement;
      pli.margin                   = _meta.LPub.multiStep.pli.margin;
      pli.placement                = _meta.LPub.multiStep.pli.placement;
      rotateIconMeta               = _meta.LPub.multiStep.rotateIcon;
      numberPlacemetMeta           = _meta.LPub.multiStep.stepNum;
      stepNumber.placement         = _meta.LPub.multiStep.stepNum.placement;
      stepNumber.font              = _meta.LPub.multiStep.stepNum.font.valueFoo();
      stepNumber.color             = _meta.LPub.multiStep.stepNum.color.value();
      stepNumber.margin            = _meta.LPub.multiStep.stepNum.margin;
      subModel.margin              = _meta.LPub.multiStep.subModel.margin;
      subModel.placement           = _meta.LPub.multiStep.subModel.placement;
      pliPerStep                   = _meta.LPub.multiStep.pli.perStep.value();
      csiStepMeta                  = _meta.LPub.multiStep.csi;
      justifyStep                  = _meta.LPub.multiStep.justifyStep;
      adjustOnItemOffset           = _meta.LPub.multiStep.adjustOnItemOffset.value();
      justifyYAxisOutsidePlacement = _meta.LPub.multiStep.justifyYAxisOutsidePlacement.value();
      stepSize                     = _meta.LPub.multiStep.stepSize;
    } else {
      csiPlacement.margin          = _meta.LPub.assem.margin;
      csiPlacement.placement       = _meta.LPub.assem.placement;
      placement                    = _meta.LPub.assem.placement;
      pli.margin                   = _meta.LPub.pli.margin;
      pli.placement                = _meta.LPub.pli.placement;
      rotateIconMeta               = _meta.LPub.rotateIcon;
      stepNumber.placement         = _meta.LPub.stepNumber.placement;
      stepNumber.font              = _meta.LPub.stepNumber.font.valueFoo();
      stepNumber.color             = _meta.LPub.stepNumber.color.value();
      stepNumber.margin            = _meta.LPub.stepNumber.margin;
      subModel.margin              = _meta.LPub.subModel.margin;
      subModel.placement           = _meta.LPub.subModel.placement;
      pliPerStep                   = false;
      csiStepMeta.fadeSteps        = _meta.LPub.assem.fadeSteps;
      csiStepMeta.highlightStep    = _meta.LPub.assem.highlightStep;
      csiStepMeta.preferredRenderer= _meta.LPub.assem.preferredRenderer;
      csiStepMeta.studStyle        = _meta.LPub.assem.studStyle;
      csiStepMeta.cameraAngles     = _meta.LPub.assem.cameraAngles;
      csiStepMeta.modelScale       = _meta.LPub.assem.modelScale;
      csiStepMeta.cameraDistance   = _meta.LPub.assem.cameraDistance;
      csiStepMeta.cameraName       = _meta.LPub.assem.cameraName;
      csiStepMeta.cameraFoV        = _meta.LPub.assem.cameraFoV;
      csiStepMeta.cameraZNear      = _meta.LPub.assem.cameraZNear;
      csiStepMeta.cameraZFar       = _meta.LPub.assem.cameraZFar;
      csiStepMeta.isOrtho          = _meta.LPub.assem.isOrtho;
      csiStepMeta.imageSize        = _meta.LPub.assem.imageSize;
      csiStepMeta.target           = _meta.LPub.assem.target;
      csiStepMeta.position         = _meta.LPub.assem.position;
      csiStepMeta.upvector         = _meta.LPub.assem.upvector;
    }
  pli.steps                 = grandparent();
  pli.step                  = this;
  subModel.steps            = grandparent();
  subModel.step             = this;

  ldviewParms               = _meta.LPub.assem.ldviewParms;
  ldgliteParms              = _meta.LPub.assem.ldgliteParms;
  povrayParms               = _meta.LPub.assem.povrayParms;

  showStepNumber            = _meta.LPub.assem.showStepNumber.value();

  rotateIcon.placement      = rotateIconMeta.placement;
  rotateIcon.margin         = rotateIconMeta.margin;
  rotateIcon.setSize(         rotateIconMeta.size,
                              rotateIconMeta.border.valuePixels().thickness);

  placeSubModel             = false;
  placeRotateIcon           = false;
  placeCsiAnnotation        = false;
  updateViewer              = true;    // this is set to false on csiItem mouseReleaseEvent and countPages
  gStep = this;
}

/* step destructor destroys all callouts */

Step::~Step() {
  for (int i = 0; i < list.size(); i++) {
      Callout *callout = list[i];
      delete callout;
    }
  list.clear();
  pli.clear();
  subModel.clear();
  for (int i = 0; i < csiAnnotations.size(); i++) {
      CsiAnnotation *ca = csiAnnotations[i];
      delete ca;
  }
  csiAnnotations.clear();
  gStep = nullptr;
}

Step *Step::nextStep()
{
  const AbstractRangeElement *re = dynamic_cast<const AbstractRangeElement *>(this);
  return dynamic_cast<Step *>(nextElement(re));
}

Range *Step::range()
{
  Range *range = dynamic_cast<Range *>(parent);
  return range;
}

Page *Step::page()
{
   Page *page = dynamic_cast<Page *>(range()->parent);
   return page;
}

/*
 * given a set of parts, generate a CSI
 */

int Step::createCsi(
    QString      const &addLine,
    QStringList  const &csiParts,       // the partially assembles model
    QVector<int> const &_lineTypeIndexes,
    QPixmap           *pixmap,
    Meta              &meta)
{
  bool csiExist         = false;
  bool nativeRenderer   = Preferences::preferredRenderer == RENDERER_NATIVE;
  int  nType            = static_cast<int>(NTypeDefault);
  float cameraFoV       = csiStepMeta.cameraFoV.value();
  float modelScale      = csiStepMeta.modelScale.value();
  float camDistance     = csiStepMeta.cameraDistance.value();

  if (nativeRenderer)
    nType = calledOut ? static_cast<int>(NTypeCalledOut) : multiStep ? static_cast<int>(NTypeMultiStep) : static_cast<int>(NTypeDefault);

  rotStepMeta = meta.rotStep;

  // set camera angles
  bool  customViewpoint = csiStepMeta.cameraAngles.customViewpoint();
  bool  noCA            = !customViewpoint && rotStepMeta.value().type.toUpper() == QLatin1String("ABS");
  FloatPairMeta cameraAngles;
  if (!noCA)
      cameraAngles.setValues( csiStepMeta.cameraAngles.value(0),
                              csiStepMeta.cameraAngles.value(1));

  QString nameSuffix      = lpub->mi.viewerStepKeySuffix(top, this);
  QString csi_Name        = QString("%1%2-%3").arg(csiName(), nameSuffix, QString::number(Preferences::preferredRenderer));
  bool    invalidIMStep   = displayStep != DT_DEFAULT || stepNumber.number == 1;
  bool    useImageSize    = csiStepMeta.imageSize.value(0) > 0;
  lineTypeIndexes         = _lineTypeIndexes;

  ldrName.clear();

//  if (multiStep)
//  logTrace() << "STEP NO: "<< stepNumber.number << ", STUD LOGO: " << meta.LPub.multiStep.csi.studStyle.value();

  // 1 color x y z a b c d e f g h i foo.dat
  // 0 1     2 3 4 5 6 7 8 9 0 1 2 3 4
  QStringList tokens;
  split(addLine,tokens);
  QString orient;
  if (tokens.size() == 15) {
      for (int i = 5; i < 14; i++) {
          orient += "_" + tokens[i];
        }
    }

  // define csi file paths
  QString csiLdrFilePath = QString("%1%2%3").arg(QDir::currentPath(), QDir::separator(), Paths::tmpDir);
  QString csiPngFilePath = QString("%1%2%3").arg(QDir::currentPath(), QDir::separator(), Paths::assemDir);
  QString csiLdrFile = QString("%1%2%3").arg(csiLdrFilePath, QDir::separator(), Gui::m_partListCSIFile ?
                               QFileInfo(Gui::getCurFile()).completeBaseName()+"_snapshot.ldr" : "csi.ldr");
  QString keyPart1 = QString("%1").arg(csi_Name+orient);
  QString keyPart2 = QString("%1_%2_%3_%4_%5_%6_%7_%8")
                             .arg(stepNumber.number)
                             .arg(useImageSize ? int(csiStepMeta.imageSize.value(0)) : lpub->pageSize(meta.LPub.page, 0))
                             .arg(double(resolution()))
                             .arg(resolutionType() == DPI ? "DPI" : "DPCM")
                             .arg(double(modelScale))
                             .arg(double(cameraFoV))
                             .arg(double(cameraAngles.value(0)))
                             .arg(double(cameraAngles.value(1)));

  // append target vector if specified
  if (csiStepMeta.target.isPopulated())
      keyPart2.append(QString("_%1_%2_%3")
                     .arg(double(csiStepMeta.target.x()))
                     .arg(double(csiStepMeta.target.y()))
                     .arg(double(csiStepMeta.target.z())));

  // populate csi name and step number key
  QString nameAndStepKey = QString("%1_%2").arg(csi_Name).arg(stepNumber.number);

  // populate csiKey - Add CompareKey and NameAndStepKey if LDView Single Call
  // key components: compareKey|nameAndStepKey|rendererParms
  if (Render::useLDViewSCall()) {
      QString compareKey = keyPart2;
      // append rotate type if specified
      if (rotStepMeta.isPopulated())
          compareKey.append(QString("_%1")
                                  .arg(rotStepMeta.value().type.isEmpty() ? "REL" :
                                       rotStepMeta.value().type));
      csiKey = QString("CSI_%1|%2").arg(compareKey).arg(nameAndStepKey);
      // add LDView parms to csiKey if not empty
      if (!ldviewParms.value().isEmpty())
          csiKey.append(QString("|%1").arg(ldviewParms.value()));
  }
  // add NameAndStepKey
  else {
      csiKey = nameAndStepKey;
  }

  // append rotstep if specified
  if (rotStepMeta.isPopulated())
      keyPart2.append(QString("_%1")
                     .arg(renderer->getRotstepMeta(rotStepMeta,true)));

  QString key = QString("%1_%2").arg(keyPart1).arg(keyPart2);

  // populate png name
  pngName = QDir::toNativeSeparators(QString("%1%2%3.png").arg(csiPngFilePath, QDir::separator(), key));

  // image path max length check - warning when approaching and hard stop when exceeded.
  int imagePathLength = pngName.size();
  bool imagePathControl = imagePathLength + 5 > LP3D_MAX_IMAGE_RENDER_PATH;
  if (imagePathControl && Preferences::preferredRenderer == RENDERER_LDVIEW) {
      QMessageBox::Icon messageIcon = QMessageBox::Warning;
      Where here(QFileInfo(Gui::getCurFile()).completeBaseName(),1);
      QString imagePathMessage = QObject::tr("is [%1] characters, which is approaching<br>").arg(imagePathLength);
      QString imagePathRequest = QObject::tr("Please consider revising your");
      bool imagePathCritical = imagePathLength > LP3D_MAX_IMAGE_RENDER_PATH;
      if (imagePathCritical) {
          messageIcon = QMessageBox::Critical;
          imagePathMessage = QObject::tr("is [%1] characters, which has exceeded<br>").arg(imagePathLength);
          imagePathRequest = QObject::tr("Please revise your");
      }
      QString const message = QObject::tr("Your complete model file path %1 the maximum "
                                          "number of characters [%2] supported by %3.<br><br>"
                                          "%4 LDraw file path and/or model names to avoid "
                                          "undefined renderer behaviour or switch to the %5 renderer.")
                                          .arg(imagePathMessage).arg(LP3D_MAX_IMAGE_RENDER_PATH)
                                          .arg(rendererNames[Preferences::preferredRenderer])
                                          .arg(imagePathRequest).arg(rendererNames[RENDERER_NATIVE]);
      emit gui->parseErrorSig(message, here, Preferences::ParseErrors,
                              false/*messageOption*/, false/*messageOverride*/, messageIcon,
                              QObject::tr("Renderer Path Control"),
                              QObject::tr("renderer image path length warning"));
      if (imagePathCritical)
          return HitAbortProcess;
  }

  // add csiKey and pngName to ImageMatte repository - exclude first step

  if (Preferences::enableFadeSteps && Preferences::enableImageMatting && !invalidIMStep) {
      if (!LDVImageMatte::validMatteCSIImage(nameAndStepKey))
          LDVImageMatte::insertMatteCSIImage(nameAndStepKey, pngName);
    }

  // Check if png file date modified is older than model file (on the stack) date modified
  csiOutOfDate = false;

  QFile csi(pngName);
  csiExist = csi.exists();
  if (csiExist) {
      QString parentModelName = parent->modelName();
      QDateTime lastModified = QFileInfo(pngName).lastModified();
      QStringList parsedStack = submodelStack();
      parsedStack << parentModelName;
      if ( ! isOlder(parsedStack,lastModified)) {
          csiOutOfDate = true;
          emit gui->messageSig(LOG_DEBUG,QString("CSI image out of date %1.").arg(QFileInfo(pngName).fileName()));
          if (! csi.remove()) {
              emit gui->messageSig(LOG_ERROR,QString("Failed to remove out of date CSI image file %1.").arg(QFileInfo(pngName).fileName()));
          }
      }
  }

  // populate viewerStepKey variable
  viewerStepKey = QString("%1;%2;%3%4")
                          .arg(top.modelIndex)
                          .arg(top.lineNumber)
                          .arg(stepNumber.number)
                          .arg(nameSuffix);
/*
#ifdef QT_DEBUG_MODE
  const QString keyMessage = QString("Create CSI ViewerStepKey "
                                     "Key: '%1' ["
                                     "ModelIndex: %2 (%3), "
                                     "LineNumber: %4, "
                                     "StepNumber: %5%6]")
                              .arg(viewerStepKey)
                              .arg(gui->getSubmodelIndex(top.modelName))
                              .arg(top.modelName)
                              .arg(top.lineNumber)
                              .arg(stepNumber.number)
                              .arg(nameSuffix.isEmpty() ? "" : QString(", Suffix: %1").arg(nameSuffix));
  //emit gui->messageSig(LOG_DEBUG, keyMessage);
  qDebug() << qPrintable(QString("DEBUG: %1").arg(message));
#endif
//*/

  int rc = 0;

  QElapsedTimer timer;

  // Generate Visual Editor CSI entry - this must come before 'Generate the renderer CSI file'
  // as we are using the entered key to render the CSI
  if (!Gui::exportingObjects() || nativeRenderer) {

      timer.start();

//#ifdef QT_DEBUG_MODE
//      // Viewer step png file is out of date
//      qDebug() << qPrintable(QString("DEBUG: %1 out of date %2.")
//                                     .arg(csiOutOfDate
//                                          ? "TRUE - Viewer Step CSI image IS"
//                                          : "FALSE - Viewer Step CSI image IS NOT")
//                                     .arg(QFileInfo(pngName).fileName()));
//#endif

      // Viewer step does not yet exist in repository
      bool addViewerStepContent = !lpub->ldrawFile.viewerStepContentExist(viewerStepKey);

//#ifdef QT_DEBUG_MODE
//      qDebug() << qPrintable(QString("DEBUG: %1 - Viewer Step Does Not Exist for Key (Model,Line,Step) %2")
//                                     .arg(addViewerStepContent ? "TRUE " : "FALSE").arg(viewerStepKey));
//#endif

      // We are processing again the current step but the CSI has changed - e.g. updated in the viewer
      bool viewerUpdate = viewerStepKey == gui->getViewerStepKey();

//#ifdef QT_DEBUG_MODE
//      qDebug() << qPrintable(QString("DEBUG: %1 - Viewer Step Updated for Key (Model,Line,Step) %2")
//                                     .arg(viewerUpdate ? "TRUE " : "FALSE").arg(viewerStepKey));
//#endif

      // Or update if the unrotated content has changed - e.g. colour change for material colour lines
      viewerUpdate |= lpub->ldrawFile.viewerStepContentChanged(viewerStepKey, csiParts);

//#ifdef QT_DEBUG_MODE
//      qDebug() << qPrintable(QString("DEBUG: %1 - Viewer Step Content Changed for Key (Model,Line,Step) %2")
//                                     .arg(viewerUpdate ? "TRUE " : "FALSE").arg(viewerStepKey));
//#endif

      // However, update if we are processing an updated step - e.g. a step in a step-group
      viewerUpdate |= lpub->ldrawFile.viewerStepModified(viewerStepKey,true/*reset*/);

//#ifdef QT_DEBUG_MODE
//      qDebug() << qPrintable(QString("DEBUG: %1 - Viewer Step Modified for Key (Model,Line,Step) %2")
//                                     .arg(viewerUpdate ? "TRUE " : "FALSE").arg(viewerStepKey));
//#endif

      // Or the current step submodel file has been updated
      viewerUpdate |= lpub->ldrawFile.modified(top.modelName,false/*reset*/);

//#ifdef QT_DEBUG_MODE
//      qDebug() << qPrintable(QString("DEBUG: %1 - Viewer Step Model [%2] Is Modified for Key (Model,Line,Step) %3")
//                                     .arg(viewerUpdate ? "TRUE " : "FALSE").arg(top.modelName).arg(viewerStepKey));
//#endif

      // But do not update viewer contents for steps generated from a buildMod action (buildMod change)
      viewerUpdate &= !lpub->ldrawFile.getViewerStepHasBuildModAction(viewerStepKey);

//#ifdef QT_DEBUG_MODE
//      qDebug() << qPrintable(QString("DEBUG: %1 - Viewer Step Has BuildMod Action for Key (Model,Line,Step) %2")
//                                     .arg(viewerUpdate ? "TRUE " : "FALSE").arg(viewerStepKey));

//      // Finally, will the current step's viewer content be processed ?
//      qDebug() << qPrintable(QString("DEBUG: %1 - Update Viewer Step Content for Key (Model,Line,Step) %2")
//                                     .arg(addViewerStepContent || csiOutOfDate || viewerUpdate ? "TRUE " : "FALSE").arg(viewerStepKey));
//#endif

      if (addViewerStepContent || csiOutOfDate || viewerUpdate) {
/*
#ifdef QT_DEBUG_MODE
          const QString action = addViewerStepContent ? "Create" : csiOutOfDate ? "Update Stale" : "Update";
          const QString stepType = calledOut ? "called out" : multiStep ? "step group" : "single step";
          const int stepTypeLineNum = calledOut ? topOfCallout().lineNumber : multiStep ? topOfSteps().lineNumber: topOfStep().lineNumber;
          const QString message = QString("%1 CSI ViewerStep Entry"
                                          "Key: '%2' ["
                                          "ModelIndex: %3 (%4), "
                                          "LineNumber: %5, "
                                          "StepNumber: %6%7], "
                                          "Type: [%8]%9")
                                      .arg(action)
                                      .arg(viewerStepKey)
                                      .arg(gui->getSubmodelIndex(top.modelName))
                                      .arg(top.modelName)
                                      .arg(top.lineNumber)
                                      .arg(stepNumber.number)
                                      .arg(nameSuffix.isEmpty() ? "" : QString(", Suffix: %1").arg(nameSuffix))
                                      .arg(stepType)
                                      .arg(stepType == "single step" ? "" : QString(", StepsLineNumber: [%1]").arg(stepTypeLineNum));
          qDebug() << qPrintable(QString("DEBUG: %1").arg(message));
#endif
//*/
          updateViewer = true; // just to be safe
          QStringList viewerParts = csiParts;
          bool doFadeSteps = csiStepMeta.fadeSteps.enable.value();
          bool doHighlightStep = csiStepMeta.highlightStep.enable.value();
          bool singleSubfile = renderer->isSingleSubfile(viewerParts);

          // set rotated parts - input is csiParts
          QFuture<QStringList> future = QtConcurrent::run([&] () {
              // process part list with single submodel
              if (singleSubfile)
                  renderer->createNativeModelFile(viewerParts,doFadeSteps,doHighlightStep,Options::CSI,singleSubfile);

              QStringList futureParts = viewerParts;
              // RotateParts #3 - 5 parms, rotate parts for Visual Editor, apply ROTSTEP without camera angles - this rotateParts routine updates the parts list
              if (renderer->rotateParts(
                          addLine,
                          rotStepMeta,
                          futureParts,
                          cameraAngles,
                          false/*applyCA*/,
                          singleSubfile) != 0) {
                  emit gui->messageSig(LOG_ERROR,QString("Failed to rotate viewer CSI parts"));
                  pngName = QString(":/resources/missingimage.png");
                  futureParts.clear();
              }
              // add ROTSTEP command - used by Visual Editor to properly adjust rotated parts
              if (futureParts.size())
                  futureParts.prepend(renderer->getRotstepMeta(rotStepMeta));

              return futureParts;
          });

          QStringList rotatedParts = future.result();

          rc = rotatedParts.isEmpty();

          // rotated parts without ROTSTEP
          QStringList rotatedPartsNH = rotatedParts;
          if (rotatedPartsNH.size() && rotatedPartsNH.at(0)[0] == '0')
              rotatedPartsNH.takeFirst();

          // process part list for Native renderer - if not already processed in singleSubfile above
          if (!singleSubfile) {
              if (!rc && Preferences::inlineNativeContent) {
                  future = QtConcurrent::run([&] () {
                      QStringList futureParts = rotatedParts;
                      // header and closing meta for Visual Editor - this call returns an updated rotatedParts file
                      renderer->setLDrawHeaderAndFooterMeta(futureParts,top.modelName,Options::CSI,displayStep);
                      // consolidate subfiles and parts into single file
                      if (renderer->createNativeModelFile(futureParts,doFadeSteps,doHighlightStep) != 0) {
                          emit gui->messageSig(LOG_ERROR,QString("Failed to consolidate Viewer CSI parts"));
                          pngName = QString(":/resources/missingimage.png");
                          futureParts.clear();
                      }
                      return futureParts;
                  });

                  rotatedParts = future.result();

                  rc = rotatedParts.isEmpty();
              }
          }

          // store rotated and unrotated (csiParts). Unrotated parts are used to generate LDView pov file
          if (!csiStepMeta.target.isPopulated())
              keyPart2.append(QString("_0_0_0"));
          if (!rotStepMeta.isPopulated())
              keyPart2.append(QString("_0_0_0_REL"));
          QString stepKey = QString("%1;%3").arg(keyPart1).arg(keyPart2);
          lpub->ldrawFile.insertViewerStep(viewerStepKey,rotatedParts,rotatedPartsNH,csiParts,csiLdrFile,pngName,stepKey/*keyPart2*/,multiStep,calledOut,Options::CSI);
      }

      // Are any children submodels modified ? - trigger the native renderer to update input files on disc
      if (modified(lpub->ldrawFile.getSubmodelIndexes(top.modelName), false/*reset*/))
          lpub->ldrawFile.setViewerStepModified(viewerStepKey);

      // for now, we always set viewer display options
      StudStyleMeta* ssm = meta.LPub.studStyle.value() ? &meta.LPub.studStyle : &csiStepMeta.studStyle;
      AutoEdgeColorMeta* aecm = meta.LPub.autoEdgeColor.enable.value() ? &meta.LPub.autoEdgeColor : &csiStepMeta.autoEdgeColor;
      HighContrastColorMeta* hccm = meta.LPub.studStyle.value() ? &meta.LPub.highContrast : &csiStepMeta.highContrast;
      bool lpubFadeHighlight  = csiStepMeta.fadeSteps.lpubFade.value() || csiStepMeta.highlightStep.lpubHighlight.value();
      viewerOptions                 = new NativeOptions();
      viewerOptions->HighlightNewParts = false; // Gui::suppressColourMeta();
      viewerOptions->CameraDistance = camDistance > 0 ? camDistance : renderer->ViewerCameraDistance(meta,csiStepMeta.modelScale.value());
      viewerOptions->CameraName     = csiStepMeta.cameraName.value();
      viewerOptions->FoV            = csiStepMeta.cameraFoV.value();
      viewerOptions->ImageFileName  = pngName;
      viewerOptions->IsOrtho        = csiStepMeta.isOrtho.value();
      viewerOptions->Viewpoint      = static_cast<int>(csiStepMeta.cameraAngles.cameraView());
      viewerOptions->CustomViewpoint= customViewpoint;
      viewerOptions->Latitude       = cameraAngles.value(0);
      viewerOptions->Longitude      = cameraAngles.value(1);
      viewerOptions->ModelScale     = csiStepMeta.modelScale.value();
      viewerOptions->PageHeight     = lpub->pageSize(meta.LPub.page, 1);
      viewerOptions->PageWidth      = lpub->pageSize(meta.LPub.page, 0);
      viewerOptions->Position       = Vector3(csiStepMeta.position.x(),csiStepMeta.position.y(),csiStepMeta.position.z());
      viewerOptions->Resolution     = resolution();
      viewerOptions->RotStep        = Vector3(float(rotStepMeta.value().rots[0]),float(rotStepMeta.value().rots[1]),float(rotStepMeta.value().rots[2]));
      viewerOptions->RotStepType    = rotStepMeta.value().type;
      viewerOptions->LPubFadeHighlight = lpubFadeHighlight;
      viewerOptions->FadeParts      = csiStepMeta.fadeSteps.enable.value() || Preferences::enableFadeSteps;
      viewerOptions->HighlightParts = csiStepMeta.highlightStep.enable.value() || Preferences::enableHighlightStep;
      viewerOptions->AutoEdgeColor  = aecm->enable.value();
      viewerOptions->EdgeContrast   = aecm->contrast.value();
      viewerOptions->EdgeSaturation = aecm->saturation.value();
      viewerOptions->StudStyle      = ssm->value();
      viewerOptions->ExportMode     = Gui::m_exportMode;
      viewerOptions->LineWidth      = lcGetPreferences().mLineWidth;
      viewerOptions->LightDarkIndex = hccm->lightDarkIndex.value();
      viewerOptions->StudCylinderColor = hccm->studCylinderColor.value();
      viewerOptions->StudCylinderColorEnabled = hccm->studCylinderColorEnabled.value();
      viewerOptions->PartEdgeColor  = hccm->partEdgeColor.value();
      viewerOptions->PartEdgeColorEnabled = hccm->partEdgeColorEnabled.value();
      viewerOptions->BlackEdgeColor = hccm->blackEdgeColor.value();
      viewerOptions->BlackEdgeColorEnabled = hccm->blackEdgeColorEnabled.value();
      viewerOptions->DarkEdgeColor  = hccm->darkEdgeColor.value();
      viewerOptions->DarkEdgeColorEnabled = hccm->darkEdgeColorEnabled.value();
      viewerOptions->Target         = Vector3(csiStepMeta.target.x(),csiStepMeta.target.y(),csiStepMeta.target.z());
      viewerOptions->UpVector       = Vector3(csiStepMeta.upvector.x(),csiStepMeta.upvector.y(),csiStepMeta.upvector.z());
      viewerOptions->ViewerStepKey  = viewerStepKey;
      viewerOptions->ZFar           = csiStepMeta.cameraZFar.value();
      viewerOptions->ZNear          = csiStepMeta.cameraZNear.value();
      viewerOptions->DDF            = meta.LPub.cameraDDF.value();

#ifdef QT_DEBUG_MODE
      emit gui->messageSig(LOG_INFO,
                           QString("Generate Visual Editor step options entry took %1.")
                                   .arg(Gui::elapsedTime(timer.elapsed(),false)));
#endif
//      moved to drawPage::StepRc
//      if (!calledOut && !multiStep && updateViewer)
//          loadTheViewer();
  } // Generate Visual Editor CSI entry

#ifdef QT_DEBUG_MODE
  LDrawFile &ldrawFile = lpub->ldrawFile;
  Q_UNUSED(ldrawFile)
#endif

  // Generate the renderer CSI file

  if (!rc && (!csiExist || csiOutOfDate)) {

     timer.start();

     // this is initialized to true but set to false on csiItem mouseReleaseEvent so reset here
     updateViewer = true;

     // populate ldr file name
     ldrName = QDir::toNativeSeparators(QString("%1/%2.ldr").arg(csiLdrFilePath).arg(key));

     // rotate parts and create the CSI file for LDView single call and Native renderering
     if (Render::useLDViewSCall() || nativeRenderer) {

         if (nativeRenderer)
            ldrName = csiLdrFile;

         // set rotated parts
         QFuture<int> future = QtConcurrent::run([&] () {
             int rcf = 0;
             QStringList futureParts = csiParts;
             // RotateParts #2 - 8 parms, Camera angles not applied but ROTSTEP applied to rotated parts for Native renderer - this rotateParts routine generates an ldr file
             if ((rcf = renderer->rotateParts(
                      addLine,
                      rotStepMeta,
                      futureParts,
                      ldrName,
                      top.modelName,
                      cameraAngles,
                      DT_DEFAULT,
                      Options::CSI)) != 0) {
                 emit gui->messageSig(LOG_ERROR,QString("Failed to create and rotate CSI ldr file: %1.").arg(ldrName));
                 pngName = QString(":/resources/missingimage.png");
                 rcf = -1;
             }
             return rcf;
         });

         rc = future.result();
     }

     bool showStatus = Gui::m_partListCSIFile;

     if (!rc && !Render::useLDViewSCall() && ! Gui::m_partListCSIFile) {
         showStatus = true;

         // set camera
         meta.LPub.assem.studStyle      = csiStepMeta.studStyle;

         meta.LPub.assem.cameraAngles   = csiStepMeta.cameraAngles;
         meta.LPub.assem.cameraDistance = csiStepMeta.cameraDistance;
         meta.LPub.assem.modelScale     = csiStepMeta.modelScale;
         meta.LPub.assem.cameraFoV      = csiStepMeta.cameraFoV;
         meta.LPub.assem.isOrtho        = csiStepMeta.isOrtho;
         meta.LPub.assem.imageSize      = csiStepMeta.imageSize;
         meta.LPub.assem.target         = csiStepMeta.target;

         // set the extra renderer parms
         meta.LPub.assem.ldviewParms =
              Render::getRenderer() == RENDERER_LDVIEW ? ldviewParms :
              Render::getRenderer() == RENDERER_LDGLITE ? ldgliteParms :
                            /*POV scene file generator*/  ldviewParms ;
         if (Preferences::preferredRenderer == RENDERER_POVRAY) {
            meta.LPub.assem.povrayParms = povrayParms;

            if (lightList.size()) {
                QStringList pl;
                if (!meta.LPub.assem.ldviewParms.value().isEmpty()) {
                    QRegExp quotedRx("\"|'");
                    pl = meta.LPub.assem.ldviewParms.value().split(' ');
                    for (int i = 0; i < pl.size(); i++) {
                        if (QString(pl.at(i)).contains(quotedRx))
                            continue;
                        else
                            pl[i] = QString("'%1'").arg(pl.at(i));
                    }
                }
                int i = 0;
                QString ls = QString("'-PovExporter/PovLights=\"");
                for (const LightData &ld : lightList) {
                    ls.append(QString("%1%2").arg(i ? ";" : "").arg(ld.getPOVLightMacroString()));
                    i++;
                }
                ls.append("\"'");
                pl.append(ls);

                meta.LPub.assem.ldviewParms.setValue(pl.join(" "));
            }
         }

         // render the partially assembled model
         QStringList csiKeys = QStringList() << csiKey; // adding just a single key - i.e.nameAndStepKey

         // set the current step - enable access from other parts of the application - e.g. Renderer
         if (nativeRenderer) {
             lpub->setCurrentStep(this);
         }

         //QFuture<int> RenderFuture = QtConcurrent::run([this, &addLine, &csiParts, &csiKeys, &meta, nType] () {
         //    int frc = 0;
         //    QStringList futureParts = csiParts;
             if ((/*f*/rc = renderer->renderCsi(addLine, /*futureParts*/csiParts, csiKeys, pngName, meta, nType)) != 0) {
                 emit gui->messageSig(LOG_ERROR,QString("%1 CSI render failed for<br>%2")
                                      .arg(rendererNames[Render::getRenderer()]).arg(QFileInfo(pngName).fileName()));
                 pngName = QString(":/resources/missingimage.png");
                 /*f*/rc = -1;
             }
         //    return frc;
         //});

         //rc = RenderFuture.result();
     }

     if (!rc && showStatus) {
         emit gui->messageSig(LOG_INFO,
                                  QString("%1 CSI render call took %2 "
                                          "to render %3 for %4 %5 %6 on page %7.")
                                          .arg(rendererNames[Render::getRenderer()])
                                          .arg(Gui::elapsedTime(timer.elapsed(),false))
                                          .arg(pngName)
                                          .arg(calledOut ? "called out," : "simple,")
                                          .arg(multiStep ? "step group" : "single step")
                                          .arg(stepNumber.number)
                                          .arg(Gui::stepPageNum));
     }

     if (Gui::exportingObjects() && Gui::m_partListCSIFile) {
         pixmap->load(":/resources/placeholderimage.png");  // image placeholder
         csiPlacement.size[0] = pixmap->width();
         csiPlacement.size[1] = pixmap->height();
         return rc;
     }
  } // Generate the renderer CSI file

  // Reset updateViewer - leave value in place for calledOut and multiStep
  if (!calledOut && !multiStep)
      updateViewer = true;

  // If not using LDView SCall, populate pixmap
  if (! Render::useLDViewSCall()) {
      pixmap->load(pngName);
      csiPlacement.size[0] = pixmap->width();
      csiPlacement.size[1] = pixmap->height();
      viewerOptions->ImageWidth  = pixmap->width();
      viewerOptions->ImageHeight = pixmap->height();
  }

  return rc;
}

/*
 * Process csiParts list - fade previous step-parts and or highlight current step-parts.
 * To get the previous content position, take the previous cisFile file size.
 * The csiFile entries are only parts with no formatting or meta commands so it is
 * well suited to provide the delta between steps.
 * When using the Native renderer, it is possible to 'disable' colour code and part
 * name formatting with the lpubFadeHighlight flag in order to use the Native renderer
 * fade and highlight behaviour.
 * The lpubFadeHighlight flag is set from the LPUB_FADE/LPUB_HIGHLIGHT meta command.
 */
QStringList Step::configureModelStep(const QStringList &csiParts, Where &current/*topOfStep*/)
{
  QStringList processedCsiParts, stepColourList;

  bool enableFadeSteps       = csiStepMeta.fadeSteps.enable.value();
  bool enableHighlightStep   = csiStepMeta.highlightStep.enable.value();
  bool lpubFadeHighlight     = csiStepMeta.fadeSteps.lpubFade.value() ||
                               csiStepMeta.highlightStep.lpubHighlight.value();

  bool highlightFirstStep    = Preferences::highlightFirstStep;

  QString fadeColour         = LDrawColor::code(Preferences::validFadeStepsColour);
  bool fadeStepsUseColour    = Preferences::fadeStepsUseColour;
  int fadeStepsOpacity       = Preferences::fadeStepsOpacity;
  if (Gui::m_fadeStepsSetup || csiStepMeta.fadeSteps.setup.value()) {
    fadeColour               = LDrawColor::code(csiStepMeta.fadeSteps.color.value().color);
    fadeStepsOpacity         = csiStepMeta.fadeSteps.opacity.value();
    fadeStepsUseColour       = csiStepMeta.fadeSteps.color.value().useColor;
  }

  QString highlightColour    = Preferences::highlightStepColour;
  int highlightStepLineWidth = Preferences::highlightStepLineWidth;
  if (Gui::m_highlightStepSetup || csiStepMeta.highlightStep.setup.value()) {
    highlightStepLineWidth   = csiStepMeta.highlightStep.lineWidth.value();
    highlightColour          = csiStepMeta.highlightStep.color.value();
  }

  bool FadeMetaAdded = false;
  bool SilhouetteMetaAdded = false;
  bool fadeCommand = false;
  bool highlightCommand = false;
  int stepNum = 0;

  if (displayStep) {
    if ((highlightFirstStep = displayStep == DT_MODEL_DISPLAY))
      stepNum = stepNumber.number > 1 ? stepNumber.number : static_cast<int>(displayStep);
    else
      stepNum = -1;
  } else {
    stepNum = stepNumber.number;
  }

  std::function<void(const QStringList&)> processCsiParts;
  processCsiParts = [&](const QStringList &csiParts)
  {
    QStringList parts = csiParts;

    // retrieve the previous step position
    int prevStepPosition = 0;
    if (!displayStep) {
      if (Gui::savePrevStepPosition && !calledOut && !multiStep)
        prevStepPosition = Gui::savePrevStepPosition;
      else
        prevStepPosition = lpub->ldrawFile.getPrevStepPosition(current.modelName, current.lineNumber, stepNum);
    }

/*
#ifdef QT_DEBUG_MODE
    bool fromSavePrev = Gui::savePrevStepPosition && !calledOut && !multiStep;
    bool backward = Gui::pageDirection >= PAGE_BACKWARD && prevStepPosition > parts.size();
    QString const pageDirection = "Backward - Adjustment: (%1), PrevPos: %2 - CsiSize: %3";
    emit lpub->messageSig(LOG_DEBUG,
    QString("\nConfigure Model Page..........%1"
            "\n - StepNum....................%2"
            "\n - PreviousStepSource.........%3"
            "\n - PreviousStepPos............%4"
            "\n - CurrentStepPos (CSI Size)..%5"
            "\n - SubModelName...............%6"
            "\n - CalledOut..................%7"
            "\n - MultiStep..................%8"
            "\n - SubModelSize...............%9"
            "\n - PageDirection..............%10"
            "\n - LPubFadeHighlight..........%11"
            "\n - DisplayStep................%12"
            "\n - EnableFadeSteps............%13"
            "\n - FadeStepsUseColour.........%14"
            "\n - FadeColourCode.............%15"
            "\n - FadeStepsOpacity...........%16"
            "\n - FadeStepsGuiSetup..........%17"
            "\n - FadeStepsMetaSetup.........%18"
            "\n - EnableHighlightStep........%19"
            "\n - HighlightFirstStep.........%20"
            "\n - HighlightColour............%21"
            "\n - HighlightStepLineWidth.....%22"
            "\n - HighlightStepGuiSetup......%23"
            "\n - HighlightStepMetaSetup.....%24"
           ).arg(Gui::stepPageNum)                                                     //01
            .arg(stepNum)                                                              //02
            .arg(fromSavePrev ? "SavePrevStep" : "GetPrevStep")                        //03
            .arg(prevStepPosition)                                                     //04
            .arg(parts.size())                                                         //05
            .arg(current.modelName)                                                    //06
            .arg(calledOut ? "Yes" : "No")                                             //07
            .arg(multiStep ? "Yes" : "No")                                             //08
            .arg(lpub->ldrawFile.size(current.modelName))                              //09
            .arg(backward ? prevStepPosition - parts.size() ? pageDirection            //10
            .arg(prevStepPosition - parts.size())
            .arg(prevStepPosition).arg(parts.size()) : "Backward" : "Forward")
            .arg(lpubFadeHighlight ? "Yes" : "No")                                     //11
            .arg(displayStep ? "Yes" : "No")                                           //12
            .arg(enableFadeSteps ? "Yes" : "No")                                       //13
            .arg(fadeStepsUseColour ? "Yes" : "No")                                    //14
            .arg(QString("%1 (%2)").arg(fadeColour).arg(LDrawColor::name(fadeColour))) //15
            .arg(fadeStepsOpacity)                                                     //16
            .arg(Gui::m_fadeStepsSetup ? "Yes" : "No")                                 //17
            .arg(csiStepMeta.fadeSteps.setup.value() ? "Yes" : "No")                   //18
            .arg(enableHighlightStep ? "Yes" : "No")                                   //19
            .arg(highlightFirstStep ? "Yes" : "No")                                    //10
            .arg(highlightColour)                                                      //21
            .arg(highlightStepLineWidth)                                               //22
            .arg(Gui::m_highlightStepSetup ? "Yes" : "No")                             //23
            .arg(csiStepMeta.highlightStep.setup.value() ? "Yes" : "No")               //24
           );
#endif
//*/

    if (Gui::pageDirection >= PAGE_BACKWARD && prevStepPosition > parts.size())
      prevStepPosition -= parts.size();

    QStringList argv;

    for (int index = 0; index < parts.size(); index++) {

      bool type_1_line = false;
      bool type_1_5_line = false;
      bool is_helper_part = false;
      bool is_colour_part = false;
      bool is_submodel_file = false;

      int updatePosition = index + 1;

      QString csiLine = parts[index];

      split(csiLine, argv);

      // parse each step line for manually entered FADE or SILHOUETTE commands
      if (argv.size() >= 2 && argv[0] == "0") {
        bool ok, update = false;
        if (enableFadeSteps && argv[1] == QStringLiteral("!FADE")) {
          FadeMetaAdded = true;
          if (argv.size() == 2) {
            fadeCommand = !fadeCommand;
          } else if (argv.size() >= 3) {
            fadeCommand = true;
            if (argv.size() == 3) {
              int opacity = argv[2].toInt(&ok);
              if (ok)
                fadeStepsOpacity = opacity;
              else
                emit lpub->messageSig(LOG_WARNING, QObject::tr("Specified fade opacity value is invalid [%1] ").arg(argv[2]));
            } else
            if (argv.size() == 4) {
              QRegExp hexRx("\\s*(0x|#)([\\dA-F]+)\\s*$",Qt::CaseInsensitive);
              if (argv[3].contains(hexRx)) {
                QColor colour(argv[3]);
                if (colour.isValid())
                  fadeColour = LDrawColor::code(argv[3]);
                else
                  emit lpub->messageSig(LOG_WARNING, QObject::tr("Specified fade colour is invalid [%1] ").arg(argv[3]));
              } else {
                if ((fadeColour = LDrawColor::code(argv[3])) == QLatin1String("-1")) {
                  emit lpub->messageSig(LOG_WARNING, QObject::tr("Specified fade colour is invalid [%1] ").arg(argv[3]));
                  fadeColour = LDrawColor::code(Preferences::validFadeStepsColour);
                }
              }
            }

            FadeColorData data = csiStepMeta.fadeSteps.color.value();
            if (fadeColour != LDrawColor::code(data.color) || fadeStepsUseColour != data.useColor) {
              data.color = fadeColour;
              data.useColor = fadeStepsUseColour;
              csiStepMeta.fadeSteps.color.setValue(data);
              update = true;
            }
            if (fadeStepsOpacity != csiStepMeta.fadeSteps.opacity.value()) {
              csiStepMeta.fadeSteps.opacity.setValue(fadeStepsOpacity);
              if (!update)
                update = true;
            }
            if (update) {
              csiStepMeta.fadeSteps.setPreferences();
              update = false;
            }
          }
        } // FADE

        if (enableHighlightStep && argv[1] == QStringLiteral("!SILHOUETTE")) {
          SilhouetteMetaAdded = true;
          if (argv.size() == 2) {
            highlightCommand = !highlightCommand;
          } else if (argv.size() >= 3) {
            highlightCommand = true;
            if (argv.size() == 3) {
              QColor colour(argv[2]);
              if (colour.isValid())
                highlightColour = argv[2];
              else
                emit lpub->messageSig(LOG_WARNING, QObject::tr("Specified highlight colour is invalid [%1] ").arg(argv[2]));
            } else
            if (argv.size() == 4) {
              int width = argv[2].toDouble(&ok);
              if (ok)
                highlightStepLineWidth = width;
              QColor colour(argv[3]);
              if (colour.isValid())
                highlightColour = argv[3];
              else
                emit lpub->messageSig(LOG_WARNING, QObject::tr("Specified highlight colour is invalid [%1] ").arg(argv[3]));
            }
            if (highlightColour != csiStepMeta.highlightStep.color.value()) {
              update = true;
              csiStepMeta.highlightStep.color.setValue(highlightColour);
            }
            if (highlightStepLineWidth != csiStepMeta.highlightStep.lineWidth.value()) {
              update = true;
              csiStepMeta.highlightStep.lineWidth.setValue(highlightStepLineWidth);
            }
            if (update) {
              csiStepMeta.highlightStep.setPreferences();
              update = false;
            }
          }
        } // SILHOUETTE
      } // manually entered FADE or SILHOUETTE commands

      // determine line type
      if (argv.size() && argv[0].size() == 1 && argv[0] >= "1" && argv[0] <= "5") {
        type_1_5_line = true;
        if (argv.size() == 15 && argv[0] == "1") {
          type_1_line = true;
          is_helper_part = ExcludedParts::isExcludedSupportPart(argv[argv.size()-1]) == ExcludedParts::EP_HELPER;
        }
      }

      // process parts - except helper parts
      if (!is_helper_part) {
        // process type 1 parts
        QString fileNameStr, extension;
        if (type_1_line) {
          // set part name and extension
          fileNameStr = QString(argv[argv.size()-1]).toLower();
          extension   = QFileInfo(fileNameStr).suffix();

          // check if is color part
          is_colour_part = lpubFadeHighlight && LDrawColourParts::isLDrawColourPart(fileNameStr);

          // check if part is submodel
          is_submodel_file = lpub->ldrawFile.isSubmodel(fileNameStr);

          // interrogate submodel contents for invalid lines
          if (displayStep && is_submodel_file) {
            std::function<void(const QString&)> processSubmodel;
            processSubmodel = [&](const QString &submodelName)
            {
              QStringList const &contents = lpub->ldrawFile.contents(submodelName);
              if (contents.size()) {
                QRegExp invalidMetaRx("^0 STEP|^0 ROTSTEP|^0 !?FADE|^0 !?SILHOUETTE|^0 !?LPUB (?:MULTI_STEP|CALLOUT|INSERT (?:PAGE|BOM|MODEL|DISPLAY_MODEL|COVER_PAGE))");
                for (int i = 0; i < contents.size(); i++) {
                  QString const &line = contents.at(i);
                  if(line.isEmpty())
                    continue;
                  else if (line.contains(invalidMetaRx)) {
                    Where where(submodelName, lpub->ldrawFile.getSubmodelIndex(submodelName), i);
                    QString const &message = QObject::tr("Meta command not supported in STEP for display submodel '%1'.<br>Line [%2]").arg(submodelName).arg(line);
                    emit gui->parseErrorSig(message,where,Preferences::ParseErrors,true,false,QMessageBox::Warning);
                  } else {
                    QStringList argv;
                    split(line, argv);
                    if (argv[0] == "1" && argv.size() == 15) {
                      if (lpub->ldrawFile.isSubmodel(argv[argv.size()-1]))
                        processSubmodel(argv[argv.size()-1]);
                    }
                  }
                }
              }
            };
            processSubmodel(fileNameStr);
          } // display step submodel

          /*
          if (is_colour_part)
            emit messageSig(LOG_NOTICE, "Static color part - " + fileNameStr);
          //*/
        } // type_1_line

        // write fade step entries
        if ((highlightFirstStep ? stepNum > 1 : true) && enableFadeSteps && (fadeCommand || updatePosition <= prevStepPosition)) {
          // process type 1 - 5 part lines
          if (type_1_5_line) {
            // Insert opening fade meta
            if (!FadeMetaAdded && enableFadeSteps) {
              processedCsiParts.insert(index,QString("0 !FADE %1").arg(fadeStepsOpacity));
              FadeMetaAdded = true;
            }
            // process colour code and part naming
            if (lpubFadeHighlight) {
              // process colour codes
              if (argv[1] != LDRAW_EDGE_MATERIAL_COLOUR &&
                argv[1] != LDRAW_MAIN_MATERIAL_COLOUR) {
                // generate fade color entry
                QString colourCode = fadeStepsUseColour ? fadeColour : argv[1];
                if (!Gui::colourEntryExist(stepColourList,argv[1], FADE_PART, fadeStepsUseColour))
                  stepColourList << Gui::createColourEntry(colourCode, FADE_PART, ""/*highlightColour*/, fadeColour, fadeStepsUseColour, fadeStepsOpacity);
                // set fade color code
                argv[1] = QString("%1%2").arg(LPUB3D_COLOUR_FADE_PREFIX).arg(colourCode);
              }
              // process type 1 line part naming
              if (type_1_line) {
                // process static color part naming
                if (is_colour_part) {
                  if (extension.isEmpty()) {
                    fileNameStr = fileNameStr.append(QString("%1.dat").arg(FADE_SFX));
                  } else {
                    fileNameStr = fileNameStr.replace("."+extension, QString("%1.%2").arg(FADE_SFX).arg(extension));
                  }
                }
                // process subfiles naming
                if (!displayStep && is_submodel_file) {
                  if (extension.isEmpty()) {
                    fileNameStr = fileNameStr.append(QString("%1.ldr").arg(FADE_SFX));
                  } else {
                    fileNameStr = fileNameStr.replace("."+extension, QString("%1.%2").arg(FADE_SFX).arg(extension));
                  }
                }
                // assign fade part name
                argv[argv.size()-1] = fileNameStr;
              } // process type 1 line part naming
            } // process colour code and part naming
          } // process type 1 - 5 part lines
        } // write fade step entries

        // write highlight entries
        if (enableHighlightStep && (highlightCommand || updatePosition > prevStepPosition)) {
          // process type 1 - 5 part lines
          if (type_1_5_line) {
            // Insert opening silhouette meta
            if (!SilhouetteMetaAdded && enableHighlightStep) {
              processedCsiParts.append(QString("0 !SILHOUETTE %1 %2").arg(highlightStepLineWidth).arg(highlightColour));
              SilhouetteMetaAdded = true;
            }
            // process colour code and part naming
            if (lpubFadeHighlight) {
              // process colour codes
              if (argv[1] != LDRAW_EDGE_MATERIAL_COLOUR && argv[1] != LDRAW_MAIN_MATERIAL_COLOUR) {
                // generate highlight color entry
                QString colourCode = argv[1];
                if (!Gui::colourEntryExist(stepColourList,argv[1], HIGHLIGHT_PART))
                  stepColourList << Gui::createColourEntry(colourCode, HIGHLIGHT_PART, highlightColour, ""/*fadeColour*/);
                // set highlight color code
                argv[1] = QString("%1%2").arg(LPUB3D_COLOUR_HIGHLIGHT_PREFIX).arg(colourCode);
              }
              // process type 1 line part naming
              if (type_1_line) {
                // process static color part naming
                if (is_colour_part) {
                  if (extension.isEmpty()) {
                    fileNameStr = fileNameStr.append(QString("%1.dat").arg(HIGHLIGHT_SFX));
                  } else {
                    fileNameStr = fileNameStr.replace("."+extension, QString("%1.%2").arg(HIGHLIGHT_SFX).arg(extension));
                  }
                }
                // process subfiles naming
                if (!displayStep && is_submodel_file) {
                  if (extension.isEmpty()) {
                    fileNameStr = fileNameStr.append(QString("%1.ldr").arg(HIGHLIGHT_SFX));
                  } else {
                    fileNameStr = fileNameStr.replace("."+extension, QString("%1.%2").arg(HIGHLIGHT_SFX).arg(extension));
                  }
                }
                // assign fade part name
                argv[argv.size()-1] = fileNameStr;
              } // process type 1 line part naming
            } // process colour code and part naming
          } // process type 1 - 5 part lines
        } // write highlight entries
      } // process parts - except helper parts

      if (isGhost(csiLine))
        argv.prepend(GHOST_META);

      // previous step parts
      csiLine = argv.join(" ");

      // current step parts
      processedCsiParts << csiLine;

      // insert closing meta commands
      if (!displayStep && !fadeCommand && !highlightCommand) {
        // insert closing fade meta
        if (updatePosition == prevStepPosition) {
          if (FadeMetaAdded)
            processedCsiParts.append(QStringLiteral("0 !FADE"));
        }

        // insert closing silhouette meta
        if (index+1 == parts.size()) {
          if (SilhouetteMetaAdded)
            processedCsiParts.append(QStringLiteral("0 !SILHOUETTE"));
        }
      }
    }

    // insert missing closing command for manually entered FADE or SILHOUETTE
    if (fadeCommand)
      processedCsiParts.append(QStringLiteral("0 !FADE"));
    if (highlightCommand)
      processedCsiParts.append(QStringLiteral("0 !SILHOUETTE"));

    // Step position accurately saved in findPage/drawPage

  };

  if (csiParts.size()                         &&
     (enableFadeSteps || enableHighlightStep) &&
     (highlightFirstStep ? true : stepNum > 1)) {

    QFuture<void> future = QtConcurrent::run([&] () { processCsiParts(csiParts); });
    future.waitForFinished();

  } else {

    // Step position accurately saved in findPage/drawPage

    return csiParts;

  }

  // add the fade color list to the header of the CsiParts list
  if (!stepColourList.isEmpty()) {
    stepColourList.removeDuplicates();  // remove dupes
    processedCsiParts.prepend("0");

    for (int i = 0; i < stepColourList.size(); ++i)
       processedCsiParts.prepend(stepColourList.at(i));

    processedCsiParts.prepend(QString("0 // %1 step custom colours").arg(VER_PRODUCTNAME_STR));
    processedCsiParts.prepend("0");
  }

  return processedCsiParts;
}


bool Step::loadTheViewer() {
    if (Preferences::modeGUI && ! Gui::exporting()) {
        // set the pixmap size of the step to display
        viewerOptions->ImageWidth  = csiPixmap.width();
        viewerOptions->ImageHeight = csiPixmap.height();
        if (! renderer->LoadViewer(viewerOptions)) {
            emit gui->messageSig(LOG_ERROR,QString("Could not load Visual Editor with CSI key: %1")
                                                   .arg(viewerStepKey));
            return false;
        }
    }
    return true;
}

int Step::getLineTypeRelativeIndex(int lineTypeIndx) {
    if (lineTypeIndexes.size() > lineTypeIndx)
        return lineTypeIndexes.at(lineTypeIndx);
    return -1;
}

int Step::getLineTypeIndex(int relativeTypeIndx) {
    for (int i = 0; i < lineTypeIndexes.size(); ++i)
        if (lineTypeIndexes.at(i) == relativeTypeIndx)
            return i;
    return -1;
}

void Step::getStepLocation(Where &top, Where &bottom) {
  if (multiStep) {
        top = topOfSteps();
        bottom = bottomOfSteps();
   } else if (calledOut) {
       top = callout()->topOfCallout();
       bottom = callout()->bottomOfCallout();
   } else /*Single Step*/ {
       top = topOfStep();
       bottom = bottomOfStep();
   }
}

/*
 *
 * Place the CSI step annotation metas
 *
 */
int Step::setCsiAnnotationMetas(Meta &_meta, bool force)
{
    int adjust = 0;

    return setCsiAnnotationMetas(_meta, adjust, force);
}

int Step::setCsiAnnotationMetas(Meta &_meta, int &_adjust, bool force)
{
    TraverseRc trc = HitNothing;

    // sometime we may already have annotations for the
    // step defined - such as after printing or exporting
    if (csiAnnotations.size() && ! force) {
       return static_cast<int>(trc);
    }

    Meta *meta = &_meta;

    if (!meta->LPub.assem.annotation.display.value())
        return static_cast<int>(trc);

    // process callouts if any;
    for (int i = 0; i < list.size(); i++) {
        if (list[i]->relativeType == CalloutType) {
            Callout *callout = dynamic_cast<Callout *>(list[i]);
            if (callout) {
                for (int j = 0; j < callout->list.size(); j++) {
                    Range *range = dynamic_cast<Range *>(callout->list[j]);
                    if (range) {
                        for (int k = 0; k < range->list.size(); k++) {
                            if (range->relativeType == RangeType) {
                                Step *step = dynamic_cast<Step *>(range->list[k]);
                                if (step && step->relativeType == StepType) {
                                    if (step->setCsiAnnotationMetas(callout->meta, _adjust, force))
                                        trc = HitCsiAnnotation;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    QHash<QString, PliPart*> pliParts;

    pli.getParts(pliParts);

    if (!pliParts.size())
        return static_cast<int>(trc);

    QStringList parts;
    Where start,undefined,fromHere,toHere;
    QString savePartIds,partIds,lineNumbers;

    fromHere = topOfStep() + _adjust;
    toHere   = bottomOfStep() + _adjust;
    if (toHere == undefined)
        toHere = fromHere;
    if (toHere == fromHere) {
        lpub->mi.scanForward(toHere,StepMask);
    }

    if (fromHere == undefined) {
        emit gui->messageSig(LOG_ERROR, QObject::tr("CSI annotations cound not retrieve topOfStep settings"));
        return static_cast<int>(trc);
    }

    start = fromHere;

    // process step lines;
    for (; start.lineNumber < toHere.lineNumber; ++start) {

        QString line = gui->readLine(start);
        QStringList argv;
        split(line,argv);

        if (argv.size() == 15 && argv[0] == "1") {
            QString key = QString("%1_%2").arg(QFileInfo(argv[14]).completeBaseName()).arg(argv[1]);
            PliPart *part = pliParts[key];

            if (!part)
                continue;

            if (part->type != argv[14])
                continue;

            if (part->annotateText) {
                QString typeName = QFileInfo(part->type).completeBaseName();
                QRegExp rx(QString("^\\s*0\\s+(\\!*LPUB ASSEM ANNOTATION ICON).*("+typeName+"|HIDDEN|HIDE).*$"));
                Where walk = start;
                line = gui->readLine(++walk); // check next line - skip if meta exist
                if (((line.contains(rx) && typeName == rx.cap(2)) ||
                     (rx.cap(2) == "HIDDEN" || rx.cap(2) == "HIDE")) && ! force)
                    continue;

                bool display = false;
                AnnotationCategory annotationCategory = AnnotationCategory(Annotations::getAnnotationCategory(part->type));
                switch (annotationCategory)
                {
                case AnnotationCategory::axle:
                    display = meta->LPub.assem.annotation.axleDisplay.value();
                    break;
                case AnnotationCategory::beam:
                    display = meta->LPub.assem.annotation.beamDisplay.value();
                    break;
                case AnnotationCategory::cable:
                    display = meta->LPub.assem.annotation.cableDisplay.value();
                    break;
                case AnnotationCategory::connector:
                    display = meta->LPub.assem.annotation.connectorDisplay.value();
                    break;
                case AnnotationCategory::hose:
                    display = meta->LPub.assem.annotation.hoseDisplay.value();
                    break;
                case AnnotationCategory::panel:
                    display = meta->LPub.assem.annotation.panelDisplay.value();
                    break;
                default:
                    display = meta->LPub.assem.annotation.extendedDisplay.value();
                    break;
                }
                if (display) {
                    // Pack parts type, partIds and instance line(s) into stringlist - do not reorder
                    for (int i = 0; i < part->instances.size(); ++i) {
                        savePartIds = typeName+";"+part->color+";"+part->instances[i].modelName;
                        if (partIds == savePartIds) {
                            lineNumbers += QString("%1;").arg(part->instances[i].lineNumber);
                        } else {
                            partIds     = savePartIds;
                            lineNumbers = QString("%1;").arg(part->instances[i].lineNumber);
                            parts.append(partIds+"@"+lineNumbers);
                        }
                    }
                }
            }
        }
    }

    // process parts
    if (parts.size()) {
        _adjust += parts.size();
        QFuture<void> future = QtConcurrent::run([&]() {
            lpub->mi.writeCsiAnnotationMeta(parts,fromHere,toHere,meta,force);
        });
        future.waitForFinished();
        trc = HitCsiAnnotation;
    }

    return static_cast<int>(trc);
}

/*
 *
 * Add the CSI step annotation metas to a list
 * to be accessed by the csiItem constructor
 *
 */

void Step::appendCsiAnnotation(
     const Where           &_here,
     CsiAnnotationMeta     &_caMeta)
{
  CsiAnnotation *ca = new CsiAnnotation(_here,_caMeta);
  csiAnnotations.append(ca);
}

/*
 * LPub is able to pack steps together into multi-step pages or callouts.
 *
 * Multiple steps gathered on a page and callouts share a lot of
 * commonality.  They are organized into rows or columns of steps.
 *
 * From this springs two algorithms, the first algorithm is based on
 * similarity between steps, in that across steps sub-components
 * within steps are placed in sub-columns or sub-rows. This format
 * is common these days in LEGO building instructions.  For lack of
 * a better name, I refer to this modern algorithm as tabular.
 *
 * The other algorithm, which is new to LPub 3, is that of a more
 * free format.
 *
 * These concepts and algorithms are described below.
 *   1. tabular format
 *      a) either Vertically broken down into sub-columns for
 *         csi, pli, stepNumber, rotateIcon and/or callouts.
 *      b) or Horizontally broken down into sub-rows for
 *         csi, pli, stepNumber, rotateIcon and/or callouts.
 *
 *   2. free form format
 *      a) either Vertically composed into columns of steps
 *      b) or rows of steps
 *
 *      This format does not force PLI's or step numbers
 *      to be organized across steps, but it does force steps themselves
 *      to be organized into columns or rows.
 *
 * The default is tabular format because that is the first algorithm
 * implemented.  This is also the most common algorithm used by LEGO
 * today (2007 AD).
 *
 * The free format is similar to the algorithms used early by LEGO
 * and provides the maximum area compression of building instructions,
 * even if they are possibly harder to follow.
 */

/*
 * the algorithms below implement tabular placement.
 *
 * size - allocate step sub-components into sub-rows or sub-columns.
 * place - determine the rectangle that is needed to totally contain
 *   the subcomponents (CSI, step number, PLI, rotateIcon, step-relative callouts.)
 *   Also place the CSI, step number, PLI, rotateIcon and step-relative callouts
 *   within the step's rectangle.
 *
 * making all this look nice takes a few passes:
 *   1.  determine the height and width of each step's sub-columns and
 *       sub-rows.
 *   2.  If we're creating a Vertically allocated multi-step or callout
 *       then make all the sub-columns line up.
 *
 *       If we're creating a Horizontally allocated multi-step or callout
 *       them make all the sub-rows line up.
 *
 * from here we've sized each of the steps.
 *
 * From here, we sum up the the height of each column or row, depending on
 * whether we're creating a Vertical or Horizontal multi-step/callout.  We
 * also keep track of the tallest (widest) column/row within the sets of rows,
 * and how wide (tall) the multi-step/callout is.
 *
 * Now we know the enclosing rectangle for the entire multi-step or callout.
 * Given this we can place the multi-step or callout conglomeration relative
 * to the thing they are to be placed next to.
 *
 * Multi-steps can only be placed relative to the page.
 *
 * Callouts can be place relative to CSI, PLI, step-number, rotateIcon, multi-step, or
 * page.
 */

/*
 * Size the set of ranges by sizing each range
 * and then placing them relative to each other
 */

/*
 * Think of the possible placement as a two dimensional table, of
 * places where something can be placed within a rectangle.
 * -- see step.cpp for detail walkthrough --
 *
 *  CCCCCCCCCCCCCCCCCCC
 *  CSSSSSSSSSSSSSSSSSC
 *  CSCCCCCCCCCCCCCCCSC
 *  CSCRRRRRRRRRRRRRCSC
 *  CSCRCCCCCCCCCCCRCSC
 *  CSCRCMMMMMMMMMCRCSC
 *  CSCRCMCCCCCCCMCRCSC
 *  CSCRCMCPPPPPCMCRCSC
 *  CSCRCMCPCCCPCMCRCSC
 *  CSCRCMCPCACPCMCRCSC
 *  CSCRCMCPCCCPCMCRCSC
 *  CSCRCMCPPPPPCMCRCSC
 *  CSCRCMCCCCCCCMCRCSC
 *  CSCRCMMMMMMMMMCRCSC
 *  CSCRCCCCCCCCCCCRCSC
 *  CSCRRRRRRRRRRRRRCSC
 *  CSCCCCCCCCCCCCCCCSC
 *  CSSSSSSSSSSSSSSSSSC
 *  CCCCCCCCCCCCCCCCCCC
 *
 *  The table above represents either the Horizontal slice
 *  going through the CSI (represented by A for assembly),
 *  or the Vertical slice going through the CSI.
 *
 * C0 - 0  - Callout relative to StepNumber
 * S0 - 1  - StepNumber relative to CSI
 * C1 - 2  - Callout relative to RotateIcon
 * R0 - 3  - RotateIcon relative to CSI
 * C2 - 4  - Callout relative to Submodel
 * M0 - 5  - Submodel relative to CSI
 * C3 - 6  - Callout relative to PLI
 * P0 - 7  - Pli relative to CSI
 * C4 - 8  - Callout relative to CSI
 * A  - 9  - CSI
 * C5 - 10 - Callout relative to CSI
 * P1 - 11 - Pli relative to CSI
 * C6 - 12 - Callout relative to PLI
 * M1 - 13 - Submodel relative to CSI
 * C7 - 14 - Callout relative to Submodel
 * R1 - 15 - RotateIcon relative to CSI
 * C8 - 16 - Callout relative to RotateIcon
 * S1 - 17 - StepNumber relative to CSI
 * C9 - 18 - Callout relative to StepNumber
 *
 */

/*
 * this tells us where to place the stepNumber when placing
 * relative to csi
 */

const int stepNumberPlace[NumPlacements][2] =
{
  { TblSn0, TblSn0 }, // 0 TopLeft
  { TblCsi, TblSn0 }, // 1 Top
  { TblSn1, TblSn0 }, // 2 TopRight
  { TblSn1, TblCsi }, // 3 Righ
  { TblSn1, TblSn1 }, // 4 BottomRigh
  { TblCsi, TblSn1 }, // 5 Bottom
  { TblSn0, TblSn1 }, // 6 BottomLeft
  { TblSn0, TblCsi }, // 7 Left
  { TblCsi, TblCsi },
};

/*
 * this tells us where to place the pli when placing
 * relative to csi
 */

const int pliPlace[NumPlacements][2] =
{
  { TblPli0, TblPli0 }, // 0 TopLeft
  { TblCsi,  TblPli0 }, // 1 Top
  { TblPli1, TblPli0 }, // 2 TopRight
  { TblPli1, TblCsi  }, // 3 Righ
  { TblPli1, TblPli1 }, // 4 BottomRigh
  { TblCsi,  TblPli1 }, // 5 Bottom
  { TblPli0, TblPli1 }, // 6 BottomLeft
  { TblPli0, TblCsi  }, // 7 Left
  { TblCsi,  TblCsi },
};

/*
 * this tells us where to place the Submodel when placing
 * relative to csi
 */

const int subModelPlace[NumPlacements][2] =
{
  { TblSm0, TblSm0 }, // 0 TopLeft
  { TblCsi, TblSm0 }, // 1 Top
  { TblSm1, TblSm0 }, // 2 TopRight
  { TblSm1, TblCsi }, // 3 Righ
  { TblSm1, TblSm1 }, // 4 BottomRigh
  { TblCsi, TblSm1 }, // 5 Bottom
  { TblSm0, TblSm1 }, // 6 BottomLeft
  { TblSm0, TblCsi }, // 7 Left
  { TblCsi, TblCsi },
};

/*
 * relative to csi
 * this tells us where to place a rotateIcon when placing
 */

const int rotateIconPlace[NumPlacements][2] =
{
  { TblRi0, TblRi0 },  // 0 TopLeft
  { TblCsi, TblRi0 },  // 1 Top
  { TblRi1, TblRi0 },  // 2 TopRight
  { TblRi1, TblCsi },  // 3 Righ
  { TblRi1, TblRi1 },  // 4 BottomRigh
  { TblCsi, TblRi1 },  // 5 Bottom
  { TblRi0, TblRi1 },  // 6 BottomLeft
  { TblRi0, TblCsi },  // 7 Left
  { TblCsi, TblCsi },
};

/*
 * this tells us where to place a callout when placing
 * relative to csi
 */

const int coPlace[NumPlacements][2] =
{
  { TblCo4, TblCo4 }, // 0 TopLeft
  { TblCsi, TblCo4 }, // 1 Top
  { TblCo5, TblCo4 }, // 2 TopRight
  { TblCo5, TblCsi }, // 3 Righ
  { TblCo5, TblCo5 }, // 4 BottomRigh
  { TblCsi, TblCo5 }, // 5 Bottom
  { TblCo4, TblCo5 }, // 6 BottomLeft
  { TblCo4, TblCsi }, // 7 Left
  { TblCsi, TblCsi },
};

/*
 * this tells us the row/col offset when placing
 * relative to something other than csi
 */

const int relativePlace[NumPlacements][2] =
{
  { -1, -1 }, // 0 TopLeft
  {  0, -1 }, // 1 Top
  {  1, -1 }, // 2 TopRight
  {  1,  0 }, // 3 Righ
  {  1,  1 }, // 4 BottomRigh
  {  0,  1 }, // 5 Bottom
  { -1,  1 }, // 6 BottomLeft
  { -1,  0 }, // 7 Left
  {  0,  0 },
};

void Step::maxMargin(
    MarginsMeta &margin,
    int tbl[2],
    int marginRows[][2],
    int marginCols[][2])
{
  if (margin.valuePixels(XX) > marginCols[tbl[XX]][0]) {
      marginCols[tbl[XX]][0] = margin.valuePixels(XX);
    }
  if (margin.valuePixels(XX) > marginCols[tbl[XX]][1]) {
      marginCols[tbl[XX]][1] = margin.valuePixels(XX);
    }
  if (margin.valuePixels(YY) > marginRows[tbl[YY]][0]) {
      marginRows[tbl[YY]][0] = margin.valuePixels(YY);
    }
  if (margin.valuePixels(YY) > marginRows[tbl[YY]][1]) {
      marginRows[tbl[YY]][1] = margin.valuePixels(YY);
    }
}

/*
 * This is the first pass of sizing a step.
 *
 *   locate the proper row/col in the placement table (see above)
 *   for each component (csi, pli, subModel, stepNumber, rotateIcon, callout) in the step
 *
 *     locate the proper row/col for those relative to CSI (absolute)
 *
 *     locate the proper row/col for those relative to (pli, stepNumber, subModel, RotateIcon)
 *
 *   determine the largest dimensions for each row/col in the table
 *
 *   record the height of this step
 *
 *   determine the pixel offset for each row/col in the table
 *
 *   place the components Vertically in pixel units using row
 */

int Step::sizeit(
        int  rows[],         // accumulate sub-row heights here
        int  cols[],         // accumulate sub-col widths here
        int  marginRows[][2],// accumulate sub-row margin heights here
        int  marginCols[][2],// accumulate sub-col margin widths here
        int  x,
        int  y)
{

  // size up each callout

  int numCallouts = list.size();

  for (int i = 0; i < numCallouts; i++) {
      list[i]->sizeIt();
    }

  // size up the step number

  if (showStepNumber && ! onlyChild()) {
      stepNumber.sizeit();
    }

  // size up the rotate icon

  if (placeRotateIcon) {
      rotateIcon.sizeit();
    }

  /****************************************************/
  /* figure out who is placed in which row and column */
  /****************************************************/

  csiPlacement.tbl[XX] = TblCsi;
  csiPlacement.tbl[YY] = TblCsi;

  /* Lets start with the absolutes (those relative to the CSI) */

  // PLI relative to CSI

  PlacementData pliPlacement = pli.placement.value();

  if (pliPlacement.relativeTo == CsiType) {
      if (pliPlacement.preposition == Outside) {
          pli.tbl[XX] = pliPlace[pliPlacement.placement][XX];
          pli.tbl[YY] = pliPlace[pliPlacement.placement][YY];
        } else {
          pli.tbl[XX] = TblCsi;
          pli.tbl[YY] = TblCsi;
        }
    }

  // Submodel relative to CSI

  PlacementData subModelPlacement = subModel.placement.value();

  // if SubModel relative to Step Number, but no Step Number, SubModel is relative to CSI

  if (subModelPlacement.relativeTo == StepNumberType && onlyChild()) {
      subModelPlacement.relativeTo = CsiType;
    }

  if (placeSubModel) {
    if (subModelPlacement.relativeTo == CsiType) {
        if (subModelPlacement.preposition == Outside) {
            subModel.tbl[XX] = subModelPlace[subModelPlacement.placement][XX];
            subModel.tbl[YY] = subModelPlace[subModelPlacement.placement][YY];
          } else {
            subModel.tbl[XX] = TblCsi;
            subModel.tbl[YY] = TblCsi;
          }
      }
  }

  // Rotate Icon relative to CSI

  PlacementData rotateIconPlacement = rotateIcon.placement.value();

  if (placeRotateIcon) {
      if (rotateIconPlacement.relativeTo == CsiType) {
          if (rotateIconPlacement.preposition == Outside) {
              rotateIcon.tbl[XX] = rotateIconPlace[rotateIconPlacement.placement][XX];
              rotateIcon.tbl[YY] = rotateIconPlace[rotateIconPlacement.placement][YY];
            } else {
              rotateIcon.tbl[XX] = TblCsi;
              rotateIcon.tbl[YY] = TblCsi;
            }
        }
    }

  PlacementData stepNumberPlacement = stepNumber.placement.value();

  // if Step Number relative to PLI, but no PLI,
  //    Step Number is relative to CSI (Assem)

  if (stepNumberPlacement.relativeTo == PartsListType && ! pliPerStep) {
      stepNumberPlacement.relativeTo = CsiType;
    }

  if (stepNumberPlacement.relativeTo == CsiType) {
      if (stepNumberPlacement.preposition == Outside) {
          stepNumber.tbl[XX] = stepNumberPlace[stepNumberPlacement.placement][XX];
          stepNumber.tbl[YY] = stepNumberPlace[stepNumberPlacement.placement][YY];
        } else {
          stepNumber.tbl[XX] = TblCsi;
          stepNumber.tbl[YY] = TblCsi;
        }
    }

  /* Now lets place things relative to others row/columns */

  /* first the known entities (SN, SM, RI, PLI)*/

  if (pliPlacement.relativeTo == StepNumberType) {
      if (pliPerStep && pli.tsize()) {
          pli.tbl[XX] = stepNumber.tbl[XX]+relativePlace[pliPlacement.placement][XX];
          pli.tbl[YY] = stepNumber.tbl[YY]+relativePlace[pliPlacement.placement][YY];
        } else {
          stepNumber.tbl[XX] = stepNumberPlace[stepNumberPlacement.placement][XX];
          stepNumber.tbl[YY] = stepNumberPlace[stepNumberPlacement.placement][YY];
        }
    }

  if (pliPlacement.relativeTo == SubModelType) {
      if (pliPerStep && pli.tsize()) {
          pli.tbl[XX] = subModel.tbl[XX]+relativePlace[pliPlacement.placement][XX];
          pli.tbl[YY] = subModel.tbl[YY]+relativePlace[pliPlacement.placement][YY];
        } else {
          subModel.tbl[XX] = subModelPlace[subModelPlacement.placement][XX];
          subModel.tbl[YY] = subModelPlace[subModelPlacement.placement][YY];
        }
    }

  if (pliPlacement.relativeTo == RotateIconType) {
      if (pliPerStep && pli.tsize()) {
          pli.tbl[XX] = rotateIcon.tbl[XX]+relativePlace[pliPlacement.placement][XX];
          pli.tbl[YY] = rotateIcon.tbl[YY]+relativePlace[pliPlacement.placement][YY];
        } else {
          rotateIcon.tbl[XX] = rotateIconPlace[rotateIconPlacement.placement][XX];
          rotateIcon.tbl[YY] = rotateIconPlace[rotateIconPlacement.placement][YY];
        }
    }

  /*
  if (subModelPlacement.relativeTo == PartsListType) {
      if (pliPerStep && placeSubModel) {
          subModel.tbl[XX] = pli.tbl[XX]+relativePlace[subModelPlacement.placement][XX];
          subModel.tbl[YY] = pli.tbl[YY]+relativePlace[subModelPlacement.placement][YY];
        } else {
          pli.tbl[XX] = pliPlace[pliPlacement.placement][XX];
          pli.tbl[YY] = pliPlace[pliPlacement.placement][YY];
        }
    }

  if (subModelPlacement.relativeTo == RotateIconType) {
      if (pliPerStep && placeSubModel) {
          subModel.tbl[XX] = rotateIcon.tbl[XX]+relativePlace[subModelPlacement.placement][XX];
          subModel.tbl[YY] = rotateIcon.tbl[YY]+relativePlace[subModelPlacement.placement][YY];
        } else {
          rotateIcon.tbl[XX] = rotateIconPlace[rotateIconPlacement.placement][XX];
          rotateIcon.tbl[YY] = rotateIconPlace[rotateIconPlacement.placement][YY];
        }
    }

  if (subModelPlacement.relativeTo == StepNumberType) {
      if (pliPerStep && placeSubModel) {
          subModel.tbl[XX] = stepNumber.tbl[XX]+relativePlace[subModelPlacement.placement][XX];
          subModel.tbl[YY] = stepNumber.tbl[YY]+relativePlace[subModelPlacement.placement][YY];
        } else {
          stepNumber.tbl[XX] = stepNumberPlace[stepNumberPlacement.placement][XX];
          stepNumber.tbl[YY] = stepNumberPlace[stepNumberPlacement.placement][YY];
        }
    }
  */

  if (stepNumberPlacement.relativeTo == PartsListType) {
      stepNumber.tbl[XX] = pli.tbl[XX]+relativePlace[stepNumberPlacement.placement][XX];
      stepNumber.tbl[YY] = pli.tbl[YY]+relativePlace[stepNumberPlacement.placement][YY];
    }

  if (stepNumberPlacement.relativeTo == SubModelType) {
      stepNumber.tbl[XX] = subModel.tbl[XX]+relativePlace[stepNumberPlacement.placement][XX];
      stepNumber.tbl[YY] = subModel.tbl[YY]+relativePlace[stepNumberPlacement.placement][YY];
    }

  if (stepNumberPlacement.relativeTo == RotateIconType) {
      stepNumber.tbl[XX] = rotateIcon.tbl[XX]+relativePlace[stepNumberPlacement.placement][XX];
      stepNumber.tbl[YY] = rotateIcon.tbl[YY]+relativePlace[stepNumberPlacement.placement][YY];
    }

  if (rotateIconPlacement.relativeTo == PartsListType) {
      if (placeRotateIcon) {
          rotateIcon.tbl[XX] = pli.tbl[XX]+relativePlace[rotateIconPlacement.placement][XX];
          rotateIcon.tbl[YY] = pli.tbl[YY]+relativePlace[rotateIconPlacement.placement][YY];
        } else {
          pli.tbl[XX] = pliPlace[pliPlacement.placement][XX];
          pli.tbl[YY] = pliPlace[pliPlacement.placement][YY];
        }
    }

  if (rotateIconPlacement.relativeTo == SubModelType) {
      if (placeRotateIcon) {
          rotateIcon.tbl[XX] = subModel.tbl[XX]+relativePlace[rotateIconPlacement.placement][XX];
          rotateIcon.tbl[YY] = subModel.tbl[YY]+relativePlace[rotateIconPlacement.placement][YY];
        } else {
          subModel.tbl[XX] = subModelPlace[subModelPlacement.placement][XX];
          subModel.tbl[YY] = subModelPlace[subModelPlacement.placement][YY];
        }
    }

  if (rotateIconPlacement.relativeTo == StepNumberType) {
      if (placeRotateIcon) {
          rotateIcon.tbl[XX] = stepNumber.tbl[XX]+relativePlace[rotateIconPlacement.placement][XX];
          rotateIcon.tbl[YY] = stepNumber.tbl[YY]+relativePlace[rotateIconPlacement.placement][YY];
        } else {
          stepNumber.tbl[XX] = stepNumberPlace[stepNumberPlacement.placement][XX];
          stepNumber.tbl[YY] = stepNumberPlace[stepNumberPlacement.placement][YY];
        }
    }

  maxMargin(pli.margin,pli.tbl,marginRows,marginCols);
//  maxMargin(subModel.margin,subModel.tbl,marginRows,marginCols);
  maxMargin(stepNumber.margin,stepNumber.tbl,marginRows,marginCols);
  maxMargin(csiPlacement.margin,csiPlacement.tbl,marginRows,marginCols);
  maxMargin(rotateIcon.margin,rotateIcon.tbl,marginRows,marginCols);

  /* now place the callouts relative to the known items (CSI, PLI, SM, SN, RI) */

  int calloutSize[2] = { 0, 0 };
  bool shared = false;

  int square[NumPlaces][NumPlaces];

  for (int i = 0; i < NumPlaces; i++) {
      for (int j = 0; j < NumPlaces; j++) {
          square[i][j] = -1;
        }
    }

  square[TblCsi][TblCsi] = CsiType;
  square[pli.tbl[XX]][pli.tbl[YY]] = PartsListType;
//  square[subModel.tbl[XX]][subModel.tbl[YY]] = SubModelType;
  square[stepNumber.tbl[XX]][stepNumber.tbl[YY]] = StepNumberType;
  square[rotateIcon.tbl[XX]][rotateIcon.tbl[YY]] = RotateIconType;

  int pixmapSize[2] = { csiPixmap.width(), csiPixmap.height() };
  int max = pixmapSize[y];

  for (int i = 0; i < numCallouts; i++) {
      Callout *callout = list[i];

      PlacementData calloutPlacement = callout->placement.value();
      bool sharable = true;
      bool onSide = false;

      if (calloutPlacement.relativeTo == CsiType) {
          onSide = x == XX ? (calloutPlacement.placement == Left ||
                              calloutPlacement.placement == Right)
                           : (calloutPlacement.placement == Top ||
                              calloutPlacement.placement == Bottom);
        }

      if (onSide) {
          if (max < callout->size[y]) {
              max = callout->size[y];
            }
        }

      PlacementEnc rp = calloutPlacement.placement;
      switch (calloutPlacement.relativeTo) {
        case CsiType:
          if (calloutPlacement.preposition == Outside) {
            callout->tbl[XX] = coPlace[rp][XX];
            callout->tbl[YY] = coPlace[rp][YY];
          } else {
            callout->tbl[XX] = TblCsi;
            callout->tbl[YY] = TblCsi;
          }
          break;
        case PartsListType:
          callout->tbl[XX] = pli.tbl[XX] + relativePlace[rp][XX];
          callout->tbl[YY] = pli.tbl[YY] + relativePlace[rp][YY];
          break;
        case SubModelType:
          callout->tbl[XX] = subModel.tbl[XX] + relativePlace[rp][XX];
          callout->tbl[YY] = subModel.tbl[YY] + relativePlace[rp][YY];
          break;
        case StepNumberType:
          callout->tbl[XX] = stepNumber.tbl[XX] + relativePlace[rp][XX];
          callout->tbl[YY] = stepNumber.tbl[YY] + relativePlace[rp][YY];
          break;
        case RotateIconType:
          callout->tbl[XX] = rotateIcon.tbl[XX] + relativePlace[rp][XX];
          callout->tbl[YY] = rotateIcon.tbl[YY] + relativePlace[rp][YY];
          break;
        default:
          sharable = false;
          break;
        }

      if ( ! pliPerStep ) {
          sharable = false;
        }

      square[callout->tbl[XX]][callout->tbl[YY]] = i + 1;
      int size = callout->submodelStack().size();
      if (sharable && size > 1) {
          if (callout->tbl[x] < TblCsi && callout->tbl[y] == TblCsi) {
              if (calloutSize[x] < callout->size[x]) {
                  calloutSize[XX] = callout->size[XX];
                  calloutSize[YY] = callout->size[YY];
                }
              callout->shared = true;
              shared = true;
            }
        }
    }

  /*now place the SubModel relative to the known items (CSI, PLI, SN, RI) */

  int subModelSize[2] = { 0, 0 };
  bool smShared = false;

  if (placeSubModel) {
     SubModel *_subModel = &subModel;

     PlacementData subModelPlacement = _subModel->placement.value();
     bool smSharable = true;
     bool onSide = false;

     if (subModelPlacement.relativeTo == CsiType) {
         onSide = x == XX ? (subModelPlacement.placement == Left ||
                             subModelPlacement.placement == Right)
                          : (subModelPlacement.placement == Top ||
                             subModelPlacement.placement == Bottom);
     }

     if (onSide) {
         if (max < _subModel->size[y]) {
             max = _subModel->size[y];
         }
     }

     int rp = subModelPlacement.placement;
     switch (subModelPlacement.relativeTo) {
       case CsiType:
         _subModel->tbl[XX] = coPlace[rp][XX];
         _subModel->tbl[YY] = coPlace[rp][YY];
         break;
       case PartsListType:
         _subModel->tbl[XX] = pli.tbl[XX] + relativePlace[rp][XX];
         _subModel->tbl[YY] = pli.tbl[YY] + relativePlace[rp][YY];
         break;
       case StepNumberType:
         _subModel->tbl[XX] = stepNumber.tbl[XX] + relativePlace[rp][XX];
         _subModel->tbl[YY] = stepNumber.tbl[YY] + relativePlace[rp][YY];
         break;
       case RotateIconType:
         _subModel->tbl[XX] = rotateIcon.tbl[XX] + relativePlace[rp][XX];
         _subModel->tbl[YY] = rotateIcon.tbl[YY] + relativePlace[rp][YY];
         break;
       default:
         smSharable = false;
         break;
     }

     if ( ! pliPerStep ) {
         smSharable = false;
       }

     square[_subModel->tbl[XX]][_subModel->tbl[YY]] = 1;
     /*int size = _subModel->submodelStack().size(); */
     if (smSharable /*&& size > 1*/) {
         if (_subModel->tbl[x] < TblCsi && _subModel->tbl[y] == TblCsi) {
            if (subModelSize[x] < _subModel->size[x]) {
                subModelSize[XX] = _subModel->size[XX];
                subModelSize[YY] = _subModel->size[YY];
            }
            _subModel->shared = true;
            smShared = true;
         }
      }
   }

  /************************************************/
  /*                                              */
  /* Determine the biggest in each column and row */
  /*                                              */
  /************************************************/

  // PLI
  if (pli.pliMeta.constrain.isDefault()) {

      int tsize = 0;

      switch (pliPlacement.placement) {
        case Top:
        case Bottom:
          tsize = csiPlacement.size[XX];
          pli.sizePli(ConstrainData::PliConstrainWidth,tsize);
          if (pli.size[YY] > lpub->pageSize(lpub->page.meta.LPub.page, YY)/3) {
              pli.sizePli(ConstrainData::PliConstrainArea,tsize);
            }
          break;
        case Left:
        case Right:
          tsize = csiPlacement.size[YY];
          pli.sizePli(ConstrainData::PliConstrainHeight,tsize);
          if (pli.size[XX] > lpub->pageSize(lpub->page.meta.LPub.page, XX)/3) {
              pli.sizePli(ConstrainData::PliConstrainArea,tsize);
            }
          break;
        default:
          pli.sizePli(ConstrainData::PliConstrainArea,tsize);
          break;
        }
    }

  // SM
  if (subModel.subModelMeta.constrain.isDefault()) {

      int tsize = 0;

      switch (subModelPlacement.placement) {
        case Top:
        case Bottom:
          tsize = csiPlacement.size[XX];
          subModel.sizeSubModel(ConstrainData::PliConstrainWidth,tsize);
          if (subModel.size[YY] > lpub->pageSize(lpub->page.meta.LPub.page, YY)/3) {
              subModel.sizeSubModel(ConstrainData::PliConstrainArea,tsize);
            }
          break;
        case Left:
        case Right:
          tsize = csiPlacement.size[YY];
          subModel.sizeSubModel(ConstrainData::PliConstrainHeight,tsize);
          if (subModel.size[XX] > lpub->pageSize(lpub->page.meta.LPub.page, XX)/3) {
              subModel.sizeSubModel(ConstrainData::PliConstrainArea,tsize);
            }
          break;
        default:
          subModel.sizeSubModel(ConstrainData::PliConstrainArea,tsize);
          break;
        }
    }

  // Allow PLI and CALLOUT to share one column

  if (shared && pli.tbl[y] == TblCsi) {
      int wX = 0, wY = 0;
      if (x == XX) {
          wX = pli.size[XX] + calloutSize[XX];
          wY = pli.size[YY];
        } else {
          wX = pli.size[XX];
          wY = pli.size[YY] + calloutSize[YY];
        }
      if (cols[pli.tbl[XX]] < wX) {
          cols[pli.tbl[XX]] = wX;
        }
      if (rows[pli.tbl[YY]] < wY) {
          rows[pli.tbl[YY]] = wY;
        }
    } else {

      bool addOn = true;

      /* Drop the PLI down on top of the CSI, and reduce the pli's size */

      if (onlyChild()) {
          switch (pliPlacement.placement) {
            case Top:
            case Bottom:
              if (pliPlacement.relativeTo == CsiType) {
                  if ( ! collide(square,pli.tbl,y, x)) {
                      int height = (max - pixmapSize[y])/2;
                      if (height > 0) {
                          if (height >= pli.size[y]) {  // entire thing fits
                              rows[pli.tbl[y]] = 0;
                              addOn = false;
                            } else {                      // fit what we can
                              rows[pli.tbl[y]] = pli.size[y] - height;
                              addOn = false;
                            }
                        }
                    }
                }
              break;
            default:
              break;
            }
        }

      if (cols[pli.tbl[XX]] < pli.size[XX]) {
          cols[pli.tbl[XX]] = pli.size[XX];  // HINT 1
        }
      if (addOn) {
          if (rows[pli.tbl[YY]] < pli.size[YY]) {
              rows[pli.tbl[YY]] = pli.size[YY];
            }
        }
    }

  // Allow SUBMODEL and CALLOUT to share one column

  if (shared && subModel.tbl[y] == TblCsi) {
      int wX = 0, wY = 0;
      if (x == XX) {
          wX = subModel.size[XX] + calloutSize[XX];
          wY = subModel.size[YY];
      } else {
          wX = subModel.size[XX];
          wY = subModel.size[YY] + calloutSize[YY];
      }
      if (cols[subModel.tbl[XX]] < wX) {
          cols[subModel.tbl[XX]] = wX;
      }
      if (rows[subModel.tbl[YY]] < wY) {
          rows[subModel.tbl[YY]] = wY;
      }

  } else {

      bool addOn = true;

      /* Drop the SubModel down on top of the CSI, and reduce the SubModel's size */

      if (onlyChild()) {
          switch (pliPlacement.placement) {
          case Top:
          case Bottom:
              if (pliPlacement.relativeTo == CsiType) {
                  if ( ! collide(square,subModel.tbl,y, x)) {
                      int height = (max - pixmapSize[y])/2;
                      if (height > 0) {
                          if (height >= subModel.size[y]) { // entire thing fits
                              rows[subModel.tbl[y]] = 0;
                              addOn = false;
                          } else {                        // fit what we can
                              rows[subModel.tbl[y]] = subModel.size[y] - height;
                              addOn = false;
                          }
                      }
                  }
              }
              break;
          default:
              break;
          }
      }

      if (cols[subModel.tbl[XX]] < subModel.size[XX]) {
          cols[subModel.tbl[XX]] = subModel.size[XX];  // HINT 1
      }
      if (addOn) {
          if (rows[subModel.tbl[YY]] < subModel.size[YY]) {
              rows[subModel.tbl[YY]] = subModel.size[YY];
          }
      }
  }

  // Allow PLI and SUBMODEL to share one column

  if (smShared && pli.tbl[y] == TblCsi) {
    int wX = 0, wY = 0;
    if (x == XX) {
        wX = pli.size[XX] + subModelSize[XX];
        wY = pli.size[YY];
      } else {
        wX = pli.size[XX];
        wY = pli.size[YY] + subModelSize[YY];
      }
    if (cols[pli.tbl[XX]] < wX) {
        cols[pli.tbl[XX]] = wX;
      }
    if (rows[pli.tbl[YY]] < wY) {
        rows[pli.tbl[YY]] = wY;
      }
  }

  /******************************************************************/
  /* Determine col/row for each step component (e.g. Step Number,   */
  /* CSI and RotateIcon                                             */
  /******************************************************************/

  if (cols[stepNumber.tbl[XX]] < stepNumber.size[XX]) {
      cols[stepNumber.tbl[XX]] = stepNumber.size[XX];
    }

  if (rows[stepNumber.tbl[YY]] < stepNumber.size[YY]) {
      rows[stepNumber.tbl[YY]] = stepNumber.size[YY];
    }

  if (cols[TblCsi] < csiPlacement.size[XX]) {
      cols[TblCsi] = csiPlacement.size[XX];
    }

  if (rows[TblCsi] < csiPlacement.size[YY]) {
      rows[TblCsi] = csiPlacement.size[YY];
    }

  if (cols[rotateIcon.tbl[XX]] < rotateIcon.size[XX]) {
      cols[rotateIcon.tbl[XX]] = rotateIcon.size[XX];
    }

  if (rows[rotateIcon.tbl[YY]] < rotateIcon.size[YY]) {
      rows[rotateIcon.tbl[YY]] = rotateIcon.size[YY];
    }

  // adjust rows and columns for rotate icon offset
  if (adjustOnItemOffset)
      adjustSize(*dynamic_cast<Placement*>(&rotateIcon),
             rows,cols);

  /******************************************************************/
  /* Determine col/row and margin for each callout that is relative */
  /* to step components (e.g. not page or multiStep)                */
  /******************************************************************/

  for (int i = 0; i < numCallouts; i++) {
      Callout *callout = list[i];
      switch (callout->placement.value().relativeTo) {
        case CsiType:
        case PartsListType:
        case StepNumberType:
        case RotateIconType:
          if (callout->shared && rows[TblCsi] < callout->size[y]) {
              rows[TblCsi] = callout->size[y];
            } else {

              if (cols[callout->tbl[XX]] < callout->size[XX]) {
                  cols[callout->tbl[XX]] = callout->size[XX];
                }
              if (rows[callout->tbl[YY]] < callout->size[YY]) {
                  rows[callout->tbl[YY]] = callout->size[YY];
                }

              maxMargin(callout->margin,
                        callout->tbl,
                        marginRows,
                        marginCols);
            }

          // adjust rows and columns for callout offset
          if (adjustOnItemOffset)
              adjustSize(*dynamic_cast<Placement*>(callout),
                     rows,cols);

          break;
        default:
          break;
        }
    }

  /******************************************************************/
  /* Determine col/row and margin for subModel that is relative     */
  /* to step components (e.g. not page or multiStep)                */
  /******************************************************************/


  if (placeSubModel) {
    SubModel *_subModel = &subModel;

    switch (_subModel->placement.value().relativeTo) {
      case CsiType:
      case PartsListType:
      case StepNumberType:
      case RotateIconType:
        if (_subModel->shared && rows[TblCsi] < _subModel->size[y]) {
            rows[TblCsi] = _subModel->size[y];
          } else {

            if (cols[_subModel->tbl[XX]] < _subModel->size[XX]) {
                cols[_subModel->tbl[XX]] = _subModel->size[XX];
              }
            if (rows[_subModel->tbl[YY]] < _subModel->size[YY]) {
                rows[_subModel->tbl[YY]] = _subModel->size[YY];
              }

            maxMargin(_subModel->margin,
                      _subModel->tbl,
                      marginRows,
                      marginCols);
          }
        break;
      default:
        break;
      }
  }

  return 0;
}

bool Step::adjustSize(
  Placement &pl1, // relative type item placement
  int  rows[],    // adjust sub-row heights here
  int  cols[])    // adjust sub-col widths here
{
    auto hasOffset = [](Placement &_pl, bool suppressOffset = false)
    {
        if(_pl.placement.value().offsets[XX] != 0.0f ||
           _pl.placement.value().offsets[YY] != 0.0f) {
            if (suppressOffset) {
                PlacementData pld = _pl.placement.value();
                pld.offsets[XX] = 0.0f;
                pld.offsets[YY] = 0.0f;
                _pl.placement.setValue(pld);
            }
            return true;
        }
        return false;
    };

    // confirm offset item
    if (!hasOffset(pl1))
        return false;

    // set relative type item default placement (without offset)
    Placement pl2 = pl1;
    Placement pl_1, pl_2;

    hasOffset(pl2,true/*suppressOffset*/);

    int rtX = 0;           // relative to cols
    int rtY = 0;           // relative to rows
    int plX = pl1.tbl[XX]; // relative type cols
    int plY = pl1.tbl[YY]; // relative type rows

    // populate offset 'pl_1' and default 'pl_2' relativeTo placement
    switch (pl1.placement.value().relativeTo) {
    case CsiType:
      rtX = TblCsi;
      rtY = TblCsi;
      pl_1 = pl_2 = csiPlacement;
    break;
    case PartsListType:
      rtX = pli.tbl[XX];
      rtY = pli.tbl[YY];
      pl_1 = pl_2 = pli;
    break;
    case SubModelType:
      rtX = subModel.tbl[XX];
      rtY = subModel.tbl[YY];
      pl_1 = pl_2 = subModel;
    break;
    case StepNumberType:
      rtX = stepNumber.tbl[XX];
      rtY = stepNumber.tbl[YY];
      pl_1 = pl_2 = stepNumber;
    break;
    case RotateIconType:
      rtX = rotateIcon.tbl[XX];
      rtY = rotateIcon.tbl[YY];
      pl_1 = pl_2 = rotateIcon;
    break;
    default:
    break;
    }

    Q_UNUSED(rtX)
    Q_UNUSED(rtY)

    pl_1.placeRelative(&pl1);
    pl_2.placeRelative(&pl2);

    // define relative type offset rect
    QRectF rectPl1(pl1.boundingLoc[XX],
                   pl1.boundingLoc[YY],
                   pl1.size[XX],
                   pl1.size[YY]);
    // define relative type item default rect
    QRectF rectPl2(pl2.boundingLoc[XX],
                   pl2.boundingLoc[YY],
                   pl2.size[XX],
                   pl2.size[YY]);

    // calculate the length and/or width delta
    // between the original 'pl2' rect position and
    // the offset 'pl1' rect position and adjust the
    // respective row 'height' and/or column 'width' value
    qreal deltaX = 0.0;
    qreal deltaY = 0.0;
    QRectF rectInt;

    // if relative type offset rect position intersects default rect position
    bool intersected = rectPl1.intersects(rectPl2);
    if (intersected) {
        // get intersect rectangle
        rectInt = rectPl2.intersected(rectPl1);
        // determine x-axis delta, adjust relative type column [Width]
        deltaX = rectPl2.width() - rectInt.width();
        // determine y-axis delta, adjust relative type row [Height]
        deltaY = rectPl2.height() - rectInt.height();
    }
    // no  rect intersect
    else {
        // determine x-axis delta, adjust relative type column [Width]
        deltaX     = qAbs(rectPl1.x() - rectPl2.x());
        // determine y-axis delta, adjust relative type row [Height]
        deltaY     = qAbs(rectPl1.y() - rectPl2.y());
        // create rectangle
        rectInt = QRectF(0,0,deltaX,deltaY);
    }

    // set rows and columns
    bool canAdjust = false;
    int indexAdjust  = -1;
    PlacementEnc placementEnc = pl2.placement.value().placement;
    switch (placementEnc) {
    case Left:
    case Right:
        //set column  (width X) offset adjustment (plX)
        if (rectPl1.x() > rectPl2.x())
            cols[plX] = int(rectPl2.width() + deltaX);
        if (rectPl1.x() < rectPl2.x())
            cols[plX] = int(rectPl2.width() - deltaX);

        // set row (height Y) offset adjustment (plY)
        if (rectPl1.y() > rectPl2.y())
            if ((canAdjust = rows[plY+1] < int(deltaY)))
                indexAdjust = plY+1;
        if (rectPl1.y() < rectPl2.y())
            if ((canAdjust = rows[plY-1] < int(deltaY)))
                indexAdjust = plY-1;
        if (canAdjust)
            rows[indexAdjust] = int(deltaY);
        break;
    case Top:
    case Bottom:
        //set column (width X) offset adjustment (plX)
        if (rectPl1.x() > rectPl2.x())
            if ((canAdjust  = cols[plX+1] < int(deltaX)))
                indexAdjust = plX+1;
        if (rectPl1.x() < rectPl2.x())
            if ((canAdjust  = cols[plX-1] < int(deltaX)))
                indexAdjust = plX-1;
        if (canAdjust) {
            if (indexAdjust)
                cols[indexAdjust] = int(deltaX);
            cols[plX] = (int(pl1.relativeToSize[XX]));
        }

        // set row (height Y) offset adjustment (plY)
        if (rectPl1.y() > rectPl2.y()) {
            if (rows[plY+1] < int(deltaY))
                rows[intersected ? plY+1 : plY+2] = int(intersected ? deltaY : deltaY - rows[plY+1]);
            if (placementEnc == Top)
                rows[plY] = int(intersected ? rectInt.height() : 0);
        }
        if (rectPl1.y() < rectPl2.y()) {
            if (rows[plY-1] < int(deltaY))
                rows[intersected ? plY-1 : plY-2] = int(intersected ? deltaY : deltaY - rows[plY-1]);;
            if (placementEnc == Bottom)
                rows[plY] = int(intersected ? rectInt.height() : 0);
        }
        break;
    default:
        break;
    }

    return true;
}

bool Step::collide(
    int square[NumPlaces][NumPlaces],
    int tbl[],
    int x,
    int y)
{
  int place;
  for (place = tbl[x]; place < TblCsi; place++) {
      if (square[place][y] != -1) {
          return true;
        }
    }
  return false;
}

int Step::maxMargin(int &top, int &bot, int y)
{
  top = csiPlacement.margin.valuePixels(y);
  bot = top;
  int top_tbl = TblCsi;
  int bot_tbl = TblCsi;

  if (stepNumber.tbl[YY] < TblCsi) {
    top = stepNumber.margin.valuePixels(y);
    top_tbl = stepNumber.tbl[y];
  } else if (stepNumber.tbl[y] == TblCsi) {
    int margin = stepNumber.margin.valuePixels(y);
    top = qMax(top,margin);
    bot = qMax(bot,margin);
  } else {
    bot = stepNumber.margin.valuePixels(y);
    bot_tbl = stepNumber.tbl[y];
  }

  if (pli.size[y]) {
    if (pli.tbl[y] < TblCsi) {
      top = pli.margin.valuePixels(y);
      top_tbl = pli.tbl[y];
    } else if (stepNumber.tbl[y] == TblCsi) {
      int margin = pli.margin.valuePixels(y);
      top = qMax(top,margin);
      bot = qMax(bot,margin);
    } else {
      bot = pli.margin.valuePixels(y);
      bot_tbl = pli.tbl[y];
    }
  }

  if (placeSubModel) {
    if (subModel.tbl[y] < TblCsi) {
      top = subModel.margin.valuePixels(y);
      top_tbl = subModel.tbl[y];
    } else if (stepNumber.tbl[y] == TblCsi) {
      int margin = subModel.margin.valuePixels(y);
      top = qMax(top,margin);
      bot = qMax(bot,margin);
    } else {
      bot = subModel.margin.valuePixels(y);
      bot_tbl = subModel.tbl[y];
    }
  }

  if (placeRotateIcon) {
    if (rotateIcon.tbl[YY] < TblCsi) {
      top = rotateIcon.margin.valuePixels(y);
      top_tbl = rotateIcon.tbl[y];
    } else if (stepNumber.tbl[y] == TblCsi) {
      int margin = rotateIcon.margin.valuePixels(y);
      top = qMax(top,margin);
      bot = qMax(bot,margin);
    } else {
      bot = rotateIcon.margin.valuePixels(y);
      bot_tbl = rotateIcon.tbl[y];
    }
  }

  for (int i = 0; i < list.size(); i++) {
    Callout *callout = list[i];
    if (callout->tbl[y] < TblCsi) {
      top = callout->margin.valuePixels(y);
      top_tbl = callout->tbl[y];
    } else if (stepNumber.tbl[y] == TblCsi) {
      int margin = callout->margin.valuePixels(y);
      top = qMax(top,margin);
      bot = qMax(bot,margin);
    } else {
      bot = callout->margin.valuePixels(y);
      bot_tbl = callout->tbl[y];
    }
  }

  return top > bot ? top_tbl : bot_tbl;
}

/***************************************************************************
 * This routine is used for tabular multi-steps.  It is used to determine
 * the location of csi, pli, sm, stepNumber, rotateIcon and step relative callouts.
 ***************************************************************************/

void Step::placeit(
    int rows[],
    int margins[],
    int _y,
    bool shared)
{
  /*********************************/
  /* record the origin of each row */
  /*********************************/
  int origin         = 0;
  int space          = 0;
  int marginAccum    = 0;
  Dim y = static_cast<Dim>(_y);
  bool centerJustify = justifyStep.value().type == JustifyCenter ||
                       justifyStep.value().type == JustifyCenterVertical;
  int spacing        = centerJustify ? justifyStep.spacingValuePixels() / 2 : 0.0f;

  int originsIni[NumPlaces];
  int origins[NumPlaces];
  int spaceOffset[NumPlaces];

  for (int i = 0; i < NumPlaces; i++) {
     originsIni[i]  = origin;
     spaceOffset[i] = space;

     if (rows[i]) {
        marginAccum += margins[i];
        space       += spacing;
        origin      += centerJustify ? rows[i] : rows[i] + margins[i];
    }
  }

  int calculatedSize = centerJustify ? origin + marginAccum : origin;
  bool adjustSize    = stepSize.valuePixels(y) && stepSize.valuePixels(y) != calculatedSize;
  int sizeAdjustment = 0; // contents justify top/left
  if (adjustSize) {
    // justify center, horizontal and vertical when manual step size specified
    bool justify = false;
    if (justifyStep.value().type == JustifyCenter)
      justify = true;
    else if (justifyStep.value().type == JustifyCenterVertical && y == XX)
      justify = true;
    else if (justifyStep.value().type == JustifyCenterHorizontal && y == YY)
      justify = true;
    sizeAdjustment = justify ? (stepSize.valuePixels(y) - calculatedSize) / 2 : 0;

    if (stepSize.valuePixels(y) < calculatedSize) {
      Where here       = stepSize.here();
      float sizeValue  = float(calculatedSize/resolution());
      QString message  = QString("The specified Step %1 %2 is less than its calculated %1 %3.")
                                 .arg(y == XX ? "width" : "height")
                                 .arg(QString::number(double(stepSize.value(y)),'f',4))
                                 .arg(resolutionType() == DPCM ? QString::number(double(centimeters2inches(sizeValue)),'f',4)
                                                               : QString::number(double(sizeValue),'f',4));
      logInfo() << qPrintable(message);
    }
  }

  size[y] = adjustSize ? stepSize.valuePixels(y) : calculatedSize;

  for (int i = 0; i < NumPlaces; i++) {
     origins[i] = originsIni[i] + sizeAdjustment;
     if (rows[i]) {
        if (centerJustify) {
           origins[i] += spaceOffset[i] + (marginAccum / 2);
        }
     }
  }

  /*******************************************/
  /* Now place the components in pixel units */
  /*******************************************/

  csiPlacement.loc[y] = origins[TblCsi] + (rows[TblCsi] - csiPlacement.size[y])/2;
  pli.loc[y]          = origins[pli.tbl[y]];
  subModel.loc[y]     = origins[subModel.tbl[y]];
  stepNumber.loc[y]   = origins[stepNumber.tbl[y]];
  rotateIcon.loc[y]   = origins[rotateIcon.tbl[y]];

  switch (y) {
  case XX:
     if (! shared) {
        pli.justifyX(origins[pli.tbl[y]],rows[pli.tbl[y]]);
        if(placeSubModel) {
           subModel.justifyX(origins[subModel.tbl[y]],rows[subModel.tbl[y]]);
        }
     }
     if(placeRotateIcon) {
        rotateIcon.justifyX(origins[rotateIcon.tbl[y]],rows[rotateIcon.tbl[y]]);
     }
     stepNumber.justifyX(origins[stepNumber.tbl[y]],rows[stepNumber.tbl[y]]);
     break;
  case YY:
     if (! shared) {
        pli.justifyY(origins[pli.tbl[y]],rows[pli.tbl[y]]);
        if(placeSubModel) {
           subModel.justifyY(origins[subModel.tbl[y]],rows[subModel.tbl[y]]);
        }
     }
     if(placeRotateIcon) {
        rotateIcon.justifyY(origins[rotateIcon.tbl[y]],rows[rotateIcon.tbl[y]]);
     }
     stepNumber.justifyY(origins[stepNumber.tbl[y]],rows[stepNumber.tbl[y]]);
     break;
  default:
     break;
  }

  /* Relative justify 'y' on outside placement - see ticket #690, #694, #703, #729 */

  Page *page = dynamic_cast<Page *>(grandparent());
  if (page) {
     /* Apply when the page is a single range - no divider. */
     const bool singleRange = page->list.size() == 1;

     if (singleRange && shared) {
        PlacementData& pd = pli.placement.value();
        if (pd.rectPlacement == TopOutside || pd.rectPlacement == BottomOutside) {
           switch (pli.placement.value().relativeTo) {
           case CsiType:
              csiPlacement.justifyRelative(&pli,y);
              break;
           case SubModelType:
              subModel.justifyRelative(&pli,y);
              break;
           case StepNumberType:
              stepNumber.justifyRelative(&pli,y);
              break;
           case RotateIconType:
              rotateIcon.justifyRelative(&pli,y);
              break;
           default:
              break;
           }
        }
        if(placeSubModel) {
           PlacementData& pd = subModel.placement.value();
           if (pd.rectPlacement == TopOutside || pd.rectPlacement == BottomOutside) {
              switch (subModel.placement.value().relativeTo) {
              case CsiType:
                csiPlacement.justifyRelative(&subModel,y);
                break;
              case PartsListType:
                pli.justifyRelative(&subModel,y);
                break;
              case StepNumberType:
                stepNumber.justifyRelative(&subModel,y);
                break;
              case RotateIconType:
                break;
              default:
                break;
              }
           }
        }
     }

     if(singleRange && placeRotateIcon) {
        PlacementData& pd = rotateIcon.placement.value();
        if (pd.rectPlacement == TopOutside || pd.rectPlacement == BottomOutside) {
           switch (rotateIcon.placement.value().relativeTo) {
           case CsiType:
              csiPlacement.justifyRelative(&rotateIcon,y);
              break;
           case PartsListType:
              pli.justifyRelative(&rotateIcon,y);
              break;
           case SubModelType:
              subModel.justifyRelative(&rotateIcon,y);
              break;
           case StepNumberType:
              stepNumber.justifyRelative(&rotateIcon,y);
              break;
           default:
              break;
           }
        }
     }

     if(singleRange || justifyYAxisOutsidePlacement) {
        PlacementData& pd = stepNumber.placement.value();
        if (pd.preposition == Outside && pd.placement == Bottom) {
           switch (stepNumber.placement.value().relativeTo) {
           case CsiType:
              csiPlacement.justifyRelative(&stepNumber,y);
              break;
           case PartsListType:
              pli.justifyRelative(&stepNumber,y);
              break;
           case SubModelType:
              subModel.justifyRelative(&stepNumber,y);
              break;
           case RotateIconType:
              rotateIcon.justifyRelative(&stepNumber,y);
              break;
           default:
              break;
           }
        }
     }
  } /* Relative justify 'y' on outside placement - end */

  /* place the callouts that are relative to step components */

  for (int i = 0; i < list.size(); i++) {
      Callout *callout = list[i];
      PlacementData calloutPlacement = callout->placement.value();
      bool noOffset = calloutPlacement.offsets[0] == 0 && calloutPlacement.offsets[1] == 0;

      if (shared && callout->shared) {
          if (callout->size[y] > origins[TblCsi]) {
              int locY = callout->size[y] - origins[TblCsi] - margins[TblCsi];
              callout->loc[y] = locY;
            } else {
              int locY = origins[TblCsi] - callout->size[y] - margins[TblCsi];
              callout->loc[y] = locY;
            }
        } else {
          switch (calloutPlacement.relativeTo) {
            case CsiType:
            case PartsListType:
            case SubModelType:
            case StepNumberType:
            case RotateIconType:
              callout->loc[y] = origins[callout->tbl[y]];
              if (callout->shared)
                callout->loc[y] -= callout->margin.value(y) - 500;
              if (y == YY)
                callout->justifyY(origins[callout->tbl[y]], rows[callout->tbl[y]]);
              else
                callout->justifyX(origins[callout->tbl[y]], rows[callout->tbl[y]]);
              if (noOffset && calloutPlacement.preposition == Inside) {
                if (calloutPlacement.relativeTo == CsiType)
                  csiPlacement.placeRelative(callout);
              }
              break;
            default:
              break;
            }
        }
    }

    /* place the subModel that is relative to step components */

    if (placeSubModel) {
      SubModel *_subModel = &subModel;
      PlacementData subModelPlacement = _subModel->placement.value();

      if (shared && _subModel->shared) {
          if (_subModel->size[y] > origins[TblCsi]) {
              int locY = _subModel->size[y] - origins[TblCsi] - margins[TblCsi];
              _subModel->loc[y] = locY;
            } else {
              int locY = origins[TblCsi] - _subModel->size[y] - margins[TblCsi];
              _subModel->loc[y] = locY;
            }
        } else {
          switch (subModelPlacement.relativeTo) {
            case CsiType:
            case PartsListType:
            case StepNumberType:
            case RotateIconType:
              if (subModelPlacement.preposition == Outside) {
                _subModel->loc[y] = origins[_subModel->tbl[y]];
                if (_subModel->shared) {
                    _subModel->loc[y] -= _subModel->margin.value(y) - 500;
                  }

                if (y == YY) {
                    _subModel->justifyY(origins[_subModel->tbl[y]],
                        rows[_subModel->tbl[y]]);
                  } else {
                    _subModel->justifyX(origins[_subModel->tbl[y]],
                        rows[_subModel->tbl[y]]);
                  }
              } else if (subModelPlacement.relativeTo == CsiType) {
                csiPlacement.placeRelative(_subModel);
              }
              break;
            default:
              break;
            }
        }
    }
}

/*
 * This method is independent of Horizontal/Vertical multi-step/callout
 * allocation, or tabular vs. freeform mode.
 */

void Step::addGraphicsItems(
    int             offsetX,
    int             offsetY,
    Meta           *meta,
    PlacementType   parentRelativeType,
    QGraphicsItem  *parent,
    bool            movable)
{
  offsetX += loc[XX];
  offsetY += loc[YY];

  // Background Rectangle Item
  if (multiStep) {
      stepBackground =
              new MultiStepStepBackgroundItem(
                  this,
                  parent);
      stepBackground->setPos(offsetX,
                             offsetY);
  }

  // CSI
  csiItem = new CsiItem(
                        this,
                        meta,
                        csiPixmap,
                        submodelLevel,
                        parent,
                        parentRelativeType);
  csiItem->assign(&csiPlacement);
  csiItem->setPos(offsetX + csiItem->loc[XX],
                  offsetY + csiItem->loc[YY]);
  csiItem->setFlag(QGraphicsItem::ItemIsMovable,movable);

  // CSI annotations
  if (csiItem->assem->annotation.display.value() &&
      ! Gui::exportingObjects())
      csiItem->placeCsiPartAnnotations();

  // PLI
  if (pli.tsize()) {
      pli.addPli(submodelLevel, parent);
      pli.setPos(offsetX + pli.loc[XX],
                 offsetY + pli.loc[YY]);
    }

  // SM
  if (subModel.tsize()) {
      subModel.addSubModel(submodelLevel, parent);
      subModel.setPos(offsetX + subModel.loc[XX],
                      offsetY + subModel.loc[YY]);
  }

  // Step Number
  if (showStepNumber && ! onlyChild() && stepNumber.number) {
      StepNumberItem *sn;
      if (calledOut) {
          sn = new StepNumberItem(this,
                                  parentRelativeType,
                                  numberPlacemetMeta,
                                  "%d",
                                  stepNumber.number,
                                  parent);
      } else {
          sn = new StepNumberItem(this,
                                  parentRelativeType,
                                  numberPlacemetMeta,
                                  "%d",
                                  stepNumber.number,
                                  parent);
      }
      sn->setPos(offsetX + stepNumber.loc[XX],
                 offsetY + stepNumber.loc[YY]);

      sn->setFlag(QGraphicsItem::ItemIsMovable,movable);
    }

  // Rotate Icon
  if (placeRotateIcon) {
      RotateIconItem *ri;
      if (calledOut) {

          ri = new RotateIconItem(this,
                                  parentRelativeType,
                                  rotateIconMeta,
                                  parent);
      } else {
          ri = new RotateIconItem(this,
                                  parentRelativeType,
                                  rotateIconMeta,
                                  parent);
      }
      // here we are using the placement values for this specific step in the step group
      rotateIcon.sizeit();
      switch (rotateIcon.placement.value().relativeTo) {
      case CsiType:
          csiPlacement.placeRelative(&rotateIcon);
          break;
      case PartsListType:
          pli.placeRelative(&rotateIcon);
          break;
      case SubModelType:
          subModel.placeRelative(&rotateIcon);
          break;
      case StepNumberType:
          stepNumber.placeRelative(&rotateIcon);
          break;
      case RotateIconType:
      default:
          break;
      }
      ri->assign(&rotateIcon);
      ri->setPos(offsetX + ri->loc[XX],
                 offsetY + ri->loc[YY]);
      ri->setFlag(QGraphicsItem::ItemIsMovable,/*movable*/true);
    }

  // Callouts
  for (int i = 0; i < list.size(); i++) {

      Callout *callout = list[i];
      PlacementData placementData = callout->placement.value();

      QRect rect(csiItem->loc[XX],
                 csiItem->loc[YY],
                 csiItem->size[XX],
                 csiItem->size[YY]);

      if (placementData.relativeTo == CalloutType) {
          callout->addGraphicsItems(offsetX-loc[XX],offsetY-loc[YY],rect,parent, movable);
        } else {
          bool callout_movable = movable;
          if (parentRelativeType == StepGroupType && (placementData.relativeTo == PageType ||
                                                      placementData.relativeTo == CsiType  ||
                                                      placementData.relativeTo == PartsListType)) {
              callout_movable = true;
          }
          callout->addGraphicsItems(callout->shared ? 0 : offsetX,offsetY,rect,parent, callout_movable);
        }
      for (int i = 0; i < callout->pointerList.size(); i++) {
          Pointer *pointer = callout->pointerList[i];
          callout->parentStep = this;
          callout->addGraphicsPointerItem(pointer,callout->underpinnings);
        }
    }
}

void Step::placeInside()
{
  if (pli.placement.value().preposition == Inside) {
      switch (pli.placement.value().relativeTo) {
        case CsiType:
          csiPlacement.placeRelative(&pli);
          break;
        case PartsListType:
          break;
        case SubModelType:
          subModel.placeRelative(&pli);
          break;
        case StepNumberType:
          stepNumber.placeRelative(&pli);
          break;
        case RotateIconType:
          rotateIcon.placeRelative(&pli);
          break;
        default:
          break;
        }
    }

  if (subModel.placement.value().preposition == Inside) {
      switch (subModel.placement.value().relativeTo) {
        case CsiType:
          csiPlacement.placeRelative(&subModel);
          break;
        case PartsListType:
          pli.placeRelative(&subModel);
          break;
        case SubModelType:
          break;
        case StepNumberType:
          stepNumber.placeRelative(&subModel);
          break;
        case RotateIconType:
          break;
        default:
          break;
        }
    }

  if (stepNumber.placement.value().preposition == Inside) {
      switch (stepNumber.placement.value().relativeTo) {
        case CsiType:
          csiPlacement.placeRelative(&stepNumber);
          break;
        case PartsListType:
          pli.placeRelative(&stepNumber);
          break;
        case SubModelType:
          subModel.placeRelative(&stepNumber);
          break;
        case StepNumberType:
          break;
        case RotateIconType:
          rotateIcon.placeRelative(&stepNumber);
          break;
        default:
          break;
        }
    }

  if (rotateIcon.placement.value().preposition == Inside) {
      switch (rotateIcon.placement.value().relativeTo) {
        case CsiType:
          csiPlacement.placeRelative(&rotateIcon);
          break;
        case PartsListType:
          pli.placeRelative(&rotateIcon);
          break;
        case SubModelType:
          subModel.placeRelative(&rotateIcon);
          break;
        case StepNumberType:
          stepNumber.placeRelative(&rotateIcon);
          break;
        case RotateIconType:
          break;
        default:
          break;
        }
    }

  for (int i = 0; i < list.size(); i++) {

      Callout *callout = list[i];
      PlacementData placementData = callout->placement.value();

      /* Offset Callouts */

      int relativeToSize[2];

      relativeToSize[XX] = 0;
      relativeToSize[YY] = 0;

      switch (placementData.relativeTo) {
        case CsiType:
          relativeToSize[XX] = csiPlacement.size[XX];
          relativeToSize[YY] = csiPlacement.size[YY];
          break;
        case PartsListType:
          relativeToSize[XX] = pli.size[XX];
          relativeToSize[YY] = pli.size[YY];
          break;
        case SubModelType:
          relativeToSize[XX] = subModel.size[XX];
          relativeToSize[YY] = subModel.size[YY];
          break;
        case StepNumberType:
          relativeToSize[XX] = stepNumber.size[XX];
          relativeToSize[YY] = stepNumber.size[YY];
          break;
        case RotateIconType:
          relativeToSize[XX] = rotateIcon.size[XX];
          relativeToSize[YY] = rotateIcon.size[YY];
          break;
        default:
          break;
        }
      callout->loc[XX] += relativeToSize[XX]*placementData.offsets[XX];
      callout->loc[YY] += relativeToSize[YY]*placementData.offsets[YY];
    }
}

/*********************************************************************
 *
 * This section implements a second, more freeform version of packing
 * steps into callouts and multiSteps.
 *
 * Steps being oriented into sub-columns or sub-rows with
 * step columns or rows, this rendering technique dues not necessarily
 * get you the most compact result.
 *
 *  In single step per page the placement algorithm is very flexible.
 * Anything can be placed relative to anything, as long as the placement
 * relationships lead everyone back to the page, then all things will
 * be placed.
 *
 * In free-form placement, some placement component is the root of all
 * placement (CSI, PLI, STEP_NUMBER, ROTATE_ICON).  All other placement components
 * are placed relative to the base, or placed relative to things that
 * are placed relative to the root.
 *
 ********************************************************************/

void Step::sizeitFreeform(
    int xx,
    int yy,
    int relativeBase,
    int relativeJustification,
    int &left,
    int &right)
{
  Q_UNUSED(relativeJustification);
  // size up each callout

  for (int i = 0; i < list.size(); i++) {
      Callout *callout = list[i];
      if (callout->meta.LPub.callout.freeform.value().mode) {
          callout->sizeitFreeform(xx,yy);
        } else {
          callout->sizeIt();
        }
    }

  // place each callout

  for (int i = 0; i < list.size(); i++) {
      Callout *callout = list[i];

      if (callout->meta.LPub.callout.freeform.value().mode) {
          if (callout->meta.LPub.callout.freeform.value().justification == Left ||
              callout->meta.LPub.callout.freeform.value().justification == Top) {
              callout->loc[xx] = callout->size[xx];
            }
        } else {
          callout->sizeIt();
        }
    }

  // size up the step number

  if (showStepNumber && ! onlyChild()) {
      stepNumber.sizeit();
    }

  // size up the Submodel

//  if (placeSubModel) {
//      subModel.sizeit();
//    }

  // size up the rotateIcon

  if (placeRotateIcon) {
      rotateIcon.sizeit();
    }

  // place everything relative to the base

  int offsetX = 0, sizeX = 0;

  PlacementData placementData;

  switch (relativeBase) {
    case CsiType:
      placementData = csiPlacement.placement.value();
      placementData.relativeTo = PageType;
      csiPlacement.placement.setValue(placementData);
      csiPlacement.relativeTo(this);
      offsetX = csiPlacement.loc[xx];
      sizeX   = csiPlacement.size[yy];
      break;
    case PartsListType:
      placementData = pli.placement.value();
      placementData.relativeTo = PageType;
      pli.placement.setValue(placementData);
      pli.relativeTo(this);
      offsetX = pli.loc[xx];
      sizeX   = pli.size[yy];
      break;
    case SubModelType:
      placementData = subModel.placement.value();
      placementData.relativeTo = PageType;
      subModel.placement.setValue(placementData);
      subModel.relativeTo(this);
      offsetX = subModel.loc[xx];
      sizeX   = subModel.size[yy];
      break;
    case StepNumberType:
      placementData = stepNumber.placement.value();
      placementData.relativeTo = PageType;
      stepNumber.placement.setValue(placementData);
      stepNumber.relativeTo(this);
      offsetX = stepNumber.loc[xx];
      sizeX   = stepNumber.size[xx];
      break;
    case RotateIconType:
      placementData = rotateIcon.placement.value();
      placementData.relativeTo = PageType;
      rotateIcon.placement.setValue(placementData);
      rotateIcon.relativeTo(this);
      offsetX = rotateIcon.loc[xx];
      sizeX   = rotateIcon.size[xx];
      break;
    }

  // FIXME: when we get here for callouts that are to the left of the CSI
  // the outermost box is correctly placed, but within there the CSI is
  // in the upper left hand corner, even if it has a callout to the left of
  // it
  //
  // Have to determine the leftmost edge of any callouts
  //   Left of CSI
  //   Left edge of Top|Bottom Center or Right justified - we need place

  // size the step

  for (int dim = XX; dim <= YY; dim++) {

      int min = 500000;
      int max = 0;

      if (csiPlacement.loc[dim] < min) {
          min = csiPlacement.loc[dim];
        }
      if (csiPlacement.loc[dim] + csiPlacement.size[dim] > max) {
          max = csiPlacement.loc[dim] + csiPlacement.size[dim];
        }
      if (pli.loc[dim] < min) {
          min = pli.loc[dim];
        }
      if (pli.loc[dim] + pli.size[dim] > max) {
          max = pli.loc[dim] + pli.size[dim];
        }
      if (subModel.loc[dim] < min) {
          min = subModel.loc[dim];
        }
      if (subModel.loc[dim] + subModel.size[dim] > max) {
          max = subModel.loc[dim] + subModel.size[dim];
        }
      if (stepNumber.loc[dim] < min) {
          min = stepNumber.loc[dim];
        }
      if (stepNumber.loc[dim] + stepNumber.size[dim] > max) {
          max = stepNumber.loc[dim] + stepNumber.size[dim];
        }
      if (rotateIcon.loc[dim] < min) {
          min = rotateIcon.loc[dim];
        }
      if (rotateIcon.loc[dim] + rotateIcon.size[dim] > max) {
          max = rotateIcon.loc[dim] + rotateIcon.size[dim];
        }

      for (int i = 0; i < list.size(); i++) {
          Callout *callout = list[i];
          if (callout->loc[dim] < min) {
              min = callout->loc[dim];
            }
          if (callout->loc[dim] + callout->size[dim] > max) {
              max = callout->loc[dim] + callout->size[dim];
            }
        }

      if (calledOut) {
          csiPlacement.loc[dim]  -= min;
          pli.loc[dim]           -= min;
          subModel.loc[dim]      -= min;
          stepNumber.loc[dim]    -= min;
          rotateIcon.loc[dim]    -= min;

          for (int i = 0; i < list.size(); i++) {
              Callout *callout = list[i];
              callout->loc[dim] -= min;
            }
        }

      size[dim] = max - min;

      if (dim == XX) {
          left = min;
          right = max;
        }
    }

  /* Now make all things relative to the base */

  csiPlacement.loc[xx] -= offsetX + sizeX;
  pli.loc[xx]          -= offsetX + sizeX;
  subModel.loc[xx]     -= offsetX + sizeX;
  stepNumber.loc[xx]   -= offsetX + sizeX;
  rotateIcon.loc[xx]   -= offsetX + sizeX;

  for (int i = 0; i < list.size(); i++) {
      list[i]->loc[xx] -= offsetX + sizeX;
    }
}
