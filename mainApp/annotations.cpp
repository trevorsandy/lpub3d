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
#include <QIcon>
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

QHash<QString, QStringList> Annotations::blCodes;
QHash<QString, QString>     Annotations::legoElements;
QHash<QString, QString>     Annotations::blColors;
QHash<QString, QString>     Annotations::ld2blColorsXRef;
QHash<QString, QString>     Annotations::ld2blCodesXRef;

QHash<QString, QString>     Annotations::ld2rbColorsXRef;
QHash<QString, QString>     Annotations::ld2rbCodesXRef;

void Annotations::loadLD2BLColorsXRef(QByteArray& Buffer){
/*
# File: ld2blcolorsxref.lst
#
# Tab-delmited LDConfig and BrickLink Color code cross reference
#
# The Regular Expression used is: ^([^\t]+)\t+\s*([^\t]+).*$
#
# 1. LDConfig Color ID: LDraw Color ID             (Required)
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
        "216\t 73\n"
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
// LDraw rubber colours
        "65\t  3\n"
        "66\t  19\n"
        "67\t  12\n"
        "256\t 11\n"
        "273\t 7\n"
        "324\t 5\n"
        "350\t 4\n"
        "375\t 9\n"
        "406\t 63\n"
        "449\t 24\n"
        "490\t 34\n"
        "496\t 86\n"
        "504\t 95\n"
        "511\t 1\n"
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
# The Regular Expression used is: ^([^\t]+)\t+\s*([^\t]+).*$
#
# 1. Design ID:            LDraw Part Number            (Required)
# 2. Item No:              BrickLink Item Number        (Required)
#
*/
    const char LEGOLD2BLCodesXRef[] = {
        "3245a\t    3245b\n"
        "3957a\t    3957\n"
        "19475-f1\t    19475c01\n"
        "19478-f1\t    19467c01\n"
        "26674-f1\t    bb875\n"
        "32123a\t    4265c\n"
        "3648b\t    3648\n"
        "4032a\t    4032\n"
        "47223d-f1\t    4694cc01\n"
        "58120\t    58120c01\n"
        "58121\t    58121c01\n"
        "58123\t    58123c01\n"
        "61929\t    bb339c01\n"
        "62821\t    62821b\n"
        "75974\t    67c01\n"
        "92693c01-f1\t    92693c01\n"
        "99498\t    99498c01\n"
// from Bricklink alternate item numbers - see https://github.com/trevorsandy/lpub3d/issues/283
        "30\t    70644\n"
        "577\t    64567\n"
        "578\t    30374\n"
        "769\t    23318\n"
        "2543\t    x70\n"
        "2588\t    2888\n"
        "2723\t    2958\n"
        "2748\t    3857\n"
        "3729\t    3731\n"
        "3815\t    970\n"
        "3816\t    971\n"
        "3817\t    972\n"
        "3818\t    982\n"
        "3819\t    981\n"
        "3820\t    983\n"
        "4268\t    353\n"
        "4328\t    x93\n"
        "4503\t    x167\n"
        "6090\t    x41\n"
        "6093\t    x104\n"
        "6098\t    3867\n"
        "6116\t    3941\n"
        "6141\t    4073\n"
        "6143\t    3941\n"
        "6188\t    4589\n"
        "6218\t    6259\n"
        "6221\t    424\n"
        "6223\t    3003\n"
        "6225\t    3023\n"
        "6227\t    3039\n"
        "6234\t    600\n"
        "6237\t    601\n"
        "6255\t    x8\n"
        "6264\t    3899\n"
        "6269\t    2343\n"
        "6270\t    3040\n"
        "6562\t    3749\n"
        "6590\t    3713\n"
        "10199\t    4672\n"
        "10201\t    2436b\n"
        "10314\t    6191\n"
        "10498\t    90641\n"
        "11153\t    61678\n"
        "12825\t    2555\n"
        "13670\t    6587\n"
        "13731\t    85970\n"
        "14395\t    2339\n"
        "14533\t    90640\n"
        "14696\t    3711\n"
        "15207\t    30413\n"
        "15379\t    3873\n"
        "15556\t    64567\n"
        "15646\t    90202\n"
        "15672\t    92946\n"
        "16091\t    30663\n"
        "17514\t    88000\n"
        "18228\t    98385\n"
        "18575\t    32269\n"
        "18626\t    4025\n"
        "18738\t    30193\n"
        "18759\t    2341\n"
        "20430\t    6259\n"
        "21709\t    30394\n"
        "21731\t    93274\n"
        "22749\t    32190\n"
        "22750\t    32191\n"
        "22972\t    32188\n"
        "22973\t    32189\n"
        "23065\t    41250\n"
        "23221\t    x153\n"
        "23893\t    87580\n"
        "24505\t    3648\n"
        "27059\t    90541\n"
        "28290\t    56890\n"
        "28618\t    6126b\n"
        "28653\t    3023\n"
        "28809\t    18677\n"
        "28920\t    13770\n"
        "28959\t    43892\n"
        "28964\t    76766\n"
        "29219\t    62462\n"
        "30007\t    4215\n"
        "30008\t    3024\n"
        "30009\t    2362\n"
        "30010\t    4865\n"
        "30039\t    3070b\n"
        "30057\t    4073\n"
        "30059\t    4346\n"
        "30060\t    4345\n"
        "30061\t    4218\n"
        "30063\t    4740\n"
        "30065\t    3960\n"
        "30068\t    3062b\n"
        "30069\t    4070\n"
        "30070\t    3069b\n"
        "30071\t    3005\n"
        "30124\t    2446\n"
        "30125\t    4533\n"
        "30130\t    6133\n"
        "30224\t    x59\n"
        "30234\t    4285b\n"
        "30244\t    2412b\n"
        "30259\t    892\n"
        "30337\t    6192\n"
        "30345\t    989\n"
        "30369\t    x52\n"
        "30370\t    x164\n"
        "30385\t    52\n"
        "30475\t    x161\n"
        "30480\t    x134\n"
        "30499\t    3684\n"
        "30561\t    x132\n"
        "30741\t    62576\n"
        "31459\t    3011\n"
        "31460\t    3437\n"
        "31511\t    98585\n"
        "32005\t    2739a\n"
        "32006\t    2825\n"
        "32086\t    551\n"
        "32123\t    4265c\n"
        "32126\t    44\n"
        "32239\t    6588\n"
        "32277\t    41239\n"
        "32294\t    x136\n"
        "32476\t    x240\n"
        "32496\t    x873\n"
        "32531\t    40344\n"
        "32532\t    40345\n"
        "33122\t    x112\n"
        "33176\t    x12\n"
        "33763\t    78c26\n"
        "34432\t    3649\n"
        "35331\t    87606\n"
        "35334\t    2447\n"
        "35338\t    54200\n"
        "35343\t    22388\n"
        "35382\t    3005\n"
        "35386\t    3069b\n"
        "37364\t    970cm00\n"
        "40240\t    x219\n"
        "40620\t    71137b\n"
        "41357\t    40249\n"
        "41669\t    x346\n"
        "41751\t    x224\n"
        "41761\t    42021\n"
        "41762\t    42022\n"
        "41763\t    42023\n"
        "42019\t    41749\n"
        "42020\t    41750\n"
        "42114\t    x156\n"
        "42284\t    30293\n"
        "42291\t    30294\n"
        "42409\t    x789\n"
        "42534\t    41539\n"
        "42657\t    4066\n"
        "43127\t    2419\n"
        "43337\t    30413\n"
        "43675\t    78c19\n"
        "43802\t    4201\n"
        "44041\t    4204\n"
        "44042\t    2356\n"
        "44237\t    2456\n"
        "44360\t    x181\n"
        "44590\t    3754\n"
        "44882\t    43898\n"
        "45505\t    3622\n"
        "46361\t    30562\n"
        "46667\t    x577\n"
        "47904\t    30602\n"
        "48310\t    47543\n"
        "49673\t    49668\n"
        "50231\t    522\n"
        "50746\t    54200\n"
        "50903\t    x1307\n"
        "50951\t    50945\n"
        "51000\t    x1435\n"
        "51011\t    42611\n"
        "51373\t    50990\n"
        "51595\t    30225\n"
        "51930\t    41250\n"
        "52211\t    3471\n"
        "52639\t    6510\n"
        "52901\t    2958\n"
        "53454\t    53705\n"
        "53933\t    30293\n"
        "53934\t    30294\n"
        "54196\t    2654\n"
        "55539\t    42604\n"
        "55709\t    32199\n"
        "56204\t    45411\n"
        "56640\t    43898\n"
        "56907\t    x1825\n"
        "57488\t    32558\n"
        "58572\t    4094\n"
        "58772\t    44359\n"
        "58773\t    44359\n"
        "59195\t    45705\n"
        "59350\t    59349\n"
        "59426\t    32209\n"
        "59443\t    6538c\n"
        "59858\t    x167\n"
        "59900\t    4589b\n"
        "60033\t    4202\n"
        "60169\t    30104\n"
        "60288\t    57587\n"
        "60475\t    30241\n"
        "60801\t    60583\n"
        "60803\t    57895\n"
        "60897\t    4085d\n"
        "61183\t    43753\n"
        "61903\t    62520c01\n"
        "62622\t    4738a\n"
        "62623\t    4739a\n"
        "62930\t    54930c02\n"
        "64288\t    4589b\n"
        "64289\t    40490\n"
        "64290\t    32525\n"
        "64634\t    62576\n"
        "64871\t    32278\n"
        "70358\t    590\n"
        "70973\t    48245\n"
        "71014\t    59\n"
        "71128\t    x85\n"
        "71874\t    4740\n"
        "71944\t    78c06\n"
        "71951\t    78c08\n"
        "71952\t    78c04\n"
        "71986\t    78c11\n"
        "72039\t    78c18\n"
        "72504\t    78c02\n"
        "72706\t    78c03\n"
        "72824\t    x45\n"
        "74695\t    60483\n"
        "75347\t    6168c01\n"
        "75977\t    30086\n"
        "76116\t    98313\n"
        "76324\t    75c09\n"
        "76385\t    989\n"
        "85543\t    x71\n"
        "85544\t    x37\n"
        "85545\t    x89\n"
        "85546\t    x90\n"
        "86035\t    4485b\n"
        "86209\t    60601\n"
        "86210\t    60603\n"
        "88323\t    57518\n"
        "88412\t    3878\n"
        "88415\t    2446\n"
        "88422\t    42511\n"
        "88513\t    53451\n"
        "89650\t    61053\n"
        "90194\t    48183\n"
        "90508\t    90301\n"
        "90510\t    30171\n"
        "91143\t    2431\n"
        "92290\t    92289\n"
        "92410\t    4532\n"
        "92411\t    44728\n"
        "92538\t    3006\n"
        "92756\t    30410\n"
        "92903\t    6005\n"
        "93791\t    2654\n"
        "93792\t    3004\n"
        "93888\t    3007\n"
        "94148\t    3022\n"
        "94638\t    87552\n"
        "95820\t    30237b\n"
        "2533c01\t    x110c01\n"
        "25866c01\t    93094c01\n"
        "25866c02\t    93094c02\n"
        "25866c03\t    93094c03\n"
        "30367b\t    553b\n"
        "30367c\t    553c\n"
        "32495c01\t    x873c01\n"
        "60475b\t    30241b\n"
        "x223\t    33320\n"
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
# Tab-delmited BrickLink Color code and Color Name cross reference
#
# The Regular Expression used is: ^([^\t]+)\t+\s*([^\t]+).*$
#
# 1. Color ID:            BrickLink Color ID             (Required)
# 2. Color Name:          BrickLink Color Name           (Required)
#
*/
    const char LEGOBLColors[] = {
        "0\t      (Not Applicable)\n"
        "41\t     Aqua\n"
        "11\t     Black\n"
        "7\t      Blue\n"
        "97\t     Blue-Violet\n"
        "36\t     Bright Green\n"
        "105\t    Bright Light Blue\n"
        "110\t    Bright Light Orange\n"
        "103\t    Bright Light Yellow\n"
        "104\t    Bright Pink\n"
        "8\t      Brown\n"
        "220\t    Coral\n"
        "153\t    Dark Azure\n"
        "63\t     Dark Blue\n"
        "109\t    Dark Blue-Violet\n"
        "85\t     Dark Bluish Gray\n"
        "120\t    Dark Brown\n"
        "91\t     Dark Flesh\n"
        "10\t     Dark Gray\n"
        "80\t     Dark Green\n"
        "68\t     Dark Orange\n"
        "47\t     Dark Pink\n"
        "89\t     Dark Purple\n"
        "59\t     Dark Red\n"
        "69\t     Dark Tan\n"
        "39\t     Dark Turquoise\n"
        "161\t    Dark Yellow\n"
        "29\t     Earth Orange\n"
        "106\t    Fabuland Brown\n"
        "160\t    Fabuland Orange\n"
        "28\t     Flesh\n"
        "6\t      Green\n"
        "154\t    Lavender\n"
        "152\t    Light Aqua\n"
        "62\t     Light Blue\n"
        "86\t     Light Bluish Gray\n"
        "90\t     Light Flesh\n"
        "9\t      Light Gray\n"
        "38\t     Light Green\n"
        "35\t     Light Lime\n"
        "32\t     Light Orange\n"
        "56\t     Light Pink\n"
        "93\t     Light Purple\n"
        "26\t     Light Salmon\n"
        "40\t     Light Turquoise\n"
        "44\t     Light Violet\n"
        "33\t     Light Yellow\n"
        "34\t     Lime\n"
        "72\t     Maersk Blue\n"
        "71\t     Magenta\n"
        "156\t    Medium Azure\n"
        "42\t     Medium Blue\n"
        "150\t    Medium Dark Flesh\n"
        "94\t     Medium Dark Pink\n"
        "37\t     Medium Green\n"
        "157\t    Medium Lavender\n"
        "76\t     Medium Lime\n"
        "31\t     Medium Orange\n"
        "73\t     Medium Violet\n"
        "166\t    Neon Green\n"
        "165\t    Neon Orange\n"
        "155\t    Olive Green\n"
        "4\t      Orange\n"
        "23\t     Pink\n"
        "24\t     Purple\n"
        "5\t      Red\n"
        "88\t     Reddish Brown\n"
        "27\t     Rust\n"
        "25\t     Salmon\n"
        "55\t     Sand Blue\n"
        "48\t     Sand Green\n"
        "54\t     Sand Purple\n"
        "58\t     Sand Red\n"
        "87\t     Sky Blue\n"
        "2\t      Tan\n"
        "99\t     Very Light Bluish Gray\n"
        "49\t     Very Light Gray\n"
        "96\t     Very Light Orange\n"
        "43\t     Violet\n"
        "1\t      White\n"
        "3\t      Yellow\n"
        "158\t    Yellowish Green\n"
        "13\t     Trans-Black\n"
        "108\t    Trans-Bright Green\n"
        "12\t     Trans-Clear\n"
        "14\t     Trans-Dark Blue\n"
        "50\t     Trans-Dark Pink\n"
        "20\t     Trans-Green\n"
        "15\t     Trans-Light Blue\n"
        "221\t    Trans-Light Green\n"
        "164\t    Trans-Light Orange\n"
        "114\t    Trans-Light Purple\n"
        "74\t     Trans-Medium Blue\n"
        "16\t     Trans-Neon Green\n"
        "18\t     Trans-Neon Orange\n"
        "121\t    Trans-Neon Yellow\n"
        "98\t     Trans-Orange\n"
        "107\t    Trans-Pink\n"
        "51\t     Trans-Purple\n"
        "17\t     Trans-Red\n"
        "113\t    Trans-Very Lt Blue\n"
        "19\t     Trans-Yellow\n"
        "57\t     Chrome Antique Brass\n"
        "122\t    Chrome Black\n"
        "52\t     Chrome Blue\n"
        "21\t     Chrome Gold\n"
        "64\t     Chrome Green\n"
        "82\t     Chrome Pink\n"
        "22\t     Chrome Silver\n"
        "84\t     Copper\n"
        "81\t     Flat Dark Gold\n"
        "95\t     Flat Silver\n"
        "78\t     Metal Blue\n"
        "77\t     Pearl Dark Gray\n"
        "115\t    Pearl Gold\n"
        "61\t     Pearl Light Gold\n"
        "66\t     Pearl Light Gray\n"
        "119\t    Pearl Very Light Gray\n"
        "83\t     Pearl White\n"
        "65\t     Metallic Gold\n"
        "70\t     Metallic Green\n"
        "67\t     Metallic Silver\n"
        "46\t     Glow In Dark Opaque\n"
        "118\t    Glow In Dark Trans\n"
        "159\t    Glow In Dark White\n"
        "60\t     Milky White\n"
        "101\t    Glitter Trans-Clear\n"
        "100\t    Glitter Trans-Dark Pink\n"
        "162\t    Glitter Trans-Light Blue\n"
        "163\t    Glitter Trans-Neon Green\n"
        "102\t    Glitter Trans-Purple\n"
        "116\t    Speckle Black-Copper\n"
        "151\t    Speckle Black-Gold\n"
        "111\t    Speckle Black-Silver\n"
        "117\t    Speckle DBGray-Silver\n"
        "142\t    Mx Aqua Green\n"
        "128\t    Mx Black\n"
        "132\t    Mx Brown\n"
        "133\t    Mx Buff\n"
        "126\t    Mx Charcoal Gray\n"
        "149\t    Mx Clear\n"
        "214\t    Mx Foil Dark Blue\n"
        "210\t    Mx Foil Dark Gray\n"
        "212\t    Mx Foil Dark Green\n"
        "215\t    Mx Foil Light Blue\n"
        "211\t    Mx Foil Light Gray\n"
        "213\t    Mx Foil Light Green\n"
        "219\t    Mx Foil Orange\n"
        "217\t    Mx Foil Red\n"
        "216\t    Mx Foil Violet\n"
        "218\t    Mx Foil Yellow\n"
        "139\t    Mx Lemon\n"
        "124\t    Mx Light Bluish Gray\n"
        "125\t    Mx Light Gray\n"
        "136\t    Mx Light Orange\n"
        "137\t    Mx Light Yellow\n"
        "144\t    Mx Medium Blue\n"
        "138\t    Mx Ochre Yellow\n"
        "140\t    Mx Olive Green\n"
        "135\t    Mx Orange\n"
        "145\t    Mx Pastel Blue\n"
        "141\t    Mx Pastel Green\n"
        "148\t    Mx Pink\n"
        "130\t    Mx Pink Red\n"
        "129\t    Mx Red\n"
        "146\t    Mx Teal Blue\n"
        "134\t    Mx Terracotta\n"
        "143\t    Mx Tile Blue\n"
        "131\t    Mx Tile Brown\n"
        "127\t    Mx Tile Gray\n"
        "147\t    Mx Violet\n"
        "123\t    Mx White\n"
        "172\t    BA Black\n"
        "207\t    BA Black Rubber\n"
        "183\t    BA Blue\n"
        "199\t    BA Blue Chrome\n"
        "197\t    BA Brass\n"
        "198\t    BA Bronze\n"
        "177\t    BA Brown\n"
        "209\t    BA Chrome\n"
        "168\t    BA Cobalt\n"
        "190\t    BA Dark Blue\n"
        "178\t    BA Dark Brown\n"
        "175\t    BA Dark Gray\n"
        "204\t    BA Dark Gray Rubber\n"
        "179\t    BA Dark Tan\n"
        "208\t    BA Glow In Dark\n"
        "203\t    BA Gray Rubber\n"
        "182\t    BA Green\n"
        "170\t    BA Gunmetal\n"
        "205\t    BA Gunmetal Rubber\n"
        "174\t    BA Light Gray\n"
        "189\t    BA OD Green\n"
        "200\t    BA OD Metallic\n"
        "181\t    BA Olive\n"
        "192\t    BA Pink\n"
        "191\t    BA Purple\n"
        "176\t    BA Red\n"
        "201\t    BA Red Chrome\n"
        "184\t    BA Sand Blue\n"
        "195\t    BA Silver\n"
        "206\t    BA Silver Rubber\n"
        "180\t    BA Tan\n"
        "196\t    BA Titanium\n"
        "186\t    BA Trans Black\n"
        "188\t    BA Trans Blue\n"
        "185\t    BA Trans Clear\n"
        "171\t    BA Trans Green\n"
        "187\t    BA Trans Orange\n"
        "169\t    BA Trans Red\n"
        "194\t    BA Trans Red Sparkle\n"
        "193\t    BA Trans Smoke\n"
        "167\t    BA UN Blue\n"
        "173\t    BA White\n"
        "202\t    BA White Rubber\n"
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
# Space-delmited LDraw Design ID, Annotation Style and Part Category cross reference
#
# The Regular Expression used is: ^(\b[^=]+\b)=([1|2])\s+([1-6])?\s*([^\s]+).*$
#
# 1. Design ID:           LDraw Part Name                 (Required)
# 2. Annotation Style:    1 Square, 2 Circle, 3 Rectangle (Required)
# 3. Part Category:       LDraw Part category             (Required)
# 4. Annotation:          Annotation text                 (Optional - uses title annotation if not defined)
# 5. Part Description:    Description for reference only  (Optional - not loaded)
#
# ---------------------------------------
# |No |Annotation Style | Part Category |
# |---|---------------------------------|
# | 1 |circle(1)        |axle(1)        |
# | 2 |square(2)        |beam(2)        |
# | 3 |rectangle(3)     |cable(3)       |
# | 4 |square(2)        |connector(4)   |
# | 5 |square(2)        |hose(5)        |
# | 6 |square(2)        |panel(6)       |
# ---------------------------------------
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
        "\n"
        "32580.dat=1   1   7    Technic Axle Flexible  7\n"
        "32199.dat=1   1  11    Technic Axle Flexible 11\n"
        "32200.dat=1   1  12    Technic Axle Flexible 12\n"
        "32201.dat=1   1  14    Technic Axle Flexible 14\n"
        "32202.dat=1   1  16    Technic Axle Flexible 16\n"
        "32235.dat=1   1  19    Technic Axle Flexible 19\n"
        "\n"
        "18654.dat=2   2   1    Technic Beam  1\n"
        "43857.dat=2   2   2    Technic Beam  2\n"
        "32523.dat=2   2   3    Technic Beam  3\n"
        "32316.dat=2   2   5    Technic Beam  5\n"
        "32524.dat=2   2   7    Technic Beam  7\n"
        "40490.dat=2   2   9    Technic Beam  9\n"
        "32525.dat=2   2  11    Technic Beam 11\n"
        "41239.dat=2   2  13    Technic Beam 13\n"
        "32278.dat=2   2  15    Technic Beam 15\n"
        "\n"
        "11145.dat=3   3  25cm  Electric Mindstorms EV3 Cable 25 cm\n"
        "11146.dat=3   3  35cm  Electric Mindstorms EV3 Cable 35 cm\n"
        "11147.dat=3   3  50cm  Electric Mindstorms EV3 Cable 50 cm\n"
        "\n"
        "55804.dat=3   3  20cm  Electric Mindstorms NXT Cable 20 cm\n"
        "55805.dat=3   3  35cm  Electric Mindstorms NXT Cable 35 cm\n"
        "55806.dat=3   3  50cm  Electric Mindstorms NXT Cable 50 cm\n"
        "\n"
        "32013.dat=2   4   1    Technic Angle Connector #1\n"
        "32034.dat=2   4   2    Technic Angle Connector #2 (180 degree)\n"
        "32016.dat=2   4   3    Technic Angle Connector #3 (157.5 degree)\n"
        "32192.dat=2   4   4    Technic Angle Connector #4 (135 degree)\n"
        "32015.dat=2   4   5    Technic Angle Connector #5 (112.5 degree)\n"
        "32014.dat=2   4   6    Technic Angle Connector #6 (90 degree)\n"
        "\n"
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
        "\n"
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
        "\n"
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
        "\n"
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
        "\n"
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
        "\n"
        "17141.dat=2   2   3    =Technic Beam  3\n"
        "16615.dat=2   2   7    =Technic Beam  7\n"
        "64289.dat=2   2   9    =Technic Beam  9\n"
        "64290.dat=2   2  11    =Technic Beam 11\n"
        "64871.dat=2   2  15    =Technic Beam 15\n"
        "\n"
        "4211550.dat=2 4   1    _Technic Angle Connector #1 Light_Bluish_Gray\n"
        "4506697.dat=2 4   5    _Technic Angle Connector #5 (112.5 degree) White\n"
        "4107767.dat=2 4   6    _Technic Angle Connector #6 (90 degree) Black\n"
        "\n"
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

void Annotations::loadLD2RBColorsXRef(QByteArray& Buffer){
/*
# File: ld2rbcolorsxref.lst
#
# Tab-delmited LDConfig and Rebrickable Color code cross reference
#
# The Regular Expression used is: ^([^\t]+)\t+\s*([^\t]+).*$
#
# 1. LDConfig Color ID: LDraw Color ID             (Required)
# 2. Color ID:          Rebrickable Color ID        (Required)
#
*/
    const char LEGOLD2RBColorsXRef[] = {
        "38\t  57\n"
        "39\t  43\n"
        "41\t  143\n"
        "43\t  41\n"
        "44\t  236\n"
        "45\t  230\n"
        "57\t  182\n"
        "326\t  158\n"
        "329\t  1000\n"
        "330\t  326\n"
        "339\t  1002\n"
        "87\t  1040\n"
    };

    const char LD2RBColorsXRef[] = {
        "no colors cross-references defined\n"
    };

    if (Preferences::validLDrawLibrary == LEGO_LIBRARY)
        Buffer.append(LEGOLD2RBColorsXRef, sizeof(LEGOLD2RBColorsXRef));
    else
        Buffer.append(LD2RBColorsXRef, sizeof(LD2RBColorsXRef));
}

void Annotations::loadLD2RBCodesXRef(QByteArray& Buffer){
/*
# File: ld2rbcodesxref.lst
#
# Tab-delmited LDraw Design ID and Rebrickable Part ID cross reference
#
# The Regular Expression used is: ^([^\t]+)\t+\s*([^\t]+).*$
#
# 1. Design ID:            LDraw Part Number            (Required)
# 2. Part ID:              Rebrickable Part Number      (Required)
#
*/
    const char LEGOLD2RBCodesXRef[] = {
        "00000\t     00000\n"
    };

    const char LD2RBCodesXRef[] = {
        "no code cross-references defined\n"
    };

    if (Preferences::validLDrawLibrary == LEGO_LIBRARY)
        Buffer.append(LEGOLD2RBCodesXRef, sizeof(LEGOLD2RBCodesXRef));
    else
        Buffer.append(LD2RBCodesXRef, sizeof(LD2RBCodesXRef));
}

void Annotations::loadTitleAnnotations(QByteArray &Buffer) {
/*
# File: titleAnnotations.lst
#
# This space-delimited list captures part category and annotation Regular Expression.
#
# The Regular Expression used is: ^(\\b.*[^\\s]\\b:)\\s+([\\(|\\^].*)$
#
*/
    const char LEGOLDefaultTitleAnnotations[] = {
        "Technic:    ^Technic Axle\\s+(\\d+\\.*\\d*)\\s*.*$\n"
        "Technic:    ^Technic Axle Flexible\\s+(\\d+)\\s*.*$\n"
        "Technic:    ^Technic Angle Connector\\s+\\(*(#*\\d*\\s*x*\\s*\\d*)\\s*\\w*\\)*\\s*.*$\n"
        "Technic:    ^Technic Arm\\s+(\\d+\\s+(x|X)\\s+\\d+)\\s*.*$\n"
        "Technic:    (?i)^(?!.*(?:Bent))Technic Beam*\\s+(\\d+\\.*\\d*\\s*(x*|X*)\\s*\\d*\\.*\\d*\\s*(x*|X*)\\s*\\d*\\.*\\d*)\\s*.*$\n"
        "Technic:    ^Technic Brick\\s+(\\d+\\s+(x|X)\\s+\\d+\\s*(x*|X*)\\s*\\d*\\.*\\d*)\\s*.*$\n"
        "Technic:    ^Technic Chain Tread\\s+(\\d+)$\n"
        "Technic:    ^Technic Connector\\s*\\w*\\s+(\\d+\\s+(x|X)\\s+\\d+\\s*(x*|X*)\\s*\\d*\\.*\\d*)\\s*.*$\n"
        "Technic:    ^Technic Cross Block\\s+(\\d+\\s+(x|X)\\s+\\d+\\s*(x*|X*)\\s*\\d*\\.*\\d*)\\s*.*$\n"
        "Technic:    ^Technic Excavator Bucket\\s+(\\d+\\s+(x|X)\\s+\\d+\\s*(x*|X*)\\s*\\d*)\\s*.*$\n"
        "Technic:    ^Technic Flex-System\\s*\\w*\\s+(\\d+L).*$\n"
        "Technic:    ^Technic Gear\\s*\\w*\\s*(\\d+\\s*(x*|X*)\\s*\\d*\\s*(x*|X*)\\s*\\d*\\.*\\d*)\\s*.*$\n"
        "Technic:    ^Technic\\s+\\w*\\s*Link\\s*(\\d+L*\\s*(x*|X*)\\s*\\d*)\\s*.*$\n"
        "Technic:    ^Technic Motor Pull Back\\s+(\\d+\\s*(x*|X*)\\s*\\d*\\s*(x*|X*)\\s*\\d*)\\s*.*$\n"
        "Technic:    ^Technic Panel\\s*\\w*\\s+(\\d+\\s*(x*|X*)\\s*\\d*\\s*(x*|X*)\\s*\\d*)$\n"
        "Technic:    ^Technic Panel Fairing\\s*\\w*\\s+(#+\\d+)\\s*.*$\n"
        "Technic:    ^Technic Panel Flat Sheet\\s*(\\d+\\s*(x*|X*)\\s*\\d*)\\s*.*$\n"
        "Technic:    ^Technic Plate\\s*(\\d+\\s*(x*|X*)\\s*\\d*)\\s*.*$\n"
        "Technic:    ^Technic Pneumatic\\s*\\w*\\s+(\\d+\\.*\\d*\\s*(x*|X*)\\s*\\d*\\s*\\d*L*\\s*(x*|X*)\\s*\\d*\\.*\\d*L*)\\s*.*$\n"
        "Technic:    ^Technic Rotor\\s+(\\d+\\s*B)\\w*\\s*(\\d*L*\\s*D*)\\s*.*$\n"
        "Technic:    ^Technic Shock Absorber\\s+(\\d+\\.*\\d*L*)\\s*.*$\n"
        "Technic:    ^Technic Sprocket Wheel\\s+(\\d+\\.*\\d*)\\s*.*$\n"
        "Technic:    ^Technic Suspension Arm\\s+(\\d+\\s+(x|X)\\s+\\d+\\s*(x*|X*)\\s*\\d*\\.*\\d*)\\s*.*$\n"
        "Technic:    ^Technic Universal Joint\\s+(\\d+\\.*\\d*L*)\\s*.*$\n"
        "\n"
        "Antenna:    ^Antenna\\s+(\\dH)\\s*.*$\n"
        "Arch:       ^Arch\\s+(\\d+\\s+(x|X)\\s+\\d+\\s*(x*|X*)\\s*\\d*\\.*\\d*)\\s*.*$\n"
        "Bar:        ^Bar\\s+(\\d+\\.*\\d*L*\\s*(x*|X*)\\s*\\d*\\s*(x*|X*)\\s*\\d*)\\s*.*$\n"
        "Barrel:     ^Barrel\\s+(\\d+\\.*\\d*\\s*(x*|X*)\\s*\\d*\\.*\\d*\\s*(x*|X*)\\s*\\d*\\.*\\d*)\\s*.*$\n"
        "Baseplate:  ^Baseplate\\s+(\\d+\\s*(x*|X*)\\s*\\d*+\\s*)\\s*.*$\n"
        "Belt:       ^Rubber Belt Round\\s+(\\d+\\s*\\/+\\s*\\d*\\.*\\d*)\\s*.*$\n"
        "Boat:       ^Boat\\s*\\w*\\s*\\w*\\s+(\\d+\\s+(x|X)\\s+\\d+\\s*(x*|X*)\\s*\\d*\\.*\\d*)\\s*.*$\n"
        "Bone:       ^Bone\\s+(\\dL)$\n"
        "Bracket:    ^Bracket\\s+(\\d+\\s+(x|X)\\s+\\d+\\s*(x*|X*)-*\\s*\\d*\\/*\\.*\\d*\\s*(x*|X*)\\s*\\d*)\\s*.*$\n"
        "Brick:      ^Brick\\s+(\\d+\\s+(x|X)\\s+\\d+\\s*(x*|X*)\\s*\\d*\\.*\\d*)\\s*.*$\n"
        "Brick:      ^Mursten Brick\\s+(\\d+\\s+(x|X)\\s+\\d+)\\s*.*$\n"
        "Brick:      ^Quatro Brick\\s+(\\d+\\s+(x|X)\\s+\\d+)\\s*.*$\n"
        "Bucket:     ^Bucket\\s+(\\d+\\s+(x|X)\\s+\\d+\\s*(x*|X*)\\s*\\d*)\\s*.*$\n"
        "Car:        ^Car\\s*\\w*\\s+(\\d+\\s+(x|X)\\s+\\d+\\.*\\d*\\s*(x*|X*)\\s*\\d*\\/*\\.*\\d*)\\s*.*$\n"
        "Claw:       ^Claw\\s+(\\dL)$\n"
        "Cockpit:    ^Cockpit\\s+(\\d+\\s+(x|X)\\s+\\d+\\s*(x*|X*)\\s*\\d*\\.*\\d*)\\s*.*$\n"
        "Cocoon:     ^Cocoon\\s+(\\d+\\s+(x|X)\\s+\\d+\\s*(x*|X*)\\s*\\d*\\.*\\d*)\\s*.*$\n"
        "Cone:       ^Cocoon\\s+(\\d+\\s+(x|X)\\s+\\d+\\s*(x*|X*)\\s*\\d*\\.*\\d*)\\s*.*$\n"
        "Container:  ^Container\\s*\\w*\\s+(\\d+\\.*\\d*\\s+(x|X)\\s+\\d+\\.*\\d*\\s*(x*|X*)\\s*\\d*\\.*\\d*)\\s*.*$\n"
        "Cylinder:   ^Cylinder\\s*\\w*\\s+(\\d+\\s+(x|X)\\s+\\d+\\s*(x*|X*)\\s*\\d*\\.*\\d*)\\s*.*$\n"
        "Dish:       ^Dish\\s+(\\d+\\s+(x|X)\\s+\\d+)\\s*.*$\n"
        "Door:       ^Door\\s*\\w*\\s+(\\d+\\s+(x|X)\\s+\\d+\\.*\\d*\\s*(x*|X*)\\s*\\d*\\.*\\d*)\\s*.*$\n"
        "Electric:   ^Electric\\s*9V\\s*\\w*\\s*\\w*\\s*(\\d*\\s*(x*|X*)\\s*\\d*\\s*(x*|X*)\\s*\\d*\\.*\\d*)\\s*.*$\n"
        "Electric:   ^Electric\\s*4\\.5V\\s*\\w*\\s*\\w*\\s*(\\d*\\s*(x*|X*)\\s*\\d*\\s*(x*|X*)\\s*\\d*\\.*\\d*)\\s*.*$\n"
        "Electric:   ^Electric Brick\\s*(\\d*\\s*(x*|X*)\\s*\\d*\\s*(x*|X*)\\s*\\d*\\.*\\d*)\\s*.*$\n"
        "Electric:   ^Electric Lightbrick\\s*(\\d*\\s*(x*|X*)\\s*\\d*\\s*(x*|X*)\\s*\\d*\\.*\\d*)\\s*.*$\n"
        "Electric:   ^Electric Sound Brick\\s*(\\d*\\s*(x*|X*)\\s*\\d*\\s*(x*|X*)\\s*\\d*\\.*\\d*)\\s*.*$\n"
        "Electric:   ^Electric Light & Sound\\s*\\w*\\s*(\\d*\\s*(x*|X*)\\s*\\d*\\s*(x*|X*)\\s*\\d*\\.*\\d*)\\s*.*$\n"
        "Electric:   ^Electric Light Prism\\s*(\\d*\\s*(x*|X*)\\s*\\d*\\s*(x*|X*)\\s*\\d*\\.*\\d*)\\s*.*$\n"
        "Electric:   ^Electric Mindstorms EV3 Cable\\s+([0-9].*)$\n"
        "Electric:   ^Electric Mindstorms NXT Cable\\s+([0-9].*)$\n"
        "Electric:   ^Electric Plate\\s+(\\d*\\s*(x*|X*)\\s*\\d*)\\s*.*$\n"
        "Electric:   ^Electric Train Light Prism\\s*(\\d*\\s*(x*|X*)\\s*\\d*)\\s*.*$\n"
        "Flag:       ^Flag\\s+(\\d+\\s+(x|X)\\s+\\d+)\\s*.*$\n"
        "Gate:       ^Gate\\s+(\\d+\\s*(x*|X*)\\s*\\d*\\s*(x*|X*)\\s*\\d*\\.*\\d*)\\s*.*$\n"
        "Glass:      ^Glass for Window\\s+(\\d+\\s+(x|X)\\s+\\d+\\s*(x*|X*)\\s*\\d*\\.*\\d*)\\s*.*$\n"
        "Hinge:      ^Hinge\\s+(\\d+\\s+(x|X)\\s+\\d+)\\s*.*$\n"
        "Hinge:      ^Hinge Car Roof\\s+(\\d+\\s+(x|X)\\s+\\d+)\\s*.*$\n"
        "Hinge:      ^Hinge Panel\\s+(\\d+\\s+(x|X)\\s+\\d+\\s*(x*|X*)\\s*\\d*\\.*\\d*)\\s*.*$\n"
        "Hinge:      ^Hinge\\s*\\w*\\s+(\\d+\\s+(x|X)\\s*\\d*\\.*\\d*)\\s*.*$\n"
        "Homemaker:  ^Homemaker\\s*\\w*\\s*\\w*\\s+(\\d+\\s*(x*|X*)\\s*\\d*\\s*(x*|X*)\\s*\\d*\\.*\\d*)\\s*.*$\n"
        "Hose:       ^Hose\\s*\\w*\\s*\\w*\\s+(\\d+\\.*\\d*L*\\s*(x*|X*)\\s*\\d*\\s*(x*|X*)\\s*\\d*)\\s*.*$\n"
        "Minifig:    ^Minifig Flame\\s+(\\d\\s*L)\\s*.*$\n"
        "MInifig:    ^Minifig Surf Board\\s+(\\d+\\s+(x|X)\\s*\\d*\\.*\\d*)\\s*.*$\n"
        "Motor:      ^Motor\\s*\\w*-*\\s*\\w*\\s+(\\d+\\s*(x*|X*)\\s*\\d*\\s*(x*|X*)\\s*\\d*\\.*\\d*)\\s*.*$\n"
        "Misc:       ^Mursten\\s*\\w*\\s+(\\d+\\s*(x*|X*)\\s*\\d*\\s*(x*|X*)\\s*\\d*\\.*\\d*)\\s*.*$\n"
        "Misc:       ^Rack Winder\\s+(\\d+\\s*(x*|X*)\\s*\\d*\\s*(x*|X*)\\s*\\d*\\.*\\d*)\\s*.*$\n"
        "Misc:       ^Roadsign Clip-on\\s+(\\d+\\.*\\d*\\s*(x*|X*)\\s*\\d*\\.*\\d*)\\s*.*$\n"
        "Misc:       ^Rock\\s+(\\d+\\s*(x*|X*)\\s*\\d*\\s*(x*|X*)\\s*\\d*\\.*\\d*)\\s*.*$\n"
        "Misc:       ^Tap\\s+(\\d+\\s+(x|X)\\s+\\d+)\\s*.*$\n"
        "Panel:      ^Panel\\s*\\w*\\s+(\\d+\\s*(x*|X*)\\s*\\d*\\s*(x*|X*)\\s*\\d*\\.*\\d*)\\s*.*$\n"
        "Plate:      ^Plate\\s+(\\d+\\s*(x*|X*)\\s*\\d*\\s*(x*|X*)\\s*\\d*\\.*\\d*)\\s*.*$\n"
        "Propellor:  ^Propellor\\s+(\\d\\s+B)\\w+\\s+(\\d+\\.*\\d*\\s+D)\\s*.*$\n"
        "Roof:       ^Roof\\s*\\w*\\s+(\\d+\\s*(x*|X*)\\s*\\d*\\s*(x*|X*)\\s*\\d*\\.*\\d*)\\s*.*$\n"
        "Slope:      ^Slope Brick\\s+(\\d*\\/*\\d*\\s*\\d+\\s*(x*|X*)\\s*\\d*\\s*(x*|X*)\\s*\\d*\\.*\\d*)\\s*.*$\n"
        "Slope:      ^Slope Brick Curved\\s+(\\d+\\s*(x*|X*)\\s*\\d*\\s*(x*|X*)\\s*\\d*\\.*\\d*)\\s*.*$\n"
        "Tail:       ^Tail\\s*\\w*\\s+(\\d+\\s*(x*|X*)\\s*\\d*\\s*(x*|X*)\\s*\\d*\\.*\\d*)\\s*.*$\n"
        "Tile:       ^Tile\\s+(\\d+\\s*(x*|X*)\\s*\\d*\\s*(x*|X*)\\s*\\d*\\.*\\d*)\\s*.*$\n"
        "Train:      ^Train\\s*\\w*\\s+(\\d+V*\\s*(x*|X*)\\s*\\d*\\s*(x*|X*)\\s*\\d*\\.*\\d*)\\s*.*$\n"
        "Turntable:  ^Turntable\\s+(\\d+\\s*(x*|X*)\\s*\\d*\\s*(x*|X*)\\s*\\d*\\.*\\d*)\\s*.*$\n"
        "Wheel:      ^Tyre\\s+(\\d+\\.*\\d*\\/*d*(x*|X*)\\s*\\d*\\s*(x*|X*)\\s*\\d*\\.*\\d*)\\s*.*$\n"
        "Wedge:      ^Wedge\\s*\\w*\\s+(\\d+\\s*(x*|X*)\\s*\\d*\\s*(x*|X*)\\s*\\d*\\.*\\d*)\\s*.*$\n"
        "Wheel:      ^Wheel\\s*\\w*\\s+(\\d+\\.*\\d*\\s*(x*|X*)\\s*\\d*\\s*(x*|X*)\\s*\\d*\\.*\\d*)\\s*.*$\n"
        "Winch:      ^Winch\\s+(\\d+\\s*(x*|X*)\\s*\\d*\\s*(x*|X*)\\s*\\d*)\\s*.*$\n"
        "Window:     ^Window\\s+(\\d+\\s*(x*|X*)\\s*\\d*\\s*(x*|X*)\\s*\\d*)\\s*.*$\n"
        "Windscreen: ^Windscreen\\s+(\\d+\\s*(x*|X*)\\s*\\d*\\s*(x*|X*)\\s*\\d*)\\s*.*$\n"
        "Wing:       ^Wing\\s+(\\d+\\s+(x|X)\\s+\\d+\\s*(x*|X*)-*\\s*\\d*\\/*\\.*\\d*\\s*(x*|X*)\\s*\\d*\\.*\\d*)\\s*.*$\n"
        "\n"
        "VEX:        ^VEX Beam  1 x\\s+(\\d+)\\s*$\n"
        "VEX:        ^VEX Beam  2 x\\s+(\\d+)\\s*$\n"
        "VEX:        ^VEX Pin Standoff\\s+([0-9]*\\.?[0-9]*)\\sM$\n"
        "VEX:        ^VEX Beam(?:\\s)(?:(?!Double Bent).)*(?!90)(\\d\\d)$\n"
        "VEX:        ^VEX Plate  4 x\\s+(\\d+)\\s*$\n"
        "VEX:        ^VEX Axle\\s+(\\d+)\\s*.*$\n"
        "VEX:        ^VEX-2 Smart Cable\\s+([0-9].*)$\n"
        "#VEX:       ^VEX-2 Rubber Belt\\s+([0-9].*)Diameter\n"
        "\n"
        "Bitbeam:    ^Bitbeam Beam  1 x\\s+(\\d+)\\s*$\n"
        "Bitbeam:    ^Bitbeam Beam  2 x\\s+(\\d+)\\s*$\n"
        "Bitbeam:    ^Bitbeam Beam  8 x\\s+(\\d+)\\s*$\n"
        "Bitbeam:    ^Bitbeam Beam 10 x\\s+(\\d+)\\s*$\n"
        "Bitbeam:    ^Bitbeam Beam 12 x\\s+(\\d+)\\s*$\n"
        "Bitbeam:    ^Bitbeam Beam 16 x\\s+(\\d+)\\s*$\n"
        "Bitbeam:    ^Bitbeam Beam 20 x\\s+(\\d+)\\s*$\n"
        "Bitbeam:    ^Bitbeam Beam L90\\s+([0-9].*)$\n"
        "Bitbeam:    ^Bitbeam Beam L45\\s+([0-9].*)$\n"
        "Bitbeam:    ^Bitbeam Beam T\\s+([0-9].*)$\n"
        "Bitbeam:    ^Bitbeam Beam X\\s+([0-9].*)$\n"
        "Bitbeam:    ^Bitbeam Axle\\s+([0-9].*)$\n"
        "Bitbeam:    ^Bitbeam Wheel Tire\\s+([0-9].*)$\n"
        "Bitbeam:    ^Bitbeam Wheel\\s+([0-9].*)$\n"
        "Bitbeam:    ^Bitbeam Screw Nut\\s+(.[0-9].*)$\n"
        "Bitbeam:    ^Bitbeam Screw Pad\\s+(.[0-9].*)$\n"
        "Bitbeam:    ^Bitbeam Screw\\s+(.[0-9].*)$\n"
        "Bitbeam:    ^Bitbeam Case for Breadboard\\s+(.[0-9].*)$\n"
    };
    const char TENTEDefaultTitleAnnotations[] = {
        "no default title annotation defined\n"
    };

    const char VEXIQDefaultTitleAnnotations[] = {
        "no default title annotation defined1\n"
    };

    if (Preferences::validLDrawLibrary == LEGO_LIBRARY)
        Buffer.append(LEGOLDefaultTitleAnnotations, sizeof(LEGOLDefaultTitleAnnotations));
    else
    if (Preferences::validLDrawLibrary == TENTE_LIBRARY)
        Buffer.append(TENTEDefaultTitleAnnotations, sizeof(TENTEDefaultTitleAnnotations));
    else
    if (Preferences::validLDrawLibrary == VEXIQ_LIBRARY)
        Buffer.append(VEXIQDefaultTitleAnnotations, sizeof(VEXIQDefaultTitleAnnotations));
}

Annotations::Annotations()
{
    returnString = QString();
    bool rxFound = false;
    QString message;
    if (titleAnnotations.size() == 0) {
        QString annotations = Preferences::titleAnnotationsFile;
        QRegExp rx("^(\\b.*[^\\s]\\b:)\\s+([\\(|\\^].*)$");
        if (!annotations.isEmpty()) {
            QFile file(annotations);
            if ( ! file.open(QFile::ReadOnly | QFile::Text)) {
                message = QString("Failed to open Title Annotations file: %1:<br>%2")
                                  .arg(annotations)
                                  .arg(file.errorString());
                if (Preferences::modeGUI){
                    QMessageBox::warning(nullptr,QMessageBox::tr(VER_PRODUCTNAME_STR),message);
                } else {
                    logError() << message.replace("<br>"," ");
                }
                return;
            }
            QTextStream in(&file);

            // Load RegExp from file;
            QRegExp rxin("^#\\sThe\\sRegular\\sExpression\\sused\\sis\\:[\\s](\\^.*)$");
            while ( ! in.atEnd()) {
                QString sLine = in.readLine(0);
                if ((rxFound = sLine.contains(rxin))) {
                    rx.setPattern(rxin.cap(1));
                    //logDebug() << "TitleAnnotations RegExp Pattern: " << rxin.cap(1);
                    break;
                }
            }

            if (rxFound) {
                in.seek(0);

                // Load input values
                while ( ! in.atEnd()) {
                    QString sLine = in.readLine(0);
                    if (sLine.contains(rx)) {
                        QString annotation = rx.cap(2);
                        titleAnnotations << annotation;
                    }
                }
            } else {
                message = QString("Regular expression pattern was not found in %1.<br>"
                                  "Be sure the following lines exist in the file header:<br>"
                                  "# File: %1<br>"
                                  "# The Regular Expression used is: ^(\\b.*[^\\s]\\b:)\\s+([\\(|\\^].*)$")
                                  .arg(QFileInfo(annotations).fileName());
                if (Preferences::modeGUI){
                    QMessageBox::warning(nullptr,QMessageBox::tr(VER_PRODUCTNAME_STR),message);
                } else {
                    logError() << message.replace("<br>"," ");
                }
            }
        } else {
            titleAnnotations.clear();
            QByteArray Buffer;
            loadTitleAnnotations(Buffer);
            QTextStream instream(Buffer);
            for (QString sLine = instream.readLine(); !sLine.isNull(); sLine = instream.readLine())
            {
                QChar comment = sLine.at(0);
                if (comment == '#' || comment == ' ')
                    continue;
                if (sLine.contains(rx)) {
                    QString annotation = rx.cap(2);
                    titleAnnotations << annotation;
                }
            }
        }
    }

    if (freeformAnnotations.size() == 0) {
        QString annotations = Preferences::freeformAnnotationsFile;
        QFile file(annotations);
        if ( ! file.open(QFile::ReadOnly | QFile::Text)) {
            QString message = QString("Failed to open Freeform Annotations file: %1:<br>%2")
                                      .arg(annotations)
                                      .arg(file.errorString());
            if (Preferences::modeGUI){
                QMessageBox::warning(nullptr,QMessageBox::tr(VER_PRODUCTNAME_STR),message);
            } else {
                logError() << message.replace("<br>"," ");
            }
            return;
        }
        QTextStream in(&file);

        // Load RegExp from file;
        QRegExp rx("^(\\b.*[^\\s]\\b)(?:\\s)\\s+(.*)$");
        QRegExp rxin("^#\\sThe\\sRegular\\sExpression\\sused\\sis\\:[\\s](\\^.*)$");
        while ( ! in.atEnd()) {
            QString sLine = in.readLine(0);
            if ((rxFound = sLine.contains(rxin))) {
                rx.setPattern(rxin.cap(1));
                //logDebug() << "FreeFormAnnotations RegExp Pattern: " << rxin.cap(1);
                break;
            }
        }

        if (rxFound) {
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
        } else {
            message = QString("Regular expression pattern was not found in %1.<br>"
                              "Be sure the following lines exist in the file header:<br>"
                              "# File: %1<br>"
                              "# The Regular Expression used is: ^(\\b.*[^\\s]\\b)(?:\\s)\\s+(.*)$")
                              .arg(QFileInfo(annotations).fileName());
            if (Preferences::modeGUI){
                QMessageBox::warning(nullptr,QMessageBox::tr(VER_PRODUCTNAME_STR),message);
            } else {
                logError() << message.replace("<br>"," ");
            }
        }
    }

    if (annotationStyles.size() == 0) {
        QString styleFile = Preferences::annotationStyleFile;
        QRegExp rx("^(\\b[^=]+\\b)=([1|2])\\s+([1-6])?\\s*([^\\s]+).*$");
        if (!styleFile.isEmpty()) {
            QFile file(styleFile);
            if ( ! file.open(QFile::ReadOnly | QFile::Text)) {
                QString message = QString("Failed to open Annotation style file: %1:<br>%2")
                                          .arg(styleFile)
                                          .arg(file.errorString());
                if (Preferences::modeGUI){
                    QMessageBox::warning(nullptr,QMessageBox::tr(VER_PRODUCTNAME_STR),message);
                } else {
                    logError() << message.replace("<br>"," ");
                }
                return;
            }
            QTextStream in(&file);

            // Load RegExp from file;
            QRegExp rxin("^#\\sThe\\sRegular\\sExpression\\sused\\sis\\:[\\s](\\^.*)$");
            while ( ! in.atEnd()) {
                QString sLine = in.readLine(0);
                if ((rxFound = sLine.contains(rxin))) {
                    rx.setPattern(rxin.cap(1));
//                    logDebug() << "AnnotationStyle RegExp Pattern: " << rxin.cap(1);
                    break;
                }
            }

            if (rxFound) {
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
                message = QString("Regular expression pattern was not found in %1.<br>"
                                  "Regenerate %1 by renaming the existing file and<br>"
                                  "select edit %1 from the configuration menu.")
                                  .arg(QFileInfo(styleFile).fileName());
                if (Preferences::modeGUI){
                    QMessageBox::warning(nullptr,QMessageBox::tr(VER_PRODUCTNAME_STR),message);
                } else {
                    logError() << message.replace("<br>"," ");
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
                QString message = QString("Failed to open BrickLink colors.txt file: %1:<br>%2")
                                          .arg(blColorsFile)
                                          .arg(file.errorString());
                if (Preferences::modeGUI){
                    QMessageBox::warning(nullptr,QMessageBox::tr(VER_PRODUCTNAME_STR),message);
                } else {
                    logError() << message.replace("<br>"," ");
                }
                return;
            }
            QTextStream in(&file);

            // Load RegExp from file;
            QRegExp rxin("^#\\sThe\\sRegular\\sExpression\\sused\\sis\\:[\\s](\\^.*)$");
            while ( ! in.atEnd()) {
                QString sLine = in.readLine(0);
                if ((rxFound = sLine.contains(rxin))) {
                    rx.setPattern(rxin.cap(1));
//                    logDebug() << "Bricklink Colors RegExp Pattern: " << rxin.cap(1);
                    break;
                }
            }

           if (rxFound) {
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
               message = QString("Regular expression pattern was not found in %1.<br>"
                                 "Regenerate %1 by renaming the existing file and<br>"
                                 "select edit %1 from the configuration menu.")
                                 .arg(QFileInfo(blColorsFile).fileName());
               if (Preferences::modeGUI){
                   QMessageBox::warning(nullptr,QMessageBox::tr(VER_PRODUCTNAME_STR),message);
               } else {
                   logError() << message.replace("<br>"," ");
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
                QString message = QString("Failed to open ld2blcolorsxref.lst file: %1:<br>%2")
                                          .arg(ld2blColorsXRefFile)
                                          .arg(file.errorString());
                if (Preferences::modeGUI){
                    QMessageBox::warning(nullptr,QMessageBox::tr(VER_PRODUCTNAME_STR),message);
                } else {
                    logError() << message.replace("<br>"," ");
                }
                return;
            }
            QTextStream in(&file);

            // Load RegExp from file;
            QRegExp rxin("^#\\sThe\\sRegular\\sExpression\\sused\\sis\\:[\\s](\\^.*)$");
            while ( ! in.atEnd()) {
                QString sLine = in.readLine(0);
                if ((rxFound = sLine.contains(rxin))) {
                    rx.setPattern(rxin.cap(1));
//                    logDebug() << "LD2BL ColorsXRef RegExp Pattern: " << rxin.cap(1);
                    break;
                }
            }

            if (rxFound) {
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
                message = QString("Regular expression pattern was not found in %1.<br>"
                                  "Regenerate %1 by renaming the existing file and<br>"
                                  "select edit %1 from the configuration menu.")
                                  .arg(QFileInfo(ld2blColorsXRefFile).fileName());
                if (Preferences::modeGUI){
                    QMessageBox::warning(nullptr,QMessageBox::tr(VER_PRODUCTNAME_STR),message);
                } else {
                    logError() << message.replace("<br>"," ");
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
                QString message = QString("Failed to open ld2blcodesxref.lst file: %1:<br>%2")
                                          .arg(ld2blCodesXRefFile)
                                          .arg(file.errorString());
                if (Preferences::modeGUI){
                    QMessageBox::warning(nullptr,QMessageBox::tr(VER_PRODUCTNAME_STR),message);
                } else {
                    logError() << message.replace("<br>"," ");
                }
                return;
            }
            QTextStream in(&file);

            // Load RegExp from file;
            QRegExp rxin("^#\\sThe\\sRegular\\sExpression\\sused\\sis\\:[\\s](\\^.*)$");
            while ( ! in.atEnd()) {
                QString sLine = in.readLine(0);
                if ((rxFound = sLine.contains(rxin))) {
                    rx.setPattern(rxin.cap(1));
//                    logDebug() << "LD2BL CodesXRef RegExp Pattern: " << rxin.cap(1);
                    break;
                }
            }

            if (rxFound) {
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
                message = QString("Regular expression pattern was not found in %1.<br>"
                                  "Regenerate %1 by renaming the existing file and<br>"
                                  "select edit %1 from the configuration menu.")
                                  .arg(QFileInfo(ld2blCodesXRefFile).fileName());
                if (Preferences::modeGUI){
                    QMessageBox::warning(nullptr,QMessageBox::tr(VER_PRODUCTNAME_STR),message);
                } else {
                    logError() << message.replace("<br>"," ");
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

    // Rebrickable Codes

    if (ld2rbColorsXRef.size() == 0) {
        QString ld2rbColorsXRefFile = Preferences::ld2rbColorsXRefFile;
        QRegExp rx("^([^\\t]+)\\t+\\s*([^\\t]+).*$");
        if (!ld2rbColorsXRefFile.isEmpty()) {
            QFile file(ld2rbColorsXRefFile);
            if ( ! file.open(QFile::ReadOnly | QFile::Text)) {
                QString message = QString("Failed to open ld2rbcolorsxref.lst file: %1:<br>%2")
                                          .arg(ld2rbColorsXRefFile)
                                          .arg(file.errorString());
                if (Preferences::modeGUI){
                    QMessageBox::warning(nullptr,QMessageBox::tr(VER_PRODUCTNAME_STR),message);
                } else {
                    logError() << message.replace("<br>"," ");
                }
                return;
            }
            QTextStream in(&file);

            // Load RegExp from file;
            QRegExp rxin("^#\\sThe\\sRegular\\sExpression\\sused\\sis\\:[\\s](\\^.*)$");
            while ( ! in.atEnd()) {
                QString sLine = in.readLine(0);
                if ((rxFound = sLine.contains(rxin))) {
                    rx.setPattern(rxin.cap(1));
//                    logDebug() << "LD2RB ColorsXRef RegExp Pattern: " << rxin.cap(1);
                    break;
                }
            }

            if (rxFound) {
                in.seek(0);

                // Load input values
                while ( ! in.atEnd()) {
                    QString sLine = in.readLine(0);
                    if (sLine.contains(rx)) {
                        QString ldcolorid = rx.cap(1);
                        QString rbcolorid = rx.cap(2).trimmed();
                        ld2rbColorsXRef[ldcolorid.toLower()] = rbcolorid;
                    }
                }
            } else {
                message = QString("Regular expression pattern was not found in %1.<br>"
                                  "Regenerate %1 by renaming the existing file and<br>"
                                  "select edit %1 from the configuration menu.")
                                  .arg(QFileInfo(ld2rbColorsXRefFile).fileName());
                if (Preferences::modeGUI){
                    QMessageBox::warning(nullptr,QMessageBox::tr(VER_PRODUCTNAME_STR),message);
                } else {
                    logError() << message.replace("<br>"," ");
                }
            }
        } else {
            ld2rbColorsXRef.clear();
            QByteArray Buffer;
            loadLD2RBColorsXRef(Buffer);
            QTextStream instream(Buffer);
            for (QString sLine = instream.readLine(); !sLine.isNull(); sLine = instream.readLine())
            {
                QChar comment = sLine.at(0);
                if (comment == '#' || comment == ' ')
                    continue;
                if (sLine.contains(rx)) {
                    QString ldcolorid = rx.cap(1);
                    QString rbcolorid = rx.cap(2).trimmed();
                    ld2rbColorsXRef[ldcolorid.toLower()] = rbcolorid;
                }
            }
        }
    }

    if (ld2rbCodesXRef.size() == 0) {
        QString ld2rbCodesXRefFile = Preferences::ld2rbCodesXRefFile;
        QRegExp rx("^([^\\t]+)\\t+\\s*([^\\t]+).*$");
        if (!ld2rbCodesXRefFile.isEmpty()) {
            QFile file(ld2rbCodesXRefFile);
            if ( ! file.open(QFile::ReadOnly | QFile::Text)) {
                QString message = QString("Failed to open ld2rbcodesxref.lst file: %1:<br>%2")
                                          .arg(ld2rbCodesXRefFile)
                                          .arg(file.errorString());
                if (Preferences::modeGUI){
                    QMessageBox::warning(nullptr,QMessageBox::tr(VER_PRODUCTNAME_STR),message);
                } else {
                    logError() << message.replace("<br>"," ");
                }
                return;
            }
            QTextStream in(&file);

            // Load RegExp from file;
            QRegExp rxin("^#\\sThe\\sRegular\\sExpression\\sused\\sis\\:[\\s](\\^.*)$");
            while ( ! in.atEnd()) {
                QString sLine = in.readLine(0);
                if ((rxFound = sLine.contains(rxin))) {
                    rx.setPattern(rxin.cap(1));
//                    logDebug() << "LD2RB CodesXRef RegExp Pattern: " << rxin.cap(1);
                    break;
                }
            }

           if (rxFound) {
                in.seek(0);

                // Load input values
                while ( ! in.atEnd()) {
                    QString sLine = in.readLine(0);
                    if (sLine.contains(rx)) {
                        QString ldpartid = rx.cap(1);
                        QString rbitemid = rx.cap(2).trimmed();
                        ld2rbCodesXRef[ldpartid.toLower()] = rbitemid;
                    }
                }
            } else {
                message = QString("Regular expression pattern was not found in %1.<br>"
                                  "Regenerate %1 by renaming the existing file and<br>"
                                  "select edit %1 from the configuration menu.")
                                  .arg(QFileInfo(ld2rbCodesXRefFile).fileName());
                if (Preferences::modeGUI){
                    QMessageBox::warning(nullptr,QMessageBox::tr(VER_PRODUCTNAME_STR),message);
                } else {
                    logError() << message.replace("<br>"," ");
                }
            }
        } else {
            ld2rbCodesXRef.clear();
            QByteArray Buffer;
            loadLD2RBCodesXRef(Buffer);
            QTextStream instream(Buffer);
            for (QString sLine = instream.readLine(); !sLine.isNull(); sLine = instream.readLine())
            {
                QChar comment = sLine.at(0);
                if (comment == '#' || comment == ' ')
                    continue;
                if (sLine.contains(rx)) {
                    QString ldpartid = rx.cap(1);
                    QString rbitemid = rx.cap(2).trimmed();
                    ld2rbCodesXRef[ldpartid.toLower()] = rbitemid;
                }
            }
        }
    }
}

// key : blitemid+blcolorid
// val1: blitemid+"-"+blcolorid
// val2: elementid
bool Annotations::loadBLCodes(){
    if (blCodes.size() == 0) {
        QString message;
        QString blCodesFile = Preferences::blCodesFile;
        QRegExp rx("^([^\\t]+)\\t+\\s*([^\\t]+)\\t+\\s*([^\\t]+).*$");
        if (! blCodesFile.isEmpty()) {
            QFile file(blCodesFile);
            if ( ! file.open(QFile::ReadOnly | QFile::Text)) {
                message = QString("Failed to open BrickLink codes.txt file: %1:<br>%2")
                        .arg(blCodesFile)
                        .arg(file.errorString());
                if (Preferences::modeGUI){
                    QMessageBox::warning(nullptr,QMessageBox::tr(VER_PRODUCTNAME_STR),message);
                } else {
                    logError() << message.replace("<br>"," ");
                }
                return false;
            }
            QTextStream in(&file);

            in.seek(0);

// DEBUG -->>>
//            QString fooFile = Preferences::blCodesFile+"demo.txt";
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
                    blCodes[QString(blitemid+blcolorid).toLower()] << QString(blitemid+"-"+blcolorid).toUpper() << elementid;
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

bool Annotations::loadBLCodes(QByteArray &Buffer){
    if (blCodes.size() == 0) {
        QString message;
        QRegExp rx("^([^\\t]+)\\t+\\s*([^\\t]+)\\t+\\s*([^\\t]+).*$");
        QTextStream instream(Buffer);

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
                blCodes[QString(blitemid+blcolorid).toLower()] << QString(blitemid+"-"+blcolorid).toUpper() << elementid;
            }
        }

        //    write stream to file
        QFile file(QString("%1/extras/%2").arg(Preferences::lpubDataPath,VER_LPUB3D_BLCODES_FILE));
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

            message = QString("Finished Writing, Proceed %1 lines for file [%2]")
                              .arg(counter)
                              .arg(file.fileName());
            if (Preferences::modeGUI){
                QMessageBox::information(nullptr,QMessageBox::tr(VER_PRODUCTNAME_STR),message);
            } else {
                logInfo() << message;
            }
        }
        else
        {
           message = QString("Failed to write file: %1:<br>%2")
                             .arg(file.fileName())
                             .arg(file.errorString());
           if (Preferences::modeGUI){
               QMessageBox::warning(nullptr,QMessageBox::tr(VER_PRODUCTNAME_STR),message);
           } else {
               logError() << message.replace("<br>"," ");
           }
           return false;
        }
    }
    return true;
}

// key: ldpartid+ldcolorid
// val: elementid
bool Annotations::loadLEGOElements(){
    if (legoElements.size() == 0) {
        QString message;
        bool rxFound = false;
        QString legoElementsFile = Preferences::legoElementsFile;
        QRegExp rx("^([^\\t]+)\\t+\\s*([^\\t]+)\\t+\\s*([^\\t]+).*$");
        if (!legoElementsFile.isEmpty()) {
            QFile file(legoElementsFile);
            if ( ! file.open(QFile::ReadOnly | QFile::Text)) {
                QString message = QString("Failed to open legoelements.lst file: %1:<br>%2")
                                          .arg(legoElementsFile)
                                          .arg(file.errorString());
                if (Preferences::modeGUI){
                    QMessageBox::warning(nullptr,QMessageBox::tr(VER_PRODUCTNAME_STR),message);
                } else {
                    logError() << message.replace("<br>"," ");
                }
                return false;
            }
            QTextStream in(&file);

            // Load RegExp from file;
            QRegExp rxin("^#\\sThe\\sRegular\\sExpression\\sused\\sis\\:[\\s](\\^.*)$");
            while ( ! in.atEnd()) {
                QString sLine = in.readLine(0);
                if ((rxFound = sLine.contains(rxin))) {
                    rx.setPattern(rxin.cap(1));
//                    logDebug() << "LEGO elements RegExp Pattern: " << rxin.cap(1);
                    break;
                }
            }

            if (rxFound) {
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
                message = QString("Regular expression pattern was not found in %1.<br>"
                                  "Regenerate %1 by renaming the existing file and<br>"
                                  "select edit %1 from the configuration menu.")
                                  .arg(QFileInfo(legoElementsFile).fileName());
                if (Preferences::modeGUI){
                    QMessageBox::warning(nullptr,QMessageBox::tr(VER_PRODUCTNAME_STR),message);
                } else {
                    logError() << message.replace("<br>"," ");
                }
            }
        } else {
            message = QString("LEGO Elements file was not found : %1").arg(legoElementsFile);
            if (Preferences::modeGUI){
                QMessageBox::warning(nullptr,QMessageBox::tr(VER_PRODUCTNAME_STR),message);
            } else {
                logError() << message.replace("<br>"," ");
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
    loadBLCodes();
    QString blcolorid,elementkey;
    if (ld2blColorsXRef.contains(ldcolorid.toLower())) {
        blcolorid = ld2blColorsXRef[ldcolorid.toLower()];
    }
    if (!blcolorid.isEmpty()){
        elementkey = QString(ldpartid+blcolorid).toLower();
        if (blCodes.contains(elementkey)){
            return blCodes[elementkey][which];
        }
        else
        if (ld2blCodesXRef.contains(ldpartid.toLower())) {
            elementkey = QString(ld2blCodesXRef[ldpartid.toLower()]+blcolorid).toLower();
            if (blCodes.contains(elementkey)) {
                return blCodes[elementkey][which];
            }
        }
    }
    return returnString;
}

const int &Annotations::getRBColorID(QString ldcolorid)
{
    returnInt = -1;
    if (ld2rbColorsXRef.contains(ldcolorid.toLower()))
        returnInt = ld2rbColorsXRef[ldcolorid.toLower()].toInt();
    return returnInt;
}

const QString &Annotations::getBrickLinkPartId(QString ldpartid)
{
    returnString = ldpartid;
    if (ld2blCodesXRef.contains(ldpartid.toLower()))
        returnString = ld2blCodesXRef[ldpartid.toLower()];
    return returnString;
}

const int &Annotations::getBrickLinkColor(int ldcolorid) {
    returnInt = 0;
    if (ld2blColorsXRef.contains(QString::number(ldcolorid)))
        returnInt = ld2blColorsXRef[QString::number(ldcolorid)].toInt();
    return returnInt;
}

const QString &Annotations::getRBPartID(QString ldpartid)
{
    if (ld2rbCodesXRef.contains(ldpartid.toLower()))
        returnString = ld2rbCodesXRef[ldpartid.toLower()];
    return returnString;
}

bool Annotations::overwriteFile(const QString &file)
{
    QFileInfo fileInfo(file);

    if (!fileInfo.exists())
        return true;

    // Get the application icon as a pixmap
    QPixmap _icon = QPixmap(":/icons/lpub96.png");
    if (_icon.isNull())
        _icon = QPixmap (":/icons/update.png");

    QMessageBox box;
    box.setWindowIcon(QIcon());
    box.setIconPixmap (_icon);
    box.setTextFormat (Qt::RichText);
    box.setWindowFlags (Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
    QString title = "<b>" + QMessageBox::tr ("Export %1").arg(fileInfo.fileName()) + "</b>";
    QString text = QMessageBox::tr("\"%1\"<br>This file already exists.<br>Replace existing file?").arg(fileInfo.fileName());
    box.setText (title);
    box.setInformativeText (text);
    box.setStandardButtons (QMessageBox::Cancel | QMessageBox::Yes);
    box.setDefaultButton   (QMessageBox::Yes);

    return (box.exec() == QMessageBox::Yes);
}

bool Annotations::exportAnnotationStyleFile(){

    QFile file(QString("%1/extras/%2").arg(Preferences::lpubDataPath,Preferences::validAnnotationStyleFile));

    if (!overwriteFile(file.fileName()))
        return true;

    if(file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        int counter = 1;
        QTextStream outstream(&file);
        outstream << "# File:" << Preferences::validAnnotationStyleFile << endl;
        outstream << "#" << endl;
        outstream << "# Space-delmited LDraw Design ID, Annotation Style and Part Category cross reference" << endl;
        outstream << "#" << endl;
        outstream << "# The Regular Expression used is: ^(\\b[^=]+\\b)=([1|2])\\s+([1-6])?\\s*([^\\s]+).*$" << endl;
        outstream << "#" << endl;
        outstream << "# 1. Design ID:           LDraw Part Name                 (Required)" << endl;
        outstream << "# 2. Annotation Style:    1 Square, 2 Circle, 3 Rectangle (Required)" << endl;
        outstream << "# 3. Part Category:       LDraw Part category             (Required)" << endl;
        outstream << "# 4. Annotation:          Annotation text                 (Optional - uses title annotation if not defined)" << endl;
        outstream << "# 5. Part Description:    Description for reference only  (Optional - not loaded)" << endl;
        outstream << "#" << endl;
        outstream << "# ---------------------------------------" << endl;
        outstream << "# |No |Annotation Style | Part Category |" << endl;
        outstream << "# |---|---------------------------------|" << endl;
        outstream << "# | 1 |circle(1)        |axle(1)        |" << endl;
        outstream << "# | 2 |square(2)        |beam(2)        |" << endl;
        outstream << "# | 3 |rectangle(3)     |cable(3)       |" << endl;
        outstream << "# | 4 |square(2)        |connector(4)   |" << endl;
        outstream << "# | 5 |square(2)        |hose(5)        |" << endl;
        outstream << "# | 6 |square(2)        |panel(6)       |" << endl;
        outstream << "# ---------------------------------------" << endl;
        outstream << "#" << endl;
        outstream << "# 32034.dat=1  4  2    Technic Angle Connector #2 (180 degree)" << endl;
        outstream << "#" << endl;
        outstream << "# This is one of five parameter files required to enable part element identification and annotation style." << endl;
        outstream << "#" << endl;
        outstream << "# ld2blcolorsxref.lst   - Tab-delmited LDConfig and BrickLink Color code cross reference" << endl;
        outstream << "# ld2blcodesxref.lst    - Tab-delmited LDraw Design ID and BrickLink Item Number cross reference" << endl;
        outstream << "# styledAnnotations.lst - Space-delmited LDraw Design ID, Annotation Style and Part Category cross reference" << endl;
        outstream << "# colors.txt            - Tab-delmited BrickLink Color codes and Color Name corss reference" << endl;
        outstream << "# codes.txt             - Tab-delimited Bricklink Design ID, Color Name and LEGO Element ID cross reference" << endl;
        outstream << "#" << endl;
        outstream << "# codes.txt and colors.txt are formatted the same their respective extract files from Bricklink.com. Thus" << endl;
        outstream << "# it is possible to simply extract those these files from Bricklnk and place them in the extras subfolder." << endl;
        outstream << "#" << endl;
        outstream << "# The main parameter file used for accessing part elements is codes.txt. The remaining four parameter files" << endl;
        outstream << "# are used to create mappings between LDraw Design ID, LDConfig Color ID, Bricklink Color ID, Bricklink" << endl;
        outstream << "# Color Name and Bricklink Item Number. With the exception of codes.txt, all parameter file data is hard" << endl;
        outstream << "# coded in the Annotations class. LPub3D will look in the extras subfolder for all parameter files, if a " << endl;
        outstream << "# parameter file is found, it will be loaded. If not found, LPub3D will revert to the hard coded data." << endl;
        outstream << "# If codes.txt is not found locally, LPub3D will attempt to download it from" << endl;
        outstream << "# https://github.com/trevorsandy/lpub3d_libs/releases/download/v1.0.1/codes.txt." << endl;
        outstream << "#" << endl;
        outstream << "# To get the Bricklink element ID, the Bricklink Color Name is matched with its Bricklink Color ID from the " << endl;
        outstream << "# Color ID reference and then the Bricklink Item No is prepended to the Bricklink Color ID." << endl;
        outstream << "#" << endl;
        outstream << "# ----------------------Do not delete above this line----------------------------------" << endl;
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
            QMessageBox::information(nullptr,QMessageBox::tr(VER_PRODUCTNAME_STR),message);
        } else {
            logNotice() << message;
        }
    }
    else
    {
        QString message = QString("Failed to open Annotation style file: %1:<br>%2")
                                  .arg(file.fileName())
                                  .arg(file.errorString());
        if (Preferences::modeGUI){
            QMessageBox::warning(nullptr,QMessageBox::tr(VER_PRODUCTNAME_STR),message);
        } else {
            logError() << message.replace("<br>"," ");
        }
       return false;
    }
    return true;
}

bool Annotations::exportBLColorsFile(){
    QFile file(QString("%1/extras/%2").arg(Preferences::lpubDataPath,"/" VER_LPUB3D_BLCOLORS_FILE));

    if (!overwriteFile(file.fileName()))
        return true;

    if(file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        int counter = 1;
        QTextStream outstream(&file);
        outstream << "# File:" << VER_LPUB3D_BLCOLORS_FILE << endl;
        outstream << "#" << endl;
        outstream << "# Tab-delmited BrickLink Color code and Color Name cross reference" << endl;
        outstream << "#" << endl;
        outstream << "# The Regular Expression used is: ^([^\\t]+)\\t+\\s*([^\\t]+).*$" << endl;
        outstream << "#" << endl;
        outstream << "# 1. Color ID:            BrickLink Color ID             (Required)" << endl;
        outstream << "# 2. Color Name:          BrickLink Color Name           (Required)" << endl;
        outstream << "#" << endl;
        outstream << "# This is one of five parameter files required to enable part element identification and annotation style." << endl;
        outstream << "#" << endl;
        outstream << "# ld2blcolorsxref.lst   - Tab-delmited LDConfig and BrickLink Color code cross reference" << endl;
        outstream << "# ld2blcodesxref.lst    - Tab-delmited LDraw Design ID and BrickLink Item Number cross reference" << endl;
        outstream << "# styledAnnotations.lst - Space-delmited LDraw Design ID, Annotation Style and Part Category cross reference" << endl;
        outstream << "# colors.txt            - Tab-delmited BrickLink Color codes and Color Name corss reference" << endl;
        outstream << "# codes.txt             - Tab-delimited Bricklink Design ID, Color Name and LEGO Element ID cross reference" << endl;
        outstream << "#" << endl;
        outstream << "# codes.txt and colors.txt are formatted the same their respective extract files from Bricklink.com. Thus" << endl;
        outstream << "# it is possible to simply extract those these files from Bricklnk and place them in the extras subfolder." << endl;
        outstream << "#" << endl;
        outstream << "# The main parameter file used for accessing part elements is codes.txt. The remaining four parameter files" << endl;
        outstream << "# are used to create mappings between LDraw Design ID, LDConfig Color ID, Bricklink Color ID, Bricklink" << endl;
        outstream << "# Color Name and Bricklink Item Number. With the exception of codes.txt, all parameter file data is hard" << endl;
        outstream << "# coded in the Annotations class. LPub3D will look in the extras subfolder for all parameter files, if a " << endl;
        outstream << "# parameter file is found, it will be loaded. If not found, LPub3D will revert to the hard coded data." << endl;
        outstream << "# If codes.txt is not found locally, LPub3D will attempt to download it from" << endl;
        outstream << "# https://github.com/trevorsandy/lpub3d_libs/releases/download/v1.0.1/codes.txt." << endl;
        outstream << "#" << endl;
        outstream << "# To get the Bricklink element ID, the Bricklink Color Name is matched with its Bricklink Color ID from the " << endl;
        outstream << "# Color ID reference and then the Bricklink Item No is prepended to the Bricklink Color ID." << endl;
        outstream << "#" << endl;
        outstream << "# ----------------------Do not delete above this line----------------------------------" << endl;
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
            QMessageBox::information(nullptr,QMessageBox::tr(VER_PRODUCTNAME_STR),message);
        } else {
            logNotice() << message;
        }
    }
    else
    {
        QString message = QString("Failed to open BrickLink Color Code file: %1:<br>%2")
                                  .arg(file.fileName())
                                  .arg(file.errorString());
        if (Preferences::modeGUI){
            QMessageBox::warning(nullptr,QMessageBox::tr(VER_PRODUCTNAME_STR),message);
        } else {
            logError() << message.replace("<br>"," ");
        }
       return false;
    }
    return true;
}

bool Annotations::exportLD2BLColorsXRefFile(){    
    QFile file(QString("%1/extras/%2").arg(Preferences::lpubDataPath,"/" VER_LPUB3D_LD2BLCOLORSXREF_FILE));

    if (!overwriteFile(file.fileName()))
        return true;

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
        outstream << "# 1. LDConfig Color ID: LDraw Color ID             (Required)" << endl;
        outstream << "# 2. Color ID:          BrickLink Color ID         (Required)" << endl;
        outstream << "#" << endl;
        outstream << "# This is one of five parameter files required to enable part element identification and annotation style." << endl;
        outstream << "#" << endl;
        outstream << "# ld2blcolorsxref.lst   - Tab-delmited LDConfig and BrickLink Color code cross reference" << endl;
        outstream << "# ld2blcodesxref.lst    - Tab-delmited LDraw Design ID and BrickLink Item Number cross reference" << endl;
        outstream << "# styledAnnotations.lst - Space-delmited LDraw Design ID, Annotation Style and Part Category cross reference" << endl;
        outstream << "# colors.txt            - Tab-delmited BrickLink Color codes and Color Name corss reference" << endl;
        outstream << "# codes.txt             - Tab-delimited Bricklink Design ID, Color Name and LEGO Element ID cross reference" << endl;
        outstream << "#" << endl;
        outstream << "# codes.txt and colors.txt are formatted the same their respective extract files from Bricklink.com. Thus" << endl;
        outstream << "# it is possible to simply extract those these files from Bricklnk and place them in the extras subfolder." << endl;
        outstream << "#" << endl;
        outstream << "# The main parameter file used for accessing part elements is codes.txt. The remaining four parameter files" << endl;
        outstream << "# are used to create mappings between LDraw Design ID, LDConfig Color ID, Bricklink Color ID, Bricklink" << endl;
        outstream << "# Color Name and Bricklink Item Number. With the exception of codes.txt, all parameter file data is hard" << endl;
        outstream << "# coded in the Annotations class. LPub3D will look in the extras subfolder for all parameter files, if a " << endl;
        outstream << "# parameter file is found, it will be loaded. If not found, LPub3D will revert to the hard coded data." << endl;
        outstream << "# If codes.txt is not found locally, LPub3D will attempt to download it from" << endl;
        outstream << "# https://github.com/trevorsandy/lpub3d_libs/releases/download/v1.0.1/codes.txt." << endl;
        outstream << "#" << endl;
        outstream << "# To get the Bricklink element ID, the Bricklink Color Name is matched with its Bricklink Color ID from the " << endl;
        outstream << "# Color ID reference and then the Bricklink Item No is prepended to the Bricklink Color ID." << endl;
        outstream << "#" << endl;
        outstream << "# ----------------------Do not delete above this line----------------------------------" << endl;
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
            QMessageBox::information(nullptr,QMessageBox::tr(VER_PRODUCTNAME_STR),message);
        } else {
            logNotice() << message;
        }
    }
    else
    {
        QString message = QString("Failed to open LDConfig and BrickLink Color Code file: %1:<br>%2")
                                  .arg(file.fileName())
                                  .arg(file.errorString());
        if (Preferences::modeGUI){
            QMessageBox::warning(nullptr,QMessageBox::tr(VER_PRODUCTNAME_STR),message);
        } else {
            logError() << message.replace("<br>"," ");
        }
       return false;
    }
    return true;
}

bool Annotations::exportLD2BLCodesXRefFile(){
    QFile file(QString("%1/extras/%2").arg(Preferences::lpubDataPath,"/" VER_LPUB3D_LD2BLCODESXREF_FILE));

    if (!overwriteFile(file.fileName()))
        return true;

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
        outstream << "# 1. Design ID:            LDraw Part Number            (Required)" << endl;
        outstream << "# 2. Item No:              BrickLink Item Number        (Required)" << endl;
        outstream << "#" << endl;
        outstream << "# This is one of five parameter files required to enable part element identification and annotation style." << endl;
        outstream << "#" << endl;
        outstream << "# ld2blcolorsxref.lst   - Tab-delmited LDConfig and BrickLink Color code cross reference" << endl;
        outstream << "# ld2blcodesxref.lst    - Tab-delmited LDraw Design ID and BrickLink Item Number cross reference" << endl;
        outstream << "# styledAnnotations.lst - Space-delmited LDraw Design ID, Annotation Style and Part Category cross reference" << endl;
        outstream << "# colors.txt            - Tab-delmited BrickLink Color codes and Color Name corss reference" << endl;
        outstream << "# codes.txt             - Tab-delimited Bricklink Design ID, Color Name and LEGO Element ID cross reference" << endl;
        outstream << "#" << endl;
        outstream << "# codes.txt and colors.txt are formatted the same their respective extract files from Bricklink.com. Thus" << endl;
        outstream << "# it is possible to simply extract those these files from Bricklnk and place them in the extras subfolder." << endl;
        outstream << "#" << endl;
        outstream << "# The main parameter file used for accessing part elements is codes.txt. The remaining four parameter files" << endl;
        outstream << "# are used to create mappings between LDraw Design ID, LDConfig Color ID, Bricklink Color ID, Bricklink" << endl;
        outstream << "# Color Name and Bricklink Item Number. With the exception of codes.txt, all parameter file data is hard" << endl;
        outstream << "# coded in the Annotations class. LPub3D will look in the extras subfolder for all parameter files, if a " << endl;
        outstream << "# parameter file is found, it will be loaded. If not found, LPub3D will revert to the hard coded data." << endl;
        outstream << "# If codes.txt is not found locally, LPub3D will attempt to download it from" << endl;
        outstream << "# https://github.com/trevorsandy/lpub3d_libs/releases/download/v1.0.1/codes.txt." << endl;
        outstream << "#" << endl;
        outstream << "# To get the Bricklink element ID, the Bricklink Color Name is matched with its Bricklink Color ID from the " << endl;
        outstream << "# Color ID reference and then the Bricklink Item No is prepended to the Bricklink Color ID." << endl;
        outstream << "#" << endl;
        outstream << "# ----------------------Do not delete above this line----------------------------------" << endl;
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
            QMessageBox::information(nullptr,QMessageBox::tr(VER_PRODUCTNAME_STR),message);
        } else {
            logNotice() << message;
        }
    }
    else
    {
        QString message = QString("Failed to open LDraw Design ID and BrickLink Item Number file: %1:<br>%2")
                                  .arg(file.fileName())
                                  .arg(file.errorString());
        if (Preferences::modeGUI){
            QMessageBox::warning(nullptr,QMessageBox::tr(VER_PRODUCTNAME_STR),message);
        } else {
            logError() << message.replace("<br>"," ");
        }
       return false;
    }
    return true;
}

bool Annotations::exportLD2RBColorsXRefFile(){
    QFile file(QString("%1/extras/%2").arg(Preferences::lpubDataPath,"/" VER_LPUB3D_LD2RBCOLORSXREF_FILE));

    if (!overwriteFile(file.fileName()))
        return true;

    if(file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        int counter = 1;
        QTextStream outstream(&file);
        outstream << "# File:" << VER_LPUB3D_LD2RBCOLORSXREF_FILE << endl;
        outstream << "#" << endl;
        outstream << "# Tab-delmited LDConfig and Rebrickable Color code cross reference" << endl;
        outstream << "#" << endl;
        outstream << "# The Regular Expression used is: ^([^\\t]+)\\t+\\s*([^\\t]+).*$" << endl;
        outstream << "#" << endl;
        outstream << "# 1. LDConfig Color ID: LDraw Color ID             (Required)" << endl;
        outstream << "# 2. Color ID:          Rebrickable Color ID       (Required)" << endl;
        outstream << "#" << endl;
        outstream << "# This is one of two parameter files that support viewing parts on Rebrickable.com." << endl;
        outstream << "#" << endl;
        outstream << "# ld2rbcolorsxref.lst   - Tab-delmited LDConfig and Rebrickable Color code cross reference" << endl;
        outstream << "# ld2rbcodesxref.lst    - Tab-delmited LDraw Design ID and Rebrickable Item Number cross reference" << endl;
        outstream << "#" << endl;
        outstream << "# Use ld2rbcolorsxref.lst to create cross-reference entries for LDConfig and Rebrickable Color IDs." << endl;
        outstream << "#" << endl;
        outstream << "# ----------------------Do not delete above this line----------------------------------" << endl;
        outstream << "#" << endl;

        QByteArray Buffer;
        loadLD2RBColorsXRef(Buffer);
        QTextStream instream(Buffer);
        for (QString sLine = instream.readLine(); !sLine.isNull(); sLine = instream.readLine())
        {
            outstream << sLine << endl;
            counter++;
        }

        file.close();
        QString message = QString("Finished Writing LDConfig and Rebrickable Color Code Entries, Processed %1 lines in file [%2]")
                                   .arg(counter)
                                   .arg(file.fileName());
        if (Preferences::modeGUI){
            QMessageBox::information(nullptr,QMessageBox::tr(VER_PRODUCTNAME_STR),message);
        } else {
            logNotice() << message;
        }
    }
    else
    {
        QString message = QString("Failed to open LDConfig and Rebrickable Color Code file: %1:<br>%2")
                                  .arg(file.fileName())
                                  .arg(file.errorString());
        if (Preferences::modeGUI){
            QMessageBox::warning(nullptr,QMessageBox::tr(VER_PRODUCTNAME_STR),message);
        } else {
            logError() << message.replace("<br>"," ");
        }
       return false;
    }
    return true;
}

bool Annotations::exportLD2RBCodesXRefFile(){
    QFile file(QString("%1/extras/%2").arg(Preferences::lpubDataPath,"/" VER_LPUB3D_LD2RBCODESXREF_FILE));

    if (!overwriteFile(file.fileName()))
        return true;

    if(file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        int counter = 1;
        QTextStream outstream(&file);
        outstream << "# File:" << VER_LPUB3D_LD2RBCODESXREF_FILE << endl;
        outstream << "#" << endl;
        outstream << "# Tab-delmited LDraw Design ID and Rebrickable Part ID cross reference" << endl;
        outstream << "#" << endl;
        outstream << "# The Regular Expression used is: ^([^\\t]+)\\t+\\s*([^\\t]+).*$" << endl;
        outstream << "#" << endl;
        outstream << "# 1. Design ID:            LDraw Part Number            (Required)" << endl;
        outstream << "# 2. Part ID:              Rebrickable Part Number      (Required)" << endl;
        outstream << "#" << endl;
        outstream << "# This is one of two parameter files that support viewing parts on Rebrickable.com." << endl;
        outstream << "#" << endl;
        outstream << "# ld2rbcolorsxref.lst   - Tab-delmited LDConfig and Rebrickable Color code cross reference" << endl;
        outstream << "# ld2rbcodesxref.lst    - Tab-delmited LDraw Design ID and Rebrickable Item Number cross reference" << endl;
        outstream << "#" << endl;
        outstream << "# Use ld2rbcodesxref.lst to create cross-reference entries for LDraw Design ID and Rebrickable Parg ID." << endl;
        outstream << "#" << endl;
        outstream << "# ----------------------Do not delete above this line----------------------------------" << endl;
        outstream << "#" << endl;

        QByteArray Buffer;
        loadLD2RBCodesXRef(Buffer);
        QTextStream instream(Buffer);
        for (QString sLine = instream.readLine(); !sLine.isNull(); sLine = instream.readLine())
        {
            outstream << sLine << endl;
            counter++;
        }

        file.close();
        QString message = QString("Finished Writing LDraw Design ID and Rebrickable Part ID Entries, Processed %1 lines in file [%2]")
                                   .arg(counter)
                                   .arg(file.fileName());
        if (Preferences::modeGUI){
            QMessageBox::information(nullptr,QMessageBox::tr(VER_PRODUCTNAME_STR),message);
        } else {
            logNotice() << message;
        }
    }
    else
    {
        QString message = QString("Failed to open LDraw Design ID and Rebrickable Part ID file: %1:<br>%2")
                                  .arg(file.fileName())
                                  .arg(file.errorString());
        if (Preferences::modeGUI){
            QMessageBox::warning(nullptr,QMessageBox::tr(VER_PRODUCTNAME_STR),message);
        } else {
            logError() << message.replace("<br>"," ");
        }
       return false;
    }
    return true;
}

bool Annotations::exportTitleAnnotationsFile(){
    QFile file(QString("%1/extras/%2").arg(Preferences::lpubDataPath,Preferences::titleAnnotationsFile));

    if (!overwriteFile(file.fileName()))
        return true;

    if(file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        int counter = 1;
        QTextStream outstream(&file);
        outstream << "# File:" << Preferences::titleAnnotationsFile << endl;
        outstream << "#" << endl;
        outstream << "# CAUTION: Editing this file requires some knowledge/experience using Regular Expressions." << endl;
        outstream << "# Additionally, you must be familiar with LDraw Parts.lst conventions." << endl;
        outstream << "# Incorrectly changing the expressions below will at best, exclude some annotations from being captured" << endl;
        outstream << "# and at worst, exclude the entire category (e.g. Technic Axle) annotations from being captured." << endl;
        outstream << "# EDIT THIS FILE ONLY IF YOU ARE COMFORTABLE WITH THE REQUIREMENTS STATED ABOVE." << endl;
        outstream << "#" << endl;
        outstream << "# This space-delimited list captures part category and annotation Regular Expression." << endl;
        outstream << "# Follow this format to define annotations derived from part title (LDraw Parts.lst)" << endl;
        outstream << "# The format is category followed by regex string" << endl;
        outstream << "# Annotations are extracted from the part title and further processed in LPub3D" << endl;
        outstream << "# For efficiency put most used parts first." << endl;
        outstream << "" << endl;
        outstream << "# This file can be edited from LPub3D from:" << endl;
        outstream << "#    Configuration=>Edit Parameter Files=>Edit Part Title PLI Annotations" << endl;
        outstream << "" << endl;
        outstream << "# LPub3D will attempt to load the regular expression below first, if the" << endl;
        outstream << "# load fails, LPub3D will load the hard-coded (default) regular expression." << endl;
        outstream << "# If you wish to modify the file import, you can edit this regular expression." << endl;
        outstream << "# It would be wise to backup the default entry before performing and update - copy" << endl;
        outstream << "# and paste to a new line with starting phrase other than 'The Regular Expression...'" << endl;
        outstream << "" << endl;
        outstream << "# The Regular Expression used is: ^(\\b.*[^\\s]\\b:)\\s+([\\(|\\^].*)$" << endl;
        outstream << "" << endl;
        outstream << "# ----------------------Do not delete above this line----------------------------------" << endl;
        outstream << "" << endl;
        outstream << "#Category   Regex (to identify and extract annotation)" << endl;

        QByteArray Buffer;
        QTextStream instream(Buffer);
        loadTitleAnnotations(Buffer);
        for (QString sLine = instream.readLine(); !sLine.isNull(); sLine = instream.readLine())
        {
            outstream << sLine << endl;
            counter++;
        }

        file.close();
        QString message = QString("Finished Writing Title Annotation Entries, Processed %1 lines in file [%2]")
                                   .arg(counter)
                                   .arg(file.fileName());
        if (Preferences::modeGUI){
            QMessageBox::information(nullptr,QMessageBox::tr(VER_PRODUCTNAME_STR),message);
        } else {
            logNotice() << message;
        }
    }
    else
    {
        QString message = QString("Failed to open Title Annotations file: %1:\n%2")
                                  .arg(file.fileName())
                                  .arg(file.errorString());
        if (Preferences::modeGUI){
            QMessageBox::warning(nullptr,QMessageBox::tr(VER_PRODUCTNAME_STR),message);
        } else {
            logError() << message;
        }
       return false;
    }
    return true;
}

bool Annotations::exportfreeformAnnotationsHeader(){
    QFile file(QString("%1/extras/%2").arg(Preferences::lpubDataPath,Preferences::freeformAnnotationsFile));

    if (!overwriteFile(file.fileName()))
        return true;

    if(file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        int counter = 1;
        QTextStream outstream(&file);
        outstream << "# File:" << Preferences::freeformAnnotationsFile << endl;
        outstream << "#" << endl;
        outstream << "# This space-delimited list captures part ID and its free form annotation." << endl;
        outstream << "# Follow this format to define freeform annotations." << endl;
        outstream << "# The format is part ID followed free-form annotation (some special characters may not be supported)." << endl;
        outstream << "" << endl;
        outstream << "# This file can be edited from LPub3D from:" << endl;
        outstream << "#    Configuration=>Edit Parameter Files=>Edit Freeform PLI Annotations" << endl;
        outstream << "" << endl;
        outstream << "# LPub3D will attempt to load the regular expression below first, if the" << endl;
        outstream << "# load fails, LPub3D will load the hard-coded (default) regular expression." << endl;
        outstream << "# If you wish to modify the file import, you can edit this regular expression." << endl;
        outstream << "# It would be wise to backup the default entry before performing and update - copy" << endl;
        outstream << "# and paste to a new line with starting phrase other than 'The Regular Expression...'" << endl;
        outstream << "" << endl;
        outstream << "# The Regular Expression used is: ^(\\b.*[^\\s]\\b)(?:\\s)\\s+(.*)$" << endl;
        outstream << "" << endl;
        outstream << "# ----------------------Do not delete above this line----------------------------------" << endl;
        outstream << "" << endl;
        outstream << "#ID         Annotation" << endl;
        outstream << "" << endl;

        QByteArray Buffer;
        QTextStream instream(Buffer);
        for (QString sLine = instream.readLine(); !sLine.isNull(); sLine = instream.readLine())
        {
            outstream << sLine << endl;
            counter++;
        }

        file.close();
        QString message = QString("Finished Writing Free-form Annotation Entries, Processed %1 lines in file [%2]")
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
        QString message = QString("Failed to open Free-form Annotations file: %1:\n%2")
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
