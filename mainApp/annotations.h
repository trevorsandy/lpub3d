/****************************************************************************
**
** Copyright (C) 2015 - 2019 Trevor SANDY. All rights reserved.
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

#ifndef ANNOTATIONS_H
#define ANNOTATIONS_H

#include <QHash>
#include <QString>
#include <QStringList>

class Annotations {
  private:
    static int                         returnInt;
    static QString                     returnString;
    static QList<QString>              titleAnnotations;
    static QHash<QString, QString>     freeformAnnotations;
    static QHash<QString, QStringList> annotationStyles;

    static QHash<QString, QStringList> blCodes;
    static QHash<QString, QString>     legoElements;
    static QHash<QString, QString>     blColors;
    static QHash<QString, QString>     ld2blColorsXRef;
    static QHash<QString, QString>     ld2blCodesXRef;

    static QHash<QString, QString>     ld2rbColorsXRef;
    static QHash<QString, QString>     ld2rbCodesXRef;
  public:
    Annotations();
    static const QString &freeformAnnotation(QString part);
    static const int &getAnnotationStyle(QString part);
    static const int &getAnnotationCategory(QString part);
    static const QString &getStyleAnnotation(QString part);
    static void loadDefaultAnnotationStyles(QByteArray &Buffer);
    static void loadTitleAnnotations(QByteArray& Buffer);

    static void loadBLColors(QByteArray &Buffer);
    static void loadLD2BLColorsXRef(QByteArray &Buffer);
    static void loadLD2BLCodesXRef(QByteArray &Buffer);

    static void loadLD2RBColorsXRef(QByteArray &Buffer);
    static void loadLD2RBCodesXRef(QByteArray &Buffer);

    static bool loadBLCodes();
    static bool loadBLCodes(QByteArray &Buffer);
    static bool loadLEGOElements();

    static const QString &getBLColorID(QString blcolorname);
    static const QString &getLEGOElement(QString elementkey);
    static const QString &getBLElement(QString ldcolorid,
                                       QString ldpartid,
                                       int     which = 0);
    static const QString &getBrickLinkPartId(QString ldpartid);
    static const int &getBrickLinkColor(int ldcolorid);
    static const int &getRBColorID(QString blcolorid);
    static const QString &getRBPartID(QString blpartid);

    static bool exportBLColorsFile();
    static bool exportLD2BLColorsXRefFile();
    static bool exportLD2BLCodesXRefFile();
    static bool exportLD2RBColorsXRefFile();
    static bool exportLD2RBCodesXRefFile();

    static bool exportAnnotationStyleFile();
    static bool exportTitleAnnotationsFile();
    static bool exportfreeformAnnotationsHeader();
    static bool overwriteFile(const QString &file);

    static const QList<QString> getTitleAnnotations()
    {
        return titleAnnotations;
    }
};

#endif
