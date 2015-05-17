#ifndef PARMSEDITOR_H
#define PARMSEDITOR_H

#include <QPlainTextEdit>
#include <QWidget>
#include <QObject>
#include "version.h"
#include "lpub_preferences.h"

class QPaintEvent;
class QResizeEvent;
class QSize;

class LineNumberArea;

namespace Ui {
class ParmsEditor;
}

class ParmsEditor : public QPlainTextEdit
{
    Q_OBJECT

public:
    explicit ParmsEditor(QWidget *parent = 0);
    ~ParmsEditor();

    void lineNumberAreaPaintEvent(QPaintEvent *event);
    int lineNumberAreaWidth();
    void parmsOpen(int &opt);
    bool parmsSave(int &opt);

protected:
    void resizeEvent(QResizeEvent *event);

private slots:
    void updateLineNumberAreaWidth(int newBlockCount);
    void highlightCurrentLine();
    void updateLineNumberArea(const QRect &, int);

private:
    QWidget *lineNumberArea;
    Ui::ParmsEditor *ui;
   int mOpt;
};


class LineNumberArea : public QWidget
{
public:
    LineNumberArea(ParmsEditor *editor) : QWidget(editor) {
        parmsEditor = editor;
    }

    QSize sizeHint() const {
        return QSize(parmsEditor->lineNumberAreaWidth(), 0);
    }

protected:
    void paintEvent(QPaintEvent *event) {
        parmsEditor->lineNumberAreaPaintEvent(event);
    }

private:
    ParmsEditor *parmsEditor;
};

#endif // PARMSEDITOR_H
