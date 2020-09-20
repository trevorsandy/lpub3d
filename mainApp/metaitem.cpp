
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
 * This file describes a class that is used to implement backannotation
 * of user Gui input into the LDraw file.  Furthermore it implements
 * some functions to provide higher level editing capabilities, such
 * as adding and removing steps from step groups, adding, moving and
 * deleting dividers.
 *
 * Please see lpub.h for an overall description of how the files in LPub
 * make up the LPub program.
 *
 ***************************************************************************/

#include <QFont>
#include <QFontDatabase>
#include <QFontDialog>
#include <QColor>
#include <QColorDialog>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QTextStream>
#include <QFileDialog>
#include <QMessageBox>
#include <QInputDialog>
#include "metaitem.h"
#include "lpub.h"
#include "range.h"
#include "color.h"
#include "placementdialog.h"
#include "pointerplacementdialog.h"
#include "pliconstraindialog.h"
#include "pliannotationdialog.h"
#include "plisortdialog.h"
#include "pairdialog.h"
#include "scaledialog.h"
#include "borderdialog.h"
#include "backgrounddialog.h"
#include "dividerdialog.h"
#include "sizeandorientationdialog.h"
#include "rotateiconsizedialog.h"
#include "submodelcolordialog.h"
#include "cameradistfactordialog.h"
#include "pointerattribdialog.h"
#include "texteditdialog.h"
#include "rotstepdialog.h"
#include "substitutepartdialog.h"
#include "paths.h"
#include "render.h"
#include "messageboxresizable.h"
#include "step.h"

#include "csiitem.h"

enum MonoColors { Blue, TransWhite, NumColors };
const QString monoColor[NumColors]     = { "blue", "transwhite" };
const QString monoColorCode[NumColors] = { "1", "11015"};

void MetaItem::setGlobalMeta(
  QString  &topLevelFile,
  LeafMeta *leaf)
{
  if (leaf == nullptr)
    return;

  QString newMeta = leaf->format(false,true);
  if (leaf->here().modelName != "undefined") {
    replaceMeta(leaf->here(),newMeta);
  } else {
    Where here = sortedGlobalWhere(topLevelFile,newMeta);
    insertMeta(here,newMeta);
  }
}

const QString stepGroupBegin   = "0 !LPUB MULTI_STEP BEGIN";
const QString stepGroupDivider = "0 !LPUB MULTI_STEP DIVIDER";
const QString stepGroupEnd     = "0 !LPUB MULTI_STEP END";
const QString step             = "0 STEP";

Rc MetaItem::scanForwardStepGroup(Where &here, bool &partsAdded)
{
  return scanForward(here,StepMask|StepGroupMask,partsAdded);
}

Rc MetaItem::scanForwardStepGroup(Where &here)
{
  return scanForward(here,StepMask|StepGroupMask);
}

Rc MetaItem::scanBackwardStepGroup(Where &here, bool &partsAdded)
{
  return scanBackward(here,StepMask|StepGroupMask,partsAdded);
}

Rc MetaItem::scanBackwardStepGroup(Where &here)
{
  return scanBackward(here,StepMask|StepGroupMask);
}

/***********************************************************************
 *
 * tools
 *
 **********************************************************************/

float determinant(
  QStringList tokens)
{

  /* a  b  c
     d  e  f
     g  h  i */

  float a = tokens[5].toFloat();
  float b = tokens[6].toFloat();
  float c = tokens[7].toFloat();
  float d = tokens[8].toFloat();
  float e = tokens[9].toFloat();
  float f = tokens[10].toFloat();
  float g = tokens[11].toFloat();
  float h = tokens[12].toFloat();
  float i = tokens[13].toFloat();

  return (a*e*i + b*f*g + c*d*h) - (g*e*c + h*f*a + i*d*b);
}

bool equivalentAdds(
  QString const &first,
  QString const &second)
{
  QStringList firstTokens, secondTokens;
  bool firstMirror, secondMirror;
  float firstDet, secondDet;

  split(first,firstTokens);
  split(second,secondTokens);

  firstDet = determinant(firstTokens);
  secondDet = determinant(secondTokens);
  firstMirror = firstDet < 0;
  secondMirror = secondDet < 0;

  return firstMirror == secondMirror && firstTokens[14] == secondTokens[14];
}

/***********************************************************************
 *
 * tools end
 *
 **********************************************************************/

//   TOS                   EOS
//   STEP (END) BEGIN PART STEP (DIVIDER)         PART STEP END
//   STEP (END) xxxxx PART STEP xxxxxxxxx         PART STEP xxx

//   TOS                   EOS
//   STEP (END) BEGIN PART STEP (DIVIDER)         PART STEP         PART
//   STEP (END) xxxxx PART STEP           (BEGIN) PART STEP         PART

//   TOS                   EOS
//   STEP (END) BEGIN PART STEP (DIVIDER)         PART STEP DIVIDER
//   STEP (END) xxxxx PART STEP           (BEGIN) PART STEP DIVIDER

int MetaItem::removeFirstStep(
  const Where &topOfSteps)
{
  int sum = 0;
  Where secondStep = topOfSteps + 1;         // STEP

  Rc rc = scanForwardStepGroup(secondStep);
  if (rc == StepGroupEndRc) {                 // (END)
    ++secondStep;
    rc = scanForwardStepGroup(secondStep);
  }
  if (rc == StepGroupBeginRc) {               // BEGIN
    Where begin = secondStep++;
    rc = scanForwardStepGroup(secondStep);
    if (rc == StepRc || rc == RotStepRc) {    // STEP
      Where thirdStep = secondStep + 1;
      rc = scanForwardStepGroup(thirdStep);
      Where divider;
      if (rc == StepGroupDividerRc) {         // (DIVIDER)
        divider = thirdStep++;
        rc = scanForwardStepGroup(thirdStep);
      }
      if (rc == StepRc || rc == RotStepRc) {  // STEP
        ++thirdStep;
        rc = scanForwardStepGroup(thirdStep);
        if (rc == StepGroupEndRc) {           // END
          // remove this end and then begin
          deleteMeta(thirdStep);
          if (divider.lineNumber) {
            deleteMeta(divider);
          }
        } else {
          if (divider.lineNumber) {
            replaceMeta(divider,stepGroupBegin);
          } else {
            appendMeta(secondStep,stepGroupBegin);
          }
        }
        deleteMeta(begin); --sum;
      }
    }
  }
  return sum;
}

int MetaItem::countInstancesInStep(Meta *meta, const QString &modelName){

    int   numLines;
    Where walk(modelName,0);

    /* submodelStack tells us where this submodel is referenced in the
     parent file so we use it to target the correct STEP*/

    SubmodelStack tos = meta->submodelStack[meta->submodelStack.size() - 1];
    Where step(tos.modelName,tos.lineNumber);

    /* Now scan the lines following this line, to see if there is another
   * submodel just like this one that needs to be added as multiplier.
   *
   * We also want to scan backward for the same submodel.
   *
   * In either direction, we need to stop on STEP/ROTSTEP.  We also need
   * to stop on other sub-models, or mirror images of the same sub-model.
   * Lastly, models that are callouts are not counted as instances.
   */

  int instanceCount = 0;

  QString firstLine;
  Where lastInstance, firstInstance;

  Rc rc;
  bool ignorePartLine = false;
  Where walkBack = step;
  for (; walkBack.lineNumber >= 0; walkBack--) {
    QString line = gui->readLine(walkBack);

    if (isHeader(line)) {
      break;
    } else {
      QStringList argv;
      split(line,argv);
      if (argv.size() >= 2 && argv[0] == "0") {
        if (argv[1] == "STEP" || argv[1] == "ROTSTEP") {
          break;
        }
        rc = meta->parse(line,walkBack);
        if (rc == PartEndRc || rc == PliEndRc || rc == MLCadSkipEndRc)
            ignorePartLine = true;
        if (rc == PartBeginIgnRc || rc == PliBeginIgnRc || rc == MLCadSkipBeginRc)
            ignorePartLine = false;
        // Sorry, models that are callouts are not counted as instances
        if (argv.size() == 4 && argv[0] == "0" &&
           (argv[1] == "LPUB" || argv[1] == "!LPUB") &&
            argv[2] == "CALLOUT" && argv[3] == "END") {
          //process callout content
          for (--walkBack; walkBack.lineNumber >= 0; walkBack--) {
            QString calloutLine = gui->readLine(walkBack);
            QStringList tokens;
            split(calloutLine,tokens);
            if (tokens.size() == 15 && tokens[0] == "1") {
              ignorePartLine = true;
            } else
            if (tokens.size() == 4 && tokens[0] == "0" &&
               (tokens[1] == "LPUB" || tokens[1] == "!LPUB") &&
                tokens[2] == "CALLOUT" && tokens[3] == "BEGIN") {
              ignorePartLine = false;
              break;
            }
          }
        }
      } else if (argv.size() == 15 && argv[0] == "1") {
        if (ignorePartLine)
          continue;
        if (gui->isSubmodel(argv[14])) {
          if (argv[14] == modelName) {
            if (firstLine == "") {
              firstLine = line;
              firstInstance = walkBack;
              lastInstance = walkBack;
              ++instanceCount;
            } else {
              if (equivalentAdds(firstLine,line)) {
                firstInstance = walkBack;
                ++instanceCount;
              } else {
                continue;
              }
            }
          } else {
            continue;
          }
        }
      }
    }
  }

  walk = step + 1;
  numLines = gui->subFileSize(walk.modelName);
  for ( ; walk.lineNumber < numLines; walk++) {
    QString line = gui->readLine(walk);
    QStringList argv;
    split(line,argv);
    if (argv.size() >= 2 && argv[0] == "0") {
      if (argv[1] == "STEP" || argv[1] == "ROTSTEP") {
        break;
      }
      rc = meta->parse(line,walk);
      if (rc == PartBeginIgnRc || rc == PliBeginIgnRc || rc == MLCadSkipBeginRc)
          ignorePartLine = true;
      if (rc == PartEndRc || rc == PliEndRc || rc == MLCadSkipEndRc)
          ignorePartLine = false;
      // models that are callouts are not counted as instances
      if (argv.size() == 4 && argv[0] == "0" &&
         (argv[1] == "LPUB" || argv[1] == "!LPUB") &&
          argv[2] == "CALLOUT" && argv[3] == "BEGIN") {
        //process callout content
        for (++walk; walk.lineNumber < numLines; walk++) {
          QString calloutLine = gui->readLine(walk);
          QStringList tokens;
          split(calloutLine,tokens);
          if (tokens.size() == 15 && tokens[0] == "1") {
            ignorePartLine = true;
          } else
          if (tokens.size() == 4 && tokens[0] == "0" &&
             (tokens[1] == "LPUB" || tokens[1] == "!LPUB") &&
              tokens[2] == "CALLOUT" && tokens[3] == "END") {
            ignorePartLine = false;
            break;
          }
        }
      }
    } else if (argv.size() == 15 && argv[0] == "1") {
      if (gui->isSubmodel(argv[14])) {
        if (ignorePartLine)
          continue;
        if (argv[14] == modelName) {
          if (firstLine == "") {
            firstLine = line;
            firstInstance = walk;
            ++instanceCount;
          } else {
            if (equivalentAdds(firstLine,line)) {
              lastInstance = walk;
              ++instanceCount;
            } else {
              continue;
            }
          }
        } else {
          continue;
        }
      }
    }
  }
  return instanceCount;
}

int MetaItem::countInstancesInModel(Meta *meta, const QString &modelName){

    int   numLines;
    Where walk(modelName,1);

    /* submodelStack tells us where this submodel is referenced in the
     parent file so we use it to target the correct SUBMODEL*/

    SubmodelStack tos = meta->submodelStack[meta->submodelStack.size() - 1];
    Where step(tos.modelName,tos.lineNumber);

    /* Scan the lines following this line, to see if there is another
   * submodel just like this one that needs to be added as multiplier.
   *
   * We need to stop on STEP/ROTSTEP.  We also need to stop
   * on other sub-models, or mirror images of the same sub-model.
   * Lastly, models that are callouts are not counted as instances.
   */

  int instanceCount = 0;

  QString firstLine;
  Where lastInstance, firstInstance;

  Rc rc;
  bool ignorePartLine = false;
  walk = step;

  numLines = gui->subFileSize(walk.modelName);
  for ( ; walk.lineNumber < numLines; walk++) {
    QString line = gui->readLine(walk);
    QStringList argv;
    split(line,argv);
    if (argv.size() >= 2 && argv[0] == "0") {
      rc = meta->parse(line,walk);
      if (rc == PartBeginIgnRc || rc == PliBeginIgnRc || rc == MLCadSkipBeginRc)
          ignorePartLine = true;
      if (rc == PartEndRc || rc == PliEndRc || rc == MLCadSkipEndRc)
          ignorePartLine = false;
      // models that are callouts are not counted as instances
      if (argv.size() == 4 && argv[0] == "0" &&
         (argv[1] == "LPUB" || argv[1] == "!LPUB") &&
          argv[2] == "CALLOUT" && argv[3] == "BEGIN") {
        //process callout content
        for (++walk; walk.lineNumber < numLines; walk++) {
          QString calloutLine = gui->readLine(walk);
          QStringList tokens;
          split(calloutLine,tokens);
          if (tokens.size() == 15 && tokens[0] == "1") {
            ignorePartLine = true;
          } else
          if (tokens.size() == 4 && tokens[0] == "0" &&
             (tokens[1] == "LPUB" || tokens[1] == "!LPUB") &&
              tokens[2] == "CALLOUT" && tokens[3] == "END") {
            ignorePartLine = false;
            break;
          }
        }
      }
    } else if (argv.size() == 15 && argv[0] == "1") {
      if (gui->isSubmodel(argv[14])) {
        if (ignorePartLine)
          continue;
        if (argv[14] == modelName) {
          if (firstLine == "") {
            firstLine = line;
            firstInstance = walk;
            ++instanceCount;
          } else {
            if (equivalentAdds(firstLine,line)) {
              lastInstance = walk;
              ++instanceCount;
            } else {
              continue;
            }
          }
        } else {
          continue;
        }
      }
    }
  }
  return instanceCount;
}

void MetaItem::addNextStepsMultiStep(
        const Where &topOfSteps,
        const Where &bottomOfSteps,
        const int   &numOfSteps)
{
    Rc rc1;
    Where startTopOfSteps = topOfSteps;
    Where nextStep = bottomOfSteps;
    bool firstChange = true;
    bool amendGroup = false;

    for (int stepNum = 1; (stepNum <= numOfSteps) && (numOfSteps > 0); stepNum++) {
        bool lastStep = stepNum == numOfSteps;
        bool partsAdded = false;

        Where walk = nextStep+1;
        Where finalTopOfSteps = startTopOfSteps;

        rc1 = scanForward(walk,StepMask|StepGroupMask,partsAdded);
        Where end;

        if (rc1 == StepGroupEndRc) {                            // END
            amendGroup = true;
            end = walk++;
            rc1 = scanForward(walk,StepMask|StepGroupMask,partsAdded);
        }

        if (rc1 == StepGroupBeginRc) {                          // BEGIN
            firstChange = false;
            beginMacro("addNextStep1");
            removeFirstStep(bottomOfSteps);                     // remove BEGIN
            partsAdded = false;
            rc1 = scanForward(walk,StepMask|StepGroupMask,partsAdded);
        }

        if (firstChange) {
            beginMacro("addNextStep2");
            firstChange = false;
        }

        if (rc1 == EndOfFileRc && partsAdded) {
            lastStep = true;
            insertMeta(walk,step);
        }

        nextStep =  walk;

        if (end.lineNumber) {
            deleteMeta(end);
        } else {
            walk = topOfSteps + 1;
            rc1 = scanForward(walk,StepMask|StepGroupMask);
            if (rc1 == StepGroupEndRc) {
                finalTopOfSteps = walk+1;
                if (lastStep)
                    appendMeta(walk,stepGroupBegin);
            }
            else
            if (rc1 == StepGroupBeginRc) {
                finalTopOfSteps = walk+1;
            } else {
                walk = topOfSteps;
                scanPastGlobal(walk);
                finalTopOfSteps = walk+1;
                if (lastStep)
                    appendMeta(walk,stepGroupBegin);
            }
        }

        startTopOfSteps = finalTopOfSteps;

        if (lastStep) {
            Where endStep = nextStep;
            if (!amendGroup) {
                scanForward(endStep,StepMask);
            }
            appendMeta(endStep,stepGroupEnd);
        }
    }
    endMacro();
}

void MetaItem::addNextMultiStep(
  const Where &topOfSteps,
  const Where &bottomOfSteps)  // always add at end
{
  Rc rc1;

  bool firstChange = true;
  bool partsAdded;
  // TOR            BOR
  // EOF       PART STEP PART EOF
  // EOF BEGIN PART STEP PART     STEP END

  // EOF       PART STEP PART     STEP
  // EOF BEGIN PART STEP PART     STEP END

  // STEP END PART EOF
  // STEP     PART STEP END

  // STEP END PART STEP
  // STEP     PART STEP END

  Where walk = bottomOfSteps + 1;
  Where finalTopOfSteps = topOfSteps;
  rc1 = scanForward(walk,StepMask|StepGroupMask,partsAdded);
  Where end;
  if (rc1 == StepGroupEndRc) {                            // END
    end = walk++;
    rc1 = scanForward(walk,StepMask|StepGroupMask,partsAdded);
  }
  if (rc1 == StepGroupBeginRc) {                          // BEGIN
    firstChange = false;
    beginMacro("addNextStep1");
    removeFirstStep(bottomOfSteps);                      // remove BEGIN
    partsAdded = false;
    rc1 = scanForwardStepGroup(walk,partsAdded);
  }

  // bottomOfSteps - STEP
  // end            - StepGroupEnd
  // walk           - (STEP || EOF)

  if (firstChange) {
    beginMacro("addNextStep2");
    firstChange = false;
  }
  if (rc1 == EndOfFileRc && partsAdded) {
    insertMeta(walk,step);
  }
  appendMeta(walk,stepGroupEnd);
  if (end.lineNumber) {
    deleteMeta(end);
  } else {
    walk = topOfSteps + 1;
    rc1 = scanForward(walk,StepMask|StepGroupMask);
    if (rc1 == StepGroupEndRc) {
      finalTopOfSteps = walk+1;
      appendMeta(walk,stepGroupBegin);
    } else {
      walk = topOfSteps;
      scanPastGlobal(walk);
      finalTopOfSteps = walk+1;
      appendMeta(walk,stepGroupBegin);
    }
  }
  //movePageToEndOfStepGroup(finalTopOfSteps);
  endMacro();
}

void MetaItem::movePageToEndOfStepGroup(
  const Where &topOfSteps)
{
  Where bottomOfSteps, topOfPage, bottomOfPage;
  while (1) {
    bottomOfSteps = topOfSteps;

    // Find bottom of step group

    scanForward(bottomOfSteps,StepGroupEndMask);

    Meta meta;

    // find top of page

    topOfPage = bottomOfSteps;
    for (topOfPage = bottomOfSteps - 1; topOfPage >= topOfSteps.lineNumber; topOfPage--) {
      QString line = gui->readLine(topOfPage);
      Rc rc = meta.parse(line,topOfPage);
      if (rc == StepGroupBeginRc) {
        return;
      }
      if (rc == InsertPageRc || rc == InsertCoverPageRc) {
        break;
      }
    }

    // find bottom of page

    for (bottomOfPage = topOfPage + 1; bottomOfPage < bottomOfSteps.lineNumber; bottomOfPage++) {
      QString line = gui->readLine(bottomOfPage);
      Rc rc = meta.parse(line,bottomOfPage);
      if (rc == StepRc || rc == RotStepRc) {
        break;
      }
    }

    Where walk = bottomOfSteps;

    // copy page to after step group

    for (Where walk2 = topOfPage; walk2 <= bottomOfPage.lineNumber; walk2++) {
      QString line = gui->readLine(walk2);
      appendMeta(walk++,line);
    }

    for (Where walk2 = topOfPage; walk2 <= bottomOfPage.lineNumber; walk2++) {
      deleteMeta(topOfPage);
    }
  }
}

//                      TOS
// BEGIN           PART STEP DIVIDER       PART STEP END
// xxxxx           PART STEP xxxxxxx       PART STEP xxx

//                      TOS
// BEGIN PART STEP PART STEP DIVIDER       PART STEP END
// BEGIN PART STEP PART STEP xxxxxxx (END) PART STEP xxx

//                      TOS
// BEGIN PART STEP PART STEP               PART STEP END
// BEGIN PART STEP PART STEP         (END) PART STEP xxx
// $$$$$                $$$  $$$$$$$                 $$$

int MetaItem::removeLastStep(
  const Where &topOfSteps,                       // STEP
  const Where &bottomOfSteps)
{
  int sum = 0;

  // Find the top of us
  Where walk = topOfSteps + 1;
  Rc rc = scanForwardStepGroup(walk);

  if (rc == StepGroupEndRc) {
    ++walk;
    rc = scanForwardStepGroup(walk);
  }

  Where begin = walk;
  if (rc == StepGroupBeginRc) {                   // BEGIN

    // Find the next step
    Where secondStep = walk + 1;
    Rc rc = scanForward(secondStep,StepMask);

    // find the next next step
    if (rc == StepRc || rc == RotStepRc) {        // STEP
      Where thirdStep = secondStep + 1;
      rc = scanForwardStepGroup(thirdStep);

      Where divider;
      if (rc == StepGroupDividerRc) {             // (DIVIDER)
        divider = thirdStep;
        ++thirdStep;
        rc = scanForwardStepGroup(thirdStep);
      }

  //-----------------------------------------------------

      Where lastStep = bottomOfSteps - 1;
      rc = scanBackward(lastStep,StepMask);

      if (rc == StepRc || rc == RotStepRc) {        // STEP
        walk = lastStep + 1;
        rc = scanForward(walk,StepMask|StepGroupMask);
        Where divider2;
        if (rc == StepGroupDividerRc) {             // (DIVIDER)
          divider2 = walk++;
          rc = scanForwardStepGroup(walk);
        }

        if (rc == StepRc || rc == RotStepRc) {      // STEP
          ++walk;
          rc = scanForwardStepGroup(walk);

          if (rc == StepGroupEndRc) {               // END
            deleteMeta(walk);
            --sum;
            if (secondStep.lineNumber == lastStep.lineNumber) {
              if (divider.lineNumber) {
                --sum;
                deleteMeta(divider);
              }
              --sum;
              deleteMeta(begin);
            } else if (divider2.lineNumber) {
              replaceMeta(divider2,stepGroupEnd);
            } else {
              ++sum;
              appendMeta(lastStep,stepGroupEnd);
            }
          }
        }
      }
    }
  }
  return sum;
}

//                                                TOS
// SOF                                                 PART STEP     PART (EOF|STEP)
// SOF                                         (BEGIN) PART STEP     PART     (STEP) (END)

//                      TOS
//           SOF   PART STEP                           PART STEP     PART (EOF|STEP)
//           SOF   PART STEP                   (BEGIN) PART STEP     PART     (STEP) (END)

//                      TOS
//           BEGIN PART STEP                           PART STEP END PART (EOF|STEP)
//           xxxxx PART STEP                   (BEGIN) PART STEP xxx PART     (STEP) (END)

//                      TOS
//           BEGIN PART STEP         DIVIDER PART STEP END PART (EOF|STEP)
//           xxxxx PART STEP         (BEGIN) PART STEP xxx PART     (STEP) (END)

//                      TOS
// BEGIN PART STEP PART STEP DIVIDER         PART STEP END PART (EOF|STEP)
// BEGIN PART STEP PART STEP (END)   (BEGIN) PART STEP xxx PART     (STEP) (END)

// BEGIN PART STEP PART STEP                 PART STEP END PART (EOF|STEP)
// BEGIN PART STEP PART STEP (END)   (BEGIN) PART STEP xxx PART     (STEP) (END)

void MetaItem::addPrevMultiStep(
  const Where &topOfStepsIn,
  const Where &bottomOfStepsIn)
{
  Rc rc1;

  bool  firstChange = true;
  bool  partsAdded;
  Where topOfSteps = topOfStepsIn;
  Where bottomOfSteps = bottomOfStepsIn;

  Where walk = topOfSteps + 1;
  rc1 = scanForward(walk,StepMask|StepGroupMask,partsAdded);

    //                           TOS  end begin                BOR
    // STEP BEGIN PART STEP PART STEP END BEGIN PART STEP PART STEP END

  Where begin;

  if (rc1 == StepGroupEndRc) {                        // END
    beginMacro("AddPreviousStep");
    firstChange = false;

    Where prevTopOfSteps = topOfSteps-1;
    scanBackward(prevTopOfSteps,StepGroupBeginMask);
    --prevTopOfSteps;
    scanBackward(prevTopOfSteps,StepMask);
    int removed = removeLastStep(prevTopOfSteps,topOfSteps); // shift by removal count
    topOfSteps.lineNumber += removed + 1;
    bottomOfSteps.lineNumber += removed;
    walk = topOfSteps + 1;                            // skip past
    rc1 = scanForwardStepGroup(walk, partsAdded);
  }

  // if I'm the first step of an existing step group

  if (rc1 == StepGroupBeginRc) {                       // BEGIN
    begin = walk++;                                    // remember
    rc1 = scanForward(walk,StepMask,partsAdded);       // next?
  }

  //           PREV           TOS  begin                BOR
  // STEP PART STEP END       PART STEP BEGIN PART STEP PART STEP END
  // STEP PART STEP END BEGIN PART STEP       PART STEP PART STEP END

  if (firstChange) {
    beginMacro("AddPreviousStep2");
    firstChange = false;
  }

  // Handle end of step/group
  if (rc1 == EndOfFileRc && partsAdded) {
    insertMeta(walk,step);
  }

  if (begin.lineNumber == 0) {
    appendMeta(walk,stepGroupEnd);
  }

  Where prevStep = topOfSteps - 1;
  scanBackward(prevStep,StepMask);

  if (begin.lineNumber) {
    deleteMeta(begin);

    Where end = prevStep+1;
    rc1 = scanForwardStepGroup(end);
    prevStep.lineNumber += rc1 == StepGroupEndRc;
  }

  appendMeta(prevStep,stepGroupBegin);
  ++prevStep;
  movePageToBeginOfStepGroup(prevStep);

//  gui->displayPageNum--;
  endMacro();
}

void MetaItem::movePageToBeginOfStepGroup(
  const Where &topOfStepsIn)
{
  Where topOfSteps, bottomOfSteps, topOfPage, bottomOfPage;
  topOfSteps = topOfStepsIn;
  while (1) {
    bottomOfSteps = topOfSteps;

    // Find bottom of step group

    scanForward(bottomOfSteps,StepGroupEndMask);

    Meta meta;

    // find top of page

    for (topOfPage = topOfSteps + 1; topOfPage <= bottomOfSteps.lineNumber; topOfPage++) {
      QString line = gui->readLine(topOfPage);
      Rc rc = meta.parse(line,topOfPage);
      if (rc == StepGroupEndRc) {
        return;
      }
      if (rc == InsertPageRc || rc == InsertCoverPageRc) {
        break;
      }
    }

    // find bottom of page

    for (bottomOfPage = topOfPage + 1; bottomOfPage < bottomOfSteps.lineNumber; bottomOfPage++) {
      QString line = gui->readLine(bottomOfPage);
      Rc rc = meta.parse(line,bottomOfPage);
      if (rc == StepRc || rc == RotStepRc) {
        break;
      }
    }

    Where walk = bottomOfSteps;

    // copy page to before step group

    for (Where walk2 = topOfPage; walk2 <= bottomOfPage.lineNumber; walk2.lineNumber += 2) {
      QString line = gui->readLine(walk2);
      insertMeta(topOfSteps,line);
      ++topOfSteps;
      ++bottomOfSteps;
      ++topOfPage;
      ++bottomOfPage;
    }

    for (Where walk2 = topOfPage; walk2 <= bottomOfPage.lineNumber; walk2++) {
      deleteMeta(topOfPage);
    }
  }
}

void MetaItem::deleteFirstMultiStep(
  const Where &topOfSteps)
{
  beginMacro("removeFirstStep");
  removeFirstStep(topOfSteps);
  endMacro();
}

void MetaItem::deleteLastMultiStep(
  const Where &topOfSteps,
  const Where &bottomOfSteps)
{
  beginMacro("deleteLastMultiStep");
  removeLastStep(topOfSteps,bottomOfSteps);
  endMacro();
}

/***********************************************************************
 *
 * These "short-cuts" move steps to the other side of a divider
 *
 **********************************************************************/

const QString calloutDivider = "0 !LPUB CALLOUT DIVIDER";

void MetaItem::addToNext(
  PlacementType parentRelativeType,
  const Where &topOfStep)
{
  if (parentRelativeType == CalloutType) {
    calloutAddToNext(topOfStep);
  } else {
    stepGroupAddToNext(topOfStep);
  }
}

void MetaItem::calloutAddToNext(
  const Where &topOfStep)
{
  Where walk = topOfStep + 1;
  Rc rc = scanForward(walk,StepMask);
  if (rc == StepRc || rc == RotStepRc) {
    ++walk;
    rc = scanForward(walk,StepMask|CalloutMask);
    if (rc == CalloutDividerRc) {
      Where test = topOfStep - 1;
      Rc rc2 = scanBackward(test,StepMask|CalloutMask);
      beginMacro("moveStepPrev");
      deleteMeta(walk);
      if (rc != EndOfFileRc && rc2 != CalloutDividerRc && rc2 != EndOfFileRc) {
        appendMeta(topOfStep,calloutDivider);
      }
      endMacro();
    }
  }
}

void MetaItem::stepGroupAddToNext(
  const Where &topOfStep)
{
  Where walk = topOfStep + 1;
  Rc rc = scanForward(walk,StepMask|StepGroupMask);
  Where divider;
  if (rc == StepGroupBeginRc || rc == StepGroupDividerRc) {
    divider = walk++;
    rc = scanForward(walk,StepMask|StepGroupMask);
  }
  if (rc == StepRc || rc == RotStepRc) {
    ++walk;
    rc = scanForward(walk,StepMask|StepGroupMask);
    if (rc == StepGroupDividerRc) {
      beginMacro("moveStepPrev");
      deleteMeta(walk);
      if (divider.lineNumber == 0) {
        appendMeta(topOfStep,stepGroupDivider);
      }
      endMacro();
    }
  }
}

void MetaItem::addToPrev(
  PlacementType parentRelativeType,
  const Where &topOfStep)
{
  if (parentRelativeType == CalloutType) {
    calloutAddToPrev(topOfStep);
  } else {
    stepGroupAddToPrev(topOfStep);
  }
}

void MetaItem::calloutAddToPrev(
  const Where &topOfStep)
{
  Where walk = topOfStep + 1;
  Rc rc;

  //   1      2         3*
  // TOS PART STEP PART STEP DIVIDER PART EOF
  // TOS PART STEP PART STEP         PART EOF

  //     1    2                 3
  // SOF PART STEP DIVIDER PART STEP         PART EOF
  // SOF PART STEP         PART STEP DIVIDER PART EOF

  rc = scanForward(walk,StepMask|CalloutMask);
  if (rc == CalloutDividerRc) {
    Where divider = walk++;
    rc = scanForward(walk,StepMask|CalloutMask);
    if (rc == StepRc || rc == RotStepRc) {
      Where lastStep = walk;
      ++walk;
      rc = scanForward(walk,StepMask|CalloutMask);
      beginMacro("moveStepNext");
      if (rc != EndOfFileRc && rc != CalloutDividerRc) {
        appendMeta(lastStep,calloutDivider);
      }
      deleteMeta(divider);
      endMacro();
    }
  }
}

void MetaItem::stepGroupAddToPrev(
  const Where &topOfStep)
{
  //   1      2         3*
  // TOS PART STEP PART STEP DIVIDER PART EOF
  // TOS PART STEP PART STEP         PART EOF

  //     1    2                 3
  // SOF PART STEP DIVIDER PART STEP         PART EOF
  // SOF PART STEP         PART STEP DIVIDER PART EOF

  Where walk = topOfStep + 1;                   // STEP
  Rc rc = scanForward(walk,StepMask|StepGroupMask);
  if (rc == StepGroupDividerRc) {               // DIVIDER
    Where divider = walk++;
    rc = scanForward(walk,StepMask|StepGroupMask);
    if (rc == StepRc || rc == RotStepRc) {      // STEP
      Where nextStep = walk++;
      rc = scanForward(walk,StepMask|StepGroupMask);
      beginMacro("moveStepNext");
      if (rc != StepGroupEndRc && rc != StepGroupDividerRc) {
        appendMeta(nextStep,stepGroupDivider);
      }
      deleteMeta(divider);
      endMacro();
    }
  }
}

void MetaItem::convertToIgnore(Meta *meta)
{
  gui->maxPages = -1;

  SubmodelStack tos = meta->submodelStack[meta->submodelStack.size() - 1];
  Where calledOut(tos.modelName,tos.lineNumber);
  Where here = calledOut+1;
  beginMacro("ignoreSubmodel");
  insertMeta(here,      "0 !LPUB PART END");
  insertMeta(calledOut, "0 !LPUB PART BEGIN IGN");
  endMacro();
}

void MetaItem::convertToPart(Meta *meta)
{
  gui->maxPages = -1;

  SubmodelStack tos = meta->submodelStack[meta->submodelStack.size() - 1];
  Where calledOut(tos.modelName,tos.lineNumber);
  Where here = calledOut+1;
  QString line = gui->readLine(calledOut);
  QStringList tokens;
  split(line,tokens);
  if (tokens.size() == 15) {
    //check for spaces in model name and wrap in quotes if yes
    QString modelName = tokens[14];
    if (modelName.contains(QRegExp("\\s+"))){
        modelName = "\"" + modelName + "\"";
      }
    beginMacro("submodelIsPart");
    insertMeta(here,      "0 !LPUB PART END");
    insertMeta(here,      "0 !LPUB PLI END");
    insertMeta(calledOut, "0 !LPUB PART BEGIN IGN");
    insertMeta(calledOut, "0 !LPUB PLI BEGIN SUB " + modelName + " " + tokens[1]);
    endMacro();
  }
}

/*******************************************************************************
 *
 *
 ******************************************************************************/

void MetaItem::setMeta(
  const Where &topOf,
  const Where &bottomOf,
  LeafMeta    *meta,
  bool         useTop,
  int          append,
  bool         local,
  bool         askLocal,
  bool         global)
{
#ifdef QT_DEBUG_MODE
//    logNotice() << "\n MOVE (CHANGE PLACEMENT) SET META VALUES - "
//                << "\n PAGE WHERE - "
//                << "\nPAGE- "
//                << (useTop ? " \nUseTop: True=Single-Step Page" : " \nUseTop: False=Multi-Step Page")
//                << "\n1. Page TopOf (Model Name):     "  << topOf.modelName
//                << "\n1. Page TopOf (Line Number):    "  << topOf.lineNumber
//                << "\n2. Page BottomOf (Model Name):  "  << bottomOf.modelName
//                << "\n2. Page BottomOf (Line Number): "  << bottomOf.lineNumber
//                << "\n FORMATTED META -               "
//                << "\n3. Meta:                        "  << meta->format(local,global)
//                << "\n META WHERE -                   "
//                << "\n3. Meta Here (Model Name):      "  << meta->here().modelName
//                << "\n3. Meta Here (Line Number):     "  << meta->here().lineNumber
//                << "\n3. Meta String:                 "  << meta->format(local,global)
//                << "\n PARAMETERS -                   "
//                << "\n4. ~(UseTop)~:                  "  << useTop
//                << "\n5. Append:                      "  << append
//                << "\n6. Local:                       "  << local
//                << "\n7. AskLocal:                    "  << askLocal
//                << "\n8. Global:                      "  << global
//              ;
#endif
  if (useTop) {
    setMetaTopOf(topOf,bottomOf,meta,append,local,askLocal,global);
//    logNotice() << "\n SET META - USE TOP OF - ";
  } else {
    setMetaBottomOf(topOf,bottomOf,meta,append,local,askLocal,global);
//    logNotice() << "\n SET META - USE BOTTOM OF - ";
  }
}

void MetaItem::setMetaTopOf(
  const Where &topOf,
  const Where &bottomOf,
  LeafMeta    *meta,
  int          append,
  bool         local,
  bool         askLocal,
  bool         global)
{
    int  lineNumber = meta->here().lineNumber;

    bool metaInRange;

    metaInRange = meta->here().modelName == topOf.modelName
                  && lineNumber >= topOf.lineNumber
                  && lineNumber <= bottomOf.lineNumber;

//#ifdef QT_DEBUG_MODE
//  logTrace() << "\nSET META TOP OF PAGE WHERE:"
//             << "\nPage TopOf Model Name:    " << topOf.modelName
//             << "\nPage TopOf Line Number:   " << topOf.lineNumber
//             << "\nPage BottomOf Model Name: " << bottomOf.modelName
//             << "\nPage BottomOf Line Number:" << bottomOf.lineNumber
//             << "\nHere Model Name:          " << meta->here().modelName
//             << "\nHere Line Number:         " << meta->here().lineNumber
//             << "\n --- "
//             << "\nAppend:                   " << (append == 0 ? "NO" : "YES")
//             << "\nMeta In Range:            " << (metaInRange ? "YES - Can replace Meta" : "NO")
//             << "\nLine (Meta in Range):     " <<  meta->format(meta->pushed,meta->global)
//             << "\nLine:                     " <<  meta->format(local, global)
//                ;
//#endif

    if (metaInRange) {
        QString line = meta->format(meta->pushed,meta->global);
        replaceMeta(meta->here(),line);
    } else {

        if (askLocal) {
            local = LocalDialog::getLocal(VER_PRODUCTNAME_STR, "Change only this step?",nullptr);
        }

        QString newMetaString = meta->format(local,global);

        Where topOfFile = topOf;

        if (topOf.lineNumber == 0) {
            QString line = gui->readLine(topOf);
            QStringList argv;
            split(line,argv);
            if (argv.size() >= 1 && argv[0] != "0") {
                insertMeta(topOf,"0");
            }
        } else {
            Where walk = topOf+1;
            Rc rc = scanForward(walk,StepMask|StepGroupMask);
            if (rc == StepGroupEndRc) {
                topOfFile = walk++;
                rc = scanForward(walk,StepMask|StepGroupMask);
            }
            if (rc == StepGroupBeginRc) {
                topOfFile = walk;
            }
        }
        topOfFile.lineNumber += append;
        insertMeta(topOfFile, newMetaString);
    }

}

void MetaItem::setMetaBottomOf(
  const Where &topOf,
  const Where &bottomOf,
  LeafMeta    *meta,
  int          append,
  bool         local,
  bool         askLocal,
  bool         global)
{
  int  lineNumber = meta->here().lineNumber;

  bool metaInRange;

  metaInRange = meta->here().modelName == topOf.modelName
                && lineNumber >= topOf.lineNumber
                && lineNumber <= bottomOf.lineNumber;

#ifdef QT_DEBUG_MODE
//  logTrace() << "\nSET META BOTTOM OF PAGE WHERE:"
//             << "\nPage TopOf Model Name:    " << topOf.modelName
//             << "\nPage TopOf Line Number:   " << topOf.lineNumber
//             << "\nPage BottomOf Model Name: " << bottomOf.modelName
//             << "\nPage BottomOf Line Number:" << bottomOf.lineNumber
//             << "\nHere Model Name:          " << meta->here().modelName
//             << "\nHere Line Number:         " << meta->here().lineNumber
//             << "\n --- "
//             << "\nAppend:                   " << (append == 0 ? "NO" : "YES")
//             << "\nMeta In Range:            " << (metaInRange ? "YES - Can replace Meta" : "NO")
//             << "\nLine (Meta in Range):     " <<  meta->format(meta->pushed,meta->global)
//             << "\nLine:                     " <<  meta->format(local, global)
//                ;
#endif

  if (metaInRange) {
    QString line = meta->format(meta->pushed,meta->global);
    replaceMeta(meta->here(),line);
  } else {
    if (askLocal) {
      local = LocalDialog::getLocal(VER_PRODUCTNAME_STR, "Change only this step?",nullptr); // changed from gui error(QLayout: Attempting to add QLayout "" to Gui "", which already has a layout)
    }
    QString line = meta->format(local, global);

    int  numLines = gui->subFileSize(topOf.modelName);
    bool eof = bottomOf.lineNumber == numLines;

    if (eof) {
      bottomOf -1;       //fix: numLines is inclusive (starts from 1) while readline index is exclusive (i.e. starts from 0)
      QString tline = gui->readLine(bottomOf);
      QStringList argv;
      split(tline,argv);
      if (argv.size() >= 2 && argv[0] == "0" && (argv[1] == "STEP" || argv[1] == "ROTSTEP")) {
        insertMeta(bottomOf,line);
      } else {
        if (append) {
          appendMeta(bottomOf,line);
        } else {
          insertMeta(bottomOf,line);
        }
      }
    } else if (append) {
      appendMeta(bottomOf,line);
    } else {
      insertMeta(bottomOf,line);
    }
  }
}

bool MetaItem::setPointerPlacement(
    PlacementMeta      *placementMeta,
    const PlacementType parentType,
    const PlacementType relativeType,
    QString             title)
{
  PlacementData placementData = placementMeta->value();
  bool ok;
  ok = PointerPlacementDialog
      ::getPointerPlacement(parentType,relativeType,placementData,title);
  if (ok) {
      placementMeta->setValue(placementData);
    }
  return ok;
}

void MetaItem::changeCsiAnnotationPlacement(
  PlacementType          parentType,
  PlacementType          relativeType,
  QString                title,
  const Where           &metaLine,
  const Where           &bottomOf,
  PlacementMeta         *placement,
  CsiAnnotationIconMeta *icon,
  bool                   useTop,
  int                    append,
  bool                   local,
  bool                   useLocal,
  int                    onPageType)
{
  Q_UNUSED(bottomOf)
  Q_UNUSED(useTop)
  Q_UNUSED(append)
  Q_UNUSED(local)
  Q_UNUSED(useLocal)

  PlacementData placementData = placement->value();
  bool ok;
  ok = PlacementDialog
       ::getPlacement(parentType,relativeType,placementData,title,onPageType);

  if (ok) {
    CsiAnnotationIconData caiData = icon->value();
    bool hasJustification = (placementData.justification != Center &&
                             placementData.preposition != Inside);
    QStringList replacements;
    replacements << QString::number(placementData.placement);
    if (hasJustification)
        replacements << QString::number(placementData.justification);
    replacements << QString::number(placementData.preposition);
    caiData.placements = replacements;

    icon->setValue(caiData);

    updateCsiAnnotationIconMeta(metaLine,icon);
  }
}

void MetaItem::changePlacement(
  PlacementType  parentType,
  PlacementType  relativeType,
  QString        title,
  const Where   &topOfSteps,
  const Where   &bottomOfSteps,
  PlacementMeta *placement,
  bool           useTop,
  int            append,
  bool           local,
  bool           useLocal,
  int            onPageType)
{
  PlacementData placementData = placement->value();
  bool ok;
  ok = PlacementDialog
       ::getPlacement(parentType,relativeType,placementData,title,onPageType);

  if (ok) {
    placement->setValue(placementData);
    setMeta(topOfSteps,bottomOfSteps,placement,useTop,append,local,useLocal);
  }
}

void MetaItem::changePlacement(
  PlacementType  parentType,
  bool           pliPerStep,
  PlacementType  relativeType,
  QString        title,
  const Where   &topOfSteps,
  const Where   &bottomOfSteps,
  PlacementMeta *placement,
  bool           useTop,
  int            append,
  bool           local,
  bool           useLocal,
  int            onPageType)
{
  PlacementData placementData = placement->value();
  bool ok;
  ok = PlacementDialog
       ::getPlacement(parentType,relativeType,placementData,title,onPageType,pliPerStep);

  if (ok) {
    placement->setValue(placementData);

    setMeta(topOfSteps,bottomOfSteps,placement,useTop,append,local,useLocal);
  }
}

void MetaItem::changePlacementOffset(
  Where          defaultWhere,
  PlacementMeta *placement,
  PlacementType  type,
  bool           /* unused */,
  bool           local,
  bool           global)
{

#ifdef QT_DEBUG_MODE
// qDebug() << "\nCHANGE PLACEMENT OFFSET -    "
//          << "\nPAGE WHERE -                 "
//          << " \nDefaultWhere (Model Name):  "   << defaultWhere.modelName
//          << " \nDefaultWhere (Line Number): "   << defaultWhere.lineNumber
//          << "\nPLACEMENT DATA -             "
//          << " \nPlacement:                  "   << PlacNames[placement->value().placement]     << " (" << placement->value().placement << ")"
//          << " \nJustification:              "   << PlacNames[placement->value().justification] << " (" << placement->value().justification << ")"
//          << " \nRelativeTo:                 "   << RelNames[placement->value().relativeTo]     << " (" << placement->value().relativeTo << ")"
//          << " \nPreposition:                "   << PrepNames[placement->value().preposition]   << " (" << placement->value().preposition << ")"
//          << " \nRectPlacement:              "   << RectNames[placement->value().rectPlacement] << " (" << placement->value().rectPlacement << ")"
//          << " \nOffset[0]:                  "   << placement->value().offsets[0]
//          << " \nOffset[1]:                  "   << placement->value().offsets[1]
//          << "\nPLACEMENT WHERE -            "
//          << " \nPlacement Here(Model Name): "   << placement->here().modelName
//          << " \nPlacement Here(Line Number):"   << placement->here().lineNumber
//          << "\nOTHER DATA -                 "
//          << " \n:Parent Relative Type:      "   << RelNames[type] << " (" << type << ")"
//          << " \n:Local:                     "   << local
//          << " \n:Global:                    "   << global
//          << "\n FORMATTED META -            "
//          << "\nMeta Format:                 "   << placement->format(local,global)
//          ;
#endif

  QString newMetaString = placement->format(local,global);

  if (placement->here().modelName == "undefined") {

    Where walk;
    bool partsAdded;
    int eof = gui->subFileSize(defaultWhere.modelName);
    defaultWhere+1 == eof ? walk = defaultWhere : walk = defaultWhere+1;

    // first, lets start from the bottom of the current step - to catch added parts
    scanForward(walk,StepMask);

    // now lets walk backward to see if we hit the end (top) of file.
    if (scanBackward(walk,StepMask,partsAdded) == EndOfFileRc) {
      defaultWhere = firstLine(defaultWhere.modelName);

#ifdef QT_DEBUG_MODE
//      logNotice() << " \nScanBackward[TOP]: EndOfFileRc (StepMask) - defaultLine is: "
//                  << firstLine(defaultWhere.modelName).lineNumber
//                  << " of model: "
//                  << defaultWhere.modelName
//                     ;
#endif

    }

    if (type == StepGroupType) {
      scanForward(defaultWhere,StepGroupBeginMask);

#ifdef QT_DEBUG_MODE
//      logNotice() << " \nScanForward[BOTTOM]: StepGroupType (StepGroupBeginMask) - file name is: "
//                  << defaultWhere.modelName
//                  << " \nStop at line: "
//                  << defaultWhere.lineNumber
//                  << " with line contents: \n"
//                  << gui->readLine(defaultWhere)
//                     ;
#endif

    } else if (type == CalloutType) {
      scanForward(defaultWhere,CalloutEndMask);
      --defaultWhere;

#ifdef QT_DEBUG_MODE
//      logNotice() << " \nScanForward[BOTTOM]: CalloutType (CalloutEndMask) - file name is: "
//                  << defaultWhere.modelName
//                  << " \nStop at line: "
//                  << defaultWhere.lineNumber
//                  << " with line contents: \n"
//                  << gui->readLine(defaultWhere)
//                     ;
#endif

    } else if (defaultWhere.modelName == gui->topLevelFile()) {
      scanPastGlobal(defaultWhere);

#ifdef QT_DEBUG_MODE
//      logNotice() << " \nTopLevelFile[TOP]: ScanPastGlobal - file name is: "
//                  << defaultWhere.modelName
//                  << " \nStop at line: "
//                  << defaultWhere.lineNumber
//                  << " with line contents: \n"
//                  << gui->readLine(defaultWhere)
//                     ;
#endif

    }

    if (defaultWhere.lineNumber == eof){
        insertMeta(defaultWhere,newMetaString);

#ifdef QT_DEBUG_MODE
//        logNotice() << " \nLast line so insert Meta:  \n" << newMetaString << " \nat line: "
//                    << defaultWhere.lineNumber
//                       ;
#endif

    } else {
        appendMeta(defaultWhere,newMetaString);

#ifdef QT_DEBUG_MODE
//        logNotice() << " \nNot last line so append Meta: \n" << newMetaString << " \nat line: "
//                    << defaultWhere.lineNumber+1
//                       ;
#endif

    }

  } else {
    replaceMeta(placement->here(),newMetaString);

#ifdef QT_DEBUG_MODE
//    logNotice() << " \nPlacement defined so replace Meta:  \n" << newMetaString << " \nat line: "
//                << defaultWhere.lineNumber
//                   ;
#endif

  }
}

void MetaItem::changeConstraint(
  const Where   &topOfStep,
  const Where   &bottomOfStep,
  ConstrainMeta *constraint,
  int            append,
  bool           useBot)
{
  if (useBot) {
    setMetaBottomOf(topOfStep,bottomOfStep,constraint,append,true,false);
  } else {
    setMetaTopOf(topOfStep,bottomOfStep,constraint,append,true,false);
  }
}

void MetaItem::changeConstraintStepGroup(
  const Where   &topOfStep,
  const Where   &bottomOfStep,
  ConstrainMeta *constraint,
  int            append)
{
  setMetaBottomOf(topOfStep,bottomOfStep,constraint,append,true,false);
}

void MetaItem::setPointerAttrib(
  QString             title,
  const Where        &topOfStep,
  const Where        &bottomOfStep,
  PointerAttribMeta  *pointerAttrib,
  bool                useTop,
  int                 append,
  bool                local,
  bool                isCallout)
{
  PointerAttribData pointerAttribData = pointerAttrib->value();
  bool ok = PointerAttribDialog::getPointerAttrib(pointerAttribData,title,isCallout);

  if (ok) {
    pointerAttrib->setValue(pointerAttribData);
    setMeta(topOfStep,bottomOfStep,pointerAttrib,useTop,append,local,false); // always local so askLocal = false.
  }
}

void MetaItem::changeInsertOffset(
  InsertMeta *placement)
{
  QString newMetaString = placement->format(false,false);
  replaceMeta(placement->here(),newMetaString);
}

void MetaItem::setPliPartGroupOffset(PliPartGroupMeta *groupMeta)
{
    bool canReplace = false;
    QString metaString;
    Where here(groupMeta->value().group.modelName,
               groupMeta->value().group.lineNumber);
    int nextLine  = here.lineNumber + 1;
    int endOfFile = gui->subFileSize(here.modelName);

    if (!(canReplace = groupMeta->here() == here)) {
        if (groupMeta->bom() && !groupMeta->bomPart()) {
            here.lineNumber = nextLine;
        } else {
            Rc rc;
            Meta mi;
            groupMeta->setWhere(here);
            metaString = gui->readLine(here);
            rc = mi.parse(metaString,here);
            if (!(canReplace = rc == PliPartGroupRc || rc == BomPartGroupRc)) {
                here.lineNumber = nextLine < endOfFile ? nextLine : endOfFile - 1;
                groupMeta->setWhere(here);
            }
        }
    }

#ifdef QT_DEBUG_MODE
//    logDebug() << "\n"
//    << "03 PLI PART GROUP ATTRIBUTES [" + groupMeta->value().type + "_" + groupMeta->value().color + "] - SET OFFSET"
//    << "\n0. BOM:        " <<(groupMeta->value().bom ? "True" : "False")
//    << "\n0. Bom Part:   " <<(groupMeta->value().bom ? groupMeta->value().bPart ? "Yes" : "No" : "N/A")
//    << "\n1. Type:       " << groupMeta->value().type
//    << "\n2. Color:      " << groupMeta->value().color
//    << "\n3. ZValue:     " << groupMeta->value().zValue
//    << "\n4. OffsetX:    " << groupMeta->value().offset[0]
//    << "\n5. OffsetY:    " << groupMeta->value().offset[1]
//    << "\n6. Group Model:" << groupMeta->value().group.modelName
//    << "\n7. Group Line: " << groupMeta->value().group.lineNumber
//    << "\n8. Meta Model: " << groupMeta->here().modelName
//    << "\n9. Meta Line:  " << groupMeta->here().lineNumber
//    ;
#endif

    if (canReplace) {
        metaString = groupMeta->format(groupMeta->pushed,groupMeta->global);
        replaceMeta(groupMeta->here(),metaString);
    } else {
        metaString = groupMeta->format(false,false);
        insertMeta(here, metaString);
    }
}

void MetaItem::togglePartGroups(
                  const Where &topOfStep,
                  const Where &bottomOfStep,
                  bool         bom,
                  BoolMeta    *meta,
                  bool         useTop,
                  int          append,
                  bool         local)
{
    beginMacro("RemovePartGroups");
    meta->setValue(!meta->value());
    setMeta(topOfStep,bottomOfStep,meta,useTop,append,local);
    if (!meta->value()) {
        if (bom) {
            deleteBOMPartGroups();
        } else {
            deletePLIPartGroups(topOfStep,bottomOfStep);
        }
    }
    endMacro();
}

void MetaItem::changeSubModelColor(
  QString title,
  const Where &topOfStep,
  const Where &bottomOfStep,
  StringListMeta *meta,
  bool        local,
  int         append,
  bool        useTop)
{
  StringListMeta _meta;
  _meta.setValue(meta->value());

  bool ok;
  ok = SubModelColorDialog::getSubModelColor(_meta,title,gui);

  if (ok) {
    meta->setValue(_meta.value());
    setMeta(topOfStep,bottomOfStep,meta,useTop,append,local);
  }
}

void MetaItem::changeBackground(
  QString         title,
  const Where    &topOfStep,
  const Where    &bottomOfStep,
  BackgroundMeta *background,
  bool            useTop,
  int             append,
  bool            local,
  bool            picSettings)
{
  BackgroundData backgroundData = background->value();
  bool ok;
  ok = BackgroundDialog::getBackground(backgroundData,title,picSettings,gui);

  if (ok) {
    background->setValue(backgroundData);
    setMeta(topOfStep,bottomOfStep,background,useTop,append,local);
  }
}

void MetaItem::changeSizeAndOrientation(
  QString              title,
  const Where         &topOfStep,
  const Where         &bottomOfStep,
  PageSizeMeta        *smeta,
  PageOrientationMeta *ometa,
  bool                 useTop,
  int                  append,
  bool                 local)
{

  PgSizeData sdata;
  sdata.sizeW  = smeta->value(0);
  sdata.sizeH  = smeta->value(1);
  sdata.sizeID = smeta->valueSizeID();

  OrientationEnc orientation;
  orientation = ometa->value();

  bool ok;
  ok = SizeAndOrientationDialog::getSizeAndOrientation(sdata,orientation,title,gui);

  if (ok) {
      if (orientation != ometa->value()) {
//          logDebug() << " SIZE (dialog return): Orientation: " << (orientation == Portrait ? "Portrait" : "Landscape");
          ometa->setValue(orientation);
          setMeta(topOfStep,bottomOfStep,ometa,useTop,append,local);
        }

      if (sdata.sizeW != smeta->value(0) || sdata.sizeID != smeta->valueSizeID() || sdata.sizeH != smeta->value(1)) {
//          logDebug() << " SIZE (dialog return): Width: " << smeta->value(0) << " Height: " << smeta->value(1) << " SizeID: " << smeta->valueSizeID();

          smeta->setValue(0,sdata.sizeW);
          smeta->setValue(1,sdata.sizeH);
          smeta->setValueSizeID(sdata.sizeID);
          setMeta(topOfStep,bottomOfStep,smeta,useTop,append,local);
        }
    }
}

void MetaItem::hideSubmodel(
        const Where &topOfStep,
        const Where &bottomOfStep,
        BoolMeta *show,
        bool useTop,
        int append,
        bool local){
    show->setValue(false);
    setMeta(topOfStep,bottomOfStep,show,useTop,append,local);
}

void MetaItem::setRendererArguments(
        const Where &top,
        const Where &bottom,
        const QString &renderer,
        StringMeta *rendererArguments,
        bool  useTop,
        int   append,
        bool  local)
{
    QString arguments = rendererArguments->value();

    bool ok = TextEditDialog::getText(arguments,renderer/*QString("%1 Renderer Arguments").arg(renderer)*/);

    if (ok && !arguments.isEmpty()) {

        QStringList list = arguments.split("\n");

        QStringList list2;
        foreach (QString string, list){
          string = string.trimmed();
          QRegExp rx2("\"");
          int pos = 0;
          QChar esc('\\');
          while ((pos = rx2.indexIn(string, pos)) != -1) {
            pos += rx2.matchedLength();
            if (pos < string.size()) {
              QChar ch = string.at(pos-1);
              if (ch != esc) {
                string.insert(pos-1,&esc,1);
                pos++;
              }
            }
          }
          // if last character is \, append space ' ' so not to escape closing string double quote
          if (string.at(string.size()-1) == QChar('\\'))
            string.append(QChar(' '));
          list2 << string;
        }
        rendererArguments->setValue(list2.join("\\n"));

        if (ok) {
            setMeta(top,bottom,rendererArguments,useTop,append,local);
        }
    }
}

void MetaItem::changeSubmodelRotStep(
        QString title,
        const Where &topOfStep,
        const Where &bottomOfStep,
        RotStepMeta *rotStep,
        bool useTop,
        int append,
        bool local){

    RotStepData rotStepData = rotStep->value();
    bool ok = RotStepDialog::getRotStep(rotStepData,title);

    if (ok) {
        rotStep->setValue(rotStepData);
        setMeta(topOfStep,bottomOfStep,rotStep,useTop,append,local);
    }
}

/*
void MetaItem::changePageSize(
  QString         title,
  const Where    &topOfStep,
  const Where    &bottomOfStep,
  UnitsMeta      *meta,
  bool            useTop,
  int             append,
  bool            local)
{
  float values[2];

  values[0]   = meta->value(0);
  values[1]   = meta->value(1);

  bool ok;
  ok = PageSizeDialog::getPageSize(values,title,gui);

  if (ok) {

      meta->setValue(0,values[0]);
      meta->setValue(1,values[1]);

      logDebug() << " SIZE (dialog return): Width: " << meta->value(0) << " Height: " << meta->value(1);

      setMeta(topOfStep,bottomOfStep,meta,useTop,append,local);
  }
}

void MetaItem::changePageOrientation(
  QString              title,
  const Where         &topOfStep,
  const Where         &bottomOfStep,
  PageOrientationMeta *meta,
  bool                 useTop,
  int                  append,
  bool                 local)
{

  OrientationEnc orientation;
  orientation = meta->value();

  bool ok;
  ok = PageOrientationDialog::getPageOrientation(orientation,title,gui);

  if (ok) {

      meta->setValue(orientation);

      logDebug() << " SIZE (dialog return): Orientation: " << (orientation == Portrait ? "Portrait" : "Landscape");

      setMeta(topOfStep,bottomOfStep,meta,useTop,append,local);

  }
}
*/

void MetaItem::changePliSort(
  QString        title,
  const Where   &topOfStep,
  const Where   &bottomOfStep,
  PliSortOrderMeta  *sortMeta,
  int            append,
  bool           local)
{
  PliSortOrderMeta so;
  so.primary            = sortMeta->primary;
  so.secondary          = sortMeta->secondary;
  so.tertiary           = sortMeta->tertiary;
  so.primaryDirection   = sortMeta->primaryDirection;
  so.secondaryDirection = sortMeta->secondaryDirection;
  so.tertiaryDirection  = sortMeta->tertiaryDirection;

  bool ok;
  ok = PliSortDialog::getPliSortOption(so,title,gui);

  if (ok) {
      beginMacro("changePliSort");
      if (so.tertiaryDirection.value() != sortMeta->tertiaryDirection.value()) {
          sortMeta->tertiaryDirection.setValue(so.tertiaryDirection.value());
          setMetaTopOf(topOfStep,bottomOfStep,&sortMeta->tertiaryDirection,append,local, false);
      }
      if (so.secondaryDirection.value() != sortMeta->secondaryDirection.value()) {
          sortMeta->secondaryDirection.setValue(so.secondaryDirection.value());
          setMetaTopOf(topOfStep,bottomOfStep,&sortMeta->secondaryDirection,append,local, false);
      }
      if (so.primaryDirection.value() != sortMeta->primaryDirection.value()) {
          sortMeta->primaryDirection.setValue(so.primaryDirection.value());
          setMetaTopOf(topOfStep,bottomOfStep,&sortMeta->primaryDirection,append,local, false);
      }

      if (so.tertiary.value() != sortMeta->tertiary.value()) {
          sortMeta->tertiary.setValue(so.tertiary.value());
          setMetaTopOf(topOfStep,bottomOfStep,&sortMeta->tertiary,append,local, false);
      }
      if (so.secondary.value() != sortMeta->secondary.value()) {
          sortMeta->secondary.setValue(so.secondary.value());
          setMetaTopOf(topOfStep,bottomOfStep,&sortMeta->secondary,append,local, false);
      }
      if (so.primary.value() != sortMeta->primary.value()) {
          sortMeta->primary.setValue(so.primary.value());
          setMetaTopOf(topOfStep,bottomOfStep,&sortMeta->primary,append,local, false);
      }
      endMacro();
  }
}

void MetaItem::changePliAnnotation(
  QString            title,
  const Where       &topOfStep,
  const Where       &bottomOfStep,
  PliAnnotationMeta *pliAnnotationMeta,
  int                append,
  bool               local)
{
  PliAnnotationMeta annotation;
  annotation.display                   = pliAnnotationMeta->display;
  annotation.titleAnnotation           = pliAnnotationMeta->titleAnnotation;
  annotation.freeformAnnotation        = pliAnnotationMeta->freeformAnnotation;
  annotation.titleAndFreeformAnnotation= pliAnnotationMeta->titleAndFreeformAnnotation;
  bool ok;
  ok = PliAnnotationDialog::getPliAnnotationOption(annotation,title,gui);

  if (ok) {
      beginMacro("changePliAnnotation");
      if(annotation.titleAndFreeformAnnotation.value() != pliAnnotationMeta->titleAndFreeformAnnotation.value()){
          pliAnnotationMeta->titleAndFreeformAnnotation.setValue(annotation.titleAndFreeformAnnotation.value());
          setMetaTopOf(topOfStep,bottomOfStep,&pliAnnotationMeta->titleAndFreeformAnnotation,append,local,true);
      }
      if(annotation.freeformAnnotation.value() != pliAnnotationMeta->freeformAnnotation.value()){
          pliAnnotationMeta->freeformAnnotation.setValue(annotation.freeformAnnotation.value());
          setMetaTopOf(topOfStep,bottomOfStep,&pliAnnotationMeta->freeformAnnotation,append,local,true);
      }
      if(annotation.titleAnnotation.value() != pliAnnotationMeta->titleAnnotation.value()){
          pliAnnotationMeta->titleAnnotation.setValue(annotation.titleAnnotation.value());
          setMetaTopOf(topOfStep,bottomOfStep,&pliAnnotationMeta->titleAnnotation,append,local,true);
      }
      if(annotation.display.value() != pliAnnotationMeta->display.value()){
          pliAnnotationMeta->display.setValue(annotation.display.value());
          setMetaTopOf(topOfStep,bottomOfStep,&pliAnnotationMeta->display,append,local,true);
      }
      endMacro();
    }
}

void MetaItem::changeConstraint(
  QString        title,
  const Where   &topOfStep,
  const Where   &bottomOfStep,
  ConstrainMeta *constraint,
  int            append,
  bool           local)
{
  ConstrainData constrainData = constraint->value();
  bool ok;
  ok = ConstrainDialog::getConstraint(constrainData,title,gui);

  if (ok) {
    constraint->setValueUnit(constrainData);
    setMetaTopOf(topOfStep,bottomOfStep,constraint,append,local, false);
  }
}

void MetaItem::changeFont(
  const Where   &topOfStep,
  const Where   &bottomOfStep,
  FontMeta      *font,
  int            append,
  bool           local,
  bool           useTop)
{

    QFont _font;
    QString fontName = font->valueFoo();
    _font.fromString(fontName);
    bool ok;
    _font = QFontDialog::getFont(&ok,_font);
    fontName = _font.toString();

  if ( ! ok) {
    return;
  }

  font->setValue(fontName);

  logTrace() << "\nCHANGE FONT FUNCTION - "
             << "\nCalling setMeta with "
             << (useTop ? "UseTop: True=Single-Step Page" : " \nFalse=Multi-Step Page")
                         ;
  setMeta(topOfStep,bottomOfStep,font,useTop,append,local);
}

void MetaItem::changeColor(
  const Where &topOfStep,
  const Where &bottomOfStep,
  StringMeta *color,
  int         append,
  bool        local,
  bool        useTop)
{
  QColor _color = LDrawColor::color(color->value());
  _color = QColorDialog::getColor(_color,nullptr);

  if (_color.isValid()) {
    color->setValue(_color.name());
    logNotice() << "\nCHANGE COLOUR FUNCTION - "
                << "\nPAGE- "
                << (useTop ? " \nUseTop: True=Single-Step Page" : " \nFalse=Multi-Step Page")
                           ;
    setMeta(topOfStep,bottomOfStep,color,useTop,append,local);
  }
}

void MetaItem::changeMargins(
  QString        title,
  const Where   &topOfStep,
  const Where   &bottomOfStep,
  MarginsMeta   *margin,
  bool           useTop,
  int            append,
  bool           local)
{
  float values[2];

  values[0] = margin->value(0);
  values[1] = margin->value(1);

  bool ok   = UnitsDialog::getUnits(values,title,QString(),gui);

  if (ok) {
    margin->setValues(values[0],values[1]);
    setMeta(topOfStep,bottomOfStep,margin,useTop,append,local);
  }
}

void MetaItem::changeStepSize(
  QString        title,
  const Where   &topOfStep,
  const Where   &bottomOfStep,
  const QString &labels,
  UnitsMeta     *units,
  int            sizeX,
  int            sizeY,
  int            append,
  bool           local,
  bool           askLocal)
{
  UnitsMeta size;
  size.setValuePixels(XX,sizeX);
  size.setValuePixels(YY,sizeY);

  float values[2];
  if (units->value(XX) == 0.0f)
     values[XX] = size.value(XX);
  else
     values[XX] = units->value(0);
  if (units->value(YY) == 0.0f)
     values[YY] = size.value(YY);
  else
     values[YY] = units->value(1);

  bool ok   = UnitsDialog::getUnits(values,title,labels,gui);
  bool changeX = values[XX] > size.value(XX) || values[XX] < size.value(XX);
  bool changeY = values[YY] > size.value(YY) || values[YY] < size.value(YY);

  if (ok && (changeX || changeY)) {
    units->setValues(values[XX],values[YY]);
    setMetaTopOf(topOfStep,bottomOfStep,units,append,local,askLocal);
  }
}

void MetaItem::changeUnits(
  QString       title,
  const Where  &topOfStep,
  const Where  &bottomOfStep,
  UnitsMeta    *units,
  int           append,
  bool          local)
{
  float values[2];

  values[0] = units->value(0);
  values[1] = units->value(1);

  bool ok   = UnitsDialog::getUnits(values,title,QString(),gui);

  if (ok) {
    units->setValues(values[0],values[1]);
    setMetaTopOf(topOfStep,bottomOfStep,units,append,local);
  }
}

void MetaItem::changeCameraAngles(
  QString        title,
  const Where   &topOfStep,
  const Where   &bottomOfStep,
  FloatPairMeta *floatPairMeta,
  int            append,
  bool           local)
{
  float data[2];
  data[0] = floatPairMeta->value(0);
  data[1] = floatPairMeta->value(1);

  bool ok = FloatPairDialog::getFloatPair(
              title,
              "Latitude",
              "Longitude",
              floatPairMeta,
              data,
              gui);

  if (ok) {
    floatPairMeta->setValues(data[0],data[1]);
    setMetaTopOf(topOfStep,bottomOfStep,floatPairMeta,append,local);
  }
}

void MetaItem::changeFloat(
  QString      title,
  QString      label,
  const Where &topOfStep,
  const Where &bottomOfStep,
  FloatMeta   *floatMeta,
  int          append,
  bool         local)
{
  float data;
  bool ok = FloatDialog::getFloat(title,
                                  label,
                                  floatMeta,
                                  data);

  if (data < floatMeta->_min) {
    data = floatMeta->_min;
  } else if (data > floatMeta->_max) {
    data = floatMeta->_max;
  }

  if (ok) {
    floatMeta->setValue(data);
    setMetaTopOf(topOfStep,bottomOfStep,floatMeta,append,local);
  }
}


void MetaItem::changeFloat(
  const Where &topOfStep,
  const Where &bottomOfStep,
  FloatMeta   *floatMeta,
  int          append,
  bool         local)
{
  setMetaTopOf(topOfStep,bottomOfStep,floatMeta,append,local);
}

void MetaItem::changeFloatSpin(
  QString      title,
  QString      label,
  const Where &topOfStep,
  const Where &bottomOfStep,
  FloatMeta   *floatMeta,
  float        step,
  int          append,
  bool         local)
{
  float data = floatMeta->value();
  bool ok = DoubleSpinDialog::getFloat(
                                  title,
                                  label,
                                  floatMeta,
                                  data,
                                  step,        // spin single step
                                  gui);
  if (ok) {
    floatMeta->setValue(data);
    setMetaTopOf(topOfStep,bottomOfStep,floatMeta,append,local);
  }
}

void MetaItem::changeBorder(
  QString      title,
  const Where &topOfStep,
  const Where &bottomOfStep,
  BorderMeta  *border,
  bool         useTop,
  int          append,
  bool         local,
  bool         rotateArrow,
  bool         corners)
{
  BorderData borderData = border->value();
  bool ok = BorderDialog::getBorder(borderData,title,rotateArrow,corners);

  if (ok) {

    border->setValue(borderData);
    setMeta(topOfStep,bottomOfStep,border,useTop,append,local);
  }
}

void MetaItem::changeImage(
  QString      title,
  const Where &topOfStep,
  const Where &bottomOfStep,
  StringMeta  *image,
  bool         useTop,
  int          append,
  bool         allowLocal,
  bool         checkLocal)
{
  QString cwd = QDir::currentPath();
  QString filter = QFileDialog::tr("Image Files (*.png *.jpg *.jpeg *.bmp)");
  QString fileName = QFileDialog::getOpenFileName(nullptr,title, cwd, filter);
  bool ok = !fileName.isEmpty();
  if (ok) {
    image->setValue(fileName);
    setMeta(topOfStep,bottomOfStep,image,useTop,append,allowLocal,checkLocal);
  }
}

void MetaItem::changeImageScale(
  QString      title,
  QString      label,
  const Where &topOfStep,
  const Where &bottomOfStep,
  FloatMeta   *floatMeta,
  float       step,
  bool  useTop,
  int   append,
  bool askLocal)
{
    float data = floatMeta->value();
    bool ok = DoubleSpinDialog::getFloat(
                                    title,
                                    label,
                                    floatMeta,
                                    data,
                                    step,        // spin single step
                                    gui);
    if (ok) {
      floatMeta->setValue(data);;
    } else {
      ok = floatMeta->value() == 1.0;
    }
    if (ok)
        setMeta(topOfStep,bottomOfStep,floatMeta,useTop,append,askLocal);
}

void MetaItem::changeBool(
  const Where &topOfSteps,
  const Where &bottomOfSteps,
  BoolMeta    *boolMeta,
  bool         useTop,
  int          append,
  bool         local,
  bool         askLocal)   // allow local metas
{
  boolMeta->setValue( ! boolMeta->value());
  setMeta(topOfSteps,bottomOfSteps,boolMeta,useTop,append,local,askLocal);
}

void MetaItem::changeImageFill(
  const Where &topOfSteps,
  const Where &bottomOfSteps,
  FillMeta    *fillMeta,
  bool         useTop,
  int          append,
  bool         local,
  bool         askLocal)   // allow local metas
{
  setMeta(topOfSteps,bottomOfSteps,fillMeta,useTop,append,local,askLocal);
}


void MetaItem::changeDivider(
  QString       title,
  const Where  &topOfStep,
  const Where  &bottomOfStep,
  SepMeta      *sepMeta,
  int           append,
  bool          local)
{
  SepData     sepData = sepMeta->value();
  bool ok = DividerDialog::getDivider(sepData,title,gui);

  if (ok) {
    sepMeta->setValue(sepData);
    setMetaTopOf(topOfStep,bottomOfStep,sepMeta,append,local);
  }
}

void MetaItem::addDivider(
  PlacementType parentRelativeType,
  const Where &bottomOfStep,
  RcMeta *divider,
  AllocEnc defAlloc,
  bool offerStepDivider)
{
  QString divString = divider->preamble;

  if (offerStepDivider) {
    QDialog *dialog = new QDialog();

    QFormLayout *form = new QFormLayout(dialog);
    form->addRow(new QLabel("Divider Allocation"));

    QGroupBox *box = new QGroupBox("Select Allocation");
    form->addWidget(box);

    QList<QRadioButton *> options;
    QStringList allocLabels = QStringList()
    << QString("Vertical") << QString("Horizontal");

    // Set default allocation
    for(int i = 0; i < allocLabels.size(); ++i) {
        QRadioButton *option = new QRadioButton(allocLabels[i],dialog);
        if (allocLabels[i] == "Vertical")
            option->setChecked(defAlloc == Vertical);
        if (allocLabels[i] == "Horizontal")
            option->setChecked(defAlloc == Horizontal);
        if (option->isChecked())
            option->setToolTip("This option places the traditional divider that bisects the current range.");
        else
            option->setToolTip("This option places a divider perpendicular to the previous range divider.");
        options << option;
    }

    QFormLayout *subform = new QFormLayout(box);
    subform->addRow(options[0],options[1]);

    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                               Qt::Horizontal, dialog);
    form->addRow(&buttonBox);
    QObject::connect(&buttonBox, SIGNAL(accepted()), dialog, SLOT(accept()));
    QObject::connect(&buttonBox, SIGNAL(rejected()), dialog, SLOT(reject()));
    dialog->setMinimumWidth(250);

    AllocEnc  alloc = Vertical;
    if (dialog->exec() == QDialog::Accepted) {
        for(int i = 0; i < allocLabels.size(); ++i) {
           QRadioButton * option = options[i];
           if (option->isChecked())
              alloc = (allocLabels[i] == "Vertical" ? Vertical : Horizontal);
        }
    } else {
      return;
    }

    bool stepDivider = alloc != defAlloc;
    if (stepDivider) {
         divString += QString(" STEPS");
    }
  }

  // set the DIVIDER meta as the first line in the next STEP
  Where walk = bottomOfStep;
  int mask = parentRelativeType == StepGroupType ? StepMask|StepGroupMask : StepMask|CalloutMask;
  Rc  rc = scanForward(walk, mask);
  if (rc == StepRc || rc == RotStepRc || rc == EndOfFileRc) {
      appendMeta(bottomOfStep,divString);
  }
}

void MetaItem::deleteDivider(
  PlacementType parentRelativeType,
  const Where &dividerIn)
{
  Where divider = dividerIn;

  if (divider.modelName != "undefined") {
    Rc rc;
    int mask = parentRelativeType == StepGroupType ? StepMask|StepGroupMask : StepMask|CalloutMask;
    Rc expRc = parentRelativeType == StepGroupType ? StepGroupDividerRc : CalloutDividerRc;

    ++divider;
    Where sepMeta(divider);
    rc = scanForward(divider,mask);

    if (rc == expRc) {
      Meta content;
      bool haveSepMeta = false;
      int numLines = gui->subFileSize(sepMeta.modelName);
      scanPastGlobal(sepMeta);
      for ( ; sepMeta < numLines; sepMeta++) {
          QString line = gui->readLine(sepMeta);
          rc = content.parse(line,sepMeta);
          if ((haveSepMeta =
               rc == SepRc)       ||
               rc == StepRc       ||
               rc == RotStepRc    ||
               rc == CalloutEndRc ||
               rc == StepGroupEndRc ) {
              break;
          }
      }

      beginMacro("deleteDivider");
      deleteMeta(divider);
      if (haveSepMeta)
          deleteMeta(sepMeta);
      endMacro();
    }
  }
}

void MetaItem::changeAlloc(
  const Where &topOfSteps,
  const Where &bottomOfSteps,
  AllocMeta   &alloc,
  int          append)
{
  bool metaInRange;
  int lineNumber = alloc.here().lineNumber;
  bool calledOut = alloc.preamble.contains("CALLOUT");
  Where topOf    = topOfSteps;

  if (calledOut && lineNumber && lineNumber < topOf.lineNumber) {
    Where walk = topOf+1;
    Rc rc = scanBackward(walk,CalloutEndMask|StepMask|StepGroupMask);
    if (rc == EndOfFileRc)
      scanPastGlobal(walk);
    if (walk.lineNumber <= lineNumber)
      topOf = walk;
  }

  metaInRange = lineNumber >= topOf.lineNumber &&
                lineNumber <= bottomOfSteps.lineNumber &&
                alloc.here().modelName == topOf.modelName;

//#ifdef QT_DEBUG_MODE
//  logInfo() << "\nCHANGE ALLOC SETTINGS - "
//            << " \nHere Alloc     Model Name: " << alloc.here().modelName
//            << " \nHere TopOf     Model Name: " << topOf.modelName
//            << " \nHere Alloc    Line Number: " << alloc.here().lineNumber
//            << " \nHere TopOf    Line Number: " << topOf.lineNumber
//            << " \nHere BottomOf Line Number: " << bottomOfSteps.lineNumber
//            << " \nCalled out:                " << (calledOut ? "YES" : "NO")
//            << " \nAppend:                    " << (metaInRange ? "YES" : calledOut ? "NO" : append == 0 ? "NO" : "YES")
//            << " \nMeta In Range:             " << (metaInRange ? QString("YES - Line %1").arg(alloc.format(alloc.pushed,alloc.global)) :
//                                                                  QString("NO - Line %1").arg(alloc.format(false,false)))
//            << "\n -- "
//            ;
//#endif

  AllocEnc allocType = alloc.value();
  alloc.setValue(allocType == Vertical ? Horizontal : Vertical);

  QString line;
  if (metaInRange) {
      line = alloc.format(alloc.pushed,alloc.global);
      replaceMeta(alloc.here(),line);
    } else {
      line = alloc.format(false,false);
      if (calledOut) {
        append = 0;
        setMetaBottomOf(topOf,bottomOfSteps,&alloc,append,false,false,false);
      } else {
        setMetaTopOf(topOf,bottomOfSteps,&alloc,append,false,false,false);
      }
    }
}

/********************************************************************
 *
 * Insert cover and non-cover pages
 *
 ********************************************************************/

bool MetaItem::okToInsertCoverPage()
{
  bool frontCover = gui->displayPageNum <= gui->firstStepPageNum;
  bool backCover  = gui->displayPageNum >    gui->lastStepPageNum;

  return frontCover || backCover;
}
bool MetaItem::okToAppendCoverPage()
{
  bool frontCover = gui->displayPageNum < gui->firstStepPageNum;
  bool backCover = gui->displayPageNum >= gui->lastStepPageNum;

  return frontCover || backCover;
}

void MetaItem::insertCoverPage()
{
  Rc rc;
  QString line;
  Meta content;
  Where here(gui->topLevelFile(),0);
  QString meta = "0 !LPUB INSERT COVER_PAGE FRONT";
  int numLines = gui->subFileSize(here.modelName);

  scanPastGlobal(here);                         //scan past headers and global

  beginMacro("InsertCoverPage");
  for ( ; here < numLines; here++) {            //scan forward
      line = gui->readLine(here);
      rc = content.parse(line,here);

      if (rc == StepRc    ||
          rc == RotStepRc) {                    //check if line is 0 STEP
          insertMeta(here,meta);                //STEP is there, so (insert) meta just before
          break;
      } else if (rc == StepGroupBeginRc) {
          insertMeta(here,meta);                //MULTI-STEP BEGIN is here, so (insert) meta just before
          appendMeta(here,step);                //then (append) step just after
          break;
      } else {                                  //NO STEP to start so...
          QStringList tokens;                   //check for non-zero line
          split(line,tokens);
          bool token_1_5 = tokens.size() && tokens[0].size() == 1 &&
               tokens[0] >= "1" && tokens[0] <= "5";
          if (token_1_5) {                      //non-zero line detected so insert cover page here
              insertMeta(here,meta);            //(insert) meta
              appendMeta(here,step);            //then (append) step just after
              break;
          }
      }
  }
  endMacro();
}


bool MetaItem::frontCoverPageExist()
{
    Rc rc;
    QString line;
    Meta content;
    Where here(gui->topLevelFile(),0);
    int numLines = gui->subFileSize(here.modelName);

    scanPastGlobal(here);                         //scan past headers and global

    for ( ; here < numLines; here++) {            //scan forward
        line = gui->readLine(here);
        rc   = content.parse(line,here);

        if (rc == InsertCoverPageRc) {            //check if line is Cover Page
            return true;                          //it is a cover page, so return true
        } else {                                  //no cover page detected so...
            QStringList tokens;                   //check if non-zero line
            split(line,tokens);
            bool token_1_5 = tokens.size() && tokens[0].size() == 1 &&
                 tokens[0] >= "1" && tokens[0] <= "5";
            if (token_1_5) {                      //non-zeor line detected so return false
                return false;
            }
        }
    }
    return false;
}

void MetaItem::appendCoverPage()
{
  Rc rc;
  QString line;
  Meta content;
  Where here(gui->topLevelFile(),0);
  here.lineNumber = gui->subFileSize(here.modelName); //start at bottom of file
  QString meta    = "0 !LPUB INSERT COVER_PAGE BACK";
  here--;

  beginMacro("AppendCoverPage");
  line = gui->readLine(here);
  rc   = content.parse(line,here);

   if ((rc == StepGroupEndRc ||                        //STEP so advance line before 'insert' meta
        rc == StepRc)) {
      here++;
      insertMeta(here,meta);
//      appendMeta(here,step);
   } else {
      appendMeta(here,step);                           //NO STEP, 'append' one before appending meta
      here++;
      appendMeta(here,meta);
//      appendMeta(here,step);
   }
   endMacro();
}

bool MetaItem::backCoverPageExist()
{
    Rc rc;
    QString line;
    Meta content;
    bool endStep = true;
    Where here(gui->topLevelFile(),0);
    here.lineNumber = gui->subFileSize(here.modelName); //start at bottom of file
    here--;                                             //adjust to readline from zero-start index

    for ( ; here >= 0; here--) {                        //scan backwards
        line = gui->readLine(here);
        rc = content.parse(line,here);
        if ((rc == StepRc) && endStep) {                //if end STEP, continue
            endStep = false;
            continue;
        } else if (rc == InsertCoverPageRc) {           //check if line is Cover Page
            return true;
        } else if (rc == StepGroupEndRc ||              //if Step Grpup end or STEP, then there is no back cover page
                  (rc == StepRc && ! endStep)) {        //so return false
            return false;
        } else {
            QStringList tokens;                         //no STEP encountered os check for non-zero line
            split(line,tokens);
            bool token_1_5 = tokens.size() && tokens[0].size() == 1 &&
                 tokens[0] >= "1" && tokens[0] <= "5";
            if (token_1_5) {                            //non-zeor line detected so no back cover page, return false
                return false;
            }
        }
    }
    return false;
}

bool MetaItem::okToInsertNumberedPage()
{
  bool frontCover = gui->displayPageNum >= gui->firstStepPageNum;
  bool backCover  = gui->displayPageNum <=  gui->lastStepPageNum;

  return frontCover || backCover;
}
bool MetaItem::okToAppendNumberedPage()
{
  return okToInsertNumberedPage();
}

void MetaItem::insertNumberedPage()
{
  QString meta = "0 !LPUB INSERT PAGE";
  insertPage(meta);
}
void MetaItem::appendNumberedPage()
{
  QString meta = "0 !LPUB INSERT PAGE";
  appendPage(meta);
}

void MetaItem::insertPage(QString &meta)
{
  Where topOfStep = gui->topOfPages[gui->displayPageNum-1];

  scanPastGlobal(topOfStep);

  beginMacro("InsertPage");
  appendMeta(topOfStep,"0 STEP");
  appendMeta(topOfStep,meta);
  endMacro();
}

void MetaItem::appendPage(QString &metaCommand)
{
  Rc rc;
  QString line;
  Meta mi;
  Where bottomOfStep(gui->topOfPages[gui->displayPageNum]);  //start at the bottom of the page's last step

  bool addStepMeta = false;
  Where here = bottomOfStep;
  int numLines = gui->subFileSize(here.modelName);

  for ( ; here > 0; --here) {                                //scan from bottom to top of file
    line = gui->readLine(here);
    rc = mi.parse(line,here);
    if (rc == StepRc || rc == RotStepRc ||
        rc == StepGroupEndRc || rc == CalloutEndRc) {         //we are on a boundary command so advance one line and break
        here++;
      break;
    }

    QStringList tokens;
    split(line,tokens);
    bool token_1_5 = tokens.size() &&                        //no boundary command so check for valid step content
         tokens[0].size() == 1 &&
         tokens[0] >= "1" && tokens[0] <= "5";
    if (token_1_5 || isHeader(line) ||
       (tokens.size() == 4 &&
        tokens[2] == "INSERT" &&
        tokens[3] == "COVER_PAGE")) {
      addStepMeta = true;
      break;
    }
  }

  if ( ! addStepMeta) {
    for ( ; here < numLines; ++here) {
      QString line = gui->readLine(here);
      QStringList tokens;
      split(line,tokens);
      bool token_1_5 = tokens.size() &&
                       tokens[0].size() == 1 &&
                       tokens[0] >= "1" &&
                       tokens[0] <= "5";                      //non-zeor line detected
      if (token_1_5 || (tokens.size() >= 4  && tokens[2].contains(QRegExp("^INSERT$")))) {
        addStepMeta = true;
        break;
      }
    }
  }

  beginMacro("appendPage");
  appendMeta(bottomOfStep,metaCommand);
  if (addStepMeta) {
    bottomOfStep++;
    appendMeta(bottomOfStep,"0 STEP");
  }
  endMacro();
}

void MetaItem::deletePage()
{
  Where topOfPage    = gui->topOfPages[gui->displayPageNum-1];
  Where bottomOfPage = gui->topOfPages[gui->displayPageNum];
  ++topOfPage;
  int numLines = bottomOfPage.lineNumber - topOfPage.lineNumber;

  beginMacro("deletePage");
  for (int i = 0; i <= numLines; i++) {
    deleteMeta(topOfPage);
  }
  endMacro();
 }

void MetaItem::insertPicture()
{
  QString title = QFileDialog::tr("Select Image");
  QString cwd = QDir::currentPath();
  QString filter = QFileDialog::tr("Image Files (*.png *.jpg *.jpeg *.bmp)");
  QString fileName = QFileDialog::getOpenFileName(nullptr,title, cwd, filter);

  if (fileName != "") {
    QString meta = QString("0 !LPUB INSERT PICTURE \"%1\" OFFSET 0.5 0.5") .arg(fileName);
    Where topOfStep = gui->topOfPages[gui->displayPageNum-1];
    scanPastGlobal(topOfStep);
    appendMeta(topOfStep,meta);
  }
}

void MetaItem::updateText(
   const Where   &here,
   const QString &_text,
   QString       &_editFont,
   QString       &_editFontColor,
   float          _offsetX,
   float          _offsetY,
   int            _parentRelaiveType,
   bool           _isRichText,
   bool           append)
{
    bool ok         = true;
    bool initialAdd = true;
    bool stepFound  = true;
    QString text    = _text;
    if (!text.isEmpty()){
      initialAdd      = false;
      QStringList pre = text.split("\\n");
      text = pre.join(" ");
    }

    int thisStep          = 1;
    Where insertPosition  = here;
    bool isRichText       = _isRichText;
    QString windowTitle   = QString("Edit %1 Text").arg(isRichText ? "Rich" : "Plain");
    QString editFont      = _editFont;
    QString editFontColor = _editFontColor;
    bool hasOffset        = _offsetX != 0.0f || _offsetY != 0.0f;
    QString offset        = hasOffset ? QString(" OFFSET %1 %2")
                                                .arg(qreal(_offsetX))
                                                .arg(qreal(_offsetY)) : QString();
    bool textPlacement    = gui->page.meta.LPub.page.textPlacement.value();
    QString placementStr;
    if (textPlacement && initialAdd) {
      PlacementMeta placementMeta = gui->page.meta.LPub.page.textPlacementMeta;
      placementMeta.preamble = QString("0 !LPUB INSERT %1 PLACEMENT ")
                                       .arg(isRichText ? "RICH_TEXT" : "TEXT");
      PlacementData placementData = placementMeta.value();
      textPlacement = PlacementDialog
           ::getPlacement(PlacementType(_parentRelaiveType),TextType,placementData,"Placement",DefaultPage);
      if (textPlacement) {
        if (hasOffset){
          placementData.offsets[XX] = _offsetX;
          placementData.offsets[YY] = _offsetY;
        }
        placementMeta.setValue(placementData);;
        placementStr = placementMeta.format(true/*local*/,false);
        bool getStep = (_parentRelaiveType == StepGroupType &&
                        placementData.relativeTo != PageType &&
                        placementData.relativeTo != PageHeaderType &&
                        placementData.relativeTo != PageFooterType && false /*KO - disabled until fixed*/);
        if (getStep) {
            // set step to insert
            thisStep = QInputDialog::getInt(gui,"Steps","Which Step",1,1,100,1,&ok);
            Steps *steps = dynamic_cast<Steps *>(&gui->page);
            if (ok && steps){
                /* foreach range */
                stepFound = false;
                for (int i = 0; i < steps->list.size() && !stepFound; i++) {
                    if (steps->list[i]->relativeType == RangeType){
                       Range *range = dynamic_cast<Range *>(steps->list[i]);
                       if (range) {
                           /* foreach step*/
                           for (int j = 0; j < range->list.size(); j++) {
                              Step *step = dynamic_cast<Step *>(range->list[j]);
                              if (step && step->stepNumber.number == thisStep) {
                                  insertPosition = step->topOfStep();
                                  stepFound      = true;
                                  break;
                              }
                           }
                       }
                    }
                }
            }
         }
      }

      bool showMessage = !textPlacement || !ok ||!stepFound;

      if (showMessage) {
        QPixmap _icon = QPixmap(":/icons/lpub96.png");
        QMessageBoxResizable box;
        box.setWindowIcon(QIcon());
        box.setIconPixmap (_icon);
        box.setTextFormat (Qt::RichText);

        box.setWindowTitle(QMessageBox::tr ("Text Placement Select"));
        box.setWindowFlags (Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
        box.setMinimumSize(40,20);

        QString message = !textPlacement ? QString("Placement selection was cancelled or not valid.&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;") :
                          !ok ?            QString("Specified step number %1 is not valid. Bottom of multi steps will be used.").arg(thisStep) :
                                           QString("Step number %1 was not found. Bottom of multi steps will be used.").arg(thisStep);
        QString body = QMessageBox::tr ("Woulld you like to cancel the %1 text action ?").arg(initialAdd ? "add" : "update");
        box.setText (message);
        box.setInformativeText (body);
        box.setStandardButtons (QMessageBox::No | QMessageBox::Yes);
        box.setDefaultButton   (QMessageBox::No);
        if (box.exec() == QMessageBox::Yes) {
            return;
        }
      }
    }

    ok = TextEditDialog::getText(text,editFont,editFontColor,isRichText,windowTitle,true);
    if (ok && !text.isEmpty()) {

    QStringList list = text.split("\n");
    QStringList list2;
    foreach (QString string, list){
      string = string.trimmed();
      QRegExp rx2("\"");
      int pos = 0;
      QChar esc('\\');
      while ((pos = rx2.indexIn(string, pos)) != -1) {
        pos += rx2.matchedLength();
        if (pos < string.size()) {
          QChar ch = string.at(pos-1);
          if (ch != esc) {
            string.insert(pos-1,&esc,1);
            pos++;
          }
        }
      }
      // if last character is \, append space ' ' so not to escape closing string double quote
      if (string.at(string.size()-1) == QChar('\\'))
        string.append(QChar(' '));
      list2 << string;
    }

    QString strMeta;
    if (isRichText)
      strMeta = QString("0 !LPUB INSERT RICH_TEXT \"%1\"%2")
                        .arg(list2.join("\\n")) .arg(textPlacement ? "" : offset);
    else
      strMeta = QString("0 !LPUB INSERT TEXT \"%1\" \"%2\" \"%3\"%4")
                        .arg(list2.join("\\n")) .arg(editFont) .arg(editFontColor) .arg(textPlacement ? "" : offset);

    beginMacro("UpdateText");
    if (append)
      appendMeta(insertPosition,strMeta);
    else
      replaceMeta(insertPosition,strMeta);
    if (textPlacement && initialAdd) {
      Where walkFwd = insertPosition+1;
      appendMeta(walkFwd,placementStr);
    }
    endMacro();
  }
}

void MetaItem::insertText()
{
    Where insertPosition, walkBack, topOfStep, bottomOfStep;

    bool append        = true;
    bool isRichText    = false;
    bool multiStep     = false;
    bool textPlacement = gui->page.meta.LPub.page.textPlacement.value();
    PlacementType parentRelativeType = PageType;

    if (gui->getCurrentStep()){
        if ((multiStep = gui->getCurrentStep()->multiStep))
            insertPosition = gui->getCurrentStep()->bottomOfSteps();
        else
            insertPosition = gui->getCurrentStep()->topOfStep();
    } else {
        insertPosition = gui->page.topOfSteps();
    }

    if (insertPosition.modelName == gui->topLevelFile() && insertPosition.lineNumber < 2)
        scanPastGlobal(insertPosition);

    if (textPlacement) {
      QStringList textFormatOptions;
      textFormatOptions << "Plain Text" << "Rich Text";
      QInputDialog textTypeDlg;
      bool ok;
      QString textFormat =
              QInputDialog::getItem(gui,
                                    QString("Add Text"),
                                    QString("Text Format:"),
                                    textFormatOptions, 0,false,&ok);
      if (ok && !textFormat.isEmpty())
        isRichText = textFormat.contains(textFormatOptions.last());

      if (!ok) {
          QPixmap _icon = QPixmap(":/icons/lpub96.png");
          QMessageBoxResizable box;
          box.setWindowIcon(QIcon());
          box.setIconPixmap (_icon);
          box.setTextFormat (Qt::RichText);

          box.setWindowTitle(QMessageBox::tr ("Text Format Select"));
          box.setWindowFlags (Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
          box.setMinimumSize(40,20);

          QString message = QString("Text format selection was cancelled or not valid.&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;");
          QString body = QMessageBox::tr ("Woulld you like to cancel the add text action ?");
          box.setText (message);
          box.setInformativeText (body);
          box.setStandardButtons (QMessageBox::No | QMessageBox::Yes);
          box.setDefaultButton   (QMessageBox::No);
          if (box.exec() == QMessageBox::Yes) {
              return;
          }
      }
    }

    if (multiStep)
        parentRelativeType = StepGroupType;

    QString empty;
    QString fontString = QString("Arial,24,-1,255,%1,0,0,0,0,0").arg(isRichText ? "75" /*Bold*/ : "50" /*Normal*/);
    updateText(insertPosition, empty, fontString, empty, 0.0f, 0.0f, parentRelativeType, isRichText, append);
}

void MetaItem::insertBOM()
{
  QString meta = QString("0 !LPUB INSERT BOM");
  Where topOfStep = gui->topOfPages[gui->displayPageNum];
  scanPastGlobal(topOfStep);
  insertMeta(topOfStep,meta);
}

int MetaItem::okToInsertFinalModel()
{
  Rc rc;
  QString line;
  Meta content;
  Where saveHere;                                          //initialize saveHere
  Where here(gui->topLevelFile(),0);
  here.lineNumber = gui->subFileSize(here.modelName);      //start at bottom of file
  here--;                                                  //adjust lineNumber using from zero-start index

  for ( ; here >= 0; here--) {                             //scan from bottom to top of file
    line = gui->readLine(here);
    rc = content.parse(line,here);
    if (rc == StepRc || rc == RotStepRc) {                 //if Step save the line number to perform perform insert (place before) later
        saveHere = here.lineNumber;
    } else if (
      rc == StepGroupEndRc || rc == CalloutEndRc) {        //if Step, StepGroup, RotStep or Callout, save the line number
      logDebug() << "End of Step or Callout - Ok to insert Final Model at line: " << here.lineNumber;
      return here.lineNumber;                              //reached a valid boundary so so return line number
    } else if (rc == InsertFinalModelRc) {                 //check if insert final model
      logStatus() << "Final model detected at line: " << here.lineNumber;
      return -1;
    } else {                                               //else keep walking back until 1_5 line
      QStringList tokens;
      split(line,tokens);
      bool token_1_5 = tokens.size() && tokens[0].size() == 1 &&
          tokens[0] >= "1" && tokens[0] <= "5";
      if (token_1_5) {                                     //non-zero line detected so no back final model
        if (saveHere.lineNumber > 0) {
            return saveHere.lineNumber;
        } else {
          int fileSize = gui->subFileSize(here.modelName) - 1;
          if (here.lineNumber < (fileSize)) {                //check that we are not at the end of the file
            logDebug() << "Line type 1-5 detected - Ok to insert Final Model at line: " << here.lineNumber;
            return here.lineNumber + 1;                      //step forward (backup one line) and return line number
          } else {
            logDebug() << "Line type 1-5 detected - Ok to insert Final Model at line: " << here.lineNumber;
            return here.lineNumber;                          //at last line so return line number
          }
        }
      }
    }
  }
  return -1;
}


void MetaItem::insertFinalModel(int atLine)
{
  // final model already installed so exit.
  if (atLine <= 0){
    return;
  }

  // grab the passed in line
  Where here(gui->topLevelFile(),atLine);
  bool atStep = (QString(gui->readLine(here))
                 .contains(QRegExp("^0 STEP$")));

  QString modelMeta = QString("0 !LPUB INSERT MODEL");
  QString pageMeta  = QString("0 !LPUB INSERT PAGE");

  beginMacro("insertFinalModel");
  if (atStep) {
    appendMeta(here,modelMeta);
    appendMeta(++here,pageMeta);
    appendMeta(++here,step);
  } else {
    appendMeta(here,step);
    appendMeta(++here,modelMeta);
    appendMeta(++here,pageMeta);
  }
  emit gui->messageSig(LOG_INFO, QString("Final model inserted at lines: %1 to %2").arg(atLine+1).arg(here.lineNumber+1));
  endMacro();
}

void MetaItem::deleteFinalModel(){

  int maxLines;
  QStringList tokens;
  Where here(gui->topLevelFile(),0);                             //start at bottom of file
  here.lineNumber = maxLines = gui->subFileSize(here.modelName);
  here--;

  Where finalModelLine;
  Where stopAtThisLine;

  Rc rc;
  Meta meta;
  bool foundFinalModel = false;
  for (; here >=0; here--) {                                    //scan backwards until Model
      QString line = gui->readLine(here);
      rc = meta.parse(line,here);
      if (rc == InsertFinalModelRc) {                           //model found so locate position of page insert line
          foundFinalModel  = true;
          finalModelLine   = here;                              //mark line as starting point for deletion
          if ((here.lineNumber + 1) < maxLines) {               //check if line before is page insert and adjust starting point for deletion
              Where lineBefore = here+1;
              line = gui->readLine(lineBefore);
              rc = meta.parse(line,lineBefore);
              if (rc == InsertPageRc) {
                  finalModelLine = lineBefore;                  //adjust starting point for deletion
                }
            }
          logInfo() << "Final model metas detected at lines: " << here.lineNumber << "and" << finalModelLine.lineNumber;
        }
      else
      if (foundFinalModel && rc == StepRc){                     //at at final model STEP command ...
          beginMacro("deleteFinalModel");
          stopAtThisLine = here/* - 1*/;
          Where walk = finalModelLine;
          for (; walk >= stopAtThisLine.lineNumber ; walk-- ){   //remove lines between model insert and model insert step
              logDebug() << "Deleting inserted final model line No"
                         << walk.lineNumber << " in "
                         << walk.modelName << "["
                         << gui->readLine(walk) << "]"
                            ;
              deleteMeta(walk);
            }
          endMacro();
          return;
      } else {
         split(line,tokens);
         bool token_1_5 = tokens.size() && tokens[0].size() == 1 &&
              tokens[0] >= "1" && tokens[0] <= "5";
         if (token_1_5) {                                      //we have reached a non-zero line so there is no final model
             return;
         }
      }
   }
}

void MetaItem::insertSplitBOM()
{
  QString pageMeta = QString("0 !LPUB INSERT PAGE");
  QString bomMeta  = QString("0 !LPUB INSERT BOM");

  Where topOfStep = gui->topOfPages[gui->displayPageNum-1];

  scanPastGlobal(topOfStep);

  beginMacro("splitBOM");
  deleteBOMPartGroups();
  insertPage(pageMeta);
  appendMeta(topOfStep+1,bomMeta);
  endMacro();
}

void MetaItem::deleteBOM()
{
  Where topOfPage    = gui->topOfPages[gui->displayPageNum-1];
  Where bottomOfPage = gui->topOfPages[gui->displayPageNum];
  for (++topOfPage; topOfPage.lineNumber < bottomOfPage.lineNumber; topOfPage++) {
    QString line = gui->readLine(topOfPage);
    Meta meta;
    Rc rc;

    rc = meta.parse(line,topOfPage);
    if (rc == InsertRc) {
      InsertData insertData = meta.LPub.insert.value();

      if (insertData.type == InsertData::InsertBom) {
        beginMacro("deleteBOM");
        deleteBOMPartGroups();
        deleteMeta(topOfPage);
        endMacro();
        break;
      }
    }
  }
}

void MetaItem::deleteBOMPartGroups()
{
  Where topOfPage    = gui->topOfPages[gui->displayPageNum-1];
  Where bottomOfPage = gui->topOfPages[gui->displayPageNum];
  for (Where walk = bottomOfPage; walk >= topOfPage.lineNumber; walk--) {
    Meta mi;
    QString metaString = gui->readLine(walk);
    Rc rc = mi.parse(metaString,walk);
    if (rc == PliPartGroupRc || rc == BomPartGroupRc){
        deleteMeta(walk);
    }
  }
}

void MetaItem::deletePLIPartGroups(
        const Where &topOfStep,
        const Where &bottomOfStep)
{
  for (Where walk = bottomOfStep; walk >= topOfStep.lineNumber; walk--) {
    Meta mi;
    QString metaString = gui->readLine(walk);
    Rc rc = mi.parse(metaString,walk);
    if (rc == PliPartGroupRc || rc == BomPartGroupRc){
        deleteMeta(walk);
    }
  }
}

void MetaItem::resetPartGroup(
        const Where &which)
{
    Where here = which;
    Meta mi;
    QString metaString = gui->readLine(here);
    Rc rc = mi.parse(metaString,here);
    if (rc == PliPartGroupRc || rc == BomPartGroupRc){
        deleteMeta(here);
    }
}

void MetaItem::changeImageItemSize(
  QString         title,
  const Where    &topOfStep,
  const Where    &bottomOfStep,
  UnitsMeta      *meta,
  bool            useTop,
  int             append,
  bool            local)
{
  float values[2];

  values[0]   = meta->value(0);
  values[1]   = meta->value(1);

  bool ok;
  ok = RotateIconSizeDialog::getRotateIconSize(values,title,gui);

  if (ok) {

      meta->setValue(0,values[0]);
      meta->setValue(1,values[1]);

      //logDebug() << " SIZE (dialog return): Width: " << meta->value(0) << " Height: " << meta->value(1);

      setMeta(topOfStep,bottomOfStep,meta,useTop,append,local);
  }
}

void MetaItem::deleteImageItem(Where &topOfStep, QString &metaCommand){
  Rc rc;
  Meta meta;
  Where walk = topOfStep + 1;                                     // advance past STEP meta
  int numLines = gui->subFileSize(topOfStep.modelName);
  scanPastGlobal(topOfStep);
  QRegExp rotateIconMeta("^\\s*0\\s+!LPUB\\s+.*"+metaCommand);
  for ( ; walk < numLines; walk++) {
      QString line = gui->readLine(walk);
      if (line.contains(rotateIconMeta)) {
          deleteMeta(walk);
          walk--;                                                 // compensate for deleted line
        } else {
          QStringList tokens;
          split(line,tokens);
          bool token_1_5 = tokens.size() && tokens[0].size() == 1 &&
              tokens[0] >= "1" && tokens[0] <= "5";
          if (token_1_5) {                                        //non-zero line detected so break
              break;
            }
          rc = meta.parse(line,walk);
          if (rc == StepRc) {                                     //reached end of step so break
              break;
            }
        }

    }
}

/***************************************************************************/

void MetaItem::scanPastLPubMeta(
  Where &topOfStep)
{
  Where walk = topOfStep + 1;

  int  numLines  = gui->subFileSize(walk.modelName);
  if (walk < numLines) {
    QString line = gui->readLine(walk);
    QRegExp lpubLine("^\\s*0\\s+!LPUB\\s+.*");
    if (line.contains(lpubLine) || isHeader(line) || isComment(line)) {
      for ( ++walk; walk < numLines; ++walk) {
        line = gui->readLine(walk);
        //logTrace() << "Scan Past GLOBAL LineNum (final -1): " << walk.lineNumber << ", Line: " << line;
        if ( ! line.contains(lpubLine) && ! isHeader(line) && ! isComment(line)) {
          topOfStep = walk - 1;
          break;
        }
      }
    }
  }
}

Rc MetaItem::scanForward(Where &here,int mask)
{
  bool partsAdded;

  return scanForward(here, mask, partsAdded);
}

Rc  MetaItem::scanForward(
  Where &here,
  int    mask,
  bool  &partsAdded)
{
  Meta tmpMeta;
  int  numLines  = gui->subFileSize(here.modelName);
  partsAdded = false;

  scanPastGlobal(here);

  for ( ; here < numLines; here++) {
    QString line = gui->readLine(here);
    QStringList tokens;

    split(line,tokens);

    bool token_1_5 = tokens.size() && tokens[0].size() == 1 && tokens[0] >= "1" && tokens[0] <= "5";

    if (token_1_5) {
      partsAdded = true;

    } else {
      Rc rc = tmpMeta.parse(line,here);

      if (rc == InsertRc && ((mask >> rc) & 1)) {
         //return rc;

      } else if (rc == StepRc || rc == RotStepRc) {

        if (((mask >> rc) & 1) && partsAdded) {

          return rc;
        }
        partsAdded = false;
      } else {
        if (rc < ClearRc && ((mask >> rc) & 1)) {

          return rc;
        }
      }
    }
  }
  return EndOfFileRc;
}

Rc MetaItem::scanBackward(Where &here,int mask)
{
  bool partsAdded;

  return scanBackward(here, mask, partsAdded);
}

Rc MetaItem::scanBackward(
  Where &here,
  int    mask,  // What we stop on
  bool  &partsAdded)
{
  Meta tmpMeta;
  partsAdded     = false;

  for ( ; here >= 0; here--) {

    QString line = gui->readLine(here);
    QStringList tokens;

#ifdef QT_DEBUG_MODE
//    logNotice() << "\n==SCAN BACKWARD READLINE==: "
//              << " \nMask:StepRc = (1 << StepRc)|(1 << RotStepRc): "  << mask
//              << " \nHere LINE:          " << here.lineNumber
//              << " \nHere Model:         " << here.modelName
//              << " \nLine:               " << line
//              << " \nIs Header:          " << isHeader(line)
//              << " \nParts Added:        " << partsAdded
//                   ;
#endif

    if (isHeader(line)) {
      scanPastGlobal(here);

#ifdef QT_DEBUG_MODE
//      logNotice() << "\nIN SCAN BACKWARD AT HEADER: "
//                << " \nScanPastGlobal"
//                << " \nRETURN EndOfFileRc at line:"  << here.lineNumber
//                   ;
#endif

      return EndOfFileRc;
    }
    split(line,tokens);

    bool token_1_5 = tokens.size() && tokens[0].size() == 1 && tokens[0] >= "1" && tokens[0] <= "5";

    if (token_1_5) {
      partsAdded = true;

    } else {
      Rc rc = tmpMeta.parse(line,here);

      if (rc == InsertPageRc /*&& ((mask >> rc) & 1)*/) {

#ifdef QT_DEBUG_MODE
//          logNotice() << "\nIN SCAN BACKWARD AT INSERT PAGE: "
//                    << " \nRETURN InsertRc at Line:"  << here.lineNumber
//                       ;
#endif

         return rc;

      } else if (rc == InsertCoverPageRc /* && ((mask >> rc) & 1)*/) {

#ifdef QT_DEBUG_MODE
//          logNotice() << "\nIN SCAN BACKWARD AT INSERT COVER PAGE: "
//                    << " \nRETURN InsertRc at Line:"  << here.lineNumber
//                       ;
#endif

         return rc;

      } else if (rc == StepRc || rc == RotStepRc) {

#ifdef QT_DEBUG_MODE
//          logNotice() << "\nIN SCAN BACKWARD AT STEP: "
//                    << " \nRc == StepRc || RotStep at Line:"  << here.lineNumber
//                       ;
#endif

          if (((mask >> rc) & 1) && partsAdded) {

#ifdef QT_DEBUG_MODE
//              logNotice() << "\nIN SCAN BACKWARD AT STEP WITH PARTS: "
//                        << " \nParts Added: " << partsAdded
//                        << " \nRETURN StepRc|RotStepRc at Line:"  << here.lineNumber
//                         ;
#endif

          return rc;
         }
        partsAdded = false;
      } else if (rc < ClearRc && ((mask >> rc) & 1)) {

#ifdef QT_DEBUG_MODE
//          logNotice() << "\nIN SCAN BACKWARD AT CLEAR: "
//                     << " \nRETURN ClearRc at Line:"  << here.lineNumber
//                       ;
#endif

        return rc;
      }
    }
  }
  return EndOfFileRc;
}

void MetaItem::insertMeta(const Where &here, const QString &line)
{
  gui->insertLine(here, line);
}
void MetaItem::appendMeta(const Where &here, const QString &line)
{
  gui->appendLine(here, line);
}

void MetaItem::replaceMeta(const Where &here, const QString &line)
{
  gui->replaceLine(here,line);
}

void MetaItem::deleteMeta(const Where &here)
{
  gui->deleteLine(here);
}

void MetaItem::beginMacro(QString name)
{
  gui->beginMacro(name);
}

void MetaItem::endMacro()
{
  gui->endMacro();
}

void MetaItem::scanPastGlobal(
  Where &topOfStep)
{
  QRegExp globalLine(GLOBAL_META_RX);
  gui->scanPast(topOfStep,globalLine);
}

Where MetaItem::firstLine(
  QString modelName)
{
  Where foo = sortedGlobalWhere(modelName,"ZZZZZ");
  return --foo;
}

Where MetaItem::sortedGlobalWhere(
  QString modelName,
  QString metaString)
{
  Meta tmpMeta;
  return sortedGlobalWhere(tmpMeta,modelName,metaString);
}

Where MetaItem::sortedGlobalWhere(
  Meta    &, /* unused */
  QString modelName,
  QString  /* unused */)
{
  Where walk = Where(modelName,0);
  int maxLines = gui->subFileSize(modelName);
  QRegExp lines1_5("^\\s*[1-5]");

  bool header = true;

  /* Skip all of the header lines */

  for ( ; walk < maxLines; walk++)
  {
    QString line = gui->readLine(walk);

    if (walk.lineNumber == 0) {
      QStringList argv;
      split(line,argv);
      if (argv[0] != "0") {
        insertMeta(walk,"0");
        maxLines++;
      }
    }

    if (walk > 0) {
      if (header) {
        header &= isHeader(line);
        if ( ! header) {
          break;
        }
      }
    }
  }

  return walk;
}

int MetaItem::numSteps(QString modelName)
{
  return gui->numSteps(modelName);
}

/***********************************************************************
 *
 * Callout tools
 *
 **********************************************************************/

int MetaItem::nestCallouts(
  Meta  *meta,
  const QString &modelName,
  bool  isMirrored)
{
  bool restart = true;

  while (restart) {

    restart = false;

    Where walk(modelName,1);

    int numLines = gui->subFileSize(walk.modelName);

    bool partIgnore = false;
    bool callout = false;

    // modelName is called out, so any submodels in modelName need to be
    // called out

    // for all the lines in the file

    for ( ; walk.lineNumber < numLines; ++walk) {

      QString line = gui->readLine(walk);

      QStringList argv;

      split(line,argv);

      // Process meta-commands so we don't turn ignored or substituted
      // submodels get called out

      if (argv.size() >= 2 && argv[0] == "0") {
        if (argv[1] == "LPUB" || argv[1] == "!LPUB") {
          if (argv.size() == 5 && argv[2] == "PART"
                               && argv[3] == "BEGIN"
                               && argv[4] == "IGN") {
            partIgnore = true;
          } else if (argv.size() == 5 && argv[2] == "PART"
                                      && argv[3] == "END") {
            partIgnore = false;
          } else if (argv.size() == 4 && argv[2] == "CALLOUT"
                                      && argv[3] == "BEGIN") {
            callout = true;
          } else if (argv.size() == 4 && argv[2] == "CALLOUT"
                                      && argv[3] == "END") {
          callout = false;
          } else if (argv.size() >= 3 && argv[3] == "MULTI_STEP") {
            deleteMeta(walk);
            --numLines;
            --walk;
          }
        }
      } else if ( ! callout && ! partIgnore) {

        // We've got a part added

        if (argv.size() == 15 && argv[0] == "1") {
          if (gui->isSubmodel(argv[14])) {
            meta->submodelStack << SubmodelStack(walk.modelName,walk.lineNumber,0);
            addCalloutMetas(meta,argv[14],isMirrored);
            nestCallouts(meta,argv[14],isMirrored);
            meta->submodelStack.removeLast();
            restart = true;
            break;
          }
        }
      }
    }
  }
  return 0;
}

bool MetaItem::canConvertToCallout(
  Meta *meta)
{
  SubmodelStack tos = meta->submodelStack[meta->submodelStack.size() - 1];
  Where walkBack(tos.modelName,tos.lineNumber);

  for (; walkBack.lineNumber >= 0; walkBack--) {
    QString line = gui->readLine(walkBack);

    if (isHeader(line)) {
      return true;
    } else {
      QStringList argv;
      split(line,argv);
      if (argv.size() == 5 && argv[0] == "0" &&
         (argv[1] == "LPUB" || argv[1] == "!LPUB") &&
          argv[2] == "CALLOUT" && argv[3] == "BEGIN") {
        return false;
      }
      if (argv.size() >= 2 && argv[0] == "0") {
        if (argv[1] == "STEP" || argv[1] == "ROTSTEP" ||
            argv[1] == "LPUB" || argv[1] == "!LPUB") {
          return true;
        }
      }
    }
  }
  return true;
}

void MetaItem::convertToCallout(
  Meta *meta,
  const QString &modelName,
  bool  isMirrored,
  bool  assembled)
{
  gui->maxPages = -1;

  beginMacro("convertToCallout");
  addCalloutMetas(meta,modelName,isMirrored,assembled);
  if ( ! assembled) {
    nestCallouts(meta,modelName,isMirrored);
  }
  endMacro();
}

void MetaItem::addCalloutMetas(
  Meta *meta,
  const QString &modelName,
  bool  /* isMirrored */,
  bool  assembled)
{
  /* Scan the file and remove any multi-step stuff from the file
     we're converting to callout*/

  int   numLines;
  Where walk(modelName,0);

  if (! assembled) {
    numLines = gui->subFileSize(modelName);
    walk.lineNumber = numLines - 1;
    QRegExp ms("^\\s*0\\s+\\!*LPUB\\s+MULTI_STEP\\s+");

    do {
      QString line = gui->readLine(walk);
      if (line.contains(ms)) {
        deleteMeta(walk);
      }
    } while (--walk >= 0);
  }

  /* submodelStack tells us where this submodel is referenced in the
     parent file */

  SubmodelStack tos = meta->submodelStack[meta->submodelStack.size() - 1];
  Where calledOut(tos.modelName,tos.lineNumber);

  /* Now scan the lines following this line, to see if there is another
   * part just like this one that needs to be added as a callout
   * multiplier.
   *
   * We also want to scan backward for the same submodel.
   *
   * In either direction, we need to stop on STEP/ROTSTEP.  We also need
   * to stop on other sub-models, or mirror images of the same sub-model.
   */

  int instanceCount = 0;

  QString firstLine;
  Where lastInstance, firstInstance;

  /* FIXME: separate submodels of different color ? */

  Where walkBack = calledOut;
  for (; walkBack.lineNumber >= 0; walkBack--) {
    QString line = gui->readLine(walkBack);

    if (isHeader(line)) {
      break;
    } else {
      QStringList argv;
      split(line,argv);
      if (argv.size() >= 2 && argv[0] == "0") {
        if (argv[1] == "STEP" || argv[1] == "ROTSTEP" ||
            argv[1] == "LPUB" || argv[1] == "!LPUB") {
          break;
        }
      } else if (argv.size() == 15 && argv[0] == "1") {
        if (gui->isSubmodel(argv[14])) {
          if (argv[14] == modelName) {
            if (firstLine == "") {
              firstLine = line;
              firstInstance = walkBack;
              lastInstance = walkBack;
              ++instanceCount;
            } else {
              if (equivalentAdds(firstLine,line)) {
                firstInstance = walkBack;
                ++instanceCount;
              } else {
                break;
              }
            }
          } else {
            break;
          }
        }
      }
    }
  }

  walk = calledOut + 1;
  numLines = gui->subFileSize(walk.modelName);
  for ( ; walk.lineNumber < numLines; walk++) {
    QString line = gui->readLine(walk);
    QStringList argv;
    split(line,argv);
    if (argv.size() >= 2 && argv[0] == "0") {
      if (argv[1] == "STEP" || argv[1] == "ROTSTEP" ||
          argv[1] == "LPUB" || argv[1] == "!LPUB") {
        break;
      }
    } else if (argv.size() == 15 && argv[0] == "1") {
      if (gui->isSubmodel(argv[14])) {
        if (argv[14] == modelName) {
          if (firstLine == "") {
            firstLine = line;
            firstInstance = walk;
            ++instanceCount;
          } else {
            if (equivalentAdds(firstLine,line)) {
              lastInstance = walk;
              ++instanceCount;
            } else {
              break;
            }
          }
        } else {
          break;
        }
      }
    }
  }

  if (instanceCount) {

    bool together;

    if (assembled) {
      if (instanceCount > 1) {
        QMessageBox::StandardButton pushed;
        pushed = QMessageBox::question(gui,gui->tr("Multiple Copies"),
                                           gui->tr("There are multiple copies, do you want them as one callout?"),
                                           QMessageBox::Yes|QMessageBox::No,
                                           QMessageBox::Yes);
        together = pushed == QMessageBox::Yes;
      } else {
        together = false;
      }
    } else {
      together = true;
    }

    Where thisInstance = firstInstance;
    for (int i = 0; i < instanceCount; i++) {
      /* defaultPointerTip is the trick - it calculates the pointer tip
         for a given instance of a callout.  It does this by rendering
         the parent image with the non-called out parts color A and
         the called out parts color B.  Then the resultant image is
         searched for color B.  The parent model needs to be rotated
         by ROTSTEP for this to work. */
      //QPointF offset = defaultPointerTip(*meta, instances[i],isMirrored);
      QStringList argv;
      split(firstLine,argv);
      QPointF offset = defaultPointerTip(*meta,
                                         walk.modelName,
                                         firstInstance.lineNumber,
                                         modelName,
                                         i,
                                         gui->isMirrored(argv));
      emit gui->messageSig(LOG_DEBUG, QString("[Tip Point] (%1, %2)").arg(QString::number(offset.x(),'f',6))
                                                                     .arg(QString::number(offset.y(),'f',6)));

      QString line = QString("%1 %2 0 0 0 0 0 0 1") .arg(offset.x()) .arg(offset.y());

      if (together) {
        appendMeta(lastInstance,"0 !LPUB CALLOUT POINTER CENTER 0 " + line);
        ++lastInstance.lineNumber;
      } else {
        appendMeta(thisInstance,"0 !LPUB CALLOUT POINTER CENTER 0 " + line);
        ++thisInstance.lineNumber;
        appendMeta(thisInstance,"0 !LPUB CALLOUT END");
        --thisInstance.lineNumber;
        if (assembled) {
          QString begin = "0 !LPUB CALLOUT BEGIN ROTATED";
          insertMeta(thisInstance,begin);
          thisInstance.lineNumber += 5;
        } else {
          insertMeta(thisInstance,"0 !LPUB CALLOUT BEGIN");
          thisInstance.lineNumber += 4;
        }
      }
    }
    if (together) {
      appendMeta(lastInstance,"0 !LPUB CALLOUT END");
      if (assembled) {
        QString begin = "0 !LPUB CALLOUT BEGIN ROTATED";
        insertMeta(firstInstance,begin);
      } else {
        insertMeta(firstInstance,"0 !LPUB CALLOUT BEGIN");
      }
    }
  }
}

/*
 *
 * Pointer Tip Content START
 *
 */

void MetaItem::addPointerTip(
    Meta *meta,
    const Where &fromHere,
    const Where &toHere,
    PlacementEnc placement,
    Rc           rc)
{
  gui->maxPages = -1;

  beginMacro("addPointerTip");
  addPointerTipMetas(meta,fromHere,toHere,placement,rc);
  endMacro();
}

void MetaItem::addPointerTipMetas(
    Meta *meta,
    const Where &fromHere,
    const Where &toHere,
    PlacementEnc placement,
    Rc           rc)
{
  QString placementName;
  if (rc == PagePointerRc) {
      placementName = bPlacementEncNames[placement];
  }

  QPointF centerOffset = QPointF(0.5,0.5);

  int partLoc[2]  = { 0,0 };
  int csiSize[2]  = { 0,0 };
  int partSize[2] = { 0,0 };

  if (offsetPoint(*meta,fromHere,toHere,partLoc,csiSize,partSize))
      centerOffset = QPointF(qreal(partLoc[XX])/csiSize[XX], qreal(partLoc[YY])/csiSize[YY]);

  QString pointerType;
  switch (rc)
  {
     case PagePointerRc:
        pointerType = "PAGE POINTER";
        break;
    case CalloutDividerPointerRc:
        pointerType = "CALLOUT DIVIDER_POINTER";
        break;
     case StepGroupDividerPointerRc:
        pointerType = "MULTI_STEP DIVIDER_POINTER";
        break;
     default:
         break;
  }

  QString preamble = QString("0 !LPUB %1 CENTER")
                             .arg(pointerType);

  QString line = QString("%1 0.0 %2 %3 0.0 0.0 0.0 0.0 0.0 0.0 0.0 1 %4")
                         .arg(preamble)
                         .arg(centerOffset.x())
                         .arg(centerOffset.y())
                         .arg(placementName);

  //logTrace() << "META" << meta->LPub.pointerBase.pointer.format(false,false);

  logTrace() << "\nPAGE_POINTER LINE: " << line;

  Where walk = toHere;
  if (rc == StepGroupDividerPointerRc)
     scanForward(walk,StepMask);

  insertMeta(walk,line);
}

void MetaItem::updateCsiAnnotationIconMeta(
  const Where &here, CsiAnnotationIconMeta *caim)
{
  if (here.modelName != "undefined") {
    QString repLine = caim->format(false,false);
    replaceMeta(here,repLine);
  }
}

void MetaItem::writeCsiAnnotationMeta(
  QStringList  &parts,
  const Where  &fromHere,
  const Where  &toHere,
  Meta         *meta,
  bool          update)
{
  QHash<QString,QString> hash;

  beginMacro("annotationIconMetas");

  Where start = fromHere;
  Where end   = toHere;

  QString preamble = "0 !LPUB ASSEM ANNOTATION ICON";

  // Default placement

  PlacementData pld = meta->LPub.assem.annotation.placement.value();
  QString placements;
  bool hasJustification = (pld.justification != Center && pld.preposition != Inside);
  placements = placementNames[pld.placement]+" ";
  if (hasJustification)
      placements += placementNames[pld.justification]+" ";
  placements +=  prepositionNames[pld.preposition];

  // Unpack the part parts, modelNames and lineNumbers involved - do not reorder

  for (int i = parts.size() - 1; i >= 0; --i) {

    // extract components

    QStringList partIds = QString(parts[i].section('@',0,0)).split(";");
    QString partName    = partIds.at(0);                // extract partName
    QString partColor   = partIds.at(1);                // extract partColor
    QString modelName   = partIds.at(2);                // extract modelName
    QString lines       = parts[i].section('@',1,1);
    lines.chop(1);                                      // remove trailing ;
    QStringList lineNumStrings = lines.split(";");      // extract lineNumbers

    // for each modelName, sort the list of lineNumbers

    QList<int> lineNumbers;
    for (int j = 0; j < lineNumStrings.size(); ++j) {
      lineNumbers << lineNumStrings[j].toInt();
    }
    qSort(lineNumbers.begin(),lineNumbers.end());

    // process from last to first so as to not disturb line numbers

    int lastLineNumber = -1;
    for (int j = lineNumbers.size() - 1; j >= 0; --j) {

      int lineNumber = lineNumbers[j];

      if (lineNumber != lastLineNumber) {

        Where here(modelName,lineNumber);

        int partLoc[2]      = { 0,0 }; // used to calculate part offset
        int csiSize[2]      = { 0,0 }; // used to calculate part offset
        int partSize[2]     = { 0,0 };
        float iconOffset[2] = { 0.0f,0.0f };
        float partOffset[2] = { 0.5f,0.5f };

        QString line;

        if (offsetPoint(*meta,start,end,partLoc,csiSize,partSize,lineNumber)) {

            // part offset is alwasy calculcated from center
            partOffset[XX] = float(partLoc[XX])/csiSize[XX];
            partOffset[YY] = float(partLoc[YY])/csiSize[YY];

            line = QString("%1 %2 %3 %4 %5 %6 %7 %8 %9 %10")
                           .arg(preamble)
                           .arg(placements)
                           .arg(iconOffset[XX],0,'f',0)
                           .arg(iconOffset[YY],0,'f',0)
                           .arg(partOffset[XX],0,'f',5)
                           .arg(partOffset[YY],0,'f',5)
                           .arg(partSize[XX])
                           .arg(partSize[YY])
                           .arg(partColor)
                           .arg(partName);
        } else {
            emit gui->messageSig(LOG_ERROR, QString("Could not generate meta for line [%1]").arg(line));
            return;
        }
#ifdef QT_DEBUG_MODE
//        logTrace() << "\nCSI ANNOTATION ICON LINE: " << line
//                  << "\nCSI ANNOTATION ICON META: " << meta->LPub.assem.annotation.icon.format(false,false)
                      ;
#endif
        if (update) {
            Where meta = here + 1;
            gui->replaceLine(meta,line);
        } else {
            gui->appendLine(here,line);
        }

        // increase toHere lines with each added line
        end++;

        lastLineNumber = lineNumber;
      }
    }
  }
  endMacro();
}

/* offsetPoint is the trick - it calculates a specified position
   point for part(s) in a specified modelName and at specified
   lineNumber(s). It does this by rendering the parent image with
   the model's non-step parts color A and the step's parts color B.
   Then the resultant image is searched for color B. The parent
   model needs to be rotated by ROTSTEP for this to work.
*/

bool MetaItem::offsetPoint(
        Meta    &meta,
  const Where   &fromHere,
  const Where   &toHere,
  int          (&partLoc)[2],
  int          (&csiSize)[2],
  int          (&partSize)[2],
  int            partLineNum)
{
  int start           = fromHere.lineNumber;
  int end             = toHere.lineNumber;
  bool partAnnotation = partLineNum > -1;
  QString modelName   = fromHere.modelName;
  QString title       = "pointer";
  QString label       = title;

  if (partAnnotation){
      title           = "annotation";
  }

#ifdef QT_DEBUG_MODE
  label              += QString("_%1_").arg(partLineNum);
#endif

  // line adjustment for custom color entry
  int colorLines      = 3; // transwhite
  end                += colorLines;
  partLineNum        += colorLines;

  /*
   * Create a "transwhite" version of the model/submodel
   */
  QString monoOutName = QDir::currentPath() + "/" + Paths::tmpDir + "/" + modelName;
  monoOutName = makeMonoName(monoOutName,monoColor[TransWhite]);
  monoColorSubmodel(modelName,monoOutName,monoColor[TransWhite]);

  QFile inFile(monoOutName);
  if ( ! inFile.open(QFile::ReadOnly | QFile::Text)) {
    emit gui->messageSig(LOG_ERROR,QString("Generate %1 offset cannot read file %2: %3")
                                           .arg(title)
                                           .arg(monoOutName)
                                           .arg(inFile.errorString()));
      return false; //pagePosition.center();
  }

  QTextStream in(&inFile);
  QStringList csiParts;

  /*
   * Gather up the "transwhite" parent model up to the step
   * then gather up the "blue" step parts
   */

  QString partType = QString();
  for (int i = 0; i < end; i++) {
      QString line = in.readLine(0);
      if (line.contains("0 Name: "))
          continue;
      // create blue part(s)
      if (i >= start) {
          QStringList argv;
          split(line,argv);
          if (argv.size() == 15) {
              if (partAnnotation) {
                  if (i == partLineNum) {
                      argv[1] = monoColorCode[Blue];
                      partType = argv[14];
                  }
              } else {
                  argv[1] = monoColorCode[Blue];
              }
              line = argv.join(" ");
          }
      }
      csiParts << line;
  }

  if (csiParts.size() == 0) {
      return false;
  }

  bool ok[2];
  QString pngName, ldrName;
  QStringList ldrNames, csiKeys;
  QString addLine = "1 0 0 0 0 1 0 0 0 1 0 0 0 1 " + modelName;

  // this block has been refactored to reflect that this function exclusively uses the Native Renderer
  if (renderer->useLDViewSCall()) {
      ldrName = QDir::currentPath() + "/" + Paths::tmpDir + "/" + label + "Mono.ldr";
      pngName = QDir::currentPath() + "/" + Paths::assemDir + "/" + label + "Mono.png";
      ldrNames << ldrName;
      csiKeys << title + "Mono";
      ok[0] = (renderer->rotateParts(addLine,meta.rotStep,csiParts,ldrName,modelName,meta.LPub.assem.cameraAngles) == 0);
      ok[1] = (renderer->renderCsi(addLine,ldrNames,csiKeys,pngName,meta) == 0);
    } else {
      ok[0] = true;
      pngName = QDir::currentPath() + "/" + Paths::tmpDir + "/" + label + "Mono.png";
      if (Preferences::usingNativeRenderer){
          ldrName = QDir::currentPath() + "/" + Paths::tmpDir + "/csi.ldr";
          ok[0] = (renderer->rotateParts(addLine,meta.rotStep,csiParts,ldrName,modelName,meta.LPub.assem.cameraAngles) == 0);
      }
      ok[1] = (renderer->renderCsi(addLine,csiParts,csiKeys,pngName,meta) == 0);
    }

  if (ok[0] && ok[1]) {
    QPixmap pixmap;
    pixmap.load(pngName);

    QImage color = pixmap.toImage();
    QImage alpha = pixmap.toImage();

    int width = color.width();
    int height = color.height();
    int left = 32000, top = -1, right = -1, bottom = -1;

    for (int y = 0; y < height; y++) {
      for (int x = 0; x < width; x++)  {
        QColor a = alpha.pixel(x,y);

        if (a.blue()) {
          QColor c = color.pixel(x,y);

          int red = c.red(), green = c.green(), blue = c.blue();

          if (blue - (red + green)/2 > 64) {
            if (top == -1) {
              top = y;
            }
            if (left > x) {
              left = x;
            }
            if (bottom < y) {
              bottom = y;
            }
            if (right < x) {
              right = x;
            }
          }
        }
      }
    }

    left = partAnnotation ? left : (right+left)/2;
    top  = partAnnotation ? top : (top+bottom)/2;

    if (left > width || top > height) {
      left = partAnnotation ? width : width/2;
      top  = partAnnotation ? height : height/2;
    }

    csiSize[0]  = width;      // csi
    csiSize[1]  = height;     // csi

    partSize[0] = right-left; // part
    partSize[1] = bottom-top; // part

    partLoc [0] = left;
    partLoc [1] = top;

    emit gui->messageSig(LOG_DEBUG,QString("%1 for model [%2]:")
                                           .arg(partAnnotation ? "Part ["+partType+"] annotation" :
                                                           "Default pointer tip position")
                                           .arg(modelName));
    emit gui->messageSig(LOG_DEBUG,QString(" -partLeft (locX):   %1").arg(QString::number(left)));
    emit gui->messageSig(LOG_DEBUG,QString(" -partTop  (locY):   %1").arg(QString::number(top)));
    emit gui->messageSig(LOG_DEBUG,QString(" -partBottom:        %1").arg(QString::number(bottom)));
    emit gui->messageSig(LOG_DEBUG,QString(" -partRight:         %1").arg(QString::number(right)));
    emit gui->messageSig(LOG_DEBUG,QString(" -partWidth (sizeX): %1").arg(QString::number(right-left)));
    emit gui->messageSig(LOG_DEBUG,QString(" -partHeight(sizeY): %1").arg(QString::number(bottom-top)));
    emit gui->messageSig(LOG_DEBUG,QString(" -csiWidth  (sizeX): %1").arg(QString::number(width)));
    emit gui->messageSig(LOG_DEBUG,QString(" -csiHeight (sizeY): %1").arg(QString::number(height)));

    return true;
  }
  emit gui->messageSig(LOG_ERROR, QString("Render momo %1 image for %2 failed.")
                       .arg(title.toLower())
                       .arg(partAnnotation ? "part ["+partType+"]" : "model ["+modelName+"]"));
  return false;
}

/*
 *
 * Pointer Tip Content END
 *
 */

void MetaItem::removeCallout(
  const QString &modelName,
  const Where   &topOfCallout,
  const Where   &bottomOfCallout)
{
  gui->maxPages = -1;

  /* scan the called out model and remove any dividers */

  int  numLines = gui->subFileSize(modelName);

  Where walk(modelName,numLines-1);
  Rc rc;

  beginMacro("removeCallout");

  do {
    rc = scanBackward(walk,CalloutDividerMask);
    if (rc == CalloutDividerRc) {
      deleteMeta(walk);
    }
  } while (rc != EndOfFileRc);

  QRegExp callout("^\\s*0\\s+\\!*LPUB\\s+CALLOUT");
  for (walk = bottomOfCallout;
       walk >= topOfCallout.lineNumber;
       walk--)
  {
    QString line = gui->readLine(walk);
    if (line.contains(callout)) {
      deleteMeta(walk);
    }
  }
  endMacro();
}

void MetaItem::unnestCallouts(
  const QString &modelName)
{
  Where walk(modelName,1);

  int numLines = gui->subFileSize(walk.modelName);

  bool partIgnore = false;
  bool callout = false;

  for ( ; walk.lineNumber < numLines; ++walk) {

    QString line = gui->readLine(walk);

    QStringList argv;

    split(line,argv);

    if (argv.size() >= 2 && argv[0] == "0") {
      if (argv[1] == "LPUB" || argv[1] == "!LPUB") {
        if (argv.size() == 4 && argv[2] == "CALLOUT"
                             && argv[3] == "BEGIN") {
          callout = true;
          deleteMeta(walk);
          --numLines;
          --walk;
        } else if (argv.size() == 4 && argv[2] == "CALLOUT"
                                    && argv[3] == "END") {
          callout = false;
          deleteMeta(walk);
          --numLines;
          --walk;
        }
      }
    } else if ( ! callout && ! partIgnore) {
      if (argv.size() == 15 && argv[0] == "1") {
        if (gui->isSubmodel(argv[14]) && callout) {
          unnestCallouts(argv[14]);
        }
      }
    }
  }
}

void MetaItem::updatePointer(
  const Where &here, PointerMeta *pointer)
{
  if (here.modelName != "undefined") {
    QString repLine = pointer->format(false,false);
    replaceMeta(here,repLine);
  }
}

void MetaItem::deletePointer(const Where &here, bool line, bool border)
{
   Where _here = here;
   if (line)
       deletePointerAttribute(_here,true);
   if (border)
       deletePointerAttribute(_here,true);
   deleteMeta(_here);
}

void  MetaItem::deletePointerAttribute(const Where &here, bool all)
{
  if (!all) {
     deleteMeta(here);
  } else {
     Where walk = here;
     QString pattern = QString("^.*(POINTER_ATTRIBUTE (LINE|BORDER)).*$");
     QRegExp rx(pattern);
     QString line = gui->readLine(++walk); // advance 1 line
     if (line.contains(rx)) {
         deleteMeta(walk);                 // check first line
     }
     else
     {
        ++walk;                           // check second line
        line = gui->readLine(walk);
        if (line.contains(rx)) {
            deleteMeta(walk);
        }
     }
  }
}

/*
 * Algorithm for figuring out a good location for callout pointer tips.
 *
 * information we need:
 *   1. the model up to this instance of the callout
 *
 * for each part in the model {
 *   if (part is submodel) {
 *     create transwhite version of submodel;
 *   } else {
 *     color part transwhite
 *   }
 * }
 *
 * create red version of callout
 * change this instance of the callout to red.
 *
 * render the picture
 *
 * scan image for bounding box of red
 *
 * calculate offset using center of bounding box
 *
 */

QString MetaItem::makeMonoName(const QString &fileName,
   const QString &color)
{
  QString mono = "mono_";
  QString altColor = "_" + monoColor[(color == monoColor[Blue] ? TransWhite : Blue)];
  QFileInfo info(fileName);
  QString suffix = info.suffix().isEmpty() ? QString() : "." + info.suffix();
  QString baseName = info.completeBaseName();
  if (info.completeBaseName().right(altColor.size()) == ("_" + monoColor[TransWhite]))
      baseName = info.completeBaseName().left(info.completeBaseName().length() - altColor.size());
  if ((info.fileName().left(mono.size())) == mono)
      return info.absolutePath() + "/" + baseName + "_" + color + suffix;
  return info.absolutePath() + "/" + mono + baseName + "_" + color + suffix;
}

int MetaItem::monoColorSubmodel(
  const QString &modelName,
  const QString &monoOutName,
  const QString &color)
{
  MonoColors colorCode = (color == monoColor[TransWhite] ? TransWhite : Blue);

  QFile outFile(monoOutName);
  if ( ! outFile.open(QFile::WriteOnly | QFile::Text)) {
    emit gui->messageSig(LOG_ERROR,QString("MonoColorSubmodel cannot write file %1: %2.")
                         .arg(monoOutName)
                         .arg(outFile.errorString()));
    return -1;
  }

  QTextStream out(&outFile);
  out << "0 // LPub3D part custom color" << endl;
  out << "0 !COLOUR LPub3D_White CODE 11015 VALUE #FFFFFF EDGE #FFFFFF ALPHA 32" << endl;
  out << "0" << endl;

  int numLines = gui->subFileSize(modelName);

  Where walk(modelName,0);

  for ( ; walk < numLines; walk++) {
    QString line = gui->readLine(walk);
    QStringList argv;

    split(line,argv);

    if (argv.size() == 15 && argv[0] == "1") {
      QFileInfo info(argv[14]);
      QString submodel = info.completeBaseName();
      QString suffix = info.suffix().isEmpty() ? QString() : "." + info.suffix();

      if (submodel.right(color.size()) == color) {
        submodel = submodel.left(submodel.length() - color.size());
      }
      submodel += suffix;
      if (gui->isSubmodel(submodel)) {
        QString model = QDir::currentPath() + "/" + Paths::tmpDir + "/" + argv[14];
        model = makeMonoName(model,color);
        monoColorSubmodel(submodel,model,color);
        QFileInfo info(model);
        argv[14] = info.fileName();
      }
      argv[1] = monoColorCode[colorCode];
    } else if ((argv.size() == 8 && argv[0]  == "2") ||
               (argv.size() == 11 && argv[0] == "3") ||
               (argv.size() == 14 && argv[0] == "4") ||
               (argv.size() == 14 && argv[0] == "5")) {
      argv[1] = monoColorCode[colorCode];
    }
    line = argv.join(" ");
    out << line << endl;
  }

  outFile.close();
  return 0;
}

QPointF MetaItem::defaultPointerTip(
  Meta    &meta,
  QString &modelName,         // where modelName
  int      lineNumber,        // where lineNumber @first Callout instance
  const QString &subModel,    // called out submodel
  int     instance,
  bool    isMirrored)
{
  QPointF centerOffset = QPointF(0.5,0.5);

  /*
   * Create a "transwhite" version of the submodel that calls out our callout
   */

  QString monoOutName = QDir::currentPath() + "/" + Paths::tmpDir + "/" + modelName;
  monoOutName = makeMonoName(monoOutName,monoColor[TransWhite]);
  monoColorSubmodel(modelName,monoOutName,monoColor[TransWhite]);

  QFile inFile(monoOutName);
  if ( ! inFile.open(QFile::ReadOnly | QFile::Text)) {
    emit gui->messageSig(LOG_ERROR,QString("defaultPointerTip cannot read file %1: %2.")
                         .arg(monoOutName)
                         .arg(inFile.errorString()));
    return centerOffset;
  }

  QTextStream in(&inFile);
  QStringList csiParts;

  /*
   * Gather up the "transwhite" parent model up to the callout
   */

  // line adjustment for custom color entry
  int colorLines         = 3; // transwhite
  int numLines           = gui->subFileSize(modelName) + colorLines;
  int adjustedLineNumber = lineNumber + colorLines;
  int instances          = 0;
  QFileInfo info(subModel);
  QString suffix = info.suffix().isEmpty() ? QString() : "." + info.suffix();
  QString monoSubModel = "mono_" + info.completeBaseName() + "_" + monoColor[TransWhite] + suffix;
  QStringList argv;
  int i;
  for (i = 0; i < numLines; i++) {
    QString line = in.readLine(0);
    split(line,argv);
    if (i >= adjustedLineNumber) {
      if (argv.size() == 15) {
        bool mirrored = gui->isMirrored(argv);
        if (argv[0] == "1" &&
            argv[14] == monoSubModel &&
            mirrored == isMirrored) {
          if (instances++ == instance) {
            break;
          }
        }
      }
    }
    else
    {
      // get the last ROTSTEP...
      if (argv.size() > 0 && argv[0] == "0") {
        Where here(modelName,i);
        Rc rc = meta.parse(line,here,false);
        if (rc == RotStepRc) {
#ifdef QT_DEBUG_MODE
          gui->messageSig(LOG_DEBUG,QString("Called out subModel %1 ROTSTEP %2")
                          .arg(subModel)
                          .arg(renderer->getRotstepMeta(meta.rotStep)));
#endif
        }
      }
    }

    csiParts << line;
  }
  if (i == numLines) {
    return centerOffset;
  }

  /*
   * Create a "blue" version of the called out assembly
   */

  QString fileName = QDir::currentPath() + "/" + Paths::tmpDir + "/" + argv[14]; // monoSubModel
  fileName = makeMonoName(fileName,monoColor[Blue]);
  QString tmodelName = info.fileName();
  monoColorSubmodel(tmodelName,fileName,monoColor[Blue]);
  info.setFile(fileName);
  argv[1] = monoColorCode[Blue];
  argv[14] = info.fileName();

  /*
   * Add blue submodel to csiParts
   */

  csiParts << argv.join(" ");

  QString addLine;

  if (isMirrored) {

    SubmodelStack tos = meta.submodelStack[meta.submodelStack.size() - 1];

    if (meta.submodelStack.size() > 1) {
      tos = meta.submodelStack[meta.submodelStack.size() - 2];
      Where here(tos.modelName, tos.lineNumber+1);
      addLine = gui->readLine(here);
    } else {
      addLine = "1 0 0 0 0 1 0 0 0 1 0 0 0 1 " + modelName;
    }
  } else {
    addLine = "1 0 0 0 0 1 0 0 0 1 0 0 0 1 " + modelName;
  }

  /*
   * Rotate and render transwhite and blue image
   */

  bool ok[2];
  QString pngName, ldrName, monoOutPngBaseName;
#ifdef QT_DEBUG_MODE
  monoOutPngBaseName =   QString("mono_%1").arg(QFileInfo(subModel).completeBaseName());
#else
  monoOutPngBaseName = "mono";
#endif
  QStringList csiKeys, ldrNames;
  if (renderer->useLDViewSCall()) {
      csiKeys << "mono";
      ldrNames << monoOutName;
      ldrName = ldrNames.first();
      pngName = QDir::currentPath() + "/" + Paths::assemDir + "/" + monoOutPngBaseName + ".png";
      ok[0] = (renderer->rotateParts(addLine,meta.rotStep,csiParts,ldrName,modelName,meta.LPub.assem.cameraAngles) == 0);
      ok[1] = (renderer->renderCsi(addLine,ldrNames,csiKeys,pngName,meta) == 0);
  } else {
      ok[0] = true;
      pngName = QDir::currentPath() + "/" + Paths::tmpDir + "/" + monoOutPngBaseName + ".png";
      if (Preferences::usingNativeRenderer){
         ldrName = QDir::currentPath() + "/" + Paths::tmpDir + "/csi.ldr";
         ok[0] = (renderer->rotateParts(addLine,meta.rotStep,csiParts,ldrName,modelName,meta.LPub.assem.cameraAngles) == 0);
      }
      ok[1] = (renderer->renderCsi(addLine,csiParts,csiKeys,pngName,meta) == 0);
  }

  if (ok[0] && ok[1]) {
    QPixmap pixmap;

    pixmap.load(pngName);

    QImage color = pixmap.toImage();

    QImage alpha = pixmap.toImage();

    int width = color.width();
    int height = color.height();
    int left = 32000, top = -1, right = -1, bottom = -1;

    for (int y = 0; y < height; y++) {
      for (int x = 0; x < width; x++)  {
        QColor a = alpha.pixel(x,y);

        if (a.blue()) {
          QColor c = color.pixel(x,y);

          int red = c.red(), green = c.green(), blue = c.blue();

          if (blue - (red + green)/2 > 64) {
            if (top == -1) {
              top = y;
            }
            if (left > x) {
              left = x;
            }
            if (bottom < y) {
              bottom = y;
            }
            if (right < x) {
              right = x;
            }
          }
        }
      }
    }

    left = (right+left)/2;
    top  = (top+bottom)/2;

    if (left > width || top > height) {
      left = width/2;
      top  = height/2;
    }

    emit gui->messageSig(LOG_DEBUG,QString("Called out submodel [%1] for parent model [%2] default pointer tip position:")
                                           .arg(subModel)
                                           .arg(modelName));
    emit gui->messageSig(LOG_DEBUG,QString(" -top:    %1").arg(QString::number(top)));
    emit gui->messageSig(LOG_DEBUG,QString(" -left:   %1").arg(QString::number(left)));
    emit gui->messageSig(LOG_DEBUG,QString(" -bottom: %1").arg(QString::number(bottom)));
    emit gui->messageSig(LOG_DEBUG,QString(" -right:  %1").arg(QString::number(right)));
    emit gui->messageSig(LOG_DEBUG,QString(" -width:  %1").arg(QString::number(width)));
    emit gui->messageSig(LOG_DEBUG,QString(" -height: %1").arg(QString::number(height)));
    QPointF offset = QPointF(qreal(left)/width, qreal(top)/height);
    emit gui->messageSig(LOG_DEBUG,QString(" -X (left[%1]/width[%2]): %3")
                                                                   .arg(QString::number(left))
                                                                   .arg(QString::number(width))
                                                                   .arg(QString::number(offset.x(),'f',6)));

    emit gui->messageSig(LOG_DEBUG,QString(" -Y (top[%1]/height[%2]): %3")
                                                                   .arg(QString::number(top))
                                                                   .arg(QString::number(height))
                                                                   .arg(QString::number(offset.y(),'f',6)));
    emit gui->messageSig(LOG_DEBUG,QString(" -Tip Point: (%1, %2)")
                                                 .arg(QString::number(offset.x(),'f',6))
                                                 .arg(QString::number(offset.y(),'f',6)));

    return QPointF(qreal(left)/width, qreal(top)/height);
  }
  emit gui->messageSig(LOG_ERROR,QString("Render momo image for pointer tip location failed."));
  return centerOffset;
}

void MetaItem::changeRotation(
  const Where &here)
{
  int numLines = gui->subFileSize(here.modelName);
  if (here.lineNumber < numLines) {
    QString line = gui->readLine(here);
    QStringList tokens;
    split(line,tokens);
    if (tokens.size() == 5 && tokens[0] == "0" && tokens[2] == "CALLOUT" && tokens[3] == "BEGIN") {
      if (tokens[4] == "ASSEMBLED") {
        tokens[4] = "ROTATED";
      } else {
        tokens[4] = "ASSEMBLED";
      }
    }
    line = tokens.join(" ");
    beginMacro("changeRotation");
    replaceMeta(here,line);
    endMacro();
  }
}

void MetaItem::hidePLIParts(
  QList<Where> &parts)
{
  QMap<QString,QString> map;

  beginMacro("hideParts");

  // Make a list of the file names and line numbers involved

  for (int i = 0; i < parts.size(); ++i) {
    QString modelName = parts[i].modelName;
    QString lineNumber = QString("%1").arg(parts[i].lineNumber);

    map[modelName] += lineNumber + ";";
  }

  // for each filename, sort the list of lineNumbers

  QStringList fileNames = map.keys();
  for (int i = 0; i < fileNames.size(); ++i) {
    QString fileName = fileNames[i];
    QString line = map.value(fileName);
    line.chop(1); // remove trailing ;
    QStringList lineNumStrings = line.split(";");

    QList<int> lineNums;
    for (int j = 0; j < lineNumStrings.size(); ++j) {
      lineNums << lineNumStrings[j].toInt();
    }
    qSort(lineNums.begin(),lineNums.end());

    // work it from last to first so as to not screw up our line numbers
    int lastLineNum = -1;
    for (int j = lineNums.size() - 1; j >= 0; --j) {
      int lineNum = lineNums[j];
      if (lineNum != lastLineNum) {
        Where here(fileName,lineNum);
        gui->appendLine(here,"0 !LPUB PLI END");
        gui->insertLine(here,"0 !LPUB PLI BEGIN IGN");
        lastLineNum = lineNum;
      }
    }
  }
  endMacro();
}

void MetaItem::substitutePLIPart(
        const QStringList  &_attributes,
        const QList<Where> &_instances,
        int                 _action,
        const QStringList  &_defaultList)
{
  QStringList attributes = _attributes;
  QList<Where> instances = _instances;

  bool ok;
  ok = SubstitutePartDialog::getSubstitutePart(attributes,gui,_action,_defaultList);

  if (ok) {
    QMap<QString,QString> map;

    beginMacro("substituteParts");

    // Make a list of the file names and line numbers involved

    for (int i = 0; i < instances.size(); ++i) {
      QString modelName = instances[i].modelName;
      QString lineNumber = QString("%1").arg(instances[i].lineNumber);

      map[modelName] += lineNumber + ";";
    }

    // for each filename, sort the list of lineNumbers

    QStringList fileNames = map.keys();
    for (int i = 0; i < fileNames.size(); ++i) {
      QString fileName = fileNames[i];
      QString line = map.value(fileName);
      line.chop(1); // remove trailing ;
      QStringList lineNumStrings = line.split(";");

      QList<int> lineNums;
      for (int j = 0; j < lineNumStrings.size(); ++j) {
        lineNums << lineNumStrings[j].toInt();
      }
      qSort(lineNums.begin(),lineNums.end());

      // work it from last to first so as to not screw up our line numbers

      int lastLineNum = -1;
      for (int j = lineNums.size() - 1; j >= 0; --j) {
        int lineNum = lineNums[j];
        if (lineNum != lastLineNum) {
          Where here(fileName,lineNum);
          if (_action == sRemove) {
            deleteMeta(here+4);
            deleteMeta(here+3);
            deleteMeta(here+1);
            deleteMeta(here);
          } else if (_action == sUpdate){
            replaceMeta(here,"0 !LPUB PLI BEGIN SUB " + attributes.join(" "));
          } else {
            appendMeta(here,"0 !LPUB PART END");
            appendMeta(here,"0 !LPUB PLI END");
            insertMeta(here,"0 !LPUB PART BEGIN IGN");
            insertMeta(here,"0 !LPUB PLI BEGIN SUB " + attributes.join(" "));
          }
          lastLineNum = lineNum;
        }
      }
    }
    endMacro();
  }
}

void MetaItem::removeLPubFormatting()
{
  beginMacro("RemoveLPubFormatting");
  QStringList fileList = gui->fileList();

  for (int i = 0; i < fileList.size(); ++i) {
    Where walk(fileList[i],0);
    int numLines = gui->subFileSize(fileList[i]);
    for (; walk.lineNumber < numLines; ) {
      QString line = gui->readLine(walk);
      QStringList argv;
      split(line,argv);
      if (argv.size() > 2 && argv[0] == "0" && (argv[1] == "LPUB" || argv[1] == "!LPUB")) {
        gui->deleteLine(walk);
        --numLines;
      } else {
        ++walk;
      }
    }
  }
  endMacro();
}

void MetaItem::setMetaAlt(const Where &itemTop, const QString metaString, bool newCommand)
{
    Where itemTopOf = itemTop;
    if (newCommand){
        if (itemTopOf.modelName == gui->topLevelFile())
            scanPastGlobal(itemTopOf);
        // place below item command unless end of file
        int eof = gui->subFileSize(itemTopOf.modelName);
        if (itemTopOf.lineNumber == eof)
            insertMeta(itemTopOf,metaString);
        else
            appendMeta(itemTopOf,metaString);
    } else {
        replaceMeta(itemTopOf,metaString);
    }
}

