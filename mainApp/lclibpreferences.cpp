/****************************************************************************
**
** Copyright (C) 2022 - 2023 Trevor SANDY. All rights reserved.
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

#include "preferencesdialog.h"
#include "lpub_object.h"

#include "lc_library.h"
#include "lc_glextensions.h"
#include "lc_edgecolordialog.h"
#include "camera.h"

void PreferencesDialog::lcQPreferencesInit()
{
//    connect(ui.FadeStepsColor, &QToolButton::clicked, this, &PreferencesDialog::ColorButtonClicked);
//    connect(ui.HighlightNewPartsColor, &QToolButton::clicked, this, &PreferencesDialog::ColorButtonClicked);
//    connect(ui.HighContrastButton, SIGNAL(clicked()), this, SLOT(AutomateEdgeColor()));
//    connect(ui.AutomateEdgeColorButton, SIGNAL(clicked()), this, SLOT(AutomateEdgeColor()));
//    connect(ui.ResetFadeStepsButton, SIGNAL(clicked()), this, SLOT(ResetFadeHighlightColor()));
//    connect(ui.ResetHighlightNewPartsButton, SIGNAL(clicked()), this, SLOT(ResetFadeHighlightColor()));

    ui.antiAliasing->setChecked(mOptions->AASamples != 1);
    if (mOptions->AASamples == 8)
        ui.antiAliasingSamples->setCurrentIndex(2);
    else if (mOptions->AASamples == 4)
        ui.antiAliasingSamples->setCurrentIndex(1);
    else
        ui.antiAliasingSamples->setCurrentIndex(0);
    ui.edgeLines->setChecked(mOptions->Preferences.mDrawEdgeLines);
    ui.MeshLOD->setChecked(mOptions->Preferences.mAllowLOD);

    ui.MeshLODSlider->setRange(0, 1500.0f / mMeshLODMultiplier);
    ui.MeshLODSlider->setValue(mOptions->Preferences.mMeshLODDistance / mMeshLODMultiplier);
    if (!gSupportsShaderObjects)
        ui.ShadingMode->removeItem(static_cast<int>(lcShadingMode::DefaultLights));
    ui.ShadingMode->setCurrentIndex(static_cast<int>(mOptions->Preferences.mShadingMode));
    if (!lcGetPiecesLibrary()->SupportsStudStyle())
        ui.studStyleCombo->setEnabled(false);
    ui.studStyleCombo->setCurrentIndex(static_cast<int>(mOptions->StudStyle));
    ui.ConditionalLinesCheckBox->setChecked(mOptions->Preferences.mDrawConditionalLines);
    if (!gSupportsShaderObjects) {
        ui.ConditionalLinesCheckBox->setChecked(false);
        ui.ConditionalLinesCheckBox->setEnabled(false);
    }
#ifndef LC_OPENGLES
    if (QSurfaceFormat::defaultFormat().samples() > 1) {
        glGetFloatv(GL_SMOOTH_LINE_WIDTH_RANGE, mLineWidthRange);
        glGetFloatv(GL_SMOOTH_LINE_WIDTH_GRANULARITY, &mLineWidthGranularity);
    }
    else
#endif
    {
        glGetFloatv(GL_ALIASED_LINE_WIDTH_RANGE, mLineWidthRange);
        mLineWidthGranularity = 1.0f;
    }

    ui.LineWidthSlider->setRange(0, (mLineWidthRange[1] - mLineWidthRange[0]) / qMax(mLineWidthGranularity, 1.0f));
    ui.LineWidthSlider->setValue((mOptions->Preferences.mLineWidth - mLineWidthRange[0]) / mLineWidthGranularity);

    ui.FadeSteps->setChecked(mOptions->Preferences.mFadeSteps);
    ui.HighlightNewParts->setChecked(mOptions->Preferences.mHighlightNewParts);
    ui.AutomateEdgeColor->setChecked(mOptions->Preferences.mAutomateEdgeColor);
    ui.ProjectionCombo->setCurrentIndex(mOptions->Preferences.mNativeProjection);
    ui.ViewpointsCombo->setCurrentIndex(mOptions->Preferences.mNativeViewpoint);
    ui.LPubFadeHighlight->setChecked(mOptions->Preferences.mLPubFadeHighlight);
    ui.ZoomExtentCombo->setCurrentIndex(mOptions->Preferences.mZoomExtents);

    LPub::ViewpointsComboSaveIndex = mOptions->Preferences.mNativeViewpoint;

    auto SetButtonPixmap = [](quint32 Color, QToolButton* Button)
    {
        QPixmap Pixmap(12, 12);

        Pixmap.fill(QColor(LC_RGBA_RED(Color), LC_RGBA_GREEN(Color), LC_RGBA_BLUE(Color)));
        Button->setIcon(Pixmap);
    };

    SetButtonPixmap(mOptions->Preferences.mFadeStepsColor, ui.FadeStepsColor);
    SetButtonPixmap(mOptions->Preferences.mHighlightNewPartsColor, ui.HighlightNewPartsColor);

    on_antiAliasing_toggled();
    on_edgeLines_toggled();
    on_MeshLODSlider_valueChanged();
    on_studStyleCombo_currentIndexChanged(ui.studStyleCombo->currentIndex());
    on_FadeSteps_toggled();
    on_HighlightNewParts_toggled();
    on_AutomateEdgeColor_toggled();

    QPalette readOnlyPalette = QApplication::palette();
    if (mOptions->Preferences.mColorTheme == lcColorTheme::Dark)
        readOnlyPalette.setColor(QPalette::Base,QColor("#3E3E3E")); // THEME_DARK_PALETTE_MIDLIGHT
    else
        readOnlyPalette.setColor(QPalette::Base,QColor("#AEADAC")); // THEME_DEFAULT_PALETTE_LIGHT
    readOnlyPalette.setColor(QPalette::Text,QColor("#808080"));     // LPUB3D_DISABLED_TEXT_COLOUR

    ui.defaultCameraProperties->setChecked(mOptions->Preferences.mDefaultCameraProperties);
    ui.cameraDefaultDistanceFactor->setValue(qreal(mOptions->Preferences.mDDF));
    ui.cameraDefaultPosition->setValue(qreal(mOptions->Preferences.mCDP));
    ui.cameraFoV->setValue(qreal(mOptions->Preferences.mCFoV));
    ui.cameraNearPlane->setValue(qreal(mOptions->Preferences.mCNear));
    ui.cameraFarPlane->setValue(qreal(mOptions->Preferences.mCFar));
    ui.cameraDistanceFactor->setText(QString::number(qreal(mOptions->Preferences.mCDP) /
                                                     -qreal(mOptions->Preferences.mDDF)));

//    connect(ui.resetDefaultDistanceFactor, SIGNAL(clicked()), this, SLOT(cameraPropertyReset()));
//    connect(ui.resetDefaultPosition, SIGNAL(clicked()), this, SLOT(cameraPropertyReset()));
//    connect(ui.resetFoV, SIGNAL(clicked()), this, SLOT(cameraPropertyReset()));
//    connect(ui.resetNearPlane, SIGNAL(clicked()), this, SLOT(cameraPropertyReset()));
//    connect(ui.resetFarPlane, SIGNAL(clicked()), this, SLOT(cameraPropertyReset()));

    ui.cameraDistanceFactor->setReadOnly(true);
    ui.cameraDistanceFactor->setPalette(readOnlyPalette);
}

void PreferencesDialog::lcQPreferencesAccept()
{
    if (!ui.antiAliasing->isChecked())
        mOptions->AASamples = 1;
    else if (ui.antiAliasingSamples->currentIndex() == 2)
        mOptions->AASamples = 8;
    else if (ui.antiAliasingSamples->currentIndex() == 1)
        mOptions->AASamples = 4;
    else
        mOptions->AASamples = 2;
    float const LineWidth = mLineWidthRange[0] + static_cast<float>(ui.LineWidthSlider->value()) * mLineWidthGranularity;
    mOptions->Preferences.mAllowLOD = ui.MeshLOD->isChecked();
    mOptions->Preferences.mDrawEdgeLines = ui.edgeLines->isChecked();
    mOptions->Preferences.mLineWidth = LineWidth > 0.9f && LineWidth < 1.09f ? 1.0f : LineWidth;
    mOptions->Preferences.mMeshLODDistance = ui.MeshLODSlider->value() * mMeshLODMultiplier;
    mOptions->Preferences.mShadingMode = (lcShadingMode)ui.ShadingMode->currentIndex();
    mOptions->StudStyle = static_cast<lcStudStyle>(ui.studStyleCombo->currentIndex());
    mOptions->Preferences.mDrawConditionalLines = ui.ConditionalLinesCheckBox->isChecked();
    mOptions->Preferences.mFadeSteps = ui.FadeSteps->isChecked();
    mOptions->Preferences.mHighlightNewParts = ui.HighlightNewParts->isChecked();
    mOptions->Preferences.mAutomateEdgeColor = ui.AutomateEdgeColor->isChecked();
    mOptions->Preferences.mNativeViewpoint = ui.ViewpointsCombo->currentIndex();
    mOptions->Preferences.mNativeProjection = ui.ProjectionCombo->currentIndex();
    mOptions->Preferences.mLPubFadeHighlight = ui.LPubFadeHighlight->isChecked();
    mOptions->Preferences.mZoomExtents = ui.ZoomExtentCombo->currentIndex();
    mOptions->Preferences.mDefaultCameraProperties = ui.defaultCameraProperties->isChecked();
    mOptions->Preferences.mDDF = float(ui.cameraDefaultDistanceFactor->value());
    mOptions->Preferences.mCDP = float(ui.cameraDefaultPosition->value());
    mOptions->Preferences.mCFoV = float(ui.cameraFoV->value());
    mOptions->Preferences.mCNear = float(ui.cameraNearPlane->value());
    mOptions->Preferences.mCFar = float(ui.cameraFarPlane->value());
}

// function calls
void PreferencesDialog::on_antiAliasing_toggled()
{
    ui.antiAliasingSamples->setEnabled(ui.antiAliasing->isChecked());
}

void PreferencesDialog::on_edgeLines_toggled()
{
    const bool Enable = ui.edgeLines->isChecked() || ui.ConditionalLinesCheckBox->isChecked();

    ui.LineWidthSlider->setEnabled(Enable);
    ui.LineWidthLabel->setEnabled(Enable);
}

void PreferencesDialog::on_ConditionalLinesCheckBox_toggled()
{
    const bool Enable = ui.edgeLines->isChecked() || ui.ConditionalLinesCheckBox->isChecked();

    ui.LineWidthSlider->setEnabled(Enable);
    ui.LineWidthLabel->setEnabled(Enable);
}

void PreferencesDialog::on_LineWidthSlider_valueChanged()
{
    float Value = mLineWidthRange[0] + static_cast<float>(ui.LineWidthSlider->value()) * mLineWidthGranularity;
    ui.LineWidthLabel->setText(QString::number((Value > 0.9f && Value < 1.09f) ? 1.0f : Value));
}

void PreferencesDialog::on_MeshLODSlider_valueChanged()
{
    float Value = ui.MeshLODSlider->value() * mMeshLODMultiplier;
    ui.MeshLODLabel->setText(QString::number(static_cast<int>(Value)));
}

void PreferencesDialog::on_studStyleCombo_currentIndexChanged(int index)
{
    ui.HighContrastButton->setEnabled(lcIsHighContrast(static_cast<lcStudStyle>(index)));
}

void PreferencesDialog::AutomateEdgeColor()
{
    lcAutomateEdgeColorDialog Dialog(this, sender() == ui.HighContrastButton);
    if (Dialog.exec() == QDialog::Accepted)
    {
        mOptions->Preferences.mPartEdgeContrast = Dialog.mPartEdgeContrast;
        mOptions->Preferences.mPartColorValueLDIndex = Dialog.mPartColorValueLDIndex;
        mOptions->Preferences.mStudCylinderColorEnabled = Dialog.mStudCylinderColorEnabled;
        mOptions->Preferences.mStudCylinderColor = Dialog.mStudCylinderColor;
        mOptions->Preferences.mPartEdgeColorEnabled = Dialog.mPartEdgeColorEnabled;
        mOptions->Preferences.mPartEdgeColor = Dialog.mPartEdgeColor;
        mOptions->Preferences.mBlackEdgeColorEnabled = Dialog.mBlackEdgeColorEnabled;
        mOptions->Preferences.mBlackEdgeColor = Dialog.mBlackEdgeColor;
        mOptions->Preferences.mDarkEdgeColorEnabled = Dialog.mDarkEdgeColorEnabled;
        mOptions->Preferences.mDarkEdgeColor = Dialog.mDarkEdgeColor;
    }
}

void PreferencesDialog::on_LPubFadeHighlight_toggled()
{
    if (ui.LPubFadeHighlight->isChecked())
    {
        if (ui.FadeSteps->isChecked())
            ui.FadeSteps->setChecked(false);
        if (ui.HighlightNewParts->isChecked())
            ui.HighlightNewParts->setChecked(false);
    }
}

void PreferencesDialog::on_FadeSteps_toggled()
{
    ui.FadeStepsColor->setEnabled(ui.FadeSteps->isChecked());
    ui.ResetFadeStepsButton->setEnabled(ui.FadeSteps->isChecked());
    if (ui.FadeSteps->isChecked() && ui.LPubFadeHighlight->isChecked())
        ui.LPubFadeHighlight->setChecked(false);
}

void PreferencesDialog::on_HighlightNewParts_toggled()
{
    ui.HighlightNewPartsColor->setEnabled(ui.HighlightNewParts->isChecked());
    ui.ResetHighlightNewPartsButton->setEnabled(ui.HighlightNewParts->isChecked());
    if (ui.HighlightNewParts->isChecked() && ui.LPubFadeHighlight->isChecked())
        ui.LPubFadeHighlight->setChecked(false);
}

void PreferencesDialog::on_AutomateEdgeColor_toggled()
{
    ui.AutomateEdgeColorButton->setEnabled(ui.AutomateEdgeColor->isChecked());
}

void PreferencesDialog::on_ViewpointsCombo_currentIndexChanged(int index)
{
    int ProjectionComboSaveIndex = ui.ProjectionCombo->currentIndex();

    if (index < static_cast<int>(lcViewpoint::Count/*Default*/))
        ui.ProjectionCombo->setCurrentIndex(2/*Default*/);

    QDialog::DialogCode DialogCode = QDialog::Accepted;

    if (index == static_cast<int>(lcViewpoint::LatLon)) {
        DialogCode = static_cast<QDialog::DialogCode>(lpub->SetViewpointLatLonDialog(true/*SetCamera*/));
        if (DialogCode != QDialog::Accepted) {
            ui.ViewpointsCombo->setCurrentIndex(LPub::ViewpointsComboSaveIndex);
            ui.ProjectionCombo->setCurrentIndex(ProjectionComboSaveIndex);
        }
    }

    if (DialogCode == QDialog::Accepted)
        LPub::ViewpointsComboSaveIndex = index;
}

void PreferencesDialog::on_ProjectionCombo_currentIndexChanged(int index)
{
    if (mSetOptions)
        return;

    if (ui.preferredRenderer->currentText() == rendererNames[RENDERER_NATIVE]) {
        const int currentIndex = index == 2 /*Default*/ ? 0 /*Perspective*/ : index;
        ui.projectionCombo->setCurrentIndex(currentIndex);
    }
    if (index < 2/*Default*/) {
        ui.ViewpointsCombo->setCurrentIndex(static_cast<int>(lcViewpoint::Count));
    }
}

void PreferencesDialog::on_cameraDefaultDistanceFactor_valueChanged(double value)
{
    qreal cdp = ui.cameraDefaultPosition->value();
    ui.cameraDistanceFactor->setText(QString::number(cdp / -value));
}

void PreferencesDialog::on_cameraDefaultPosition_valueChanged(double value)
{
    qreal ddf = ui.cameraDefaultDistanceFactor->value();
    ui.cameraDistanceFactor->setText(QString::number(value / -ddf));
}

void PreferencesDialog::cameraPropertyReset()
{
    if (sender() == ui.resetDefaultDistanceFactor) {
        ui.cameraDefaultDistanceFactor->setValue(qreal(8.0));
    } else if (sender() == ui.resetDefaultPosition) {
        ui.cameraDefaultPosition->setValue(qreal(1250.0));
    } else if (sender() == ui.resetFoV) {
        ui.cameraFoV->setValue(qreal(30.0));
    } else if (sender() == ui.resetNearPlane) {
        ui.cameraNearPlane->setValue(qreal(25));
    } else if (sender() == ui.resetFarPlane) {
        ui.cameraFarPlane->setValue(qreal(50000.0));
    }
}

// color buttons
void PreferencesDialog::ColorButtonClicked()
{
    QObject* Button = sender();
    QString Title;
    quint32* Color = nullptr;
    QColorDialog::ColorDialogOptions DialogOptions;

    if (Button == ui.FadeStepsColor) {
        Color = &mOptions->Preferences.mFadeStepsColor;
        Title = tr("Select Fade Color");
        DialogOptions = QColorDialog::ShowAlphaChannel;
    }
    else if (Button == ui.HighlightNewPartsColor) {
        Color = &mOptions->Preferences.mHighlightNewPartsColor;
        Title = tr("Select Highlight Color");
        DialogOptions = QColorDialog::ShowAlphaChannel;
    }
    else
        return;

    QColor oldColor = QColor(LC_RGBA_RED(*Color), LC_RGBA_GREEN(*Color), LC_RGBA_BLUE(*Color), LC_RGBA_ALPHA(*Color));
    QColor newColor = QColorDialog::getColor(oldColor, this, Title, DialogOptions);

    if (newColor == oldColor || !newColor.isValid())
        return;

    *Color = LC_RGBA(newColor.red(), newColor.green(), newColor.blue(), newColor.alpha());

    QPixmap pix(12, 12);

    newColor.setAlpha(255);
    pix.fill(newColor);
    ((QToolButton*)Button)->setIcon(pix);
}

// reset buttons
void PreferencesDialog::ResetFadeHighlightColor()
{
    quint32* Color = nullptr;
    QPixmap pix(12, 12);
    QColor resetColor;

    if (sender() == ui.ResetFadeStepsButton) {
        Color = &mOptions->Preferences.mFadeStepsColor;
       *Color = LC_RGBA(128, 128, 128, 128);
        resetColor = QColor(LC_RGBA_RED(*Color), LC_RGBA_GREEN(*Color), LC_RGBA_BLUE(*Color), LC_RGBA_ALPHA(*Color));
        pix.fill(resetColor);
        ui.FadeStepsColor->setIcon(pix);
    }
    else if (sender() == ui.ResetHighlightNewPartsButton) {
        Color = &mOptions->Preferences.mHighlightNewPartsColor;
       *Color = LC_RGBA(255, 242, 0, 192);
        resetColor = QColor(LC_RGBA_RED(*Color), LC_RGBA_GREEN(*Color), LC_RGBA_BLUE(*Color), LC_RGBA_ALPHA(*Color));
        pix.fill(resetColor);
        ui.HighlightNewPartsColor->setIcon(pix);
    }
}
