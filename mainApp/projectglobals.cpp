
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
  setLayout(layout);

  QGroupBox *box = new QGroupBox("Renderer");
  layout->addWidget(box);
  MetaGui *child =new RendererGui(&lpubMeta->nativeCD,box);
  data->children.append(child);

  box = new QGroupBox("Resolution");
  layout->addWidget(box);
  child = new ResolutionGui(&lpubMeta->resolution,box);
  data->children.append(child);
  
  box = new QGroupBox("Consolidate Submodel Instances");
  box->setCheckable(true);
  box->setChecked(lpubMeta->countInstance.value());
  layout->addWidget(box);
  CountInstanceGui *childCountInstance = new CountInstanceGui(&lpubMeta->countInstance,box);
  box->setToolTip("Consolidate submodel instances on first occurrence.");
  data->children.append(childCountInstance);
  connect (childCountInstance->getTopRadio(),   SIGNAL(clicked(bool)), this, SLOT(clearCache(bool)));
  connect (childCountInstance->getModelRadio(), SIGNAL(clicked(bool)), this, SLOT(clearCache(bool)));
  connect (childCountInstance->getStepRadio(),  SIGNAL(clicked(bool)), this, SLOT(clearCache(bool)));

  box = new QGroupBox("Step Numbers");
  layout->addWidget(box);
  ContStepNumGui *childContStepNumbersBox = new ContStepNumGui("Continuous step numbers.",&lpubMeta->contStepNumbers,box);
  box->setToolTip("Enable continuous step numbers across submodels and unassembled callouts.");
  data->children.append(childContStepNumbersBox);
  connect (childContStepNumbersBox->getCheckbox(), SIGNAL(clicked(bool)), this, SLOT(clearCache(bool)));

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

void GlobalProjectDialog::accept()
{

  MetaItem mi;

  mi.beginMacro("Global Project");

  MetaGui *child;

  foreach(child,data->children) {
    child->apply(data->topLevelFile);
  }
  mi.endMacro();

  if (data->clearCache) {
      clearAndReloadModelFile();
  }

  QDialog::accept();
}

void GlobalProjectDialog::cancel()
{
  QDialog::reject();
}
