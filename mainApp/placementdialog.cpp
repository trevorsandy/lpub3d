
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
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://www.trolltech.com/products/qt/licensing.html or contact the
** sales department at sales@trolltech.com.
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

bool PlacementDialog::getPlacement(
  PlacementType  parentType,
  PlacementType  placedType,
  PlacementData &goods,
  QString        title,
  int            onPageType,
  QWidget       *parent,
  bool           pliPerStep)
{
  PlacementDialog *dialog = new PlacementDialog(parentType,placedType,goods,title,onPageType,parent,pliPerStep);

  bool ok = dialog->exec() == QDialog::Accepted;
  if (ok) {
    goods.offsets[0] = 0;
    goods.offsets[1] = 0;
  }
  return ok;
}

QString labels[5][5] = 
{
  {"Top/Left",   "Top:Left",   "Top:Center",   "Top:Right",   "Top/Right" },
  {"Left:Top",   "Top/Left",   "Top",          "Top/Right",   "Right:Top" },
  {"Left:Center","Left",       "Center",       "Right",       "Right:Center" },
  {"Left:Bottom","Bottom/Left","Bottom",       "Bottom/Right","Right:Bottom" },
  {"Bottom/Left","Bottom:Left","Bottom:Center","Bottom:Right","Right/Bottom" },
};

const int PlacementDialog::relativeToOks[NumRelatives] =
{ //                    	      Page | Csi | Pli | Pn | Sn | Callout | ph | pf | Ms
  /*  0 Page 		  	page 	*/0,
  /*  1 Csi (assem) 	Csi		*/Page,
  /*  2 MultiStep       Ms 		*/Page       | Pli,
  /*  3 StepNum 		Sn      */Page | Csi | Pli                     | ph | pf,
  /*  4 Pli 			Pli     */Page | Csi            | Sn,
  /*  5 Callout 		Callout	*/Page | Csi | Pli      | Sn           | ph | pf,
  /*  6 PageNum 		Pn      */Page | Csi | Pli | Pn | Sn | Callout | ph | pf,

  /*  7 title           tt		*/Page ,
  /*  8 modelName       mnt		*/Page                                                                          |tt,
  /*  9 author          at		*/Page             | Pn                | ph | pf | tt |      ct | et | urlt,
  /* 10 url 			urlt	*/Page             | Pn                | ph | pf      | at | ct | et,
  /* 11 modelDesc       mdt		*/Page                                                                          | pt,
  /* 12 publishDesc 	pdt		*/Page                                                                          | mdt,
  /* 13 copyright       ct		*/Page             | Pn                | ph | pf      | at |      et | urlt,
  /* 14 email           et		*/Page             | Pn                | ph | pf      | at | ct |      urlt,
  /* 15 disclaimer  	dt		*/Page                                                          | et,
  /* 16 pieces          pt		*/Page                                                | at,
  /* 17 plug            plt		*/Page                                                                          | dt,
  /* 18 category        cat		*/Page,
  /* 19 documentLogo	dlt     */Page                                 | ph | pf,
  /* 20 coverImage  	cit		*/Page,
  /* 21 plugImage       pit		*/Page                                                                          | plt,
  /* 22 pageHeader      ph		*/Page,
  /* 23 pageFooter      pf		*/Page,
  /* 24 SingleStep              */Page | Csi,
  /* 25 SubmodelIns             */Page | Csi | Pli | Pn | Sn
};
//front cover options             Page     | ph | pf | tt | at                  | mnt | pt | mdt | pdt | dlt,
//back  cover options             Page     | ph | pf | tt | at | ct | et | urlt |                      | dlt | dt | plt | pit,

const int PlacementDialog::prepositionOks[NumRelatives] = // indexed by them
{
    /*  0 Page */        InsideOk,
    /*  1 Csi */         InsideOk|OutsideOk,
    /*  2 MultiStep */   OutsideOk,
    /*  3 StepNum */     OutsideOk,
    /*  4 Pli */         OutsideOk,
    /*  5 Callout */     OutsideOk,
    /*  6 PageNum */     OutsideOk,

    /*  7 title */       OutsideOk,
    /*  8 modelName */   OutsideOk,
    /*  9 author */      OutsideOk,
    /* 10 url */         OutsideOk,
    /* 11 modelDesc */   OutsideOk,
    /* 12 publishDesc */ OutsideOk,
    /* 13 copyright */   OutsideOk,
    /* 14 email */       OutsideOk,
    /* 15 disclaimer */  OutsideOk,
    /* 16 pieces */      OutsideOk,
    /* 17 plug */        OutsideOk,
    /* 18 category */    OutsideOk,
    /* 19 documentLogo */OutsideOk,
    /* 20 coverImage */  OutsideOk,
    /* 21 plugImage */   OutsideOk,
    /* 22 pageHeader */  InsideOk|OutsideOk,
    /* 23 pageFooter */  InsideOk|OutsideOk
};

const QString relativeNames[NumRelatives] =
{
  "Page",        				// 0 page
  "Assem",                      // 1 Csi
  "Step Group",                 // 2 Ms
  "Step Number",                // 3 Sn
  "Parts List",                 // 4 Pli
  "Callout",                    // 5 Call
  "Page Number",                // 6 pn

  "Title",                      // 7 tt
  "Model ID",                   // 8 mnt
  "Author",                     // 9 at
  "URL",                        //10 urlt
  "Model Description",          //11 mdt
  "Publish Description",        //12 pdt
  "Copyright",                  //13 ct
  "Email",                      //14 et
  "Disclaimer",                 //15 dt
  "Pieces",                     //16 pt
  "Plug",                       //17 plt
  "Category",                   //18 cat
  "Logo",                       //19 dlt
  "Cover Image",                //20 cit
  "Plug Image",                 //21 pit
  "Page Header",                //22 ph
  "Page Footer",                //23 pf

  "Single Step",                //24
  "Submodel Instance Count",    //25

  "Step",                       //27
  "Range",                      //28
  "Reserve",                    //29
  "BOM",                        //24
  "Cover Page",                 //30
  "Rotate Icon"                 //31
};                              //33 NumRelatives

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
  QWidget       *parent,
  bool           pliPerStep)
{
  goods = &_goods;

  QGridLayout *outsideGrid = new QGridLayout;
  QGridLayout *insideGrid  = new QGridLayout;
  QFrame      *insideFrame = new QFrame;

  lblRelativeTo = new QLabel("Relative To",parent);
  lblRelativeTo->setToolTip(tr("Select item that %1 will be moved relative to.").arg(title));
  outsideGrid->addWidget(lblRelativeTo,0,0);

  combo = new QComboBox;
  connect(combo,SIGNAL(activated(int)),this,SLOT(relativeToChanged(int)));
  outsideGrid->addWidget(combo,0,1);

  int oks;
  logTrace() << " \nPLACEMENT DIALOG "
             << " \nParentType: " << RelNames[parentType]     << " (" << parentType << ")"
             << " \nPlacedType: " << RelNames[placedType]     << " (" << placedType << ")"
             << " \nOnPageType: " << (onPageType == 0 ? "Content Page" : onPageType == 1 ? "Front Cover Page" : "Back Cover Page")
                ;
  switch (parentType) {
    case StepGroupType:                             //parent type
      switch (placedType) {
        case PageURLType:               //placed type
            oks =  Page | Pn | ph | pf;
        break;
        case PageEmailType:             //placed type
            oks =  Page | Pn | ph | pf |                     urlt;
        break;
        case PageAuthorType:            //placed type
            oks =  Page | Pn | ph | pf |                et | urlt;
        break;
        case PageCopyrightType:         //placed type
            oks =  Page | Pn | ph | pf |      at |      et | urlt;
        break;  
        case PartsListType:             //placed type
          if (pliPerStep) {
            oks = Csi | Sn;
          } else {
            oks = Page | Ms;
          }
        break;
        case StepNumberType:            //placed type
          oks = Csi | Pli;
        break;
        case CalloutType:               //placed type
          oks = Page | Csi | Pli | Sn;
        break;
        case SubmodelInstanceCountType: //placed type
          oks = Page | Pn;
        break;
        default:                        //placed type
          oks = Page | Pn;
        break;
      }
    break;
    case CalloutType:                               //parent type
      switch (placedType) {
        case PartsListType:             //placed type
          oks = Csi;
        break;
        case StepNumberType:            //placed type
          oks = Csi | Pli;
        break;
        case CalloutType:               //placed type
          oks = Csi | Pli | Sn | Callout;
        break;
        case SubmodelInstanceCountType: //placed type
        oks = Page | Pn;
        break;                          //placed type
        default:
          oks = Page | Csi | Pli | Sn;
        break;
      }
    break;
    case StepType:                                  //parent type
      switch (placedType) {
        case PartsListType:             //placed type
          oks = Page | Csi | Sn;
        break;
        case StepNumberType:            //placed type
          oks = Page | Csi | Pli | ph;
        break;
        case CalloutType:               //placed type
          oks = Page | Csi | Sn | Pli;
        break;
        case SubmodelInstanceCountType: //placed type
        oks = Page | Csi | Pli | Pn | Sn;
        break;
        default:                         //placed type
        oks = Page | Csi | Pli | Sn;
        break;
      }
    break;
  case SingleStepType:                              //parent type
      switch (placedType) {
      case PageURLType:                  //placed type
        if (onPageType == BackCoverPage) {
           oks = Page |                          ct;
         //oks = Page      | ph | pf | tt | at | ct | et;
        } else
           oks = Page | Pn | ph | pf;
      break;
      case PageEmailType:                //placed type
        if (onPageType == BackCoverPage) {
           oks = Page |                                    urlt;
         //oks = Page      | ph | pf | tt | at | ct |      urlt;
        } else
           oks = Page | Pn | ph | pf |                     urlt;
      break;
      case PageAuthorType:               //placed type
        if (onPageType == FrontCoverPage) {
           oks = Page |                tt;
         //oks = Page      | ph | pf | tt;
        } else if (onPageType == BackCoverPage) {
           oks = Page |                tt;
         //oks = Page      | ph | pf | tt |      ct | et | urlt;
        } else
           oks = Page | Pn | ph | pf |                et | urlt;
      break;
      case PageCopyrightType:           //placed type
        if (onPageType == BackCoverPage) {
           oks = Page |                     at;
         //oks = Page      | ph | pf | tt | at |      et | urlt;
        } else
           oks = Page | Pn | ph | pf |      at |      et | urlt;
      break;
      default:                          //placed type
        oks = relativeToOks[placedType];
      break;
      }
    break;
    default:                                            //parent type
      oks = relativeToOks[placedType];
    break;
  }

  int currentIndex = 0;

  for (int i = 0; i < NumRelatives; i++) {
//debug
//    logNotice() << "\n POPULATE PLACEMENT COMBO"
//                << "\n    Index: " << i   <<      " Bits: " << QString::number(i,2)
//                << "\n      Oks: " << oks <<      " Bits: " << QString::number(oks,2)
//                << "\n (1 << i): " << (1 << i) << " Bits: " << QString::number((1 << i),2)
//                   ;
//end debug
    if (oks & (1 << i)) {
//debug
//    qDebug() << " MATCH: " << i << " " << oks << " " << (1 << i)
//                 ;
//end debug
      combo->addItem(relativeNames[i]);
      if (i == goods->relativeTo) {
        currentIndex = combo->count()-1;
      }
    }
  }
  combo->setCurrentIndex(currentIndex);

  outsideGrid->addWidget(insideFrame,2,1,3,3);
  insideFrame->setLayout(insideGrid);
  insideFrame->setFrameShape(QFrame::StyledPanel);
  insideFrame->setFrameShadow(QFrame::Sunken);

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

  outsideGrid->addWidget(buttonBox,6,0,1,5);

  setLayout(outsideGrid);
  setWindowTitle(tr("%1 %2 Dialog").arg(QString::fromLatin1(VER_PRODUCTNAME_STR),title));
  setModal(true);

  setEnabled(prepositionOks[goods->relativeTo]);
  highlightPlacement(goods);
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
