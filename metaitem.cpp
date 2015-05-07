 
/****************************************************************************
**
** Copyright (C) 2007-2009 Kevin Clague. All rights reserved.
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
#include "pairdialog.h"
#include "scaledialog.h"
#include "borderdialog.h"
#include "backgrounddialog.h"
#include "dividerdialog.h"
#include "paths.h"
#include "render.h"
#include "version.h"

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
    beginMacro("submodelIsPart");
    insertMeta(here,      "0 !LPUB PART END");
    insertMeta(here,      "0 !LPUB PLI END");
    insertMeta(calledOut, "0 !LPUB PART BEGIN IGN");
    insertMeta(calledOut, "0 !LPUB PLI BEGIN SUB " + tokens[14] + " " + tokens[1]);
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
  if (useTop) {
    setMetaTopOf(topOf,bottomOf,meta,append,local,askLocal,global);
  } else {
    setMetaBottomOf(topOf,bottomOf,meta,append,local,askLocal,global);
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
      local = LocalDialog::getLocal(VER_PRODUCTNAME_STR, "Change only this step?",gui);
    }
    QString line = meta->format(local,global);

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
    insertMeta(topOfFile, line);
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

  if (metaInRange) {
    QString line = meta->format(meta->pushed,meta->global);
    replaceMeta(meta->here(),line);
  } else {
    if (askLocal) {
      local = LocalDialog::getLocal(VER_PRODUCTNAME_STR, "Change only this step?",gui);
    }
    QString line = meta->format(local, global);

    int  numLines = gui->subFileSize(topOf.modelName);
    bool eof = bottomOf.lineNumber + 1 == numLines;
    
    if (eof) {
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
  bool           useLocal) 
{
  PlacementData placementData = placement->value();
  bool ok;
  ok = PlacementDialog
       ::getPlacement(parentType,relativeType,placementData,title);

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
  bool           useLocal)
{
  PlacementData placementData = placement->value();
  bool ok;
  ok = PlacementDialog
       ::getPlacement(parentType,relativeType,placementData,title,NULL,pliPerStep);

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
  QString newMetaString = placement->format(local,global);

  Where walk = defaultWhere + 1;
  if (placement->here().modelName == "undefined") {    
    Where walk = defaultWhere + 1;

    bool partsAdded;

    if (scanBackward(walk,StepMask,partsAdded) == EndOfFileRc) {
      defaultWhere = firstLine(defaultWhere.modelName);
    }
    if (type == StepGroupType) {
      scanForward(defaultWhere,StepGroupBeginMask);
    } else if (type == CalloutType) {
      scanForward(defaultWhere,CalloutEndMask);
      --defaultWhere;
    } else if (defaultWhere.modelName == gui->topLevelFile()) {
      scanPastGlobal(defaultWhere);
    }
    appendMeta(defaultWhere,newMetaString);
  } else {
    replaceMeta(placement->here(),newMetaString);
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
  bool           local)
{
  bool ok;
  QString fontName = font->value();
  QString fontName2;
  QFont _font(fontName);

  QFont newFont;

  newFont = QFontDialog::getFont(&ok, _font, gui);
  if ( ! ok) {
    return;
  }

  fontName2 = newFont.toString();

  font->setValue(fontName2);
  setMetaTopOf(topOfStep,bottomOfStep,font,append,local);
}

void MetaItem::changeColor(
  const Where &topOfStep,
  const Where &bottomOfStep,
  StringMeta *color,
  int         append,
  bool        local)
{
  QColor _color = LDrawColor::color(color->value());
  _color = QColorDialog::getColor(_color,NULL);

  if (_color.isValid()) {
    color->setValue(_color.name());
    setMetaTopOf(topOfStep,bottomOfStep,color,append,local);
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

void MetaItem::changeBorder(
  QString      title,
  const Where &topOfStep,
  const Where &bottomOfStep,
  BorderMeta  *border,
  bool         useTop,
  int          append,
  bool         local)
{
  BorderData borderData = border->value();
  bool ok = BorderDialog::getBorder(borderData,title);

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
  AllocEnc allocType = alloc.value();
  alloc.setValue(allocType == Vertical ? Horizontal : Vertical);
  setMetaBottomOf(topOfSteps,bottomOfSteps,&alloc,append,false,false,false);
}

/********************************************************************
 *
 * Insert cover and non-cover pages
 *
 ********************************************************************/

bool MetaItem::okToInsertCoverPage()
{
  bool frontCover = gui->displayPageNum <= gui->firstStepPageNum;
  bool backCover  = gui->displayPageNum >    gui->lastStepPageNum;;

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
  QString meta = "0 !LPUB INSERT COVER_PAGE";
  insertPage(meta);
}
void MetaItem::appendCoverPage()
{
  QString meta = "0 !LPUB INSERT COVER_PAGE";
  appendPage(meta);
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

    if (tokens.size() == 15 && tokens[0] == "1" ||
        tokens.size() == 4  && tokens[0] == "0" && tokens[1] == "!LPUB" && tokens[2] == "INSERT" && tokens[3] == "COVER_PAGE") {
      addStep = true;
      break;
    }

    if (tokens.size() == 2 && tokens[0] == "0" && tokens[1] == "STEP" ||
        tokens.size() > 2  && tokens[0] == "0" && tokens[1] == "ROTSTEP") {
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

/***************************************************************************/

void MetaItem::scanPastGlobal(
  Where &topOfStep)
{
  Where walk = topOfStep + 1;
  
  int  numLines  = gui->subFileSize(walk.modelName);
  if (walk < numLines) {
    QString line = gui->readLine(walk);
    QRegExp globalLine("^\\s*0\\s+!LPUB\\s+.*GLOBAL");
    if (line.contains(globalLine)) {
      for ( ++walk; walk < numLines; ++walk) {
        line = gui->readLine(walk);
        if ( ! line.contains(globalLine)) {
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

    bool token_1_5 = tokens.size() && tokens[0].size() == 1 && 
         tokens[0] >= "1" && tokens[0] <= "5";

    if (token_1_5) {
      partsAdded = true;
    } else {
      Rc rc = tmpMeta.parse(line,here);
      
      if (rc == InsertRc && ((mask >> rc) & 1)) {
        // return rc;
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
  
  partsAdded = false;

  for ( ; here >= 0; here--) {

    QString line = gui->readLine(here);
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
      if (rc == StepRc || rc == RotStepRc) {
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
    } else if (argv.size() == 8 && argv[0] == "2" ||
               argv.size() == 11 && argv[0] == "3" ||
               argv.size() == 14 && argv[0] == "4" ||
               argv.size() == 14 && argv[0] == "5") {
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

    if (ok && multiStep) {

        Rc rc;
        QString line;
        here = Where(modelName,lineNumber);

        if (here.lineNumber == 0) {
            int  numLines  = gui->subFileSize(here.modelName);
            scanPastGlobal(here);
            for ( ; here < numLines; here++) {
                line = gui->readLine(here);
                rc = content.parse(line,here);
                if (rc == StepRc || rc == RotStepRc || rc == StepGroupBeginRc) {
                    lineNumber = here.lineNumber;
                    break;
                } else {
                    QStringList tokens;
                    split(line,tokens);
                    bool token_1_5 = tokens.size() && tokens[0].size() == 1 &&
                            tokens[0] >= "1" && tokens[0] <= "5";
                    if (token_1_5) {
                        lineNumber = here.lineNumber;
                        break;
                    }
                }
            }
        }

        line = gui->readLine(here);
        rc = content.parse(line,here);

        if (rc == StepRc){
            rotStep = false;
            here++;
            QString line = gui->readLine(here);
            Rc rc1 = content.parse(line,here);

            if (rc1 == RotStepRc){
                rotStep = true;
            }
        }

    } else if (ok) {

        Where pagePosition = gui->topOfPages[gui->displayPageNum];
        Rc rc = scanBackward(pagePosition,StepMask);
        here = pagePosition;

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

        } else if (rc == RotStepRc) {
            rotStep = true;
        }
    }

    if (! rotStep && ! multiStep){
        appendMeta(here,meta);
    }
    if (multiStep && ! rotStep){
        insertMeta(here,meta);
    }
    if (rotStep){
        replaceMeta(here,meta);
    }
}
