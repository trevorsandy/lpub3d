
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
 * The traverse function is the one function that traverses the LDraw model
 * higherarchy seaching for pages to render.  It tracks the partial assembly
 * contents, parts list contents, step group contents, and callouts.
 *
 * It can count pages in the design, gather page contents for translation
 * into graphical representation of pages for the user.  In the future it
 * will gather Bill of Materials contents.
 *
 * Please see lpub.h for an overall description of how the files in LPub
 * make up the LPub program.
 *
 ***************************************************************************/

#include "lpub.h"
#include <QtWidgets>
#include <QGraphicsItem>
#include <QString>
#include <QFileInfo>
#include "lpub_preferences.h"
#include "ranges.h"
#include "callout.h"
#include "pointer.h"
#include "range.h"
#include "reserve.h"
#include "step.h"
#include "paths.h"
#include "metaitem.h"
#include "pointer.h"
#include "pagepointer.h"
#include "ranges_item.h"
#include "separatorcombobox.h"

#include "QsLog.h"

// Set to enable PageSize trace logging
#ifndef SIZE_DEBUG
//#define SIZE_DEBUG
#endif

#define FIRST_STEP 1
#define FIRST_PAGE 1

static QString AttributeNames[] =
{
    "Line",
    "Border"
};

static QString PositionNames[] =
{
    "BASE_TOP",
    "BASE_BOTTOM",
    "BASE_LEFT",
    "BASE_RIGHT"
};

//struct PAMItem
//{
//    PointerAttribMeta pam;
//    Positions pos;
//};

/*********************************************
 *
 * remove_group
 *
 * this removes members of a group from the
 * ldraw file held in the the ldr string
 *
 ********************************************/
static bool enableLineTypeIndexes;

static void remove_group(
    QStringList  in,     // csiParts
    QVector<int> tin,    // typeIndexes
    QString      group,  // steps->meta.LPub.remove.group.value()
    QStringList  &out,   // newCSIParts
    QVector<int> &tiout, // newTypeIndexes
    Meta         &meta)
{

  bool    grpMatch = false;
  int     grpLevel = 0;
  Rc      grpType;
  QRegExp grpRx;

  if (in.size() != tin.size()) {
      enableLineTypeIndexes = false;
      QString message(QString("CSI part list size [%1] does not match line index size [%2]. Line type indexes disabled.")
                              .arg(in.size()).arg(tin.size()));
      emit gui->messageSig(LOG_NOTICE, message);
  }

  for (int i = 0; i < in.size(); i++) {

      QString line = in.at(i);
      grpRx = meta.groupRx(line,grpType);

      if (!grpRx.isEmpty() && grpType) {
          // MLCad Groups
          if (grpType == MLCadGroupRc) {
             if (grpRx.cap(grpRx.captureCount()) == group) {
               i++;
             } else {
               out << line;
               tiout << tin.at(i);
             }
          }
          // LDCad Groups
          else
          if (grpType == LDCadGroupRc) {
              QStringList lids = grpRx.cap(grpRx.captureCount()).split(" ");
              if (lids.size() && gui->ldcadGroupMatch(group,lids)) {
                  i++;
              } else {
                  out << line;
                  tiout << tin.at(i);
              }
          }
          // LeoCAD	Group Begin
          else
          if (grpType == LeoCadGroupBeginRc) {
              if ((grpRx.cap(grpRx.captureCount()) == group)){
                grpMatch = true;
                i++;
              }
              else
              if (grpMatch) {
                  grpLevel++;
                  i++;
              }
              else {
                 out << line;
                 tiout << tin.at(i);
              }
          }
          // LeoCAD	Group End
          else
          if (grpType == LeoCadGroupEndRc) {
              if (grpMatch) {
                  if (grpLevel == 0) {
                    grpMatch = false;
                  } else {
                    grpLevel--;
                  }
              }
              else {
                  out << line;
                  tiout << tin.at(i);
              }
          }
          else
          if (grpMatch) {
                 i++;
          }
          else {
             out << line;
             tiout << tin.at(i);
          }
          // End groups
      }
    }

  return;
}

/*********************************************
 *
 * remove_part
 *
 * this removes members of a part from the
 * ldraw file held in the the ldr string
 *
 ********************************************/

static void remove_parttype(
    QStringList   in,    // csiParts
    QVector<int>  tin,   // typeIndexes
    QString       model, // part type
    QStringList  &out,   // newCSIParts
    QVector<int> &tiout) // newTypeIndexes
{

  model = model.toLower();

  if (in.size() != tin.size()) {
      enableLineTypeIndexes = false;
      QString message(QString("CSI part list size [%1] does not match line index size [%2]. Line type indexes disabled.")
                              .arg(in.size()).arg(tin.size()));
      emit gui->messageSig(LOG_NOTICE, message);
  }

  for (int i = 0; i < in.size(); i++) {
      QString line = in.at(i);
      QStringList tokens;

      split(line,tokens);

      if (tokens.size() == 15 && tokens[0] == "1") {
          QString type = tokens[14].toLower();
          if (type != model) {
              out << line;
              tiout << tin.at(i);
            }
        } else {
          out << line;
          tiout << tin.at(i);
        }
    }

  return;
}

/*********************************************
 *
 * remove_name
 *
 ********************************************/

static void remove_partname(
    QStringList   in,    // csiParts
    QVector<int>  tin,   // typeIndexes
    QString       name,  // partName
    QStringList  &out,   // newCSIParts
    QVector<int> &tiout) // newCSIParts
{
  name = name.toLower();

  if (in.size() != tin.size()) {
      enableLineTypeIndexes = false;
      QString message(QString("CSI part list size [%1] does not match line index size [%2]. Line type indexes disabled.")
                              .arg(in.size()).arg(tin.size()));
      emit gui->messageSig(LOG_NOTICE, message);
  }

  for (int i = 0; i < in.size(); i++) {
      QString line = in.at(i);
      QStringList tokens;

      split(line,tokens);

      if (tokens.size() == 4 && tokens[0] == "0" &&
          (tokens[1] == "!LPUB" || tokens[1] == "LPUB") &&
          tokens[2] == "NAME") {
          QString type = tokens[3].toLower();
          if (type == name) {
              for ( ; i < in.size(); i++) {
                  line = in.at(i);
                  split(line,tokens);
                  if (tokens.size() == 15 && tokens[0] == "1") {
                      break;
                    } else {
                      out << line;
                      tiout << tin.at(i);
                    }
                }
            } else {
              out << line;
              tiout << tin.at(i);
            }
        } else {
          out << line;
          tiout << tin.at(i);
        }
    }

  return;
}

/*********************************************
 *
 * set_divider_pointers
 *
 * this processes step_group or callout divider
 * pointers and pointer attributes
 *
 ********************************************/

static void set_divider_pointers(
        Meta &curMeta,
        Where &current,
        Range *range,
        LGraphicsView *view,
        DividerType dividerType,
        int stepNum,
        Rc rct){

    QString sType = (rct == CalloutDividerRc ? "CALLOUT" : "STEPGROUP");

    Rc pRc  = (rct == CalloutDividerRc ? CalloutDividerPointerRc :
                                         StepGroupDividerPointerRc);
    Rc paRc = (rct == CalloutDividerRc ? CalloutDividerPointerAttribRc :
                                         StepGroupDividerPointerAttribRc);
    PointerAttribMeta pam = (rct == CalloutDividerRc ? curMeta.LPub.callout.divPointerAttrib :
                                                       curMeta.LPub.multiStep.divPointerAttrib);

    Where walk(current.modelName,current.lineNumber);
    walk++;

    int numLines = gui->subFileSize(walk.modelName);

    bool rd = dividerType == RangeDivider;

    int sn  = stepNum - 1; // set the previous STEP's step number

    for ( ; walk.lineNumber < numLines; walk++) {
        QString stepLine = gui->readLine(walk);
        Rc mRc = curMeta.parse(stepLine,walk);
        if (mRc == StepRc || mRc == RotStepRc) {
            break;
        } else if (mRc == pRc) {
            PointerMeta pm = (rct == CalloutDividerRc ? curMeta.LPub.callout.divPointer :
                                                        curMeta.LPub.multiStep.divPointer);
            range->appendDividerPointer(walk,pm,pam,view,sn,rd);
        } else if (mRc == paRc) {
            QStringList argv;
            split(stepLine,argv);
            pam.setValueInches(pam.parseAttributes(argv,walk));
            Pointer          *p = nullptr;
            int i               = pam.value().id - 1;
            int validIndex      = rd ? range->rangeDividerPointerList.size() - 1 :
                                       range->stepDividerPointerList.size() - 1; /*0-index*/
            if (i <= validIndex) {
                p = rd ? range->rangeDividerPointerList[i] :
                         range->stepDividerPointerList[i];
            } else {
                gui->parseError(QString("Invalid Divider pointer attribute index.<br>"
                                        "Expected value &#60;= %1, received %2")
                                        .arg(validIndex).arg(i),current);
                break;
            }
            if (p && pam.value().id == p->id) {
                pam.setAltValueInches(p->getPointerAttribInches());
                p->setPointerAttribInches(pam);
                if (rd)
                    range->rangeDividerPointerList.replace(i,p);
                else
                    range->stepDividerPointerList.replace(i,p);
            }
        }
    }
}

/*
 * This function, drawPage, is handed the parse state going into the page
 * that is to be displayed.  It gathers up a step group, or a single step,
 * including any called out models (think recursion), but ignores non-called
 * out submodels.  It stops parsing the LDraw files when it hits end of
 * page, at which point, it calls a function to convert the parsed and
 * retained results into Qt GraphicsItems for display to the user.
 *
 * This drawPage function is only called by the findPage function.  The findPage
 * function and this drawPage function used to be one function, but doing
 * this processing in one function was problematic.  The design issue is that
 * at start of step, or multistep, you do not know the page number, because
 * the step could contain submodels that are not called out, which produce at
 * least one page each.
 *
 * findPage (is below this drawPage function in this file), is lightweight
 * in that it is much smaller that the original combined function traverse.
 * Its design goal is to find the page the user wants displayed, and present
 * the parse state of the start of page to this function drawPage.
 *
 * depends on the current page number of the parse, and the page number the
 * user wants to see.  If the current page number is lower than the "display"
 * page number, the state of meta, the parts in the submodel, the filename
 * and linenumber of the first line of page is saved.  When findPage hits end
 * of page for the "display" page, it hands the saved start of page state to
 * drawPage.  drawPage parses from start of page, creating a tree of data
 * structures representing the content of the page.  At end of page, the
 * tree is converted into Qt GraphicsItems for display.
 *
 * One thing to note is that findPage does the bulk of the LDraw file parsing
 * and is as lightweight (e.g. small) as I could make it.  Since callouts do
 * not have pages of their own (they are on the page of their parent step),
 * findPage ignores callouts.  Since findPage deals with non-callout submodels,
 * drawPage ignores non-called out submodels, and only deals with callout
 * submodels.
 *
 * After drawPage finishes gathering the page and converting the tree to
 * graphics items, it returns to findPage, which discards the parse state,
 * but continues parsing through to the last page, so we know how many pages
 * are in the building instructions.
 *
 */

Range *newRange(
    Steps  *steps,
    bool    calledOut)
{
  Range *range;

  if (calledOut) {
      range = new Range(steps,
                        steps->meta.LPub.callout.alloc.value(),
                        steps->meta.LPub.callout.freeform);
    } else {
      range = new Range(steps,
                        steps->meta.LPub.multiStep.alloc.value(),
                        steps->meta.LPub.multiStep.freeform);
    }
  return range;
}

int Gui::drawPage(
    LGraphicsView  *view,
    LGraphicsScene *scene,
    Steps          *steps,
    QString const   &addLine,
    DrawPageOptions &opts)
{
  emit messageSig(LOG_INFO_STATUS, QString("Processing draw %1, model [%2]...")
                  .arg(stepContains(opts.current,"MULTI_STEP BEGIN") ?
                           QString("multi-step page %1").arg(displayPageNum) :
                           QString("single-step page %1, step %2").arg(displayPageNum).arg(opts.stepNum))
                  .arg(opts.current.modelName));

  QApplication::processEvents();

  QElapsedTimer pageRenderTimer;
  pageRenderTimer.start();

  QRegExp partLineRx("^\\s*1|\\bBEGIN SUB\\b");
  QStringList configuredCsiParts; // fade and highlight configuration
  QString  line, csiName;
  Callout *callout         = nullptr;
  Range   *range           = nullptr;
  Step    *step            = nullptr;

  bool     global          = true;
  bool     pliIgnore       = false;
  bool     partIgnore      = false;
  bool     synthBegin      = false;
  bool     multiStep       = false;
  bool     partsAdded      = false;
  bool     coverPage       = false;
  bool     bfxStore1       = false;
  bool     bfxLoad         = false;
  bool     firstStep       = true;
  bool     noStep          = false;
  bool     rotateIcon      = false;
  bool     assemAnnotation = false;
  bool     displayCount    = false;
  bool     previewNotPerStep = false;
  int      countInstances  = steps->meta.LPub.countInstance.value();

  // Build mod update flags
  Rc   buildModAction    = BuildModApplyRc;
  bool buildModItems     = false;
  bool buildModIgnore    = false;
  bool buildModPliIgnore = false;
  bool buildMod[3]       = { false, false, false };

  QVector<int>  buildModLineTypeIndexes;
  QStringList   buildModCsiParts;
  QString       buildModKey;

  QMap<int, Rc> buildModActions;
  QMap<int, QString> buildModKeys;
  QMap<int, QVector<int>> buildModAttributes;

  DividerType dividerType  = NoDivider;

  PagePointer *pagePointer = nullptr;
  QMap<Positions, PagePointer *> pagePointers;
  QMap<Where, SceneObjectData> selectedSceneItems;

  QList<InsertMeta> inserts;
  QMap<QString, LightData> lightList;
  QStringList calloutParts;

  Where topOfStep = opts.current;
  steps->setTopOfSteps(topOfStep/*opts.current*/);
  steps->isMirrored = opts.isMirrored;
  page.coverPage = false;

  Rc gprc = OkRc;
  Rc rc;
  int retVal = 0;

  // include file vars
  Where includeHere;
  Rc includeFileRc        = EndOfFileRc;
  bool inserted           = false;
  bool resetIncludeRc     = false;

  // set page header/footer width
  float pW;
  int which;
  if (callout){
      which = callout->meta.LPub.page.orientation.value() == Landscape ? 1 : 0;
      pW = callout->meta.LPub.page.size.value(which);
      callout->meta.LPub.page.pageHeader.size.setValue(0,pW);
      callout->meta.LPub.page.pageFooter.size.setValue(0,pW);
    } else {
      which = steps->meta.LPub.page.orientation.value() == Landscape ? 1 : 0;
      pW = steps->meta.LPub.page.size.value(which);
      steps->meta.LPub.page.pageHeader.size.setValue(0,pW);
      steps->meta.LPub.page.pageFooter.size.setValue(0,pW);
    }

  auto getTopOfPreviousStep = [this,&topOfStep] () {
      int adjustedIndx = exporting() ? displayPageNum : displayPageNum - 1;
      int displayPageIndx = adjustedIndx ? adjustedIndx-- : adjustedIndx; // top of 1 step back
      bool displayPageIndxOk = topOfPages.size() && topOfPages.size() >= displayPageIndx;
      Where top = displayPageIndxOk ? topOfPages[displayPageIndx] : topOfStep;
      return top;
  };

  auto drawPageElapsedTime = [this, &partsAdded, &pageRenderTimer](){
    QString pageRenderMessage = QString("%1 ").arg(VER_PRODUCTNAME_STR);
    if (!page.coverPage && partsAdded) {
      pageRenderMessage += QString("using %1 ").arg(Render::getRenderer());
      QString renderAttributes;
      if (Render::getRenderer() == RENDERER_LDVIEW) {
        if (Preferences::enableLDViewSingleCall)
          renderAttributes += QString("Single Call");
        if (Preferences::enableLDViewSnaphsotList)
          renderAttributes += QString(", Snapshot List");
      }
      if (!renderAttributes.isEmpty())
        pageRenderMessage += QString("(%1) ").arg(renderAttributes);
      pageRenderMessage += QString("render ");
    }
    pageRenderMessage += QString("rendered page %1. %2")
                                 .arg(displayPageNum)
                                 .arg(elapsedTime(pageRenderTimer.elapsed()));
    emit messageSig(LOG_TRACE, pageRenderMessage);
  };

  auto insertAttribute =
          [this,
           &opts,
           &topOfStep] (
          QMap<int, QVector<int>> &buildModAttributes,
          int index, const Where &here)
  {
      int  buildModLevel = opts.buildModLevel;
      int  fileNameIndex = getSubmodelIndex(topOfStep.modelName);
      QMap<int, QVector<int>>::iterator i = buildModAttributes.find(buildModLevel);
      if (i == buildModAttributes.end()) {
          QVector<int> modAttributes = { 0, 0, 0, 1/*placeholder*/, fileNameIndex, 0/*placeholder*/ };
          modAttributes[index] = here.lineNumber;
          buildModAttributes.insert(buildModLevel, modAttributes);
      } else {
          i.value()[index] = here.lineNumber;
      }
  };

  auto insertBuildModification =
          [this,
          &buildModAttributes,
          &buildModKeys,
          &buildModActions,
          &topOfStep] (int buildModLevel)
  {
      int buildModStepIndex = getBuildModStepIndex(topOfStep);
      int fileNameIndex     = getSubmodelIndex(topOfStep.modelName);
      int modAction         = buildModActions.value(buildModLevel);
      int lineNumber        = topOfStep.lineNumber;
      QString buildModKey   = buildModKeys.value(buildModLevel);

      QString modStepKey = QString("%1;%2;%3")
                                   .arg(fileNameIndex)
                                   .arg(lineNumber)
                                   .arg(1/*placeholder*/);

      QVector<int> modAttributes = { 0, 0, 0, 1, -1, 0 };
      QMap<int, QVector<int>>::iterator i = buildModAttributes.find(buildModLevel);
      if (i != buildModAttributes.end())
          modAttributes = i.value();

      insertBuildMod(buildModKey,
                     modStepKey,
                     modAttributes,
                     modAction,
                     buildModStepIndex);
#ifdef QT_DEBUG_MODE
      emit messageSig(LOG_DEBUG, QString(
                      "DrawPage Insert BuildMod StepIndx: %1, "
                      "Action: %2, "
                      "Attributes: %3 %4 %5 1* %6 0*, "
                      "StepKey: %7, "
                      "ModKey: %8, "
                      "Level: %9, *=placeholder")
                      .arg(buildModStepIndex)
                      .arg(modAction == BuildModApplyRc ? "Apply" : "Remove")
                      .arg(modAttributes.at(BM_BEGIN_LINE_NUM))
                      .arg(modAttributes.at(BM_ACTION_LINE_NUM))
                      .arg(modAttributes.at(BM_END_LINE_NUM))
                      .arg(fileNameIndex)
                      .arg(modStepKey)
                      .arg(buildModKey)
                      .arg(buildModLevel));
#endif
  };

  PartLineAttributes pla(
     opts.csiParts,
     opts.lineTypeIndexes,
     buildModCsiParts,
     buildModLineTypeIndexes,
     opts.buildModLevel,
     buildModIgnore,
     buildModItems);

  /*
   * do until end of page
   */
  int numLines = ldrawFile.size(opts.current.modelName);

  for ( ; opts.current <= numLines; opts.current++) {

      // if reading include file, return to current line, do not advance

      if (includeFileRc != EndOfFileRc) {
         opts.current.lineNumber--;
      }

      // load initial meta values

      /*
       * For drawPage(), the BuildMod behaviour performs two functions:
       * Funct 1. Capture the appropriate 'block' of lines to be written to the csiPart list (partIgnore).
       * Funct 2. Write the appropriate part lines to the pliPart list. (pliIgnore)
       *
       * The buildModLevel flag is enabled for the lines between BUILD_MOD BEGIN and BUILD_MOD END
       * Lines between BUILD_MOD BEGIN and BUILD_MOD END_MOD represent the modified content
       * Lines between BUILD_MOD END_MOD and BUILD_MOD END represent the original content
       *
       * When the buildModLevel flag is true (greater than 0), and:
       * Funct 1 (csiParts):
       * Parse is enabled when 'buildModIgnore' is false.
       * When the build mod action is 'apply', the modification block is parsed. (buildModIgnore is false)
       * When the build mod action is 'remove', the default block is parsed.     (buildModIgnore is false)
       * Remove group, partType and partName is only applied when 'buildModIgnore is false.
       *
       * Funct 2 (pliParts):
       * Process pliParts is enabled when 'buildModPliIgnore' is false.
       * When the build mod action is 'apply', the modification block is parsed but pliParts are
       * not written to the pliList (buildModPliIgnore is true).
       * When the build mod action is 'remove', the defaul block is parsed and the pliParts are
       * written to the pliList (buildModPliIgnore false false)
       *
       * When the build mod meta command is BUILD_MOD END 'buildModIgnore' and 'buildModPliIgnore'
       * are reset to false while buildModLevel is reset to false (0) if the build mod command is not nested
       *
       * BUILD_MOD APPLY or BUILD_MOD REMOVE action meta commands are ignored as they are set in findPage()
       *
       * When the buildModLevel flag is false (0), pli and csi lines are processed normally
       */

      Meta   &curMeta = callout ? callout->meta : steps->meta;

      QStringList tokens;

      // If we hit end of file we've got to note end of step

      if (opts.current >= numLines) {
          line.clear();
          gprc = EndOfFileRc;
          tokens << "0";

          // not end of file, so get the next LDraw line

        } else {

          // read the line from the ldrawFile db

          line = ldrawFile.readLine(opts.current.modelName,opts.current.lineNumber);
          split(line,tokens);
        }

      // STEP - Process part type
      if (tokens.size() == 15 && tokens[0] == "1") {

          QString color = tokens[1];
          QString type  = tokens[tokens.size()-1];

          if (color == "16") {
              QStringList addTokens;
              split(addLine,addTokens);
              if (addTokens.size() == 15) {
                  tokens[1] = addTokens[1];
                }
              line = tokens.join(" ");
              color = tokens[1];
            }

          if (! buildModIgnore){

              CsiItem::partLine(line,pla,opts.current.lineNumber,OkRc);
              partsAdded = true;

              // STEP - Allocate STEP

              /* since we have a part usage, we have a valid STEP */

              if (step == nullptr  && ! noStep) {
                  if (range == nullptr) {
                      range = newRange(steps,opts.calledOut);
                      steps->append(range);
                    }

                  step = new Step(topOfStep,
                                  range,
                                  opts.stepNum,
                                  curMeta,
                                  opts.calledOut,
                                  multiStep);

                  range->append(step);
              }
          }

          // STEP - Allocate PLI

          /* check if part is on excludedPart.lst and set pliIgnore*/

          if (ExcludedParts::hasExcludedPart(type))
              pliIgnore = true;

          /* addition of ldraw parts */

          if (curMeta.LPub.pli.show.value()
              && ! pliIgnore
              && ! partIgnore
              && ! buildModPliIgnore
              && ! synthBegin) {
              QString colorType = color+type;

              if (! isSubmodel(type) || curMeta.LPub.pli.includeSubs.value()) {

                  /*  check if alternative part exist and replace */
                  if(PliSubstituteParts::hasSubstitutePart(type)) {

                      QStringList substituteToken;
                      split(line,substituteToken);
                      QString substitutePart = type;

                      if (PliSubstituteParts::getSubstitutePart(substitutePart)){
                          substituteToken[substituteToken.size()-1] = substitutePart;
                        }

                      line = substituteToken.join(" ");
                    }

                  if (opts.bfxStore2 && bfxLoad) {
                      for (int i = 0; i < opts.bfxParts.size(); i++) {
                          if (opts.bfxParts[i] == colorType) {
                              opts.bfxParts.removeAt(i);
                              break;
                            }
                        }

                      // Danny: the following condition should help LPUB to remove automatically from PLI the parts in the buffer,
                      // but does not work if two buffers are used one after another in a multi step page.
                      // Better to make the user use the !LPUB PLI BEGIN IGN / END

                      //if ( ! removed )  {
                      opts.pliParts << Pli::partLine(line,opts.current,steps->meta);
                      //}
                    } else {

                      opts.pliParts << Pli::partLine(line,opts.current,steps->meta);
                    }
                }

              // bfxStore1 goes true when we've seen BFX STORE the first time
              // in a sequence of steps.  This used to be commented out which
              // means it didn't work in some cases, but we need it in step
              // group cases, so.... bfxStore1 && multiStep (was just bfxStore1)
              if (bfxStore1 && (multiStep || opts.calledOut)) {
                  opts.bfxParts << colorType;
                }
            } // STEP - Process shown PLI parts

          /* if it is a called out sub-model, then process it */

          if (ldrawFile.isSubmodel(type) && callout && ! noStep && ! buildModIgnore) {

              CalloutBeginMeta::CalloutMode calloutMode = callout->meta.LPub.callout.begin.value();

//              logDebug() << "CALLOUT MODE: " << (calloutMode == CalloutBeginMeta::Unassembled ? "Unassembled" :
//                                                 calloutMode == CalloutBeginMeta::Rotated ? "Rotated" : "Assembled");

              /* we are a callout, so gather all the steps within the callout */
              /* start with new meta, but no rotation step */

              QString thisType = type;

             /* t.s. Rotated or assembled callout here (treated like a submodel) */
              if ((opts.assembledCallout = calloutMode != CalloutBeginMeta::Unassembled)) {

                  /* So, we process these callouts in-line, not when we finally hit the STEP or
                     ROTSTEP that ends this processing, but for ASSEMBLED or ROTATED
                     callouts, the ROTSTEP state affects the results, so we have to search
                     forward until we hit STEP or ROTSTEP to know how the submodel might
                     want to be rotated.  Also, for submodel's who's scale is different
                     than their parent's scale, we want to scan ahead and find out the
                     parent's scale and "render" the submodels at the parent's scale */

                  Meta tmpMeta = curMeta;
                  Where walk = opts.current;
                  for (++walk; walk < numLines; ++walk) {
                      QStringList tokens;
                      QString scanLine = ldrawFile.readLine(walk.modelName,walk.lineNumber);
                      split(scanLine,tokens);
                      if (tokens.size() > 0 && tokens[0] == "0") {
                          Rc rc = tmpMeta.parse(scanLine,walk,false);
                          if (rc == StepRc || rc == RotStepRc) {
                              break;
                            }
                        }
                    }

                  if (calloutMode == CalloutBeginMeta::Rotated) {
                      // When renderers apply CA rotation, set cameraAngles to 0 so only ROTSTEP is sent to renderers.
                      if (! Preferences::applyCALocally)
                          callout->meta.LPub.assem.cameraAngles.setValues(0.0, 0.0);
                  }
                  callout->meta.rotStep = tmpMeta.rotStep;
                  callout->meta.LPub.assem.modelScale = tmpMeta.LPub.assem.modelScale;
                  // The next command applies the rotation due to line, but not due to callout->meta.rotStep
                  thisType = callout->wholeSubmodel(callout->meta,type,line,0);
                }

              if (callout->bottom.modelName != thisType) {

                  Where current2(thisType,0);
                  skipHeader(current2);
                  if (calloutMode == CalloutBeginMeta::Assembled) {
                      // In this case, no additional rotation should be applied to the submodel
                      callout->meta.rotStep.clear();
                  }
                  SubmodelStack tos(opts.current.modelName,opts.current.lineNumber,opts.stepNum);
                  callout->meta.submodelStack << tos;
                  Meta saveMeta = callout->meta;
                  callout->meta.LPub.pli.constrain.resetToDefault();

                  step->append(callout);

                  calloutParts.clear();

                  QStringList csiParts2;
                  QVector<int> lineTypeIndexes2;

                  QHash<QString, QStringList> calloutBfx;
                  QHash<QString, QVector<int>> calloutBfxLineTypeIndexes;

                  DrawPageOptions calloutOpts(
                              current2,           /*where*/
                              csiParts2,          /*CSI parts*/
                              calloutParts,       /*PLI parts*/
                              opts.bfxParts,
                              opts.ldrStepFiles,
                              opts.csiKeys,
                              calloutBfx,
                              opts.pliPartGroups,
                              lineTypeIndexes2,
                              calloutBfxLineTypeIndexes,
                              1                   /*stepNum*/,
                              opts.groupStepNumber,
                              ldrawFile.mirrored(tokens),
                              opts.printing,
                              opts.buildModLevel,
                              opts.bfxStore2,
                              opts.assembledCallout,
                              true               /*calledOut*/
                              );
                  int rc = drawPage(view, scene, callout, line, calloutOpts);

                  callout->meta = saveMeta;

                  if (callout->meta.LPub.pli.show.value() &&
                      ! callout->meta.LPub.callout.pli.perStep.value() &&
                      ! pliIgnore && ! partIgnore && ! buildModPliIgnore && ! synthBegin &&
                      calloutMode == CalloutBeginMeta::Unassembled) {

                      opts.pliParts += calloutParts;
                    }

                  if (rc != 0) {
                      steps->placement = steps->meta.LPub.assem.placement;
                      return rc;
                    }
                } else {
                  callout->instances++;
                  if (calloutMode == CalloutBeginMeta::Unassembled) {
                      opts.pliParts += calloutParts;
                    }
                }

              /* remind user what file we're working on */
              emit messageSig(LOG_STATUS, "Processing " + opts.current.modelName + "...");

            } // STEP - Process called out submodel

          // Set flag to display submodel at first submodel step
          if (step && steps->meta.LPub.subModel.show.value()) {
              bool topModel       = (topLevelFile() == topOfStep.modelName);
              bool showTopModel   = (steps->meta.LPub.subModel.showTopModel.value());
              bool showStepOk     = (steps->meta.LPub.subModel.showStepNum.value() == opts.stepNum || opts.stepNum == 1);
              if (showStepOk && !opts.calledOut && (!topModel || showTopModel)){
                  if (multiStep && steps->meta.LPub.multiStep.pli.perStep.value() == false) {
                      previewNotPerStep = !previewNotPerStep ? true : previewNotPerStep;
                  } else {
                      step->placeSubModel = true;
                  }
              }
          }
      }
      // STEP - Process line, triangle, or polygon type
      else if ((tokens.size() == 8  && tokens[0] == "2") ||
               (tokens.size() == 11 && tokens[0] == "3") ||
               (tokens.size() == 14 && tokens[0] == "4") ||
               (tokens.size() == 14 && tokens[0] == "5")) {

          /* we've got a line, triangle or polygon, so add it to the list */
          /* and make sure we know we have a step */

          if (! buildModIgnore) {

            CsiItem::partLine(line,pla,opts.current.lineNumber,OkRc);
            partsAdded = true;

            if (step == nullptr && ! noStep) {
              if (range == nullptr) {
                  range = newRange(steps,opts.calledOut);
                  steps->append(range);
                }

              step = new Step(topOfStep,
                              range,
                              opts.stepNum,
                              steps->meta,
                              opts.calledOut,
                              multiStep);

              range->append(step);
            }
          } // ! buildModIgnore
        }
      // STEP - Process meta command
      else if ( (tokens.size() && tokens[0] == "0") || gprc == EndOfFileRc) {

          /* must be meta-command (or comment) */

          if (global && tokens.contains("!LPUB") && tokens.contains("GLOBAL")) {
              topOfStep = opts.current;
            } else {
              global = false;
            }

          if (gprc == EndOfFileRc) {
              rc = gprc;
          } else {

              // intercept include file flag

              if (includeFileRc != EndOfFileRc) {
                  if (resetIncludeRc) {
                      rc = IncludeRc;                         // return to IncludeRc to parse another line
                  } else {
                      rc = includeFileRc;                     // execute the Rc returned by include(...)
                      resetIncludeRc = true;                  // reset to run include(...) to parse another line
                  }
              } else {
                  rc = curMeta.parse(line,opts.current,true); // continue
              }
          }

          /* handle specific meta-commands */

          switch (rc) {
            /* toss it all out the window, per James' original plan */
            case ClearRc:
              opts.pliParts.clear();
              opts.csiParts.clear();
              opts.lineTypeIndexes.clear();
              //steps->freeSteps();  // had to remove this because it blows steps following clear
              // in step group.
              break;

              /* Buffer exchange */
            case BufferStoreRc:
              opts.bfx[curMeta.bfx.value()] = opts.csiParts;
              opts.bfxLineTypeIndexes[curMeta.bfx.value()] = opts.lineTypeIndexes;
              bfxStore1 = true;
              opts.bfxParts.clear();
              break;

            case BufferLoadRc:
              opts.csiParts = opts.bfx[curMeta.bfx.value()];
              opts.lineTypeIndexes = opts.bfxLineTypeIndexes[curMeta.bfx.value()];
              if (!partsAdded) {
                  ldrawFile.setPrevStepPosition(opts.current.modelName,opts.csiParts.size());
                  //qDebug() << "Model:" << current.modelName << ", Step:"  << stepNum << ", bfx Set Fade Position:" << csiParts.size();
              }
              bfxLoad = true;
              break;

            case PartNameRc:
            case PartTypeRc:
            case MLCadGroupRc:
            case LDCadGroupRc:
            case LeoCadModelRc:
            case LeoCadPieceRc:
            case LeoCadCameraRc:
            case LeoCadLightRc:
            case LeoCadLightWidthRc:
            case LeoCadLightTypeRc:
            case LeoCadSynthRc:
            case LeoCadGroupBeginRc:
            case LeoCadGroupEndRc:
              CsiItem::partLine(line,pla,opts.current.lineNumber,rc);
              if (rc == LeoCadLightWidthRc)
              {
                 // Light WIDTH and HEIGHT written on same line
                 QString height = line.trimmed().split(" ").last();
                 curMeta.LeoCad.light.height.setValue(height.toFloat());
              }
              if (rc == LeoCadLightTypeRc) {
                  // Light TYPE and NAME written on same line
                  int index = line.size() - line.lastIndexOf("NAME") - 5;
                  QString lightName = line.right(index).replace("\"", "");
                  curMeta.LeoCad.light.lightName.setValue(lightName);

                  LightData lightData = curMeta.LeoCad.light.value();
                  QString lightKey = QString("%1 %2").arg(lightData.lightType.value()).arg(lightName);
                  lightList.insert(lightKey, lightData);
              }
              break;

            case IncludeRc:
              includeFileRc = Rc(include(curMeta,includeHere,inserted)); // includeHere and inserted are include(...) vars
              if (includeFileRc != EndOfFileRc) {                        // still reading so continue
                  resetIncludeRc = false;                                // do not reset, allow includeFileRc to execute
                  continue;
              }
              break;

              /* substitute part/parts with this */

            case PliBeginSub1Rc:
            case PliBeginSub2Rc:
            case PliBeginSub3Rc:
            case PliBeginSub4Rc:
            case PliBeginSub5Rc:
            case PliBeginSub6Rc:
            case PliBeginSub7Rc:
            case PliBeginSub8Rc:
              if (pliIgnore) {
                  parseError("Nested PLI BEGIN/ENDS not allowed",opts.current);
                }
              if (steps->meta.LPub.pli.show.value() &&
                  ! pliIgnore &&
                  ! partIgnore &&
                  ! buildModPliIgnore &&
                  ! synthBegin) {
                  QString addPart = QString("1 %1  0 0 0  0 0 0 0 0 0 0 0 0 %2")
                                            .arg(curMeta.LPub.pli.begin.sub.value().color)
                                            .arg(curMeta.LPub.pli.begin.sub.value().part);
                  opts.pliParts << Pli::partLine(addPart,opts.current,curMeta);
                }

              if (step == nullptr && ! noStep && ! buildModIgnore) {
                  if (range == nullptr) {
                      range = newRange(steps,opts.calledOut);
                      steps->append(range);
                    }
                  step = new Step(topOfStep,
                                  range,
                                  opts.stepNum,
                                  curMeta,
                                  opts.calledOut,
                                  multiStep);

                  range->append(step);
                }
              pliIgnore = true;
              break;

              /* do not put subsequent parts into PLI */
            case PliBeginIgnRc:
              if (pliIgnore) {
                  parseError("Nested PLI BEGIN/ENDS not allowed",opts.current);
                }
              pliIgnore = true;
              break;
            case PliEndRc:
              if ( ! pliIgnore) {
                  parseError("PLI END with no PLI BEGIN",opts.current);
                }
              pliIgnore = false;
              curMeta.LPub.pli.begin.sub.clearAttributes();
              break;

            case AssemAnnotationIconRc:
              if (assemAnnotation) {
                  parseError("Nested ASSEM ANNOTATION ICON not allowed",opts.current);
              } else {
                  if (step && ! exportingObjects())
                      step->appendCsiAnnotation(opts.current,curMeta.LPub.assem.annotation/*,view*/);
                  assemAnnotation = false;
              }
              break;

              /* discard subsequent parts, and don't create CSI's for them */
            case PartBeginIgnRc:
            case MLCadSkipBeginRc:
              if (partIgnore) {
                  parseError("Nested BEGIN/ENDS not allowed",opts.current);
                }
              partIgnore = true;
              break;

            case PartEndRc:
            case MLCadSkipEndRc:
              if (! partIgnore) {
                  parseError("Ignore ending with no ignore begin",opts.current);
                }
              partIgnore = false;
              break;

            case SynthBeginRc:
              if (synthBegin) {
                  parseError("Nested LSynth BEGIN/ENDS not allowed",opts.current);
                }
              synthBegin = true;
              break;

            case SynthEndRc:
              if ( ! synthBegin) {
                  parseError("LSynth ignore ending with no ignore begin",opts.current);
                }
              synthBegin = false;
              break;

              /* remove a group or all instances of a part type */
            case RemoveGroupRc:
            case RemovePartTypeRc:
            case RemovePartNameRc:
            {
              if (! buildModIgnore) {
                QStringList newCSIParts;
                QVector<int> newLineTypeIndexes;
                if (rc == RemoveGroupRc) {
                    remove_group(opts.csiParts,opts.lineTypeIndexes,curMeta.LPub.remove.group.value(),newCSIParts,newLineTypeIndexes,curMeta);
                } else if (rc == RemovePartTypeRc) {
                    remove_parttype(opts.csiParts,opts.lineTypeIndexes,curMeta.LPub.remove.parttype.value(),newCSIParts,newLineTypeIndexes);
                } else {
                    remove_partname(opts.csiParts,opts.lineTypeIndexes,curMeta.LPub.remove.partname.value(),newCSIParts,newLineTypeIndexes);
                }
                opts.csiParts = newCSIParts;
                opts.lineTypeIndexes = newLineTypeIndexes;

                if (step == nullptr && ! noStep) {
                    if (range == nullptr) {
                        range = newRange(steps,opts.calledOut);
                        steps->append(range);
                    }
                    step = new Step(topOfStep,
                                    range,
                                    opts.stepNum,
                                    curMeta,
                                    opts.calledOut,
                                    multiStep);

                    range->append(step);
                }
              } // ! buildModIgnore
            }
              break;

            case ReserveSpaceRc:
              /* since we have a part usage, we have a valid step */
              if (opts.calledOut || multiStep) {
                  step = nullptr;
                  Reserve *reserve = new Reserve(opts.current,steps->meta.LPub);
                  if (range == nullptr) {
                      range = newRange(steps,opts.calledOut);
                      steps->append(range);
                    }
                  range->append(reserve);
                }
              break;

            case InsertFinalModelRc:
            case InsertDisplayModelRc:
              {
                // This is not a step but it's necessary to use the step object to place the model
                // Increment the step number down - so basically use the previous step number for this step.
                // Do this before creating the step so we can use the file name during csi generation
                // to indicate this step file is not an actual step - just a model display
                // The Display Model command syntax is:
                // 0 STEP
                // 0 !LPUB ASSEM CAMERA_ANGLES LOCAL 40.0000 65.0000
                // 0 !LPUB ASSEM MODEL_SCALE LOCAL  2.0000
                // 0 !LPUB INSERT PAGE
                // 0 !LPUB INSERT DISPLAY_MODEL
                // 0 STEP
                // Note that LOCAL settings must be placed before INSERT PAGE meta command

                Where top;
                QString message;
                bool proceed = true;
                if (rc == InsertFinalModelRc) {
                    // for final model, check from top of previous step
                    top = getTopOfPreviousStep();
                    message = QString("INSERT MODEL meta must be preceded by 0 STEP before part (type 1) at line");

                    proceed = curMeta.LPub.fadeStep.fadeStep.value() || curMeta.LPub.highlightStep.highlightStep.value();
                } else { /*InsertDisplayModelRc*/
                    top = opts.current;
                    message = QString("INSERT DISPLAY_MODEL meta must be followed by 0 STEP before part (type 1) at line");
                }
                if (stepContains(top,partLineRx)) {
                    parseError(message.append(QString(" %1.").arg(top.lineNumber+1)), opts.current);
                }
                if (proceed) {
                    opts.stepNum--;
                    if (step == nullptr) {
                        if (range == nullptr) {
                            range = newRange(steps,opts.calledOut);
                            steps->append(range);
                        }
                        step = new Step(topOfStep,
                                        range,
                                        opts.stepNum,
                                        curMeta,
                                        opts.calledOut,
                                        multiStep);

                        step->modelDisplayOnlyStep = true;

                        range->append(step);
                    }
                 }
              }
              break;

            case InsertCoverPageRc:
              {
                Where top;
                QString message;
                coverPage = true;
                partsAdded = true;
                page.coverPage = true;
                QRegExp backCoverPage("^\\s*0\\s+!LPUB\\s+.*BACK");
                if (line.contains(backCoverPage)){
                    page.backCover  = true;
                    page.frontCover = false;

                    // for back cover page, check from top of previous step
                    top = getTopOfPreviousStep();
                    message = QString("INSERT COVER_PAGE BACK meta must be preceded by 0 STEP before part (type 1) at line");
                  } else {
                    page.frontCover = true;
                    page.backCover  = false;

                    top = topOfStep;
                    message = QString("INSERT COVER_PAGE FRONT meta must be followed by 0 STEP before part (type 1) at line");
                  }
                  if (stepContains(top,partLineRx)) {
                      parseError(message.append(QString(" %1.").arg(top.lineNumber+1)), opts.current);
                  }
              }
              break;

            case InsertPageRc:
              {
                if (stepContains(topOfStep,partLineRx))
                    parseError(QString("INSERT PAGE meta must be followed by 0 STEP before part (type 1) at line %1.").arg(topOfStep.lineNumber+1),
                               opts.current);

                partsAdded = true;

                // nothing to display in 3D Window
                if (! exporting())
                  emit clearViewerWindowSig();
              }
              break;

            case InsertRc:
              {
                 InsertData insertData = curMeta.LPub.insert.value();
                 if (insertData.type == InsertData::InsertRotateIcon) { // indicate that we have a rotate icon for this step
                     rotateIcon = (opts.calledOut && opts.assembledCallout ? false : true);
                 }
                 if (insertData.type == InsertData::InsertBom) {
                     // nothing to display in 3D Window
                     if (! exporting())
                         emit clearViewerWindowSig();
                 }
                 if (insertData.type == InsertData::InsertText ||
                     insertData.type == InsertData::InsertRichText) {
                     if (insertData.defaultPlacement &&
                         !curMeta.LPub.page.textPlacement.value())
                         curMeta.LPub.insert.initPlacement();
                 }
                 inserts.append(curMeta.LPub.insert);                  // these are always placed before any parts in step
              }
              break;

           case SceneItemZValueDirectionRc:
              {
                   auto insertItem = [&selectedSceneItems](Where here, SceneObjectData soData){
                      if (selectedSceneItems.contains(here))
                          selectedSceneItems.remove(here);
                      selectedSceneItems.insert(here,soData);
                      if (Preferences::debugLogging){
                          emit gui->messageSig(LOG_DEBUG, QString("Selected item %1 (%2) added to the current page item list.")
                                              .arg(soMap[SceneObject(soData.itemObj)])
                                              .arg(soData.itemObj));
                      }
                  };

                  if (line.contains(curMeta.LPub.page.scene.assemAnnotation.preamble))
                      insertItem(curMeta.LPub.page.scene.assemAnnotation.here(),
                                 curMeta.LPub.page.scene.assemAnnotation.value());
                  else if (line.contains(curMeta.LPub.page.scene.assemAnnotationPart.preamble))
                      insertItem(curMeta.LPub.page.scene.assemAnnotationPart.here(),
                                 curMeta.LPub.page.scene.assemAnnotationPart.value());
                  else if (line.contains(curMeta.LPub.page.scene.assem.preamble))
                      insertItem(curMeta.LPub.page.scene.assem.here(),
                                 curMeta.LPub.page.scene.assem.value());
                  else if (line.contains(curMeta.LPub.page.scene.calloutUnderpinning.preamble))
                      insertItem(curMeta.LPub.page.scene.calloutUnderpinning.here(),
                                 curMeta.LPub.page.scene.calloutUnderpinning.value());
                  else if (line.contains(curMeta.LPub.page.scene.calloutBackground.preamble))
                      insertItem(curMeta.LPub.page.scene.calloutBackground.here(),
                                 curMeta.LPub.page.scene.calloutBackground.value());
                  else if (line.contains(curMeta.LPub.page.scene.calloutPointer.preamble))
                      insertItem(curMeta.LPub.page.scene.calloutPointer.here(),
                                 curMeta.LPub.page.scene.calloutPointer.value());
                  else if (line.contains(curMeta.LPub.page.scene.calloutInstance.preamble))
                      insertItem(curMeta.LPub.page.scene.calloutInstance.here(),
                                 curMeta.LPub.page.scene.calloutInstance.value());
                  else if (line.contains(curMeta.LPub.page.scene.dividerBackground.preamble))
                      insertItem(curMeta.LPub.page.scene.dividerBackground.here(),
                                 curMeta.LPub.page.scene.dividerBackground.value());
                  else if (line.contains(curMeta.LPub.page.scene.divider.preamble))
                      insertItem(curMeta.LPub.page.scene.divider.here(),
                                 curMeta.LPub.page.scene.divider.value());
                  else if (line.contains(curMeta.LPub.page.scene.dividerLine.preamble))
                      insertItem(curMeta.LPub.page.scene.dividerLine.here(),
                                 curMeta.LPub.page.scene.dividerLine.value());
                  else if (line.contains(curMeta.LPub.page.scene.dividerPointer.preamble))
                      insertItem(curMeta.LPub.page.scene.dividerPointer.here(),
                                 curMeta.LPub.page.scene.dividerPointer.value());
                  else if (line.contains(curMeta.LPub.page.scene.pointerGrabber.preamble))
                      insertItem(curMeta.LPub.page.scene.pointerGrabber.here(),
                                 curMeta.LPub.page.scene.pointerGrabber.value());
                  else if (line.contains(curMeta.LPub.page.scene.pliGrabber.preamble))
                      insertItem(curMeta.LPub.page.scene.pliGrabber.here(),
                                 curMeta.LPub.page.scene.pliGrabber.value());
                  else if (line.contains(curMeta.LPub.page.scene.submodelGrabber.preamble))
                      insertItem(curMeta.LPub.page.scene.submodelGrabber.here(),
                                 curMeta.LPub.page.scene.submodelGrabber.value());
                  else if (line.contains(curMeta.LPub.page.scene.insertPicture.preamble))
                      insertItem(curMeta.LPub.page.scene.insertPicture.here(),
                                 curMeta.LPub.page.scene.insertPicture.value());
                  else if (line.contains(curMeta.LPub.page.scene.insertText.preamble))
                      insertItem(curMeta.LPub.page.scene.insertText.here(),
                                 curMeta.LPub.page.scene.insertText.value());
                  else if (line.contains(curMeta.LPub.page.scene.pageAttributePixmap.preamble))
                      insertItem(curMeta.LPub.page.scene.pageAttributePixmap.here(),
                                 curMeta.LPub.page.scene.pageAttributePixmap.value());
                  else if (line.contains(curMeta.LPub.page.scene.pageAttributeText.preamble))
                      insertItem(curMeta.LPub.page.scene.pageAttributeText.here(),
                                 curMeta.LPub.page.scene.pageAttributeText.value());
                  else if (line.contains(curMeta.LPub.page.scene.pageNumber.preamble))
                      insertItem(curMeta.LPub.page.scene.pageNumber.here(),
                                 curMeta.LPub.page.scene.pageNumber.value());
                  else if (line.contains(curMeta.LPub.page.scene.pagePointer.preamble))
                      insertItem(curMeta.LPub.page.scene.pagePointer.here(),
                                 curMeta.LPub.page.scene.pagePointer.value());
                  else if (line.contains(curMeta.LPub.page.scene.partsListAnnotation.preamble))
                      insertItem(curMeta.LPub.page.scene.pagePointer.here(),
                                 curMeta.LPub.page.scene.partsListAnnotation.value());
                  else if (line.contains(curMeta.LPub.page.scene.partsListBackground.preamble))
                      insertItem(curMeta.LPub.page.scene.partsListBackground.here(),
                                 curMeta.LPub.page.scene.partsListBackground.value());
                  else if (line.contains(curMeta.LPub.page.scene.partsListInstance.preamble))
                      insertItem(curMeta.LPub.page.scene.pageNumber.here(),
                                 curMeta.LPub.page.scene.partsListInstance.value());
                  else if (line.contains(curMeta.LPub.page.scene.pointerHead.preamble))
                      insertItem(curMeta.LPub.page.scene.pageNumber.here(),
                                 curMeta.LPub.page.scene.pointerHead.value());
                  else if (line.contains(curMeta.LPub.page.scene.pointerFirstSeg.preamble))
                      insertItem(curMeta.LPub.page.scene.pageNumber.here(),
                                 curMeta.LPub.page.scene.pointerFirstSeg.value());
                  else if (line.contains(curMeta.LPub.page.scene.pointerSecondSeg.preamble))
                      insertItem(curMeta.LPub.page.scene.pageNumber.here(),
                                 curMeta.LPub.page.scene.pointerSecondSeg.value());
                  else if (line.contains(curMeta.LPub.page.scene.pointerThirdSeg.preamble))
                      insertItem(curMeta.LPub.page.scene.pageNumber.here(),
                                 curMeta.LPub.page.scene.pointerThirdSeg.value());
                  else if (line.contains(curMeta.LPub.page.scene.rotateIconBackground.preamble))
                      insertItem(curMeta.LPub.page.scene.rotateIconBackground.here(),
                                 curMeta.LPub.page.scene.rotateIconBackground.value());
                  else if (line.contains(curMeta.LPub.page.scene.stepNumber.preamble))
                      insertItem(curMeta.LPub.page.scene.stepNumber.here(),
                                 curMeta.LPub.page.scene.stepNumber.value());
                  else if (line.contains(curMeta.LPub.page.scene.subModelBackground.preamble))
                      insertItem(curMeta.LPub.page.scene.subModelBackground.here(),
                                 curMeta.LPub.page.scene.subModelBackground.value());
                  else if (line.contains(curMeta.LPub.page.scene.subModelInstance.preamble))
                      insertItem(curMeta.LPub.page.scene.subModelInstance.here(),
                                 curMeta.LPub.page.scene.subModelInstance.value());
                  else if (line.contains(curMeta.LPub.page.scene.submodelInstanceCount.preamble))
                      insertItem(curMeta.LPub.page.scene.submodelInstanceCount.here(),
                                 curMeta.LPub.page.scene.submodelInstanceCount.value());
                  else if (line.contains(curMeta.LPub.page.scene.partsListPixmap.preamble))
                      insertItem(curMeta.LPub.page.scene.partsListPixmap.here(),
                                 curMeta.LPub.page.scene.partsListPixmap.value());
                  else if (line.contains(curMeta.LPub.page.scene.partsListGroup.preamble))
                      insertItem(curMeta.LPub.page.scene.partsListGroup.here(),
                                 curMeta.LPub.page.scene.partsListGroup.value());
                  else if (line.contains(curMeta.LPub.page.scene.stepBackground.preamble))
                      insertItem(curMeta.LPub.page.scene.stepBackground.here(),
                                 curMeta.LPub.page.scene.stepBackground.value());
                  else if (line.contains(curMeta.LPub.page.scene.multiStepBackground.preamble))
                      insertItem(curMeta.LPub.page.scene.stepBackground.here(),
                                 curMeta.LPub.page.scene.multiStepBackground.value());
                  else /*if (line.contains(curMeta.LPub.page.scene.multiStepsBackground.preamble))*/
                      insertItem(curMeta.LPub.page.scene.stepBackground.here(),
                                 curMeta.LPub.page.scene.multiStepsBackground.value());
              }
              break;

           case PliPartGroupRc:
                curMeta.LPub.pli.pliPartGroup.setWhere(opts.current);
                opts.pliPartGroups.append(curMeta.LPub.pli.pliPartGroup);
              break;

            case PagePointerRc:
              {
                  if (pagePointer) {
                      parseError("Nested page pointers not allowed within the same page",opts.current);
                  } else {
                      Positions position    = PP_LEFT;
                      PointerMeta ppm       = curMeta.LPub.page.pointer;
                      PointerAttribMeta pam = curMeta.LPub.page.pointerAttrib;
                      PointerAttribData pad = pam.valueInches();
                      RectPlacement currRp  = curMeta.LPub.page.pointer.value().rectPlacement;

                      if (currRp == TopInside)
                          position = PP_TOP;
                      else
                      if (currRp == BottomInside)
                          position = PP_BOTTOM;
                      else
                      if (currRp == LeftInside)
                          position = PP_LEFT;
                      else
                      if (currRp == RightInside)
                          position = PP_RIGHT;

                      // DEBUG
                      bool newPP = true;
                      pagePointer = pagePointers.value(position);
                      if (pagePointer) {
                          newPP = false;
                          pad.id     = pagePointer->pointerList.size() + 1;
                          pad.parent = PositionNames[position];
                          pam.setValueInches(pad);
                          pagePointer->appendPointer(opts.current,ppm,pam);
                          pagePointers.remove(position);
                          pagePointers.insert(position,pagePointer);
                      } else {
                          pagePointer = new PagePointer(&curMeta,view);
                          pagePointer->parentStep = step;
                          pagePointer->setTopOfPagePointer(opts.current);
                          pagePointer->setBottomOfPagePointer(opts.current);
                          if (multiStep){
                              pagePointer->parentRelativeType = StepGroupType;
                          } else
                            if (opts.calledOut){
                              pagePointer->parentRelativeType = CalloutType;
                          } else {
                              pagePointer->parentRelativeType = step->relativeType;
                          }
                          PlacementMeta placementMeta;
                          placementMeta.setValue(currRp, PageType);
                          pagePointer->placement = placementMeta;

                          pad.id     = 1;
                          pad.parent = PositionNames[position];
                          pam.setValueInches(pad);
                          pagePointer->appendPointer(opts.current,ppm,pam);
                          pagePointers.insert(position,pagePointer);
                      }
                      pagePointer = nullptr;
                  }
              }
              break;

            case PagePointerAttribRc:
              {
                  PointerAttribMeta pam = curMeta.LPub.page.pointerAttrib;
                  pam.setValueInches(pam.parseAttributes(tokens,opts.current));

                  Positions position = PP_LEFT;
                  if (pam.value().parent == "BASE_TOP")
                      position = PP_TOP;
                  else
                  if (pam.value().parent == "BASE_BOTTOM")
                      position = PP_BOTTOM;
                  else
                  if (pam.value().parent == "BASE_LEFT")
                      position = PP_LEFT;
                  else
                  if (pam.value().parent == "BASE_RIGHT")
                      position = PP_RIGHT;

                  PagePointer *pp = pagePointers.value(position);
                  if (pp) {
                      Pointer          *p = nullptr;
                      int i               = pam.value().id - 1;
                      int validIndex      = pp->pointerList.size() - 1; /*0-index*/
                      if (i <= validIndex) {
                          p = pp->pointerList[i];
                      } else {
                          parseError(QString("Invalid Page pointer attribute index.<br>"
                                             "Expected value &#60;= %1, received %2")
                                             .arg(validIndex).arg(i),opts.current);
                          break;
                      }
                      if (p && pam.value().id == p->id) {
                          pam.setAltValueInches(p->getPointerAttribInches());
                          p->setPointerAttribInches(pam);
                          pp->pointerList.replace(i,p);
                          pagePointers.remove(position);
                          pagePointers.insert(position,pp);
                      }
                  } else {
                      emit messageSig(LOG_ERROR, QString("Page Position %1 does not exist.").arg(PositionNames[position]));
                  }
              }
              break;

            case CalloutBeginRc:
              if (callout) {
                  parseError("Nested CALLOUT not allowed within the same file",opts.current);
                } else {
                  callout = new Callout(curMeta,view);
                  callout->setTopOfCallout(opts.current);
                }
              break;

            case CalloutPointerRc:
              if (callout) {
                  callout->appendPointer(opts.current,
                                         curMeta.LPub.callout.pointer,
                                         curMeta.LPub.callout.pointerAttrib);
                }
              break;

          case CalloutPointerAttribRc:
              if (callout) {
                  PointerAttribMeta pam = curMeta.LPub.callout.pointerAttrib;
                  pam.setValueInches(pam.parseAttributes(tokens,opts.current));
                  Pointer          *p = nullptr;
                  int i               = pam.value().id - 1;
                  int validIndex      = callout->pointerList.size() - 1; /*0-index*/
                  if (i <= validIndex) {
                      p = callout->pointerList[i];
                  } else {
                      parseError(QString("Invalid Callout pointer attribute index.<br>"
                                         "Expected value &#60;= %1, received %2")
                                         .arg(validIndex).arg(i),opts.current);
                      break;
                  }
                  if (p && pam.value().id == p->id) {
                      pam.setAltValueInches(p->getPointerAttribInches());
                      p->setPointerAttribInches(pam);
                      callout->pointerList.replace(i,p);
                  }
              }
            break;

            case CalloutDividerRc:
              if (range) {
                  range->sepMeta = curMeta.LPub.callout.sep;
                  dividerType = (line.contains("STEPS") ? StepDivider : RangeDivider);
                  set_divider_pointers(curMeta,opts.current,range,view,dividerType,opts.stepNum,CalloutDividerRc);
                  if (dividerType != StepDivider) {
                      range = nullptr;
                      step = nullptr;
                  }
                }
              break;

            case CalloutEndRc:
              if ( ! callout) {
                  parseError("CALLOUT END without a CALLOUT BEGIN",opts.current);
                }
              else
              if (! step) {
                  parseError("CALLOUT does not contain a valid STEP",opts.current);
               }
              else
                {
                  callout->parentStep = step;
                  if (multiStep) {
                      callout->parentRelativeType = StepGroupType;
                    } else if (opts.calledOut) {
                      callout->parentRelativeType = CalloutType;
                    } else {
                      callout->parentRelativeType = step->relativeType;
                    }
                  // set csi annotations - callout
//                  if (! exportingObjects())
//                      callout->setCsiAnnotationMetas();
                  callout->pli.clear();
                  callout->placement = curMeta.LPub.callout.placement;
                  callout->margin = curMeta.LPub.callout.margin;
                  callout->setBottomOfCallout(opts.current);
                  callout = nullptr;
                }
              break;

            case StepGroupBeginRc:
              if (opts.calledOut) {
                  parseError("MULTI_STEP not allowed inside callout models",opts.current);
              } else {
                  if (multiStep) {
                      parseError("Nested MULTI_STEP not allowed",opts.current);
                  }
                  multiStep = true;
              }
              steps->relativeType = StepGroupType;
              break;

            case StepGroupDividerRc:
              if (range) {
                  range->sepMeta = steps->meta.LPub.multiStep.sep;
                  dividerType = (line.contains("STEPS") ? StepDivider : RangeDivider);
                  set_divider_pointers(curMeta,opts.current,range,view,dividerType,opts.stepNum,StepGroupDividerRc);
                  if (dividerType != StepDivider) {
                      range = nullptr;
                      step = nullptr;
                  }
                }
              break;

              /* finished off a multiStep */
            case StepGroupEndRc:
              if (multiStep && steps->list.size()) {
                  // save the current meta as the meta for step group
                  // PLI for non-pli-per-step
                  if (partsAdded) {
                      parseError("Expected STEP before MULTI_STEP END", opts.current);
                    }
                  multiStep = false;

                 /*
                  * TODO
                  * Consider setting steps->meta to current meta here. This way we pass the latest settings
                  * to formatPage processing - this is particularly helpful if we want to capture step group
                  * metas that are set in steps beyond the first group step by the editor.
                  * be updated after the first step in the step group (when the groupStepMeta is set).
                  * Populating the groupStepMeta at the first group step is necessary to capture any
                  * step group specific settings as there is no reasonable way to know when the step group
                  * ends and capturing the curMeta afte is too late as it is popped at the end of every step
                  * steps->meta = curMeta
                  */
                  // get the number of submodel instances in the model file
                  int countInstanceOverride = /*steps->meta*/steps->groupStepMeta.LPub.page.countInstanceOverride.value();
                  int instances = countInstanceOverride ? countInstanceOverride :
                                                          ldrawFile.instances(opts.current.modelName, opts.isMirrored);
                  // count the instances - use steps->meta (vs. steps->groupStepMeta) to access current submodelStack
                  //
                  // ldrawFile.instances() configuration is CountAtTop - the historic LPub count scheme. However, the updated
                  // the updated countInstances routine's configuration is CountAtModel - this is the default options set
                  // and configurable in Project globals
                  displayCount = countInstances && instances > 1;
                  if (displayCount && countInstances != CountAtTop && !countInstanceOverride) {
                      MetaItem mi;
                      if (countInstances == CountAtStep)
                          instances = mi.countInstancesInStep(&steps->meta, opts.current.modelName);
                      else
                      if (countInstances > CountFalse && countInstances < CountAtStep)
                          instances = mi.countInstancesInModel(&steps->meta, opts.current.modelName);
                  }
#ifdef QT_DEBUG_MODE
                  if (steps->meta.LPub.multiStep.pli.perStep.value() !=
                      steps->groupStepMeta.LPub.multiStep.pli.perStep.value())
                      emit messageSig(LOG_TRACE, QString("COMPARE - StepGroup PLI per step: stepsMeta %1")
                                      .arg(steps->meta.LPub.multiStep.pli.perStep.value() ? "[On], groupStepMeta [Off]" : "[Off], groupStepMeta [On]"));
#endif
                  if (opts.pliParts.size() && /*steps->meta*/steps->groupStepMeta.LPub.multiStep.pli.perStep.value() == false) {
                      PlacementData placementData;
                      // Override default assignments, which is for PliPerStep true
                      // Step Number
                      if (/*steps->meta*/steps->groupStepMeta.LPub.assem.showStepNumber.value()) {
                          placementData = steps->groupStepMeta.LPub.multiStep.stepNum.placement.value();
                          if ((placementData.relativeTo    == CsiType ||
                              (placementData.relativeTo    == PartsListType &&
                               placementData.justification == Top &&
                               placementData.placement     == Left &&
                               placementData.preposition   == Outside))) {
                              steps->groupStepMeta.LPub.multiStep.stepNum.placement.setValue(BottomLeftOutside,PageHeaderType);
                            }

                          // add the step number
                          steps->groupStepNumber.placement = steps->groupStepMeta.LPub.multiStep.stepNum.placement;
                          steps->groupStepNumber.margin    = steps->groupStepMeta.LPub.multiStep.stepNum.margin;
                          steps->groupStepNumber.number    = opts.groupStepNumber;
                          steps->groupStepNumber.sizeit();

                          emit messageSig(LOG_INFO_STATUS, "Add Step group step number for multi-step page " + opts.current.modelName);

                          // if PLI and Submodel Preview are relative to StepNumber or PLI relative to CSI (default)
                          placementData = steps->groupStepMeta.LPub.multiStep.pli.placement.value();
                          if (previewNotPerStep &&
                             ((steps->groupStepMeta.LPub.multiStep.subModel.placement.value().relativeTo == StepNumberType &&
                              placementData.relativeTo == StepNumberType) || placementData.relativeTo == CsiType))
                          {
                              // Redirect Pli relative to SubModel Preview
                              steps->groupStepMeta.LPub.multiStep.pli.placement.setValue(BottomLeftOutside,SubModelType);
                          }
                          // Redirect Pli relative to Step Number if relative to CSI (default)
                          else if(placementData.relativeTo == CsiType) {
                              steps->groupStepMeta.LPub.multiStep.pli.placement.setValue(RightTopOutside,StepNumberType);
                          }
                      }
                      // if no step number
                      else {
                          // if Submodel Preview relative to StepNumber
                          if (previewNotPerStep &&
                                  steps->groupStepMeta.LPub.multiStep.subModel.placement.value().relativeTo == StepNumberType) {
                              // Redirect Submodel Preview relative to Page
                              steps->groupStepMeta.LPub.multiStep.subModel.placement.setValue(BottomLeftOutside,PageHeaderType);
                          }
                          // if Pli relative to StepNumber or CSI
                          if (steps->groupStepMeta.LPub.multiStep.pli.placement.value().relativeTo == StepNumberType ||
                              steps->groupStepMeta.LPub.multiStep.pli.placement.value().relativeTo == CsiType) {
                              if (previewNotPerStep)
                              {
                                  // Redirect Pli relative to SubModel Preview
                                  steps->groupStepMeta.LPub.multiStep.pli.placement.setValue(BottomLeftOutside,SubModelType);
                              }
                              // Redirect Pli relative to Page if relative to CSI (default)
                              else if(placementData.relativeTo == CsiType) {
                                  steps->groupStepMeta.LPub.multiStep.pli.placement.setValue(BottomLeftOutside,PageHeaderType);
                              }
                          }
                      }

                      // PLI
                      steps->pli.bom    = false;
                      steps->pli.margin = steps->groupStepMeta.LPub.multiStep.pli.margin;
                      steps->pli.setParts(opts.pliParts,opts.pliPartGroups,steps->groupStepMeta);

                      emit messageSig(LOG_STATUS, "Add PLI images for multi-step page " + opts.current.modelName);

                      if (steps->pli.sizePli(&steps->groupStepMeta, StepGroupType, false) != 0)
                          emit messageSig(LOG_ERROR, "Failed to set PLI (per Page) for " + topOfStep.modelName + "...");

                      // SubModel Preview
                      if (previewNotPerStep) {
                          steps->groupStepMeta.LPub.subModel.instance.setValue(instances);
                          steps->subModel.margin = steps->groupStepMeta.LPub.subModel.margin;
                          steps->subModel.setSubModel(opts.current.modelName,steps->groupStepMeta);

                          emit messageSig(LOG_INFO_STATUS, "Add Submodel Preview for multi-step page " + opts.current.modelName);

                          steps->subModel.displayInstanceCount = displayCount;
                          if (steps->subModel.sizeSubModel(&steps->groupStepMeta,StepGroupType,false) != 0)
                              emit messageSig(LOG_ERROR, "Failed to set Submodel Preview for " + topOfStep.modelName + "...");
                      }
                  } else {
                      steps->groupStepNumber.number     = 0;
                      steps->groupStepNumber.stepNumber = nullptr;
                  }
                  opts.pliParts.clear();
                  opts.pliPartGroups.clear();

                  /* this is a page we're supposed to process */

                  // Update steps->meta args with stepGroup args - perhaps update entire steps->meta ?
                  // because steps->meta is popped at the top of the STEP containing MULTI_STEP END
                  steps->placement = steps->groupStepMeta.LPub.multiStep.placement;                  // was steps->meta.LPub.multiStep.placement
                  steps->meta.LPub.page.background = steps->groupStepMeta.LPub.page.background;
                  steps->meta.LPub.multiStep.placement = steps->groupStepMeta.LPub.multiStep.placement;

                  showLine(steps->topOfSteps());

                  bool endOfSubmodel =
                          /*steps->meta*/steps->groupStepMeta.LPub.contStepNumbers.value() ?
                              /*steps->meta*/steps->groupStepMeta.LPub.contModelStepNum.value() >= ldrawFile.numSteps(opts.current.modelName) :
                              opts.stepNum - 1 >= ldrawFile.numSteps(opts.current.modelName);

                  // set csi annotations - multistep
//                  if (! exportingObjects())
//                      steps->setCsiAnnotationMetas();

                  Page *page = dynamic_cast<Page *>(steps);
                  if (page) {
                      page->inserts              = inserts;
                      page->instances            = instances;
                      page->displayInstanceCount = displayCount;
                      page->pagePointers         = pagePointers;
                      page->selectedSceneItems   = selectedSceneItems;
                    }

                  emit messageSig(LOG_STATUS, "Generate CSI images for multi-step page " + opts.current.modelName);

                  if (renderer->useLDViewSCall() && opts.ldrStepFiles.size() > 0) {
                      QElapsedTimer timer;
                      timer.start();
                      QString empty("");

                      // renderer parms are added to csiKeys in createCsi()

                      int rc = renderer->renderCsi(empty,opts.ldrStepFiles,opts.csiKeys,empty,/*steps->meta*/steps->groupStepMeta);
                      if (rc != 0) {
                          emit messageSig(LOG_ERROR,QMessageBox::tr("Render CSI images failed."));
                          return rc;
                        }

                      emit messageSig(LOG_INFO,
                                          QString("%1 CSI (Single Call) render took "
                                                  "%2 milliseconds to render %3 [Step %4] %5 "
                                                  "for %6 step group on page %7.")
                                             .arg(Render::getRenderer())
                                             .arg(timer.elapsed())
                                             .arg(opts.ldrStepFiles.size())
                                             .arg(opts.stepNum)
                                             .arg(opts.ldrStepFiles.size() == 1 ? "image" : "images")
                                             .arg(opts.calledOut ? "called out," : "simple,")
                                             .arg(stepPageNum));
                  }

                  // load the 3DViewer with the last step of the step group
                  Step *lastStep = step;
                  if (!lastStep) {
                      Range *lastRange = range;
                      if (!lastRange)
                          lastRange = dynamic_cast<Range *>(steps->list[steps->list.size() - 1]);
                      lastStep = dynamic_cast<Step *>(lastRange->list[lastRange->list.size() - 1]);
                      emit messageSig(LOG_DEBUG,QString("Step group last step number %2").arg(lastStep->stepNumber.number));
                  }
                  lastStep->loadTheViewer();

                  addGraphicsPageItems(steps, coverPage, endOfSubmodel, view, scene, opts.printing);

                  previewNotPerStep = false;
                  drawPageElapsedTime();
                  return HitEndOfPage;
                }
              inserts.clear();
              buildModKeys.clear();
              pagePointers.clear();
              selectedSceneItems.clear();
              break;

            // Update BuildMod action for 'current' step
            case BuildModApplyRc:
            case BuildModRemoveRc:
              if (!Preferences::buildModEnabled)
                  break;
              if ((multiStep && topOfStep != steps->topOfSteps()) || opts.calledOut) {
                  if (partsAdded) {
                      parseError(QString("BUILD_MOD REMOVE/APPLY action command must be placed before step parts"),
                                 opts.current);
                    }

                  int buildModStepIndex = getBuildModStepIndex(topOfStep);
                  buildModKey = page.meta.LPub.buildMod.key();
                  if (buildModContains(buildModKey))
                      buildModAction = Rc(getBuildModAction(buildModKey, buildModStepIndex));
                  else
                      parseError(QString("BuildMod for key '%1' not found").arg(buildModKey),
                                 opts.current,Preferences::ParseErrors);

                  if (buildModAction != rc) {
                      // set BuildMod action for current step
                      setBuildModAction(buildModKey, buildModStepIndex, rc);
                      // set buildModStepIndex for writeToTmp() and findPage() content
                      setBuildModNextStepIndex(topOfStep);
                      // clear viewerStepKey for previous step to not trigger viewerUpdate in createCsi()
                      viewerStepKey.clear();
                      // Rerun to findPage() to regenerate parts and options for buildMod action
                      return HitBuildModAction;
                  }
              }
              break;

            // Get BuildMod attributes and set ModIgnore based on 'current' step buildModAction
            case BuildModBeginRc:
              if (!Preferences::buildModEnabled)
                  break;
              buildModKey        = curMeta.LPub.buildMod.key();
              opts.buildModLevel = getLevel(buildModKey, BM_BEGIN);
              if (! buildModKeys.contains(opts.buildModLevel))
                  buildModKeys.insert(opts.buildModLevel, buildModKey);
              // create this buildMod
              if ((multiStep && topOfStep != steps->topOfSteps()) || opts.calledOut) {
                  insertAttribute(buildModAttributes, BM_BEGIN_LINE_NUM, opts.current);
                  buildModActions.insert(opts.buildModLevel, BuildModApplyRc);
              }
              buildModIgnore     = false;
              buildModPliIgnore  = true;
              buildMod[BM_BEGIN] = true;
              break;

            // Set modActionLineNum and ModIgnore based on 'current' step buildModAction
            case BuildModEndModRc:
              if (!Preferences::buildModEnabled)
                  break;
              if (opts.buildModLevel > 1 && curMeta.LPub.buildMod.key().isEmpty())
                  parseError("Key required for nested build mod meta command",
                             opts.current,Preferences::BuildModErrors);
              if (!buildMod[BM_BEGIN])
                  parseError(QString("Required meta BUILD_MOD BEGIN not found"), opts.current, Preferences::BuildModErrors);
              if ((multiStep && topOfStep != steps->topOfSteps()) || opts.calledOut)
                  insertAttribute(buildModAttributes, BM_ACTION_LINE_NUM, opts.current);
              buildModIgnore       = true;
              buildModPliIgnore    = false;
              buildMod[BM_END_MOD] = true;
              break;

            // Get buildModLevel and reset ModIgnore to default
            case BuildModEndRc:
              if (!Preferences::buildModEnabled)
                  break;
              if (!buildMod[BM_END_MOD])
                  parseError(QString("Required meta BUILD_MOD END_MOD not found"), opts.current, Preferences::BuildModErrors);
              if ((multiStep && topOfStep != steps->topOfSteps()) || opts.calledOut)
                  insertAttribute(buildModAttributes, BM_ACTION_LINE_NUM, opts.current);
              opts.buildModLevel  = getLevel(QString(), BM_END);
              buildModIgnore      = buildModPliIgnore = false;
              buildMod[BM_END]    = true;
              break;

            case NoStepRc:
              noStep = true;
              break;

              /* we've hit some kind of step, or implied step and end of file */
            case EndOfFileRc:
            case RotStepRc:
            case StepRc:
             /*
              * STEP - special case of step group with NOSTEP as last step and rotated or assembled called out submodel
              */
              if (! buildModIgnore && noStep && opts.calledOut) {
                if (opts.current.modelName.contains("whole_rotated_") ||
                    opts.current.modelName.contains("whole_assembled_")) {
                  bool nsHasParts    = false;
                  bool nsHasNoStep   = false;
                  bool nsIsStepGroup = false;
                  Meta nsMeta        = curMeta;
                  QStringList nsTokens;
                  split(addLine,nsTokens);
                  // start with the original subfile content
                  QStringList nsContent = ldrawFile.contents(nsTokens[14]);
                  int nsNumLines = nsContent.size();
                  Where nsWalkBack(nsTokens[14],nsNumLines);
                  for (; nsWalkBack.lineNumber >= 0; nsWalkBack--) {
                    QString nsLine = readLine(nsWalkBack);
                    if (isHeader(nsLine)) {
                      // if we reached the top of the submodel so break
                      break;
                    } else {
                      nsTokens.clear();
                      split(nsLine,nsTokens);
                      bool nsLine_1_5 = nsTokens.size() && nsTokens[0].size() == 1 &&
                           nsTokens[0] >= "1" && nsTokens[0] <= "5";
                      bool nsLine_0 = nsTokens.size() > 0 && nsTokens[0] == "0";
                      if (nsLine_1_5) {
                        // we have a valid part so record part added
                        nsHasParts = true;
                      } else if (nsLine_0) {
                        Rc rc = nsMeta.parse(nsLine,nsWalkBack,false);
                        if (rc == StepRc || rc == RotStepRc) {
                          // are we in a new step which is in a step group ?
                          if (nsIsStepGroup) {
                            // confirm previous step does not have a NOSTEP command
                            // and parts have been added
                            if (nsHasParts && !nsHasNoStep) {
                              // we have a STEP where parts were added and no NOSTEP command encountered
                              // so this step group can be rendered as a rotated or assembled callout
                              noStep = false;
                              break;
                            } else {
                              // clear the registers for new parts added and NOSTEP check
                              nsHasParts  = false;
                              nsHasNoStep = false;
                            }
                          } else {
                            // the last step did not have MULTI_STEP_END so break
                            break;
                          }
                        } else if (rc == NoStepRc) {
                          // NOSTEP encountered so record it and continue to the top of the step group
                          nsHasNoStep = true;
                        } else if (rc == StepGroupEndRc) {
                          // we are in a step group so proceed
                          nsIsStepGroup = true;
                        } else if (rc == StepGroupBeginRc) {
                          // we have reached the top of the step group so break
                          break;
                        }
                      }
                    }
                  }
                }
              } // STEP - special case of step group with NOSTEP as last step and rotated or assembled called out submodel

             /*
              * STEP - special case of no parts added, but BFX load and not NOSTEP and not BUILD_MOD ignore
              */
              if (! partsAdded && bfxLoad && ! noStep && ! buildModIgnore) {
                  if (step == nullptr) {
                      if (range == nullptr) {
                          range = newRange(steps,opts.calledOut);
                          steps->append(range);
                        }
                      step = new Step(topOfStep,
                                      range,
                                      opts.stepNum,
                                      curMeta,
                                      opts.calledOut,
                                      multiStep);

                      range->append(step);
                    }

                  emit messageSig(LOG_INFO, "Processing CSI bfx load special case for " + topOfStep.modelName + "...");
                  (void) step->createCsi(
                        opts.isMirrored ? addLine : "1 color 0 0 0 1 0 0 0 1 0 0 0 1 foo.ldr",
                        configuredCsiParts = configureModelStep(opts.csiParts, opts.stepNum, topOfStep),
                        opts.lineTypeIndexes,
                        &step->csiPixmap,
                        steps->meta,
                        bfxLoad);

                  if (renderer->useLDViewSCall() && ! step->ldrName.isNull()) {
                      opts.ldrStepFiles << step->ldrName;
                      opts.csiKeys << step->csiKey; // No parts to process
                    }

                  partsAdded = true; // OK, so this is a lie, but it works
                } // STEP - special case of no parts added, but BFX load

             /*
              *  STEP - normal case of parts added, and not NOSTEP
              */
              if (partsAdded && ! noStep && ! buildModIgnore) {

                  // set step group page meta attributes first step
                  if (firstStep) {
                      steps->groupStepMeta = curMeta;
                      firstStep = false;
                  }

                  if (pliIgnore) {
                      parseError("PLI BEGIN then STEP. Expected PLI END",opts.current);
                      pliIgnore = false;
                  }
                  if (partIgnore) {
                      parseError("PART BEGIN then STEP. Expected PART END",opts.current);
                      partIgnore = false;
                  }
                  if (synthBegin) {
                      parseError("SYNTH BEGIN then STEP. Expected SYNTH_END",opts.current);
                      synthBegin = false;
                  }

                  bool pliPerStep;

                  if (multiStep && steps->meta.LPub.multiStep.pli.perStep.value()) {
                      pliPerStep = true;
                  } else if (opts.calledOut && steps->meta.LPub.callout.pli.perStep.value()) {
                      pliPerStep = true;
                  } else if ( ! multiStep && ! opts.calledOut && steps->meta.LPub.stepPli.perStep.value()) {
                      pliPerStep = true;
                  } else {
                      pliPerStep = false;
                  }

                 /*
                  * STEP - Actual parts added, simple, mulitStep or calledOut (no draw graphics)
                  */
                  if (step) {

                      Page *page = dynamic_cast<Page *>(steps);
                      if (page) {
                          page->inserts              = inserts;
                          page->pagePointers         = pagePointers;
                          page->modelDisplayOnlyStep = step->modelDisplayOnlyStep;
                          page->selectedSceneItems   = selectedSceneItems;
                      }

                      step->lightList = lightList;

                      PlacementType relativeType = SingleStepType;
                      if (pliPerStep) { // Pli per Step
                          if (multiStep) {
                              relativeType = StepGroupType;
                          } else if (opts.calledOut) {
                              relativeType = CalloutType;
                          } else {
                              relativeType = SingleStepType;
                          }

                          step->pli.setParts(opts.pliParts,opts.pliPartGroups,steps->meta);
                          opts.pliParts.clear();
                          opts.pliPartGroups.clear();

                          emit messageSig(LOG_INFO, "Add step PLI for " + topOfStep.modelName + "...");

                          step->pli.sizePli(&steps->meta,relativeType,pliPerStep);

                          if (step->placeSubModel) { // Place SubModel at Step 1
                              emit messageSig(LOG_INFO, "Set first step submodel display for " + topOfStep.modelName + "...");

                              // get the number of submodel instances in the model file
                              int countInstanceOverride = steps->meta.LPub.page.countInstanceOverride.value();
                              int instances = countInstanceOverride ? countInstanceOverride :
                                                                      ldrawFile.instances(opts.current.modelName, opts.isMirrored);
                              displayCount = steps->meta.LPub.subModel.showInstanceCount.value() && instances > 1;
                              // ldrawFile.instances() configuration is CountAtTop - the historic LPub count scheme. However, the updated
                              // the updated countInstances routine's configuration is CountAtModel - this is the default options set
                              // and configurable in Project globals
                              if (displayCount && countInstances != CountAtTop && !countInstanceOverride) {
                                  MetaItem mi;
                                  if (countInstances == CountAtStep)
                                      instances = mi.countInstancesInStep(&steps->meta, opts.current.modelName);
                                  else
                                      if (countInstances > CountFalse && countInstances < CountAtStep)
                                          instances = mi.countInstancesInModel(&steps->meta, opts.current.modelName);
                              }

                              steps->meta.LPub.subModel.instance.setValue(instances);
                              step->subModel.setSubModel(opts.current.modelName,steps->meta);

                              step->subModel.displayInstanceCount = displayCount;

                              if (step->subModel.sizeSubModel(&steps->meta,relativeType,pliPerStep) != 0)
                                  emit messageSig(LOG_ERROR, "Failed to set first step submodel display for " + topOfStep.modelName + "...");
                          } // Place SubModel
                          else
                          {
                              step->subModel.clear();
                          }
                      } // Pli per Step

                      switch (dividerType){
                      // for range divider, we set the dividerType for the last STEP of the previous RANGE.
                      case RangeDivider:
                          if (steps && steps->list.size() > 1) {
                              int i = steps->list.size()-2;           // previous range index
                              Range *previousRange = dynamic_cast<Range *>(steps->list[i]);
                              if (previousRange){
                                  i = previousRange->list.size()-1;   // last step index in previous range
                                  Step *lastStep = dynamic_cast<Step *>(previousRange->list[i]);
                                  if (lastStep)
                                      lastStep->dividerType = dividerType;
                              }
                          }
                          break;
                          // for steps divider, we set the dividerType for the previous STEP
                      case StepDivider:
                          if (range && range->list.size() > 1) {
                              int i = range->list.size()-2;            // previous step index
                              Step *previousStep = dynamic_cast<Step *>(range->list[i]);
                              if (previousStep)
                                  previousStep->dividerType = dividerType;
                          }
                          break;
                          // no divider
                      default:
                          step->dividerType = dividerType;
                          break;
                      }

                      step->placeRotateIcon = rotateIcon;

                      emit messageSig(LOG_INFO_STATUS, "Processing CSI for " + topOfStep.modelName + "...");
                      int rc = step->createCsi(
                                  opts.isMirrored ? addLine : "1 color 0 0 0 1 0 0 0 1 0 0 0 1 foo.ldr",
                                  configuredCsiParts = configureModelStep(opts.csiParts, step->modelDisplayOnlyStep ? -1 : opts.stepNum, topOfStep),
                                  opts.lineTypeIndexes,
                                  &step->csiPixmap,
                                  steps->meta);

                      if (rc) {
                          emit messageSig(LOG_ERROR, QMessageBox::tr("Failed to create CSI file."));
                          return rc;
                      }

                      // BuildMod create and update
                      if (buildModKeys.size()) {
                          if (buildMod[BM_BEGIN] && ! buildMod[BM_END])
                              parseError(QString("Required meta BUILD_MOD END not found"), opts.current, Preferences::BuildModErrors);
                          int viewerPieces = renderer->getViewerPieces();
                          foreach (int buildModLevel, buildModKeys.keys()) {
                              QString buildModKey = buildModKeys.value(buildModLevel);
                              // Create BuildMods
                              if ((multiStep && topOfStep != steps->topOfSteps()) || opts.calledOut)
                                  insertBuildModification(buildModLevel);
                              // Populate viewerPieces, viewerStepKey, displayPageNum
                              setBuildModStepKey(buildModKey, step->viewerStepKey);
                              setBuildModDisplayPageNumber(buildModKey, displayPageNum);
                              setBuildModStepPieces(buildModKey, viewerPieces);
                          }
                          buildModKeys.clear();
                          buildModActions.clear();
                          buildModAttributes.clear();
                          buildMod[2] = buildMod[1] = buildMod[0] = false;
                      }

                      // Set CSI annotations - single step only
                      if (! exportingObjects() &&  ! multiStep && ! opts.calledOut)
                          step->setCsiAnnotationMetas(steps->meta);

                      if (renderer->useLDViewSCall() && ! step->ldrName.isNull()) {
                          opts.ldrStepFiles << step->ldrName;
                          opts.csiKeys << step->csiKey;
                      }

                  } // STEP - Actual parts added, simple, mulitStep or calledOut (no draw graphics)

                  else
                 /*
                  * STEP - No step object, e.g. inserted page (no draw graphics)
                  */
                  {

                      if (pliPerStep) {
                          opts.pliParts.clear();
                          opts.pliPartGroups.clear();
                      }

                      // Only pages or step can have inserts and pointers... not callouts
                      if ( ! multiStep && ! opts.calledOut) {

                          Page *page = dynamic_cast<Page *>(steps);
                          if (page) {
                              page->inserts            = inserts;
                              page->pagePointers       = pagePointers;
                              page->selectedSceneItems = selectedSceneItems;
                          }
                      }
                  } // STEP - No step object, e.g. inserted page (no draw graphics)

                 /*
                  *  STEP - Simple, not mulitStep, not calledOut (draw graphics)
                  */
                  if ( ! multiStep && ! opts.calledOut) {

                      steps->placement = steps->meta.LPub.assem.placement;

                      showLine(topOfStep);

                      int  numSteps = ldrawFile.numSteps(opts.current.modelName);

                      bool endOfSubmodel =
                              numSteps == 0 ||
                              steps->meta.LPub.contStepNumbers.value() ?
                                  steps->meta.LPub.contModelStepNum.value() >= numSteps :
                                  opts.stepNum >= numSteps;

                      int countInstanceOverride = steps->meta.LPub.page.countInstanceOverride.value();
                      int instances = countInstanceOverride ? countInstanceOverride :
                                                              ldrawFile.instances(opts.current.modelName, opts.isMirrored);
                      displayCount = countInstances && instances > 1;
                      // ldrawFile.instances() configuration is CountAtTop - the historic LPub count scheme. However, the updated
                      // the updated countInstances routine's configuration is CountAtModel - this is the default options set
                      // and configurable in Project globals
                      if (displayCount && countInstances != CountAtTop && !countInstanceOverride) {
                          MetaItem mi;
                          if (countInstances == CountAtStep)
                              instances = mi.countInstancesInStep(&steps->meta, opts.current.modelName);
                          else
                              if (countInstances > CountFalse && countInstances < CountAtStep)
                                  instances = mi.countInstancesInModel(&steps->meta, opts.current.modelName);
                      }

                      Page *page = dynamic_cast<Page *>(steps);
                      if (page && instances > 1) {
                          page->instances            = instances;
                          page->displayInstanceCount = displayCount;
                          page->selectedSceneItems   = selectedSceneItems;

                          if (step) {
                              page->modelDisplayOnlyStep = step->modelDisplayOnlyStep;
                              step->lightList = lightList;
                          }

                          if (! steps->meta.LPub.stepPli.perStep.value()) {

                              PlacementType relativeType = SingleStepType;

                              QStringList instancesPliParts;
                              if (opts.pliParts.size() > 0) {
                                  for (int index = 0; index < opts.pliParts.size(); index++) {
                                      QString pliLine = opts.pliParts[index];
                                      for (int i = 0; i < instances; i++) {
                                          instancesPliParts << pliLine;
                                      }
                                  }
                              }

                              if (step) {
                                  // PLI
                                  step->pli.setParts(instancesPliParts,opts.pliPartGroups,steps->meta);
                                  instancesPliParts.clear();
                                  opts.pliParts.clear();
                                  opts.pliPartGroups.clear();

                                  emit messageSig(LOG_INFO, "Add PLI images for single-step page...");

                                  step->pli.sizePli(&steps->meta,relativeType,pliPerStep);

                                  // SM
                                  if (step->placeSubModel){
                                      emit messageSig(LOG_INFO, "Set first step submodel display for " + topOfStep.modelName + "...");

                                      steps->meta.LPub.subModel.instance.setValue(instances);
                                      step->subModel.setSubModel(opts.current.modelName,steps->meta);

                                      step->subModel.displayInstanceCount = displayCount;

                                      if (step->subModel.sizeSubModel(&steps->meta,relativeType,pliPerStep) != 0)
                                          emit messageSig(LOG_ERROR, "Failed to set first step submodel display for " + topOfStep.modelName + "...");
                                  }
                              }
                          }
                      }

                      emit messageSig(LOG_INFO, "Generate CSI image for single-step page...");

                      if (renderer->useLDViewSCall() && opts.ldrStepFiles.size() > 0){

                          QElapsedTimer timer;
                          timer.start();
                          QString empty("");

                          // LDView renderer parms are added to csiKeys in createCsi()

                          // render the partially assembled model
                          int rc = renderer->renderCsi(empty,opts.ldrStepFiles,opts.csiKeys,empty,steps->meta);
                          if (rc != 0) {
                              emit messageSig(LOG_ERROR,QMessageBox::tr("Render CSI images failed."));
                              return rc;
                          }

                          emit messageSig(LOG_INFO,
                                               QString("%1 CSI (Single Call) render took "
                                                       "%2 milliseconds to render %3 [Step %4] %5 for %6 "
                                                       "single step on page %7.")
                                               .arg(Render::getRenderer())
                                               .arg(timer.elapsed())
                                               .arg(opts.ldrStepFiles.size())
                                               .arg(opts.stepNum)
                                               .arg(opts.ldrStepFiles.size() == 1 ? "image" : "images")
                                               .arg(opts.calledOut ? "called out," : "simple,")
                                               .arg(stepPageNum));
                      }

                      addGraphicsPageItems(steps,coverPage,endOfSubmodel,view,scene,opts.printing);
                      stepPageNum += ! coverPage;
                      steps->setBottomOfSteps(opts.current);
                      drawPageElapsedTime();
                      return HitEndOfPage;
                  } // STEP - Simple, not mulitStep, not calledOut (draw graphics)

                  lightList.clear();

                  dividerType = NoDivider;

                  steps->meta.pop();
                  steps->meta.LPub.buildMod.clear();
                  curMeta.LPub.buildMod.clear();
                  opts.stepNum += partsAdded;
                  topOfStep     = opts.current;  // set next step
                  partsAdded    = false;
                  coverPage     = false;
                  rotateIcon    = false;
                  step          = nullptr;
                  opts.bfxStore2= bfxStore1;
                  bfxStore1     = false;
                  bfxLoad       = false;

              } // STEP - normal case of parts added, and not NOSTEP

              if ( ! multiStep) {
                  inserts.clear();
                  pagePointers.clear();
                  selectedSceneItems.clear();
              }

              steps->setBottomOfSteps(opts.current);
              noStep = false;
              break;

            case RangeErrorRc:
            {
              showLine(opts.current);
              QString message;
              if (Preferences::usingNativeRenderer &&
                  line.indexOf("CAMERA_FOV") != -1)
                  message = QString("Native renderer CAMERA_FOV value is out of range [%1:%2]"
                                    "<br>Meta command: %3"
                                    "<br>Valid values: minimum 1.0, maximum 359.0")
                                        .arg(opts.current.modelName)
                                        .arg(opts.current.lineNumber)
                                        .arg(line);
              else
                  message = QString("Parameter(s) out of range: %1:%2<br>Meta command: %3")
                          .arg(opts.current.modelName)
                          .arg(opts.current.lineNumber)
                          .arg(line);

              emit messageSig(LOG_ERROR,message);
              return RangeErrorRc;
            }
            default:
              break;
            }
        }
      // STEP - Process invalid line
      else if (line != "") {
          const QChar type = line.at(0);
          parseError(QString("Invalid LDraw type %1 line. Expected %2 elements, got \"%3\".")
                     .arg(type).arg(type == '1' ? 15 : type == '2' ? 8 : type == '3' ? 11 : 14).arg(line),opts.current);
          retVal = InvalidLDrawLineRc;
      }
      /* if part is on excludedPart.lst, unset pliIgnore if still set */
      if (pliIgnore && tokens[0] == "1" &&
          ExcludedParts::hasExcludedPart(tokens[tokens.size()-1])) {
          pliIgnore = false;
      }
    } // for every line

  drawPageElapsedTime();
  return retVal;
}

int Gui::findPage(
    LGraphicsView   *view,
    LGraphicsScene  *scene,
    Meta             meta,
    QString const   &addLine,
    FindPageOptions &opts)
{

  bool stepGroup  = false; // opts.multiStep
  bool partIgnore = false;
  bool coverPage  = false;
  bool stepPage   = false;
  bool bfxStore1  = false;
  bool bfxStore2  = false;
  bool callout    = false;
  bool noStep     = false;
  bool noStep2    = false;
  bool stepGroupBfxStore2 = false;
  bool pageSizeUpdate     = false;
  bool isPreDisplayPage   = true;
  bool isDisplayPage      = false;

  emit messageSig(LOG_STATUS, "Processing find page for " + opts.current.modelName + "...");

  skipHeader(opts.current);

  if (opts.pageNum == 1) {
      topOfPages.clear();
      topOfPages.append(opts.current);
  }

  Rc rc;
  QStringList bfxParts;
  QStringList saveBfxParts;
  QStringList ldrStepFiles;
  QStringList csiKeys;
  int  partsAdded = 0;
  int  stepNumber = 1;

  QStringList  csiParts;
  QStringList  saveCsiParts;
  QVector<int> lineTypeIndexes;
  QVector<int> saveLineTypeIndexes;
  Where        saveCurrent = opts.current;
  Where        topOfStep = opts.current;
  Where        stepGroupCurrent;
  int          saveStepNumber = 1;

  saveStepPageNum = stepPageNum;

  Meta         saveMeta = meta;

  QHash<QString, QStringList>  bfx;
  QHash<QString, QStringList>  saveBfx;
  QHash<QString, QVector<int>> bfxLineTypeIndexes;
  QHash<QString, QVector<int>> saveBfxLineTypeIndexes;
  QList<PliPartGroupMeta>      emptyPartGroups;

  int numLines = ldrawFile.size(opts.current.modelName);

  int  countInstances = meta.LPub.countInstance.value();

  RotStepMeta saveRotStep = meta.rotStep;

  Rc   buildModAction        = BuildModApplyRc;
  int  buildModBeginLineNum  = 0;
  bool buildModIgnore = false;
  bool buildModItems  = false;
  bool buildModFile   = false;
  QString buildModKey;
  QVector<int> buildModLineTypeIndexes;
  QStringList  buildModCsiParts;

  PartLineAttributes pla(
  csiParts,
  lineTypeIndexes,
  buildModCsiParts,
  buildModLineTypeIndexes,
  opts.buildModLevel,
  buildModIgnore,
  buildModItems);

  // include file vars
  Where includeHere;
  Rc includeFileRc        = EndOfFileRc;
  bool inserted           = false;
  bool resetIncludeRc     = false;

  ldrawFile.setRendered(opts.current.modelName, opts.isMirrored, opts.renderParentModel, opts.renderStepNumber, countInstances);

  /*
   * For findPage(), the BuildMod behaviour captures the appropriate 'block' of lines
   * to be written to the csiPart list and writes the build mod action setting at each
   * step where the BUILD_MOD APPLY or BUILD_MOD REMOVE action meta command is encountered.
   *
   * The buildModLevel flag is enabled for the lines between BUILD_MOD BEGIN and BUILD_MOD END
   * Lines between BUILD_MOD BEGIN and BUILD_MOD END_MOD represent the modified content
   * Lines between BUILD_MOD END_MOD and BUILD_MOD END represent the original content
   *
   * When the buildModLevel flag is true (greater than 0):
   * Funct 1 (csiParts):
   * Parse is enabled when 'buildModIgnore' is false.
   * When the build mod action is 'apply', the modification block is parsed. (buildModIgnore is false)
   * When the build mod action is 'remove', the default block is parsed.     (buildModIgnore is false)
   * Remove group, partType and partName is only applied when 'buildModIgnore is false.
   *
   * When the build mod meta command is BUILD_MOD END 'buildModIgnore' and 'buildModPliIgnore'
   * are reset to false while buildModLevel is reset to false (0), if the build mod command is not nested
   *
   * When BUILD_MOD APPLY or BUILD_MOD REMOVE action meta command is encountered,
   * the respective build mod action is set or updated in ldrawFiles buildMod. Build
   * action is persisted as a key,value pair whereby the key is the step number and
   * the value is the build action. These updates are performed for the entire model
   * at each execution of findPage().
   *
   * When the buildModLevel flag is false pli and csi lines are processed normally
   */

  for ( ;
        opts.current.lineNumber < numLines;
        opts.current.lineNumber++) {

      // if reading include file, return to current line, do not advance

      if (includeFileRc != EndOfFileRc) {
         opts.current.lineNumber--;
      }

      // scan through the rest of the model counting pages
      // if we've already hit the display page, then do as little as possible

      QString line = ldrawFile.readLine(opts.current.modelName,opts.current.lineNumber).trimmed();

      if (line.startsWith("0 GHOST ")) {
          line = line.mid(8).trimmed();
      }

      // Set display vars
      isPreDisplayPage  = opts.pageNum < displayPageNum;
      isDisplayPage     = opts.pageNum == displayPageNum;

      QStringList tokens, addTokens;

      switch (line.toLatin1()[0]) {
      case '1':
          split(line,tokens);

          if (tokens.size() > 2 && tokens[1] == "16") {
              split(addLine,addTokens);
              if (addTokens.size() == 15) {
                  tokens[1] = addTokens[1];
              }
              line = tokens.join(" ");
          }

          // process submodel...
          if (! buildModIgnore) {

              if (! partIgnore) {

                  // csiParts << line;

                  if (firstStepPageNum == -1) {
                      firstStepPageNum = opts.pageNum;
                  }
                  lastStepPageNum = opts.pageNum;

                  QStringList token;

                  split(line,token);

                  if (token.size() == 15) {

                      QString    type = token[token.size()-1];
                      QString colorType = token[1]+type;

                      bool contains   = ldrawFile.isSubmodel(type);
                      CalloutBeginMeta::CalloutMode calloutMode = meta.LPub.callout.begin.value();

                      // if submodel or assembled/rotated callout
                      if (contains && (!callout || (callout && calloutMode != CalloutBeginMeta::Unassembled))) {

                          // check if submodel was rendered
                          bool rendered = ldrawFile.rendered(type,ldrawFile.mirrored(token),opts.current.modelName,stepNumber,countInstances);

                          // if the submodel was not rendered, and (is not in the buffer exchange call setRendered for the submodel.
                          if (! rendered && (! bfxStore2 || ! bfxParts.contains(colorType))) {

                              opts.isMirrored = ldrawFile.mirrored(token);

                              // add submodel to the model stack - it can't be a callout
                              SubmodelStack tos(opts.current.modelName,opts.current.lineNumber,stepNumber);
                              meta.submodelStack << tos;
                              Where current2(type,0);

                              ldrawFile.setModelStartPageNumber(current2.modelName,opts.pageNum);

                              // save rotStep, clear it, and restore it afterwards
                              // since rotsteps don't affect submodels
                              RotStepMeta saveRotStep2 = meta.rotStep;
                              meta.rotStep.clear();

                              // save Default pageSize information
                              PgSizeData pageSize2;
                              if (exporting()) {
                                  pageSize2       = pageSizes[DEF_SIZE];
                                  pageSizeUpdate  = false;
#ifdef SIZE_DEBUG
                                  logDebug() << "SM: Saving    Default Page size info at PageNumber:" << opts.pageNum
                                             << "W:"    << pageSize2.sizeW << "H:"    << pageSize2.sizeH
                                             << "O:"    <<(pageSize2.orientation == Portrait ? "Portrait" : "Landscape")
                                             << "ID:"   << pageSize2.sizeID
                                             << "Model:" << opts.current.modelName;
#endif
                              }

                              // set the step number and parent model where the submodel will be rendered
                              FindPageOptions calloutOpts(
                                          opts.pageNum,
                                          current2,
                                          opts.pageSize,
                                          opts.isMirrored,
                                          opts.printing,
                                          opts.buildModLevel,
                                          opts.contStepNumber,
                                          stepNumber,            /*renderStepNumber */
                                          opts.current.modelName /*renderParentModel*/);
                              findPage(view, scene, meta, line, calloutOpts);

                              saveStepPageNum = stepPageNum;
                              meta.submodelStack.pop_back();
                              meta.rotStep = saveRotStep2;            // restore old rotstep
                              if (opts.contStepNumber) {              // capture continuous step number from exited submodel
                                  opts.contStepNumber = saveContStepNum;
                              }

                              if (exporting()) {
                                  pageSizes.remove(DEF_SIZE);
                                  pageSizes.insert(DEF_SIZE,pageSize2);  // restore old Default pageSize information
#ifdef SIZE_DEBUG
                                  logDebug() << "SM: Restoring Default Page size info at PageNumber:" << opts.pageNum
                                             << "W:"    << pageSizes[DEF_SIZE].sizeW << "H:"    << pageSizes[DEF_SIZE].sizeH
                                             << "O:"    << (pageSizes[DEF_SIZE].orientation == Portrait ? "Portrait" : "Landscape")
                                             << "ID:"   << pageSizes[DEF_SIZE].sizeID
                                             << "Model:" << opts.current.modelName;
#endif
                              }
                          }
                      }
                     if (bfxStore1) {
                         bfxParts << colorType;
                     }
                  }
              } // ! partIgnore
        case '2':
        case '3':
        case '4':
        case '5':
            ++partsAdded;
            CsiItem::partLine(line,pla,opts.current.lineNumber,OkRc);
          } // ! buildModIgnore
            break;

        case '0':

          // intercept include file flag

          if (includeFileRc != EndOfFileRc) {
              if (resetIncludeRc) {
                  rc = IncludeRc;                 // return to IncludeRc to parse another line
              } else {
                  rc = includeFileRc;             // execute the Rc returned by include(...)
                  resetIncludeRc = true;          // reset to run include(...) to parse another line
              }
          } else {
              rc = meta.parse(line,opts.current); // continue
          }

          switch (rc) {
            case StepGroupBeginRc:
              stepGroup = true;
              stepGroupCurrent = topOfStep;
              if (opts.contStepNumber){    // save starting step group continuous step number to pass to drawPage for submodel preview
                  int showStepNum = opts.contStepNumber == 1 ? stepNumber : opts.contStepNumber;
                  if (opts.pageNum == 1) {
                      meta.LPub.subModel.showStepNum.setValue(showStepNum);
                  } else {
                      saveMeta.LPub.subModel.showStepNum.setValue(showStepNum);
                  }
              }
              // Steps within step group modify bfxStore2 as they progress
              // so we must save bfxStore2 and use the saved copy when
              // we call drawPage for a step group.
              stepGroupBfxStore2 = bfxStore2;
              break;

            case StepGroupEndRc:
              if (stepGroup && ! noStep2 && ! buildModIgnore) {
                  stepGroup = false;

                  if (isPreDisplayPage/*opts.pageNum < displayPageNum*/) {
                      saveLineTypeIndexes    = lineTypeIndexes;
                      saveCsiParts           = csiParts;
                      saveStepNumber         = stepNumber;
                      saveMeta               = meta;
                      saveBfx                = bfx;
                      saveBfxParts           = bfxParts;
                      saveBfxLineTypeIndexes = bfxLineTypeIndexes;
                      saveRotStep            = meta.rotStep;
                      bfxParts.clear();
                  } else if (isDisplayPage/*opts.pageNum == displayPageNum*/) {
                      // ignored when processing a buildModDisplay
                      savePrevStepPosition = saveCsiParts.size();
                      stepPageNum = saveStepPageNum;
                      if (opts.pageNum == 1) {
                          page.meta = meta;
                      } else {
                          page.meta = saveMeta;
                      }
                      if (opts.contStepNumber) {  // pass continuous step number to drawPage
                          page.meta.LPub.contModelStepNum.setValue(saveStepNumber);
                          saveStepNumber = saveContStepNum;
                      }
                      page.meta.pop();
                      page.meta.rotStep = saveRotStep;

                      QStringList pliParts;
                      DrawPageOptions pageOptions(
                                  stepGroupCurrent,
                                  saveCsiParts,
                                  pliParts,
                                  saveBfxParts,
                                  ldrStepFiles,
                                  csiKeys,
                                  saveBfx,
                                  emptyPartGroups,
                                  saveLineTypeIndexes,
                                  saveBfxLineTypeIndexes,
                                  saveStepNumber,
                                  opts.renderStepNumber,
                                  opts.isMirrored,
                                  opts.printing,
                                  opts.buildModLevel,
                                  stepGroupBfxStore2,
                                  false /*assembledCallout*/,
                                  false /*calldeOut*/);
                      if (drawPage(view, scene, &page, addLine, pageOptions) == HitBuildModAction) {
                          // return to init drawPage to rerun findPage to regenerate content
                          return HitBuildModAction;
                      }

                      lineTypeIndexes.clear();
                      csiParts.clear();

                      saveCurrent.modelName.clear();
                      saveCsiParts.clear();
                      saveLineTypeIndexes.clear();
                  } // isDisplayPage/*opts.pageNum == displayPageNum*/

                  // ignored when processing buildMod display
                  if (exporting()) {
                      pageSizes.remove(opts.pageNum);
                      if (pageSizeUpdate) {
                          pageSizeUpdate = false;
                          pageSizes.insert(opts.pageNum,opts.pageSize);
#ifdef SIZE_DEBUG
                          logTrace() << "SG: Inserting New Page size info     at PageNumber:" << opts.pageNum
                                     << "W:"    << opts.pageSize.sizeW << "H:"    << opts.pageSize.sizeH
                                     << "O:"    <<(opts.pageSize.orientation == Portrait ? "Portrait" : "Landscape")
                                     << "ID:"   << opts.pageSize.sizeID
                                     << "Model:" << opts.current.modelName;
#endif
                      } else {
                          pageSizes.insert(opts.pageNum,pageSizes[DEF_SIZE]);
#ifdef SIZE_DEBUG
                          logTrace() << "SG: Inserting Default Page size info at PageNumber:" << opts.pageNum
                                     << "W:"    << pageSizes[DEF_SIZE].sizeW << "H:"    << pageSizes[DEF_SIZE].sizeH
                                     << "O:"    << (pageSizes[DEF_SIZE].orientation == Portrait ? "Portrait" : "Landscape")
                                     << "ID:"   << pageSizes[DEF_SIZE].sizeID
                                     << "Model:" << opts.current.modelName;
#endif
                      }
                  } // exporting
                  ++opts.pageNum;
                  topOfPages.append(opts.current);  // TopOfSteps (StepGroup)
                  saveStepPageNum = ++stepPageNum;
                } // StepGroup
              noStep2 = false;
              break;

              // Get BuildMod attributes and set ignore based on 'next' step buildModAction
              case BuildModBeginRc:
                if (!Preferences::buildModEnabled)
                    break;
                if (isPreDisplayPage/*opts.pageNum < displayPageNum*/) {
                     if ((buildModFile = opts.current.lineNumber < getBuildModStepLineNumber(getBuildModNextStepIndex()))) {
                        buildModKey          = meta.LPub.buildMod.key();
                        opts.buildModLevel   = getLevel(buildModKey, BM_BEGIN);
                        buildModBeginLineNum = opts.current.lineNumber;
                        buildModAction       = Rc(getBuildModAction(buildModKey,getBuildModNextStepIndex()));
                        if (buildModAction   == BuildModApplyRc){
                            buildModIgnore = false;
                        } else if (buildModAction == BuildModRemoveRc) {
                            buildModIgnore = true;
                        }
                    }
                }
                break;

              // Set buildModIgnore based on 'next' step buildModAction
              case BuildModEndModRc:
                if (Preferences::buildModEnabled && buildModFile) {
                    if (opts.buildModLevel > 1 && meta.LPub.buildMod.key().isEmpty())
                        parseError("Key required for nested build mod meta command",
                                   opts.current,Preferences::BuildModErrors);
                    if (buildModAction == BuildModApplyRc){
                        buildModIgnore = true;
                    } else if (buildModAction == BuildModRemoveRc) {
                        buildModIgnore = false;
                    }
                }
                break;

              // Get buildModLevel and reset buildModIgnore to default
              case BuildModEndRc:
                if (Preferences::buildModEnabled && buildModFile) {
                    opts.buildModLevel = getLevel(QString(), BM_END);
                    buildModIgnore = false;
                    buildModFile   = false;
                }
                break;

            case RotStepRc:
            case StepRc:
              if (partsAdded && ! noStep && ! buildModIgnore) {
                  if (opts.contStepNumber) {   // increment continuous step number until we hit the display page
                      if (isPreDisplayPage/*opts.pageNum < displayPageNum*/ &&
                         (stepNumber > FIRST_STEP || displayPageNum > FIRST_PAGE)) { // skip the first step
                          opts.contStepNumber += ! coverPage && ! stepPage;
                      }
                      if (! stepGroup && stepNumber == 1) {
                          if (opts.pageNum == 1 && topOfStep.modelName == topLevelFile()) { // when pageNum is 1 and not multistep, persist contStepNumber to 'meta' only if we are in the main model
                              meta.LPub.subModel.showStepNum.setValue(opts.contStepNumber);
                          } else {
                              saveMeta.LPub.subModel.showStepNum.setValue(opts.contStepNumber);
                          }
                      }
                  }
                  stepNumber  += ! coverPage && ! stepPage;
                  stepPageNum += ! coverPage && ! stepGroup;
                  if (isPreDisplayPage/*opts.pageNum < displayPageNum*/) {
                      if ( ! stepGroup) {
                          saveLineTypeIndexes    = lineTypeIndexes;
                          saveStepNumber         = stepNumber;
                          saveCsiParts           = csiParts;
                          saveMeta               = meta;
                          saveBfx                = bfx;
                          saveBfxParts           = bfxParts;
                          saveBfxLineTypeIndexes = bfxLineTypeIndexes;
                          saveStepPageNum        = stepPageNum;
                          // bfxParts.clear();
                        }
                      if (opts.contStepNumber) { // save continuous step number from current model
                          saveContStepNum = opts.contStepNumber;
                      }
                      saveCurrent = opts.current;
                      saveRotStep = meta.rotStep;
                    } // isPreDisplayPage/*opts.pageNum < displayPageNum*/

                  if ( ! stepGroup) {
                      if (isDisplayPage/*opts.pageNum == displayPageNum*/) {
                          lineTypeIndexes.clear();
                          csiParts.clear();
                          savePrevStepPosition = saveCsiParts.size();
                          stepPageNum = saveStepPageNum;
                          if (opts.pageNum == 1) {
                              page.meta = meta;
                          } else {
                              page.meta = saveMeta;
                          }
                          if (opts.contStepNumber) { // pass continuous step number to drawPage
                              page.meta.LPub.contModelStepNum.setValue(saveStepNumber);
                              saveStepNumber = opts.contStepNumber;
                          }
                          page.meta.pop();
                          page.meta.LPub.buildMod.clear();
                          page.meta.rotStep = saveRotStep;
                          page.meta.rotStep = meta.rotStep;

                          QStringList pliParts;
                          DrawPageOptions pageOptions(
                                      saveCurrent,
                                      saveCsiParts,
                                      pliParts,
                                      saveBfxParts,
                                      ldrStepFiles,
                                      csiKeys,
                                      saveBfx,
                                      emptyPartGroups,
                                      saveLineTypeIndexes,
                                      saveBfxLineTypeIndexes,
                                      saveStepNumber,
                                      opts.renderStepNumber,
                                      opts.isMirrored,
                                      opts.printing,
                                      opts.buildModLevel,
                                      bfxStore2);

                          if (drawPage(view, scene, &page, addLine, pageOptions) == HitBuildModAction) {
                              // Set opts.current to topOfStep
                              opts.current = pageOptions.current;
                              // rerun findPage to reflect change in pre-displayPageNum csiParts
                              return HitBuildModAction;
                          }

                          saveCurrent.modelName.clear();
                          saveCsiParts.clear();
                          saveLineTypeIndexes.clear();
                        } // isDisplayPage/*opts.pageNum == displayPageNum*/

                      if (exporting()) {
                          pageSizes.remove(opts.pageNum);
                          if (pageSizeUpdate) {
                              pageSizeUpdate = false;
                              pageSizes.insert(opts.pageNum,opts.pageSize);
#ifdef SIZE_DEBUG
                              logTrace() << "ST: Inserting New Page size info     at PageNumber:" << opts.pageNum
                                         << "W:"    << opts.pageSize.sizeW << "H:"    << opts.pageSize.sizeH
                                         << "O:"    <<(opts.pageSize.orientation == Portrait ? "Portrait" : "Landscape")
                                         << "ID:"   << opts.pageSize.sizeID
                                         << "Model:" << opts.current.modelName;
#endif
                            } else {
                              pageSizes.insert(opts.pageNum,pageSizes[DEF_SIZE]);
#ifdef SIZE_DEBUG
                              logTrace() << "ST: Inserting Default Page size info at PageNumber:" << opts.pageNum
                                         << "W:"    << pageSizes[DEF_SIZE].sizeW << "H:"    << pageSizes[DEF_SIZE].sizeH
                                         << "O:"    << (pageSizes[DEF_SIZE].orientation == Portrait ? "Portrait" : "Landscape")
                                         << "ID:"   << pageSizes[DEF_SIZE].sizeID
                                         << "Model:" << opts.current.modelName;
#endif
                            }
                        } // exporting

                      ++opts.pageNum;
                      topOfPages.append(opts.current); // TopOfStep (Step)
                    } // ! StepGroup

                  topOfStep = opts.current;
                  partsAdded = 0;
                  meta.pop();
                  meta.LPub.buildMod.clear();
                  coverPage = false;
                  stepPage = false;
                  bfxStore2 = bfxStore1;
                  bfxStore1 = false;
                  if ( ! bfxStore2) {
                      bfxParts.clear();
                    }
                } else if ( ! stepGroup) {
                  saveCurrent = opts.current;  // so that draw page doesn't have to
                  // deal with steps that are not steps
                }
              noStep2 = noStep;
              noStep = false;
              break;

            case CalloutBeginRc:
              callout = true;
              break;

            case CalloutEndRc:
              callout = false;
              meta.LPub.callout.placement.clear();
              break;

            case InsertCoverPageRc:
              coverPage  = true;
              partsAdded = true;
              break;

            case InsertPageRc:
              stepPage   = true;
              partsAdded = true;
              break;

            case PartBeginIgnRc:
              partIgnore = true;
              break;

            case PartEndRc:
              partIgnore = false;
              break;

              // Any of the metas that can change csiParts needs
              // to be processed here

            case ClearRc:
              if (isPreDisplayPage/*opts.pageNum < displayPageNum*/) {
                  csiParts.clear();
                  saveCsiParts.clear();
                  lineTypeIndexes.clear();
                  saveLineTypeIndexes.clear();
                }
              break;

              /* Buffer exchange */
            case BufferStoreRc:
              if (isPreDisplayPage/*opts.pageNum < displayPageNum*/) {
                  bfx[meta.bfx.value()] = csiParts;
                  bfxLineTypeIndexes[meta.bfx.value()] = lineTypeIndexes;
                }
              bfxStore1 = true;
              bfxParts.clear();
              break;

            case BufferLoadRc:
              if (isPreDisplayPage/*opts.pageNum < displayPageNum*/) {
                  csiParts = bfx[meta.bfx.value()];
                  lineTypeIndexes = bfxLineTypeIndexes[meta.bfx.value()];
                }
              partsAdded = true;
              break;

            case PartNameRc:
            case PartTypeRc:
            case MLCadGroupRc:
            case LDCadGroupRc:
            case LeoCadModelRc:
            case LeoCadPieceRc:
            case LeoCadCameraRc:
            case LeoCadLightRc:
            case LeoCadLightWidthRc:
            case LeoCadLightTypeRc:
            case LeoCadSynthRc:
            case LeoCadGroupBeginRc:
            case LeoCadGroupEndRc:
              if (isPreDisplayPage/*opts.pageNum < displayPageNum*/) {
                  CsiItem::partLine(line,pla,opts.current.lineNumber,rc);
                  partsAdded = true;
                }
              break;

              /* remove a group or all instances of a part type */
            case RemoveGroupRc:
            case RemovePartTypeRc:
            case RemovePartNameRc:
              if (! buildModIgnore) {
                if (isPreDisplayPage/*opts.pageNum < displayPageNum*/) {
                    QStringList newCSIParts;
                    QVector<int> newLineTypeIndexes;
                    if (rc == RemoveGroupRc) {
                        remove_group(csiParts,lineTypeIndexes,meta.LPub.remove.group.value(),newCSIParts,newLineTypeIndexes,meta);
                    } else if (rc == RemovePartTypeRc) {
                        remove_parttype(csiParts,lineTypeIndexes,meta.LPub.remove.parttype.value(),newCSIParts,newLineTypeIndexes);
                    } else {
                        remove_partname(csiParts,lineTypeIndexes,meta.LPub.remove.partname.value(),newCSIParts,newLineTypeIndexes);
                    }
                    csiParts = newCSIParts;
                    lineTypeIndexes = newLineTypeIndexes;
                  }
              } // ! buildModIgnore
              break;

            case IncludeRc:
              includeFileRc = Rc(include(meta,includeHere,inserted)); // includeHere and inserted are include(...) vars
              if (includeFileRc != EndOfFileRc) {                     // still reading so continue
                  resetIncludeRc = false;                             // do not reset, allow includeFileRc to execute
                  continue;
              }
              break;

            case PageSizeRc:
              {
                if (exporting()) {
                    pageSizeUpdate  = true;

                    opts.pageSize.sizeW  = meta.LPub.page.size.valueInches(0);
                    opts.pageSize.sizeH  = meta.LPub.page.size.valueInches(1);
                    opts.pageSize.sizeID = meta.LPub.page.size.valueSizeID();

                    pageSizes.remove(DEF_SIZE);
                    pageSizes.insert(DEF_SIZE,opts.pageSize);
#ifdef SIZE_DEBUG
                    logTrace() << "1. New Page Size entry for Default  at PageNumber:" << opts.pageNum
                               << "W:"  << opts.pageSize.sizeW << "H:"    << opts.pageSize.sizeH
                               << "O:"  << (opts.pageSize.orientation == Portrait ? "Portrait" : "Landscape")
                               << "ID:" << opts.pageSize.sizeID
                               << "Model:" << opts.current.modelName;
#endif
                  }
              }
              break;

            case CountInstanceRc:
              countInstances = meta.LPub.countInstance.value();
              break;

            case ContStepNumRc:
              {
                  if (meta.LPub.contStepNumbers.value()) {
                      if (! opts.contStepNumber)
                          opts.contStepNumber++;
                  } else {
                      opts.contStepNumber = 0;
                  }
              }
              break;

            case BuildModEnableRc:
              {
                bool value = meta.LPub.buildModEnabled.value();
                if (Preferences::buildModEnabled != value) {
                    Preferences::buildModEnabled  = value;
                    reset3DViewerMenusAndToolbars();
                    emit messageSig(LOG_INFO, QString("Build Modifications are %1")
                                                      .arg(value ? "Enabled" : "Disabled"));
                }
              }
              break;

            case PageOrientationRc:
              {
                if (exporting()){
                    pageSizeUpdate      = true;

                    if (opts.pageSize.sizeW == 0.0f)
                      opts.pageSize.sizeW    = pageSizes[DEF_SIZE].sizeW;
                    if (opts.pageSize.sizeH == 0.0f)
                      opts.pageSize.sizeH    = pageSizes[DEF_SIZE].sizeH;
                    if (opts.pageSize.sizeID.isEmpty())
                      opts.pageSize.sizeID   = pageSizes[DEF_SIZE].sizeID;
                    opts.pageSize.orientation= meta.LPub.page.orientation.value();

                    pageSizes.remove(DEF_SIZE);
                    pageSizes.insert(DEF_SIZE,opts.pageSize);
#ifdef SIZE_DEBUG
                    logTrace() << "1. New Orientation entry for Default at PageNumber:" << opts.pageNum
                               << "W:"  << opts.pageSize.sizeW << "H:"    << opts.pageSize.sizeH
                               << "O:"  << (opts.pageSize.orientation == Portrait ? "Portrait" : "Landscape")
                               << "ID:" << opts.pageSize.sizeID
                               << "Model:" << opts.current.modelName;
#endif
                  }
              }
              break;

            case NoStepRc:
              noStep = true;
              break;
            default:
              break;
            } // switch
          break;
        }
    } // for every line

  csiParts.clear();
  lineTypeIndexes.clear();

  // last step in submodel
  if (partsAdded && ! noStep && ! buildModIgnore) {
      // increment continuous step number
      // save continuous step number from current model
      // pass continuous step number to drawPage
      if (opts.contStepNumber) {
          if (! countInstances && isPreDisplayPage/*opts.pageNum < displayPageNum*/ &&
             (stepNumber > FIRST_STEP || displayPageNum > FIRST_PAGE)) {
              opts.contStepNumber += ! coverPage && ! stepPage;
          }
          if (isDisplayPage/*opts.pageNum == displayPageNum*/) {
              saveMeta.LPub.contModelStepNum.setValue(saveStepNumber);
              saveStepNumber = opts.contStepNumber;
          }
          saveContStepNum = opts.contStepNumber;
      }

      if (isDisplayPage/*opts.pageNum == displayPageNum*/) {
          savePrevStepPosition = saveCsiParts.size();
          page.meta = saveMeta;
          QStringList pliParts;
          DrawPageOptions pageOptions(
                      saveCurrent,
                      saveCsiParts,
                      pliParts,
                      saveBfxParts,
                      ldrStepFiles,
                      csiKeys,
                      saveBfx,
                      emptyPartGroups,
                      saveLineTypeIndexes,
                      saveBfxLineTypeIndexes,
                      saveStepNumber,
                      opts.renderStepNumber,
                      opts.isMirrored,
                      opts.printing,
                      opts.buildModLevel,
                      bfxStore2);
          if (drawPage(view, scene, &page, addLine, pageOptions) == HitBuildModAction) {
              // Set opts.current to topOfStep
              opts.current = pageOptions.current;
              // rerun findPage to reflect change in pre-displayPageNum csiParts
              return HitBuildModAction;
          }
        }
      if (exporting()) {
          pageSizes.remove(opts.pageNum);
          if (pageSizeUpdate) {
              pageSizeUpdate = false;
              pageSizes.insert(opts.pageNum,opts.pageSize);
#ifdef SIZE_DEBUG
              logTrace() << "PG: Inserting New Page size info     at PageNumber:" << opts.pageNum
                         << "W:"    << opts.pageSize.sizeW << "H:"    << opts.pageSize.sizeH
                         << "O:"    <<(opts.pageSize.orientation == Portrait ? "Portrait" : "Landscape")
                         << "ID:"   << opts.pageSize.sizeID
                         << "Model:" << opts.current.modelName;
#endif
            } else {
              pageSizes.insert(opts.pageNum,pageSizes[DEF_SIZE]);
#ifdef SIZE_DEBUG
              logTrace() << "PG: Inserting Default Page size info at PageNumber:" << opts.pageNum
                         << "W:"    << pageSizes[DEF_SIZE].sizeW << "H:"    << pageSizes[DEF_SIZE].sizeH
                         << "O:"    << (pageSizes[DEF_SIZE].orientation == Portrait ? "Portrait" : "Landscape")
                         << "ID:"   << pageSizes[DEF_SIZE].sizeID
                         << "Model:" << opts.current.modelName;
#endif
            }
        } // exporting
      ++opts.pageNum;
      topOfPages.append(opts.current); // TopOfStep (Last Step)
      ++stepPageNum;
    }  // Last Step in Submodel
  return 0;
}

int Gui::getBOMParts(
    Where        current,
    QString     &addLine,
    QStringList &pliParts,
    QList<PliPartGroupMeta> &bomPartGroups)
{
  QString buildModKey;
  bool partIgnore   = false;
  bool pliIgnore    = false;
  bool synthBegin   = false;
  bool bfxStore1    = false;
  bool bfxStore2    = false;
  bool bfxLoad      = false;
  bool partsAdded   = false;
  bool excludedPart = false;

  bool buildModIgnore = false;
  int  buildModLevel  = 0;

  QStringList bfxParts;

  Meta meta;

  skipHeader(current);

  QHash<QString, QStringList> bfx;

  int numLines = ldrawFile.size(current.modelName);

  Rc rc;

  for ( ;
        current.lineNumber < numLines;
        current.lineNumber++) {

      // scan through the rest of the model counting pages
      // if we've already hit the display page, then do as little as possible

      QString line = ldrawFile.readLine(current.modelName,current.lineNumber).trimmed();

      if (line.startsWith("0 GHOST ")) {
          line = line.mid(8).trimmed();
        }

      switch (line.toLatin1()[0]) {
        case '1':
          /* check if part is in excludedPart.lst*/
          excludedPart = ExcludedParts::lineHasExcludedPart(line);

          if ( ! excludedPart && ! partIgnore && ! pliIgnore && ! buildModIgnore && ! synthBegin) {

              QStringList token,addToken;

              split(line,token);

              QString    type = token[token.size()-1];

              if (token[1] == "16") {
                  split(addLine,addToken);
                  if (addToken.size() == 15) {
                      token[1] = addToken[1];
                      line = token.join(" ");
                    }
                }

          /*
           * Automatically ignore parts added twice due to buffer exchange
           */
              bool removed = false;
              QString colorPart = QString("%1%2%3").arg(current.lineNumber).arg(token[1]).arg(type);

              if (bfxStore2 && bfxLoad) {
                  int i;
                  for (i = 0; i < bfxParts.size(); i++) {
                      if (bfxParts[i] == colorPart) {
                          emit messageSig(LOG_NOTICE, QString("Duplicate PliPart at line [%1] removed [%2].")
                                          .arg(current.lineNumber).arg(line));
                          bfxParts.removeAt(i);
                          removed = true;
                          break;
                        }
                    }
                }

              if ( ! removed) {

                  if (ldrawFile.isSubmodel(type)) {

                      Where current2(type,0);

                      getBOMParts(current2,line,pliParts,bomPartGroups);

                    } else {

                      /*  check if alternative part exist and replace */
                      if(PliSubstituteParts::hasSubstitutePart(type)) {

                          QStringList substituteToken;
                          split(line,substituteToken);
                          QString substitutePart = type;

                          if (PliSubstituteParts::getSubstitutePart(substitutePart)){
                              substituteToken[substituteToken.size()-1] = substitutePart;
                            }
                          line = substituteToken.join(" ");
                        }

                      QString newLine = Pli::partLine(line,current,meta);

                      pliParts << newLine;
                    }
                }

              if (bfxStore1) {
                  bfxParts << colorPart;
                }

              partsAdded = true;
            }
          break;
        case '0':
          rc = meta.parse(line,current);

          /* substitute part/parts with this */

          switch (rc) {
            case PliBeginSub1Rc:
            case PliBeginSub2Rc:
            case PliBeginSub3Rc:
            case PliBeginSub4Rc:
            case PliBeginSub5Rc:
            case PliBeginSub6Rc:
            case PliBeginSub7Rc:
            case PliBeginSub8Rc:
              if (! pliIgnore &&
                  ! partIgnore &&
                  ! buildModIgnore &&
                  ! synthBegin) {
                  QString addPart = QString("1 %1 0 0 0 1 0 0 0 1 0 0 0 1 %2")
                                            .arg(meta.LPub.pli.begin.sub.value().color)
                                            .arg(meta.LPub.pli.begin.sub.value().part);
                  pliParts << Pli::partLine(addPart,current,meta);
                  pliIgnore = true;
                }
              break;

            case PliBeginIgnRc:
              pliIgnore = true;
              break;

            case PliEndRc:
              pliIgnore = false;
              meta.LPub.pli.begin.sub.clearAttributes();
              break;

            case PartBeginIgnRc:
              partIgnore = true;
              break;

            case PartEndRc:
              partIgnore = false;
              break;

            case SynthBeginRc:
              synthBegin = true;
              break;

            case SynthEndRc:
              synthBegin = false;
              break;

              /* Buffer exchange */
            case BufferStoreRc:
              bfxStore1 = true;
              bfxParts.clear();
              break;
            case BufferLoadRc:
              bfxLoad = true;
              break;
            case BomPartGroupRc:
              meta.LPub.bom.pliPartGroup.setWhere(current);
              meta.LPub.bom.pliPartGroup.setBomPart(true);
              bomPartGroups.append(meta.LPub.bom.pliPartGroup);
              break;

              // Any of the metas that can change pliParts needs
              // to be processed here

            case ClearRc:
              pliParts.empty();
              break;

              /*
               * For getBOMParts(), the BuildMod behaviour only processes
               * top level mods for pliParts, nested mods are ignored.
               * The aim is to process original pli parts and ignore those
               * that are in the build modification block.
               *
               * The buildModLevel flag is enabled (greater than or equal to 1) for the lines between
               * BUILD_MOD BEGIN and BUILD_MOD END
               * Lines between BUILD_MOD BEGIN and BUILD_MOD END_MOD represent
               * the modified content
               * Lines between BUILD_MOD END_MOD and BUILD_MOD END
               * represent the original content
               *
               * When processing a modification block, we end at the build
               * mod action meta command 'BuildModEndModRc'
               * to include the original PLI parts as they are not added
               * in PLI::partLine as they are for CSI parts.
               *
               * BUILD_MOD APPLY or BUILD_MOD REMOVE action meta commands
               * are ignored
               *
               */

            case BuildModBeginRc:
              buildModKey    = meta.LPub.buildMod.key();
              buildModLevel  = getLevel(buildModKey, BM_BEGIN);
              buildModIgnore = true;
              break;

            case BuildModEndModRc:
              buildModLevel = getLevel(QString(), BM_END);
              if (!buildModLevel)
                  buildModIgnore = false;
              break;

            case PartNameRc:
            case PartTypeRc:
            case MLCadGroupRc:
            case LDCadGroupRc:
            case LeoCadGroupBeginRc:
            case LeoCadGroupEndRc:
              pliParts << Pli::partLine(line,current,meta);
              break;

              /* remove a group or all instances of a part type */
            case RemoveGroupRc:
            case RemovePartTypeRc:
            case RemovePartNameRc:
              if (! buildModIgnore) {
                  QStringList newPLIParts;
                  QVector<int> dummy;
                  if (rc == RemoveGroupRc) {
                      remove_group(pliParts,dummy,meta.LPub.remove.group.value(),newPLIParts,dummy,meta);
                  } else if (rc == RemovePartTypeRc) {
                      remove_parttype(pliParts,dummy,meta.LPub.remove.parttype.value(),newPLIParts,dummy);
                  } else {
                      remove_partname(pliParts,dummy,meta.LPub.remove.partname.value(),newPLIParts,dummy);
                  }
                  pliParts = newPLIParts;
              }
              break;

            case StepRc:

              if (partsAdded) {
                  bfxStore2 = bfxStore1;
                  bfxStore1 = false;
                  bfxLoad = false;
                  if ( ! bfxStore2) {
                      bfxParts.clear();
                    }
                }
              partsAdded = false;
              break;

            default:
              break;
            } // switch
          break;
        }
    } // for every line
  return 0;
}

int Gui::getBOMOccurrence(Where	current) {		// start at top of ldrawFile

  // traverse content to find the number and location of BOM pages
  // key=modelName_LineNumber, value=occurrence
  QHash<QString, int> bom_Occurrence;
  Meta meta;

  skipHeader(current);

  int numLines        = ldrawFile.size(current.modelName);
  int occurrenceNum   = 0;
  boms                = 0;
  bomOccurrence       = 0;
  Rc rc;

  for ( ; current.lineNumber < numLines;
        current.lineNumber++) {

      QString line = ldrawFile.readLine(current.modelName,current.lineNumber).trimmed();
      switch (line.toLatin1()[0]) {
        case '1':
          {
            QStringList token;
            split(line,token);
            QString type = token[token.size()-1];

            if (ldrawFile.isSubmodel(type)) {
                Where current2(type,0);
                getBOMOccurrence(current2);
              }
            break;
          }
        case '0':
          {
            rc = meta.parse(line,current);
            switch (rc) {
              case InsertRc:
                {
                  InsertData insertData = meta.LPub.insert.value();
                  if (insertData.type == InsertData::InsertBom){

                      QString uniqueID = QString("%1_%2").arg(current.modelName).arg(current.lineNumber);
                      occurrenceNum++;
                      bom_Occurrence[uniqueID] = occurrenceNum;
                    }
                }
                break;
              default:
                break;
              } // switch metas
            break;
          }  // switch line type
        }
    } // for every line

  if (occurrenceNum > 1) {
      // now set the bom occurrance based on our current position
      Where here = topOfPages[displayPageNum-1];
      for (++here; here.lineNumber < ldrawFile.size(here.modelName); here++) {
          QString line = readLine(here);
          Meta meta;
          Rc rc;

          rc = meta.parse(line,here);
          if (rc == InsertRc) {

              InsertData insertData = meta.LPub.insert.value();
              if (insertData.type == InsertData::InsertBom) {

                  QString bomID   = QString("%1_%2").arg(here.modelName).arg(here.lineNumber);
                  bomOccurrence   = bom_Occurrence[bomID];
                  boms            = bom_Occurrence.size();
                  break;
                }
            }
        }
    }
  return 0;
}

bool Gui::generateBOMPartsFile(const QString &bomFileName){
    QString addLine;
    QStringList tempParts, bomParts;
    QList<PliPartGroupMeta> bomPartGroups;
    Where current(ldrawFile.topLevelFile(),0);
    getBOMParts(current,addLine,tempParts,bomPartGroups);

    if (! tempParts.size()) {
        emit messageSig(LOG_ERROR,QMessageBox::tr("No BOM parts were detected."));
        return false;
    }

    foreach (QString bomPartsString, tempParts){
        if (bomPartsString.startsWith("1")) {
            QStringList partComponents = bomPartsString.split(";");
            bomParts << partComponents.at(0);
            emit messageSig(LOG_DEBUG,QMessageBox::tr("%1 added to export list.").arg(partComponents.at(0)));
        }
    }
    emit messageSig(LOG_INFO,QMessageBox::tr("%1 BOM parts processed.").arg(bomParts.size()));

    // create a BOM parts file
    QFile bomFile(bomFileName);
    if ( ! bomFile.open(QIODevice::WriteOnly)) {
        emit messageSig(LOG_ERROR,QMessageBox::tr("Cannot open BOM parts file for writing: %1, %2.")
                              .arg(bomFileName)
                              .arg(bomFile.errorString()));
        return false;
    }

    QTextStream out(&bomFile);
    out << QString("0 Name: %1").arg(QFileInfo(bomFileName).fileName()) << endl;
    foreach (QString bomPart, bomParts)
        out << bomPart << endl;
    bomFile.close();
    return true;
}

void Gui::attitudeAdjustment()
{
  Meta meta;
  bool callout = false;
  int numFiles = ldrawFile.subFileOrder().size();

  for (int i = 0; i < numFiles; i++) {
      QString fileName = ldrawFile.subFileOrder()[i];
      int numLines     = ldrawFile.size(fileName);

      QStringList pending;

      for (Where current(fileName,0);
           current.lineNumber < numLines;
           current.lineNumber++) {

          QString line = ldrawFile.readLine(current.modelName,current.lineNumber);
          QStringList argv;
          split(line,argv);

          if (argv.size() >= 4 &&
              argv[0] == "0" &&
              (argv[1] == "!LPUB" || argv[1] == "LPUB") &&
              argv[2] == "CALLOUT") {
              if (argv.size() == 4 && argv[3] == "BEGIN") {
                  callout = true;
                  pending.clear();
                } else if (argv[3] == "END") {
                  callout = false;
                  for (int i = 0; i < pending.size(); i++) {
                      ldrawFile.insertLine(current.modelName,current.lineNumber, pending[i]);
                      ++numLines;
                      ++current;
                    }
                  pending.clear();
                } else if (argv[3] == "ALLOC" ||
                           argv[3] == "BACKGROUND" ||
                           argv[3] == "BORDER" ||
                           argv[3] == "MARGINS" ||
                           argv[3] == "PLACEMENT") {
                  if (callout && argv.size() >= 5 && argv[4] != "GLOBAL") {
                      ldrawFile.deleteLine(current.modelName,current.lineNumber);
                      pending << line;
                      --numLines;
                      --current;
                    }
                }
            }
        }
    }
}


void Gui::countPages()
{
  if (maxPages < 1) {
      writeToTmp();
      statusBarMsg("Counting...");
      Where current(ldrawFile.topLevelFile(),0);
      int savedDpn     =  displayPageNum;
      displayPageNum   =  1 << 31;  // really large number: 2147483648
      firstStepPageNum = -1;
      lastStepPageNum  = -1;
      maxPages         =  1;
      Meta meta;
      QString empty;
      PgSizeData emptyPageSize;
      stepPageNum = 1;
      FindPageOptions findOptions(
                  maxPages,
                  current,
                  emptyPageSize,
                  false          /*mirrored*/,
                  false          /*printing*/,
                  0              /*buildModLevel*/,
                  0              /*contStepNumber*/,
                  0              /*renderStepNumber*/,
                  empty          /*renderParentModel*/);
      findPage(KpageView,KpageScene,meta,empty/*addLine*/,findOptions);
      topOfPages.append(current);
      maxPages--;

      if (displayPageNum > maxPages) {
          displayPageNum = maxPages;
        } else {
          displayPageNum = savedDpn;
        }
      QString string = QString("%1 of %2") .arg(displayPageNum) .arg(maxPages);
      setPageLineEdit->setText(string);
      statusBarMsg("");
    }
}

void Gui::drawPage(
    LGraphicsView  *view,
    LGraphicsScene *scene,
    bool            printing,
    bool            buildMod/*false*/)
{
  QApplication::setOverrideCursor(Qt::WaitCursor);

  Where current(ldrawFile.topLevelFile(),0);

  maxPages    = 1;
  stepPageNum = 1;
  currentStep = nullptr;

  // if not buildMod action
  if (! buildMod) {
      // initialize ldrawFile registers
      ldrawFile.unrendered();
      ldrawFile.countInstances();
      ldrawFile.setModelStartPageNumber(current.modelName,maxPages);

      // Set BuildMod action options for next step
      int displayPageIndx = exporting() ? displayPageNum : displayPageNum - 1;
      if (Preferences::buildModEnabled) {
          bool displayPageIndxOk = topOfPages.size() && topOfPages.size() >= displayPageIndx;
          QElapsedTimer t; t.start();

          setBuildModForNextStep(displayPageIndxOk ? topOfPages[displayPageIndx] : current);

          emit messageSig(LOG_DEBUG,QString("Build modifications check - %1")
                                            .arg(elapsedTime(t.elapsed())));
      }
  }

  writeToTmp();
  //logTrace() << "SET INITIAL Model: " << current.modelName << " @ Page: " << maxPages;
  QString empty;
  Meta    meta;
  firstStepPageNum = -1;
  lastStepPageNum  = -1;
  savePrevStepPosition = 0;
  saveContStepNum = 1;

  enableLineTypeIndexes = true;

  PgSizeData pageSize;
  if (exporting()) {
      pageSize.sizeW      = meta.LPub.page.size.valueInches(0);
      pageSize.sizeH      = meta.LPub.page.size.valueInches(1);
      pageSize.sizeID     = meta.LPub.page.size.valueSizeID();
      pageSize.orientation= meta.LPub.page.orientation.value();
      pageSizes.insert(     DEF_SIZE,pageSize);
#ifdef SIZE_DEBUG
      logTrace() << "0. Inserting INIT page size info    at PageNumber:" << DEF_SIZE
                 << "W:"  << pageSize.sizeW << "H:"    << pageSize.sizeH
                 << "O:"  << (pageSize.orientation == Portrait ? "Portrait" : "Landscape")
                 << "ID:" << pageSize.sizeID
                 << "Model:" << current.modelName;
#endif
    }
  FindPageOptions findOptions(
              maxPages,
              current,
              pageSize,
              false        /*mirrored*/,
              printing,
              0            /*buildModLevel*/,
              0            /*contStepNumber*/,
              0            /*renderStepNumber*/,
              empty        /*renderParentModel*/);
  if (findPage(view,scene,meta,empty/*addLine*/,findOptions) == HitBuildModAction && Preferences::buildModEnabled) {
      QApplication::restoreOverrideCursor();
      clearPage(KpageView,KpageScene);
      drawPage(view,scene,printing,true/*buildMod*/);
  } else {
      topOfPages.append(current);
/*
#ifdef QT_DEBUG_MODE
      emit messageSig(LOG_NOTICE, QString("DrawPage StepIndex"));
      for (int i = 0; i < topOfPages.size(); i++)
      {
          emit messageSig(LOG_NOTICE, QString("StepIndex: %1, SubmodelIndex: %2: LineNumber: %3, ModelName: %4")
                                             .arg(i)                                            // index
                                             .arg(getSubmodelIndex(topOfPages.at(i).modelName)) // modelIndex
                                             .arg(topOfPages.at(i).lineNumber)                  // lineNumber
                                             .arg(topOfPages.at(i).modelName));                 // modelName
      }
#endif
*/

      maxPages--;

      QString string = QString("%1 of %2") .arg(displayPageNum) .arg(maxPages);
      if (! exporting())
        setPageLineEdit->setText(string);

      QApplication::restoreOverrideCursor();
  }
}

void Gui::skipHeader(Where &current)
{
    int numLines = ldrawFile.size(current.modelName);
    for ( ; current.lineNumber < numLines; current.lineNumber++) {
        QString line = readLine(current);
        int p;
        for (p = 0; p < line.size(); ++p) {
            if (line[p] != ' ') {
                break;
            }
        }
        if (line[p] >= '1' && line[p] <= '5') {
            if (current.lineNumber > 0) {
                --current;
            }
            break;
        } else if ( ! isHeader(line)) {
            if (current.lineNumber != 0) {
                --current;
                break;
            }
        }
    }
}

int Gui::include(Meta &meta, Where &includeHere, bool &inserted)
{
    Rc rc;
    QString filePath = meta.LPub.include.value();
    QFileInfo fileInfo(filePath);
    QString fileName = fileInfo.fileName();
    if (includeHere == Where())
        includeHere = Where(fileName,0);

    auto processLine =
            [this,
             &meta,
             &fileName,
             &includeHere] () {
        Rc prc = InvalidLineRc;
        QString line = ldrawFile.readLine(fileName,includeHere.lineNumber);
        switch (line.toLatin1()[0]) {
        case '1':
            parseError(QString("Invalid include line [%1].<br>"
                               "Part lines (type 1 to 5) are ignored in include file.").arg(line),includeHere,Preferences::MsgKey::IncludeFileErrors);
            return prc;
        case '0':
            prc = meta.parse(line,includeHere);
            switch (prc) {
            // Add unsupported include file meta commands here - i.e. commands that involve type 1-5 lines
            case PliBeginSub1Rc:
            case PliBeginSub2Rc:
            case PliBeginSub3Rc:
            case PliBeginSub4Rc:
            case PliBeginSub5Rc:
            case PliBeginSub6Rc:
            case PliBeginSub7Rc:
            case PliBeginSub8Rc:
                parseError(QString("Substitute part meta commands are not supported in include file: [%1].<br>"
                                   "Add this command to the model file or to a submodel.").arg(line),includeHere,Preferences::MsgKey::IncludeFileErrors);
                return InvalidLineRc;
            default:
                break;
            }
            break;
        }
        return prc;
    };

    if (!inserted) {
        inserted = ldrawFile.isIncludeFile(fileName);
    }
    if (inserted) {
        int numLines = ldrawFile.size(fileName);
        for (; includeHere < numLines; includeHere++) {
            rc = processLine();
            if (rc != InvalidLineRc)
                break;
        }
        if (includeHere.lineNumber < numLines)
            includeHere++;
        else
            rc = EndOfFileRc;
        return rc;
    } else {
        QFile file(filePath);
        if ( ! file.open(QFile::ReadOnly | QFile::Text)) {
            emit messageSig(LOG_ERROR, QString("Cannot read include file %1<br>%2")
                            .arg(filePath)
                            .arg(file.errorString()));
            return EndOfFileRc;
        }

        emit messageSig(LOG_TRACE, QString("Loading include file '%1'...").arg(filePath));

        QTextStream in(&file);
        in.setCodec(ldrawFile._currFileIsUTF8 ? QTextCodec::codecForName("UTF-8") : QTextCodec::codecForName("System"));

        /* Read it in to put into subFiles in order of appearance */
        QStringList contents;
        while ( ! in.atEnd()) {
            QString line = in.readLine(0);
            contents << line.trimmed();
        }
        file.close();

        disableWatcher();
        QDateTime datetime = fileInfo.lastModified();
        ldrawFile.insert(fileName,contents,datetime,true/*unofficialPart*/,true/*generated*/,true/*includeFile*/,fileInfo.absoluteFilePath());

        int comboIndex = mpdCombo->count() - 1;
        if (ldrawFile.includeFileList().size() == 1) {
            mpdCombo->addSeparator();
            comboIndex++;
        }
        mpdCombo->addItem(QString("%1 - Include File").arg(fileName),fileName);
        comboIndex++;
        mpdCombo->setItemData(comboIndex, QBrush(Qt::blue), Qt::TextColorRole);
        enableWatcher();

        emit messageSig(LOG_TRACE, QString("Include file '%1' with %2 lines loaded.").arg(fileName).arg(contents.size()));

        rc = Rc(include(meta,includeHere,inserted));
    }
    return rc;
}

static Where dummy;

Where &Gui::topOfPage()
{
  int pageNum = displayPageNum - 1;
  if (pageNum < topOfPages.size()) {
      return topOfPages[pageNum];
    } else {
      return dummy;
    }
}

Where &Gui::bottomOfPage()
{
  if (displayPageNum < topOfPages.size()) {
      return topOfPages[displayPageNum];
    } else {
      return dummy;
    }
}

/*
 * For setBuildModForNextStep(), the BuildMod behaviour searches ahead for any BuildMod action meta command in 'next step'.
 *
 * Three operations are performed in this function:
 *
 * 1. BuildMod attributes and content line numbers are captured when BUILD_MOD BEGIN is detected
 * When BUILD_MOD END is detected, the BuildMod item is inserted into the BuildMod list in ldrawfile.
 * The buildModLevel flag uses the getLevel() function to determine the current BuildMod when mods are nested.
 * At this stage, the BuildMod action is set to BuildModApply by default.
 *
 * 2. The BuildMod action for the 'next' step being configured by this instance of writeToTmp() is updated.
 * The action update proceedes as follows: (a.) The index for the 'next' step is captured in buildModStepIndex
 * and used to correctly identify the BuldMod action slot. (b.) The corresponding BuildMod action is determined
 * snd subsequently updated when BUILD_MOD APPLY or BUILD_MOD REMOVE meta commands are encountered. These
 * commands must include their respective buildModKey
 *
 * 3. Part lines are written to to buildModCsiParts, their corresponding index is added to buildModLineTypeIndexes
 * and buildModItems (bool) is set to then number of mod lines. The buildModCsiParts is added to csiParts and
 * buildModLineTypeIndexes is added to lineTypeIndexes when buildModLevel is false (0).
 */

bool Gui::setBuildModForNextStep(
        Where topOfNextStep,
        Where bottomOfNextStep,
        Where topOfSubmodel,
        bool  change,
        bool  submodel)
{
    int  progressMin           = 0;
    int  progressMax           = 0;
    int  buildModLevel         = 0;
    int  buildModNextStepIndex = 0;
    int  buildModPrevStepIndex = 0;
    int  startLine             = topOfNextStep.lineNumber;               // always start at the top
    QString startModel         = submodel ? topOfSubmodel.modelName  : topOfNextStep.modelName;
    Where topOfStep            = submodel ? topOfSubmodel            : topOfNextStep;
    bool buildMod[3]           = { false, false, false };                // validate buildMod meta command set
    enum D_Step{ D_NEXT, D_JUMP_FORWARD, D_JUMP_BACKWARD };
    D_Step stepDir             = D_NEXT;

    emit messageSig(LOG_INFO_STATUS, QString("Build modifications check for %1 [%2]...")
                                             .arg(submodel ? "submodel" : "model").arg(startModel));

    auto setBottomOfNextStep = [
            this,
            &buildModNextStepIndex,
            &topOfStep] (Where &bottomOfNextStep) {
        Rc rc;
        MetaItem mi;
        Where walk = topOfStep + 1;
        getBuildModStepIndexHere(buildModNextStepIndex, bottomOfNextStep);         // initialize bottomOfNextStep Where

        rc = mi.scanForward(walk,StepMask|StepGroupMask);                          // check next step for step group command
        if (rc == StepGroupEndRc) {                                                // we have an end of previous step group
            rc = mi.scanForward(walk,StepGroupBeginMask);                          //   so scan forward again to start of step group
            if (rc == StepGroupBeginRc)                                            //   step starts a step group
                mi.scanForward(walk,StepGroupEndMask);                             //     scan forward to end of step group
        } else if (rc == StepGroupBeginRc) {                                       // step starts a step group
            mi.scanForward(walk,StepGroupEndMask);                                 //   scan forward to end of step group
        }
        bottomOfNextStep = walk;                                                   //   set bottomOfNextStep to bottom of step
    };

    if (!submodel) {

        buildModSubmodels.clear();

        if (!topOfStep.lineNumber)
            skipHeader(topOfStep);

        setBuildModNextStepIndex(topOfStep);                                       // set next step

        buildModNextStepIndex = getBuildModNextStepIndex();                        // set next/'display' step index

        buildModPrevStepIndex = getBuildModPrevStepIndex();                        // set previous step index;

        if ((buildModNextStepIndex - buildModPrevStepIndex) > 1) { // if jump forward, modify BuildMod processing start...
            stepDir = D_JUMP_FORWARD;
            Where topOfFromStep;
            getBuildModStepIndexHere(buildModPrevStepIndex, topOfFromStep);        // set start Where to previous step index
            startLine  = getBuildModStepLineNumber(buildModPrevStepIndex);         // set start Where lineNumber to bottom of previous step
            startModel = topOfFromStep.modelName;
            progressMax = 0;
            emit messageSig(LOG_NOTICE, QString("Jump forward - StartModel: %1, StartLineNum: %2, EndModel %3, EndLineNum %4")
                            .arg(startModel).arg(startLine)
                            .arg(bottomOfNextStep.modelName)
                            .arg(bottomOfNextStep.lineNumber));
        } else if ((buildModNextStepIndex - buildModPrevStepIndex) < 0) {
            // stepDir = D_JUMP_BACKWARD;
            emit messageSig(LOG_NOTICE, QString("Jump backward - StartModel: %1, StartLineNum: %2, EndModel %3, EndLineNum %4")
                            .arg(startModel).arg(startLine)
                            .arg(bottomOfNextStep.modelName)
                            .arg(bottomOfNextStep.lineNumber));
            // Nothing to do at jump backward as all models would have already been checked for modifications up to this point
            return true;
        }

        if (stepDir == D_NEXT) {
            setBottomOfNextStep(bottomOfNextStep);
            progressMax = bottomOfNextStep.lineNumber - topOfStep.lineNumber;             // progress bar max
            progressMin = 1;
#ifdef QT_DEBUG_MODE
            emit messageSig(LOG_DEBUG, QString("BuildMod bottomOfStep lineNumber [%1], step numberOfLines [%2]...")
                                              .arg(bottomOfNextStep.lineNumber).arg(progressMax));
#endif
        }

        emit progressBarPermInitSig();
        emit progressPermMessageSig("Build modification check...");
        emit progressPermRangeSig(progressMin, progressMax);
    }

    Rc rc;
    QString buildModKey;
    Rc buildModAction     = BuildModApplyRc;
    QMap<int, QString>      buildModKeys;
    QMap<int, Rc>           buildModActions;
    QMap<int, QVector<int>> buildModAttributes;

    auto insertAttribute =
            [this,
             &buildModLevel,
             &topOfStep] (
            QMap<int, QVector<int>> &buildModAttributes,
            int index, const Where &here)
    {
        int  fileNameIndex = getSubmodelIndex(topOfStep.modelName);
        QMap<int, QVector<int>>::iterator i = buildModAttributes.find(buildModLevel);
        if (i == buildModAttributes.end()) {
            QVector<int> modAttributes = { 0, 0, 0, 1/*placeholder*/, fileNameIndex, 0/*placeholder*/ };
            modAttributes[index] = here.lineNumber;
            buildModAttributes.insert(buildModLevel, modAttributes);
        } else {
            i.value()[index] = here.lineNumber;
        }
    };

    auto insertBuildModification =
            [this,
            &buildModNextStepIndex,
            &buildModAttributes,
            &buildModKeys,
            &buildModActions,
            &topOfStep] (int buildModLevel)
    {
        int fileNameIndex     = getSubmodelIndex(topOfStep.modelName);
        int modAction         = buildModActions.value(buildModLevel);
        int lineNumber        = topOfStep.lineNumber;
        QString buildModKey   = buildModKeys.value(buildModLevel);

        QString modStepKey = QString("%1;%2;%3")
                                     .arg(fileNameIndex)
                                     .arg(lineNumber)
                                     .arg(1/*placeholder*/);

        QVector<int> modAttributes = { 0, 0, 0, 1, -1, 0 };
        QMap<int, QVector<int>>::iterator i = buildModAttributes.find(buildModLevel);
        if (i != buildModAttributes.end())
            modAttributes = i.value();

        insertBuildMod(buildModKey,
                       modStepKey,
                       modAttributes,
                       modAction,
                       buildModNextStepIndex);
#ifdef QT_DEBUG_MODE
        emit messageSig(LOG_DEBUG, QString(
                        "Insert BuildMod StepIndx: %1, "
                        "Action: %2, "
                        "Attributes: %3 %4 %5 1* %6 0*, "
                        "StepKey: %7, "
                        "ModKey: %8, "
                        "Level: %9, *=placeholder")
                        .arg(buildModNextStepIndex)
                        .arg(modAction == BuildModApplyRc ? "Apply" : "Remove")
                        .arg(modAttributes.at(BM_BEGIN_LINE_NUM))
                        .arg(modAttributes.at(BM_ACTION_LINE_NUM))
                        .arg(modAttributes.at(BM_END_LINE_NUM))
                        .arg(fileNameIndex)
                        .arg(modStepKey)
                        .arg(buildModKey)
                        .arg(buildModLevel));
#endif
    };

    Where walk(startModel, startLine);
    QString line = readLine(walk);
    rc =  page.meta.parse(line, walk, false);
    if (rc == StepRc || rc == RotStepRc)
        walk++;   // Advance past STEP meta

    // next step lines index
    int stepLines = 0;
    int modelIndx = getSubmodelIndex(startModel);

    // Parse the step lines
    for ( ;
          walk.lineNumber < subFileSize(walk.modelName);
          walk.lineNumber++) {

        if (progressMax && modelIndx == buildModNextStepIndex) {
            stepLines++;
            emit progressPermSetValueSig(stepLines);
        }

        line = readLine(walk);

        if (line.toLatin1()[0] == '0') {

            Where here(walk.modelName,walk.lineNumber);
            rc =  page.meta.parse(line,here,false);

            switch (rc) {

            // Update BuildMod action for 'current' step
            case BuildModApplyRc:
            case BuildModRemoveRc:
                buildModKey = page.meta.LPub.buildMod.key();
                if (buildModContains(buildModKey))
                    buildModAction = Rc(getBuildModAction(buildModKey, buildModNextStepIndex));
                else
                    parseError(QString("BuildMod for key '%1' not found").arg(buildModKey),
                                        here,Preferences::ParseErrors);
                if (buildModAction != rc)
                    change = setBuildModAction(buildModKey, buildModNextStepIndex, rc);
                break;

            // Get BuildMod attributes and set buildModIgnore based on 'next' step buildModAction
            case BuildModBeginRc:
                buildModKey   = page.meta.LPub.buildMod.key();
                buildModLevel = getLevel(buildModKey, BM_BEGIN);
                buildModKeys.insert(buildModLevel, buildModKey);
                insertAttribute(buildModAttributes, BM_BEGIN_LINE_NUM, here);
                buildModActions.insert(buildModLevel, BuildModApplyRc);
                buildMod[BM_BEGIN] = true;
                break;

            // Set modActionLineNum and buildModIgnore based on 'next' step buildModAction
            case BuildModEndModRc:
                if (buildModLevel > 1 && page.meta.LPub.buildMod.key().isEmpty())
                    parseError("Key required for nested build mod meta command",
                               here,Preferences::BuildModErrors);
                if (!buildMod[BM_BEGIN])
                    parseError(QString("Required meta BUILD_MOD BEGIN not found"), here, Preferences::BuildModErrors);
                insertAttribute(buildModAttributes, BM_ACTION_LINE_NUM, here);
                buildMod[BM_END_MOD] = true;
                break;

            // Insert buildModAttributes and reset buildModLevel and buildModIgnore to default
            case BuildModEndRc:
                if (!buildMod[BM_END_MOD])
                    parseError(QString("Required meta BUILD_MOD END_MOD not found"), here, Preferences::BuildModErrors);
                insertAttribute(buildModAttributes, BM_END_LINE_NUM, here);
                buildModLevel    = getLevel(QString(), BM_END);
                buildMod[BM_END] = true;
                break;

            // Search until next step/rotstep meta
            case RotStepRc:
            case StepRc:
                if (buildMod[BM_BEGIN] && !buildMod[BM_END])
                    parseError(QString("Required meta BUILD_MOD END not found"), here, Preferences::BuildModErrors);
                foreach (int buildModLevel, buildModKeys.keys())
                    insertBuildModification(buildModLevel);
                topOfStep = here;
                buildModKeys.clear();
                buildModActions.clear();
                buildModAttributes.clear();
                buildMod[2] = buildMod[1] = buildMod[0] = false;
                if (here == bottomOfNextStep)
                    return change;
                break;

            default:
                break;
            }
        } else if (line.toLatin1()[0] == '1') {
            QStringList token;
            split(line,token);
            if (token.size() == 15) {
                QString modelName = token[token.size() - 1];
                if (isSubmodel(modelName) && !buildModSubmodels.contains(modelName)) {
                    buildModSubmodels.append(modelName);
                    Where topOfSubmodel(modelName, 0);
                    setBuildModForNextStep(topOfStep, bottomOfNextStep, topOfSubmodel, change, true);
                }
            }
        }
    }

    if (progressMax) {
        emit progressPermSetValueSig(progressMax);
        emit progressPermStatusRemoveSig();
    }

    return change;
}

/*
 * This function applies buffer exchange and LPub's remove
 * meta commands before writing them out for the renderers to use.
 * Fade, Highlight and COLOUR meta commands are preserved.
 * This eliminates the need for ghosting parts removed by buffer
 * exchange
 */

void Gui::writeToTmp(const QString &fileName,
                     const QStringList &contents)
{
  QString fname = QDir::toNativeSeparators(QDir::currentPath()) + QDir::separator() + Paths::tmpDir + QDir::separator() + fileName;
  QFileInfo fileInfo(fname);
  if(!fileInfo.dir().exists()) {
     fileInfo.dir().mkpath(".");
    }
  QFile file(fname);
  if ( ! file.open(QFile::WriteOnly|QFile::Text)) {
      QMessageBox::warning(nullptr,QMessageBox::tr("LPub3D"),
                           QMessageBox::tr("Failed to open %1 for writing: %2")
                           .arg(fname) .arg(file.errorString()));
    } else {

      Where topOfStep(fileName,0);
      skipHeader(topOfStep);

      int  buildModLevel  = 0;
      int  fileNameIndex  = getSubmodelIndex(fileName);
      bool buildModIgnore = false;
      bool buildModItems  = false;
      bool buildModFile   = false;

      QString buildModKey;
      QMap<int, QString> buildModKeys;
      QMap<int, Rc>      buildModActions;
      QMap<int, QVector<int>> buildModAttributes;

      QVector<int> lineTypeIndexes, buildModLineTypeIndexes;
      QStringList  csiParts, buildModCsiParts;
      QHash<QString, QStringList> bfx;

      PartLineAttributes pla(
         csiParts,
         lineTypeIndexes,
         buildModCsiParts,
         buildModLineTypeIndexes,
         buildModLevel,
         buildModIgnore,
         buildModItems);

      Rc    rc;
      Meta  meta;
      for (int i = 0; i < contents.size(); i++) {
          QString line = contents[i];
          QStringList tokens;

          split(line,tokens);
          if (tokens.size()) {
              if (tokens[0] != "0") {
                  if (! buildModIgnore)
                      CsiItem::partLine(line,pla,i/*relativeTypeIndx*/,OkRc);
              } else {

                  Where here(fileName,i);
                  rc =  meta.parse(line,here,false);

                  switch (rc) {
                  case FadeRc:
                  case SilhouetteRc:
                  case ColourRc:
                      CsiItem::partLine(line,pla,i/*relativeTypeIndx*/,rc);
                      break;

                      /* Buffer exchange */
                  case BufferStoreRc:
                      bfx[meta.bfx.value()] = csiParts;
                      break;

                  case BufferLoadRc:
                      csiParts = bfx[meta.bfx.value()];
                      break;

                  // Get BuildMod attributes and set buildModIgnore based on 'next' step buildModAction
                  case BuildModBeginRc:
                      if (!Preferences::buildModEnabled) {
                          parseError("Build mod meta command encountered but this functionality is currently disabled.<br>"
                                     "Enable at Build Instructions Setup -> Project Setup or remove build mod file content. ",
                                     here,Preferences::BuildModErrors);
                          break;
                      }
                      if ((buildModFile = i < getBuildModStepLineNumber(getBuildModNextStepIndex()))) {
                          buildModKey   = meta.LPub.buildMod.key();
                          buildModLevel = getLevel(buildModKey, BM_BEGIN);
                          buildModKeys.insert(buildModLevel, buildModKey);
                          if (! buildModContains(buildModKey))
                              buildModActions.insert(buildModLevel, BuildModApplyRc);
                          else
                              buildModActions.insert(buildModLevel, Rc(getBuildModAction(buildModKey, getBuildModNextStepIndex())));
                          if (buildModActions.value(buildModLevel) == BuildModApplyRc)
                              buildModIgnore = false;
                          else if (buildModActions.value(buildModLevel) == BuildModRemoveRc)
                              buildModIgnore = true;
                      }
                      break;

                  // Set modActionLineNum and buildModIgnore based on 'next' step buildModAction
                  case BuildModEndModRc:
                      if (buildModFile) {
                          if (buildModLevel > 1 && meta.LPub.buildMod.key().isEmpty())
                                  parseError("Key required for nested build mod meta command",
                                             here,Preferences::BuildModErrors);
                          if (buildModActions.value(buildModLevel) == BuildModApplyRc)
                              buildModIgnore = true;
                          else if (buildModActions.value(buildModLevel) == BuildModRemoveRc)
                              buildModIgnore = false;
                      }
                    break;

                  // Insert buildModAttributes and reset buildModLevel and buildModIgnore to default
                  case BuildModEndRc:
                      buildModLevel  = getLevel(QString(), BM_END);
                      buildModIgnore = false;
                      buildModFile   = false;
                    break;

                  case PartNameRc:
                  case PartTypeRc:
                  case MLCadGroupRc:
                  case LDCadGroupRc:
                  case LeoCadModelRc:
                  case LeoCadPieceRc:
                  case LeoCadCameraRc:
                  case LeoCadLightRc:
                  case LeoCadLightWidthRc:
                  case LeoCadLightTypeRc:
                  case LeoCadSynthRc:
                  case LeoCadGroupBeginRc:
                  case LeoCadGroupEndRc:
                      CsiItem::partLine(line,pla,i/*relativeTypeIndx*/,rc);
                      break;

                      /* remove a group or all instances of a part type */
                  case RemoveGroupRc:
                  case RemovePartTypeRc:
                  case RemovePartNameRc:
                      if (! buildModIgnore) {
                          QStringList newCSIParts;
                          QVector<int> newLineTypeIndexes;
                          if (rc == RemoveGroupRc) {
                              remove_group(csiParts,lineTypeIndexes,meta.LPub.remove.group.value(),newCSIParts,newLineTypeIndexes,meta);
                          } else if (rc == RemovePartTypeRc) {
                              remove_parttype(csiParts,lineTypeIndexes,meta.LPub.remove.parttype.value(),newCSIParts,newLineTypeIndexes);
                          } else {
                              remove_partname(csiParts,lineTypeIndexes,meta.LPub.remove.partname.value(),newCSIParts,newLineTypeIndexes);
                          }
                          csiParts = newCSIParts;
                          lineTypeIndexes = newLineTypeIndexes;
                      }
                      break;

                  default:
                      break;
                  }
              }
          }
      }

      ldrawFile.setLineTypeRelativeIndexes(fileNameIndex,lineTypeIndexes);

      QTextStream out(&file);
      for (int i = 0; i < csiParts.size(); i++) {
          out << csiParts[i] << endl;
        }
      file.close();
    }
}

void Gui::writeToTmp()
{
  writingToTmp = true;
  if (Preferences::modeGUI && ! exporting()) {
      emit progressBarPermInitSig();
      emit progressPermRangeSig(1, ldrawFile._subFileOrder.size());
      emit progressPermMessageSig("Writing submodels...");
    }
  emit messageSig(LOG_INFO_STATUS, "Writing submodels to temp directory...");

  QApplication::processEvents();

  bool upToDate = true;
  bool doFadeStep  = page.meta.LPub.fadeStep.fadeStep.value();
  bool doHighlightStep = page.meta.LPub.highlightStep.highlightStep.value() && !suppressColourMeta();

  QString fadeColor = LDrawColor::ldColorCode(page.meta.LPub.fadeStep.fadeColor.value());

  QStringList content, configuredContent;

  LDrawFile::_currentLevels.clear();

  for (int i = 0; i < ldrawFile._subFileOrder.size(); i++) {

      Where start(ldrawFile._subFileOrder[i],0);

      skipHeader(start);

      QString fileName = ldrawFile._subFileOrder[i].toLower();

      content = ldrawFile.contents(fileName);

      if (ldrawFile.changedSinceLastWrite(fileName)) {
          // write normal submodels...
          upToDate = false;
          emit messageSig(LOG_INFO_STATUS, QString("Writing submodel to temp directory: [%1]...").arg(fileName));

          writeToTmp(fileName,content);

          // capture file name extensions
          QString fileNameStr;
          QString extension = QFileInfo(fileName).suffix().toLower();

          // write configured (Fade) submodels
          if (doFadeStep) {
             fileNameStr = fileName;
             if (extension.isEmpty()) {
               fileNameStr = fileNameStr.append(QString("%1.ldr").arg(FADE_SFX));
             } else {
               fileNameStr = fileNameStr.replace("."+extension, QString("%1.%2").arg(FADE_SFX).arg(extension));
             }

            /* Faded version of submodels */
            emit messageSig(LOG_INFO, "Writing fade submodels to temp directory: " + fileNameStr);
            configuredContent = configureModelSubFile(content, fadeColor, FADE_PART);
            insertConfiguredSubFile(fileNameStr,configuredContent);
            writeToTmp(fileNameStr,configuredContent);
          }
          // write configured (Highlight) submodels
          if (doHighlightStep) {
            fileNameStr = fileName;
            if (extension.isEmpty()) {
              fileNameStr = fileNameStr.append(QString("%1.ldr").arg(HIGHLIGHT_SFX));
            } else {
              fileNameStr = fileNameStr.replace("."+extension, QString("%1.%2").arg(HIGHLIGHT_SFX).arg(extension));
            }
            /* Highlighted version of submodels */
            emit messageSig(LOG_INFO, "Writing highlight submodel to temp directory: " + fileNameStr);
            configuredContent = configureModelSubFile(content, fadeColor, HIGHLIGHT_PART);
            insertConfiguredSubFile(fileNameStr,configuredContent);
            writeToTmp(fileNameStr,configuredContent);
          }
      }

      if (Preferences::modeGUI && ! exporting())
        emit progressPermSetValueSig(i);
  }

  LDrawFile::_currentLevels.clear();

  bool generateSubModelImages = Preferences::modeGUI &&
                                GetViewPieceIcons() &&
                                ! submodelIconsLoaded;
  if (generateSubModelImages) {
      if (Preferences::modeGUI && ! exporting())
          emit progressPermSetValueSig(ldrawFile._subFileOrder.size());

      // generate submodel icons...
      emit messageSig(LOG_INFO_STATUS, "Creating submodel icons...");
      Pli pli;
      int rc = pli.createSubModelIcons();
      if (rc == 0)
          SetSubmodelIconsLoaded(submodelIconsLoaded = true);
      else
          emit messageSig(LOG_ERROR, "Could not create submodel icons...");
      if (Preferences::modeGUI && ! exporting())
          emit progressPermStatusRemoveSig();
  } else
  if (Preferences::modeGUI && ! exporting()) {
      emit progressPermSetValueSig(ldrawFile._subFileOrder.size());
      emit progressPermStatusRemoveSig();
  }
  emit messageSig(LOG_STATUS, upToDate ? "No submodels written; temp directory up to date." : "Submodels written to temp directory.");
  writingToTmp = false;
}

/*
 * Configure writeToTmp content - make fade or highlight copies of submodel files.
 */
QStringList Gui::configureModelSubFile(const QStringList &contents, const QString &fadeColour, const PartType partType)
{
  QString nameMod, colourPrefix;
  if (partType == FADE_PART){
    nameMod = FADE_SFX;
    colourPrefix = LPUB3D_COLOUR_FADE_PREFIX;
  } else if (partType == HIGHLIGHT_PART) {
    nameMod = HIGHLIGHT_SFX;
    colourPrefix = LPUB3D_COLOUR_HIGHLIGHT_PREFIX;
  }

  QStringList configuredContents, subfileColourList;
  bool FadeMetaAdded = false;
  bool SilhouetteMetaAdded = false;

  if (contents.size() > 0) {

      QStringList argv;

      for (int index = 0; index < contents.size(); index++) {

          QString contentLine = contents[index];
          split(contentLine, argv);
          if (argv.size() == 15 && argv[0] == "1") {
              // Insert opening fade meta
              if (!FadeMetaAdded && Preferences::enableFadeSteps && partType == FADE_PART){
                 configuredContents.insert(index,QString("0 !FADE %1").arg(Preferences::fadeStepsOpacity));
                 FadeMetaAdded = true;
              }
              // Insert opening silhouette meta
              if (!SilhouetteMetaAdded && Preferences::enableHighlightStep && partType == HIGHLIGHT_PART){
                 configuredContents.insert(index,QString("0 !SILHOUETTE %1 %2")
                                                         .arg(Preferences::highlightStepLineWidth)
                                                         .arg(Preferences::highlightStepColour));
                 SilhouetteMetaAdded = true;
              }
              if (argv[1] != LDRAW_EDGE_MATERIAL_COLOUR &&
                  argv[1] != LDRAW_MAIN_MATERIAL_COLOUR) {
                  QString colourCode;
                  // Insert color code for fade part
                  if (partType == FADE_PART)
                      colourCode = Preferences::fadeStepsUseColour ? fadeColour : argv[1];
                  // Insert color code for silhouette part
                  if (partType == HIGHLIGHT_PART)
                      colourCode = argv[1];
                  // generate fade color entry
                  if (!colourEntryExist(subfileColourList,argv[1], partType))
                      subfileColourList << createColourEntry(colourCode, partType);
                  // set color code - fade, highlight or both
                  argv[1] = QString("%1%2").arg(colourPrefix).arg(colourCode);
              }
              // process file naming
              QString fileNameStr = QString(argv[argv.size()-1]).toLower();
              QString extension = QFileInfo(fileNameStr).suffix().toLower();
              // static color parts
              if (ldrawColourParts.isLDrawColourPart(fileNameStr)){
                  if (extension.isEmpty()) {
                    fileNameStr = fileNameStr.append(QString("%1.ldr").arg(nameMod));
                  } else {
                    fileNameStr = fileNameStr.replace("."+extension, QString("%1.%2").arg(nameMod).arg(extension));
                  }
                }
              // subfiles
              if (ldrawFile.isSubmodel(fileNameStr)) {
                  if (extension.isEmpty()) {
                    fileNameStr = fileNameStr.append(QString("%1.ldr").arg(nameMod));
                  } else {
                    fileNameStr = fileNameStr.replace("."+extension, QString("%1.%2").arg(nameMod).arg(extension));
                  }
                }
              argv[argv.size()-1] = fileNameStr;
            }
          if (isGhost(contentLine))
              argv.prepend(GHOST_META);
          contentLine = argv.join(" ");
          configuredContents  << contentLine;

          // Insert closing fade and silhouette metas
          if (index+1 == contents.size()){
              if (FadeMetaAdded){
                 configuredContents.append(QString("0 !FADE"));
              }
              if (SilhouetteMetaAdded){
                 configuredContents.append(QString("0 !SILHOUETTE"));
              }
          }
      }
  } else {
    return contents;
  }
  // add the color list to the header of the configuredContents
  if (!subfileColourList.isEmpty()){
      subfileColourList.toSet().toList();  // remove dupes
      configuredContents.prepend("0");
      for (int i = 0; i < subfileColourList.size(); ++i)
          configuredContents.prepend(subfileColourList.at(i));
      configuredContents.prepend("0 // LPub3D step custom colours");
      configuredContents.prepend("0");
  }
  return configuredContents;
}

/*
 * Process csiParts list - fade previous step-parts and or highlight current step-parts.
 * To get the previous content position, take the previous cisFile file size.
 * The csiFile entries are only parts with not formatting or meta commands so it is
 * well suited to provide the delta between steps.
 */
QStringList Gui::configureModelStep(const QStringList &csiParts, const int &stepNum,  Where &current) {

  QStringList configuredCsiParts, stepColourList;
  bool doFadeStep  = page.meta.LPub.fadeStep.fadeStep.value();
  bool doHighlightStep = page.meta.LPub.highlightStep.highlightStep.value() && !suppressColourMeta();
  bool doHighlightFirstStep = Preferences::highlightFirstStep;
  bool FadeMetaAdded = false;
  bool SilhouetteMetaAdded = false;

  if (csiParts.size() > 0 && (doHighlightFirstStep ? true : stepNum > 1)) {

      QString fadeColour  = LDrawColor::ldColorCode(page.meta.LPub.fadeStep.fadeColor.value());

      // retrieve the previous step position
      int prevStepPosition = ldrawFile.getPrevStepPosition(current.modelName);
      if (prevStepPosition == 0 && savePrevStepPosition > 0)
          prevStepPosition = savePrevStepPosition;

      // save the current step position
      ldrawFile.setPrevStepPosition(current.modelName,csiParts.size());

      //qDebug() << "Model:" << current.modelName << ", Step:"  << stepNum << ", PrevStep Get Previous Step Position:" << prevStepPosition
      //         << ", CSI Size:" << csiParts.size() << ", Model Size:"  << ldrawFile.size(current.modelName);
      QStringList argv;

      for (int index = 0; index < csiParts.size(); index++) {

          bool type_1_line = false;
          bool type_1_5_line = false;
          bool is_colour_part = false;
          bool is_submodel_file = false;

          int updatePosition = index+1;
          QString csiLine = csiParts[index];
          split(csiLine, argv);

          // determine line type
          if (argv.size() && argv[0].size() == 1 &&
              argv[0] >= "1" && argv[0] <= "5") {
              type_1_5_line = true;
              if (argv.size() == 15 && argv[0] == "1")
                  type_1_line = true;
          }

          // process parts naming
          QString fileNameStr, extension;
          if (type_1_line){
              fileNameStr = argv[argv.size()-1].toLower();
              extension = QFileInfo(fileNameStr).suffix().toLower();

              // check if is color part
              is_colour_part = ldrawColourParts.isLDrawColourPart(fileNameStr);

              // check if is submodel
              is_submodel_file = ldrawFile.isSubmodel(fileNameStr);

              //if (is_colour_part)
              //    emit messageSig(LOG_NOTICE, "Static color part - " + fileNameStr);
          }

          // write fade step entries
          if ((doHighlightFirstStep ? stepNum > 1 : true) && doFadeStep && (updatePosition <= prevStepPosition)) {
              if (type_1_5_line) {
                  // Insert opening fade meta
                  if (!FadeMetaAdded && Preferences::enableFadeSteps){
                     configuredCsiParts.insert(index,QString("0 !FADE %1").arg(Preferences::fadeStepsOpacity));
                     FadeMetaAdded = true;
                  }
                  if (argv[1] != LDRAW_EDGE_MATERIAL_COLOUR &&
                      argv[1] != LDRAW_MAIN_MATERIAL_COLOUR) {
                      // generate fade color entry
                      QString colourCode = Preferences::fadeStepsUseColour ? fadeColour : argv[1];
                      if (!colourEntryExist(stepColourList,argv[1], FADE_PART))
                        stepColourList << createColourEntry(colourCode, FADE_PART);
                      // set fade color code
                      argv[1] = QString("%1%2").arg(LPUB3D_COLOUR_FADE_PREFIX).arg(colourCode);
                  }
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
                      if (is_submodel_file) {
                          if (extension.isEmpty()) {
                            fileNameStr = fileNameStr.append(QString("%1.ldr").arg(FADE_SFX));
                          } else {
                            fileNameStr = fileNameStr.replace("."+extension, QString("%1.%2").arg(FADE_SFX).arg(extension));
                          }
                      }
                      // assign fade part name
                      argv[argv.size()-1] = fileNameStr;
                  }
              }
          }
          // write highlight entries
          if (doHighlightStep && (updatePosition > prevStepPosition)) {
              if (type_1_5_line) {
                  // Insert opening silhouette meta
                  if (!SilhouetteMetaAdded && Preferences::enableHighlightStep){
                     configuredCsiParts.append(QString("0 !SILHOUETTE %1 %2")
                                                       .arg(Preferences::highlightStepLineWidth)
                                                       .arg(Preferences::highlightStepColour));
                     SilhouetteMetaAdded = true;
                  }
                  if (argv[1] != LDRAW_EDGE_MATERIAL_COLOUR &&
                      argv[1] != LDRAW_MAIN_MATERIAL_COLOUR) {
                      // generate fade color entry
                      QString colourCode = argv[1];
                      if (!colourEntryExist(stepColourList,argv[1], HIGHLIGHT_PART))
                        stepColourList << createColourEntry(colourCode, HIGHLIGHT_PART);
                      // set fade color code
                      argv[1] = QString("%1%2").arg(LPUB3D_COLOUR_HIGHLIGHT_PREFIX).arg(colourCode);
                  }
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
                      if (is_submodel_file) {
                          if (extension.isEmpty()) {
                            fileNameStr = fileNameStr.append(QString("%1.ldr").arg(HIGHLIGHT_SFX));
                          } else {
                            fileNameStr = fileNameStr.replace("."+extension, QString("%1.%2").arg(HIGHLIGHT_SFX).arg(extension));
                          }
                      }
                      // assign fade part name
                      argv[argv.size()-1] = fileNameStr;
                  }
              }
          }

          if (isGhost(csiLine))
              argv.prepend(GHOST_META);

          // previous step parts
          csiLine = argv.join(" ");

          // current step parts
          configuredCsiParts  << csiLine;

          // Insert closing fade meta
          if (updatePosition == prevStepPosition) {
              if (FadeMetaAdded){
                 configuredCsiParts.append(QString("0 !FADE"));
              }
          }

          // Insert closing silhouette meta
          if (index+1 == csiParts.size()){
              if (SilhouetteMetaAdded){
                 configuredCsiParts.append(QString("0 !SILHOUETTE"));
              }
          }
        }
    } else {
      // save the current step position
      ldrawFile.setPrevStepPosition(current.modelName,csiParts.size());
      return csiParts;
    }

  // add the fade color list to the header of the CsiParts list
  if (!stepColourList.isEmpty()){
      stepColourList.toSet().toList();  // remove dupes
      configuredCsiParts.prepend("0");
      for (int i = 0; i < stepColourList.size(); ++i)
        configuredCsiParts.prepend(stepColourList.at(i));
      configuredCsiParts.prepend("0 // LPub3D step custom colours");
      configuredCsiParts.prepend("0");
  }

  return configuredCsiParts;
}

bool Gui::colourEntryExist(const QStringList &colourEntries, const QString &code, const PartType partType)
{
    bool fadePartType = partType == FADE_PART;

    if (Preferences::fadeStepsUseColour && fadePartType && colourEntries.size() > 0)
        return true;

    QStringList colourComponents;
    QString colourPrefix = fadePartType ? LPUB3D_COLOUR_FADE_PREFIX : LPUB3D_COLOUR_HIGHLIGHT_PREFIX;
    QString colourCode   = QString("%1%2").arg(colourPrefix).arg(code);
    for (int i = 0; i < colourEntries.size(); ++i){
        QString colourLine = colourEntries[i];
        split(colourLine,colourComponents);
        if (colourComponents.size() == 11 && colourComponents[4] == colourCode) {
            return true;
        }
    }
    return false;
}

QString Gui::createColourEntry(const QString &colourCode, const PartType partType)
{
  // Fade Step Alpha Percent (default = 100%) -  e.g. 50% of Alpha 255 rounded up we get ((255 * 50) + (100 - 1)) / 100

  bool fadePartType          = partType == FADE_PART;

  QString _colourPrefix      = fadePartType ? LPUB3D_COLOUR_FADE_PREFIX : LPUB3D_COLOUR_HIGHLIGHT_PREFIX;  // fade prefix 100, highlight prefix 110
  QString _fadeColour        = LDrawColor::ldColorCode(page.meta.LPub.fadeStep.fadeColor.value());
  QString _colourCode        = _colourPrefix + (fadePartType ? Preferences::fadeStepsUseColour ? _fadeColour : colourCode : colourCode);
  QString _mainColourValue   = "#" + ldrawColors.value(colourCode);
  QString _edgeColourValue   = fadePartType ? "#" + ldrawColors.edge(colourCode) : Preferences::highlightStepColour;
  QString _colourDescription = LPUB3D_COLOUR_TITLE_PREFIX + ldrawColors.name(colourCode);
  int _fadeAlphaValue        = ((ldrawColors.alpha(colourCode) * (100 - Preferences::fadeStepsOpacity)) + (100 - 1)) / 100;
  int _alphaValue            = fadePartType ? _fadeAlphaValue : ldrawColors.alpha(colourCode);             // use 100% opacity with highlight color

  return QString("0 !COLOUR %1 CODE %2 VALUE %3 EDGE %4 ALPHA %5")
                 .arg(_colourDescription)   // description
                 .arg(_colourCode)          // original color code
                 .arg(_mainColourValue)     // main color value
                 .arg(_edgeColourValue)     // edge color value
                 .arg(_alphaValue);         // color alpha value
}
