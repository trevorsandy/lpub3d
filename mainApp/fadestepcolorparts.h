#ifndef FADESTEPCOLORPARTS_H
#define FADESTEPCOLORPARTS_H

#include <QString>
#include <QStringList>

class FadeStepColorParts
{
  private:
    static bool     				result;
    static QString     				empty;
    static QString                              path;
    static QMap<QString, QString>               fadeStepStaticColorParts;
  public:
    FadeStepColorParts();
    static const bool &isStaticColorPart(QString part);
    static const bool &getStaticColorPartPath(QString &part);
    static const QString &staticColorPartPath(QString part);
};

#endif // FADESTEPCOLORPARTS_H
