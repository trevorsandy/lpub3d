
/****************************************************************************
**
** Copyright (C) 2007-2009 Kevin Clague. All rights reserved.
** Copyright (C) 2015 - 2023 Trevor SANDY. All rights reserved.
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
  bool     reloadWhatsThis;

  GlobalProjectPrivate(const QString &_topLevelFile, Meta &_meta)
  {
    topLevelFile    = _topLevelFile;
    meta            = _meta;
    clearCache      = false;
    reloadFile      = false;
    reloadWhatsThis = false;

    MetaItem mi; // examine all the globals and then return

    mi.sortedGlobalWhere(meta,topLevelFile,"ZZZZZZZ");
  }
};

/**********************************************************************
 *
 * Global to project
 *
 *********************************************************************/

GlobalProjectDialog::GlobalProjectDialog(
  const QString &topLevelFile, Meta &meta)
{
  data = new GlobalProjectPrivate(topLevelFile,meta);

  LPubMeta *lpubMeta = &data->meta.LPub;

  setWindowTitle(tr("Project Globals Setup"));

  setWhatsThis(lpubWT(WT_SETUP_PROJECT,windowTitle()));

  QTabWidget  *tabwidget = new QTabWidget(nullptr);
  QVBoxLayout *layout = new QVBoxLayout(tabwidget);

  GlobalSizeWidget sw(QSize(80,120), QSize(80,80));
  layout->addWidget(&sw);
  setLayout(layout);
  layout->addWidget(tabwidget);

  QWidget     *widget;
  QGroupBox   *box;
  QVBoxLayout *vlayout;
  QSpacerItem *vSpacer;

  /*
   * Renderer Options Tab
   */

  widget = new QWidget();
  widget->setObjectName(tr("Renderer Options"));
  widget->setWhatsThis(lpubWT(WT_SETUP_PROJECT_RENDERER_OPTIONS,widget->objectName()));
  vlayout = new QVBoxLayout(nullptr);
  widget->setLayout(vlayout);

  box = new QGroupBox(tr("Preferred Renderer"));
  vlayout->addWidget(box);
  QVBoxLayout *childlayout = new QVBoxLayout(nullptr);
  box->setLayout(childlayout);

  childPreferredRenderer = new PreferredRendererGui(&lpubMeta->preferredRenderer);
  childPreferredRenderer->setToolTip(tr("Select the default image renderer"));
  connect (childPreferredRenderer, SIGNAL(rendererChanged(int)),  this, SLOT(enableCameraDDF(int)));
  connect (childPreferredRenderer, SIGNAL(rendererChanged(int)),  this, SLOT(reloadWhatsThis(int)));
  connect (childPreferredRenderer, SIGNAL(settingsChanged(bool)), this, SLOT(clearCache(bool)));
  data->children.append(childPreferredRenderer);
  childlayout->addWidget(childPreferredRenderer);

  childCameraDDF = new CameraDDFGui(tr("Native Camera Default Distance Factor"),&lpubMeta->cameraDDF);
  childCameraDDF->setToolTip(tr("Camera default distance factor"));
  connect (childCameraDDF, SIGNAL(settingsChanged(bool)), this, SLOT(clearCache(bool)));
  data->children.append(childCameraDDF);
  enableCameraDDF(Preferences::preferredRenderer);
  childlayout->addWidget(childCameraDDF);

  box = new QGroupBox(tr("Dot Resolution"));
  vlayout->addWidget(box);
  childResolution = new ResolutionGui(&lpubMeta->resolution,box);
  box->setToolTip(tr("Set the default document resolution."));
  connect (childResolution, SIGNAL(unitsChanged(int)), this, SLOT(reloadWhatsThis(int)));
  connect (childResolution, SIGNAL(settingsChanged(bool)), this, SLOT(clearCache(bool)));
  data->children.append(childResolution);

  box = new QGroupBox(tr("Stud Style And Automate Edge Color"));
  vlayout->addWidget(box);
  StudStyleGui *childStudStyle = new StudStyleGui(&lpubMeta->autoEdgeColor,&lpubMeta->studStyle,&lpubMeta->highContrast,box);
  box->setToolTip(tr("Select stud style or automate edge colors. High Contrast styles repaint stud cylinders and part edges."));
  data->children.append(childStudStyle);
  connect (childStudStyle, SIGNAL(settingsChanged(bool)), this, SLOT(clearCache(bool)));

  box = new QGroupBox(tr("Cover Page Viewer Display"));
  vlayout->addWidget(box);
  CoverPageViewEnabledGui *childCoverPageViewEnabled = new CoverPageViewEnabledGui(tr("Enable model view at cover page"),&lpubMeta->coverPageViewEnabled,box);
  box->setToolTip(tr("Enable top model view (read only) in the Visual Editor when a cover page is loaded."));
  data->children.append(childCoverPageViewEnabled);

  //spacer
  vSpacer = new QSpacerItem(1,1,QSizePolicy::Fixed,QSizePolicy::Expanding);
  vlayout->addSpacerItem(vSpacer);

  tabwidget->addTab(widget,widget->objectName());

  /*
   * Parse Options Tab
   */

  widget = new QWidget(nullptr);
  widget->setObjectName(tr("Parse Options"));
  widget->setWhatsThis(lpubWT(WT_SETUP_PROJECT_PARSE_OPTIONS,widget->objectName()));
  vlayout = new QVBoxLayout(nullptr);
  widget->setLayout(vlayout);

  box = new QGroupBox(tr("Build Modifications"));
  vlayout->addWidget(box);
  BuildModEnabledGui *childBuildModEnabled = new BuildModEnabledGui(tr("Enable build modifications"),&lpubMeta->buildModEnabled,box);
  box->setToolTip(tr("Enable Build Modification meta commands. This functionality is a substitute for the MLCad BUFEXCHG framework."));
  data->children.append(childBuildModEnabled);
  connect (childBuildModEnabled->getCheckBox(), SIGNAL(clicked(bool)), this, SLOT(clearCache(bool)));

  box = new QGroupBox("Buffer Exchange");
  box->setWhatsThis(lpubWT(WT_SETUP_PROJECT_PARSE_BUFFER_EXCHANGE,box->title()));
  vlayout->addWidget(box);
  CheckBoxGui *childParseNoStep = new CheckBoxGui(tr("Parse Single Step With NOSTEP Or BUFEXCHG Commands"),&lpubMeta->parseNoStep,box);
  box->setToolTip(("Parse single steps containing NOSTEP or BUFEXCHG commands. Multi-step groups are automatically parsed."));
  data->children.append(childParseNoStep);
  connect (childParseNoStep->getCheckBox(), SIGNAL(clicked(bool)), this, SLOT(clearCache(bool)));

  box = new QGroupBox(tr("Submodel Instances"));
  vlayout->addWidget(box);
  CountInstanceGui *childCountInstance = new CountInstanceGui(&lpubMeta->countInstance,box);
  box->setToolTip(tr("Consolidate submodel instance count on first occurrence"));
  data->children.append(childCountInstance);
  connect (childCountInstance, SIGNAL(settingsChanged(bool)), this, SLOT(clearCache(bool)));

  box = new QGroupBox(tr("Continuous Step Numbers"));
  vlayout->addWidget(box);
  childContStepNumbers = new ContStepNumGui(tr("Enable continuous step numbers"),&lpubMeta->contStepNumbers,box);
  box->setToolTip(tr("Enable continuous step numbers across submodels and unassembled callouts."));
  data->children.append(childContStepNumbers);
  connect (childContStepNumbers->getCheckBox(), SIGNAL(clicked(bool)), this, SLOT(checkConflict(bool)));

  box = new QGroupBox(tr("Unofficial Parts In Editor"));
  vlayout->addWidget(box);
  LoadUnoffPartsEnabledGui *childLoadUnoffPartsEnabled = new LoadUnoffPartsEnabledGui(tr("Enable unofficial parts load in command editor"),&lpubMeta->loadUnoffPartsInEditor,box);
  box->setToolTip(tr("Enable loading unofficial parts in the command editor - setting enabled when unofficial parts are detected."));
  data->children.append(childLoadUnoffPartsEnabled);
  connect (childLoadUnoffPartsEnabled->getCheckBox(), SIGNAL(clicked(bool)), this, SLOT(reloadDisplayPage(bool)));

  box = new QGroupBox(tr("Start Numbers"));
  box->setWhatsThis(lpubWT(WT_SETUP_PROJECT_START_NUMBERS,box->title()));
  vlayout->addWidget(box);
  QGridLayout *grid = new QGridLayout();
  box->setLayout(grid);
  box->setToolTip(tr("Set start step and or page number used for multi-book instruction documents."));

  childStartStepNumberSpin = new SpinGui(tr("Step number"), &lpubMeta->startStepNumber,0,10000,1);
  data->children.append(childStartStepNumberSpin);
  grid->addWidget(childStartStepNumberSpin,0,0);

  childStartPageNumberSpin = new SpinGui(tr("Page number"), &lpubMeta->startPageNumber,0,10000,1);
  data->children.append(childStartPageNumberSpin);
  grid->addWidget(childStartPageNumberSpin,0,1);

  //spacer
  vSpacer = new QSpacerItem(1,1,QSizePolicy::Fixed,QSizePolicy::Expanding);
  vlayout->addSpacerItem(vSpacer);

  tabwidget->addTab(widget,widget->objectName());

  QDialogButtonBox *buttonBox = new QDialogButtonBox(this);
  buttonBox->addButton(QDialogButtonBox::Ok);
  connect(buttonBox,SIGNAL(accepted()),SLOT(accept()));
  buttonBox->addButton(QDialogButtonBox::Cancel);
  connect(buttonBox,SIGNAL(rejected()),SLOT(cancel()));

  layout->addWidget(buttonBox);

  setModal(true);
}

void GlobalProjectDialog::getProjectGlobals(
  const QString topLevelFile, Meta &meta)
{
  GlobalProjectDialog *dialog = new GlobalProjectDialog(topLevelFile,meta);
  dialog->exec();
}

void GlobalProjectDialog::reloadDisplayPage(bool b)
{
  Q_UNUSED(b)
  if (!data->reloadFile)
    data->reloadFile = b;
}

void GlobalProjectDialog::clearCache(bool b)
{
  Q_UNUSED(b)
  if (!data->clearCache)
    data->clearCache = b;
}

void GlobalProjectDialog::reloadWhatsThis(int value)
{
  if (sender() == childPreferredRenderer)
    data->reloadWhatsThis = Preferences::preferredRenderer != value;
  else
    data->reloadWhatsThis = Preferences::preferCentimeters != (bool)value;
}

void GlobalProjectDialog::enableCameraDDF(int renderer)
{
   childCameraDDF->setEnabled(renderer == RENDERER_NATIVE);
}

void GlobalProjectDialog::checkConflict(bool b)
{
  if (b && data->meta.LPub.multiStep.countGroupSteps.value()) {
    childContStepNumbers->getCheckBox()->setChecked(false);
    QMessageBox box;
    box.setTextFormat (Qt::RichText);
    box.setIcon (QMessageBox::Critical);
    box.setWindowFlags (Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
    box.setWindowTitle(tr ("Continuous Step Numbers"));
    box.setStandardButtons (QMessageBox::Ok);
    QString text = tr("<b> Count group step numbers conflict </b>");
    QString message = tr("Continuous step numbers cannot coexist with count group step numbers.");
    box.setText (text);
    box.setInformativeText (message);
    box.exec();
  }
}

void GlobalProjectDialog::accept()
{

  MetaItem mi;

  if (data->clearCache && !data->reloadFile)
    mi.clearAllCaches();

  mi.beginMacro("Global Project");

  bool noFileDisplay = false;

  MetaGui *child;
  Q_FOREACH (child,data->children) {
    child->apply(data->topLevelFile);
    noFileDisplay |= child->modified;
  }

  if (data->reloadFile)
    mi.setSuspendFileDisplayFlag(true);           // when true, do not cyclePageDisplay at endMacro()
  else
    mi.setSuspendFileDisplayFlag(!noFileDisplay); // when false, cyclePageDisplay at endMacro()

  mi.endMacro();

  if (data->reloadFile)
    mi.clearAndReloadModelFile(false, true);      // if true, close and reload file, if true, prompt to save - clear all caches

  if (data->reloadWhatsThis)
    mi.reloadWhatsThis();

  QDialog::accept();
}

void GlobalProjectDialog::cancel()
{
  QDialog::reject();
}
