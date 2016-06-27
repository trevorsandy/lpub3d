 
/****************************************************************************
**
** Copyright (C) 2007-2009 Kevin Clague. All rights reserved.
** Copyright (C) 2016 Trevor SANDY. All rights reserved.
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
#include "pliconstraindialog.h"
#include "pliannotationdialog.h"
#include "plisortdialog.h"
#include "pairdialog.h"
#include "scaledialog.h"
#include "borderdialog.h"
#include "backgrounddialog.h"
#include "dividerdialog.h"
#include "sizeandorientationdialog.h"
//#include "pagesizedialog.h"
//#include "pageorientationdialog.h"
#include "paths.h"
#include "render.h"

#include "csiitem.h"

void MetaItem::setGlobalMeta(
  QString  &topLevelFile,
  LeafMeta *leaf)
{
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

void MetaItem::addDivider(
  PlacementType parentRelativeType,
  const Where &bottomOfStep,
  RcMeta *divider)
{
  Where   walk = bottomOfStep;
  Rc      rc;
  QString divString = divider->preamble;
  int     mask = parentRelativeType == StepGroupType ? StepMask|StepGroupMask : StepMask|CalloutMask;

  rc = scanForward(walk, mask);

  if (rc == StepRc || rc == RotStepRc) {
    // we can add a divider after meta->context.curStep().lineNumber    
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
    rc = scanForward(divider,mask);

    if (rc == expRc) {
      beginMacro("deleteDivider");
      deleteMeta(divider);
      endMacro();
    }
  }
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
    if (rc == StepRc || RotStepRc) {
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
    QString modelName = meta->here().modelName;

    metaInRange = meta->here().modelName == topOf.modelName;

    metaInRange = metaInRange
            && lineNumber >= topOf.lineNumber
            && lineNumber <= bottomOf.lineNumber;

    if (metaInRange) {
        QString line = meta->format(meta->pushed,meta->global);
        replaceMeta(meta->here(),line);
    } else {

        if (askLocal) {
            local = LocalDialog::getLocal(VER_PRODUCTNAME_STR, "Change only this step?",NULL);
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

//  logTrace() << "\nSET META BOTTOM OF - "
//            << "\nPAGE WHERE - "
//            << " \nPage TopOf Model Name: "    << topOf.modelName
//            << " \nPage TopOf Line Number: "   << topOf.lineNumber
//            << " \nPage BottomOf Model Name: " << bottomOf.modelName
//            << " \nPage BottomOf Line Number: "<< bottomOf.lineNumber
//            << " \nHere Model Name: "          << meta->here().modelName
//            << " \nHere Line Number: "         << meta->here().lineNumber
//            << "\n --- "
//            << " \nAppend: "                   << (append == 0 ? "NO" : "YES")
//            << " \nMeta In Range: "            << (metaInRange ? "YES - Can replace Meta" : "NO")
//            << " \nLine (Meta in Range): "     <<  meta->format(meta->pushed,meta->global)
//            << " \nLine: "                     <<  meta->format(local, global)
//            << "\n - "
//               ;

  if (metaInRange) {
    QString line = meta->format(meta->pushed,meta->global);
    replaceMeta(meta->here(),line);
  } else {
    if (askLocal) {
      local = LocalDialog::getLocal(VER_PRODUCTNAME_STR, "Change only this step?",NULL); // changed from gui error(QLayout: Attempting to add QLayout "" to Gui "", which already has a layout)
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
       ::getPlacement(parentType,relativeType,placementData,title,onPageType,NULL,pliPerStep);

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

 qDebug() << "\nCHANGE PLACEMENT OFFSET -    "
          << "\nPAGE WHERE -                 "
          << " \nDefaultWhere (Model Name):  "   << defaultWhere.modelName
          << " \nDefaultWhere (Line Number): "   << defaultWhere.lineNumber
          << "\nPLACEMENT DATA -             "
          << " \nPlacement:                  "   << PlacNames[placement->value().placement]     << " (" << placement->value().placement << ")"
          << " \nJustification:              "   << PlacNames[placement->value().justification] << " (" << placement->value().justification << ")"
          << " \nRelativeTo:                 "   << RelNames[placement->value().relativeTo]     << " (" << placement->value().relativeTo << ")"
          << " \nPreposition:                "   << PrepNames[placement->value().preposition]   << " (" << placement->value().preposition << ")"
          << " \nRectPlacement:              "   << RectNames[placement->value().rectPlacement] << " (" << placement->value().rectPlacement << ")"
          << " \nOffset[0]:                  "   << placement->value().offsets[0]
          << " \nOffset[1]:                  "   << placement->value().offsets[1]
          << "\nPLACEMENT WHERE -            "
          << " \nPlacement Here(Model Name): "   << placement->here().modelName
          << " \nPlacement Here(Line Number):"   << placement->here().lineNumber
          << "\nOTHER DATA -                 "
          << " \n:Type:                      "   << RelNames[type] << " (" << type << ")"
          << " \n:Local:                     "   << local
          << " \n:Global:                    "   << global
          << "\n FORMATTED META -            "
          << "\nMeta Format:                 "   << placement->format(local,global)
          ;

  QString newMetaString = placement->format(local,global);

  if (placement->here().modelName == "undefined") {

    Where walk;
    bool partsAdded;
    int eof = gui->subFileSize(defaultWhere.modelName);
    defaultWhere+1 == eof ? walk = defaultWhere : walk = defaultWhere+1;

    if (scanBackward(walk,StepMask,partsAdded) == EndOfFileRc) {        
      defaultWhere = firstLine(defaultWhere.modelName);
      logNotice() << " \nScanBackward[TOP]: EndOfFileRc (StepMask) - defaultLine is: "
                  << firstLine(defaultWhere.modelName).lineNumber
                  << " of model: "
                  << defaultWhere.modelName
                     ;
    }

    if (type == StepGroupType) {             
      scanForward(defaultWhere,StepGroupBeginMask);
      logNotice() << " \nScanForward[BOTTOM]: StepGroupType (StepGroupBeginMask) - file name is: "
                  << defaultWhere.modelName
                  << " \nStop at line: "
                  << defaultWhere.lineNumber
                  << " with line contents: \n"
                  << gui->readLine(defaultWhere)
                     ;
    } else if (type == CalloutType) {
      scanForward(defaultWhere,CalloutEndMask);
      --defaultWhere;
      logNotice() << " \nScanForward[BOTTOM]: CalloutType (CalloutEndMask) - file name is: "
                  << defaultWhere.modelName
                  << " \nStop at line: "
                  << defaultWhere.lineNumber
                  << " with line contents: \n"
                  << gui->readLine(defaultWhere)
                     ;
    } else if (defaultWhere.modelName == gui->topLevelFile()) {       
      scanPastGlobal(defaultWhere);
      logNotice() << " \nTopLevelFile[TOP]: ScanPastGlobal - file name is: "
                  << defaultWhere.modelName
                  << " \nStop at line: "
                  << defaultWhere.lineNumber
                  << " with line contents: \n"
                  << gui->readLine(defaultWhere)
                     ;
    }

    if (defaultWhere.lineNumber == eof){
        insertMeta(defaultWhere,newMetaString);
        logNotice() << " \nLast line so insert Meta:  \n" << newMetaString << " \nat line: "
                    << defaultWhere.lineNumber
                       ;
    } else {
        appendMeta(defaultWhere,newMetaString);
        logNotice() << " \nNot last line so append Meta: \n" << newMetaString << " \nat line: "
                    << defaultWhere.lineNumber+1
                       ;
    }

  } else {
    replaceMeta(placement->here(),newMetaString);
    logNotice() << " \nPlacement defined so replace Meta:  \n" << newMetaString << " \nat line: "
                << defaultWhere.lineNumber
                   ;
  }
}

void MetaItem::changeConstraint(
  Where          topOfStep,
  Where          bottomOfStep,
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
  Where          topOfStep,
  Where          bottomOfStep,
  ConstrainMeta *constraint,
  int            append)
{
  setMetaBottomOf(topOfStep,bottomOfStep,constraint,append,true,false);
}

void MetaItem::changeInsertOffset(
  InsertMeta *placement)
{
  QString newMetaString = placement->format(false,false);
  replaceMeta(placement->here(),newMetaString);
}

void MetaItem::changeBackground(
  QString         title,
  const Where    &topOfStep,
  const Where    &bottomOfStep,
  BackgroundMeta *background,
  bool            useTop,
  int             append,
  bool            local)
{
  BackgroundData backgroundData = background->value();
  bool ok;
  ok = BackgroundDialog::getBackground(backgroundData,title,gui);

  if (ok) {
    background->setValue(backgroundData);
    setMeta(topOfStep,bottomOfStep,background,useTop,append,local);
  }
}

void MetaItem::changeSizeAndOrientation(
  QString              title,
  const Where         &topOfStep,
  const Where         &bottomOfStep,
  UnitsMeta           *smeta,
  PageOrientationMeta *ometa,
  bool                 useTop,
  int                  append,
  bool                 local)
{
  float values[2];

  values[0]   = smeta->value(0);
  values[1]   = smeta->value(1);

  OrientationEnc orientation;
  orientation = ometa->value();

  bool ok;
  ok = SizeAndOrientationDialog::getSizeAndOrientation(values,orientation,title,gui);

  if (ok) {

      logDebug() << " SIZE (dialog return): Orientation: " << (orientation == Portrait ? "Portrait" : "Landscape");
      ometa->setValue(orientation);
      setMeta(topOfStep,bottomOfStep,ometa,useTop,append,local);

      logDebug() << " SIZE (dialog return): Width: " << smeta->value(0) << " Height: " << smeta->value(1);
      smeta->setValue(0,values[0]);
      smeta->setValue(1,values[1]);
      setMeta(topOfStep,bottomOfStep,smeta,useTop,append,local);

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
  StringMeta    *sortMeta,
  int            append,
  bool           local)
{
  QString sortBy = sortMeta->value();
  bool ok;
  ok = PliSortDialog::getPliSortOption(sortBy,title,gui);

  if (ok) {
    sortMeta->setValue(sortBy);
    setMetaTopOf(topOfStep,bottomOfStep,sortMeta,append,local, false);
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
      if(annotation.display.value() != pliAnnotationMeta->display.value()){
          pliAnnotationMeta->display.setValue(annotation.display.value());
          setMetaTopOf(topOfStep,bottomOfStep,&pliAnnotationMeta->display,append,local,true);
        }
      if(annotation.titleAnnotation.value() != pliAnnotationMeta->titleAnnotation.value()){
          pliAnnotationMeta->titleAnnotation.setValue(annotation.titleAnnotation.value());
          setMetaTopOf(topOfStep,bottomOfStep,&pliAnnotationMeta->titleAnnotation,append,local,true);
        }
      if(annotation.freeformAnnotation.value() != pliAnnotationMeta->freeformAnnotation.value()){
          pliAnnotationMeta->freeformAnnotation.setValue(annotation.freeformAnnotation.value());
          setMetaTopOf(topOfStep,bottomOfStep,&pliAnnotationMeta->freeformAnnotation,append,local,true);
        }
      if(annotation.titleAndFreeformAnnotation.value() != pliAnnotationMeta->titleAndFreeformAnnotation.value()){
          pliAnnotationMeta->titleAndFreeformAnnotation.setValue(annotation.titleAndFreeformAnnotation.value());
          setMetaTopOf(topOfStep,bottomOfStep,&pliAnnotationMeta->titleAndFreeformAnnotation,append,local,true);
        }
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
  _color = QColorDialog::getColor(_color,NULL);

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

  bool ok   = UnitsDialog::getUnits(values,title,gui);

  if (ok) {
    margin->setValues(values[0],values[1]);
    setMeta(topOfStep,bottomOfStep,margin,append,useTop,local);
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

  bool ok   = UnitsDialog::getUnits(values,title,gui);
  
  if (ok) {
    units->setValues(values[0],values[1]);
    setMetaTopOf(topOfStep,bottomOfStep,units,append,local);
  }
}

void MetaItem::changeViewAngle(
  QString        title,
  const Where   &topOfStep,
  const Where   &bottomOfStep,
  FloatPairMeta *va,
  int            append,
  bool           local)
{
  float floats[2];
  floats[0] = va->value(0);
  floats[1] = va->value(1);
  bool ok = FloatPairDialog::getFloatPair(
              floats,
              title,
              "Lattitude",
              "Longitude",
              gui);

  if (ok) {
    va->setValues(floats[0],floats[1]);
    setMetaTopOf(topOfStep,bottomOfStep,va,append,local);
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
  int          append,
  bool         local)
{
  float data = floatMeta->value();
  bool ok = DoubleSpinDialog::getFloat(
                                  data,
                                  floatMeta->_min,
                                  floatMeta->_max,
                                  0.01,
                                  title,
                                  label,
                                  gui);
  if (ok) {
    floatMeta->setValue(data);
    setMetaTopOf(topOfStep,bottomOfStep,floatMeta,append,local);
  }
}

void MetaItem::changeBorder(QString      title,
  const Where &topOfStep,
  const Where &bottomOfStep,
  BorderMeta  *border,
  bool         useTop,
  int          append,
  bool         local,
  bool         rotateArrow)
{
  BorderData borderData = border->value();
  bool ok = BorderDialog::getBorder(borderData,title,rotateArrow);

  if (ok) {

    border->setValue(borderData);
    setMeta(topOfStep,bottomOfStep,border,useTop,append,local);
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

void MetaItem::changeAlloc(
  const Where &topOfSteps,
  const Where &bottomOfSteps,
  AllocMeta   &alloc,
  int          append)
{
  bool metaInRange;
  metaInRange = alloc.here().lineNumber != 0 && alloc.here().modelName != "undefined";

  logInfo() << "\nCHANGE ALLOC - "
            << " \nHere Model Name: "          << alloc.here().modelName
            << " \nHere Line Number: "         << alloc.here().lineNumber
            << " \nAppend: "                   << (metaInRange ? "NO" : alloc.format(false,false).contains("CALLOUT ALLOC") ? "NO" : append == 0 ? "NO" : "YES")
            << " \nMeta In Range: "            << (metaInRange ? QString("YES - Line %1").arg(alloc.format(alloc.pushed,alloc.global)) : "NO")
            << "\n -- "
            ;

  AllocEnc allocType = alloc.value();
  alloc.setValue(allocType == Vertical ? Horizontal : Vertical);

  if (metaInRange) {
      QString line = alloc.format(alloc.pushed,alloc.global);
      replaceMeta(alloc.here(),line);
    } else {
      QString line = alloc.format(false,false);
      if (line.contains("CALLOUT ALLOC"))
        append = 0;
      setMetaBottomOf(topOfSteps,bottomOfSteps,&alloc,append,false,false,false);
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

void MetaItem::appendPage(QString &meta)
{
  Where bottomOfStep = gui->topOfPages[gui->displayPageNum+1];
  if (bottomOfStep.lineNumber == gui->subFileSize(bottomOfStep.modelName)) {
    --bottomOfStep;
  }
  bool addStep = false;

  Where walk = bottomOfStep;
  int numLines = gui->subFileSize(walk.modelName);

  for (; walk < numLines && walk > 0; --walk) {
    QString line = gui->readLine(walk);
    QStringList tokens;
    split(line,tokens);

    if ((tokens.size() == 15 && tokens[0] == "1") ||
        (tokens.size() == 4  && tokens[0] == "0" && tokens[1] == "!LPUB" && tokens[2] == "INSERT" && tokens[3] == "COVER_PAGE")) {
      addStep = true;
      break;
    }

    if ((tokens.size() == 2 && tokens[0] == "0" && tokens[1] == "STEP") ||
        (tokens.size() > 2  && tokens[0] == "0" && tokens[1] == "ROTSTEP")) {
      break;
    }
  }
  walk = bottomOfStep;
  if ( ! addStep) {
    for ( ; walk < numLines; ++walk) {
      QString line = gui->readLine(walk);
      QStringList tokens;
      split(line,tokens);

      if (tokens.size() == 15 && tokens[0] == "1") {
        addStep = true;
        break;
      }
    }
  }
  beginMacro("appendPage");
  if (addStep) {
    appendMeta(bottomOfStep,"0 STEP");
    bottomOfStep++;
  }
  appendMeta(bottomOfStep,meta);
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
  QString title = QFileDialog::tr("Open Image");
  QString cwd = QDir::currentPath();
  QString filter = QFileDialog::tr("Image Files (*.png *.jpg *.jpeg *.bmp)");
  QString fileName = QFileDialog::getOpenFileName(NULL,title, cwd, filter);

  if (fileName != "") {
    QString meta = QString("0 !LPUB INSERT PICTURE \"%1\" OFFSET 0.5 0.5") .arg(fileName);
    Where topOfStep = gui->topOfPages[gui->displayPageNum-1];
    scanPastGlobal(topOfStep);
    appendMeta(topOfStep,meta);
  }
}

void MetaItem::insertText()
{
  bool ok;
  QString text = QInputDialog::getText(NULL, QInputDialog::tr("Text"),
                                             QInputDialog::tr("Input:"),
                                             QLineEdit::Normal,
                                             QString(""), &ok);
  if (ok && !text.isEmpty()) {
    QString meta = QString("0 !LPUB INSERT TEXT \"%1\" \"%2\" \"%3\"") .arg(text) .arg("Arial,36,-1,255,75,0,0,0,0,0") .arg("Black");
    Where topOfStep;

    bool multiStep = false;

    Steps *steps = dynamic_cast<Steps *>(&gui->page);
    if (steps && steps->list.size() > 0) {
      if (steps->list.size() > 1) {
        multiStep = true;
      } else {
        Range *range = dynamic_cast<Range *>(steps->list[0]);
        if (range && range->list.size() > 1) {
          multiStep = true;
        }
      }
    }

    if (multiStep) {
      topOfStep = steps->bottomOfSteps();
    } else {
      topOfStep = gui->topOfPages[gui->displayPageNum-1];
      scanPastGlobal(topOfStep);
    }
    appendMeta(topOfStep,meta);
  }
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
    Where here(gui->topLevelFile(),0);
    here.lineNumber = gui->subFileSize(here.modelName);                 //start at bottom of file
    here--;                                                             //adjust to readline using from zero-start index

    for ( ; here >= 0; here--) {                                        //scan backwards
        line = gui->readLine(here);
        rc = content.parse(line,here);

        if (rc == InsertRc) {                                           //check if insert line is Insert Model
            QRegExp InsertFinalModel("^\\s*0\\s+!LPUB\\s+.*MODEL");
            if (line.contains(InsertFinalModel)){
//                logInfo() << " \nModel detected at line: " << here.lineNumber;
                return -1;                                              //insert line exist so return -1;
            }
            continue;
        } else if (rc == StepGroupEndRc) {                              //if Step Grpup, then there is no final model
//            logInfo() << " \nOK to Insert Model after StepGroup at line: " << here.lineNumber;
            return here.lineNumber;                                     //so return line number
        } else {                                                        //no Insert or stepgroup line encountered os check for non-zero line
            QStringList tokens;
            split(line,tokens);
            bool token_1_5 = tokens.size() && tokens[0].size() == 1 &&
                 tokens[0] >= "1" && tokens[0] <= "5";
            if (token_1_5) {                                            //non-zero line detected so no back final model
                int fileSize = gui->subFileSize(here.modelName) - 1;    //adjust to readline using from zero-start index
                if (here.lineNumber < (fileSize)) {                     //check if at end of file
                    Where walkForward = here;
                    walkForward++;
                    line = gui->readLine(walkForward);
                    rc = content.parse(line,walkForward);
                    if (rc == StepRc){                                  //check if previous line was a STEP
//                    logInfo() << " \nOK to Insert Model after STEP at line: " << here.lineNumber;
                        return walkForward.lineNumber;                  // if previous line was a STEP line number after STEP
                    } else {
//                    logInfo() << " \nOK to Insert Model after Parts at line: " << here.lineNumber;
                        return here.lineNumber;                         // else return line number
                    }
                } else {
//                    logInfo() << " \nOK to Insert Model after Parts at line: " << here.lineNumber;
                    return here.lineNumber;                             // at last line so return line number
                }
            }
        }
    }
    return -1;
}


void MetaItem::insertFinalModel(int atLine)
{
  QString pageMeta  = QString("0 !LPUB INSERT PAGE");
  QString modelMeta = QString("0 !LPUB INSERT MODEL");

  if (atLine == -1){
    return;
  }

  Where here(gui->topLevelFile(),atLine);

  beginMacro("insertFinalModel");
  appendMeta(here,step);
  appendMeta(here,pageMeta);
  appendMeta(here,modelMeta);
  endMacro();
}

void MetaItem::deleteFinalModel(){

  Where here(gui->topLevelFile(),0);
  here.lineNumber = gui->subFileSize(here.modelName);
  here--;
  logTrace() << " In delete Final Model: ";
  Rc rc;
  Meta meta;
  bool foundFinalModel = false;
  for (; here >=0; here--) {                            //scan backwards until Model
      QString line = gui->readLine(here);
      rc = meta.parse(line,here);
      if (rc == InsertRc) {
          QRegExp InsertFinalModel("^\\s*0\\s+!LPUB\\s+.*MODEL");
          if (line.contains(InsertFinalModel)){
              foundFinalModel = true;
              continue;                               //model found so continue backwards until Model's begin STEP
            }
        } else if (foundFinalModel && rc == StepRc){   //at STEP...
          Where walk = here;                           //delete each line until at Model's end Step
          for(; walk >=0; walk++){
              QString line = gui->readLine(walk);
              rc = meta.parse(line,here);
              if(rc != StepRc){
                  beginMacro("deleteFinalModel");
                  deleteMeta(walk);
                  endMacro();
                } else {
                  break;
                }
            }
          break;
        } else if (rc == StepGroupEndRc){
          break;
        } else {
          QStringList tokens;
          split(line,tokens);
          bool token_1_5 = tokens.size() && tokens[0].size() == 1 &&
              tokens[0] >= "1" && tokens[0] <= "5";
          if (token_1_5) {                              //non-zero line detected so break
              break;
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
  insertPage(pageMeta);
  appendMeta(topOfStep,bomMeta);
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
        deleteMeta(topOfPage);
        endMacro();
        break;
      }
    }
  }
}

void MetaItem::deleteRotateIcon(Where &topOfStep){
  Rc rc;
  Meta meta;
  Where walk = topOfStep + 1;                                     // advance past STEP meta
  int numLines = gui->subFileSize(topOfStep.modelName);
  scanPastGlobal(topOfStep);
  QRegExp rotateIconMeta("^\\s*0\\s+!LPUB\\s+.*ROTATE_ICON");
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

void MetaItem::scanPastGlobal(
  Where &topOfStep)
{
  Where walk = topOfStep + 1;
  
  int  numLines  = gui->subFileSize(walk.modelName);
  if (walk < numLines) {
    QString line = gui->readLine(walk);
    QRegExp globalLine("^\\s*0\\s+!LPUB\\s+.*GLOBAL");
    if (line.contains(globalLine) || isHeader(line)) {
      for ( ++walk; walk < numLines; ++walk) {
        line = gui->readLine(walk);
        //logTrace() << "Scan Past GLOBAL LineNum (final -1): " << walk.lineNumber << ", Line: " << line;
        if ( ! line.contains(globalLine) && ! isHeader(line)) {
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

    logWarn() << "\n==SCAN BACKWARD READLINE==: "
              << " \nMask:StepRc = (1 << StepRc)|(1 << RotStepRc): "  << mask
              << " \nHere LINE:          " << here.lineNumber
              << " \nHere Model:         " << here.modelName
              << " \nLine:               " << line
              << " \nIs Header:          " << isHeader(line)
              << " \nParts Added:        " << partsAdded
                   ;

    if (isHeader(line)) {
      scanPastGlobal(here);

      logWarn() << "\nIN SCAN BACKWARD AT HEADER: "
                << " \nScanPastGlobal"
                << " \nRETURN EndOfFileRc at line:"  << here.lineNumber
                   ;
      return EndOfFileRc;
    }
    split(line,tokens);
    
    bool token_1_5 = tokens.size() && tokens[0].size() == 1 && tokens[0] >= "1" && tokens[0] <= "5";

    if (token_1_5) {
      partsAdded = true;

    } else {
      Rc rc = tmpMeta.parse(line,here);

      if (rc == InsertPageRc /*&& ((mask >> rc) & 1)*/) {
          logWarn() << "\nIN SCAN BACKWARD AT INSERT PAGE: "
                    << " \nRETURN InsertRc at Line:"  << here.lineNumber
                       ;
         return rc;

      } else if (rc == InsertCoverPageRc /* && ((mask >> rc) & 1)*/) {
          logWarn() << "\nIN SCAN BACKWARD AT INSERT COVER PAGE: "
                    << " \nRETURN InsertRc at Line:"  << here.lineNumber
                       ;
         return rc;

      } else if (rc == StepRc || rc == RotStepRc) {
          logWarn() << "\nIN SCAN BACKWARD AT STEP: "
                    << " \nRc == StepRc || RotStep at Line:"  << here.lineNumber
                       ;
          if (((mask >> rc) & 1) && partsAdded) {
              logWarn() << "\nIN SCAN BACKWARD AT STEP WITH PARTS: "
                        << " \nParts Added: " << partsAdded
                        << " \nRETURN StepRc|RotStepRc at Line:"  << here.lineNumber
                         ;
          return rc;
         }
        partsAdded = false;
      } else if (rc < ClearRc && ((mask >> rc) & 1)) {
          logWarn() << "\nIN SCAN BACKWARD AT CLEAR: "
                     << " \nRETURN ClearRc at Line:"  << here.lineNumber
                       ;
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
   * top stop on other sub-models, or mirror images of the same sub-model.
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
         the called out parts color B.  Then the resulatant image is
         searched for color B.  The parent model needs to be rotated
         by ROTSTEP for this to work. */
      //QPointF offset = defaultPointerTip(*meta, instances[i],isMirrored);
      QStringList argv;
      split(firstLine,argv);
      QPointF offset = defaultPointerTip(*meta,
                                         walk.modelName, firstInstance.lineNumber,
                                         modelName, i,
                                         gui->isMirrored(argv));

      QString line = QString("%1 %2") .arg(offset.x()) .arg(offset.y());

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

void MetaItem::deletePointer(const Where &here)
{
  deleteMeta(here);
}

/*
 * Algorithm for figuring out a good location for callout arrow tips.
 *
 * information we need:
 *   1. the model up to this instance of the callout
 *
 * for each part in the model {
 *   if (part is submodel) {
 *     create white version of submodel;
 *   } else {
 *     color part white
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
 
QString MetaItem::makeMonoName(const QString &fileName, QString &color)
{
  QFileInfo info(fileName);
  return info.absolutePath() + "/" + info.baseName() + "_" + color + "." + info.suffix();
}
  
int MetaItem::monoColorSubmodel(
  QString &modelName,
  QString &outFileName,
  QString &color)
{    
  QFile outFile(outFileName);
  if ( ! outFile.open(QFile::WriteOnly | QFile::Text)) {
    QMessageBox::warning(NULL, 
      QMessageBox::tr(VER_PRODUCTNAME_STR),
      QMessageBox::tr("MonoColorSubmodel cannot write file %1:\n%2.")
      .arg(outFileName)
      .arg(outFile.errorString()));
    return -1;
  }

  QTextStream out(&outFile);
  int numLines = gui->subFileSize(modelName);

  Where walk(modelName,0);
  
  for ( ; walk < numLines; walk++) {
    QString line = gui->readLine(walk);
    QStringList argv;
    
    split(line,argv);
    
    if (argv.size() == 15 && argv[0] == "1") {
      QFileInfo info(argv[14]);
      QString submodel = info.baseName();
      QString suffix = info.suffix();
      if (submodel.right(3) == "_15") {
        submodel = submodel.left(submodel.length()-3);
      } else if (submodel.right(5) == "_15_1") {
        submodel = submodel.left(submodel.length()-5);
      }
      submodel += "." + suffix;
      if (gui->isSubmodel(submodel)) {
        QString model = QDir::currentPath() + "/" + Paths::tmpDir + "/" + argv[14];
        model = makeMonoName(model,color);
        monoColorSubmodel(submodel,model,color);
        QFileInfo info(model);
        argv[14] = info.fileName();
      }
      argv[1] = color;
    } else if ((argv.size() == 8 && argv[0] == "2") ||
               (argv.size() == 11 && argv[0] == "3") ||
               (argv.size() == 14 && argv[0] == "4") ||
               (argv.size() == 14 && argv[0] == "5")) {
      argv[1] = color;
    }
    line = argv.join(" ");
    out << line << endl;
  }
  
  outFile.close();
  return 0;
}

QPointF MetaItem::defaultPointerTip(
  Meta    &meta,
  QString &modelName,
  int      lineNumber,
  const QString &subModel,
  int     instance,
  bool    isMirrored)
{
  QString white("15");
  QString blue("1");

  /*
   * Create a "white" version of the submodel that callouts out our callout
   */

  QString monoName = QDir::currentPath() + "/" + Paths::tmpDir + "/" + modelName;
  monoName = makeMonoName(monoName,white);
  monoColorSubmodel(modelName,monoName,white);

  QFile inFile(monoName);
  if ( ! inFile.open(QFile::ReadOnly | QFile::Text)) {
    QMessageBox::warning(NULL,
      QMessageBox::tr(VER_PRODUCTNAME_STR),
      QMessageBox::tr("defaultPointerTip cannot read file %1:\n%2.")
      .arg(monoName)
      .arg(inFile.errorString()));
    return QPointF(0.5,0.5);
  }

  QTextStream in(&inFile);
  QStringList csiParts;

  /*
   * Gather up the "white" parent model up to the callout
   */

  int numLines = gui->subFileSize(modelName);
  int instances = 0;
  QStringList argv;
  int i;
  QFileInfo info(subModel);
  QString monoSubModel = info.baseName() + "_" + white + "." + info.suffix();
  for (i = 0; i < numLines; i++) {
    QString line = in.readLine(0);
    if (i >= lineNumber) {

      split(line,argv);
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

    csiParts << line;
  }
  if (i == numLines) {
    return QPointF(0.5,0.5);
  }

  QString fileName = QDir::currentPath() + "/" + Paths::tmpDir + "/" + argv[14];
  fileName = makeMonoName(fileName,blue);
  QString tmodelName = info.fileName();
  monoColorSubmodel(tmodelName,fileName,blue); // create a blue version of the callout
  info.setFile(fileName);
  argv[1] = blue;
  argv[14] = info.fileName();
  csiParts << argv.join(" ");  // add blue submodel to csiParts

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
  QString imageName = QDir::currentPath() + "/" + Paths::tmpDir + "/mono.png";

  int rc = renderer->renderCsi(addLine,csiParts,imageName,meta);

  if (rc == 0) {
    QPixmap pixmap;

    pixmap.load(imageName);

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

    return QPointF(float(left)/width, float(top)/height);
  }
  return QPointF(0.5,0.5);
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

void MetaItem::writeRotateStep(QString &value)
{
    Meta content;
    Where here;
    QString modelName;
    int firstPos                = 0;
    QString prefix              = "0 ROTSTEP ";
    bool multiStep              = false;
    bool rotStep                = false;
    bool ok;

    QStringList argv = value.split(QRegExp("\\s"));
    int lineNumber = argv[firstPos].toInt(&ok);
    QString meta("%1 %2 %3 %4 %5");
    meta = meta.arg(prefix,argv[1],argv[2],argv[3],argv[4]);

    Steps *steps = dynamic_cast<Steps *>(&gui->page);
    if (steps && steps->list.size() > 0) {
        modelName = steps->modelName();
        if (steps->list.size() > 1) {
            multiStep = true;
        } else {
            Range *range = dynamic_cast<Range *>(steps->list[0]);
            if (range && range->list.size() > 1) {
                multiStep = true;
            }
        }
    }

    QString stepType = (multiStep)? "iMulti-Step " : "iSingle-Step ";
    logTrace() << "-STARTING INPUT: " << stepType << " iModel: " << modelName << "iLineNum: " << lineNumber << " iMeta (ROTSTEP): " << meta ;

    if (ok && multiStep) {                                                          //ok, we have a multi-step page

        Rc rc;
        QString line;
        here = Where(modelName,lineNumber);

        if (here.lineNumber == 0) {                                                 //if starting from [modelName]top.lineNumber because the passed in line number is 0
            int  numLines  = gui->subFileSize(here.modelName);
            scanPastGlobal(here);                                                   //scan down the page - going from low pgNum to high pgNum
            for ( ; here < numLines; here++) {
                line = gui->readLine(here);
                rc = content.parse(line,here);

                if (rc == StepRc || rc == RotStepRc || rc == StepGroupBeginRc) {
                    lineNumber = here.lineNumber;
                    break;                                                           //stop at 1st occurrence of Step/RotStep or StepGroupBegin
                } else {
                    QStringList tokens;
                    split(line,tokens);
                    bool token_1_5 = tokens.size() && tokens[0].size() == 1 &&
                         tokens[0] >= "1" && tokens[0] <= "5";
                    if (token_1_5) {
                        lineNumber = here.lineNumber;                               //traverse non-starting-zero lines
                        break;
                    }
                }
            }
        }

        line = gui->readLine(here);                                                 //Here we are not starting from [modelName]top.lineNumber
        rc = content.parse(line,here);                                              //so we read the pass-in line number and check...

        if (rc == StepRc){                                                          //if we reach a step...
            rotStep = false;
            here++;
            QString line = gui->readLine(here);
            Rc rc1 = content.parse(line,here);                                      //check the line below the see if it's  RotStep

            if (rc1 == RotStepRc){
                rotStep = true;                                                     //if it's  a RotStep we'll have to update with new value
            }
        }

    } else if (ok) {

        logTrace() << "-SS MODEL NAME      :       " << modelName;
        Where pagePosition = gui->topOfPages[gui->displayPageNum];
        Rc rc = scanBackward(pagePosition,StepMask);
        here = pagePosition;
        logTrace() << "-SS PAGE LINE NUMBER: " << pagePosition.lineNumber;

        if (rc == StepRc) {
            rotStep = false;
            here = pagePosition + 1;
            QString line = gui->readLine(here);
            Rc rc1 = content.parse(line,here);

            if (rc1 == RotStepRc){
                rotStep = true;
            } else {
                here = here - 1;
            }
            logDebug() << "-SS LINE NUMBER  :       " << here.lineNumber;
            logDebug() << "-SS RC1: " << rc1 << "   -STEP LINE: " << line;

        } else if (rc == RotStepRc) {
            rotStep = true;
        }
    }

    if (! rotStep && ! multiStep){
        logNotice() << "FIN - INSERT SINGLE PAGE STEP HERE - LINE: " << here.lineNumber;
        appendMeta(here,meta);
    }
    if (multiStep && ! rotStep){
        logNotice() << "FIN - INSERT MULTI STEP PAGE HERE - LINE:  " << here.lineNumber;
        insertMeta(here,meta);
    }
    if (rotStep){
        logNotice() << "FIN - REPLACE ROTATE STEP HERE AT LINE: " << here.lineNumber;
        replaceMeta(here,meta);
    }
}

