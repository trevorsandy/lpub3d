/****************************************************************************
**
** Copyright (C) 2015 Trevor SANDY. All rights reserved.
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

/****************************************************************************
**
** Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
**     of its contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtGui>
#include <QtOpenGL>

#include <math.h>

#include "glwidget.h"

#include "lc_global.h"
#include "lc_model.h"
#include "lc_context.h"
#include "pieceinf.h"
#include "texfont.h"
#include "lc_texture.h"
#include "threedwindow.h"
#include "lc_library.h"
//#include "lc_qglwidget.h"
//#include "lc_glwidget.h"
//#include "view.h"
//#include "lc_application.h"

#ifndef GL_MULTISAMPLE
#define GL_MULTISAMPLE  0x809D
#endif

GLWidget::GLWidget(QWidget *parent)
    : QGLWidget(QGLFormat(QGL::SampleBuffers), parent)
{    
    xRot = 0;
    yRot = 0;
    zRot = 0;

    mContext = new lcContext();                 // lc_glwidget
    mDeleteContext = true;                      // lc_glwidget

    m_PieceInfo = NULL;                         // preview
    m_RotateX = 60.0f;                          // preview
    m_RotateZ = 225.0f;                         // preview
    m_Distance = 10.0f;                         // preview
    m_AutoZoom = true;                          // preview

    mUpdateTimer.setSingleShot(true);           // lc_qglwidget
    connect(&mUpdateTimer, SIGNAL(timeout()), this, SLOT(updateGL()));  // lc_qglwidget

    makeCurrent();                              // lc_qglwidget
//    GL_InitializeSharedExtensions(this);        // lc_qglwidget

    gTexFont.Load();
    if (!gGridTexture)
    {
        gGridTexture = new lcTexture;
        gGridTexture->CreateGridTexture();
    }
    gGridTexture->AddRef();

    preferredSize = QSize(0, 0);

    qtPurple = QColor::fromCmykF(0.39, 0.39, 0.0, 0.0);
}

GLWidget::~GLWidget()
{
    gTexFont.Release();
    if (!gGridTexture->Release())
    {
        delete gGridTexture;
        gGridTexture = NULL;
    }
    if (mDeleteContext)
        delete mContext;
}

QSize GLWidget::minimumSizeHint() const
{
    return QSize(50, 50);
}

QSize GLWidget::sizeHint() const
{
    if (preferredSize.isEmpty())                // lc_qglwidget
        return QGLWidget::sizeHint();
    else
        return preferredSize;
}


static void qNormalizeAngle(int &angle)
{
    while (angle < 0)
        angle += 360 * 16;
    while (angle > 360 * 16)
        angle -= 360 * 16;
}

void GLWidget::setXRotation(int angle)
{
    qNormalizeAngle(angle);
    if (angle != xRot) {
        xRot = angle;
        emit xRotationChanged(angle);
        updateGL();
    }
}

void GLWidget::setYRotation(int angle)
{
    qNormalizeAngle(angle);
    if (angle != yRot) {
        yRot = angle;
        emit yRotationChanged(angle);
        updateGL();
    }
}

void GLWidget::setZRotation(int angle)
{
    qNormalizeAngle(angle);
    if (angle != zRot) {
        zRot = angle;
        emit zRotationChanged(angle);
        updateGL();
    }
}

void GLWidget::initializeGL()
{
 //   qglClearColor(qtPurple.dark());

    glEnable(GL_POLYGON_OFFSET_FILL);           // lc_qglwidget
    glPolygonOffset(0.5f, 0.1f);                // lc_qglwidget
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);                     // lc_qglwidget
    glDepthMask(GL_TRUE);                       // lc_qglwidget

//    glEnable(GL_CULL_FACE);
//    glShadeModel(GL_SMOOTH);
//    glEnable(GL_LIGHTING);
//    glEnable(GL_LIGHT0);
//    glEnable(GL_MULTISAMPLE);
//    static GLfloat lightPosition[4] = { 0.5, 5.0, 7.0, 1.0 };
//    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
}

void GLWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    glTranslatef(0.0, 0.0, -10.0);
    glRotatef(xRot / 16.0, 1.0, 0.0, 0.0);
    glRotatef(yRot / 16.0, 0.0, 1.0, 0.0);
    glRotatef(zRot / 16.0, 0.0, 0.0, 1.0);
    OnDraw();
}

void GLWidget::resizeGL(int width, int height)
{

    mWidth = width;                             // lc_qglwidget
    mHeight = height;                           // lc_qglwidget

//    int side = qMin(width, height);
//    glViewport((width - side) / 2, (height - side) / 2, side, side);

//    glMatrixMode(GL_PROJECTION);
//    glLoadIdentity();
//#ifdef QT_OPENGL_ES_1
//    glOrthof(-0.5, +0.5, -0.5, +0.5, 4.0, 15.0);
//#else
//    glOrtho(-0.5, +0.5, -0.5, +0.5, 4.0, 15.0);
//#endif
//    glMatrixMode(GL_MODELVIEW);
}

void GLWidget::mousePressEvent(QMouseEvent *event)
{
    lastPos = event->pos();
}

void GLWidget::mouseMoveEvent(QMouseEvent *event)
{
    int dx = event->x() - lastPos.x();
    int dy = event->y() - lastPos.y();

    if (event->buttons() & Qt::LeftButton) {
        setXRotation(xRot + 8 * dy);
        setYRotation(yRot + 8 * dx);
    } else if (event->buttons() & Qt::RightButton) {
        setXRotation(xRot + 8 * dy);
        setZRotation(zRot + 8 * dx);
    }
    lastPos = event->pos();
}

void GLWidget::wheelEvent(QWheelEvent *event)   // new
{
    // Zoom.
//    e->delta() > 0 ? scale += scale*0.1f : scale -= scale*0.1f;
//    m_Distance += (float)(m_DownY - mInputState.y) * 0.2f;

    int dy = event->y() - lastPos.y();

    if (event->delta() > 0)
        m_Distance += (float)(dy) * 0.2f;

    m_AutoZoom = false;

    if (m_Distance < 0.5f)
        m_Distance = 0.5f;

    Redraw();
}

void GLWidget::Redraw()
{
    mUpdateTimer.start(0);
}

void GLWidget::OnDraw()                         // preview
{
    if (m_PieceInfo == NULL)
        return;

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(0.5f, 0.1f);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    float aspect = (float)mWidth/(float)mHeight;
    mContext->SetViewport(0, 0, mWidth, mHeight);
    glEnableClientState(GL_VERTEX_ARRAY);

    lcGetActiveModel()->DrawBackground(mContext);

    lcVector3 Eye(0.0f, 0.0f, 1.0f);

    Eye = lcMul30(Eye, lcMatrix44RotationX(-m_RotateX * LC_DTOR));
    Eye = lcMul30(Eye, lcMatrix44RotationZ(-m_RotateZ * LC_DTOR));

    if (m_AutoZoom)
    {
        Eye = Eye * 100.0f;
        m_PieceInfo->ZoomExtents(30.0f, aspect, Eye);

        // Update the new camera distance.
        lcVector3 d = Eye - m_PieceInfo->GetCenter();
        m_Distance = d.Length();
    }
    else
    {
        glMatrixMode(GL_PROJECTION);
        glLoadMatrixf(lcMatrix44Perspective(30.0f, aspect, 1.0f, 2500.0f));
        glMatrixMode(GL_MODELVIEW);
        glLoadMatrixf(lcMatrix44LookAt(Eye * m_Distance, m_PieceInfo->GetCenter(), lcVector3(0, 0, 1)));
    }

    m_PieceInfo->RenderPiece(gui->mColorIndex);

    glDisableClientState(GL_VERTEX_ARRAY);
}

void* GLWidget::GetExtensionAddress(const char* FunctionName)         // lc_qglwidget
{
    QGLWidget* Widget = (QGLWidget*)mWidget;

    return (void*)Widget->context()->getProcAddress(FunctionName);
}

void GLWidget::SetCurrentPiece(PieceInfo *pInfo)                     // preview
{
    makeCurrent();

    if (m_PieceInfo != NULL)
        m_PieceInfo->Release();

    m_PieceInfo = pInfo;

    if (m_PieceInfo != NULL)
    {
        m_PieceInfo->AddRef();
        Redraw();
    }
}
