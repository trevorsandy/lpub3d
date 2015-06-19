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

  QTabWidget    *tab         = new QTabWidget();
  QTabWidget    *childtab    = new QTabWidget();
  QVBoxLayout   *layout      = new QVBoxLayout();
  QVBoxLayout   *childlayout = new QVBoxLayout();

  setLayout(layout);
  layout->addWidget(tab);
  
  QWidget       *widget;
  QWidget       *childwidget;
  QGridLayout   *grid;
  
  widget = new QWidget();
  grid = new QGridLayout();
  widget->setLayout(grid);

  MetaGui   *child;
  QGroupBox *box;

  PageMeta *pageMeta = &data->meta.LPub.page;
  
  //~~~~~~~~~~~~ page tab ~~~~~~~~~~~~~~~~//
  box = new QGroupBox(tr("Size"));
  grid->addWidget(box,0,0);
  child = new UnitsGui("",&pageMeta->size,box);
  data->children.append(child);
 
  box = new QGroupBox(tr("Margins"));
  grid->addWidget(box,1,0);
  child = new UnitsGui("",&pageMeta->margin,box);
  data->children.append(child);

  box = new QGroupBox(tr("Background"));
  grid->addWidget(box, 2, 0);
  child = new BackgroundGui(&pageMeta->background,box);
  data->children.append(child);

  box = new QGroupBox(tr("Border"));
  grid->addWidget(box, 3, 0);
  child = new BorderGui(&pageMeta->border,box);
  data->children.append(child);

  tab->addTab(widget,"Page");
  
  //~~~~~~~~~~~~ model tab ~~~~~~~~~~~~~~~//  
  childwidget = new QWidget();                  //START DO THIS FOR MODEL, PUBLISHER AND DISCLAIMER
  childlayout = new QVBoxLayout;                //new QVBox layout - to apply tabl later
  childwidget->setLayout(childlayout);          //new 'model' widget - tab not yet added

  childtab = new QTabWidget();                  // new tab object
  childlayout->addWidget(childtab);             // new add the tab
  tab->addTab(childwidget, tr("Model"));            // new add the tab  (This is the new 'Model' tab
                                                // END
  // child header (one) start
  widget = new QWidget();                       // existig 'model' widget definintion
  grid = new QGridLayout();
  widget->setLayout(grid);
  // child header end
  /*
    Title,
    Cover Image
  */ 
  //child body (many) start
  box = new QGroupBox(tr("Display Title"));
  grid->addWidget(box, 0, 0);
  child = new PageAttributeTextGui(&pageMeta->title,box);
  data->children.append(child);
  //child body end

  //child body (many) start
  box = new QGroupBox(tr("Display Cover Image"));
  grid->addWidget(box, 1, 0);
  child = new PageAttributePictureGui(&pageMeta->coverImage,box);
  data->children.append(child);
  //child body end

  // child footer (one) end
  childtab->addTab(widget,"Title/Cover Image");         //new 'childModel' added to childtab
  // child footer end

  // child header (one) start
  widget = new QWidget();
  grid = new QGridLayout();
  widget->setLayout(grid);
  // child header end
  /*
    Description,
    Description Dialog,
  */
  // child body (many) start
  box = new QGroupBox(tr("Display Model Description"));
  grid->addWidget(box, 0, 0);
  child = new PageAttributeTextGui(&pageMeta->modelDesc,box);
  data->children.append(child);
  // child body end

  // child footer (one) end
  childtab->addTab(widget,tr("Description"));
  // child footer end


  // child header (one) start
  widget = new QWidget();
  grid = new QGridLayout();
  widget->setLayout(grid);
  // child header end
  /*
    Model ID,
    Pieces,
  */
  // child body (many) start
  box = new QGroupBox(tr("Display Model Identification"));
  grid->addWidget(box, 0, 0);
  child = new PageAttributeTextGui(&pageMeta->modelName,box);
  data->children.append(child);
  // child body end

  // child body (many) start
  box = new QGroupBox(tr("Display Pieces"));
  grid->addWidget(box, 1, 0);
  child = new PageAttributeTextGui(&pageMeta->pieces,box);
  data->children.append(child);
  // child body end

  // child footer (one) end
  childtab->addTab(widget,tr("Model ID/Pieces"));
  // child footer end

  //~~~~~~~~~~~~ publisher tab ~~~~~~~~~~~~//
  childwidget = new QWidget();
  childlayout = new QVBoxLayout;
  childwidget->setLayout(childlayout);

  childtab = new QTabWidget();
  childlayout->addWidget(childtab);
  tab->addTab(childwidget, tr("Publisher"));

  widget = new QWidget();
  grid = new QGridLayout();
  widget->setLayout(grid);
  /*
    Author,
    Email,
  */
  box = new QGroupBox(tr("Display Author"));
  grid->addWidget(box, 0, 0);
  child = new PageAttributeTextGui(&pageMeta->author,box);
  data->children.append(child);

  box = new QGroupBox(tr("Display Email"));
  grid->addWidget(box, 1, 0);
  child = new PageAttributeTextGui(&pageMeta->email,box);
  data->children.append(child);

  childtab->addTab(widget,tr("Author/Email"));

  widget = new QWidget();
  grid = new QGridLayout();
  widget->setLayout(grid);
  /*
    Publish desription
    URL
  */
  box = new QGroupBox(tr("Display URL"));
  grid->addWidget(box, 0, 0);
  child = new PageAttributeTextGui(&pageMeta->url,box);
  data->children.append(child);

  box = new QGroupBox(tr("Publisher Description"));
  grid->addWidget(box, 1, 0);
  child = new PageAttributeTextGui(&pageMeta->publishDesc,box);
  data->children.append(child);

  childtab->addTab(widget,tr("URL/Description"));

  widget = new QWidget();
  grid = new QGridLayout();
  widget->setLayout(grid);
  /*
    Copyright
    Logo
  */
  box = new QGroupBox(tr("Display Copyright"));
  grid->addWidget(box, 0, 0);
  child = new PageAttributeTextGui(&pageMeta->copyright,box);
  data->children.append(child);

  box = new QGroupBox(tr("Display Logo"));
  grid->addWidget(box, 1, 0);
  child = new PageAttributePictureGui(&pageMeta->documentLogo,box);
  data->children.append(child);

  childtab->addTab(widget,tr("Copyright/Logo"));

  //~~~~~~~~~~~~ disclaimer tab ~~~~~~~~~~~//
  childwidget = new QWidget();
  childlayout = new QVBoxLayout;
  childwidget->setLayout(childlayout);

  childtab = new QTabWidget();
  childlayout->addWidget(childtab);
  tab->addTab(childwidget,tr("Disclaimer"));

  widget = new QWidget();
  grid = new QGridLayout();
  widget->setLayout(grid);
  /*
    LEGO disclaimer,
  */
  box = new QGroupBox(tr("Display LEGO Disclaimer"));
  grid->addWidget(box, 0, 0);
  child = new PageAttributeTextGui(&pageMeta->disclaimer,box);
  data->children.append(child);

  //empty box
//  box = new QGroupBox(NULL);
//  grid->addWidget(box, 1,0);

  childtab->addTab(widget,tr("Disclaimer"));

  widget = new QWidget();
  grid = new QGridLayout();
  widget->setLayout(grid);
  /*
    Plug
    Plug image
  */
  box = new QGroupBox(tr("Display LPub3D Plug"));
  grid->addWidget(box, 1, 0);
  child = new PageAttributeTextGui(&pageMeta->plug,box);
  data->children.append(child);

  box = new QGroupBox(tr("Display LPub3D Logo"));
  grid->addWidget(box, 2, 0);
  child = new PageAttributePictureGui(&pageMeta->plugImage,box);
  data->children.append(child);

  childtab->addTab(widget,tr("LPub3D PLug"));

  //~~~~~~~~~ page number tab ~~~~~~~~~~~~//
  widget = new QWidget();
  grid = new QGridLayout();
  widget->setLayout(grid);

  box = new QGroupBox(tr("Display"));
  grid->addWidget(box, 0,0);
  child = new CheckBoxGui(tr("Display Page Number"),&pageMeta->dpn,box);
  data->children.append(child);

  box = new QGroupBox(tr("Look"));
  grid->addWidget(box,1,0);
  child = new NumberGui(&pageMeta->number,box);
  data->children.append(child);

  box = new QGroupBox(tr("Placement"));
  grid->addWidget(box,2,0);
  child = new BoolRadioGui(
    "Alternate Corners (like books)",
    "Page Number Always in Same Place",
    &pageMeta->togglePnPlacement,box);
  data->children.append(child);

  tab->addTab(widget,tr("Page Number"));
 
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
