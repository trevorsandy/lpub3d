/****************************************************************************
**
** Copyright (C) 2016 - 2020 Trevor SANDY. All rights reserved.
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

LGraphicsScene::LGraphicsScene(QObject *parent)
  : QGraphicsScene(parent),
    guidePen(QPen(QBrush(QColor(THEME_GUIDE_PEN_DEFAULT)), 2, Qt::DashLine)),
    gridPen(QPen(QBrush(QColor(THEME_GRID_PEN_DEFAULT)), 2, Qt::SolidLine)),
    rulerTrackingPen(QPen(QBrush(QColor(THEME_RULER_TRACK_PEN_DEFAULT)), 2, Qt::SolidLine)),
    mValidItem(false),
    mPliPartGroup(false),
    mSceneGuides(false),
    mBaseItem(nullptr),
    mSnapToGrid(false),
    mRulerTracking(false),
    mGuidesCoordinates(false),
    mTrackingCoordinates(false),
    mGridSize(GridSizeTable[GRID_SIZE_INDEX_DEFAULT]),
    mGuidesPlacement(GUIDES_TOP_LEFT),
    mCoordMargin(0.125) // inches
{
    Q_ASSERT(mGridSize > 0);
}

void LGraphicsScene::updateGuidePos(){
    if (!mSceneGuides)
        return;

    if (mValidItem){
        if (mItemType == PointerGrabberObj ||
            mItemType == PliGrabberObj     ||
            mItemType == SubmodelGrabberObj||
            mGuidesPlacement == GUIDES_CENTRE)
            mGuidePos = QPointF(mBaseItem->sceneBoundingRect().center().x(),
                                mBaseItem->sceneBoundingRect().center().y());
        else
        if (mGuidesPlacement == GUIDES_TOP_LEFT)
            mGuidePos = QPointF(mBaseItem->sceneBoundingRect().left(),
                                mBaseItem->sceneBoundingRect().top());
        else
        if (mGuidesPlacement == GUIDES_TOP_RIGHT)
            mGuidePos = QPointF(mBaseItem->sceneBoundingRect().right(),
                                mBaseItem->sceneBoundingRect().top());
        else
        if (mGuidesPlacement == GUIDES_BOT_LEFT)
            mGuidePos = QPointF(mBaseItem->sceneBoundingRect().left(),
                                mBaseItem->sceneBoundingRect().bottom());
        else /* GUIDES_BOT_RIGHT */
            mGuidePos = QPointF(mBaseItem->sceneBoundingRect().right(),
                                mBaseItem->sceneBoundingRect().bottom());
    }
    else
    if (mPliPartGroup){
        if (mGuidesPlacement == GUIDES_TOP_LEFT)
            mGuidePos = QPointF(mBaseItem->parentItem()->sceneBoundingRect().left(),
                                mBaseItem->parentItem()->sceneBoundingRect().top());
        else
        if (mGuidesPlacement == GUIDES_TOP_RIGHT)
            mGuidePos = QPointF(mBaseItem->parentItem()->sceneBoundingRect().right(),
                                mBaseItem->parentItem()->sceneBoundingRect().top());
        else
        if (mGuidesPlacement == GUIDES_BOT_LEFT)
            mGuidePos = QPointF(mBaseItem->parentItem()->sceneBoundingRect().left(),
                                mBaseItem->parentItem()->sceneBoundingRect().bottom());
        else
        if (mGuidesPlacement == GUIDES_BOT_RIGHT)
            mGuidePos = QPointF(mBaseItem->parentItem()->sceneBoundingRect().right(),
                                mBaseItem->parentItem()->sceneBoundingRect().bottom());
        else /* GUIDES_CENTRE */
            mGuidePos = QPointF(mBaseItem->parentItem()->sceneBoundingRect().center().x(),
                                mBaseItem->parentItem()->sceneBoundingRect().center().y());
    }
    update();
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

    mItemType = SceneObject(mBaseItem->data(ObjectId).toInt());

    auto isExemptSceneObject = [](const SceneObject so)
    {
        for ( const auto eso : ExemptSceneObjects)
            if (eso == so)
                return true;
        return false;
    };

    if (!(mValidItem = !isExemptSceneObject(mItemType)))
        return false;

    return true;
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
        update();
    }
}

QMatrix LGraphicsScene::stableMatrix(const QMatrix &matrix, const QPointF &p)
{
    QMatrix newMatrix = matrix;

    qreal scaleX, scaleY;
    scaleX = newMatrix.m11();
    scaleY = newMatrix.m22();
    newMatrix.scale(1.0/scaleX, 1.0/scaleY);

    qreal offsetX, offsetY;
    offsetX = p.x()*(scaleX-1.0);
    offsetY = p.y()*(scaleY-1.0);
    newMatrix.translate(offsetX, offsetY);

    return newMatrix;
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

    QPen guidPosPen(QPen(QBrush(QColor(THEME_GUIDE_PEN_DEFAULT)), 0, Qt::SolidLine));
    QPen rulerTrackingPosPen(QPen(QBrush(QColor(THEME_RULER_TRACK_PEN_DEFAULT)), 0, Qt::SolidLine));
    QPen rulerCrosshairPen(QPen(QBrush(QColor(Qt::red)), 0, Qt::SolidLine));

    QPointF starPt;
    QPointF endPt;

    QPen savedPen = painter->pen();
    QFont f("times",24,0);
    setFont(f);
    QFontMetricsF fm(painter->font());
    qreal h = fm.height();

    if (mRulerTracking) {
        painter->setRenderHints(QPainter::TextAntialiasing | QPainter::HighQualityAntialiasing);
        painter->setOpacity(0.6);
        painter->setPen(rulerTrackingPen);
        starPt = mHorzCursorPos;
        starPt.setY(rect.top());
        endPt.setX(starPt.x());
        endPt.setY(rect.bottom());
        painter->drawLine(starPt,endPt);
        starPt = mVertCursorPos;
        starPt.setX(rect.left());
        endPt.setX(rect.right());
        endPt.setY(starPt.y());
        painter->drawLine(starPt,endPt);
        painter->setPen(savedPen);
        painter->setOpacity(1.0);

        if (mTrackingCoordinates) {
            QString t = QString("%1x%2 pixels")
                    .arg(QString::number(mHorzCursorPos.x(),'f',0))
                    .arg(QString::number(mVertCursorPos.y(),'f',0));
            qreal w = fm.width(t);
            QPointF p;
            switch (mGuidesPlacement) {
            case GUIDES_CENTRE:
            case GUIDES_TOP_LEFT:
                p = QPointF(mHorzCursorPos.x() - (w*2.5 + coordMargin()/**3 + 20*/),
                            mVertCursorPos.y() +  coordMargin()/*20*/);
                break;
            case GUIDES_BOT_LEFT:
                p = QPointF(mHorzCursorPos.x() - (w*2.5 + coordMargin()/**3 + 20*/),
                            mVertCursorPos.y() - (h*2.5 + coordMargin()/**2.5 + 20*/));
                break;
            case GUIDES_TOP_RIGHT:
                p = QPointF(mHorzCursorPos.x() + coordMargin()/*20*/,
                            mVertCursorPos.y() + coordMargin()/*20*/);
                break;
            case GUIDES_BOT_RIGHT:
                p = QPointF(mHorzCursorPos.x() +  coordMargin()/*20*/,
                            mVertCursorPos.y() - (h*2.5 + coordMargin()/**2.5 + 20*/));
                break;
            }
            QRectF r = QRectF(p.x(), p.y(), w + 5, h);
            painter->save();
            painter->setMatrix(stableMatrix(painter->worldMatrix(), p));
            painter->drawText(r, Qt::AlignHCenter | Qt::AlignVCenter, t);
            painter->setPen(rulerTrackingPosPen);
            painter->setOpacity(0.6);
            painter->drawRect(r);
            painter->setPen(savedPen);
            painter->setOpacity(1.0);
            painter->restore();
        }
    }

    if (! mSceneGuides || ! mValidItem)
        return;

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

    if (mGuidesCoordinates) {
        QString t = QString("%1x%2 pixels")
                .arg(QString::number(mGuidePos.x(),'f',0))
                .arg(QString::number(mGuidePos.y(),'f',0));
        qreal w = fm.width(t);
        QPointF p;
        qreal ver = mBaseItem->boundingRect().height()/2;
        qreal hor = mBaseItem->boundingRect().width()/2;
        switch (mGuidesPlacement) {
        case GUIDES_CENTRE:
            p = QPointF(mGuidePos.x() + (ver + 5),
                        mGuidePos.y() + (hor + 5));
            break;
        case GUIDES_TOP_LEFT:
            p = QPointF(mGuidePos.x() - (w*2.5 + coordMargin()/**3 + 20*/),
                        mGuidePos.y() - (h*2.5 + coordMargin()/**2.5 + 20*/));
            break;
        case GUIDES_BOT_LEFT:
            p = QPointF(mGuidePos.x() - (w*2.5 + coordMargin()/**3 + 20*/),
                        mGuidePos.y() +  coordMargin()/*20*/);
            break;
        case GUIDES_TOP_RIGHT:
            p = QPointF(mGuidePos.x() +  coordMargin()/*20*/,
                        mGuidePos.y() - (h*2.5 + coordMargin()/**2.5 + 20*/));
            break;
        case GUIDES_BOT_RIGHT:
            p = QPointF(mGuidePos.x() + coordMargin()/*20*/,
                        mGuidePos.y() + coordMargin()/*20*/);
            break;
        }
        QRectF r = QRectF(p.x(), p.y(), w + 5, h);

        painter->setPen(savedPen);
        painter->save();
        painter->setMatrix(stableMatrix(painter->worldMatrix(), p));
        painter->drawText(r, Qt::AlignHCenter | Qt::AlignVCenter, t);
        savedPen = painter->pen();
        painter->setPen(guidPosPen);
        painter->drawRect(r);
        painter->restore();
        painter->setPen(savedPen);
    }
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
    mMouseUpPos = event->scenePos();
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

