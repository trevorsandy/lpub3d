
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

/***************************************************************************

 * This header file describes the global setup dialogs used to configur
 * global settings for page, assembly, parts lists, callouts, step groups
 * and projects
 *
 * Please see lpub.h for an overall description of how the files in LPu
 * make up the LPub program
 *
 ***************************************************************************/

#ifndef GLOBAL_H
#define GLOBAL_H

#include <QtGlobal>
#include <QWidget>
#include <QDialog>
#include <QGroupBox>
#include <QGridLayout>
#include <QTabWidget>

#include "meta.h"
#include "metaitem.h"
#include "metagui.h"
#include "commonmenus.h"

class Meta;

// Set minimum size
class GlobalSizeWidget : public QWidget
{
    Q_OBJECT
public:
    GlobalSizeWidget(const QSize _sh, const QSize _msh, QWidget *_parent = 0) :
        QWidget(_parent), sh(_sh), msh(_msh) {}
    ~GlobalSizeWidget() {}
    QSize sizeHint() const override { QSize s = QWidget::sizeHint(); return s.expandedTo(sh); }
    QSize minimumSizeHint() const override { QSize s = QWidget::minimumSizeHint(); return s.expandedTo(msh); }
private:
    QSize sh;
    QSize msh;
};

/*****************************************************************
 *
 * Global to page
 *
 ****************************************************************/

class GlobalPagePrivate;
class GlobalPageDialog : public QDialog
{
  Q_OBJECT
private:
  GlobalPagePrivate *data;

public:
  GlobalPageDialog(QString &topLevelModel, Meta &meta);
  ~GlobalPageDialog() {}
  static void getPageGlobals(QString topLevelFile, Meta &meta);

  int             sectionIndex;

  QGroupBox      *coverPageBox;
  QGroupBox      *coverPageBorderBox;

  QGroupBox      *lpub3dLogoBox;
  QGroupBox      *lpub3dLogoBorderBox;

  QGroupBox      *box;

  QGroupBox      *authorBoxFront;
  QGroupBox      *authorBoxBack;
  QGroupBox      *authorBox;

  QGroupBox      *titleBoxFront;
  QGroupBox      *titleBoxBack;

  QGroupBox      *urlBoxBack;
  QGroupBox      *urlBox;

  QGroupBox      *documentLogoBoxFront;
  QGroupBox      *documentLogoBoxBack;
  QGroupBox      *documentLogoBorderBoxFront;
  QGroupBox      *documentLogoBorderBoxBack;

  QGroupBox      *copyrightBox;
  QGroupBox      *copyrightBoxBack;

  QGroupBox      *emailBoxBack;
  QGroupBox      *emailBox;

  MetaGui        *child;
  MetaGui        *textPlacementChild;

  PointerAttribGui      *pointerBorderChild;
  PointerAttribGui      *pointerLineChild;
  PointerAttribGui      *pointerTipChild;

  PageAttributeTextGui  *titleChildFront;
  PageAttributeTextGui  *titleChildBack;

  PageAttributeImageGui *coverImageChildFront;
  BorderGui             *coverImageBorderChildFront;

  PageAttributeTextGui  *modelDescChildFront;
  PageAttributeTextGui  *modelIdChildFront;
  PageAttributeTextGui  *partsChildFront;

  PageAttributeTextGui  *authorChildFront;
  PageAttributeTextGui  *authorChildBack;
  PageAttributeTextGui  *authorChildHeaderFooter;

  PageAttributeTextGui  *emailChildBack;
  PageAttributeTextGui  *emailChildHeaderFooter;

  PageAttributeTextGui  *urlChildBack;
  PageAttributeTextGui  *urlChildHeaderFooter;

  PageAttributeTextGui  *publishDescChildFront;

  PageAttributeTextGui  *copyrightChildBack;
  PageAttributeTextGui  *copyrightChildHeaderFooter;

  PageAttributeImageGui *documentLogoChildFront;
  BorderGui             *documentLogoBorderChildFront;

  PageAttributeImageGui *documentLogoChildBack;
  BorderGui             *documentLogoBorderChildBack;

  PageAttributeTextGui  *disclaimerChildBack;
  PageAttributeTextGui  *lpub3dPlugChildBack;

  PageAttributeImageGui *lpub3dLogoChildBack;
  BorderGui             *lpub3dLogoBorderChildBack;

public slots:
  void accept() override;
  void cancel();
  void indexChanged(int selection);
  void displayGroup(bool b);
  void enablePointerTip(bool b);
  void enableTextPlacement(bool b);
};

/*****************************************************************
 *
 * Global to single step assem and step number
 *
 ****************************************************************/

class GlobalAssemPrivate;
class GlobalAssemDialog : public QDialog
{
  Q_OBJECT
private:
  GlobalAssemPrivate *data;

public:
  GlobalAssemDialog(QString &topLevelFile, Meta &meta);
  ~GlobalAssemDialog() {}
  static void getAssemGlobals(QString topLevelFile, Meta &meta);

public slots:
  void clearCache(bool);
  void accept() override;
  void cancel();
};

/*****************************************************************
 *
 * Global to parts list (pli)
 *
 ****************************************************************/

class GlobalPliPrivate;
class GlobalPliDialog : public QDialog
{
  Q_OBJECT
private:
  GlobalPliPrivate *data;

  QRadioButton     *noStyleButton;
  QGroupBox        *annotationEditStyleBox;
  PliAnnotationGui *childPliAnnotation;
  PliPartElementGui*childPliPartElement;
  NumberGui        *childTextFormat;
  NumberMeta       *styleMeta;

  QRadioButton     *squareStyleButton;
  QGroupBox        *squareBkGrndStyleBox;
  QGroupBox        *squareBorderStyleBox;
  QGroupBox        *squareFormatStyleBox;
  QGroupBox        *squareSizeStyleBox;

  QRadioButton     *circleStyleButton;
  QGroupBox        *circleBkGrndStyleBox;
  QGroupBox        *circleBorderStyleBox;
  QGroupBox        *circleFormatStyleBox;
  QGroupBox        *circleSizeStyleBox;

  QRadioButton     *elementStyleButton;
  QGroupBox        *elementBkGrndStyleBox;
  QGroupBox        *elementBorderStyleBox;
  QGroupBox        *elementFormatStyleBox;
  QGroupBox        *elementSizeStyleBox;

  QRadioButton     *rectangleStyleButton;
  QGroupBox        *rectangleBkGrndStyleBox;
  QGroupBox        *rectangleBorderStyleBox;
  QGroupBox        *rectangleFormatStyleBox;
  QGroupBox        *rectangleSizeStyleBox;

public:
  GlobalPliDialog(QString &topLevelFile, Meta &meta, bool);
  ~GlobalPliDialog() {}
  static void getPliGlobals(QString topLevelFile, Meta &meta);
  static void getBomGlobals(QString topLevelFile, Meta &meta);

public slots:
  void clearCache(bool);
  void styleOptionChanged(bool);
  void displayAnnotationsChanged(bool);
  void enableElementStyleChanged(bool);
  void accept() override;
  void cancel();
};

/*****************************************************************
 *
 * Global to submodel
 *
 ****************************************************************/

class GlobalSubModelPrivate;
class GlobalSubModelDialog : public QDialog
{
  Q_OBJECT
private:
  GlobalSubModelPrivate *data;
  QGroupBox *instanceCountBox;

public:
  GlobalSubModelDialog(QString &topLevelFile, Meta &meta);
  ~GlobalSubModelDialog() {}
  static void getSubModelGlobals(QString topLevelFile, Meta &meta);

public slots:
  void accept() override;
  void cancel();
  void clearCache(bool);
  void instanceCountClicked(bool);
};

/*****************************************************************
 *
 * Global to callout
 *
 ****************************************************************/

class GlobalCalloutPrivate;
class GlobalCalloutDialog : public QDialog
{
  Q_OBJECT
private:
  GlobalCalloutPrivate *data;

public:
  GlobalCalloutDialog(QString &topLevelFile, Meta &meta);
  ~GlobalCalloutDialog() {}
  static void getCalloutGlobals(QString topLevelFile, Meta &meta);

public slots:
  void clearCache(bool);
  void clearPliCache(bool);
  void accept() override;
  void cancel();
};

/*****************************************************************
 *
 * Global to multiStep
 *
 ****************************************************************/

class GlobalMultiStepPrivate;
class GlobalMultiStepDialog : public QDialog
{
  Q_OBJECT
private:
  GlobalMultiStepPrivate *data;

  CheckBoxGui *showGrpStepNumCheckBoxGui;
  CheckBoxGui *countGrpStepsCheckBoxGui;

public:
  GlobalMultiStepDialog(QString &topLevelFile, Meta &meta);
  ~GlobalMultiStepDialog() {}
  static void getMultiStepGlobals(QString topLevelFile, Meta &meta);

public slots:
  void clearCache(bool);
  void pliPerStepStateChanged(int state);
  void showGrpStepNumStateChanged(int state);
  void accept() override;
  void cancel();
};

/*****************************************************************
 *
 * Global to FadeStep
 *
 ****************************************************************/

class GlobalFadeStepPrivate;
class GlobalFadeStepDialog : public QDialog
{
  Q_OBJECT
private:
  GlobalFadeStepPrivate *data;
  FadeStepsGui          *fadeStepsChild;
  CheckBoxGui           *fadeStepsSetupChild;
  FinalModelEnabledGui  *finalModelEnabledChild;

public:
  GlobalFadeStepDialog(QString &topLevelFile, Meta &meta);
  ~GlobalFadeStepDialog() {}
  static void getFadeStepGlobals(QString topLevelFile, Meta &meta);

public slots:
  void accept() override;
  void cancel();
  void enableControls(bool);
  void reloadDisplayPage(bool);
};

/*****************************************************************
 *
 * Global to HighlightStep
 *
 ****************************************************************/

class GlobalHighlightStepPrivate;
class GlobalHighlightStepDialog : public QDialog
{
  Q_OBJECT
private:
  GlobalHighlightStepPrivate *data;
  HighlightStepGui           *highlightStepChild;
  CheckBoxGui                *highlightStepSetupChild;
  FinalModelEnabledGui       *finalModelEnabledChild;

public:
  GlobalHighlightStepDialog(QString &topLevelFile, Meta &meta);
  ~GlobalHighlightStepDialog() {}
  static void getHighlightStepGlobals(QString topLevelFile, Meta &meta);

public slots:
  void accept() override;
  void cancel();
  void reloadDisplayPage(bool);
  void enableControls(bool);
};

/*****************************************************************
 *
 * Global to project
 *
 ****************************************************************/

class GlobalProjectPrivate;
class GlobalProjectDialog : public QDialog
{
  Q_OBJECT
private:
  GlobalProjectPrivate *data;
  PreferredRendererGui *childPreferredRenderer;
  ResolutionGui        *childResolution;
  CameraDDFGui         *childCameraDDF;
  ContStepNumGui       *childContStepNumbers;
  SpinGui              *childStartStepNumberSpin;
  SpinGui              *childStartPageNumberSpin;

public:

  GlobalProjectDialog(const QString &topLevelFile, Meta &meta);
  ~GlobalProjectDialog() {}
  static void getProjectGlobals(const QString topLevelFile, Meta &meta);

public slots:
  void accept() override;
  void cancel();
  void clearCache(bool);
  void reloadWhatsThis(int value);
  void reloadDisplayPage(bool);
  void checkConflict(bool);
  void enableCameraDDF(int);
};
#endif
