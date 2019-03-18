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
#include "annotations.h"

#include <QMessageBox>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include "lpub_preferences.h"
#include "name.h"
#include "version.h"
#include "QsLog.h"

int                         Annotations::returnInt;
QString                     Annotations::returnString;
QList<QString>              Annotations::titleAnnotations;
QHash<QString, QString>     Annotations::freeformAnnotations;
QHash<QString, QStringList> Annotations::annotationStyles;

QHash<QString, QStringList> Annotations::blElements;
QHash<QString, QString>     Annotations::legoElements;
QHash<QString, QString>     Annotations::blColors;
QHash<QString, QString>     Annotations::ld2blColorsXRef;
QHash<QString, QString>     Annotations::ld2blCodesXRef;

void Annotations::loadLD2BLColorsXRef(QByteArray& Buffer){
/*
# File: ld2blcolorsxref.lst
#
# Tab-delmited LDConfig and BrickLink Color code cross reference
#
# The Regular Expression used is: ^([^\t]+)\t+\s*([^\t]+).*$
#
# 1. LDConfig ID:       LDraw Color ID             (Required)
# 2. Color ID:          BrickLink Color ID         (Required)
#
*/
    const char LEGOLD2BLColorsXRef[] = {
        "15\t  1\n"
        "19\t  2\n"
        "14\t  3\n"
        "25\t  4\n"
        "4\t   5\n"
        "2\t   6\n"
        "1\t   7\n"
        "6\t   8\n"
        "7\t   9\n"
        "8\t   10\n"
        "0\t   11\n"
        "47\t  12\n"
        "40\t  13\n"
        "33\t  14\n"
        "43\t  15\n"
        "42\t  16\n"
        "36\t  17\n"
        "38\t  18\n"
        "46\t  19\n"
        "34\t  20\n"
        "334\t 21\n"
        "383\t 22\n"
        "13\t  23\n"
        "22\t  24\n"
        "12\t  25\n"
        "100\t 26\n"
        "92\t  28\n"
        "366\t 29\n"
        "462\t 31\n"
        "18\t  33\n"
        "27\t  34\n"
        "120\t 35\n"
        "10\t  36\n"
        "74\t  37\n"
        "17\t  38\n"
        "3\t   39\n"
        "11\t  40\n"
        "118\t 41\n"
        "73\t  42\n"
        "110\t 43\n"
        "20\t  44\n"
        "21\t  46\n"
        "5\t   47\n"
        "378\t 48\n"
        "503\t 49\n"
        "37\t  50\n"
        "52\t  51\n"
        "61\t  52\n"
        "373\t 54\n"
        "379\t 55\n"
        "60\t  57\n"
        "335\t 58\n"
        "320\t 59\n"
        "79\t  60\n"
        "142\t 61\n"
        "135\t 61\n"
        "9\t   62\n"
        "272\t 63\n"
        "62\t  64\n"
        "82\t  65\n"
        "135\t 66\n"
        "80\t  67\n"
        "484\t 68\n"
        "28\t  69\n"
        "81\t  70\n"
        "26\t  71\n"
        "313\t 72\n"
        "112\t 73\n"
        "219\t 73\n"
        "41\t  74\n"
        "115\t 76\n"
        "148\t 77\n"
        "87\t  77\n"
        "137\t 78\n"
        "288\t 80\n"
        "178\t 81\n"
        "63\t  82\n"
        "183\t 83\n"
        "134\t 84\n"
        "72\t  85\n"
        "71\t  86\n"
        "232\t 87\n"
        "70\t  88\n"
        "85\t  89\n"
        "78\t  90\n"
        "86\t  91\n"
        "69\t  93\n"
        "351\t 94\n"
        "135\t 95\n"
        "68\t  96\n"
        "89\t  97\n"
        "57\t  98\n"
        "151\t 99\n"
        "114\t 100\n"
        "117\t 101\n"
        "129\t 102\n"
        "226\t 103\n"
        "29\t  104\n"
        "212\t 105\n"
        "450\t 106\n"
        "45\t  107\n"
        "35\t  108\n"
        "285\t 108\n"
        "35\t  108\n"
        "23\t  109\n"
        "191\t 110\n"
        "79\t  111\n"
        "39\t  113\n"
        "297\t 115\n"
        "75\t  116\n"
        "79\t  117\n"
        "294\t 118\n"
        "150\t 119\n"
        "308\t 120\n"
        "54\t  121\n"
        "64\t  122\n"
        "44\t  144\n"
        "84\t  150\n"
        "133\t 151\n"
        "323\t 152\n"
        "321\t 153\n"
        "31\t  154\n"
        "330\t 155\n"
        "322\t 156\n"
        "30\t  157\n"
        "326\t 158\n"
        "329\t 159\n"
        "302\t 162\n"
        "339\t 163\n"
    };

    const char LD2BLColorsXRef[] = {
        "no colors cross-references defined\n"
    };

    if (Preferences::validLDrawLibrary == LEGO_LIBRARY)
        Buffer.append(LEGOLD2BLColorsXRef, sizeof(LEGOLD2BLColorsXRef));
    else
        Buffer.append(LD2BLColorsXRef, sizeof(LD2BLColorsXRef));
}

void Annotations::loadLD2BLCodesXRef(QByteArray& Buffer){
/*
# File: ld2blcodesxref.lst
#
# Tab-delmited LDraw Design ID and BrickLink Item Number cross reference
#
# The Regular Expression used is: ^([^\\t]+)\\t+\\s*([^\\t]+).*$
#
# 1. LDraw ID:             LDraw Design ID              (Required)
# 2. Item No:              BrickLink Item Number        (Required)
#
*/
    const char LEGOLD2BLCodesXRef[] = {
        "3245a\t     3245b\n"
        "32126\t     44\n"
        "3957a\t     3957\n"
        "76116\t     98313\n"
        "3819\t      981\n"
        "3818\t      982\n"
    };

    const char LD2BLCodesXRef[] = {
        "no code cross-references defined\n"
    };

    if (Preferences::validLDrawLibrary == LEGO_LIBRARY)
        Buffer.append(LEGOLD2BLCodesXRef, sizeof(LEGOLD2BLCodesXRef));
    else
        Buffer.append(LD2BLCodesXRef, sizeof(LD2BLCodesXRef));
}

void Annotations::loadBLColors(QByteArray& Buffer){
/*
# File: colors.txt
#
# Tab-delmited BrickLink Color codes
#
# The Regular Expression used is: ^([^\t]+)\t+\s*([^\t]+).*$
#
# 1. Color ID:            BrickLink Color ID             (Required)
# 2. Color Name:          BrickLink Color Name           (Required)
# 3+ Other Fields:        Other fields are not used
#
*/
    const char LEGOBLColors[] = {
        "0\t    (Not Applicable)\n"
        "41\t   Aqua\n"
        "11\t   Black\n"
        "7\t    Blue\n"
        "97\t   Blue-Violet\n"
        "36\t   Bright Green\n"
        "105\t  Bright Light Blue\n"
        "110\t  Bright Light Orange\n"
        "103\t  Bright Light Yellow\n"
        "104\t  Bright Pink\n"
        "8\t    Brown\n"
        "153\t  Dark Azure\n"
        "63\t   Dark Blue\n"
        "109\t  Dark Blue-Violet\n"
        "85\t   Dark Bluish Gray\n"
        "120\t  Dark Brown\n"
        "91\t   Dark Flesh\n"
        "10\t   Dark Gray\n"
        "80\t   Dark Green\n"
        "68\t   Dark Orange\n"
        "47\t   Dark Pink\n"
        "89\t   Dark Purple\n"
        "59\t   Dark Red\n"
        "69\t   Dark Tan\n"
        "39\t   Dark Turquoise\n"
        "161\t  Dark Yellow\n"
        "29\t   Earth Orange\n"
        "106\t  Fabuland Brown\n"
        "160\t  Fabuland Orange\n"
        "28\t   Flesh\n"
        "6\t    Green\n"
        "154\t  Lavender\n"
        "152\t  Light Aqua\n"
        "62\t   Light Blue\n"
        "86\t   Light Bluish Gray\n"
        "90\t   Light Flesh\n"
        "9\t    Light Gray\n"
        "38\t   Light Green\n"
        "35\t   Light Lime\n"
        "32\t   Light Orange\n"
        "56\t   Light Pink\n"
        "93\t   Light Purple\n"
        "26\t   Light Salmon\n"
        "40\t   Light Turquoise\n"
        "44\t   Light Violet\n"
        "33\t   Light Yellow\n"
        "34\t   Lime\n"
        "72\t   Maersk Blue\n"
        "71\t   Magenta\n"
        "156\t  Medium Azure\n"
        "42\t   Medium Blue\n"
        "150\t  Medium Dark Flesh\n"
        "94\t   Medium Dark Pink\n"
        "37\t   Medium Green\n"
        "157\t  Medium Lavender\n"
        "76\t   Medium Lime\n"
        "31\t   Medium Orange\n"
        "73\t   Medium Violet\n"
        "166\t  Neon Green\n"
        "165\t  Neon Orange\n"
        "155\t  Olive Green\n"
        "4\t    Orange\n"
        "23\t   Pink\n"
        "24\t   Purple\n"
        "5\t    Red\n"
        "88\t   Reddish Brown\n"
        "27\t   Rust\n"
        "25\t   Salmon\n"
        "55\t   Sand Blue\n"
        "48\t   Sand Green\n"
        "54\t   Sand Purple\n"
        "58\t   Sand Red\n"
        "87\t   Sky Blue\n"
        "2\t    Tan\n"
        "99\t   Very Light Bluish Gray\n"
        "49\t   Very Light Gray\n"
        "96\t   Very Light Orange\n"
        "43\t   Violet\n"
        "1\t    White\n"
        "3\t    Yellow\n"
        "158\t  Yellowish Green\n"
        "13\t   Trans-Black\n"
        "108\t  Trans-Bright Green\n"
        "12\t   Trans-Clear\n"
        "14\t   Trans-Dark Blue\n"
        "50\t   Trans-Dark Pink\n"
        "20\t   Trans-Green\n"
        "15\t   Trans-Light Blue\n"
        "164\t  Trans-Light Orange\n"
        "114\t  Trans-Light Purple\n"
        "74\t   Trans-Medium Blue\n"
        "16\t   Trans-Neon Green\n"
        "18\t   Trans-Neon Orange\n"
        "121\t  Trans-Neon Yellow\n"
        "98\t   Trans-Orange\n"
        "107\t  Trans-Pink\n"
        "51\t   Trans-Purple\n"
        "17\t   Trans-Red\n"
        "113\t  Trans-Very Lt Blue\n"
        "19\t   Trans-Yellow\n"
        "57\t   Chrome Antique Brass\n"
        "122\t  Chrome Black\n"
        "52\t   Chrome Blue\n"
        "21\t   Chrome Gold\n"
        "64\t   Chrome Green\n"
        "82\t   Chrome Pink\n"
        "22\t   Chrome Silver\n"
        "84\t   Copper\n"
        "81\t   Flat Dark Gold\n"
        "95\t   Flat Silver\n"
        "78\t   Metal Blue\n"
        "77\t   Pearl Dark Gray\n"
        "115\t  Pearl Gold\n"
        "61\t   Pearl Light Gold\n"
        "66\t   Pearl Light Gray\n"
        "119\t  Pearl Very Light Gray\n"
        "83\t   Pearl White\n"
        "65\t   Metallic Gold\n"
        "70\t   Metallic Green\n"
        "67\t   Metallic Silver\n"
        "46\t   Glow In Dark Opaque\n"
        "118\t  Glow In Dark Trans\n"
        "159\t  Glow In Dark White\n"
        "60\t   Milky White\n"
        "101\t  Glitter Trans-Clear\n"
        "100\t  Glitter Trans-Dark Pink\n"
        "162\t  Glitter Trans-Light Blue\n"
        "163\t  Glitter Trans-Neon Green\n"
        "102\t  Glitter Trans-Purple\n"
        "116\t  Speckle Black-Copper\n"
        "151\t  Speckle Black-Gold\n"
        "111\t  Speckle Black-Silver\n"
        "117\t  Speckle DBGray-Silver\n"
        "142\t  Mx Aqua Green\n"
        "128\t  Mx Black\n"
        "132\t  Mx Brown\n"
        "133\t  Mx Buff\n"
        "126\t  Mx Charcoal Gray\n"
        "149\t  Mx Clear\n"
        "214\t  Mx Foil Dark Blue\n"
        "210\t  Mx Foil Dark Gray\n"
        "212\t  Mx Foil Dark Green\n"
        "215\t  Mx Foil Light Blue\n"
        "211\t  Mx Foil Light Gray\n"
        "213\t  Mx Foil Light Green\n"
        "219\t  Mx Foil Orange\n"
        "217\t  Mx Foil Red\n"
        "216\t  Mx Foil Violet\n"
        "218\t  Mx Foil Yellow\n"
        "139\t  Mx Lemon\n"
        "124\t  Mx Light Bluish Gray\n"
        "125\t  Mx Light Gray\n"
        "136\t  Mx Light Orange\n"
        "137\t  Mx Light Yellow\n"
        "144\t  Mx Medium Blue\n"
        "138\t  Mx Ochre Yellow\n"
        "140\t  Mx Olive Green\n"
        "135\t  Mx Orange\n"
        "145\t  Mx Pastel Blue\n"
        "141\t  Mx Pastel Green\n"
        "148\t  Mx Pink\n"
        "130\t  Mx Pink Red\n"
        "129\t  Mx Red\n"
        "146\t  Mx Teal Blue\n"
        "134\t  Mx Terracotta\n"
        "143\t  Mx Tile Blue\n"
        "131\t  Mx Tile Brown\n"
        "127\t  Mx Tile Gray\n"
        "147\t  Mx Violet\n"
        "123\t  Mx White\n"
        "172\t  BA Black\n"
        "207\t  BA Black Rubber\n"
        "183\t  BA Blue\n"
        "199\t  BA Blue Chrome\n"
        "197\t  BA Brass\n"
        "198\t  BA Bronze\n"
        "177\t  BA Brown\n"
        "209\t  BA Chrome\n"
        "168\t  BA Cobalt\n"
        "190\t  BA Dark Blue\n"
        "178\t  BA Dark Brown\n"
        "175\t  BA Dark Gray\n"
        "204\t  BA Dark Gray Rubber\n"
        "179\t  BA Dark Tan\n"
        "208\t  BA Glow In Dark\n"
        "203\t  BA Gray Rubber\n"
        "182\t  BA Green\n"
        "170\t  BA Gunmetal\n"
        "205\t  BA Gunmetal Rubber\n"
        "174\t  BA Light Gray\n"
        "189\t  BA OD Green\n"
        "200\t  BA OD Metallic\n"
        "181\t  BA Olive\n"
        "192\t  BA Pink\n"
        "191\t  BA Purple\n"
        "176\t  BA Red\n"
        "201\t  BA Red Chrome\n"
        "184\t  BA Sand Blue\n"
        "195\t  BA Silver\n"
        "206\t  BA Silver Rubber\n"
        "180\t  BA Tan\n"
        "196\t  BA Titanium\n"
        "186\t  BA Trans Black\n"
        "188\t  BA Trans Blue\n"
        "185\t  BA Trans Clear\n"
        "171\t  BA Trans Green\n"
        "187\t  BA Trans Orange\n"
        "169\t  BA Trans Red\n"
        "194\t  BA Trans Red Sparkle\n"
        "193\t  BA Trans Smoke\n"
        "167\t  BA UN Blue\n"
        "173\t  BA White\n"
        "202\t  BA White Rubber\n"
    };

    const char BLColors[] = {
        "no Bricklnk colors defined\n"
    };

    if (Preferences::validLDrawLibrary == LEGO_LIBRARY)
        Buffer.append(LEGOBLColors, sizeof(LEGOBLColors));
    else
        Buffer.append(BLColors, sizeof(BLColors));
}

void Annotations::loadDefaultAnnotationStyles(QByteArray& Buffer){

/*
# File: styledAnnotations.lst
#
# The Regular Expression used is: ^(\b[^=]+\b)=([1|2])\s*([1-6])?\s*([^\s]+).*$
#
# 1. Part ID:             LDraw Part Name              (Required)
# 2. Annotation Style:    1 for Square or 2 for Circle (Required)
# 3. Annotation Category: Part category                (Required)
# 4. Annotation:          Annotation text (uses part title annotation if no text defined)
# 5. Part Description:    Description for reference only. Not loaded
# Fields 4-5 are optional
#
# LEGO Standard:
# ------------------------------------
# |No |Category      |Annotation Style|
# |---|-------------------------------|
# | 1 |round(1)      |axle(1)         |
# | 2 |square(2)     |beam(2)         |
# | 3 |square(2)     |cable(3)        |
# | 4 |square(2)     |connector(4)    |
# | 5 |square(2)     |hose(5)         |
# | 6 |round(1)      |panel(6)        |
# ------------------------------------
#
# 32034.dat=1  4  2    Technic Angle Connector #2 (180 degree)
#
*/
    const char LEGODefaultAnnotationStyles[] = {
        "3704.dat=1    1   2    Technic Axle  2\n"
        "32062.dat=1   1   2    Technic Axle  2 Notched\n"
        "4519.dat=1    1   3    Technic Axle  3\n"
        "24316.dat=1   1   3    Technic Axle  3 with Stop\n"
        "6587.dat=1    1   3    Technic Axle  3 with Stud\n"
        "27940.dat=1   1   3    Technic Axle  3L with Middle Perpendicular Pin Hole\n"
        "3705.dat=1    1   4    Technic Axle  4\n"
        "3705c01.dat=1 1   4    Technic Axle  4 Threaded\n"
        "99008.dat=1   1   4    Technic Axle  4 with Middle Cylindrical Stop\n"
        "87083.dat=1   1   4    Technic Axle  4 with Stop\n"
        "32073.dat=1   1   5    Technic Axle  5\n"
        "15462.dat=1   1   5    Technic Axle  5 with Stop\n"
        "32209.dat=1   1   5.5  Technic Axle  5.5 with Stop\n"
        "59426.dat=1   1   5.5  Technic Axle  5.5 with Stop Type 2\n"
        "3706.dat=1    1   6    Technic Axle  6\n"
        "44294.dat=1   1   7    Technic Axle  7\n"
        "3707.dat=1    1   8    Technic Axle  8\n"
        "55013.dat=1   1   8    Technic Axle  8 with Stop\n"
        "60485.dat=1   1   9    Technic Axle  9\n"
        "3737.dat=1    1  10    Technic Axle 10\n"
        "3737c01.dat=1 1  10    Technic Axle 10 Threaded\n"
        "23948.dat=1   1  11    Technic Axle 11\n"
        "3708.dat=1    1  12    Technic Axle 12\n"
        "50451.dat=1   1  16    Technic Axle 16\n"
        "50450.dat=1   1  32    Technic Axle 32\n"

        "32580.dat=1   1   7    Technic Axle Flexible  7\n"
        "32199.dat=1   1  11    Technic Axle Flexible 11\n"
        "32200.dat=1   1  12    Technic Axle Flexible 12\n"
        "32201.dat=1   1  14    Technic Axle Flexible 14\n"
        "32202.dat=1   1  16    Technic Axle Flexible 16\n"
        "32235.dat=1   1  19    Technic Axle Flexible 19\n"

        "18654.dat=2   2   1    Technic Beam  1\n"
        "43857.dat=2   2   2    Technic Beam  2\n"
        "32523.dat=2   2   3    Technic Beam  3\n"
        "32316.dat=2   2   5    Technic Beam  5\n"
        "32524.dat=2   2   7    Technic Beam  7\n"
        "40490.dat=2   2   9    Technic Beam  9\n"
        "32525.dat=2   2  11    Technic Beam 11\n"
        "41239.dat=2   2  13    Technic Beam 13\n"
        "32278.dat=2   2  15    Technic Beam 15\n"

        "11145.dat=2   3  25    Electric Mindstorms EV3 Cable 25 cm\n"
        "11146.dat=2   3  35    Electric Mindstorms EV3 Cable 35 cm\n"
        "11147.dat=2   3  50    Electric Mindstorms EV3 Cable 50 cm\n"

        "55804.dat=2   3  20    Electric Mindstorms NXT Cable 20 cm\n"
        "55805.dat=2   3  35    Electric Mindstorms NXT Cable 35 cm\n"
        "55806.dat=2   3  50    Electric Mindstorms NXT Cable 50 cm\n"

        "32013.dat=2   4   1    Technic Angle Connector #1\n"
        "32034.dat=2   4   2    Technic Angle Connector #2 (180 degree)\n"
        "32016.dat=2   4   3    Technic Angle Connector #3 (157.5 degree)\n"
        "32192.dat=2   4   4    Technic Angle Connector #4 (135 degree)\n"
        "32015.dat=2   4   5    Technic Angle Connector #5 (112.5 degree)\n"
        "32014.dat=2   4   6    Technic Angle Connector #6 (90 degree)\n"

        "76263.dat=2   5   3    Technic Flex-System Hose  3L (60LDU)\n"
        "76250.dat=2   5   4    Technic Flex-System Hose  4L (80LDU)\n"
        "76307.dat=2   5   5    Technic Flex-System Hose  5L (100LDU)\n"
        "76279.dat=2   5   6    Technic Flex-System Hose  6L (120LDU)\n"
        "76289.dat=2   5   7    Technic Flex-System Hose  7L (140LDU)\n"
        "76260.dat=2   5   8    Technic Flex-System Hose  8L (160LDU)\n"
        "76324.dat=2   5   9    Technic Flex-System Hose  9L (180LDU)\n"
        "76348.dat=2   5  10    Technic Flex-System Hose 10L (200LDU)\n"
        "71505.dat=2   5  11    Technic Flex-System Hose 11L (220LDU)\n"
        "71175.dat=2   5  12    Technic Flex-System Hose 12L (240LDU)\n"
        "71551.dat=2   5  13    Technic Flex-System Hose 13L (260LDU)\n"
        "71177.dat=2   5  14    Technic Flex-System Hose 14L (280LDU)\n"
        "71194.dat=2   5  15    Technic Flex-System Hose 15L (300LDU)\n"
        "71192.dat=2   5  16    Technic Flex-System Hose 16L (320LDU)\n"
        "76270.dat=2   5  17    Technic Flex-System Hose 17L (340LDU)\n"
        "71582.dat=2   5  18    Technic Flex-System Hose 18L (360LDU)\n"
        "22463.dat=2   5  19    Technic Flex-System Hose 19L (380LDU)\n"
        "76276.dat=2   5  20    Technic Flex-System Hose 20L (400LDU)\n"
        "70978.dat=2   5  21    Technic Flex-System Hose 21L (420LDU)\n"
        "76252.dat=2   5  22    Technic Flex-System Hose 22L (440LDU)\n"
        "76254.dat=2   5  23    Technic Flex-System Hose 23L (460LDU)\n"
        "76277.dat=2   5  24    Technic Flex-System Hose 24L (480LDU)\n"
        "53475.dat=2   5  26    Technic Flex-System Hose 26L (520LDU)\n"
        "76280.dat=2   5  28    Technic Flex-System Hose 28L (560LDU)\n"
        "76389.dat=2   5  29    Technic Flex-System Hose 29L (580LDU)\n"
        "76282.dat=2   5  30    Technic Flex-System Hose 30L (600LDU)\n"
        "76283.dat=2   5  31    Technic Flex-System Hose 31L (620LDU)\n"
        "57274.dat=2   5  32    Technic Flex-System Hose 32L (640LDU)\n"
        "42688.dat=2   5  33    Technic Flex-System Hose 33L (660LDU)\n"
        "22461.dat=2   5  34    Technic Flex-System Hose 34L (680LDU)\n"
        "46305.dat=2   5  40    Technic Flex-System Hose 40L (800LDU)\n"
        "76281.dat=2   5  45    Technic Flex-System Hose 45L (900LDU)\n"
        "22296.dat=2   5  53    Technic Flex-System Hose 53L (1060LDU)\n"

        "72504.dat=2   5   2    Technic Ribbed Hose  2L\n"
        "72706.dat=2   5   3    Technic Ribbed Hose  3L\n"
        "71952.dat=2   5   4    Technic Ribbed Hose  4L\n"
        "72853.dat=2   5   5    Technic Ribbed Hose  5L\n"
        "71944.dat=2   5   6    Technic Ribbed Hose  6L\n"
        "57719.dat=2   5   7    Technic Ribbed Hose  7L\n"
        "71951.dat=2   5   8    Technic Ribbed Hose  8L\n"
        "71917.dat=2   5   9    Technic Ribbed Hose  9L\n"
        "71949.dat=2   5  10    Technic Ribbed Hose 10L\n"
        "71986.dat=2   5  11    Technic Ribbed Hose 11L\n"
        "71819.dat=2   5  12    Technic Ribbed Hose 12L\n"
        "71923.dat=2   5  14    Technic Ribbed Hose 14L\n"
        "71946.dat=2   5  15    Technic Ribbed Hose 15L\n"
        "71947.dat=2   5  16    Technic Ribbed Hose 16L\n"
        "22900.dat=2   5  17    Technic Ribbed Hose 17L\n"
        "72039.dat=2   5  18    Technic Ribbed Hose 18L\n"
        "43675.dat=2   5  19    Technic Ribbed Hose 19L\n"
        "23397.dat=2   5  24    Technic Ribbed Hose 24L\n"

        "32190.dat=1   6   1    Technic Panel Fairing #1\n"
        "32191.dat=1   6   2    Technic Panel Fairing #2\n"
        "44350.dat=1   6  20    Technic Panel Fairing #20\n"
        "44351.dat=1   6  21    Technic Panel Fairing #21\n"
        "44352.dat=1   6  22    Technic Panel Fairing #22\n"
        "44353.dat=1   6  23    Technic Panel Fairing #23\n"
        "47712.dat=1   6  24    Technic Panel Fairing #24\n"
        "47713.dat=1   6  25    Technic Panel Fairing #25\n"
        "32188.dat=1   6   3    Technic Panel Fairing #3\n"
        "32189.dat=1   6   4    Technic Panel Fairing #4\n"
        "32527.dat=1   6   5    Technic Panel Fairing #5\n"
        "32528.dat=1   6   6    Technic Panel Fairing #6\n"
        "32534.dat=1   6   7    Technic Panel Fairing #7\n"
        "32535.dat=1   6   8    Technic Panel Fairing #8\n"

        "87080.dat=1   6   1    Technic Panel Fairing Smooth #1 (Short)\n"
        "64394.dat=1   6  13    Technic Panel Fairing Smooth #13 (Wide Medium)\n"
        "64680.dat=1   6  14    Technic Panel Fairing Smooth #14 (Wide Medium)\n"
        "64392.dat=1   6  17    Technic Panel Fairing Smooth #17 (Wide Long)\n"
        "64682.dat=1   6  18    Technic Panel Fairing Smooth #18 (Wide Long)\n"
        "87086.dat=1   6   2    Technic Panel Fairing Smooth #2 (Short)\n"
        "11946.dat=1   6  21    Technic Panel Fairing Smooth #21 (Thin Short)\n"
        "11947.dat=1   6  22    Technic Panel Fairing Smooth #22 (Thin Short)\n"
        "64683.dat=1   6   3    Technic Panel Fairing Smooth #3 (Medium)\n"
        "64391.dat=1   6   4    Technic Panel Fairing Smooth #4 (Medium)\n"
        "64681.dat=1   6   5    Technic Panel Fairing Smooth #5 (Long)\n"
        "64393.dat=1   6   6    Technic Panel Fairing Smooth #6 (Long)\n"

        "4109810.dat=1 1   2    _Technic Axle  2 Notched Black\n"
        "4211815.dat=1 1   3    _Technic Axle  3 Light_Bluish_Gray\n"
        "370526.dat=1  1   4    _Technic Axle  4 Black\n"
        "73839.dat=1   1   4    _Technic Axle  4 Threaded Black\n"
        "370626.dat=1  1   6    _Technic Axle  6 Black\n"
        "370726.dat=1  1   8    _Technic Axle  8 Black\n"
        "73485.dat=1   1  10    _Technic Axle 10 Threaded Black\n"
        "370826.dat=1  1  12    _Technic Axle 12 Black\n"
        "4263624.dat=1 1  5.5   _Technic Axle 5.5 With Stop Dark_Bluish_Gray\n"
        "55709.dat=1   1  11    =Technic Axle Flexible 11\n"

        "17141.dat=2   2   3    =Technic Beam  3\n"
        "16615.dat=2   2   7    =Technic Beam  7\n"
        "64289.dat=2   2   9    =Technic Beam  9\n"
        "64290.dat=2   2  11    =Technic Beam 11\n"
        "64871.dat=2   2  15    =Technic Beam 15\n"

        "4211550.dat=2 4   1    _Technic Angle Connector #1 Light_Bluish_Gray\n"
        "4506697.dat=2 4   5    _Technic Angle Connector #5 (112.5 degree) White\n"
        "4107767.dat=2 4   6    _Technic Angle Connector #6 (90 degree) Black\n"

        "22749.dat=1   6   1    =Technic Panel Fairing #1\n"
        "22750.dat=1   6   2    =Technic Panel Fairing #2\n"
        "22972.dat=1   6   3    =Technic Panel Fairing #3\n"
        "22973.dat=1   6   4    =Technic Panel Fairing #4\n"
    };

    const char TENTEDefaultAnnotationStyles[] = {
        "no default annotation styles defined\n"
    };

    const char VEXIQDefaultAnnotationStyles[] = {
        "no default annotation styles defined1\n"
    };

    if (Preferences::validLDrawLibrary == LEGO_LIBRARY)
        Buffer.append(LEGODefaultAnnotationStyles, sizeof(LEGODefaultAnnotationStyles));
    else
    if (Preferences::validLDrawLibrary == TENTE_LIBRARY)
        Buffer.append(TENTEDefaultAnnotationStyles, sizeof(TENTEDefaultAnnotationStyles));
    else
    if (Preferences::validLDrawLibrary == VEXIQ_LIBRARY)
        Buffer.append(VEXIQDefaultAnnotationStyles, sizeof(VEXIQDefaultAnnotationStyles));
}

Annotations::Annotations()
{
    if (titleAnnotations.size() == 0) {
        QString annotations = Preferences::titleAnnotationsFile;
        QFile file(annotations);
        if ( ! file.open(QFile::ReadOnly | QFile::Text)) {
            QString message = QString("Failed to open Title Annotations file: %1:\n%2")
                                      .arg(annotations)
                                      .arg(file.errorString());
            if (Preferences::modeGUI){
                QMessageBox::warning(nullptr,QMessageBox::tr("LPub3D"),message);
            } else {
                logError() << message;
            }
            return;
        }
        QTextStream in(&file);

        // Load RegExp from file;
        QRegExp rx("^(\\b.*[^\\s]\\b:)\\s+([\\(|\\^].*)$");
        QRegExp rxin("^#\\sThe\\sRegular\\sExpression\\sused\\sis\\:[\\s](\\^.*)$");
        while ( ! in.atEnd()) {
            QString sLine = in.readLine(0);
            if (sLine.contains(rxin)) {
                rx.setPattern(rxin.cap(1));
                //logDebug() << "TitleAnnotations RegExp Pattern: " << rxin.cap(1);
                break;
            }
        }

        in.seek(0);

        // Load input values
        while ( ! in.atEnd()) {
            QString sLine = in.readLine(0);
            if (sLine.contains(rx)) {
                QString annotation = rx.cap(2);
                titleAnnotations << annotation;
            }
        }
    }

    if (freeformAnnotations.size() == 0) {
        QString annotations = Preferences::freeformAnnotationsFile;
        QFile file(annotations);
        if ( ! file.open(QFile::ReadOnly | QFile::Text)) {
            QString message = QString("Failed to open Freeform Annotations file: %1:\n%2")
                                      .arg(annotations)
                                      .arg(file.errorString());
            if (Preferences::modeGUI){
                QMessageBox::warning(nullptr,QMessageBox::tr("LPub3D"),message);
            } else {
                logError() << message;
            }
            return;
        }
        QTextStream in(&file);

        // Load RegExp from file;
        QRegExp rx("^(\\b.*[^\\s]\\b)(?:\\s)\\s+(.*)$");
        QRegExp rxin("^#\\sThe\\sRegular\\sExpression\\sused\\sis\\:[\\s](\\^.*)$");
        while ( ! in.atEnd()) {
            QString sLine = in.readLine(0);
            if (sLine.contains(rxin)) {
                rx.setPattern(rxin.cap(1));
                //logDebug() << "FreeFormAnnotations RegExp Pattern: " << rxin.cap(1);
                break;
            }
        }

        in.seek(0);

        // Load input values
        while ( ! in.atEnd()) {
            QString sLine = in.readLine(0);
            if (sLine.contains(rx)) {
                QString parttype = rx.cap(1);
                QString annotation = rx.cap(2);
                freeformAnnotations[parttype.toLower()] = annotation;
            }
        }
    }

    if (annotationStyles.size() == 0) {
        QString styleFile = Preferences::annotationStyleFile;
        QRegExp rx("^(\\b[^=]+\\b)=([1|2])\\s+([1-6])?\\s*([^\\s]+).*$");
        if (!styleFile.isEmpty()) {
            QFile file(styleFile);
            if ( ! file.open(QFile::ReadOnly | QFile::Text)) {
                QString message = QString("Failed to open Annotation style file: %1:\n%2")
                                          .arg(styleFile)
                                          .arg(file.errorString());
                if (Preferences::modeGUI){
                    QMessageBox::warning(nullptr,QMessageBox::tr("LPub3D"),message);
                } else {
                    logError() << message;
                }
                return;
            }
            QTextStream in(&file);

            // Load RegExp from file;
            QRegExp rxin("^#\\sThe\\sRegular\\sExpression\\sused\\sis\\:[\\s](\\^.*)$");
            while ( ! in.atEnd()) {
                QString sLine = in.readLine(0);
                if (sLine.contains(rxin)) {
                    rx.setPattern(rxin.cap(1));
//                    logDebug() << "AnnotationStyle RegExp Pattern: " << rxin.cap(1);
                    break;
                }
            }

            in.seek(0);

            // Load input values
            while ( ! in.atEnd()) {
                QString sLine = in.readLine(0);
                if (sLine.contains(rx)) {
                    QString parttype = rx.cap(1);
                    QString style = rx.cap(2).isEmpty() ? QString() : rx.cap(2);
                    QString category = rx.cap(3).isEmpty() ? QString() : rx.cap(3);
                    QString annotation = rx.cap(4).isEmpty() ? QString() : rx.cap(4);
                    annotationStyles[parttype.toLower()] << style << category << annotation;
                }
            }
        } else {
            annotationStyles.clear();
            QByteArray Buffer;
            loadDefaultAnnotationStyles(Buffer);
            QTextStream instream(Buffer);
            for (QString sLine = instream.readLine(); !sLine.isNull(); sLine = instream.readLine())
            {
                int Equals = sLine.indexOf('=');
                if (Equals == -1)
                    continue;
                if (sLine.contains(rx)) {
                    QString parttype = rx.cap(1);
                    QString style = rx.cap(2).isEmpty() ? QString() : rx.cap(2);
                    QString category = rx.cap(3).isEmpty() ? QString() : rx.cap(3);
                    QString annotation = rx.cap(4).isEmpty() ? QString() : rx.cap(4);
                    annotationStyles[parttype.toLower()] << style << category << annotation;
                }
            }
        }
    }

    if (blColors.size() == 0) {
        QString blColorsFile = Preferences::blColorsFile;
        QRegExp rx("^([^\\t]+)\\t+\\s*([^\\t]+).*$");
        if (!blColorsFile.isEmpty()) {
            QFile file(blColorsFile);
            if ( ! file.open(QFile::ReadOnly | QFile::Text)) {
                QString message = QString("Failed to open BrickLink colors.txt file: %1:\n%2")
                                          .arg(blColorsFile)
                                          .arg(file.errorString());
                if (Preferences::modeGUI){
                    QMessageBox::warning(nullptr,QMessageBox::tr("LPub3D"),message);
                } else {
                    logError() << message;
                }
                return;
            }
            QTextStream in(&file);

            // Load RegExp from file;
            QRegExp rxin("^#\\sThe\\sRegular\\sExpression\\sused\\sis\\:[\\s](\\^.*)$");
            while ( ! in.atEnd()) {
                QString sLine = in.readLine(0);
                if (sLine.contains(rxin)) {
                    rx.setPattern(rxin.cap(1));
//                    logDebug() << "Bricklink Colors RegExp Pattern: " << rxin.cap(1);
                    break;
                }
            }

           in.seek(0);

            // Load input values
            while ( ! in.atEnd()) {
                QString sLine = in.readLine(0);
                if (sLine.contains(rx)) {
                    QString colorid = rx.cap(1);
                    QString colorname = rx.cap(2).trimmed();
                    blColors[colorname.toLower()] = colorid;
                }
            }
        } else {
            blColors.clear();
            QByteArray Buffer;
            loadBLColors(Buffer);
            QTextStream instream(Buffer);
            for (QString sLine = instream.readLine(); !sLine.isNull(); sLine = instream.readLine())
            {
                QChar comment = sLine.at(0);
                if (comment == '#' || comment == ' ')
                    continue;
                if (sLine.contains(rx)) {
                    QString colorid = rx.cap(1);
                    QString colorname = rx.cap(2).trimmed();
                    blColors[colorname.toLower()] = colorid;
                }
            }
        }
    }

    if (ld2blColorsXRef.size() == 0) {
        QString ld2blColorsXRefFile = Preferences::ld2blColorsXRefFile;
        QRegExp rx("^([^\\t]+)\\t+\\s*([^\\t]+).*$");
        if (!ld2blColorsXRefFile.isEmpty()) {
            QFile file(ld2blColorsXRefFile);
            if ( ! file.open(QFile::ReadOnly | QFile::Text)) {
                QString message = QString("Failed to open ld2blcolorsxref.lst file: %1:\n%2")
                                          .arg(ld2blColorsXRefFile)
                                          .arg(file.errorString());
                if (Preferences::modeGUI){
                    QMessageBox::warning(nullptr,QMessageBox::tr("LPub3D"),message);
                } else {
                    logError() << message;
                }
                return;
            }
            QTextStream in(&file);

            // Load RegExp from file;
            QRegExp rxin("^#\\sThe\\sRegular\\sExpression\\sused\\sis\\:[\\s](\\^.*)$");
            while ( ! in.atEnd()) {
                QString sLine = in.readLine(0);
                if (sLine.contains(rxin)) {
                    rx.setPattern(rxin.cap(1));
//                    logDebug() << "LD2BL ColorsXRef RegExp Pattern: " << rxin.cap(1);
                    break;
                }
            }

            in.seek(0);

            // Load input values
            while ( ! in.atEnd()) {
                QString sLine = in.readLine(0);
                if (sLine.contains(rx)) {
                    QString ldcolorid = rx.cap(1);
                    QString blcolorid = rx.cap(2).trimmed();
                    ld2blColorsXRef[ldcolorid.toLower()] = blcolorid;
                }
            }
        } else {
            ld2blColorsXRef.clear();
            QByteArray Buffer;
            loadLD2BLColorsXRef(Buffer);
            QTextStream instream(Buffer);
            for (QString sLine = instream.readLine(); !sLine.isNull(); sLine = instream.readLine())
            {
                QChar comment = sLine.at(0);
                if (comment == '#' || comment == ' ')
                    continue;
                if (sLine.contains(rx)) {
                    QString ldcolorid = rx.cap(1);
                    QString blcolorid = rx.cap(2).trimmed();
                    ld2blColorsXRef[ldcolorid.toLower()] = blcolorid;
                }
            }
        }
    }

    if (ld2blCodesXRef.size() == 0) {
        QString ld2blCodesXRefFile = Preferences::ld2blCodesXRefFile;
        QRegExp rx("^([^\\t]+)\\t+\\s*([^\\t]+).*$");
        if (!ld2blCodesXRefFile.isEmpty()) {
            QFile file(ld2blCodesXRefFile);
            if ( ! file.open(QFile::ReadOnly | QFile::Text)) {
                QString message = QString("Failed to open ld2blcodesxref.lst file: %1:\n%2")
                                          .arg(ld2blCodesXRefFile)
                                          .arg(file.errorString());
                if (Preferences::modeGUI){
                    QMessageBox::warning(nullptr,QMessageBox::tr("LPub3D"),message);
                } else {
                    logError() << message;
                }
                return;
            }
            QTextStream in(&file);

            // Load RegExp from file;
            QRegExp rxin("^#\\sThe\\sRegular\\sExpression\\sused\\sis\\:[\\s](\\^.*)$");
            while ( ! in.atEnd()) {
                QString sLine = in.readLine(0);
                if (sLine.contains(rxin)) {
                    rx.setPattern(rxin.cap(1));
//                    logDebug() << "LD2BL CodesXRef RegExp Pattern: " << rxin.cap(1);
                    break;
                }
            }

           in.seek(0);

            // Load input values
            while ( ! in.atEnd()) {
                QString sLine = in.readLine(0);
                if (sLine.contains(rx)) {
                    QString ldpartid = rx.cap(1);
                    QString blitemid = rx.cap(2).trimmed();
                    ld2blCodesXRef[ldpartid.toLower()] = blitemid;
                }
            }
        } else {
            ld2blCodesXRef.clear();
            QByteArray Buffer;
            loadLD2BLCodesXRef(Buffer);
            QTextStream instream(Buffer);
            for (QString sLine = instream.readLine(); !sLine.isNull(); sLine = instream.readLine())
            {
                QChar comment = sLine.at(0);
                if (comment == '#' || comment == ' ')
                    continue;
                if (sLine.contains(rx)) {
                    QString ldpartid = rx.cap(1);
                    QString blitemid = rx.cap(2).trimmed();
                    ld2blCodesXRef[ldpartid.toLower()] = blitemid;
                }
            }
        }
    }
}

// key : blitemid+blcolorid
// val1: blitemid+"-"+blcolorid
// val2: elementid
bool Annotations::loadBLElements(){
    if (blElements.size() == 0) {
        QString blElementsFile = Preferences::blElementsFile;
        QRegExp rx("^([^\\t]+)\\t+\\s*([^\\t]+)\\t+\\s*([^\\t]+).*$");
        if (! blElementsFile.isEmpty()) {
            QFile file(blElementsFile);
            if ( ! file.open(QFile::ReadOnly | QFile::Text)) {
                QString message = QString("Failed to open BrickLink codes.txt file: %1:\n%2")
                                          .arg(blElementsFile)
                                          .arg(file.errorString());
                if (Preferences::modeGUI){
                    QMessageBox::warning(nullptr,QMessageBox::tr("LPub3D"),message);
                } else {
                    logError() << message;
                }
                return false;
            }
            QTextStream in(&file);

            // Load RegExp from file;
            QRegExp rxin("^#\\sThe\\sRegular\\sExpression\\sused\\sis\\:[\\s](\\^.*)$");
            while ( ! in.atEnd()) {
                QString sLine = in.readLine(0);
                if (sLine.contains(rxin)) {
                    rx.setPattern(rxin.cap(1));
//                    logDebug() << "Bricklink elements RegExp Pattern: " << rxin.cap(1);
                    break;
                }
            }

            in.seek(0);

// DEBUG -->>>
//            QString fooFile = Preferences::blElementsFile+"demo.txt";
//            QFile File(fooFile);
//            if (!File.open(QIODevice::WriteOnly))
//                return false;
//            QTextStream Stream(&File);
// DEBUG <<<---

            // Load input values
            while ( ! in.atEnd()) {
                QString sLine = in.readLine(0);
                if (sLine.contains(rx)) {
                    QString blitemid = rx.cap(1);
                    QString blcolorid = getBLColorID(rx.cap(2));
                    QString elementid = rx.cap(3);
                    blElements[QString(blitemid+blcolorid).toLower()] << QString(blitemid+"-"+blcolorid).toUpper() << elementid;
// DEBUG -->>>
//                    Stream << QString("Key: %1 Value[0]: %2 Value[1]: %3")
//                                      .arg(QString(blitemid+blcolorid).toLower())
//                                      .arg(QString(blitemid+"-"+blcolorid).toUpper())
//                                      .arg(elementid);
// DEBUG <<<---
                }
            }
// DEBUG -->>>
//            Stream.flush();
// DEBUG <<<---

        } else {
           return  false;
        }
    }
    return true;
}

bool Annotations::loadBLElements(QByteArray &Buffer){
    if (blElements.size() == 0) {
        QRegExp rx("^([^\\t]+)\\t+\\s*([^\\t]+)\\t+\\s*([^\\t]+).*$");
        QTextStream instream(Buffer);

        // Load RegExp pattern from Buffer;
        QRegExp rxin("^#\\sThe\\sRegular\\sExpression\\sused\\sis\\:[\\s](\\^.*)$");
        while ( ! instream.atEnd()) {
            QString sLine = instream.readLine(0);
            if (sLine.contains(rxin)) {
                rx.setPattern(rxin.cap(1));
//                logDebug() << "Bricklink elements RegExp Pattern: " << rxin.cap(1);
                break;
            }
        }

        instream.seek(0);

        //     Load input values from instream
        for (QString sLine = instream.readLine(); !sLine.isNull(); sLine = instream.readLine())
        {
            QChar comment = sLine.at(0);
            if (comment == '#' || comment == ' ')
                continue;
            if (sLine.contains(rx)) {
                QString blitemid = rx.cap(1);
                QString blcolorid = getBLColorID(rx.cap(2));
                QString elementid = rx.cap(3);
                blElements[QString(blitemid+blcolorid).toLower()] << QString(blitemid+"-"+blcolorid).toUpper() << elementid;
            }
        }

        //    write stream to file
        QFile file(QString("%1/extras/%2").arg(Preferences::lpubDataPath,VER_LPUB3D_BLELEMENTS_FILE));
        if(file.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            int counter = 1;
            QTextStream instream(Buffer);
            QTextStream outstream(&file);

            while ( ! instream.atEnd()) {
                QString sLine = instream.readLine(0);
                outstream << sLine << endl;
                counter++;
            }

            outstream.flush();
            file.close();

            QString message = QString("Finished Writing, Proceed %1 lines for file [%2]")
                                      .arg(counter)
                                      .arg(file.fileName());
            if (Preferences::modeGUI){
                QMessageBox::information(nullptr,QMessageBox::tr("LPub3D"),message);
            } else {
                logInfo() << message;
            }
        }
        else
        {
           QString message = QString("Failed to write file: %1:\n%2")
                                     .arg(file.fileName())
                                     .arg(file.errorString());
           if (Preferences::modeGUI){
               QMessageBox::warning(nullptr,QMessageBox::tr("LPub3D"),message);
           } else {
               logError() << message;
           }
           return false;
        }
        return true;
    }
    return true;
}

// key: ldpartid+ldcolorid
// val: elementid
bool Annotations::loadLEGOElements(){
    if (legoElements.size() == 0) {
        QString legoElementsFile = Preferences::legoElementsFile;
        QRegExp rx("^([^\\t]+)\\t+\\s*([^\\t]+)\\t+\\s*([^\\t]+).*$");
        if (!legoElementsFile.isEmpty()) {
            QFile file(legoElementsFile);
            if ( ! file.open(QFile::ReadOnly | QFile::Text)) {
                QString message = QString("Failed to open legoelements.lst file: %1:\n%2")
                                          .arg(legoElementsFile)
                                          .arg(file.errorString());
                if (Preferences::modeGUI){
                    QMessageBox::warning(nullptr,QMessageBox::tr("LPub3D"),message);
                } else {
                    logError() << message;
                }
                return false;
            }
            QTextStream in(&file);

            // Load RegExp from file;
            QRegExp rxin("^#\\sThe\\sRegular\\sExpression\\sused\\sis\\:[\\s](\\^.*)$");
            while ( ! in.atEnd()) {
                QString sLine = in.readLine(0);
                if (sLine.contains(rxin)) {
                    rx.setPattern(rxin.cap(1));
//                    logDebug() << "LEGO elements RegExp Pattern: " << rxin.cap(1);
                    break;
                }
            }

            in.seek(0);

            // Load input values
            while ( ! in.atEnd()) {
                QString sLine = in.readLine(0);
                if (sLine.contains(rx)) {
                    QString ldpartid = rx.cap(1);
                    QString ldcolorid = rx.cap(2);
                    QString elementid = rx.cap(3);
                    legoElements[QString(ldpartid+ldcolorid).toLower()] = elementid;
                }
            }
        } else {
            QString message = QString("LEGO Elements file was not found : %1").arg(legoElementsFile);
            if (Preferences::modeGUI){
                QMessageBox::warning(nullptr,QMessageBox::tr("LPub3D"),message);
            } else {
                logError() << message;
            }
            return  false;
        }
    }
    return true;
}

const QString &Annotations::freeformAnnotation(QString part)
{
  if (freeformAnnotations.contains(part.toLower())) {
    return freeformAnnotations[part.toLower()];
  }
  return returnString;
}

const int &Annotations::getAnnotationStyle(QString part)
{
  returnInt = 0;
  if (annotationStyles.contains(part.toLower())) {
    bool ok;
    returnInt = annotationStyles[part.toLower()][0].toInt(&ok);
    if (ok)
      return returnInt;
  }
  return returnInt;
}

const int &Annotations::getAnnotationCategory(QString part){
  returnInt = 0;
  if (annotationStyles.contains(part.toLower())) {
    bool ok;
    returnInt = annotationStyles[part.toLower()][1].toInt(&ok);
    if (ok)
      return returnInt;
  }
  return returnInt;
}

const QString &Annotations::getStyleAnnotation(QString part)
{
  if (annotationStyles.contains(part.toLower())) {
    return annotationStyles[part.toLower()][2];
  }
  return returnString;
}

const QString &Annotations::getBLColorID(QString blcolorname)
{
  if (blColors.contains(blcolorname.toLower())) {
    return blColors[blcolorname.toLower()];
  }
  return returnString;
}

//key: ldpartid+ldcolorid
const QString &Annotations::getLEGOElement(QString elementkey)
{
    loadLEGOElements();
    if (legoElements.contains(elementkey.toLower())) {
        return legoElements[elementkey.toLower()];
    }

    return returnString;
}

const QString &Annotations::getBLElement(QString ldcolorid, QString ldpartid, int which)
{
    loadBLElements();
    QString blcolorid,elementkey;
    if (ld2blColorsXRef.contains(ldcolorid.toLower())) {
        blcolorid = ld2blColorsXRef[ldcolorid.toLower()];
    }
    if (!blcolorid.isEmpty()){
        elementkey = QString(ldpartid+blcolorid).toLower();
        if (blElements.contains(elementkey)){
            return blElements[elementkey][which];
        }
        else
        if (ld2blCodesXRef.contains(ldpartid.toLower())) {
            elementkey = QString(ld2blCodesXRef[ldpartid.toLower()]+blcolorid).toLower();
            if (blElements.contains(elementkey)) {
                return blElements[elementkey][which];
            }
        }
    }
    return returnString;
}

bool Annotations::exportAnnotationStyleFile(){
    QFile file(QString("%1/extras/%2").arg(Preferences::lpubDataPath,Preferences::validAnnotationStyleFile));
    if(file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        int counter = 1;
        QTextStream outstream(&file);
        outstream << "# File:" << Preferences::validAnnotationStyleFile << endl;
        outstream << "#" << endl;
        outstream << "# The Regular Expression used is: ^([^\\t]+)\\t+\\s*([^\\t]+)\\t+\\s*([^\\t]+).*$" << endl;
        outstream << "#" << endl;
        outstream << "# 1. Part ID:             LDraw Part Name              (Required)" << endl;
        outstream << "# 2. Annotation Style:    1 for Square or 2 for Circle (Required)" << endl;
        outstream << "# 3. Annotation Category: Part category                (Required)" << endl;
        outstream << "# 4. Annotation:          Annotation text (uses part title annotation if no text defined)" << endl;
        outstream << "# 5. Part Description:    Description for reference only. Not loaded" << endl;
        outstream << "# Fields 4-5 are optional" << endl;
        outstream << "#" << endl;
        outstream << "# LEGO Standard:" << endl;
        outstream << "# ------------------------------------" << endl;
        outstream << "# |No |Category      |Annotation Style|" << endl;
        outstream << "# |---|-------------------------------|" << endl;
        outstream << "# | 1 |round(1)      |axle(1)         |" << endl;
        outstream << "# | 2 |square(2)     |beam(2)         |" << endl;
        outstream << "# | 3 |square(2)     |cable(3)        |" << endl;
        outstream << "# | 4 |square(2)     |connector(4)    |" << endl;
        outstream << "# | 5 |square(2)     |hose(5)         |" << endl;
        outstream << "# | 6 |square(2)     |panel(6)        |" << endl;
        outstream << "# ------------------------------------" << endl;
        outstream << "#" << endl;
        outstream << "# 32034.dat=1  4  2    Technic Angle Connector #2 (180 degree)" << endl;
        outstream << "#" << endl;

        QByteArray Buffer;
        loadDefaultAnnotationStyles(Buffer);
        QTextStream instream(Buffer);
        for (QString sLine = instream.readLine(); !sLine.isNull(); sLine = instream.readLine())
        {
            outstream << sLine << endl;
            counter++;
        }

        file.close();
        QString message = QString("Finished Writing Annotation Style Entries, Processed %1 lines in file [%2]")
                                   .arg(counter)
                                   .arg(file.fileName());
        if (Preferences::modeGUI){
            QMessageBox::information(nullptr,QMessageBox::tr("LPub3D"),message);
        } else {
            logNotice() << message;
        }
    }
    else
    {
        QString message = QString("Failed to open Annotation style file: %1:\n%2")
                                  .arg(file.fileName())
                                  .arg(file.errorString());
        if (Preferences::modeGUI){
            QMessageBox::warning(nullptr,QMessageBox::tr("LPub3D"),message);
        } else {
            logError() << message;
        }
       return false;
    }
    return true;
}

bool Annotations::exportBLColorsFile(){
    QFile file(QString("%1/extras/%2").arg(Preferences::lpubDataPath,"/" VER_LPUB3D_BLCOLORS_FILE));
    if(file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        int counter = 1;
        QTextStream outstream(&file);
        outstream << "# File:" << VER_LPUB3D_BLCOLORS_FILE << endl;
        outstream << "#" << endl;
        outstream << "# Tab-delmited BrickLink Color codes" << endl;
        outstream << "#" << endl;
        outstream << "# The Regular Expression used is: ^([^\\t]+)\\t+\\s*([^\\t]+).*$" << endl;
        outstream << "#" << endl;
        outstream << "# 1. Color ID:            BrickLink Color ID             (Required)" << endl;
        outstream << "# 2. Color Name:          BrickLink Color Name           (Required)" << endl;
        outstream << "# 3+ Other Fields:        Other fields are not used" << endl;
        outstream << "#" << endl;

        QByteArray Buffer;
        loadBLColors(Buffer);
        QTextStream instream(Buffer);
        for (QString sLine = instream.readLine(); !sLine.isNull(); sLine = instream.readLine())
        {
            outstream << sLine << endl;
            counter++;
        }

        file.close();
        QString message = QString("Finished Writing BrickLink Color Code Entries, Processed %1 lines in file [%2]")
                                   .arg(counter)
                                   .arg(file.fileName());
        if (Preferences::modeGUI){
            QMessageBox::information(nullptr,QMessageBox::tr("LPub3D"),message);
        } else {
            logNotice() << message;
        }
    }
    else
    {
        QString message = QString("Failed to open BrickLink Color Code file: %1:\n%2")
                                  .arg(file.fileName())
                                  .arg(file.errorString());
        if (Preferences::modeGUI){
            QMessageBox::warning(nullptr,QMessageBox::tr("LPub3D"),message);
        } else {
            logError() << message;
        }
       return false;
    }
    return true;
}

bool Annotations::exportLD2BLColorsXRefFile(){
    QFile file(QString("%1/extras/%2").arg(Preferences::lpubDataPath,"/" VER_LPUB3D_LD2BLCOLORSXREF_FILE));
    if(file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        int counter = 1;
        QTextStream outstream(&file);
        outstream << "# File:" << VER_LPUB3D_LD2BLCOLORSXREF_FILE << endl;
        outstream << "#" << endl;
        outstream << "# Tab-delmited LDConfig and BrickLink Color code cross reference" << endl;
        outstream << "#" << endl;
        outstream << "# The Regular Expression used is: ^([^\\t]+)\\t+\\s*([^\\t]+).*$" << endl;
        outstream << "#" << endl;
        outstream << "# 1. LDConfig ID:       LDraw Color ID             (Required)" << endl;
        outstream << "# 2. Color ID:          BrickLink Color ID         (Required)" << endl;
        outstream << "#" << endl;

        QByteArray Buffer;
        loadLD2BLColorsXRef(Buffer);
        QTextStream instream(Buffer);
        for (QString sLine = instream.readLine(); !sLine.isNull(); sLine = instream.readLine())
        {
            outstream << sLine << endl;
            counter++;
        }

        file.close();
        QString message = QString("Finished Writing LDConfig and BrickLink Color Code Entries, Processed %1 lines in file [%2]")
                                   .arg(counter)
                                   .arg(file.fileName());
        if (Preferences::modeGUI){
            QMessageBox::information(nullptr,QMessageBox::tr("LPub3D"),message);
        } else {
            logNotice() << message;
        }
    }
    else
    {
        QString message = QString("Failed to open LDConfig and BrickLink Color Code file: %1:\n%2")
                                  .arg(file.fileName())
                                  .arg(file.errorString());
        if (Preferences::modeGUI){
            QMessageBox::warning(nullptr,QMessageBox::tr("LPub3D"),message);
        } else {
            logError() << message;
        }
       return false;
    }
    return true;
}

bool Annotations::exportLD2BLCodesXRefFile(){
    QFile file(QString("%1/extras/%2").arg(Preferences::lpubDataPath,"/" VER_LPUB3D_LD2BLCODESXREF_FILE));
    if(file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        int counter = 1;
        QTextStream outstream(&file);
        outstream << "# File:" << VER_LPUB3D_LD2BLCODESXREF_FILE << endl;
        outstream << "#" << endl;
        outstream << "# Tab-delmited LDraw Design ID and BrickLink Item Number cross reference" << endl;
        outstream << "#" << endl;
        outstream << "# The Regular Expression used is: ^([^\\t]+)\\t+\\s*([^\\t]+).*$" << endl;
        outstream << "#" << endl;
        outstream << "# 1. LDraw ID:             LDraw Design ID              (Required)" << endl;
        outstream << "# 2. Item No:              BrickLink Item Number        (Required)" << endl;
        outstream << "#" << endl;

        QByteArray Buffer;
        loadLD2BLCodesXRef(Buffer);
        QTextStream instream(Buffer);
        for (QString sLine = instream.readLine(); !sLine.isNull(); sLine = instream.readLine())
        {
            outstream << sLine << endl;
            counter++;
        }

        file.close();
        QString message = QString("Finished Writing LDraw Design ID and BrickLink Item Number Entries, Processed %1 lines in file [%2]")
                                   .arg(counter)
                                   .arg(file.fileName());
        if (Preferences::modeGUI){
            QMessageBox::information(nullptr,QMessageBox::tr("LPub3D"),message);
        } else {
            logNotice() << message;
        }
    }
    else
    {
        QString message = QString("Failed to open LDraw Design ID and BrickLink Item Number file: %1:\n%2")
                                  .arg(file.fileName())
                                  .arg(file.errorString());
        if (Preferences::modeGUI){
            QMessageBox::warning(nullptr,QMessageBox::tr("LPub3D"),message);
        } else {
            logError() << message;
        }
       return false;
    }
    return true;
}
