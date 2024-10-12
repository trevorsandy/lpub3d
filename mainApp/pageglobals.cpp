/****************************************************************************
**
** Copyright (C) 2007-2009 Kevin Clague. All rights reserved.
** Copyright (C) 2015 - 2024 Trevor SANDY. All rights reserved.
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
#include <QGroupBox>
#include <QGridLayout>
#include <QTabWidget>

#include <QDialog>
#include <QDialogButtonBox>

#include "meta.h"
#include "metaitem.h"
#include "metagui.h"
#include "lpub_preferences.h"
#include "placementdialog.h"
#include "lpub.h"

/**********************************************************************
 *
 * Page
 *
 *********************************************************************/

class GlobalPagePrivate
{
public:
  Meta       meta;
  QString    topLevelFile;
  QList<MetaGui *> children;

  GlobalPagePrivate(QString &_topLevelFile, Meta &_meta)
  {
    topLevelFile = _topLevelFile;
    meta = _meta;

    MetaItem mi; // examine all the globals and then return

    mi.sortedGlobalWhere(meta,topLevelFile,"ZZZZZZZ");
  }
};

GlobalPageDialog::GlobalPageDialog(
  QString &topLevelFile, Meta &meta)
{
  data = new GlobalPagePrivate(topLevelFile,meta);

  sectionIndex = 0;

  setWindowTitle(tr("Page Globals Setup"));

  setWhatsThis(lpubWT(WT_SETUP_PAGE,windowTitle()));

  QTabWidget  *tabwidget = new QTabWidget(nullptr);
  QVBoxLayout *layout = new QVBoxLayout(tabwidget);
  QTabWidget  *childtabwidget;
  QVBoxLayout *childlayout;
  QSpacerItem *childspacer;
  QWidget     *childwidget;

  GlobalSizeWidget sw(QSize(200,300), QSize(200,200));
  layout->addWidget(&sw);
  setLayout(layout);
  layout->addWidget(tabwidget);

  float pW                       = data->meta.LPub.page.size.value(0);
  PageMeta       *pageMeta       = &data->meta.LPub.page;
  PageHeaderMeta *pageHeaderMeta = &data->meta.LPub.page.pageHeader;
  pageHeaderMeta->size.setValue(0, pW);
  PageFooterMeta *pageFooterMeta = &data->meta.LPub.page.pageFooter;
  pageFooterMeta->size.setValue(0, pW);

  QPalette readOnlyPalette       = QApplication::palette();
  if (Preferences::darkTheme)
      readOnlyPalette.setColor(QPalette::Base,QColor(Preferences::themeColors[THEME_DARK_PALETTE_MIDLIGHT]));
  else
      readOnlyPalette.setColor(QPalette::Base,QColor(Preferences::themeColors[THEME_DEFAULT_PALETTE_LIGHT]));

  /*
   * Page tab
   */

  QWidget *widget = new QWidget();
  widget->setObjectName(tr("Page"));
  widget->setWhatsThis(lpubWT(WT_SETUP_PAGE_VIEW,widget->objectName()));
  childlayout = new QVBoxLayout();
  widget->setLayout(childlayout);

  bool dpi = lpub->page.meta.LPub.resolution.type() == DPI;
  QString header = (dpi ? tr("Size and Orientation (Inches)") : tr("Size and Orientation (Centimetres)"));

  box = new QGroupBox(header);
  childlayout->addWidget(box);
  child = new SizeAndOrientationGui("",&pageMeta->size,&pageMeta->orientation,box);
  data->children.append(child);

  box = new QGroupBox(tr("Background"));
  childlayout->addWidget(box);

  //gradient settings
  if (pageMeta->background.value().gsize[0] == 0 &&
      pageMeta->background.value().gsize[1] == 0) {

      pageMeta->background.value().gsize[0] = Preferences::pageHeight;
      pageMeta->background.value().gsize[1] = Preferences::pageWidth;
      QSize gSize(pageMeta->background.value().gsize[0],
                  pageMeta->background.value().gsize[1]);
      int h_off = gSize.width() / 10;
      int v_off = gSize.height() / 8;
      pageMeta->background.value().gpoints << QPointF(gSize.width() / 2, gSize.height() / 2)
                                           << QPointF(gSize.width() / 2 - h_off, gSize.height() / 2 - v_off);
  }

  child = new BackgroundGui(&pageMeta->background,box);
  data->children.append(child);

  box = new QGroupBox(tr("Border"));
  childlayout->addWidget(box);
  child = new BorderGui(&pageMeta->border,box);
  data->children.append(child);

  box = new QGroupBox(tr("Margins"));
  childlayout->addWidget(box);
  child = new UnitsGui(tr("L/R|T/B"),&pageMeta->margin,box);
  data->children.append(child);

  box = new QGroupBox(tr("Header Height"));
  childlayout->addWidget(box);
  child = new HeaderFooterHeightGui("",&pageHeaderMeta->size,box);
  data->children.append(child);

  box = new QGroupBox(tr("Footer Height"));
  childlayout->addWidget(box);
  child = new HeaderFooterHeightGui("",&pageFooterMeta->size,box);
  data->children.append(child);

  tabwidget->addTab(widget,widget->objectName());

  /*
   * Pointers tab
   */

  widget = new QWidget(nullptr);
  widget->setObjectName(tr("Pointers"));
  widget->setWhatsThis(lpubWT(WT_SETUP_SHARED_POINTERS,widget->objectName()));
  childlayout = new QVBoxLayout(nullptr);
  widget->setLayout(childlayout);

  box = new QGroupBox(tr("Border"));
  childlayout->addWidget(box);
  PointerAttribData pad = pageMeta->pointerAttrib.value();
  pad.attribType = PointerAttribData::Border;
  pageMeta->pointerAttrib.setValue(pad);
  pointerBorderChild = new PointerAttribGui(&pageMeta->pointerAttrib,box);
  data->children.append(pointerBorderChild);

  box = new QGroupBox(tr("Line"));
  childlayout->addWidget(box);
  pad.attribType = PointerAttribData::Line;
  pageMeta->pointerAttrib.setValue(pad);
  pointerLineChild = new PointerAttribGui(&pageMeta->pointerAttrib,box);
  connect (pointerLineChild->getHideTipCheck(), SIGNAL(clicked(bool)),
           this,                                SLOT(enablePointerTip(bool)));
  data->children.append(pointerLineChild);

  box = new QGroupBox(("Tip"));
  childlayout->addWidget(box);
  pad.attribType = PointerAttribData::Tip;
  pageMeta->pointerAttrib.setValue(pad);
  pointerTipChild = new PointerAttribGui(&pageMeta->pointerAttrib,box);
  data->children.append(pointerTipChild);

  childspacer = new QSpacerItem(1,1,QSizePolicy::Fixed,QSizePolicy::Expanding);
  childlayout->addSpacerItem(childspacer);

  tabwidget->addTab(widget,widget->objectName());

  /*
   * Model parent tab
   */

  childwidget = new QWidget();
  childwidget->setObjectName(tr("Model"));
  childwidget->setWhatsThis(lpubWT(WT_SETUP_PAGE_MODEL,childwidget->objectName()));
  childlayout = new QVBoxLayout;
  childwidget->setLayout(childlayout);

  childtabwidget = new QTabWidget();
  childlayout->addWidget(childtabwidget);
  tabwidget->addTab(childwidget,childwidget->objectName());

  /*
   * Title tab
   */

  widget = new QWidget();                       // existing 'model' widget definition
  widget->setObjectName(tr("Title"));
  widget->setWhatsThis(lpubWT(WT_SETUP_PAGE_TITLE,widget->objectName()));
  childlayout = new QVBoxLayout();
  widget->setLayout(childlayout);

  titleBoxFront = new QGroupBox(tr("Display Title Front Cover"));
  childlayout->addWidget(titleBoxFront);
  titleChildFront = new PageAttributeTextGui(&pageMeta->titleFront,titleBoxFront);
  titleChildFront->getContentEditGBox()->setTitle(tr("Title Description"));
  titleChildFront->getContentLineEdit()->setToolTip(tr("Enter front cover document title"));
  data->children.append(titleChildFront);
  connect(titleChildFront, SIGNAL(indexChanged(int)),
          this,            SLOT(indexChanged(int)));

  titleBoxBack = new QGroupBox(tr("Display Title Back Cover"));
  childlayout->addWidget(titleBoxBack);
  titleBoxBack->hide();
  titleChildBack = new PageAttributeTextGui(&pageMeta->titleBack,titleBoxBack);
  titleChildBack->getContentEditGBox()->setTitle(tr("Title Description"));
  titleChildFront->getContentLineEdit()->setToolTip(tr("Enter back cover document title"));
  data->children.append(titleChildBack);
  connect(titleChildBack, SIGNAL(indexChanged(int)),
          this,           SLOT(indexChanged(int)));

  childlayout->addSpacerItem(childspacer);

  childtabwidget->addTab(widget,widget->objectName());

  /*
   * Cover Image tab
   */

  widget = new QWidget();
  widget->setObjectName(tr("Cover Image"));
  widget->setWhatsThis(lpubWT(WT_SETUP_PAGE_COVER_IMAGE,widget->objectName()));
  childlayout = new QVBoxLayout();
  widget->setLayout(childlayout);

  coverPageBox = new QGroupBox(tr("Display Cover Image"));
  childlayout->addWidget(coverPageBox);
  coverImageChildFront = new PageAttributeImageGui(&pageMeta->coverImage,coverPageBox);
  coverImageChildFront->getImageEdit()->setToolTip(tr("Enter conver page image path"));
  data->children.append(coverImageChildFront);
  connect(coverPageBox, SIGNAL(toggled(bool)),
          this,         SLOT(displayGroup(bool)));

  coverPageBorderBox = new QGroupBox(tr("Border"));
  coverPageBorderBox->setEnabled(coverPageBox->isChecked());
  childlayout->addWidget(coverPageBorderBox);
  coverImageBorderChildFront = new BorderGui(&pageMeta->coverImage.border,coverPageBorderBox);
  data->children.append(coverImageBorderChildFront);

  childlayout->addSpacerItem(childspacer);

  childtabwidget->addTab(widget,widget->objectName());

  /*
   * Description tab
   */

  widget = new QWidget();
  widget->setObjectName(tr("Description"));
  widget->setWhatsThis(lpubWT(WT_SETUP_PAGE_DESCRIPTION,widget->objectName()));
  childlayout = new QVBoxLayout();
  widget->setLayout(childlayout);

  box = new QGroupBox(tr("Display Model Description"));
  childlayout->addWidget(box);
  modelDescChildFront = new PageAttributeTextGui(&pageMeta->modelDesc,box);
  modelDescChildFront->getContentEditGBox()->setTitle(tr("Model Description"));
  modelDescChildFront->getContentTextEdit()->setToolTip(tr("Enter model description"));
  data->children.append(modelDescChildFront);

  childtabwidget->addTab(widget,widget->objectName());

  /*
   * Model Parts and ID tab
   */

  widget = new QWidget();
  widget->setObjectName(tr("Model ID / Parts"));
  widget->setWhatsThis(lpubWT(WT_SETUP_PAGE_MODEL_PARTS_ID,widget->objectName()));
  childlayout = new QVBoxLayout();
  widget->setLayout(childlayout);

  box = new QGroupBox(tr("Display Model Identification"));
  childlayout->addWidget(box);
  modelIdChildFront = new PageAttributeTextGui(&pageMeta->modelName,box);
  modelIdChildFront->getContentEditGBox()->setTitle(tr("Model Identification"));
  modelIdChildFront->getContentLineEdit()->setToolTip(tr("Enter model identification"));
  data->children.append(modelIdChildFront);

  box = new QGroupBox(tr("Display Parts"));
  childlayout->addWidget(box);
  partsChildFront = new PageAttributeTextGui(&pageMeta->parts,box);
  partsChildFront->getContentEditGBox()->setTitle(tr("Parts Count"));
  partsChildFront->getContentLineEdit()->setToolTip(tr("Enter number of parts - e.g. 420 Parts"));
  data->children.append(partsChildFront);

  childtabwidget->addTab(widget,widget->objectName());

  /*
   * Submodel Level Color tab
   */

  widget = new QWidget();
  widget->setObjectName(tr("Colors"));
  widget->setWhatsThis(lpubWT(WT_SETUP_SHARED_SUBMODEL_LEVEL_COLORS,widget->objectName()));
  childlayout = new QVBoxLayout();
  widget->setLayout(childlayout);

  box = new QGroupBox(tr("Submodel Level Colors"));
  childlayout->addWidget(box);
  child = new SubModelColorGui(&pageMeta->subModelColor,box);
  data->children.append(child);

  childlayout->addSpacerItem(childspacer);

  childtabwidget->addTab(widget,widget->objectName());

  /*
   * Publish parent tab
   */

  childwidget = new QWidget();
  childwidget->setObjectName(tr("Publish"));
  childwidget->setWhatsThis(lpubWT(WT_SETUP_PAGE_PUBLISH,childwidget->objectName()));
  childlayout = new QVBoxLayout();
  childwidget->setLayout(childlayout);

  childtabwidget = new QTabWidget();
  childlayout->addWidget(childtabwidget);
  tabwidget->addTab(childwidget,childwidget->objectName());

  /*
   * Author and Email tab
   */

  widget = new QWidget();
  widget->setObjectName(tr("Author / Email"));
  widget->setWhatsThis(lpubWT(WT_SETUP_PAGE_AUTHOR_EMAIL,widget->objectName()));
  childlayout = new QVBoxLayout();
  widget->setLayout(childlayout);

  authorBoxFront = new QGroupBox(tr("Display Author Front Cover"));
  childlayout->addWidget(authorBoxFront);
  authorChildFront = new PageAttributeTextGui(&pageMeta->authorFront,authorBoxFront);
  authorChildFront->getContentEditGBox()->setTitle(tr("Author Name"));
  authorChildFront->getContentLineEdit()->setToolTip(tr("Enter model author"));
  data->children.append(authorChildFront);
  connect(authorChildFront, SIGNAL(indexChanged(int)),
          this,             SLOT(indexChanged(int)));

  authorBoxBack = new QGroupBox(tr("Display Author Back Cover"));
  childlayout->addWidget(authorBoxBack);
  authorBoxBack->hide();
  authorChildBack = new PageAttributeTextGui(&pageMeta->authorBack,authorBoxBack);
  authorChildBack->getContentEditGBox()->setTitle(tr("Author Name"));
  authorChildBack->getContentLineEdit()->setToolTip(tr("Enter model author"));
  data->children.append(authorChildBack);
  connect(authorChildBack, SIGNAL(indexChanged(int)),
          this,            SLOT(indexChanged(int)));

  authorBox = new QGroupBox(tr("Display Author Header/Footer"));
  childlayout->addWidget(authorBox);
  authorBox->hide();
  authorChildHeaderFooter = new PageAttributeTextGui(&pageMeta->author,authorBox);
  authorChildHeaderFooter->getContentEditGBox()->setTitle(tr("Author Name"));
  authorChildHeaderFooter->getContentLineEdit()->setToolTip(tr("Enter model author"));
  data->children.append(authorChildHeaderFooter);
  connect(authorChildHeaderFooter, SIGNAL(indexChanged(int)),
          this,                    SLOT(indexChanged(int)));

  emailBoxBack = new QGroupBox(tr("Display Email Back Cover"));
  childlayout->addWidget(emailBoxBack);
  emailChildBack = new PageAttributeTextGui(&pageMeta->emailBack,emailBoxBack);
  emailChildBack->getContentEditGBox()->setTitle(tr("Email Address"));
  emailChildBack->getContentLineEdit()->setToolTip(tr("Enter email address"));
  data->children.append(emailChildBack);
  connect(emailChildBack, SIGNAL(indexChanged(int)),
          this,           SLOT(indexChanged(int)));

  emailBox = new QGroupBox(tr("Display Email Header/Footer"));
  childlayout->addWidget(emailBox);
  emailBox->hide();
  emailChildHeaderFooter = new PageAttributeTextGui(&pageMeta->email,emailBox);
  emailChildHeaderFooter->getContentEditGBox()->setTitle(tr("Email Address"));
  emailChildHeaderFooter->getContentLineEdit()->setToolTip(tr("Enter email address"));
  data->children.append(emailChildHeaderFooter);
  connect(emailChildHeaderFooter, SIGNAL(indexChanged(int)),
          this,                   SLOT(indexChanged(int)));

  childtabwidget->addTab(widget,widget->objectName());

  /*
   * URL tab
   */

  widget = new QWidget();
  widget->setObjectName(tr("URL / Copyright"));
  widget->setWhatsThis(lpubWT(WT_SETUP_PAGE_WEBSITE_URL_COPYRIGHT,widget->objectName()));
  childlayout = new QVBoxLayout();
  widget->setLayout(childlayout);

  urlBoxBack = new QGroupBox(tr("Display URL Back Cover"));
  childlayout->addWidget(urlBoxBack);
  urlChildBack = new PageAttributeTextGui(&pageMeta->urlBack,urlBoxBack);
  urlChildBack->getContentEditGBox()->setTitle(tr("URL"));
  urlChildBack->getContentLineEdit()->setToolTip(tr("Enter website URL (Universal Resource Locator)"));
  data->children.append(urlChildBack);
  connect(urlChildBack, SIGNAL(indexChanged(int)),
          this,         SLOT(indexChanged(int)));

  urlBox = new QGroupBox(tr("Display URL Header/Footer"));
  childlayout->addWidget(urlBox);
  urlBox->hide();
  urlChildHeaderFooter = new PageAttributeTextGui(&pageMeta->url,urlBox);
  urlChildHeaderFooter->getContentEditGBox()->setTitle(tr("URL"));
  urlChildHeaderFooter->getContentLineEdit()->setToolTip(tr("Enter website URL (Universal Resource Locator)"));
  data->children.append(urlChildHeaderFooter);
  connect(urlChildHeaderFooter, SIGNAL(indexChanged(int)),
          this,                 SLOT(indexChanged(int)));

  copyrightBoxBack = new QGroupBox(tr("Display Copyright Back Cover"));
  childlayout->addWidget(copyrightBoxBack);
  copyrightChildBack = new PageAttributeTextGui(&pageMeta->copyrightBack,copyrightBoxBack);
  copyrightChildBack->getContentEditGBox()->setTitle(tr("Copyright Description"));
  copyrightChildBack->getContentLineEdit()->setToolTip(tr("Enter copyright - %1").arg(Preferences::copyright));
  data->children.append(copyrightChildBack);
  connect(copyrightChildBack, SIGNAL(indexChanged(int)),
          this,               SLOT(indexChanged(int)));

  copyrightBox = new QGroupBox(tr("Display Copyright Header/Footer"));
  childlayout->addWidget(copyrightBox);
  copyrightBox->hide();
  copyrightChildHeaderFooter = new PageAttributeTextGui(&pageMeta->copyright,copyrightBox);
  copyrightChildHeaderFooter->getContentEditGBox()->setTitle(tr("Copyright Description"));
  copyrightChildHeaderFooter->getContentLineEdit()->setToolTip(tr("Enter copyright - %1").arg(Preferences::copyright));
  data->children.append(copyrightChildHeaderFooter);
  connect(copyrightChildHeaderFooter, SIGNAL(indexChanged(int)),
          this,                       SLOT(indexChanged(int)));

  childtabwidget->addTab(widget,widget->objectName());

  /*
   * Publish Description tab
   */

  widget = new QWidget();
  widget->setObjectName(tr("Description"));
  widget->setWhatsThis(lpubWT(WT_SETUP_PAGE_PUBLISH_DESCRIPTION,widget->objectName()));
  childlayout = new QVBoxLayout();
  widget->setLayout(childlayout);

  box = new QGroupBox(tr("Publisher Description"));
  childlayout->addWidget(box);
  publishDescChildFront = new PageAttributeTextGui(&pageMeta->publishDesc,box);
  publishDescChildFront->getContentEditGBox()->setTitle(tr("Model Description"));
  publishDescChildFront->getContentTextEdit()->setToolTip(tr("Enter publisher model description"));
  data->children.append(publishDescChildFront);

  childtabwidget->addTab(widget,widget->objectName());

  /*
   * Document Logo tab
   */

  widget = new QWidget();
  widget->setObjectName(tr("Logo"));
  widget->setWhatsThis(lpubWT(WT_SETUP_PAGE_DOCUMENT_LOGO,widget->objectName()));
  childlayout = new QVBoxLayout();
  widget->setLayout(childlayout);

  documentLogoBoxFront = new QGroupBox(tr("Display Logo Front Cover"));
  childlayout->addWidget(documentLogoBoxFront);
  documentLogoChildFront = new PageAttributeImageGui(&pageMeta->documentLogoFront,documentLogoBoxFront);
  documentLogoChildFront->getImageEdit()->setToolTip(tr("Enter document logo image path"));
  data->children.append(documentLogoChildFront);
  connect(documentLogoChildFront, SIGNAL(indexChanged(int)),
          this,                   SLOT(indexChanged(int)));
  connect(documentLogoBoxFront,   SIGNAL(toggled(bool)),
          this,                   SLOT(displayGroup(bool)));

  documentLogoBorderBoxFront = new QGroupBox(tr("Logo Front Cover Border"));
  documentLogoBorderBoxFront->setEnabled(documentLogoBoxFront->isChecked());
  childlayout->addWidget(documentLogoBorderBoxFront);
  documentLogoBorderChildFront = new BorderGui(&pageMeta->plugImage.border,documentLogoBorderBoxFront);
  data->children.append(documentLogoBorderChildFront);

  documentLogoBoxBack = new QGroupBox(tr("Display Logo Back Cover"));
  childlayout->addWidget(documentLogoBoxBack);
  documentLogoBoxBack->hide();
  documentLogoChildBack = new PageAttributeImageGui(&pageMeta->documentLogoBack,documentLogoBoxBack);
  documentLogoChildBack->getImageEdit()->setToolTip(tr("Enter document logo image path"));
  data->children.append(documentLogoChildBack);
  connect(documentLogoChildBack, SIGNAL(indexChanged(int)),
          this,                  SLOT(indexChanged(int)));
  connect(documentLogoBoxBack,   SIGNAL(toggled(bool)),
          this,                  SLOT(displayGroup(bool)));

  documentLogoBorderBoxBack = new QGroupBox(tr("Logo Back Cover Border"));
  documentLogoBorderBoxBack->hide();
  documentLogoBorderBoxBack->setEnabled(documentLogoBoxBack->isChecked());
  childlayout->addWidget(documentLogoBorderBoxBack);
  documentLogoBorderChildBack = new BorderGui(&pageMeta->plugImage.border,documentLogoBorderBoxBack);
  data->children.append(documentLogoBorderChildBack);

  childlayout->addSpacerItem(childspacer);

  childtabwidget->addTab(widget,widget->objectName());

  /*
   * Disclaimer parent tab
   */

  childwidget = new QWidget();
  childwidget->setObjectName(tr("Disclaimer"));
  childwidget->setWhatsThis(lpubWT(WT_SETUP_PAGE_DISCLAIMER,childwidget->objectName()));
  childlayout = new QVBoxLayout;
  childwidget->setLayout(childlayout);

  childtabwidget = new QTabWidget();
  childlayout->addWidget(childtabwidget);
  tabwidget->addTab(childwidget,childwidget->objectName());

  /*
   * LEGO Disclaimer tab
   */

  widget = new QWidget();
  widget->setObjectName(tr("Disclaimer"));
  widget->setWhatsThis(lpubWT(WT_SETUP_PAGE_LEGO_DISCLAIMER,widget->objectName()));
  childlayout = new QVBoxLayout();
  widget->setLayout(childlayout);

  box = new QGroupBox(tr("Display LEGO Disclaimer"));
  childlayout->addWidget(box);
  disclaimerChildBack = new PageAttributeTextGui(&pageMeta->disclaimer,box);
  disclaimerChildBack->getContentEditGBox()->setTitle(tr("LEGO Disclaimer"));
  disclaimerChildBack->getContentTextEdit()->setToolTip(tr("Enter disclaimer paragraph"));
  data->children.append(disclaimerChildBack);

  childtabwidget->addTab(widget,widget->objectName());

  /*
   * LPub3D Plug tab
   */

  widget = new QWidget();
  widget->setObjectName(tr("%1 Plug").arg(VER_PRODUCTNAME_STR));
  widget->setWhatsThis(lpubWT(WT_SETUP_PAGE_LPUB3D_PLUG, widget->objectName()));
  childlayout = new QVBoxLayout();
  widget->setLayout(childlayout);

  box = new QGroupBox(tr("Display %1 Plug").arg(VER_PRODUCTNAME_STR));
  childlayout->addWidget(box);
  readOnlyPalette.setColor(QPalette::Text,QColor(LPUB3D_DISABLED_TEXT_COLOUR));
  lpub3dPlugChildBack = new PageAttributeTextGui(&pageMeta->plug,box);
  lpub3dPlugChildBack->getContentEditGBox()->setTitle(tr("%1 Plug Description").arg(VER_PRODUCTNAME_STR));
  lpub3dPlugChildBack->getContentTextEdit()->setReadOnly(true);
  lpub3dPlugChildBack->getContentTextEdit()->setPalette(readOnlyPalette);
  data->children.append(lpub3dPlugChildBack);

  childlayout->addSpacerItem(childspacer);

  childtabwidget->addTab(widget,widget->objectName());

  /*
   * LPub"D Logo tab
   */

  widget = new QWidget();
  widget->setObjectName(tr("%1 Logo").arg(VER_PRODUCTNAME_STR));
  widget->setWhatsThis(lpubWT(WT_SETUP_PAGE_LPUB3D_LOGO,widget->objectName()));
  childlayout = new QVBoxLayout();
  widget->setLayout(childlayout);

  lpub3dLogoBox = new QGroupBox(tr("Display %1 Logo").arg(VER_PRODUCTNAME_STR));
  childlayout->addWidget(lpub3dLogoBox);
  lpub3dLogoChildBack = new PageAttributeImageGui(&pageMeta->plugImage,lpub3dLogoBox);
  readOnlyPalette.setColor(QPalette::Base,Qt::lightGray);
  lpub3dLogoChildBack->getImageEdit()->setReadOnly(true);
  lpub3dLogoChildBack->getImageEdit()->setPalette(readOnlyPalette);
  lpub3dLogoChildBack->getImageEdit()->setEnabled(false);
  data->children.append(lpub3dLogoChildBack);
  connect(lpub3dLogoBox, SIGNAL(toggled(bool)),
          this,          SLOT(displayGroup(bool)));

  lpub3dLogoBorderBox = new QGroupBox(tr("Logo Border"));
  lpub3dLogoBorderBox->setEnabled(lpub3dLogoBorderBox->isChecked());
  childlayout->addWidget(lpub3dLogoBorderBox);
  lpub3dLogoBorderChildBack = new BorderGui(&pageMeta->plugImage.border,lpub3dLogoBorderBox);
  data->children.append(lpub3dLogoBorderChildBack);

  childlayout->addSpacerItem(childspacer);

  childtabwidget->addTab(widget,widget->objectName());

  /*
   * Page Number and Text Placement tab
   */

  widget = new QWidget();
  widget->setObjectName(tr("Number / Text"));
  widget->setWhatsThis(lpubWT(WT_SETUP_PAGE_NUMBER_TEXT,widget->objectName()));
  childlayout = new QVBoxLayout();
  widget->setLayout(childlayout);

  box = new QGroupBox(tr("Display"));
  box->setWhatsThis(lpubWT(WT_SETUP_PAGE_NUMBER_DISPLAY,box->title()));
  childlayout->addWidget(box);
  child = new CheckBoxGui(tr("Display Page Number"),&pageMeta->dpn,box);
  data->children.append(child);

  box = new QGroupBox(tr("Look"));
  childlayout->addWidget(box);
  child = new NumberGui("",&pageMeta->number,box);
  data->children.append(child);

  box = new QGroupBox(tr("Page Number Placement"));
  box->setWhatsThis(lpubWT(WT_SETUP_PAGE_NUMBER_PLACEMENT,box->title()));
  childlayout->addWidget(box);
  child = new BoolRadioGui(
    tr("Alternate Corners (like books)"),
    tr("Page Number Always in Same Place"),
    &pageMeta->togglePnPlacement,box);
  data->children.append(child);

  // text placement
  box = new QGroupBox(tr("Text Placement"));
  box->setWhatsThis(lpubWT(WT_SETUP_PAGE_TEXT_PLACEMENT,box->title()));
  childlayout->addWidget(box);
  QHBoxLayout *childHLayout = new QHBoxLayout(nullptr);
  box->setLayout(childHLayout);
  CheckBoxGui *childTextPlacement = new CheckBoxGui(tr("Enable Text Placement"),&pageMeta->textPlacement);
  childTextPlacement->setToolTip(tr("Launch the text placement dialog when inserting or updating text."));
  childHLayout->addWidget(childTextPlacement);
  textPlacementChild = new PlacementGui(tr("%1 Placement").arg(PlacementDialog::placementTypeName(TextType)), &pageMeta->textPlacementMeta, TextType);
  textPlacementChild->setEnabled(pageMeta->textPlacement.value());
  childHLayout->addWidget(textPlacementChild);
  // these are placed in reverse order so the meta commands are properly written
  data->children.append(textPlacementChild);
  data->children.append(childTextPlacement);
  connect (childTextPlacement->getCheckBox(), SIGNAL(clicked(bool)), this, SLOT(enableTextPlacement(bool)));

  childlayout->addSpacerItem(childspacer);

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

void GlobalPageDialog::displayGroup(bool b) {
    if (sender() == coverPageBox)
        coverPageBorderBox->setEnabled(b);
    if (sender() == lpub3dLogoBox)
        lpub3dLogoBorderBox->setEnabled(b);

    if (sender() == documentLogoBoxBack)
        documentLogoBorderBoxBack->setEnabled(b);
    if (sender() == documentLogoBoxFront)
        documentLogoBorderBoxFront->setEnabled(b);
}

void GlobalPageDialog::enablePointerTip(bool b) {
    pointerTipChild->setEnabled(b);
}

void GlobalPageDialog::enableTextPlacement(bool b) {
    textPlacementChild->setEnabled(b);
}

void GlobalPageDialog::indexChanged(int selection) {

    sectionIndex = selection;

    QObject *obj = sender();

//    logTrace() << " RECIEVED SINGAL: "
//               << " INDEX: " << sectionIndex
//               << " Sender Class Name: "  << sender()->metaObject()->className()
//               << " Sender Object Name: " << sender()->objectName()
                  ;
    if (obj == authorChildFront || obj == authorChildBack || obj == authorChildHeaderFooter) {
        switch(sectionIndex) {
        case 0: //FrontCover
            logTrace() << " AUTHOR FRONT COVER: ";
            authorBoxFront->show();
            authorBoxBack->hide();
            authorBox->hide();
            break;
        case 1: //BackCover
            logTrace() << " AUTHOR BACK COVER: ";
            authorBoxFront->hide();
            authorBoxBack->show();
            authorBox->hide();
            break;
        case 2: //Header
        case 3: //Footer
            logTrace() << " AUTHOR HEADER/FOOTER: ";
            authorBoxFront->hide();
            authorBoxBack->hide();
            authorBox->show();
            break;
        }
    }
    else if (obj == titleChildFront || obj == titleChildBack) {
        switch(sectionIndex) {
        case 0: //FrontCover
            logTrace() << " TITLE FRONT COVER: ";
            titleBoxFront->show();            
            titleBoxBack->hide();
            break;
        case 1: //BackCover
            logTrace() << " TITLE BACK COVER: ";
            titleBoxFront->hide();
            titleBoxBack->show();
            break;
        }
    }
    else if (obj == urlChildBack || obj == urlChildHeaderFooter) {
        switch(sectionIndex) {
        case 0: //BackCover
            urlBoxBack->show();
            urlBox->hide();
            break;
        case 1: //Header
        case 2: //Footer
            urlBoxBack->hide();
            urlBox->show();
            break;
        }
    }
    else if (obj == emailChildBack || obj == emailChildHeaderFooter) {
        switch(sectionIndex) {
        case 0: //BackCover
            emailBoxBack->show();
            emailBox->hide();
            break;
        case 1: //Header
        case 2: //Footer
            emailBoxBack->hide();
            emailBox->show();
            break;
        }
    }
    else if (obj == copyrightChildHeaderFooter || obj == copyrightChildBack) {
        switch(sectionIndex) {
        case 0: //BackCover
            copyrightBoxBack->show();
            copyrightBox->hide();
            break;
        case 1: //Header
        case 2: //Footer
            copyrightBoxBack->hide();
            copyrightBox->show();
            break;
        }
    }
    else if (obj == documentLogoChildFront || obj == documentLogoChildBack) {
        switch(sectionIndex) {
        case 0: //FrontCover
            documentLogoBoxFront->show();
            documentLogoBoxBack->hide();
            documentLogoBorderBoxFront->show();
            documentLogoBorderBoxBack->hide();
            break;
        case 1: //BackCover
            documentLogoBoxFront->hide();
            documentLogoBoxBack->show();
            documentLogoBorderBoxFront->hide();
            documentLogoBorderBoxBack->show();
            break;
        }
    }
}

void GlobalPageDialog::getPageGlobals(
  QString topLevelFile, Meta &meta)
{
  GlobalPageDialog *dialog = new GlobalPageDialog(topLevelFile,meta);
  dialog->exec();
}

void GlobalPageDialog::accept()
{
  MetaItem mi;

  mi.beginMacro("GlobalPage");

  bool noFileDisplay = false;
  MetaGui *child;
  Q_FOREACH (child,data->children) {
      if ((child == authorChildFront && authorChildFront->modified) ||
          (child == titleChildFront && titleChildFront->modified) ||
          (child == documentLogoChildFront && documentLogoChildFront->modified) ||
          (child == coverImageChildFront && coverImageChildFront->modified) ||
          (child == modelDescChildFront && modelDescChildFront->modified) ||
          (child == modelIdChildFront && modelIdChildFront->modified) ||
          (child == partsChildFront && partsChildFront->modified) ||
          (child == publishDescChildFront && publishDescChildFront->modified)) {
          if (!mi.frontCoverPageExist())
              mi.insertCoverPage();
      }
      if ((child == authorChildBack && authorChildBack->modified) ||
          (child == titleChildBack && titleChildBack->modified) ||
          (child == documentLogoChildBack && documentLogoChildBack->modified) ||
          (child == urlChildBack && urlChildBack->modified) ||
          (child == copyrightChildBack && copyrightChildBack->modified) ||
          (child == emailChildBack && emailChildBack->modified) ||
          (child == disclaimerChildBack && disclaimerChildBack->modified) ||
          (child == lpub3dPlugChildBack && lpub3dPlugChildBack->modified) ||
          (child == lpub3dLogoChildBack && lpub3dLogoChildBack->modified)) {
          if (!mi.backCoverPageExist())
              mi.appendCoverPage();
      }
      child->apply(data->topLevelFile);
      noFileDisplay |= child->modified;
  }

  mi.setSuspendFileDisplayFlag(!noFileDisplay);

  mi.endMacro();

  QDialog::accept();
}

void GlobalPageDialog::cancel()
{
  QDialog::reject();
}
