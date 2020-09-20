
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
#include <QTabWidget>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QDialogButtonBox>

#include "meta.h"
#include "metagui.h"
#include "metaitem.h"
#include "name.h"

/*****************************************************************
 *
 * Global to pli
 *
 ****************************************************************/

class GlobalPliPrivate
{
public:
  Meta     meta;
  QString  topLevelFile;
  QList<MetaGui *> children;
  bool     bom;
  bool     clearCache;

  GlobalPliPrivate(QString &_topLevelFile, Meta &_meta, bool _bom = false)
  {
    topLevelFile = _topLevelFile;
    meta         = _meta;
    bom          = _bom;
    clearCache   = false;
    MetaItem mi; // examine all the globals and then return

    mi.sortedGlobalWhere(meta,topLevelFile,"ZZZZZZZ");
  }
};

GlobalPliDialog::GlobalPliDialog(
  QString &topLevelFile, Meta &meta, bool bom)
{
  data = new GlobalPliPrivate(topLevelFile,meta,bom);

  if (bom) {
    setWindowTitle(tr("Bill of Materials Globals Setup"));
  } else {
    setWindowTitle(tr("Parts List Globals Setup"));
  }


  QTabWidget  *tab = new QTabWidget(nullptr);
  QVBoxLayout *layout = new QVBoxLayout(nullptr);
  QVBoxLayout *childlayout = new QVBoxLayout(nullptr);
  QSpacerItem *vSpacer = new QSpacerItem(1,1,QSizePolicy::Fixed,QSizePolicy::Expanding);

  setLayout(layout);
  layout->addWidget(tab);

  QWidget *widget;
  QVBoxLayout *vlayout;
  QVBoxLayout *svlayout;

  MetaGui *child;
  QGroupBox *box;

  PliMeta *pliMeta = bom ? &data->meta.LPub.bom : &data->meta.LPub.pli;

  /*
   * Background/Border tab
   */
  vlayout  = new QVBoxLayout(nullptr);
  svlayout = new QVBoxLayout(nullptr);
  widget   = new QWidget(nullptr);

  widget->setLayout(vlayout);

  if ( ! bom) {
    QHBoxLayout *childHLayout = new QHBoxLayout(nullptr);
    box = new QGroupBox("Parts List");
    vlayout->addWidget(box);
    box->setLayout(childHLayout);
    child = new CheckBoxGui("Show Parts List",&pliMeta->show);
    childHLayout->addWidget(child);
    data->children.append(child);

    child = new PlacementGui(&pliMeta->placement,"Pli Placement");
    childHLayout->addWidget(child);
    data->children.append(child);
  }

  box = new QGroupBox("Background");
  vlayout->addWidget(box);
  child = new BackgroundGui(&pliMeta->background,box);
  data->children.append(child);

  box = new QGroupBox("Border");
  vlayout->addWidget(box);
  child = new BorderGui(&pliMeta->border,box);
  data->children.append(child);
  
  box = new QGroupBox("Margins");
  vlayout->addWidget(box);
  child = new UnitsGui("L/R|T/B",&pliMeta->margin,box);
  data->children.append(child);
  
  box = new QGroupBox("Constrain");
  vlayout->addWidget(box);
  child = new ConstrainGui("",&pliMeta->constrain,box);
  data->children.append(child);

  //spacer
  vSpacer = new QSpacerItem(1,1,QSizePolicy::Fixed,QSizePolicy::Expanding);
  vlayout->addSpacerItem(vSpacer);

  tab->addTab(widget,"Background/Border");

  /*
   * Contents tab
   */
  widget = new QWidget(nullptr);
  vlayout = new QVBoxLayout(nullptr);
  widget->setLayout(vlayout);

  QTabWidget *childtab    = new QTabWidget();
  vlayout->addWidget(childtab);
  tab->addTab(widget, "Content");

  widget = new QWidget(nullptr);
  vlayout = new QVBoxLayout(nullptr);
  widget->setLayout(vlayout);

  box = new QGroupBox("Part Images");
  vlayout->addWidget(box);
  box->setLayout(childlayout);

  // Scale
  child = new DoubleSpinGui("Scale",
                            &pliMeta->modelScale,
                            pliMeta->modelScale._min,
                            pliMeta->modelScale._max,
                            float(0.01));
  data->children.append(child);
  data->clearCache = child->modified;
  childlayout->addWidget(child);

  child = new UnitsGui("Margins L/R|T/B",&pliMeta->part.margin);
  data->children.append(child);
  childlayout->addWidget(child);

  /* Camera settings */

  box = new QGroupBox("Default Part Orientation");
  vlayout->addWidget(box);
  childlayout = new QVBoxLayout(nullptr);
  box->setLayout(childlayout);

  // camera field ov view
  child = new DoubleSpinGui("Camera FOV",
                            &pliMeta->cameraFoV,
                            pliMeta->cameraFoV._min,
                            pliMeta->cameraFoV._max,
                            pliMeta->cameraFoV.value());
  data->children.append(child);
  childlayout->addWidget(child);

  // view angles
  child = new FloatsGui("Latitude","Longitude",&pliMeta->cameraAngles);
  data->children.append(child);
  data->clearCache = child->modified;
  childlayout->addWidget(child);

  box = new QGroupBox("Part Groups");
  vlayout->addWidget(box);
  QString description = "Movable part groups (part image, instance count ";
  if (bom)
    description += "annotation and element id)";
  else
    description += "and annotation)";
  child = new CheckBoxGui(description,&pliMeta->enablePliPartGroup,box);
  data->children.append(child);

  box = new QGroupBox("Part Counts");
  vlayout->addWidget(box);
  child = new NumberGui(&pliMeta->instance,box);
  data->children.append(child);

  vSpacer = new QSpacerItem(1,1,QSizePolicy::Fixed,QSizePolicy::Expanding);
  vlayout->addSpacerItem(vSpacer);

  childtab->addTab(widget,"Parts");

  widget = new QWidget();
  vlayout = new QVBoxLayout(nullptr);
  widget->setLayout(vlayout);

  box = new QGroupBox("Sort Order and Direction");
  vlayout->addWidget(box);
  child = new PliSortOrderGui("",&pliMeta->sortOrder,box,bom);
  data->children.append(child);

  box = new QGroupBox("Studs");
  vlayout->addWidget(box);
  child = new ComboGui("Display Stud Logo","Logo1|Logo2|Logo3|Logo4|Logo5",&pliMeta->studLogo,box,true);
  data->children.append(child);

  if ( ! bom) {
    box = new QGroupBox("Submodels");
    vlayout->addWidget(box);
    child = new CheckBoxGui("Show in Parts List",&pliMeta->includeSubs,box);
    data->children.append(child);
  }

  //spacer
  vSpacer = new QSpacerItem(1,1,QSizePolicy::Fixed,QSizePolicy::Expanding);
  vlayout->addSpacerItem(vSpacer);

  childtab->addTab(widget,tr("More..."));

  /*
   * PLI Annotations
   */
  widget = new QWidget(nullptr);
  vlayout = new QVBoxLayout(nullptr);
  widget->setLayout(vlayout);

  childtab    = new QTabWidget();
  vlayout->addWidget(childtab);
  tab->addTab(widget, "Annotations");

  widget = new QWidget();
  vlayout = new QVBoxLayout(nullptr);
  widget->setLayout(vlayout);

  box = new QGroupBox("Annotation Options");
  vlayout->addWidget(box);

  childlayout = new QVBoxLayout(nullptr);
  box->setLayout(childlayout);

  childPliAnnotation = new PliAnnotationGui("",&pliMeta->annotation,nullptr,bom);
  childPliAnnotation->enableElementStyle(pliMeta->partElements.display.value());
  connect(childPliAnnotation, SIGNAL(toggled(bool)),
          this,               SLOT(  displayAnnotationsChanged(bool)));
  data->children.append(childPliAnnotation);
  childlayout->addWidget(childPliAnnotation);

  if (bom) {
      childPliPartElement = new PliPartElementGui("",&pliMeta->partElements);
      childPliPartElement->enablePliPartElementGroup(pliMeta->annotation.display.value());
      connect(childPliPartElement, SIGNAL(toggled(bool)),
              this,                SLOT(  enableElementStyleChanged(bool)));
      data->children.append(childPliPartElement);
      childlayout->addWidget(childPliPartElement);
  }

  childTextFormat = new NumberGui(&pliMeta->annotate,nullptr,"Default Text Format");
  childTextFormat->enableTextFormatGroup(pliMeta->annotation.display.value());
  data->children.append(childTextFormat);
  childlayout->addWidget(childTextFormat);

  vSpacer = new QSpacerItem(1,1,QSizePolicy::Fixed,QSizePolicy::Expanding);
  vlayout->addSpacerItem(vSpacer);

  childtab->addTab(widget,"Options");

  widget = new QWidget();
  vlayout = new QVBoxLayout(nullptr);
  widget->setLayout(vlayout);

  //Edit PLI Annotation Style Selection
  annotationEditStyleBox = new QGroupBox("Edit Annotation Style");
  annotationEditStyleBox->setLayout(svlayout);
  annotationEditStyleBox->setEnabled(pliMeta->annotation.display.value());
  vlayout->addWidget(annotationEditStyleBox);

  QGroupBox *sbox = new QGroupBox("Select Style to Edit");
  svlayout->addWidget(sbox);

  QHBoxLayout *shlayout = new QHBoxLayout(nullptr);
  sbox->setLayout(shlayout);

  noStyleButton = new QRadioButton("None",sbox);
  noStyleButton->setChecked(true);
  connect(noStyleButton,SIGNAL(clicked(bool)),
          this,   SLOT(  styleOptionChanged(bool)));
  shlayout->addWidget(noStyleButton);

  squareStyleButton = new QRadioButton("Square",sbox);
  squareStyleButton->setChecked(false);
  connect(squareStyleButton,SIGNAL(clicked(bool)),
          this,       SLOT(  styleOptionChanged(bool)));
  shlayout->addWidget(squareStyleButton);

  circleStyleButton = new QRadioButton("Circle",sbox);
  circleStyleButton->setChecked(false);
  connect(circleStyleButton,SIGNAL(clicked(bool)),
          this,       SLOT(  styleOptionChanged(bool)));
  shlayout->addWidget(circleStyleButton);

  rectangleStyleButton = new QRadioButton("Rectangle",sbox);
  rectangleStyleButton->setChecked(false);
  connect(rectangleStyleButton,SIGNAL(clicked(bool)),
          this,          SLOT(  styleOptionChanged(bool)));
  shlayout->addWidget(rectangleStyleButton);

  if (bom) {
      elementStyleButton = new QRadioButton("Element",sbox);
      elementStyleButton->setChecked(false);
      connect(elementStyleButton,SIGNAL(clicked(bool)),
              this,        SLOT(  styleOptionChanged(bool)));
      shlayout->addWidget(elementStyleButton);
  }

  // square style settings
  squareBkGrndStyleBox = new QGroupBox("Square Background");
  svlayout->addWidget(squareBkGrndStyleBox);
  squareBkGrndStyleBox->hide();
  child = new BackgroundGui(&pliMeta->squareStyle.background,squareBkGrndStyleBox,false);
  data->children.append(child);

  squareBorderStyleBox = new QGroupBox("Square Border");
  svlayout->addWidget(squareBorderStyleBox);
  squareBorderStyleBox->hide();
  child = new BorderGui(&pliMeta->squareStyle.border,squareBorderStyleBox);
  data->children.append(child);

  squareFormatStyleBox = new QGroupBox("Square Annotation Text Format");
  svlayout->addWidget(squareFormatStyleBox);
  squareFormatStyleBox->hide();
  styleMeta = new NumberMeta();
  styleMeta->margin = pliMeta->squareStyle.margin;
  styleMeta->font   = pliMeta->squareStyle.font;
  styleMeta->color  = pliMeta->squareStyle.color;
  child = new NumberGui(styleMeta,squareFormatStyleBox);
  data->children.append(child);

  squareSizeStyleBox = new QGroupBox("Square Size");
  svlayout->addWidget(squareSizeStyleBox);
  squareSizeStyleBox->hide();
  child = new FloatsGui("Width","Height",&pliMeta->squareStyle.size,squareSizeStyleBox,3);
  data->children.append(child);

  // circle style settings
  circleBkGrndStyleBox = new QGroupBox("Circle Background");
  svlayout->addWidget(circleBkGrndStyleBox);
  circleBkGrndStyleBox->hide();
  child = new BackgroundGui(&pliMeta->circleStyle.background,circleBkGrndStyleBox,false);
  data->children.append(child);

  circleBorderStyleBox = new QGroupBox("Circle Border");
  svlayout->addWidget(circleBorderStyleBox);
  circleBkGrndStyleBox->hide();
  child = new BorderGui(&pliMeta->circleStyle.border,circleBorderStyleBox,false,false);
  data->children.append(child);

  circleFormatStyleBox = new QGroupBox("Circle Annotation Text Format");
  svlayout->addWidget(circleFormatStyleBox);
  circleFormatStyleBox->hide();
  styleMeta = new NumberMeta();
  styleMeta->margin = pliMeta->circleStyle.margin;
  styleMeta->font   = pliMeta->circleStyle.font;
  styleMeta->color  = pliMeta->circleStyle.color;
  child = new NumberGui(styleMeta,circleFormatStyleBox);
  data->children.append(child);

  circleSizeStyleBox = new QGroupBox("Circle Size");
  svlayout->addWidget(circleSizeStyleBox);
  circleSizeStyleBox->hide();
  child = new FloatsGui("Diameter","",&pliMeta->circleStyle.size,circleSizeStyleBox,3,false/*show pair*/);
  data->children.append(child);

  // rectangle style settings
  rectangleBkGrndStyleBox = new QGroupBox("Rectangle Background");
  svlayout->addWidget(rectangleBkGrndStyleBox);
  rectangleBkGrndStyleBox->hide();
  child = new BackgroundGui(&pliMeta->rectangleStyle.background,rectangleBkGrndStyleBox,false);
  data->children.append(child);

  rectangleBorderStyleBox = new QGroupBox("Rectangle Border");
  svlayout->addWidget(rectangleBorderStyleBox);
  rectangleBorderStyleBox->hide();
  child = new BorderGui(&pliMeta->rectangleStyle.border,rectangleBorderStyleBox);
  data->children.append(child);

  rectangleFormatStyleBox = new QGroupBox("Rectangle Annotation Text Format");
  svlayout->addWidget(rectangleFormatStyleBox);
  rectangleFormatStyleBox->hide();
  styleMeta = new NumberMeta();
  styleMeta->margin = pliMeta->rectangleStyle.margin;
  styleMeta->font   = pliMeta->rectangleStyle.font;
  styleMeta->color  = pliMeta->rectangleStyle.color;
  child = new NumberGui(styleMeta,rectangleFormatStyleBox);
  data->children.append(child);

  rectangleSizeStyleBox = new QGroupBox("Rectangle Size");
  svlayout->addWidget(rectangleSizeStyleBox);
  child = new FloatsGui("Width","Height",&pliMeta->rectangleStyle.size,rectangleSizeStyleBox,3);
  data->children.append(child);

  if (bom) {
      // element style settings
      elementBkGrndStyleBox = new QGroupBox("Element Background");
      svlayout->addWidget(elementBkGrndStyleBox);
      elementBkGrndStyleBox->hide();
      child = new BackgroundGui(&pliMeta->elementStyle.background,elementBkGrndStyleBox,false);
      data->children.append(child);

      elementBorderStyleBox = new QGroupBox("Element Border");
      svlayout->addWidget(elementBorderStyleBox);
      elementBorderStyleBox->hide();
      child = new BorderGui(&pliMeta->elementStyle.border,elementBorderStyleBox);
      data->children.append(child);

      elementFormatStyleBox = new QGroupBox("Element Annotation Text Format");
      svlayout->addWidget(elementFormatStyleBox);
      elementFormatStyleBox->hide();
      styleMeta = new NumberMeta();
      styleMeta->margin = pliMeta->elementStyle.margin;
      styleMeta->font   = pliMeta->elementStyle.font;
      styleMeta->color  = pliMeta->elementStyle.color;
      child = new NumberGui(styleMeta,elementFormatStyleBox);
      data->children.append(child);

      elementSizeStyleBox = new QGroupBox("Element Size");
      svlayout->addWidget(elementSizeStyleBox);
      child = new FloatsGui("Width","Height",&pliMeta->elementStyle.size,elementSizeStyleBox,3);
      elementSizeStyleBox->setDisabled(true);
      elementSizeStyleBox->setToolTip("Size automatically adjusts to the size of the annotation text");
      data->children.append(child);
  }
  //Edit PLI Annotation Style Selection END

  //spacer
  vSpacer = new QSpacerItem(1,1,QSizePolicy::Fixed,QSizePolicy::Expanding);
  vlayout->addSpacerItem(vSpacer);

  childtab->addTab(widget,tr("Edit Styles"));

  /*
   * Submodel colors
   */

  widget = new QWidget();
  vlayout = new QVBoxLayout(nullptr);
  widget->setLayout(vlayout);

  box = new QGroupBox(tr("Submodel Level Colors"));
  vlayout->addWidget(box);
  child = new SubModelColorGui(&pliMeta->subModelColor,box);
  data->children.append(child);

  //spacer
  vSpacer = new QSpacerItem(1,1,QSizePolicy::Fixed,QSizePolicy::Expanding);
  vlayout->addSpacerItem(vSpacer);

  tab->addTab(widget,"Submodel Colors");

  QDialogButtonBox *buttonBox;

  buttonBox = new QDialogButtonBox(nullptr);
  buttonBox->addButton(QDialogButtonBox::Ok);
  connect(buttonBox,SIGNAL(accepted()),SLOT(accept()));
  buttonBox->addButton(QDialogButtonBox::Cancel);
  connect(buttonBox,SIGNAL(rejected()),SLOT(cancel()));

  layout->addWidget(buttonBox);

  styleOptionChanged(true);

  setModal(true);
  setMinimumSize(40,20);
  adjustSize();
}

void GlobalPliDialog::getPliGlobals(
  QString topLevelFile, Meta &meta)
{
  GlobalPliDialog *dialog = new GlobalPliDialog(topLevelFile, meta, false);
  dialog->exec();
}

void GlobalPliDialog::getBomGlobals(
  QString topLevelFile, Meta &meta)
{
  GlobalPliDialog *dialog = new GlobalPliDialog(topLevelFile, meta, true);
  dialog->exec();
}

void GlobalPliDialog::styleOptionChanged(bool b){

  Q_UNUSED(b)

  QObject *obj = sender();
  if (obj == squareStyleButton) {
      squareBkGrndStyleBox->setDisabled(false);
      squareBorderStyleBox->setDisabled(false);
      squareFormatStyleBox->setDisabled(false);
      squareSizeStyleBox->setDisabled(false);

      squareBkGrndStyleBox->show();
      squareBorderStyleBox->show();
      squareFormatStyleBox->show();
      squareSizeStyleBox->show();

      circleBkGrndStyleBox->hide();
      circleBorderStyleBox->hide();
      circleFormatStyleBox->hide();
      circleSizeStyleBox->hide();

      rectangleBkGrndStyleBox->hide();
      rectangleBorderStyleBox->hide();
      rectangleFormatStyleBox->hide();
      rectangleSizeStyleBox->hide();
      if (data->bom) {
            elementBkGrndStyleBox->hide();
            elementBorderStyleBox->hide();
            elementFormatStyleBox->hide();
            elementSizeStyleBox->hide();
      }
  }
  else
  if (obj == circleStyleButton) {
      circleBkGrndStyleBox->show();
      circleBorderStyleBox->show();
      circleFormatStyleBox->show();
      circleSizeStyleBox->show();

      squareBkGrndStyleBox->hide();
      squareBorderStyleBox->hide();
      squareFormatStyleBox->hide();
      squareSizeStyleBox->hide();

      rectangleBkGrndStyleBox->hide();
      rectangleBorderStyleBox->hide();
      rectangleFormatStyleBox->hide();
      rectangleSizeStyleBox->hide();

      if (data->bom) {
            elementBkGrndStyleBox->hide();
            elementBorderStyleBox->hide();
            elementFormatStyleBox->hide();
            elementSizeStyleBox->hide();
      }
  }
  else
  if (obj == rectangleStyleButton) {
      rectangleBkGrndStyleBox->show();
      rectangleBorderStyleBox->show();
      rectangleFormatStyleBox->show();
      rectangleSizeStyleBox->show();

      squareBkGrndStyleBox->hide();
      squareBorderStyleBox->hide();
      squareFormatStyleBox->hide();
      squareSizeStyleBox->hide();

      circleBkGrndStyleBox->hide();
      circleBorderStyleBox->hide();
      circleFormatStyleBox->hide();
      circleSizeStyleBox->hide();

      if (data->bom) {
            elementBkGrndStyleBox->hide();
            elementBorderStyleBox->hide();
            elementFormatStyleBox->hide();
            elementSizeStyleBox->hide();
      }
  }
  else
  if (data->bom && obj == elementStyleButton) {
      elementBkGrndStyleBox->show();
      elementBorderStyleBox->show();
      elementFormatStyleBox->show();
      elementSizeStyleBox->show();

      rectangleBkGrndStyleBox->hide();
      rectangleBorderStyleBox->hide();
      rectangleFormatStyleBox->hide();
      rectangleSizeStyleBox->hide();

      squareBkGrndStyleBox->hide();
      squareBorderStyleBox->hide();
      squareFormatStyleBox->hide();
      squareSizeStyleBox->hide();

      circleBkGrndStyleBox->hide();
      circleBorderStyleBox->hide();
      circleFormatStyleBox->hide();
      circleSizeStyleBox->hide();
  } else {
      squareBkGrndStyleBox->show();
      squareBorderStyleBox->show();
      squareFormatStyleBox->show();
      squareSizeStyleBox->show();

      circleBkGrndStyleBox->hide();
      circleBorderStyleBox->hide();
      circleFormatStyleBox->hide();
      circleSizeStyleBox->hide();

      rectangleBkGrndStyleBox->hide();
      rectangleBorderStyleBox->hide();
      rectangleFormatStyleBox->hide();
      rectangleSizeStyleBox->hide();

      if (data->bom) {
            elementBkGrndStyleBox->hide();
            elementBorderStyleBox->hide();
            elementFormatStyleBox->hide();
            elementSizeStyleBox->hide();
      }

      squareBkGrndStyleBox->setDisabled(true);
      squareBorderStyleBox->setDisabled(true);
      squareFormatStyleBox->setDisabled(true);
      squareSizeStyleBox->setDisabled(true);
  }
}

void GlobalPliDialog::displayAnnotationsChanged(bool b) {
    annotationEditStyleBox->setEnabled(b);
    childTextFormat->enableTextFormatGroup(b);
    if (data->bom)
        childPliPartElement->enablePliPartElementGroup(b);
}

void GlobalPliDialog::enableElementStyleChanged(bool b) {
    childPliAnnotation->enableElementStyle(b);
}

void GlobalPliDialog::accept()
{
  if (data->clearCache) {
    clearPliCache();
  }

  MetaItem mi;

  mi.beginMacro("Global Pli");

  MetaGui *child;

  foreach(child,data->children) {
    child->apply(data->topLevelFile);
  }
  mi.endMacro();
  QDialog::accept();
}

void GlobalPliDialog::cancel()
{
  QDialog::reject();
}
