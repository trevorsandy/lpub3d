/****************************************************************************
**
** Copyright (C) 2018 - 2022 Trevor SANDY. All rights reserved.
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

#include "lpub_object.h"

LPub *lpub;

LPub::LPub()
{
  lpub = this;
}

LPub::~LPub()
{
  lpub = nullptr;
}

QString LPub::elapsedTime(const qint64 &duration) {

    qint64 elapsed = duration;
    int milliseconds = int(elapsed % 1000);
    elapsed /= 1000;
    int seconds = int(elapsed % 60);
    elapsed /= 60;
    int minutes = int(elapsed % 60);
    elapsed /= 60;
    int hours = int(elapsed % 24);

    return QString("Elapsed time: %1%2%3")
                   .arg(hours   >   0 ?
                                  QString("%1 %2 ")
                                          .arg(hours)
                                          .arg(hours   > 1 ? "hours"   : "hour") :
                                  QString())
                   .arg(minutes > 0 ?
                                  QString("%1 %2 ")
                                          .arg(minutes)
                                          .arg(minutes > 1 ? "minutes" : "minute") :
                                  QString())
                   .arg(QString("%1.%2 %3")
                                .arg(seconds)
                                .arg(milliseconds,3,10,QLatin1Char('0'))
                                .arg(seconds > 1 ? "seconds" : "second"));

}
