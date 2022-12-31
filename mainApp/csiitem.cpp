 
/****************************************************************************
**
** Copyright (C) 2007-2009 Kevin Clague. All rights reserved.
** Copyright (C) 2015 - 2022 Trevor SANDY. All rights reserved.
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
#include "dependencies.h"
#include "metaitem.h"
#include "commonmenus.h"
#include "pointer.h"
#include "paths.h"
#include "editwindow.h"

#include "lc_viewwidget.h"
#include "lc_previewwidget.h"

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

    setToolTip(tr("Assembly (%1) [%2 x %3 px] - right-click to modify")
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

void CsiItem::loadTheViewer(bool override, bool zoomExtents)
{
    bool stepAlreadySet = gui->getViewerStepKey() == step->viewerStepKey;
    if (!stepAlreadySet || override) {
        if (!stepAlreadySet) {
            lpub->setCurrentStep(step);
            gui->showLine(step->topOfStep());
        }
        gui->enableBuildModActions();
        step->viewerOptions->ZoomExtents = zoomExtents;
        step->loadTheViewer();
    }
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
    // All Visual Editor (LeoCad) metas are written to csiParts
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

 /*
 * For partLine(), the BuildMod behaviour, when we are processing a buildModLevel, populates
 * the buildModCsiParts list, its corresponding buildModLineTypeIndexes and sets
 * buildModItems (bool) to then number of mod lines.
 * Otherwise, the csiParts list is updatd. If there are buildModItems, buildModCsiParts
 * are added to csiParts and the BuildMod registers (parts, indexes and items flag) are
 * reset. Lastly, csiParts and there repspective lineTypeIndexes are updated.
 */

void CsiItem::partLine(
     const QString      &partLine,
     PartLineAttributes &pla,
     int                 index,
     Rc                  rc)
{
    if (pla.buildModLevel) {
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
    if (!assem->annotation.display.value() || !step->csiAnnotations.size())
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

void CsiItem::previewCsi(bool useDockable) {
    const QString stepKey  = gui->getViewerStepKey();
    if (stepKey.isEmpty())
        return;

    const int colorCode = LDRAW_MATERIAL_COLOUR;
    const QString csiFileName = QString("csip_%1.ldr").arg(QString(stepKey).replace(";","_"));

    // Check if CSI file date modified is older than model file (on the stack) date modified
    bool csiOutOfDate = false;
    const QString csiFile = QString("%1/%2/%3").arg(QDir::currentPath()).arg(Paths::tmpDir).arg(csiFileName);
    QFile csi(csiFile);
    bool csiExist = csi.exists();
    if (csiExist) {
        QString parentModelName = step->parent->modelName();
        QDateTime lastModified = QFileInfo(csiFile).lastModified();
        QStringList parsedStack = step->submodelStack();
        parsedStack << parentModelName;
        if ( ! isOlder(parsedStack,lastModified)) {
            csiOutOfDate = true;
            emit gui->messageSig(LOG_TRACE,tr("CSI file out of date %1.").arg(csiFileName));
            if (! csi.remove()) {
                emit gui->messageSig(LOG_ERROR,tr("Failed to remove out of date CSI file %1.").arg(csiFileName));
            }
        }
    }

    if (!csiExist || csiOutOfDate) {
        const QStringList content = gui->getViewerStepRotatedContents(stepKey);
        if (content.size()) {
            QFile file(csiFile);
            if ( ! file.open(QFile::WriteOnly | QFile::Text)) {
                emit gui->messageSig(LOG_ERROR,tr("Cannot open file %1 for writing: %2")
                                                  .arg(csiFile) .arg(file.errorString()));
                return;
            }
            QTextStream out(&file);
            for (int i = 0; i < content.size(); i++) {
                QString line = content[i];
                out << line << lpub_endl;
            }
            file.close();
        } else {
            return;
        }
    }

    lcPreferences& Preferences = lcGetPreferences();
    bool dockable = Preferences.mPreviewPosition == lcPreviewPosition::Dockable && useDockable;
    QPoint position;
    if (!dockable) {
        QPointF sceneP;
        switch (Preferences.mPreviewLocation)
        {
        case lcPreviewLocation::TopRight:
            sceneP = mapToScene(boundingRect().topRight());
            break;
        case lcPreviewLocation::TopLeft:
            sceneP = mapToScene(boundingRect().topLeft());
            break;
        case lcPreviewLocation::BottomRight:
            sceneP = mapToScene(boundingRect().bottomRight());
            break;
        default:
            sceneP = mapToScene(boundingRect().bottomLeft());
            break;
        }
        QGraphicsView *view = scene()->views().first();
        QPoint viewP = view->mapFromScene(sceneP);
        position = view->viewport()->mapToGlobal(viewP);
    }

    gui->previewPiece(csiFileName, colorCode, dockable, QRect(), position);
}

void CsiItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    QMenu menu;
    const QString name    = tr("Assembly");
    QString whatsThis     = QString();
    bool dividerDetected  = false;
    bool offerStepDivider = false;
    bool fullContextMenu  = ! step->modelDisplayOnlyStep;
    bool allowLocal       = (parentRelativeType != StepGroupType) && (parentRelativeType != CalloutType);
    Boundary boundary     = step->boundary();
    Step    *lastStep     = nullptr;
    int numOfSteps        = numSteps(step->top.modelName);
    int maxSteps          = 0;
    int lastStepNumber    = 0;

    QAction *addNextStepAction         = nullptr;
    QAction *addNextStepsAction        = nullptr;
    if (fullContextMenu  &&
        step->stepNumber.number != numOfSteps &&
        (parentRelativeType == SingleStepType ||
         (parentRelativeType == StepGroupType &&  (boundary & EndOfSteps)))) {
        AbstractStepsElement *range = lpub->page.list[lpub->page.list.size()-1];
        if (range->relativeType == RangeType) {
            AbstractRangeElement *rangeElement = range->list[range->list.size()-1];
            if (rangeElement->relativeType == StepType) {
                lastStep = dynamic_cast<Step *> (rangeElement);
                lastStepNumber = lastStep->stepNumber.number;
                maxSteps = numSteps(lastStep->topOfStep().modelName);
                if (lastStepNumber != maxSteps) {
                    addNextStepAction  = lpub->getAct("addNextStepAction.1");
                    commonMenus.addAction(addNextStepAction,menu);
                }
                if ((maxSteps - lastStepNumber) >= 2) {
                    addNextStepsAction = lpub->getAct("addNextStepsAction.1");
                    commonMenus.addAction(addNextStepsAction,menu);
                }
            }
        }
    }

    QAction *addPrevStepAction         = nullptr;
    if ( fullContextMenu  &&
         step->stepNumber.number > 1 &&
        (parentRelativeType == SingleStepType ||
        (parentRelativeType == StepGroupType && (boundary & StartOfSteps)))) {
        addPrevStepAction              = lpub->getAct("addPrevStepAction.1");
        commonMenus.addAction(addPrevStepAction,menu);
    }

    QAction *removeStepAction          = nullptr;
    if (parentRelativeType == StepGroupType &&
        (boundary & (StartOfSteps | EndOfSteps))) {
        removeStepAction               = lpub->getAct("removeStepAction.1");
        if (boundary & StartOfSteps) {
            removeStepAction->setStatusTip(tr("Move this step from this page to the previous page"));
            removeStepAction->setWhatsThis(tr("Remove this Step:\n  Move this step from this page to the previous page"));
        }
        commonMenus.addAction(removeStepAction,menu);
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

    QAction *addDividerPointerAction   = nullptr;
    if (dividerDetected) {
        addDividerPointerAction = lpub->getAct("addDividerPointerAction.1");
        commonMenus.addAction(addDividerPointerAction,menu);
    }

    QAction *movePrevAction            = nullptr;
    QAction *moveNextAction            = nullptr;
    QAction *addDividerAction          = nullptr;
    QAction *allocAction               = nullptr;
    AllocEnc allocType = step->parent->allocType();
    if (parentRelativeType == StepGroupType || parentRelativeType == CalloutType) {
        if ((boundary & StartOfRange) && ! (boundary & StartOfSteps)) {
            movePrevAction             = lpub->getAct("movePrevAction.1");
            if (allocType == Vertical) {
                movePrevAction->setText(tr("Add to Previous Column"));
                movePrevAction->setWhatsThis(tr("Add to Previous Column:\n"
                                                "  Move this step to the previous column"));
            }
            commonMenus.addAction(movePrevAction,menu);
        }

        if ((boundary & EndOfRange) && ! (boundary & EndOfSteps)) {
            moveNextAction             = lpub->getAct("moveNextAction.1");
            if (allocType == Vertical) {
                moveNextAction->setText(tr("Add to Next Column"));
                moveNextAction->setWhatsThis(tr("Add to Next Column:\n"
                                                "  Remove this step from its current column,\n"
                                                "  and put it in the column to the right"));
            }
            commonMenus.addAction(moveNextAction,menu);
        }

        if ( ! (boundary & EndOfRange) && ! (boundary & EndOfSteps) && ! dividerDetected) {
            addDividerAction           = lpub->getAct("addDividerAction.1");
            if (allocType == Vertical) {
                addDividerAction->setWhatsThis(tr("Add Divider:\n"
                                               "  Before step - Place a divider at the top of this step\n"
                                               "  After step  - Put the step(s) after this into a new column"));
            }
            commonMenus.addAction(addDividerAction,menu);
        }

        if (allocType == Vertical) {
            allocAction                = lpub->getAct("displayRowsAction.1");
        } else {
            allocAction                = lpub->getAct("displayColumnsAction.1");
        }
        commonMenus.addAction(allocAction,menu,tr("Step"));
    }

    QAction *placementAction           = nullptr;
    if (fullContextMenu  && parentRelativeType == SingleStepType) {
        placementAction                = lpub->getAct("placementAction.1");
        PlacementData placementData    = placement.value();
        placementAction->setWhatsThis(commonMenus.naturalLanguagePlacementWhatsThis(relativeType,placementData,name));
        commonMenus.addAction(placementAction,menu,name);
    }

    QAction *addCsiAnnoAction          = nullptr;
    QAction *refreshCsiAnnoAction      = nullptr;
    QAction *showCsiAnnoAction         = nullptr;
    if (fullContextMenu && meta->LPub.assem.annotation.display.value()) {
        if (!step->csiAnnotations.size()) {
            addCsiAnnoAction           = lpub->getAct("addCsiAnnoAction.1");
            commonMenus.addAction(addCsiAnnoAction,menu);
        } else {
            refreshCsiAnnoAction       = lpub->getAct("refreshCsiAnnoAction.1");
            commonMenus.addAction(refreshCsiAnnoAction,menu);
        }

        if (hiddenAnnotations) {
            showCsiAnnoAction          = lpub->getAct("showCsiAnnoAction.1");
            commonMenus.addAction(showCsiAnnoAction,menu);
        }
    }

    QAction *cameraAnglesAction        = lpub->getAct("cameraAnglesAction.1");
    commonMenus.addAction(cameraAnglesAction,menu,name);

    QAction *cameraFoVAction           = lpub->getAct("cameraFoVAction.1");
    commonMenus.addAction(cameraFoVAction,menu,name);

    QAction *scaleAction               = lpub->getAct("scaleAction.1");
    commonMenus.addAction(scaleAction,menu,name);

    QAction *marginsAction             = lpub->getAct("marginAction.1");
    switch (parentRelativeType)
    {
      case SingleStepType:
        whatsThis = tr("Change Assembly Margins:\n"
                       "Margins are the empty space around this assembly picture.\n"
                       "You can change the margins if things are too close together,\n"
                       "or too far apart. ");
        marginsAction->setWhatsThis(whatsThis);
        break;
      case StepGroupType:
        whatsThis = tr("Change Assembly Margins:\n"
                       "Margins are the empty space around this assembly picture.\n"
                       "You can change the margins if things are too close together,\n"
                       "or too far apart. You can change the margins around the\n"
                       "whole group of steps, by clicking the menu button with your\n"
                       "cursor near this assembly image, and using that\n"
                       "\"Change Step Group Margins\" menu");
        marginsAction->setWhatsThis(whatsThis);
        break;
      case CalloutType:
        whatsThis = tr("Change Assembly Margins:\n"
                       "Margins are the empty space around this assembly picture.\n"
                       "You can change the margins if things are too close together,\n"
                       "or too far apart. You can change the margins around callout\n"
                       "this step is in, by putting your cursor on the background\n"
                       "of the callout, clicking the menu button, and using that\n"
                       "\"Change Callout Margins\" menu");
        marginsAction->setWhatsThis(whatsThis);
        break;
      default:
        break;
    }
    commonMenus.addAction(marginsAction,menu,name);

    QAction *insertRotateIconAction    = nullptr;
    if (fullContextMenu) {
        if (! step->placeRotateIcon) { // rotate icon already in place so don't show menu item
            if (parentRelativeType != CalloutType ||
                meta->LPub.callout.begin.mode == CalloutBeginMeta::Unassembled) {
                insertRotateIconAction = lpub->getAct("insertRotateIconAction.1");
                commonMenus.addAction(insertRotateIconAction,menu);
            }
        }
    }

    QAction *addPagePointerAction      = lpub->getAct("addPagePointerAction.1");
    commonMenus.addAction(addPagePointerAction,menu);

    QAction *setHighlightStepAction    = lpub->getAct("highlightStepAction.1");
    setHighlightStepAction->setEnabled(meta->LPub.highlightStep.setup.value());
    commonMenus.addAction(setHighlightStepAction,menu,name);

    QAction *setFadeStepsAction        = lpub->getAct("fadeStepsAction.1");
    setFadeStepsAction->setEnabled(meta->LPub.fadeSteps.setup.value());
    commonMenus.addAction(setFadeStepsAction,menu,name);

    QAction *rendererAction            = lpub->getAct("preferredRendererAction.1");
    commonMenus.addAction(rendererAction,menu,name);

    QAction *rendererArgumentsAction   = nullptr;
    QAction *povrayRendererArgumentsAction = nullptr;
    bool usingPovray = Preferences::preferredRenderer == RENDERER_POVRAY;
    if (Preferences::preferredRenderer != RENDERER_NATIVE) {
        rendererArgumentsAction        = lpub->getAct("rendererArgumentsAction.1");
        commonMenus.addAction(rendererArgumentsAction,menu,name);
        if (usingPovray) {
            povrayRendererArgumentsAction = lpub->getAct("povrayRendererArgumentsAction.1");
            commonMenus.addAction(povrayRendererArgumentsAction,menu,name);
        }
    }

    QAction *refreshStepCacheAction      = nullptr;
    if (parentRelativeType == StepGroupType) {
        refreshStepCacheAction           = lpub->getAct("refreshStepCacheAction.1");
        commonMenus.addAction(refreshStepCacheAction,menu,name);
    }

    QAction *noStepAction              = lpub->getAct("noStepAction.1");
    noStepAction->setText(tr("Do Not Show This %1").arg(fullContextMenu ? tr("Step") : tr("Final Model")));
    commonMenus.addAction(noStepAction,menu);


    QAction *previewCsiAction          = lpub->getAct("previewPartAction.1");
    lcPreferences& Preferences         = lcGetPreferences();
    previewCsiAction->setEnabled(Preferences.mPreviewEnabled);
    commonMenus.addAction(previewCsiAction,menu,name);

    // Build modification actions
    bool canUpdatePreview = true;
    if (LPub::viewerStepKey != step->viewerStepKey) {
        lpub->setCurrentStep(step);
        gui->showLine(step->topOfStep());
        if (gui->saveBuildModification()) {
            canUpdatePreview = false;
            loadTheViewer(false/*override*/, false/*zoomExtents*/);
        }
    }

    gui->enableBuildModActions();

    QAction *applyBuildModAction       = nullptr;
    QAction *removeBuildModAction      = nullptr;
    QAction *deleteBuildModAction      = nullptr;
    if (Preferences::buildModEnabled) {
        menu.addSeparator();
        // these trigger the gui slot so nothing is done in selectedAction below
        applyBuildModAction            = lpub->getAct("ApplyBuildModAct.4");
        commonMenus.addAction(applyBuildModAction,menu);

        removeBuildModAction           = lpub->getAct("RemoveBuildModAct.4");
        commonMenus.addAction(removeBuildModAction,menu);

        deleteBuildModAction           = lpub->getAct("DeleteBuildModAct.4");
        commonMenus.addAction(deleteBuildModAction,menu);
    }

    QAction *resetViewerImageAction    = nullptr;
    if (canUpdatePreview) {
        menu.addSeparator();
        resetViewerImageAction         = lpub->getAct("resetViewerImageAction.1");
        commonMenus.addAction(resetViewerImageAction,menu,name);
    }

    QAction *copyCsiImagePathAction    = nullptr;
#ifndef QT_NO_CLIPBOARD
    menu.addSeparator();
    copyCsiImagePathAction             = lpub->getAct("copyToClipboardAction.1");
    commonMenus.addAction(copyCsiImagePathAction,menu,name);
#endif

    QAction *viewCSIFileAction         = lpub->getAct("viewCSIFileAction.1");
    viewCSIFileAction->setText(tr("View Step %1 Assembly File") .arg(step->stepNumber.number));
    commonMenus.addAction(viewCSIFileAction,menu);

#ifdef QT_DEBUG_MODE
    QAction *view3DViewerFileAction    = lpub->getAct("view3DViewerFileAction.1");
    view3DViewerFileAction->setText(tr("View Step %1 Assembly Visual Editor File") .arg(step->stepNumber.number));
    commonMenus.addAction(view3DViewerFileAction,menu);
#endif

    QAction *selectedAction            = menu.exec(event->screenPos());

    if ( ! selectedAction ) {
        return;
    }

    if (selectedAction == previewCsiAction) {
          previewCsi(true /*previewCsiAction*/);
    } else if (selectedAction == addPrevStepAction) {
        addPrevMultiStep(topOfSteps,bottomOfSteps);
    } else if (selectedAction == addNextStepAction) {
        addNextMultiStep(topOfSteps,bottomOfSteps);
    } else if (selectedAction == addNextStepsAction) {
        bool ok;
        int maxNextSteps = maxSteps - lastStepNumber;
        int numOfSteps = QInputDialog::getInt(gui,QMessageBox::tr("Next Steps"),QMessageBox::tr("Number of next steps"),maxNextSteps,1,maxNextSteps,1,&ok);
        if (ok) {
            if (numOfSteps > 1)
                addNextStepsMultiStep(lastStep->topOfSteps(),lastStep->bottomOfSteps(),numOfSteps);
            else
                addNextMultiStep(topOfSteps,bottomOfSteps);
        }
    } else if (selectedAction == removeStepAction) {
        if (boundary & StartOfSteps) {
            deleteFirstMultiStep(topOfSteps);
        } else {
            deleteLastMultiStep(topOfSteps,bottomOfSteps);
        }
    } else if (selectedAction == cameraFoVAction) {
        changeCameraFOV(tr("%1 Field Of View").arg(name),
                        tr("Camera FOV"),
                        topOfStep,
                        bottomOfStep,
                        &step->csiStepMeta.cameraFoV,
                        &step->csiStepMeta.cameraZNear,
                        &step->csiStepMeta.cameraZFar,
                        1,allowLocal);
    } else if (selectedAction == cameraAnglesAction) {
          changeCameraAngles(tr("%1 Camera Angles").arg(name),
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
                                      tr("%1 Pointer Placement").arg(name));
        if (ok) {
            addPointerTip(meta,topOfStep,bottomOfStep,pointerPlacement.value().placement,PagePointerRc);
        }
    } else if (selectedAction == addDividerPointerAction) {

          PlacementEnc placement = Center;
          addPointerTip(meta,topOfStep,bottomOfStep,placement,StepGroupDividerPointerRc);

    } else if (selectedAction == resetViewerImageAction) {
        if (gui->saveBuildModification())
            loadTheViewer(true/*override*/, false/*zoomExtents*/);
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
                        tr("%1 Placement").arg(name),
                        topOfStep,
                        bottomOfStep,
                        &step->csiPlacement.placement);
    } else if (selectedAction == rendererAction) {
        changePreferredRenderer(tr("%1 Preferred Renderer").arg(name),
                                topOfStep,
                                bottomOfStep,
                                &step->csiStepMeta.preferredRenderer);
    } else if (selectedAction == setFadeStepsAction) {
        setFadeSteps(tr("Fade Previous Step %1s").arg(name),
                      topOfStep,
                      bottomOfStep,
                      &step->csiStepMeta.fadeSteps);
    } else if (selectedAction == setHighlightStepAction) {
        setHighlightStep(tr("Highlight Current Step %1").arg(name),
                         topOfStep,
                         bottomOfStep,
                         &step->csiStepMeta.highlightStep);
    } else if (selectedAction == scaleAction) {
        changeFloatSpin(tr("%1 Scale").arg(name),
                        tr("Model Size"),
                        topOfStep,
                        bottomOfStep,
                        &step->csiStepMeta.modelScale,
                        0.01f,1,true, // step, append, checklocal
                        DoubleSpinScale);
    } else if (selectedAction == marginsAction) {
        changeMargins(tr("%1 Margins").arg(name),
                      topOfStep,
                      bottomOfStep,
                      &step->csiPlacement.margin);
    } else if (selectedAction == noStepAction) {
        appendMeta(topOfStep,"0 !LPUB NOSTEP");
    } else if (selectedAction == insertRotateIconAction) {
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
        const QString rendererLabel = tr("Add %1 Arguments")
                                         .arg(usingPovray ? tr("POV Generation"):
                                                            tr("%1 Renderer").arg(rendererNames[Render::getRenderer()]));
        StringMeta rendererArguments =
                   Render::getRenderer() == RENDERER_LDVIEW ? step->ldviewParms :
                   Render::getRenderer() == RENDERER_LDGLITE ? step->ldgliteParms :
                                 /*POV scene file generator*/  step->ldviewParms ;
        setRendererArguments(topOfStep,
                           bottomOfStep,
                           rendererLabel,
                           &rendererArguments);
    } else if (selectedAction == povrayRendererArgumentsAction) {
        setRendererArguments(topOfStep,
                           bottomOfStep,
                           rendererNames[Render::getRenderer()],
                           &step->povrayParms);
    } else if (selectedAction == refreshStepCacheAction) {
        Page *page = step->page();
        clearPageCache(parentRelativeType,page,Options::CSI);
        //gui->clearStepCSICache(step->pngName);
    } else if (selectedAction == copyCsiImagePathAction) {
        QObject::connect(copyCsiImagePathAction, SIGNAL(triggered()), gui, SLOT(updateClipboard()));
        copyCsiImagePathAction->setData(step->pngName);
        emit copyCsiImagePathAction->triggered();
    } else if (selectedAction == viewCSIFileAction) {
        QFontMetrics currentMetrics(gui->getEditModeWindow()->font());
        QString elidedModelName = currentMetrics.elidedText(step->topOfStep().modelName, Qt::ElideRight, gui->getEditModeWindow()->width());
        const QString modelName = tr("%1 Step %2").arg(elidedModelName).arg(step->stepNumber.number);
        QString csiFile = QDir::toNativeSeparators(QDir::currentPath() + "/" + Paths::tmpDir + "/csi.ldr");
        gui->displayFile(nullptr, Where(csiFile, 0), true/*editModelFile*/);
        gui->getEditModeWindow()->setWindowTitle(tr("Detached LDraw Viewer - %1").arg(modelName));
        gui->getEditModeWindow()->setReadOnly(true);
        gui->getEditModeWindow()->show();
    }
#ifdef QT_DEBUG_MODE
    else if (selectedAction == view3DViewerFileAction) {
        QFontMetrics currentMetrics(gui->getEditModeWindow()->font());
        QString elidedModelName = currentMetrics.elidedText(step->topOfStep().modelName, Qt::ElideRight, gui->getEditModeWindow()->width());
        const QString modelName = tr("%1 Step %2").arg(elidedModelName).arg(step->stepNumber.number);
        QString csiFilePath = QDir::currentPath() + "/" + Paths::tmpDir + "/csi.ldr";
        QStringList Keys    = LPub::getViewerStepKeys(true/*get Name*/, false/*PLI*/, step->viewerStepKey);
        QString csiFile     = QString("%1/viewer_csi_%2.ldr")
                                      .arg(QFileInfo(csiFilePath).absolutePath())
                                      .arg(QString("%1_%2_%3")
                                                   .arg(Keys.at(0))    // Name
                                                   .arg(Keys.at(1))    // Line Number
                                                   .arg(Keys.at(2)));  // Step Number
        gui->displayFile(nullptr, Where(csiFile, 0), true/*editModelFile*/);
        gui->getEditModeWindow()->setWindowTitle(tr("Detached LDraw Viewer - %1").arg(modelName));
        gui->getEditModeWindow()->setReadOnly(true);
        gui->getEditModeWindow()->show();
    }
#endif
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

void CsiItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsItem::mouseDoubleClickEvent(event);
    if ( event->button() == Qt::LeftButton )
    {
        lcPreferences& Preferences = lcGetPreferences();
        if (Preferences.mPreviewEnabled && Preferences.mPreviewPosition == lcPreviewPosition::Floating)
        {
            previewCsi();
        }
    }
}

void CsiItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    mouseIsDown = true;
    QGraphicsItem::mousePressEvent(event);
    positionChanged = false;
    //placeGrabbers();
    position = pos();

    if ( event->button() == Qt::LeftButton ) {
        bool lineShown = false;
        if ((lineShown = gui->saveBuildModification()))
            loadTheViewer();
        if (!lineShown)
            gui->showLine(step->topOfStep());
    }

    //  update();
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
                Q_FOREACH (QGraphicsItem *item, pointerItem->collidingItems(Qt::IntersectsItemBoundingRect)) {
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
            if (step) {
                step->updateViewer = false; // nothing new to visualize

                PlacementData placementData = step->csiPlacement.placement.value();
                qreal topLeft[2] = { sceneBoundingRect().left(),  sceneBoundingRect().top() };
                qreal size[2]    = { sceneBoundingRect().width(), sceneBoundingRect().height() };
                calcOffsets(placementData,placementData.offsets,topLeft,size);
                step->csiPlacement.placement.setValue(placementData);

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
                    Q_FOREACH (QGraphicsItem *item, pointerItem->collidingItems(Qt::IntersectsItemBoundingRect)) {
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
                Q_FOREACH (QGraphicsItem *item, pointerItem->collidingItems(Qt::IntersectsItemBoundingRect)) {
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
