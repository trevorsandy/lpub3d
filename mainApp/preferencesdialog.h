/**************************************************************************** 
**
** Copyright (C) 2007-2009 Kevin Clague. All rights reserved.
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
#ifndef PREFERENCES_DIALOG_H
#define PREFERENCES_DIALOG_H

#include "ui_preferences.h"

#include "colourpartlist.h"
#include "meta.h"
#include "lc_qpreferencesdialog.h"
#include "ui_lc_qpreferencesdialog.h"

class PreferencesDialog : public QDialog
{
  Q_OBJECT
  
  public:
    PreferencesDialog(QWidget *parent = 0);
	
	QString const ldrawPath();
    QString const leocadLibFile();
	QString const lgeoPath();
	QString const pliFile();
	QString const l3pExe();
	QString const povrayExe();
	QString const ldgliteExe();
	QString const ldviewExe();
    QString const fadeStepColor();
	QString const preferredRenderer();
    QString const defaultAuthor();
    QString const defaultURL();
    QString const defaultEmail();
    QString const documentLogoFile();
    QString const publishDescription();
    bool          printCopyright();
    bool          centimeters();
    bool          titleAnnotation();
    bool          freeformAnnotation();
    bool          titleAndFreeformAnnotation();
    bool          enableFadeStep();
    bool          enableDocumentLogo();

  public slots:
    void accept();
    void cancel();

  private slots:
    void on_browseLDraw_clicked();
    void on_browseLeoCADLibrary_clicked();
	void on_browseLGEO_clicked();
	void on_browsePli_clicked();
	void on_browseL3P_clicked();
	void on_browsePOVRAY_clicked();
	void on_browseLDView_clicked();
	void on_browseLDGLite_clicked();
    void on_browsePublishLogo_clicked();
    void colorChange(QString const &colorName);

private:
    QWidget *parent;
    Ui::PreferencesDialog ui;
    FadeStepMeta    fadeStepMeta;             // propagate fade color and fade bool
    ColourPartList      colourPart;
};

#endif
