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

  QTabWidget     *tab         = new QTabWidget();
  QTabWidget     *childtab    = new QTabWidget();
  QGridLayout    *grid        = new QGridLayout();
  QVBoxLayout    *layout      = new QVBoxLayout();
  QVBoxLayout    *childlayout = new QVBoxLayout();
  QSpacerItem    *vSpacer     = new QSpacerItem(1,1,QSizePolicy::Fixed,QSizePolicy::Expanding);

  QWidget        *widget      = new QWidget();
  QWidget        *childwidget;

  float pW                       = data->meta.LPub.page.size.value(0);
  PageMeta       *pageMeta       = &data->meta.LPub.page;
  PageHeaderMeta *pageHeaderMeta = &data->meta.LPub.page.pageHeader;
  pageHeaderMeta->size.setValue(0, pW);
  PageFooterMeta *pageFooterMeta = &data->meta.LPub.page.pageFooter;
  pageFooterMeta->size.setValue(0, pW);
  PageAttributeTextGui *childTextGui;
  PageAttributePictureGui *chilPicdGui;

  setLayout(layout);
  layout->addWidget(tab);
  widget->setLayout(grid);

  //~~~~~~~~~~~~ page tab ~~~~~~~~~~~~~~~~//
//  box = new QGroupBox(tr("Size"));
//  grid->addWidget(box,0,0);
//  child = new PageSizeGui("",&pageMeta->size,box);
//  data->children.append(child);

//  box = new QGroupBox(tr("Orientation"));
//  grid->addWidget(box,1,0);
//  child = new PageOrientationGui("",&pageMeta->orientation,box);
//  data->children.append(child);

  bool dpi = gui->page.meta.LPub.resolution.type() == DPI;
  QString header = (dpi ? "Size and Orientation (Inches) " : "Size and Orientation (Centimeters)" );

  box = new QGroupBox(tr("%1").arg(header));
  grid->addWidget(box,0,0);
  child = new SizeAndOrientationGui("",&pageMeta->size,&pageMeta->orientation,box);
  data->children.append(child);

  box = new QGroupBox(tr("Background"));
  grid->addWidget(box,2,0);

  //gradient settings
//  logTrace() << "\nbackground.value().gsize[0]: " << pageMeta->background.value().gsize[0]
//             << "\nbackground.value().gsize[1]: " << pageMeta->background.value().gsize[1]
//                ;
  if (pageMeta->background.value().gsize[0] == 0 &&
      pageMeta->background.value().gsize[1] == 0) {

      pageMeta->background.value().gsize[0] = Preferences::pageHeight;
      pageMeta->background.value().gsize[1] = Preferences::pageWidth;
//      logTrace() << "\nbackground.value().gsize[0]: " << pageMeta->background.value().gsize[0]
//                 << "\nbackground.value().gsize[1]: " << pageMeta->background.value().gsize[1]
//                    ;
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
  grid->addWidget(box,3,0);
  child = new BorderGui(&pageMeta->border,box);
  data->children.append(child);

  box = new QGroupBox(tr("Margins"));
  grid->addWidget(box,4,0);
  child = new UnitsGui("",&pageMeta->margin,box);
  data->children.append(child);

  box = new QGroupBox(tr("Header Height"));
  grid->addWidget(box,5,0);
  child = new HeaderFooterHeightGui("",&pageHeaderMeta->size,box);
  data->children.append(child);

  box = new QGroupBox(tr("Footer Height"));
  grid->addWidget(box,6,0);
  child = new HeaderFooterHeightGui("",&pageFooterMeta->size,box);
  data->children.append(child);



  tab->addTab(widget,"Page");

  //~~~~~~~~~~~~ model tab ~~~~~~~~~~~~~~~//
  childwidget = new QWidget();                  //START DO THIS FOR MODEL, PUBLISHER AND DISCLAIMER
  childlayout = new QVBoxLayout;                //new QVBox layout - to apply tabl later
  childwidget->setLayout(childlayout);          //new 'model' widget - tab not yet added

  childtab = new QTabWidget();                  // new tab object
  childlayout->addWidget(childtab);             // new add the tab
  tab->addTab(childwidget, tr("Model"));        // new add the tab  (This is the new 'Model' tab
                                                // END

  // child header (one) start
  widget = new QWidget();                       // existing 'model' widget definition
  childlayout = new QVBoxLayout();
  widget->setLayout(childlayout);
  // child header end

  /*
    Title,
    Cover Image
  */

  //child body (many) start
  titleBoxFront = new QGroupBox(tr("Display Title Front Cover"));
  childlayout->addWidget(titleBoxFront);
  //grid->addWidget(titleBoxFront, 0, 0);
  titleChildFront = new PageAttributeTextGui(&pageMeta->titleFront,titleBoxFront);
  childTextGui = static_cast<PageAttributeTextGui*>(titleChildFront);
  childTextGui->contentEdit->setToolTip("Enter model title");
  data->children.append(titleChildFront);
  connect(titleChildFront, SIGNAL(indexChanged(int)),
          SLOT(indexChanged(int)));

  titleBoxBack = new QGroupBox(tr("Display Title Back Cover"));
  childlayout->addWidget(titleBoxBack);
  //grid->addWidget(titleBoxBack, 0, 0);
  titleBoxBack->hide();
  titleChildBack = new PageAttributeTextGui(&pageMeta->titleBack,titleBoxBack);
  childTextGui = static_cast<PageAttributeTextGui*>(titleChildBack);
  childTextGui->contentEdit->setToolTip("Enter model title");
  data->children.append(titleChildBack);
  connect(titleChildBack, SIGNAL(indexChanged(int)),
          SLOT(indexChanged(int)));
  //child body end

  //child body (many) start
  box = new QGroupBox(tr("Display Cover Image"));
  childlayout->addWidget(box);
  //grid->addWidget(box, 1, 0);
  child = new PageAttributePictureGui(&pageMeta->coverImage,box);
  chilPicdGui = static_cast<PageAttributePictureGui*>(child);
  chilPicdGui->pictureEdit->setToolTip("Enter image path");
  data->children.append(child);
  //child body end

  //spacer
  childlayout->addSpacerItem(vSpacer);

  // child footer (one) end
  childtab->addTab(widget,"Title/Cover Image");
  // child footer end

  // child header (one) start
  widget = new QWidget();
  grid = new QGridLayout();
  widget->setLayout(grid);
  // child header end
  /*
    Description,
    Description Dialog,
  */
  // child body (many) start
  box = new QGroupBox(tr("Display Model Description"));
  grid->addWidget(box, 0, 0);
  child = new PageAttributeTextGui(&pageMeta->modelDesc,box);
  childTextGui = static_cast<PageAttributeTextGui*>(child);
  childTextGui->contentEdit->setToolTip("Enter model description");
  data->children.append(child);
  // child body end

  // child footer (one) end
  childtab->addTab(widget,tr("Description"));
  // child footer end

  // child header (one) start
  widget = new QWidget();
  grid = new QGridLayout();
  widget->setLayout(grid);
  // child header end
  /*
    Model ID,
    Pieces,
  */
  // child body (many) start
  box = new QGroupBox(tr("Display Model Identification"));
  grid->addWidget(box, 0, 0);
  child = new PageAttributeTextGui(&pageMeta->modelName,box);
  childTextGui = static_cast<PageAttributeTextGui*>(child);
  childTextGui->contentEdit->setToolTip("Enter model identification");
  data->children.append(child);
  // child body end

  // child body (many) start
  box = new QGroupBox(tr("Display Pieces"));
  grid->addWidget(box, 1, 0);
  child = new PageAttributeTextGui(&pageMeta->pieces,box);
  childTextGui = static_cast<PageAttributeTextGui*>(child);
  childTextGui->contentEdit->setToolTip(" Enter number of parts - e.g. 420 Pieces");
  data->children.append(child);
  // child body end

  // child footer (one) end
  childtab->addTab(widget,tr("Model ID/Pieces"));
  // child footer end

  /*
    SubModel Color,
  */
  // child header (one) start
  widget = new QWidget();
  childlayout = new QVBoxLayout();
  widget->setLayout(childlayout);
  // child header end

  //child body (many) start
  box = new QGroupBox(tr("Sub-Model Level Colors"));
  childlayout->addWidget(box);
  child = new SubModelColorGui(&pageMeta->subModelColor,box);
  data->children.append(child);
  //child body end

  /*
    "Native Renderer Camera Distance",
  */
  //child body (many) start
  box = new QGroupBox(tr("Native Renderer Camera Distance"));
  childlayout->addWidget(box);
  child = new CameraDistFactorGui("Factor",&pageMeta->cameraDistNative,box);
  data->children.append(child);
  //child body end

  //spacer
  childlayout->addSpacerItem(vSpacer);

  // child footer (one) end
  childtab->addTab(widget,"SubModel/Factor");
  // child footer end

  //~~~~~~~~~~~~ publisher tab ~~~~~~~~~~~~//
  childwidget = new QWidget();
  childlayout = new QVBoxLayout();
  childwidget->setLayout(childlayout);

  childtab = new QTabWidget();
  childlayout->addWidget(childtab);
  tab->addTab(childwidget, tr("Publish"));

  widget = new QWidget();
  grid = new QGridLayout();
  widget->setLayout(grid);
  /*
    Author,
    Email,
  */
  authorBoxFront = new QGroupBox(tr("Display Author Front Cover"));
  grid->addWidget(authorBoxFront, 0, 0);
  authorChildFront = new PageAttributeTextGui(&pageMeta->authorFront,authorBoxFront);
  childTextGui = static_cast<PageAttributeTextGui*>(authorChildFront);
  childTextGui->contentEdit->setToolTip("Enter model author");
  data->children.append(authorChildFront);
  connect(authorChildFront, SIGNAL(indexChanged(int)),
          SLOT(indexChanged(int)));

  authorBoxBack = new QGroupBox(tr("Display Author Back Cover"));
  grid->addWidget(authorBoxBack, 0, 0);
  authorBoxBack->hide();
  authorChildBack = new PageAttributeTextGui(&pageMeta->authorBack,authorBoxBack);
  childTextGui = static_cast<PageAttributeTextGui*>(authorChildBack);
  childTextGui->contentEdit->setToolTip("Enter model author");
  data->children.append(authorChildBack);
  connect(authorChildBack, SIGNAL(indexChanged(int)),
          SLOT(indexChanged(int)));

  authorBox = new QGroupBox(tr("Display Author Header/Footer"));
  grid->addWidget(authorBox, 0, 0);
  authorBox->hide();
  authorChild = new PageAttributeTextGui(&pageMeta->author,authorBox);
  childTextGui = static_cast<PageAttributeTextGui*>(authorChild);
  childTextGui->contentEdit->setToolTip("Enter model author");
  data->children.append(authorChild);
  connect(authorChild, SIGNAL(indexChanged(int)),
          SLOT(indexChanged(int)));

  emailBoxBack = new QGroupBox(tr("Display Email Back Cover"));
  grid->addWidget(emailBoxBack, 1, 0);
  emailChildBack = new PageAttributeTextGui(&pageMeta->emailBack,emailBoxBack);
  childTextGui = static_cast<PageAttributeTextGui*>(emailChildBack);
  childTextGui->contentEdit->setToolTip("Enter email address");
  data->children.append(emailChildBack);
  connect(emailChildBack, SIGNAL(indexChanged(int)),
          SLOT(indexChanged(int)));

  emailBox = new QGroupBox(tr("Display Email Header/Footer"));
  grid->addWidget(emailBox, 1, 0);
  emailBox->hide();
  emailChild = new PageAttributeTextGui(&pageMeta->email,emailBox);
  childTextGui = static_cast<PageAttributeTextGui*>(emailChild);
  childTextGui->contentEdit->setToolTip("");
  data->children.append(emailChild);
  connect(emailChild, SIGNAL(indexChanged(int)),
          SLOT(indexChanged(int)));

  childtab->addTab(widget,tr("Author/Email"));

  widget = new QWidget();
  grid = new QGridLayout();
  widget->setLayout(grid);
  /*
    Publish desription
    URL
  */
  urlBoxBack = new QGroupBox(tr("Display URL Back Cover"));
  grid->addWidget(urlBoxBack, 0, 0);
  urlChildBack = new PageAttributeTextGui(&pageMeta->urlBack,urlBoxBack);
  childTextGui = static_cast<PageAttributeTextGui*>(urlChildBack);
  childTextGui->contentEdit->setToolTip("Enter website URL");
  data->children.append(urlChildBack);
  connect(urlChildBack, SIGNAL(indexChanged(int)),
         SLOT(indexChanged(int)));

  urlBox = new QGroupBox(tr("Display URL Header/Footer"));
  grid->addWidget(urlBox, 0, 0);
  urlBox->hide();
  urlChild = new PageAttributeTextGui(&pageMeta->url,urlBox);
  childTextGui = static_cast<PageAttributeTextGui*>(urlChild);
  childTextGui->contentEdit->setToolTip("Enter website URL");
  data->children.append(urlChild);
  connect(urlChild, SIGNAL(indexChanged(int)),
         SLOT(indexChanged(int)));

  box = new QGroupBox(tr("Publisher Description"));
  grid->addWidget(box, 1, 0);
  child = new PageAttributeTextGui(&pageMeta->publishDesc,box);
  childTextGui = static_cast<PageAttributeTextGui*>(child);
  childTextGui->contentEdit->setToolTip("Enter model publisher");
  data->children.append(child);

  childtab->addTab(widget,tr("URL/Description"));

  widget = new QWidget();
  childlayout = new QVBoxLayout();
  widget->setLayout(childlayout);
  /*
    Copyright
    Logo
  */
  copyrightBoxBack = new QGroupBox(tr("Display Copyright Back Cover"));
  childlayout->addWidget(copyrightBoxBack);
  copyrightChildBack = new PageAttributeTextGui(&pageMeta->copyrightBack,copyrightBoxBack);
  childTextGui = static_cast<PageAttributeTextGui*>(copyrightChildBack);
  childTextGui->contentEdit->setToolTip("Enter copyright - Copyright © 2018");
  data->children.append(copyrightChildBack);
  connect(copyrightChildBack, SIGNAL(indexChanged(int)),
         SLOT(indexChanged(int)));

  copyrightBox = new QGroupBox(tr("Display Copyright Header/Footer"));
  childlayout->addWidget(copyrightBox);
  copyrightBox->hide();
  copyrightChild = new PageAttributeTextGui(&pageMeta->copyright,copyrightBox);
  childTextGui = static_cast<PageAttributeTextGui*>(copyrightChild);
  childTextGui->contentEdit->setToolTip("Enter copyright - Copyright © 2018");
  data->children.append(copyrightChild);
  connect(copyrightChild, SIGNAL(indexChanged(int)),
         SLOT(indexChanged(int)));

  documentLogoBoxFront = new QGroupBox(tr("Display Logo Front Cover"));
  childlayout->addWidget(documentLogoBoxFront);
  documentLogoChildFront = new PageAttributePictureGui(&pageMeta->documentLogoFront,documentLogoBoxFront);
  chilPicdGui = static_cast<PageAttributePictureGui*>(documentLogoChildFront);
  chilPicdGui->pictureEdit->setToolTip("Enter logo image path");
  data->children.append(documentLogoChildFront);
  connect(documentLogoChildFront, SIGNAL(indexChanged(int)),
         SLOT(indexChanged(int)));

  documentLogoBoxBack = new QGroupBox(tr("Display Logo Back Cover"));
  childlayout->addWidget(documentLogoBoxBack);
  documentLogoBoxBack->hide();
  documentLogoChildBack = new PageAttributePictureGui(&pageMeta->documentLogoBack,documentLogoBoxBack);
  chilPicdGui = static_cast<PageAttributePictureGui*>(documentLogoChildBack);
  chilPicdGui->pictureEdit->setToolTip("Enter logo image path");
  data->children.append(documentLogoChildBack);
  connect(documentLogoChildBack, SIGNAL(indexChanged(int)),
         SLOT(indexChanged(int)));

  //spacer
  childlayout->addSpacerItem(vSpacer);

  childtab->addTab(widget,tr("Copyright/Logo"));

  //~~~~~~~~~~~~ disclaimer tab ~~~~~~~~~~~//
  childwidget = new QWidget();
  childlayout = new QVBoxLayout;
  childwidget->setLayout(childlayout);

  childtab = new QTabWidget();
  childlayout->addWidget(childtab);
  tab->addTab(childwidget,tr("Disclaimer"));

  widget = new QWidget();
  grid = new QGridLayout();
  widget->setLayout(grid);
  /*
    LEGO disclaimer,
  */
  box = new QGroupBox(tr("Display LEGO Disclaimer"));
  grid->addWidget(box, 0, 0);
  child = new PageAttributeTextGui(&pageMeta->disclaimer,box);
  childTextGui = static_cast<PageAttributeTextGui*>(child);
  childTextGui->contentEdit->setToolTip("Enter disclaimer paragraph");
  data->children.append(child); 

  childtab->addTab(widget,tr("Disclaimer"));

  widget = new QWidget();
  childlayout = new QVBoxLayout();
  widget->setLayout(childlayout);
  /*
    Plug
    Plug image
  */
  box = new QGroupBox(tr("Display LPub3D Plug"));
  childlayout->addWidget(box);
  child = new PageAttributeTextGui(&pageMeta->plug,box);
  QPalette readOnlyPalette;
  readOnlyPalette.setColor(QPalette::Base,Qt::lightGray);
  childTextGui = static_cast<PageAttributeTextGui*>(child);
  childTextGui->contentEdit->setReadOnly(true);
  childTextGui->contentEdit->setPalette(readOnlyPalette);
  data->children.append(child);

  box = new QGroupBox(tr("Display LPub3D Logo"));
  childlayout->addWidget(box);
  child = new PageAttributePictureGui(&pageMeta->plugImage,box);
  readOnlyPalette.setColor(QPalette::Base,Qt::lightGray);
  chilPicdGui = static_cast<PageAttributePictureGui*>(child);
  chilPicdGui->pictureEdit->setReadOnly(true);
  chilPicdGui->pictureEdit->setPalette(readOnlyPalette);
  data->children.append(child);

  //spacer
  childlayout->addSpacerItem(vSpacer);

  childtab->addTab(widget,tr("LPub3D PLug"));

  //~~~~~~~~~ page number tab ~~~~~~~~~~~~//
  widget = new QWidget();
  QVBoxLayout *vLayout = new QVBoxLayout(nullptr);
  widget->setLayout(vLayout);

  box = new QGroupBox(tr("Display"));
  vLayout->addWidget(box);
  child = new CheckBoxGui(tr("Display Page Number"),&pageMeta->dpn,box);
  data->children.append(child);

  box = new QGroupBox(tr("Look"));
  vLayout->addWidget(box);
  child = new NumberGui(&pageMeta->number,box);
  data->children.append(child);

  box = new QGroupBox(tr("Placement"));
  //grid->addWidget(box,2,0);
  vLayout->addWidget(box);
  child = new BoolRadioGui(
    "Alternate Corners (like books)",
    "Page Number Always in Same Place",
    &pageMeta->togglePnPlacement,box);
  data->children.append(child);

  //spacer
  vLayout->addSpacerItem(vSpacer);

  tab->addTab(widget,tr("Page Number"));

  QDialogButtonBox *buttonBox;

  buttonBox = new QDialogButtonBox();
  buttonBox->addButton(QDialogButtonBox::Ok);
  connect(buttonBox,SIGNAL(accepted()),SLOT(accept()));
  buttonBox->addButton(QDialogButtonBox::Cancel);
  connect(buttonBox,SIGNAL(rejected()),SLOT(cancel()));

  layout->addWidget(buttonBox);
  setModal(true);
  setMinimumSize(40,20);
}

void GlobalPageDialog::indexChanged(int selection){

    sectionIndex = selection;

    QObject *obj = sender();

//    logTrace() << " RECIEVED SINGAL: "
//               << " INDEX: " << sectionIndex
//               << " Sender Class Name: "  << sender()->metaObject()->className()
//               << " Sender Object Name: " << sender()->objectName()
                  ;
    if (obj == authorChildFront || obj == authorChildBack || obj == authorChild) {
        switch(sectionIndex){
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
    else if (obj == titleChildFront || obj == titleChildBack){
        switch(sectionIndex){
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
    else if (obj == urlChildBack || obj == urlChild) {
        switch(sectionIndex){
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
    else if (obj == emailChildBack || obj == emailChild) {
        switch(sectionIndex){
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
    else if (obj == copyrightChild || obj == copyrightChildBack) {
        switch(sectionIndex){
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
        switch(sectionIndex){
        case 0: //FrontCover
            documentLogoBoxFront->show();
            documentLogoBoxBack->hide();
            break;
        case 1: //BackCover
            documentLogoBoxFront->hide();
            documentLogoBoxBack->show();
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

  MetaGui *child;
  foreach(child,data->children) {
    child->apply(data->topLevelFile);
  }
  mi.endMacro();

  QDialog::accept();
}

void GlobalPageDialog::cancel()
{
  QDialog::reject();
}
