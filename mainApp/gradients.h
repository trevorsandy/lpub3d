/****************************************************************************
**
** Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the demonstration applications of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef GRADIENTS_H
#define GRADIENTS_H

#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
#include <QtWidgets>
#else
#include <QtGui>
#endif
#include "hoverpoints.h"
#include "meta.h"

#include "QsLog.h"

class HoverPoints;
class ShadeWidget : public QWidget
{
    Q_OBJECT

public:
    enum ShadeType {
        RedShade,
        GreenShade,
        BlueShade,
        ARGBShade
    };

    ShadeWidget(ShadeType type, QWidget *parent);

    void setGradientStops(const QGradientStops &stops);

    void paintEvent(QPaintEvent *e);

    QSize sizeHint() const { return QSize(150, 40); }
    QPolygonF points() const;

    HoverPoints *hoverPoints() const { return m_hoverPoints; }

    uint colorAt(int x);

signals:
    void colorsChanged();

private:
    void generateShade();

    ShadeType m_shade_type;
    QImage m_shade;
    HoverPoints *m_hoverPoints;
    QLinearGradient m_alpha_gradient;
};

class GradientRenderer : public QWidget
{
    Q_OBJECT

public:
    GradientRenderer(QSize bgSize, QWidget *parent);

    void paint(QPainter *p);

    QSize sizeHint() const { return QSize(400, 400); }
    HoverPoints *hoverPoints() const { return m_hoverPoints; }
    void paintEvent(QPaintEvent *);
    bool preferImage() const { return m_prefer_image; }
    QGradient getGradient();

#if defined(QT_OPENGL_SUPPORT)
    QGLWidget *glWidget() const { return glw; }
#endif

public slots:
    void setGradientStops(const QGradientStops &stops);

    void setPadSpread() { m_spread = QGradient::PadSpread; update(); }
    void setRepeatSpread() { m_spread = QGradient::RepeatSpread; update(); }
    void setReflectSpread() { m_spread = QGradient::ReflectSpread; update(); }

    void setLinearGradient() { m_gradientType = Qt::LinearGradientPattern; update(); }
    void setRadialGradient() { m_gradientType = Qt::RadialGradientPattern; update(); }
    void setConicalGradient() { m_gradientType = Qt::ConicalGradientPattern; update(); }

#ifdef QT_OPENGL_SUPPORT
    void enableOpenGL(bool use_opengl)
    bool usesOpenGL() { return m_use_opengl;}
#endif

private:
#if defined(QT_OPENGL_SUPPORT)
    GLWidget *glw;
    bool m_use_opengl;
    bool m_use_opengl;
#endif
    QSize m_size;
    QPixmap m_tile;

#ifdef Q_WS_QWS
    static QPixmap *static_image;
#else
    static QImage *static_image;
#endif

    bool m_prefer_image;
    QGradientStops m_stops;
    HoverPoints *m_hoverPoints;

    QGradient::Spread m_spread;
    Qt::BrushStyle m_gradientType;
};

class GradientEditor : public QWidget
{
    Q_OBJECT
public:
    GradientEditor(QWidget *parent);

    void setGradientStops(const QGradientStops &stops);

public slots:
    void pointsUpdated();

signals:
    void gradientStopsChanged(const QGradientStops &stops);

private:
    ShadeWidget *m_red_shade;
    ShadeWidget *m_green_shade;
    ShadeWidget *m_blue_shade;
    ShadeWidget *m_alpha_shade;
};

class BackgroundGui;
class GradientDialog : public QDialog
{
    Q_OBJECT

public:
    GradientDialog(QSize bgSize, QGradient *bgGradient, QDialog *parent = 0);
    ~GradientDialog()
    {}
    QGradient getGradient();

public slots:
    void setDefault1() { setDefault(1); }
    void setDefault2() { setDefault(2); }
    void setDefault3() { setDefault(3); }
    void setDefault4() { setDefault(4); }
    void setGradient() { setGradient(m_gradient);}
    void accept();
    void cancel();

private:
    void setDefault(int i);
    void setGradient(QGradient *inbgGradient);

    QGradient  *m_gradient;
    GradientRenderer *m_renderer;
    GradientEditor   *m_editor;
    QRadioButton *m_linearButton;
    QRadioButton *m_radialButton;
    QRadioButton *m_conicalButton;
    QRadioButton *m_padSpreadButton;
    QRadioButton *m_reflectSpreadButton;
    QRadioButton *m_repeatSpreadButton;

};

#if defined(QT_OPENGL_SUPPORT)
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
#include <QOpenGLWidget>
#else
#include <QGLWidget>
#endif
#include <QEvent>
class GLWidget : public QGLWidget
{
public:
    GLWidget(QWidget *parent)
        : QGLWidget(QGLFormat(QGL::SampleBuffers), parent)
    {
        setAttribute(Qt::WA_AcceptTouchEvents);
    }
    void disableAutoBufferSwap() { setAutoBufferSwap(false); }
    void paintEvent(QPaintEvent *) { parentWidget()->update(); }
protected:
    bool event(QEvent *event)
    {
        switch (event->type()) {
        case QEvent::TouchBegin:
        case QEvent::TouchUpdate:
        case QEvent::TouchEnd:
            event->ignore();
            return false;
            break;
        default:
            break;
        }
        return QGLWidget::event(event);
    }
};
#endif

#endif // GRADIENTS_H
