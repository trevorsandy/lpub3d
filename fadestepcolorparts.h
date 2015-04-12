#ifndef FADESTEPCOLORPARTS_H
#define FADESTEPCOLORPARTS_H

#include <QString>
#include <QStringList>

class FadeStepColorParts
{
  private:
    static bool     				result;
    static QList<QString>			fadeStepStaticColorParts;
  public:
    FadeStepColorParts();
    static const bool &isStaticColorPart(QString part);
};

#endif // FADESTEPCOLORPARTS_H
