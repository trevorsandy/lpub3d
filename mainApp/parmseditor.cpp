#include <QtGui>
#include <QFrame>
#include "parmseditor.h"
#include "ui_parmseditor.h"
#include "lpub_preferences.h"

#include "QPushButton"
#include "QHBoxLayout"
#include "QVBoxLayout"

ParmsEditor::ParmsEditor(QWidget *parent) :
    QPlainTextEdit(parent),
    ui(new Ui::ParmsEditor)
{
    ui->setupUi(this);

    lineNumberArea = new LineNumberArea(this);

    connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(updateLineNumberAreaWidth(int)));
    connect(this, SIGNAL(updateRequest(QRect,int)), this, SLOT(updateLineNumberArea(QRect,int)));
    connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(highlightCurrentLine()));

    updateLineNumberAreaWidth(0);
    highlightCurrentLine();

    //dynamic widget
//     QWidget *window = new QWidget;
//     window->setWindowTitle("My TEST Layout");

//     QGridLayout *glayout = new QGridLayout;

//     QLabel *label1 = new QLabel("Name");
//     QLineEdit *txtName1 = new QLineEdit;

//     QLabel *label2 = new QLabel("Name");
//     QLineEdit *txtName2 = new QLineEdit;

//     glayout->addWidget(label1,0,0);
//     glayout->addWidget(txtName1,0,1);

//     glayout->addWidget(label2,1,0);
//     glayout->addWidget(txtName2,1,1);

//     QPushButton *button1 = new QPushButton("OK");

//     glayout->addWidget(button1,2,0,1,2);

//     window->setLayout(glayout);
//     window->show();

    //menu
//    QMenuBar* menuBar = new QMenuBar();
//    QMenu *fileMenu = new QMenu("File");
//    menuBar->addMenu(fileMenu);
//    fileMenu->addAction("Save");
//    fileMenu->addAction("Exit");

//    this->layout()->setMenuBar(menuBar);

    //layout
//    QVBoxLayout *boxLayout = new QVBoxLayout(this); // Main layout of widget
//    QTextEdit editor = QPlainTextEdit(this);
//    editor.setLineWrapColumnOrWidth(200);
//    editor.setLineWrapMode(QTextEdit.FixedPixelWidth);
//    boxLayout->addWidget(editor);
}

ParmsEditor::~ParmsEditor()
{
    delete ui;
}

int ParmsEditor::lineNumberAreaWidth()
{
    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }

    int space = 3 + fontMetrics().width(QLatin1Char('9')) * digits;

    return space;
}

void ParmsEditor::updateLineNumberAreaWidth(int /* newBlockCount */)
{
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void ParmsEditor::updateLineNumberArea(const QRect &rect, int dy)
{
    if (dy)
        lineNumberArea->scroll(0, dy);
    else
        lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());

    if (rect.contains(viewport()->rect()))
        updateLineNumberAreaWidth(0);
}

void ParmsEditor::resizeEvent(QResizeEvent *e)
{
    QPlainTextEdit::resizeEvent(e);

    QRect cr = contentsRect();
    lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void ParmsEditor::highlightCurrentLine()
{
    QList<QTextEdit::ExtraSelection> extraSelections;

    if (!isReadOnly()) {
        QTextEdit::ExtraSelection selection;

        QColor lineColor = QColor(Qt::yellow).lighter(160);

        selection.format.setBackground(lineColor);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        extraSelections.append(selection);
    }

    setExtraSelections(extraSelections);
}

void ParmsEditor::lineNumberAreaPaintEvent(QPaintEvent *event)
{
    QPainter painter(lineNumberArea);
    painter.fillRect(event->rect(), Qt::lightGray);

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = (int) blockBoundingGeometry(block).translated(contentOffset()).top();
    int bottom = top + (int) blockBoundingRect(block).height();

    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString number = QString::number(blockNumber + 1);
            painter.setPen(Qt::black);
            painter.drawText(0, top, lineNumberArea->width(), fontMetrics().height(),
                             Qt::AlignRight, number);
        }

        block = block.next();
        top = bottom;
        bottom = top + (int) blockBoundingRect(block).height();
        ++blockNumber;
    }
}

void ParmsEditor::parmsOpen(int &opt){

    QString fileName;
    mOpt = opt;

    if (mOpt == 1)
        fileName = Preferences::freeformAnnotationsFile;
    else
        fileName = Preferences::fadeStepColorPartsFile;

    QFile file(fileName);

    if (! file.open(QIODevice::ReadOnly | QIODevice::Text)){
        QMessageBox::warning(NULL,
                 QMessageBox::tr(VER_PRODUCTNAME_STR),
                 QMessageBox::tr("Cannot read file %1:\n%2.")
                 .arg(fileName)
                 .arg(file.errorString()));
        return;
    }

    QTextStream ss(&file);

    //TEST
    while (! ss.atEnd()){
        QString line = ss.readLine(0);
        qDebug() << "QTextStream Line: " << line;
    }//

    ParmsEditor editor;
    editor.setPlainText(ss.readAll());

    file.close();
}

bool ParmsEditor::parmsSave(int &opt){

    QString fileName;
    mOpt = opt;

    if (mOpt == 1)
        fileName = Preferences::freeformAnnotationsFile;
    else
        fileName = Preferences::fadeStepColorPartsFile;

    bool success = false;

    // check for dirty editor
    if (document()->isModified())

    {
        QFile file(fileName);
        if (!file.open(QFile::WriteOnly | QFile::Text)) {
            QMessageBox::warning(NULL,
                                 QMessageBox::tr(VER_PRODUCTNAME_STR),
                                 QMessageBox::tr("Cannot write file %1:\n%2.")
                                 .arg(fileName)
                                 .arg(file.errorString()));
            return !success;
        }

        QTextDocumentWriter writer(fileName, "plaintext");
        success = writer.write(document());

        if (success){

            document()->setModified(false);
        }
    }
    return success;
}
