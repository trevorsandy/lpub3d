
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
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
#include <QtConcurrent>
#endif

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
#include "waitingspinnerwidget.h"
#include "lc_application.h"

#include "QsLog.h"

// Set to enable write parts output file for debugging // TODO: delete from header
#ifndef WRITE_PARTS_DEBUG
//#define WRITE_PARTS_DEBUG
#endif

#define FIRST_STEP 1
#define FIRST_PAGE 1

QString Gui::AttributeNames[] =
{
    "Line",
    "Border"
};

QString Gui::PositionNames[] =
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
void Gui::remove_group(
    QStringList  in,     // csiParts
    QVector<int> tin,    // typeIndexes
    QString      group,  // steps->meta.LPub.remove.group.value()
    QStringList  &out,   // newCSIParts
    QVector<int> &tiout, // newTypeIndexes
    Meta         *meta)
{
  Q_UNUSED(meta)

  bool    grpMatch = false;
  bool    exclude  = false;
  bool    tinNull  = false;
  int     grpLevel = 0;
  Rc      grpType  = OkRc;
  QString line, grpData;
  QStringList lids;

  if ((tinNull = in.size() != tin.size())) {
    QString message(tr("CSI part list size [%1] does not match line index size [%2].")
                       .arg(in.size()).arg(tin.size()));
    emit gui->messageSig(LOG_NOTICE, message);
  }

  auto parseGroupMeta = [&line](Rc &grpType)
  {
    QHash<Rc, QRegExp>::const_iterator i = groupRegExMap.constBegin();
    while (i != groupRegExMap.constEnd()) {
      QRegExp rx(i.value());
      if (line.contains(rx)) {
        grpType = i.key();
        return rx.cap(rx.captureCount());
      }
      ++i;
    }
    return QString();
  };

  for (int i = 0; i < in.size(); i++) {

    line = in.at(i);

    grpData = parseGroupMeta(grpType);

    switch (grpType)
    {
      case MLCadGroupRc:
        if ((exclude = grpData == group))
          i++;
        break;
      case LDCadGroupRc:
        lids = grpData.split(" ");
        if ((exclude = lids.size() && lpub->ldrawFile.ldcadGroupMatch(group, lids)))
          i++;
        break;
      case LeoCadGroupBeginRc:
        if ((exclude = grpData == group)) {
          grpMatch = true;
          i++;
        }
        else if ((exclude = grpMatch)) {
          grpLevel++;
          i++;
        }
        break;
      case LeoCadGroupEndRc:
        if ((exclude = grpMatch)) {
          if (grpLevel == 0) {
            grpMatch = false;
          } else {
            grpLevel--;
          }
        }
        break;
      default:
        break;
    }

    if (grpMatch)
      i++;
    else
    if (!exclude) {
      out << line;
      if (!tinNull)
        tiout << tin.at(i);
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

void Gui::remove_parttype(
    QStringList   in,    // csiParts
    QVector<int>  tin,   // typeIndexes
    QString       model, // part type
    QStringList  &out,   // newCSIParts
    QVector<int> &tiout) // newTypeIndexes
{
  bool exclude  = false;
  bool tinNull  = false;

  if ((tinNull = in.size() != tin.size())) {
    QString message(tr("CSI part list size [%1] does not match line index size [%2].")
                            .arg(in.size()).arg(tin.size()));
    emit gui->messageSig(LOG_NOTICE, message);
  }

  for (int i = 0; i < in.size(); i++) {
    QString line = in.at(i);
    QStringList tokens;
    split(line,tokens);

    if (tokens.size() == 15 && tokens[0] == "1") {
      exclude = tokens[14].toLower() == model.toLower();
    }

    if (!exclude) {
      out << line;
      if (!tinNull)
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

void Gui::remove_partname(
    QStringList   in,    // csiParts
    QVector<int>  tin,   // typeIndexes
    QString       name,  // partName
    QStringList  &out,   // newCSIParts
    QVector<int> &tiout) // newCSIParts
{
  bool tinNull  = false;
  name = name.toLower();

  if ((tinNull = in.size() != tin.size())) {
    QString message(QString("CSI part list size [%1] does not match line index size [%2].")
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
      if (tokens[3].toLower() == name.toLower()) {
        for ( ; i < in.size(); i++) {
          line = in.at(i);
          split(line,tokens);
          if (tokens.size() == 15 && tokens[0] == "1") {
            break;
          } else {
            out << line;
            if (!tinNull)
              tiout << tin.at(i);
          }
        }
      } else {
        out << line;
        if (!tinNull)
          tiout << tin.at(i);
      }
    } else {
      out << line;
      if (!tinNull)
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

void Gui::set_divider_pointers(
        Meta &curMeta,
        Where &current,
        Range *range,
        LGraphicsView *view,
        DividerType dividerType,
        int stepNum,
        Rc rct) {

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
                pam.setOtherDataInches(p->getPointerAttribInches());
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

Range *Gui::newRange(
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
  pageProcessRunning = PROC_DRAW_PAGE;
  QElapsedTimer pageRenderTimer;
  pageRenderTimer.start();

  QRegExp partTypeLineRx("^\\s*1|\\bBEGIN SUB\\b");
  QStringList configuredCsiParts; // fade and highlight configuration
  QString  line;
  Callout *callout         = nullptr;
  Range   *range           = nullptr;
  Step    *step            = nullptr;

  int      instances       = 1;
  bool     global          = true;
  bool     pliIgnore       = false;
  bool     partIgnore      = false;
  bool     excludedPart    = false;
  bool     synthBegin      = false;
  bool     multiStep       = false;
  bool     multiStepPage   = false;
  bool     partsAdded      = false;
  bool     coverPage       = false;
  bool     bfxStore1       = false;
  bool     bfxLoad         = false;
  bool     firstGroupStep  = true;
  bool     noStep          = false;
  bool     rotateIcon      = false;
  bool     assemAnnotation = false;
  bool     displayInstanceCount = false;
  bool     previewNotPerStep = false;
  int      countInstances  = steps->meta.LPub.countInstance.value();

  // Build mod update flags
  int      buildModStepIndex  = -1;
  bool     buildModItems      = false;
  bool     buildModInsert     = false;
  bool     buildModExists     = false;
  bool     buildModChange     = false;
  bool     buildModPliIgnore  = false;
  bool     buildModActionStep = false;
  bool     buildModTypeIgnore = false;

  QVector<int>  buildModLineTypeIndexes;
  QStringList   buildModCsiParts;
  BuildModFlags buildMod;
  BuildModMeta  buildModMeta;
  BuildModMeta  buildModActionMeta;

  QMap<int, QString> buildModKeys;
  QMap<int, QVector<int>> buildModAttributes;
  QMap<int,int> buildModActions;

  DividerType dividerType  = NoDivider;

  PagePointer *pagePointer = nullptr;
  QMap<Positions, PagePointer *> pagePointers;
  QMap<Where, SceneObjectData> selectedSceneItems;

  QList<InsertMeta> inserts;
  QMap<QString, LightData> lightList;
  QStringList calloutParts;

  Where topOfStep = opts.current;

  enum draw_page_stat { begin, end };

  steps->setTopOfSteps(topOfStep/*opts.current*/);
  steps->isMirrored = opts.isMirrored;
  lpub->page.coverPage = false;

  Rc gprc    = OkRc;
  Rc rc      = OkRc;

  TraverseRc returnValue = HitNothing;

  // include file vars
  Where includeHere;
  Rc    includeFileRc   = EndOfIncludeFileRc;
  bool includeFileFound = false;
  bool resetIncludeRc   = false;

  // set page header/footer width
  float pW;
  int which;
  if (callout) {
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

  auto drawPageStatus = [this, &opts, &multiStep, &coverPage, &topOfStep] (draw_page_stat status) {
      int charWidth = QFontMetrics(font()).averageCharWidth();
      QFontMetrics currentMetrics(font());
      QString elidedModelName = currentMetrics.elidedText(opts.current.modelName,
                                                          Qt::ElideRight, charWidth * 30/*characters*/);
      Where where = topOfStep;
      bool fin = status == end;
      QRegExp multiStepRx(" MULTI_STEP BEGIN$");
      bool stepGroup = fin ? multiStep : stepContains(where, multiStepRx);
      QString message = QString("%1 %2 draw-page for page %3, step %4, model '%5'%6")
                                .arg(fin ? "Processed" : "Processing").arg(stepGroup ? "multi-step" : opts.calledOut ? "called out" : coverPage ? "cover page" : "single-step")
                                .arg(displayPageNum).arg(opts.stepNum).arg(elidedModelName).arg(fin ? "" : "...");
      emit messageSig(LOG_STATUS, message);
      emit messageSig(fin ? LOG_TRACE : LOG_INFO, message);
      //QApplication::processEvents();
  };

  auto drawPageElapsedTime = [this, &partsAdded, &pageRenderTimer, &coverPage](){
    QString pageRenderMessage = QString("%1 ").arg(VER_PRODUCTNAME_STR);
    if (!lpub->page.coverPage && partsAdded) {
      pageRenderMessage += QString("using %1 ").arg(rendererNames[Render::getRenderer()]);
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
    pageRenderMessage += QString("rendered page %1 - %2")
                                 .arg(QString("%1%2").arg(displayPageNum).arg(coverPage ? " (Cover Page)" : ""))
                                 .arg(elapsedTime(pageRenderTimer.elapsed()));
    emit messageSig(LOG_TRACE, pageRenderMessage);
    pageProcessRunning = PROC_COUNT_PAGE;
    //QApplication::processEvents();
  };

  auto insertAttribute =
          [&opts,
           &buildMod,
           &topOfStep] (
          QMap<int, QVector<int>> &buildModAttributes,
          int index, const Where &here)
  {
      int  fileNameIndex = topOfStep.modelIndex;
      QMap<int, QVector<int>>::iterator i = buildModAttributes.find(buildMod.level);
      if (i == buildModAttributes.end()) {
          QVector<int> modAttributes = { 0, 0, 0, displayPageNum, 0/*step pieces*/, fileNameIndex, topOfStep.lineNumber, opts.stepNum };
          modAttributes[index] = here.lineNumber;
          buildModAttributes.insert(buildMod.level, modAttributes);
      } else {
          i.value()[index] = here.lineNumber;
      }
  };

  auto insertBuildModification =
          [this,
          &step,
          &opts,
          &buildModAttributes,
          &buildModKeys,
          &topOfStep] (int buildModLevel)
  {
      int buildModStepIndex = getBuildModStepIndex(topOfStep);
      QString buildModKey   = buildModKeys.value(buildModLevel);
      QVector<int> modAttributes = { 0, 0, 0, 0, 0, -1, 0, 0 };

      QMap<int, QVector<int>>::iterator i = buildModAttributes.find(buildModLevel);
      if (i == buildModAttributes.end()) {
          emit gui->messageSig(LOG_ERROR, QString("Invalid BuildMod Entry for key: %1").arg(buildModKey));
          return;
      }
      modAttributes = i.value();

      modAttributes[BM_DISPLAY_PAGE_NUM] = displayPageNum;
      modAttributes[BM_STEP_PIECES]      = opts.csiParts.size();
      modAttributes[BM_MODEL_NAME_INDEX] = topOfStep.modelIndex;
      modAttributes[BM_MODEL_LINE_NUM]   = topOfStep.lineNumber;
      modAttributes[BM_MODEL_STEP_NUM]   = opts.stepNum;

#ifdef QT_DEBUG_MODE
      emit messageSig(LOG_DEBUG, QString(
                      "Insert DrawPage BuildMod StepIndex: %1, "
                      "Action: Apply(64), "
                      "Attributes: %2 %3 %4 %5 %6 %7 %8 %9, "
                      "ModKey: %10, "
                      "Level: %11")
                      .arg(buildModStepIndex)                      // Attribute Default Initial:
                      .arg(modAttributes.at(BM_BEGIN_LINE_NUM))    // 0         0       this
                      .arg(modAttributes.at(BM_ACTION_LINE_NUM))   // 1         0       this
                      .arg(modAttributes.at(BM_END_LINE_NUM))      // 2         0       this
                      .arg(modAttributes.at(BM_DISPLAY_PAGE_NUM))  // 3         0       this
                      .arg(modAttributes.at(BM_STEP_PIECES))       // 4         0       this
                      .arg(modAttributes.at(BM_MODEL_NAME_INDEX))  // 5        -1       this
                      .arg(modAttributes.at(BM_MODEL_LINE_NUM))    // 6         0       this
                      .arg(modAttributes.at(BM_MODEL_STEP_NUM))    // 7         0       this
                      .arg(buildModKey)
                      .arg(buildModLevel));
#endif

      insertBuildMod(buildModKey,
                     modAttributes,
                     buildModStepIndex);
      if (step->submodelStack().size())
          setBuildModSubmodelStack(buildModKey,step->submodelStack());
  };

  auto buildModIgnoreOverride = [&multiStep, &buildMod] (bool &buildModIgnore, bool &buildModTypeIgnore)
  {
      if (buildModIgnore) {
          buildModIgnore = (multiStep && buildMod.state != BM_BEGIN);
          // if ignore override, set flag to ignore CSI parts
          buildModTypeIgnore = ! buildModIgnore;
      }
      return ! buildModIgnore;
  };

#ifdef WRITE_PARTS_DEBUG
  auto writeDrawPartsFile = [this,&topOfStep, &opts](const QString &insert = QString())
  {
      if (opts.csiParts.isEmpty())
          return;
      const QString &nameInsert = insert.isEmpty() ? "c_draw_parts" : insert;
      const QString filePath = QDir::currentPath() + "/" + Paths::tmpDir;
      const QString outfileName = QString("%1/%2_%3.ldr")
                   .arg(filePath)
                   .arg(nameInsert)
                   .arg(QString("page_%1_step_%2_model_%3_line_%4")
                        .arg(displayPageNum)        // Page Number
                        .arg(opts.stepNum)          // Step Number
                        .arg(topOfStep.modelIndex)  // ModelIndex
                        .arg(topOfStep.lineNumber));// LineNumber
      QFile file(outfileName);
      if ( ! file.open(QFile::WriteOnly | QFile::Text))
          messageSig(LOG_ERROR,QString("Cannot open draw_parts file %1 for writing: %2")
                                       .arg(outfileName) .arg(file.errorString()));
      QTextStream out(&file);
      for (int i = 0; i < opts.csiParts.size(); i++)
          out << opts.csiParts[i] << lpub_endl;
      file.close();
  };
#endif

  // Update buildMod intiialized in setBuildModForNextStep
  auto updateBuildModification = [this, &step, &opts, &buildModKeys] (int buildModLevel)
  {
      const QString buildModKey = buildModKeys.value(buildModLevel);
      setBuildModStepKey(buildModKey, step->viewerStepKey);
      setBuildModStepPieces(buildModKey, opts.csiParts.size());
      if (step->submodelStack().size())
          setBuildModSubmodelStack(buildModKey,step->submodelStack());
  };

  PartLineAttributes pla(
     opts.csiParts,
     opts.lineTypeIndexes,
     buildModCsiParts,
     buildModLineTypeIndexes,
     buildMod.level,
     buildMod.ignore,
     buildModItems);

  drawPageStatus(begin);

  /*
   * do until end of page
   */
  int numLines = lpub->ldrawFile.size(opts.current.modelName);

  //* local ldrawFile used for debugging
#ifdef QT_DEBUG_MODE
  LDrawFile *ldrawFile = &lpub->ldrawFile;
  Q_UNUSED(ldrawFile)
#endif
  //*/

  for ( ; opts.current <= numLines && !Gui::abortProcess() ; opts.current++) {

      // if reading include file, return to current line, do not advance

      if (includeFileRc != EndOfIncludeFileRc && includeFileFound) {
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

      //* local optsPageNum used to set breakpoint condition (e.g. optsPageNum > 7)
#ifdef QT_DEBUG_MODE
      int debugDisplayPageNum = displayPageNum;
      Q_UNUSED(debugDisplayPageNum)
#endif
      //*/

      Meta &curMeta = callout ? callout->meta : steps->meta;

      QStringList tokens;

      /* If we hit end of file we've got to note end of step */

      if (opts.current >= numLines) {
          line.clear();
          gprc = EndOfFileRc;
          tokens << "0";
      }

      /* not end of file, so get the next LDraw line */

      else {

          /* read the line from the ldrawFile repository */

          line = lpub->ldrawFile.readLine(opts.current.modelName,opts.current.lineNumber);
          split(line,tokens);
      } // If we hit end of file, note end of step or if not, get the next LDraw line

      // STEP - Process part type

      if (tokens.size() == 15 && tokens[0] == "1") {
          // STEP - Create partType

          QString color = tokens[1];
          QString type  = tokens[tokens.size()-1];

          if (color == LDRAW_MAIN_MATERIAL_COLOUR) {
              QStringList addTokens;
              split(addLine,addTokens);
              if (addTokens.size() == 15) {
                  tokens[1] = addTokens[1];
              }
              line = tokens.join(" ");
              color = tokens[1];
          }

          // STEP - Allocate step for type

          if (! buildMod.ignore) {

              /* for multistep build mod case where we are processing a callout
                 and we are overriding the last action, buildModTypeIgnore is set
                 so we do not render overridden types */

              if (! buildModTypeIgnore) {
                  CsiItem::partLine(line,pla,opts.current.lineNumber,OkRc);
                  partsAdded = true;
              }

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
          } // Allocate step for type

          /* if part is on excludedPart.lst, set excludedPart */

          excludedPart = ExcludedParts::isExcludedPart(type);

          /* addition of ldraw parts */

          // STEP - Allocate PLI part

          if (curMeta.LPub.pli.show.value()
              && ! excludedPart
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

                  /* remove part from buffer parts if in step group and buffer load */

                  if (opts.bfxStore2 && bfxLoad) {
                      // bool removed = false;
                      for (int i = 0; i < opts.bfxParts.size(); i++) {
                          if (opts.bfxParts[i] == colorType) {
                              opts.bfxParts.removeAt(i);
                              // removed = true;
                              break;
                            }
                        }

                      // Danny: the following condition should help LPUB to remove automatically from PLI the parts in the buffer,
                      // but does not work if two buffers are used one after another in a multi step page.
                      // Better to make the user use the !LPUB PLI BEGIN IGN / END

                     // if ( ! removed )  {
                        if (! noStep) {
                            opts.pliParts << Pli::partLine(line,opts.current,steps->meta);
                        }
                     // }
                    } else {

                      opts.pliParts << Pli::partLine(line,opts.current,steps->meta);
                    }
                }

              /* bfxStore1 goes true when we've seen BFX STORE the first time
                 in a sequence of steps.  This used to be commented out which
                 means it didn't work in some cases, but we need it in step
                 group cases, so.... bfxStore1 && multiStep (was just bfxStore1) */

              if (bfxStore1 && (multiStep || opts.calledOut)) {
                  opts.bfxParts << colorType;
              }
          } // Allocate PLI part

          // STEP - Process callout

          /* if it is a called out sub-model, then process it.
           * Non called out sub-model are processed in findPage() */

          if (lpub->ldrawFile.isSubmodel(type) && callout && ! noStep && ! buildMod.ignore) {

              CalloutBeginMeta::CalloutMode calloutMode = callout->meta.LPub.callout.begin.value();

//              logDebug() << "CALLOUT MODE: " << (calloutMode == CalloutBeginMeta::Unassembled ? "Unassembled" :
//                                                 calloutMode == CalloutBeginMeta::Rotated ? "Rotated" : "Assembled");

              /* we are a callout, so gather all the steps within the callout
                 start with new meta, but no rotation step */

              QString thisType = type;

              // STEP - Process rotated or assembled callout

              if ((opts.assembledCallout = calloutMode != CalloutBeginMeta::Unassembled)) {

                  /* So, we process these callouts in-line, not when we finally hit the STEP or
                     ROTSTEP that ends this processing, but for ASSEMBLED or ROTATED
                     callouts, the ROTSTEP state affects the results, so we have to search
                     forward until we hit STEP or ROTSTEP to know how the submodel might
                     want to be rotated.  Also, for submodel's who's scale is different
                     than their parent's scale, we want to scan ahead and find out the
                     parent's scale and "render" the submodels at the parent's scale */

                  Rc rrc = OkRc;
                  Meta tmpMeta = curMeta;
                  Where walk = opts.current;
                  for (++walk; walk < numLines; ++walk) {
                      QStringList tokens;
                      QString scanLine = lpub->ldrawFile.readLine(walk.modelName,walk.lineNumber);
                      split(scanLine,tokens);
                      if (tokens.size() > 0 && tokens[0] == "0") {
                          rrc = tmpMeta.parse(scanLine,walk,false);
                          if (rrc == StepRc || rrc == RotStepRc) {
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

              } // Process rotated or assembled callout

              // STEP - Set callout in parent step

              if (callout->bottom.modelName != thisType) {

                  Where current2(thisType,getSubmodelIndex(thisType),0);
                  skipHeader(current2);
                  if (calloutMode == CalloutBeginMeta::Assembled) {
                      // In this case, no additional rotation should be applied to the submodel
                      callout->meta.rotStep.clear();
                  }
                  SubmodelStack tos(opts.current.modelName,opts.current.lineNumber,opts.stepNum);
                  const QString levelKey = QString("DrawPage BuildMod Key: %1, ParentModel: %2, LineNumber: %3")
                                                   .arg(buildMod.key)
                                                   .arg(opts.current.modelName)
                                                   .arg(opts.current.lineNumber);
                  int buildModLevel = buildMod.state == BM_BEGIN ? getLevel(levelKey, BM_CURRENT) : opts.buildModLevel;
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
                              buildModLevel,
                              opts.updateViewer,
                              lpub->ldrawFile.mirrored(tokens),
                              opts.printing,
                              opts.bfxStore2,
                              opts.assembledCallout,
                              true               /*calledOut*/
                              );

                  returnValue = static_cast<TraverseRc>(drawPage(view, scene, callout, line, calloutOpts));

                  callout->meta = saveMeta;

                  bool calloutPliPerStep = callout->meta.LPub.pli.show.value() &&
                                           callout->meta.LPub.callout.pli.perStep.value();

                  buildModPliIgnore = calloutPliPerStep || pliIgnore || excludedPart;

                  if (! calloutPliPerStep && ! excludedPart && ! pliIgnore && ! buildModPliIgnore && ! partIgnore && ! synthBegin && calloutMode == CalloutBeginMeta::Unassembled) {
                      opts.pliParts += calloutParts;
                  }

                  if (returnValue != HitNothing) {
                      steps->placement = steps->meta.LPub.assem.placement;
                      if (returnValue == HitBuildModAction || returnValue == HitCsiAnnotation)
                          // return to init drawPage to rerun findPage to regenerate content
                          pageProcessRunning = PROC_DISPLAY_PAGE;
                      else
                          // return to init findPage
                          pageProcessRunning = PROC_FIND_PAGE;
                      return static_cast<int>(returnValue);
                  }
              } else {
                callout->instances++;
                if (calloutMode == CalloutBeginMeta::Unassembled) {
                    opts.pliParts += calloutParts;
                  }
              } // Set callout in parent step

              /* remind user what file we're working on */

              emit messageSig(LOG_STATUS, "Processing " + opts.current.modelName + "...");

          } // Process called out submodel

          // STEP - Set display submodel at first submodel step

          if (step && steps->meta.LPub.subModel.show.value()) {
              bool topModel     = (lpub->ldrawFile.topLevelFile() == topOfStep.modelName);
              bool showTopModel = (steps->meta.LPub.subModel.showTopModel.value());
              bool calloutOk    = (opts.calledOut ? opts.assembledCallout : true ) &&
                                  (opts.calledOut ? steps->meta.LPub.subModel.showSubmodelInCallout.value() : true);
              bool showStepOk   = (steps->meta.LPub.subModel.showStepNum.value() == opts.stepNum || opts.stepNum == 1);
              if (showStepOk && calloutOk && (!topModel || showTopModel)){
                  if (multiStep && steps->meta.LPub.multiStep.pli.perStep.value() == false) {
                      previewNotPerStep = !previewNotPerStep ? true : previewNotPerStep;
                  } else {
                      step->placeSubModel = true;
                  }
              }
          } // Set display submodel at first submodel step
      } // Process part type

      // STEP - Process line, triangle, or polygon type

      else if ((tokens.size() == 8  &&  tokens[0] == "2") ||
               (tokens.size() == 11 &&  tokens[0] == "3") ||
               (tokens.size() == 14 && (tokens[0] == "4"  || tokens[0] == "5"))) {

          // STEP - Allocate step for type

          /* we've got a line, triangle or polygon, so add it to the list
             and make sure we know we have a step */

          if (! buildMod.ignore) {

            /* for multistep build mod case where we are processing a callout
               and we are overriding the last action, buildModTypeIgnore is set
               so we do not render overridden types */

            if (! buildModTypeIgnore) {
              CsiItem::partLine(line,pla,opts.current.lineNumber,OkRc);
              partsAdded = true;
            }

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
        } // Allocate step for type
      } // Process line, triangle, or polygon type

      // STEP - Process meta-command

      else if ((tokens.size() && tokens[0] == "0") || gprc == EndOfFileRc) {

          /* must be a meta-command (or comment) */

          if (global && tokens.contains("!LPUB") && tokens.contains("GLOBAL")) {
              topOfStep = opts.current;
            } else {
              global = false;
            }

          if (gprc == EndOfFileRc) {
              rc = gprc;
          } else {

              // intercept include file flag

              if (includeFileRc != EndOfIncludeFileRc) {
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

          /* handle specific meta-command */

          switch (rc) {
            /* toss it all out the window, per James' original plan */
            case ClearRc:
              opts.pliParts.clear();
              opts.csiParts.clear();
              opts.lineTypeIndexes.clear();
              //steps->freeSteps();  // had to remove this because it blows steps following clear
              // in step group.
              break;

            case PreferredRendererRc:
              curMeta.LPub.preferredRenderer.setPreferences();
              curMeta.LPub.resetCamerasFoV();
              break;

            case PreferredRendererAssemRc:
              curMeta.LPub.assem.preferredRenderer.setPreferences();
              curMeta.LPub.assem.resetCameraFoV();
              if (step)
                  step->csiStepMeta.preferredRenderer = curMeta.LPub.assem.preferredRenderer;
              break;

            case PreferredRendererCalloutAssemRc:
              curMeta.LPub.callout.csi.preferredRenderer.setPreferences();
              curMeta.LPub.callout.csi.resetCameraFoV();
              if (step)
                  step->csiStepMeta.preferredRenderer = curMeta.LPub.callout.csi.preferredRenderer;
              break;

            case PreferredRendererGroupAssemRc:
              curMeta.LPub.multiStep.csi.preferredRenderer.setPreferences();
              curMeta.LPub.multiStep.csi.resetCameraFoV();
              if (step)
                  step->csiStepMeta.preferredRenderer = curMeta.LPub.multiStep.csi.preferredRenderer;
              break;

            case PreferredRendererSubModelRc:
              curMeta.LPub.subModel.preferredRenderer.setPreferences();
              curMeta.LPub.subModel.resetCameraFoV();
              break;

            case PreferredRendererPliRc:
              curMeta.LPub.pli.preferredRenderer.setPreferences();
              curMeta.LPub.pli.resetCameraFoV();
              break;

            case PreferredRendererBomRc:
              curMeta.LPub.bom.preferredRenderer.setPreferences();
              curMeta.LPub.bom.resetCameraFoV();
              break;

            case EnableFadeStepsCalloutAssemRc:
            case EnableFadeStepsGroupAssemRc:
            case EnableFadeStepsAssemRc:
              if ((rc == EnableFadeStepsCalloutAssemRc ? curMeta.LPub.callout.csi.fadeSteps.enable.value() :
                   rc == EnableFadeStepsGroupAssemRc ? curMeta.LPub.multiStep.csi.fadeSteps.enable.value() :
                   rc == EnableFadeStepsAssemRc ? curMeta.LPub.assem.fadeSteps.enable.value() :
                         curMeta.LPub.fadeSteps.enable.value()) && !mSetupFadeSteps) {
                      parseError(tr("Fade previous steps command ignored. %1 must be set to TRUE.")
                                    .arg(curMeta.LPub.fadeSteps.enable.value() ? "FADE_STEPS ENABLED" :
                                                                                "FADE_STEPS SETUP (in the first step of the main model)"),opts.current);
              } else if (rc == EnableFadeStepsCalloutAssemRc) {
                  curMeta.LPub.callout.csi.fadeSteps.setPreferences();
                  if (step)
                      step->csiStepMeta.fadeSteps = curMeta.LPub.callout.csi.fadeSteps;
              } else if (rc == EnableFadeStepsGroupAssemRc) {
                  curMeta.LPub.multiStep.csi.fadeSteps.setPreferences();
                  if (step)
                      step->csiStepMeta.fadeSteps = curMeta.LPub.multiStep.csi.fadeSteps;
              } else if (rc == EnableFadeStepsAssemRc) {
                  curMeta.LPub.assem.fadeSteps.setPreferences();
                  if (step)
                      step->csiStepMeta.fadeSteps = curMeta.LPub.assem.fadeSteps;
              }
              break;
            case EnableFadeStepsRc:
                  curMeta.LPub.fadeSteps.setPreferences();
                  Gui::suspendFileDisplay = true;
                  insertFinalModelStep();
                  Gui::suspendFileDisplay = false;
              break;

            case EnableHighlightStepCalloutAssemRc:
            case EnableHighlightStepGroupAssemRc:
            case EnableHighlightStepAssemRc:
              if ((rc == EnableHighlightStepCalloutAssemRc ? curMeta.LPub.callout.csi.highlightStep.enable.value() :
                   rc == EnableHighlightStepGroupAssemRc ? curMeta.LPub.multiStep.csi.highlightStep.enable.value() :
                   rc == EnableHighlightStepAssemRc ? curMeta.LPub.assem.highlightStep.enable.value() :
                         curMeta.LPub.highlightStep.enable.value()) && !mSetupHighlightStep) {
                  parseError(tr("Highlight current step command ignored. %1 must be set to TRUE.")
                                .arg(curMeta.LPub.highlightStep.enable.value() ? "HIGHLIGHT_STEP ENABLED" :
                                                                                 "HIGHLIGHT_STEP SETUP (in the first STEP of the main model)"),opts.current);
              } else if (rc == EnableHighlightStepCalloutAssemRc) {
                  curMeta.LPub.callout.csi.highlightStep.setPreferences();
                  if (step)
                      step->csiStepMeta.highlightStep = curMeta.LPub.callout.csi.highlightStep;
              } else if (rc == EnableHighlightStepGroupAssemRc) {
                  curMeta.LPub.multiStep.csi.highlightStep.setPreferences();
                  if (step)
                      step->csiStepMeta.highlightStep = curMeta.LPub.multiStep.csi.highlightStep;
              } else if (rc == EnableHighlightStepAssemRc) {
                  curMeta.LPub.assem.highlightStep.setPreferences();
                  if (step)
                      step->csiStepMeta.highlightStep = curMeta.LPub.assem.highlightStep;
              }
              break;
           case EnableHighlightStepRc:
                  curMeta.LPub.highlightStep.setPreferences();
                  Gui::suspendFileDisplay = true;
                  insertFinalModelStep();
                  Gui::suspendFileDisplay = false;
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
                  lpub->ldrawFile.setPrevStepPosition(opts.current.modelName,opts.stepNum,opts.csiParts.size());
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
              includeFileRc = Rc(include(curMeta,includeHere.lineNumber,includeFileFound)); // includeHere and inserted are include(...) vars
              if (includeFileRc == IncludeFileErrorRc) {
                  includeFileRc = EndOfIncludeFileRc;
                  parseError(tr("INCLUDE file was not resolved."),opts.current,Preferences::IncludeFileErrors);  // file parse error
              } else if (includeFileRc != EndOfIncludeFileRc) {                             // still reading so continue
                  resetIncludeRc = false;                                                   // do not reset, allow includeFileRc to execute
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
              if (steps->meta.LPub.pli.show.value()
                  && ! excludedPart
                  && ! pliIgnore
                  && ! partIgnore
                  && ! buildModPliIgnore
                  && ! synthBegin) {
                  QString addPart = QString("1 %1  0 0 0  0 0 0 0 0 0 0 0 0 %2")
                                            .arg(curMeta.LPub.pli.begin.sub.value().color)
                                            .arg(curMeta.LPub.pli.begin.sub.value().part);
                  opts.pliParts << Pli::partLine(addPart,opts.current,curMeta);
                }

              if (step == nullptr && ! noStep && ! buildMod.ignore) {
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
              if (! buildMod.ignore) {
                QStringList newCSIParts;
                QVector<int> newLineTypeIndexes;
                if (rc == RemoveGroupRc) {
                    remove_group(opts.csiParts,opts.lineTypeIndexes,curMeta.LPub.remove.group.value(),newCSIParts,newLineTypeIndexes,&curMeta);
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
                  Reserve *reserve = new Reserve(topOfStep/*opts.current*/,steps->meta.LPub);
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
                // 0 [ROT]STEP
                // 0 !LPUB INSERT PAGE
                // 0 !LPUB ASSEM CAMERA_ANGLES LOCAL 40.0000 65.0000
                // 0 !LPUB ASSEM MODEL_SCALE LOCAL  2.0000
                // 0 !LPUB INSERT DISPLAY_MODEL
                // 0 [ROT]STEP
                // Note that LOCAL settings must be placed before INSERT PAGE meta command

                Where top = opts.current;
                QString message;
                bool proceed = true;
                if (rc == InsertFinalModelRc) {
                    proceed = Preferences::enableFadeSteps || Preferences::enableHighlightStep;
                    lpub->mi.scanBackwardNoParts(top, StepMask);
                    message = QString("INSERT MODEL meta must be preceded by 0 [ROT]STEP before part (type 1) at line");
                } else { /*InsertDisplayModelRc*/
                    lpub->mi.scanForwardNoParts(top, StepMask);
                    message = QString("INSERT DISPLAY_MODEL meta must be followed by 0 [ROT]STEP before part (type 1) at line");
                }
                if (stepContains(top,partTypeLineRx)) {
                    parseError(message.append(QString(" %1.").arg(top.lineNumber+1)), opts.current, Preferences::InsertErrors);
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
                Where top = opts.current;
                QString message;
                coverPage = true;
                partsAdded = true;
                lpub->page.coverPage = true;
                QRegExp rx("^0 !?LPUB INSERT COVER_PAGE (FRONT|BACK)?$");
                if (line.contains(rx) && rx.cap(1) == "BACK") {
                  lpub->page.backCover  = true;
                  lpub->page.frontCover = false;
                  lpub->mi.scanBackwardNoParts(top, StepMask);
                  message = QString("INSERT COVER_PAGE BACK meta must be preceded by 0 [ROT]STEP before part (type 1) at line");
                } else if (line.contains(rx) && rx.cap(1) == "FRONT") {
                  lpub->page.frontCover = true;
                  lpub->page.backCover  = false;
                  lpub->mi.scanForwardNoParts(top, StepMask);
                  message = QString("INSERT COVER_PAGE FRONT meta must be followed by 0 [ROT]STEP before part (type 1) at line");
                }

                if (stepContains(top,partTypeLineRx)) {
                  parseError(message.append(QString(" %1.").arg(top.lineNumber+1)), opts.current, Preferences::InsertErrors);
                }
              }
              break;

            case InsertPageRc:
              {
                if (stepContains(topOfStep,partTypeLineRx))
                    parseError(QString("INSERT PAGE meta command STEP cannot contain type 1 to 5 line. Invalid type at line %1.").arg(topOfStep.lineNumber+1),
                               opts.current, Preferences::InsertErrors);

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
                     //if (! exporting())
                     //    emit clearViewerWindowSig();
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
              setSceneItemZValueDirection(&selectedSceneItems, curMeta, line);
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

                      pagePointer = pagePointers.value(position);
                      if (pagePointer) {
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
                                if (step)
                                    pagePointer->parentRelativeType = step->relativeType;
                                else
                                    pagePointer->parentRelativeType = SingleStepType;
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
                          pam.setOtherDataInches(p->getPointerAttribInches());
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
                } else if (! buildModIgnoreOverride(buildMod.ignore, buildModTypeIgnore)) {
                  parseError("Failed to process previous BUILD_MOD action for CALLOUT.",opts.current);
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
                  if (!pam.value().id)
                      break;
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
                      pam.setOtherDataInches(p->getPointerAttribInches());
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
              } else if (multiStep) {
                  parseError("Nested MULTI_STEP not allowed",opts.current);
              } else if (! (multiStep = buildModIgnoreOverride(buildMod.ignore, buildModTypeIgnore))) {
                  parseError("Failed to process previous BUILD_MOD action for MULTI_STEP.",opts.current);
              } else {
                steps->relativeType = StepGroupType;
                steps->setTopOfSteps(opts.current);
              }
              break;

            case StepGroupDividerRc:
              if (multiStep && range) {
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
              if ((multiStepPage = multiStep && steps->list.size())) {
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
                  * Steps can be updated after the first step group step (when the groupStepMeta is set).
                  * Populating the groupStepMeta at the first group step is necessary to capture any
                  * step group specific settings as there is no reasonable way to know when the step group
                  * ends and capturing the curMeta after is too late as it is popped at the end of every step.
                  * steps->meta = curMeta
                  */

                  // get the default number of submodel instances in the model file
                  instances = lpub->ldrawFile.instances(opts.current.modelName, opts.isMirrored);
                  if (countInstances)
                     displayInstanceCount = instances > 1 || steps->meta.LPub.page.countInstanceOverride.value() > 1;
                  // count the instances - use steps->meta (vs. steps->groupStepMeta) to access current submodelStack
                  //
                  // lpub->ldrawFile.instances() configuration is CountAtTop - the historic LPub count scheme. However,
                  // the updated countInstances routine's configuration is CountAtModel - this is the default options set
                  // and configurable in Project globals
                  if (displayInstanceCount) {
                      // manually override the count instance value using 0 !LPUB SUBMODEL_INSTANCE_COUNT_OVERRIDE
                      if (steps->groupStepMeta.LPub.page.countInstanceOverride.value())
                          instances = steps->groupStepMeta.LPub.page.countInstanceOverride.value();
                      else
                      if (countInstances == CountAtStep)
                          instances = lpub->mi.countInstancesInStep(&steps->meta, opts.current.modelName);
                      else
                      if (countInstances > CountFalse && countInstances < CountAtStep)
                          instances = lpub->mi.countInstancesInModel(&steps->meta, opts.current.modelName);
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
                      if (/*steps->meta*/steps->groupStepMeta.LPub.multiStep.showGroupStepNumber.value()) {
                          placementData = steps->groupStepMeta.LPub.multiStep.stepNum.placement.value();
                          if ((placementData.relativeTo    == CsiType ||
                              (placementData.relativeTo    == PartsListType &&
                               placementData.justification == Top &&
                               placementData.placement     == Left &&
                               placementData.preposition   == Outside))) {
                              // Place the step number relative to page header
                              steps->groupStepMeta.LPub.multiStep.stepNum.placement.setValue(BottomLeftOutside,PageHeaderType);
                            }

                          // add the step number
                          steps->groupStepNumber.placement = steps->groupStepMeta.LPub.multiStep.stepNum.placement;
                          steps->groupStepNumber.margin    = steps->groupStepMeta.LPub.multiStep.stepNum.margin;
                          steps->groupStepNumber.number    = opts.groupStepNumber;
                          steps->groupStepNumber.sizeit();

                          emit messageSig(LOG_DEBUG, "Add Step group step number for multi-step page " + opts.current.modelName);

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
                      // step number not shown
                      else {
                          placementData = steps->groupStepMeta.LPub.multiStep.pli.placement.value();
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

                          steps->subModel.displayInstanceCount = displayInstanceCount;
                          if (steps->subModel.sizeSubModel(&steps->groupStepMeta,StepGroupType,false) != 0)
                              emit messageSig(LOG_ERROR, "Failed to set Submodel Preview for " + topOfStep.modelName + "...");
                      }
                  } // pli per step = false
                  else
                  { // pli per step = true
                      steps->groupStepNumber.number     = 0;
                      steps->groupStepNumber.stepNumber = nullptr;

                      // check the pli placement to be sure it's relative to CsiType
                      if (steps->groupStepMeta.LPub.multiStep.pli.placement.value().relativeTo != CsiType) {
                          PlacementData placementData = steps->groupStepMeta.LPub.multiStep.pli.placement.value();
                          QString message = QString("Invalid PLI placement. "
                                                    "Step group PLI per STEP set to TRUE but PLI placement is "
                                                    + placementNames[  placementData.placement] + " "
                                                    + relativeNames [  placementData.relativeTo] + " "
                                                    + prepositionNames[placementData.preposition] + "<br>"
                                                    "It should be relative to "
                                                    + relativeNames [CsiType] + ".<br>"
                                                    "A valid placeemnt is MULTI_STEP PLI PLACEMENT "
                                                    + placementNames[TopLeft] + " "
                                                    + relativeNames [CsiType] + " "
                                                    + prepositionNames[Outside] + "<br>");
                          parseError(message, opts.current,Preferences::ParseErrors,false,true/*overide*/);
                      }
                  }
                  opts.pliParts.clear();
                  opts.pliPartGroups.clear();

                  /* this is a page we're supposed to process */

                  // Update steps->meta args with stepGroup args - perhaps update entire steps->meta ?
                  // because steps->meta is popped at the top of the STEP containing MULTI_STEP END
                  steps->placement = steps->groupStepMeta.LPub.multiStep.placement;                  // was steps->meta.LPub.multiStep.placement
                  steps->meta.LPub.page.background = steps->groupStepMeta.LPub.page.background;
                  steps->meta.LPub.multiStep.placement = steps->groupStepMeta.LPub.multiStep.placement;

                  bool endOfSubmodel =
                          /*steps->meta*/steps->groupStepMeta.LPub.contStepNumbers.value() ?
                              /*steps->meta*/steps->groupStepMeta.LPub.contModelStepNum.value() >= lpub->ldrawFile.numSteps(opts.current.modelName) :
                              opts.stepNum - 1 >= lpub->ldrawFile.numSteps(opts.current.modelName);

                  // set csi annotations - multistep
                  if (! exportingObjects()) {
                      suspendFileDisplay = true;
                      //steps->setCsiAnnotationMetas();
                      returnValue = static_cast<TraverseRc>(lpub->mi.setCsiAnnotationMetas(steps));
                      suspendFileDisplay = false;
                      if (returnValue == HitCsiAnnotation)
                          return static_cast<int>(returnValue);
                  }

                  Page *page = dynamic_cast<Page *>(steps);
                  if (page) {
                      page->inserts              = inserts;
                      page->instances            = instances;
                      page->displayInstanceCount = displayInstanceCount;
                      page->pagePointers         = pagePointers;
                      page->selectedSceneItems   = selectedSceneItems;
                    }

                  emit messageSig(LOG_STATUS, "Generate CSI images for multi-step page " + opts.current.modelName);

                  if (renderer->useLDViewSCall() && opts.ldrStepFiles.size() > 0) {
                      QElapsedTimer timer;
                      timer.start();
                      QString empty("");

                      // renderer parms are added to csiKeys in createCsi()

                      if (static_cast<TraverseRc>(renderer->renderCsi(empty,opts.ldrStepFiles,opts.csiKeys,empty,/*steps->meta*/steps->groupStepMeta)) != HitNothing) {
                          emit messageSig(LOG_ERROR,QMessageBox::tr("Render CSI images failed."));
                      }

                      emit messageSig(LOG_INFO,
                                          QString("%1 CSI (Single Call) render took "
                                                  "%2 milliseconds to render %3 [Step %4] %5 "
                                                  "for %6 step group on page %7.")
                                             .arg(rendererNames[Render::getRenderer()])
                                             .arg(timer.elapsed())
                                             .arg(opts.ldrStepFiles.size())
                                             .arg(opts.stepNum)
                                             .arg(opts.ldrStepFiles.size() == 1 ? "image" : "images")
                                             .arg(opts.calledOut ? "called out," : "simple,")
                                             .arg(stepPageNum));
                  }

                  if (Preferences::modeGUI) {
                      // Load the Visual Editor with the last step of the step group
                      Step *lastStep = step;
                      if (!lastStep) {
                          Range *lastRange = range;
                          if (!lastRange)
                              lastRange = dynamic_cast<Range *>(steps->list[steps->list.size() - 1]);
                          if (lastRange) {
                              int lastStepIndex = lastRange->list.size() - 1;
                              lastStep = dynamic_cast<Step *>(lastRange->list[lastStepIndex]);
                          }
                      }

                      if (lastStep && !lastStep->csiPixmap.isNull()) {
                          emit messageSig(LOG_DEBUG,QString("Step group last step number %1").arg(lastStep->stepNumber.number));
                          lpub->setCurrentStep(lastStep);
                          if (!exportingObjects()) {
                              showLine(lastStep->topOfStep());
                              lastStep->loadTheViewer();
                          }
                      } else if (!exportingObjects() && step) {
                          lpub->setCurrentStep(step);
                          showLine(steps->topOfSteps());
                      }
                  }

                  if ((returnValue = static_cast<TraverseRc>(addGraphicsPageItems(steps, coverPage, endOfSubmodel, view, scene, opts.printing))) != HitAbortProcess)
                      returnValue = HitEndOfPage;

                  drawPageElapsedTime();

                  if (Gui::abortProcess())
                      returnValue = HitAbortProcess;
                }
              inserts.clear();
              buildModKeys.clear();
              pagePointers.clear();
              selectedSceneItems.clear();
              if (multiStepPage)
                  return static_cast<int>(returnValue);
              break;

            // Update BuildMod action for 'current' step
            case BuildModApplyRc:
            case BuildModRemoveRc:
              if (!Preferences::buildModEnabled)
                  break;
              if (partsAdded)
                  parseError(tr("BUILD_MOD REMOVE/APPLY action command must be placed before step parts"),
                             opts.current,Preferences::BuildModErrors);
              buildModStepIndex = getBuildModStepIndex(topOfStep);
              buildModActionMeta = curMeta.LPub.buildMod;
              buildMod.key = curMeta.LPub.buildMod.key();
              if (buildModContains(buildMod.key)) {
                  if (getBuildModActionPrevIndex(buildMod.key, buildModStepIndex, rc) < buildModStepIndex)
                      parseError(tr("Redundant build modification meta command '%1' - this command can be removed.")
                                    .arg(buildMod.key),opts.current,Preferences::BuildModErrors);
              } else {
                  const QString action = rc == BuildModApplyRc ? tr("Apply") : tr("Remove");
                  parseError(tr("DrawPage %1 BuildMod for key '%2' not found.")
                             .arg(action).arg(buildMod.key),
                             opts.current,Preferences::BuildModErrors);
              }
              buildModActionStep = true;
              if (multiStep || opts.calledOut)
                  buildModChange = topOfStep != steps->topOfSteps(); // uses list[0].topOfStep for both multiStep and callout
              if (buildModChange) {
                  buildModActions.insert(buildMod.level, getBuildModAction(buildMod.key, buildModStepIndex));
                  if (buildModActions.value(buildMod.level) != rc) {
#ifdef QT_DEBUG_MODE
                      emit gui->messageSig(LOG_NOTICE, QString("Setup Reset Build Mod - Key: '%1', Current Action: %2, Next Action: %3")
                                           .arg(buildMod.key)
                                           .arg(buildMod.action == BuildModRemoveRc ? "Remove(65)" : "Apply(64)")
                                           .arg(rc == BuildModRemoveRc ? "Remove(65)" : "Apply(64)"));
#endif
                      // set BuildMod step has action in current step
                      const QString buildModStepKey = getViewerStepKey(getBuildModStepIndex(buildMod.key));
                      lpub->ldrawFile.setViewerStepHasBuildModAction(buildModStepKey, true);
                      // set BuildMod action for current step
                      setBuildModAction(buildMod.key, buildModStepIndex, rc);
                      // set buildModStepIndex for writeToTmp() and findPage() content
                      setBuildModNextStepIndex(topOfStep);
                      // Check if CsiAnnotation and process them if any
                      if (! exportingObjects() && (multiStep || opts.calledOut))
                          lpub->mi.setCsiAnnotationMetas(steps);
                      // Rerun to findPage() to regenerate parts and options for buildMod action
                      pageProcessRunning = PROC_FIND_PAGE;
                      return static_cast<int>(HitBuildModAction);
                  } // buildMod action != rc
              } // buildModChange
              break;

            // Get BuildMod attributes and set ModIgnore based on 'current' step buildModActions
            case BuildModBeginRc:
              if (!Preferences::buildModEnabled) {
                  buildMod.ignore = true;
                  break;
              }
              buildModMeta = curMeta.LPub.buildMod;
              buildMod.key = curMeta.LPub.buildMod.key();
              buildModExists = buildModContains(buildMod.key);
              buildMod.level = getLevel(buildMod.key, BM_BEGIN);
              // assign buildMod key
              if (! buildModKeys.contains(buildMod.level))
                  buildModKeys.insert(buildMod.level, buildMod.key);
              // step-group and callout build modifications are parsed and configured in the following lines
              // insert new or update existing buildMod
              if (multiStep)
                  buildModInsert = topOfStep != steps->topOfSteps();
              if (opts.calledOut)
                  buildModInsert = !buildModExists;
              if (buildModInsert)
                  insertAttribute(buildModAttributes, BM_BEGIN_LINE_NUM, opts.current);
              // set buildModActions
              if (buildModExists)
                  if (multiStep || opts.calledOut) // the last action is appended at each step so it should always be current
                      buildModActions.insert(buildMod.level, getBuildModAction(buildMod.key, BM_LAST_ACTION));
                  else // take the action for the current step if exists or last action
                      buildModActions.insert(buildMod.level, getBuildModAction(buildMod.key, getBuildModStepIndex(topOfStep), BM_LAST_ACTION));
              else if (buildModInsert)
                  buildModActions.insert(buildMod.level, BuildModApplyRc);
              else
                  parseError(tr("Build modification '%1' is not registered. BuildMod key was not found.").arg(buildMod.key),
                                opts.current,Preferences::BuildModErrors);
              // set buildMod and buildModPli ignore
              if (buildModActions.value(buildMod.level) == BuildModApplyRc) {
                  buildMod.ignore = false;
                  buildModPliIgnore = true;
              } else if (buildModActions.value(buildMod.level) == BuildModRemoveRc) {
                  buildMod.ignore = true;
                  buildModPliIgnore = pliIgnore || excludedPart ;
              }
              buildMod.state = BM_BEGIN;
              break;

            // Set modActionLineNum and ModIgnore based on 'current' step buildModActions
            case BuildModEndModRc:
              if (!Preferences::buildModEnabled) {
                  buildMod.ignore = getLevel(QString(), BM_END);
                  break;
              }
              if (buildMod.level > 1 && buildMod.key.isEmpty())
                  parseError("Key required for nested build mod meta command",
                             opts.current,Preferences::BuildModErrors);
              if (buildMod.state != BM_BEGIN)
                  parseError(QString("Required meta BUILD_MOD BEGIN not found"), opts.current, Preferences::BuildModErrors);
              if (buildModInsert)
                  insertAttribute(buildModAttributes, BM_ACTION_LINE_NUM, opts.current);
              // set buildMod and buildModPli ignore
              if (buildModActions.value(buildMod.level) == BuildModApplyRc) {
                  buildMod.ignore = true;
                  buildModPliIgnore = pliIgnore || excludedPart;
              } else if (buildModActions.value(buildMod.level) == BuildModRemoveRc) {
                  buildMod.ignore = false;
                  buildModPliIgnore = true;
              }
              buildModTypeIgnore = false;
              buildMod.state = BM_END_MOD;
              break;

            // Get buildModLevel and reset ModIgnore to default
            case BuildModEndRc:
              if (!Preferences::buildModEnabled)
                  break;
              if (buildMod.state != BM_END_MOD)
                  parseError(QString("Required meta BUILD_MOD END_MOD not found"), opts.current, Preferences::BuildModErrors);
              if (buildModInsert)
                  insertAttribute(buildModAttributes, BM_END_LINE_NUM, opts.current);
              buildMod.level = getLevel(QString(), BM_END);
              if (buildMod.level == opts.buildModLevel) {
                  buildMod.ignore = false;
                  buildModPliIgnore = pliIgnore || excludedPart;
              }
              buildMod.state = BM_END;
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
              if (! buildMod.ignore && noStep && opts.calledOut) {
                if (opts.current.modelName.contains("whole_rotated_") ||
                    opts.current.modelName.contains("whole_assembled_")) {
                  bool nsHasParts    = false;
                  bool nsHasNoStep   = false;
                  bool nsIsStepGroup = false;
                  Meta nsMeta        = curMeta;
                  QStringList nsTokens;
                  split(addLine,nsTokens);
                  // start with the original subfile content
                  QStringList nsContent = lpub->ldrawFile.contents(nsTokens[14]);
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
                        Rc nsrc = nsMeta.parse(nsLine,nsWalkBack,false);
                        if (nsrc == StepRc || nsrc == RotStepRc) {
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
                        } else if (nsrc == NoStepRc) {
                          // NOSTEP encountered so record it and continue to the top of the step group
                          nsHasNoStep = true;
                        } else if (nsrc == StepGroupEndRc) {
                          // we are in a step group so proceed
                          nsIsStepGroup = true;
                        } else if (nsrc == StepGroupBeginRc) {
                          // we have reached the top of the step group so break
                          break;
                        }
                      }
                    }
                  }
                }
              } // STEP - special case of step group with NOSTEP as last step and rotated or assembled called out submodel

             /*
              * STEP - special case of no parts added, but BFX load or BuildMod Action and not NOSTEP
              */
              if (! partsAdded && ! noStep && (bfxLoad || buildModActionStep)) {
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

                  QString caseType;
                  if (bfxLoad) {
                      caseType.append("bfx load");
                      step->bfxLoadStep = bfxLoad;
                  }
                  if (buildModActionStep) {
                      caseType.append(" build modification");
                      step->buildModActionStep = buildModActionStep;
                      if (buildModActionMeta.action())
                          step->buildModActionMeta = buildModActionMeta;
                  }
                  emit messageSig(LOG_INFO, QString("Processing CSI %1 special case for %2...").arg(caseType).arg(topOfStep.modelName));

                  step->updateViewer = opts.updateViewer;

                  if (buildModActionMeta.action())
                      step->buildModActionMeta = buildModActionMeta;

                  returnValue = static_cast<TraverseRc>(step->createCsi(
                        opts.isMirrored ? addLine : "1 color 0 0 0 1 0 0 0 1 0 0 0 1 foo.ldr",
                        configuredCsiParts = configureModelStep(opts.csiParts, opts.stepNum, topOfStep),
                        opts.lineTypeIndexes,
                        &step->csiPixmap,
                        steps->meta));

                  if (renderer->useLDViewSCall() && ! step->ldrName.isNull()) {
                      opts.ldrStepFiles << step->ldrName;
                      opts.csiKeys << step->csiKey; // No parts to process
                    }

                  partsAdded = true; // OK, so this is a lie, but it works
              } // STEP - special case of no parts added, but BFX load or BuildMod Action and not NOSTEP

             /*
              *  STEP - case of not NOSTEP and not BuildMod ignore
              */
              if (! noStep && ! buildMod.ignore) {
                  /*
                   * STEP - normal case of parts added
                   */
                  if (partsAdded) {

                      // set step group page meta attributes first step
                      if (firstGroupStep) {
                          steps->groupStepMeta = curMeta;
                          firstGroupStep = false;
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
                      * STEP - Actual parts added, simple, mulitStep or calledOut (no render graphics)
                      */
                      if (step) {

                          Page *page = dynamic_cast<Page *>(steps);
                          if (page) {
                              page->inserts              = inserts;
                              page->pagePointers         = pagePointers;
                              page->modelDisplayOnlyStep = step->modelDisplayOnlyStep;
                              page->selectedSceneItems   = selectedSceneItems;
                          }

                          emit messageSig(LOG_INFO_STATUS, "Processing CSI for " + topOfStep.modelName + "...");

                          step->updateViewer = opts.updateViewer;

                          if (buildModMeta.action())
                              step->buildModMeta = buildModMeta;

                          if (buildModActionMeta.action())
                              step->buildModActionMeta = buildModActionMeta;

                          returnValue = static_cast<TraverseRc>(step->createCsi(
                                      opts.isMirrored ? addLine : "1 color 0 0 0 1 0 0 0 1 0 0 0 1 foo.ldr",
                                      configuredCsiParts = configureModelStep(opts.csiParts, step->modelDisplayOnlyStep ? -1 : opts.stepNum, topOfStep),
                                      opts.lineTypeIndexes,
                                      &step->csiPixmap,
                                      steps->meta));

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

                              emit messageSig(LOG_INFO, "Processing PLI for " + topOfStep.modelName + "...");

                              step->pli.sizePli(&steps->meta,relativeType,pliPerStep);

                              if (step->placeSubModel) { // Place SubModel at Step 1
                                  emit messageSig(LOG_INFO, "Set first step submodel display for " + topOfStep.modelName + "...");

                                  // get the number of submodel instances in the model file
                                  instances = lpub->ldrawFile.instances(opts.current.modelName, opts.isMirrored);
                                  if (steps->meta.LPub.subModel.showInstanceCount.value())
                                     displayInstanceCount = instances > 1 || steps->meta.LPub.page.countInstanceOverride.value() > 1;
                                  // lpub->ldrawFile.instances() configuration is CountAtTop - the historic LPub count scheme. However,
                                  // the updated countInstances routine's configuration is CountAtModel - this is the default options set
                                  // and configurable in Project globals
                                  if (displayInstanceCount) {
                                      // manually override the count instance value using 0 !LPUB SUBMODEL_INSTANCE_COUNT_OVERRIDE
                                      if (steps->groupStepMeta.LPub.page.countInstanceOverride.value())
                                          instances = steps->groupStepMeta.LPub.page.countInstanceOverride.value();
                                      else
                                      if (countInstances == CountAtStep)
                                          instances = lpub->mi.countInstancesInStep(&steps->meta, opts.current.modelName);
                                      else
                                      if (countInstances > CountFalse && countInstances < CountAtStep)
                                          instances = lpub->mi.countInstancesInModel(&steps->meta, opts.current.modelName);
                                  }

                                  steps->meta.LPub.subModel.instance.setValue(instances);
                                  step->subModel.setSubModel(opts.current.modelName,steps->meta);

                                  step->subModel.displayInstanceCount = displayInstanceCount;

                                  if (step->subModel.sizeSubModel(&steps->meta,relativeType,pliPerStep) != 0)
                                      emit messageSig(LOG_ERROR, "Failed to set first step submodel display for " + topOfStep.modelName + "...");
                              } // Place SubModel
                              else
                              {
                                  step->subModel.clear();
                              }
                          } // Pli per Step

                          switch (dividerType) {
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
                          } // divider type

                          step->placeRotateIcon = rotateIcon;

                          if (returnValue != HitNothing)
                              emit messageSig(LOG_ERROR, QMessageBox::tr("Create CSI failed to create file."));

                          // BuildMod create and update - performed after createCsi to enable viewerStepKey
                          if (buildModKeys.size()) {
                              if (buildMod.state != BM_END)
                                  parseError(QString("Required meta BUILD_MOD END not found"), opts.current, Preferences::BuildModErrors);
                              Q_FOREACH (int buildModLevel, buildModKeys.keys()) {
                                  if (buildModInsert)
                                      insertBuildModification(buildModLevel);
                                  else
                                      updateBuildModification(buildModLevel);
                              }
                              buildModKeys.clear();
                              buildModAttributes.clear();
                          }

                          // Set CSI annotations - single step only
                          if (! exportingObjects() &&  ! multiStep && ! opts.calledOut) {
                              suspendFileDisplay = true;
                              returnValue = static_cast<TraverseRc>(step->setCsiAnnotationMetas(steps->meta));
                              suspendFileDisplay = false;
                              if (returnValue == HitCsiAnnotation)
                                  return static_cast<int>(returnValue);
                          }

                          if (renderer->useLDViewSCall() && ! step->ldrName.isNull()) {
                              opts.ldrStepFiles << step->ldrName;
                              opts.csiKeys << step->csiKey;
                          }

                      } // STEP - Actual parts added, simple, mulitStep or calledOut (no render graphics)

                      else
                     /*
                      * NO STEP - No step object, e.g. BOM, inserted page (no rendered graphics)
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
                      } // NO STEP - No step object, e.g. inserted page (no rendered graphics)

                     /*
                      *  STEP - Simple, not mulitStep, not calledOut (render graphics)
                      */
                      if ( ! multiStep && ! opts.calledOut) {

                          PlacementType relativeType = SingleStepType;

                          steps->placement = steps->meta.LPub.assem.placement;

                          int  numSteps = lpub->ldrawFile.numSteps(opts.current.modelName);

                          bool endOfSubmodel =
                                  numSteps == 0 ||
                                  steps->meta.LPub.contStepNumbers.value() ?
                                      steps->meta.LPub.contModelStepNum.value() >= numSteps :
                                      opts.stepNum >= numSteps;

                          // get the number of submodel instances in the model file
                          int instances = lpub->ldrawFile.instances(opts.current.modelName, opts.isMirrored);
                          if (countInstances)
                             displayInstanceCount = instances > 1 || steps->meta.LPub.page.countInstanceOverride.value() > 1;
                          // lpub->ldrawFile.instances() configuration is CountAtTop - the historic LPub count scheme. However,
                          // the updated countInstances routine's configuration is CountAtModel - this is the default options set
                          // and configurable in Project globals
                          if (displayInstanceCount) {
                              // manually override the count instance value using 0 !LPUB SUBMODEL_INSTANCE_COUNT_OVERRIDE
                              if (steps->meta.LPub.page.countInstanceOverride.value())
                                  instances = steps->meta.LPub.page.countInstanceOverride.value();
                              else
                              if (countInstances == CountAtStep)
                                  instances = lpub->mi.countInstancesInStep(&steps->meta, opts.current.modelName);
                              else
                              if (countInstances > CountFalse && countInstances < CountAtStep)
                                  instances = lpub->mi.countInstancesInModel(&steps->meta, opts.current.modelName);
                          }

                          Page *page = dynamic_cast<Page *>(steps);
                          if (page && instances > 1) {
                              page->instances            = instances;
                              page->displayInstanceCount = displayInstanceCount;
                              page->inserts              = inserts;
                              page->pagePointers         = pagePointers;
                              page->selectedSceneItems   = selectedSceneItems;

                              if (step) {
                                  page->modelDisplayOnlyStep = step->modelDisplayOnlyStep;
                                  step->lightList = lightList;
                                  step->viewerStepKey = QString("%1;%2;%3%4")
                                          .arg(topOfStep.modelIndex)
                                          .arg(topOfStep.lineNumber)
                                          .arg(opts.stepNum)
                                          .arg(lpub->mi.viewerStepKeySuffix(topOfStep, step));
                              }

                              if (! steps->meta.LPub.stepPli.perStep.value()) {

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

                                          step->subModel.displayInstanceCount = displayInstanceCount;

                                          if (step->subModel.sizeSubModel(&steps->meta,relativeType,pliPerStep) != 0)
                                              emit messageSig(LOG_ERROR, "Failed to set first step submodel display for " + topOfStep.modelName + "...");
                                      }
                                  }
                              } // Not PLI per step
                          } // Page

                          emit messageSig(LOG_INFO, "Generate CSI image for single-step page...");

                          if (renderer->useLDViewSCall() && opts.ldrStepFiles.size() > 0) {

                              QElapsedTimer timer;
                              timer.start();
                              QString empty("");

                              // LDView renderer parms are added to csiKeys in createCsi()

                              // render the partially assembled model
                              returnValue = static_cast<TraverseRc>(renderer->renderCsi(empty,opts.ldrStepFiles,opts.csiKeys,empty,steps->meta));
                              if (returnValue != HitNothing)
                                  emit messageSig(LOG_ERROR,QMessageBox::tr("Render CSI images failed."));

                              emit messageSig(LOG_INFO,
                                                   QString("%1 CSI (Single Call) render took "
                                                           "%2 milliseconds to render %3 [Step %4] %5 for %6 "
                                                           "single step on page %7.")
                                                           .arg(rendererNames[Render::getRenderer()])
                                                           .arg(timer.elapsed())
                                                           .arg(opts.ldrStepFiles.size())
                                                           .arg(opts.stepNum)
                                                           .arg(opts.ldrStepFiles.size() == 1 ? "image" : "images")
                                                           .arg(opts.calledOut ? "called out," : "simple,")
                                                           .arg(stepPageNum));
                          } // useLDViewSCall()

                          // Load the Visual Editor on Step - callouts and multistep Steps are not loaded
                          if (step) {
                              step->setBottomOfStep(opts.current);
                              if (Preferences::modeGUI) {
                                  if (partsAdded && !coverPage) {
                                      lpub->setCurrentStep(step);
                                      if (!exportingObjects()) {
                                          showLine(topOfStep);
                                          step->loadTheViewer();
                                      }
                                  }
                              }
                          } // Load the Visual Editor on Step

                          // Load the top model into the visual editor on cover page
                          if (coverPage && Preferences::modeGUI && !exportingObjects()) {
                              if (curMeta.LPub.coverPageViewEnabled.value()) {
                                  if (step == nullptr) {
                                      if (range == nullptr) {
                                          range = newRange(steps,opts.calledOut);
                                      }
                                      step = new Step(topOfStep,
                                                      range,
                                                      0     /* stepNum */,
                                                      curMeta,
                                                      false /* calledOut */,
                                                      false /* multiStep */);
                                  }

                                  if (step) {
                                      emit messageSig(LOG_INFO, QString("Set cover page model display for %1...")
                                                      .arg(topOfStep.modelName));
                                      step->setBottomOfStep(opts.current);
                                      step->modelDisplayOnlyStep = true;
                                      step->subModel.viewerSubmodel = true;
                                      step->subModel.setSubModel(topOfStep.modelName,steps->meta);
                                      if (step->subModel.sizeSubModel(&steps->meta,relativeType,true) != 0) {
                                          emit gui->messageSig(LOG_ERROR, tr("Failed to set cover page model (%1) display (%2.ldr).").arg(topOfStep.modelName).arg(SUBMODEL_IMAGE_BASENAME));
                                      } else {
                                          // set the current step - enable access from other parts of the application - e.g. Renderer
                                          lpub->setCurrentStep(step);
                                          if (lpub->currentStep) {
                                              if (step->subModel.viewerSubmodelKey == lpub->currentStep->viewerStepKey) {
                                                  showLine(topOfStep);
                                                  const QString modelFileName = QString("%1/%2/%3.ldr").arg(QDir::currentPath()).arg(Paths::tmpDir).arg(SUBMODEL_IMAGE_BASENAME);
                                                  if (!gui->PreviewPiece(modelFileName, LDRAW_MATERIAL_COLOUR))
                                                      emit gui->messageSig(LOG_WARNING, tr("Could not load preview model (%1) file '%2'.").arg(topOfStep.modelName).arg(modelFileName));
                                              } else {
                                                  QString const currentStepKey = lpub->currentStep->viewerStepKey;
                                                  emit gui->messageSig(LOG_WARNING, QObject::tr("The specified submodel step key: '%1' does not match the current step key: '%2'")
                                                                       .arg(step->subModel.viewerSubmodelKey).arg(currentStepKey));
                                              }
                                          }
                                      }
                                  } // step
                              } // cover page view enabled
                          } // cover page

                          if ((returnValue = static_cast<TraverseRc>(addGraphicsPageItems(steps,coverPage,endOfSubmodel,view,scene,opts.printing))) != HitAbortProcess)
                              returnValue = HitEndOfPage;

                          stepPageNum += ! coverPage;

                          steps->setBottomOfSteps(opts.current);

                          drawPageElapsedTime();

                          if (Gui::abortProcess())
                              returnValue = HitAbortProcess;

                          return static_cast<int>(returnValue);
                      } // STEP - Simple, not mulitStep, not calledOut (draw graphics)
#ifdef WRITE_PARTS_DEBUG
                      writeDrawPartsFile();
#endif
                      drawPageStatus(end);

                      lightList.clear();

                      dividerType = NoDivider;

                      // increment continuous step number
                      if (multiStep && steps->groupStepMeta.LPub.contStepNumbers.value())
                          steps->groupStepMeta.LPub.contModelStepNum.setValue(
                                      steps->groupStepMeta.LPub.contModelStepNum.value() + partsAdded);

                      // reset local fade previous steps
                      int local = 1; bool reset = true;
                      if (curMeta.LPub.callout.csi.fadeSteps.enable.pushed == local)
                          curMeta.LPub.callout.csi.fadeSteps.setPreferences(reset);
                      else if (curMeta.LPub.multiStep.csi.fadeSteps.enable.pushed == local)
                          curMeta.LPub.multiStep.csi.fadeSteps.setPreferences(reset);
                      else if (curMeta.LPub.assem.fadeSteps.enable.pushed == local)
                          curMeta.LPub.assem.fadeSteps.setPreferences(reset);
                      else if (curMeta.LPub.fadeSteps.enable.pushed == local) {
                          Gui::suspendFileDisplay = true;
                          deleteFinalModelStep();
                          Gui::suspendFileDisplay = false;
                          curMeta.LPub.fadeSteps.setPreferences(reset);
                      }

                      // reset local highlight current step
                      if (curMeta.LPub.callout.csi.highlightStep.enable.pushed == local)
                          curMeta.LPub.callout.csi.highlightStep.setPreferences(reset);
                      else if (curMeta.LPub.multiStep.csi.highlightStep.enable.pushed == local)
                          curMeta.LPub.multiStep.csi.highlightStep.setPreferences(reset);
                      else if (curMeta.LPub.assem.highlightStep.enable.pushed == local)
                          curMeta.LPub.assem.highlightStep.setPreferences(reset);
                      else if (curMeta.LPub.highlightStep.enable.pushed == local) {
                          Gui::suspendFileDisplay = true;
                          deleteFinalModelStep();
                          Gui::suspendFileDisplay = false;
                          curMeta.LPub.highlightStep.setPreferences(reset);
                      }

                      // reset local preferred renderer
                      if (curMeta.LPub.callout.csi.preferredRenderer.pushed == local) {
                          curMeta.LPub.callout.csi.preferredRenderer.setPreferences(reset);
                          curMeta.LPub.callout.csi.resetCameraFoV();
                      } else if (curMeta.LPub.multiStep.csi.preferredRenderer.pushed == local) {
                          curMeta.LPub.multiStep.csi.preferredRenderer.setPreferences(reset);
                          curMeta.LPub.multiStep.csi.resetCameraFoV();
                      } else if (curMeta.LPub.assem.preferredRenderer.pushed == local) {
                          curMeta.LPub.assem.preferredRenderer.setPreferences(reset);
                          curMeta.LPub.assem.resetCameraFoV();
                      } else if (curMeta.LPub.subModel.preferredRenderer.pushed == local) {
                          curMeta.LPub.subModel.preferredRenderer.setPreferences(reset);
                          curMeta.LPub.subModel.resetCameraFoV();
                      } else if (curMeta.LPub.pli.preferredRenderer.pushed == local) {
                          curMeta.LPub.pli.preferredRenderer.setPreferences(reset);
                          curMeta.LPub.pli.resetCameraFoV();
                      } else if (curMeta.LPub.bom.preferredRenderer.pushed == local) {
                          curMeta.LPub.bom.preferredRenderer.setPreferences(reset);
                          curMeta.LPub.bom.resetCameraFoV();
                      }

                      steps->meta.pop();
                      curMeta.LPub.buildMod.clear();
                      opts.stepNum  += partsAdded;
                      opts.bfxStore2 = bfxStore1;
                      topOfStep      = opts.current;  // set next step
                      partsAdded     = false;
                      coverPage      = false;
                      rotateIcon     = false;
                      bfxStore1      = false;
                      bfxLoad        = false;
                      buildModActionStep = false;
                      step           = nullptr;

                  } // STEP - normal case of parts added
                  else
                  /*
                   *  STEP - case no parts added - e.g. model starting with ROTSTEP END
                   */
                  {
                      ;
//                      topOfStep     = opts.current;  // set next step
                  }
              } // STEP - case of not NOSTEP and not BuildMod ignore

              if ( ! multiStep) {
                  inserts.clear();
                  pagePointers.clear();
                  selectedSceneItems.clear();
              }

              steps->setBottomOfSteps(opts.current);
              steps->meta.LPub.buildMod.clear();
              buildModTypeIgnore = false;
              buildModActions.clear();
              buildMod.clear();
              noStep = false;
              break;

            case RangeErrorRc:
              {
                showLine(opts.current);
                QString message;
                if (Preferences::preferredRenderer == RENDERER_NATIVE &&
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

                returnValue = HitRangeError;

                emit messageSig(LOG_ERROR,message);

              }
            default:
              break;
            } // Handle Specific meta-command
      } // STEP - Process meta-command

      // STEP - Process invalid line

      else if (line != "") {
          const QChar type = line.at(0);

          parseError(QString("Invalid LDraw type %1 line. Expected %2 elements, got \"%3\".")
                     .arg(type).arg(type == '1' ? 15 : type == '2' ? 8 : type == '3' ? 11 : 14).arg(line),opts.current);

          returnValue = HitInvalidLDrawLine;
      } // Process invalid line

      /* unset excludedPart */

      excludedPart = false;

    } // for every line

  // if we get here it's likely and empty page or cover page...
  drawPageElapsedTime();

  if (Gui::abortProcess()) {
      pageProcessRunning = PROC_FIND_PAGE;
      if (returnValue != HitInvalidLDrawLine && returnValue != HitRangeError)
          returnValue = HitAbortProcess;
  }

  return static_cast<int>(returnValue);
}

int Gui::findPage(
    LGraphicsView   *view,
    LGraphicsScene  *scene,
    Meta             meta,
    QString const   &addLine,
    FindPageOptions &opts)
{
  bool isPreDisplayPage = true;
  bool isDisplayPage    = false;

  opts.pageDisplayed    = (opts.pageNum > displayPageNum) && (opts.printing ? displayPageNum : true);

  pageProcessRunning    = PROC_FIND_PAGE;

  emit messageSig(LOG_STATUS, "Processing find page for " + opts.current.modelName + "...");

  skipHeader(opts.current);

  opts.stepNumber = 1 + sa;

  if (opts.pageNum == 1 + pa) {
#ifdef QT_DEBUG_MODE
      emit messageSig(LOG_NOTICE, QString("FINDPAGE  - Page 000 topOfPage First Page Start   (ops) - LineNumber %1, ModelName %2")
                      .arg(opts.current.lineNumber, 3, 10, QChar('0')).arg(opts.current.modelName));
#endif
      topOfPages.clear();
      topOfPages.append(opts.current);
      LDrawFile::_currentLevels.clear();
  }

  Rc rc;
  QStringList bfxParts;
  QStringList saveBfxParts;
  QStringList ldrStepFiles;
  QStringList csiKeys;

  int  saveStepNumber = opts.stepNumber;

  QStringList  csiParts;
  QStringList  saveCsiParts;
  QVector<int> lineTypeIndexes;
  QVector<int> saveLineTypeIndexes;
  Where        saveCurrent = opts.current;
  Where        topOfStep = opts.current;
  Where        stepGroupCurrent;

  saveStepPageNum = stepPageNum;

  Meta         saveMeta = meta;

  QHash<QString, QStringList>  bfx;
  QHash<QString, QStringList>  saveBfx;
  QHash<QString, QVector<int>> bfxLineTypeIndexes;
  QHash<QString, QVector<int>> saveBfxLineTypeIndexes;
  QList<PliPartGroupMeta>      emptyPartGroups;

  opts.flags.numLines = lpub->ldrawFile.size(opts.current.modelName);

  opts.flags.countInstances = meta.LPub.countInstance.value();

  RotStepMeta saveRotStep = meta.rotStep;

  bool                    buildModItems = false;
  bool                    buildModInsert = false;

  BuildModFlags           buildMod;
  QMap<int, QString>      buildModKeys;
  QMap<int,int>           buildModActions;
  QStringList             buildModCsiParts;
  QMap<int, QVector<int>> buildModAttributes;
  QVector<int>            buildModLineTypeIndexes;

  auto insertAttribute =
          [&buildMod,
           &topOfStep] (
          QMap<int, QVector<int>> &buildModAttributes,
          int index, const Where &here)
  {
      QMap<int, QVector<int>>::iterator i = buildModAttributes.find(buildMod.level);
      if (i == buildModAttributes.end()) {
          QVector<int> modAttributes = { 0, 0, 0, 1, 0, topOfStep.modelIndex, 0, 0 };
          modAttributes[index] = here.lineNumber;
          buildModAttributes.insert(buildMod.level, modAttributes);
      } else {
          i.value()[index] = here.lineNumber;
      }
  };

  auto insertBuildModification =
         [this,
          &opts,
          &buildModAttributes,
          &buildModKeys,
          &topOfStep] (int buildModLevel)
  {
      int buildModStepIndex = getBuildModStepIndex(topOfStep);
      QString buildModKey = buildModKeys.value(buildModLevel);
      QVector<int> modAttributes = { 0, 0, 0, 0, 0, -1, 0, 0 };

      QMap<int, QVector<int>>::iterator i = buildModAttributes.find(buildModLevel);
      if (i == buildModAttributes.end()) {
          emit gui->messageSig(LOG_ERROR, QString("Invalid BuildMod Entry for key: %1").arg(buildModKey));
          return;
      }
      modAttributes = i.value();

      modAttributes[BM_DISPLAY_PAGE_NUM] = displayPageNum;
      modAttributes[BM_STEP_PIECES]      = opts.flags.partsAdded;
      modAttributes[BM_MODEL_NAME_INDEX] = topOfStep.modelIndex;
      modAttributes[BM_MODEL_LINE_NUM]   = topOfStep.lineNumber;
      modAttributes[BM_MODEL_STEP_NUM]   = opts.stepNumber;

#ifdef QT_DEBUG_MODE
    emit gui->messageSig(LOG_DEBUG, QString(
                         "Insert FindPage BuildMod StepIndex: %1, "
                         "Action: Apply(64), "
                         "Attributes: %2 %3 %4 %5 %6 %7 %8 %9, "
                         "ModKey: %10, "
                         "Level: %11")
                         .arg(buildModStepIndex)                      // Attribute Default Initial:
                         .arg(modAttributes.at(BM_BEGIN_LINE_NUM))    // 0         0       this
                         .arg(modAttributes.at(BM_ACTION_LINE_NUM))   // 1         0       this
                         .arg(modAttributes.at(BM_END_LINE_NUM))      // 2         0       this
                         .arg(modAttributes.at(BM_DISPLAY_PAGE_NUM))  // 3         0       this
                         .arg(modAttributes.at(BM_STEP_PIECES))       // 4         0       this
                         .arg(modAttributes.at(BM_MODEL_NAME_INDEX))  // 5        -1       this
                         .arg(modAttributes.at(BM_MODEL_LINE_NUM))    // 6         0       this
                         .arg(modAttributes.at(BM_MODEL_STEP_NUM))    // 7         0       this
                         .arg(buildModKey)
                         .arg(buildModLevel));
#endif

      insertBuildMod(buildModKey,
                     modAttributes,
                     buildModStepIndex);
  };

  PartLineAttributes pla(
  csiParts,
  lineTypeIndexes,
  buildModCsiParts,
  buildModLineTypeIndexes,
  buildMod.level,
  buildMod.ignore,
  buildModItems);

#ifdef WRITE_PARTS_DEBUG
  auto writeFindPartsFile = [this,&topOfStep, &saveCsiParts, &opts](
          const QString &insert = QString(),
          const QStringList &parts = QStringList())
  {
      const QStringList &partList = parts.isEmpty() ? saveCsiParts : parts;
      if (partList.isEmpty())
          return;
      const QString nameInsert = insert.isEmpty() ? "a_find_parts" : insert;
      const QString filePath = QDir::currentPath() + "/" + Paths::tmpDir;
      const QString outfileName = QString("%1/%2_%3.ldr")
                    .arg(filePath)
                    .arg(insert)
                    .arg(QString("page_%1_step_%2_model_%3_line_%4")
                         .arg(opts.pageNum)          // Page Number
                         .arg(opts.stepNumber)       // Step Number
                         .arg(topOfStep.modelIndex)  // ModelIndex
                         .arg(topOfStep.lineNumber));// LineNumber

      QFile file(outfileName);
      if ( ! file.open(QFile::WriteOnly | QFile::Text))
          messageSig(LOG_ERROR,QString("Cannot open find_parts file %1 for writing: %2")
                                       .arg(outfileName) .arg(file.errorString()));
      QTextStream out(&file);
      for (int i = 0; i < partList.size(); i++)
          out << partList[i] << lpub_endl;
      file.close();
  };
#endif

  lpub->ldrawFile.setRendered(opts.current.modelName, opts.isMirrored, opts.renderParentModel, opts.stepNumber/*opts.groupStepNumber*/, opts.flags.countInstances);

  /*
   * For findPage(), the BuildMod behaviour captures the appropriate 'block' of lines
   * to be written to the csiPart list and writes the build mod action setting at each
   * step where the BUILD_MOD APPLY or BUILD_MOD REMOVE action meta command is encountered.
   *
   * The buildModLevel flag is enabled for the lines between BUILD_MOD BEGIN and BUILD_MOD END
   * Lines between BUILD_MOD BEGIN and BUILD_MOD END_MOD are the modified content
   * Lines between BUILD_MOD END_MOD and BUILD_MOD END are the original content
   *
   * When the buildModLevel flag is true (greater than 0):
   * Funct 1 (csiParts):
   * Parse is enabled when 'buildModIgnore' is false.
   * When the build mod action is 'apply', the modified content block is parsed. (buildModIgnore is false)
   * When the build mod action is 'remove', the original content block is parsed.     (buildModIgnore is false)
   * Remove group, partType and partName are only applied when 'buildModIgnore is false.
   *
   * When the build mod meta command is BUILD_MOD END 'buildModIgnore' and 'buildModPliIgnore'
   * are reset to false and buildModLevel is reset to false (0), if the build mod command is not nested
   * in the same
   *
   * When BUILD_MOD APPLY or BUILD_MOD REMOVE action meta command is encountered,
   * the respective build mod action is set or updated in ldrawFiles buildMod. Build
   * action is persisted as a key,value pair whereby the key is the step number and
   * the value is the build action. These updates are performed up to the display page
   * at each execution of findPage().
   *
   * However additional buildMod parsing are performed in the countPages() call. Particularly
   * processing buildMod commands in partially processed submodels (e.g. when a submodel
   * has multiple pages whereby the buildMod command starts in findPage() and ends in countPages().
   * Additionally, when jumping ahead during navigation, buildMod commands are processed in countPages()
   *
   * When the buildModLevel flag is false pli and csi lines are processed normally
   */

  for ( ;
        opts.current.lineNumber < opts.flags.numLines && ! opts.pageDisplayed && ! Gui::abortProcess();
        opts.current.lineNumber++) {

      // if reading include file, return to current line, do not advance

      if (static_cast<Rc>(opts.flags.includeFileRc) != EndOfIncludeFileRc && opts.flags.includeFileFound) {
         opts.current.lineNumber--;
      }

      // scan through the rest of the model counting pages
      // if we've already hit the display page, then do as little as possible

      QString line = lpub->ldrawFile.readLine(opts.current.modelName,opts.current.lineNumber).trimmed();

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

          if (tokens.size() > 2 && tokens[1] == LDRAW_MAIN_MATERIAL_COLOUR) {
              split(addLine,addTokens);
              if (addTokens.size() == 15) {
                  tokens[1] = addTokens[1];
              }
              line = tokens.join(" ");
          }

          // process type 1 line...
          if (! opts.flags.partIgnore) {

              // csiParts << line;

              if (firstStepPageNum == -1) {
                  firstStepPageNum = opts.pageNum;
              }
              lastStepPageNum = opts.pageNum;

              QStringList token;

              split(line,token);

              if (token.size() == 15) {

                  QString type = token[token.size()-1];
                  QString colorType = token[1]+type;

                  /* if it is a sub-model (or assembled/rotated callout), then process it.
                   * Called out sub-models (except those assembled/rotated) are processed in drawPage() */

                  bool contains = lpub->ldrawFile.isSubmodel(type);
                  CalloutBeginMeta::CalloutMode calloutMode = meta.LPub.callout.begin.value();

                  // if submodel
                  if (contains) {
                      // when the display page is not the end of a submodel
                      bool partiallyRendered = false;

                      // check if submodel is in current step build modification
                      bool buildModRendered = Preferences::buildModEnabled && (buildMod.ignore || getBuildModRendered(buildMod.key, colorType));

                      // if not callout or assembled/rotated callout
                      if (! opts.flags.callout || (opts.flags.callout && calloutMode != CalloutBeginMeta::Unassembled)) {

                          // check if submodel was rendered
                          bool rendered = lpub->ldrawFile.rendered(type,lpub->ldrawFile.mirrored(token),opts.current.modelName,opts.stepNumber,opts.flags.countInstances);

                          // if the submodel was not rendered, and is not in the buffer exchange call setRendered for the submodel.
                          if (! rendered && ! buildModRendered && (! opts.flags.bfxStore2 || ! bfxParts.contains(colorType))) {

                              if (! buildMod.ignore || ! buildModRendered) {

                                  opts.isMirrored = lpub->ldrawFile.mirrored(token);

                                  // add submodel to the model stack - it can't be a callout
                                  SubmodelStack tos(opts.current.modelName,opts.current.lineNumber,opts.stepNumber);
                                  meta.submodelStack << tos;
                                  Where current2(type,getSubmodelIndex(type),0);

                                  // add buildMod settings for this step, needed for submodels in a buildMod.
                                  const QString levelKey = QString("FindPage BuildMod Key: %1, ParentModel: %2, LineNumber: %3")
                                                                   .arg(buildMod.key)
                                                                   .arg(opts.current.modelName)
                                                                   .arg(opts.current.lineNumber);
                                  FindPageFlags flags2;
                                  flags2.buildModStack << buildMod;
                                  flags2.buildModLevel = buildMod.state == BM_BEGIN ? getLevel(levelKey, BM_CURRENT) : opts.flags.buildModLevel;

                                  lpub->ldrawFile.setModelStartPageNumber(current2.modelName,opts.pageNum);

                                  // save rotStep, clear it, and restore it afterwards
                                  // since rotsteps don't affect submodels
                                  RotStepMeta saveRotStep2 = meta.rotStep;
                                  meta.rotStep.clear();

                                  // set the group step number to the first step of the submodel
                                  if (meta.LPub.multiStep.pli.perStep.value() == false &&
                                      meta.LPub.multiStep.showGroupStepNumber.value()) {
                                      opts.groupStepNumber = opts.stepNumber;
                                  }

                                  // save Default pageSize information
                                  PageSizeData pageSize2;
                                  if (exporting()) {
                                      pageSize2       = pageSizes[DEF_SIZE];
                                      opts.flags.pageSizeUpdate  = false;
#ifdef PAGE_SIZE_DEBUG
                                      logDebug() << "SM: Saving    Default Page size info at PageNumber:" << opts.pageNum
                                                 << "W:"    << pageSize2.sizeW << "H:"    << pageSize2.sizeH
                                                 << "O:"    <<(pageSize2.orientation == Portrait ? "Portrait" : "Landscape")
                                                 << "ID:"   << pageSize2.sizeID
                                                 << "Model:" << opts.current.modelName;
#endif
                                  }

                                  // set the step number and parent model where the submodel will be rendered
                                  FindPageOptions modelOpts(
                                              opts.pageNum,
                                              current2,
                                              opts.pageSize,
                                              flags2,
                                              meta.submodelStack,
                                              opts.pageDisplayed,
                                              opts.updateViewer,
                                              opts.isMirrored,
                                              opts.printing,
                                              opts.stepNumber,
                                              opts.contStepNumber,
                                              opts.groupStepNumber,
                                              opts.current.modelName /*renderParentModel*/);

                                  const TraverseRc frc = static_cast<TraverseRc>(findPage(view, scene, meta, line, modelOpts));
                                  if (frc == HitBuildModAction || frc == HitCsiAnnotation || frc == HitAbortProcess) {
                                      // Set processing state and return to parent findPage
                                      pageProcessRunning = PROC_DISPLAY_PAGE;
                                      return static_cast<int>(frc);
                                  }

                                  opts.pageDisplayed = modelOpts.pageDisplayed;
                                  partiallyRendered = modelOpts.current.lineNumber < modelOpts.flags.numLines;
                                  if (opts.pageDisplayed) {                 // capture where we stopped in the submodel
                                      // when we stop before the end of a child submodel,
                                      // we need to capture the child submodel flags for countPages
                                      if (partiallyRendered) {
                                          opts.pageNum           = modelOpts.pageNum;
                                          opts.current           = modelOpts.current;
                                          opts.pageSize          = modelOpts.pageSize;
                                          opts.flags             = modelOpts.flags;
                                          opts.modelStack        = modelOpts.modelStack;
                                          opts.stepNumber        = modelOpts.stepNumber;
                                          opts.renderParentModel = modelOpts.renderParentModel;
                                          // decrement current lineNumber by 1 line to account for
                                          // lineNumber increment as we iterate to terminate the
                                          // line processing loop at 'pageDisplayed'
                                          opts.current--;
                                          // if we are at the last step of the submodel, with no parts added, turn
                                          // on parseBuildMods in countPage in case ther is a BUILD_MOD REMOVE command.
                                          bool partsAdded;
                                          Where walk = modelOpts.current;
                                          Rc rc = lpub->mi.scanForward(walk,StepMask,partsAdded);
                                          opts.flags.parseBuildMods = (rc == EndOfFileRc && ! partsAdded);
                                          // if no parts added to last step, set partsAdded to -1 so later increment
                                          // will result in a value of 0.
                                          if (opts.flags.parseBuildMods) {
                                              opts.flags.partsAdded = -1;
                                              opts.flags.buildModLevel = modelOpts.flags.buildModLevel;
                                          }
                                      } else {
                                          // capture the final buildMod flags for this page
                                          opts.flags.buildModStack << buildMod;
                                      }
                                  } // opts.pageDisplayed
                                  else if (meta.submodelStack.size())
                                      meta.submodelStack.pop_back();

                                  saveStepPageNum = stepPageNum;
                                  meta.rotStep  = saveRotStep2;             // restore old rotstep

                                  if (opts.contStepNumber) {                // capture continuous step number from exited submodel
                                      opts.contStepNumber = saveContStepNum;
                                  }

                                  if (opts.groupStepNumber) {               // capture group step number from exited submodel
                                      opts.groupStepNumber = saveGroupStepNum;
                                  }

                                  if (exporting()) {
                                      pageSizes.remove(DEF_SIZE);
                                      pageSizes.insert(DEF_SIZE,pageSize2); // restore old Default pageSize information
#ifdef PAGE_SIZE_DEBUG
                                      logDebug() << "SM: Restoring Default Page size info at PageNumber:" << opts.pageNum
                                                 << "W:"    << pageSizes[DEF_SIZE].sizeW << "H:"    << pageSizes[DEF_SIZE].sizeH
                                                 << "O:"    << (pageSizes[DEF_SIZE].orientation == Portrait ? "Portrait" : "Landscape")
                                                 << "ID:"   << pageSizes[DEF_SIZE].sizeID
                                                 << "Model:" << opts.current.modelName;
#endif
                                  } // Exporting

                              } // ! BuildModIgnore

                          } // ! Rendered && (! BfxStore2 || ! BfxParts.contains(colorType))

                      } // ! Callout || (Callout && CalloutMode != CalloutBeginMeta::Unassembled)

                      // add submodel to buildMod rendered list
                      if (Preferences::buildModEnabled &&
                          buildMod.state == BM_BEGIN   &&
                          ! partiallyRendered          &&
                          ! buildModRendered) {
                          setBuildModRendered(buildMod.key, colorType);
                      }

                  } // Contains [IsSubmodel]

                  if (opts.flags.bfxStore1) {
                      bfxParts << colorType;
                  }

              } // Type 1 Line

          } // ! PartIgnore
        case '2':
        case '3':
        case '4':
        case '5':
          if (! buildMod.ignore) {
              ++opts.flags.partsAdded;
              CsiItem::partLine(line,pla,opts.current.lineNumber,OkRc);
            } // ! BuildModIgnore, for each
            break;

        case '0':

          // intercept include file flag

          if (static_cast<Rc>(opts.flags.includeFileRc) != EndOfIncludeFileRc) {
              if (opts.flags.resetIncludeRc) {
                  rc = IncludeRc;                    // return to IncludeRc to parse another line
              } else {
                  rc = static_cast<Rc>(opts.flags.includeFileRc); // execute the Rc returned by include(...)
                  opts.flags.resetIncludeRc = true;  // reset to run include(...) to parse another line
              }
          } else {
              rc = meta.parse(line,opts.current);    // continue
          }

          switch (rc) {
            case StepGroupBeginRc:
              opts.flags.stepGroup = true;
              stepGroupCurrent = topOfStep;
              if (! opts.pageDisplayed) {
                  if (opts.contStepNumber) {    // save starting step group continuous step number to pass to drawPage for submodel preview
                      if (opts.stepNumber == 1 + sa) {
                          int showStepNum = opts.contStepNumber == 1 + sa ? opts.stepNumber : opts.contStepNumber;
                          if (opts.pageNum == 1 + pa) {
                              meta.LPub.subModel.showStepNum.setValue(showStepNum);
                          } else {
                              saveMeta.LPub.subModel.showStepNum.setValue(showStepNum);
                          }
                      }
                  }

                  // New step group page so increment group step number and persisst to global
                  if (opts.groupStepNumber && meta.LPub.multiStep.countGroupSteps.value()) {
                      Where walk(opts.current.modelName);
                      lpub->mi.scanForwardStepGroup(walk);
                      if (opts.current.lineNumber > walk.lineNumber) {
                          opts.groupStepNumber += 1 + sa;
                          saveGroupStepNum = opts.groupStepNumber;
                      }
                  }
              }

              // Steps within step group modify bfxStore2 as they progress
              // so we must save bfxStore2 and use the saved copy when
              // we call drawPage for a step group.
              opts.flags.stepGroupBfxStore2 = opts.flags.bfxStore2;
              break;

            case StepGroupEndRc:
              if (opts.flags.stepGroup && ! opts.flags.noStep2) {
                  opts.flags.stepGroup = false;
                  if (isPreDisplayPage/*opts.pageNum < displayPageNum*/) {
                      saveLineTypeIndexes    = lineTypeIndexes;
                      saveCsiParts           = csiParts;
                      saveStepNumber         = opts.stepNumber;
                      saveMeta               = meta;
                      saveBfx                = bfx;
                      saveBfxParts           = bfxParts;
                      saveBfxLineTypeIndexes = bfxLineTypeIndexes;
                      saveRotStep            = meta.rotStep;
                      bfxParts.clear();
#ifdef WRITE_PARTS_DEBUG
                      writeFindPartsFile("a_find_csi_parts", csiParts);
#endif
                  } else if (isDisplayPage/*opts.pageNum == displayPageNum*/) {
                      // ignored when processing a buildModDisplay
                      savePrevStepPosition = saveCsiParts.size();
                      stepPageNum = saveStepPageNum;
                      if (opts.pageNum == 1 + pa) {
                          lpub->page.meta = meta;
                      } else {
                          lpub->page.meta = saveMeta;
                      }
                      if (opts.contStepNumber) {  // pass continuous step number to drawPage
                          lpub->page.meta.LPub.contModelStepNum.setValue(saveStepNumber);
                          saveStepNumber = saveContStepNum;
                      }
                      if (opts.groupStepNumber) { // persist step group step number
                          saveGroupStepNum = opts.groupStepNumber;
                      }
                      lpub->page.meta.pop();
                      lpub->page.meta.rotStep = saveRotStep;

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
                                  opts.groupStepNumber,    // pass group step number to drawPage
                                  opts.flags.buildModLevel,
                                  opts.updateViewer,
                                  opts.isMirrored,
                                  opts.printing,
                                  opts.flags.stepGroupBfxStore2,
                                  false /*assembledCallout*/,
                                  false /*calldeOut*/);
#ifdef WRITE_PARTS_DEBUG
                      writeFindPartsFile("a_find_save_csi_parts");
#endif
                      const TraverseRc drc = static_cast<TraverseRc>(drawPage(view, scene, &lpub->page, addLine, pageOptions));
                      if (drc == HitBuildModAction || drc == HitCsiAnnotation || drc == HitAbortProcess) {
                          // Set processing state and return to init drawPage
                          pageProcessRunning = PROC_DISPLAY_PAGE;
                          return static_cast<int>(drc);;
                      }

                      opts.pageDisplayed = true;

                      lineTypeIndexes.clear();
                      csiParts.clear();

                      saveCurrent.modelName.clear();
                      saveCurrent.modelIndex = -1;
                      saveCsiParts.clear();
                      saveLineTypeIndexes.clear();
                  } // IsDisplayPage /*opts.pageNum == displayPageNum*/

                  // ignored when processing buildMod display
                  if (exporting()) {
                      pageSizes.remove(opts.pageNum);
                      if (opts.flags.pageSizeUpdate) {
                          opts.flags.pageSizeUpdate = false;
                          pageSizes.insert(opts.pageNum,opts.pageSize);
#ifdef PAGE_SIZE_DEBUG
                          logTrace() << "SG: Inserting New Page size info     at PageNumber:" << opts.pageNum
                                     << "W:"    << opts.pageSize.sizeW << "H:"    << opts.pageSize.sizeH
                                     << "O:"    <<(opts.pageSize.orientation == Portrait ? "Portrait" : "Landscape")
                                     << "ID:"   << opts.pageSize.sizeID
                                     << "Model:" << opts.current.modelName;
#endif
                      } else {
                          pageSizes.insert(opts.pageNum,pageSizes[DEF_SIZE]);
#ifdef PAGE_SIZE_DEBUG
                          logTrace() << "SG: Inserting Default Page size info at PageNumber:" << opts.pageNum
                                     << "W:"    << pageSizes[DEF_SIZE].sizeW << "H:"    << pageSizes[DEF_SIZE].sizeH
                                     << "O:"    << (pageSizes[DEF_SIZE].orientation == Portrait ? "Portrait" : "Landscape")
                                     << "ID:"   << pageSizes[DEF_SIZE].sizeID
                                     << "Model:" << opts.current.modelName;
#endif
                      }
                  } // Exporting

#ifdef QT_DEBUG_MODE
                  emit messageSig(LOG_NOTICE, QString("FINDPAGE  - Page %1 topOfPage StepGroup End      (tos) - LineNumber %2, ModelName %3")
                                  .arg(opts.pageNum, 3, 10, QChar('0')).arg(topOfStep.lineNumber, 3, 10, QChar('0')).arg(topOfStep.modelName));
#endif
                  ++opts.pageNum;
                  opts.flags.addCountPage = true;
                  topOfPages.append(topOfStep/*opts.current*/);  // TopOfSteps(Page) (Next StepGroup), BottomOfSteps(Page) (Current StepGroup)
                  saveStepPageNum = ++stepPageNum;

                  opts.flags.noStep2 = false;

                } // StepGroup && ! NoStep2 (StepGroupEndRc)

              if (opts.current.modelName == topLevelFile())
                  opts.pageDisplayed = opts.pageNum > displayPageNum;

                opts.flags.noStep2 = false;
                break;

               case BuildModApplyRc:
               case BuildModRemoveRc:
                 if (Preferences::buildModEnabled) {
                   // special case where we have BUILD_MOD and NOSTEP commands in the same single STEP
                   if (! opts.flags.parseNoStep && ! opts.pageDisplayed && ! opts.flags.stepGroup && opts.flags.noStep)
                       opts.flags.parseNoStep = meta.LPub.parseNoStep.value();
                   Where current = opts.current;
                   if (lpub->mi.scanForwardNoParts(current, StepMask|StepGroupMask) == StepGroupEndRc)
                       gui->parseErrorSig(QString("BUILD_MOD %1 '%2' must be placed after MULTI_STEP END")
                                                  .arg(rc == BuildModRemoveRc ? QString("REMOVE") : QString("APPLY"))
                                                  .arg(meta.LPub.buildMod.key()), opts.current,Preferences::ParseErrors,false,false);
                 }
                   break;

              // Get BuildMod attributes and set ignore based on 'next' step buildModAction
              case BuildModBeginRc:
                if (!Preferences::buildModEnabled) {
                    buildMod.ignore = true;
                    break;
                }
                buildMod.key    = meta.LPub.buildMod.key();
                buildMod.level  = getLevel(buildMod.key, BM_BEGIN);
                buildMod.action = BuildModApplyRc;
                buildModInsert  = ! buildModContains(buildMod.key);
                if (! opts.pageDisplayed) {
                    if (! buildModInsert)
                        buildModActions.insert(buildMod.level,
                                               getBuildModAction(buildMod.key, getBuildModNextStepIndex(), BM_PREVIOUS_ACTION));
                    else
                        buildModActions.insert(buildMod.level, BuildModApplyRc);
                    if (buildModActions.value(buildMod.level) == BuildModApplyRc)
                        buildMod.ignore = false;
                    else if (buildModActions.value(buildMod.level) == BuildModRemoveRc)
                        buildMod.ignore = true;
                }
                // special case where callout or submodel is in a Build Mod
                buildModInsert &= !isPreDisplayPage;
                if (buildModInsert) {
                    buildModKeys.insert(buildMod.level, buildMod.key);
                    insertAttribute(buildModAttributes, BM_BEGIN_LINE_NUM, opts.current);
                }
                buildMod.state = BM_BEGIN;
                break;

              // Set buildModIgnore based on 'next' step buildModAction
              case BuildModEndModRc:
                if (!Preferences::buildModEnabled) {
                    buildMod.ignore = getLevel(QString(), BM_END);
                    break;
                }
                if (! opts.pageDisplayed) {
                    if (buildModActions.value(buildMod.level) == BuildModApplyRc)
                        buildMod.ignore = true;
                    else if (buildModActions.value(buildMod.level) == BuildModRemoveRc)
                        buildMod.ignore = false;
                }
                if (buildModInsert) {
                    if (buildMod.level > 1 && buildMod.key.isEmpty())
                        parseError("Key required for nested build mod meta command",
                                opts.current,Preferences::BuildModErrors);
                    if (buildMod.state != BM_BEGIN)
                        parseError(QString("Required meta BUILD_MOD BEGIN not found"),
                                opts.current, Preferences::BuildModErrors);
                    insertAttribute(buildModAttributes, BM_ACTION_LINE_NUM, opts.current);
                }
                buildMod.state = BM_END_MOD;
                break;

              // Get buildModLevel and reset buildModIgnore to default
              case BuildModEndRc:
                if (!Preferences::buildModEnabled)
                    break;
                if (! opts.pageDisplayed) {
                    buildMod.level = getLevel(QString(), BM_END);
                    if (buildMod.level == opts.flags.buildModLevel)
                        buildMod.ignore = false;
                }
                if (buildModInsert) {
                    if (buildMod.state != BM_END_MOD)
                        parseError(QString("Required meta BUILD_MOD END_MOD not found"),
                                opts.current, Preferences::BuildModErrors);
                    insertAttribute(buildModAttributes, BM_END_LINE_NUM, opts.current);
                }
                buildMod.state = BM_END;
                break;

            case RotStepRc:
            case StepRc:
              if (opts.flags.partsAdded && (! opts.flags.noStep || opts.flags.parseNoStep)) {
                  if (! buildMod.ignore) {
                    if (opts.contStepNumber) {   // increment continuous step number until we hit the display page
                        if (isPreDisplayPage/*opts.pageNum < displayPageNum*/ &&
                           (opts.stepNumber > FIRST_STEP + sa || displayPageNum > FIRST_PAGE + sa)) { // skip the first step
                            opts.contStepNumber += ! opts.flags.coverPage && ! opts.flags.stepPage;
                        }
                        if (! opts.flags.stepGroup && opts.stepNumber == 1 + sa) {
                            if (opts.pageNum == 1 + pa && topOfStep.modelName == topLevelFile()) { // when pageNum is 1 and not multistep, persist contStepNumber to 'meta' only if we are in the main model
                                meta.LPub.subModel.showStepNum.setValue(opts.contStepNumber);
                            } else {
                                saveMeta.LPub.subModel.showStepNum.setValue(opts.contStepNumber);
                            }
                        }
                    }

                    opts.stepNumber  += ! opts.flags.coverPage && ! opts.flags.stepPage;
                    stepPageNum += ! opts.flags.coverPage && ! opts.flags.stepGroup;

                    if (isPreDisplayPage/*opts.pageNum < displayPageNum*/) {
                        if ( ! opts.flags.stepGroup) {
                            saveLineTypeIndexes    = lineTypeIndexes;
                            saveStepNumber         = opts.stepNumber;
                            saveCsiParts           = csiParts;
                            saveMeta               = meta;
                            saveBfx                = bfx;
                            saveBfxParts           = bfxParts;
                            saveBfxLineTypeIndexes = bfxLineTypeIndexes;
                            saveStepPageNum        = stepPageNum;
                            // bfxParts.clear();
                            if (opts.groupStepNumber &&
                                meta.LPub.multiStep.countGroupSteps.value()) { // count group step number and persist
                                opts.groupStepNumber += ! opts.flags.coverPage && ! opts.flags.stepPage;
                                saveGroupStepNum      = opts.groupStepNumber;
                            }
                            // insert build Mods when processing step group steps after the first step
                            if (opts.flags.stepGroup) {
                                // insert build modifications
                                if (buildModKeys.size()) {
                                    if (buildMod.state != BM_END)
                                        parseError(QString("Required meta BUILD_MOD END not found"),
                                                   opts.current, Preferences::BuildModErrors);
                                    Q_FOREACH (int buildModLevel, buildModKeys.keys()) {
                                        insertBuildModification(buildModLevel);
                                    }
                                    buildModKeys.clear();
                                    buildModAttributes.clear();
                                } // BuildModKeys
                            } // StepGroup
#ifdef WRITE_PARTS_DEBUG
                            writeFindPartsFile("a_find_csi_parts", csiParts);
#endif
                          }
                        if (opts.contStepNumber) { // save continuous step number from current model
                            saveContStepNum = opts.contStepNumber;
                        }
                        saveCurrent = opts.current;
                        saveRotStep = meta.rotStep;
                      } // isPreDisplayPage/*opts.pageNum < displayPageNum*/

                    if ( ! opts.flags.stepGroup) {
                        if (isDisplayPage) {
                            lineTypeIndexes.clear();
                            csiParts.clear();
                            savePrevStepPosition = saveCsiParts.size();
                            stepPageNum = saveStepPageNum;
                            if (opts.pageNum == 1 + pa) {
                                lpub->page.meta = meta;
                            } else {
                                lpub->page.meta = saveMeta;
                            }
                            if (opts.contStepNumber) { // pass continuous step number to drawPage
                                lpub->page.meta.LPub.contModelStepNum.setValue(saveStepNumber);
                                saveStepNumber    = opts.contStepNumber;
                            }
                            if (opts.groupStepNumber) { // pass group step number to drawPage and persist
                                saveGroupStepNum  = opts.groupStepNumber;
                                saveStepNumber    = opts.groupStepNumber;
                            }
                            lpub->page.meta.pop();
                            lpub->page.meta.LPub.buildMod.clear();
                            lpub->page.meta.rotStep = saveRotStep;
                            lpub->page.meta.rotStep = meta.rotStep;

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
                                        opts.groupStepNumber,
                                        opts.flags.buildModLevel,
                                        opts.updateViewer,
                                        opts.isMirrored,
                                        opts.printing,
                                        opts.flags.bfxStore2);
#ifdef WRITE_PARTS_DEBUG
                            writeFindPartsFile("b_find_save_csi_parts");
#endif
                            const TraverseRc drc = static_cast<TraverseRc>(drawPage(view, scene, &lpub->page, addLine, pageOptions));
                            if (drc == HitBuildModAction || drc == HitCsiAnnotation || drc == HitAbortProcess) {
                                // Set processing state and return to init drawPage
                                pageProcessRunning = PROC_DISPLAY_PAGE;
                                return static_cast<int>(drc);
                            }

                            opts.pageDisplayed = true;

                            saveCurrent.modelName.clear();
                            saveCurrent.modelIndex = -1;
                            saveCsiParts.clear();
                            saveLineTypeIndexes.clear();
                            //buildModActions.clear();

                          } // IsDisplayPage /*opts.pageNum == displayPageNum*/

                        if (! opts.flags.noStep) {
                            if (exporting() && ! opts.flags.noStep) {
                                pageSizes.remove(opts.pageNum);
                                if (opts.flags.pageSizeUpdate) {
                                    opts.flags.pageSizeUpdate = false;
                                    pageSizes.insert(opts.pageNum,opts.pageSize);
#ifdef PAGE_SIZE_DEBUG
                                    logTrace() << "ST: Inserting New Page size info     at PageNumber:" << opts.pageNum
                                               << "W:"    << opts.pageSize.sizeW << "H:"    << opts.pageSize.sizeH
                                               << "O:"    <<(opts.pageSize.orientation == Portrait ? "Portrait" : "Landscape")
                                               << "ID:"   << opts.pageSize.sizeID
                                               << "Model:" << opts.current.modelName;
#endif
                                } else {
                                    pageSizes.insert(opts.pageNum,pageSizes[DEF_SIZE]);
#ifdef PAGE_SIZE_DEBUG
                                    logTrace() << "ST: Inserting Default Page size info at PageNumber:" << opts.pageNum
                                               << "W:"    << pageSizes[DEF_SIZE].sizeW << "H:"    << pageSizes[DEF_SIZE].sizeH
                                               << "O:"    << (pageSizes[DEF_SIZE].orientation == Portrait ? "Portrait" : "Landscape")
                                               << "ID:"   << pageSizes[DEF_SIZE].sizeID
                                               << "Model:" << opts.current.modelName;
#endif
                                }
                            } // Exporting

#ifdef QT_DEBUG_MODE
                            emit messageSig(LOG_NOTICE, QString("FINDPAGE  - Page %1 topOfPage Step, Not Group    (opt) - LineNumber %2, ModelName %3")
                                            .arg(opts.pageNum, 3, 10, QChar('0')).arg(opts.current.lineNumber, 3, 10, QChar('0')).arg(opts.current.modelName));
#endif
                            ++opts.pageNum;
                            opts.flags.addCountPage = true;
                            topOfPages.append(opts.current); // TopOfStep (Next Step), BottomOfStep (Current Step)
                        } // ! opts.flags.noStep && ! StepGroup (StepRc,RotStepRc)

                        // insert build Mods when processing single step
                        if (/*opts.pageDisplayed*/isPreDisplayPage) {
                            // insert build modifications
                            if (buildModKeys.size()) {
                                if (buildMod.state != BM_END)
                                    parseError(QString("Required meta BUILD_MOD END not found"),
                                               opts.current, Preferences::BuildModErrors);
                                Q_FOREACH (int buildModLevel, buildModKeys.keys()) {
                                    insertBuildModification(buildModLevel);
                                }
                                buildModKeys.clear();
                                buildModAttributes.clear();
                            } // BuildModKeys
                        } // PageDisplayed
                    } // ! StepGroup

                    topOfStep = opts.current;  // Set next step
                    opts.flags.partsAdded = 0;
                    opts.flags.coverPage  = false;
                    opts.flags.stepPage   = false;
                    opts.flags.bfxStore2  = opts.flags.bfxStore1;
                    opts.flags.bfxStore1  = false;
                    if ( ! opts.flags.bfxStore2) {
                        bfxParts.clear();
                    } // ! BfxStore2
                    meta.pop();
                    lpub->ldrawFile.clearBuildModRendered();
                  } // ! buildMod.ignore
                } // PartsAdded && ! NoStep
              else if ( ! opts.flags.stepGroup)
                {
                  // Adjust current so that draw page doesn't have to deal with
                  // no PartsAdded, NoStep or BuildModIgnore Steps
                  saveCurrent = opts.current;
                } // ! StepGroup

              if (opts.current.modelName == topLevelFile())
                  opts.pageDisplayed = opts.pageNum > displayPageNum;

              buildMod.clear();
              meta.LPub.buildMod.clear();
              opts.flags.noStep2 = opts.flags.noStep;
              opts.flags.noStep = false;
              opts.flags.parseNoStep = false;
              break;

            case CalloutBeginRc:
              opts.flags.callout = true;
              break;

            case CalloutEndRc:
              opts.flags.callout = false;
              meta.LPub.callout.placement.clear();
              break;

            case InsertCoverPageRc:
              opts.flags.coverPage  = true;
              opts.flags.partsAdded = true;
              break;

            case InsertPageRc:
              opts.flags.stepPage   = true;
              opts.flags.partsAdded = true;
              lastStepPageNum       = opts.pageNum;
              break;

            case InsertFinalModelRc:
            case InsertDisplayModelRc:
              lastStepPageNum = opts.pageNum;
              break;

            case PartBeginIgnRc:
              opts.flags.partIgnore = true;
              break;

            case PartEndRc:
              opts.flags.partIgnore = false;
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
              opts.flags.bfxStore1 = true;
              bfxParts.clear();
              break;

            case BufferLoadRc:
              if (isPreDisplayPage/*opts.pageNum < displayPageNum*/) {
                  csiParts = bfx[meta.bfx.value()];
                  lineTypeIndexes = bfxLineTypeIndexes[meta.bfx.value()];
                }
              // special case where we have BUFEXCHG load and NOSTEP commands in the same single STEP
              if (! opts.flags.parseNoStep && ! opts.pageDisplayed && ! opts.flags.stepGroup && opts.flags.noStep)
                  opts.flags.parseNoStep = meta.LPub.parseNoStep.value();
              opts.flags.partsAdded = true;
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
              }
              opts.flags.partsAdded = true;
              break;

              /* remove a group or all instances of a part type */
            case RemoveGroupRc:
            case RemovePartTypeRc:
            case RemovePartNameRc:
              if (isPreDisplayPage/*opts.pageNum < displayPageNum*/) {
                  if (! buildMod.ignore) {
                    QStringList newCSIParts;
                    QVector<int> newLineTypeIndexes;
                    if (rc == RemoveGroupRc) {
                        remove_group(csiParts,lineTypeIndexes,meta.LPub.remove.group.value(),newCSIParts,newLineTypeIndexes,&meta);
                    } else if (rc == RemovePartTypeRc) {
                        remove_parttype(csiParts,lineTypeIndexes,meta.LPub.remove.parttype.value(),newCSIParts,newLineTypeIndexes);
                    } else {
                        remove_partname(csiParts,lineTypeIndexes,meta.LPub.remove.partname.value(),newCSIParts,newLineTypeIndexes);
                    }
                    csiParts = newCSIParts;
                    lineTypeIndexes = newLineTypeIndexes;
                  } // ! buildModIgnore
              }
              break;

            case IncludeRc:
              opts.flags.includeFileRc = include(meta,opts.flags.includeLineNum,opts.flags.includeFileFound);  // includeHere and inserted are include(...) vars
              if (opts.flags.includeFileRc == static_cast<int>(IncludeFileErrorRc)) {
                  opts.flags.includeFileRc = static_cast<int>(EndOfIncludeFileRc);
                  parseError(tr("INCLUDE file was not resolved."),opts.current,Preferences::IncludeFileErrors); // file parse error
              } else if (static_cast<Rc>(opts.flags.includeFileRc) != EndOfIncludeFileRc) { // still reading so continue
                  opts.flags.resetIncludeRc = false;                                        // do not reset, allow includeFileRc to execute
                  continue;
              }
              break;

            case PageSizeRc:
              {
                if (exporting()) {
                    opts.flags.pageSizeUpdate  = true;

                    opts.pageSize.sizeW  = meta.LPub.page.size.valueInches(0);
                    opts.pageSize.sizeH  = meta.LPub.page.size.valueInches(1);
                    opts.pageSize.sizeID = meta.LPub.page.size.valueSizeID();

                    if (meta.LPub.page.size.valueOrientation() != InvalidOrientation) {
                      opts.pageSize.orientation = meta.LPub.page.size.valueOrientation();
                      meta.LPub.page.orientation.setValue(opts.pageSize.orientation);
                    }

                    pageSizes.remove(DEF_SIZE);
                    pageSizes.insert(DEF_SIZE,opts.pageSize);
#ifdef PAGE_SIZE_DEBUG
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
              if (! opts.pageDisplayed)
                  opts.flags.countInstances = meta.LPub.countInstance.value();
              break;

            case ContStepNumRc:
              if (isPreDisplayPage/*opts.pageNum < displayPageNum*/)
              {
                  if (meta.LPub.contStepNumbers.value()) {
                      if (! opts.contStepNumber)
                          opts.contStepNumber += 1 + sa;
                  } else {
                      opts.contStepNumber = 0;
                  }
              }
              break;

            case StartStepNumberRc:
              if (isPreDisplayPage/*opts.pageNum < displayPageNum*/)
              {
                  if ((opts.current.modelName == lpub->ldrawFile.topLevelFile() && opts.flags.partsAdded) ||
                          opts.current.modelName != lpub->ldrawFile.topLevelFile())
                      parseError("Start step number must be specified in the top model header.", opts.current);
                  sa = meta.LPub.startStepNumber.value() - 1;
              }
              break;

            case StartPageNumberRc:
              if (isPreDisplayPage/*opts.pageNum < displayPageNum*/)
              {
                  if ((opts.current.modelName == lpub->ldrawFile.topLevelFile() && opts.flags.partsAdded) ||
                          opts.current.modelName != lpub->ldrawFile.topLevelFile())
                      parseError("Start page number must be specified in the top model header.", opts.current);
                  pa = meta.LPub.startPageNumber.value() - 1;
              }
              break;

            case BuildModEnableRc:
              if (isPreDisplayPage/*opts.pageNum < displayPageNum*/)
              {
                  bool enabled = meta.LPub.buildModEnabled.value();
                  if (Preferences::buildModEnabled != enabled) {
                      Preferences::buildModEnabled  = enabled;
                      enableVisualBuildModification();
                      emit messageSig(LOG_INFO, QString("Build Modifications are %1")
                                      .arg(enabled ? "Enabled" : "Disabled"));
                  }
              }
              break;


          case FinalModelEnableRc:
            if (isPreDisplayPage/*opts.pageNum < displayPageNum*/)
            {
                bool enabled = meta.LPub.finalModelEnabled.value();
                if (Preferences::finalModelEnabled != enabled) {
                    Preferences::finalModelEnabled  = enabled;
                    enableVisualBuildModification();
                    emit messageSig(LOG_INFO, QString("Fade/Highlight final model step is %1")
                                    .arg(enabled ? "Enabled" : "Disabled"));
                }
            }
            break;

            case PageOrientationRc:
              {
                if (exporting()){
                    opts.flags.pageSizeUpdate = true;

                    if (opts.pageSize.sizeW == 0.0f)
                      opts.pageSize.sizeW    = pageSizes[DEF_SIZE].sizeW;
                    if (opts.pageSize.sizeH == 0.0f)
                      opts.pageSize.sizeH    = pageSizes[DEF_SIZE].sizeH;
                    if (opts.pageSize.sizeID.isEmpty())
                      opts.pageSize.sizeID   = pageSizes[DEF_SIZE].sizeID;
                    opts.pageSize.orientation= meta.LPub.page.orientation.value();

                    pageSizes.remove(DEF_SIZE);
                    pageSizes.insert(DEF_SIZE,opts.pageSize);
#ifdef PAGE_SIZE_DEBUG
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
              opts.flags.noStep = true;
              break;
            default:
              break;
            } // Switch Rc
          break;
        }
    } // For Every Line

  csiParts.clear();
  lineTypeIndexes.clear();

  // last step in submodel
  if (! Gui::abortProcess() &&
        opts.flags.partsAdded &&
      ! opts.pageDisplayed &&
      (! opts.flags.noStep || opts.flags.parseNoStep)) {
      isPreDisplayPage = opts.pageNum < displayPageNum;
      isDisplayPage = opts.pageNum == displayPageNum;
      // increment continuous step number
      // save continuous step number from current model
      // pass continuous step number to drawPage
      if (opts.contStepNumber) {
          if (isPreDisplayPage/*opts.pageNum < displayPageNum*/ && ! opts.flags.countInstances &&
             (opts.stepNumber > FIRST_STEP + sa || displayPageNum > FIRST_PAGE + sa)) {
              opts.contStepNumber += ! opts.flags.coverPage && ! opts.flags.stepPage;
          }
          if (isDisplayPage/*opts.pageNum == displayPageNum*/) {
              saveMeta.LPub.contModelStepNum.setValue(saveStepNumber);
              saveStepNumber = opts.contStepNumber;
          }
          saveContStepNum = opts.contStepNumber;
      }
      if (isPreDisplayPage && opts.groupStepNumber &&
          meta.LPub.multiStep.countGroupSteps.value()) { // count group step number and persist
          opts.contStepNumber += ! opts.flags.coverPage && ! opts.flags.stepPage;
          saveGroupStepNum     = opts.contStepNumber;
      }

      if (isDisplayPage/*opts.pageNum == displayPageNum*/) {
          if (opts.groupStepNumber) {                   // pass group step number to drawPage
              saveStepNumber = saveGroupStepNum;
          }
          savePrevStepPosition = saveCsiParts.size();
          lpub->page.meta = saveMeta;
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
                      opts.groupStepNumber,
                      opts.flags.buildModLevel,
                      opts.updateViewer,
                      opts.isMirrored,
                      opts.printing,
                      opts.flags.bfxStore2);
#ifdef WRITE_PARTS_DEBUG
                      writeFindPartsFile("b_find_save_csi_parts");
#endif
          const TraverseRc drc = static_cast<TraverseRc>(drawPage(view, scene, &lpub->page, addLine, pageOptions));
          if (drc == HitBuildModAction || drc == HitCsiAnnotation || drc == HitAbortProcess) {
              // Set processing state and return to init drawPage
              pageProcessRunning = PROC_DISPLAY_PAGE;
              return static_cast<int>(drc);
          }

          opts.pageDisplayed = true;

      } // IsDisplayPage

      if (! opts.flags.noStep) {
          if (exporting()) {
              pageSizes.remove(opts.pageNum);
              if (opts.flags.pageSizeUpdate) {
                  pageSizes.insert(opts.pageNum,opts.pageSize);
#ifdef PAGE_SIZE_DEBUG
                  logTrace() << "PG: Inserting New Page size info     at PageNumber:" << opts.pageNum
                             << "W:"    << opts.pageSize.sizeW << "H:"    << opts.pageSize.sizeH
                             << "O:"    <<(opts.pageSize.orientation == Portrait ? "Portrait" : "Landscape")
                             << "ID:"   << opts.pageSize.sizeID
                             << "Model:" << opts.current.modelName;
#endif
              } else {
                  pageSizes.insert(opts.pageNum,pageSizes[DEF_SIZE]);
#ifdef PAGE_SIZE_DEBUG
                  logTrace() << "PG: Inserting Default Page size info at PageNumber:" << opts.pageNum
                             << "W:"    << pageSizes[DEF_SIZE].sizeW << "H:"    << pageSizes[DEF_SIZE].sizeH
                             << "O:"    << (pageSizes[DEF_SIZE].orientation == Portrait ? "Portrait" : "Landscape")
                             << "ID:"   << pageSizes[DEF_SIZE].sizeID
                             << "Model:" << opts.current.modelName;
#endif
              }
          } // Exporting

#ifdef QT_DEBUG_MODE
          emit messageSig(LOG_NOTICE, QString("FINDPAGE  - Page %1 topOfPage Step, Submodel End (opt) - LineNumber %2, ModelName %3")
                          .arg(opts.pageNum, 3, 10, QChar('0')).arg(opts.current.lineNumber, 3, 10, QChar('0')).arg(opts.current.modelName));
#endif
          ++opts.pageNum;
          ++stepPageNum;
          topOfPages.append(opts.current); // TopOfStep (Next Step), BottomOfStep (Current/Last Step)

          if (opts.current.modelName == topLevelFile()) {
              if (! opts.pageDisplayed) {
                  opts.pageDisplayed = opts.pageNum > displayPageNum;
                  opts.flags.addCountPage = true;
              }
          }
      } // ! opts.flags.noStep (last step in submodel)

      // Clear parts added so we dont count again in countPage;
      opts.flags.partsAdded = 0;
      opts.flags.parseNoStep = false;

    }  // Last Step in Submodel

  // Set processing state
  pageProcessRunning = PROC_DISPLAY_PAGE;
  return Gui::abortProcess() ? static_cast<int>(HitAbortProcess) : static_cast<int>(HitNothing);
}

int Gui::getBOMParts(
          Where   current,
    const QString &addLine)
{
  bool partIgnore   = false;
  bool pliIgnore    = false;
  bool synthBegin   = false;
  bool bfxStore1    = false;
  bool bfxStore2    = false;
  bool bfxLoad      = false;
  bool partsAdded   = false;
  bool excludedPart = false;

  bool buildModIgnore = false;

  QStringList bfxParts;

  skipHeader(current);

  QHash<QString, QStringList> bfx;

  int numLines = lpub->ldrawFile.size(current.modelName);

  Rc rc;

  for ( ;
        current.lineNumber < numLines;
        current.lineNumber++) {

      // scan through the rest of the model counting pages
      // if we've already hit the display page, then do as little as possible

      QString line = lpub->ldrawFile.readLine(current.modelName,current.lineNumber).trimmed();

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

              if (token[1] == LDRAW_MAIN_MATERIAL_COLOUR) {
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

                  if (lpub->ldrawFile.isSubmodel(type)) {

                      Where current2(type,0);

                      getBOMParts(current2,line);

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

                      QString newLine = Pli::partLine(line,current,lpub->page.meta);

                      bomParts << newLine;
                    }
                }

              if (bfxStore1) {
                  bfxParts << colorPart;
                }

              partsAdded = true;
            }
          break;
        case '0':
          rc = lpub->page.meta.parse(line,current);

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
                                            .arg(lpub->page.meta.LPub.pli.begin.sub.value().color)
                                            .arg(lpub->page.meta.LPub.pli.begin.sub.value().part);
                  bomParts << Pli::partLine(addPart,current,lpub->page.meta);
                  pliIgnore = true;
                }
              break;

            case PliBeginIgnRc:
              pliIgnore = true;
              break;

            case PliEndRc:
              pliIgnore = false;
              lpub->page.meta.LPub.pli.begin.sub.clearAttributes();
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
              lpub->page.meta.LPub.bom.pliPartGroup.setWhere(current);
              lpub->page.meta.LPub.bom.pliPartGroup.setBomPart(true);
              bomPartGroups.append(lpub->page.meta.LPub.bom.pliPartGroup);
              break;

              // Any of the metas that can change pliParts needs
              // to be processed here

            case ClearRc:
              bomParts.empty();
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
              buildModIgnore = true;
              break;

            case BuildModEndModRc:
              buildModIgnore = getLevel(QString(), BM_END);
              break;

            case PartNameRc:
            case PartTypeRc:
            case MLCadGroupRc:
            case LDCadGroupRc:
            case LeoCadGroupBeginRc:
            case LeoCadGroupEndRc:
              bomParts << Pli::partLine(line,current,lpub->page.meta);
              break;

              /* remove a group or all instances of a part type */
            case RemoveGroupRc:
            case RemovePartTypeRc:
            case RemovePartNameRc:
              if (! buildModIgnore) {
                  QStringList newBOMParts;
                  QVector<int> dummy;
                  if (rc == RemoveGroupRc) {
                      remove_group(bomParts,dummy,lpub->page.meta.LPub.remove.group.value(),newBOMParts,dummy,&lpub->page.meta);
                  } else if (rc == RemovePartTypeRc) {
                      remove_parttype(bomParts,dummy,lpub->page.meta.LPub.remove.parttype.value(),newBOMParts,dummy);
                  } else {
                      remove_partname(bomParts,dummy,lpub->page.meta.LPub.remove.partname.value(),newBOMParts,dummy);
                  }
                  bomParts = newBOMParts;
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

  skipHeader(current);

  int numLines        = lpub->ldrawFile.size(current.modelName);
  int occurrenceNum   = 0;
  boms                = 0;
  bomOccurrence       = 0;
  Rc rc;

  for ( ; current.lineNumber < numLines;
        current.lineNumber++) {

      QString line = lpub->ldrawFile.readLine(current.modelName,current.lineNumber).trimmed();
      switch (line.toLatin1()[0]) {
          case '1':
          {
              QStringList token;
              split(line,token);
              QString type = token[token.size()-1];

              if (lpub->ldrawFile.isSubmodel(type)) {
                  Where current2(type,0);
                  getBOMOccurrence(current2);
              }
              break;
          }
          case '0':
          {
              rc = lpub->page.meta.parse(line,current);
              switch (rc) {
                  case InsertRc:
                  {
                      InsertData insertData = lpub->page.meta.LPub.insert.value();
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
      for (++here; here.lineNumber < lpub->ldrawFile.size(here.modelName); here++) {
          QString line = readLine(here);
          rc = lpub->page.meta.parse(line,here);
          if (rc == InsertRc) {
              InsertData insertData = lpub->page.meta.LPub.insert.value();
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
    Where current(lpub->ldrawFile.topLevelFile(),0);
    QFuture<void> future = QtConcurrent::run([this, current]() {
        bomParts.clear();
        bomPartGroups.clear();
        getBOMParts(current, QString());
    });
    future.waitForFinished();
    if (! bomParts.size()) {
        emit messageSig(LOG_ERROR,QMessageBox::tr("No BOM parts were detected."));
        return false;
    }

    QStringList tempParts = bomParts;

    bomParts.clear();

    Q_FOREACH (QString bomPartsString, tempParts) {
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
    out << QString("0 Name: %1").arg(QFileInfo(bomFileName).fileName()) << lpub_endl;
    Q_FOREACH (QString bomPart, bomParts)
        out << bomPart << lpub_endl;
    bomFile.close();
    return true;
}

void Gui::attitudeAdjustment()
{
  bool callout = false;
  int numFiles = lpub->ldrawFile.subFileOrder().size();

  for (int i = 0; i < numFiles; i++) {
      QString fileName = lpub->ldrawFile.subFileOrder()[i];

      if (lpub->ldrawFile.isUnofficialPart(fileName))
          continue;

      int numLines     = lpub->ldrawFile.size(fileName);

      QStringList pending;

      for (Where current(fileName,0);
           current.lineNumber < numLines;
           current.lineNumber++) {

          QString line = lpub->ldrawFile.readLine(current.modelName,current.lineNumber);
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
                      lpub->ldrawFile.insertLine(current.modelName,current.lineNumber, pending[i]);
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
                      lpub->ldrawFile.deleteLine(current.modelName,current.lineNumber);
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
  if (maxPages < 1 + pa || buildModJumpForward) {
      pageProcessRunning = PROC_COUNT_PAGE;

      Meta meta;
      QString empty;
      FindPageFlags fpFlags;
      PageSizeData emptyPageSize;
      QList<SubmodelStack> modelStack;

      current              =  Where(lpub->ldrawFile.topLevelFile(),0,0);
      saveDisplayPageNum   =  displayPageNum;
      displayPageNum       =  1 << 24; // really large number: 16777216
      firstStepPageNum     = -1;       // for front cover page
      lastStepPageNum      = -1;       // for back cover page
      maxPages             =  1 + pa;
      stepPageNum          =  1 + pa;

      skipHeader(current);

      // set model start page - used to enable mpd combo to jump to start page
      lpub->ldrawFile.setModelStartPageNumber(current.modelName,maxPages);

      QString message = tr("Counting pages...");
      if (buildModJumpForward) {
          fpFlags.parseBuildMods = true;
          message = tr("BuildMod Next parsing from countPage for jump to page %1...").arg(saveDisplayPageNum);
      }

      emit messageSig(LOG_TRACE, message);

      FindPageOptions opts(
                  maxPages,      /*pageNum*/
                  current,
                  emptyPageSize,
                  fpFlags,
                  modelStack,
                  false          /*pageDisplayed*/,
                  false          /*updateViewer*/,
                  false          /*mirrored*/,
                  false          /*printing*/,
                  0              /*stepNumber*/,
                  0              /*contStepNumber*/,
                  0              /*groupStepNumber*/,
                  empty          /*renderParentModel*/);

      LDrawFile::_currentLevels.clear();

      QFuture<int> future = QtConcurrent::run(CountPageWorker::countPage, &meta, &lpub->ldrawFile, opts);
      future.waitForFinished();
      pagesCounted();
   }
}

void Gui::drawPage(
    LGraphicsView  *view,
    LGraphicsScene *scene,
    DrawPageFlags  &dpFlags)
{
  QApplication::setOverrideCursor(Qt::WaitCursor);

  if (Preferences::modeGUI && ! exporting() && ! ContinuousPage())
    enableNavigationActions(false);

  current      = Where(lpub->ldrawFile.topLevelFile(),0,0);
  saveMaxPages = maxPages;
  maxPages     = 1 + pa;
  stepPageNum  = maxPages;
  lpub->page.relativeType = SingleStepType;

  // set submodels unrendered
  lpub->ldrawFile.unrendered();

  // if not buildMod action
  if (!dpFlags.buildModActionChange && !dpFlags.csiAnnotation) {
    int displayPageIndx  = -1;
    int nextStepIndex    = -1;
    bool firstPage       = true;
    bool adjustTopOfStep = false;
    Where topOfStep      = current;
    lpub->meta           = Meta();
#ifdef QT_DEBUG_MODE
    emit messageSig(LOG_NOTICE, "---------------------------------------------------------------------------");
    emit messageSig(LOG_NOTICE, QString("BEGIN    -  Page %1").arg(displayPageNum));
#endif
    // set next step index and test index is display page index - i.e. refresh a page
    if (Preferences::buildModEnabled) {
      displayPageIndx = exporting() ? displayPageNum : displayPageNum - 1;
      firstPage       = ! topOfPages.size() || topOfPages.size() < displayPageIndx;

      if (!firstPage) {
        if (topOfPages.size() > displayPageIndx) {
            topOfStep     = topOfPages[displayPageIndx];
        } else {
            topOfStep     = topOfPages.takeLast();
        }
      }

      if (!topOfStep.lineNumber)
        skipHeader(topOfStep);

      nextStepIndex = getStepIndex(topOfStep);

      setBuildModNextStepIndex(topOfStep);

      if (!firstPage)
        adjustTopOfStep  = nextStepIndex == getBuildModNextStepIndex();

      lpub->ldrawFile.clearBuildModRendered();

      if (adjustTopOfStep)
        if (! getBuildModStepIndexWhere(nextStepIndex, topOfStep))
          topOfStep = firstPage ? current : topOfPages[displayPageIndx];

      // if page direction is jump forward, reset vars that may be updated by the count page call
      if ((buildModJumpForward = pageDirection == PAGE_JUMP_FORWARD)) {
        const int saveJumpDisplayPageNum = displayPageNum;
        const QList<Where> saveJumpTopOfPages = topOfPages;
        const Where saveJumpCurrent = current;

        if (static_cast<TraverseRc>(setBuildModForNextStep(topOfStep)) == HitAbortProcess)
          return;

        // revert registers to pre jump forward count page settings
        buildModJumpForward = false;
        displayPageNum = saveJumpDisplayPageNum;
        current     = saveJumpCurrent;
        maxPages    = 1 + pa;
        stepPageNum = maxPages;
        topOfPages  = saveJumpTopOfPages;
        lpub->ldrawFile.unrendered();

      } else {
        if (static_cast<TraverseRc>(setBuildModForNextStep(topOfStep)) == HitAbortProcess)
          return;
      }
    } // buildModEnabled

    // populate buildMod registers
    setLoadBuildMods(false); // turn off parsing BuildMods in countInstance call until future update
    lpub->ldrawFile.countInstances();

    // set model start page - used to enable mpd combo to jump to start page
    if (firstPage)
        lpub->ldrawFile.setModelStartPageNumber(current.modelName,maxPages);

  } // not build mod action change
  else if (dpFlags.csiAnnotation) {
      lpub->ldrawFile.countInstances();
  }

  // this call is used primarily by the undo/redo calls when editing BuildMods
  if (!buildModClearStepKey.isEmpty()) {
#ifdef QT_DEBUG_MODE
    emit messageSig(LOG_DEBUG, QString("Reset BuildMod images from step key %1...").arg(buildModClearStepKey));
#endif

    QStringList keys = buildModClearStepKey.split("_");
    QString key      = keys.first();
    QString option   = keys.last();
    // reset key to avoid endless loop - displayPage called in clear... calls
    buildModClearStepKey.clear();

    // viewer step key or clear submodel (cm) - clear step(s) image and flag submodel stack item(s) as modified
    if (keys.size() == 1 || option == "cm") {
      clearWorkingFiles(getPathsFromViewerStepKey(key));

    // clear page
    } else if (option == "cp") {
      bool multiStepPage = isViewerStepMultiStep(key);
      PlacementType relativeType = multiStepPage ? StepGroupType : SingleStepType;
      clearPageCache(relativeType, &lpub->page, Options::CSI);

    // clear step
    } else if (option == "cs") {
      QString csiPngName = getViewerStepImagePath(key);
      clearStepCSICache(csiPngName);
    }
  }

  writeToTmp();

  firstStepPageNum     = -1;
  lastStepPageNum      = -1;
  savePrevStepPosition =  0;
  saveGroupStepNum     =  1 + sa;
  saveContStepNum      =  1 + sa;

  // reset buildModActionChange
  dpFlags.buildModActionChange = false;

  QString empty;
  FindPageFlags fpFlags;
  QList<SubmodelStack> modelStack;

  PageSizeData pageSize;
  if (exporting()) {
    pageSize.sizeW      = lpub->meta.LPub.page.size.valueInches(0);
    pageSize.sizeH      = lpub->meta.LPub.page.size.valueInches(1);
    pageSize.sizeID     = lpub->meta.LPub.page.size.valueSizeID();
    pageSize.orientation= lpub->meta.LPub.page.orientation.value();
    pageSizes.insert(     DEF_SIZE,pageSize);
#ifdef PAGE_SIZE_DEBUG
    logTrace() << "0. Inserting INIT page size info at PageNumber:" << maxPages
               << "W:"  << pageSize.sizeW << "H:"    << pageSize.sizeH
               << "O:"  << (pageSize.orientation == Portrait ? "Portrait" : "Landscape")
               << "ID:" << pageSize.sizeID
               << "Model:" << current.modelName;
#endif
  }

  FindPageOptions opts(
              maxPages,    /*pageNum*/
              current,
              pageSize,
              fpFlags,
              modelStack,
              dpFlags.updateViewer,
              false        /*pageDisplayed*/,
              false        /*mirrored*/,
              dpFlags.printing,
              0            /*stepNumber*/,
              0            /*contStepNumber*/,
              0            /*groupStepNumber*/,
              empty        /*renderParentModel*/);

  const TraverseRc frc = static_cast<TraverseRc>(findPage(view,scene,lpub->meta,empty/*addLine*/,opts));
  if (frc == HitAbortProcess) {

    if (m_exportMode == GENERATE_BOM) {
        emit clearViewerWindowSig();
        m_exportMode = m_saveExportMode;
    }
    pageProcessRunning = PROC_NONE;
    QApplication::restoreOverrideCursor();
    return;

  } else if (Preferences::buildModEnabled && (frc == HitBuildModAction || frc == HitCsiAnnotation)) {

    dpFlags.buildModActionChange = frc == HitBuildModAction;
    dpFlags.csiAnnotation = frc == HitCsiAnnotation;
    pageProcessRunning = PROC_DISPLAY_PAGE;
    clearPage(KpageView,KpageScene);
    QApplication::restoreOverrideCursor();
    drawPage(view,scene,dpFlags);

  } else {

    int modelStackCount = opts.modelStack.size();

    auto countPage = [&] (int modelStackCount)
    {
      QFuture<int> future = QtConcurrent::run(CountPageWorker::countPage, &lpub->meta, &lpub->ldrawFile, opts);
      if (exporting() || ContinuousPage() || countWaitForFinished() || suspendFileDisplay || modelStackCount) {
#ifdef QT_DEBUG_MODE
        if (modelStackCount)
          emit gui->messageSig(LOG_DEBUG, QString(" COUNTING  - WaitForFinsished modelStack.size WAIT YES Stack [%1]").arg(modelStackCount));
        if (countWaitForFinished())
          emit gui->messageSig(LOG_DEBUG, QString(" COUNTING  - WaitForFinsished countWaitForFinished WAIT YES"));
        if (suspendFileDisplay)
          emit gui->messageSig(LOG_DEBUG, QString(" COUNTING  - WaitForFinsished suspendFileDisplay WAIT YES"));
#endif
        future.waitForFinished();
        if (static_cast<TraverseRc>(future.result()) == HitAbortProcess)
            return static_cast<int>(HitAbortProcess);
        if (!modelStackCount)
          pagesCounted();
      } else {
#ifdef QT_DEBUG_MODE
        emit gui->messageSig(LOG_DEBUG, QString(" COUNTING  - FutureWatcher setFuture WAIT NO"));
#endif
        futureWatcher.setFuture(future);
      }
      return static_cast<int>(HitNothing);
    };

#ifdef QT_DEBUG_MODE
    emit gui->messageSig(LOG_DEBUG, QString(" COUNTING  - Submodel Page entry for LineNumber %1, ModelName %2")
                        .arg(opts.current.lineNumber, 3, 10, QChar('0'))
                        .arg(opts.current.modelName));
#endif

    // global meta settings from findPage that went out of scope
    lpub->meta.LPub.countInstance.setValue(opts.flags.countInstances);

    // pass buildMod settings to parent model
    if (Preferences::buildModEnabled && opts.flags.buildModStack.size()) {
      opts.flags.buildMod = opts.flags.buildModStack.last();
//*
#ifdef QT_DEBUG_MODE
      const QString bma [ ] = {"BuildModNoActionRc [0]","BuildModBeginRc [61]","BuildModEndModRc [62]","BuildModEndRc [63]","BuildModApplyRc [64]","BuildModRemoveRc [65]"};
      const QString bms [ ] = {"BM_NONE [-1]","BM_BEGIN [0]","BM_END_MOD [1]","BM_END [2]"};
      emit gui->messageSig(LOG_DEBUG, QString(" COUNTING  - BuildMod flags: "
                                              "No. %1, key: '%2', action: %3, level: %4, state %5, ignore: %6")
                          .arg(opts.flags.buildModStack.size(), 2, 10, QChar('0'))
                          .arg(opts.flags.buildMod.key)
                          .arg(opts.flags.buildMod.action ? bma[opts.flags.buildMod.action - BomEndRc] : bma[BuildModNoActionRc])
                          .arg(opts.flags.buildMod.level, 2, 10, QChar('0'))
                          .arg(opts.flags.buildMod.state > BM_NONE ? bms[opts.flags.buildMod.state + 1] : bms[BM_BEGIN])
                          .arg(opts.flags.buildMod.ignore ? "TRUE [1]" : "FALSE [0]"));
#endif
//*/
      // remove buildMod from where we stopped in the parent model
      opts.flags.buildModStack.pop_back();
    }

    if (static_cast<TraverseRc>(countPage(modelStackCount)) == HitAbortProcess)
        return;

    // if we start counting from a child submodel, load where findPage stopped in the parent model
    for (int i = 0; i < modelStackCount; i++) {
      // set the step number where the submodel will be rendered
      opts.current = Where(opts.modelStack.last().modelName,
                     getSubmodelIndex(opts.modelStack.last().modelName),
                     opts.modelStack.last().lineNumber);

      // set parent model of the submodel being rendered
      opts.renderParentModel = QString(opts.modelStack.last().modelName == topLevelFile() ?
                               QString() : opts.modelStack.last().modelName);

#ifdef QT_DEBUG_MODE
      emit gui->messageSig(LOG_DEBUG, QString(" COUNTING  - Model Stack Submodel Page entry No. %1 for LineNumber %2, ModelName %3")
                          .arg(opts.modelStack.size(), 2, 10, QChar('0'))
                          .arg(opts.current.lineNumber, 3, 10, QChar('0'))
                          .arg(opts.current.modelName));
#endif

      // pass buildMod settings to parent model
      if (Preferences::buildModEnabled && opts.flags.buildModStack.size()) {
        opts.flags.buildMod = opts.flags.buildModStack.last();
//*
#ifdef QT_DEBUG_MODE
        const QString bma [ ] = {"BuildModNoActionRc [0]","BuildModBeginRc [61]","BuildModEndModRc [62]","BuildModEndRc [63]","BuildModApplyRc [64]","BuildModRemoveRc [65]"};
        const QString bms [ ] = {"BM_NONE [-1]","BM_BEGIN [0]","BM_END_MOD [1]","BM_END [2]"};
        emit gui->messageSig(LOG_DEBUG, QString(" COUNTING  - BuildMod flags: "
                                                "No. %1, key: '%2', action: %3, level: %4, state %5, ignore: %6")
                            .arg(opts.flags.buildModStack.size(), 2, 10, QChar('0'))
                            .arg(opts.flags.buildMod.key)
                            .arg(opts.flags.buildMod.action ? bma[opts.flags.buildMod.action - BomEndRc] : bma[BuildModNoActionRc])
                            .arg(opts.flags.buildMod.level, 2, 10, QChar('0'))
                            .arg(opts.flags.buildMod.state > BM_NONE ? bms[opts.flags.buildMod.state + 1] : bms[BM_BEGIN])
                            .arg(opts.flags.buildMod.ignore ? "TRUE [1]" : "FALSE [0]"));
#endif
//*/
        // remove buildMod from where we stopped in the parent model
        opts.flags.buildModStack.pop_back();
      }

      // set flags and increment the parent model lineNumber by 1 if the line is the child submodel
      if (opts.current.lineNumber < lpub->ldrawFile.size(opts.current.modelName)) {
        QString line = lpub->ldrawFile.readLine(opts.current.modelName,opts.current.lineNumber).trimmed();
        QStringList token;
        split(line,token);

        if (token.size() == 15) {
          QString type = token[token.size()-1];

          if (lpub->ldrawFile.isSubmodel(type)) {
            Where walk = opts.current;
            Rc rc = lpub->mi.scanBackward(walk,StepMask|StepGroupMask|CalloutMask);
            opts.flags.stepGroup = (rc == StepGroupBeginRc || rc == StepGroupDividerRc);
            opts.flags.callout   = (rc == CalloutDividerRc || rc == CalloutBeginRc);
            opts.flags.partsAdded++;
            opts.current++;

#ifdef QT_DEBUG_MODE
            emit gui->messageSig(LOG_DEBUG, QString(" COUNTING  - Model Page entry ADJUSTED, PART ADDED for LineNumber %1, ModelName %2, Line [%3]")
                                 .arg(opts.current.lineNumber, 3, 10, QChar('0'))
                                 .arg(opts.current.modelName)
                                 .arg(line));
#endif
          }
        }
      }

      // remove where we stopped in the parent model
      if (opts.modelStack.size())
          opts.modelStack.pop_back();

      // upate the modelstack count
      int stackCount = opts.modelStack.size();

      // let's go
      if (static_cast<TraverseRc>(countPage(stackCount)) == HitAbortProcess)
          return;
    } // iterate the model stack

    if (Preferences::modeGUI && ! exporting() && ! Gui::abortProcess()) {
        bool enable =  m_exportMode != GENERATE_BOM &&
                     (!lpub->page.coverPage || (lpub->page.coverPage &&
                      !lpub->page.meta.LPub.coverPageViewEnabled.value()));
        enable3DActions(enable);
    } // modeGUI and not exporting

    QCoreApplication::processEvents();

    QApplication::restoreOverrideCursor();
  }
}

void Gui::finishedCountingPages()
{
    if (static_cast<TraverseRc>(futureWatcher.result()) == HitAbortProcess)
        return;
    pagesCounted();
}

void Gui::pagesCounted()
{
    topOfPages.append(current);

    if (maxPages > 1)
        maxPages--;

    pageProcessRunning = PROC_NONE;

#ifdef QT_DEBUG_MODE
//*
    emit messageSig(LOG_NOTICE, QString("COUNTED   - Page %1 topOfPage Final Page Finish  (cur) - LineNumber %2, ModelName %3")
                    .arg(maxPages, 3, 10, QChar('0')).arg(current.lineNumber, 3, 10, QChar('0')).arg(current.modelName));
    if (!saveDisplayPageNum) {
        emit messageSig(LOG_NOTICE, "---------------------------------------------------------------------------");
        emit messageSig(LOG_NOTICE, QString("RENDERED -  Page %1 of %2").arg(displayPageNum).arg(maxPages));
        emit messageSig(LOG_NOTICE, "---------------------------------------------------------------------------");
/*
        for (int i = 0; i < topOfPages.size(); i++)
        {
            Where top = topOfPages.at(i);
            emit messageSig(LOG_NOTICE, QString("COUNTED  -  PageIndex: %1, SubmodelIndex: %2: LineNumber: %3, ModelName: %4")
                            .arg(i, 3, 10, QChar('0'))               // index
                            .arg(top.modelIndex, 3, 10, QChar('0'))  // modelIndex
                            .arg(top.lineNumber, 3, 10, QChar('0'))  // lineNumber
                            .arg(top.modelName)); // modelName
        }
//*/
    }
#endif

    if (Preferences::modeGUI && ! exporting()) {
        QString message;
        if (saveDisplayPageNum)
            message = QString("%1 of %2") .arg(saveDisplayPageNum) .arg(saveMaxPages);
        else
            message = QString("%1 of %2") .arg(displayPageNum) .arg(maxPages);

        getAct("setPageLineEditResetAct.1")->setEnabled(false);
        setPageLineEdit->setText(message);
    } // modeGUI and not exporting - countPage and drawPage

    // countPage
    if (saveDisplayPageNum) {
        if (displayPageNum > maxPages)
            displayPageNum = maxPages;
        else
            displayPageNum = saveDisplayPageNum;

        saveDisplayPageNum = 0;

        emit messageSig(LOG_STATUS,QString());
    }
    // drawPage
    else
    {
        if (suspendFileDisplay) {
            if (Preferences::modeGUI && ! exporting()) {
                QFileInfo fileInfo(getCurFile());
                emit messageSig(LOG_INFO_STATUS, lpub->ldrawFile._loadAborted ?
                                    tr("Load LDraw file %1 aborted.").arg(fileInfo.fileName()) :
                                    tr("Loaded LDraw file %1 (%2 pages, %3 parts). %4")
                                       .arg(fileInfo.fileName())
                                       .arg(maxPages)
                                       .arg(lpub->ldrawFile.getPartCount())
                                       .arg(elapsedTime(timer.elapsed())));
                if (!maxPages && !lpub->ldrawFile.getPartCount()) {
                    emit messageSig(LOG_ERROR,tr("LDraw file '%1' is invalid - nothing loaded.")
                                                 .arg(fileInfo.absoluteFilePath()));
                    closeModelFile();
                    if (waitingSpinner->isSpinning())
                        waitingSpinner->stop();
                }
            } // modeGUI and not exporting
        } else if (! ContinuousPage()) {
            emit messageSig(LOG_INFO_STATUS,tr("Page %1 %2. %3.")
                            .arg(exporting() && displayPageNum < maxPages ? displayPageNum + 1 : displayPageNum)
                            .arg(exporting() ? tr("exported") : tr("loaded"))
                            .arg(gui->elapsedTime(timer.elapsed())));
        }

        if (Preferences::modeGUI && ! exporting() && ! Gui::abortProcess()) {
            enableEditActions();
            if (!ContinuousPage())
                enableNavigationActions(true);
            if (m_exportMode == GENERATE_BOM) {
                emit clearViewerWindowSig();
                m_exportMode = m_saveExportMode;
            }
            if (waitingSpinner->isSpinning())
                waitingSpinner->stop();
        } // modeGUI and not exporting
    } // drawPage

    if (suspendFileDisplay) {
        suspendFileDisplay = false;
        enableActions();
    }

    // reset countPage future wait on last drawPage call from export 'printfile' where exporting() is reset to false
    if (!exporting() && countWaitForFinished())
        setCountWaitForFinished(false);

    QApplication::restoreOverrideCursor();
}

int Gui::include(Meta &meta, int &lineNumber, bool &includeFileFound)
{
    Rc rc = OkRc;
    QString filePath = meta.LPub.include.value();
    QFileInfo fileInfo(filePath);
    QString fileName = fileInfo.fileName();

    auto processLine =
            [this,
             &meta,
             &fileName,
             &lineNumber] () {
        Rc prc = InvalidLineRc;
        Where here(fileName,lineNumber);
        QString line = readLine(here);
        switch (line.toLatin1()[0]) {
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
            parseError(QString("Invalid include line [%1].<br>"
                               "Part lines (type 1 to 5) are ignored in include file.").arg(line),here,Preferences::IncludeFileErrors);
            return prc;
        case '0':
            prc = meta.parse(line,here);
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
                                   "Add this command to the model file or to a submodel.").arg(line),here,Preferences::IncludeFileErrors);
                prc = InvalidLineRc;
            default:
                break;
            }
            break;
        }
        return prc;
    };

    if (!includeFileFound) {
        includeFileFound = lpub->ldrawFile.isIncludeFile(fileName);
    }

    if (includeFileFound) {
        int numLines = lpub->ldrawFile.size(fileName);
        for (; lineNumber < numLines; lineNumber++) {
            rc = processLine();
            if (rc != InvalidLineRc)
                break;
        }
        if (lineNumber < numLines)
            lineNumber++;
        else
            rc = EndOfIncludeFileRc;
    } else if (!filePath.isEmpty()) {
        QFile file(filePath);
        if ( ! file.open(QFile::ReadOnly | QFile::Text)) {
            emit messageSig(LOG_ERROR, QString("Cannot read include file %1<br>%2")
                            .arg(filePath)
                            .arg(file.errorString()));
            meta.LPub.include.setValue(QString());
            return static_cast<int>(IncludeFileErrorRc);
        }

        emit messageSig(LOG_TRACE, QString("Loading include file '%1'...").arg(filePath));

        QTextStream in(&file);
        in.setCodec(lpub->ldrawFile._currFileIsUTF8 ? QTextCodec::codecForName("UTF-8") : QTextCodec::codecForName("System"));

        /* Read it in to put into subFiles in order of appearance */
        QStringList contents;
        while ( ! in.atEnd()) {
            QString line = in.readLine(0);
            if (!line.isEmpty())
                contents << line.trimmed();
        }
        file.close();

        disableWatcher();
        QDateTime datetime = fileInfo.lastModified();
        lpub->ldrawFile.insert(fileName,
                               contents,
                               datetime,
                               UNOFFICIAL_OTHER,
                               true/*generated*/,
                               true/*includeFile*/,
                               fileInfo.absoluteFilePath(),
                               QFileInfo(fileName).completeBaseName());

        int comboIndex = mpdCombo->count() - 1;
        if (lpub->ldrawFile.includeFileList().size() == 1) {
            mpdCombo->addSeparator();
            comboIndex++;
        }
        mpdCombo->addItem(QString("%1 - Include File").arg(fileName),fileName);
        comboIndex++;
        bool dark = Preferences::displayTheme == THEME_DARK;
        mpdCombo->setItemData(comboIndex, QBrush(dark ? Qt::cyan : Qt::blue), Qt::TextColorRole);
        enableWatcher();

        emit messageSig(LOG_TRACE, QString("Include file '%1' with %2 lines loaded.").arg(fileName).arg(contents.size()));

        rc = Rc(include(meta,lineNumber,includeFileFound));
    } else {
        rc = IncludeFileErrorRc;
        meta.LPub.include.setValue(QString());
    }
    return static_cast<int>(rc);
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
 * When BUILD_MOD END is detected, the BuildMod item is inserted into the BuildMod list in ldrawFile.
 * The buildModLevel flag uses the getLevel() function to determine the current BuildMod when mods are nested.
 * At this stage, the BuildMod action is set to BuildModApply by default.
 *
 * 2. The BuildMod action for the 'next' step being configured by this instance of writeToTmp() is updated.
 * The action update proceedes as follows: (a.) The index for the 'next' step is captured in buildModStepIndex
 * and used to correctly identify the BuldMod action slot. (b.) The corresponding BuildMod action is determined
 * and subsequently updated when BUILD_MOD APPLY or BUILD_MOD REMOVE meta commands are encountered. These
 * commands must include their respective buildModKey
 *
 * 3. Part lines are written to buildModCsiParts, their corresponding index is added to buildModLineTypeIndexes
 * and buildModItems (bool) is set to the number of mod lines. The buildModCsiParts are added to csiParts and
 * buildModLineTypeIndexes are added to lineTypeIndexes when buildModLevel is BM_BASE_LEVEL [0].
 */

int Gui::setBuildModForNextStep(
        const Where topOfNextStep,
        Where topOfSubmodel)
{
    int  buildModNextStepIndex = getBuildModNextStepIndex(); // set next/'display' step index
    int buildModStepIndex      = BM_FIRST_INDEX;
    int startLine              = BM_BEGIN;
    int partsAdded             = 0;
    bool bottomOfStep          = false;
    bool submodel              = topOfSubmodel.modelIndex > 0 && topOfSubmodel.modelIndex != topOfNextStep.modelIndex;
    QString startModel         = topOfNextStep.modelName;
    Where topOfStep            = topOfNextStep;

    BuildModFlags           buildMod;
    QMap<int, QString>      buildModKeys;
    QMap<int, QVector<int>> buildModAttributes;

    auto insertAttribute =
            [&buildMod,
             &topOfStep] (
            QMap<int, QVector<int>> &buildModAttributes,
            int index, const Where &here)
    {
        QMap<int, QVector<int>>::iterator i = buildModAttributes.find(buildMod.level);
        if (i == buildModAttributes.end()) {
            QVector<int> modAttributes = { 0, 0, 0, 1, 0, topOfStep.modelIndex, 0, 0 };
            modAttributes[index] = here.lineNumber;
            buildModAttributes.insert(buildMod.level, modAttributes);
        } else {
            i.value()[index] = here.lineNumber;
        }
    };

    auto insertBuildModification =
           [this,
            &partsAdded,
            &buildModAttributes,
            &buildModKeys,
            &topOfStep] (int buildModLevel)
    {
        int buildModStepIndex = getBuildModStepIndex(topOfStep);
        QString buildModKey = buildModKeys.value(buildModLevel);
        QVector<int> modAttributes = { 0, 0, 0, 0, 0, -1, 0, 0 };

        QMap<int, QVector<int>>::iterator i = buildModAttributes.find(buildModLevel);
        if (i == buildModAttributes.end()) {
            emit gui->messageSig(LOG_ERROR, QString("Invalid BuildMod Entry for key: %1").arg(buildModKey));
            return;
        }
        modAttributes = i.value();

        modAttributes[BM_DISPLAY_PAGE_NUM] = displayPageNum;
        modAttributes[BM_STEP_PIECES]      = partsAdded;
        modAttributes[BM_MODEL_NAME_INDEX] = topOfStep.modelIndex;
        modAttributes[BM_MODEL_LINE_NUM]   = topOfStep.lineNumber;

#ifdef QT_DEBUG_MODE
      emit gui->messageSig(LOG_DEBUG, QString(
                           "Insert NextStep BuildMod StepIndex: %1, "
                           "Action: Apply(64), "
                           "Attributes: %2 %3 %4 %5 %6* %7 %8 %9*, "
                           "ModKey: '%10', "
                           "Level: %11, "
                           "Model: %12")
                           .arg(buildModStepIndex)                      // Attribute Default Initial:
                           .arg(modAttributes.at(BM_BEGIN_LINE_NUM))    // 0         0       this
                           .arg(modAttributes.at(BM_ACTION_LINE_NUM))   // 1         0       this
                           .arg(modAttributes.at(BM_END_LINE_NUM))      // 2         0       this
                           .arg(modAttributes.at(BM_DISPLAY_PAGE_NUM))  // 3         0       this
                           .arg(modAttributes.at(BM_STEP_PIECES))       // 4         0       drawPage
                           .arg(modAttributes.at(BM_MODEL_NAME_INDEX))  // 5        -1       this
                           .arg(modAttributes.at(BM_MODEL_LINE_NUM))    // 6         0       this
                           .arg(modAttributes.at(BM_MODEL_STEP_NUM))    // 7         0       drawPage
                           .arg(buildModKey)
                           .arg(buildModLevel)
                           .arg(topOfStep.modelName));
#endif

        insertBuildMod(buildModKey,
                       modAttributes,
                       buildModStepIndex);
    };

    // get start index when navigating backwards - check step for first submodel that contan a build modification
    std::function<int(const Where &, int, bool)> getStartIndex;
    getStartIndex = [&](const Where &top, int startIndex, bool submodel) -> int {
        int index = submodel ? startIndex : startIndex + 1;
        Where walk(top);
        if (!walk.lineNumber)
            skipHeader(walk);  // advance past headers
        else
            walk++;            // Advance past STEP meta
        Rc rc;
        QStringList token;
        int numLines = subFileSize(walk.modelName);
        for ( ;walk.lineNumber < numLines && !Gui::abortProcess(); walk.lineNumber++) {
            QString line = gui->readLine(walk);
            switch (line.toLatin1()[0]) {
            case '1':
                split(line,token);
                if (token.size() == 15) {
                    const QString modelName = token[token.size() - 1];
                    if (isSubmodel(modelName)) {
                        Where topOfSubmodel(modelName, getSubmodelIndex(modelName), 0);
                        index = getStartIndex(topOfSubmodel, index, true);
                    }
                }
            case '0':
                rc = lpub->page.meta.parse(line,walk);
                switch (rc) {
                case BuildModBeginRc:
                case BuildModApplyRc:
                case BuildModRemoveRc:
                    if (submodel)                   // we enountered a  build modification in the step submodel so increment and return the index
                        return index + 1;
                    break;
                case RotStepRc:
                case StepRc:                        // at the end of the current/submodel step so return the index
                    return index;
                default:
                    break;
                }
            }
        }
        return index;                               // return the index
    };

    // we do this submodel->else block because this call only processes to the end of the specified next step
    if (submodel) {
        if (!topOfSubmodel.lineNumber)
            skipHeader(topOfSubmodel);              // advance past headers

        startLine  = topOfSubmodel.lineNumber;
        startModel = topOfSubmodel.modelName;
        topOfStep  = topOfSubmodel;

#ifdef QT_DEBUG_MODE
        emit gui->messageSig(LOG_NOTICE, QString("Build Modification Next Step Check - Index: %1, Submodel: '%2'...")
                                                 .arg(buildModNextStepIndex).arg(topOfSubmodel.modelName));
#endif

    } else {
        //* local ldrawFile used for debugging
#ifdef QT_DEBUG_MODE
        LDrawFile *ldrawFile = &lpub->ldrawFile;
        Q_UNUSED(ldrawFile)
#endif
        //*/
        emit gui->messageSig(LOG_INFO_STATUS, QString("Build Modification Next Step Check - Index: %1, Model: '%2', Line '%3'...")
                                                      .arg(buildModNextStepIndex).arg(topOfStep.modelName).arg(topOfStep.lineNumber));

        startLine = topOfStep.lineNumber;           // set starting line number

        int startIndex = buildModNextStepIndex;     // when navigating forward, set the delete index to the next step index
        if (pageDirection > PAGE_JUMP_FORWARD)
            startIndex =
               getStartIndex(topOfStep,startIndex,false); // when navigating backward, set the delete index to the index after the next step index

        deleteBuildMods(startIndex);                // clear all build mods at and after delete index - used after jump ahead and for backward navigation


#ifdef QT_DEBUG_MODE
        emit gui->messageSig(LOG_TRACE, QString("BuildMod Next StartStep - Index: %1, ModelName: %2, LineNumber: %3")
                                                .arg(buildModNextStepIndex).arg(startModel).arg(startLine));
#endif

        if (buildModJumpForward)                    // jump forward by more than one step/page
            countPages();                           // set build mods in countPages call
        else if (pageDirection != PAGE_FORWARD)     // jump backward by one or more steps/pages
            setBuildModNavBackward();               // set build mod last action for mods up to next step index

        if (pageDirection != PAGE_FORWARD) {
#ifdef QT_DEBUG_MODE
            emit gui->messageSig(LOG_TRACE, QString("BuildMod Next Jump %1 - Amount: %2 (Steps), StartModel: %3, "
                                                    "StartLine: %4, ModelName: %5, LineNumber: %6")
                                                    .arg(buildModJumpForward ? "Forward" : "Backward")
                                                    .arg(qAbs(buildModNextStepIndex - getBuildModPrevStepIndex()))
                                                    .arg(startModel).arg(startLine)
                                                    .arg(topOfNextStep.modelName)
                                                    .arg(topOfNextStep.lineNumber));
#endif
            return HitBottomOfStep;
        }
    }

    Where walk(startModel, getSubmodelIndex(startModel), startLine);
    int numLines = subFileSize(walk.modelName);

    // Check if we are at the end of the current submodel
    int hitEndOfSubmodel = 0;
    while (walk.lineNumber == numLines && walk.modelName != topLevelFile()) {
        int nextStepIndex = getStepIndex(walk);
#ifdef QT_DEBUG_MODE
        emit gui->messageSig(LOG_DEBUG, QString("Skip BuildMod Next StepIndex %1, hit end of submodel at ModelName: %2, LineNumber: %3.")
                           .arg(nextStepIndex).arg(walk.modelName).arg(walk.lineNumber));
#endif
        if (hitEndOfSubmodel > 1)
            setBuildModNextStepIndex(walk);
        if (getBuildModStepIndexWhere(++nextStepIndex, walk)) {
            hitEndOfSubmodel++;
            numLines = subFileSize(walk.modelName);
        }
    }

    if (hitEndOfSubmodel) {
#ifdef QT_DEBUG_MODE
        emit gui->messageSig(LOG_TRACE, QString("BuildMod Next EndStep - Index: %1, ModelName: %2, LineNumber: %3")
                             .arg(buildModNextStepIndex)
                             .arg(walk.modelName)
                             .arg(walk.lineNumber));
#endif
        return HitEndOfSubmodel;
    }

    if (!submodel)
        LDrawFile::_currentLevels.clear();

    TraverseRc returnValue = HitNothing;
    QString line = readLine(walk);
    Rc rc = lpub->page.meta.parse(line, walk, false);
    if (rc == StepRc || rc == RotStepRc)
        walk++;   // Advance past STEP meta

    // Parse the step lines
    for ( ;
          walk.lineNumber < numLines && !bottomOfStep && !Gui::abortProcess();
          walk.lineNumber++) {

       line = readLine(walk);

       switch (line.toLatin1()[0]) {
       case '0':

            rc =  lpub->page.meta.parse(line,walk,false);

            switch (rc) {

            // Populate BuildMod action and begin, mod_end and end line numbers for 'current' step
            case BuildModApplyRc:
            case BuildModRemoveRc:
                buildModStepIndex = getBuildModStepIndex(topOfStep);
                buildMod.key = lpub->page.meta.LPub.buildMod.key();
                if (buildModContains(buildMod.key)) {
                    buildMod.action = getBuildModAction(buildMod.key, buildModStepIndex);
                } else {
                    const QString action = rc == BuildModApplyRc ? tr("Apply") : tr("Remove");
                    gui->parseErrorSig(QString("Next Step %1 BuildMod for key '%2' not found.")
                                               .arg(action).arg(buildMod.key),
                                               walk,Preferences::ParseErrors,false,false);
                }
                if ((Rc)buildMod.action != rc) {
                    setBuildModAction(buildMod.key, buildModStepIndex, rc);
                }
                buildMod.state = BM_NONE;
                break;

            // Get BuildMod attributes and set buildModIgnore based on 'next' step buildModAction
            case BuildModBeginRc:
                if (buildMod.state == BM_BEGIN)
                    emit gui->parseErrorSig(QString("BUILD_MOD BEGIN '%1' encountered but '%2' was already defined in this STEP.<br><br>"
                                                    "Multiple build modifications per STEP are not allowed.")
                                                    .arg(lpub->page.meta.LPub.buildMod.key()).arg(buildMod.key),
                                                    walk,Preferences::BuildModErrors,false,false);
                buildMod.key   = lpub->page.meta.LPub.buildMod.key();
                buildMod.level = getLevel(buildMod.key, BM_BEGIN);
                buildModKeys.insert(buildMod.level, buildMod.key);
                insertAttribute(buildModAttributes, BM_BEGIN_LINE_NUM, walk);
                buildMod.state = BM_BEGIN;
                break;

            // Set modActionLineNum and buildModIgnore based on 'next' step buildModAction
            case BuildModEndModRc:
                if (buildMod.level > BM_FIRST_LEVEL && buildMod.key.isEmpty())
                    emit gui->parseErrorSig(QString("Key required for nested build mod meta command"),
                                                    walk,Preferences::BuildModErrors,false,false);
                if (buildMod.state != BM_BEGIN)
                    emit gui->parseErrorSig(QString("Required meta BUILD_MOD BEGIN for key '%1' not found")
                                                    .arg(buildMod.key), walk, Preferences::BuildModErrors,false,false);
                insertAttribute(buildModAttributes, BM_ACTION_LINE_NUM, walk);
                buildMod.state = BM_END_MOD;
                break;

            // Insert buildModAttributes and reset buildMod.level and buildModIgnore to default
            case BuildModEndRc:
                if (buildMod.state != BM_END_MOD)
                    emit gui->parseErrorSig(QString("Required meta BUILD_MOD END_MOD for key '%1' not found")
                                                   .arg(buildMod.key), walk, Preferences::BuildModErrors,false,false);
                insertAttribute(buildModAttributes, BM_END_LINE_NUM, walk);
                buildMod.level = getLevel(QString(), BM_END);
                buildMod.state = BM_END;
                break;

            // Search until next occurrence of step/rotstep meta or bottom of step
            // We only need the next STEP build mod even if the next step is in a
            // callout or step group UNLESS, we have callout(s) - i.e. submodel(s)
            // before a build modification in the same STEP in which case we must
            // register the build modification to avoid a 'not registered' error
            // when we get to the drawPage call - basically, we must process any
            // build modifications in the root STEP.
            case RotStepRc:
            case StepRc:
                if (buildModKeys.size()) {
                    if (buildMod.state != BM_END)
                        emit gui->parseErrorSig(QString("Required meta BUILD_MOD END not found"),
                                                        walk, Preferences::BuildModErrors,false,false);
                    Q_FOREACH (int buildModLevel, buildModKeys.keys())
                        insertBuildModification(buildModLevel);
                }
                bottomOfStep = partsAdded;
                topOfStep = walk;
                buildModKeys.clear();
                buildModAttributes.clear();
                buildMod.clear();
                partsAdded = 0;
                break;
            default:
                break;
            }
            break;

        case '1':
            if (buildMod.state < BM_END_MOD) {
                QStringList token;
                split(line,token);
                if (token.size() == 15) {
                    const QString modelName = token[token.size() - 1];
                    if (isSubmodel(modelName)) {
                        Where topOfSubmodel(modelName, getSubmodelIndex(modelName), 0);
                        const TraverseRc nrc = static_cast<TraverseRc>(setBuildModForNextStep(topOfNextStep, topOfSubmodel));
                        if (nrc == HitAbortProcess) {
                            return static_cast<int>(HitAbortProcess);
                        } else {
                            bool buildModFound = false;
                            if (walk.modelIndex == topOfNextStep.modelIndex) {
                                QRegExp buildModBeginRx("^0 !LPUB BUILD_MOD BEGIN ");
                                if ((buildModFound = Gui::stepContains(walk, buildModBeginRx)))
                                    walk--; // Adjust for line increment
                            }
                            if (!buildModFound) {
                                bottomOfStep = nrc == HitBottomOfStep || nrc == HitEndOfFile;
                            }
                        }
                    }
                }
            }
        case '2':
        case '3':
        case '4':
        case '5':
            partsAdded++;
            break;
        } // Process meta-command
    } // For every line

    if (Gui::abortProcess()) {
        returnValue = HitAbortProcess;
    } else {
        // Last step of submodel
        if (buildModKeys.size()) {
            Q_FOREACH (int buildModLevel, buildModKeys.keys())
                insertBuildModification(buildModLevel);
        }

        if (Gui::abortProcess()) {
            returnValue = HitAbortProcess;
        } else if (bottomOfStep) {
#ifdef QT_DEBUG_MODE
            emit gui->messageSig(LOG_TRACE, QString("BuildMod Next EndStep - Index: %1, ModelName: %2, LineNumber: %3")
                                                    .arg(buildModNextStepIndex)
                                                    .arg(walk.modelName)
                                                    .arg(walk.lineNumber));
#endif
            returnValue = HitBottomOfStep;
        } else {
            returnValue = HitEndOfFile;
        }
    }

    return static_cast<int>(returnValue);
} // Gui::setBuildModForNextStep()

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
  QMutexLocker writeLocker(&writeMutex);

  QString filePath = QDir::toNativeSeparators(QDir::currentPath()) + QDir::separator() + Paths::tmpDir + QDir::separator() + fileName;
  QFileInfo fileInfo(filePath);
  if(!fileInfo.dir().exists()) {
     fileInfo.dir().mkpath(".");
    }
  QFile file(filePath);
  if ( ! file.open(QFile::WriteOnly|QFile::Text)) {
      QMessageBox::critical(nullptr,QMessageBox::tr("%1 Write To Temp").arg(VER_PRODUCTNAME_STR),
                           QMessageBox::tr("Failed to open %1 for writing: %2")
                           .arg(filePath) .arg(file.errorString()));
      return;
    } else {

      LDrawFile::_currentLevels.clear();

      Where topOfStep(fileName, getSubmodelIndex(fileName), 0);

      skipHeader(topOfStep);

      int  buildModBottom     = 0;
      int  buildModLevel      = BM_BASE_LEVEL; 
      bool buildModIgnore     = false;
      bool buildModItems      = false;
      bool buildModApplicable = false;

      QString                 buildModKey;
      QMap<int, int>          buildModActions;

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
      for (int i = 0; i < contents.size() && !Gui::abortProcess(); i++) {
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
                          buildModIgnore = true;
                          break;
                      }
                      buildModBottom    = getBuildModStepLineNumber(getBuildModStepIndex(topOfStep));
                      if ((buildModApplicable = i < buildModBottom)) {
                          buildModKey   = meta.LPub.buildMod.key();
                          buildModLevel = getLevel(buildModKey, BM_BEGIN);
                          // insert 'BuildModRemoveRc' if key not yet created (buildModIgnore = true)
                          if (! buildModContains(buildModKey))
                              buildModActions.insert(buildModLevel, BuildModRemoveRc);
                          else
                              buildModActions.insert(buildModLevel, getBuildModAction(buildModKey, getBuildModNextStepIndex()));
                          if (buildModActions.value(buildModLevel) == BuildModApplyRc)
                              buildModIgnore = false;
                          else if (buildModActions.value(buildModLevel) == BuildModRemoveRc)
                              buildModIgnore = true;
                      }
                      break;

                  // Set modActionLineNum and buildModIgnore based on 'next' step buildModAction
                  case BuildModEndModRc:
                      if (!Preferences::buildModEnabled)
                          break;
                      if (buildModApplicable) {
                          if (buildModLevel > 1 && meta.LPub.buildMod.key().isEmpty())
                              emit gui->parseErrorSig("Key required for nested build mod meta command",
                                                 here,Preferences::BuildModErrors,false/*option*/,false/*override*/);
                          if (buildModActions.value(buildModLevel) == BuildModApplyRc)
                              buildModIgnore = true;
                          else if (buildModActions.value(buildModLevel) == BuildModRemoveRc)
                              buildModIgnore = false;
                      }
                      break;

                  // Insert buildModAttributes and reset buildModLevel and buildModIgnore to default
                  case BuildModEndRc:
                      if (!Preferences::buildModEnabled)
                          break;
                      if (buildModApplicable) {
                          buildModLevel      = getLevel(QString(), BM_END);
                          if (buildModLevel == BM_BASE_LEVEL) {
                              buildModIgnore     = false;
                              buildModApplicable = false;
                          }
                      }
                      break;

                  case RotStepRc:
                  case StepRc:
                      buildModApplicable = false;
                      topOfStep.lineNumber = i;
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
                              remove_group(csiParts,lineTypeIndexes,meta.LPub.remove.group.value(),newCSIParts,newLineTypeIndexes,&meta);
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

      lpub->ldrawFile.setLineTypeRelativeIndexes(topOfStep.modelIndex,lineTypeIndexes);

      QTextStream out(&file);
      for (int i = 0; i < csiParts.size(); i++) {
          out << csiParts[i] << lpub_endl;
        }
      file.close();
    }
}

void Gui::writeToTmp()
{
  pageProcessRunning = PROC_WRITE_TO_TMP;
  QList<QFuture<void>> writeToTmpFutures;
  QElapsedTimer writeToTmpTimer;
  writeToTmpTimer.start();

  int writtenFiles = 0;
  int subFileCount = lpub->ldrawFile._subFileOrder.size();
  bool doFadeStep  = (Preferences::enableFadeSteps || lpub->page.meta.LPub.fadeSteps.setup.value());
  bool doHighlightStep = (Preferences::enableHighlightStep || lpub->page.meta.LPub.highlightStep.setup.value()) && !suppressColourMeta();

  QString fadeColor = LDrawColor::ldColorCode(lpub->page.meta.LPub.fadeSteps.color.value().color);

  QString message;
  QString fileName;
  QString fileType;
  QStringList fileContent;

  std::function<QStringList()> getFileContent;
  getFileContent = [&fileName] ()
  {
      QFile file(fileName);
      if (!file.open(QFile::ReadOnly | QFile::Text)) {
          emit gui->messageSig(LOG_ERROR, QString("Cannot read external file %1<br>%2")
                               .arg(fileName)
                               .arg(file.errorString()));
          return QStringList();
      }

      QStringList externalFileContents;
      QTextStream in(&file);
      while ( ! in.atEnd()) {
          QString sLine = in.readLine(0);
          externalFileContents << sLine.trimmed();
      }
      file.close();
      return externalFileContents;
  };

  if (!ContinuousPage()) {
      emit progressBarPermInitSig();
      emit progressPermRangeSig(1, subFileCount);
  }

  for (int i = 0; i < subFileCount && !Gui::abortProcess(); i++) {

      fileName = lpub->ldrawFile._subFileOrder[i].toLower();

      // write normal submodels...
      if (lpub->ldrawFile.changedSinceLastWrite(fileName)) {

          writtenFiles++;

          int numberOfLines = lpub->ldrawFile.size(fileName);

          QString sourceFilePath = QDir::toNativeSeparators(lpub->ldrawFile.getSubFilePath(fileName));

          bool externalFile = !sourceFilePath.isEmpty();

          fileType = externalFile ? "external" : "";
          fileType += lpub->ldrawFile.isUnofficialPart(fileName) ? "unofficial part" : "submodel";

          message = QString("Writing %1 '%2' to temp folder...").arg(fileType).arg(fileName);

          emit messageSig(LOG_INFO_STATUS, message);

          if (Gui::suspendFileDisplay) {
              message = QString("Writing %1 %2 of %3 files (%4 lines)...")
                      .arg(fileType)
                      .arg(QStringLiteral("%1").arg(i + 1, 3, 10, QLatin1Char('0')))
                      .arg(QStringLiteral("%1").arg(subFileCount, 3, 10, QLatin1Char('0')))
                      .arg(QStringLiteral("%1").arg(numberOfLines, 5, 10, QLatin1Char('0')));
          } else {
              message = QString("Writing %1 %2 (%3 lines)").arg(fileType).arg(fileName).arg(numberOfLines);
          }

          if (!ContinuousPage()) {
              emit progressPermMessageSig(message);
              emit progressPermSetValueSig(i + 1);
          }

          if (externalFile) {
             QString destinationPath = QDir::toNativeSeparators(QDir::currentPath()) + QDir::separator() + Paths::tmpDir + QDir::separator() + fileName;
             if (QFile::exists(destinationPath)) {
                 QFile::remove(destinationPath);
             }
             if(!QFile::copy(sourceFilePath, destinationPath)) {
                 emit messageSig(LOG_ERROR, QString("Could not write external file '%1' to temp folder...").arg(fileName));
             }
          } else {
              writeToTmpFutures.append(QtConcurrent::run([this, fileName]() {
                  QStringList *modelContent = new QStringList;
                  modelContent->append(lpub->ldrawFile.contents(fileName));
                  writeSmiContent(modelContent, fileName);
                  writeToTmp(fileName, *modelContent);
              }));
          }

          QString fileNameStr = fileName;
          QString extension = QFileInfo(fileNameStr).suffix().toLower();

          // Write faded version of submodels
          if (externalFile && (doFadeStep || doHighlightStep)) {
              fileContent.clear();
              fileContent = getFileContent();
          }

          if (doFadeStep) {
            if (extension.isEmpty())
              fileNameStr = fileNameStr.append(QString("%1.ldr").arg(FADE_SFX));
            else
              fileNameStr = fileNameStr.replace("."+extension, QString("%1.%2").arg(FADE_SFX).arg(extension));

            emit messageSig(LOG_INFO_STATUS, QString("Writing %1 '%2' to temp folder...").arg(fileType).arg(fileNameStr));

            writeToTmpFutures.append(QtConcurrent::run([this, fileName, fileNameStr, fadeColor, externalFile, fileContent]() {
                QStringList *modelContent = new QStringList;
                externalFile ? modelContent->append(fileContent) : modelContent->append(lpub->ldrawFile.contents(fileName));
                QStringList *configuredContent = new QStringList;
                configuredContent->append(configureModelSubFile(*modelContent, fadeColor, FADE_PART));
                insertConfiguredSubFile(fileNameStr, *configuredContent);
                writeToTmp(fileNameStr, *configuredContent);
            }));
          }

          // Write highlighted version of submodels
          if (doHighlightStep) {
            if (extension.isEmpty())
              fileNameStr = fileNameStr.append(QString("%1.ldr").arg(HIGHLIGHT_SFX));
            else
              fileNameStr = fileNameStr.replace("."+extension, QString("%1.%2").arg(HIGHLIGHT_SFX).arg(extension));

            emit messageSig(LOG_INFO_STATUS, QString("Writing %1 '%2' to temp folder...").arg(fileType).arg(fileNameStr));

            writeToTmpFutures.append(QtConcurrent::run([this, fileName, fileNameStr, fadeColor, externalFile, fileContent]() {
                QStringList *modelContent = new QStringList;
                externalFile ? modelContent->append(fileContent) : modelContent->append(lpub->ldrawFile.contents(fileName));
                QStringList *configuredContent = new QStringList;
                configuredContent->append(configureModelSubFile(*modelContent, fadeColor, HIGHLIGHT_PART));
                insertConfiguredSubFile(fileNameStr, *configuredContent);
                writeToTmp(fileNameStr, *configuredContent);
            }));
          }

          // Write children on Buld Modification jump forward - deprecated. Setting submodels in stack to 'modified'at setBuildModAction
          //if (Preferences::buildModEnabled && pageDirection != PAGE_NEXT && pageDirection < PAGE_BACKWARD) {
          //    Q_FOREACH (int modelIndex, lpub->ldrawFile.getSubmodelIndexes(fileName)) {
          //        const QString modelFile = lpub->ldrawFile.getSubmodelName(modelIndex);
          //        emit messageSig(LOG_TRACE, QString("Writing submodel '%1' subfile '%2' to temp folder...").arg(fileName).arg(modelFile));
          //        writtenFiles++;
          //        writeToTmpFutures.append(QtConcurrent::run([this, modelFile]() {
          //            QStringList *modelContent = new QStringList;
          //            modelContent->append(lpub->ldrawFile.contents(modelFile));
          //            writeToTmp(modelFile, *modelContent);
          //        }));
          //    }
          //} // BuildMod jump forward
      } // ChangedSinceLastWrite
  } // Parse _subFileOrder

  for (QFuture<void>& Future : writeToTmpFutures)
      Future.waitForFinished();

  writeToTmpFutures.clear();

  if (Preferences::modeGUI && !exporting() && !ContinuousPage()) {
      if (lcGetPreferences().mViewPieceIcons && !submodelIconsLoaded) {
          // complete previous progress
          emit progressPermSetValueSig(subFileCount);

          // generate submodel icons...
          emit messageSig(LOG_INFO_STATUS, "Creating submodel icons...");
          Pli pli;
          if (pli.createSubModelIcons() == 0)
              SetSubmodelIconsLoaded(submodelIconsLoaded = true);
          else
              emit messageSig(LOG_ERROR, "Could not create submodel icons...");
          emit progressPermStatusRemoveSig();
      } else {
          // complete and close progress
          emit progressPermSetValueSig(subFileCount);
          emit progressPermStatusRemoveSig();
      }
  }
  QString writeToTmpElapsedTime = elapsedTime(writeToTmpTimer.elapsed());
  emit gui->messageSig(LOG_INFO_STATUS, QString("%1 %2 written to temp folder. %3")
                                        .arg(writtenFiles ? QString::number(writtenFiles) : "No")
                                        .arg(writtenFiles == 1 ? "file" : "files")
                                        .arg(writtenFiles ? writeToTmpElapsedTime : QString()));
  pageProcessRunning = PROC_NONE;
}

void Gui::writeSmiContent(QStringList *content, const QString &fileName)
{
    QMutexLocker writeLocker(&writeMutex);
        
    if (! Preferences::buildModEnabled)
        return;

    QStringList smiContent;

    Where topOfStep(fileName, 0);

    gui->skipHeader(topOfStep);

    bool partIgnore         = false;
    bool buildModIgnore     = false;

    QHash<QString, QStringList> bfx;

    Rc    rc;
    Meta  meta;

    for (int i = 0; i < content->size(); i++) {
        QString line = content->at(i);
        QStringList tokens;
        split(line,tokens);
        if (tokens.size()) {
            if (tokens[0] != "0") {
                if (! buildModIgnore && ! partIgnore)
                    smiContent << line;
            } else {
                Where here(fileName,i);
                rc =  meta.parse(line,here,false);

                switch (rc) {
                /* buffer exchange */
                case BufferStoreRc:
                    bfx[meta.bfx.value()] = smiContent;
                    break;

                case BufferLoadRc:
                    smiContent = bfx[meta.bfx.value()];
                    break;

                    /* get BuildMod attributes and set buildModIgnore based on 'next' step buildModAction */
                case BuildModBeginRc:
                    buildModIgnore = true;
                    break;

                    /* set modActionLineNum and buildModIgnore based on 'next' step buildModAction */
                case BuildModEndModRc:
                    if (getLevel(QString(), BM_END) == BM_BEGIN)
                        buildModIgnore = false;
                    break;

                case PartBeginIgnRc:
                    partIgnore = true;
                    break;

                case PartEndRc:
                    partIgnore = false;
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
                    smiContent << line;
                    break;

                    /* remove a group or all instances of a part type */
                case RemoveGroupRc:
                case RemovePartTypeRc:
                case RemovePartNameRc:
                    if (! buildModIgnore) {
                        QStringList newSmiContent;
                        QVector<int> dummy;
                        if (rc == RemoveGroupRc) {
                            gui->remove_group(smiContent,dummy,meta.LPub.remove.group.value(),newSmiContent,dummy,&meta);
                        } else if (rc == RemovePartTypeRc) {
                            gui->remove_parttype(smiContent,dummy,meta.LPub.remove.parttype.value(),newSmiContent,dummy);
                        } else {
                            gui->remove_partname(smiContent,dummy,meta.LPub.remove.partname.value(),newSmiContent,dummy);
                        }
                        smiContent = newSmiContent;
                    }
                    break;

                default:
                    break;
                }
            }
        }
    } // for each line

    if (smiContent.size()) {
        lpub->ldrawFile.setSmiContent(fileName, smiContent);
    }
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
  bool doFadeStep  = (Preferences::enableFadeSteps || lpub->page.meta.LPub.fadeSteps.setup.value());
  bool doHighlightStep = (Preferences::enableHighlightStep || lpub->page.meta.LPub.highlightStep.setup.value()) && !suppressColourMeta();
  bool FadeMetaAdded = false;
  bool SilhouetteMetaAdded = false;

  QMutex WriteToTmpMutex;

  if (contents.size() > 0) {

      QStringList argv;

      for (int index = 0; index < contents.size(); index++) {

          WriteToTmpMutex.lock();

          QString contentLine = contents[index];
          split(contentLine, argv);
          if (argv.size() == 15 && argv[0] == "1") {
              // Insert opening fade meta
              if (!FadeMetaAdded && doFadeStep && partType == FADE_PART){
                 configuredContents.insert(index,QString("0 !FADE %1").arg(Preferences::fadeStepsOpacity));
                 FadeMetaAdded = true;
              }
              // Insert opening silhouette meta
              if (!SilhouetteMetaAdded && doHighlightStep && partType == HIGHLIGHT_PART){
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
              if (LDrawColourParts::isLDrawColourPart(fileNameStr)){
                  if (extension.isEmpty()) {
                    fileNameStr = fileNameStr.append(QString("%1.ldr").arg(nameMod));
                  } else {
                    fileNameStr = fileNameStr.replace("."+extension, QString("%1.%2").arg(nameMod).arg(extension));
                  }
                }
              // subfiles
              if (lpub->ldrawFile.isSubmodel(fileNameStr)) {
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

          WriteToTmpMutex.unlock();
      }
  } else {
    return contents;
  }

  // add the color list to the header of the configuredContents
  if (!subfileColourList.isEmpty()){
      WriteToTmpMutex.lock();

      subfileColourList.toSet().toList();  // remove dupes
      configuredContents.prepend("0");
      for (int i = 0; i < subfileColourList.size(); ++i)
          configuredContents.prepend(subfileColourList.at(i));
      configuredContents.prepend(QString("0 // %1 step custom colours").arg(VER_PRODUCTNAME_STR));
      configuredContents.prepend("0");

      WriteToTmpMutex.unlock();
  }

  return configuredContents;
}

/*
 * Process csiParts list - fade previous step-parts and or highlight current step-parts.
 * To get the previous content position, take the previous cisFile file size.
 * The csiFile entries are only parts with not formatting or meta commands so it is
 * well suited to provide the delta between steps.
 */
QStringList Gui::configureModelStep(const QStringList &csiParts, const int &stepNum,  Where &current/*topOfStep*/) {

  QStringList configuredCsiParts, stepColourList;
  bool doFadeStep  = Preferences::enableFadeSteps;
  bool doHighlightStep = Preferences::enableHighlightStep && !suppressColourMeta();
  bool doHighlightFirstStep = Preferences::highlightFirstStep;
  bool FadeMetaAdded = false;
  bool SilhouetteMetaAdded = false;

  if (csiParts.size() > 0 && (doHighlightFirstStep ? true : stepNum > 1)) {

      QString fadeColour  = LDrawColor::ldColorCode(lpub->page.meta.LPub.fadeSteps.color.value().color);

      // retrieve the previous step position
      int prevStepPosition = lpub->ldrawFile.getPrevStepPosition(current.modelName,current.lineNumber,stepNum);
      if (prevStepPosition == 0 && Gui::savePrevStepPosition > 0)
          prevStepPosition = Gui::savePrevStepPosition;

//#ifdef QT_DEBUG_MODE
//      emit messageSig(LOG_DEBUG, QString("Configure StepNum: %1, PrevStepPos: %2, StepPos: %3, ModelSize: %4, ModelName: %5")
//                      .arg(stepNum)
//                      .arg(prevStepPosition)
//                      .arg(csiParts.size())
//                      .arg(lpub->ldrawFile.size(current.modelName))
//                      .arg(current.modelName));
//#endif

      QStringList argv;

      int type_1_5_line_count = 0;

      for (int index = 0; index < csiParts.size(); index++) {

          bool type_1_line = false;
          bool type_1_5_line = false;
          bool is_helper_part = false;
          bool is_colour_part = false;
          bool is_submodel_file = false;

          int updatePosition = index + 1;

          QString csiLine = csiParts[index];
          split(csiLine, argv);

          // determine line type
          if (argv.size() && argv[0].size() == 1 &&
              argv[0] >= "1" && argv[0] <= "5") {
              type_1_5_line = true;
              type_1_5_line_count++;
              if (argv.size() == 15 && argv[0] == "1") {
                  type_1_line = true;
                  is_helper_part = ExcludedParts::isExcludedHelperPart(argv[argv.size()-1]);
              }
          }

          if (!is_helper_part) {
              // process parts naming
              QString fileNameStr, extension;
              if (type_1_line) {
                  // set part name and extension
                  fileNameStr = QString(argv[argv.size()-1]).toLower();
                  extension = QFileInfo(fileNameStr).suffix();

                  // check if is color part
                  is_colour_part = LDrawColourParts::isLDrawColourPart(fileNameStr);

                  // check if is submodel
                  is_submodel_file = lpub->ldrawFile.isSubmodel(fileNameStr);

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
          }

          if (isGhost(csiLine))
              argv.prepend(GHOST_META);

          // previous step parts
          csiLine = argv.join(" ");

          // current step parts
          configuredCsiParts  << csiLine;

          // Insert closing fade meta
          if (updatePosition == prevStepPosition) {
              if (FadeMetaAdded)
                  configuredCsiParts.append(QString("0 !FADE"));
          }

          // Insert closing silhouette meta
          if (index+1 == csiParts.size()) {
              if (SilhouetteMetaAdded)
                  configuredCsiParts.append(QString("0 !SILHOUETTE"));
          }
      }

      // save the current step position
      lpub->ldrawFile.setPrevStepPosition(current.modelName,stepNum,type_1_5_line_count);

  } else {

      // save the current step position
      lpub->ldrawFile.setPrevStepPosition(current.modelName,stepNum,csiParts.size());
      return csiParts;

  }

  // add the fade color list to the header of the CsiParts list
  if (!stepColourList.isEmpty()) {
      stepColourList.toSet().toList();  // remove dupes
      configuredCsiParts.prepend("0");

      for (int i = 0; i < stepColourList.size(); ++i)
          configuredCsiParts.prepend(stepColourList.at(i));

      configuredCsiParts.prepend(QString("0 // %1 step custom colours").arg(VER_PRODUCTNAME_STR));
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
  // Fade Steps Alpha Percent (default = 100%) -  e.g. 50% of Alpha 255 rounded up we get ((255 * 50) + (100 - 1)) / 100

  bool fadePartType          = partType == FADE_PART;

  QString _colourPrefix      = fadePartType ? LPUB3D_COLOUR_FADE_PREFIX : LPUB3D_COLOUR_HIGHLIGHT_PREFIX;  // fade prefix 100, highlight prefix 110
  QString _fadeColour        = LDrawColor::ldColorCode(lpub->page.meta.LPub.fadeSteps.color.value().color);
  QString _colourCode        = _colourPrefix + (fadePartType ? Preferences::fadeStepsUseColour ? _fadeColour : colourCode : colourCode);
  QString _mainColourValue   = LDrawColor::value(colourCode);
  QString _edgeColourValue   = fadePartType ? LDrawColor::edge(colourCode) : Preferences::highlightStepColour;
  QString _colourDescription = LPUB3D_COLOUR_NAME_PREFIX + LDrawColor::name(colourCode);
  int _fadeAlphaValue        = ((LDrawColor::alpha(colourCode) * (100 - Preferences::fadeStepsOpacity)) + (100 - 1)) / 100;
  int _alphaValue            = fadePartType ? _fadeAlphaValue : LDrawColor::alpha(colourCode);             // use 100% opacity with highlight color

  return QString("0 !COLOUR %1 CODE %2 VALUE %3 EDGE %4 ALPHA %5")
                 .arg(_colourDescription)   // description
                 .arg(_colourCode)          // original color code
                 .arg(_mainColourValue)     // main color value
                 .arg(_edgeColourValue)     // edge color value
                 .arg(_alphaValue);         // color alpha value
}

void Gui::setSceneItemZValueDirection(
        QMap<Where, SceneObjectData> *selectedSceneItems,
        Meta &curMeta,
  const QString &line)
{
    auto insertItem = [&selectedSceneItems](Where here, SceneObjectData soData) {
       if (selectedSceneItems->contains(here))
           selectedSceneItems->remove(here);
       selectedSceneItems->insert(here,soData);
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
   else if (line.contains(curMeta.LPub.page.scene.reserveBackground.preamble))
          insertItem(curMeta.LPub.page.scene.reserveBackground.here(),
                  curMeta.LPub.page.scene.reserveBackground.value());
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
