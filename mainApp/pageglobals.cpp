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
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QGroupBox>
#include <QGridLayout>
#include <QWidget>
#include <QTabWidget>

#include <QDialog>
#include <QDialogButtonBox>

#include "globals.h"
#include "meta.h"
#include "metaitem.h"
#include "metagui.h"
#include "lpub_preferences.h"

/**********************************************************************
 *
 * Page
 *
 *********************************************************************/

class GlobalPagePrivate
{
public:
  Meta       meta;
  QString    topLevelFile;
  QList<MetaGui *> children;

  GlobalPagePrivate(QString &_topLevelFile, Meta &_meta)
  {
    topLevelFile = _topLevelFile;
    meta = _meta;

    MetaItem mi; // examine all the globals and then return

    mi.sortedGlobalWhere(meta,topLevelFile,"ZZZZZZZ");
  }
};

GlobalPageDialog::GlobalPageDialog(
  QString &topLevelFile, Meta &meta)
{
  data = new GlobalPagePrivate(topLevelFile,meta);

  setWindowTitle(tr("Page Globals Setup"));

  QTabWidget    *tab = new QTabWidget();
  QVBoxLayout   *layout = new QVBoxLayout();
  setLayout(layout);
  layout->addWidget(tab);
  
  QWidget       *widget;
  QGridLayout   *grid;
  
  widget = new QWidget();
  grid = new QGridLayout();
  widget->setLayout(grid);

  MetaGui   *child;
  QGroupBox *box;

  PageMeta *pageMeta = &data->meta.LPub.page;
  
  //~~~~~~~~~~~~ page tab ~~~~~~~~~~~~~~~~//
  box = new QGroupBox("Size");
  grid->addWidget(box,0,0);
  child = new UnitsGui("",&pageMeta->size,box);
  data->children.append(child);
 
  box = new QGroupBox("Margins");
  grid->addWidget(box,1,0);
  child = new UnitsGui("",&pageMeta->margin,box);
  data->children.append(child);

  box = new QGroupBox("Background");
  grid->addWidget(box, 2, 0);
  child = new BackgroundGui(&pageMeta->background,box);
  data->children.append(child);

  box = new QGroupBox("Border");
  grid->addWidget(box, 3, 0);
  child = new BorderGui(&pageMeta->border,box);
  data->children.append(child);

  tab->addTab(widget,"Page");
  
  //~~~~~~~~~~~~ model tab ~~~~~~~~~~~~~~~//
  widget = new QWidget();
  grid = new QGridLayout();
  widget->setLayout(grid);

  /*
    Title,
    Model ID,
    Pieces,
    Model Description
  */
  box = new QGroupBox("Title");
  grid->addWidget(box, 0, 0);
  child = new PageAttributeTextGui(&pageMeta->title,box);
  data->children.append(child);

  tab->addTab(widget,"Model");

  //~~~~~~~~~~~~ publisher tab ~~~~~~~~~~~~//
  widget = new QWidget();
  grid = new QGridLayout();
  widget->setLayout(grid);

  /*
    Author,
    Email,
    Author URL,
    Copyright
    Publish desription
    Logo
  */

  // this will go away cuz the chkbox will go into PageAttributePictureGui
  box = new QGroupBox("Display");
  grid->addWidget(box, 0, 0);
//  child = new CheckBoxGui("Display Logo",false,box);
//  data->children.append(child);
  //  child = new PageAttributePictureGui(&pageMeta->logo,box);
  //  data->children.append(child);

  tab->addTab(widget,"Publisher");

  //~~~~~~~~~~~~ disclaimer tab ~~~~~~~~~~~//
  widget = new QWidget();
  grid = new QGridLayout();
  widget->setLayout(grid);

  /*
    LEGO disclaimer,
    Plug
    Plug image
  */

  tab->addTab(widget,"Disclaimer");

  //~~~~~~~~~ page number tab ~~~~~~~~~~~~//

  widget = new QWidget();
  grid = new QGridLayout();
  widget->setLayout(grid);

  box = new QGroupBox("Display");
  grid->addWidget(box, 0,0);
  child = new CheckBoxGui("Display Page Number",&pageMeta->dpn,box);
  data->children.append(child);

  box = new QGroupBox("Look");
  grid->addWidget(box,1,0);
  child = new NumberGui(&pageMeta->number,box);
  data->children.append(child);

  box = new QGroupBox("Placement");
  grid->addWidget(box,2,0);
  child = new BoolRadioGui(
    "Alternate Corners (like books)",
    "Page Number Always in Same Place",
    &pageMeta->togglePnPlacement,box);
  data->children.append(child);

  tab->addTab(widget,"Page Number");
 
  QDialogButtonBox *buttonBox;

  buttonBox = new QDialogButtonBox();
  buttonBox->addButton(QDialogButtonBox::Ok);
  connect(buttonBox,SIGNAL(accepted()),SLOT(accept()));
  buttonBox->addButton(QDialogButtonBox::Cancel);
  connect(buttonBox,SIGNAL(rejected()),SLOT(cancel()));

  layout->addWidget(buttonBox);
  setModal(true);
}

void GlobalPageDialog::getPageGlobals(
  QString topLevelFile, Meta &meta)
{
  GlobalPageDialog *dialog = new GlobalPageDialog(topLevelFile,meta);
  dialog->exec();
}

void GlobalPageDialog::accept()
{
  MetaItem mi;

  mi.beginMacro("GlobalPage");

  MetaGui *child;
  foreach(child,data->children) {
    child->apply(data->topLevelFile);
  }
  mi.endMacro();

  QDialog::accept();
}

void GlobalPageDialog::cancel()
{
  QDialog::reject();
}
