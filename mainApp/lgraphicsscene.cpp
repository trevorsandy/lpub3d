/****************************************************************************
**
** Copyright (C) 2016 - 2019 Trevor SANDY. All rights reserved.
**
** This file may be used under the terms of the
** GNU General Public Liceense (GPL) version 3.0
** which accompanies this distribution, and is
** available at http://www.gnu.org/licenses/gpl.html
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QDebug>
#include <QGraphicsItem>
#include <QPoint>
#include <QPainter>
#include <QPen>
#include <QGraphicsSceneMouseEvent>
#include "lgraphicsscene.h"

enum ZValueSort { largestZ, smallestZ };

QHash<SceneObject, QString> soMap;

LGraphicsScene::LGraphicsScene(QObject *parent)
  : QGraphicsScene(parent),
    guidePen(QPen(QBrush(QColor(THEME_GUIDE_PEN_DEFAULT)), 2, Qt::DashLine)),
    gridPen(QPen(QBrush(QColor(THEME_GRID_PEN_DEFAULT)), 2, Qt::SolidLine)),
    mValidItem(false),
    mPliPartGroup(false),
    mSceneGuides(false),
    mBaseItem(nullptr),
    mIsItemOnTop(false),
    mIsItemOnBottom(false),
    mShowContextAction(false),
    mSnapToGrid(false),
    mGridSize(GridSizeTable[GRID_SIZE_INDEX_DEFAULT]),
    minZ(Z_VALUE_DEFAULT),
    maxZ(Z_VALUE_DEFAULT)
{
    Q_ASSERT(mGridSize > 0);

#ifdef QT_DEBUG_MODE
    // This is only used for debug tracing at this point.
    if (soMap.size() == 0) {
        soMap[AssemAnnotationObj]       = QString("CSI_ANNOTATION");       //  0 CsiAnnotationType
        soMap[AssemAnnotationPartObj]   = QString("CSI_ANNOTATION_PART");  //  1 CsiPartType
        soMap[AssemObj]                 = QString("ASSEM");                //  2 CsiType
        soMap[CalloutAssemObj]          = QString("CALLOUT_ASSEM");        //  3
        soMap[CalloutBackgroundObj]     = QString("CALLOUT");              //  4 CalloutType
        soMap[CalloutInstanceObj]       = QString("CALLOUT_INSTANCE");     //  5
        soMap[CalloutPointerObj]        = QString("CALLOUT_POINTER");      //  6
        soMap[CalloutUnderpinningObj]   = QString("CALLOUT_UNDERPINNING"); //  7
        soMap[DividerBackgroundObj]     = QString("DIVIDER_ITEM");         //  8
        soMap[DividerObj]               = QString("DIVIDER");              //  9
        soMap[DividerLineObj]           = QString("DIVIDER_LINE");         // 10
        soMap[DividerPointerObj]        = QString("DIVIDER_POINTER");      // 11 DividerPointerType
        soMap[PointerGrabberObj]        = QString("POINTER_GRABBER");      // 12
        soMap[PliGrabberObj]            = QString("PLI_GRABBER");          // 13
        soMap[SubmodelGrabberObj]       = QString("SUBMODEL_GRABBER");     // 14
        soMap[InsertPixmapObj]          = QString("PICTURE");              // 15
        soMap[InsertTextObj]            = QString("TEXT");                 // 16
        soMap[MultiStepBackgroundObj]   = QString("MULTI_STEP");           // 17 StepGroupType
        soMap[MultiStepsBackgroundObj]  = QString("MULTI_STEPS");          // 18
        soMap[PageAttributePixmapObj]   = QString("ATTRIBUTE_PIXMAP");     // 19
        soMap[PageAttributeTextObj]     = QString("ATTRIBUTE_TEXT");       // 20
        soMap[PageBackgroundObj]        = QString("PAGE [ROOT]");          // 21 PageType [Root Item]
        soMap[PageNumberObj]            = QString("PAGE_NUMBER");          // 22 PageNumberType
        soMap[PagePointerObj]           = QString("PAGE_POINTER");         // 23 PagePointerType
        soMap[PartsListAnnotationObj]   = QString("PLI_ANNOTATION");       // 24
        soMap[PartsListBackgroundObj]   = QString("PLI");                  // 25 PartsListType
        soMap[PartsListInstanceObj]     = QString("PLI_INSTANCE");         // 26
        soMap[PointerFirstSegObj]       = QString("POINTER_SEG_FIRST");    // 27
        soMap[PointerHeadObj]           = QString("POINTER_HEAD");         // 28
        soMap[PointerSecondSegObj]      = QString("POINTER_SEG_SECOND");   // 29
        soMap[PointerThirdSegObj]       = QString("POINTER_SEG_THIRD");    // 30
        soMap[RotateIconBackgroundObj]  = QString("ROTATE_ICON");          // 31 RotateIconType
        soMap[StepNumberObj]            = QString("STEP_NUMBER");          // 32 StepNumberType
        soMap[SubModelBackgroundObj]    = QString("SUBMODEL_DISPLAY");     // 33 SubModelType
        soMap[SubModelInstanceObj]      = QString("SUBMODEL_INSTANCE");    // 34
        soMap[SubmodelInstanceCountObj] = QString("SUBMODEL_INST_COUNT");  // 35 SubmodelInstanceCountType
        soMap[PartsListPixmapObj]       = QString("PLI_PART");             // 36
        soMap[PartsListGroupObj]        = QString("PLI_PART_GROUP");       // 37
    }
#endif
}

void LGraphicsScene::updateGuidePos(){
    if (!mSceneGuides)
        return;

    if (mValidItem){
        if (mItemType == PointerGrabberObj ||
            mItemType == PliGrabberObj     ||
            mItemType == SubmodelGrabberObj)
            mGuidePos = QPointF(mBaseItem->sceneBoundingRect().center().x(), mBaseItem->sceneBoundingRect().center().y());
        else
            mGuidePos = QPointF(mBaseItem->sceneBoundingRect().left(), mBaseItem->sceneBoundingRect().top());
    }
    else
    if (mPliPartGroup){
        mGuidePos = QPointF(mBaseItem->parentItem()->sceneBoundingRect().left(),
                            mBaseItem->parentItem()->sceneBoundingRect().top());
    }
}

bool LGraphicsScene::isSelectedItemOnTop()
{
    return mIsItemOnTop;
}

bool LGraphicsScene::isSelectedItemOnBottom()
{
    return mIsItemOnBottom;
}

bool LGraphicsScene::showContextAction()
{
    return mShowContextAction;
}

qreal LGraphicsScene::getSelectedItemZValue()
{
    if (!mBaseItem)
        return 0;
    return mBaseItem->zValue();
}

bool LGraphicsScene::setSelectedItem(const QPointF &scenePos){
    mBaseItem = itemAt(scenePos, QTransform());

    auto checkPliPartGroupSceneObject = [this]()
    {
        if (mBaseItem) {
            SceneObject so = SceneObject(mBaseItem->data(ObjectId).toInt());
            for ( const auto pso : PliPartGroupSceneObjects)
            {
                if (pso == so) {
                    mBaseItem = mBaseItem->parentItem();
                    break;
                }
            }
        }
    };

    checkPliPartGroupSceneObject();

    if (!mBaseItem)
        return false;

    mItemType = mBaseItem->data(ObjectId).toInt();
#ifdef QT_DEBUG_MODE
    qDebug() << QString("BASE........BaseItem [%1], BaseParent [%2], BaseZValue [%3]")
                        .arg(soMap[SceneObject(mItemType)])
                        .arg(soMap[SceneObject(mBaseItem->parentItem()->data(ObjectId).toInt())])
                        .arg(mBaseItem->zValue());
#endif

    auto isExemptSceneObject = [](const SceneObject so)
    {
        for ( const auto eso : ExemptSceneObjects)
            if (eso == so)
                return true;
        return false;
    };

    if (!(mValidItem = !isExemptSceneObject(SceneObject(mItemType))))
        return false;

    // TODO - Temporary workaround to disable functionality on multi-step pages
    auto isNoContextSceneObject = [](const SceneObject so)
    {
        for ( const auto aso : NoContextSceneObjects)
            if (aso == so)
                return true;
        return false;
    };

    if (isNoContextSceneObject(SceneObject(mItemType)))
        return true;

    return setSelectedItemZValue();
}

bool LGraphicsScene::setSelectedItemZValue()
{
    if (!mBaseItem)
        return false;

    auto isIncludedSceneObject = [](const SceneObject so)
    {
        for ( const auto iso : IncludedSceneObjects)
            if (iso == so)
                return true;
        return false;
    };

    QMap<int, qreal> overlapItems;
    foreach (QGraphicsItem *item, mBaseItem->collidingItems(Qt::IntersectsItemBoundingRect)) {
        SceneObject so = SceneObject(item->data(ObjectId).toInt());
#ifdef QT_DEBUG_MODE
        SceneObject pso = SceneObject(item->parentItem()->data(ObjectId).toInt());
        SceneObject bpso = SceneObject(mBaseItem->parentItem()->data(ObjectId).toInt());
        qDebug() << QString("DETECTED....CollidingItem [%1], CollidingParent [%2], CollidingZValue [%3]")
                            .arg(soMap[so]).arg(soMap[pso]).arg(item->zValue());
        if (pso == bpso)
            qDebug() << QString("SIBLING.....CollidingItem [%1], CollidingParent [%2], CollidingZValue [%3]")
                                .arg(soMap[so]).arg(soMap[pso]).arg(item->zValue());
#endif
        // TODO - Temporary workaround to disable functionality on multi-step pages
        auto isNoContextSceneObject = [](const SceneObject so)
        {
            for ( const auto aso : NoContextSceneObjects)
                if (aso == so)
                    return true;
            return false;
        };

        if (isNoContextSceneObject(so))
            return true;

        if (isIncludedSceneObject(so)) {
            overlapItems.insert(so,item->zValue());
#ifdef QT_DEBUG_MODE
            qDebug() << QString("OVERLAP.....CollidingItem [%1], CollidingParent [%2], CollidingZValue [%3]")
                                .arg(soMap[so]).arg(soMap[pso]).arg(item->zValue());
#endif
        }
    }

    auto isExcludedSceneObject = [](const SceneObject so)
    {
        for ( const auto eso : ExcludedSceneObjects)
            if (eso == so)
                return true;
        return false;
    };

     mShowContextAction = (overlapItems.size() > 0 &&
                           !isExcludedSceneObject(SceneObject(mItemType)));

    qreal max = overlapItems.size() > 0 ? overlapItems.first() : 0;
    qreal min = max;

    for (auto item: overlapItems.keys())
    {
            max = qMax(max, overlapItems.value(item));
            min = qMin(min, overlapItems.value(item));
    }

    maxZ = max;
    minZ = min;

    mIsItemOnTop = mBaseItem->zValue() >= maxZ;
    mIsItemOnBottom = mBaseItem->zValue() <= minZ;

    return true;
}

void LGraphicsScene::bringSelectedItemToFront(){
    if (setSelectedItemZValue())
    {
        qreal z = maxZ + 0.1;
        mBaseItem->setZValue(z);
    }
}

void LGraphicsScene::sendSelectedItemToBack(){
    if (setSelectedItemZValue())
    {
        qreal z = minZ - 0.1;
        mBaseItem->setZValue(z);
    }
}

void LGraphicsScene::snapToGrid()
{
    if (mSnapToGrid && mValidItem) {
        qreal bx = mBaseItem->x();
        qreal by = mBaseItem->y();
        qreal gx = int(bx) / mGridSize;
        qreal gy = int(by) / mGridSize;
        qreal gs = qreal(mGridSize);
        if(gx < bx/gs || gx > bx/gs)
            bx = gs*qRound(bx/gs);
        if(gy < by/gs || gy > by/gs)
            by = gs*qRound(by/gs);
        mBaseItem->setPos(bx,by);
    }
}

void LGraphicsScene::drawBackground(QPainter *painter, const QRectF &rect){

    QGraphicsScene::drawBackground(painter, rect);

    if (! mSnapToGrid)
        return;

    painter->setPen(gridPen);

    qreal left = int(rect.left()) - (int(rect.left()) % mGridSize);
    qreal top = int(rect.top()) - (int(rect.top()) % mGridSize);
    QVector<QPointF> points;
    for (qreal x = left; x < rect.right(); x += mGridSize){
        for (qreal y = top; y < rect.bottom(); y += mGridSize){
            points.append(QPointF(x,y));
        }
    }

    painter->drawPoints(points.data(), points.size());
    update();
}

void LGraphicsScene::drawForeground(QPainter *painter, const QRectF &rect){
    if (! mSceneGuides || ! mValidItem)
        return;

    QPointF starPt;
    QPointF endPt;
    painter->setClipRect(rect);
    painter->setRenderHints(QPainter::TextAntialiasing | QPainter::HighQualityAntialiasing);
    painter->setPen(guidePen);
    starPt.setX(mGuidePos.x());
    starPt.setY(rect.top());
    endPt.setX(mGuidePos.x());
    endPt.setY(rect.bottom());
    painter->drawLine(starPt,endPt);
    starPt.setX(rect.left());
    starPt.setY(mGuidePos.y());
    endPt.setX(rect.right());
    endPt.setY(mGuidePos.y());
    painter->drawLine(starPt,endPt);
    update();
}

void LGraphicsScene::mouseMoveEvent(QGraphicsSceneMouseEvent* event) {
    updateGuidePos();
    snapToGrid();
    QGraphicsScene::mouseMoveEvent(event);
}

void LGraphicsScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
    if (event->button() == Qt::LeftButton){
        updateGuidePos();
        snapToGrid();
    }
    mValidItem = false;
    QGraphicsScene::mouseReleaseEvent(event);
}

void LGraphicsScene::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    if (setSelectedItem(event->scenePos())){
        if (event->button() == Qt::LeftButton){
            updateGuidePos();
        }
    }
    QGraphicsScene::mousePressEvent(event);
}

