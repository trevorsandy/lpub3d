 
/****************************************************************************
**
** Copyright (C) 2007-2009 Kevin Clague. All rights reserved.
** Copyright (C) 2015 - 2022 Trevor SANDY. All rights reserved.
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

  setWhatsThis(lpubWT(WT_SETUP_CALLOUT,windowTitle()));

  QTabWidget  *tabwidget = new QTabWidget();
  QVBoxLayout *layout = new QVBoxLayout(tabwidget);
  GlobalSizeWidget sw(QSize(200,300), QSize(200,200));
  layout->addWidget(&sw);

  setLayout(layout);
  layout->addWidget(tabwidget);

  QVBoxLayout *vlayout;
  QSpacerItem *vSpacer;

  QWidget *widget;
  MetaGui *child;
  QGroupBox *box;

  CalloutMeta *calloutMeta = &data->meta.LPub.callout;

  /*
   * Background/Border tab
   */

  widget = new QWidget();
  widget->setObjectName(tr("Background / Border"));
  widget->setWhatsThis(lpubWT(WT_SETUP_CALLOUT_BACKGROUND_BORDER,widget->objectName()));
  vlayout = new QVBoxLayout(nullptr);
  widget->setLayout(vlayout);

  box = new QGroupBox(tr("Background"));
  vlayout->addWidget(box);
  child = new BackgroundGui(&calloutMeta->background,box);
  data->children.append(child);

  box = new QGroupBox(tr("Border"));
  vlayout->addWidget(box);
  child = new BorderGui(&calloutMeta->border,box);
  data->children.append(child);
  
  box = new QGroupBox(tr("Margins"));
  vlayout->addWidget(box);
  child = new UnitsGui(tr("L/R|T/B"),&calloutMeta->margin,box);
  data->children.append(child);
  
  box = new QGroupBox(tr("Divider"));
  vlayout->addWidget(box);
  child = new SepGui(&calloutMeta->sep,box);
  data->children.append(child);

  tabwidget->addTab(widget,widget->objectName());

  /*
   * Contents tab
   */

  widget = new QWidget();
  vlayout = new QVBoxLayout(nullptr);
  widget->setObjectName(tr("Contents"));
  widget->setWhatsThis(lpubWT(WT_SETUP_CALLOUT_CONTENTS,widget->objectName()));
  widget->setLayout(vlayout);
  
  QTabWidget *childtabwidget = new QTabWidget();
  vlayout->addWidget(childtabwidget);

  tabwidget->addTab(widget,widget->objectName());

  /*
   * Callout Tab
   */

  widget = new QWidget(nullptr);
  widget->setObjectName(tr("Callout"));
  widget->setWhatsThis(lpubWT(WT_SETUP_CALLOUT_CALLOUT,widget->objectName()));
  vlayout = new QVBoxLayout(nullptr);
  widget->setLayout(vlayout);
  
  QVBoxLayout *childlayout = new QVBoxLayout();
  box = new QGroupBox(tr("Parts List"));
  box->setWhatsThis(lpubWT(WT_SETUP_CALLOUT_CALLOUT_PARTS_LIST,box->title()));
  vlayout->addWidget(box);
  box->setLayout(childlayout);

  child = new CheckBoxGui(tr("Per Step"),&calloutMeta->pli.perStep);
  data->children.append(child);
  childlayout->addWidget(child);

  child = new UnitsGui(tr("Margins L/R|T/B"),&calloutMeta->pli.margin);
  data->children.append(child);
  childlayout->addWidget(child);

  box = new QGroupBox(tr("Step Justification"));
  vlayout->addWidget(box);
  child = new JustifyStepGui(tr("Set Step Justification"),&calloutMeta->justifyStep,box);
  data->children.append(child);

  box = new QGroupBox(tr("Submodel Preview"));
  box->setWhatsThis(lpubWT(WT_SETUP_CALLOUT_CALLOUT_SUBMODEL_PREVIEW,box->title()));
  vlayout->addWidget(box);
  child = new CheckBoxGui(tr("Show Submodel Preview At First Step"),&calloutMeta->subModel.show, box);
  data->children.append(child);

  box = new QGroupBox(tr("Step Number"));
  vlayout->addWidget(box);
  child = new NumberGui("",&calloutMeta->stepNum,box);
  data->children.append(child);

  box = new QGroupBox(tr("Times Used"));
  vlayout->addWidget(box);
  child = new NumberGui("",&calloutMeta->instance,box);
  data->children.append(child);

  //spacer
  vSpacer = new QSpacerItem(1,1,QSizePolicy::Fixed,QSizePolicy::Expanding);
  vlayout->addSpacerItem(vSpacer);

  childtabwidget->addTab(widget,widget->objectName());

  /*
   * CSI Assembly Tab
   */
  QGridLayout *boxGrid = new QGridLayout();
  widget = new QWidget(nullptr);
  widget->setObjectName(tr("Assembly"));
  widget->setWhatsThis(lpubWT(WT_SETUP_CALLOUT_ASSEMBLY,widget->objectName()));
  vlayout = new QVBoxLayout(nullptr);
  widget->setLayout(vlayout);

  box = new QGroupBox(tr("Assembly Image"));
  box->setWhatsThis(lpubWT(WT_SETUP_SHARED_IMAGE_SIZING,box->title()));
  vlayout->addWidget(box);
  box->setLayout(boxGrid);

  // scale
  child = new ScaleGui(tr("Scale"),&calloutMeta->csi.modelScale);
  data->children.append(child);
  connect (child, SIGNAL(settingsChanged(bool)), this, SLOT(clearCache(bool)));
  boxGrid->addWidget(child,0,0);

  child = new UnitsGui(tr("Margins L/R|T/B"),&calloutMeta->margin);
  data->children.append(child);
  boxGrid->addWidget(child,1,0);

  /* assembly camera settings */
  box = new QGroupBox(tr("Assembly Camera Orientation"));
  box->setWhatsThis(lpubWT(WT_SETUP_SHARED_MODEL_ORIENTATION,box->title()));
  vlayout->addWidget(box);
  boxGrid = new QGridLayout();
  box->setLayout(boxGrid);

  // camera field of view
  child = new CameraFOVGui(tr("FOV"),&calloutMeta->csi.cameraFoV);
  child->setToolTip(tr("Camera field of view"));
  data->children.append(child);
  connect (child, SIGNAL(settingsChanged(bool)), this, SLOT(clearCache(bool)));
  boxGrid->addWidget(child,0,0);

  // camera z near
  child = new CameraZPlaneGui(tr("Z Near"),&calloutMeta->csi.cameraZNear);
  child->setToolTip(tr("Camera Z near plane"));
  connect (child, SIGNAL(settingsChanged(bool)), this, SLOT(clearCache(bool)));
  data->children.append(child);
  boxGrid->addWidget(child,0,1);

  // camera z far
  child = new CameraZPlaneGui(tr("Z Far"),&calloutMeta->csi.cameraZFar,true/*ZFar*/);
  child->setToolTip(tr("Camera Z far plane"));
  connect (child, SIGNAL(settingsChanged(bool)), this, SLOT(clearCache(bool)));
  data->children.append(child);
  boxGrid->addWidget(child,0,2);

  // view angles
  child = new CameraAnglesGui(tr("Camera Angles"),&calloutMeta->csi.cameraAngles);
  child->setToolTip(tr("Camera Latitude and Longitude angles"));
  data->children.append(child);
  connect (child, SIGNAL(settingsChanged(bool)), this, SLOT(clearCache(bool)));
  boxGrid->addWidget(child,1,0,1,3);

  box = new QGroupBox(tr("Assembly Margins"));
  vlayout->addWidget(box);
  child = new UnitsGui(tr("L/R|T/B"),&calloutMeta->csi.margin,box);
  data->children.append(child);

  box = new QGroupBox(tr("Stud Style And Automate Edge Color"));
  vlayout->addWidget(box);
  StudStyleGui *childStudStyle = new StudStyleGui(&calloutMeta->csi.autoEdgeColor,&calloutMeta->csi.studStyle,&calloutMeta->csi.highContrast,box);
  childStudStyle->setToolTip(tr("Select stud style or automate edge colors. High Contrast styles repaint stud cylinders and part edges."));
  data->children.append(childStudStyle);
  connect (childStudStyle, SIGNAL(settingsChanged(bool)), this, SLOT(clearCache(bool)));

  //spacer
  vSpacer = new QSpacerItem(1,1,QSizePolicy::Fixed,QSizePolicy::Expanding);
  vlayout->addSpacerItem(vSpacer);

  childtabwidget->addTab(widget,widget->objectName());

  /*
   * Pointer Tab
   */

  widget = new QWidget(nullptr);
  widget->setObjectName(tr("Pointers"));
  widget->setWhatsThis(lpubWT(WT_SETUP_CALLOUT_POINTERS,widget->objectName()));
  vlayout = new QVBoxLayout(nullptr);
  widget->setLayout(vlayout);

  QTabWidget *grandchildtabwidget = new QTabWidget();
  vlayout->addWidget(grandchildtabwidget);

  childtabwidget->addTab(widget,widget->objectName());

  /*
   * Callout Pointers Tab
   */

  widget = new QWidget(nullptr);
  widget->setObjectName(tr("Callout Pointers"));
  widget->setWhatsThis(lpubWT(WT_SETUP_CALLOUT_CALLOUT_POINTERS,widget->objectName()));
  vlayout = new QVBoxLayout(nullptr);
  widget->setLayout(vlayout);

  box = new QGroupBox(tr("Border"));
  vlayout->addWidget(box);
  PointerAttribData pad = calloutMeta->pointerAttrib.value();
  pad.attribType = PointerAttribData::Border;
  calloutMeta->pointerAttrib.setValue(pad);
  child = new PointerAttribGui(&calloutMeta->pointerAttrib,box,true/*callout*/);
  data->children.append(child);

  box = new QGroupBox(tr("Line"));
  vlayout->addWidget(box);
  pad.attribType = PointerAttribData::Line;
  calloutMeta->pointerAttrib.setValue(pad);
  child = new PointerAttribGui(&calloutMeta->pointerAttrib,box,true/*callout*/);
  data->children.append(child);

  box = new QGroupBox(tr("Tip"));
  vlayout->addWidget(box);
  pad.attribType = PointerAttribData::Tip;
  calloutMeta->pointerAttrib.setValue(pad);
  child = new PointerAttribGui(&calloutMeta->pointerAttrib,box,true/*callout*/);
  data->children.append(child);

    //spacer
  vSpacer = new QSpacerItem(1,1,QSizePolicy::Fixed,QSizePolicy::Expanding);
  vlayout->addSpacerItem(vSpacer);

  grandchildtabwidget->addTab(widget,widget->objectName());

  /*
   * Divider Pointers Tab
   */

  widget = new QWidget(nullptr);
  widget->setObjectName(tr("Divider Pointers"));
  widget->setWhatsThis(lpubWT(WT_SETUP_CALLOUT_DIVIDER_POINTERS,widget->objectName()));
  vlayout = new QVBoxLayout(nullptr);
  widget->setLayout(vlayout);

  box = new QGroupBox(tr("Border"));
  vlayout->addWidget(box);
  pad = calloutMeta->divPointerAttrib.value();
  pad.attribType = PointerAttribData::Border;
  calloutMeta->divPointerAttrib.setValue(pad);
  child = new PointerAttribGui(&calloutMeta->divPointerAttrib,box,true/*callout*/);
  data->children.append(child);

  box = new QGroupBox(tr("Line"));
  vlayout->addWidget(box);
  pad.attribType = PointerAttribData::Line;
  calloutMeta->divPointerAttrib.setValue(pad);
  child = new PointerAttribGui(&calloutMeta->divPointerAttrib,box,true/*callout*/);
  data->children.append(child);

  box = new QGroupBox(tr("Tip"));
  vlayout->addWidget(box);
  pad.attribType = PointerAttribData::Tip;
  calloutMeta->divPointerAttrib.setValue(pad);
  child = new PointerAttribGui(&calloutMeta->divPointerAttrib,box,true/*callout*/);
  data->children.append(child);

  //spacer
  vSpacer = new QSpacerItem(1,1,QSizePolicy::Fixed,QSizePolicy::Expanding);
  vlayout->addSpacerItem(vSpacer);

  grandchildtabwidget->addTab(widget,widget->objectName());

  /*
   * Submodel colors
   */

  widget = new QWidget();
  widget->setObjectName(tr("Submodel Colors"));
  widget->setWhatsThis(lpubWT(WT_SETUP_SHARED_SUBMODEL_LEVEL_COLORS,widget->objectName()));
  vlayout = new QVBoxLayout(nullptr);
  widget->setLayout(vlayout);

  box = new QGroupBox(tr("Submodel Level Colors"));
  vlayout->addWidget(box);
  child = new SubModelColorGui(&calloutMeta->subModelColor,box);
  data->children.append(child);

  //spacer
  vSpacer = new QSpacerItem(1,1,QSizePolicy::Fixed,QSizePolicy::Expanding);
  vlayout->addSpacerItem(vSpacer);

  tabwidget->addTab(widget,widget->objectName());

  QDialogButtonBox *buttonBox;

  buttonBox = new QDialogButtonBox();
  buttonBox->addButton(QDialogButtonBox::Ok);
  connect(buttonBox,SIGNAL(accepted()),SLOT(accept()));
  buttonBox->addButton(QDialogButtonBox::Cancel);
  connect(buttonBox,SIGNAL(rejected()),SLOT(cancel()));

  layout->addWidget(buttonBox);

  setModal(true);
}

void GlobalCalloutDialog::getCalloutGlobals(
  QString topLevelFile, Meta &meta)
{
  GlobalCalloutDialog *dialog = new GlobalCalloutDialog(topLevelFile,meta);
  dialog->exec();
}

void GlobalCalloutDialog::clearCache(bool b)
{
  if (!data->clearCache)
    data->clearCache = b;
}

void GlobalCalloutDialog::accept()
{
  MetaItem mi;

  if (data->clearCache) {
    mi.clearCsiCache();
    mi.clearPliCache();
  }

  mi.beginMacro("Global Callout");

  bool noFileDisplay = false;

  MetaGui *child;
  Q_FOREACH (child,data->children) {
    child->apply(data->topLevelFile);
    noFileDisplay |= child->modified;
  }

  mi.setSuspendFileDisplayFlag(!noFileDisplay);

  mi.endMacro();

  QDialog::accept();
}

void GlobalCalloutDialog::cancel()
{
  QDialog::reject();
}
