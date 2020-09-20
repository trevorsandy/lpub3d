 
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
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

/****************************************************************************
 *
 * This file implements a dialog that lets the user configure global settings
 * for assembly images (see global.h for the class declarations)
 *
 * Please see lpub.h for an overall description of how the files in LPub
 * make up the LPub program.
 *
 ***************************************************************************/

#include <QDialog>
#include <QGridLayout>
#include <QGroupBox>
#include <QDialogButtonBox>

#include "globals.h"
#include "meta.h"
#include "metaitem.h"
#include "metagui.h"
#include "name.h"

class GlobalAssemPrivate
{
public:
  Meta       meta;
  QString    topLevelFile;
  QList<MetaGui *> children;
  bool     clearCache;

  GlobalAssemPrivate(QString &_topLevelFile, Meta &_meta)
  {
    topLevelFile = _topLevelFile;
    meta = _meta;
    clearCache   = false;

    MetaItem mi; // examine all the globals and then return

    mi.sortedGlobalWhere(meta,topLevelFile,"ZZZZZZZ");
  }
};

GlobalAssemDialog::GlobalAssemDialog(
  QString &topLevelFile, Meta &meta)
{
  data = new GlobalAssemPrivate(topLevelFile,meta);

  setWindowTitle(tr("Assembly Globals Setup"));

  QTabWidget  *tab = new QTabWidget(nullptr);
  QVBoxLayout *layout = new QVBoxLayout(nullptr);
  QGridLayout *boxGrid = new QGridLayout();
  QVBoxLayout *childlayout = new QVBoxLayout(nullptr);
   QSpacerItem *vSpacer = new QSpacerItem(1,1,QSizePolicy::Fixed,QSizePolicy::Expanding);

  setLayout(layout);
  layout->addWidget(tab);

  QWidget *widget;
  QVBoxLayout *vlayout;
  MetaGui *child;
  QGroupBox *box;

  AssemMeta *assem = &data->meta.LPub.assem;

  /*
   * Contents tab
   */

  widget = new QWidget(nullptr);
  vlayout  = new QVBoxLayout(nullptr);
  widget->setLayout(vlayout);

  box = new QGroupBox("Assembly");
  vlayout->addWidget(box);
  box->setLayout(boxGrid);

  // Scale
  child = new DoubleSpinGui("Scale",
                            &assem->modelScale,
                            assem->modelScale._min,
                            assem->modelScale._max,
                            0.01f);
  data->children.append(child);
  data->clearCache = child->modified;
  boxGrid->addWidget(child,0,0);

  child = new UnitsGui("Margins L/R|T/B",&assem->margin);
  data->children.append(child);
  boxGrid->addWidget(child,1,0);

  /* Assembly camera settings */

  box = new QGroupBox("Default Assembly Orientation");
  vlayout->addWidget(box);
  boxGrid = new QGridLayout();
  box->setLayout(boxGrid);

  // camera field of view
  child = new DoubleSpinGui("Camera FOV",
                            &assem->cameraFoV,
                            assem->cameraFoV._min,
                            assem->cameraFoV._max,
                            assem->cameraFoV.value());
  data->children.append(child);
  boxGrid->addWidget(child,0,0,1,2);

  // view angles
  child = new FloatsGui("Latitude","Longitude",&assem->cameraAngles);
  data->children.append(child);
  boxGrid->addWidget(child,1,0);

  /* Step Number */
  box = new QGroupBox("Step Number");
  vlayout->addWidget(box);
  NumberPlacementMeta *stepNumber = &data->meta.LPub.stepNumber;
  child = new NumberGui(stepNumber,box);
  data->children.append(child);

  tab->addTab(widget,"Contents");

  /*
   * Display tab
   */

  bool enableAnnotations = data->meta.LPub.pli.annotation.display.value();
  bool fixedAnnotations  = data->meta.LPub.pli.annotation.fixedAnnotations.value();

  widget = new QWidget(nullptr);
  vlayout = new QVBoxLayout(nullptr);
  widget->setLayout(vlayout);

  box = new QGroupBox("Step");
  vlayout->addWidget(box);
  box->setLayout(childlayout);

  child = new CheckBoxGui("Step Number",&assem->showStepNumber);
  data->children.append(child);
  childlayout->addWidget(child);

  box = new QGroupBox("Annotation");
  vlayout->addWidget(box);
  childlayout = new QVBoxLayout();
  box->setLayout(childlayout);

  child = new CsiAnnotationGui("",&assem->annotation,nullptr,fixedAnnotations);
  data->children.append(child);
  childlayout->addWidget(child);
  box->setEnabled(enableAnnotations);
  if (!enableAnnotations)
      box->setToolTip("'Display Part List (PLI) Annotations' must be enabled to set Assembly (CSI) Part annotation.");

  box = new QGroupBox("Studs");
  vlayout->addWidget(box);
  child = new ComboGui("Display Stud Logo","Logo1|Logo2|Logo3|Logo4|Logo5",&assem->studLogo,box,true);
  data->children.append(child);

  //spacer

  vlayout->addSpacerItem(vSpacer);

  tab->addTab(widget,"Display");

  QDialogButtonBox *buttonBox;

  buttonBox = new QDialogButtonBox(this);
  buttonBox->addButton(QDialogButtonBox::Ok);
  connect(buttonBox,SIGNAL(accepted()),SLOT(accept()));
  buttonBox->addButton(QDialogButtonBox::Cancel);
  connect(buttonBox,SIGNAL(rejected()),SLOT(cancel()));

  layout->addWidget(buttonBox);

  setModal(true);
  setMinimumSize(40,20);
}

void GlobalAssemDialog::getAssemGlobals(
  QString topLevelFile, Meta &meta)
{
  GlobalAssemDialog *dlg = new GlobalAssemDialog(topLevelFile, meta);

  dlg->exec();
}

void GlobalAssemDialog::accept()
{
  MetaItem mi;

  if (data->clearCache) {
    clearCsiCache();
    clearTempCache();
  }

  mi.beginMacro("Global Assem");

  MetaGui *child;

  foreach(child,data->children) {
    child->apply(data->topLevelFile);
  }
  mi.endMacro();

  QDialog::accept();
}

void GlobalAssemDialog::cancel()
{
  QDialog::reject();
}
