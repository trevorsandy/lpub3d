 
/****************************************************************************
**
** Copyright (C) 2007-2009 Kevin Clague. All rights reserved.
** Copyright (C) 2015 - 2020 Trevor SANDY. All rights reserved.
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

#include "globals.h"
#include <QWidget>
#include <QDialogButtonBox>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QGroupBox>

#include "meta.h"
#include "metagui.h"
#include "metaitem.h"

class GlobalCalloutPrivate
{
public:
  Meta       meta;
  QString    topLevelFile;
  QList<MetaGui *> children;
  bool     clearCache;

  GlobalCalloutPrivate(QString &_topLevelFile, Meta &_meta)
  {
    topLevelFile = _topLevelFile;
    meta         = _meta;
    clearCache   = false;

    MetaItem mi; // examine all the globals and then return

    mi.sortedGlobalWhere(meta,topLevelFile,"ZZZZZZZ");
  }
};

GlobalCalloutDialog::GlobalCalloutDialog(
   QString &topLevelFile,
   Meta &meta)
{
  data = new GlobalCalloutPrivate(topLevelFile, meta);

  setWindowTitle(tr("Callout Globals Setup"));

  QTabWidget  *tab = new QTabWidget();
  QVBoxLayout *layout = new QVBoxLayout();

  setLayout(layout);
  layout->addWidget(tab);

  QVBoxLayout *vlayout;
  QSpacerItem *vSpacer;

  QWidget *widget;
  MetaGui *child;
  QGroupBox *box;

  CalloutMeta *calloutMeta = &data->meta.LPub.callout;

  /*
   * Background tab
   */
  widget = new QWidget();
  vlayout = new QVBoxLayout(nullptr);
  widget->setLayout(vlayout);

  box = new QGroupBox("Background");
  vlayout->addWidget(box);
  child = new BackgroundGui(&calloutMeta->background,box);
  data->children.append(child);

  box = new QGroupBox("Border");
  vlayout->addWidget(box);
  child = new BorderGui(&calloutMeta->border,box);
  data->children.append(child);
  
  box = new QGroupBox("Margins");
  vlayout->addWidget(box);
  child = new UnitsGui("L/R|T/B",&calloutMeta->margin,box);
  data->children.append(child);
  
  box = new QGroupBox("Divider");
  vlayout->addWidget(box);
  child = new SepGui(&calloutMeta->sep,box);
  data->children.append(child);

  tab->addTab(widget,"Background/Border");

  /*
   * Contents tab
   */
  widget = new QWidget();
  vlayout = new QVBoxLayout(nullptr);
  widget->setLayout(vlayout);
  
  QTabWidget *childtab = new QTabWidget();
  vlayout->addWidget(childtab);
  tab->addTab(widget, "Contents");

  /*
   * Callout Tab
   */
  QHBoxLayout *childlayout = new QHBoxLayout();
  widget = new QWidget(nullptr);
  vlayout = new QVBoxLayout(nullptr);
  widget->setLayout(vlayout);
  
  box = new QGroupBox("Parts List");
  vlayout->addWidget(box);
  box->setLayout(childlayout);

  child = new CheckBoxGui("Per Step",&calloutMeta->pli.perStep);
  data->children.append(child);
  childlayout->addWidget(child);

  child = new UnitsGui("Margins L/R|T/B",&calloutMeta->pli.margin);
  data->children.append(child);
  childlayout->addWidget(child);

  box = new QGroupBox("Step Justification");
  vlayout->addWidget(box);
  child = new JustifyStepGui("Set step justification",&calloutMeta->justifyStep,box);
  data->children.append(child);

  box = new QGroupBox("Step Number");
  vlayout->addWidget(box);
  child = new NumberGui(&calloutMeta->stepNum,box);
  data->children.append(child);

  box = new QGroupBox("Times Used");
  vlayout->addWidget(box);
  child = new NumberGui(&calloutMeta->instance,box);
  data->children.append(child);

  //spacer
  vSpacer = new QSpacerItem(1,1,QSizePolicy::Fixed,QSizePolicy::Expanding);
  vlayout->addSpacerItem(vSpacer);

  childtab->addTab(widget,"Callout");

  /*
   * CSI Assembly Tab
   */
  QGridLayout *boxGrid = new QGridLayout();
  widget = new QWidget(nullptr);
  vlayout = new QVBoxLayout(nullptr);
  widget->setLayout(vlayout);

  box = new QGroupBox("Assembly Image");
  vlayout->addWidget(box);
  box->setLayout(boxGrid);

  // Scale
  child = new DoubleSpinGui("Scale",
                            &calloutMeta->csi.modelScale,
                            calloutMeta->csi.modelScale._min,
                            calloutMeta->csi.modelScale._max,
                            0.01f);
  data->children.append(child);
  boxGrid->addWidget(child,0,0);

  data->clearCache = (data->clearCache ? data->clearCache : child->modified);

  child = new UnitsGui("Margins L/R|T/B",&calloutMeta->csi.margin);
  data->children.append(child);
  boxGrid->addWidget(child,1,0);

  /* Assembly camera settings */

  box = new QGroupBox("Default Assembly Orientation");
  vlayout->addWidget(box);
  boxGrid = new QGridLayout();
  box->setLayout(boxGrid);

  // camera field of view
  child = new DoubleSpinGui("Camera FOV",
                            &calloutMeta->csi.cameraFoV,
                            calloutMeta->csi.cameraFoV._min,
                            calloutMeta->csi.cameraFoV._max,
                            calloutMeta->csi.cameraFoV.value());
  data->children.append(child);
  data->clearCache = (data->clearCache ? data->clearCache : child->modified);
  boxGrid->addWidget(child,0,0,1,2);

  // view angles
  child = new FloatsGui("Latitude","Longitude",&calloutMeta->csi.cameraAngles);
  data->children.append(child);
  data->clearCache = (data->clearCache ? data->clearCache : child->modified);
  boxGrid->addWidget(child,1,0);

  box = new QGroupBox("Assembly Margins");
  vlayout->addWidget(box);
  child = new UnitsGui("L/R|T/B",&calloutMeta->csi.margin,box);
  data->children.append(child);

  //spacer
  vSpacer = new QSpacerItem(1,1,QSizePolicy::Fixed,QSizePolicy::Expanding);
  vlayout->addSpacerItem(vSpacer);

  childtab->addTab(widget,"Assembly");

  /*
   * Submodel colors
   */
  widget = new QWidget();
  vlayout = new QVBoxLayout(nullptr);
  widget->setLayout(vlayout);

  box = new QGroupBox(tr("Submodel Level Colors"));
  vlayout->addWidget(box);
  child = new SubModelColorGui(&calloutMeta->subModelColor,box);
  data->children.append(child);

  //spacer
  vSpacer = new QSpacerItem(1,1,QSizePolicy::Fixed,QSizePolicy::Expanding);
  vlayout->addSpacerItem(vSpacer);

  tab->addTab(widget,"Submodel Colors");

  QDialogButtonBox *buttonBox;

  buttonBox = new QDialogButtonBox();
  buttonBox->addButton(QDialogButtonBox::Ok);
  connect(buttonBox,SIGNAL(accepted()),SLOT(accept()));
  buttonBox->addButton(QDialogButtonBox::Cancel);
  connect(buttonBox,SIGNAL(rejected()),SLOT(cancel()));

  layout->addWidget(buttonBox);

  setModal(true);
  setMinimumSize(40,20);
  adjustSize();
}

void GlobalCalloutDialog::getCalloutGlobals(
  QString topLevelFile, Meta &meta)
{
  GlobalCalloutDialog *dialog = new GlobalCalloutDialog(topLevelFile,meta);
  dialog->exec();
}

void GlobalCalloutDialog::accept()
{
  MetaItem mi;

  if (data->clearCache) {
    clearCsiCache();
    clearTempCache();
  }

  mi.beginMacro("Global Callout");

  MetaGui *child;

  foreach(child,data->children) {
    child->apply(data->topLevelFile);
  }
  mi.endMacro();
  QDialog::accept();
}

void GlobalCalloutDialog::cancel()
{
  QDialog::reject();
}
