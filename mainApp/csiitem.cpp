 
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
  step = _step;
  meta = _meta;
  submodelLevel = _submodelLevel;
  parentRelativeType = _parentRelativeType;
  setPixmap(pixmap);
  setParentItem(parent);
  
  assem = &meta->LPub.assem;
  if (parentRelativeType == StepGroupType) {
    divider = &meta->LPub.multiStep.divider;
  } else if (parentRelativeType == CalloutType) {
    divider = &meta->LPub.callout.divider;
  } else {
    divider = NULL;
  }

  setTransformationMode(Qt::SmoothTransformation);

  setToolTip(step->path() + "- right-click to modify");

  setFlag(QGraphicsItem::ItemIsSelectable,true);
  setFlag(QGraphicsItem::ItemIsMovable,true);

  modelScale = meta->LPub.assem.modelScale;
}

void CsiItem::setFlag(GraphicsItemFlag flag, bool value)
{
  QGraphicsItem::setFlag(flag,value);
}

void CsiItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
  QMenu menu;
  QString name = "Step";

  MetaItem mi;
  int numSteps = mi.numSteps(step->top.modelName);
  bool fullContextMenu = step->stepNumber.number != NOSTEPNUMBER;
  Boundary boundary = step->boundary();

  QAction *addNextAction = NULL;
  if (fullContextMenu  &&
      step->stepNumber.number != numSteps &&
      (parentRelativeType == SingleStepType ||
       (parentRelativeType == StepGroupType &&  (boundary & EndOfSteps)))) {
      addNextAction = menu.addAction("Add Next Step");
      addNextAction->setIcon(QIcon(":/resources/addnext.png"));
      addNextAction->setWhatsThis("Add Next Step:\n  Add the first step of the next page to this page");
    }

  QAction *addPrevAction = NULL;
  if ( step->stepNumber.number > 1 &&
       (parentRelativeType == SingleStepType ||
        (parentRelativeType == StepGroupType && (boundary & StartOfSteps)))) {
      addPrevAction = menu.addAction("Add Previous Step");
      addPrevAction->setIcon(QIcon(":/resources/addprevious.png"));
      addPrevAction->setWhatsThis("Add Previous Step:\n  Add the last step of the previous page to this page");
    }

  QAction *removeAction = NULL;
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

  QAction *movePrevAction = NULL;
  QAction *moveNextAction = NULL;
  QAction *addDividerAction = NULL;
  QAction *allocAction = NULL;

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
      if ( ! (boundary & EndOfRange) && ! (boundary & EndOfSteps)) {
          addDividerAction = menu.addAction("Add Divider After Step");
          addDividerAction->setIcon(QIcon(":/resources/divider.png"));
          if (allocType == Vertical) {
              addDividerAction->setWhatsThis(
                    "Add Divider After Step:\n"
                    "  Put the step(s) after this into a new column");
            } else {
              addDividerAction->setWhatsThis(
                    "Add Divider After Step:\n"
                    "  Put the step(s) after this into a new row");
            }
        }
      if (allocType == Vertical) {
          allocAction = commonMenus.displayRowsMenu(menu,name);
        } else {
          allocAction = commonMenus.displayColumnsMenu(menu, name);
        }
    }

  QAction *placementAction = NULL;
  if (parentRelativeType == SingleStepType) {
      placementAction = menu.addAction("Move This Step");
      placementAction->setIcon(QIcon(":/resources/placement.png"));
      placementAction->setWhatsThis(
            "Move This Step:\n"
            "  Move this assembly step image using a dialog (window)\n"
            "  with buttons.  You can also move this step image around\n"
            "  by clicking and dragging it using the mouse.");
    }

  QAction *scaleAction = menu.addAction("Change Scale");
  scaleAction->setIcon(QIcon(":/resources/scale.png"));
  scaleAction->setWhatsThis("Change Scale:\n"
                            "  You can change the size of this assembly image using the scale\n"
                            "  dialog (window).  A scale of 1.0 is true size.  A scale of 2.0\n"
                            "  doubles the size of your model.  A scale of 0.5 makes your model\n"
                            "  half real size\n");

  QAction *marginsAction = menu.addAction("Change Assembly Margins");
  marginsAction->setIcon(QIcon(":/resources/margins.png"));
  switch (parentRelativeType) {
    case SingleStepType:
      marginsAction->setWhatsThis("Change Assembly Margins:\n"
                                  "  Margins are the empty space around this assembly picture.\n"
                                  "  You can change the margins if things are too close together,\n"
                                  "  or too far apart. ");
      break;
    case StepGroupType:
      marginsAction->setWhatsThis("Change Assembly Margins:\n"
                                  "  Margins are the empty space around this assembly picture.\n"
                                  "  You can change the margins if things are too close together,\n"
                                  "  or too far apart. You can change the margins around the\n"
                                  "  whole group of steps, by clicking the menu button with your\n"
                                  "  cursor near this assembly image, and using that\n"
                                  "  \"Change Step Group Margins\" menu");
      break;
    case CalloutType:
      marginsAction->setWhatsThis("Change Assembly Margins:\n"
                                  "  Margins are the empty space around this assembly picture.\n"
                                  "  You can change the margins if things are too close together,\n"
                                  "  or too far apart. You can change the margins around callout\n"
                                  "  this step is in, by putting your cursor on the background\n"
                                  "  of the callout, clicking the menu button, and using that\n"
                                  "  \"Change Callout Margins\" menu");
      break;
    default:
      break;
    }

  QAction *insertRotateIconAction = NULL;
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

  QAction *noStepAction = menu.addAction(fullContextMenu ? "Don't Show This Page" : "Don't Show This Step");
  noStepAction->setIcon(QIcon(":/resources/display.png"));

  QAction *selectedAction = menu.exec(event->screenPos());

  if ( ! selectedAction ) {
      return;
    }
  
  Callout *callout = step->callout();
  
  Where topOfStep    = step->topOfStep();
  Where bottomOfStep = step->bottomOfStep();
  Where topOfSteps  = step->topOfSteps();
  Where bottomOfSteps = step->bottomOfSteps();
  Where begin = topOfSteps;
  
  if (parentRelativeType == StepGroupType) {
      MetaItem mi;
      mi.scanForward(begin,StepGroupMask);
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
    } else if (selectedAction == movePrevAction) {

      addToPrev(parentRelativeType,topOfStep);

    } else if (selectedAction == moveNextAction) {

      addToNext(parentRelativeType,topOfStep);

    } else if (selectedAction == addDividerAction) {
      addDivider(parentRelativeType,bottomOfStep,divider);
    } else if (selectedAction == allocAction) {
      if (parentRelativeType == StepGroupType) {
          changeAlloc(begin,
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
                      "Assembly Placement",
                      topOfStep,
                      bottomOfStep,
                      &meta->LPub.assem.placement);
    } else if (selectedAction == scaleAction) {
      bool allowLocal = parentRelativeType != StepGroupType &&
          parentRelativeType != CalloutType;
      changeFloatSpin("Assembly",
                      "Model Size",
                      begin,
                      bottomOfSteps,
                      &meta->LPub.assem.modelScale,
                      1,allowLocal);
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
      changeMargins("Assembly Margins",
                    topOfStep,
                    bottomOfStep,
                    margins);
    } else if (selectedAction == noStepAction) {
      appendMeta(topOfStep,"0 !LPUB NOSTEP");
    }

  if (selectedAction == insertRotateIconAction && fullContextMenu) {
      appendMeta(topOfStep,"0 !LPUB INSERT ROTATE_ICON OFFSET 0.5 0.5");
    }

}


void CsiItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
  QGraphicsItem::mousePressEvent(event);
  positionChanged = false;
  //placeGrabbers();
  position = pos();
  gui->showLine(step->topOfStep());
  int  rc = step->Load3DCsi(step->csi3DName);

  if (rc == -1){
      qDebug() << "\nCreating CsiItem 3D-render file: " << step->csi3DName;
      QStringList fileFilters;
      fileFilters << QString("%1_%2_*.ldr")
                     .arg(step->csiName())
                     .arg(step->stepNumber.number);
      QFileInfo renderFileInfo(step->csi3DName);
      QDir dir(renderFileInfo.absolutePath());
      QStringList dirs = dir.entryList(fileFilters);
      if (dirs.size() > 0) {
        dir.rename(dirs.at(0), renderFileInfo.fileName());
        rc = step->Load3DCsi(step->csi3DName);
        if (rc != 0)
          qDebug() << "\nCsiItem 3D-render failed to load: " << step->csi3DName;
      }
  }
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
