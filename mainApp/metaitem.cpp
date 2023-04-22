
/****************************************************************************
**
** Copyright (C) 2007-2009 Kevin Clague. All rights reserved.
** Copyright (C) 2015 - 2023 Trevor SANDY. All rights reserved.
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
#include "paths.h"
#include "render.h"
#include "step.h"
#include "csiitem.h"
#include "commonmenus.h"
#include "messageboxresizable.h"

#include "backgrounddialog.h"
#include "bomoptionsdialog.h"
#include "borderdialog.h"
#include "dividerdialog.h"
#include "fadehighlightdialog.h"
#include "pairdialog.h"
#include "placementdialog.h"
#include "pliannotationdialog.h"
#include "pliconstraindialog.h"
#include "plisortdialog.h"
#include "pointerattribdialog.h"
#include "pointerplacementdialog.h"
#include "preferredrendererdialog.h"
#include "rotateiconsizedialog.h"
#include "rotstepdialog.h"
#include "scaledialog.h"
#include "sizeandorientationdialog.h"
#include "submodelcolordialog.h"
#include "substitutepartdialog.h"
#include "texteditdialog.h"

enum AppendType { AppendNoOption, AppendAtModel, AppendAtPage, AppendAtSubmodel };
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

  /* Now we want to scan backward to see if there is another
   * submodel just like this one then we scan the lines
   * following this line, to see if there is another submodel
   * just like this one that needs to be added as multiplier.
   *
   * In either direction, we need to stop on STEP/ROTSTEP.
   * Models that are callouts are not counted as instances and
   * Build Modification content are also excluded from the instance count
   */

  int instanceCount = 0;

  QString firstLine;
  Where lastInstance, firstInstance;

  int buildModLevel = 0;
  bool ignorePartLine = false;
  Where walkBack = step;
  for (; walkBack.lineNumber >= 0; walkBack--) {
    QString line = lpub->ldrawFile.readLine(walkBack.modelName,walkBack.lineNumber);

    if (isHeader(line)) {
      break;
    } else {
      QStringList argv;
      split(line,argv);
      if (argv.size() >= 2 && argv[0] == "0") {
        if (argv[1] == "STEP" || argv[1] == "ROTSTEP") {
          break;
        }
        // part substitutions are not counted
        if ((argv.size() == 5 && argv[0] == "0" &&
                (argv[1] == "!LPUB" || argv[1] == "LPUB") &&
                (argv[2] == "PART"  || argv[2] == "PLI") &&
                 argv[3] == "BEGIN"  &&
                 argv[4] == "IGN") ||
                (argv.size() == 3 && argv[0] == "0" &&
                 argv[1] == "MLCAD" &&
                 argv[2] == "SKIP_BEGIN"))
            ignorePartLine = true;
        if ((argv.size() == 4 && argv[0] == "0" &&
                (argv[1] == "!LPUB" || argv[1] == "LPUB") &&
                (argv[2] == "PART" || argv[2] == "PLI") &&
                 argv[3] == "END") ||
                (argv.size() == 3 && argv[0] == "0" &&
                 argv[1] == "MLCAD" &&
                 argv[2] == "SKIP_END"))
            ignorePartLine = false;
        // Sorry, models that are callouts are not counted as instances
        if (argv.size() == 4 && argv[0] == "0" &&
           (argv[1] == "!LPUB" || argv[1] == "LPUB") &&
            argv[2] == "CALLOUT" && argv[3] == "END") {
          //process callout content
          for (--walkBack; walkBack.lineNumber >= 0; walkBack--) {
            QString calloutLine = lpub->ldrawFile.readLine(walkBack.modelName,walkBack.lineNumber);
            QStringList tokens;
            split(calloutLine,tokens);
            if (tokens.size() == 15 && tokens[0] == "1") {
              ignorePartLine = true;
            } else
            if (tokens.size() == 4 && tokens[0] == "0" &&
               (tokens[1] == "!LPUB" || tokens[1] == "LPUB") &&
                tokens[2] == "CALLOUT" && tokens[3] == "BEGIN") {
              ignorePartLine = false;
              break;
            }
          }
        } // callout submodel
        // build modification content are not counted
        else if (argv.size() >= 4 && argv[0] == "0" &&
           (argv[1] == "!LPUB" || argv[1] == "LPUB") &&
            argv[2] == "BUILD_MOD") {
          if (argv[3] == "BEGIN") {
            buildModLevel = getLevel(argv[4],BM_BEGIN);
          } else if (argv[3] == "END_MOD") {
            buildModLevel = getLevel(QString(), BM_END);
          }
          ignorePartLine = buildModLevel;
        } // build modification
      } else if (argv.size() == 15 && argv[0] == "1") {
        if (ignorePartLine)
          continue;
        if (lpub->ldrawFile.isSubmodel(argv[14])) {
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
  numLines = lpub->ldrawFile.size(walk.modelName);
  for ( ; walk.lineNumber < numLines; walk++) {
    QString line = lpub->ldrawFile.readLine(walk.modelName,walk.lineNumber);
    QStringList argv;
    split(line,argv);
    if (argv.size() >= 2 && argv[0] == "0") {
      if (argv[1] == "STEP" || argv[1] == "ROTSTEP") {
        break;
      }
      // part substitutions are not counted
      if ((argv.size() == 5 && argv[0] == "0" &&
              (argv[1] == "!LPUB" || argv[1] == "LPUB") &&
              (argv[2] == "PART"  || argv[2] == "PLI") &&
               argv[3] == "BEGIN"  &&
               argv[4] == "IGN") ||
              (argv.size() == 3 && argv[0] == "0" &&
               argv[1] == "MLCAD" &&
               argv[2] == "SKIP_BEGIN"))
          ignorePartLine = true;
      if ((argv.size() == 4 && argv[0] == "0" &&
              (argv[1] == "!LPUB" || argv[1] == "LPUB") &&
              (argv[2] == "PART" || argv[2] == "PLI") &&
               argv[3] == "END") ||
              (argv.size() == 3 && argv[0] == "0" &&
               argv[1] == "MLCAD" &&
               argv[2] == "SKIP_END"))
          ignorePartLine = false;
      // models that are callouts are not counted as instances
      if (argv.size() == 4 && argv[0] == "0" &&
         (argv[1] == "!LPUB" || argv[1] == "LPUB") &&
          argv[2] == "CALLOUT" && argv[3] == "BEGIN") {
        //process callout content
        for (++walk; walk.lineNumber < numLines; walk++) {
          QString calloutLine = lpub->ldrawFile.readLine(walk.modelName,walk.lineNumber);
          QStringList tokens;
          split(calloutLine,tokens);
          if (tokens.size() == 15 && tokens[0] == "1") {
            ignorePartLine = true;
          } else
          if (tokens.size() == 4 && tokens[0] == "0" &&
             (tokens[1] == "!LPUB" || tokens[1] == "LPUB") &&
              tokens[2] == "CALLOUT" && tokens[3] == "END") {
            ignorePartLine = false;
            break;
          }
        }
      } // callout
      // build modification content are not counted
      else if (argv.size() >= 4 && argv[0] == "0" &&
         (argv[1] == "!LPUB" || argv[1] == "LPUB") &&
          argv[2] == "BUILD_MOD") {
        if (argv[3] == "BEGIN") {
          buildModLevel = getLevel(argv[4],BM_BEGIN);
        } else if (argv[3] == "END_MOD") {
          buildModLevel = getLevel(QString(), BM_END);
        }
        ignorePartLine = buildModLevel;
      } // build modification
    } else if (argv.size() == 15 && argv[0] == "1") {
      if (lpub->ldrawFile.isSubmodel(argv[14])) {
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

int MetaItem::countInstancesInModel(Meta *meta, const QString &modelName) {

  if (lpub->ldrawFile.isDisplayModel(modelName))
    return 0;

  int   numLines;
  Where walk(modelName,1);

  /* submodelStack tells us where this submodel is referenced in the
   parent file so we use it to target the correct SUBMODEL*/

  SubmodelStack tos = meta->submodelStack[meta->submodelStack.size() - 1];
  Where subModel(tos.modelName,0);
  lpub->ldrawFile.skipHeader(subModel.modelName,subModel.lineNumber);

    /* Scan the lines following this line, to see if there is another
   * submodel just like this one that needs to be added as multiplier.
   *
   * Models that are callouts are not counted as instances and
   * Build Modification content are also excluded from the instance count
   */

  int instanceCount = 0;

  QString firstLine;
  Where lastInstance, firstInstance;

  int buildModLevel = 0;
  bool ignorePartLine = false;
  walk = subModel;

  numLines = lpub->ldrawFile.size(walk.modelName);
  for ( ; walk.lineNumber < numLines; walk++) {
    QString line = lpub->ldrawFile.readLine(walk.modelName,walk.lineNumber);
    QStringList argv;
    split(line,argv);
    if (argv.size() >= 2 && argv[0] == "0") {
      // part substitutions are not counted
      if ((argv.size() == 5 && argv[0] == "0" &&
              (argv[1] == "!LPUB" || argv[1] == "LPUB") &&
              (argv[2] == "PART"  || argv[2] == "PLI") &&
               argv[3] == "BEGIN"  &&
               argv[4] == "IGN") ||
              (argv.size() == 3 && argv[0] == "0" &&
               argv[1] == "MLCAD" &&
               argv[2] == "SKIP_BEGIN"))
          ignorePartLine = true;
      if ((argv.size() == 4 && argv[0] == "0" &&
              (argv[1] == "!LPUB" || argv[1] == "LPUB") &&
              (argv[2] == "PART" || argv[2] == "PLI") &&
               argv[3] == "END") ||
              (argv.size() == 3 && argv[0] == "0" &&
               argv[1] == "MLCAD" &&
               argv[2] == "SKIP_END"))
          ignorePartLine = false;
      // models that are callouts are not counted as instances
      if (argv.size() == 4 && argv[0] == "0" &&
         (argv[1] == "!LPUB" || argv[1] == "LPUB") &&
          argv[2] == "CALLOUT" && argv[3] == "BEGIN") {
        //process callout content
        for (++walk; walk.lineNumber < numLines; walk++) {
          QString calloutLine = lpub->ldrawFile.readLine(walk.modelName,walk.lineNumber);
          QStringList tokens;
          split(calloutLine,tokens);
          if (tokens.size() == 15 && tokens[0] == "1") {
            ignorePartLine = true;
          } else
          if (tokens.size() == 4 && tokens[0] == "0" &&
             (tokens[1] == "!LPUB" || tokens[1] == "LPUB") &&
              tokens[2] == "CALLOUT" && tokens[3] == "END") {
            ignorePartLine = false;
            break;
          }
        }
      } // callout
      // build modification content are not counted
      else if (argv.size() >= 4 && argv[0] == "0" &&
         (argv[1] == "!LPUB" || argv[1] == "LPUB") &&
          argv[2] == "BUILD_MOD") {
          if (argv[3] == "BEGIN") {
              buildModLevel = getLevel(argv[4],BM_BEGIN);
          } else if ((argv[3] == "END_MOD")) {
              buildModLevel = getLevel(QString(), BM_END);
          }
          ignorePartLine = buildModLevel;
      } // build modification end
    } else if (argv.size() == 15 && argv[0] == "1") {
      if (lpub->ldrawFile.isSubmodel(argv[14])) {
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
    Rc rc1 = OkRc;
    Where startTopOfSteps = topOfSteps;
    Where nextStep = bottomOfSteps;
    bool firstChange = true;
    bool amendGroup = false;

    for (int stepNum = 1; (stepNum <= numOfSteps) && (numOfSteps > 0); stepNum++) {
        bool lastStep = stepNum == numOfSteps;
        bool partsAdded = false;

        // after first pass, nextStep may be on '0 STEP',
        // so increment to allow scanForward
        Where walk = nextStep+1;
        Where finalTopOfSteps = startTopOfSteps;

        bool onStepMeta = lpub->ldrawFile.readLine(walk.modelName,walk.lineNumber) == "0 STEP";
        if (! onStepMeta)
            rc1 = scanForward(walk,StepMask|StepGroupMask,partsAdded);

        // encountered end of current step group
        Where end;
        if (rc1 == StepGroupEndRc) {        // END
            amendGroup = true;
            end = walk++;                   // encountered current multi step end meta
            rc1 = scanForward(walk,StepMask|StepGroupMask,partsAdded);
        }

        // encountered beginning of next step group
        if (rc1 == StepGroupBeginRc) {      // BEGIN
            firstChange = false;
            beginMacro("addNextStep1");
            removeFirstStep(bottomOfSteps); // remove BEGIN
            partsAdded = false;
            rc1 = scanForward(walk,StepMask|StepGroupMask,partsAdded);
        }

        if (firstChange) {
            beginMacro("addNextStep2");
            firstChange = false;
        }

        // check if encountered last step in model
        if (rc1 == EndOfFileRc && partsAdded) {
            lastStep = true;
            insertMeta(walk,step);
        }

        // increment to next step in number of steps to append
        nextStep =  walk;

        // if current multi step end meta encountered, delete it
        if (end.lineNumber) {
            deleteMeta(end);
        } else {
            walk = topOfSteps;
            rc1 = scanForward(walk,StepMask|StepGroupMask);
            // encountered 0 STEP top of step (after headers for first step)
            if (rc1 == StepRc) {
                walk = topOfSteps + 1;
                rc1 = scanForward(walk,StepMask|StepGroupMask);
            }
            // encountered 0 !LPUB MULTI_STEP BEGIN at top of step
            if (rc1 == StepGroupBeginRc) {
                            finalTopOfSteps = walk+1;
            }
            // encountered 0 !LPUB MULTI_STEP END at top of step
            else if (rc1 == StepGroupEndRc) {
                finalTopOfSteps = walk+1;
                if (lastStep)
                    appendMeta(walk,stepGroupBegin);
            }
            // did not encounter STEP or MULTI_STEP at top of step
            else  {
                walk = topOfSteps;
                scanPastGlobal(walk);
                finalTopOfSteps = walk+1;
                if (lastStep)
                    appendMeta(walk,stepGroupBegin);
            }
        }

        // reset to top of step group
        startTopOfSteps = finalTopOfSteps;

        // encountered last step in model = set step group end meta
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
    removeFirstStep(bottomOfSteps);                       // remove BEGIN
    partsAdded = false;
    rc1 = scanForwardStepGroup(walk,partsAdded);
  }

  // bottomOfSteps  - STEP
  // end            - StepGroupEnd
  // walk           - (STEP || EOF)

  if (firstChange) {
    beginMacro("addNextStep2");
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
      QString line = lpub->ldrawFile.readLine(topOfPage.modelName,topOfPage.lineNumber);
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
      QString line = lpub->ldrawFile.readLine(bottomOfPage.modelName,bottomOfPage.lineNumber);
      Rc rc = meta.parse(line,bottomOfPage);
      if (rc == StepRc || rc == RotStepRc) {
        break;
      }
    }

    Where walk = bottomOfSteps;

    // copy page to after step group

    for (Where walk2 = topOfPage; walk2 <= bottomOfPage.lineNumber; walk2++) {
      QString line = lpub->ldrawFile.readLine(walk2.modelName,walk2.lineNumber);
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

//  Gui::displayPageNum--;
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
      QString line = lpub->ldrawFile.readLine(topOfPage.modelName,topOfPage.lineNumber);
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
      QString line = lpub->ldrawFile.readLine(bottomOfPage.modelName,bottomOfPage.lineNumber);
      Rc rc = meta.parse(line,bottomOfPage);
      if (rc == StepRc || rc == RotStepRc) {
        break;
      }
    }

    Where walk = bottomOfSteps;

    // copy page to before step group

    for (Where walk2 = topOfPage; walk2 <= bottomOfPage.lineNumber; walk2.lineNumber += 2) {
      QString line = lpub->ldrawFile.readLine(walk2.modelName,walk2.lineNumber);
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
  Gui::maxPages = -1;

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
  Gui::maxPages = -1;

  SubmodelStack tos = meta->submodelStack[meta->submodelStack.size() - 1];
  Where calledOut(tos.modelName,tos.lineNumber);
  Where here = calledOut+1;
  QString line = lpub->ldrawFile.readLine(calledOut.modelName,calledOut.lineNumber);
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
//#ifdef QT_DEBUG_MODE
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
//#endif
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
            QString line = lpub->ldrawFile.readLine(topOf.modelName,topOf.lineNumber);
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

//#ifdef QT_DEBUG_MODE
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
//#endif

  if (metaInRange) {
    QString line = meta->format(meta->pushed,meta->global);
    replaceMeta(meta->here(),line);
  } else {
    if (askLocal) {
      local = LocalDialog::getLocal(VER_PRODUCTNAME_STR, "Change only this step?",nullptr); // changed from gui error(QLayout: Attempting to add QLayout "" to Gui "", which already has a layout)
    }
    QString line = meta->format(local, global);

    int  numLines = lpub->ldrawFile.size(topOf.modelName);
    bool eof = bottomOf.lineNumber == numLines;

    if (eof) {
      bottomOf -1;       //fix: numLines is inclusive (starts from 1) while readline index is exclusive (i.e. starts from 0)
      QString tline = lpub->ldrawFile.readLine(bottomOf.modelName,bottomOf.lineNumber);
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
  bool                   useLocal)
{
  Q_UNUSED(bottomOf)
  Q_UNUSED(useTop)
  Q_UNUSED(append)
  Q_UNUSED(local)
  Q_UNUSED(useLocal)

  PlacementData placementData = placement->value();
  bool ok = PlacementDialog::getPlacement(parentType,relativeType,placementData,title);
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
  bool           useLocal)
{
  PlacementData placementData = placement->value();
  bool ok = PlacementDialog::getPlacement(parentType,relativeType,placementData,title);
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

//#ifdef QT_DEBUG_MODE
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
//#endif

  QString newMetaString = placement->format(local,global);

  if (placement->here().modelName == "undefined") {

    Where walk;
    bool partsAdded;
    int eof = lpub->ldrawFile.size(defaultWhere.modelName);
    defaultWhere+1 == eof ? walk = defaultWhere : walk = defaultWhere+1;

    // first, lets start from the bottom of the current step - to catch added parts
    scanForward(walk,StepMask);

    // now lets walk backward to see if we hit the end (top) of file.
    if (scanBackward(walk,StepMask,partsAdded) == EndOfFileRc) {
      defaultWhere = firstLine(defaultWhere.modelName);

//#ifdef QT_DEBUG_MODE
//      logNotice() << " \nScanBackward[TOP]: EndOfFileRc (StepMask) - defaultLine is: "
//                  << firstLine(defaultWhere.modelName).lineNumber
//                  << " of model: "
//                  << defaultWhere.modelName
//                     ;
//#endif

    }

    if (type == StepGroupType) {
      scanForward(defaultWhere,StepGroupBeginMask);

//#ifdef QT_DEBUG_MODE
//      logNotice() << " \nScanForward[BOTTOM]: StepGroupType (StepGroupBeginMask) - file name is: "
//                  << defaultWhere.modelName
//                  << " \nStop at line: "
//                  << defaultWhere.lineNumber
//                  << " with line contents: \n"
//                  << lpub->ldrawFile.readLine(defaultWhere.modelName,defaultWhere.lineNumber)
//                     ;
//#endif

    } else if (type == CalloutType) {
      scanForward(defaultWhere,CalloutEndMask);
      --defaultWhere;

//#ifdef QT_DEBUG_MODE
//      logNotice() << " \nScanForward[BOTTOM]: CalloutType (CalloutEndMask) - file name is: "
//                  << defaultWhere.modelName
//                  << " \nStop at line: "
//                  << defaultWhere.lineNumber
//                  << " with line contents: \n"
//                  << lpub->ldrawFile.readLine(defaultWhere.modelName,defaultWhere.lineNumber)
//                     ;
//#endif

    } else if (defaultWhere.modelName == lpub->ldrawFile.topLevelFile()) {
      scanPastGlobal(defaultWhere);

//#ifdef QT_DEBUG_MODE
//      logNotice() << " \nTopLevelFile[TOP]: ScanPastGlobal - file name is: "
//                  << defaultWhere.modelName
//                  << " \nStop at line: "
//                  << defaultWhere.lineNumber
//                  << " with line contents: \n"
//                  << lpub->ldrawFile.readLine(defaultWhere.modelName,defaultWhere.lineNumber)
//                     ;
//#endif

    }

    if (defaultWhere.lineNumber == eof){
        insertMeta(defaultWhere,newMetaString);

//#ifdef QT_DEBUG_MODE
//        logNotice() << " \nLast line so insert Meta:  \n" << newMetaString << " \nat line: "
//                    << defaultWhere.lineNumber
//                       ;
//#endif

    } else {
        appendMeta(defaultWhere,newMetaString);

//#ifdef QT_DEBUG_MODE
//        logNotice() << " \nNot last line so append Meta: \n" << newMetaString << " \nat line: "
//                    << defaultWhere.lineNumber+1
//                       ;
//#endif

    }

  } else {
    replaceMeta(placement->here(),newMetaString);

//#ifdef QT_DEBUG_MODE
//    logNotice() << " \nPlacement defined so replace Meta:  \n" << newMetaString << " \nat line: "
//                << defaultWhere.lineNumber
//                   ;
//#endif

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
    int endOfFile = lpub->ldrawFile.size(here.modelName);

    if (!(canReplace = groupMeta->here() == here)) {
        if (groupMeta->bom() && !groupMeta->bomPart()) {
            here.lineNumber = nextLine;
        } else {
            Rc rc;
            Meta mi;
            groupMeta->setWhere(here);
            metaString = lpub->ldrawFile.readLine(here.modelName,here.lineNumber);
            rc = mi.parse(metaString,here);
            if (!(canReplace = rc == PliPartGroupRc || rc == BomPartGroupRc)) {
                here.lineNumber = nextLine < endOfFile ? nextLine : endOfFile - 1;
                groupMeta->setWhere(here);
            }
        }
    }

//#ifdef QT_DEBUG_MODE
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
//#endif

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
  PageSizeMeta        *metaS,
  PageOrientationMeta *metaO,
  bool                 useTop,
  int                  append,
  bool                 local)
{

  PageSizeData dataS = metaS->value();
  OrientationEnc dataO = metaO->value();

  bool ok;
  ok = SizeAndOrientationDialog::getSizeAndOrientation(dataS,dataO,title,gui);

  if (ok) {
    if (dataO != metaO->value()) {
//#ifdef QT_DEBUG_MODE
//      logDebug() << " SIZE (dialog return): Orientation: "
//                 << (dataO == Portrait ? "Portrait" : "Landscape");
//#endif
      metaO->setValue(dataO);
      setMeta(topOfStep,bottomOfStep,metaO,useTop,append,local);
    }

    if (dataS.sizeW != metaS->value(0) || dataS.sizeID != metaS->valueSizeID() || dataS.sizeH != metaS->value(1)) {
//#ifdef QT_DEBUG_MODE
//      logDebug() << " SIZE (dialog return): Width: "
//                 << metaS->value(0) << " Height: "
//                 << metaS->value(1) << " SizeID: "
//                 << metaS->valueSizeID();
//#endif
      metaS->setValue(dataS);
      setMeta(topOfStep,bottomOfStep,metaS,useTop,append,local);
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

void MetaItem::setRendererArguments(const Where &top,
        const Where &bottom,
        const QString &rendererLabel,
        StringMeta *rendererArguments,
        bool  useTop,
        int   append,
        bool  local)
{
    QString arguments = rendererArguments->value();

    bool ok = TextEditDialog::getText(arguments,rendererLabel/*QString("%1 Renderer Arguments").arg(renderer)*/);

    if (ok && !arguments.isEmpty()) {

        QStringList list = arguments.split("\n");

        QStringList list2;
        Q_FOREACH (QString string, list){
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
  QString           title,
  const Where      &topOfStep,
  const Where      &bottomOfStep,
  CameraAnglesMeta *meta,
  int               append,
  bool              local)
{
  CameraAnglesData data = meta->value();

  bool ok = CameraAnglesDialog::getCameraAngles(
              title,
              data,
              gui);

  if (ok) {
    meta->setValue(data);
    setMetaTopOf(topOfStep,bottomOfStep,meta,append,local);
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
  bool         local,
  DoubleSpinEnc spinGui)
{
  float data = floatMeta->value();
  bool ok = DoubleSpinDialog::getFloat(
                                  title,
                                  label,
                                  floatMeta,
                                  data,
                                  step, // spin single step
                                  spinGui,
                                  gui);
  if (ok) {
    floatMeta->setValue(data);
    setMetaTopOf(topOfStep,bottomOfStep,floatMeta,append,local);
  }
}

void MetaItem::changeCameraFOV(
  QString      title,
  QString      label,
  const Where &topOfStep,
  const Where &bottomOfStep,
  FloatMeta   *fovMeta,
  FloatMeta   *zNearMeta,
  FloatMeta   *zFarMeta,
  int          append,
  bool         local)
{
  float fovData   = fovMeta->value();
  float zNearData = zNearMeta->value();
  float zFarData  = zFarMeta->value();

  bool ok = CameraFOVDialog::getCameraFOV(
                                  title,
                                  label,
                                  fovMeta,
                                  zNearMeta,
                                  zFarMeta,
                                  fovData,
                                  zNearData,
                                  zFarData,
                                  gui);

  auto notEqual = [] (const double v1, const double v2, int p)
  {
    const QString _v1 = QString::number(v1,'f',p);
    const QString _v2 = QString::number(v2,'f',p);
    const bool     r  = _v1 != _v2;
    return r;
  };

  if (ok) {
    if (notEqual(fovMeta->value(), fovData, 1)) {
      fovMeta->setValue(fovData);
      setMetaTopOf(topOfStep,bottomOfStep,fovMeta,append,local);
    }
    if (Preferences::preferredRenderer == RENDERER_NATIVE) {
      if (notEqual(zNearMeta->value(), zNearData, 1)) {
        zNearMeta->setValue(zNearData);
        setMetaTopOf(topOfStep,bottomOfStep,zNearMeta,append,local);
      }
      if (notEqual(zFarMeta->value(), zFarData, 1)) {
        zFarMeta->setValue(zFarData);
        setMetaTopOf(topOfStep,bottomOfStep,zFarMeta,append,local);
      }
    }
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
  QString filePath = QFileDialog::getOpenFileName(nullptr,title, cwd, filter);
 if (!filePath.isEmpty()) {
    if (filePath.startsWith(cwd))
       filePath = filePath.replace(cwd,".");
    image->setValue(filePath);
    setMeta(topOfStep,bottomOfStep,image,useTop,append,allowLocal,checkLocal);
  }
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
      int numLines = lpub->ldrawFile.size(sepMeta.modelName);
      scanPastGlobal(sepMeta);
      for ( ; sepMeta < numLines; sepMeta++) {
          QString line = lpub->ldrawFile.readLine(sepMeta.modelName,sepMeta.lineNumber);
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
  // before first step page
  return Gui::displayPageNum <= Gui::firstStepPageNum;
}

bool MetaItem::okToAppendCoverPage()
{
  // after last step page
  return Gui::displayPageNum >= Gui::lastStepPageNum;
}

void MetaItem::insertCoverPage()
{
  Rc rc;
  QString line;
  Meta content;
  Where here(lpub->ldrawFile.topLevelFile(),0);
  QString meta = "0 !LPUB INSERT COVER_PAGE FRONT";
  int numLines = lpub->ldrawFile.size(here.modelName);

  scanPastGlobal(here);                         //scan past headers and global

  beginMacro("InsertCoverPage");
  for ( ; here < numLines; here++) {            //scan forward
      line = lpub->ldrawFile.readLine(here.modelName,here.lineNumber);
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
  QRegExp rx("^0 !?LPUB INSERT COVER_PAGE(?: FRONT)?$");
  Where here(lpub->ldrawFile.topLevelFile(),0);
  return Gui::stepContains(here, rx);
}

void MetaItem::appendCoverPage()
{
  Rc rc;
  QString line;
  Meta content;
  Where here(lpub->ldrawFile.topLevelFile(),0);
  here.lineNumber = lpub->ldrawFile.size(here.modelName); //start at bottom of file
  QString meta    = "0 !LPUB INSERT COVER_PAGE BACK";
  here--;

  beginMacro("AppendCoverPage");
  line = lpub->ldrawFile.readLine(here.modelName,here.lineNumber);
  rc   = content.parse(line,here);

   if ((rc == StepGroupEndRc ||                       //STEP so advance line before 'insert' meta
        rc == StepRc)) {
      here++;
      insertMeta(here,meta);
   } else {
      appendMeta(here,step);                          //NO STEP, 'append' one before appending meta
      here++;
      appendMeta(here,meta);
   }
   endMacro();
}

bool MetaItem::backCoverPageExist()
{
  QRegExp rx("^0 !?LPUB INSERT COVER_PAGE(?: BACK)?$");
  Where here(lpub->ldrawFile.topLevelFile(), lpub->ldrawFile.size(lpub->ldrawFile.topLevelFile())); //start at bottom of file
  if (here.lineNumber)
      scanBackward(here, StepMask | StepGroupMask);
  return Gui::stepContains(here, rx);
}

bool MetaItem::okToInsertNumberedPage()
{
  const bool afterOrAtFirstStepPage = Gui::displayPageNum >= Gui::firstStepPageNum;
  const bool beforeOrAtLastStepPage = Gui::displayPageNum <= Gui::lastStepPageNum;
  return afterOrAtFirstStepPage || beforeOrAtLastStepPage;
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
  Where dummy;
  appendPage(meta, dummy);
}

void MetaItem::insertPage(QString &meta)
{
  Where topOfStep = Gui::topOfPages[Gui::displayPageNum-1];

  scanPastGlobal(topOfStep);

  QString line = lpub->ldrawFile.readLine(topOfStep.modelName,topOfStep.lineNumber + 1); // appended line
  QStringList argv;
  split(line,argv);
  bool skipStepMeta = (argv.size() >= 2 &&
                       argv[0] == "0"   &&
                       (argv[1] == "STEP" ||
                        argv[1] == "ROTSTEP"));

  beginMacro("InsertPage");
  if (!skipStepMeta)
      appendMeta(topOfStep,"0 STEP");
  appendMeta(topOfStep,meta);
  endMacro();
}

bool MetaItem::appendPage(QString &metaCommand, Where &where, int option)
{
  Rc rc;
  QString line;
  Meta mi;
  Where bottomOfStep;
  bool appendStepMeta = false;
  bool insertStepMeta = false;
  bool askAfterCoverPage = option != AppendAtModel;

  if (option) {
      bottomOfStep    = where;                               //stat at the specified bottom of page step
  } else {
      bottomOfStep    = Gui::topOfPages[Gui::displayPageNum];//start at the bottom of the page's last step
      QString title   = QString("%1 - Append Page").arg(VER_PRODUCTNAME_STR);
      QString options = QString("At end of main model ?|At current page ?%1")
                                .arg(where.modelIndex ? QString("|At end of current model ?") : QString());
      if ((option = OptionDialog::getOption(title, options,nullptr)) == AppendNoOption)
          return false;

      if (option == AppendAtSubmodel)                        //start at the bottom of the model's last step
         bottomOfStep = Where(bottomOfStep.modelName, lpub->ldrawFile.size(bottomOfStep.modelName));
      else if (option == AppendAtModel)                      //start at the bottom of the main model's last step
         bottomOfStep = Where(lpub->ldrawFile.topLevelFile(), lpub->ldrawFile.size(lpub->ldrawFile.topLevelFile()));
  }

  Where here = bottomOfStep;

  int numLines = lpub->ldrawFile.size(here.modelName);

  for ( ; here > 0; --here) {                                //scan from bottom to top of file
    line = lpub->ldrawFile.readLine(here.modelName,here.lineNumber);
    rc = mi.parse(line,here);
    if (rc == StepRc || rc == RotStepRc ||
      rc == StepGroupEndRc || rc == CalloutEndRc) {          //we are on a boundary command so advance one line and break
      here++;
      break;
    }

    QStringList tokens;
    split(line,tokens);
    bool token_1_5 = tokens.size() &&                        //no boundary command so check for valid step content
         tokens[0].size() == 1 &&
         tokens[0] >= "1" && tokens[0] <= "5";
    // type 1 - 5 line
    if (token_1_5 || isHeader(line)) {
      appendStepMeta = true;
      break;
    }
    // cover page
    else if (tokens.size() >= 4 &&
               tokens[2] == "INSERT" &&
               tokens[3] == "COVER_PAGE") {
      bool afterCoverPage = false;
      if (askAfterCoverPage) {
        afterCoverPage = LocalDialog::getLocal(VER_PRODUCTNAME_STR, "Append after cover page ?", nullptr);
      }
      if (afterCoverPage) {
        appendStepMeta = true;
      } else {                                               //insert before cover page
        for ( ; here > 0; --here) {
          line = lpub->ldrawFile.readLine(here.modelName,here.lineNumber);
          rc = mi.parse(line,here);
          if (rc == StepRc || rc == RotStepRc )
            break;
        }
        bottomOfStep = here;
        insertStepMeta = true;
      }
      break;
    }
    // final model
    else if (tokens.size() >= 4 &&
             tokens[2] == "INSERT" &&
            (tokens[3] == "PAGE" ||
             tokens[3] == "MODEL")) {
        appendStepMeta = true;
    }
  }

  if ( ! appendStepMeta && ! insertStepMeta) {
    for ( ; here < numLines; ++here) {
      QString line = lpub->ldrawFile.readLine(here.modelName,here.lineNumber);
      QStringList tokens;
      split(line,tokens);
      bool token_1_5 = tokens.size() &&
                       tokens[0].size() == 1 &&
                       tokens[0] >= "1" &&
                       tokens[0] <= "5";                     //non-zero line detected
      if (token_1_5) {
        appendStepMeta = true;
        break;
      }
    }
  }

  beginMacro("appendPage");
  if (appendStepMeta) {
    appendMeta(bottomOfStep,"0 STEP");
    bottomOfStep++;
  } else if (insertStepMeta) {
    insertMeta(bottomOfStep, "0 STEP");
  }
  appendMeta(bottomOfStep,metaCommand);
  bottomOfStep++;
  endMacro();

  where = bottomOfStep;

  return appendStepMeta || insertStepMeta;
}

void MetaItem::deletePage()
{
  Where topOfPage    = Gui::topOfPages[Gui::displayPageNum-1];
  Where bottomOfPage = Gui::topOfPages[Gui::displayPageNum];
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
  QString filePath = QFileDialog::getOpenFileName(nullptr,title, cwd, filter);

  if (!filePath.isEmpty()) {
    if (filePath.startsWith(cwd))
        filePath = filePath.replace(cwd,".");
    QString meta = QString("0 !LPUB INSERT PICTURE \"%1\" OFFSET 0.5 0.5") .arg(filePath);
    Where insertPosition;
    bool multiStep = false;
    if (lpub->currentStep) {
        if ((multiStep = lpub->currentStep->multiStep))
            insertPosition = lpub->currentStep->bottomOfSteps() - 1;
        else
            insertPosition = lpub->currentStep->topOfStep();
    } else {
        insertPosition = lpub->page.topOfSteps();
    }
    if (insertPosition.modelName == lpub->ldrawFile.topLevelFile() && insertPosition.lineNumber < 2)
        scanPastGlobal(insertPosition);
    appendMeta(insertPosition, meta);
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
    QString windowTitle = _isRichText ? QMessageBox::tr("Edit Rich Text") : QMessageBox::tr("Edit Plain Text");

    bool stepOk      = true;
    bool initialAdd  = true;
    bool stepFound   = true;
    QString text     = _text;
    QString placementStr;

    if (!text.isEmpty()){
      initialAdd      = false;
      QStringList pre = text.split("\\n");
      text = pre.join(" ");
    }

    int thisStep          = 1;
    Where insertPosition  = here;
    bool getStep          = _parentRelaiveType == StepGroupType;
    bool hasOffset        = _offsetX != 0.0f || _offsetY != 0.0f;
    QString offset        = hasOffset ? QString(" OFFSET %1 %2")
                                                .arg(qreal(_offsetX))
                                                .arg(qreal(_offsetY)) : QString();
    bool textPlacement    = lpub->page.meta.LPub.page.textPlacement.value();

    if (textPlacement && initialAdd) {
        bool placementOk = false;
        PlacementMeta placementMeta = lpub->page.meta.LPub.page.textPlacementMeta;
        placementMeta.preamble = QString("0 !LPUB INSERT %1 PLACEMENT ")
                .arg(_isRichText ? "RICH_TEXT" : "TEXT");
        PlacementData placementData = placementMeta.value();
        placementData.pageType = DefaultPage;

        placementOk = PlacementDialog
                ::getPlacement(PlacementType(_parentRelaiveType),TextType,placementData,QMessageBox::tr("Placement"));

        if (placementOk) {
            if (hasOffset) {
                placementData.offsets[XX] = _offsetX;
                placementData.offsets[YY] = _offsetY;
            }
            placementMeta.setValue(placementData);;
            placementStr = placementMeta.format(true/*local*/,false);

            getStep = _parentRelaiveType == StepGroupType &&
                    placementData.relativeTo != PageType &&
                    placementData.relativeTo != PageHeaderType &&
                    placementData.relativeTo != PageFooterType;
        }

        if (getStep) {
            // set step to insert
            Steps *steps = dynamic_cast<Steps *>(&lpub->page);
            if (steps){
                /* foreach range */
                stepFound = false;
                for (int i = 0; i < steps->list.size() && !stepFound; i++) {
                    if (steps->list[i]->relativeType == RangeType) {
                       Range *range = dynamic_cast<Range *>(steps->list[i]);
                       thisStep = QInputDialog::getInt(gui,QMessageBox::tr("Steps"),QMessageBox::tr("Which Step"),1,1,range->list.size(),1,&stepOk);
                       if (range && stepOk) {
                           /* foreach step*/
                           for (int j = 0; j < range->list.size(); j++) {
                              Step *step = dynamic_cast<Step *>(range->list[j]);
                              if (step && step->stepNumber.number == thisStep) {
                                  insertPosition = step->topOfStep();
                                  stepFound = true;
                                  break;
                              }
                           }
                       }
                    }
                }
            }
         }

        if (!placementOk || !stepOk ||!stepFound) {
            QPixmap _icon = QPixmap(":/icons/lpub96.png");
            QMessageBoxResizable box;
            box.setWindowIcon(QIcon());
            box.setIconPixmap (_icon);
            box.setTextFormat (Qt::RichText);

            box.setWindowTitle(QMessageBox::tr ("Text Placement Select"));
            box.setWindowFlags (Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
            box.setMinimumSize(40,20);

            QString message;
            if (!placementOk)
                message += QMessageBox::tr ("Placement selection was cancelled or not valid.<br>");
            if (!stepOk)
                message += QMessageBox::tr ("Specified step number %1 is not valid. Bottom of multi steps will be used.<br>").arg(thisStep);
            if (!stepFound)
                message += QMessageBox::tr ("Step number %1 was not found. Bottom of multi steps will be used.").arg(thisStep);
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

    stepOk = TextEditDialog::getText(text,_editFont,_editFontColor,_isRichText,windowTitle,true);
    if (stepOk && !text.isEmpty()) {

    QStringList list = text.split("\n");
    QStringList list2;
    Q_FOREACH (QString string, list){
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
    if (_isRichText)
      strMeta = QString("0 !LPUB INSERT RICH_TEXT \"%1\"%2")
                        .arg(list2.join("\\n")) .arg(textPlacement ? "" : offset);
    else
      strMeta = QString("0 !LPUB INSERT TEXT \"%1\" \"%2\" \"%3\"%4")
                        .arg(list2.join("\\n")) .arg(_editFont) .arg(_editFontColor) .arg(textPlacement ? "" : offset);

    beginMacro("UpdateText");
    if (append)
      appendMeta(insertPosition,strMeta);
    else
      replaceMeta(insertPosition,strMeta);
    if (textPlacement && initialAdd) {
      Where walkFwd = insertPosition + 1;
      appendMeta(walkFwd,placementStr);
    }
    endMacro();
  }
}

void MetaItem::insertText()
{
    Where insertPosition;

    bool append        = true;
    bool isRichText    = false;
    bool multiStep     = false;
    PlacementType parentRelativeType = PageType;

    if (lpub->currentStep) {
        if ((multiStep = lpub->currentStep->multiStep))
            insertPosition = lpub->currentStep->bottomOfSteps() - 1;
        else
            insertPosition = lpub->currentStep->topOfStep();
    } else {
        insertPosition = lpub->page.topOfSteps();
    }

    if (insertPosition.modelName == lpub->ldrawFile.topLevelFile() && insertPosition.lineNumber < 2)
        scanPastGlobal(insertPosition);

    QStringList textFormatOptions;
    textFormatOptions << QMessageBox::tr("Plain Text") << QMessageBox::tr("Rich Text");
    bool ok;
    QString textFormat =
            QInputDialog::getItem(gui,
                                  QMessageBox::tr("Add Text"),
                                  QMessageBox::tr("Text Format:"),
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

        QString message = QMessageBox::tr ("Text format selection was cancelled or not valid.");
        QString body = QMessageBox::tr ("Woulld you like to cancel the add text action ?");
        box.setText (message);
        box.setInformativeText (body);
        box.setStandardButtons (QMessageBox::No | QMessageBox::Yes);
        box.setDefaultButton   (QMessageBox::No);
        if (box.exec() == QMessageBox::Yes) {
            return;
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
  QString meta = QLatin1String("0 !LPUB INSERT BOM");

  Where bottomOfPage = Gui::topOfPages[Gui::displayPageNum]; //start at the bottom of the page's last step

  bool forModel;
  int option = BomOptionDialog::getOption(forModel, bottomOfPage.modelIndex, nullptr);

  if (option == AppendNoOption)
    return;

  if (!forModel)
      meta.append(" FOR_SUBMODEL");

  if (option == AppendAtPage) {
    if (lpub->page.coverPage) {
        emit gui->messageSig(LOG_ERROR, QObject::tr("Adding a bill of materials to a cover page is not allowed."));
        return;
    }
    scanPastGlobal(bottomOfPage);
    insertMeta(bottomOfPage,meta);
  } else if (option && option != AppendAtPage) {
    if (option == AppendAtSubmodel)                          //start at the bottom of the model's last step
     bottomOfPage = Where(bottomOfPage.modelName, lpub->ldrawFile.size(bottomOfPage.modelName));
    else if (option == AppendAtModel)                        //start at the bottom of the main model's last step
     bottomOfPage = Where(lpub->ldrawFile.topLevelFile(), lpub->ldrawFile.size(lpub->ldrawFile.topLevelFile()));

    QString pageMeta = "0 !LPUB INSERT PAGE";

    beginMacro("insertBOM");
    appendPage(pageMeta, bottomOfPage, option);              // bottomOfPage adjusted
    insertMeta(bottomOfPage, meta);
    endMacro();
  }
}

int MetaItem::displayModelStepExists(Rc &rc, bool deleteStep)
{
  Where saveHere;                                            //initialize saveHere
  Where bottom(lpub->ldrawFile.topLevelFile(),0,lpub->ldrawFile.size(lpub->ldrawFile.topLevelFile()));
  Where here = bottom;                                       //start at bottom of file
  here--;                                                    //adjust lineNumber for zero-start index

  auto thisLine = [&] (const Where &here) { return deleteStep ? 0 : here.lineNumber; };

  for ( ; here >= 0; here--) {                               //scan from bottom to top of file
    QString line = lpub->ldrawFile.readLine(here.modelName,here.lineNumber);
    rc = lpub->meta.parse(line,here);
    if (rc == StepRc || rc == RotStepRc || rc == NoStepRc) { //if Step, RotStep, save the line number to perform insert (place before) later
      if (saveHere == Where())                                //if nothing saved perform save
        saveHere = here.lineNumber;
    } else if (rc == StepGroupEndRc || rc == CalloutEndRc) { //if StepGroup or Callout, return the line number
#ifdef QT_DEBUG_MODE
      emit gui->messageSig(LOG_DEBUG, QObject::tr("Insert Final Model - hit end of StepGroup or Callout - Ok to insert Model at line: %1").arg(here.lineNumber));
#endif
      return thisLine(here);                                 //reached a valid boundary so return line number
    } else if (rc == InsertFinalModelRc ) {                  //check for inserted final model
      emit lpub->messageSig(LOG_INFO, QObject::tr("Final model detected at line: %1").arg(here.lineNumber));
      return here.lineNumber /*DM_FINAL_MODEL*/;
    } else if (rc == InsertDisplayModelRc ) {                //check for inserted display model
      emit lpub->messageSig(LOG_INFO, QObject::tr("Display model detected at line: %1").arg(here.lineNumber));
      return thisLine(here)  /*DM_DISPLAY_MODEL*/;
    } else {                                                 //else keep walking back until 1_5 line
      QStringList args;
      split(line,args);
      if (args.size() && args[0] >= "1" && args[0] <= "5") { //non-zero line detected so no back final model
        if (saveHere.lineNumber) {
#ifdef QT_DEBUG_MODE
          emit lpub->messageSig(LOG_DEBUG, QObject::tr("Insert Final Model - hit end of Step - Ok to insert model at line: %1").arg(saveHere.lineNumber));
#endif
          return thisLine(saveHere);
        } else {
          if (here.lineNumber >= bottom.lineNumber - 1) {    //check if 'here' is at the end of the file
#ifdef QT_DEBUG_MODE
            emit lpub->messageSig(LOG_DEBUG, QObject::tr("Insert Final Model - hit line type 1-5 - Ok to insert model at EOF, line: %1").arg(here.lineNumber));
#endif
            return thisLine(bottom - 1);                     //return last line adjust for zero-start index
          }
        }
      }
    }
  }
  return thisLine(bottom - 1);                               //adjust lineNumber for zero-start index
}

void MetaItem::insertDisplayModelStep(Where &here, bool finalModel)
{
    bool isNotFinalStep = lpub->ldrawFile.readLine(here.modelName,here.lineNumber).contains(QRegExp("^0 STEP$"));
    if (!isNotFinalStep)
        isNotFinalStep = here.lineNumber < lpub->ldrawFile.size(lpub->ldrawFile.topLevelFile()) - 1; //adjust lineNumber for zero-start index

    QString textInsert;
    if (Preferences::enableFadeSteps)
        textInsert = QString("fade previous steps");
    if (Preferences::enableHighlightStep)
        textInsert += Preferences::enableFadeSteps ? QString(" and highlight current step") : QString("highlight current step");
    
    const QString disclaimerText1 = QString(    "0 // These 6 command lines were auto-generated for %1.").arg(textInsert);
    const QString disclaimerText2 = QObject::tr("0 // These lines are not saved and should not be modified, but they can be replaced.");
    const QString disclaimerText3 = QObject::tr("0 // Remove comment lines and replace MODEL with DISPLAY_MODEL to override this behaviour.");
    const QString modelInsertMeta = finalModel ? QString("0 !LPUB INSERT MODEL") :
                                                 QString("0 !LPUB INSERT DISPLAY_MODEL");
    const QString pageMeta        = QString("0 !LPUB INSERT PAGE");

    const QString macroLabel = finalModel ? QString("insertFinalModelStep") : QString("insertDisplayModelStep");

    beginMacro(macroLabel);

    if (isNotFinalStep) {
      if (finalModel) {
         appendMeta(here,disclaimerText1);
         appendMeta(++here,disclaimerText2);
         appendMeta(++here,disclaimerText3);
         appendMeta(++here,modelInsertMeta);
      } else /*displayModel*/ {
         appendMeta(here,modelInsertMeta);
      }
      appendMeta(++here,pageMeta);
      appendMeta(++here,step);
    } else /*insert final step*/ {
      if (finalModel) {
         appendMeta(here,disclaimerText1);
         appendMeta(++here,disclaimerText2);
         appendMeta(++here,disclaimerText3);
         appendMeta(++here,step);
      } else /*displayModel*/ {
         appendMeta(here,step);
      }
      appendMeta(++here,modelInsertMeta);
      appendMeta(++here,pageMeta);
    }

    endMacro();
}

void MetaItem::insertFinalModelStep()
{
  if (currentFile() && Preferences::finalModelEnabled && (Preferences::enableFadeSteps || Preferences::enableHighlightStep)) {
    Rc rc = OkRc;
    int lineNumber = displayModelStepExists(rc);
    if (lineNumber) {
      if (rc != InsertFinalModelRc && rc != InsertDisplayModelRc) {
        emit lpub->messageSig(LOG_INFO, QObject::tr("Inserting fade/highlight final model step at line %1...").arg(lineNumber));
        insertFinalModelStep(lineNumber);
      }
    }
  }
}

void MetaItem::insertFinalModelStep(int atLine)
{
  if (atLine <= 0) // final model already installed so exit.
    return;

  Where here(lpub->ldrawFile.topLevelFile(),atLine);
  insertDisplayModelStep(here, true /*Final Model*/);
  emit lpub->messageSig(LOG_INFO, QMessageBox::tr("Final model inserted at lines %1 to %2").arg(atLine+1).arg(here.lineNumber+1));
}

bool MetaItem::deleteFinalModelStep(bool fromPreferences) {

  bool foundFinalModel = false;
  Where walk,here;
  Rc rc = OkRc;

  if (currentFile() && (fromPreferences || (Preferences::finalModelEnabled && (Preferences::enableFadeSteps || Preferences::enableHighlightStep)))) {
    int finalModelLine = displayModelStepExists(rc, true/*deleteStep*/);
    if ((foundFinalModel = finalModelLine && rc == InsertFinalModelRc)) {
      emit lpub->messageSig(LOG_INFO, QObject::tr("Removing fade/highlight final model step at line %1...").arg(finalModelLine));
      here = Where(lpub->ldrawFile.topLevelFile(),finalModelLine);// start at insert meta command line //start at bottom of file
      walk = here;                                                //mark line as starting point for deletion
      int maxLines = lpub->ldrawFile.size(lpub->ldrawFile.topLevelFile());
      if (walk < maxLines)                                        //check if last line and adjust starting point for deletion
        rc = scanForwardNoParts(walk, StepMask);                  //scan to end of final model step
#ifdef QT_DEBUG_MODE
      emit lpub->messageSig(LOG_DEBUG, QObject::tr("Final model meta commands detected at lines %1 to %2")
                                                  .arg(here.lineNumber)
                                                  .arg(walk.lineNumber));
#endif
    } else return foundFinalModel;
  } else return foundFinalModel;

  if (foundFinalModel && (rc == StepRc || rc == RotStepRc || rc == EndOfFileRc)) {
    QRegExp rx("^\\s*0\\s+\\/\\/\\s*These 6 command lines were auto-generated for (?:fade previous steps(?: and)? highlight current step).$",Qt::CaseInsensitive);
    bool eof = rc == EndOfFileRc;
    int disclaimerTextLines = eof ? 4 : 3;                        //if EOF (final step), account for 0 STEP command before insert INSERT MODEL
    for (int i = 0; i < disclaimerTextLines; i++) {
      here.lineNumber -= 1;                                       //return to disclaimer text before insert model meta command line
      QString line = lpub->ldrawFile.readLine(here.modelName,here.lineNumber);
      if (isComment(line) || line.contains(rx) || (eof ? line == step : true))
        continue;
      else
        break;
    }

    beginMacro("deleteFinalModelStep");
    for (; walk.lineNumber >= here.lineNumber ; walk-- ) {        //remove lines between model insert and model insert step
#ifdef QT_DEBUG_MODE
      emit lpub->messageSig(LOG_DEBUG, QObject::tr("Deleting inserted final model line %1 in '%2' [%3]")
                                                  .arg(walk.lineNumber)
                                                  .arg(walk.modelName)
                                                  .arg(lpub->ldrawFile.readLine(walk.modelName,walk.lineNumber)));
#endif
      deleteMeta(walk);
    }
    endMacro();
    return foundFinalModel;
  }

  return foundFinalModel;
}

QString MetaItem::viewerStepKeySuffix(const Where &top, Step *step, bool stepCheck)
{
    QRegExp bufExLoadRx("^0 BUFEXCHG \\S+ RETRIEVE$");                   // _bfx
    QRegExp dispModelRx("^0 !?LPUB INSERT DISPLAY_MODEL$");              // _dm
    QRegExp buildModActRx("^0 !?LPUB BUILD_MOD (?:REMOVE|APPLY) \\S+$"); // _bm
    Where here;
    bool partsAdded = false;
    bool hasDispModel = false;
    bool hasBufExLoad = false;
    bool hasBuildModAct = false;
    if (!(hasDispModel = step ? step->modelDisplayOnlyStep : false)) {
        if (stepCheck) {
            here = top;
            hasDispModel = Gui::stepContains(here, dispModelRx);
        }
    } else if (!(hasBuildModAct = step ? step->buildModActionStep : false)) {
        if (stepCheck) {
            here = top;
            scanForward(here, StepMask|StepGroupMask, partsAdded);
            if (!partsAdded) {
                here = top;
                hasBuildModAct = Gui::stepContains(here, buildModActRx);
            }
        }
    } else if (!(hasBufExLoad = step ? step->bfxLoadStep : false)) {
        if (stepCheck) {
            here = top;
            scanForward(here, StepMask|StepGroupMask, partsAdded);
            if (!partsAdded) {
                here = top;
                hasBufExLoad = Gui::stepContains(here, bufExLoadRx);
            }
        }
    }

//#ifdef QT_DEBUG_MODE
//    qDebug() << qPrintable(QString("DEBUG: %1VIEWER STEP KEY ends with%2")
//                           .arg(stepCheck ? "STEP_CHECK " : "")
//                           .arg(hasDispModel ? " DISPLAY_MODEL (_dm)" :
//                                hasBufExLoad ? " BUFEXCHG RETRIEVE (_bfx)" :
//                                hasBuildModAct ? " BUILD_MOD ACTION (_bm)" : " STEP_NUMBER"));
//#endif
    return hasDispModel ? "_dm" : hasBufExLoad ? "_bfx" : hasBuildModAct ? "_bm" : QString();
}

void MetaItem::changePreferredRenderer(
  QString      title,
  const Where &topOfStep,
  const Where &bottomOfStep,
  PreferredRendererMeta *meta,
  int          append,
  bool         local,
  bool         askLocal)
{
  PreferredRendererMeta _meta;
  _meta.setValue(meta->value());

  bool ok;
  ok = PreferredRendererDialog::getPreferredRenderer(_meta,title,gui);

  if (ok && _meta.value().renderer != meta->value().renderer) {
//    bool reloadFile = false;
    QRegExp calloutRx("CALLOUT BEGIN");
    Where here(topOfStep + 1);
    bool useTop = !Gui::stepContains(here,calloutRx);
//    beginMacro("PreferredRenderer");
    meta->setValue(_meta.value());
    setMeta(topOfStep,bottomOfStep,meta,useTop,append,local,askLocal);
//    reloadFile = true;
//    setSuspendFileDisplayFlag(true);
//    endMacro();
//    if (reloadFile) {
//      reloadDisplayPage(true);
//    }
  }
}

void MetaItem::setFadeSteps(
QString         title,
  const Where  &topOfStep,
  const Where  &bottomOfStep,
  FadeStepsMeta *meta,
  int           append,
  bool          local,
  bool          askLocal)
{
  FadeStepsMeta _meta;
  _meta.enable   = meta->enable;
  _meta.color    = meta->color;
  _meta.opacity  = meta->opacity;
  bool ok;
  ok = FadeHighlightDialog::getFadeSteps(_meta,title,gui);

  if (ok) {
    bool reloadFile = false;
    QRegExp calloutRx("CALLOUT BEGIN");
    Where here(topOfStep + 1);
    bool useTop = !Gui::stepContains(here,calloutRx);
    beginMacro("SetFadeSteps");
    if(_meta.enable.value() != meta->enable.value()) {
      meta->enable.setValue(_meta.enable.value());
      setMeta(topOfStep,bottomOfStep,&meta->enable,useTop,append,local,askLocal);
      reloadFile = meta->enable.value();
      setSuspendFileDisplayFlag(reloadFile);
    }
    if(_meta.color.value().color != meta->color.value().color ||
       _meta.color.value().useColor != meta->color.value().useColor) {
      meta->color.setValue(_meta.color.value());
      setMeta(topOfStep,bottomOfStep,&meta->color,useTop,append,local,askLocal);
    }
    if(_meta.opacity.value() != meta->opacity.value()) {
      meta->opacity.setValue(_meta.opacity.value());
      setMeta(topOfStep,bottomOfStep,&meta->opacity,useTop,append,local,askLocal);
    }
    endMacro();
    if (reloadFile) {
      clearAndReloadModelFile();
    } else {
      clearCsiCache();
    }
  }
}

void MetaItem::setHighlightStep(
  QString            title,
  const Where       &topOfStep,
  const Where       &bottomOfStep,
  HighlightStepMeta *meta,
  int                append,
  bool               local,
  bool               askLocal)
{
  HighlightStepMeta _meta;
  _meta.enable    = meta->enable;
  _meta.color     = meta->color;
  _meta.lineWidth = meta->lineWidth;
  bool ok;
  ok = FadeHighlightDialog::getHighlightStep(_meta,title,gui);

  if (ok) {
    bool reloadFile = false;
    QRegExp calloutRx("CALLOUT BEGIN");
    Where here(topOfStep + 1);
    bool useTop = (append = !Gui::stepContains(here,calloutRx));
    beginMacro("SetHighlightStep");
    if(_meta.enable.value() != meta->enable.value()) {
      meta->enable.setValue(_meta.enable.value());
      setMeta(topOfStep,bottomOfStep,&meta->enable,useTop,append,local,askLocal);
      reloadFile = meta->enable.value();
      setSuspendFileDisplayFlag(reloadFile);
    }
    if(_meta.color.value() != meta->color.value()) {
      meta->color.setValue(_meta.color.value());
      setMeta(topOfStep,bottomOfStep,&meta->color,useTop,append,local,askLocal);
    }
    if(_meta.lineWidth.value() != meta->lineWidth.value()) {
      meta->lineWidth.setValue(_meta.lineWidth.value());
      setMeta(topOfStep,bottomOfStep,&meta->lineWidth,useTop,append,local,askLocal);
    }
    endMacro();
    if (reloadFile) {
      clearAndReloadModelFile();
    } else {
      clearCsiCache();
    }
  }
}

void MetaItem::insertSplitBOM()
{
  QString pageMeta = QString("0 !LPUB INSERT PAGE");
  QString bomMeta  = QString("0 !LPUB INSERT BOM");

  Where topOfStep = Gui::topOfPages[Gui::displayPageNum-1];

  scanPastGlobal(topOfStep);

  beginMacro("splitBOM");
  deleteBOMPartGroups();
  insertPage(pageMeta);
  appendMeta(topOfStep+1,bomMeta);
  endMacro();
}

void MetaItem::deleteBOM()
{
  Where topOfPage    = Gui::topOfPages[Gui::displayPageNum-1];
  Where bottomOfPage = Gui::topOfPages[Gui::displayPageNum];
  for (++topOfPage; topOfPage.lineNumber < bottomOfPage.lineNumber; topOfPage++) {
    QString line = lpub->ldrawFile.readLine(topOfPage.modelName,topOfPage.lineNumber);
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
  Where topOfPage    = Gui::topOfPages[Gui::displayPageNum-1];
  Where bottomOfPage = Gui::topOfPages[Gui::displayPageNum];
  for (Where walk = bottomOfPage; walk >= topOfPage.lineNumber; walk--) {
    Meta mi;
    QString metaString = lpub->ldrawFile.readLine(walk.modelName,walk.lineNumber);
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
    QString metaString = lpub->ldrawFile.readLine(walk.modelName,walk.lineNumber);
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
    QString metaString = lpub->ldrawFile.readLine(here.modelName,here.lineNumber);
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
  int numLines = lpub->ldrawFile.size(topOfStep.modelName);
  scanPastGlobal(topOfStep);
  QRegExp rotateIconMeta("^\\s*0\\s+!LPUB\\s+.*"+metaCommand);
  for ( ; walk < numLines; walk++) {
      QString line = lpub->ldrawFile.readLine(walk.modelName,walk.lineNumber);
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

  int  numLines  = lpub->ldrawFile.size(walk.modelName);
  if (walk < numLines) {
    QString line = lpub->ldrawFile.readLine(walk.modelName,walk.lineNumber);
    QRegExp lpubLine("^\\s*0\\s+!LPUB\\s+.*");
    if (line.contains(lpubLine) || isHeader(line) || isComment(line)) {
      for ( ++walk; walk < numLines; ++walk) {
        line = lpub->ldrawFile.readLine(walk.modelName,walk.lineNumber);
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

Rc MetaItem::scanForwardNoParts(Where &here,int mask)
{
  bool fakePartAdd;

  return scanForward(here, mask, fakePartAdd, true/*noPartCheck*/);
}

Rc  MetaItem::scanForward(
  Where &here,
  int    mask,
  bool  &partsAdded,
  bool noPartCheck)
{
  Meta tmpMeta;
  int  numLines  = lpub->ldrawFile.size(here.modelName);
  partsAdded = noPartCheck;

  scanPastGlobal(here);

  for ( ; here < numLines; here++) {
    QString line = lpub->ldrawFile.readLine(here.modelName,here.lineNumber);
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

Rc MetaItem::scanBackwardNoParts(Where &here,int mask)
{
  bool fakePartAdd;

  return scanBackward(here, mask, fakePartAdd, true/*noPartCheck*/);
}

Rc MetaItem::scanBackward(
  Where &here,
  int    mask,  // What we stop on
  bool  &partsAdded,
  bool noPartCheck)
{
  Meta tmpMeta;
  partsAdded = noPartCheck;

  for ( ; here >= 0; here--) {

    QString line = lpub->ldrawFile.readLine(here.modelName,here.lineNumber);
    QStringList tokens;

    if (isHeader(line)) {
      scanPastGlobal(here);

      return EndOfFileRc;
    }
    split(line,tokens);

    bool token_1_5 = tokens.size() && tokens[0].size() == 1 && tokens[0] >= "1" && tokens[0] <= "5";

    if (token_1_5) {
      partsAdded = true;

    } else {
      Rc rc = tmpMeta.parse(line,here);

// TODO - InsertPageRc and InsertCoverPageRc if blocks are hacks
//        to allow the scan to terminate on these metas if no parts are added
//        I'm not sure of the impact on reversing it as it was done so long
//        ago. But it should be refersed.

      if (rc == InsertPageRc /*&& ((mask >> rc) & 1)*/) {

         return rc;
      } else if (rc == InsertCoverPageRc /*&& ((mask >> rc) & 1)*/) {

         return rc;
      } else if (rc == StepRc || rc == RotStepRc) {

        if (((mask >> rc) & 1) && partsAdded) {

          return rc;
         }

        partsAdded = false;
      } else if (rc < ClearRc && ((mask >> rc) & 1)) {

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
  Gui::scanPast(topOfStep,globalLine);
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
  int maxLines = lpub->ldrawFile.size(modelName);
  QRegExp lines1_5("^\\s*[1-5]");

  bool header = true;

  /* Skip all of the header lines */

  for ( ; walk < maxLines; walk++)
  {
    QString line = lpub->ldrawFile.readLine(walk.modelName,walk.lineNumber);

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
  return lpub->ldrawFile.numSteps(modelName);
}

/***********************************************************************
 *
 * Callout tools
 *
 **********************************************************************/

int MetaItem::nestCallouts(
  Meta  *meta,
  const QString &modelName,
  bool  isMirrored,
  bool  pointerless)
{
  bool restart = true;

  while (restart) {

    restart = false;

    Where walk(modelName,1);

    int numLines = lpub->ldrawFile.size(walk.modelName);

    bool partIgnore = false;
    bool callout = false;

    // modelName is called out, so any submodels in modelName need to be
    // called out

    // for all the lines in the file

    for ( ; walk.lineNumber < numLines; ++walk) {

      QString line = lpub->ldrawFile.readLine(walk.modelName,walk.lineNumber);

      QStringList argv;

      split(line,argv);

      // Process meta-commands so we don't turn ignored or substituted
      // submodels get called out

      if (argv.size() >= 2 && argv[0] == "0") {
        if (argv[1] == "!LPUB" || argv[1] == "LPUB") {
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
          if (lpub->ldrawFile.isSubmodel(argv[14])) {
            meta->submodelStack << SubmodelStack(walk.modelName,walk.lineNumber,0);
            addCalloutMetas(meta,argv[14],isMirrored,pointerless);
            nestCallouts(meta,argv[14],isMirrored,pointerless);
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
    QString line = lpub->ldrawFile.readLine(walkBack.modelName,walkBack.lineNumber);

    if (isHeader(line)) {
      return true;
    } else {
      QStringList argv;
      split(line,argv);
      if (argv.size() == 5 && argv[0] == "0" &&
         (argv[1] == "!LPUB" || argv[1] == "LPUB") &&
          argv[2] == "CALLOUT" && argv[3] == "BEGIN") {
        return false;
      }
      if (argv.size() >= 2 && argv[0] == "0") {
        if (argv[1] == "STEP" || argv[1] == "ROTSTEP" ||
            argv[1] == "!LPUB" || argv[1] == "LPUB") {
          return true;
        }
      }
    }
  }
  return true;
}

void MetaItem::convertToCallout(Meta *meta,
  const QString &modelName,
  bool  isMirrored,
  bool  assembled,
  bool pointerless)
{
  Gui::maxPages = -1;

  beginMacro("convertToCallout");
  addCalloutMetas(meta,modelName,isMirrored,assembled,pointerless);
  if ( ! assembled) {
    nestCallouts(meta,modelName,isMirrored,pointerless);
  }
  endMacro();
}

void MetaItem::addCalloutMetas(
  Meta *meta,
  const QString &modelName,
  bool  /* isMirrored */,
  bool  assembled,
  bool  pointerless)
{
  /* Scan the file and remove any multi-step stuff from the file
     we're converting to callout - starting from the bottom up*/

  int   numLines;
  Where walk(modelName,0);

  if (! assembled) {
    numLines = lpub->ldrawFile.size(modelName);
    walk.lineNumber = numLines - 1;
    QRegExp ms("^\\s*0\\s+\\!*LPUB\\s+MULTI_STEP\\s+");

    do {
      QString line = lpub->ldrawFile.readLine(walk.modelName,walk.lineNumber);
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
    QString line = lpub->ldrawFile.readLine(walkBack.modelName,walkBack.lineNumber);

    if (isHeader(line)) {
      break;
    } else {
      QStringList argv;
      split(line,argv);
      if (argv.size() >= 2 && argv[0] == "0") {
        if (argv[1] == "STEP" || argv[1] == "ROTSTEP" ||
            argv[1] == "!LPUB" || argv[1] == "LPUB") {
          break;
        }
      } else if (argv.size() == 15 && argv[0] == "1") {
        if (lpub->ldrawFile.isSubmodel(argv[14])) {
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
  numLines = lpub->ldrawFile.size(walk.modelName);
  for ( ; walk.lineNumber < numLines; walk++) {
    QString line = lpub->ldrawFile.readLine(walk.modelName,walk.lineNumber);
    QStringList argv;
    split(line,argv);
    if (argv.size() >= 2 && argv[0] == "0") {
      if (argv[1] == "STEP" || argv[1] == "ROTSTEP" ||
          argv[1] == "!LPUB" || argv[1] == "LPUB") {
        break;
      }
    } else if (argv.size() == 15 && argv[0] == "1") {
      if (lpub->ldrawFile.isSubmodel(argv[14])) {
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
        pushed = QMessageBox::question(gui,QMessageBox::tr("Multiple Copies"),
                                           QMessageBox::tr("There are multiple copies, do you want them as one callout?"),
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
      QString pointerLine;
      if (!pointerless) {
        /*
        defaultPointerTip is the trick - it calculates the pointer tip
        for a given instance of a callout.  It does this by rendering
        the parent image with the non-called out parts color A and
        the called out parts color B.  Then the resultant image is
        searched for color B.  The parent model needs to be rotated
        by ROTSTEP for this to work.
        */
        QStringList argv;
        split(firstLine,argv);
        QPointF offset = defaultPointerTip(*meta,
                                           walk.modelName,
                                           firstInstance.lineNumber,
                                           modelName,
                                           i,
                                           lpub->ldrawFile.mirrored(argv));
#ifdef QT_DEBUG_MODE
        emit gui->messageSig(LOG_DEBUG, QString("[Tip Point] (%1, %2)").arg(QString::number(offset.x(),'f',6))
                             .arg(QString::number(offset.y(),'f',6)));
#endif
        pointerLine = QString("%1 %2 0 0 0 0 0 0 1") .arg(offset.x()) .arg(offset.y());
      }

      if (together) {
        if (!pointerless) {
          appendMeta(lastInstance,"0 !LPUB CALLOUT POINTER CENTER 0 " + pointerLine);
          ++lastInstance.lineNumber;
        }
      } else {
        if (!pointerless) {
          appendMeta(thisInstance,"0 !LPUB CALLOUT POINTER CENTER 0 " + pointerLine);
          ++thisInstance.lineNumber;
        }
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
  Gui::maxPages = -1;

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

/*
 *
 * CSI step annotations
 *
 */

int MetaItem::setCsiAnnotationMetas(Steps *steps)
{
    TraverseRc trc = HitNothing;

    if (!steps->groupStepMeta.LPub.assem.annotation.display.value())
        return static_cast<int>(trc);

    for (int i = 0; i < steps->list.size(); i++) {
        Range *range = dynamic_cast<Range *>(steps->list[i]);
        if (range) {
            for (int j = 0; j < range->list.size(); j++) {
                if (range->relativeType == RangeType) {
                    Step *step = dynamic_cast<Step *>(range->list[j]);
                    if (step && step->relativeType == StepType) {
                        int adjust = 0;
                        if(step->setCsiAnnotationMetas(steps->groupStepMeta, adjust, false/*force*/)) {
                            trc = HitCsiAnnotation;
                        }
#ifdef QT_DEBUG_MODE
                        emit gui->messageSig(LOG_DEBUG, QObject::tr("Step %1 Line Adjustment %2")
                                                                    .arg(step->stepNumber.number).arg(adjust));
#endif
                    }
                }
            }
        }
    }

    return static_cast<int>(trc);
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
    lpub_sort(lineNumbers.begin(),lineNumbers.end());

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
//#ifdef QT_DEBUG_MODE
//        logTrace() << "\nCSI ANNOTATION ICON LINE: " << line
//                  << "\nCSI ANNOTATION ICON META: " << meta->LPub.assem.annotation.icon.format(false,false)
                      ;
//#endif
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

//#ifdef QT_DEBUG_MODE
//  label              += QString("_%1_").arg(partLineNum);
//#endif

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
  FloatPairMeta cameraAngles;
  cameraAngles.setValues(meta.LPub.assem.cameraAngles.value(0),
                         meta.LPub.assem.cameraAngles.value(1));


  // this block has been refactored to reflect that this function exclusively uses the Native Renderer
  if (renderer->useLDViewSCall()) {
      ldrName = QDir::currentPath() + "/" + Paths::tmpDir + "/" + label + "Mono.ldr";
      pngName = QDir::currentPath() + "/" + Paths::assemDir + "/" + label + "Mono.png";
      ldrNames << ldrName;
      csiKeys << title + "Mono";
      // RotateParts #2 - 8 parms
      ok[0] = (renderer->rotateParts(addLine,meta.rotStep,csiParts,ldrName,modelName,cameraAngles,false/*ldv*/,Options::MON) == 0);
      ok[1] = (renderer->renderCsi(addLine,ldrNames,csiKeys,pngName,meta) == 0);
    } else {
      ok[0] = true;
      pngName = QDir::currentPath() + "/" + Paths::tmpDir + "/" + label + "Mono.png";
      if (Preferences::preferredRenderer == RENDERER_NATIVE) {
          ldrName = QDir::currentPath() + "/" + Paths::tmpDir + "/csi.ldr";
          // RotateParts #2 - 8 parms
          ok[0] = (renderer->rotateParts(addLine,meta.rotStep,csiParts,ldrName,modelName,cameraAngles,false/*ldv*/,Options::MON) == 0);
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

#ifdef QT_DEBUG_MODE
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
#endif
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
  Gui::maxPages = -1;

  /* scan the called out model and remove any dividers */

  int  numLines = lpub->ldrawFile.size(modelName);

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
    QString line = lpub->ldrawFile.readLine(walk.modelName,walk.lineNumber);
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

  int numLines = lpub->ldrawFile.size(walk.modelName);

  bool partIgnore = false;
  bool callout = false;

  for ( ; walk.lineNumber < numLines; ++walk) {

    QString line = lpub->ldrawFile.readLine(walk.modelName,walk.lineNumber);

    QStringList argv;

    split(line,argv);

    if (argv.size() >= 2 && argv[0] == "0") {
      if (argv[1] == "!LPUB" || argv[1] == "LPUB") {
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
        if (lpub->ldrawFile.isSubmodel(argv[14]) && callout) {
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

void MetaItem::deletePointer(const Where &here, bool line, bool border, bool tip)
{
   Where _here = here;
   if (line)
       deletePointerAttribute(_here,true);
   if (border)
       deletePointerAttribute(_here,true);
   if (tip)
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
     QString line = lpub->ldrawFile.readLine(walk.modelName,++walk.lineNumber); // advance 1 line
     if (line.contains(rx)) {
         deleteMeta(walk);                 // check first line
     }
     else
     {
        ++walk;                           // check second line
        line = lpub->ldrawFile.readLine(walk.modelName,walk.lineNumber);
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

QString MetaItem::makeMonoName(
   const QString &fileName,
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
  bool whiteModel = color == monoColor[TransWhite];
  MonoColors colorCode = (whiteModel ? TransWhite : Blue);

  QFile outFile(monoOutName);
  if ( ! outFile.open(QFile::WriteOnly | QFile::Text)) {
    emit gui->messageSig(LOG_ERROR,QString("MonoColorSubmodel cannot write file %1: %2.")
                         .arg(monoOutName)
                         .arg(outFile.errorString()));
    return -1;
  }

  int numLines = lpub->ldrawFile.size(modelName);

 /*
  * scan past header
  */
  Where walk(modelName,0);
  Where here = walk;
  if (whiteModel) {
    bool header = true;
    bool firstLine = true;
    for ( ; here < numLines; here++)
    {
      QString line = lpub->ldrawFile.readLine(here.modelName,here.lineNumber);
      if (here > 0) {
        if (header) {
          header &= (isHeader(line) || firstLine);
          firstLine = false;
          if ( ! header) {
            break;
          }
        }
      }
    }
  }

  bool monoColorAdded = false;

  QTextStream out(&outFile);

  for ( ; walk < numLines; walk++) {
    if (whiteModel && !monoColorAdded && walk.lineNumber == here.lineNumber) {
      out << QString("0 // %1 part custom color").arg(VER_PRODUCTNAME_STR) << lpub_endl;
      out << QString("0 !COLOUR %1_White CODE 11015 VALUE #FFFFFF EDGE #FFFFFF ALPHA 32").arg(VER_PRODUCTNAME_STR) << lpub_endl;
      out << "0" << lpub_endl;
      monoColorAdded = true;
    }

    QString line = lpub->ldrawFile.readLine(walk.modelName,walk.lineNumber);
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
      if (lpub->ldrawFile.isSubmodel(submodel)) {
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
    out << line << lpub_endl;
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
    emit gui->messageSig(LOG_ERROR,QString("Cannot read defaultPointerTip file %1: %2.")
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
  int numLines           = lpub->ldrawFile.size(modelName) + colorLines;
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
        bool mirrored = lpub->ldrawFile.mirrored(argv);
        if (argv[0] == "1" &&
            argv[14] == monoSubModel &&
            mirrored == isMirrored) {
          if (instances++ == instance) {
            break;
          }
        }
      }
    }
//#ifdef QT_DEBUG_MODE
//    else
//    {
//      // get the last ROTSTEP...
//      if (argv.size() > 0 && argv[0] == "0") {
//        Where here(modelName,i);
//        Rc rc = meta.parse(line,here,false);
//        if (rc == RotStepRc) {
//
//          gui->messageSig(LOG_DEBUG,QString("Called out subModel %1 ROTSTEP %2")
//                          .arg(subModel)
//                          .arg(renderer->getRotstepMeta(meta.rotStep)));
//        }
//      }
//    }
//#endif
    csiParts << line;
  }

  /* we've reached the end of the model file */

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
   * Append blue submodel to csiParts
   */

  csiParts << argv.join(" ");

  QString addLine;

  if (isMirrored) {

    SubmodelStack tos = meta.submodelStack[meta.submodelStack.size() - 1];

    if (meta.submodelStack.size() > 1) {
      tos = meta.submodelStack[meta.submodelStack.size() - 2];
      Where here(tos.modelName, tos.lineNumber+1);
      addLine = lpub->ldrawFile.readLine(here.modelName,here.lineNumber);
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
  monoOutPngBaseName = QString("mono_%1").arg(QFileInfo(subModel).completeBaseName());
#else
  monoOutPngBaseName = QString("mono");
#endif
  QStringList csiKeys, ldrNames;
  FloatPairMeta cameraAngles;
  cameraAngles.setValues(meta.LPub.assem.cameraAngles.value(0),
                         meta.LPub.assem.cameraAngles.value(1));
  if (renderer->useLDViewSCall()) {
      csiKeys << "mono";
      ldrNames << monoOutName;
      ldrName = ldrNames.first();
      pngName = QDir::currentPath() + "/" + Paths::assemDir + "/" + monoOutPngBaseName + ".png";
      // RotateParts #2 - 8 parms
      ok[0] = (renderer->rotateParts(addLine,meta.rotStep,csiParts,ldrName,modelName,cameraAngles,false/*ldv*/,Options::MON) == 0);
      ok[1] = (renderer->renderCsi(addLine,ldrNames,csiKeys,pngName,meta) == 0);
  } else {
      ok[0] = true;
      pngName = QDir::currentPath() + "/" + Paths::tmpDir + "/" + monoOutPngBaseName + ".png";
      if (Preferences::preferredRenderer == RENDERER_NATIVE) {
         ldrName = QDir::currentPath() + "/" + Paths::tmpDir + "/csi.ldr";
         // RotateParts #2 - 8 parms
         ok[0] = (renderer->rotateParts(addLine,meta.rotStep,csiParts,ldrName,modelName,cameraAngles,false/*ldv*/,Options::MON) == 0);
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
//#ifdef QT_DEBUG_MODE
//    emit gui->messageSig(LOG_DEBUG,QString("Called out submodel [%1] for parent model [%2] default pointer tip position:")
//                                           .arg(subModel)
//                                           .arg(modelName));
//    emit gui->messageSig(LOG_DEBUG,QString(" -top:    %1").arg(QString::number(top)));
//    emit gui->messageSig(LOG_DEBUG,QString(" -left:   %1").arg(QString::number(left)));
//    emit gui->messageSig(LOG_DEBUG,QString(" -bottom: %1").arg(QString::number(bottom)));
//    emit gui->messageSig(LOG_DEBUG,QString(" -right:  %1").arg(QString::number(right)));
//    emit gui->messageSig(LOG_DEBUG,QString(" -width:  %1").arg(QString::number(width)));
//    emit gui->messageSig(LOG_DEBUG,QString(" -height: %1").arg(QString::number(height)));
//    QPointF offset = QPointF(qreal(left)/width, qreal(top)/height);
//    emit gui->messageSig(LOG_DEBUG,QString(" -X (left[%1]/width[%2]): %3")
//                                                                   .arg(QString::number(left))
//                                                                   .arg(QString::number(width))
//                                                                   .arg(QString::number(offset.x(),'f',6)));
//
//    emit gui->messageSig(LOG_DEBUG,QString(" -Y (top[%1]/height[%2]): %3")
//                                                                   .arg(QString::number(top))
//                                                                   .arg(QString::number(height))
//                                                                   .arg(QString::number(offset.y(),'f',6)));
//    emit gui->messageSig(LOG_DEBUG,QString(" -Tip Point: (%1, %2)")
//                                                 .arg(QString::number(offset.x(),'f',6))
//                                                 .arg(QString::number(offset.y(),'f',6)));
//#endif

    return QPointF(qreal(left)/width, qreal(top)/height);
  }
  emit gui->messageSig(LOG_ERROR,QString("Render momo image for pointer tip location failed."));
  return centerOffset;
}

void MetaItem::changeRotation(
  const Where &here)
{
  int numLines = lpub->ldrawFile.size(here.modelName);
  if (here.lineNumber < numLines) {
    QString line = lpub->ldrawFile.readLine(here.modelName,here.lineNumber);
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
    lpub_sort(lineNums.begin(),lineNums.end());

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
      lpub_sort(lineNums.begin(),lineNums.end());

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

void MetaItem::removeLPubFormatting(int option, const Where &_top, const Where &_bottom)
{
  std::function<void(int, Where &, Where &)> removeFormatting;
  removeFormatting = [&] (int option, Where &top, Where &bottom)
  {
      //start at the bottom of the model's last step
      QStringList argv;
      bool bmMeta = false;
      bool bmLine = false;
      bool bomIns = false;

      for (Where walk = bottom; walk >= top.lineNumber; walk--) {
          QString line = lpub->ldrawFile.readLine(walk.modelName,walk.lineNumber);
          if (!line.size())
              continue;
          switch (line.toLatin1()[0])
          {
              case '0':
                  if (Preferences::buildModEnabled && option != RLPF_BOM) {
                      QRegExp bmRx("(BUILD_MOD_ENABLED|BUILD_MOD BEGIN|BUILD_MOD END_MOD|BUILD_MOD END)");
                      if ((bmMeta = line.contains(bmRx))) {
                          if (bmRx.cap(1).endsWith("END_MOD"))
                              bmLine = true;
                          else if (bmRx.cap(1).endsWith("BEGIN"))
                              bmLine = false;
                          bmMeta = !Preferences::removeBuildModFormat && option != RLPF_DOCUMENT;
                      }
                  }
                  split(line,argv);
                  if (argv.size() > 2 && (argv[1] == "!LPUB" || argv[1] == "LPUB") && !bmMeta) {
                      if (option == RLPF_BOM) {
                          QRegExp bomRx("(INSERT BOM|INSERT PAGE|BOM PART_GROUP| BOM )");
                          if (line.contains(bomRx)) {
                              if (bomRx.cap(1) == "INSERT BOM")
                                  bomIns = true;
                              if (bomRx.cap(1) == "INSERT PAGE") {
                                  if (!bomIns)
                                      continue;
                                  else
                                      bomIns = false;
                              }
                              gui->deleteLine(walk);
                          }
                      } else {
                          gui->deleteLine(walk);
                      }
                  }
                  break;
              default:
                  if (option == RLPF_BOM)
                      return;
                  else if (Preferences::removeChildSubmodelFormat && option != RLPF_DOCUMENT) {
                      if (argv.size() == 15 && argv[0] == "1" && lpub->ldrawFile.isSubmodel(argv[14])) {
                          option = RLPF_SUBMODEL;
                          top = Where(argv[14], 1);
                          bottom = Where(argv[14], lpub->ldrawFile.size(argv[14]));
                          removeFormatting(option, top, bottom);
                      }
                  }
                  if (bmLine)
                      gui->deleteLine(walk);
                  break;
          }
      }
  };

  Where top = _top;
  Where bottom = _bottom;

  if (option < RLPF_PAGE) {
      QStringList fileList;
      if (option == RLPF_DOCUMENT)    // RLPF_DOCUMENT
          fileList = lpub->ldrawFile.subFileOrder();
      else
          fileList << top.modelName;  // RLPF_SUBMODEL
      beginMacro("RemoveLPubFormatting");
      for (int i = 0; i < fileList.size(); ++i) {
          top.modelName = fileList[i];
          top.modelIndex = i;
          top.lineNumber = 1;
          bottom.modelName = top.modelName;
          bottom.lineNumber = lpub->ldrawFile.size(bottom.modelName);
          removeFormatting(option, top, bottom);
      }
      endMacro();
      return;
  }
  else if (option == RLPF_BOM)        // RLPF_BOM
  {
      bool forModel;
      int option = BomOptionDialog::getOption(forModel, bottom.modelIndex, nullptr, true);
      if (option == AppendNoOption)
        return;

      if (option == AppendAtPage) {
          top = Gui::topOfPages[Gui::displayPageNum-1];
      } else if (option && option != AppendAtPage) {
          //start at the bottom of the model's last step
          if (option == AppendAtSubmodel) {
              bottom = Where(bottom.modelName, lpub->ldrawFile.size(bottom.modelName));
          //start at the bottom of the main model's last step
          } else if (option == AppendAtModel) {
              bottom = Where(lpub->ldrawFile.topLevelFile(), lpub->ldrawFile.size(lpub->ldrawFile.topLevelFile()));
          }
          //walk backwards towards the start of the file until we hit a type 1 line;
          Meta meta;
          for (Where walk = bottom; walk >= _top.lineNumber; walk--) {
              QString line = lpub->ldrawFile.readLine(walk.modelName,walk.lineNumber);
              QStringList argv;
              split(line,argv);
              if (argv.size() == 15 && argv[0] == "1") {
                  top = walk.lineNumber;
                  break;
              }
              Rc rc = meta.parse(line,walk);
              if (rc == InsertRc) {
                  InsertData insertData = meta.LPub.insert.value();
                  if (insertData.type == InsertData::InsertBom) {
                      top = walk.lineNumber;
                  }
              }
          }
      }
  }
  else if (option == RLPF_CALLOUT)   // RLPF_CALLOUT
  {
      ;
  }
  // RLPF_PAGE, RLPF_STEP
  beginMacro("RemoveLPubFormatting");
  removeFormatting(option, top, bottom);
  endMacro();
}

void MetaItem::setMetaAlt(const Where &itemTop, const QString metaString, bool newCommand, bool remove)
{
    Where itemTopOf = itemTop;
    if (newCommand){
        if (itemTopOf.modelName == lpub->ldrawFile.topLevelFile())
            scanPastGlobal(itemTopOf);
        // place below item command unless end of file
        int eof = lpub->ldrawFile.size(itemTopOf.modelName);
        if (itemTopOf.lineNumber == eof)
            insertMeta(itemTopOf, metaString);
        else
            appendMeta(itemTopOf, metaString);
    } else if (remove) {
        deleteMeta(itemTopOf);
    } else {
        replaceMeta(itemTopOf, metaString);
    }
}

void MetaItem::reloadWhatsThis() const
{
    commonMenus.setWhatsThis();
}

void MetaItem::loadTheme() const
{
    gui->loadTheme();
}

void MetaItem::setSceneTheme() const
{
    gui->setSceneTheme();
}

void MetaItem::loadLDSearchDirParts(bool Process, bool OnDemand, bool Update) const
{
    gui->loadLDSearchDirParts(Process, OnDemand, Update);
}

bool MetaItem::currentFile()
{
    return !gui->getCurFile().isEmpty();
}

void MetaItem::setSuspendFileDisplayFlag(bool b) const
{
    Gui::suspendFileDisplay = b;
}

void MetaItem::clearAllCaches() const
{
    emit gui->clearAllCachesSig();
}

void MetaItem::clearStepCache(Step *step, int option) const
{
    emit gui->clearStepCacheSig(step, option);
}

void MetaItem::clearPageCache(PlacementType relativeType, Page *page, int option) const
{
    emit gui->clearPageCacheSig(relativeType, page, option);
}

void MetaItem::clearAndReloadModelFile(bool fileReload, bool savePrompt) const
{
    emit gui->clearAndReloadModelFileSig(fileReload, savePrompt);
}

void MetaItem::clearPliCache() const
{
    emit gui->clearPLICacheSig();
}

void MetaItem::clearBomCache() const
{
    emit gui->clearBOMCacheSig();
}

void MetaItem::clearCsiCache() const
{
    emit gui->clearCSICacheSig();
}

void MetaItem::clearSMICache() const
{
    emit gui->clearSMICacheSig();
}

void MetaItem::clearTempCache() const
{
    emit gui->clearTempCacheSig();
}

void MetaItem::clearCustomPartCache(bool silent) const
{
    emit gui->clearCustomPartCacheSig(silent);
}

void MetaItem::reloadCurrentPage(bool savePrompt) const
{
    emit gui->reloadCurrentPageSig(savePrompt);
}

void MetaItem::restartApplication(bool changeLibrary, bool prompt) const
{
    emit gui->restartApplicationSig(changeLibrary, prompt);
}
