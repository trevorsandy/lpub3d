
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

  GlobalProjectPrivate(const QString &_topLevelFile, Meta &_meta)
  {
    topLevelFile = _topLevelFile;
    meta = _meta;
    clearCache   = false;

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

  QVBoxLayout *layout = new QVBoxLayout(this);
  QVBoxLayout *childlayout = new QVBoxLayout(nullptr);

  setLayout(layout);

  QGroupBox *box = new QGroupBox("Renderer");
  layout->addWidget(box);
  MetaGui *child =new RendererGui(box);
  data->children.append(child);

  box = new QGroupBox("Resolution");
  layout->addWidget(box);
  child = new ResolutionGui(&lpubMeta->resolution,box);
  data->children.append(child);
  
  box = new QGroupBox("Build Modifications");
  layout->addWidget(box);
  BuildModEnabledGui *childBuildModEnabled = new BuildModEnabledGui("Enable Build Modifications",&lpubMeta->buildModEnabled,box);
  box->setToolTip("Enable Build Modification meta commands. This functionality replaces or accompanies MLCad BUFEXCHG framework.");
  data->children.append(childBuildModEnabled);
  connect (childBuildModEnabled->getCheckBox(), SIGNAL(clicked(bool)), this, SLOT(clearCache(bool)));

  box = new QGroupBox("Consolidate Submodel Instances");
  box->setCheckable(true);
  box->setChecked(lpubMeta->countInstance.value());
  layout->addWidget(box);
  CountInstanceGui *childCountInstance = new CountInstanceGui(&lpubMeta->countInstance,box);
  box->setToolTip("Consolidate submodel instances on first occurrence");
  data->children.append(childCountInstance);
  connect (childCountInstance->getTopRadio(),   SIGNAL(clicked(bool)), this, SLOT(clearCache(bool)));
  connect (childCountInstance->getModelRadio(), SIGNAL(clicked(bool)), this, SLOT(clearCache(bool)));
  connect (childCountInstance->getStepRadio(),  SIGNAL(clicked(bool)), this, SLOT(clearCache(bool)));

  box = new QGroupBox("Step Numbers");
  layout->addWidget(box);
  box->setLayout(childlayout);
  childContStepNumbersBox = new ContStepNumGui("Continuous step numbers",&lpubMeta->contStepNumbers/*,box*/);
  box->setToolTip("Enable continuous step numbers across submodels and unassembled callouts.");
  data->children.append(childContStepNumbersBox);
  childlayout->addWidget(childContStepNumbersBox);
  connect (childContStepNumbersBox->getCheckBox(), SIGNAL(clicked(bool)), this, SLOT(clearCache(bool)));
  connect (childContStepNumbersBox->getCheckBox(), SIGNAL(clicked(bool)), this, SLOT(checkConflict(bool)));

  childStartStepNumberSpin = new SpinGui("Start Step Number", &lpubMeta->startStepNumber,0,10000,1/*,box*/);
  data->children.append(childStartStepNumberSpin);
  connect (childStartStepNumberSpin->getSpinBox(),   SIGNAL(valueChanged(int)), this, SLOT(clearCache(int)));
  childlayout->addWidget(childStartStepNumberSpin);

  box = new QGroupBox("Page Numbers");
  layout->addWidget(box);
  childStartPageNumberSpin = new SpinGui("Start Page Number", &lpubMeta->startPageNumber,0,10000,1,box);
  data->children.append(childStartPageNumberSpin);
  connect (childStartPageNumberSpin->getSpinBox(),   SIGNAL(valueChanged(int)), this, SLOT(clearCache(int)));

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

void GlobalProjectDialog::clearCache(bool b)
{
    if (!data->clearCache)
        data->clearCache = b;
}

void GlobalProjectDialog::clearCache(int i)
{
    clearCache(bool(i++));
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
      clearAndReloadModelFile(true);
  }

  mi.endMacro();

  QDialog::accept();
}

void GlobalProjectDialog::cancel()
{
  QDialog::reject();
}
