/****************************************************************************
**
** Copyright (C) 2015 Trevor SANDY. All rights reserved.
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

#ifndef DISPLAYPAGETHREAD_H
#define DISPLAYPAGETHREAD_H

#include <QObject>
#include <QMutex>
#include <QSize>
#include <QThread>
#include <QWaitCondition>

#include <QGraphicsView>

QT_BEGIN_NAMESPACE
class LGraphicsView;
QT_END_NAMESPACE

class DisplayPageThread : public QThread
{
  Q_OBJECT

public:
  DisplayPageThread(QObject *parent = 0);
  ~DisplayPageThread();

  drawPage(bool printing);

signals:
  drawnPage(const LGraphicsView &KpageView, const QGraphicsScene &KpageScene);

protected:
    void run();

public slots:

private:
    QMutex mutex;
    QWaitCondition condition;

    bool printing;

    bool start;
    bool abort;
};

#endif // DISPLAYPAGETHREAD_H
