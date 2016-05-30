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

#ifndef SPLASHSCREEN_H
#define SPLASHSCREEN_H

#include "version.h"
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
#include <QtWidgets/QApplication>
#else
#include <QApplication>
#endif
#include <QObject>
#include <QSplashScreen>

#include "QsLog.h"

class SplashScreen : public QSplashScreen
{
    Q_OBJECT

public:
  explicit SplashScreen(QWidget *parent = 0);

    int m_progress;

public slots:
    void setProgress(int value)
    {
      m_progress = value;
      if (m_progress > 100)
        m_progress = 100;
      if (m_progress < 0)
        m_progress = 0;
      repaint();
    }
    void updateMessage(const QString &message);

protected:
    void drawContents(QPainter *painter);

};

#endif // SPLASHSCREEN_H
