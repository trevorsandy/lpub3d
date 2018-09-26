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
 * This class represents one step including a step number, and assembly
 * image, possibly a parts list image and zero or more callouts needed for
 * the step.
 *
 * Please see lpub.h for an overall description of how the files in LPub
 * make up the LPub program.
 *
 ***************************************************************************/

#include <QFileInfo>
#include <QDir>
#include <QFile>

#include "lpub.h"
#include "step.h"
#include "range.h"
#include "ranges.h"
#include "ranges_element.h"
#include "render.h"
#include "callout.h"
#include "calloutbackgrounditem.h"
#include "pointer.h"
#include "calloutpointeritem.h"
#include "pli.h"
#include "numberitem.h"
#include "csiitem.h"
#include "resolution.h"
#include "dependencies.h"
#include "paths.h"
#include "ldrawfiles.h"
#include <LDVQt/LDVImageMatte.h>

/*********************************************************************
 *
 * Create a new step and remember the meta-command state at the time
 * that it was created.
 *
 ********************************************************************/

Step::Step(
    Where                  &topOfStep,
    AbstractStepsElement  *_parent,
    int                     num,            // step number as seen by the user
    Meta                  &_meta,           // the current state of the meta-commands
    bool                    calledOut,      // if we're a callout
    bool                    multiStep)
  : calledOut(calledOut)
  , multiStep(multiStep)
{
  top                       =  topOfStep;
  parent                    = _parent;
  submodelLevel             = _meta.submodelStack.size();
  stepNumber.number         =  num;             // record step number
  csiItem                   = nullptr;

  modelDisplayOnlyStep      = false;
  relativeType              = StepType;
  csiPlacement.relativeType = CsiType;
  stepNumber.relativeType   = StepNumberType;
  rotateIcon.relativeType   = RotateIconType;

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
      csiPlacement.margin     = _meta.LPub.callout.csi.margin;
      csiPlacement.placement  = _meta.LPub.callout.csi.placement;
      pli.margin              = _meta.LPub.callout.pli.margin;
      pli.placement           = _meta.LPub.callout.pli.placement;
      rotateIcon.placement    = _meta.LPub.callout.rotateIcon.placement;
      rotateIcon.margin       = _meta.LPub.callout.rotateIcon.margin;
      rotateIconMeta          = _meta.LPub.callout.rotateIcon;
      numberPlacemetMeta      = _meta.LPub.callout.stepNum;
      stepNumber.placement    = _meta.LPub.callout.stepNum.placement;
      stepNumber.font         = _meta.LPub.callout.stepNum.font.valueFoo();
      stepNumber.color        = _meta.LPub.callout.stepNum.color.value();
      stepNumber.margin       = _meta.LPub.callout.stepNum.margin;
      pliPerStep              = _meta.LPub.callout.pli.perStep.value();
    } else if (multiStep) {
      csiPlacement.margin     = _meta.LPub.multiStep.csi.margin;
      csiPlacement.placement  = _meta.LPub.multiStep.csi.placement;
      pli.margin              = _meta.LPub.multiStep.pli.margin;
      pli.placement           = _meta.LPub.multiStep.pli.placement;
      rotateIcon.placement    = _meta.LPub.multiStep.rotateIcon.placement;
      rotateIcon.margin       = _meta.LPub.multiStep.rotateIcon.margin;
      rotateIconMeta          = _meta.LPub.multiStep.rotateIcon;
      numberPlacemetMeta      = _meta.LPub.multiStep.stepNum;
      stepNumber.placement    = _meta.LPub.multiStep.stepNum.placement;
      stepNumber.font         = _meta.LPub.multiStep.stepNum.font.valueFoo();
      stepNumber.color        = _meta.LPub.multiStep.stepNum.color.value();
      stepNumber.margin       = _meta.LPub.multiStep.stepNum.margin;
      pliPerStep              = _meta.LPub.multiStep.pli.perStep.value();
    } else {
      csiPlacement.margin     = _meta.LPub.assem.margin;
      csiPlacement.placement  = _meta.LPub.assem.placement;
      placement               = _meta.LPub.assem.placement;
      pli.margin              = _meta.LPub.assem.margin;
      pli.placement           = _meta.LPub.pli.placement;
      rotateIcon.placement    = _meta.LPub.rotateIcon.placement;
      rotateIcon.margin       = _meta.LPub.rotateIcon.margin;
      stepNumber.placement    = _meta.LPub.stepNumber.placement;
      stepNumber.font         = _meta.LPub.stepNumber.font.valueFoo();
      stepNumber.color        = _meta.LPub.stepNumber.color.value();
      stepNumber.margin       = _meta.LPub.stepNumber.margin;
      pliPerStep              = false;
    }
  pli.steps                 = grandparent();
  pli.step                  = this;
  showStepNumber            = _meta.LPub.assem.showStepNumber.value();
  rotateIcon.setSize(         _meta.LPub.rotateIcon.size,
                              _meta.LPub.rotateIcon.border.valuePixels().thickness);
  placeRotateIcon           = false;

}

/* step destructor destroys all callouts */

Step::~Step() {
  for (int i = 0; i < list.size(); i++) {
      Callout *callout = list[i];
      delete callout;
    }
  list.clear();
  pli.clear();
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

/*
 * given a set of parts, generate a CSI
 */

int Step::createCsi(
    QString     const &addLine,
    QStringList const &csiParts,  // the partially assembles model
    QPixmap           *pixmap,
    Meta              &meta,
    bool               bfxLoad)   // Bfx load special case (no parts added)
{
  bool    csiExist        = false;
  bool    nativeRenderer  = (Preferences::preferredRenderer == RENDERER_NATIVE ||
                            (Preferences::preferredRenderer == RENDERER_POVRAY &&
                             Preferences::povFileGenerator == RENDERER_NATIVE));
  QString csi_Name        = modelDisplayOnlyStep ? csiName()+"_fm" : bfxLoad ? csiName()+"_bfx" : csiName();
  qreal   modelScale      = meta.LPub.assem.modelScale.value();
  bool    doFadeStep      = meta.LPub.fadeStep.fadeStep.value();
  bool    doHighlightStep = meta.LPub.highlightStep.highlightStep.value() && !gui->suppressColourMeta();
  bool    invalidIMStep   = ((modelDisplayOnlyStep) || (stepNumber.number == 1));
  if (meta.rotStep.value().type == "ABS") meta.LPub.assem.cameraAngles.setValues(0.0f,0.0f);

  QString viewerCsiName;

  ldrName.clear();

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

  // Define csi file paths
  QString csiLdrFilePath = QString("%1/%2").arg(QDir::currentPath()).arg(Paths::tmpDir);
  QString csiPngFilePath = QString("%1/%2").arg(QDir::currentPath()).arg(Paths::assemDir);
  QString csiLdrFile = QString("%1/csi.ldr").arg(csiLdrFilePath);

  QString key = QString("%1_%2_%3_%4_%5_%6_%7_%8_%9")
      .arg(csi_Name+orient)
      .arg(stepNumber.number)
      .arg(gui->pageSize(meta.LPub.page, 0))
      .arg(resolution())
      .arg(resolutionType() == DPI ? "DPI" : "DPCM")
      .arg(modelScale)
      .arg(meta.LPub.assem.cameraFoV.value())
      .arg(meta.LPub.assem.cameraAngles.value(0))
      .arg(meta.LPub.assem.cameraAngles.value(1));;

  // populate png name
  pngName = QString("%1/%2.png").arg(csiPngFilePath).arg(key);

  // create ImageMatte csiKey
  csiKey = QString("%1_%2").arg(csi_Name).arg(stepNumber.number);

  // add csiKey and pngName to ImageMatte repository - exclude first step
  if (Preferences::enableFadeSteps && Preferences::enableImageMatting && !invalidIMStep) {
      if (!LDVImageMatte::validMatteCSIImage(csiKey))
          LDVImageMatte::insertMatteCSIImage(csiKey, pngName);
    }

  // Check if png file date modified is older than madel file (on the stack) date modified
  csiOutOfDate = false;

  QFile csi(pngName);
  csiExist = csi.exists();
  if (csiExist) {
      QDateTime lastModified = QFileInfo(pngName).lastModified();
      QStringList parsedStack = submodelStack();
      parsedStack << parent->modelName();
      if ( ! isOlder(parsedStack,lastModified)) {
          csiOutOfDate = true;
          if (! csi.remove()) {
              emit gui->messageSig(LOG_ERROR,QMessageBox::tr("Failed to remove out of date CSI PNG file."));
          }
      }
  }

  int rc;

  // Populate viewerCsiName variable
  viewerCsiName = QString("%1;%2;%3%4")
          .arg(top.modelName)
          .arg(top.lineNumber)
          .arg(stepNumber.number)
          .arg(modelDisplayOnlyStep ? "_fm" : "");

  // Viewer Csi does not yet exist in repository
  bool addViewerStepContent = !gui->viewerStepContentExist(viewerCsiName);
  // We are processing again the current step so Csi must have been updated by from viewer
  bool viewerUpdate = viewerCsiName == gui->getViewerCsiName();

  // Generage 3DViewer CSI entry
  if ( addViewerStepContent || csiOutOfDate || viewerUpdate ) {

      // set rotated parts
      QStringList rotatedParts = csiParts;

      // rotate parts for 3DViewer display
      if ((rc = renderer->rotateParts(addLine,meta.rotStep,rotatedParts)) != 0)
        emit gui->messageSig(LOG_ERROR,QMessageBox::tr("Failed to rotate viewer CSI parts"));

      // add ROTSTEP command
      rotatedParts.prepend(renderer->getRotstepMeta(meta.rotStep));

      // header and closing meta
      QString modelName = QFileInfo(top.modelName).baseName().toLower();
      modelName = modelName.replace(modelName.at(0),modelName.at(0).toUpper());
      rotatedParts.prepend(QString("0 !LEOCAD MODEL NAME %1%2").arg(modelName).arg(modelDisplayOnlyStep ? " (Final Model)" : ""));
      rotatedParts.prepend(QString("0 %1%2").arg(modelName).arg(modelDisplayOnlyStep ? " (Final Model)" : ""));
      rotatedParts.prepend(QString("0 FILE %1").arg(top.modelName));
      rotatedParts.append("0 NOFILE");

      // consolidate subfiles and parts into single file - I don't think this is still needed but I keep it anyway
      createViewerCSI(rotatedParts, doFadeStep, doHighlightStep);

      gui->insertViewerStep(viewerCsiName,rotatedParts,csiLdrFile,multiStep,calledOut);
  }

  // Generate renderer CSI file
  if ( ! csiExist || csiOutOfDate ) {

     QElapsedTimer timer;
     timer.start();

     // populate ldr file name
     ldrName = QString("%1/%2.ldr").arg(csiLdrFilePath).arg(key);

     // create the CSI ldr file and rotate its parts for single call LDView and Native renderers
     if (renderer->useLDViewSCall() || nativeRenderer) {

         if (nativeRenderer)
            ldrName = csiLdrFile;

         if ((rc = renderer->rotateParts(addLine, meta.rotStep, csiParts, ldrName, top.modelName)) != 0) {
             emit gui->messageSig(LOG_ERROR,QMessageBox::tr("Failed to create and rotate CSI ldr file: %1.")
                                                   .arg(ldrName));
             return rc;
         }
     }

     if (!renderer->useLDViewSCall()) {
         // render the partially assembled model
         QStringList csiKeys;
         if (nativeRenderer)
             csiKeys = (QStringList() << viewerCsiName);
         else
             csiKeys = (QStringList() << csiKey); // adding just a single key

         if ((rc = renderer->renderCsi(addLine, csiParts, csiKeys, pngName, meta)) != 0) {
             emit gui->messageSig(LOG_ERROR,QMessageBox::tr("Render CSI part failed for %1.")
                                                            .arg(pngName));
             return rc;
         }

         emit gui->messageSig(LOG_INFO, qPrintable(
                                  QString("%1 CSI render call took %2 milliseconds "
                                          "to render %3 for %4 %5 %6 on page %7.")
                                          .arg(Render::getRenderer())
                                          .arg(timer.elapsed())
                                          .arg(pngName)
                                          .arg(calledOut ? "called out," : "simple,")
                                          .arg(multiStep ? "step group" : "single step")
                                          .arg(stepNumber.number)
                                          .arg(gui->stepPageNum)));
     }
  }

  // Load the 3DViewer
  if (! gui->exporting() /* && !Preferences::preferredRenderer == RENDERER_NATIVE */) {

      // set viewer camera options
      viewerOptions.ViewerCsiName  = viewerCsiName;
      viewerOptions.FoV            = meta.LPub.assem.cameraFoV.value();
      viewerOptions.Latitude       = meta.LPub.assem.cameraAngles.value(0);
      viewerOptions.Longitude      = meta.LPub.assem.cameraAngles.value(1);

      if (! renderer->LoadViewer(viewerOptions)) {
          emit gui->messageSig(LOG_ERROR,QMessageBox::tr("Load viewer failed for csi_Name: %1")
                               .arg(viewerCsiName));
          return -1;
      }
  }

  // If not using LDView SCall, populate pixmap
  if (! renderer->useLDViewSCall()) {
      pixmap->load(pngName);
      csiPlacement.size[0] = pixmap->width();
      csiPlacement.size[1] = pixmap->height();
    }

  return 0;
}

// create 3D Viewer version of the csi file
int Step::createViewerCSI(
    QStringList &csiRotatedParts,
    bool         doFadeStep,
    bool         doHighlightStep)
{
  QStringList csiSubModels;
  QStringList csiSubModelParts;
  QStringList csiParts = csiRotatedParts;

  QStringList argv;
  bool        alreadyInserted;
  int         rc;

  if (csiRotatedParts.size() > 0) {

      for (int index = 0; index < csiRotatedParts.size(); index++) {

          QString csiLine = csiRotatedParts[index];
          split(csiLine, argv);
          if (argv.size() == 15 && argv[0] == "1") {

              /* process subfiles in csiRotatedParts */
              QString type = argv[argv.size()-1];

              bool isCustomSubModel = false;
              bool isCustomPart = false;
              QString customType;

              // Custom part types
              if (doFadeStep) {
                  bool isFadedItem = type.contains("-fade.");
                  // Fade file
                  if (isFadedItem) {
                      customType = type;
                      customType = customType.replace("-fade.",".");
                      isCustomSubModel = gui->isSubmodel(customType);
                      isCustomPart = gui->isUnofficialPart(customType);
                    }
                }
              else
                if (doHighlightStep) {
                    bool isHighlightItem = type.contains("-highlight");
                    // Highlight file
                    if (isHighlightItem) {
                        customType = type;
                        customType = customType.replace("-highlight.",".");
                        isCustomSubModel = gui->isSubmodel(customType);
                        isCustomPart = gui->isUnofficialPart(customType);
                      }
                  }

              if (gui->isSubmodel(type) || gui->isUnofficialPart(type) || isCustomSubModel || isCustomPart) {
                  /* capture subfiles (full string) to be processed when finished */
                  if (!csiSubModels.contains(type))
                       csiSubModels << type.toLower();
                }
            }
        } //end for

      /* process extracted submodels and unofficial files */
      if (csiSubModels.size() > 0){
          if (csiSubModels.size() > 2)
              csiSubModels.removeDuplicates();
          if ((rc = mergeViewerCSISubModels(csiSubModels, csiSubModelParts, doFadeStep, doHighlightStep)) != 0){
              emit gui->messageSig(LOG_ERROR,QMessageBox::tr("Failed to process viewer CSI submodels"));
              return rc;
            }
        }

      /* add sub model content to csiRotatedParts file */
      if (! csiSubModelParts.empty())
        {
          for (int i = 0; i < csiSubModelParts.size(); i++) {
              QString smLine = csiSubModelParts[i];
              csiParts << smLine;
            }
        }
      csiRotatedParts = csiParts;
    }
  return 0;
}

int Step::mergeViewerCSISubModels(QStringList &subModels,
                                  QStringList &subModelParts,
                                  bool doFadeStep,
                                  bool doHighlightStep)
{
  QStringList csiSubModels        = subModels;
  QStringList csiSubModelParts    = subModelParts;
  QStringList newSubModels;

  QStringList argv;
  int         rc;

  if (csiSubModels.size() > 0) {

      /* read in all detected sub model file content */
      for (int index = 0; index < csiSubModels.size(); index++) {
          QString ldrName(QDir::currentPath() + "/" +
                          Paths::tmpDir + "/" +
                          csiSubModels[index]);

          /* initialize the working submodel file - define header. */
          QString modelName = QFileInfo(csiSubModels[index]).baseName().toLower();
          modelName = modelName.replace(modelName.at(0),modelName.at(0).toUpper());
          csiSubModelParts << QString("0 FILE %1").arg(csiSubModels[index]);
          csiSubModelParts << QString("0 %1").arg(modelName);
          csiSubModelParts << QString("0 !LEOCAD MODEL NAME %1").arg(modelName);

          /* read the actual submodel file */
          QFile ldrfile(ldrName);
          if ( ! ldrfile.open(QFile::ReadOnly | QFile::Text)) {
              emit gui->messageSig(LOG_ERROR,QMessageBox::tr("Could not read CSI submodel file %1: %2")
                                   .arg(ldrName)
                                   .arg(ldrfile.errorString()));
              return -1;
            }
          /* populate file contents into working submodel csi parts */
          QTextStream in(&ldrfile);
          while ( ! in.atEnd()) {
              QString csiLine = in.readLine(0);
              split(csiLine, argv);

              if (argv.size() == 15 && argv[0] == "1") {
                  /* check and process any subfiles in csiRotatedParts */
                  QString type = argv[argv.size()-1];

                  bool isCustomSubModel = false;
                  bool isCustomPart = false;
                  QString customType;

                  // Custom part types
                  if (doFadeStep) {
                      bool isFadedItem = type.contains("-fade.");
                      // Fade file
                      if (isFadedItem) {
                          customType = type;
                          customType = customType.replace("-fade.",".");
                          isCustomSubModel = gui->isSubmodel(customType);
                          isCustomPart = gui->isUnofficialPart(customType);
                        }
                    }
                  else
                    if (doHighlightStep) {
                        bool isHighlightItem = type.contains("-highlight");
                        // Highlight file
                        if (isHighlightItem) {
                            customType = type;
                            customType = customType.replace("-highlight.",".");
                            isCustomSubModel = gui->isSubmodel(customType);
                            isCustomPart = gui->isUnofficialPart(customType);
                          }
                      }

                  if (gui->isSubmodel(type) || gui->isUnofficialPart(type) || isCustomSubModel || isCustomPart) {
                      /* capture all subfiles (full string) to be processed when finished */
                      if (!newSubModels.contains(type))
                              newSubModels << type.toLower();
                    }
                }
              csiLine = argv.join(" ");
              csiSubModelParts << csiLine;
            }
          csiSubModelParts << "0 NOFILE";
        }

      /* recurse and process any identified submodel files */
      if (newSubModels.size() > 0){
          if (newSubModels.size() > 2)
              newSubModels.removeDuplicates();
          if ((rc = mergeViewerCSISubModels(newSubModels, csiSubModelParts, doFadeStep, doHighlightStep)) != 0){
              emit gui->messageSig(LOG_ERROR,QMessageBox::tr("Failed to recurse viewer CSI submodels"));
              return rc;
            }
        }
      subModelParts = csiSubModelParts;
    }
  return 0;
}

/*
 * LPub is able to pack steps together into multi-step pages or callouts.
 *
 * Multiple steps gathered on a page and callouts share a lot of
 * commonality.  They are organized into rows or columns of steps.
 *
 * From this springs two algorithms, the first algorithm is based on
 * similarity between steps, in that that across steps sub-components
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
 *   the subcomponents (CSI, step number, PLI, roitateIcon, step-relative callouts.)
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
 * places where something can be placed within a step's rectangle.
 *
 *  CCCCCCCCCCCCCCC
 *  CRRRRRRRRRRRRRC
 *  CRCCCCCCCCCCCRC
 *  CRCSSSSSSSSSCRC
 *  CRCSCCCCCCCSCRC
 *  CRCSCPPPPPCSCRC
 *  CRCSCPCCCPCSCRC
 *  CRCSCPCACPCSCRC
 *  CRCSCPCCCPCSCRC
 *  CRCSCPPPPPCSCRC
 *  CRCSCCCCCCCSCRC
 *  CRCSSSSSSSSSCRC
 *  CRCCCCCCCCCCCRC
 *  CRRRRRRRRRRRRRC
 *  CCCCCCCCCCCCCCC

 *  C0 - callout relative to rotateIcon
 *  R0 - rotateIcon relateive to csi
 *  C1 - callout relative to step number
 *  S0 - step number relative to csi
 *  C2 - callout relative to PLI
 *  P0 - pli relative to csi
 *  C3 - callout relative to csi
 *  A  - csi
 *  C4 - callout relative to csi
 *  P1 - pli relative to csi
 *  C5 - callout relative to PLI
 *  S1 - step number relative to csi
 *  C6 - callout relative to step number
 *  R1 - rotateIcon relateive to csi
 *  C7 - callout relative to rotateIcon
 */

/*
 * this tells us where to place the stepNumber when placing
 * relative to csi
 */

const int stepNumberPlace[NumPlacements][2] =
{
  { TblSn0, TblSn0 },  // Top_Left
  { TblCsi, TblSn0 },  // Top
  { TblSn1, TblSn0 },  // Top_Right
  { TblSn1, TblCsi },  // Right
  { TblSn1, TblSn1 },  // BOTTOM_RIGHT
  { TblCsi, TblSn1 },  // BOTTOM
  { TblSn0, TblSn1 },  // BOTTOM_LEFT
  { TblSn0, TblCsi },  // LEFT
  { TblCsi, TblCsi },
};

/*
 * this tells us where to place the pli when placing
 * relative to csi
 */

const int pliPlace[NumPlacements][2] =
{
  { TblPli0, TblPli0 }, // Top_Left
  { TblCsi,  TblPli0 }, // Top
  { TblPli1, TblPli0 }, // Top_Right
  { TblPli1, TblCsi  }, // Right
  { TblPli1, TblPli1 }, // BOTTOM_RIGHT
  { TblCsi,  TblPli1 }, // BOTTOM
  { TblPli0, TblPli1 }, // BOTTOM_LEFT
  { TblPli0, TblCsi  }, // LEFT
  { TblCsi,  TblCsi },
};

/*
 * this tells us where to place a rotateIcon when placing
 * relative to csi
 */

const int rotateIconPlace[NumPlacements][2] =
{
  { TblRi0, TblRi0 },  // Top_Left
  { TblCsi, TblRi0 },  // Top
  { TblRi1, TblRi0 },  // Top_Right
  { TblRi1, TblCsi },  // Right
  { TblRi1, TblRi1 },  // BOTTOM_RIGHT
  { TblCsi, TblRi1 },  // BOTTOM
  { TblRi0, TblRi1 },  // BOTTOM_LEFT
  { TblRi0, TblCsi },  // LEFT
  { TblCsi, TblCsi },
};

/*
 * this tells us where to place a callout when placing
 * relative to csi
 */

const int coPlace[NumPlacements][2] =
{
  { TblCo3, TblCo3 }, // Top_Left
  { TblCsi, TblCo3 }, // Top
  { TblCo4, TblCo3 }, // Top_Right
  { TblCo4, TblCsi }, // Right
  { TblCo4, TblCo4 }, // BOTTOM_RIGHT
  { TblCsi, TblCo4 }, // BOTTOM
  { TblCo3, TblCo4 }, // BOTTOM_LEFT
  { TblCo3, TblCsi }, // LEFT
  { TblCsi, TblCsi },
};

/*
 * this tells us the row/col offset when placing
 * relative to something other than csi
 */

const int relativePlace[NumPlacements][2] =
{
  { -1, -1 },
  {  0, -1 },
  {  1, -1 },
  {  1,  0 },
  {  1,  1 },
  {  0,  1 },
  { -1,  1 },
  { -1,  0 },
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
 *   for each component (csi, pli, stepNumber, roateIcon, callout) in the step
 *
 *     locate the proper row/col for those relative to CSI (absolute)
 *
 *     locate the proper row/col for those relative to (pli,stepNumber)
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
int  marginCols[][2],
int  x,
int  y)// accumulate sub-col margin widths here
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

  if (placeRotateIcon){
      rotateIcon.sizeit();
    }

  /****************************************************/
  /* figure out who is placed in which row and column */
  /****************************************************/

  csiPlacement.tbl[XX] = TblCsi;
  csiPlacement.tbl[YY] = TblCsi;

  /* Lets start with the absolutes (those relative to the CSI) */

  PlacementData pliPlacement = pli.placement.value();

  // PLI relative to CSI

  if (pliPlacement.relativeTo == CsiType) {
      if (pliPlacement.preposition == Outside) {
          pli.tbl[XX] = pliPlace[pliPlacement.placement][XX];
          pli.tbl[YY] = pliPlace[pliPlacement.placement][YY];
        } else {
          pli.tbl[XX] = TblCsi;
          pli.tbl[YY] = TblCsi;
        }
    }

  // Rotate Icon relative to CSI

  PlacementData rotateIconPlacement = rotateIcon.placement.value();

  if (placeRotateIcon){

      if (rotateIconPlacement.relativeTo == CsiType){
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

  // if Step Number relative to parts list, but no parts list,
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

  /* first the known entities */

  if (pliPlacement.relativeTo == StepNumberType) {
      if (pliPerStep && pli.tsize()) {
          pli.tbl[XX] = stepNumber.tbl[XX]+relativePlace[pliPlacement.placement][XX];
          pli.tbl[YY] = stepNumber.tbl[YY]+relativePlace[pliPlacement.placement][YY];
        } else {
          stepNumber.tbl[XX] = stepNumberPlace[stepNumberPlacement.placement][XX];
          stepNumber.tbl[YY] = stepNumberPlace[stepNumberPlacement.placement][YY];
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

  if (stepNumberPlacement.relativeTo == PartsListType) {
      stepNumber.tbl[XX] = pli.tbl[XX]+relativePlace[stepNumberPlacement.placement][XX];
      stepNumber.tbl[YY] = pli.tbl[YY]+relativePlace[stepNumberPlacement.placement][YY];
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
  maxMargin(stepNumber.margin,stepNumber.tbl,marginRows,marginCols);
  maxMargin(csiPlacement.margin,csiPlacement.tbl,marginRows,marginCols);
  maxMargin(rotateIcon.margin,rotateIcon.tbl,marginRows,marginCols);

  /* now place the callouts relative to the known (CSI, PLI, SN, RI) */

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

      int rp = calloutPlacement.placement;
      switch (calloutPlacement.relativeTo) {
        case CsiType:
          callout->tbl[XX] = coPlace[rp][XX];
          callout->tbl[YY] = coPlace[rp][YY];
          break;
        case PartsListType:
          callout->tbl[XX] = pli.tbl[XX] + relativePlace[rp][XX];
          callout->tbl[YY] = pli.tbl[YY] + relativePlace[rp][YY];
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

      if ( ! pliPerStep) {
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

  /************************************************/
  /*                                              */
  /* Determine the biggest in each column and row */
  /*                                              */
  /************************************************/

  if (pli.pliMeta.constrain.isDefault()) {

      int tsize = 0;

      switch (pliPlacement.placement) {
        case Top:
        case Bottom:
          tsize = csiPlacement.size[XX];
          pli.sizePli(ConstrainData::PliConstrainWidth,tsize);
          if (pli.size[YY] > gui->pageSize(gui->page.meta.LPub.page, YY)/3) {
              pli.sizePli(ConstrainData::PliConstrainArea,tsize);
            }
          break;
        case Left:
        case Right:
          tsize = csiPlacement.size[YY];
          pli.sizePli(ConstrainData::PliConstrainHeight,tsize);
          if (pli.size[XX] > gui->pageSize(gui->page.meta.LPub.page, XX)/3) {
              pli.sizePli(ConstrainData::PliConstrainArea,tsize);
            }
          break;
        default:
          pli.sizePli(ConstrainData::PliConstrainArea,tsize);
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
          break;
        default:
          break;
        }
    }

  return 0;
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

void Step::maxMargin(int &top, int &bot, int y)
{
  top = csiPlacement.margin.valuePixels(y);
  bot = top;

  if (stepNumber.tbl[YY] < TblCsi) {
      top = stepNumber.margin.valuePixels(y);
    } else if (stepNumber.tbl[y] == TblCsi) {
      int margin = stepNumber.margin.valuePixels(y);
      top = qMax(top,margin);
      bot = qMax(bot,margin);
    } else {
      bot = stepNumber.margin.valuePixels(y);
    }

  if (pli.size[y]) {
      if (pli.tbl[y] < TblCsi) {
          top = pli.margin.valuePixels(y);
        } else if (stepNumber.tbl[y] == TblCsi) {
          int margin = pli.margin.valuePixels(y);
          top = qMax(top,margin);
          bot = qMax(bot,margin);
        } else {
          bot = pli.margin.valuePixels(y);
        }
    }

  if (placeRotateIcon){
      if (rotateIcon.tbl[YY] < TblCsi) {
          top = rotateIcon.margin.valuePixels(y);
        } else if (stepNumber.tbl[y] == TblCsi) {
          int margin = rotateIcon.margin.valuePixels(y);
          top = qMax(top,margin);
          bot = qMax(bot,margin);
        } else {
          bot = rotateIcon.margin.valuePixels(y);
        }
    }

  for (int i = 0; i < list.size(); i++) {
      Callout *callout = list[i];
      if (callout->tbl[y] < TblCsi) {
          top = callout->margin.valuePixels(y);
        } else if (stepNumber.tbl[y] == TblCsi) {
          int margin = callout->margin.valuePixels(y);
          top = qMax(top,margin);
          bot = qMax(bot,margin);
        } else {
          bot = callout->margin.valuePixels(y);
        }
    }
}

/***************************************************************************
 * This routine is used for tabular multi-steps.  It is used to determine
 * the location of csi, pli, stepNumber, rotateIcon and step relative callouts.
 ***************************************************************************/

void Step::placeit(
    int rows[],
    int margins[],
    int y,
    bool shared)
{

  /*********************************/
  /* record the origin of each row */
  /*********************************/

  int origin = 0;

  int origins[NumPlaces];

  for (int i = 0; i < NumPlaces; i++) {
      origins[i] = origin;
      if (rows[i]) {
          origin += rows[i] + margins[i];
        }
    }

  size[y] = origin;

  /*******************************************/
  /* Now place the components in pixel units */
  /*******************************************/

  csiPlacement.loc[y] = origins[TblCsi] + (rows[TblCsi] - csiPlacement.size[y])/2;
  pli.loc[y]          = origins[pli.tbl[y]];
  stepNumber.loc[y]   = origins[stepNumber.tbl[y]];
  rotateIcon.loc[y]   = origins[rotateIcon.tbl[y]];

  switch (y) {
    case XX:
      if ( ! shared) {
          pli.justifyX(origins[pli.tbl[y]],rows[pli.tbl[y]]);
        }
      stepNumber.justifyX(origins[stepNumber.tbl[y]],rows[stepNumber.tbl[y]]);
      if(placeRotateIcon){
          rotateIcon.justifyX(origins[rotateIcon.tbl[y]],rows[rotateIcon.tbl[y]]);
        }
      break;
    case YY:
      if ( ! shared) {
          pli.justifyY(origins[pli.tbl[y]],rows[pli.tbl[y]]);
        }
      stepNumber.justifyY(origins[stepNumber.tbl[y]],rows[stepNumber.tbl[y]]);
      if(placeRotateIcon){
          rotateIcon.justifyY(origins[rotateIcon.tbl[y]],rows[rotateIcon.tbl[y]]);
        }
      break;
    default:
      break;
    }

  /* place the callouts that are relative to step components */

  for (int i = 0; i < list.size(); i++) {
      Callout *callout = list[i];
      PlacementData calloutPlacement = callout->placement.value();

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
            case StepNumberType:
            case RotateIconType:
              callout->loc[y] = origins[callout->tbl[y]];
              if (callout->shared) {
                  callout->loc[y] -= callout->margin.value(y) - 500;
                }

              if (y == YY) {
                  callout->justifyY(origins[callout->tbl[y]],
                      rows[callout->tbl[y]]);
                } else {
                  callout->justifyX(origins[callout->tbl[y]],
                      rows[callout->tbl[y]]);
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

  // CSI
  csiItem = new CsiItem(this,
                        meta,
                        csiPixmap,
                        submodelLevel,
                        parent,
                        parentRelativeType);
  csiItem->assign(&csiPlacement);
  csiItem->setPos(offsetX + csiItem->loc[XX],
                  offsetY + csiItem->loc[YY]);
  csiItem->setFlag(QGraphicsItem::ItemIsMovable,movable);

  // PLI
  if (pli.tsize()) {
      pli.addPli(submodelLevel, parent);
      pli.setPos(offsetX + pli.loc[XX],
                 offsetY + pli.loc[YY]);
    }

  // Step Number
  if (stepNumber.number > 0 && ! onlyChild() && showStepNumber) {
      StepNumberItem *sn;
      sn = new StepNumberItem(this,
                              parentRelativeType,
                              numberPlacemetMeta,
                              "%d",
                              stepNumber.number,
                              parent);

      sn->setPos(offsetX + stepNumber.loc[XX],
                 offsetY + stepNumber.loc[YY]);

      sn->setFlag(QGraphicsItem::ItemIsMovable,movable);
    }

  // Rotate Icon
  if (placeRotateIcon){
      RotateIconItem *ri;
      ri = new RotateIconItem(this,
                              parentRelativeType,
                              rotateIconMeta,
                              parent);
      //      logNotice() << "\nROTATE_ICON MULTI-STEP - "
      //                  << "\nBACKGROUND Meta (curMeta) - "
      //                  << "\nColour - " << rotateIconMeta.background.value().string
      //                     ;

      //      if (calledOut){
      //          ri = new CalloutRotateIconItem(this,
      //                                         parentRelativeType,
      //                                         meta->LPub.callout.rotateIcon,
      //                                         parent);
      //          ri = new CalloutRotateIconItem(this,
      //                                         parentRelativeType,
      //                                         meta->LPub.callout.rotateIcon,
      //                                         parent);
      //        } else {

      //          ri = new MultiStepRotateIconItem(this,
      //                                           parentRelativeType,
      //                                           multiStepRotateIconMeta,
      //                                           parent);
      //          ri = new MultiStepRotateIconItem(this,
      //                                           parentRelativeType,
      //                                           meta->LPub.multiStep.rotateIcon,
      //                                           parent);
      //          logNotice() << "\nROTATE_ICON MULTI-STEP - "
      //                      << "\nBACKGROUND Meta (curMeta) - "
      //                      << "\nColour - " << rotateIconMeta.background.value().string
      //                         ;
      //        }
      ri->setPos(offsetX + rotateIcon.loc[XX],
                 offsetY + rotateIcon.loc[YY]);

      ri->setFlag(QGraphicsItem::ItemIsMovable,movable);
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
          bool callout_movable = true /*movable*/;
          if (parentRelativeType == StepGroupType && placementData.relativeTo == StepGroupType) {
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
  if (stepNumber.placement.value().preposition == Inside) {
      switch (pli.placement.value().relativeTo) {
        case CsiType:
          csiPlacement.placeRelative(&stepNumber);
          break;
        case PartsListType:
          pli.placeRelative(&stepNumber);
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
      switch (pli.placement.value().relativeTo) {
        case CsiType:
          csiPlacement.placeRelative(&rotateIcon);
          break;
        case PartsListType:
          pli.placeRelative(&rotateIcon);
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
  relativeJustification = relativeJustification;
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

  // size up the rotateIcon

  if (placeRotateIcon){
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

  // FIXME: when we get here for callouts that are to to the left of the CSI
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
          stepNumber.loc[dim]    -= min;
          rotateIcon.loc[dim] -= min;

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
  stepNumber.loc[xx]   -= offsetX + sizeX;
  rotateIcon.loc[xx]   -= offsetX + sizeX;

  for (int i = 0; i < list.size(); i++) {
      list[i]->loc[xx] -= offsetX + sizeX;
    }
}
