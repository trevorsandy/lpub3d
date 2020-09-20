/****************************************************************************
**
** Copyright (C) 2018 - 2020 Trevor SANDY. All rights reserved.
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

#include "lgraphicsview.h"

#include <QtOpenGL>

// drag and drop
#include <QDragLeaveEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QUrl>
#include <QList>
#include <QPoint>

#include "lgraphicsscene.h"
#include "lpub.h"

typedef QList<QObject*> LDVQObjectList;

LGraphicsView::LGraphicsView(LGraphicsScene *scene)
  : pageBackgroundItem(nullptr),
    fitMode(FitVisible),
    mPageRect(QRectF(0,0,0,0)),
    mGridLayoutSet(false),
    mGridLayout(nullptr)
{
  setScene(scene);
  setAcceptDrops(true);

  connect(this,SIGNAL(setGridSizeSig(int)),           scene,SLOT(setGridSize(int)));
  connect(this,SIGNAL(setSceneGuidesLineSig(int)),    scene,SLOT(setSceneGuidesLine(int)));
  connect(this,SIGNAL(setSceneGuidesPosSig(int)),     scene,SLOT(setSceneGuidesPos(int)));
  connect(this,SIGNAL(setSnapToGridSig(bool)),        scene,SLOT(setSnapToGrid(bool)));
  connect(this,SIGNAL(setSceneGuidesSig(bool)),       scene,SLOT(setSceneGuides(bool)));
  connect(this,SIGNAL(setGuidePenSig(QString,int)),   scene,SLOT(setGuidePen(QString,int)));
  connect(this,SIGNAL(setGridPenSig(QString)),        scene,SLOT(setGridPen(QString)));
  connect(this,SIGNAL(setResolutionSig(float)),       scene,SLOT(setResolution(float)));
  connect(this,SIGNAL(setShowCoordinates(bool,bool)), scene,SLOT(setShowCoordinates(bool,bool)));

  connect(this,  SIGNAL(setSceneHorzRulerPositionSig(QPointF)),
          scene, SLOT(  setSceneHorzRulerPosition(   QPointF)));

  connect(this,  SIGNAL(setSceneVertRulerPositionSig(QPointF)),
          scene, SLOT(  setSceneVertRulerPosition(   QPointF)));

  connect(this,  SIGNAL(setSceneRulerTrackingPenSig(QString)),
          scene, SLOT(  setSceneRulerTrackingPen(   QString)));

  connect(this,  SIGNAL(setSceneRulerTrackingSig(bool)),
          scene, SLOT(  setSceneRulerTracking(   bool)));

}

void LGraphicsView::setSceneRuler(){

  if (Preferences::sceneRuler) {

      if (mGridLayoutSet) {
          removeGridItem(mGridLayout,0,0,true);
          removeGridItem(mGridLayout,0,1,true);
          removeGridItem(mGridLayout,1,0,true);
        } else {
          mGridLayout = new QGridLayout();
          mGridLayout->setSpacing(0);
          mGridLayout->setMargin(0);
        }

      setViewportMargins(RULER_BREADTH,RULER_BREADTH,0,0);

      QWidget* fake = new QWidget();
      fake->setBackgroundRole(QPalette::Window);
      fake->setFixedSize(RULER_BREADTH,RULER_BREADTH);

      mHorzRuler = new LRuler(LRuler::Horizontal,fake);
      mVertRuler = new LRuler(LRuler::Vertical,fake);

      connect(mHorzRuler,SIGNAL(setRulerPositionSig(  QPoint)),
              this,      SLOT(  setSceneHorzRulerPosition(QPoint)));

      connect(mVertRuler,SIGNAL(setRulerPositionSig(  QPoint)),
              this,      SLOT(  setSceneVertRulerPosition(QPoint)));

      mGridLayout->addWidget(fake,0,0);
      mGridLayout->addWidget(mHorzRuler,0,1);
      mGridLayout->addWidget(mVertRuler,1,0);

      if (! mGridLayoutSet) {
          mGridLayout->addWidget(this->viewport(),1,1);
          this->setLayout(mGridLayout);
          mGridLayoutSet = true;
        } else {
          update();
        }

      setSceneRulerTracking();

    } else {

      setViewportMargins(0,0,0,0);

      if (mGridLayoutSet) {
          removeGridItem(mGridLayout,0,0,true);
          removeGridItem(mGridLayout,0,1,true);
          removeGridItem(mGridLayout,1,0,true);
        }
      update();
    }
}

void LGraphicsView::setSceneRulerTracking(){
    if (!Preferences::sceneRuler)
        return;

    if (mHorzRuler)
        mHorzRuler->setMouseTrack(Preferences::sceneRulerTracking != TRACKING_NONE);
    if (mVertRuler)
        mVertRuler->setMouseTrack(Preferences::sceneRulerTracking != TRACKING_NONE);

    emit setSceneRulerTrackingSig(Preferences::sceneRulerTracking == TRACKING_LINE);
    if (Preferences::sceneRulerTracking == TRACKING_LINE) {
        emit setSceneRulerTrackingPenSig(Preferences::sceneRulerTrackingColor);
        emit setSceneVertRulerPositionSig(mapToScene(QPoint(0,0)));
        emit setSceneHorzRulerPositionSig(mapToScene(QPoint(0,0)));
        setShowCoordinates();
    }
}

void LGraphicsView::setSceneVertRulerPosition(QPoint p)
{
    emit setSceneVertRulerPositionSig(mapToScene(p));
}

void LGraphicsView::setSceneHorzRulerPosition(QPoint p)
{
    emit setSceneHorzRulerPositionSig(mapToScene(p));
}

void LGraphicsView::setGridSize(){
  if (Preferences::snapToGrid)
    emit setGridSizeSig(GridSizeTable[Preferences::gridSizeIndex]);
}

void LGraphicsView::setShowCoordinates(){
  bool guides = Preferences::sceneGuides &&
                Preferences::showGuidesCoordinates;
  bool tracking = Preferences::sceneRuler &&
                  Preferences::sceneRulerTracking == TRACKING_LINE &&
                  Preferences::showTrackingCoordinates;
  emit setShowCoordinates(guides,tracking);
}

void LGraphicsView::setSnapToGrid(){
  emit setSnapToGridSig(Preferences::snapToGrid);
  if (Preferences::snapToGrid) {
    emit setGridPenSig(Preferences::sceneGridColor);
    emit setGridSizeSig(GridSizeTable[Preferences::gridSizeIndex]);
  }
}

void LGraphicsView::setSceneGuides(){
  emit setSceneGuidesSig(Preferences::sceneGuides);
  if (Preferences::sceneGuides) {
    emit setGuidePenSig(Preferences::sceneGuideColor,
                        Preferences::sceneGuidesLine);
    emit setSceneGuidesLineSig(Preferences::sceneGuidesLine);
    emit setSceneGuidesPosSig(Preferences::sceneGuidesPosition);
    setShowCoordinates();
  }
}

void LGraphicsView::setSceneGuidesLine(){
  if (Preferences::sceneGuides) {
    emit setSceneGuidesLineSig(Preferences::sceneGuidesLine);
  }
}

void LGraphicsView::setSceneGuidesPos(){
  if (Preferences::sceneGuides)
    emit setSceneGuidesPosSig(Preferences::sceneGuidesPosition);
}

void LGraphicsView::setSceneBackgroundBrush(){
  this->scene()->setBackgroundBrush(QColor(Preferences::sceneBackgroundColor));
}

void LGraphicsView::setSceneTheme(){
  setSceneBackgroundBrush();
  setSceneRuler();
  setSceneGuides();
  setSceneGuidesLine();
  setSceneGuidesPos();
  setSnapToGrid();
}

void LGraphicsView::setResolution(float r){
  emit setResolutionSig(r);
}

void LGraphicsView::fitVisible(const QRectF rect)
{

  scale(1.0,1.0);
  mPageRect = rect;

  QRectF unity = matrix().mapRect(QRectF(0,0,1,1));
  scale(1/unity.width(), 1 / unity.height());

  int margin = 2;
  QRectF viewRect = viewport()->rect().adjusted(margin, margin, -margin, -margin);
  QRectF sceneRect = matrix().mapRect(mPageRect);
  qreal xratio = viewRect.width() / sceneRect.width();
  qreal yratio = viewRect.height() / sceneRect.height();

  xratio = yratio = qMin(xratio,yratio);
  xratio = yratio = qMin(xratio,yratio);
  xratio = yratio = qMin(xratio,yratio);
  scale(xratio,yratio);
  centerOn(mPageRect.center());
  fitMode = FitVisible;
}

void LGraphicsView::fitWidth(const QRectF rect)
{
  scale(1.0,1.0);
  mPageRect = rect;

  QRectF unity = matrix().mapRect(QRectF(0,0,1,1));
  scale(1 / unity.width(), 1 / unity.height());

  int margin = 2;
  QRectF viewRect = viewport()->rect().adjusted(margin, margin, -margin, -margin);
  QRectF sceneRect = matrix().mapRect(mPageRect);
  qreal xratio = viewRect.width() / sceneRect.width();

  scale(xratio,xratio);
  fitMode = FitWidth;
}

void LGraphicsView::fitScene(const QRectF rect)
{
  QRectF bounds = rect; // Scene->itemsBoundingRect()
  int margin = 2;
  bounds.setWidth(bounds.width()+margin*frameWidth());
  bounds.setHeight(bounds.height()+margin*frameWidth());
  fitInView(bounds, Qt::KeepAspectRatio);
  centerOn(bounds.center());
  fitMode = FitNone;
}

void LGraphicsView::actualSize(){
  resetMatrix();
  fitMode = FitNone;
}

void LGraphicsView::resizeEvent(QResizeEvent *event)
{
  Q_UNUSED(event);
  if (pageBackgroundItem) {
      if (fitMode == FitVisible) {
          fitVisible(mPageRect);
        } else if (fitMode == FitWidth) {
          fitWidth(mPageRect);
        }
    }
  centerOn(viewport()->rect().center());
}

void LGraphicsView::zoomIn(){
  scale(1.1,1.1);
  fitMode = FitNone;
}

void LGraphicsView::zoomOut(){
  scale(1.0/1.1,1.0/1.1);
  fitMode = FitNone;
}

void LGraphicsView::wheelEvent(QWheelEvent *event){
    if (event->delta() < 0) {
        scale(1.0/1.1,1.0/1.1);
    } else {
        scale(1.1,1.1);
    }
    fitMode = FitNone;
}

/* drag and drop */
void LGraphicsView::dragMoveEvent(QDragMoveEvent *event){
  if (event->mimeData()->hasUrls()) {
      event->acceptProposedAction();
    }
}

void LGraphicsView::dragEnterEvent(QDragEnterEvent *event){
  if (event->mimeData()->hasUrls()) {
      event->acceptProposedAction();
    }
}

void LGraphicsView::dragLeaveEvent(QDragLeaveEvent *event){
  event->accept();
}

void LGraphicsView::dropEvent(QDropEvent *event){
  const QMimeData* mimeData = event->mimeData();
  if (mimeData->hasUrls()) {
      QList<QUrl> urlList = mimeData->urls();
      QString fileName = urlList.at(0).toLocalFile();   // load the first file only
      if (urlList.size() > 1) {
          emit gui->messageSig(LOG_ERROR, QMessageBox::tr("%1 files selected.\nOnly file %2 will be opened.")
                               .arg(urlList.size())
                               .arg(fileName));
        }
      gui->openDropFile(fileName);
      event->acceptProposedAction();
    }
}

/**
 * Removes all layout items within the given @a layout
 * which either span the given @a row or @a column. If @a deleteWidgets
 * is true, all concerned child widgets become not only removed from the
 * layout, but also deleted.
 */
void LGraphicsView::removeGridItem(QGridLayout *layout, int row, int column, bool deleteWidgets) {
    // We avoid usage of QGridLayout::itemAtPosition() here to improve performance.
    for (int i = layout->count() - 1; i >= 0; i--) {
        int r, c, rs, cs;
        layout->getItemPosition(i, &r, &c, &rs, &cs);
        if ((r <= row && r + rs - 1 >= row) && (c <= column && c + cs - 1 >= column)) {
            // This layout item is subject to deletion.
            QLayoutItem *item = layout->takeAt(i);
            if (deleteWidgets) {
                deleteGridWidgets(item);
            }
            delete item;
        }
    }
}

/**
 * Helper function. Deletes all child widgets of the given layout @a item.
 */
void LGraphicsView::deleteGridWidgets(QLayoutItem *item) {
    if (item->layout()) {
        // Process all child items recursively.
        for (int i = 0; i < item->layout()->count(); i++) {
            deleteGridWidgets(item->layout()->itemAt(i));
        }
    }
    delete item->widget();
}

/* ruler block */
void LRuler::setOrigin(const qreal origin)
{
  if (mOrigin < origin || mOrigin > origin)
  {
    mOrigin = origin;
    update();
  }
}

void LRuler::setRulerTickPen() {
  // zero width pen is cosmetic pen
  mRulerTickPen =  QPen(QBrush(QColor(Preferences::sceneRulerTickColor)), 0, Qt::SolidLine);
}

void LRuler::setRulerNMLPen() {
  mRulerNMLPen = QPen(QBrush(QColor(Preferences::sceneGridColor)), 2, Qt::SolidLine);
}

void LRuler::setRulerTrackingPen() {
  mRulerTrackingPen = QPen(QBrush(QColor(Preferences::sceneRulerTrackingColor)), 2, Qt::SolidLine);
}

void LRuler::setRulerBackgroundColor(){
  mRulerBgColor = QColor(Preferences::sceneBackgroundColor);
}

void LRuler::setRulerUnit(const qreal rulerUnit)
{
  if (mRulerUnit < rulerUnit || mRulerUnit > rulerUnit)
  {
    mRulerUnit = rulerUnit;
    update();
  }
}

void LRuler::setRulerZoom(const qreal rulerZoom)
{
  if (mRulerZoom < rulerZoom || mRulerZoom > rulerZoom)
  {
    mRulerZoom = rulerZoom;
    update();
  }
}


void LRuler::setCursorPos(const QPoint cursorPos)
{
  mCursorPos = this->mapFromGlobal(cursorPos);
  mCursorPos += QPoint(RULER_BREADTH,RULER_BREADTH);
  update();
}

void LRuler::setMouseTrack(const bool track)
{
  if (mMouseTracking != track)
  {
    mMouseTracking = track;
    update();
  }
}

void LRuler::mouseMoveEvent(QMouseEvent* event)
{
  mCursorPos = event->pos();
  update();
  QWidget::mouseMoveEvent(event);
}

void LRuler::paintEvent(QPaintEvent* event)
{
  Q_UNUSED(event);
  QPainter painter(this);
    painter.setRenderHints(QPainter::TextAntialiasing | QPainter::HighQualityAntialiasing);
    QPen pen(mRulerTickPen);
    pen.setCosmetic(true);
    painter.setPen(pen);
  // We want to work with floating point, so define the rect as QRectF
  QRectF rulerRect = this->rect();

  // at first fill the rect
  painter.fillRect(rulerRect,mRulerBgColor);

  // drawing a scale of 25
  drawAScaleMeter(&painter,rulerRect,10,(Horizontal == mRulerType ? rulerRect.height()
        : rulerRect.width())/2);
  // drawing a scale of 50
  drawAScaleMeter(&painter,rulerRect,50,(Horizontal == mRulerType ? rulerRect.height()
        : rulerRect.width())/4);
  // drawing a scale of 100
  mDrawText = true;
  drawAScaleMeter(&painter,rulerRect,100,0);
  mDrawText = false;

  // drawing the current mouse position indicator
    painter.setOpacity(0.4);
    drawMousePosTick(&painter);
    painter.setOpacity(1.0);

  // drawing no man's land between the ruler & view
  QPointF starPt = Horizontal == mRulerType ? rulerRect.bottomLeft()
      : rulerRect.topRight();
  QPointF endPt = Horizontal == mRulerType ? rulerRect.bottomRight()
      : rulerRect.bottomRight();
  painter.setPen(mRulerNMLPen);
  painter.drawLine(starPt,endPt);
}

void LRuler::drawAScaleMeter(QPainter* painter, QRectF rulerRect, qreal scaleMeter, qreal startPositoin)
{
  // Flagging whether we are horizontal or vertical only to reduce
  // to cheching many times
  bool isHorzRuler = Horizontal == mRulerType;

  scaleMeter  = scaleMeter * mRulerUnit * mRulerZoom;

  // Ruler rectangle starting mark
  qreal rulerStartMark = isHorzRuler ? rulerRect.left() : rulerRect.top();
  // Ruler rectangle ending mark
  qreal rulerEndMark = isHorzRuler ? rulerRect.right() : rulerRect.bottom();

  // Condition A # If origin point is between the start & end mard,
  // we have to draw both from origin to left mark & origin to right mark.
  // Condition B # If origin point is left of the start mark, we have to draw
  // from origin to end mark.
  // Condition C # If origin point is right of the end mark, we have to draw
  // from origin to start mark.
  if (mOrigin >= rulerStartMark && mOrigin <= rulerEndMark)
  {
    drawFromOriginTo(painter, rulerRect, mOrigin, rulerEndMark, 0, scaleMeter, startPositoin);
    drawFromOriginTo(painter, rulerRect, mOrigin, rulerStartMark, 0, -scaleMeter, startPositoin);
  }
  else if (mOrigin < rulerStartMark)
  {
        int tickNo = int((rulerStartMark - mOrigin) / scaleMeter);
        drawFromOriginTo(painter, rulerRect, mOrigin + scaleMeter * tickNo,
            rulerEndMark, tickNo, scaleMeter, startPositoin);
  }
  else if (mOrigin > rulerEndMark)
  {
        int tickNo = int((mOrigin - rulerEndMark) / scaleMeter);
    drawFromOriginTo(painter, rulerRect, mOrigin - scaleMeter * tickNo,
            rulerStartMark, tickNo, -scaleMeter, startPositoin);
  }
}

void LRuler::drawFromOriginTo(QPainter* painter, QRectF rulerRect, qreal startMark, qreal endMark, int startTickNo, qreal step, qreal startPosition)
{
  bool isHorzRuler = Horizontal == mRulerType;
  int iterate = 0;

  for (qreal current = startMark;
      (step < 0 ? current >= endMark : current <= endMark); current += step)
  {
    qreal x1 = isHorzRuler ? current : rulerRect.left() + startPosition;
    qreal y1 = isHorzRuler ? rulerRect.top() + startPosition : current;
    qreal x2 = isHorzRuler ? current : rulerRect.right();
    qreal y2 = isHorzRuler ? rulerRect.bottom() : current;
    painter->drawLine(QLineF(x1,y1,x2,y2));
    if (mDrawText)
    {
      QPainterPath txtPath;
      txtPath.addText(x1 + 1,y1 + (isHorzRuler ? 7 : -2),this->font(),QString::number(qAbs(int(step) * startTickNo++)));
      painter->drawPath(txtPath);
      iterate++;
    }
  }
}

void LRuler::drawMousePosTick(QPainter* painter)
{
  if (mMouseTracking)
  {
    QPen savedPen = painter->pen();
    QPen pen(mRulerTrackingPen);
    painter->setPen(pen);

    QPoint starPt = mCursorPos;
    emit setRulerPositionSig(mCursorPos);

    QPoint endPt;
    if (Horizontal == mRulerType)
    {
      starPt.setY(this->rect().top());
      endPt.setX(starPt.x());
      endPt.setY(this->rect().bottom());
    }
    else
    {
      starPt.setX(this->rect().left());
      endPt.setX(this->rect().right());
      endPt.setY(starPt.y());
    }
    painter->drawLine(starPt,endPt);
    painter->setPen(savedPen);
  }
}
