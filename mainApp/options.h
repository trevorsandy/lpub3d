#ifndef OPTIONS_H
#define OPTIONS_H

#include <QString>
#include <QStringList>

namespace Options
{
    enum Mt { PLI, CSI ,SMP};
}

class xyzVector
{
private:
bool _populated;

public:
  xyzVector()
  {
    _populated  = false;
  }
  xyzVector(const float _x, const float _y, const float _z)
      : x(_x), y(_y), z(_z)
  {
    _populated  = x != 0.0f || y != 0.0f || z != 0.0f;
  }
  bool isPopulated()
  {
    return _populated;
  }

  friend bool operator==(const xyzVector& a, const xyzVector& b);
  friend bool operator!=(const xyzVector& a, const xyzVector& b);

  float x, y, z;
};

inline bool operator==(const xyzVector &a, const xyzVector &b)
{
    return a.x == b.x && a.y == b.y && a.z == b.z;
}

inline bool operator!=(const xyzVector &a, const xyzVector &b)
{
    return a.x != b.x || a.y != b.y || a.z != b.z;
}

class ViewerOptions
{
public:
  ViewerOptions()
  {
    ImageType      = Options::CSI;
    CameraName     = QString();
    UsingViewpoint = false;
    IsOrtho        = false;
    StudLogo       = 0;
    ImageWidth     = 800 ;
    ImageHeight    = 600;
    PageWidth      = 800;
    PageHeight     = 600;
    NativeCDF      = -260;
    Resolution     = 150.0f;
    ModelScale     = 1.0f;
    CameraDistance = 0.0f;
    FoV            = 0.0f;
    ZNear          = 0.0f;
    ZFar           = 0.0f;
    Latitude       = 0.0f;
    Longitude      = 0.0f;
    RotStep        = xyzVector(0, 0, 0);
    Target         = xyzVector(0, 0, 0);
  }
//  virtual ~ViewerOptions(){}
  QString ViewerStepKey;
  Options::Mt ImageType;
  QString ImageFileName;
  QString RotStepType;
  QString CameraName;
  int ImageWidth;
  int ImageHeight;
  int PageWidth;
  int PageHeight;
  int NativeCDF;
  int StudLogo;
  float Resolution;
  float ModelScale;
  float CameraDistance;
  float FoV;
  float ZNear;
  float ZFar;
  float Latitude;
  float Longitude;
  bool UsingViewpoint;
  bool IsOrtho;
  xyzVector RotStep;
  xyzVector Target;
};

class NativeOptions : public ViewerOptions
{
public:
  NativeOptions(const ViewerOptions &rhs)
      : ViewerOptions(rhs),
        IniFlag(-1),
        ExportMode(-1),
        LineWidth(1.0),
        TransBackground(true),
        HighlightNewParts(false)
  { }
  NativeOptions()
      : ViewerOptions()
  {
    TransBackground   = true;
    HighlightNewParts = false;
    LineWidth         = 1.0;
    ExportMode        = -1; //NONE
    IniFlag           = -1; //NONE
  }
//  virtual ~NativeOptions(){}
  QStringList ExportArgs;
  QString InputFileName;
  QString OutputFileName;
  QString ExportFileName;
  int IniFlag;
  int ExportMode;
  float LineWidth;
  bool TransBackground;
  bool HighlightNewParts;
};

#endif // OPTIONS_H
