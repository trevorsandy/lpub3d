
/****************************************************************************
**
** Copyright (C) 2018 - 2023 Trevor SANDY. All rights reserved.
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

#include "pagesizes.h"

struct pageSizeTypes
{
  QString pageTypeSizeID;
  float   pageWidthCm;
  float   pageHeightCm;
  float   pageWidthIn;
  float   pageHeightIn;
} pageSizeTypes[] = {
  { "A0",         84.1000f, 118.9000f,  33.1102f,  46.8110f },
  { "A1",         59.4000f,  84.1000f,  23.3858f,  33.1102f },
  { "A2",         42.0000f,  59.4000f,  16.5354f,  23.3858f },
  { "A3",         29.7000f,  42.0000f,  11.6929f,  16.5354f },
  { "A4",         21.0000f,  29.7000f,   8.2677f,  11.6929f },
  { "A5",         14.8000f,  21.0000f,   5.8268f,   8.2677f },
  { "A6",         10.5000f,  14.8000f,   4.1339f,   5.8268f },
  { "A7",          7.4000f,  10.5000f,   2.9134f,   4.1339f },
  { "A8",          5.2000f,   7.4000f,   2.0472f,   2.9134f },
  { "A9",          3.7000f,   5.2000f,   1.4567f,   2.0472f },
  { "A10",         2.6000f,   3.7000f,   1.0236f,   1.4567f },
  { "ArchA",      22.8600f,  30.4800f,   9.0000f,  12.0000f },
  { "ArchB",      30.4800f,  45.7200f,  12.0000f,  18.0000f },
  { "ArchC",      45.7200f,  60.9600f,  18.0000f,  24.0000f },
  { "ArchD",      60.9600f,  91.4400f,  24.0000f,  36.0000f },
  { "ArchE",      91.4400f, 121.9200f,  36.0000f,  48.0000f },
  { "ArchE1",     76.2000f, 106.6800f,  30.0000f,  42.0000f },
  { "ArchE2",     66.0400f,  96.5200f,  26.0000f,  38.0000f },
  { "ArchE3",     68.5800f,  99.0600f,  27.0000f,  39.0000f },
  { "AnsiA",      21.5900f,  27.9400f,   8.5000f,  11.0000f },
  { "AnsiB",      27.9400f,  43.1800f,  11.0000f,  17.0000f },
  { "AnsiC",      43.1800f,  55.8800f,  17.0000f,  22.0000f },
  { "AnsiD",      55.8800f,  86.3600f,  22.0000f,  34.0000f },
  { "AnsiE",      86.3600f, 111.7600f,  34.0000f,  44.0000f },
  { "B0",        100.0000f, 141.4000f,  39.3701f,  55.6693f },
  { "B1",         70.7000f, 100.0000f,  27.8346f,  39.3701f },
  { "B2",         50.0000f,  70.7000f,  19.6850f,  27.8346f },
  { "B3",         35.3000f,  50.0000f,  13.8976f,  19.6850f },
  { "B4",         25.0000f,  35.3000f,   9.8425f,  13.8976f },
  { "B5",         17.6000f,  25.0000f,   6.9291f,   9.8425f },
  { "B6",         12.5000f,  17.6000f,   4.9213f,   6.9291f },
  { "B7",          8.8000f,  12.5000f,   3.4646f,   4.9213f },
  { "B8",          6.2000f,   8.8000f,   2.4409f,   3.4646f },
  { "B9",          4.4000f,   6.2000f,   1.7323f,   2.4409f },
  { "B10",         3.1000f,   4.4000f,   1.2205f,   1.7323f },
  { "Comm10E",    10.5000f,  24.1000f,   4.1339f,   9.4882f },
  { "DLE",        11.0000f,  22.0000f,   4.3307f,   8.6614f },
  { "Executive",  18.4150f,  26.6700f,   7.2500f,  10.5000f },
  { "Folio",      21.0000f,  33.0000f,   8.2677f,  12.9921f },
  { "Ledger",     43.1800f,  27.9400f,  17.0000f,  11.0000f },
  { "Legal",      21.5900f,  35.5600f,   8.5000f,  14.0000f },
  { "Letter",     21.5900f,  27.9400f,   8.5000f,  11.0000f },
  { "Tabloid",    27.9400f,  43.1800f,  11.0000f,  17.0000f },
  { "Custom",      0.0000f,   0.0000f,   0.0000f,   0.0000f }
};

int PageSizes::numPageTypes(){
  return sizeof(pageSizeTypes)/sizeof(pageSizeTypes[0]);
}

QString PageSizes::pageTypeSizeID(int i) {
  return pageSizeTypes[i].pageTypeSizeID;
}

float PageSizes::pageWidthCm(int i) {
  return pageSizeTypes[i].pageWidthCm;
}

float PageSizes::pageHeightCm(int i) {
  return pageSizeTypes[i].pageHeightCm;
}

float PageSizes::pageWidthIn(int i) {
  return pageSizeTypes[i].pageWidthIn;
}

float PageSizes::pageHeightIn(int i) {
  return pageSizeTypes[i].pageHeightIn;
}


