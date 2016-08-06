
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
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
#include <QtWidgets>
#else
#include <QtGui>
#endif
#include <QGraphicsItem>
#include <QGraphicsScene>
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

#include "QsLog.h"

/*********************************************
 *
 * remove_group
 *
 * this removes members of a group from the
 * ldraw file held in the the ldr string
 *
 ********************************************/

static void remove_group(
    QStringList  in,
    QString      group,
    QStringList &out)
{
  QRegExp bgt("^\\s*0\\s+MLCAD\\s+BTG\\s+(.*)$");

  for (int i = 0; i < in.size(); i++) {
      QString line = in.at(i);

      if (line.contains(bgt)) {
          if (bgt.cap(bgt.captureCount()) == group) {
              i++;
            } else {
              out << line;
            }
        } else {
          out << line;
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
    QStringList  in,
    QString      model,
    QStringList &out)
{

  model = model.toLower();

  for (int i = 0; i < in.size(); i++) {
      QString line = in.at(i);
      QStringList tokens;

      split(line,tokens);

      if (tokens.size() == 15 && tokens[0] == "1") {
          QString type = tokens[14].toLower();
          if (type != model) {
              out << line;
            }
        } else {
          out << line;
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
    QStringList  in,
    QString      name,
    QStringList &out)
{
  name = name.toLower();

  for (int i = 0; i < in.size(); i++) {
      QString line = in.at(i);
      QStringList tokens;

      split(line,tokens);

      if (tokens.size() == 4 && tokens[0] == "0" &&
          tokens[1] == "LPUB" &&
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
                    }
                }
            } else {
              out << line;
            }
        } else {
          out << line;
        }
    }

  return;
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
 * are in the building instuctions.
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

int Gui::drawPage(LGraphicsView  *view,
    QGraphicsScene *scene,
    Steps          *steps,
    int             stepNum,
    QString const  &addLine,
    Where          &current,
    QStringList    &csiParts,
    QStringList    &pliParts,
    bool            isMirrored,
    QHash<QString, QStringList> &bfx,
    bool            printing,
    bool            bfxStore2,
    QStringList    &bfxParts,
    QStringList    &ldrStepFiles,
    bool            supressRotateIcon,
    bool            calledOut)
{
  QStringList saveCsiParts;
  bool        global = true;
  QString     line;
  Callout    *callout     = NULL;
  Range      *range       = NULL;
  Step       *step        = NULL;
  bool        pliIgnore   = false;
  bool        partIgnore  = false;
  bool        synthBegin  = false;
  bool        multiStep   = false;
  bool        partsAdded  = false;
  bool        coverPage   = false;
  bool        bfxStore1   = false;
  bool        bfxLoad     = false;
  int         numLines    = ldrawFile.size(current.modelName);
  bool        firstStep   = true;
  bool        noStep      = false;
  bool        modelDisplayPage  = false;
  bool        rotateIcon  = false;

  steps->isMirrored = isMirrored;
  steps->setTopOfSteps(current);

  QList<InsertMeta> inserts;

  Where topOfStep = current;
  Rc gprc = OkRc;
  Rc rc;

  page.coverPage = false;

  QStringList calloutParts;

  emit messageSig(true, "Processing draw page for " + current.modelName);

  /*
   * do until end of page
   */
  for ( ; current <= numLines; current++) {

      Meta   &curMeta = callout ? callout->meta : steps->meta;

      QStringList tokens;

      // If we hit end of file we've got to note end of step

      if (current >= numLines) {
          line.clear();
          gprc = EndOfFileRc;
          tokens << "0";

          // not end of file, so get the next LDraw line

        } else {
          line = ldrawFile.readLine(current.modelName,current.lineNumber);
          split(line,tokens);
        }

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

          csiParts << line;
          partsAdded = true;

          /* since we have a part usage, we have a valid step */

          if (step == NULL  && ! noStep) {
              if (range == NULL) {
                  range = newRange(steps,calledOut);
                  steps->append(range);
                }

              step = new Step(topOfStep,
                              range,
                              stepNum,
                              curMeta,
                              calledOut,
                              multiStep);

              range->append(step);
            }

          /* addition of ldraw parts */
          if (curMeta.LPub.pli.show.value()
              && ! pliIgnore
              && ! partIgnore
              && ! synthBegin) {
              QString colorType = color+type;
              if (! isSubmodel(type) || curMeta.LPub.pli.includeSubs.value()) {

                  /*  check if substitute part exist and replace */
                  if(PliSubstituteParts::hasSubstitutePart(type)) {

                      QStringList substituteToken;
                      split(line,substituteToken);
                      QString substitutePart = type;

                      if (PliSubstituteParts::getSubstitutePart(substitutePart)){
                          substituteToken[substituteToken.size()-1] = substitutePart;
                        }

                      line = substituteToken.join(" ");
                    }

                  if (bfxStore2 && bfxLoad) {
                      for (int i = 0; i < bfxParts.size(); i++) {
                          if (bfxParts[i] == colorType) {
                              bfxParts.removeAt(i);
                              break;
                            }
                        }

                      // Danny: the following condition should help LPUB to remove automatically from PLI the parts in the buffer,
                      // but does not work if two buffers are used one after another in a multi step page.
                      // Better to make the user use the !LPUB PLI BEGIN IGN / END

                      //if ( ! removed )  {
                      pliParts << Pli::partLine(line,current,steps->meta);
                      //}
                    } else {

                      pliParts << Pli::partLine(line,current,steps->meta);
                    }
                }
              // bfxStore1 goes true when we've seen BFX STORE the first time
              // in a sequence of steps.  This used to be commented out which
              // means it didn't work in some cases, but we need it in step
              // group cases, so.... bfxStore1 && multiStep (was just bfxStore1)
              if (bfxStore1 && (multiStep || calledOut)) {
                  bfxParts << colorType;
                }
            }

          /* if it is a sub-model, then process it */

          if (ldrawFile.isSubmodel(type) && callout && ! noStep) {

              CalloutBeginMeta::CalloutMode mode = callout->meta.LPub.callout.begin.value();

//              qDebug() << "CALLOUT MODE: " << (mode == CalloutBeginMeta::Unassembled ? "Unassembled" :
//                                               mode == CalloutBeginMeta::Rotated ? "Rotated" : "Assembled");
              // If callout is rotated or assembled then suppress rotate icon
              supressRotateIcon = (mode == CalloutBeginMeta::Unassembled ? false : true);

              /* we are a callout, so gather all the steps within the callout */
              /* start with new meta, but no rotation step */

              QString thisType = type;

              if (mode != CalloutBeginMeta::Unassembled) {
                  /* So, we process callouts in-line, not when we finally hit the STEP or
             ROTSTEP that ends this processing, but for ASSEMBLED or ROTATED
             callouts, the ROTSTEP state affects the results, so we have to search
             forward until we hit STEP or ROTSTEP to know how the submodel might
             want to be rotated.  Also, for submodel's who's scale is different
             than their parent's scale, we want to scan ahead and find out the
             parent's scale and "render" the submodels at the parent's scale */
                  Meta tmpMeta = curMeta;
                  Where walk = current;
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
                  callout->meta.rotStep = tmpMeta.rotStep;
                  callout->meta.LPub.assem.modelScale = tmpMeta.LPub.assem.modelScale;
                  // The next command applies the rotation due to line, but not due to callout->meta.rotStep
                  thisType = callout->wholeSubmodel(callout->meta,type,line,0);
                }

              if (callout->bottom.modelName != thisType) {

                  Where current2(thisType,0);
                  skipHeader(current2);
                  if (mode == CalloutBeginMeta::Assembled) {
                      // In this case, no additional rotation should be applied to the submodel
                      callout->meta.rotStep.clear();
                    }
                  SubmodelStack tos(current.modelName,current.lineNumber,stepNum);
                  callout->meta.submodelStack << tos;
                  Meta saveMeta = callout->meta;
                  callout->meta.LPub.pli.constrain.resetToDefault();

                  step->append(callout);

                  calloutParts.clear();
                  QStringList csiParts2;

                  QHash<QString, QStringList> calloutBfx;

                  int rc;
                  rc = drawPage(
                        view,
                        scene,
                        callout,
                        1,
                        line,
                        current2,
                        csiParts2,
                        calloutParts,
                        ldrawFile.mirrored(tokens),
                        calloutBfx,
                        printing,
                        bfxStore2,
                        bfxParts,
                        ldrStepFiles,
                        supressRotateIcon,
                        true);

                  callout->meta = saveMeta;

                  if (callout->meta.LPub.pli.show.value() &&
                      ! callout->meta.LPub.callout.pli.perStep.value() &&
                      ! pliIgnore && ! partIgnore && ! synthBegin &&
                      mode == CalloutBeginMeta::Unassembled) {

                      pliParts += calloutParts;
                    }

                  if (rc != 0) {
                      steps->placement = steps->meta.LPub.assem.placement;
                      return rc;
                    }
                } else {
                  callout->instances++;
                  if (mode == CalloutBeginMeta::Unassembled) {
                      pliParts += calloutParts;
                    }
                }

              /* remind user what file we're working on */

//              statusBar()->showMessage("Processing " + current.modelName);
              emit messageSig(true, "Processing " + current.modelName);
            }

        } else if (tokens.size() > 0 &&
                   (tokens[0] == "2" ||
                    tokens[0] == "3" ||
                    tokens[0] == "4" ||
                    tokens[0] == "5")) {

          csiParts << line;
          partsAdded = true;

          /* we've got a line, triangle or polygon, so add it to the list */
          /* and make sure we know we have a step */

          if (step == NULL && ! noStep) {
              if (range == NULL) {
                  range = newRange(steps,calledOut);
                  steps->append(range);
                }

              step = new Step(topOfStep,
                              range,
                              stepNum,
                              steps->meta,
                              calledOut,
                              multiStep);

              range->append(step);
            }

        } else if ( (tokens.size() > 0 && tokens[0] == "0") || gprc == EndOfFileRc) {

          /* must be meta-command (or comment) */
          if (global && tokens.contains("!LPUB") && tokens.contains("GLOBAL")) {
              topOfStep = current;
            } else {
              global = false;
            }

          QString part;

          if (gprc == EndOfFileRc) {
              rc = gprc;
            } else {
              rc = curMeta.parse(line,current,true);
            }

          InsertData insertData;

          /* handle specific meta-commands */

          switch (rc) {
            /* toss it all out the window, per James' original plan */
            case ClearRc:
              pliParts.clear();
              csiParts.clear();
              //steps->freeSteps();  // had to remove this because it blows steps following clear
              // in step group.
              break;

              /* Buffer exchange */
            case BufferStoreRc:
              bfx[curMeta.bfx.value()] = csiParts;
              bfxStore1 = true;
              bfxParts.clear();
              break;

            case BufferLoadRc:
              csiParts = bfx[curMeta.bfx.value()];
              ldrawFile.setFadePosition(current.modelName,csiParts.size());
              //qDebug() << "Model:" << current.modelName << ", Step:"  << stepNum << ", bfx Set Fade Position:" << csiParts.size();
              bfxLoad = true;
              break;

            case MLCadGroupRc:
              csiParts << line;
              break;

            case IncludeRc:
              include(curMeta);
              break;

              /* substitute part/parts with this */

            case PliBeginSub1Rc:
              if (pliIgnore) {
                  parseError("Nested PLI BEGIN/ENDS not allowed\n",current);
                }
              if (steps->meta.LPub.pli.show.value() &&
                  ! pliIgnore &&
                  ! partIgnore &&
                  ! synthBegin) {

                  SubData subData = curMeta.LPub.pli.begin.sub.value();
                  QString addPart = QString("1 0  0 0 0  0 0 0 0 0 0 0 0 0 %1") .arg(subData.part);
                  pliParts << Pli::partLine(addPart,current,curMeta);
                }

              if (step == NULL && ! noStep) {
                  if (range == NULL) {
                      range = newRange(steps,calledOut);
                      steps->append(range);
                    }
                  step = new Step(topOfStep,
                                  range,
                                  stepNum,
                                  curMeta,
                                  calledOut,
                                  multiStep);

                  range->append(step);
                }
              pliIgnore = true;
              break;

              /* substitute part/parts with this */
            case PliBeginSub2Rc:
              if (pliIgnore) {
                  parseError("Nested BEGIN/ENDS not allowed\n",current);
                }
              if (steps->meta.LPub.pli.show.value() &&
                  ! pliIgnore &&
                  ! partIgnore &&
                  ! synthBegin) {

                  SubData subData = curMeta.LPub.pli.begin.sub.value();
                  QString addPart = QString("1 %1  0 0 0  0 0 0 0 0 0 0 0 0 %2") .arg(subData.color) .arg(subData.part);
                  pliParts << Pli::partLine(addPart,current,curMeta);
                }

              if (step == NULL && ! noStep) {
                  if (range == NULL) {
                      range = newRange(steps,calledOut);
                      steps->append(range);
                    }
                  step = new Step(topOfStep,
                                  range,
                                  stepNum,
                                  curMeta,
                                  calledOut,
                                  multiStep);

                  range->append(step);
                }
              pliIgnore = true;
              break;

              /* do not put subsequent parts into PLI */
            case PliBeginIgnRc:
              if (pliIgnore) {
                  parseError("Nested BEGIN/ENDS not allowed\n",current);
                }
              pliIgnore = true;
              break;
            case PliEndRc:
              if ( ! pliIgnore) {
                  parseError("PLI END with no PLI BEGIN",current);
                }
              pliIgnore = false;
              break;

              /* discard subsequent parts, and don't create CSI's for them */
            case PartBeginIgnRc:
            case MLCadSkipBeginRc:
              if (partIgnore) {
                  parseError("Nested BEGIN/ENDS not allowed\n",current);
                }
              partIgnore = true;
              break;

            case PartEndRc:
            case MLCadSkipEndRc:
              if (partIgnore) {
                  parseError("Ignore ending with no ignore begin",current);
                }
              partIgnore = false;
              break;

            case SynthBeginRc:
              if (synthBegin) {
                  parseError("Nested BEGIN/ENDS not allowed\n",current);
                }
              synthBegin = true;
              break;

            case SynthEndRc:
              if ( ! synthBegin) {
                  parseError("Ignore ending with no ignore begin",current);
                }
              synthBegin = false;
              break;


              /* remove a group or all instances of a part type */
            case GroupRemoveRc:
            case RemoveGroupRc:
            case RemovePartRc:
            case RemoveNameRc:
              {
                QStringList newCSIParts;

                if (rc == RemoveGroupRc) {
                    remove_group(csiParts,steps->meta.LPub.remove.group.value(),newCSIParts);
                  } else if (rc == RemovePartRc) {
                    remove_parttype(csiParts, steps->meta.LPub.remove.parttype.value(),newCSIParts);
                  } else {
                    remove_partname(csiParts, steps->meta.LPub.remove.partname.value(),newCSIParts);
                  }
                csiParts = newCSIParts;

                if (step == NULL && ! noStep) {
                    if (range == NULL) {
                        range = newRange(steps,calledOut);
                        steps->append(range);
                      }
                    step = new Step(topOfStep,
                                    range,
                                    stepNum,
                                    curMeta,
                                    calledOut,
                                    multiStep);

                    range->append(step);
                  }
              }
              break;

            case ReserveSpaceRc:
              /* since we have a part usage, we have a valid step */
              if (calledOut || multiStep) {
                  step = NULL;
                  Reserve *reserve = new Reserve(current,steps->meta.LPub);
                  if (range == NULL) {
                      range = newRange(steps,calledOut);
                      steps->append(range);
                    }
                  range->append(reserve);
                }
              break;

            case InsertCoverPageRc:
              {
                coverPage = true;
                page.coverPage = true;
                QRegExp frontCoverPage("^\\s*0\\s+!LPUB\\s+.*FRONT");
                QRegExp backCoverPage("^\\s*0\\s+!LPUB\\s+.*BACK");
                if (line.contains(frontCoverPage)){
                    page.frontCover = true;
                    page.backCover  = false;
                  }
                if (line.contains(backCoverPage)){
                    page.backCover  = true;
                    page.frontCover = false;
                  }
                // nothing to display in 3D Window
                gMainWindow->NewProject();
              }
            case InsertPageRc:
              {
                partsAdded = true;

                // nothing to display in 3D Window
                gMainWindow->NewProject();
              }
              break;

            case InsertRc:
              {
                inserts.append(curMeta.LPub.insert);         // these are always placed before any parts in step
                insertData = curMeta.LPub.insert.value();

                if (insertData.type == InsertData::InsertRotateIcon) { // indicate that we have a rotate icon for this step

//                   qDebug() << "CALLED OUT: " << calledOut << " SUPRESS ROTATE ICON: " << supressRotateIcon;

                  if (calledOut && supressRotateIcon) {
                      rotateIcon = false;
                    } else {
                      rotateIcon = true;
                    }
                }

                if (insertData.type == InsertData::InsertFinalModel){

                    if (curMeta.LPub.fadeStep.fadeStep.value()){
                        // this is not a step but it's necessary to use the step object to place the model
                        if (step == NULL) {
                            if (range == NULL) {
                                range = newRange(steps,calledOut);
                                steps->append(range);
                              }
                            stepNum = NOSTEPNUMBER; //send special value so csiitem can supress some context menu items
                            step = new Step(topOfStep,
                                            range,
                                            stepNum,
                                            curMeta,
                                            calledOut,
                                            multiStep);
                            range->append(step);
                          }
                        partsAdded  = true; // OK, so this is a lie, but it works
                        modelDisplayPage  = true;
                      }
                  }
              }
              break;

            case CalloutBeginRc:
              if (callout) {
                  parseError("Nested CALLOUT not allowed within the same file",current);
                } else {
                  callout = new Callout(curMeta,view);
                  callout->setTopOfCallout(current);
                }
              break;

            case CalloutDividerRc:
              if (range) {
                  range->sepMeta = curMeta.LPub.callout.sep;
                  range = NULL;
                  step = NULL;
                }
              break;

            case CalloutPointerRc:
              if (callout) {
                  callout->appendPointer(current,curMeta.LPub.callout);
                }
              break;

            case CalloutEndRc:
              if ( ! callout) {
                  parseError("CALLOUT END without a CALLOUT BEGIN",current);
                } else {
                  callout->parentStep = step;
                  if (multiStep) {
                      callout->parentRelativeType = StepGroupType;
                    } else if (calledOut) {
                      callout->parentRelativeType = CalloutType;
                    } else {
                      callout->parentRelativeType = step->relativeType;
                    }
                  callout->pli.clear();
                  callout->placement = curMeta.LPub.callout.placement;
                  callout->setBottomOfCallout(current);
                  callout = NULL;
                }
              break;

            case StepGroupBeginRc:
              if (calledOut) {
                  parseError("MULTI_STEP not allowed inside callout models",current);
                } else {
                  if (multiStep) {
                      parseError("Nested MULTI_STEP not allowed",current);
                    }
                  multiStep = true;
                }
              steps->relativeType = StepGroupType;
              break;

            case StepGroupDividerRc:
              if (range) {
                  range->sepMeta = steps->meta.LPub.multiStep.sep;
                  range = NULL;
                  step = NULL;
                }
              break;

              /* finished off a multiStep */
            case StepGroupEndRc:
              if (multiStep && steps->list.size()) {
                  // save the current meta as the meta for step group
                  // PLI for non-pli-per-step
                  if (partsAdded) {
                      parseError("Expected STEP before MULTI_STEP END", current);
                    }
                  multiStep = false;

                  if (pliParts.size() && steps->meta.LPub.multiStep.pli.perStep.value() == false) {
                      PlacementData placementData = steps->stepGroupMeta.LPub.multiStep.pli.placement.value();
                      // Override default, which is for PliPerStep true
                      if (placementData.relativeTo != PageType &&
                          placementData.relativeTo != StepGroupType) {
                          placementData.relativeTo  = PageType;
                          placementData.placement   = TopLeft;
                          placementData.preposition = Inside;
                          placementData.offsets[0]  = 0;
                          placementData.offsets[1]  = 0;
                          steps->stepGroupMeta.LPub.multiStep.pli.placement.setValue(placementData);
                        }
                      steps->pli.bom = false;
                      steps->pli.setParts(pliParts,steps->stepGroupMeta);
                      steps->pli.sizePli(&steps->stepGroupMeta, StepGroupType, false);
                    }
                  pliParts.clear();

                  /* this is a page we're supposed to process */

                  steps->placement = steps->meta.LPub.multiStep.placement;

                  showLine(steps->topOfSteps());

                  Page *page = dynamic_cast<Page *>(steps);                  
                  if (page) {
                      page->inserts = inserts;
                    }

                  bool endOfSubmodel = stepNum >= ldrawFile.numSteps(current.modelName);;

                  int  instances = ldrawFile.instances(current.modelName,isMirrored);

                  emit messageSig(true, "Add graphics for multi-step page " + current.modelName);

                  if (renderer->useLDViewSCall() && ldrStepFiles.size() > 0){
                      QElapsedTimer timer;
                      timer.start();

                      int rc = renderer->renderLDViewSCallCsi(ldrStepFiles, steps->meta);
                      if (rc != 0) {
                          QMessageBox::critical(NULL,QMessageBox::tr(VER_PRODUCTNAME_STR),
                                                QMessageBox::tr("Render CSI images failed."));
                          return rc;
                        }

                      logTrace() << Render::getRenderer()
                                 << "CSI single call render took"
                                 << timer.elapsed() << "milliseconds"
                                 << "to render " << ldrStepFiles.size()
                                 << (ldrStepFiles.size() > 1 ? "images" : "image")
                                 << (calledOut ? "for called out," : "for simple,")
                                 << "step group on page" << stepPageNum << ".";
                    }

                  addGraphicsPageItems(steps, coverPage, modelDisplayPage, endOfSubmodel,instances, view, scene, printing);

                  return HitEndOfPage;
                }
              inserts.clear();
              break;

            case NoStepRc:
              noStep = true;
              break;

              /* we're hit some kind of step, or implied step and end of file */
            case EndOfFileRc:
            case RotStepRc:
            case StepRc:

              // special case of no parts added, but BFX load and not NOSTEP
              if ( ! partsAdded && bfxLoad && ! noStep) {
                  if (step == NULL) {
                      if (range == NULL) {
                          range = newRange(steps,calledOut);
                          steps->append(range);
                        }
                      step = new Step(topOfStep,
                                      range,
                                      stepNum,
                                      curMeta,
                                      calledOut,
                                      multiStep);

                      range->append(step);
                    }

                  emit messageSig(true, "Processing bfx model (CSI) for " + current.modelName);

                  (void) step->createCsi(
                        isMirrored ? addLine : "1 color 0 0 0 1 0 0 0 1 0 0 0 1 foo.ldr",
                        saveCsiParts = fadeStep(csiParts, stepNum, current),
                        &step->csiPixmap,
                        steps->meta);

                  if (renderer->useLDViewSCall() && ! step->ldrName.isNull()) {
                      ldrStepFiles << step->ldrName;
                      //qDebug() << "CSI ldr file #"<< ldrStepFiles.count() <<"added: " << step->ldrName;
                    }

                  partsAdded = true; // OK, so this is a lie, but it works
                }

              // normal case of parts added, and not NOSTEP
              if (partsAdded && ! noStep) {

                  if (firstStep) {
                      steps->stepGroupMeta = curMeta;
                      firstStep = false;
                    }

                  if (pliIgnore) {
                      parseError("PLI BEGIN then STEP. Expected PLI END",current);
                      pliIgnore = false;
                    }
                  if (partIgnore) {
                      parseError("PART BEGIN then STEP. Expected PART END",current);
                      partIgnore = false;
                    }
                  if (synthBegin) {
                      parseError("SYNTH BEGIN then STEP. Expected SYNTH_END",current);
                      synthBegin = false;
                    }

                  bool pliPerStep;

                  if (multiStep && steps->meta.LPub.multiStep.pli.perStep.value()) {
                      pliPerStep = true;
                    } else if (calledOut && steps->meta.LPub.callout.pli.perStep.value()) {
                      pliPerStep = true;
                    } else if ( ! multiStep && ! calledOut) {
                      pliPerStep = true;
                    } else {
                      pliPerStep = false;
                    }

                  if (step) {
                      Page *page = dynamic_cast<Page *>(steps);
                      if (page) {
                          page->inserts = inserts;
                        }

                      if (pliPerStep) {
                          PlacementType relativeType;
                          if (multiStep) {
                              relativeType = StepGroupType;
                            } else if (calledOut) {
                              relativeType = CalloutType;
                            } else {
                              relativeType = SingleStepType;
                            }

                          step->pli.setParts(pliParts,steps->meta);
                          pliParts.clear();

                          step->pli.sizePli(&steps->meta,relativeType,pliPerStep);
                        }

                      if (rotateIcon) {
                          step->placeRotateIcon = true;
                        }

                      emit messageSig(true, "Processing model (CSI) for " + current.modelName);

                      int rc = step->createCsi(
                            isMirrored ? addLine : "1 color 0 0 0 1 0 0 0 1 0 0 0 1 foo.ldr",
                            saveCsiParts = modelDisplayPage ? csiParts : fadeStep(csiParts, stepNum, current),
                            &step->csiPixmap,
                            steps->meta);

                      if (rc) {
                          QMessageBox::critical(NULL,QMessageBox::tr(VER_PRODUCTNAME_STR),
                                                QMessageBox::tr("Failed to create CSI."));
                          return rc;
                        }

                      if (renderer->useLDViewSCall() && ! step->ldrName.isNull()) {
                          ldrStepFiles << step->ldrName;
                          //qDebug() << "CSI ldr file #"<< ldrStepFiles.count() <<"added: " << step->ldrName;
                        }

                    } else {

                      if (pliPerStep) {
                          pliParts.clear();
                        }

                      // Only pages or step can have inserts.... not callouts
                      if ( ! multiStep && ! calledOut) {
                          Page *page = dynamic_cast<Page *>(steps);
                          if (page) {
                              page->inserts = inserts;
                            }
                        }
                    }

                  if ( ! multiStep && ! calledOut) {

                      // Simple step

                      steps->placement = steps->meta.LPub.assem.placement;

                      showLine(topOfStep);

                      int  numSteps = ldrawFile.numSteps(current.modelName);

                      bool endOfSubmodel = numSteps == 0 || stepNum >= numSteps;

                      int  instances = ldrawFile.instances(current.modelName, isMirrored);

                      emit messageSig(true, "Add graphics for single-step page.");

                      if (renderer->useLDViewSCall() && ldrStepFiles.size() > 0){

                          QElapsedTimer timer;
                          timer.start();

                          int rc = renderer->renderLDViewSCallCsi(ldrStepFiles, steps->meta);
                          if (rc != 0) {
                              QMessageBox::critical(NULL,QMessageBox::tr(VER_PRODUCTNAME_STR),
                                                    QMessageBox::tr("Render CSI images failed."));
                              return rc;
                            }

                          logTrace() << Render::getRenderer()
                                     << "CSI single call render took"
                                     << timer.elapsed() << "milliseconds"
                                     << "to render " << ldrStepFiles.size()
                                     << (ldrStepFiles.size() > 1 ? "images" : "image")
                                     << (calledOut ? "for called out," : "for simple,")
                                     << "single step on page" << stepPageNum << ".";
                        }

                      addGraphicsPageItems(steps,coverPage,modelDisplayPage, endOfSubmodel,instances,view,scene,printing);
                      stepPageNum += ! coverPage;
                      steps->setBottomOfSteps(current);

                      return HitEndOfPage;
                    }

                  steps->meta.pop();
                  stepNum += partsAdded;
                  topOfStep = current;

                  partsAdded = false;
                  coverPage = false;
                  rotateIcon = false;
//                supressRotateIcon = false;
                  step = NULL;
                  bfxStore2 = bfxStore1;
                  bfxStore1 = false;
                  bfxLoad = false;
                }
              if ( ! multiStep) {
                  inserts.clear();
                }
              steps->setBottomOfSteps(current);
              noStep = false;
              break;
            case RangeErrorRc:
              showLine(current);
              QMessageBox::critical(NULL,
                                    QMessageBox::tr("LPub3D"),
                                    QMessageBox::tr("Parameter(s) out of range: %1:%2\n%3")
                                    .arg(current.modelName)
                                    .arg(current.lineNumber)
                                    .arg(line));
              return RangeErrorRc;
              break;
            default:
              break;
            }
        } else if (line != "") {
          showLine(current);
          QMessageBox::critical(NULL,
                                QMessageBox::tr("LPub3D"),
                                QMessageBox::tr("Invalid LDraw Line Type: %1:%2\n  %3")
                                .arg(current.modelName)
                                .arg(current.lineNumber)
                                .arg(line));
          return InvalidLDrawLineRc;
        }
    }
  return 0;
}

int Gui::findPage(
    LGraphicsView  *view,
    QGraphicsScene *scene,
    int            &pageNum,
    QString const  &addLine,
    Where          &current,
    bool            isMirrored,
    Meta            meta,
    bool            printing)
{
  bool stepGroup  = false;
  bool partIgnore = false;
  bool coverPage  = false;
  bool stepPage   = false;
  bool bfxStore1  = false;
  bool bfxStore2  = false;
  bool stepGroupBfxStore2 = false;
  bool callout    = false;
  bool noStep     = false;
  bool noStep2    = false;

  QStringList bfxParts;
  QStringList saveBfxParts;
  QStringList ldrStepFiles;
  int  partsAdded = 0;
  int  stepNumber = 1;
  Rc   rc;

  skipHeader(current);

  if (pageNum == 1) {
      topOfPages.clear();
      topOfPages.append(current);
    }

  QStringList csiParts;
  QStringList saveCsiParts;
  Where       saveCurrent = current;
  Where       stepGroupCurrent;
  int         saveStepNumber = 1;
  saveStepPageNum = stepPageNum;

  Meta        saveMeta = meta;

  QHash<QString, QStringList> bfx;
  QHash<QString, QStringList> saveBfx;

  int numLines = ldrawFile.size(current.modelName);

  Where topOfStep = current;

  ldrawFile.setRendered(current.modelName, isMirrored);

  RotStepMeta saveRotStep = meta.rotStep;

  emit messageSig(true, "Processing find page for " + current.modelName);

  for ( ;
        current.lineNumber < numLines;
        current.lineNumber++) {

      // scan through the rest of the model counting pages
      // if we've already hit the display page, then do as little as possible

      QString line = ldrawFile.readLine(current.modelName,current.lineNumber).trimmed();

      if (line.startsWith("0 GHOST ")) {
          line = line.mid(8).trimmed();
        }

      QStringList tokens, addTokens;

      switch (line.toLatin1()[0]) {
        case '1':
          split(line,tokens);

          if (tokens[1] == "16") {
              split(addLine,addTokens);
              if (addTokens.size() == 15) {
                  tokens[1] = addTokens[1];
                }
              line = tokens.join(" ");
            }

          if ( ! partIgnore) {

              // csiParts << line;

              if (firstStepPageNum == -1) {
                  firstStepPageNum = pageNum;
                }
              lastStepPageNum = pageNum;

              QStringList token;

              split(line,token);

              QString    type = token[token.size()-1];

              bool contains   = ldrawFile.isSubmodel(type);
              bool rendered   = ldrawFile.rendered(type,ldrawFile.mirrored(token));
              CalloutBeginMeta::CalloutMode mode = meta.LPub.callout.begin.value();

              if (contains && (!callout || (callout && mode != CalloutBeginMeta::Unassembled) )) {
                  if ( ! rendered && (! bfxStore2 || ! bfxParts.contains(token[1]+type))) {

                      isMirrored = ldrawFile.mirrored(token);

                      // can't be a callout
                      SubmodelStack tos(current.modelName,current.lineNumber,stepNumber);
                      meta.submodelStack << tos;
                      Where current2(type,0);

                      ldrawFile.setModelStartPageNumber(current2.modelName,pageNum);
                      //logTrace() << "SET Model: " << current2.modelName << " @ Page: " << pageNum;

                      // save rotStep, clear it, and restore it afterwards
                      // since rotsteps don't affect submodels
                      RotStepMeta saveRotStep2 = meta.rotStep;
                      meta.rotStep.clear();

                      findPage(view,scene,pageNum,line,current2,isMirrored,meta,printing);
                      saveStepPageNum = stepPageNum;
                      meta.submodelStack.pop_back();
                      meta.rotStep = saveRotStep2; // restore old rotstep
                    }
                }
              if (bfxStore1) {
                  bfxParts << token[1]+type;
                }
            } else if (partIgnore){

              if (tokens.size() == 15){
                  QString lineItem = tokens[tokens.size()-1];

                  if (ldrawFile.isSubmodel(lineItem)){
                      //Where model(lineItem,0);
                      statusBarMsg("Submodel " + lineItem + " is set to ignore (IGN).");
                      //ldrawFile.setModelStartPageNumber(model.modelName,pageNum);
                      //logTrace() << "SET Model (Ignore): " << model.modelName << " @ Page: " << pageNum;
                    }
                }
            }
        case '2':
        case '3':
        case '4':
        case '5':
          ++partsAdded;
          csiParts << line;
          break;

        case '0':
          rc = meta.parse(line,current);
          switch (rc) {
            case StepGroupBeginRc:
              stepGroup = true;
              stepGroupCurrent = topOfStep;
              // Steps within step group modify bfxStore2 as they progress
              // so we must save bfxStore2 and use the saved copy when
              // we call drawPage for a step group.
              stepGroupBfxStore2 = bfxStore2;
              break;
            case StepGroupEndRc:
              if (stepGroup && ! noStep2) {
                  stepGroup = false;
                  if (pageNum < displayPageNum) {
                      saveCsiParts   = csiParts;
                      saveStepNumber = stepNumber;
                      saveMeta       = meta;
                      saveBfx        = bfx;
                      saveBfxParts   = bfxParts;
                      bfxParts.clear();
                      saveRotStep = meta.rotStep;
                    } else if (pageNum == displayPageNum) {
                      csiParts.clear();
                      stepPageNum = saveStepPageNum;
                      if (pageNum == 1) {
                          page.meta = meta;
                        } else {
                          page.meta = saveMeta;
                        }
                      page.meta.pop();
                      page.meta.rotStep = saveRotStep;

                      QStringList pliParts;

                      (void) drawPage(view,
                                      scene,
                                      &page,
                                      saveStepNumber,
                                      addLine,
                                      stepGroupCurrent,
                                      saveCsiParts,
                                      pliParts,
                                      isMirrored,
                                      saveBfx,
                                      printing,
                                      stepGroupBfxStore2,
                                      saveBfxParts,
                                      ldrStepFiles);

                      saveCurrent.modelName.clear();
                      saveCsiParts.clear();
                    }
                  ++pageNum;
                  topOfPages.append(current);
                  saveStepPageNum = ++stepPageNum;
                }
              noStep2 = false;
              break;

            case RotStepRc:
            case StepRc:
              if (partsAdded && ! noStep) {
                  stepNumber += ! coverPage && ! stepPage;
                  stepPageNum += ! coverPage && ! stepGroup;
                  if (pageNum < displayPageNum) {
                      if ( ! stepGroup) {
                          saveCsiParts   = csiParts;
                          saveStepNumber = stepNumber;
                          saveMeta       = meta;
                          saveBfx        = bfx;
                          saveBfxParts   = bfxParts;
                          saveStepPageNum = stepPageNum;
                          // bfxParts.clear();
                        }
                      saveCurrent    = current;
                      saveRotStep = meta.rotStep;
                    }
                  if ( ! stepGroup) {
                      if (pageNum == displayPageNum) {
                          csiParts.clear();
                          stepPageNum = saveStepPageNum;
                          if (pageNum == 1) {
                              page.meta = meta;
                            } else {
                              page.meta = saveMeta;
                            }
                          page.meta.pop();
                          page.meta.rotStep = saveRotStep;
                          page.meta.rotStep = meta.rotStep;
                          QStringList pliParts;

                          (void) drawPage(view,
                                          scene,
                                          &page,
                                          saveStepNumber,
                                          addLine,
                                          saveCurrent,
                                          saveCsiParts,
                                          pliParts,
                                          isMirrored,
                                          saveBfx,
                                          printing,
                                          bfxStore2,
                                          saveBfxParts,
                                          ldrStepFiles);

                          saveCurrent.modelName.clear();
                          saveCsiParts.clear();
                        }
                      ++pageNum;
                      topOfPages.append(current);
                    }
                  topOfStep = current;
                  partsAdded = 0;
                  meta.pop();
                  coverPage = false;
                  stepPage = false;

                  bfxStore2 = bfxStore1;
                  bfxStore1 = false;
                  if ( ! bfxStore2) {
                      bfxParts.clear();
                    }
                } else if ( ! stepGroup) {
                  saveCurrent = current;  // so that draw page doesn't have to
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
              if (pageNum < displayPageNum) {
                  csiParts.clear();
                  saveCsiParts.clear();
                }
              break;

              /* Buffer exchange */
            case BufferStoreRc:
              if (pageNum < displayPageNum) {
                  bfx[meta.bfx.value()] = csiParts;
                }
              bfxStore1 = true;
              bfxParts.clear();
              break;
            case BufferLoadRc:
              if (pageNum < displayPageNum) {
                  csiParts = bfx[meta.bfx.value()];
                }
              partsAdded = true;
              break;

            case MLCadGroupRc:
              if (pageNum < displayPageNum) {
                  csiParts << line;
                  partsAdded = true;
                }
              break;

              /* remove a group or all instances of a part type */
            case GroupRemoveRc:
            case RemoveGroupRc:
            case RemovePartRc:
            case RemoveNameRc:
              if (pageNum < displayPageNum) {
                  QStringList newCSIParts;
                  if (rc == RemoveGroupRc) {
                      remove_group(csiParts,    meta.LPub.remove.group.value(),newCSIParts);
                    } else if (rc == RemovePartRc) {
                      remove_parttype(csiParts, meta.LPub.remove.parttype.value(),newCSIParts);
                    } else {
                      remove_partname(csiParts, meta.LPub.remove.partname.value(),newCSIParts);
                    }
                  csiParts = newCSIParts;
                  newCSIParts.empty();
                }
              break;

            case IncludeRc:
              include(meta);
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

  if (partsAdded && ! noStep) {
      if (pageNum == displayPageNum) {

          page.meta = saveMeta;
          QStringList pliParts;

          (void) drawPage(view,
                          scene,
                          &page,
                          saveStepNumber,
                          addLine,
                          saveCurrent,
                          saveCsiParts,
                          pliParts,
                          isMirrored,
                          saveBfx,
                          printing,
                          bfxStore2,
                          saveBfxParts,
                          ldrStepFiles);
        }
      ++pageNum;
      topOfPages.append(current);
      ++stepPageNum;
    }
  return 0;
}

int Gui::getBOMParts(
    Where        current,
    QString     &addLine,
    QStringList &pliParts)
{
  bool partIgnore = false;
  bool pliIgnore = false;
  bool synthBegin = false;
  bool bfxStore1 = false;
  bool bfxStore2 = false;
  bool bfxLoad = false;
  bool partsAdded = false;
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
          if ( ! partIgnore && ! pliIgnore && ! synthBegin) {

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
              QString colorPart = token[1] + type;

              if (bfxStore2 && bfxLoad) {
                  int i;
                  for (i = 0; i < bfxParts.size(); i++) {
                      if (bfxParts[i] == colorPart) {
                          bfxParts.removeAt(i);
                          removed = true;
                          break;
                        }
                    }
                }

              if ( ! removed) {

                  if (ldrawFile.isSubmodel(type)) {

                      Where current2(type,0);

                      getBOMParts(current2,line,pliParts);

                    } else {

                      /*  check if substitute part exist and replace */
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
              if (! pliIgnore &&
                  ! partIgnore &&
                  ! synthBegin) {

                  QString line = QString("1 0  0 0 0  0 0 0  0 0 0  0 0 0 %1") .arg(meta.LPub.pli.begin.sub.value().part);
                  pliParts << Pli::partLine(line,current,meta);
                  pliIgnore = true;
                }
              break;

              /* substitute part/parts with this */
            case PliBeginSub2Rc:
              if (! pliIgnore &&
                  ! partIgnore &&
                  ! synthBegin) {
                  QString line = QString("1 %1  0 0 0  0 0 0  0 0 0  0 0 0 %2")
                      .arg(meta.LPub.pli.begin.sub.value().color)
                      .arg(meta.LPub.pli.begin.sub.value().part);
                  pliParts << Pli::partLine(line,current,meta);
                  pliIgnore = true;
                }
              break;

            case PliBeginIgnRc:
              pliIgnore = true;
              break;

            case PliEndRc:
              pliIgnore = false;
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

              // Any of the metas that can change pliParts needs
              // to be processed here

            case ClearRc:
              pliParts.empty();
              break;

            case MLCadGroupRc:
              pliParts << Pli::partLine(line,current,meta);
              break;

              /* remove a group or all instances of a part type */
            case GroupRemoveRc:
            case RemoveGroupRc:
            case RemovePartRc:
            case RemoveNameRc:
              {
                QStringList newCSIParts;
                if (rc == RemoveGroupRc) {
                    remove_group(pliParts,meta.LPub.remove.group.value(),newCSIParts);
                  } else if (rc == RemovePartRc) {
                    remove_parttype(pliParts, meta.LPub.remove.parttype.value(),newCSIParts);
                  } else {
                    remove_partname(pliParts, meta.LPub.remove.partname.value(),newCSIParts);
                  }
                pliParts = newCSIParts;
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
      Where here = gui->topOfPages[gui->displayPageNum-1];
      for (++here; here.lineNumber < ldrawFile.size(here.modelName); here++) {
          QString line = gui->readLine(here);
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
              (argv[1] == "LPUB" || argv[1] == "!LPUB") &&
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
      statusBarMsg("Counting");
      Where current(ldrawFile.topLevelFile(),0);
      int savedDpn     = displayPageNum;
      displayPageNum   = 1 << 31;
      firstStepPageNum = -1;
      lastStepPageNum  = -1;
      maxPages         = 1;
      Meta meta;
      QString empty;
      stepPageNum = 1;
      findPage(KpageView,KpageScene,maxPages,empty,current,false,meta,false);
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
    QGraphicsScene *scene,
    bool            printing)
{
  QApplication::setOverrideCursor(Qt::WaitCursor);

  ldrawFile.unrendered();
  ldrawFile.countInstances();
  writeToTmp();
  Where       current(ldrawFile.topLevelFile(),0);
  maxPages = 1;
  stepPageNum = 1;
  ldrawFile.setModelStartPageNumber(current.modelName,maxPages);
  //logTrace() << "SET INITIAL Model: " << current.modelName << " @ Page: " << maxPages;
  QString empty;
  Meta    meta;
  firstStepPageNum = -1;
  lastStepPageNum = -1;

  findPage(view,scene,maxPages,empty,current,false,meta,printing);
  topOfPages.append(current);
  maxPages--;

  QString string = QString("%1 of %2") .arg(displayPageNum) .arg(maxPages);
  setPageLineEdit->setText(string);
  QApplication::restoreOverrideCursor();
}

void Gui::skipHeader(Where &current)
{
  int numLines = ldrawFile.size(current.modelName);
  for ( ; current.lineNumber < numLines; current.lineNumber++) {
      QString line = gui->readLine(current);
      int p;
      for (p = 0; p < line.size(); ++p) {
          if (line[p] != ' ') {
              break;
            }
        }
      if (line[p] >= '1' && line[p] <= '5') {
          if (current.lineNumber == 0) {
              QString empty = "0 ";
              gui->insertLine(current,empty,NULL);
            } else if (current > 0) {
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

void Gui::include(Meta &meta)
{
  QString fileName = meta.LPub.include.value();
  if (ldrawFile.isSubmodel(fileName)) {
      int numLines = ldrawFile.size(fileName);

      Where current(fileName,0);
      for (; current < numLines; current++) {
          QString line = ldrawFile.readLine(fileName,current.lineNumber);
          meta.parse(line,current);
        }
    } else {
      QFileInfo fileInfo(fileName);
      if (fileInfo.exists()) {
          QFile file(fileName);
          if ( ! file.open(QFile::ReadOnly | QFile::Text)) {
              QMessageBox::warning(NULL,
                                   QMessageBox::tr(VER_PRODUCTNAME_STR),
                                   QMessageBox::tr("Cannot read file %1:\n%2.")
                                   .arg(fileName)
                                   .arg(file.errorString()));
              return;
            }

          /* Read it in the first time to put into fileList in order of
         appearance */

          QTextStream in(&file);
          QStringList contents;
          Where       current(fileName,0);

          while ( ! in.atEnd()) {
              QString line = in.readLine(0);
              meta.parse(line,current);
              ++current;
            }
          file.close();
        }
    }
}

Where dummy;

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
 * This function applies buffer exchange and LPub's remove
 * meta commands before writing them out for the renderers to use.
 * This eliminates the need for ghosting parts removed by buffer
 * exchange
 */

void Gui::writeToTmp(const QString &fileName,
                     const QStringList &contents)
{
  QString fname = QDir::currentPath() + "/" + Paths::tmpDir + "/" + fileName;
  QFileInfo fileInfo(fname);
  if(!fileInfo.dir().exists()) {
     fileInfo.dir().mkpath(".");
    }
  QFile file(fname);
  if ( ! file.open(QFile::WriteOnly|QFile::Text)) {
      QMessageBox::warning(NULL,QMessageBox::tr("LPub3D"),
                           QMessageBox::tr("Failed to open %1 for writing: %2")
                           .arg(fname) .arg(file.errorString()));
    } else {
      QStringList csiParts;
      QHash<QString, QStringList> bfx;

      for (int i = 0; i < contents.size(); i++) {
          QString line = contents[i];
          QStringList tokens;

          split(line,tokens);
          if (tokens.size()) {
              if (tokens[0] != "0") {
                  csiParts << line;
                } else {
                  Meta meta;
                  Rc   rc;
                  Where here(fileName,i);
                  rc = meta.parse(line,here,false);

                  switch (rc) {

                    /* Buffer exchange */
                    case BufferStoreRc:
                      bfx[meta.bfx.value()] = csiParts;
                      break;
                    case BufferLoadRc:
                      csiParts = bfx[meta.bfx.value()];
                      break;

                      /* remove a group or all instances of a part type */
                    case GroupRemoveRc:
                    case RemoveGroupRc:
                    case RemovePartRc:
                    case RemoveNameRc:
                      {
                        QStringList newCSIParts;
                        if (rc == RemoveGroupRc) {
                            remove_group(csiParts,meta.LPub.remove.group.value(),newCSIParts);
                          } else if (rc == RemovePartRc) {
                            remove_parttype(csiParts, meta.LPub.remove.parttype.value(),newCSIParts);
                          } else {
                            remove_partname(csiParts, meta.LPub.remove.partname.value(),newCSIParts);
                          }
                        csiParts = newCSIParts;
                      }
                      break;
                    default:
                      break;
                    }
                }
            }
        }

      QTextStream out(&file);
      for (int i = 0; i < csiParts.size(); i++) {
          out << csiParts[i] << endl;
        }
      file.close();
    }
}

void Gui::writeToTmp()
{
  emit progressBarPermInitSig();
  emit progressPermRangeSig(1, ldrawFile._subFileOrder.size());
  emit progressPermMessageSig("Submodels...");
  emit messageSig(true, "Writing submodels to temp directory");

  bool    doFadeStep  = (page.meta.LPub.fadeStep.fadeStep.value() || Preferences::enableFadeStep);
  QString fadeColor   = LDrawColor::ldColorCode(page.meta.LPub.fadeStep.fadeColor.value());

  QStringList content;

  for (int i = 0; i < ldrawFile._subFileOrder.size(); i++) {

      QString fileName = ldrawFile._subFileOrder[i].toLower();

      emit progressPermSetValueSig(i);
      emit messageSig(true, "Writing submodel to temp directory: " + fileName);

      if (doFadeStep) {
          QString fadeFileName = fileName;
          QString extension = QFileInfo(fileName).suffix().toLower();
          bool ldr = extension == "ldr";
          bool mpd = extension == "mpd";
          bool dat = extension == "dat";
          if (ldr) {
              fadeFileName = fadeFileName.replace(".ldr","-fade.ldr");
            } else if (mpd) {
              fadeFileName = fadeFileName.replace(".mpd","-fade.mpd");
            } else if (dat) {
              fadeFileName = fadeFileName.replace(".dat","-fade.dat");
            }
          content = ldrawFile.contents(fileName);
          if (ldrawFile.changedSinceLastWrite(fileName)) {

              writeToTmp(fileName,content);
              content = fadeSubFile(content,fadeColor);

              /* Faded version of submodels */
              writeToTmp(fadeFileName,content);
            }
        } else {

          content = ldrawFile.contents(fileName);
          if (ldrawFile.changedSinceLastWrite(fileName)) {
              writeToTmp(fileName,content);
            }
        }
    }
  emit progressPermSetValueSig(ldrawFile._subFileOrder.size());
  emit removeProgressPermStatusSig();
  emit messageSig(true, "Submodels written to temp directory.");
}

/*
 * Fade writeToTmp content - make fade copies of submodel files.
 */
QStringList Gui::fadeSubFile(const QStringList &contents, const QString &color)
{
  QStringList fadeContents;

  if (contents.size() > 0) {

      QString edgeColor = "24";  // Internal Common Material Color (edge)
      QString fadeColor = color;
      QStringList argv;

      for (int index = 0; index < contents.size(); index++) {

          QString contentLine = contents[index];
          split(contentLine, argv);
          if (argv.size() == 15 && argv[0] == "1") {
              // set fade colour
              if (argv[1] != edgeColor){
                  argv[1] = fadeColor;}
              // process static colored parts
              QString fileNameStr = argv[argv.size()-1].toLower();
              if (FadeStepColorParts::isStaticColorPart(fileNameStr)){
                  fileNameStr = QDir::toNativeSeparators(fileNameStr.replace(".dat","-fade.dat"));
                }
              // process subfile naming
              if (ldrawFile.isSubmodel(fileNameStr)) {
                  QString extension = QFileInfo(fileNameStr).suffix().toLower();
                  bool ldr = extension == "ldr";
                  bool mpd = extension == "mpd";
                  bool dat = extension == "dat";
                  if (ldr) {
                      fileNameStr = fileNameStr.replace(".ldr","-fade.ldr");
                    } else if (mpd) {
                      fileNameStr = fileNameStr.replace(".mpd","-fade.mpd");
                    } else if (dat) {
                      fileNameStr = fileNameStr.replace(".dat","-fade.dat");
                    }
                }
              argv[argv.size()-1] = fileNameStr;
            } else if ((argv.size() == 8  && argv[0] == "2") ||
                       (argv.size() == 11 && argv[0] == "3") ||
                       (argv.size() == 14 && argv[0] == "4") ||
                       (argv.size() == 14 && argv[0] == "5")) {
              if (argv[1] != edgeColor){
                  argv[1] = fadeColor;}
            }
          contentLine = argv.join(" ");
          fadeContents  << contentLine;
        }
    } else {
      return contents;
    }
  return fadeContents;
}

/*
 * Process csiParts list - fade all non-current step-parts.
 */
QStringList Gui::fadeStep(const QStringList &csiParts, const int &stepNum,  Where &current) {

  QStringList fadeCsiParts;
  bool    doFadeStep  = (page.meta.LPub.fadeStep.fadeStep.value() || Preferences::enableFadeStep);

  if (csiParts.size() > 0 && stepNum > 1 && doFadeStep) {

      QString fadeColor   = LDrawColor::ldColorCode(page.meta.LPub.fadeStep.fadeColor.value());
      QString edgeColor   = "24";  // Internal Common Material Color (edge)
      int  fadePosition   = ldrawFile.getFadePosition(current.modelName);
      //qDebug() << "Model:" << current.modelName << ", Step:"  << stepNum << ", FadeStep Get Fade Position:" << fadePosition
      //         << ", CSI Size:" << csiParts.size() << ", Model Size:"  << ldrawFile.size(current.modelName);
      QStringList argv;

      for (int index = 0; index < csiParts.size(); index++) {

          QString csiLine = csiParts[index];
          if ((index + 1) <= fadePosition) {
              split(csiLine, argv);
              if (argv.size() == 15 && argv[0] == "1") {
                  // update fade colour
                  if (argv[1] != edgeColor){
                      argv[1] = fadeColor;}
                  // process color parts naming
                  QString fileNameStr = argv[argv.size()-1].toLower();

                  emit messageSig(true, "Do fadeStep for " + fileNameStr);

                  if (FadeStepColorParts::isStaticColorPart(fileNameStr)){
                      fileNameStr = QDir::toNativeSeparators(fileNameStr.replace(".dat","-fade.dat"));
                    }
                  // process subfile naming
                  if (ldrawFile.isSubmodel(fileNameStr)) {
                      QString extension = QFileInfo(fileNameStr).suffix().toLower();
                      bool ldr = extension == "ldr";
                      bool mpd = extension == "mpd";
                      bool dat = extension == "dat";
                      if (ldr) {
                          fileNameStr = fileNameStr.replace(".ldr","-fade.ldr");
                        } else if (mpd) {
                          fileNameStr = fileNameStr.replace(".mpd","-fade.mpd");
                        } else if (dat) {
                          fileNameStr = fileNameStr.replace(".dat","-fade.dat");
                        }
                    }
                  argv[argv.size()-1] = fileNameStr;
                } else if ((argv.size() == 8  && argv[0] == "2") ||
                           (argv.size() == 11 && argv[0] == "3") ||
                           (argv.size() == 14 && argv[0] == "4") ||
                           (argv.size() == 14 && argv[0] == "5")) {
                  if (argv[1] != edgeColor){
                      argv[1] = fadeColor;}
                }
              csiLine = argv.join(" ");
            }
          fadeCsiParts  << csiLine;
        }
    } else {
      ldrawFile.setFadePosition(current.modelName,csiParts.size());
      return csiParts;
    }
  ldrawFile.setFadePosition(current.modelName,fadeCsiParts.size());
  return fadeCsiParts;
}
