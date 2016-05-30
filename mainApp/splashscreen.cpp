/****************************************************************************
**
** Copyright (C) 2016 Trevor SANDY. All rights reserved.
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

#include "splashscreen.h"

SplashScreen::SplashScreen(QWidget *parent) :
    QSplashScreen(parent), m_progress(0)

{
    this->setAttribute(Qt::WA_TranslucentBackground);
    this->setPixmap(QPixmap(":/resources/LPub512Splash.png"));
    this->setCursor(Qt::BusyCursor);

    QFont splashFont;
    splashFont.setFamily("Arial");
    splashFont.setPixelSize(14);
    splashFont.setStretch(130);
    this->setFont(splashFont);
}

void SplashScreen::drawContents(QPainter *painter)
{
  QSplashScreen::drawContents(painter);

  // Set style for progressbar...
  QStyleOptionProgressBarV2 pbstyle;
  pbstyle.initFrom(this);
  pbstyle.state = QStyle::State_Enabled;
  pbstyle.textVisible = false;
  pbstyle.minimum = 0;
  pbstyle.maximum = 100;
  pbstyle.progress = m_progress;
  pbstyle.invertedAppearance = false;
  pbstyle.rect = QRect(0, 515, 380, 19); // Where is it.

  // Draw it...
  style()->drawControl(QStyle::CE_ProgressBar, &pbstyle, painter, this);
}

void SplashScreen::updateMessage(const QString &message){
  this->showMessage(message, Qt::AlignBottom | Qt::AlignCenter, Qt::white);
}
