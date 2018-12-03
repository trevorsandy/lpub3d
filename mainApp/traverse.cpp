
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

#include "QsLog.h"

// Set to enable PageSize trace logging
#ifndef SIZE_DEBUG
//#define SIZE_DEBUG
#endif

/*********************************************
 *
 * remove_group
 *
 * this removes members of a group from the
 * ldraw file held in the the ldr string
 *
 ********************************************/

static void remove_group(
    QStringList  in,      // csiParts
    QString      group,   // steps->meta.LPub.remove.group.value()
    QStringList &out)     // newCSIParts
{
  QRegExp bgt(  "^\\s*0\\s+MLCAD\\s+BTG\\s+(.*)$");
  QRegExp ldcg( "^\\s*0\\s+!?LDCAD\\s+GROUP_NXT\\s+\\[ids=(\\d[^\\]]*)");
  QRegExp leogb("^\\s*0\\s+!?LEOCAD\\s+GROUP\\s+BEGIN\\s+Group\\s+(#\\d+)$",Qt::CaseInsensitive);
  QRegExp leoge("^\\s*0\\s+!?LEOCAD\\s+GROUP\\s+END$");

  bool leoRemove = false;
  int leoNest = 0;
  for (int i = 0; i < in.size(); i++) {
    QString line = in.at(i);

      // MLCad and LDCad Groups
      if (line.contains(bgt) ||
          line.contains(ldcg)) {
          if ((bgt.cap(bgt.captureCount()) == group) ||
              (ldcg.cap(ldcg.captureCount()) == group)) {
              i++;
            } else {
              out << line;
            }
      }
      // LeoCAD	Group Begin
      else
      if (line.contains(leogb)) {
          if ((leogb.cap(leogb.captureCount()) == group)){
            leoRemove = true;
            i++;
          }
          else
          if (leoRemove) {
              leoNest++;
              i++;
          }
          else {
             out << line;
          }
      }
      // LeoCAD	Group End
      else
      if (line.contains(leoge)) {
          if (leoRemove) {
              if (leoNest == 0) {
                leoRemove = false;
              } else {
                leoNest--;
              }
          }
          else {
              out << line;
          }
      }
      else
      if (leoRemove) {
             i++;
      }
      else {
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

int Gui::drawPage(
    LGraphicsView  *view,
    LGraphicsScene *scene,
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
    QStringList    &csiKeys,
    bool            unAssCallout,
    bool            calledOut)
{
  QStringList saveCsiParts;
  bool     global = true;
  QString  line, csiName;
  Callout *callout         = nullptr;
  Range   *range           = nullptr;
  Step    *step            = nullptr;
  bool     pliIgnore       = false;
  bool     partIgnore      = false;
  bool     synthBegin      = false;
  bool     multiStep       = false;
  bool     partsAdded      = false;
  bool     coverPage       = false;
  bool     bfxStore1       = false;
  bool     bfxLoad         = false;
  int      numLines        = ldrawFile.size(current.modelName);
  bool     firstStep       = true;
  bool     noStep          = false;
  bool     rotateIcon      = false;

  PagePointer *pagePointer= nullptr;
  QMap<Positions, PagePointer*> pagePointers;

  steps->isMirrored = isMirrored;
  steps->setTopOfSteps(current);

  QList<InsertMeta> inserts;

  Where topOfStep = current;
  Rc gprc = OkRc;
  Rc rc;

  page.coverPage = false;

  QStringList calloutParts;

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

  emit messageSig(LOG_STATUS, "Processing draw page for " + current.modelName + "...");

  /*
   * do until end of page
   */
  for ( ; current <= numLines; current++) {

      // load initial meta values

      Meta   &curMeta = callout ? callout->meta : steps->meta;

      QStringList tokens;

      // If we hit end of file we've got to note end of step

      if (current >= numLines) {
          line.clear();
          gprc = EndOfFileRc;
          tokens << "0";

          // not end of file, so get the next LDraw line

        } else {

          // read the line from the ldrawFile db

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

          /* since we have a part usage, we have a valid STEP */

          if (step == nullptr  && ! noStep) {
              if (range == nullptr) {
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
            } // STEP - Allocate STEP

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
            } // STEP - Process PLI parts

          /* if it is a called out sub-model, then process it */

          if (ldrawFile.isSubmodel(type) && callout && ! noStep) {

              CalloutBeginMeta::CalloutMode mode = callout->meta.LPub.callout.begin.value();

//              qDebug() << "CALLOUT MODE: " << (mode == CalloutBeginMeta::Unassembled ? "Unassembled" :
//                                               mode == CalloutBeginMeta::Rotated ? "Rotated" : "Assembled");

              // If callout is rotated or assembled then suppress rotate icon
              unAssCallout = (mode == CalloutBeginMeta::Unassembled);

              /* we are a callout, so gather all the steps within the callout */
              /* start with new meta, but no rotation step */

              QString thisType = type;

             /* t.s. Rotated or assembled callout here (treated like a submodel) */
              if (mode != CalloutBeginMeta::Unassembled) {
             /* So, we process these callouts in-line, not when we finally hit the STEP or
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

                  if (mode == CalloutBeginMeta::Rotated) {
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
                        csiKeys,
                        unAssCallout,
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
              emit messageSig(LOG_STATUS, "Processing " + current.modelName + "...");

            } // STEP - Process called out submodel

          if (step && steps->meta.LPub.subModel.show.value()) {
              bool calloutOk      = (calledOut ? unAssCallout : true ) &&
                                    (calledOut ? steps->meta.LPub.subModel.showSubmodelInCallout.value(): true);
              bool topModel       = (topLevelFile() == topOfStep.modelName);
              bool showTopModel   = (steps->meta.LPub.subModel.showTopModel.value());
              step->placeSubModel = (calloutOk && (stepNum == 1) && (!topModel || showTopModel));
          }

        }
      // STEP - Process line, triangle, or polygon
      else if (tokens.size() > 0 &&
                   (tokens[0] == "2" ||
                    tokens[0] == "3" ||
                    tokens[0] == "4" ||
                    tokens[0] == "5")) {

          csiParts << line;
          partsAdded = true;

          /* we've got a line, triangle or polygon, so add it to the list */
          /* and make sure we know we have a step */

          if (step == nullptr && ! noStep) {
              if (range == nullptr) {
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

        }
      // STEP - Process meta command
      else if ( (tokens.size() > 0 && tokens[0] == "0") || gprc == EndOfFileRc) {

          /* must be meta-command (or comment) */

          if (global && tokens.contains("!LPUB") && tokens.contains("GLOBAL")) {
              topOfStep = current;
            } else {
              global = false;
            }

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
              ldrawFile.setPrevStepPosition(current.modelName,csiParts.size());
              //qDebug() << "Model:" << current.modelName << ", Step:"  << stepNum << ", bfx Set Fade Position:" << csiParts.size();
              bfxLoad = true;
              break;

            case MLCadGroupRc:
            case LDCadGroupRc:
            case LeoCadGroupBeginRc:
            case LeoCadGroupEndRc:
              csiParts << line;
              break;

            case IncludeRc:
              include(curMeta);
              break;

              /* substitute part/parts with this */

            case PliBeginSub1Rc:
              if (pliIgnore) {
                  parseError("Nested PLI BEGIN/ENDS not allowed",current);
                }
              if (steps->meta.LPub.pli.show.value() &&
                  ! pliIgnore &&
                  ! partIgnore &&
                  ! synthBegin) {

                  SubData subData = curMeta.LPub.pli.begin.sub.value();
                  QString addPart = QString("1 0  0 0 0  0 0 0 0 0 0 0 0 0 %1") .arg(subData.part);
                  pliParts << Pli::partLine(addPart,current,curMeta);
                }

              if (step == nullptr && ! noStep) {
                  if (range == nullptr) {
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
                  parseError("Nested PLI BEGIN/ENDS not allowed",current);
                }
              if (steps->meta.LPub.pli.show.value() &&
                  ! pliIgnore &&
                  ! partIgnore &&
                  ! synthBegin) {

                  SubData subData = curMeta.LPub.pli.begin.sub.value();
                  QString addPart = QString("1 %1  0 0 0  0 0 0 0 0 0 0 0 0 %2") .arg(subData.color) .arg(subData.part);
                  pliParts << Pli::partLine(addPart,current,curMeta);
                }

              if (step == nullptr && ! noStep) {
                  if (range == nullptr) {
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
                  parseError("Nested PLI BEGIN/ENDS not allowed",current);
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
                  parseError("Nested BEGIN/ENDS not allowed",current);
                }
              partIgnore = true;
              break;

            case PartEndRc:
            case MLCadSkipEndRc:
              if (! partIgnore) {
                  parseError("Ignore ending with no ignore begin",current);
                }
              partIgnore = false;
              break;

            case SynthBeginRc:
              if (synthBegin) {
                  parseError("Nested LSynth BEGIN/ENDS not allowed",current);
                }
              synthBegin = true;
              break;

            case SynthEndRc:
              if ( ! synthBegin) {
                  parseError("LSynth ignore ending with no ignore begin",current);
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

                if (step == nullptr && ! noStep) {
                    if (range == nullptr) {
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
                  step = nullptr;
                  Reserve *reserve = new Reserve(current,steps->meta.LPub);
                  if (range == nullptr) {
                      range = newRange(steps,calledOut);
                      steps->append(range);
                    }
                  range->append(reserve);
                }
              break;

            case InsertFinalModelRc:
              {
                if (curMeta.LPub.fadeStep.fadeStep.value() || curMeta.LPub.highlightStep.highlightStep.value()){
                    // this is not a step but it's necessary to use the step object to place the model
                    // increment the step number down - so basically use previous number for step
                    // do this before creating the step so we can use in the file name during
                    // csi generation to indicate this step file is not an actual step - just a model display
                    stepNum--;
                    if (step == nullptr) {
                        if (range == nullptr) {
                            range = newRange(steps,calledOut);
                            steps->append(range);
                          }
                        step = new Step(topOfStep,
                                        range,
                                        stepNum,
                                        curMeta,
                                        calledOut,
                                        multiStep);

                        step->modelDisplayOnlyStep = true;

                        range->append(step);
                      }
                  }
              }
              break;

            case InsertCoverPageRc:
              {
                coverPage = true;
                page.coverPage = true;
                QRegExp backCoverPage("^\\s*0\\s+!LPUB\\s+.*BACK");
                if (line.contains(backCoverPage)){
                    page.backCover  = true;
                    page.frontCover = false;
                  } else {
                    page.frontCover = true;
                    page.backCover  = false;
                  }
                // nothing to display in 3D Window
                if (! exporting())
                  emit clearViewerWindowSig();
              }
            case InsertPageRc:
              {
                partsAdded = true;

                // nothing to display in 3D Window
                if (! exporting())
                  emit clearViewerWindowSig();
              }
              break;

            case InsertRc:
              {
                inserts.append(curMeta.LPub.insert);         // these are always placed before any parts in step
                insertData = curMeta.LPub.insert.value();

                if (insertData.type == InsertData::InsertRotateIcon) { // indicate that we have a rotate icon for this step

 //                   qDebug() << "CALLED OUT: " << calledOut << " SUPRESS ROTATE ICON: " << unAssCallout;

                    if (calledOut && unAssCallout) {
                        rotateIcon = true;
                      } else {
                        rotateIcon = false;
                      }
                  }
                if (insertData.type == InsertData::InsertBom){
                    // nothing to display in 3D Window
                    if (! exporting())
                      emit clearViewerWindowSig();
                  }
              }
              break;

            case PagePointerRc:
              {
                if (pagePointer){
                    parseError("Nested page pointers not allowed within the same file",current);
                  } else {
                    Positions position;
                    bool pointerExist = false;
                    if (curMeta.LPub.pagePointer.pointer.value().rectPlacement == TopInside) {
                        position = PP_TOP;
                        if (pagePointers[position])
                          pointerExist = true;
                      }
                    else if (curMeta.LPub.pagePointer.pointer.value().rectPlacement == BottomInside) {
                        position = PP_BOTTOM;
                        if (pagePointers[position])
                          pointerExist = true;
                      }
                    else if (curMeta.LPub.pagePointer.pointer.value().rectPlacement == LeftInside) {
                        position = PP_LEFT;
                        if (pagePointers[position])
                          pointerExist = true;
                      }
                    else if (curMeta.LPub.pagePointer.pointer.value().rectPlacement == RightInside) {
                        position = PP_RIGHT;
                        if (pagePointers[position])
                          pointerExist = true;
                      }

                    if (pointerExist) {
                        pagePointers[position]->appendPointer(current,curMeta.LPub.pagePointer.pointer);
                      } else {
                        pagePointer = new PagePointer(curMeta,view);
                        pagePointer->parentStep = step;
                        pagePointer->setTopOfPagePointer(current);
                        pagePointer->setBottomOfPagePointer(current);
                        if (multiStep){
                            pagePointer->parentRelativeType = StepGroupType;
                          } else if (calledOut){
                            pagePointer->parentRelativeType = CalloutType;
                          } else {
                            pagePointer->parentRelativeType = step->relativeType;
                          }
                        PlacementMeta pointerPlacement;
                        pointerPlacement.setValue(curMeta.LPub.pagePointer.pointer.value().rectPlacement, PageType);
                        pagePointer->placement = pointerPlacement;
                        pagePointer->appendPointer(current,curMeta.LPub.pagePointer.pointer);
                        pagePointers.insert(position,pagePointer);
                        pagePointer = nullptr;
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
                  range = nullptr;
                  step = nullptr;
                }
              break;

            case CalloutPointerRc:
              if (callout) {
                  callout->appendPointer(current,curMeta.LPub.callout.pointer);
                }
              break;

            case CalloutEndRc:
              if ( ! callout) {
                  parseError("CALLOUT END without a CALLOUT BEGIN",current);
                }
              else
              if (! step) {
                  parseError("CALLOUT does not contain a valid STEP",current);
               }
              else
                {
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
                  callout = nullptr;
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
                  range = nullptr;
                  step = nullptr;
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

                      emit messageSig(LOG_STATUS, "Add PLI images for multi-step page " + current.modelName);

                      steps->pli.sizePli(&steps->stepGroupMeta, StepGroupType, false);
                    }
                  pliParts.clear();

                  /* this is a page we're supposed to process */

                  steps->placement = steps->meta.LPub.multiStep.placement;

                  showLine(steps->topOfSteps());

                  bool endOfSubmodel = stepNum - 1 >= ldrawFile.numSteps(current.modelName);

                  // Get submodel instance count
                  int  instances = ldrawFile.instances(current.modelName, isMirrored);
                  if (instances > 1 && ! steps->meta.LPub.mergeInstanceCount.value()) {
                      MetaItem mi;
                      instances = mi.countInstancesInStep(&steps->meta, current.modelName);
                  }

                  Page *page = dynamic_cast<Page *>(steps);
                  if (page) {
                      page->inserts      = inserts;
                      page->instances    = instances;
                      page->pagePointers = pagePointers;
                    }

                  emit messageSig(LOG_STATUS, "Add CSI images for multi-step page " + current.modelName);

                  if (renderer->useLDViewSCall() && ldrStepFiles.size() > 0){
                      QElapsedTimer timer;
                      timer.start();

                      QString empty("");
                      int rc = renderer->renderCsi(empty,ldrStepFiles,csiKeys,empty,steps->meta);
                      if (rc != 0) {
                          emit messageSig(LOG_ERROR,QMessageBox::tr("Render CSI images failed."));
                          return rc;
                        }

                      emit gui->messageSig(LOG_INFO, qPrintable(
                                          QString("%1 CSI (Single Call) render took "
                                                  "%2 milliseconds to render %3 [Step %4] %5 "
                                                  "for %6 step group on page %7.")
                                             .arg(Render::getRenderer())
                                             .arg(timer.elapsed())
                                             .arg(ldrStepFiles.size())
                                             .arg(stepNum)
                                             .arg(ldrStepFiles.size() == 1 ? "image" : "images")
                                             .arg(calledOut ? "called out," : "simple,")
                                             .arg(stepPageNum)));
                    }

                  addGraphicsPageItems(steps, coverPage, endOfSubmodel, view, scene, printing);

                  return HitEndOfPage;
                }
              inserts.clear();
              pagePointers.clear();
              break;

            case NoStepRc:
              noStep = true;
              break;

              /* we've hit some kind of step, or implied step and end of file */
            case EndOfFileRc:
            case RotStepRc:
            case StepRc:

              // STEP - special case of no parts added, but BFX load and not NOSTEP
              if (! partsAdded && bfxLoad && ! noStep) {
                  if (step == nullptr) {
                      if (range == nullptr) {
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

                  emit messageSig(LOG_INFO, "Processing CSI bfx load special case for " + topOfStep.modelName + "...");
                  csiName = step->csiName();
                  (void) step->createCsi(
                        isMirrored ? addLine : "1 color 0 0 0 1 0 0 0 1 0 0 0 1 foo.ldr",
                        saveCsiParts = configureModelStep(csiParts, stepNum, topOfStep),
                        &step->csiPixmap,
                        steps->meta,
                        bfxLoad);

                  if (renderer->useLDViewSCall() && ! step->ldrName.isNull()) {
                      ldrStepFiles << step->ldrName;
                      csiKeys << step->csiKey; // No parts to process
                      //qDebug() << "CSI ldr file #"<< ldrStepFiles.count() <<"added: " << step->ldrName;
                      //qDebug() << "CSI key #"<< csiKeys.count() <<"added: " << step->csiKey;
                    }

                  partsAdded = true; // OK, so this is a lie, but it works
                }

              // STEP - normal case of parts added, and not NOSTEP
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
                  } else if ( ! multiStep && ! calledOut && steps->meta.LPub.stepPli.perStep.value()) {
                      pliPerStep = true;
                  } else {
                      pliPerStep = false;
                  }

                  if (step) {
                      Page *page = dynamic_cast<Page *>(steps);
                      if (page) {
                          page->inserts              = inserts;
                          page->pagePointers         = pagePointers;
                          page->modelDisplayOnlyStep = step->modelDisplayOnlyStep;
                      }

                      PlacementType relativeType;
                      if (pliPerStep) {
                          if (multiStep) {
                              relativeType = StepGroupType;
                          } else if (calledOut) {
                              relativeType = CalloutType;
                          } else {
                              relativeType = SingleStepType;
                          }

                          step->pli.setParts(pliParts,steps->meta);
                          pliParts.clear();

                          emit messageSig(LOG_STATUS, "Add step PLI for " + topOfStep.modelName + "...");

                          step->pli.sizePli(&steps->meta,relativeType,pliPerStep);
                      }

                      if (step->placeSubModel){
                          emit messageSig(LOG_INFO, "Set first step submodel display for " + topOfStep.modelName + "...");

                          // Get submodel instance count
                          int  instances = 0;
                          if ( ! multiStep && ! calledOut) {
                              if (instances > 1 && ! steps->meta.LPub.mergeInstanceCount.value()) {
                                  MetaItem mi;
                                  instances = mi.countInstancesInStep(&steps->meta, current.modelName);
                              } else {
                                  instances = ldrawFile.instances(current.modelName, isMirrored);
                              }
                              relativeType = SingleStepType;
                          } else {
                              MetaItem mi;
                              instances = mi.countInstancesInBlock(&steps->meta, current.modelName,CalloutMask|StepGroupMask);
                              if (multiStep) {
                                  relativeType = StepGroupType;
                              } else if (calledOut) {
                                  relativeType = CalloutType;
                              }
                          }

                          steps->meta.LPub.subModel.instance.setValue(instances);
                          step->subModel.setSubModel(current.modelName,steps->meta);
                          if (step->subModel.sizeSubModel(&steps->meta,relativeType) != 0)
                              emit messageSig(LOG_ERROR, "Failed to set first step submodel display for " + topOfStep.modelName + "...");
                      } else {
                          step->subModel.clear();
                      }

                      if (rotateIcon) {
                          step->placeRotateIcon = true;
                      }

                      emit messageSig(LOG_STATUS, "Processing step (CSI) for " + topOfStep.modelName + "...");
                      csiName = step->csiName();
                      int rc = step->createCsi(
                                  isMirrored ? addLine : "1 color 0 0 0 1 0 0 0 1 0 0 0 1 foo.ldr",
                                  saveCsiParts = configureModelStep(csiParts, step->modelDisplayOnlyStep ? -1 : stepNum, topOfStep),
                                  &step->csiPixmap,
                                  steps->meta);

                      if (rc) {
                          emit messageSig(LOG_ERROR, QMessageBox::tr("Failed to create CSI file."));
                          return rc;
                      }

                      if (renderer->useLDViewSCall() && ! step->ldrName.isNull()) {
                          ldrStepFiles << step->ldrName;
                          csiKeys << step->csiKey;
                          //qDebug() << "CSI ldr file #"<< ldrStepFiles.count() <<"added: " << step->ldrName;
                          //qDebug() << "CSI key #"<< csiKeys.count() <<"added: " << step->csiKey;
                      }

                  } else {

                      if (pliPerStep) {
                          pliParts.clear();
                      }

                      // Only pages or step can have inserts and pointers... not callouts
                      if ( ! multiStep && ! calledOut) {
                          Page *page = dynamic_cast<Page *>(steps);
                          if (page) {
                              page->inserts      = inserts;
                              page->pagePointers = pagePointers;
                          }
                      }
                  }

                  // STEP - Simple STEP
                  if ( ! multiStep && ! calledOut) {

                      steps->placement = steps->meta.LPub.assem.placement;

                      showLine(topOfStep);

                      int  numSteps = ldrawFile.numSteps(current.modelName);

                      bool endOfSubmodel = numSteps == 0 || stepNum >= numSteps;

                      // Get submodel instance count
                      int  instances = ldrawFile.instances(current.modelName, isMirrored);
                      if (instances > 1 && ! steps->meta.LPub.mergeInstanceCount.value()) {
                          MetaItem mi;
                          instances = mi.countInstancesInStep(&steps->meta, current.modelName);
                      }

                      Page *page = dynamic_cast<Page *>(steps);
                      if (page && instances > 1) {
                          page->instances = instances;

                          if (! steps->meta.LPub.stepPli.perStep.value()) {

                              PlacementType relativeType = SingleStepType;

                              QStringList instancesPliParts;
                              if (pliParts.size() > 0) {
                                  for (int index = 0; index < pliParts.size(); index++) {
                                      QString pliLine = pliParts[index];
                                      for (int i = 0; i < instances; i++) {
                                          instancesPliParts << pliLine;
                                      }
                                  }
                              }

                              step->pli.setParts(instancesPliParts,steps->meta);
                              instancesPliParts.clear();
                              pliParts.clear();

                              emit messageSig(LOG_STATUS, "Add PLI images for single-step page...");

                              step->pli.sizePli(&steps->meta,relativeType,pliPerStep);
                          }
                      }

                      emit messageSig(LOG_STATUS, "Add CSI image for single-step page...");

                      if (renderer->useLDViewSCall() && ldrStepFiles.size() > 0){

                          QElapsedTimer timer;
                          timer.start();
                          QString empty("");
                          int rc = renderer->renderCsi(empty,ldrStepFiles,csiKeys,empty,steps->meta);
                          if (rc != 0) {
                              emit messageSig(LOG_ERROR,QMessageBox::tr("Render CSI images failed."));
                              return rc;
                          }

                          emit gui->messageSig(LOG_INFO,
                                               QString("%1 CSI (Single Call) render took "
                                                       "%2 milliseconds to render %3 [Step %4] %5 for %6 "
                                                       "single step on page %7.")
                                               .arg(Render::getRenderer())
                                               .arg(timer.elapsed())
                                               .arg(ldrStepFiles.size())
                                               .arg(stepNum)
                                               .arg(ldrStepFiles.size() == 1 ? "image" : "images")
                                               .arg(calledOut ? "called out," : "simple,")
                                               .arg(stepPageNum));
                      }

                      addGraphicsPageItems(steps,coverPage,endOfSubmodel,view,scene,printing);
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
                  //                unAssCallout = false;
                  step = nullptr;
                  bfxStore2 = bfxStore1;
                  bfxStore1 = false;
                  bfxLoad = false;
              }

              if ( ! multiStep) {
                  inserts.clear();
                  pagePointers.clear();
                }
              steps->setBottomOfSteps(current);
              noStep = false;
              break;
            case RangeErrorRc:
              showLine(current);
              emit gui->messageSig(LOG_ERROR,
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
          emit gui->messageSig(LOG_ERROR,
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
    LGraphicsScene *scene,
    int            &pageNum,      //maxPages
    QString const  &addLine,
    Where          &current,
    PgSizeData     &pageSize,
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
  bool callout    = false;
  bool noStep     = false;
  bool noStep2    = false;
  bool stepGroupBfxStore2 = false;
  bool pageSizeUpdate     = false;

  QStringList bfxParts;
  QStringList saveBfxParts;
  QStringList ldrStepFiles;
  QStringList csiKeys;
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

  emit messageSig(LOG_STATUS, "Processing find page for " + current.modelName + "...");

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
              CalloutBeginMeta::CalloutMode mode = meta.LPub.callout.begin.value();

              // if submodel or callout treated as part (added to parent as assembled image)
              if (contains && (!callout || (callout && mode != CalloutBeginMeta::Unassembled))) {

                  bool rendered = ldrawFile.rendered(type,ldrawFile.mirrored(token));
                  if (! meta.LPub.mergeInstanceCount.value())
                      rendered = ldrawFile.rendered(type,ldrawFile.mirrored(token)) && stepNumber == renderStepNum;

//                  logTrace() << QString("Submodel %1 in parent %4 at line %3, step %5 %2")
//                                .arg(type)
//                                .arg(rendered?"is RENDERED":"is not rendered.")
//                                .arg(current.lineNumber).arg(current.modelName)
//                                .arg(stepNumber);

                  if ( ! rendered && (! bfxStore2 || ! bfxParts.contains(token[1]+type))) {

                      // store the step where the submodel is rendered for later comparison
                      renderStepNum = stepNumber;

                      isMirrored = ldrawFile.mirrored(token);

                      // can't be a callout
                      SubmodelStack tos(current.modelName,current.lineNumber,stepNumber);
                      meta.submodelStack << tos;
                      Where current2(type,0);

                      ldrawFile.setModelStartPageNumber(current2.modelName,pageNum);

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
                          logDebug() << "SM: Saving    Default Page size info at PageNumber:" << pageNum
                                     << "W:"    << pageSize2.sizeW << "H:"    << pageSize2.sizeH
                                     << "O:"    <<(pageSize2.orientation == Portrait ? "Portrait" : "Landscape")
                                     << "ID:"   << pageSize2.sizeID
                                     << "Model:" << current.modelName;
#endif
                        }

                      findPage(view,scene,pageNum,line,current2,pageSize,isMirrored,meta,printing);
                      saveStepPageNum = stepPageNum;
                      meta.submodelStack.pop_back();
                      meta.rotStep = saveRotStep2;    // restore old rotstep

                      if (exporting()) {
                          pageSizes.remove(DEF_SIZE);
                          pageSizes.insert(DEF_SIZE,pageSize2);  // restore old Default pageSize information
#ifdef SIZE_DEBUG
                          logDebug() << "SM: Restoring Default Page size info at PageNumber:" << pageNum
                                     << "W:"    << pageSizes[DEF_SIZE].sizeW << "H:"    << pageSizes[DEF_SIZE].sizeH
                                     << "O:"    << (pageSizes[DEF_SIZE].orientation == Portrait ? "Portrait" : "Landscape")
                                     << "ID:"   << pageSizes[DEF_SIZE].sizeID
                                     << "Model:" << current.modelName;
#endif
                        }
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
                      savePrevStepPosition = saveCsiParts.size();
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
                                      ldrStepFiles,
                                      csiKeys);

                      saveCurrent.modelName.clear();
                      saveCsiParts.clear();
                    }
                  if (exporting()) {
                      pageSizes.remove(pageNum);
                      if (pageSizeUpdate) {
                          pageSizeUpdate = false;
                          pageSizes.insert(pageNum,pageSize);
#ifdef SIZE_DEBUG
                          logTrace() << "SG: Inserting New Page size info     at PageNumber:" << pageNum
                                     << "W:"    << pageSize.sizeW << "H:"    << pageSize.sizeH
                                     << "O:"    <<(pageSize.orientation == Portrait ? "Portrait" : "Landscape")
                                     << "ID:"   << pageSize.sizeID
                                     << "Model:" << current.modelName;
#endif
                        } else {
                          pageSizes.insert(pageNum,pageSizes[DEF_SIZE]);
#ifdef SIZE_DEBUG
                          logTrace() << "SG: Inserting Default Page size info at PageNumber:" << pageNum
                                     << "W:"    << pageSizes[DEF_SIZE].sizeW << "H:"    << pageSizes[DEF_SIZE].sizeH
                                     << "O:"    << (pageSizes[DEF_SIZE].orientation == Portrait ? "Portrait" : "Landscape")
                                     << "ID:"   << pageSizes[DEF_SIZE].sizeID
                                     << "Model:" << current.modelName;
#endif
                        }
                    } // exporting
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
                      saveCurrent = current;
                      saveRotStep = meta.rotStep;
                    }
                  if ( ! stepGroup) {
                      if (pageNum == displayPageNum) {
                          csiParts.clear();
                          savePrevStepPosition = saveCsiParts.size();
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
                                          ldrStepFiles,
                                          csiKeys);

                          saveCurrent.modelName.clear();
                          saveCsiParts.clear();
                        }
                      if (exporting()) {
                          pageSizes.remove(pageNum);
                          if (pageSizeUpdate) {
                              pageSizeUpdate = false;
                              pageSizes.insert(pageNum,pageSize);
#ifdef SIZE_DEBUG
                              logTrace() << "ST: Inserting New Page size info     at PageNumber:" << pageNum
                                         << "W:"    << pageSize.sizeW << "H:"    << pageSize.sizeH
                                         << "O:"    <<(pageSize.orientation == Portrait ? "Portrait" : "Landscape")
                                         << "ID:"   << pageSize.sizeID
                                         << "Model:" << current.modelName;
#endif
                            } else {
                              pageSizes.insert(pageNum,pageSizes[DEF_SIZE]);
#ifdef SIZE_DEBUG
                              logTrace() << "ST: Inserting Default Page size info at PageNumber:" << pageNum
                                         << "W:"    << pageSizes[DEF_SIZE].sizeW << "H:"    << pageSizes[DEF_SIZE].sizeH
                                         << "O:"    << (pageSizes[DEF_SIZE].orientation == Portrait ? "Portrait" : "Landscape")
                                         << "ID:"   << pageSizes[DEF_SIZE].sizeID
                                         << "Model:" << current.modelName;
#endif
                            }
                        } // exporting
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
            case LDCadGroupRc:
            case LeoCadGroupBeginRc:
            case LeoCadGroupEndRc:
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
            case PageSizeRc:
              {
                if (exporting()) {
                    pageSizeUpdate  = true;

                    pageSize.sizeW  = meta.LPub.page.size.valueInches(0);
                    pageSize.sizeH  = meta.LPub.page.size.valueInches(1);
                    pageSize.sizeID = meta.LPub.page.size.valueSizeID();

                    pageSizes.remove(DEF_SIZE);
                    pageSizes.insert(DEF_SIZE,pageSize);
#ifdef SIZE_DEBUG                
                    logTrace() << "1. New Page Size entry for Default  at PageNumber:" << pageNum
                               << "W:"  << pageSize.sizeW << "H:"    << pageSize.sizeH
                               << "O:"  << (pageSize.orientation == Portrait ? "Portrait" : "Landscape")
                               << "ID:" << pageSize.sizeID
                               << "Model:" << current.modelName;
#endif
                  }
              }
              break;
            case PageOrientationRc:
              {
                if (exporting()){
                    pageSizeUpdate      = true;

                    if (pageSize.sizeW == 0)
                      pageSize.sizeW    = pageSizes[DEF_SIZE].sizeW;
                    if (pageSize.sizeH == 0)
                      pageSize.sizeH    = pageSizes[DEF_SIZE].sizeH;
                    if (pageSize.sizeID.isEmpty())
                      pageSize.sizeID   = pageSizes[DEF_SIZE].sizeID;
                    pageSize.orientation= meta.LPub.page.orientation.value();

                    pageSizes.remove(DEF_SIZE);
                    pageSizes.insert(DEF_SIZE,pageSize);
#ifdef SIZE_DEBUG
                    logTrace() << "1. New Orientation entry for Default at PageNumber:" << pageNum
                               << "W:"  << pageSize.sizeW << "H:"    << pageSize.sizeH
                               << "O:"  << (pageSize.orientation == Portrait ? "Portrait" : "Landscape")
                               << "ID:" << pageSize.sizeID
                               << "Model:" << current.modelName;
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

  if (partsAdded && ! noStep) {
      if (pageNum == displayPageNum) {

          savePrevStepPosition = saveCsiParts.size();
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
                          ldrStepFiles,
                          csiKeys);
        }
      if (exporting()) {
          pageSizes.remove(pageNum);
          if (pageSizeUpdate) {
              pageSizeUpdate = false;
              pageSizes.insert(pageNum,pageSize);
#ifdef SIZE_DEBUG
              logTrace() << "PG: Inserting New Page size info     at PageNumber:" << pageNum
                         << "W:"    << pageSize.sizeW << "H:"    << pageSize.sizeH
                         << "O:"    <<(pageSize.orientation == Portrait ? "Portrait" : "Landscape")
                         << "ID:"   << pageSize.sizeID
                         << "Model:" << current.modelName;
#endif
            } else {
              pageSizes.insert(pageNum,pageSizes[DEF_SIZE]);
#ifdef SIZE_DEBUG
              logTrace() << "PG: Inserting Default Page size info at PageNumber:" << pageNum
                         << "W:"    << pageSizes[DEF_SIZE].sizeW << "H:"    << pageSizes[DEF_SIZE].sizeH
                         << "O:"    << (pageSizes[DEF_SIZE].orientation == Portrait ? "Portrait" : "Landscape")
                         << "ID:"   << pageSizes[DEF_SIZE].sizeID
                         << "Model:" << current.modelName;
#endif
            }
        } // exporting
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
            case LDCadGroupRc:
            case LeoCadGroupBeginRc:
            case LeoCadGroupEndRc:
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
      PgSizeData empty1;
      stepPageNum = 1;
      findPage(KpageView,KpageScene,maxPages,empty,current,empty1,false,meta,false);
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
    bool            printing)
{
  QApplication::setOverrideCursor(Qt::WaitCursor);

  ldrawFile.unrendered();
  ldrawFile.countInstances();
  writeToTmp();
  Where       current(ldrawFile.topLevelFile(),0);
  maxPages    = 1;
  stepPageNum = 1;
  ldrawFile.setModelStartPageNumber(current.modelName,maxPages);
  //logTrace() << "SET INITIAL Model: " << current.modelName << " @ Page: " << maxPages;
  QString empty;
  Meta    meta;
  firstStepPageNum = -1;
  lastStepPageNum  = -1;
  renderStepNum    = 0;
  savePrevStepPosition = 0;

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

  findPage(view,scene,maxPages,empty,current,pageSize,false,meta,printing);
  topOfPages.append(current);
  maxPages--;

  QString string = QString("%1 of %2") .arg(displayPageNum) .arg(maxPages);
  if (! exporting())
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
              gui->insertLine(current,empty,nullptr);
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
              QMessageBox::warning(nullptr,
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
      QMessageBox::warning(nullptr,QMessageBox::tr("LPub3D"),
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
              } else if (tokens.size() == 11 &&
                          tokens[0] == "0"    &&
                          tokens[1] == "!COLOUR") {
                 csiParts << line;
              } else if ((tokens.size() == 2 || tokens.size() == 3) &&
                         tokens[0] == "0"    &&
                        (tokens[1] == "!FADE")) {
                csiParts << line;
             } else if ((tokens.size() == 2 || tokens.size() == 4) &&
                          tokens[0] == "0"    &&
                         (tokens[1] == "!SILHOUETTE")) {
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

      // Here we override the removal of these attributes for the 3DViewer
      QString modelName = QFileInfo(fileName).baseName().toLower();
      modelName = modelName.replace(modelName.at(0),modelName.at(0).toUpper());
      csiParts.prepend(QString("0 !LEOCAD MODEL NAME %1").arg(modelName));
      csiParts.prepend(QString("0 Name: %1").arg(fileName));
      csiParts.prepend(QString("0 %1").arg(modelName));

      QTextStream out(&file);
      for (int i = 0; i < csiParts.size(); i++) {
          out << csiParts[i] << endl;
        }
      file.close();
    }
}

void Gui::writeToTmp()
{
  if (Preferences::modeGUI && ! exporting()) {
      emit progressBarPermInitSig();
      emit progressPermRangeSig(1, ldrawFile._subFileOrder.size());
      emit progressPermMessageSig("Writing submodels...");
    }
  emit messageSig(LOG_STATUS, "Writing submodels to temp directory...");

  bool upToDate = true;
  bool doFadeStep  = page.meta.LPub.fadeStep.fadeStep.value();
  bool doHighlightStep = page.meta.LPub.highlightStep.highlightStep.value() && !suppressColourMeta();

  QString fadeColor = LDrawColor::ldColorCode(page.meta.LPub.fadeStep.fadeColor.value());

  QStringList content, configuredContent;

  for (int i = 0; i < ldrawFile._subFileOrder.size(); i++) {

      QString fileName = ldrawFile._subFileOrder[i].toLower();

      if (Preferences::modeGUI && ! exporting())
        emit progressPermSetValueSig(i);

      content = ldrawFile.contents(fileName);

      if (ldrawFile.changedSinceLastWrite(fileName)) {
          // write normal submodels...
          upToDate = false;
          emit messageSig(LOG_STATUS, "Writing submodel to temp directory: " + fileName);
          writeToTmp(fileName,content);

          // capture file name extensions
          QString extension = QFileInfo(fileName).suffix().toLower();
          bool ldr = extension == "ldr";
          bool mpd = extension == "mpd";
          bool dat = extension == "dat";

          // write configured (Fade) submodels
          if (doFadeStep) {
            QString fadeFileName = fileName;
            if (ldr) {
              fadeFileName = fadeFileName.replace(".ldr", "-fade.ldr");
            } else if (mpd) {
              fadeFileName = fadeFileName.replace(".mpd", "-fade.mpd");
            } else if (dat) {
              fadeFileName = fadeFileName.replace(".dat", "-fade.dat");
            }
            /* Faded version of submodels */
            emit messageSig(LOG_STATUS, "Writing fade submodels to temp directory: " + fadeFileName);
            configuredContent = configureModelSubFile(content, fadeColor, FADE_PART);
            writeToTmp(fadeFileName,configuredContent);
          }
          // write configured (Highlight) submodels
          if (doHighlightStep) {
            QString highlightFileName = fileName;
            if (ldr) {
              highlightFileName = highlightFileName.replace(".ldr", "-highlight.ldr");
            } else if (mpd) {
              highlightFileName = highlightFileName.replace(".mpd", "-highlight.mpd");
            } else if (dat) {
              highlightFileName = highlightFileName.replace(".dat", "-highlight.dat");
            }
            /* Highlighted version of submodels */
            emit messageSig(LOG_STATUS, "Writing highlight submodel to temp directory: " + highlightFileName);
            configuredContent = configureModelSubFile(content, fadeColor, HIGHLIGHT_PART);
            writeToTmp(highlightFileName,configuredContent);
          }
      }
  }

  bool generateSubModelImages = Preferences::modeGUI &&
                                gApplication->mPreferences.mViewPieceIcons &&
                                ! submodelIconsLoaded;
  if (generateSubModelImages) {
      if (Preferences::modeGUI && ! exporting())
          emit progressPermSetValueSig(ldrawFile._subFileOrder.size());

      // generate submodel icons...
      emit messageSig(LOG_INFO_STATUS, "Creating submodel icons...");
      Pli pli;
      int rc = pli.createSubModelIcons();
      if (rc == 0)
          gMainWindow->mSubmodelIconsLoaded = submodelIconsLoaded = true;
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
}

/*
 * Configure writeToTmp content - make fade or highlight copies of submodel files.
 */
QStringList Gui::configureModelSubFile(const QStringList &contents, const QString &fadeColour, const PartType partType)
{
  QString nameMod, colourPrefix;
  if (partType == FADE_PART){
    nameMod = LPUB3D_COLOUR_FADE_SUFFIX;
    colourPrefix = LPUB3D_COLOUR_FADE_PREFIX;
  } else if (partType == HIGHLIGHT_PART) {
    nameMod = LPUB3D_COLOUR_HIGHLIGHT_SUFFIX;
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
              // process static colored parts
              QString fileNameStr = QString(argv[argv.size()-1]).toLower();
              if (ldrawColourParts.isLDrawColourPart(fileNameStr)){
                  fileNameStr = QDir::toNativeSeparators(fileNameStr.replace(".dat", "-" + nameMod + ".dat"));
                }
              // process subfile naming
              if (ldrawFile.isSubmodel(fileNameStr)) {
                  QString extension = QFileInfo(fileNameStr).suffix().toLower();
                  bool ldr = extension == "ldr";
                  bool mpd = extension == "mpd";
                  bool dat = extension == "dat";
                  if (ldr) {
                      fileNameStr = fileNameStr.replace(".ldr", "-" + nameMod + ".ldr");
                    } else if (mpd) {
                      fileNameStr = fileNameStr.replace(".mpd", "-" + nameMod + ".mpd");
                    } else if (dat) {
                      fileNameStr = fileNameStr.replace(".dat", "-" + nameMod + ".dat");
                    }
                }
              argv[argv.size()-1] = fileNameStr;
            }
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
  bool FadeMetaAdded = false;
  bool SilhouetteMetaAdded = false;

  if (csiParts.size() > 0 && stepNum > 1) {

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

          bool ldr = false, mpd = false, dat= false;
          bool type_1_line = false;
          bool type_1_5_line = false;
          bool is_colour_part = false;
          bool is_submodel_file = false;

          int updatePosition = index+1;
          QString fileNameStr;
          QString csiLine = csiParts[index];
          split(csiLine, argv);

          // determine line type
          if (argv.size() && argv[0].size() == 1 &&
              argv[0] >= "1" && argv[0] <= "5") {
              type_1_5_line = true;
              if (argv.size() == 15 && argv[0] == "1")
                  type_1_line = true;
          }

          if (type_1_line){
              // process color parts naming
              fileNameStr = argv[argv.size()-1].toLower();

              // check if is color part
              is_colour_part = ldrawColourParts.isLDrawColourPart(fileNameStr);

              //if (is_colour_part)
              //    emit messageSig(LOG_NOTICE, "Static color part - " + fileNameStr);
          }

          // check if is submodel
          if (ldrawFile.isSubmodel(fileNameStr)) {
                 is_submodel_file = true;
                 QString extension = QFileInfo(fileNameStr).suffix().toLower();
                 ldr = extension == "ldr";
                 mpd = extension == "mpd";
                 dat = extension == "dat";
          }

          // write fade step entries
          if (doFadeStep && (updatePosition <= prevStepPosition)) {
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
                        if (is_colour_part)
                               fileNameStr = QDir::toNativeSeparators(fileNameStr.replace(".dat", "-fade.dat"));
                        // process subfiles naming
                        if (is_submodel_file) {
                               if (ldr) {
                                 fileNameStr = fileNameStr.replace(".ldr", "-fade.ldr");
                               } else if (mpd) {
                                 fileNameStr = fileNameStr.replace(".mpd", "-fade.mpd");
                               } else if (dat) {
                                 fileNameStr = fileNameStr.replace(".dat", "-fade.dat");
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
                        if (is_colour_part)
                               fileNameStr = QDir::toNativeSeparators(fileNameStr.replace(".dat", "-highlight.dat"));
                        // process subfiles naming
                        if (is_submodel_file) {
                               if (ldr) {
                                 fileNameStr = fileNameStr.replace(".ldr", "-highlight.ldr");
                               } else if (mpd) {
                                 fileNameStr = fileNameStr.replace(".mpd", "-highlight.mpd");
                               } else if (dat) {
                                 fileNameStr = fileNameStr.replace(".dat", "-highlight.dat");
                               }
                        }
                        // assign fade part name
                        argv[argv.size()-1] = fileNameStr;
                  }
              }
          }

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
