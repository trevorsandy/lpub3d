
/****************************************************************************
**
** Copyright (C) 2007-2009 Kevin Clague. All rights reserved.
** Copyright (C) 2015 - 2021 Trevor SANDY. All rights reserved.
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
#include <QString>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QDialogButtonBox>

#include "meta.h"
#include "metagui.h"
#include "metaitem.h"

class GlobalMultiStepPrivate
{
public:
  Meta       meta;
  QString    topLevelFile;
  QList<MetaGui *> children;
  bool       clearCache;

  GlobalMultiStepPrivate(QString &_topLevelFile, Meta &_meta)
  {
    topLevelFile = _topLevelFile;
    meta         = _meta;
    clearCache   = false;

    MetaItem mi; // examine all the globals and then return

    mi.sortedGlobalWhere(meta,topLevelFile,"ZZZZZZZ");
  }
};


GlobalMultiStepDialog::GlobalMultiStepDialog(
  QString &topLevelFile,
  Meta &meta)
{
  data = new GlobalMultiStepPrivate(topLevelFile,meta);

  setWindowTitle(tr("Step Group Globals Setup"));

  QTabWidget  *tab = new QTabWidget(nullptr);
  QVBoxLayout *layout = new QVBoxLayout(nullptr);
  QGridLayout *boxGrid = new QGridLayout();

  setLayout(layout);
  layout->addWidget(tab);

  QVBoxLayout *vlayout;
  QSpacerItem *vSpacer;

  QWidget *widget;
  MetaGui *child;
  QGroupBox *box;

  MultiStepMeta *multiStepMeta = &data->meta.LPub.multiStep;

  /*
   * Step group
   */
  widget = new QWidget();
  vlayout = new QVBoxLayout(nullptr);
  widget->setLayout(vlayout);

  box = new QGroupBox("Step Group Margins");
  vlayout->addWidget(box);
  child = new UnitsGui("L/R|T/B",&multiStepMeta->margin,box);
  data->children.append(child);

  box = new QGroupBox("Step Justification");
  vlayout->addWidget(box);
  child = new JustifyStepGui("Set step justification",&multiStepMeta->justifyStep,box);
  data->children.append(child);

  box = new QGroupBox("Callout and Rotate Icon");
  vlayout->addWidget(box);
  child = new CheckBoxGui("Adjust Step row or column when Callout or Rotate Icon dragged.",&multiStepMeta->adjustOnItemOffset,box);
  data->children.append(child);

  box = new QGroupBox("Step Number");
  vlayout->addWidget(box);
  child = new NumberGui(&multiStepMeta->stepNum,box);
  data->children.append(child);

  tab->addTab(widget,"Contents");

  /*
  * Display Tab
  */

  widget = new QWidget();
  vlayout = new QVBoxLayout(nullptr);
  widget->setLayout(vlayout);

  QVBoxLayout *childlayout = new QVBoxLayout(nullptr);

  box = new QGroupBox("Parts List");
  vlayout->addWidget(box);
  box->setLayout(childlayout);

  CheckBoxGui *childPliPerStep = new CheckBoxGui("Per Step",&multiStepMeta->pli.perStep);
  connect(childPliPerStep->getCheckBox(), SIGNAL(stateChanged(int)),
          this,                           SLOT(  pliPerStepStateChanged(int)));
  data->children.append(childPliPerStep);
  childlayout->addWidget(childPliPerStep);

  showGrpStepNumCheckBoxGui = new CheckBoxGui("Show Group Step Number",&multiStepMeta->showGroupStepNumber);
  showGrpStepNumCheckBoxGui->getCheckBox()->setEnabled(!multiStepMeta->pli.perStep.value());
  data->children.append(showGrpStepNumCheckBoxGui);
  childlayout->addWidget(showGrpStepNumCheckBoxGui);
  connect(showGrpStepNumCheckBoxGui->getCheckBox(), SIGNAL(stateChanged(int)),
          this,                                     SLOT(  showGrpStepNumStateChanged(int)));

  countGrpStepsCheckBoxGui = new CheckBoxGui("Count Group Steps",&multiStepMeta->countGroupSteps);
  countGrpStepsCheckBoxGui->getCheckBox()->setEnabled(multiStepMeta->showGroupStepNumber.value());
  data->children.append(countGrpStepsCheckBoxGui);
  childlayout->addWidget(countGrpStepsCheckBoxGui);

  child = new UnitsGui("Margins L/R|T/B",&multiStepMeta->pli.margin);
  data->children.append(child);
  childlayout->addWidget(child);

  box = new QGroupBox("Submodel");
  vlayout->addWidget(box);
  child = new CheckBoxGui("Show Submodel image at first step",&multiStepMeta->subModel.show,box);
  data->children.append(child);

  //spacer
  vSpacer = new QSpacerItem(1,1,QSizePolicy::Fixed,QSizePolicy::Expanding);
  vlayout->addSpacerItem(vSpacer);

  tab->addTab(widget,"Display");

  /*
  * CSI Assembly Tab
  */
  widget = new QWidget();
  vlayout = new QVBoxLayout(nullptr);
  widget->setLayout(vlayout);

  box = new QGroupBox("Assembly Image");
  vlayout->addWidget(box);
  box->setLayout(boxGrid);

  // Scale
  child = new DoubleSpinGui("Scale",
                            &multiStepMeta->csi.modelScale,
                            multiStepMeta->csi.modelScale._min,
                            multiStepMeta->csi.modelScale._max,
                            0.01f);
  data->children.append(child);
  boxGrid->addWidget(child);

  data->clearCache = (data->clearCache ? data->clearCache : child->modified);

  child = new UnitsGui("Margins L/R|T/B",&multiStepMeta->margin);
  data->children.append(child);
  boxGrid->addWidget(child);

  /* Assembly camera settings */

  box = new QGroupBox("Default Assembly Orientation");
  vlayout->addWidget(box);
  boxGrid = new QGridLayout();
  box->setLayout(boxGrid);

  // camera field of view
  child = new DoubleSpinGui("Camera FOV",
                            &multiStepMeta->csi.cameraFoV,
                            multiStepMeta->csi.cameraFoV._min,
                            multiStepMeta->csi.cameraFoV._max,
                            multiStepMeta->csi.cameraFoV.value());
  data->children.append(child);
  data->clearCache = (data->clearCache ? data->clearCache : child->modified);
  boxGrid->addWidget(child,0,0,1,2);

  // view angles
  child = new FloatsGui("Latitude","Longitude",&multiStepMeta->csi.cameraAngles);
  data->clearCache = (data->clearCache ? data->clearCache : child->modified);
  data->children.append(child);
  boxGrid->addWidget(child,1,0);

  box = new QGroupBox("Assembly Margins");
  vlayout->addWidget(box);
  child = new UnitsGui("L/R|T/B",&multiStepMeta->csi.margin,box);
  data->children.append(child);

  box = new QGroupBox("Stud Style and Automate Edge Color");
  vlayout->addWidget(box);
  StudStyleGui *childStudStyle = new StudStyleGui(&multiStepMeta->csi.autoEdgeColor,&multiStepMeta->csi.studStyle,&multiStepMeta->csi.highContrast,box);
  childStudStyle->setToolTip("Select stud style or automate edge colors. High Contrast styles repaint stud cylinders and part edges.");
  data->children.append(childStudStyle);
  connect (childStudStyle, SIGNAL(settingsChanged(bool)), this, SLOT(clearCache(bool)));

  //spacer
  vSpacer = new QSpacerItem(1,1,QSizePolicy::Fixed,QSizePolicy::Expanding);
  vlayout->addSpacerItem(vSpacer);

  tab->addTab(widget,"Assembly");

  /*
   * Divider tab
   */
  widget = new QWidget(nullptr);
  vlayout = new QVBoxLayout(nullptr);
  widget->setLayout(vlayout);

  box = new QGroupBox("Divider");
  vlayout->addWidget(box);
  child = new SepGui(&multiStepMeta->sep,box);
  data->children.append(child);

  box = new QGroupBox("Divider Pointers");
  vlayout->addWidget(box);
  QVBoxLayout *childLayout = new QVBoxLayout(nullptr);
  box->setLayout(childLayout);

  QGroupBox * childBox = new QGroupBox("Border");
  childLayout->addWidget(childBox);
  PointerAttribData pad = multiStepMeta->divPointerAttrib.value();
  pad.attribType = PointerAttribData::Border;
  multiStepMeta->divPointerAttrib.setValue(pad);
  child = new PointerAttribGui(&multiStepMeta->divPointerAttrib,childBox);
  data->children.append(child);

  childBox = new QGroupBox("Line");
  childLayout->addWidget(childBox);
  pad.attribType = PointerAttribData::Line;
  multiStepMeta->divPointerAttrib.setValue(pad);
  child = new PointerAttribGui(&multiStepMeta->divPointerAttrib,childBox);
  data->children.append(child);

  childBox = new QGroupBox("Tip");
  childLayout->addWidget(childBox);
  pad.attribType = PointerAttribData::Tip;
  multiStepMeta->divPointerAttrib.setValue(pad);
  child = new PointerAttribGui(&multiStepMeta->divPointerAttrib,childBox);
  data->children.append(child);

  //spacer
  vSpacer = new QSpacerItem(1,1,QSizePolicy::Fixed,QSizePolicy::Expanding);
  vlayout->addSpacerItem(vSpacer);

  tab->addTab(widget,"Divider");

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

void GlobalMultiStepDialog::getMultiStepGlobals(QString topLevelFile, Meta &meta)
{
  GlobalMultiStepDialog *dialog = new GlobalMultiStepDialog(topLevelFile, meta);
  dialog->exec();
}

void GlobalMultiStepDialog::pliPerStepStateChanged(int state)
{
    showGrpStepNumCheckBoxGui->getCheckBox()->setEnabled(!state);
    countGrpStepsCheckBoxGui->getCheckBox()->setEnabled(!state &&
    showGrpStepNumCheckBoxGui->getCheckBox()->isChecked());
}

void GlobalMultiStepDialog::showGrpStepNumStateChanged(int state){
    if (state && !data->meta.LPub.multiStep.pli.perStep.value()
              && data->meta.LPub.contStepNumbers.value()) {
        QMessageBox box;
        box.setTextFormat (Qt::RichText);
        box.setIcon (QMessageBox::Critical);
        box.setWindowFlags (Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
        box.setWindowTitle(tr ("Count Group Step Numbers&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"));
        box.setStandardButtons (QMessageBox::Ok);
        QString text = "<b> Continuous step numbers conflict; </b>";
        QString message = QString("Count group step numbers cannot coexist with continuous step numbers.");
        box.setText (text);
        box.setInformativeText (message);
        box.exec();
    } else {
        countGrpStepsCheckBoxGui->getCheckBox()->setEnabled(state);
    }
}

void GlobalMultiStepDialog::clearCache(bool b)
{
  if (!data->clearCache)
    data->clearCache = b;
}

void GlobalMultiStepDialog::accept()
{
  MetaItem mi;

  mi.beginMacro("GlobalMultiStep");

  MetaGui *child;

  Q_FOREACH (child,data->children) {
    child->apply(data->topLevelFile);
  }

  if (data->clearCache) {
    mi.setLoadingFileFlag(false);
    mi.clearCsiCache();
  }

  mi.endMacro();

  QDialog::accept();
}
void GlobalMultiStepDialog::cancel()
{
  QDialog::reject();
}

