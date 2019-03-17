
/****************************************************************************
**
** Copyright (C) 2007-2009 Kevin Clague. All rights reserved.
** Copyright (C) 2015 - 2018 Trevor SANDY. All rights reserved.
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
  MetaGui *child = new RendererGui(&lpubMeta->cameraDistNative,box);
  RendererGui *childRenderGui = static_cast<RendererGui*>(child);;
  data->clearCache = childRenderGui->clearCaches;
  childRenderGui = nullptr;
  data->children.append(child);

  box = new QGroupBox("Resolution");
  layout->addWidget(box);
  child = new ResolutionGui(&lpubMeta->resolution,box);
  data->children.append(child);
  
  box = new QGroupBox("Submodel Instance Count");
  layout->addWidget(box);
  child = new CheckBoxGui("Consolidate submodel instance count.",&lpubMeta->mergeInstanceCount,box);
  box->setToolTip("Consolidate submodel instance count at first occurrence in model.");
  data->children.append(child);

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
      clearAndRedrawPage();
  }

  QDialog::accept();
}

void GlobalProjectDialog::cancel()
{
  QDialog::reject();
}
