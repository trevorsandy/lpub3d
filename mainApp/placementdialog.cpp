
/****************************************************************************
**
** Copyright (C) 2007-2009 Kevin Clague. All rights reserved.
** Copyright (C) 2015 - 2019 Trevor SANDY. All rights reserved.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://www.trolltech.com/products/qt/opensource.html
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://www.trolltech.com/products/qt/licensing.html or contact the
** sales department At sales@trolltech.com.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

/****************************************************************************
 *
 * This class implements a dialog that lets the user pick the placement for
 * something relative to other things.
 *
 * Please see lpub.h for an overall description of how the files in LPub
 * make up the LPub program.
 *
 ***************************************************************************/

#include "metatypes.h"
#include "placementdialog.h"
#include "meta.h"
#include "version.h"

#include <QString>
#include <QGridLayout>
#include <QPushButton>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QFrame>
#include <QComboBox>

#include "QsLog.h"

const QString PlacementDialog::labels[][5] =
{
  {"Top/Left",   "Top:Left",   "Top:Center",   "Top:Right",   "Top/Right" },
  {"Left:Top",   "Top/Left",   "Top",          "Top/Right",   "Right:Top" },
  {"Left:Center","Left",       "Center",       "Right",       "Right:Center" },
  {"Left:Bottom","Bottom/Left","Bottom",       "Bottom/Right","Right:Bottom" },
  {"Bottom/Left","Bottom:Left","Bottom:Center","Bottom:Right","Right/Bottom" },
};

const QList<int> PlacementDialog::relativeToOks[NumRelatives] =
{ //                               {Page , Csi , Pli , Pn , Sn , Callout , Sm , Ph , Pf, Ms}
  /*  0 Page             Page    */{0},
  /*  1 Csi (Assem)      Csi     */{Page},
  /*  2 MultiStep        Ms      */{Page       , Pli},
  /*  3 StepNumber       Sn      */{Page , Csi , Pli                          , Ph , Pf},
  /*  4 Pli              Pli     */{Page , Csi       , Pn , Sn           , Sm , Ph , Pf},
  /*  5 Callout          Callout */{Page , Csi , Pli      , Sn                , Ph , Pf},
  /*  6 PageNum          Pn      */{Page , Csi , Pli , Pn , Sn , Callout      , Ph , Pf},

  /*  7 Title            Tt      */{Page},
  /*  8 ModelName        Mnt     */{Page                                                                               , Tt},
  /*  9 Author           At      */{Page             , Pn                     , Ph , Pf , Tt ,      Ct , Et , Urlt},
  /* 10 Url              Urlt    */{Page             , Pn                     , Ph , Pf      , At , Ct , Et},
  /* 11 ModelDesc        Mdt     */{Page                                                                               , Pt},
  /* 12 PublishDesc      Pdt     */{Page                                                                               , Mdt},
  /* 13 Copyright        Ct      */{Page             , Pn                     , Ph , Pf      , At ,      Et , Urlt},
  /* 14 Email            Et      */{Page             , Pn                     , Ph , Pf      , At , Ct ,      Urlt},
  /* 15 Disclaimer       Dt      */{Page                                                               , Et},
  /* 16 Parts            Pt      */{Page                                                     , At},
  /* 17 Plug             Plt     */{Page                                                                               , Dt},
  /* 18 SubmodelInsCount Sic     */{Page , Csi , Pli , Pn , Sn},
  /* 19 DocumentLogo     Dlt     */{Page                                      , Ph , Pf},
  /* 20 CoverImage       Cit     */{Page},
  /* 21 PlugImage        Pit     */{Page                                           , Pf                                , Plt},
  /* 22 PageHeader       Ph      */{Page},
  /* 23 PageFooter       Pf      */{Page},
  /* 24 Category         Cat     */{Page             , Pn                     , Ph , Pf , Tt ,      Ct , Et , Urlt},
  /* 25 Submodel         Sm      */{Page , Csi , Pli , Pn , Sn                , Ph , Pf},
  /* 26 RotateIcon       Ri      */{Page , Csi , Pli      , Sn},
  /* 27 Csi Part         Cp      */{       Csi},
  /* 28 Step             Stp     */{Page , Rng},
  /* 29 Range                    */{Page},
  /* 30 Text                     */{Page                                      , Ph , Pf},
  /* 31 Bom                      */{Page                                      , Ph , Pf},

  /* 32 PagePointer              */{Page , Csi                                , Ph , Pf},
  /* 33 SingleStep               */{Page , Csi},
  /* 34 Reserve                  */{Page},
  /* 35 CoverPage                */{Page},
  /* 36 CsiAnnotationType        */{                             Callout ,               Ms},
  /* 37 DividerPinter            */{       Cp}

  /* 38 NumRelatives             */
};

//front cover options   Page     | Ph | Pf | Tt | At                  | mnt | pt | mdt | pdt | dlt,
//back  cover options   Page     | Ph | Pf | Tt | At | Ct | Et | Urlt |                      | dlt | dt | plt | pit,

const int PlacementDialog::prepositionOks[NumRelatives] = // indexed by them
{
  /*  0 Page                     */ InsideOk,
  /*  1 Csi                      */ InsideOk|OutsideOk,
  /*  2 MultiStep                */ OutsideOk,
  /*  3 StepNumber               */ OutsideOk,
  /*  4 Pli                      */ OutsideOk,
  /*  5 Callout                  */ OutsideOk,
  /*  6 PageNum                  */ OutsideOk,

  /*  7 Title                    */ OutsideOk,
  /*  8 ModelName                */ OutsideOk,
  /*  9 Author                   */ OutsideOk,
  /* 10 Url                      */ OutsideOk,
  /* 11 ModelDesc                */ OutsideOk,
  /* 12 PublishDesc              */ OutsideOk,
  /* 13 Copyright                */ OutsideOk,
  /* 14 Email                    */ OutsideOk,
  /* 15 Disclaimer               */ OutsideOk,
  /* 16 Parts                    */ OutsideOk,
  /* 17 Plug                     */ OutsideOk,
  /* 18 SubmodelInsCount         */ OutsideOk,
  /* 19 DocumentLogo             */ OutsideOk,
  /* 20 CoverImage               */ OutsideOk,
  /* 21 PlugImage                */ OutsideOk,
  /* 22 PageHeader               */ InsideOk|OutsideOk,
  /* 23 PageFooter               */ InsideOk|OutsideOk,
  /* 24 Category                 */ OutsideOk,
  /* 25 Submodel                 */ OutsideOk,
  /* 26 RotateIcon               */ OutsideOk,
  /* 27 Csi Part                 */ InsideOk|OutsideOk,
  /* 28 Step                     */ InsideOk,
  /* 29 Range                    */ InsideOk,
  /* 30 Text                     */ OutsideOk,
  /* 31 Bom                      */ OutsideOk,

  /* 32 PagePointer              */ OutsideOk,
  /* 33 SingleStep               */ OutsideOk,
  /* 34 Reserve                  */ OutsideOk,
  /* 35 CoverPage                */ InsideOk,
  /* 36 CsiAnnotationType        */ OutsideOk,
  /* 37 DividerPointer           */ InsideOk

  /* 38 NumRelatives             */
};

const QString PlacementDialog::relativeNames[NumRelatives] =
{
  "Page",        		        // 0 Page
  "Assem",                      // 1 Csi
  "Step Group",                 // 2 Ms
  "Step Number",                // 3 Sn
  "Parts List",                 // 4 Pli
  "Callout",                    // 5 Callout
  "Page Number",                // 6 Pn

  "Title",                      // 7 Tt
  "Model ID",                   // 8 Mnt
  "Author",                     // 9 At
  "URL",                        //10 Urlt
  "Model Description",          //11 Mdt
  "Publish Description",        //12 Pdt
  "Copyright",                  //13 Ct
  "Email",                      //14 Et
  "Disclaimer",                 //15 Dt
  "Parts",                      //16 Pt
  "Plug",                       //17 Plt
  "Submodel Instance Count",    //18 Sic
  "Logo",                       //19 Dlt
  "Cover Image",                //20 Cit
  "Plug Image",                 //21 Pit
  "Page Header",                //22 Ph
  "Page Footer",                //23 Pf
  "Category",                   //24 Cat
  "Submodel",                   //25 Cat
  "Rotate Icon",                //26 Ri
  "CSI Part",                   //27 Cp
  "Step Rectangle",             //28 Stp
  "Range",                      //29 Rng
  "Text",                       //30
  "BOM",                        //31

  "Page Pointer",               //32
  "Single Step",                //33
  "Reserve",                    //34
  "Cover Page",                 //35
  "CSI Annotation",             //36
  "Divider Pointer",            //37

 /*NumRelatives               *///38 NumRelatives
};

bool PlacementDialog::getPlacement(
  PlacementType  parentType,
  PlacementType  placedType,
  PlacementData &goods,
  QString        title,
  int            onPageType,
  bool           pliPerStep,
  QWidget       *parent)
{
  PlacementDialog *dialog = new PlacementDialog(parentType,placedType,goods,title,onPageType,pliPerStep,parent);

  bool ok = dialog->exec() == QDialog::Accepted;
  if (ok) {
    goods.offsets[0] = 0;
    goods.offsets[1] = 0;
  }
  return ok;
}

QString PlacementDialog::relativeToName(
  int relativeTo)
{
  if (relativeTo >= NumRelatives) {
    return relativeNames[0];
  } else {
    return relativeNames[relativeTo];
  }
}

PlacementDialog::PlacementDialog(
  PlacementType  parentType,
  PlacementType  placedType,
  PlacementData &_goods,
  QString        title,
  int            onPageType,
  bool           pliPerStep,
  QWidget       *parent)
{
  goods = &_goods;

  outsideGrid    = new QGridLayout;
  insideGrid     = new QGridLayout;
  insideFrame    = new QFrame;
  horizontalLine = new QFrame;

  lblRelativeTo = new QLabel("Relative To",parent);
  lblRelativeTo->setToolTip(tr("Select item that %1 will be moved relative to.").arg(title));
  outsideGrid->addWidget(lblRelativeTo,0,0);

  combo = new QComboBox;
  connect(combo,SIGNAL(activated(int)),this,SLOT(relativeToChanged(int)));
  outsideGrid->addWidget(combo,0,1);

  QList<int> oks;

//  logTrace() << " \nPLACEMENT DIALOG "
//             << " \nParentType: " << RelNames[parentType]     << " (" << parentType << ")"
//             << " \nPlacedType: " << RelNames[placedType]     << " (" << placedType << ")"
//             << " \nOnPageType: " << (onPageType == 0 ? "Content Page" : onPageType == 1 ? "Front Cover Page" : "Back Cover Page")
//                ;

  switch (parentType) {
    case StepGroupType:                             //parent type
      switch (placedType) {
        case StepType:                 //placed type
            oks << Page << Rng;
        break;
        case PageURLType:               //placed type
            oks << Page << Pn << Ph << Pf;
        break;
        case PageEmailType:             //placed type
            oks << Page << Pn << Ph << Pf             << Urlt;
        break;
        case PageAuthorType:            //placed type
            oks << Page << Pn << Ph << Pf       << Et << Urlt;
        break;
        case PageCopyrightType:         //placed type
            oks << Page << Pn << Ph << Pf << At << Et << Urlt;
        break;
        case PartsListType:             //placed type
          if (pliPerStep) {
            oks << Csi  << Sm << Sn /*<< Stp*/;
          } else {
            oks << Page << Sm << Sn << Ph << Pf << Ms;
          }
        break;
        case StepNumberType:            //placed type
          if (pliPerStep) {
            oks << Csi << Pli << Sm /*<< Stp*/;
          } else {
            oks << Page << Pli  << Sm << Ph << Pf << Ms;
          }
        break;
        case SubModelType:              //placed type
          if (pliPerStep) {
            oks << Csi << Pli << Sn /*<< Stp*/;
          } else {
            oks << Page << Pli << Sn << Ph << Pf << Ms;
          }
        break;
        case CalloutType:               //placed type
          oks << Page << Csi << Pli << Sn /*<< Stp*/;
        break;
        case SubmodelInstanceCountType: //placed type
          oks << Page << Pn;
        break;
        case PagePointerType:           //placed type
          oks << Page << Csi << Ph << Pf;
        break;
        case RotateIconType:            //placed type
          oks << Csi << Pli << Sn /*<< Stp*/;
        break;
        case TextType:                  //placed type
          oks << Page << Ph << Pf/* << Stp  << Csi*/;
        break;
        default:                        //placed type
          oks << Page << Pn /*<< Stp*/;
        break;
      }
    break;
    case CalloutType:                               //parent type
      switch (placedType) {
        case PartsListType:             //placed type
          oks << Csi << Sm;
        break;
        case StepNumberType:            //placed type
          oks << Csi  << Pli << Sm;
        break;
        case CalloutType:               //placed type
          oks << Csi  << Pli << Sn << Callout << Sm;
        break;
        case SubmodelInstanceCountType: //placed type
          oks << Page << Pn;
        break;
        case PagePointerType:           //placed type
          oks << Page << Csi << Ph << Pf;
        break;
        case SubModelType:             //placed type
        case RotateIconType:           //placed type
          oks << Csi << Pli << Sn;
        break;
        default:
          oks << Csi << Pli << Sn;
        break;
      }
    break;
    case StepType:                                  //parent type
      switch (placedType) {
        case PartsListType:             //placed type
          oks << Page << Csi << Sn << Sm << Ph << Pf;
        break;
        case StepNumberType:            //placed type
          oks << Page << Csi << Pli << Sm << Ph << Pf;
        break;
        case CalloutType:               //placed type
          oks << Page << Csi << Sn << Pli;
        break;
        case SubmodelInstanceCountType: //placed type
          oks << Page << Csi << Pli << Pn << Sn;
        break;
        case PagePointerType:           //placed type
          oks << Page << Csi << Ph << Pf;
        break;
        case SubModelType:              //placed type
          oks << Page << Csi << Pli << Sn << Ph << Pf;
        break;
        case RotateIconType:           //placed type
          oks << Csi << Pli << Sn;
        break;
        default:                        //placed type
          oks << Page << Csi << Pli << Sn;
        break;
      }
    break;
  case SingleStepType:                              //parent type
      switch (placedType) {
      case PageURLType:                  //placed type
        if (onPageType == BackCoverPage) {
           oks << Page                                       << Ct;
        } else
           oks << Page << Pn << Ph << Pf;
      break;
      case PageEmailType:                //placed type
        if (onPageType == BackCoverPage) {
           oks << Page                               << Urlt;
        } else
           oks << Page << Pn << Ph << Pf             << Urlt;
      break;
      case PageAuthorType:               //placed type
        if (onPageType == FrontCoverPage) {
           oks << Page                                        << Tt;
        } else if (onPageType == BackCoverPage) {
           oks << Page                                        << Tt;
        } else
           oks << Page << Pn << Ph << Pf       << Et << Urlt;
      break;
      case PageCopyrightType:            //placed type
        if (onPageType == BackCoverPage) {
           oks << Page                   << At;
        } else
           oks << Page << Pn << Ph << Pf << At << Et << Urlt;
      break;
      default:                           //placed type
        oks << relativeToOks[placedType];
      break;
      }
    break;
    default:                             //parent type
      oks << relativeToOks[placedType];
    break;
  }

  int currentIndex = 0;

  for (int i = 0; i < NumRelatives; i++) {

//      logNotice() << "\n POPULATE PLACEMENT COMBO"
//                     << "\nPlacedType:" << RelNames[placedType]
//                     << "\n     Index:" << i
//                     << "\n MatchType:" << RelNames[i];

      foreach(int ok, oks){
          if (ok == i){
              // logDebug() << "MATCH: Ok:" << ok << "Type:" << RelNames[i];
              combo->addItem(relativeNames[i]);
              if (i == goods->relativeTo) {
                  currentIndex = combo->count()-1;
                }
            }
        }
    }
  combo->setCurrentIndex(currentIndex);

  outsideGrid->addWidget(insideFrame,2,1,3,3);
  insideFrame->setLayout(insideGrid);
  insideFrame->setFrameShape(QFrame::StyledPanel);
  insideFrame->setFrameShadow(QFrame::Sunken);
  horizontalLine->setFrameShape(QFrame::HLine);
  horizontalLine->setFrameShadow(QFrame::Sunken);
  outsideGrid->addWidget(horizontalLine,6,0,1,5);

  for (int y = 0; y < 5; y++) {
    for (int x = 0; x < 5; x++) {
      QPushButton *button;
      button = new QPushButton(labels[y][x],parent);
      buttons[y][x] = button;

      if (y == 0 || y == 4) {
        outsideGrid->addWidget(button,y+1,x);
      } else if (x == 0 || x == 4) {
        outsideGrid->addWidget(button,y+1,x);
      } else {
        insideGrid->addWidget(button,y-1+1,x-1);
      }

      switch (y) {
        case 0:
          switch (x) {
            case 0:
              connect(button,SIGNAL(clicked()),this,SLOT(topLeftO()));
            break;
            case 1:
              connect(button,SIGNAL(clicked()),this,SLOT(topleft()));
            break;
            case 2:
              connect(button,SIGNAL(clicked()),this,SLOT(topcenter()));
            break;
            case 3:
              connect(button,SIGNAL(clicked()),this,SLOT(topright()));
            break;
            case 4:
              connect(button,SIGNAL(clicked()),this,SLOT(topRightO()));
            break;
          }
        break;
        case 1:
          switch (x) {
            case 0:
              connect(button,SIGNAL(clicked()),this,SLOT(lefttop()));
            break;
            case 1:
              connect(button,SIGNAL(clicked()),this,SLOT(topLeftI()));
            break;
            case 2:
              connect(button,SIGNAL(clicked()),this,SLOT(top()));
            break;
            case 3:
              connect(button,SIGNAL(clicked()),this,SLOT(topRightI()));
            break;
            case 4:
              connect(button,SIGNAL(clicked()),this,SLOT(righttop()));
            break;
          }
        break;
        case 2:
          switch (x) {
            case 0:
              connect(button,SIGNAL(clicked()),this,SLOT(leftcenter()));
            break;
            case 1:
              connect(button,SIGNAL(clicked()),this,SLOT(left()));
            break;
            case 2:
              connect(button,SIGNAL(clicked()),this,SLOT(center()));
            break;
            case 3:
              connect(button,SIGNAL(clicked()),this,SLOT(right()));
            break;
            case 4:
              connect(button,SIGNAL(clicked()),this,SLOT(rightcenter()));
            break;
          }
        break;
        case 3:
          switch (x) {
            case 0:
              connect(button,SIGNAL(clicked()),this,SLOT(leftbottom()));
            break;
            case 1:
              connect(button,SIGNAL(clicked()),this,SLOT(bottomLeftI()));
            break;
            case 2:
              connect(button,SIGNAL(clicked()),this,SLOT(bottom()));
            break;
            case 3:
              connect(button,SIGNAL(clicked()),this,SLOT(bottomRightI()));
            break;
            case 4:
              connect(button,SIGNAL(clicked()),this,SLOT(rightbottom()));
            break;
          }
        break;
        case 4:
          switch (x) {
            case 0:
              connect(button,SIGNAL(clicked()),this,SLOT(bottomLeftO()));
            break;
            case 1:
              connect(button,SIGNAL(clicked()),this,SLOT(bottomleft()));
            break;
            case 2:
              connect(button,SIGNAL(clicked()),this,SLOT(bottomcenter()));
            break;
            case 3:
              connect(button,SIGNAL(clicked()),this,SLOT(bottomright()));
            break;
            case 4:
              connect(button,SIGNAL(clicked()),this,SLOT(bottomRightO()));
            break;
          }
        break;
      }
    }
  }

  buttonBox = new QDialogButtonBox();               //removed 'this' from constructor
  buttonBox->addButton(QDialogButtonBox::Ok);
  connect(buttonBox,SIGNAL(accepted()),SLOT(accept()));
  buttonBox->addButton(QDialogButtonBox::Cancel);
  connect(buttonBox,SIGNAL(rejected()),SLOT(reject()));

  outsideGrid->addWidget(buttonBox,7,0,1,5);

  setLayout(outsideGrid);
  setWindowTitle(tr("%1 %2 Dialogue").arg(QString::fromLatin1(VER_PRODUCTNAME_STR),title));
  
  setEnabled(prepositionOks[goods->relativeTo]);
  highlightPlacement(goods);
  
  setModal(true);
  setMinimumSize(40,20);
}

PlacementDialog::~PlacementDialog()
{  
}

void PlacementDialog::relativeToChanged(int /* unused */)
{
  QString name = combo->currentText();

  for (int i = 0; i < NumRelatives; i++) {
    if (name == relativeNames[i]) {
      setEnabled(prepositionOks[i]);
      goods->relativeTo = PlacementType(i);
      goods->offsets[0] = 0;
      goods->offsets[1] = 0;
      break;
    }
  }
}

void PlacementDialog::setEnabled(int okPrepositions)
{
  for (int y = 0; y < 5; y++) {
    for (int x = 0; x < 5; x++) {
      buttons[y][x]->setEnabled(false);
    }
  }
  for (int y = 0; y < 5; y++) {
    for (int x = 0; x < 5; x++) {
      if (y == 0 || y == 4 || x == 0 || x == 4) {
        if (okPrepositions & OutsideOk) {
          buttons[y][x]->setEnabled(true);
        }
      } else {
        if (okPrepositions & InsideOk) {
          buttons[y][x]->setEnabled(true);
        }
      }
    }
  }
}

int insideDecode[NumPlacements][2] = 
{
  { 1, 1 }, { 1, 2 }, { 1, 3 },
  { 2, 3 }, { 3, 3,}, { 3, 2 },
  { 3, 1 }, { 2, 1 }, { 2, 2 }
};

void PlacementDialog::highlightPlacement(
  PlacementData *goods)
{
  int r = 0, c = 0;

  if (goods->preposition == Inside) {
    r = insideDecode[goods->placement][0];
    c = insideDecode[goods->placement][1];
  } else {
    switch (goods->placement) {
      case TopLeft:
        r = 0; c = 0;
      break;
      case TopRight:
        r = 0; c = 4;
      break;
      case BottomLeft:
        r = 4; c = 0;
      break;
      case BottomRight:
        r = 4; c = 4;
      break;
      case Top:
      case Bottom:
        if (goods->placement == Top) {
          r = 0;
        } else {
          r = 4;
        }
        switch (goods->justification) {
          case Left:
            c = 1;
          break;
          case Center:
            c = 2;
          break;
          case Right:
            c = 3;
          break;
          default:
          break;
        }
      break;
      case Right:
      case Left:
        if (goods->placement == Left) {
          c = 0;
        } else {
          c = 4;
        }
        switch (goods->justification) {
          case Top:
            r = 1;
          break;
          case Center:
            r = 2;
          break;
          case Bottom:
            r = 3;
          break;
          default:
          break;
        }
      break;
      default:
      break;
    } 
  }
  QFont font;
  for (int R = 0; R < 5; R++) {
    for (int C = 0; C < 5; C++) {
      font = buttons[R][C]->font();
      font.setBold(R == r && C == c);
      buttons[R][C]->setFont(font);
    }
  }   
}

void PlacementDialog::topLeftO()
{
  goods->placement = TopLeft;
  goods->preposition = Outside;
  highlightPlacement(goods);
}

void PlacementDialog::topleft()
{
  goods->placement = Top;
  goods->justification = Left;
  goods->preposition = Outside;
  highlightPlacement(goods);
}

void PlacementDialog::topcenter()
{
  goods->placement = Top;
  goods->justification = Center;
  goods->preposition = Outside;
  highlightPlacement(goods);
}

void PlacementDialog::topright()
{
  goods->placement = Top;
  goods->justification = Right;
  goods->preposition = Outside;
  highlightPlacement(goods);
}

void PlacementDialog::topRightO()
{
  goods->placement = TopRight;
  goods->preposition = Outside;
  highlightPlacement(goods);
}

/*************************/

void PlacementDialog::lefttop()
{
  goods->placement = Left;
  goods->justification = Top;
  goods->preposition = Outside;
  highlightPlacement(goods);
}

void PlacementDialog::topLeftI()
{
  goods->placement = TopLeft;
  goods->preposition = Inside;
  highlightPlacement(goods);
}

void PlacementDialog::top()
{
  goods->placement = Top;
  goods->justification = Center;
  goods->preposition = Inside;
  highlightPlacement(goods);
}

void PlacementDialog::topRightI()
{
  goods->placement = TopRight;
  goods->preposition = Inside;
  highlightPlacement(goods);
}

void PlacementDialog::righttop()
{
  goods->placement = Right;
  goods->justification = Top;
  goods->preposition = Outside;
  highlightPlacement(goods);
}

/*************************/

void PlacementDialog::leftcenter()
{
  goods->placement = Left;
  goods->justification = Center;
  goods->preposition = Outside;
  highlightPlacement(goods);
}

void PlacementDialog::left()
{
  goods->placement = Left;
  goods->justification = Center;
  goods->preposition = Inside;
  highlightPlacement(goods);
}

void PlacementDialog::center()
{
  goods->placement = Center;
  goods->justification = Center;
  goods->preposition = Inside;
  highlightPlacement(goods);
}

void PlacementDialog::right()
{
  goods->placement = Right;
  goods->preposition = Inside;
  highlightPlacement(goods);
}

void PlacementDialog::rightcenter()
{
  goods->placement = Right;
  goods->justification = Center;
  goods->preposition = Outside;
  highlightPlacement(goods);
}

/*************************/

void PlacementDialog::leftbottom()
{
  goods->placement = Left;
  goods->justification = Bottom;
  goods->preposition = Outside;
  highlightPlacement(goods);
}

void PlacementDialog::bottomLeftI()
{
  goods->placement = BottomLeft;
  goods->preposition = Inside;
  highlightPlacement(goods);
}

void PlacementDialog::bottom()
{
  goods->placement = Bottom;
  goods->justification = Center;
  goods->preposition = Inside;
  highlightPlacement(goods);
}

void PlacementDialog::bottomRightI()
{
  goods->placement = BottomRight;
  goods->preposition = Inside;
  highlightPlacement(goods);
}

void PlacementDialog::rightbottom()
{
  goods->placement = Right;
  goods->justification = Bottom;
  goods->preposition = Outside;
  highlightPlacement(goods);
}

/*************************/

void PlacementDialog::bottomLeftO()
{
  goods->placement = BottomLeft;
  goods->preposition = Outside;
  highlightPlacement(goods);
}

void PlacementDialog::bottomleft()
{
  goods->placement = Bottom;
  goods->justification = Left;
  goods->preposition = Outside;
  highlightPlacement(goods);
}

void PlacementDialog::bottomcenter()
{
  goods->placement = Bottom;
  goods->justification = Center;
  goods->preposition = Outside;
  highlightPlacement(goods);
}

void PlacementDialog::bottomright()
{
  goods->placement = Bottom;
  goods->justification = Right;
  goods->preposition = Outside;
  highlightPlacement(goods);
}

void PlacementDialog::bottomRightO()
{
  goods->placement = BottomRight;
  goods->preposition = Outside;
  highlightPlacement(goods);
}
