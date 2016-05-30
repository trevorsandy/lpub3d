 
/****************************************************************************
**
** Copyright (C) 2007-2009 Kevin Clague. All rights reserved.
** Copyright (C) 2016 Trevor SANDY. All rights reserved.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://www.trolltech.com/products/qt/opensource.html
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "lpub.h"
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
#include <QtWidgets>
#else
#include <QtGui>
#endif
#include <QStringList>
#include <QFile>
#include <QTextStream>
#include <QRegExp>
#include <math.h>
#include "render.h"
#include "ldrawfiles.h"



/*****************************************************************************
 * Rotation routines
 ****************************************************************************/

void
matrixMakeRot(
  double rm[3][3],
  double rots[3])
{
  double pi = 2*atan2(1.0,0.0);

  double s1 = sin(2*pi*rots[0]/360.0);
  double c1 = cos(2*pi*rots[0]/360.0);
  double s2 = sin(2*pi*rots[1]/360.0);
  double c2 = cos(2*pi*rots[1]/360.0);
  double s3 = sin(2*pi*rots[2]/360.0);
  double c3 = cos(2*pi*rots[2]/360.0);

  rm[0][0] = c2*c3;
  rm[0][1] = -c2*s3;
  rm[0][2] = s2;

  rm[1][0] = c1 * s3 + s1 * s2 * c3;
  rm[1][1] = c1 * c3 - s1 * s2 * s3;
  rm[1][2] = -s1 * c2;

  rm[2][0] = s1 * s3 - c1 * s2 * c3;
  rm[2][1] = s1 * c3 + c1 * s2 * s3;
  rm[2][2] = c1 * c2;
}

void
matrixCp(
  double dst[3][3],
  double src[3][3])
{
  int i,j;

  for (i = 0; i < 3; i++) {
    for (j = 0; j < 3; j++) {
      dst[i][j] = src[i][j];
    }
  }
}

void
matrixMult3(
  double res[3][3],
  double lft[3][3],
  double rht[3][3])
{
  int i,j,k;

  for (i = 0; i < 3; i++) {
    for (j = 0; j < 3; j++) {
      res[i][j] = 0.0;
    }
  }

  for (i = 0; i < 3; i++) {
    for (j = 0; j < 3; j++) {
      for (k = 0; k < 3; k++) {
        res[i][j] += lft[i][k] * rht[k][j];
      }
    }
  }
}

void
matrixMult(
  double res[3][3],
  double src[3][3])
{
  double t[3][3];

  matrixCp(t,res);
  matrixMult3(res,t,src);
}

void rotatePoint(
  double p[3],
  double rm[3][3])
{
  double X = rm[0][0]*p[0] + rm[0][1]*p[1] + rm[0][2]*p[2];
  double Y = rm[1][0]*p[0] + rm[1][1]*p[1] + rm[1][2]*p[2];
  double Z = rm[2][0]*p[0] + rm[2][1]*p[1] + rm[2][2]*p[2];

  p[0] = X;
  p[1] = Y;
  p[2] = Z;
}

void rotateMatrix(
  double pm[3][3],
  double rm[3][3])
{

  double res[3][3];

  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
       res[i][j] = 0.0;
    }
  }

  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      for (int k = 0; k < 3; k++) {
        res[i][j] += rm[i][k] * pm[k][j];
      }
    }
  }

  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      pm[i][j] = res[i][j];
    }
  }
}

int Render::rotateParts(
  const QString     &addLine,
        RotStepMeta &rotStep,
  const QStringList &parts,
        QString     &ldrName)
{
  QStringList rotatedParts = parts;

  rotateParts(addLine,rotStep,rotatedParts);

  QFile file(ldrName);
  if ( ! file.open(QFile::WriteOnly | QFile::Text)) {
    QMessageBox::warning(NULL,
                         QMessageBox::tr("LPub3D"),
                         QMessageBox::tr("Cannot open file %1 for writing:\n%2")
                         .arg(ldrName) .arg(file.errorString()));
    return -1;
  }

  QTextStream out(&file);

  RotStepData rotStepData = rotStep.value();
  QString rotsComment = QString("0 // ROTSTEP %1 %2 %3 %4")
                                .arg(rotStepData.type)
                                .arg(rotStepData.rots[0])
                                .arg(rotStepData.rots[1])
                                .arg(rotStepData.rots[2]);
                                
  out << rotsComment << endl;

  for (int i = 0; i < rotatedParts.size(); i++) {
    QString line = rotatedParts[i];
    out << line << endl;
  }

  file.close();

  return 0;
}

int Render::rotateParts(
  const QString     &addLine,
        RotStepMeta &rotStep,
        QStringList &parts,
        bool         defaultRot)
{
  double min[3], max[3];

  min[0] = 1e23, max[0] = -1e23,
  min[1] = 1e23, max[1] = -1e23,
  min[2] = 1e23, max[2] = -1e23;

  double defaultViewMatrix[3][3], defaultViewRots[3];

  if (defaultRot) {
    defaultViewRots[0] = 23;
    defaultViewRots[1] = 45;
    defaultViewRots[2] = 0;
  } else {
    defaultViewRots[0] = 0;
    defaultViewRots[1] = 0;
    defaultViewRots[2] = 0;
  }

  matrixMakeRot(defaultViewMatrix,defaultViewRots);

  RotStepData rotStepData = rotStep.value();

  double rm[3][3];

  if (rotStepData.type.size() == 0) {
    matrixCp(rm,defaultViewMatrix);
  } else {
    double rotStepMatrix[3][3];
    matrixMakeRot(rotStepMatrix,rotStepData.rots);
    if (rotStepData.type == "ABS") {
      matrixCp(rm,rotStepMatrix);
    } else {
      matrixMult3(rm,defaultViewMatrix,rotStepMatrix);
    }
  }

  QStringList tokens;

  split(addLine,tokens);

  if (addLine.size() && tokens.size() == 15 && tokens[0] == "1") {
    if (LDrawFile::mirrored(tokens) || ! defaultRot) {

      double alm[3][3];

      for (int token = 5; token < 14; token++) {
        double value = tokens[token].toFloat();
        alm[(token-5) / 3][(token-5) % 3] = value;
      }
      matrixMult(rm,alm);
    }
  }

  // rotate all the parts

  QString processed_parts;

  for (int i = 0; i < parts.size(); i++) {

    QString line = parts[i];
    QStringList tokens;

    split(line,tokens);

    if (tokens.size() < 2) {
      continue;
    }

    double v[4][3];

    if (tokens[0] == "1") {
      v[0][0] = tokens[2].toFloat();
      v[0][1] = tokens[3].toFloat();
      v[0][2] = tokens[4].toFloat();

      rotatePoint(v[0],rm);

      for (int d = 0; d < 3; d++) {
        if (v[0][d] < min[d]) {
          min[d] = v[0][d];
        }
        if (v[0][d] > max[d]) {
          max[d] = v[0][d];
        }
      }
    } else if (tokens[0] == "2") {
      int c = 2;
      for (int j = 0; j < 2; j++) {
        v[j][0] = tokens[c].toDouble();
        v[j][1] = tokens[c+1].toDouble();
        v[j][2] = tokens[c+2].toDouble();
        c += 3;
        rotatePoint(v[j],rm);

        for (int d = 0; d < 3; d++) {
          if (v[j][d] < min[d]) {
            min[d] = v[j][d];
          }
          if (v[j][d] > max[d]) {
            max[d] = v[j][d];
          }
        }
      }
    } else if (tokens[0] == "3") {
      int c = 2;
      for (int j = 0; j < 3; j++) {
        v[j][0] = tokens[c].toDouble();
        v[j][1] = tokens[c+1].toDouble();
        v[j][2] = tokens[c+2].toDouble();
        c += 3;
        rotatePoint(v[j],rm);

        for (int d = 0; d < 3; d++) {
          if (v[j][d] < min[d]) {
            min[d] = v[j][d];
          }
          if (v[j][d] > max[d]) {
            max[d] = v[j][d];
          }
        }
      }
    } else if (tokens[0] == "4") {
      int c = 2;
      for (int j = 0; j < 4; j++) {
        v[j][0] = tokens[c].toDouble();
        v[j][1] = tokens[c+1].toDouble();
        v[j][2] = tokens[c+2].toDouble();
        c += 3;
        rotatePoint(v[j],rm);

        for (int d = 0; d < 3; d++) {
          if (v[j][d] < min[d]) {
            min[d] = v[j][d];
          }
          if (v[j][d] > max[d]) {
            max[d] = v[j][d];
          }
        }
      }
    } else if (tokens[0] == "5") {
      int c = 2;
      for (int j = 0; j < 4; j++) {
        v[j][0] = tokens[c].toDouble();
        v[j][1] = tokens[c+1].toDouble();
        v[j][2] = tokens[c+2].toDouble();
        c += 3;
        rotatePoint(v[j],rm);

        for (int d = 0; d < 3; d++) {
          if (v[j][d] < min[d]) {
            min[d] = v[j][d];
          }
          if (v[j][d] > max[d]) {
            max[d] = v[j][d];
          }
        }
      }
    }
  }

  // center the design at the LDraw origin

  double center[3];
  for (int d = 0; d < 3; d++) {
    center[d] = (min[d] + max[d])/2;
  }

  for (int i = 0; i < parts.size(); i++) {
    QString line = parts[i];
    QStringList tokens;

    split(line,tokens);

    if (tokens.size() < 2) {
      continue;
    }

    double v[4][3];
    double pm[3][3];
    QString t1;

    if (tokens[0] == "1") {
      int c = 2;
      v[0][0] = tokens[c].toFloat();
      v[0][1] = tokens[c+1].toFloat();
      v[0][2] = tokens[c+2].toFloat();
      c += 3;
      for (int y = 0; y < 3; y++) {
        for (int x = 0; x < 3; x++) {
          pm[y][x] = tokens[c++].toDouble();
        }
      }
      rotatePoint(v[0],rm);
      v[0][0] -= center[0];
      v[0][1] -= center[1];
      v[0][2] -= center[2];
      rotateMatrix(pm,rm);

      t1 = QString("1 %1 "
                   "%2 %3 %4 "
                   "%5 %6 %7 "
                   "%8 %9 %10 "
                   "%11 %12 %13 "
                   "%14")

                   .arg(tokens[1])
                   .arg( v[0][0]) .arg( v[0][1]) .arg( v[0][2])
                   .arg(pm[0][0]) .arg(pm[0][1]) .arg(pm[0][2])
                   .arg(pm[1][0]) .arg(pm[1][1]) .arg(pm[1][2])
                   .arg(pm[2][0]) .arg(pm[2][1]) .arg(pm[2][2])
                   .arg(tokens[tokens.size()-1]);

      parts[i] = t1;
    } else if (tokens[0] == "2") {

      int c = 2;
      for (int n = 0; n < 2; n++) {
        for (int d = 0; d < 3; d++) {
          v[n][d] = tokens[c++].toDouble();
        }
        rotatePoint(v[n],rm);
        for (int d = 0; d < 3; d++) {
          v[n][d] -= center[d];
        }
      }

      t1 = QString("2 %1 "
                   "%2 %3 %4 "
                   "%5 %6 %7")
                   .arg(tokens[1])
                   .arg( v[0][0]) .arg( v[0][1]) .arg( v[0][2])
                   .arg( v[1][0]) .arg( v[1][1]) .arg( v[1][2]);
      parts[i] = t1;
    } else if (tokens[0] == "3") {

      int c = 2;
      for (int n = 0; n < 3; n++) {
        for (int d = 0; d < 3; d++) {
          v[n][d] = tokens[c++].toDouble();
        }
        rotatePoint(v[n],rm);
        for (int d = 0; d < 3; d++) {
          v[n][d] -= center[d];
        }
      }

      t1 = QString("3 %1 "
                   "%2 %3 %4  "
                   "%5 %6 %7  "
                   "%8 %9 %10")
                     .arg(tokens[1])
                     .arg( v[0][0]) .arg( v[0][1]) .arg( v[0][2])
                     .arg( v[1][0]) .arg( v[1][1]) .arg( v[1][2])
                     .arg( v[2][0]) .arg( v[2][1]) .arg( v[2][2]);
      parts[i] = t1;
    } else if (tokens[0] == "4") {

      int c = 2;
      for (int n = 0; n < 4; n++) {
        for (int d = 0; d < 3; d++) {
          v[n][d] = tokens[c++].toDouble();
        }
        rotatePoint(v[n],rm);
        for (int d = 0; d < 3; d++) {
          v[n][d] -= center[d];
        }
      }

      t1 = QString("4 %1 "
                   "%2 %3 %4 "
                   "%5 %6 %7 "
                   "%8 %9 %10 "
                   "%11 %12 %13")
                     .arg(tokens[1])
                     .arg( v[0][0]) .arg( v[0][1]) .arg( v[0][2])
                     .arg( v[1][0]) .arg( v[1][1]) .arg( v[1][2])
                     .arg( v[2][0]) .arg( v[2][1]) .arg( v[2][2])
                     .arg( v[3][0]) .arg( v[3][1]) .arg( v[3][2]);
      parts[i] = t1;
    } else if (tokens[0] == "5") {

      int c = 2;
      for (int n = 0; n < 4; n++) {
        for (int d = 0; d < 3; d++) {
          v[n][d] = tokens[c++].toDouble();
        }
        rotatePoint(v[n],rm);
        for (int d = 0; d < 3; d++) {
          v[n][d] -= center[d];
        }
      }

      t1 = QString("5 %1 "
                   "%2 %3 %4 "
                   "%5 %6 %7 "
                   "%8 %9 %10 "
                   "%11 %12 %13")
                     .arg(tokens[1])
                     .arg( v[0][0]) .arg( v[0][1]) .arg( v[0][2])
                     .arg( v[1][0]) .arg( v[1][1]) .arg( v[1][2])
                     .arg( v[2][0]) .arg( v[2][1]) .arg( v[2][2])
                     .arg( v[3][0]) .arg( v[3][1]) .arg( v[3][2]);
      parts[i] = t1;
    }
  }
  return 0;
}
