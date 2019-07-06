 
/****************************************************************************
**
** Copyright (C) 2007-2009 Kevin Clague. All rights reserved.
** Copyright (C) 2015 - 2019 Trevor SANDY. All rights reserved.
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

CsiItem::CsiItem(
  Step          *_step,
  Meta          *_meta,
  QPixmap       &pixmap,
  int            _submodelLevel,
  QGraphicsItem *parent,
  PlacementType  _parentRelativeType)
{
  step               = _step;
  meta               = _meta;
  submodelLevel      = _submodelLevel;
  parentRelativeType = _parentRelativeType;
  hiddenAnnotations  = false;

  setPixmap(pixmap);
  setParentItem(parent);
  
  assem = &meta->LPub.assem;
  if (parentRelativeType == StepGroupType) {
    divider = &meta->LPub.multiStep.divider;
  } else if (parentRelativeType == CalloutType) {
    divider = &meta->LPub.callout.divider;
  } else {
    divider = nullptr;
  }

  modelScale = meta->LPub.assem.modelScale;

  setTransformationMode(Qt::SmoothTransformation);

  setToolTip(step->path() + "- right-click to modify");

  setFlag(QGraphicsItem::ItemIsSelectable,true);
  setFlag(QGraphicsItem::ItemIsMovable,true);
  setData(ObjectId, AssemObj);
  setZValue(meta->LPub.page.scene.assem.zValue());
}

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
                    caItem->setZValue(meta->LPub.page.scene.assemAnnotation.zValue());
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
      Rc rc = scanForward(walk,StepMask);
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

  QAction *cameraDistFactorAction = nullptr;
  QAction *scaleAction = nullptr;
  if (Preferences::usingNativeRenderer){
      cameraDistFactorAction  = commonMenus.cameraDistFactorrMenu(menu, pl);
  } else {
      scaleAction             = commonMenus.scaleMenu(menu, pl);
  }
  QAction *cameraFoVAction    = commonMenus.cameraFoVMenu(menu, pl);
  QAction *cameraAnglesAction = commonMenus.cameraAnglesMenu(menu, pl);

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

  QAction *noStepAction = menu.addAction(fullContextMenu ? "Don't Show This Step"
                                                         : "Don't Show This Final Model");
  noStepAction->setIcon(QIcon(":/resources/display.png"));

  QAction *bringToFrontAction = nullptr;
  QAction *sendToBackBackAction = nullptr;
  if (gui->pagescene()->showContextAction()) {
      if (!gui->pagescene()->isSelectedItemOnTop())
          bringToFrontAction = commonMenus.bringToFrontMenu(menu, pl);
      if (!gui->pagescene()->isSelectedItemOnBottom())
          sendToBackBackAction  = commonMenus.sendToBackMenu(menu, pl);
  }

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
                      &meta->LPub.assem.cameraFoV,
                      0.01,
                      1,allowLocal);
    } else if (selectedAction == cameraAnglesAction) {
        changeCameraAngles(pl+" Camera Angles",
                          topOfStep,
                          bottomOfStep,
                          &meta->LPub.assem.cameraAngles,
                         1,allowLocal);
     } else if (selectedAction == movePrevAction) {

      addToPrev(parentRelativeType,topOfStep);

    } else if (selectedAction == moveNextAction) {

      addToNext(parentRelativeType,topOfStep);

    } else if (selectedAction == addDividerAction) {

      addDivider(parentRelativeType,bottomOfStep,divider,allocType);

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
                        &meta->LPub.assem.placement);
    } else if (selectedAction == cameraDistFactorAction) {
        changeCameraDistFactor(pl+" Camera Distance",
                               "Native Camera Distance",
                               topOfSteps,
                               bottomOfSteps,
                               &meta->LPub.assem.cameraDistNative.factor);
    } else if (selectedAction == scaleAction){
        changeFloatSpin(pl+" Scale",
                        "Model Size",
                        topOfSteps,
                        bottomOfSteps,
                        &meta->LPub.assem.modelScale);
    } else if (selectedAction == marginsAction) {

      MarginsMeta *margins;

      switch (parentRelativeType) {
        case StepGroupType:
          margins = &meta->LPub.multiStep.csi.margin;
          break;
        case CalloutType:
          margins = &meta->LPub.callout.csi.margin;
          break;
        default:
          margins = &meta->LPub.assem.margin;
          break;
        }
      changeMargins(pl+" Margins",
                    topOfStep,
                    bottomOfStep,
                    margins);
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
    } else if (selectedAction == bringToFrontAction) {
      setSelectedItemZValue(top,
                            bottom,
                            BringToFront,
                            &meta->LPub.page.scene.assem);
    } else if (selectedAction == sendToBackBackAction) {
      setSelectedItemZValue(top,
                            bottom,
                            SendToBack,
                            &meta->LPub.page.scene.assem);
    }
}

void CsiItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
  QGraphicsItem::mousePressEvent(event);
  positionChanged = false;
  //placeGrabbers();
  position = pos();
  gui->showLine(step->topOfStep());
  step->loadTheViewer();
}

void CsiItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
  if (isSelected() && (flags() & QGraphicsItem::ItemIsMovable)) {
    QPoint delta(int(pos().x() - position.x() + 0.5),
                 int(pos().y() - position.y() + 0.5));

    if (delta.x() || delta.y()) {
      for (int i = 0; i < step->list.size(); i++) {
        Callout *callout = step->list[i];
        callout->drawTips(delta);
      }      
      positionChanged = true;
      //placeGrabbers();
    }
    QGraphicsPixmapItem::mouseMoveEvent(event);
  }
}

void CsiItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
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
        for (int i = 0; i < step->list.size(); i++) {
          Callout *callout = step->list[i];
          callout->updatePointers(deltaI);
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
      for (int i = 0; i < step->list.size(); i++) {
        Callout *callout = step->list[i];
        callout->updatePointers(deltaI);
      }
      changePlacementOffset(step->topOfStep(),&meta->LPub.assem.placement,CsiType,true,false);  
      
      modelScale.setValue(modelScale.value()*oldScale);
      changeFloat(step->topOfStep(),step->bottomOfStep(),&modelScale,1,0);
      endMacro();
    }
  }
}
