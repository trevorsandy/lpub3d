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
    ui->version->setText(tr("<b>%1</b> Version <b>%2</b> - Build %3").arg(QString::fromLatin1(VER_PRODUCTNAME_STR))
                                                                     .arg(QString::fromLatin1(VER_TEXT))
                                                                     .arg(QString::number(VER_BUILD)));

    QString VersionFormat = tr("OpenGL Version %1<br>%2 - %3<br><br>");
	QString Version = VersionFormat.arg(QString((const char*)glGetString(GL_VERSION)), QString((const char*)glGetString(GL_RENDERER)), QString((const char*)glGetString(GL_VENDOR)));
    QString AboutFormat = tr("<b>LPubV</b> is a WYSIWYG tool for creating<br>"
                             "LEGO(c) style building instructions.<br>"
                             "LPubV is a significant update and refresh of LPub.<br>"
                             "Original LPub source code and application can be found on<br>"
                             "www.sourceforge.net/projects/lpub4/files.<br>"
                             "Copyright 2000-2011 Kevin Clague<br>"
                             "kevin.clague@gmail.com<br>"
                             "2014 - Daniele Benedettelli<br>"
                             "2015 - Trevor SANDY "
                             "<A HREF=\"%1\">trevor.sandy@gmail.com</A>");
    QString About = AboutFormat.arg(QString::fromLatin1(VER_PUBLISHER_EMAIL));

    ui->info->setTextFormat(Qt::RichText);
    ui->info->setOpenExternalLinks(true);
    ui->info->setText(Version + About);
}

AboutDialog::~AboutDialog()
{
	delete ui;
}

