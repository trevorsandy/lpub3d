/****************************************************************************
**
** Copyright (C) 2018 Trevor SANDY. All rights reserved.
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
  : pageBackgroundItem(NULL),
    fitMode(FitVisible),
    mPageRect(QRectF(0,0,0,0)),
    mGridLayoutSet(false),
    mGridLayout(NULL)
{
  setScene(scene);
  setAcceptDrops(true);

  connect(this,SIGNAL(setPageGuidesSig(bool)),  scene,SLOT(setPageGuides(bool)));
  connect(this,SIGNAL(setGuidePenSig(Theme)),   scene,SLOT(setGuidePen(Theme)));
  connect(this,SIGNAL(setSceneThemeSig(Theme)), scene,SLOT(setGuidePen(Theme)));
  connect(this,SIGNAL(setSceneThemeSig(Theme)), this, SLOT(setPageRuler(Theme)));
  connect(this,SIGNAL(setSceneThemeSig(Theme)), this, SLOT(setSceneBackground(Theme)));

}

void LGraphicsView::setPageGuides(Theme t){
  emit setGuidePenSig(t);
  emit setPageGuidesSig(Preferences::pageGuides);
}

void LGraphicsView::setPageRuler(Theme t){

  if (Preferences::pageRuler) {

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

      LRuler * mHorzRuler = new LRuler(LRuler::Horizontal,t,fake);
      LRuler * mVertRuler = new LRuler(LRuler::Vertical,t,fake);

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
  scale(1/unity.width(), 1 / unity.height());

  int margin = 2;
  QRectF viewRect = viewport()->rect().adjusted(margin, margin, -margin, -margin);
  QRectF sceneRect = matrix().mapRect(mPageRect);
  qreal xratio = viewRect.width() / sceneRect.width();

  scale(xratio,xratio);
  centerOn(mPageRect.center());
  fitMode = FitWidth;
}

void LGraphicsView::actualSize(){
  resetMatrix();
  fitMode = FitNone;
}

void LGraphicsView::zoomIn(){
  scale(1.1,1.1);
  fitMode = FitNone;
}

void LGraphicsView::zoomOut(){
  scale(1.0/1.1,1.0/1.1);
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
}

/* Theme related */
void LGraphicsView::setSceneBackground(Theme t){
  QColor c;
  c = t == ThemeDark ? QColor(THEME_MAIN_BGCOLOR_DARK) :
                       QColor(THEME_MAIN_BGCOLOR_DEFAULT) ;
  this->scene()->setBackgroundBrush(c);
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
  if (mOrigin != origin)
  {
    mOrigin = origin;
    update();
  }
}

void LRuler::setRulerTickPen(Theme t) {
  t == ThemeDark ? mRulerTickPen.setColor(THEME_TICK_PEN_DARK) :
                   mRulerTickPen.setColor(THEME_TICK_PEN_DEFAULT);
  mRulerTickPen.setWidth(0);  // zero width pen is cosmetic pen
}

void LRuler::setRulerNMLPen(Theme t) {
  t == ThemeDark ? mRulerNMLPen.setColor(THEME_NML_PEN_DARK) :
                   mRulerNMLPen.setColor(THEME_NML_PEN_DEFAULT);
  mRulerNMLPen.setWidth(2);
}

void LRuler::setRulerColor(Theme t){
  mRulerColor = t == ThemeDark ? QColor(THEME_MAIN_BGCOLOR_DARK) :
                                 QColor(THEME_MAIN_BGCOLOR_DEFAULT);
}

void LRuler::setRulerUnit(const qreal rulerUnit)
{
  if (mRulerUnit != rulerUnit)
  {
    mRulerUnit = rulerUnit;
    update();
  }
}

void LRuler::setRulerZoom(const qreal rulerZoom)
{
  if (mRulerZoom != rulerZoom)
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
  painter.fillRect(rulerRect,mRulerColor);

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
    QPoint starPt = mCursorPos;
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
  }
}
