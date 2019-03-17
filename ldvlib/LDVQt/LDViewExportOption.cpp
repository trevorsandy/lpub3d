/****************************************************************************
**
** Copyright (C) 2018 Trevor SANDY. All rights reserved.
**
** This file may be used under the terms of the
** GNU General Public Liceense (GPL) version 3.0
** which accompanies this distribution, and is
** available at http://www.gnu.org/licenses/gpl.html
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QCheckBox>
#include <QToolTip>
#include <TCFoundation/TCStringArray.h>
#include <TCFoundation/TCUserDefaults.h>
#include <TCFoundation/mystring.h>
#include <LDLib/LDUserDefaultsKeys.h>
#include <LDExporter/LDExporter.h>
#include <LDVMisc.h>
#include "LDViewExportOption.h"
#include "LDVWidget.h"

#include "ui_LDVExportOptionPanel.h"

#include "lpub_preferences.h"
#include "name.h"

TCStringArray* LDViewExportOption::extraSearchDirs = nullptr;

#define	MAX_EXTRA_DIR	10

LDViewExportOption::LDViewExportOption(LDVWidget *modelWidget)
    :QDialog(qobject_cast<QWidget*>(modelWidget)),
    LDVExportOptionPanel(),
    m_modelViewer(modelWidget->getModelViewer() ? ((LDrawModelViewer*)modelWidget->getModelViewer()->retain()) : nullptr),
    m_exporter(nullptr),
    m_box(nullptr),
    m_lay(nullptr),
    m_POVLights(false)
{
    setupUi(this);

    IniFlag iniFlag = modelWidget->getIniFlag();

    if (m_modelViewer) {
        switch (iniFlag){
        case NativePOVIni:
            m_exporter  = m_modelViewer->getExporter(LDrawModelViewer::ETPov, true);
            m_POVLights = true;
            break;
        case NativeSTLIni:
            m_exporter  = m_modelViewer->getExporter(LDrawModelViewer::ETStl, true);
            break;
        case Native3DSIni:
            m_exporter  = m_modelViewer->getExporter(LDrawModelViewer::ET3ds, true);
            break;
        default:
            break;
        }
    }

    connect( okButton, SIGNAL( clicked() ), this, SLOT( doOk() ) );
    connect( applyButton, SIGNAL( pressed() ), this, SLOT( doApply() ) );
    connect( cancelButton, SIGNAL( clicked() ), this, SLOT( doCancel() ) );
    connect( resetButton, SIGNAL( clicked() ), this, SLOT( doReset() ) );

    if (!extraSearchDirs)
    {
        extraSearchDirs = new TCStringArray;
        captureExtraSearchDirs();
    }

    if (m_POVLights)
    {
        connect( povLightLatitudeLnEdit, SIGNAL( editingFinished() ), this, SLOT( enableApply() ) );
        connect( povLightLongitudeLnEdit, SIGNAL( editingFinished() ), this, SLOT( enableApply() ) );
        connect( povLightingGrpResetBtn, SIGNAL( clicked() ),          this, SLOT( resetLights() ) );
        setLights();
    }
    else
    {
        povLightingGrpBox->hide();
    }

    populateExportSettings();

    this->setWindowTitle(modelWidget->getIniTitle().append("Export Options"));

    QPalette readOnlyPalette;
    readOnlyPalette.setColor(QPalette::Base,Qt::lightGray);
    QString iniFileMessage;
    if (TCUserDefaults::isIniFileSet())
    {
        iniFileMessage = QString("%1").arg(modelWidget->getIniFile());
        iniBox->setTitle(QString("%1 INI file").arg(modelWidget->getIniTitle()));
    } else {
        iniFileMessage = QString("INI file not specified. Using built-in default settings.");
        iniFileEdit->setStyleSheet("QLineEdit { background-color : red; color : white; }");
    }

    iniFileEdit->setReadOnly(true);
    iniFileEdit->setPalette(readOnlyPalette);
    iniFileEdit->setText(iniFileMessage);

    applyButton->setEnabled(false);
}

LDViewExportOption::~LDViewExportOption()
{
}

void LDViewExportOption::show(void)
{
    applyButton->setEnabled(false);
    QDialog::show();
    raise();
}

void LDViewExportOption::populateExportSettings(void)
{
    if (!m_exporter)
        return;

    QWidget *parent;
    parent = m_box;
    LDExporterSettingList &settings = m_exporter->getSettings();
    LDExporterSettingList::iterator it;

    if (m_box != nullptr)
    {
        scrollArea->adjustSize();
        delete m_box;
    }
    m_box = new QWidget(scrollArea);
    m_box->setObjectName("m_box");
    m_lay = new QVBoxLayout();
    m_lay->setObjectName("m_lay");
    m_lay->setMargin(11);
    m_lay->setSpacing(4);
    m_box->setLayout(m_lay);
    QVBoxLayout *vbl= nullptr;
    std::stack<int> groupSizes;
    std::stack<QWidget *> parents;
    int groupSize = 0;

    for (it = settings.begin(); it != settings.end(); it++)
    {
        bool inGroup = groupSize > 0;

        if (groupSize > 0)
        {
            groupSize--;
            if (groupSize == 0)
            {
                // We just got to the end of a group, so pop the previous
                // groupSize value off the groupSizes stack.
                groupSize = groupSizes.top();
                groupSizes.pop();
                parent = parents.top();
                parents.pop();
            }
        }
        if (it->getGroupSize() > 0)
        {
            // This item is the start of a group.
            if (inGroup)
            {
                // At the beginning of this iteration we were in a group, so
                // use a bool setting instead of a group setting.
                QString qstmp;
                ucstringtoqstring(qstmp,it->getName());

                QCheckBox *check;
                check = new QCheckBox(qstmp,parent);
                check->setObjectName(qstmp);
                check->setChecked(it->getBoolValue());
                m_settings[&*it] = check;
                if (vbl)
                {
                    vbl->addWidget(check);
                    m_groups[vbl][&*it] = check;
                }
                connect( check, SIGNAL( toggled(bool) ), this, SLOT( enableApply() ) );
            }
            else
            {
                // Top level group; use a group setting.
                if (vbl)
                {
                    QSpacerItem *sp = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
                    QHBoxLayout *hbox;
                    QPushButton *rg;
                    hbox = new QHBoxLayout();
                    rg = new QPushButton("Reset Group");
                    hbox->addItem(sp);
                    hbox->addWidget(rg);
                    rg->setObjectName("Reset Group");
                    vbl->addLayout(hbox);
                    connect( rg, SIGNAL( clicked() ), this, SLOT( doResetGroup() ) );
                }
                QString qstmp;
                ucstringtoqstring(qstmp,it->getName());
                QGroupBox *gb;
                gb = new QGroupBox (qstmp, m_box);
                gb->setObjectName(qstmp);
                m_lay->addWidget(gb);
                vbl = new QVBoxLayout(gb);
                gb->setLayout(vbl);
                parent=gb;
                if (it->getType() == LDExporterSetting::TBool)
                {
                    gb->setCheckable(true);
                    gb->setChecked(it->getBoolValue());
                    m_settings[&*it] = gb;
                    m_groups[vbl][&*it] = gb;
                }
            }
            parents.push(parent);
            // We're now in a new group, so push the current groupSize onto
            // the groupSizes stack.
            groupSizes.push(groupSize);
            // Update groupSize based on the new group's size.
            groupSize = it->getGroupSize();
        }
        else
        {
            // This setting isn't the start of a group; add the appropriate type
            // of option UI to the canvas.
            QString qstmp;
            ucstringtoqstring(qstmp,it->getName());
            QHBoxLayout *hbox;
            QVBoxLayout *vbox;
            QLineEdit *li;
            QLabel *label;
            QHBoxLayout *hbox2;
            QCheckBox *check;
            hbox = new QHBoxLayout();
            hbox->setSpacing(4);
            switch (it->getType())
            {
            case LDExporterSetting::TBool:
                check = new QCheckBox(qstmp);
                check->setChecked(it->getBoolValue());
                check->setObjectName(qstmp);
                hbox->addWidget(check);
                m_settings[&*it] = check;
                if (vbl != nullptr)
                {
                    m_groups[vbl][&*it] = check;
                }
                connect( check, SIGNAL( toggled(bool) ), this, SLOT( enableApply() ) );
                break;
            case LDExporterSetting::TFloat:
            case LDExporterSetting::TLong:
                // Long and float are intentionally handled the same.
                label = new QLabel(qstmp);
                label->setObjectName(qstmp);
                hbox->addWidget(label);
                li = new QLineEdit(qstmp);
                li->setObjectName(qstmp);
                hbox->addWidget(li);
                ucstringtoqstring(qstmp,it->getStringValue());
                li->setText(qstmp);
                m_settings[&*it] = li;
                if (vbl != nullptr)
                {
                    m_groups[vbl][&*it] = li;
                }
                connect( li, SIGNAL( editingFinished() ), this, SLOT( enableApply() ) );
                break;
            case LDExporterSetting::TString:
                vbox = new QVBoxLayout();
                vbox->setSpacing(4);
                hbox->addLayout(vbox);
                label = new QLabel(qstmp);
                vbox->addWidget(label);
                hbox2 = new QHBoxLayout();
                hbox2->setSpacing(4);
                vbox->addLayout(hbox2);
                li = new QLineEdit(qstmp);
                hbox2->addWidget(li);
                ucstringtoqstring(qstmp,it->getStringValue());
                li->setText(qstmp);
                m_settings[&*it] = li;
                if (vbl != nullptr)
                {
                    m_groups[vbl][&*it] = li;
                }
                connect( li, SIGNAL( editingFinished() ), this, SLOT( enableApply() ) );
                if (it->isPath())
                {
                    QPushButton *but = new QPushButton();
                    but->setText(TCObject::ls("LDXBrowse..."));
                    hbox2->addWidget(but);
                    connect( but, SIGNAL( clicked() ), this, SLOT( doBrowse()));
                    m_button[but]=li;
                }
                break;
            case LDExporterSetting::TEnum:
                vbox = new QVBoxLayout();
                vbox->setSpacing(4);
                hbox->addLayout(vbox);
                label = new QLabel(qstmp);
                vbox->addWidget(label);
                QComboBox *combo;
                combo = new QComboBox();
                vbox->addWidget(combo);
                for (size_t i = 0; i < it->getOptions().size(); i++)
                {
                    ucstringtoqstring(qstmp,it->getOptions()[i]);
                    combo->addItem(qstmp);
                }
                combo->setCurrentIndex(it->getSelectedOption());
                m_settings[&*it] = combo;
                if (vbl != nullptr)
                {
                    m_groups[vbl][&*it] = combo;
                }
                connect( combo, SIGNAL( currentIndexChanged(int) ), this, SLOT( enableApply() ) );
                break;
            default:
                throw "not implemented";
            }
            if (it->getTooltip().size() > 0)
            {
                ucstringtoqstring(qstmp, it->getTooltip());
                //QToolTip::add(hbox, qstmp);
            }
            if (vbl) vbl->addLayout(hbox);
        }
    } // Settings

    if (vbl)
    {
        QSpacerItem *sp = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
        QHBoxLayout *hbox;
        QPushButton *rg;
        hbox = new QHBoxLayout();
        rg = new QPushButton("Reset Group");
        rg->setObjectName("Reset Group");
        hbox->addItem(sp);
        hbox->addWidget(rg);
        vbl->addLayout(hbox);
        connect( rg, SIGNAL( clicked() ), this, SLOT( doResetGroup() ) );
    }

    m_lay->addStretch();
    scrollArea->setWidget(m_box);
    m_box->adjustSize();
    scrollArea->adjustSize();
    setFixedWidth(width());

    setMinimumSize(50,50);
}

void LDViewExportOption::enableApply(void)
{
    applyButton->setEnabled(true);
}

void LDViewExportOption::doApply(void)
{
    SettingsMap::const_iterator it;
    ucstring value;

    for (it = m_settings.begin(); it != m_settings.end(); it++)
    {
        LDExporterSetting *setting = it->first;

        switch (setting->getType())
        {
        case LDExporterSetting::TBool:
            if (strcmp(it->second->metaObject()->className(), "QGroupBox") == 0)
            {
                setting->setValue(((QGroupBox *)it->second)->isChecked(), true);
            }
            else
            {
                setting->setValue(((QCheckBox *)it->second)->isChecked(), true);
            }
            break;
        case LDExporterSetting::TLong:
        case LDExporterSetting::TFloat:
        case LDExporterSetting::TString:
            qstringtoucstring(value, ((QLineEdit *)it->second)->text());
            setting->setValue(value.c_str(), true);
            break;
        case LDExporterSetting::TEnum:
            setting->selectOption(((QComboBox *)it->second)->currentIndex(), true);
            break;
        default:
            // No default, but gets rid of warnings.
            break;
        }
    }
    if (m_POVLights)
        applyLights();
    applyExtraSearchDirs();
    TCUserDefaults::flush();
    applyButton->setEnabled(false);
}

void LDViewExportOption::doOk(void)
{
    doApply();
    QDialog::close();
}

void LDViewExportOption::doCancel(void)
{
    QDialog::close();
}

void LDViewExportOption::doReset(void)
{
    resetSettings(m_settings);
    if (m_POVLights)
        resetLights();
    captureExtraSearchDirs();
    enableApply();
}

void LDViewExportOption::doResetGroup()
{
    QPushButton *pb = qobject_cast<QPushButton *>(sender());
    resetSettings(m_groups[pb->parentWidget()->layout()]);
    enableApply();
}

void LDViewExportOption::doBrowse()
{
    QPushButton *pb = qobject_cast<QPushButton *>(sender());
    QString dir;
    dir = QFileDialog::getExistingDirectory(this,"",m_button[pb]->text());
    if (!dir.isEmpty())
    {
            m_button[pb]->setText (dir);
            enableApply();
    }
}

void LDViewExportOption::resetSettings(SettingsMap &settings)
{
    SettingsMap::const_iterator it;
    QString value;

    for (it = settings.begin(); it != settings.end(); it++)
    {
        LDExporterSetting *setting = it->first;

        setting->reset();
        switch (setting->getType())
        {
        case LDExporterSetting::TBool:
            if (strcmp(it->second->metaObject()->className(), "QGroupBox") == 0)
            {
                ((QGroupBox *)it->second)->setChecked(setting->getBoolValue());
            }
            else
            {
                ((QCheckBox *)it->second)->setChecked(setting->getBoolValue());
            }
            break;
        case LDExporterSetting::TLong:
        case LDExporterSetting::TFloat:
        case LDExporterSetting::TString:
            ucstringtoqstring(value, setting->getStringValue());
            ((QLineEdit *)it->second)->setText(value);
            break;
        case LDExporterSetting::TEnum:
            ((QComboBox *)it->second)->setCurrentIndex(
                setting->getSelectedOption());
            break;
        default:
            // No default, but gets rid of warnings.
            break;
        }
    }
}

void LDViewExportOption::applyLights()
{
   QSettings Settings;
   Preferences::ldvLights = getLights();
   QVariant cValue(Preferences::ldvLights);
   Settings.setValue(QString("%1/%2").arg(POVRAY,"LDVLights"),cValue);
}

void LDViewExportOption::resetLights()
{
    QSettings Settings;
    Preferences::ldvLights = LIGHTS_COMBO_DEFAULT;
    QVariant cValue(Preferences::ldvLights);
    Settings.setValue(QString("%1/%2").arg(POVRAY,"LDVLights"),cValue);
    setLights();
}

void LDViewExportOption::setLights(void)
{
    if (m_POVLights)
    {
        povLightsCombo->clear();
        QStringList myLigths = Preferences::ldvLights.split(",", QString::SkipEmptyParts);

        povLightsCombo->addItems(                    myLigths);
        povLightsCombo->setCurrentIndex(             povLightsCombo->count() - 1);

        QStringList lightEntryFields = povLightsCombo->currentText().split(" ",QString::SkipEmptyParts); // 1. Latitude 45.0 Longitude 0.0
        label_12->setText(                          "Light No");
        label_13->setText(                          "Latitude");
        label_14->setText(                          "Longitude");

        povLightLatitudeLnEdit->setText(            lightEntryFields.at(2));
        povLightLongitudeLnEdit->setText(           lightEntryFields.at(4));
        povLightNumLnEdit->setText(                 QString("%1").arg(lightEntryFields.at(0)).replace(".",""));

        QPalette palette;
        palette.setColor(QPalette::Base,Qt::lightGray);
        povLightNumLnEdit->setPalette(palette);
        povLightNumLnEdit->setReadOnly(true);
    }
}

QString const LDViewExportOption::getLights()
{
  QStringList povLightsComboList;
  for (int index = 0; index < povLightsCombo->count(); index++)
  {
      if (index == 0)
        povLightsComboList << povLightsCombo->itemText(index);
      else
        povLightsComboList << "," << povLightsCombo->itemText(index);
  }
  return povLightsComboList.join(",");
}

void LDViewExportOption::applyExtraSearchDirs(void)
{
    int i;
    int count = extraSearchDirs->getCount();

    for (i = 0; i <= count; i++)
    {
        char key[128];
        char *extraDir;

        sprintf(key, "%s/Dir%03d", EXTRA_SEARCH_DIRS_KEY, i + 1);
        extraDir = extraSearchDirs->stringAtIndex(i);
        if (extraDir)
        {
            TCUserDefaults::setStringForKey(extraDir, key, false);
        }
        else
        {
            TCUserDefaults::removeValue(key, false);
        }
    }
        for (i=count; i<MAX_EXTRA_DIR;i++)
        {
                char key[128];
        char extraDir[]="";


	sprintf(key, "%s/Dir%03d", EXTRA_SEARCH_DIRS_KEY, i + 1);
		TCUserDefaults::setStringForKey(extraDir, key, false);
	}
    if (m_modelViewer)
    {
        m_modelViewer->setExtraSearchDirs(extraSearchDirs);
    }
}

void LDViewExportOption::captureExtraSearchDirs(void)
{
    int i;

    extraSearchDirs->removeAll();
    for (i = 1; true; i++)
    {
        char key[128];
        char *extraSearchDir;

        sprintf(key, "%s/Dir%03d", EXTRA_SEARCH_DIRS_KEY, i);
        extraSearchDir = TCUserDefaults::stringForKey(key, nullptr, false);
        if (extraSearchDir && extraSearchDir[0])
        {
            extraSearchDirs->addString(extraSearchDir);
            delete extraSearchDir;
        }
        else
        {
            break;
        }
    }
}

void LDViewExportOption::on_povLightsCombo_currentIndexChanged(int index)
{
  Q_UNUSED(index)

    if (povLightsCombo->count() == 0)
        return;

    QStringList lightEntryFields = povLightsCombo->currentText().split(" ",QString::SkipEmptyParts);
    povLightNumLnEdit->setText(                QString("%1").arg(lightEntryFields.at(0)).replace(".",""));
    povLightLatitudeLnEdit->setText(           lightEntryFields.at(2));
    povLightLongitudeLnEdit->setText(          lightEntryFields.at(4));
}

void LDViewExportOption::on_povAddLightBtn_clicked()
{
    if (povLightLatitudeLnEdit->displayText().isEmpty() || povLightLongitudeLnEdit->displayText().isEmpty())
        return;

    QString lightPosition = QString("Latitude %1 Longitude %2")
            .arg(povLightLatitudeLnEdit->displayText())
            .arg(povLightLongitudeLnEdit->displayText());

    QString lightNumber = QString("%1").arg(povLightNumLnEdit->displayText());

    QString lightEntry = QString("%1. %2").arg(lightNumber).arg(lightPosition);

    int lightIndex = povLightsCombo->findText(lightEntry);

    if (lightIndex != -1)
        return;

    lightNumber = QString("%1").arg(povLightsCombo->count() + 1);

    lightEntry = QString("%1. %2").arg(lightNumber).arg(lightPosition);

    povLightsCombo->addItem(                    lightEntry);
    povLightsCombo->setCurrentIndex(            povLightsCombo->count() - 1);
    povLightNumLnEdit->setText(                 QString("%1").arg(povLightsCombo->count()));
    enableApply();
}

void LDViewExportOption::on_povRemoveLightBtn_clicked()
{
    int lightIndex = povLightNumLnEdit->displayText().toInt() - 1;
    povLightsCombo->removeItem(                 lightIndex);
    povLightsCombo->setCurrentIndex(            povLightsCombo->count() - 1);
    povLightNumLnEdit->setText(                 QString("%1").arg(povLightsCombo->count()));
    enableApply();
}
