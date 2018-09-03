/****************************************************************************
**
** Copyright (C) 2016 - 2018 Trevor SANDY. All rights reserved.
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

/****************************************************************************
 *
 * This class implements a dialog that lets the user pick the placement for
 * page pointers placing them relative to the page.
 *
 * Please see lpub.h for an overall description of how the files in LPub
 * make up the LPub program.
 *
 ***************************************************************************/

#include <QString>
#include <QGridLayout>
#include <QPushButton>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QFrame>
#include <QComboBox>

#include "pointerplacementdialog.h"
#include "metatypes.h"
#include "meta.h"
#include "version.h"

#include "QsLog.h"

#define NUM_REL 1

QString ppLabels[3][3] =
{
  {"Top/Left",   "Top",   "Top/Right"   },
  {"Left",       "Center","Right"       },
  {"Bottom/Left","Bottom","Bottom/Right"},
};

const QString ppRelativeNames[NUM_REL] =
{
  "Page", // 0 page
};        //1 NUM_REL [NumRelatives]

const int PointerPlacementDialog::relativeToOks[NUM_REL] =
{ //            Page
  /*  0 page */ 0,
};

const int PointerPlacementDialog::prepositionOks[NUM_REL] = // indexed by them
{
  /*  0 Page */ InsideOk,
};

bool PointerPlacementDialog::getPointerPlacement(
  PlacementType  parentType,
  PlacementType  placedType,
  PlacementData &goods,
  QString        title,
  QWidget       *parent)
{
  PointerPlacementDialog *dialog = new PointerPlacementDialog(parentType,placedType,goods,title,parent);

  bool ok = dialog->exec() == QDialog::Accepted;
  if (ok) {
    goods.offsets[0] = 0;
    goods.offsets[1] = 0;
  }
  return ok;
}

QString PointerPlacementDialog::relativeToName(
  int relativeTo)
{
  if (relativeTo >= NUM_REL) {
    return ppRelativeNames[0];
  } else {
    return ppRelativeNames[relativeTo];
  }
}

PointerPlacementDialog::PointerPlacementDialog(
  PlacementType  _parentType,
  PlacementType  _placedType,
  PlacementData &_goods,
  QString         title,
  QWidget        *parent)
{
  goods      = &_goods;
  parentType =  _parentType;
  placedType =  _placedType;

  outsideGrid    = new QGridLayout;
  horizontalLine = new QFrame;

  lblRelativeTo = new QLabel("Relative To",parent);
  lblRelativeTo->setToolTip(tr("Select item that %1 will be moved relative to.").arg(title));
  outsideGrid->addWidget(lblRelativeTo,0,0);

  combo = new QComboBox;
  connect(combo,SIGNAL(activated(int)),this,SLOT(relativeToChanged(int)));
  outsideGrid->addWidget(combo,0,1);

//  logTrace() << " \nPLACEMENT DIALOG "
//             << " \nParentType: " << RelNames[parentType]     << " (" << parentType << ")"
//             << " \nPlacedType: " << RelNames[placedType]     << " (" << placedType << ")"
//                ;
  int oks;
  switch (parentType) {
    case StepGroupType:                             //parent type
      switch (placedType) {
        default:                        //placed type
          oks = Page;
          break;
        }
      break;
    case CalloutType:                               //parent type
      switch (placedType) {
        default:
          oks = Page;
          break;
        }
      break;
    case StepType:                                  //parent type
      switch (placedType) {
        default:                        //placed type
          oks = Page;
          break;
        }
      break;
    case CsiType:                                  //parent type
      switch (placedType) {
        default:                         //placed type
          oks = relativeToOks[placedType];
          break;
        }
      break;
    case SingleStepType:                           //parent type
      switch (placedType) {
        default:                         //placed type
          oks = relativeToOks[placedType];
          break;
        }
      break;
    default:                                       //parent type
      oks = relativeToOks[placedType];
      break;
    }

  int currentIndex = 0;

  for (int i = 0; i < NUM_REL; i++) {
    if (oks & (1 << i)) {
      combo->addItem(ppRelativeNames[i]);
      if (i == goods->relativeTo) {
        currentIndex = combo->count()-1;
      }
    }
  }
  combo->setCurrentIndex(currentIndex);

  horizontalLine->setFrameShape(QFrame::HLine);
  horizontalLine->setFrameShadow(QFrame::Sunken);
  outsideGrid->addWidget(horizontalLine,4,0,1,3);

  for (int y = 0; y < 3; y++) {
    for (int x = 0; x < 3; x++) {
      QPushButton *button;
      button = new QPushButton(ppLabels[y][x],parent);
      buttons[y][x] = button;

      if (y == 0 || y == 2) {
        outsideGrid->addWidget(button,y+1,x);
      } else if (x == 0 || x == 2) {
        outsideGrid->addWidget(button,y+1,x);
      } else {
        outsideGrid->addWidget(button,y+1,x);
      }

      switch (y) {
        case 0:
          switch (x) {
            case 0:
              connect(button,SIGNAL(clicked()),this,SLOT(topLeftI()));
            break;
            case 1:
              connect(button,SIGNAL(clicked()),this,SLOT(top()));
            break;
            case 2:
              connect(button,SIGNAL(clicked()),this,SLOT(topRightI()));
            break;
          }
        break;
        case 1:
          switch (x) {
            case 0:
              connect(button,SIGNAL(clicked()),this,SLOT(left()));
            break;
            case 1:
              connect(button,SIGNAL(clicked()),this,SLOT(center()));
            break;
            case 2:
              connect(button,SIGNAL(clicked()),this,SLOT(right()));
            break;
          }
        break;
        case 2:
          switch (x) {
            case 0:
              connect(button,SIGNAL(clicked()),this,SLOT(bottomLeftI()));
            break;
            case 1:
              connect(button,SIGNAL(clicked()),this,SLOT(bottom()));
            break;
            case 2:
              connect(button,SIGNAL(clicked()),this,SLOT(bottomRightI()));
            break;
          }
        break;
      }
    }
  }

  buttonBox = new QDialogButtonBox();
  buttonBox->addButton(QDialogButtonBox::Ok);
  connect(buttonBox,SIGNAL(accepted()),SLOT(accept()));
  buttonBox->addButton(QDialogButtonBox::Cancel);
  connect(buttonBox,SIGNAL(rejected()),SLOT(reject()));

  outsideGrid->addWidget(buttonBox,5,0,1,3);

  setLayout(outsideGrid);
  setWindowTitle(tr("%1 %2 Dialog").arg(QString::fromLatin1(VER_PRODUCTNAME_STR),title));
  setModal(true);

  setEnabled(prepositionOks[goods->relativeTo]);
  highlightPlacement(goods);
}

PointerPlacementDialog::~PointerPlacementDialog()
{
}

void PointerPlacementDialog::relativeToChanged(int /* unused */)
{
  QString name = combo->currentText();

  for (int i = 0; i < NUM_REL; i++) {
    if (name == ppRelativeNames[i]) {
      setEnabled(prepositionOks[i]);
      goods->relativeTo = PlacementType(i);
      goods->offsets[0] = 0;
      goods->offsets[1] = 0;
      break;
    }
  }
}

void PointerPlacementDialog::setEnabled(int okPrepositions)
{
  for (int y = 0; y < 3; y++) {
    for (int x = 0; x < 3; x++) {
      buttons[y][x]->setEnabled(false);
    }
  }
  for (int y = 0; y < 3; y++) {
    for (int x = 0; x < 3; x++) {
        if ((parentType == StepType ||
             parentType == CsiType  ||
             parentType == SingleStepType ||
             parentType == StepGroupType) &&
             ((y == 0 && x == 0) ||
              (y == 2 && x == 0) ||
              (y == 1 && x == 1) ||
              (y == 0 && x == 2) ||
              (y == 2 && x == 2))){
          if (okPrepositions & InsideOk) {
              buttons[y][x]->setVisible(false);
          }
        } else {
          if (okPrepositions & InsideOk) {
            buttons[y][x]->setEnabled(true);
        }
      }
    }
  }
}

int ppInsideDecode[NumPlacements][2] =
{
  { 0, 0 }, { 0, 1 }, { 0, 2 },
  { 1, 2 }, { 2, 2,}, { 2, 1 },
  { 2, 0 }, { 1, 0 }, { 1, 1 }
};

void PointerPlacementDialog::highlightPlacement(
  PlacementData *goods)
{
  int r = 0, c = 0;

  if (goods->preposition == Inside) {
    r = ppInsideDecode[goods->placement][0];
    c = ppInsideDecode[goods->placement][1];
  }
  QFont font;
  for (int R = 0; R < 3; R++) {
    for (int C = 0; C < 3; C++) {
      font = buttons[R][C]->font();
      font.setBold(R == r && C == c);
      buttons[R][C]->setFont(font);
    }
  }
}

/*************************/

void PointerPlacementDialog::topLeftI()
{
  goods->placement     = TopLeft;
  goods->preposition   = Inside;
  goods->rectPlacement = TopLeftInsideCorner;
  highlightPlacement(goods);
}

void PointerPlacementDialog::top()
{
  goods->placement     = Top;
  goods->justification = Center;
  goods->preposition   = Inside;
  goods->rectPlacement = TopInside;
  highlightPlacement(goods);
}

void PointerPlacementDialog::topRightI()
{
  goods->placement     = TopRight;
  goods->preposition   = Inside;
  goods->rectPlacement = TopRightInsideCorner;
  highlightPlacement(goods);
}

/*************************/

void PointerPlacementDialog::left()
{
  goods->placement     = Left;
  goods->justification = Center;
  goods->preposition   = Inside;
  goods->rectPlacement = LeftInside;
  highlightPlacement(goods);
}

void PointerPlacementDialog::center()
{
  goods->placement     = Center;
  goods->justification = Center;
  goods->preposition   = Inside;
  goods->rectPlacement = CenterCenter;
  highlightPlacement(goods);
}

void PointerPlacementDialog::right()
{
  goods->placement     = Right;
  goods->preposition   = Inside;
  goods->rectPlacement = RightInside;
  highlightPlacement(goods);
}

/*************************/

void PointerPlacementDialog::bottomLeftI()
{
  goods->placement     = BottomLeft;
  goods->preposition   = Inside;
  goods->rectPlacement = BottomLeftInsideCorner;
  highlightPlacement(goods);
}

void PointerPlacementDialog::bottom()
{
  goods->placement     = Bottom;
  goods->justification = Center;
  goods->preposition   = Inside;
  goods->rectPlacement = BottomInside;
  highlightPlacement(goods);
}

void PointerPlacementDialog::bottomRightI()
{
  goods->placement     = BottomRight;
  goods->preposition   = Inside;
  goods->rectPlacement = BottomRightInsideCorner;
  highlightPlacement(goods);
}

/*************************/
