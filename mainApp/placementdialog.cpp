 
/****************************************************************************
**
** Copyright (C) 2007-2009 Kevin Clague. All rights reserved.
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

#include <QString>
#include <QGridLayout>
#include <QPushButton>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QFrame>
#include <QComboBox>

bool PlacementDialog::getPlacement(
  PlacementType  parentType,
  PlacementType  placedType,
  PlacementData &goods,
  QString        title,
  QWidget       *parent,
  bool           pliPerStep)
{
  PlacementDialog *dialog = new PlacementDialog(parentType,placedType,goods,title,parent,pliPerStep);

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
{
    /* Page */        0,
    /* Csi  */        Page,
    /* MultiStep */   Page | Pli,
    /* StepNum */     Page | Csi | Pli,
    /* Pli */         Page | Csi | Ms | Sn,
    /* Callout */     Page | Csi | Ms | Sn | Pli | Callout,
    /* PageNum */     Page,
    /* SingleStep */  Page | Csi,
    /* SubmodelIns*/  Page |                  Pn,
    /* title */       Page | Csi | Pli |  mnt | mdt | pdt | at  | pt   | cat | dlt | cit,
    /* modelName */   Page | Csi | Pli |  tt  | mdt | pdt | at  | pt   | cat | dlt | cit,
    /* modelDesc */   Page | Csi | Pli |  tt  | mnt | pdt | at  | pt   | cat | dlt | cit,
    /* publishDesc */ Page | Csi | Pli |  tt  | mnt | mdt | at  | pt   | cat | dlt | cit,
    /* author */      Page | Csi | Pli |  tt  | mnt | mdt | pdt | pt   | cat | dlt | cit | urlt,
    /* pieces */      Page | Csi | Pli |  tt  | mnt | mdt | pdt | at   | cat | dlt | cit,
    /* category */    Page | Csi | Pli |  tt  | mnt | mdt | pdt | at   | pt  | dlt | cit,
    /* url */         Page | Csi | Pli |  et  | ct  | dt  | plt | pit  | at,
    /* email */       Page | Csi | Pli |  ct  | dt  | plt | pit | urlt | at,
    /* copyright */   Page | Csi | Pli |  et  | dt  | plt | pit | urlt | at,
    /* disclaimer */  Page | Csi | Pli |  et  | ct  | plt | pit | urlt | at,
    /* documentLogo */Page | Csi | Pli |  tt  | mnt | pdt | at  | pt   | cat | mdt | cit,
    /* coverImage */  Page | Csi | Pli |  tt  | mnt | pdt | at  | pt   | cat | dlt | mdt,
    /* plug */        Page | Csi | Pli |  et  | ct  | dt  | pit | urlt | at,
    /* plugImage */   Page | Csi | Pli |  et  | ct  | plt | dt  | urlt | at
};

const int PlacementDialog::prepositionOks[NumRelatives] = // indexed by them
{
    /* Page */        InsideOk,
    /* Csi */         InsideOk|OutsideOk,
    /* MultiStep */   OutsideOk,
    /* StepNum */     OutsideOk,
    /* Pli */         OutsideOk,
    /* Callout */     OutsideOk,
    /* PageNum */     OutsideOk,
    /* title */       OutsideOk,
    /* modelName */   OutsideOk,
    /* modelDesc */   OutsideOk,
    /* publishDesc */ OutsideOk,
    /* author */      OutsideOk,
    /* pieces */      OutsideOk,
    /* category */    OutsideOk,
    /* url */         OutsideOk,
    /* email */       OutsideOk,
    /* copyright */   OutsideOk,
    /* disclaimer */  OutsideOk,
    /* documentLogo */OutsideOk,
    /* coverImage */  OutsideOk,
    /* plug */        OutsideOk,
    /* plugImage */   OutsideOk
};

const QString relativeNames[NumRelatives] =
{
  "Page",          "Assem",      "Step Group",  "Step Number",
  "Parts List",    "Callout",    "Page Number",
  "Title",         "Model ID",   "Author",      "URL",         "Model Description", "Publish Description",
  "Copyright",     "Email",      "Disclaimer",  "Pieces",      "Plug",              "Category",
  "Logo",          "Cover Image","Plug Image",
  "Single Step",   "Submodel Instance Count",   "Step",        "Range",             "Reserve",
  "BOM",           "Cover Page",                "Front Cover", "Back Cover"
};

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
  QWidget       *parent,
  bool           pliPerStep)
{
  goods = &_goods;

  QGridLayout *outsideGrid = new QGridLayout;
  QGridLayout *insideGrid  = new QGridLayout;
  QFrame      *insideFrame = new QFrame;

  combo = new QComboBox;
  connect(combo,SIGNAL(activated(int)),this,SLOT(relativeToChanged(int)));
  outsideGrid->addWidget(combo,0,0);

  int oks;

  switch (parentType) {
    case StepGroupType:
      switch (placedType) {
        case PartsListType:
          if (pliPerStep) {
            oks = Csi | Sn;
          } else {
            oks = Page | Ms;
          }
        break;
        case StepNumberType:
          oks = Csi | Pli;
        break;
        case CalloutType:
          oks = Page | Csi | Pli | Sn | Ms;
        break;
        default:
          oks = Csi | Pli | Sn;
        break;
      }
    break;
    case CalloutType:
      switch (placedType) {
        case PartsListType:
          oks = Csi;
        break;
        case StepNumberType:
          oks = Csi | Pli;
        break;
        case CalloutType:
          oks = Csi | Pli | Sn | Callout;
        break;
        default:
          oks = Csi | Pli | Sn;
        break;
      }
    break;
    case StepType:
      switch (placedType) {
        case PartsListType:
          oks = Page | Csi | Sn;
        break;
        case StepNumberType:
          oks = Page | Csi | Pli;
        break;
        case CalloutType:
          oks = Page | Csi | Sn | Pli;
        break;
        default:
          oks = Csi | Pli | Sn;
        break;
      }
    break;
    default:
      oks = relativeToOks[placedType];
    break;
  }

  int currentIndex = 0;

  for (int i = 0; i < NumRelatives; i++) {
    if (oks & (1 << i)) {
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

  buttonBox = new QDialogButtonBox(this);
  buttonBox->addButton(QDialogButtonBox::Ok);
  connect(buttonBox,SIGNAL(accepted()),SLOT(accept()));
  buttonBox->addButton(QDialogButtonBox::Cancel);
  connect(buttonBox,SIGNAL(rejected()),SLOT(reject()));

  outsideGrid->addWidget(buttonBox,6,0,1,5);

  setLayout(outsideGrid);
  setWindowTitle(tr("LPub: %1 Dialog") .arg(title));
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
