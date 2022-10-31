/****************************************************************************
**
** Copyright (C) 2021 - 2022 Trevor SANDY. All rights reserved.
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

/****************************************************************************
 *
 * This file implements a dialog that allows the user to select the
 * fade previous steps or highlight current step settings.
 *
 * Please see lpub.h for an overall description of how the files in LPub
 * make up the LPub program.
 *
 ***************************************************************************/

#ifndef FADEHIGHLIGHTDIALOG_H
#define FADEHIGHLIGHTDIALOG_H

#include <QDialog>
#include "meta.h"

class FadeStepsGui;
class HighlightStepGui;
class FadeHighlightDialog : public QDialog
{
  Q_OBJECT

private:
   enum UseMeta { UseFadeMeta, UseHighlightMeta };

public:

  FadeHighlightDialog(
    FadeStepsMeta     &fadeGoods,
    HighlightStepMeta &highlightGoods,
    UseMeta            useMeta,
    QString            name,
    QWidget           *parent);

  virtual ~FadeHighlightDialog();

  static bool getFadeSteps(
    FadeStepsMeta     &fadeGoods,
    QString            name,
    QWidget           *parent)
  {
    HighlightStepMeta dummy;
    return getFadeHighlight(fadeGoods,dummy,UseFadeMeta,name,parent);
  }

  static bool getHighlightStep(
    HighlightStepMeta &highlightGoods,
    QString            name,
    QWidget           *parent)
  {
    FadeStepsMeta dummy;
    return getFadeHighlight(dummy,highlightGoods,UseHighlightMeta,name,parent);
  }

  static bool getFadeHighlight(
    FadeStepsMeta     &fadeGoods,
    HighlightStepMeta &highlightGoods,
    UseMeta            useMeta,
    QString            name,
    QWidget           *parent);

  UseMeta useMeta;
  FadeStepsMeta fadeMeta;
  HighlightStepMeta highlightMeta;

public slots:
  void accept();
  void cancel();

private:
  FadeStepsGui     *fadeSteps;
  HighlightStepGui *highlightStep;
};

#endif // FADEHIGHLIGHTDIALOG_H
