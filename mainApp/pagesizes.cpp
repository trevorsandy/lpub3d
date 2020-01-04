
/****************************************************************************
**
** Copyright (C) 2018 - 2020 Trevor SANDY. All rights reserved.
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

struct pageSizeTypes {
   QString pageTypeSizeID;
   float   pageWidthCm;
   float   pageHeightCm;
   float   pageWidthIn;
   float   pageHeightIn;
} pageSizeTypes[] = {
{"A0",         84.1000, 118.9000,  33.1102,  46.8110},
{"A1",         59.4000,  84.1000,  23.3858,  33.1102},
{"A2",         42.0000,  59.4000,  16.5354,  23.3858},
{"A3",         29.7000,  42.0000,  11.6929,  16.5354},
{"A4",         21.0000,  29.7000,   8.2677,  11.6929},
{"A5",         14.8000,  21.0000,   5.8268,   8.2677},
{"A6",         10.5000,  14.8000,   4.1339,   5.8268},
{"A7",          7.4000,  10.5000,   2.9134,   4.1339},
{"A8",          5.2000,   7.4000,   2.0472,   2.9134},
{"A9",          3.7000,   5.2000,   1.4567,   2.0472},
{"A10",         2.6000,   3.7000,   1.0236,   1.4567},
{"ArchA",      22.8600,  30.4800,   9.0000,  12.0000},
{"ArchB",      30.4800,  45.7200,  12.0000,  18.0000},
{"ArchC",      45.7200,  60.9600,  18.0000,  24.0000},
{"ArchD",      60.9600,  91.4400,  24.0000,  36.0000},
{"ArchE",      91.4400, 121.9200,  36.0000,  48.0000},
{"ArchE1",     76.2000, 106.6800,  30.0000,  42.0000},
{"ArchE2",     66.0400,  96.5200,  26.0000,  38.0000},
{"ArchE3",     68.5800,  99.0600,  27.0000,  39.0000},
{"AnsiA",      21.5900,  27.9400,   8.5000,  11.0000},
{"AnsiB",      27.9400,  43.1800,  11.0000,  17.0000},
{"AnsiC",      43.1800,  55.8800,  17.0000,  22.0000},
{"AnsiD",      55.8800,  86.3600,  22.0000,  34.0000},
{"AnsiE",      86.3600, 111.7600,  34.0000,  44.0000},
{"B0",        100.0000, 141.4000,  39.3701,  55.6693},
{"B1",         70.7000, 100.0000,  27.8346,  39.3701},
{"B2",         50.0000,  70.7000,  19.6850,  27.8346},
{"B3",         35.3000,  50.0000,  13.8976,  19.6850},
{"B4",         25.0000,  35.3000,   9.8425,  13.8976},
{"B5",         17.6000,  25.0000,   6.9291,   9.8425},
{"B6",         12.5000,  17.6000,   4.9213,   6.9291},
{"B7",          8.8000,  12.5000,   3.4646,   4.9213},
{"B8",          6.2000,   8.8000,   2.4409,   3.4646},
{"B9",          4.4000,   6.2000,   1.7323,   2.4409},
{"B10",         3.1000,   4.4000,   1.2205,   1.7323},
{"Comm10E",    10.5000,  24.1000,   4.1339,   9.4882},
{"DLE",        11.0000,  22.0000,   4.3307,   8.6614},
{"Executive",  18.4150,  26.6700,   7.2500,  10.5000},
{"Folio",      21.0000,  33.0000,   8.2677,  12.9921},
{"Ledger",     43.1800,  27.9400,  17.0000,  11.0000},
{"Legal",      21.5900,  35.5600,   8.5000,  14.0000},
{"Letter",     21.5900,  27.9400,   8.5000,  11.0000},
{"Tabloid",    27.9400,  43.1800,  11.0000,  17.0000},
{"Custom",      0.0000,   0.0000,   0.0000,   0.0000}
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


