/****************************************************************************
**
** Copyright (C) 2021 - 2023 Trevor SANDY. All rights reserved.
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
 * preferred renderer and its associated settings.
 *
 * Please see lpub.h for an overall description of how the files in LPub
 * make up the LPub program.
 *
 ***************************************************************************/

#ifndef PREFERREDRENDERERDIALOG_H
#define PREFERREDRENDERERDIALOG_H

#include <QDialog>
#include "meta.h"

class PreferredRendererGui;
class PreferredRendererDialog : public QDialog
{
  Q_OBJECT
public:

  PreferredRendererDialog(
    RendererData &goods,
    QString       name,
    QWidget      *parent);

  virtual ~PreferredRendererDialog();

  static bool getPreferredRenderer(
    PreferredRendererMeta &meta,
    QString                name,
    QWidget               *parent);

  PreferredRendererMeta meta;

public slots:
  void accept() override;
  void cancel();

private:
  PreferredRendererGui *preferredRenderer;
};

#endif // PREFERREDRENDERERDIALOG_H
