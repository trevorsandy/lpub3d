#ifndef ANNOTATIONS_H
#define ANNOTATIONS_H

#include <QHash>
#include <QString>
#include <QStringList>

class Annotations {
  private:
    static QString     				empty;
    static QList<QString>			titleAnnotations;
    static QHash<QString, QString> 	freeformAnnotations;
  public:
    Annotations();
    static const QString &freeformAnnotation(QString part);
    static const QList<QString> getTitleAnnotations()
    {
        return titleAnnotations;
    }
};

#endif
