/****************************************************************************
**
** Copyright (C) 2018 - 2019 Trevor SANDY. All rights reserved.
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
#include <LDExporter/LDExporterSetting.h>
#include <LDExporter/LDPovExporter.h>
#include <LDVWidgetDefaultKeys.h>
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
    m_POVLightsLoaded(false)
{
    setupUi(this);

    IniFlag iniFlag = modelWidget->getIniFlag();

    if (m_modelViewer) {
        switch (iniFlag){
        case NativePOVIni:
        case LDViewPOVIni:
        case POVRayRender:
            m_exporter  = m_modelViewer->getExporter(LDrawModelViewer::ETPov, true);
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

    connect( okButton,     SIGNAL( clicked() ), this, SLOT( doOk() ) );
    connect( applyButton,  SIGNAL( pressed() ), this, SLOT( doApply() ) );
    connect( cancelButton, SIGNAL( clicked() ), this, SLOT( doCancel() ) );
    connect( resetButton,  SIGNAL( clicked() ), this, SLOT( doReset() ) );

    if (!extraSearchDirs)
    {
        extraSearchDirs = new TCStringArray;
        captureExtraSearchDirs();
    }

    populateExportSettings();

    this->setWindowTitle(modelWidget->getIniTitle().append(" Export Options"));

    QPalette readOnlyPalette;
    readOnlyPalette.setColor(QPalette::Base,Qt::lightGray);
    QString iniFileMessage;
    if (TCUserDefaults::isIniFileSet())
    {
        QString prefSet = TCUserDefaults::getSessionName();
        iniFileMessage = QString("%1").arg(modelWidget->getIniFile());
        iniBox->setTitle(QString("INI using '%1' preference set")
                         .arg(prefSet.isEmpty() ? "Default" : prefSet));
    } else {
        iniFileMessage = QString("INI file not specified. Using built-in default settings.");
        iniFileEdit->setStyleSheet("QLineEdit { background-color : red; color : white; }");
    }

    iniFileEdit->setReadOnly(true);
    iniFileEdit->setPalette(readOnlyPalette);
    iniFileEdit->setText(iniFileMessage);

    applyButton->setEnabled(false);

    adjustSize();
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
    QSpacerItem *sp = nullptr;
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
                    QHBoxLayout *hbox;
                    QPushButton *rg;
                    hbox = new QHBoxLayout();
                    rg = new QPushButton("Reset Group");
                    sp = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
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
                sp = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
                hbox->addSpacerItem(sp);
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
                if (it->isPath() ||
                      (label->text().contains(TCObject::ls("PovBottomInclude")) ||
                       label->text().contains(TCObject::ls("PovTopInclude"))))
                {
                    QPushButton *but = new QPushButton();
                    but->setText(TCObject::ls("LDXBrowse..."));
                    hbox2->addWidget(but);
                    connect( but, SIGNAL( clicked() ), this, SLOT( doBrowse()));
                    m_button[but] = li;
                } else if (label->text() != TCObject::ls("PovLights")) {
                    hbox2->addSpacerItem(sp);
                }
                if (label->text() != TCObject::ls("PovLights")){
                    connect( li, SIGNAL( editingFinished() ), this, SLOT( enableApply() ) );
                } else {
                   label->hide();
                   li->hide();
                   li->setObjectName(label->text());

                   m_pov_lightList = qstmp.split(";");

                   QGridLayout *grid = new QGridLayout();
                   grid->setSpacing(4);
                   vbox->addLayout(grid);
                   QLabel *label;
                   QValidator *validator;
                   QPalette palette;

                   label = new QLabel(TCObject::ls("PovLightNum"),this);
                   grid->addWidget(label,0,0,1,1);
                   label = new QLabel(TCObject::ls("PovLatitude"),this);
                   grid->addWidget(label,0,1,1,1);
                   label = new QLabel(TCObject::ls("PovLongitude"),this);
                   grid->addWidget(label,0,2,1,1);

                   vbox = new QVBoxLayout();
                   grid->addItem(vbox,1,3,5,1);
                   QPushButton *abut = new QPushButton(this);
                   abut->setText(TCObject::ls("PovAddLight"));
                   vbox->addWidget(abut);
                   connect( abut, SIGNAL( clicked() ), this, SLOT( addLight()));

                   m_liNumEdit =  new QLineEdit(this);
                   palette.setColor(QPalette::Base,Qt::lightGray);
                   m_liNumEdit->setPalette(palette);
                   m_liNumEdit->setReadOnly(true);
                   validator = new QIntValidator(0, 127, this);
                   m_liNumEdit->setValidator(validator);
                   grid->addWidget(m_liNumEdit,1,0,1,1);
                   connect( m_liNumEdit, SIGNAL( editingFinished() ), this, SLOT( applyLights() ) );

                   m_liLatEdit =  new QLineEdit(this);
                   validator = new QDoubleValidator(-360, 360, 4,this);
                   m_liLatEdit->setValidator(validator);
                   grid->addWidget(m_liLatEdit,1,1,1,1);
                   connect( m_liLatEdit, SIGNAL( editingFinished() ), this, SLOT( applyLights() ) );

                   m_liLonEdit =  new QLineEdit(this);
                   validator = new QDoubleValidator(-360, 360, 4,this);
                   m_liLonEdit->setValidator(validator);
                   grid->addWidget(m_liLonEdit,1,2,1,1);
                   connect( m_liLonEdit, SIGNAL( editingFinished() ), this, SLOT( applyLights() ) );

                   QPushButton *rbut = new QPushButton(this);
                   rbut->setText(TCObject::ls("PovRemoveLight"));
                   vbox->addWidget(rbut);
                   connect( rbut, SIGNAL( clicked() ), this, SLOT( removeLight()));

                   label = new QLabel(TCObject::ls("PovLightIntensity"),this);
                   grid->addWidget(label,2,0,1,1);
                   label = new QLabel(TCObject::ls("PovLightAreaSize"),this);
                   grid->addWidget(label,2,1,1,1);
                   label = new QLabel(TCObject::ls("PovAreaLightsNum"),this);
                   grid->addWidget(label,2,2,1,1);

                   sp = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
                   vbox->addSpacerItem(sp);

                   m_liIntEdit =  new QLineEdit(this);
                   validator = new QDoubleValidator(0.0, 100, 4,this);
                   m_liIntEdit->setValidator(validator);
                   grid->addWidget(m_liIntEdit,3,0,1,1);
                   connect( m_liIntEdit, SIGNAL( editingFinished() ), this, SLOT( applyLights() ) );

                   m_liASizeEdit =  new QLineEdit(this);
                   validator = new QIntValidator(0, 20000, this);
                   m_liASizeEdit->setValidator(validator);
                   grid->addWidget(m_liASizeEdit,3,1,1,1);
                   connect( m_liASizeEdit, SIGNAL( editingFinished() ), this, SLOT( applyLights() ) );

                   m_liALightsSpin =  new QSpinBox(this);
                   grid->addWidget(m_liALightsSpin,3,2,1,1);
                   connect( m_liALightsSpin, SIGNAL( valueChanged(int) ), this, SLOT( setLights(int)));

                   m_liCombo = new QComboBox(this);
                   grid->addWidget(m_liCombo,4,0,1,3);
                   connect( m_liCombo, SIGNAL( currentIndexChanged(int) ), this, SLOT( selectLight(int)));

                   m_messageLabel = new QLabel(this);
                   grid->addWidget(m_messageLabel,5,0,1,4);

                   setLights();

                   m_POVLightsLoaded = true;
                }
                break;
            case LDExporterSetting::TEnum:
                vbox = new QVBoxLayout();
                vbox->setSpacing(4);
                hbox->addLayout(vbox);
                sp = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
                hbox->addSpacerItem(sp);
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
            if (((QLineEdit *)it->second)->objectName() == TCObject::ls("PovLights"))
            {
                qstringtoucstring(value, m_pov_lightList.join(";"));
            } else {
                qstringtoucstring(value, ((QLineEdit *)it->second)->text());
            }
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
    applyExtraSearchDirs();
    TCUserDefaults::flush();
    applyButton->setEnabled(false);
}

void LDViewExportOption::doOk(void)
{
    doApply();
    QDialog::accept();
    QDialog::close();
}

void LDViewExportOption::doCancel(void)
{
    QDialog::reject();
    QDialog::close();
}

void LDViewExportOption::doReset(void)
{
    resetSettings(m_settings);
    captureExtraSearchDirs();
    enableApply();
}

void LDViewExportOption::doResetGroup()
{
    QPushButton *pb = qobject_cast<QPushButton *>(sender());
    resetSettings(m_groups[pb->parentWidget()->layout()]);
    enableApply();
}

void LDViewExportOption::resetSettings(SettingsMap &settings)
{
    SettingsMap::const_iterator it;
    QString value;

    for (it = settings.begin(); it != settings.end(); it++)
    {
        LDVExportSetting *setting = (LDVExportSetting*)it->first;
        bool povLights = ((QLineEdit *)it->second)->objectName() == TCObject::ls("PovLights");
        if (povLights){
           setting->resetLights();
        } else {
           setting->reset();
        }

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
            if (povLights)
             {
                 m_pov_lightList = value.split(";");
                 setLights();
             } else {
                 ((QLineEdit *)it->second)->setText(value);
             }
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

void LDViewExportOption::doBrowse()
{
    QPushButton *pb = qobject_cast<QPushButton *>(sender());
    QString dir;
    dir = QFileDialog::getOpenFileName(this,"",m_button[pb]->text());
    if (!dir.isEmpty())
    {
            m_button[pb]->setText (dir);
            enableApply();
    }
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

// Lights

void LDViewExportOption::setLights(void)
{
    QStringList tempList;
    QStringList lights;
    m_liCombo->clear();
    foreach (QString light, m_pov_lightList){
        tempList = light.split(" ");
        lights << QString("%1. Latitude %2 Longitude %3")
                          .arg(tempList.at(0))
                          .arg(tempList.at(1))
                          .arg(tempList.at(2));
    }
    m_liCombo->addItems(       lights);
    m_liCombo->setCurrentIndex(m_liCombo->count() - 1);

    int lightIndex = m_liCombo->currentIndex();

    tempList = m_pov_lightList.at(lightIndex).split(" ");

    m_liNumEdit->setText(     tempList.at(0));
    m_liLatEdit->setText(     tempList.at(1));
    m_liLonEdit->setText(     tempList.at(2));

    m_liIntEdit->setText(     tempList.at(3));
    m_liASizeEdit->setText(   tempList.at(4));
    m_liALightsSpin->setValue(tempList.at(5).toInt());
}

void LDViewExportOption::selectLight(int lightIndex)
{

    if (m_liCombo->count() == 0 || !m_POVLightsLoaded)
        return;

    // populate LineEdit and Spin controls
    QStringList tempList = m_pov_lightList.at(lightIndex).split(" ");

    m_liNumEdit->setText(      tempList.at(0));
    m_liLatEdit->setText( tempList.at(1));
    m_liLonEdit->setText(tempList.at(2));
    m_liIntEdit->setText(  tempList.at(3));
    m_liASizeEdit->setText(   tempList.at(4));
    m_liALightsSpin->setValue(tempList.at(5).toInt());
}

void LDViewExportOption::addLight()
{
    if (m_liLatEdit->displayText().isEmpty() || m_liLonEdit->displayText().isEmpty())
        return;


    QString num = QString("%1").arg(m_liNumEdit->displayText());
    QString pos = QString("Latitude %1 Longitude %2")
            .arg(m_liLatEdit->displayText())
            .arg(m_liLonEdit->displayText());
    QString comboItem = QString("%1. %2").arg(num).arg(pos);

    int lightIndex = m_liCombo->findText(comboItem);

    if (lightIndex != -1) {
        m_messageLabel->setText(QString("You must enter unique longitude or latitude values before clicking 'Add'."));
        m_messageLabel->setStyleSheet("QLabel { color : red; }");
        return;
    }

    num = QString("%1").arg(m_liCombo->count() + 1);

    comboItem = QString("%1. %2").arg(num).arg(pos);

    pos = QString("%1 %2")
                .arg(m_liLatEdit->displayText())
                .arg(m_liLonEdit->displayText());
    QString intensity  = m_liIntEdit->displayText();
    QString aSize   = m_liASizeEdit->displayText();
    QString aLights = QString::number(m_liALightsSpin->value());

    QString lightEntry = QString("%1 %2 %3 %4 %5").arg(num).arg(pos)
                         .arg(intensity).arg(aSize).arg(aLights);

    m_pov_lightList.append(lightEntry);

    m_liNumEdit->setText(     QString("%1").arg(m_liCombo->count()));
    m_liCombo->addItem(        comboItem);
    m_liCombo->setCurrentIndex(m_liCombo->count() - 1);

    enableApply();
}

void LDViewExportOption::removeLight()
{
    int removeIndex = m_liNumEdit->displayText().toInt() - 1;

    m_liCombo->removeItem(                 removeIndex);
    m_liCombo->setCurrentIndex(            m_liCombo->count() - 1);
    m_liNumEdit->setText(                 QString("%1").arg(m_liCombo->count()));

    m_pov_lightList.removeAt(removeIndex);

    int lightIndex = m_liCombo->currentIndex();

    QStringList tempList = m_pov_lightList.at(lightIndex).split(" ");

    m_liNumEdit->setText(      tempList.at(0));
    m_liLatEdit->setText( tempList.at(1));
    m_liLonEdit->setText(tempList.at(2));
    m_liIntEdit->setText(  tempList.at(3));
    m_liASizeEdit->setText(   tempList.at(4));
    m_liALightsSpin->setValue(tempList.at(5).toInt());

    enableApply();
}

void LDViewExportOption::setLights(int arg1)
{
    Q_UNUSED(arg1)
    applyLights();
}

void LDViewExportOption::applyLights(void)
{
    if (m_liCombo->count() == 0 || !m_POVLightsLoaded)
        return;

    m_messageLabel->clear();

    int lightIndex = m_liCombo->currentIndex();

    QString num = QString("%1").arg(m_liNumEdit->displayText());
    QString pos = QString("%1 %2")
                          .arg(m_liLatEdit->displayText())
                          .arg(m_liLonEdit->displayText());
    QString intensity  = m_liIntEdit->displayText();
    QString aSize   = m_liASizeEdit->displayText();
    QString aLights = QString::number(m_liALightsSpin->value());

    QString lightEntry = QString("%1 %2 %3 %4 %5").arg(num).arg(pos)
                                 .arg(intensity).arg(aSize).arg(aLights);

    m_pov_lightList.replace(lightIndex,lightEntry);

    enableApply();
}

void LDVExportSetting::resetLights()
{
    std::string value = TCUserDefaults::defaultStringForKey(m_key.c_str());
    stringtowstring(m_string,value);
}
