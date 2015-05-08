#include <QtOpenGL>
#include "aboutdialog.h"
#include "ui_aboutdialog.h"
#include "name.h"
#include "version.h"
 
AboutDialog::AboutDialog(QWidget *parent, void *data) :
	QDialog(parent),
	ui(new Ui::AboutDialog)
{
	ui->setupUi(this);

    ui->version->setTextFormat(Qt::RichText);
    ui->version->setText(tr("<b>%1</b> Version <b>%2</b> Rev %3 Build %4").arg(QString::fromLatin1(VER_PRODUCTNAME_STR))
                                                                          .arg(QString::fromLatin1(VER_TEXT))
                                                                          .arg(QString::fromLatin1(VER_REVISION))
                                                                          .arg(QString::fromLatin1(VER_BUILD)));
    ui->description->setTextFormat(Qt::RichText);
    ui->description->setText(tr("<p><b>%1</b> is a free application that can be used <br>"
                                "to create digital build instructions for "
                                "virtual LEGO(c) models.</p>").arg(QString::fromLatin1(VER_PRODUCTNAME_STR)));
    ui->url->setTextFormat(Qt::RichText);
    ui->url->setText(tr("<a href=\"%1\">sourceforge.net/lpub3d</a>").arg(QString::fromLatin1(VER_COMPANYDOMAIN_STR)));

    QString VersionFormat = tr("OpenGL Version %1<br>%2 - %3<br><br>");
    QString Version = VersionFormat.arg(QString((const char*)glGetString(GL_VERSION)),
                                        QString((const char*)glGetString(GL_RENDERER)),
                                        QString((const char*)glGetString(GL_VENDOR)));
    QString AboutFormat = tr("<b>%1</b> is a WYSIWYG tool for creating<br>"
                             "LEGO(c) style building instructions.<br>"
                             "%1 is a significant update and refresh of LPub.<br>"
                             "%1 source code and application can be found on<br>"
                             "%2/files.<br>"
                             "Copyright (c) 2000-2011 Kevin Clague<br>"
                             "%3<br>"
                             "%4<br>"
                             "kevin.clague@gmail.com<br>"
                             "2014 - Daniele Benedettelli<br>"
                             "2015 - %4 <a href=\"%5\">%6</a>");
    QString About = AboutFormat.arg(QString::fromLatin1(VER_PRODUCTNAME_STR))
                               .arg(QString::fromLatin1(VER_UPDATE_URL))
                               .arg(QString::fromLatin1(VER_LEGALCOPYRIGHT_STR))
                               .arg(QString::fromLatin1(VER_PUBLISHER_STR))
                               .arg(QString::fromLatin1(VER_PUBLISHER_SUPPORT_EMAIL))
                               .arg(QString::fromLatin1(VER_PUBLISHER_EMAIL));

    ui->info->setTextFormat(Qt::RichText);
    ui->info->setOpenExternalLinks(true);
    ui->info->setText(Version + About);
}

AboutDialog::~AboutDialog()
{
	delete ui;
}

