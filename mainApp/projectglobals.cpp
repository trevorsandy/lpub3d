
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
#include <QDialog>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QDialogButtonBox>

#include "meta.h"
#include "metagui.h"
#include "metaitem.h"
#include "metatypes.h"

class GlobalProjectPrivate {
public:
  Meta       meta;
  QString    topLevelFile;
  QList<MetaGui *> children;
  bool     clearCache;
  bool     reloadFile;

  GlobalProjectPrivate(const QString &_topLevelFile, Meta &_meta)
  {
    topLevelFile = _topLevelFile;
    meta         = _meta;
    clearCache   = false;
    reloadFile   = false;

    MetaItem mi; // examine all the globals and then return

    mi.sortedGlobalWhere(meta,topLevelFile,"ZZZZZZZ");
  }
};

/**********************************************************************
 *
 * 
 *
 *********************************************************************/

GlobalProjectDialog::GlobalProjectDialog(
  const QString &topLevelFile, Meta &meta)
{
  data = new GlobalProjectPrivate(topLevelFile,meta);

  LPubMeta *lpubMeta = &data->meta.LPub;

  setWindowTitle(tr("Project Globals Setup"));

  QTabWidget  *tab = new QTabWidget();
  QVBoxLayout *layout = new QVBoxLayout(this);

  setLayout(layout);
  layout->addWidget(tab);

  QWidget     *widget;
  QGroupBox   *box;
  QGridLayout *boxGrid;
  QVBoxLayout *vlayout;
  QSpacerItem *vSpacer;

  /*
   * Render options tab
   */

  widget = new QWidget();
  vlayout = new QVBoxLayout(nullptr);
  widget->setLayout(vlayout);

  box = new QGroupBox("Renderer");
  vlayout->addWidget(box);
  PreferredRendererGui *rendererChild =new PreferredRendererGui(&lpubMeta->preferredRenderer,box);
  connect (rendererChild, SIGNAL(settingsChanged(bool)), this, SLOT(reloadModelFile(bool)));
  data->children.append(rendererChild);

  box = new QGroupBox("Resolution");
  vlayout->addWidget(box);
  boxGrid = new QGridLayout();
  box->setLayout(boxGrid);

  MetaGui *child = new ResolutionGui(&lpubMeta->resolution);
  boxGrid->addWidget(child,0,0);
  boxGrid->setColumnStretch(0,1);
  boxGrid->setColumnStretch(1,1);
  data->children.append(child);

  box = new QGroupBox("Stud Style and Automate Edge Color");
  vlayout->addWidget(box);
  StudStyleGui *childStudStyle = new StudStyleGui(&lpubMeta->autoEdgeColor,&lpubMeta->studStyle,&lpubMeta->highContrast,box);
  childStudStyle->setToolTip("Select stud style or automate edge colors. High Contrast styles repaint stud cylinders and part edges.");
  data->children.append(childStudStyle);
  connect (childStudStyle, SIGNAL(settingsChanged(bool)), this, SLOT(clearCache(bool)));

  //spacer
  vSpacer = new QSpacerItem(1,1,QSizePolicy::Fixed,QSizePolicy::Expanding);
  vlayout->addSpacerItem(vSpacer);

  tab->addTab(widget,"Render Options");

  /*
   * Parse options tab
   */

  widget = new QWidget(nullptr);
  vlayout = new QVBoxLayout(nullptr);
  widget->setLayout(vlayout);

  box = new QGroupBox("Build Modifications");
  vlayout->addWidget(box);
  BuildModEnabledGui *childBuildModEnabled = new BuildModEnabledGui("Enable build modifications",&lpubMeta->buildModEnabled,box);
  box->setToolTip("Enable Build Modification meta commands. This functionality replaces or accompanies MLCad BUFEXCHG framework.");
  data->children.append(childBuildModEnabled);
  connect (childBuildModEnabled->getCheckBox(), SIGNAL(clicked(bool)), this, SLOT(clearCache(bool)));

  box = new QGroupBox("Buffer Exchange");
  vlayout->addWidget(box);
  CheckBoxGui *childParseNoStep = new CheckBoxGui("Parse Single Step with NOSTEP and BUFEXCHG commands",&lpubMeta->parseNoStep,box);
  box->setToolTip("Parse single steps containing NOSTEP and BUFEXCHG commands. Multi-step groups are automatically parsed.");
  data->children.append(childParseNoStep);
  connect (childParseNoStep->getCheckBox(), SIGNAL(clicked(bool)), this, SLOT(clearCache(bool)));

  box = new QGroupBox("Submodel Instances");
  vlayout->addWidget(box);
  CountInstanceGui *childCountInstance = new CountInstanceGui(&lpubMeta->countInstance,box);
  box->setToolTip("Consolidate submodel instances on first occurrence");
  data->children.append(childCountInstance);
  connect (childCountInstance, SIGNAL(settingsChanged(bool)), this, SLOT(clearCache(bool)));

  box = new QGroupBox("Continuous Step Numbers");
  vlayout->addWidget(box);
  childContStepNumbersBox = new ContStepNumGui("Enable continuous step numbers",&lpubMeta->contStepNumbers,box);
  box->setToolTip("Enable continuous step numbers across submodels and unassembled callouts.");
  data->children.append(childContStepNumbersBox);
  connect (childContStepNumbersBox->getCheckBox(), SIGNAL(clicked(bool)), this, SLOT(checkConflict(bool)));

  box = new QGroupBox("Start Numbers");
  vlayout->addWidget(box);
  boxGrid = new QGridLayout();
  box->setLayout(boxGrid);

  childStartStepNumberSpin = new SpinGui("Step number", &lpubMeta->startStepNumber,0,10000,1);
  data->children.append(childStartStepNumberSpin);
  boxGrid->addWidget(childStartStepNumberSpin,0,0);

  childStartPageNumberSpin = new SpinGui("Page number", &lpubMeta->startPageNumber,0,10000,1);
  data->children.append(childStartPageNumberSpin);
  boxGrid->addWidget(childStartPageNumberSpin,0,1);

  //spacer
  vSpacer = new QSpacerItem(1,1,QSizePolicy::Fixed,QSizePolicy::Expanding);
  vlayout->addSpacerItem(vSpacer);

  tab->addTab(widget,"Parse Options");

  QDialogButtonBox *buttonBox = new QDialogButtonBox(this);
  buttonBox->addButton(QDialogButtonBox::Ok);
  connect(buttonBox,SIGNAL(accepted()),SLOT(accept()));
  buttonBox->addButton(QDialogButtonBox::Cancel);
  connect(buttonBox,SIGNAL(rejected()),SLOT(cancel()));

  layout->addWidget(buttonBox);

  setModal(true);
  setMinimumSize(40,20);
}

void GlobalProjectDialog::getProjectGlobals(
  const QString topLevelFile, Meta &meta)
{
  GlobalProjectDialog *dialog = new GlobalProjectDialog(topLevelFile,meta);
  dialog->exec();
}

void GlobalProjectDialog::reloadModelFile(bool b)
{
  if (!data->reloadFile)
    data->reloadFile = b;
}

void GlobalProjectDialog::clearCache(bool b)
{
  if (!data->clearCache)
    data->clearCache = b;
}

void GlobalProjectDialog::checkConflict(bool b)
{
  if (b && data->meta.LPub.multiStep.countGroupSteps.value()) {
    childContStepNumbersBox->getCheckBox()->setChecked(false);
    QMessageBox box;
    box.setTextFormat (Qt::RichText);
    box.setIcon (QMessageBox::Critical);
    box.setWindowFlags (Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
    box.setWindowTitle(tr ("Continuous Step Numbers&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"));
    box.setStandardButtons (QMessageBox::Ok);
    QString text = "<b> Count group step numbers conflict </b>";
    QString message = QString("Continuous step numbers cannot coexist with count group step numbers.");
    box.setText (text);
    box.setInformativeText (message);
    box.exec();
  }
}

void GlobalProjectDialog::accept()
{

  MetaItem mi;

  mi.beginMacro("Global Project");

  MetaGui *child;

  Q_FOREACH (child,data->children) {
    child->apply(data->topLevelFile);
  }

  if (data->clearCache) {
    mi.setLoadingFileFlag(false);
    mi.clearCache(true);
  }
  if (data->reloadFile) {
    mi.setLoadingFileFlag(true);
  }

  mi.endMacro();

  if (data->reloadFile) {
    mi.reloadModelFile(true);
  }

  QDialog::accept();
}

void GlobalProjectDialog::cancel()
{
  QDialog::reject();
}
