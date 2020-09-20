/****************************************************************************
**
** Copyright (C) 2019 Trevor SANDY. All rights reserved.
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
 * This file implements a dialog that allows the user to substitute parts.
 *
 * Please see lpub.h for an overall description of how the files in LPub
 * make up the LPub program.
 *
 ***************************************************************************/

#include "substitutepartdialog.h"
#include "ui_substitutepartdialog.h"
#include <QFileInfo>
#include <functional>
#include "ldrawpartdialog.h"
#include "ldrawcolordialog.h"
#include "lpubalert.h"
#include "pli.h"
#include "color.h"

// For drawPreview()
#include "lc_global.h"
#include "lc_mesh.h"
#include "pieceinf.h"
//#include "lc_application.h"
//#include "lc_mainwindow.h"
//#include "lc_library.h"
//#include "lc_model.h"
//#include "view.h"
//#include "lc_glextensions.h"

SubstitutePartDialog::SubstitutePartDialog(
    const QStringList &attributes,
    QWidget           *parent,
    int                action,
    const QStringList &defaultList) :
    QDialog(parent),
    ui(new Ui::SubstitutePartDialog),
    mTypeInfo(nullptr),
    mAction(action)
{
     ui->setupUi(this);

     QString title = QString("%1 Part")
             .arg(mAction == sSubstitute ?
                   "Substitute" : mAction == sUpdate ?
                       "Change Substitute" : "Remove Substitute");
     setWindowTitle(title);

     // set initial attributes
     mDefaultAttributes = mAction == sSubstitute ? attributes :
                          QStringList() << "type" << "color" << defaultList;
     mInitialAttributes = attributes;

     initialize();

     connect(ui->typeBtn,SIGNAL(clicked(bool)),
             this,        SLOT(  browseType(bool)));

     connect(ui->substituteEdit,SIGNAL(editingFinished()),
             this,              SLOT(  typeChanged()));

     connect(ui->colorBtn,SIGNAL(clicked(bool)),
             this,        SLOT(  browseColor(bool)));

     connect(ui->scaleSpin,SIGNAL(valueChanged(double)),
             this,         SLOT  (valueChanged(double)));

     connect(ui->fovSpin,SIGNAL(valueChanged(double)),
             this,       SLOT  (valueChanged(double)));

     connect(ui->latitudeSpin,SIGNAL(valueChanged(double)),
             this,            SLOT  (valueChanged(double)));

     connect(ui->longitudeSpin,SIGNAL(valueChanged(double)),
             this,             SLOT  (valueChanged(double)));

     connect(ui->targetSpinX,SIGNAL(valueChanged(int)),
             this,           SLOT(valueChanged(int)));

     connect(ui->targetSpinY,SIGNAL(valueChanged(int)),
             this,           SLOT(valueChanged(int)));

     connect(ui->targetSpinZ,SIGNAL(valueChanged(int)),
             this,           SLOT(valueChanged(int)));

     connect(ui->rotateSpinX,SIGNAL(valueChanged(double)),
             this,           SLOT(valueChanged(double)));

     connect(ui->rotateSpinY,SIGNAL(valueChanged(double)),
             this,           SLOT(valueChanged(double)));

     connect(ui->rotateSpinZ,SIGNAL(valueChanged(double)),
             this,           SLOT(valueChanged(double)));

     connect(ui->transformCombo,SIGNAL(currentIndexChanged(QString const &)),
             this,              SLOT(  transformChanged(   QString const &)));

     mResetBtn = new QPushButton(tr("Reset"));
     ui->buttonBox->addButton(mResetBtn, QDialogButtonBox::ActionRole);
     connect(mResetBtn,SIGNAL(clicked(bool)),
             this,    SLOT(  reset(bool)));

     setMinimumSize(40, 40);

     if (mAction == sRemove)
         adjustSize();
}

SubstitutePartDialog::~SubstitutePartDialog()
{
    delete ui;
}

bool SubstitutePartDialog::getSubstitutePart(
   QStringList       &attributes,
   QWidget           *parent,
   int                action,
   const QStringList &defaultList) {

    SubstitutePartDialog *dialog = new SubstitutePartDialog(attributes,parent,action,defaultList);
    bool ok = dialog->exec() == QDialog::Accepted;
    if (ok) {
      attributes = dialog->mAttributes;
    }
    return ok;
}

void SubstitutePartDialog::initialize()
{
    mAttributes = mInitialAttributes;
    bool show    = mAction != sRemove;

    if (Preferences::debugLogging)
        emit lpubAlert->messageSig(LOG_DEBUG,QString("Loaded substitution part args for type [%1]: [%2]")
                                   .arg(mAttributes.at(sType)).arg(mAttributes.join(" ")));

    qreal min = -10000.0, max = 10000.0, step = 0.1, val = 1.0;
    auto dec = [] (const qreal v)
    {
        auto places = [&v] () {
            if (v == 0.0)
                return 2;

            int count = 0;
            qreal num = v;
            num = abs(num);
            num = num - int(num);
            while (abs(num) >= 0.0000001) {
                num = num * 10;
                count = count + 1;
                num = num - int(num);
            }
            return count;
        };

        int a = v - int(v);
        return (a < 1 ? places() : QString::number(a).size() < 3 ? 2 : QString::number(a).size());
    };

    QPalette readOnlyPalette;
    readOnlyPalette.setColor(QPalette::Base,Qt::lightGray);
    ui->nameEdit->setReadOnly(true);
    ui->nameEdit->setPalette(readOnlyPalette);
    ui->nameEdit->setText(mAttributes.at(sType));
    ui->titleLbl->setText(Pli::titleDescription(mAttributes.at(sType)));
    ui->titleLbl->adjustSize();

    ui->primarySettingsBox->setVisible(show);

    ui->substituteEdit->clear();
    ui->substituteEdit->setClearButtonEnabled(true);
    ui->substitueTitleLbl->clear();

    QColor partColor = LDrawColor::color(LDrawColor::value(mAttributes.at(sColorCode),true));
    if(partColor.isValid() ) {
      ui->colorLbl->setAutoFillBackground(true);
      QString styleSheet =
        QString("QLabel { background-color: rgb(%1, %2, %3); }")
            .arg(partColor.red())
            .arg(partColor.green())
            .arg(partColor.blue());
      ui->colorLbl->setStyleSheet(styleSheet);
      ui->colorLbl->setToolTip(QString("%1 (%2) %3")
                               .arg(LDrawColor::name(mAttributes.at(sColorCode)).replace("_"," "))
                               .arg(mAttributes.at(sColorCode))
                               .arg(LDrawColor::value(mAttributes.at(sColorCode)).toUpper()));
    }

    ui->extendedSettingsBox->setVisible(show);

    if (show)
        val = mAttributes.at(sModelScale).toDouble();
    ui->scaleSpin->setRange(min,max);
    ui->scaleSpin->setSingleStep(step);
    ui->scaleSpin->setDecimals(dec(val));
    ui->scaleSpin->setValue(val);

    if (Preferences::usingNativeRenderer){
        min = double(CAMERA_FOV_NATIVE_MIN_DEFAULT);
        max = double(CAMERA_FOV_NATIVE_MAX_DEFAULT);
    } else {
        min = double(CAMERA_FOV_MIN_DEFAULT);
        max = double(CAMERA_FOV_MAX_DEFAULT);
    }
    step = 1.0;
    if (show)
        val = mAttributes.at(sCameraFoV).toDouble();
    ui->fovSpin->setRange(min,max);
    ui->fovSpin->setSingleStep(step);
    ui->fovSpin->setDecimals(dec(val));
    ui->fovSpin->setValue(val);

    min = -360.0;
    max = 360.0;
    if (show)
        val = mAttributes.at(sCameraAngleXX).toDouble();
    ui->latitudeSpin->setRange(min,max);
    ui->latitudeSpin->setSingleStep(step);
    ui->latitudeSpin->setDecimals(dec(val));
    ui->latitudeSpin->setValue(val);

    if (show)
        val = mAttributes.at(sCameraAngleYY).toDouble();
    ui->longitudeSpin->setRange(min,max);
    ui->longitudeSpin->setSingleStep(step);
    ui->longitudeSpin->setDecimals(dec(val));
    ui->longitudeSpin->setValue(val);

    ui->targetSpinX->setRange(0,10000);
    ui->targetSpinX->setSingleStep(1);
    ui->targetSpinX->setValue(mAttributes.at(sTargetX).toInt());

    ui->targetSpinY->setRange(0,10000);
    ui->targetSpinY->setSingleStep(1);
    ui->targetSpinY->setValue(mAttributes.at(sTargetY).toInt());

    ui->targetSpinZ->setRange(0,10000);
    ui->targetSpinZ->setSingleStep(1);
    ui->targetSpinZ->setValue(mAttributes.at(sTargetZ).toInt());

    if (show)
        val = mAttributes.at(sRotX).toDouble();
    ui->rotateSpinX->setRange(0.0,360.0);
    ui->rotateSpinX->setSingleStep(1.0);
    ui->rotateSpinX->setDecimals(dec(val));
    ui->rotateSpinX->setValue(val);

    if (show)
        val = mAttributes.at(sRotY).toDouble();
    ui->rotateSpinY->setRange(0.0,360.0);
    ui->rotateSpinY->setSingleStep(1.0);
    ui->rotateSpinY->setDecimals(dec(val));
    ui->rotateSpinY->setValue(val);

    if (show)
        val = mAttributes.at(sRotZ).toDouble();
    ui->rotateSpinZ->setRange(0.0,360.0);
    ui->rotateSpinZ->setSingleStep(1.0);
    ui->rotateSpinZ->setDecimals(dec(val));
    ui->rotateSpinZ->setValue(val);

    ui->transformCombo->addItem("ABS");
    ui->transformCombo->addItem("REL");
    ui->transformCombo->addItem("ADD");
    ui->transformCombo->setVisible(show);
    if (show)
        ui->transformCombo->setCurrentIndex(!mAttributes.at(sTransform).isEmpty() ?
                                                ui->transformCombo->findText(mAttributes.at(sTransform)) : 1);

    ui->messageLbl->setVisible(show);

    mModified = !show;
}

void SubstitutePartDialog::reset(bool value)
{
    Q_UNUSED(value)
    initialize();
}
void SubstitutePartDialog::valueChanged(int value)
{
    if (sender() == ui->targetSpinX) {
            mAttributes[sTargetX] = QString::number(value);
    } else
    if (sender() == ui->targetSpinY) {
            mAttributes[sTargetY] = QString::number(value);
    } else
    if (sender() == ui->targetSpinZ) {
            mAttributes[sTargetZ] = QString::number(value);
    }
    mModified = true;
}
void SubstitutePartDialog::valueChanged(double value)
{
    if (sender() == ui->scaleSpin) {
            mAttributes[sModelScale] = QString::number(value);
    } else
    if (sender() == ui->fovSpin) {
            mAttributes[sCameraFoV] = QString::number(value);
    } else
    if (sender() == ui->latitudeSpin) {
            mAttributes[sCameraAngleXX] = QString::number(value);
    } else
    if (sender() == ui->longitudeSpin) {
            mAttributes[sCameraAngleYY] = QString::number(value);
    } else
    if (sender() == ui->rotateSpinX) {
            mAttributes[sRotX] = QString::number(value);
    } else
    if (sender() == ui->rotateSpinY) {
            mAttributes[sRotY] = QString::number(value);
    } else
    if (sender() == ui->rotateSpinZ) {
            mAttributes[sRotZ] = QString::number(value);
    }
    mModified = true;
}

void SubstitutePartDialog::typeChanged()
{
  if (ui->substituteEdit->text().isEmpty()) {
      ui->substitueTitleLbl->setText(QString());
      return;
  }
  QString const type = ui->substituteEdit->text();
  QString const newType(QFileInfo(type).suffix().isEmpty() ? type + ".dat" : type);
  QString const currentType = ui->nameEdit->text();
  if (newType != currentType) {
      ui->substituteEdit->setText(newType);
  } else {
      ui->messageLbl->setText(QString("Part type %1 is the same as current type.").arg(currentType));
      ui->messageLbl->setStyleSheet("QLabel { color : blue; }");
      return;
  }
  ui->substitueTitleLbl->setText(Pli::titleDescription(newType));
  if (!ui->substitueTitleLbl->text().isEmpty()) {
      ui->substitueTitleLbl->adjustSize();
      ui->messageLbl->setText(QString());
      mAttributes[sType] = newType;
      mModified = true;
  } else {
      if (!type.isEmpty()) {
          ui->messageLbl->setText(QString("Part type %1 was not found.").arg(type));
          ui->messageLbl->setStyleSheet("QLabel { color : red; }");
      }
  }
  //adjustSize();
}

void SubstitutePartDialog::browseType(bool clicked)
{
  Q_UNUSED(clicked);
  mTypeInfo = LDrawPartDialog::getLDrawPart(QString("%1;%2")
                                                    .arg(ui->nameEdit->text())
                                                    .arg(mAttributes.at(sColorCode)));
  if (mTypeInfo) {
      ui->substituteEdit->setText(mTypeInfo->mFileName);
      typeChanged();
  }
}

void SubstitutePartDialog::colorChanged(const QString &colorName)
{
  QColor newColor = LDrawColor::color(colorName);
  if(newColor.isValid() ) {
      mAttributes[sColorCode] = QString::number(LDrawColor::code(colorName));
      ui->colorLbl->setAutoFillBackground(true);
      QString styleSheet =
          QString("QLabel { background-color: rgb(%1, %2, %3); }")
          .arg(newColor.red())
          .arg(newColor.green())
          .arg(newColor.blue());
      ui->colorLbl->setStyleSheet(styleSheet);
      ui->colorLbl->setToolTip(QString("%1 (%2) %3")
                               .arg(LDrawColor::name(mAttributes.at(sColorCode)).replace("_"," "))
                               .arg(mAttributes.at(sColorCode))
                               .arg(LDrawColor::value(mAttributes.at(sColorCode)).toUpper()));
      mModified = true;
    }
}

void SubstitutePartDialog::browseColor(bool clicked)
{
  Q_UNUSED(clicked);
  QColor qcolor = LDrawColor::value(mAttributes.at(sColorCode),true);
  QColor newColor = LDrawColorDialog::getLDrawColor(qcolor,this);
  if (newColor.isValid() && qcolor != newColor) {
      colorChanged(newColor.name());
  }
}

void SubstitutePartDialog::transformChanged(QString const &value)
{
       mModified = true;
       mAttributes[sTransform] = value;
}

/*
void SubstitutePartDialog::drawPreview()
{
    View* ActiveView = gMainWindow->GetActiveView();
    if (!ActiveView)
        return;

    ActiveView->MakeCurrent();
    lcContext* Context = ActiveView->mContext;
    int Width = ui->previewFrame->width();
    int Height = ui->previewFrame->height();

    if (mRenderFramebuffer.first.mWidth != Width || mRenderFramebuffer.first.mHeight != Height)
    {
        Context->DestroyRenderFramebuffer(mRenderFramebuffer);
        mRenderFramebuffer = Context->CreateRenderFramebuffer(Width, Height);
    }

    if (!mRenderFramebuffer.first.IsValid())
        return;

    float Aspect = (float)Width / (float)Height;
    Context->SetViewport(0, 0, Width, Height);

    Context->SetDefaultState();
    Context->BindFramebuffer(mRenderFramebuffer.first);

    lcPiecesLibrary* Library = lcGetPiecesLibrary();

    glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    lcMatrix44 ProjectionMatrix, ViewMatrix;

    mTypeInfo->ZoomExtents(20.0f, Aspect, ProjectionMatrix, ViewMatrix);

    Context->SetProjectionMatrix(ProjectionMatrix);

    lcScene Scene;
    Scene.SetAllowWireframe(false);
    Scene.Begin(ViewMatrix);

    mTypeInfo->AddRenderMeshes(Scene, lcMatrix44Identity(), mColorIndex, lcRenderMeshState::NORMAL, false);

    Scene.End();

    Scene.Draw(Context);

    QPixmap Pixmap = QPixmap::fromImage(Context->GetRenderFramebufferImage(mRenderFramebuffer)).scaled(mIconSize, mIconSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

    Library->ReleasePieceInfo(mTypeInfo);

    Context->ClearFramebuffer();
    Context->ClearResources();
}
*/

void SubstitutePartDialog::accept()
{
    auto setAttributes = [this] ()
    {
        enum Levels{ Level1,Level2,Level3,Level4,Level5,Level6,Level7,Level8,NumLevels };
        bool removeLevel[NumLevels];
        removeLevel[Level1] = removeLevel[Level2] = removeLevel[Level3] =
        removeLevel[Level4] = removeLevel[Level5] = removeLevel[Level6] = false;
        QString label1,label2;

        // Rmovals must be in reverse order

        if (mAction != sRemove) {

            QStringList removeAttributes;
            if (mAction == sSubstitute) {
                label1 = "substitute";
                label2 = "Substitution";
            } else if (mAction == sUpdate) {
                label1 = "update";
                label2 = "Substitute update";
            }

            // Level 8 Substitution - Target and Rotation
            QString removeLevel8Sub = QString("%1%2%3%4%5%6%7")
                    .arg(mInitialAttributes.at(sTargetX))
                    .arg(mInitialAttributes.at(sTargetY))
                    .arg(mInitialAttributes.at(sTargetZ))
                    .arg(mInitialAttributes.at(sRotX))
                    .arg(mInitialAttributes.at(sRotY))
                    .arg(mInitialAttributes.at(sRotZ))
                    .arg(mInitialAttributes.at(sTransform));
            QString removeLevel8DefSub = QString("%1%2%3%4%5%6%7")
                    .arg(mDefaultAttributes.at(sTargetX))
                    .arg(mDefaultAttributes.at(sTargetY))
                    .arg(mDefaultAttributes.at(sTargetZ))
                    .arg(mDefaultAttributes.at(sRotX))
                    .arg(mDefaultAttributes.at(sRotY))
                    .arg(mDefaultAttributes.at(sRotZ))
                    .arg(mDefaultAttributes.at(sTransform));
            QString level8Sub = QString("%1%2%3%4%5%6%7")
                    .arg(mAttributes.at(sTargetX))
                    .arg(mAttributes.at(sTargetY))
                    .arg(mAttributes.at(sTargetZ))
                    .arg(mAttributes.at(sRotX))
                    .arg(mAttributes.at(sRotY))
                    .arg(mAttributes.at(sRotZ))
                    .arg(mAttributes.at(sTransform));

            if ((removeLevel[Level8] = level8Sub == removeLevel8Sub &&
                (mAction == sUpdate ? level8Sub == removeLevel8DefSub : true))) {
                mAttributes.removeAt(sTransform);
                mAttributes.removeAt(sRotZ);
                mAttributes.removeAt(sRotY);
                mAttributes.removeAt(sRotX);
                mAttributes.removeAt(sTargetZ);
                mAttributes.removeAt(sTargetY);
                mAttributes.removeAt(sTargetX);
            }

            if (!removeLevel[Level8]) {
                // Level 7 Substitution - Rotation
                QString removeLevel7Sub = QString("%1%2%3%4")
                        .arg(mInitialAttributes.at(sRotX))
                        .arg(mInitialAttributes.at(sRotY))
                        .arg(mInitialAttributes.at(sRotZ))
                        .arg(mInitialAttributes.at(sTransform));
                QString removeLevel7DefSub = QString("%1%2%3%4")
                        .arg(mDefaultAttributes.at(sRotX))
                        .arg(mDefaultAttributes.at(sRotY))
                        .arg(mDefaultAttributes.at(sRotZ))
                        .arg(mDefaultAttributes.at(sTransform));
                QString level7Sub = QString("%1%2%3%4")
                        .arg(mAttributes.at(sRotX))
                        .arg(mAttributes.at(sRotY))
                        .arg(mAttributes.at(sRotZ))
                        .arg(mAttributes.at(sTransform));

                if ((removeLevel[Level7] = level7Sub == removeLevel7Sub &&
                   (mAction == sUpdate ? level7Sub == removeLevel7DefSub : true))) {
                    mAttributes.removeAt(sTransform);
                    mAttributes.removeAt(sRotZ);
                    mAttributes.removeAt(sRotY);
                    mAttributes.removeAt(sRotX);
                }

                // Level 6 Substitution - Target
                QString removeLevel6Sub = QString("%1%2%3%4")
                        .arg(mInitialAttributes.at(sTargetX))
                        .arg(mInitialAttributes.at(sTargetY))
                        .arg(mInitialAttributes.at(sTargetZ));
                QString removeLevel6DefSub = QString("%1%2%3%4")
                        .arg(mDefaultAttributes.at(sTargetX))
                        .arg(mDefaultAttributes.at(sTargetY))
                        .arg(mDefaultAttributes.at(sTargetZ));
                QString level6Sub = QString("%1%2%3")
                        .arg(mAttributes.at(sTargetX))
                        .arg(mAttributes.at(sTargetY))
                        .arg(mAttributes.at(sTargetZ));

                if (removeLevel[Level7] &&
                   (removeLevel[Level6] = level6Sub == removeLevel6Sub &&
                   (mAction == sUpdate ? level6Sub == removeLevel6DefSub : true))) {
                    mAttributes.removeAt(sTargetZ);
                    mAttributes.removeAt(sTargetY);
                    mAttributes.removeAt(sTargetX);
                }
            }

            // Level 5 Substitution - Camera Angles
            QString removeLevel5Sub = QString("%1%2")
                    .arg(mInitialAttributes.at(sCameraAngleXX))
                    .arg(mInitialAttributes.at(sCameraAngleYY));
            QString removeLevel5DefSub = QString("%1%2")
                    .arg(mDefaultAttributes.at(sCameraAngleXX))
                    .arg(mDefaultAttributes.at(sCameraAngleYY));
            QString level5Sub = QString("%1%2")
                    .arg(mAttributes.at(sCameraAngleXX))
                    .arg(mAttributes.at(sCameraAngleYY));

            if ((removeLevel[Level8] || removeLevel[Level6]) &&
               (removeLevel[Level5] = level5Sub == removeLevel5Sub &&
               (mAction == sUpdate ? level5Sub == removeLevel5DefSub : true))) {
                mAttributes.removeAt(sCameraAngleYY);
                mAttributes.removeAt(sCameraAngleXX);
            }

            // Level 4 Substitution - Camera FOV
            if (removeLevel[Level5] &&
               (removeLevel[Level4] =
                mInitialAttributes.at(sCameraFoV) == mAttributes.at(sCameraFoV) &&
               (mAction == sUpdate ? mDefaultAttributes.at(sCameraFoV) == mAttributes.at(sCameraFoV) : true))) {
                mAttributes.removeAt(sCameraFoV);
            }

            // Level 3 Substitution - Model Scale / Camera Distance Factor
            if (removeLevel[Level4] &&
               (removeLevel[Level3] =
                mInitialAttributes.at(sModelScale) == mAttributes.at(sModelScale) &&
               (mAction == sUpdate ? mDefaultAttributes.at(sModelScale) == mAttributes.at(sModelScale) : true))) {
                mAttributes.removeAt(sModelScale);
            }

            // Level 2 Substitution - Colour [always populate colour]
            removeLevel[Level2] =
                            mInitialAttributes.at(sColorCode) == mAttributes.at(sColorCode) &&
                           (mAction == sUpdate ? mDefaultAttributes.at(sColorCode) == mAttributes.at(sColorCode) : true);

            // Level 1 Substitution - Part Type
            if (removeLevel[Level2] &&
               (removeLevel[Level1] =
                 mInitialAttributes.at(sType) == mAttributes.at(sType) &&
               (mAction == sUpdate ? mDefaultAttributes.at(sColorCode) == mAttributes.at(sColorCode) : true))) {
                emit lpubAlert->messageSig(LOG_TRACE,QString("Nothing to %1 for part type [%2]: [%3]")
                                           .arg(label1).arg(mAttributes.at(sType)).arg(mAttributes.join(" ")));
                mAttributes.clear();
                mModified = false;
                return;
            }

        } else {
            label1 = "remove";
            label2 = "Substitute removal";
        }

        emit lpubAlert->messageSig(LOG_TRACE,QString("%1 for part type %2: [%3], Input: [%4]")
                         .arg(label2).arg(mAttributes.at(sType)).arg(mAttributes.join(" ")).arg(mInitialAttributes.join(" ")));
    };

    if (mModified) {
      if (Preferences::debugLogging)
          emit lpubAlert->messageSig(LOG_DEBUG,QString("Set substitution for part type %1: [%2], Input: [%3]")
                                   .arg(mAttributes.at(sType)).arg(mAttributes.join(" ")).arg(mInitialAttributes.join(" ")));
      setAttributes();
    }

    if (mModified) {
      QDialog::accept();
    } else {
      QDialog::reject();
    }
}

void SubstitutePartDialog::cancel()
{
  QDialog::reject();
}
