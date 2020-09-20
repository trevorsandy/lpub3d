 
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
 * The class described in this file is the graphical representation of
 * a step's construction step image (CSI), or assembly image.
 *
 * Please see lpub.h for an overall description of how the files in LPub
 * make up the LPub program.
 *
 ***************************************************************************/
#include <QMenu>
#include <QAction>
#include <QGraphicsRectItem>
#include <QGraphicsSceneContextMenuEvent>

#include "lpub.h"
#include "step.h"
#include "ranges.h"
#include "ranges_element.h"
#include "callout.h"
#include "csiitem.h"
#include "csiannotation.h"
#include "metaitem.h"
#include "commonmenus.h"
#include "pointer.h"

CsiItem::CsiItem(
  Step          *_step,
  Meta          *_meta,
  QPixmap       &pixmap,
  int            _submodelLevel,
  QGraphicsItem *parent,
  PlacementType  _parentRelativeType) :
  isHovered(false),
  mouseIsDown(false)
{
  step               = _step;
  meta               = _meta;
  submodelLevel      = _submodelLevel;
  parentRelativeType = _parentRelativeType;
  hiddenAnnotations  = false;

  setPixmap(pixmap);
  setParentItem(parent);
  
  assem = &meta->LPub.assem;
  // TODO - This might be suspect, consider changing to individual instance per step
  if (step->multiStep) {
    divider = &meta->LPub.multiStep.divider;
  } else if (step->calledOut) {
    divider = &meta->LPub.callout.divider;
  } else {
    divider = nullptr;
  }

  modelScale = meta->LPub.assem.modelScale;

  setTransformationMode(Qt::SmoothTransformation);

  setToolTip(QString("%1 assembly [%2 x %3 px] - right-click to modify")
             .arg(step->path())
             .arg(boundingRect().width())
             .arg(boundingRect().height()));

  setFlag(QGraphicsItem::ItemIsSelectable,true);
  setFlag(QGraphicsItem::ItemIsFocusable, true);
  setFlag(QGraphicsItem::ItemIsMovable,true);
  setAcceptHoverEvents(true);
  setData(ObjectId, AssemObj);
  setZValue(ASSEM_ZVALUE_DEFAULT);
}

/********************************************
 *
 * partLine
 *
 * this adds a csiPart to the csiParts list
 * and updates the lineType index accordingly
 * taking into account part groups, types and
 * part names meta commands
 *
 ********************************************/

void CsiItem::partLine(
   const QString &partLine,
   QStringList   &csiParts,
   QVector<int>  &typeIndexes,
   int            lineNumber,
   Rc             rc)
{
    // All 3DViewer (LeoCad) metas are written to csiParts
    // for group meta lines, substitute lineNumber with Rc
    int index;
    switch (rc){
    case FadeRc:
    case SilhouetteRc:
    case ColourRc:
    case PartNameRc:
    case PartTypeRc:
    case MLCadGroupRc:
    case LDCadGroupRc:
        index = rc;
        break;
    default:
        index = lineNumber;
        break;
    }
    csiParts.append(partLine);
    typeIndexes.append(index);
}

void CsiItem::partLine(
     const QString      &partLine,
     PartLineAttributes &pla,
     int                 index,
     Rc                  rc)
{
    if (pla.buildMod) {
        if (! pla.buildModIgnore) {
            CsiItem::partLine(partLine,pla.buildModCsiParts,pla.buildModLineTypeIndexes,index/*relativeTypeIndx*/,rc);
            pla.buildModItems = pla.buildModCsiParts.size();
        }
    } else {
        if (pla.buildModItems) {
            pla.csiParts        << pla.buildModCsiParts;
            pla.lineTypeIndexes << pla.buildModLineTypeIndexes;
            pla.buildModCsiParts.clear();
            pla.buildModLineTypeIndexes.clear();
            pla.buildModItems = false;
        }
        CsiItem::partLine(partLine,pla.csiParts,pla.lineTypeIndexes,index/*relativeTypeIndx*/,rc);
    }
}

/**************************************/

void CsiItem::placeCsiPartAnnotations()
{
    if (!assem->annotation.display.value())
        return;

    QHash<QString, PliPart*> pliParts;

    step->pli.getParts(pliParts);

    if (!pliParts.size())
        return;

    for (int i = 0; i < step->csiAnnotations.size(); ++i) {
        CsiAnnotation *ca = step->csiAnnotations[i];
        if (!hiddenAnnotations)
            hiddenAnnotations = ca->caMeta.icon.value().hidden;
        if (!ca->caMeta.icon.value().hidden){
            QString key       = QString("%1_%2")
                                        .arg(ca->caMeta.icon.value().typeBaseName)
                                        .arg(ca->caMeta.icon.value().typeColor);
            PliPart *part     = pliParts[key];

            if (!part)
                continue;

            for (int i = 0; i < part->instances.size(); ++i) {
                if (ca->partLine == part->instances[i] && part->text.size()){
                    CsiAnnotationItem *caItem = new CsiAnnotationItem();
                    caItem->addGraphicsItems(ca,step,part,this,true);
                }
            }
        }
    }
}

void CsiItem::setFlag(GraphicsItemFlag flag, bool value)
{
  QGraphicsItem::setFlag(flag,value);
}

void CsiItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
  QMenu menu;
  QString name          = "Step";
  QString whatsThis     = QString();
  bool dividerDetected  = false;
  bool offerStepDivider = false;
  int numOfSteps        = numSteps(step->top.modelName);
  bool fullContextMenu  = ! step->modelDisplayOnlyStep;
  bool allowLocal       = (parentRelativeType != StepGroupType) && (parentRelativeType != CalloutType);
  Boundary boundary     = step->boundary();

  QAction *addNextAction = nullptr;
  if (fullContextMenu  &&
      step->stepNumber.number != numOfSteps &&
      (parentRelativeType == SingleStepType ||
       (parentRelativeType == StepGroupType &&  (boundary & EndOfSteps)))) {
      addNextAction = menu.addAction("Add Next Step");
      addNextAction->setIcon(QIcon(":/resources/addnext.png"));
      addNextAction->setWhatsThis("Add Next Step:\n  Add the first step of the next page to this page");
    }

  QAction *addPrevAction = nullptr;
  if ( fullContextMenu  &&
       step->stepNumber.number > 1 &&
       (parentRelativeType == SingleStepType ||
        (parentRelativeType == StepGroupType && (boundary & StartOfSteps)))) {
      addPrevAction = menu.addAction("Add Previous Step");
      addPrevAction->setIcon(QIcon(":/resources/addprevious.png"));
      addPrevAction->setWhatsThis("Add Previous Step:\n  Add the last step of the previous page to this page");
    }

  QAction *removeAction = nullptr;
  if (parentRelativeType == StepGroupType &&
      (boundary & (StartOfSteps | EndOfSteps))) {
      removeAction = menu.addAction("Remove this Step");
      removeAction->setIcon(QIcon(":/resources/remove.png"));
      if (boundary & StartOfSteps) {
          removeAction->setWhatsThis("Remove this Step:\n  Move this step from this page to the previous page");
        } else {
          removeAction->setWhatsThis("Remove this Step:\n  Move this step from this page to the next page");
        }
    }

  QAction *clearStepCacheAction = nullptr;
  if (parentRelativeType == StepGroupType){
      clearStepCacheAction = menu.addAction("Reset Step Assembly Image Cache");
      clearStepCacheAction->setIcon(QIcon(":/resources/clearstepcache.png"));
      clearStepCacheAction->setWhatsThis("Clear the CSI image and ldr cache files for this step.");
    }

  Where topOfStep     = step->topOfStep();
  Where bottomOfStep  = step->bottomOfStep();
  Where topOfSteps    = step->topOfSteps();
  Where bottomOfSteps = step->bottomOfSteps();

  Callout *callout    = step->callout();

  if (parentRelativeType == StepGroupType) {
      Where walk = topOfStep;
      Where walk2 = walk;
      Rc rc = scanForward(walk,StepMask);
      Rc rc2 = scanForward(walk2,StepGroupDividerMask|StepMask);
      offerStepDivider = rc2 == StepGroupDividerRc;
      if (rc == StepRc || rc == RotStepRc) {
          ++walk;
          rc = scanForward(walk,StepGroupDividerMask|StepMask);
          dividerDetected = rc == StepGroupDividerRc;
      }
  }

  QAction *movePrevAction = nullptr;
  QAction *moveNextAction = nullptr;
  QAction *addDividerAction = nullptr;
  QAction *addDividerPointerAction = nullptr;
  QAction *allocAction = nullptr;

  AllocEnc allocType = step->parent->allocType();
  if (parentRelativeType == StepGroupType || parentRelativeType == CalloutType) {
      if ((boundary & StartOfRange) && ! (boundary & StartOfSteps)) {
          if (allocType == Vertical) {
              movePrevAction = menu.addAction("Add to Previous Column");
              movePrevAction->setIcon(QIcon(":/resources/addprevious.png"));
              movePrevAction->setWhatsThis(
                    "Add to Previous Column:\n"
                    "  Move this step to the previous column");
            } else {
              movePrevAction = menu.addAction("Add to Previous Row");
              movePrevAction->setIcon(QIcon(":/resources/addprevious.png"));
              movePrevAction->setWhatsThis(
                    "Add to Previous Row:\n"
                    "  Move this step to the previous row\n");
            }
        }

      if ((boundary & EndOfRange) && ! (boundary & EndOfSteps)) {
          if (allocType == Vertical) {
              moveNextAction = menu.addAction("Add to Next Column");
              moveNextAction->setIcon(QIcon(":/resources/addnext.png"));
              moveNextAction->setWhatsThis(
                    "Add to Next Colum:\n"
                    "  Remove this step from its current column,\n"
                    "  and put it in the column to the right");
            } else {
              moveNextAction = menu.addAction("Add to Next Row");
              moveNextAction->setIcon(QIcon(":/resources/addnext.png"));
              moveNextAction->setWhatsThis(
                    "Add to Next Row:\n"
                    "  Remove this step from its current column,\n"
                    "  and put it in the row above");
            }
        }
      if ( ! (boundary & EndOfRange) && ! (boundary & EndOfSteps) && ! dividerDetected) {
          addDividerAction = menu.addAction("Add Divider");
          addDividerAction->setIcon(QIcon(":/resources/divider.png"));
          if (allocType == Vertical) {
              addDividerAction->setWhatsThis(
                    "Add Divider:\n"
                    "  Before step - Place a divider at the top of this step\n"
                    "  After step  - Put the step(s) after this into a new column");
            } else {
              addDividerAction->setWhatsThis(
                    "Add Divider:\n"
                    "  Before step - Place a divider to the left this step"
                    "  After Step - Put the step(s) after this into a new row");
            }
        }
      if (allocType == Vertical) {
          allocAction = commonMenus.displayRowsMenu(menu,name);
        } else {
          allocAction = commonMenus.displayColumnsMenu(menu, name);
        }
    }

  QString pl = "Assembly";

  QAction *placementAction = nullptr;
  if (fullContextMenu  && parentRelativeType == SingleStepType) {
      whatsThis = QString(
            "Move This Assembly:\n"
            "  Move this assembly step image using a dialog (window)\n"
            "  with buttons.  You can also move this step image around\n"
            "  by clicking and dragging it using the mouse.");
      placementAction = commonMenus.placementMenu(menu, pl, whatsThis);
    }

  QAction *addCsiAnnoAction = nullptr;
  if (fullContextMenu &&
      meta->LPub.assem.annotation.display.value() &&
      (step->calledOut || step->multiStep)) {
      addCsiAnnoAction = menu.addAction("Add Part Annotations");
      addCsiAnnoAction->setIcon(QIcon(":/resources/addpartannotation.png"));
  }

  QAction *refreshCsiAnnoAction = nullptr;
  if (fullContextMenu && step->csiAnnotations.size()){
      refreshCsiAnnoAction = menu.addAction("Reload Part Annotations");
      refreshCsiAnnoAction->setIcon(QIcon(":/resources/reloadpartannotation.png"));
  }

  QAction *showCsiAnnoAction = nullptr;
  if (fullContextMenu && hiddenAnnotations){
      showCsiAnnoAction = menu.addAction("Show Hidden Part Annotations");
      showCsiAnnoAction->setIcon(QIcon(":/resources/hidepartannotation.png"));
  }

  QAction *cameraAnglesAction = commonMenus.cameraAnglesMenu(menu, pl);
  QAction *scaleAction        = commonMenus.scaleMenu(menu, pl);
  QAction *cameraFoVAction    = commonMenus.cameraFoVMenu(menu, pl);

  QAction *marginsAction = nullptr;
  switch (parentRelativeType) {
    case SingleStepType:
      whatsThis = QString("Change Assembly Margins:\n"
                          "  Margins are the empty space around this assembly picture.\n"
                          "  You can change the margins if things are too close together,\n"
                          "  or too far apart. ");
      marginsAction = commonMenus.marginMenu(menu, pl, whatsThis);
      break;
    case StepGroupType:
      whatsThis = QString("Change Assembly Margins:\n"
                          "  Margins are the empty space around this assembly picture.\n"
                          "  You can change the margins if things are too close together,\n"
                          "  or too far apart. You can change the margins around the\n"
                          "  whole group of steps, by clicking the menu button with your\n"
                          "  cursor near this assembly image, and using that\n"
                          "  \"Change Step Group Margins\" menu");
      marginsAction = commonMenus.marginMenu(menu, pl, whatsThis);
      break;
    case CalloutType:
      whatsThis = QString("Change Assembly Margins:\n"
                          "  Margins are the empty space around this assembly picture.\n"
                          "  You can change the margins if things are too close together,\n"
                          "  or too far apart. You can change the margins around callout\n"
                          "  this step is in, by putting your cursor on the background\n"
                          "  of the callout, clicking the menu button, and using that\n"
                          "  \"Change Callout Margins\" menu");
      marginsAction = commonMenus.marginMenu(menu, pl, whatsThis);
      break;
    default:
      break;
    }

  QAction *insertRotateIconAction = nullptr;
  if (fullContextMenu) {
      if (! step->placeRotateIcon) { // rotate icon already in place so don't show menu item
          if (parentRelativeType != CalloutType) {
              insertRotateIconAction = menu.addAction("Add Rotate Icon");
              insertRotateIconAction->setIcon(QIcon(":/resources/rotateicon.png"));
            } else if (meta->LPub.callout.begin.mode == CalloutBeginMeta::Unassembled) {
              insertRotateIconAction = menu.addAction("Add Rotate Icon");
              insertRotateIconAction->setIcon(QIcon(":/resources/rotateicon.png"));
            }
        }
    }

  QAction *addPagePointerAction = menu.addAction("Place Page Pointer");
  addPagePointerAction->setWhatsThis("Add pointer from the page to this CSI image");
  addPagePointerAction->setIcon(QIcon(":/resources/addpointer.png"));

  if (dividerDetected) {
      addDividerPointerAction = menu.addAction("Place Divider Pointer");
      addDividerPointerAction->setWhatsThis("Add pointer from the step divider to this CSI image");
      addDividerPointerAction->setIcon(QIcon(":/resources/adddividerpointer.png"));
  }

  QAction *povrayRendererArgumentsAction = nullptr;
  QAction *rendererArgumentsAction = nullptr;
  bool usingPovray = Preferences::preferredRenderer == RENDERER_POVRAY;
  QString rendererName = QString("Add %1 Arguments")
                                 .arg(usingPovray ? "POV Generation":
                                                    QString("%1 Renderer").arg(Render::getRenderer()));
  if (!Preferences::usingNativeRenderer) {
      rendererArgumentsAction = menu.addAction(rendererName);
      rendererArgumentsAction->setWhatsThis("Add custom renderer arguments for this step");
      rendererArgumentsAction->setIcon(QIcon(":/resources/rendererarguments.png"));
      if (usingPovray) {
          povrayRendererArgumentsAction = menu.addAction(QString("Add %1 Renderer Arguments")
                                                                  .arg(Render::getRenderer()));
          povrayRendererArgumentsAction->setWhatsThis("Add POV-Ray custom renderer arguments for this step");
          povrayRendererArgumentsAction->setIcon(QIcon(":/resources/rendererarguments.png"));
      }
  }

  QAction *noStepAction = menu.addAction(fullContextMenu ? "Don't Show This Step"
                                                         : "Don't Show This Final Model");
  noStepAction->setIcon(QIcon(":/resources/display.png"));

  Where top, bottom;
  switch (parentRelativeType) {
    case StepGroupType:
      top = topOfSteps;
      bottom = bottomOfSteps;
      break;
    case CalloutType:
      top = callout->topOfCallout();
      bottom = callout->bottomOfCallout();
      break;
    default: /*SingleStepType*/
      top = topOfStep;
      bottom = bottomOfStep;
      break;
  }

  QAction *selectedAction = menu.exec(event->screenPos());

  if ( ! selectedAction ) {
      return;
    }

  if (selectedAction == addPrevAction) {
      addPrevMultiStep(topOfSteps,bottomOfSteps);

    } else if (selectedAction == addNextAction) {
      addNextMultiStep(topOfSteps,bottomOfSteps);

    } else if (selectedAction == removeAction) {
      if (boundary & StartOfSteps) {
          deleteFirstMultiStep(topOfSteps);
        } else {
          deleteLastMultiStep(topOfSteps,bottomOfSteps);
        }
    } else if (selectedAction == cameraFoVAction) {
      changeFloatSpin(pl+" Field Of View",
                      "Camera FOV",
                      topOfStep,
                      bottomOfStep,
                      &step->csiStepMeta.cameraFoV,
                      0.01f,
                      1,allowLocal);
    } else if (selectedAction == cameraAnglesAction) {
        changeCameraAngles(pl+" Camera Angles",
                          topOfStep,
                          bottomOfStep,
                          &step->csiStepMeta.cameraAngles,
                         1,allowLocal);
     } else if (selectedAction == movePrevAction) {

      addToPrev(parentRelativeType,topOfStep);

    } else if (selectedAction == moveNextAction) {

      addToNext(parentRelativeType,topOfStep);

    } else if (selectedAction == addDividerAction) {

      addDivider(parentRelativeType,bottomOfStep,divider,allocType,offerStepDivider);

    } else if (selectedAction == addPagePointerAction) {

      PlacementMeta pointerPlacement = meta->LPub.pointerBase.placement;
      bool ok = setPointerPlacement(&pointerPlacement,
                                    parentRelativeType,
                                    PagePointerType,
                                    pl+" Pointer Placement");
      if (ok) {
          addPointerTip(meta,topOfStep,bottomOfStep,pointerPlacement.value().placement,PagePointerRc);
        }
    } else if (selectedAction == addDividerPointerAction) {

        PlacementEnc placement = Center;
        addPointerTip(meta,topOfStep,bottomOfStep,placement,StepGroupDividerPointerRc);

    } else if (selectedAction == allocAction) {
        if (parentRelativeType == StepGroupType) {
            changeAlloc(topOfSteps,
                        bottomOfSteps,
                        step->allocMeta());
        } else {
            changeAlloc(callout->topOfCallout(),
                        callout->bottomOfCallout(),
                        step->allocMeta());
        }

    } else if (selectedAction == placementAction) {
        changePlacement(parentRelativeType,
                        CsiType,
                        pl+" Placement",
                        topOfStep,
                        bottomOfStep,
                        &step->csiPlacement.placement);
    } else if (selectedAction == scaleAction){
        changeFloatSpin(pl+" Scale",
                        "Model Size",
                        topOfStep,
                        bottomOfStep,
                        &step->csiStepMeta.modelScale);
    } else if (selectedAction == marginsAction) {
      changeMargins(pl+" Margins",
                    topOfStep,
                    bottomOfStep,
                    &step->csiPlacement.margin);
    } else if (selectedAction == clearStepCacheAction) {
      gui->clearStepCSICache(step->pngName);
    } else if (selectedAction == noStepAction) {
      appendMeta(topOfStep,"0 !LPUB NOSTEP");
    } else if (selectedAction == insertRotateIconAction &&fullContextMenu) {
      appendMeta(topOfStep,"0 !LPUB INSERT ROTATE_ICON");
    } else if (selectedAction == addCsiAnnoAction) {
      step->setCsiAnnotationMetas(*meta);
    } else if (selectedAction == refreshCsiAnnoAction) {
      step->setCsiAnnotationMetas(*meta,true);
    } else if (selectedAction == showCsiAnnoAction) {
      for (int i = 0; i < step->csiAnnotations.size(); ++i) {
          CsiAnnotation *ca = step->csiAnnotations[i];
          if (ca->caMeta.icon.value().hidden) {
              ca->caMeta.icon.value().hidden = false;
              step->csiAnnotations.replace(i,ca);
          }
      }
      hiddenAnnotations = false;
      step->setCsiAnnotationMetas(*meta,!hiddenAnnotations);
    } else if (selectedAction == rendererArgumentsAction) {
      StringMeta rendererArguments =
                 Render::getRenderer() == RENDERER_LDVIEW ? step->ldviewParms :
                 Render::getRenderer() == RENDERER_LDGLITE ? step->ldgliteParms :
                               /*POV scene file generator*/  step->ldviewParms ;
      setRendererArguments(topOfStep,
                         bottomOfStep,
                         rendererName,
                         &rendererArguments);
    } else if (selectedAction == povrayRendererArgumentsAction) {
      setRendererArguments(topOfStep,
                         bottomOfStep,
                         Render::getRenderer(),
                         &step->povrayParms);
    }
}

void CsiItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    isHovered = !this->isSelected() && !mouseIsDown;
    QGraphicsItem::hoverEnterEvent(event);
}

void CsiItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    isHovered = false;
    QGraphicsItem::hoverLeaveEvent(event);
}

void CsiItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
  mouseIsDown = true;
  QGraphicsItem::mousePressEvent(event);
  positionChanged = false;
  //placeGrabbers();
  position = pos();
  gui->showLine(step->topOfStep());
  step->loadTheViewer();
  update();
}

void CsiItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
  if (isSelected() && (flags() & QGraphicsItem::ItemIsMovable)) {
    QPoint delta(int(pos().x() - position.x() + 0.5),
                 int(pos().y() - position.y() + 0.5));

    if (delta.x() || delta.y()) {
      // callouts
      for (int i = 0; i < step->list.size(); i++) {
        Callout *callout = step->list[i];
        callout->drawTips(delta,CsiType);
      }
      // page pointers
      for (int i = 0; i < PP_POSITIONS; i++) {
          Positions position = Positions(i);
          PagePointer *pagePointer = step->page()->pagePointers.value(position);
          if (pagePointer)
              pagePointer->drawTips(delta,this,CsiType);
      }
      // dividers
      for (int i = 0; i < step->page()->graphicsDividerPointerList.size(); i++) {
        DividerPointerItem *pointerItem = step->page()->graphicsDividerPointerList[i];
        int initiator = CsiType;
        foreach (QGraphicsItem *item, pointerItem->collidingItems(Qt::IntersectsItemBoundingRect)) {
            if (item == this)
                pointerItem->drawTip(delta,initiator);
        }
      }
      positionChanged = true;
      //placeGrabbers();
    }
    QGraphicsPixmapItem::mouseMoveEvent(event);
  }
}

void CsiItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
  mouseIsDown = false;

  QGraphicsItem::mouseReleaseEvent(event);

  if (isSelected() && (flags() & QGraphicsItem::ItemIsMovable)) {
    if (positionChanged) {

      beginMacro(QString("DragCsi"));
      
      PlacementData placementData = step->csiPlacement.placement.value();
      qreal topLeft[2] = { sceneBoundingRect().left(),  sceneBoundingRect().top() };
      qreal size[2]    = { sceneBoundingRect().width(), sceneBoundingRect().height() };
      calcOffsets(placementData,placementData.offsets,topLeft,size);
      step->csiPlacement.placement.setValue(placementData);

      QPoint deltaI(int(pos().x() - position.x()),
                    int(pos().y() - position.y()));
      if (step) {
        // callouts
        for (int i = 0; i < step->list.size(); i++) {
          Callout *callout = step->list[i];
          callout->updatePointers(deltaI);
        }
        // page pointers
        for (int i = 0; i < PP_POSITIONS; i++){
            Positions position = Positions(i);
            PagePointer *pagePointer = step->page()->pagePointers.value(position);
            if (pagePointer)
                pagePointer->updatePointers(deltaI,this);
        }
        // dividers
        for (int i = 0; i < step->page()->graphicsDividerPointerList.size(); i++) {
          DividerPointerItem *pointerItem = step->page()->graphicsDividerPointerList[i];
          foreach (QGraphicsItem *item, pointerItem->collidingItems(Qt::IntersectsItemBoundingRect)) {
              if (item == this)
                  pointerItem->updatePointer(deltaI);
          }
        }
      }
      changePlacementOffset(step->topOfStep(),&step->csiPlacement.placement,CsiType);  
      endMacro();
    }
  }
}

void CsiItem::change()
{
  if (isSelected() && (flags() & QGraphicsItem::ItemIsMovable)) {
    if (sizeChanged) {

      beginMacro(QString("Resize"));

      PlacementData placementData = meta->LPub.assem.placement.value();
      qreal topLeft[2] = { sceneBoundingRect().left(),  sceneBoundingRect().top() };
      qreal size[2]    = { sceneBoundingRect().width(), sceneBoundingRect().height() };
      calcOffsets(placementData,placementData.offsets,topLeft,size);
      meta->LPub.assem.placement.setValue(placementData);

      QPoint deltaI(int(pos().x() - position.x()),
                    int(pos().y() - position.y()));
      // callouts
      for (int i = 0; i < step->list.size(); i++) {
        Callout *callout = step->list[i];
        callout->updatePointers(deltaI);
      }
      // page pointers
      for (int i = 0; i < PP_POSITIONS; i++){
          Positions position = Positions(i);
          PagePointer *pagePointer = step->page()->pagePointers.value(position);
          if (pagePointer)
              pagePointer->updatePointers(deltaI,this);
      }
      // dividers
      for (int i = 0; i < step->page()->graphicsDividerPointerList.size(); i++) {
        DividerPointerItem *pointerItem = step->page()->graphicsDividerPointerList[i];
        foreach (QGraphicsItem *item, pointerItem->collidingItems(Qt::IntersectsItemBoundingRect)) {
            if (item == this)
                pointerItem->updatePointer(deltaI);
        }
      }

      changePlacementOffset(step->topOfStep(),&meta->LPub.assem.placement,CsiType,true,false);  
      
      modelScale = step->csiStepMeta.modelScale;
      modelScale.setValue(float(modelScale.value())*float(oldScale));

      changeFloat(step->topOfStep(),step->bottomOfStep(),&modelScale,1,0);
      endMacro();
    }
  }
}

void CsiItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QPen pen;
    pen.setColor(isHovered ? QColor(Preferences::sceneGuideColor) : Qt::black);
    pen.setWidth(0/*cosmetic*/);
    pen.setStyle(isHovered ? Qt::PenStyle(Preferences::sceneGuidesLine) : Qt::NoPen);
    painter->setPen(pen);
    painter->setBrush(Qt::transparent);
    painter->drawRect(this->boundingRect());
    QGraphicsPixmapItem::paint(painter,option,widget);
}
