/****************************************************************************
**
** Copyright (C) 2020 Trevor SANDY. All rights reserved.
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
 * Piece preview is used to display the selected piece.
 *
 * Please see lpub.h for an overall description of how the files in LPub
 * make up the LPub program.
 *
 ***************************************************************************/

//#include "lc_global.h" // add on header, triggering circular reference build break here
#include "piecepreview.h"
#include "pieceinf.h"
#include "project.h"
#include "lc_model.h"
#include "lc_library.h"
#include "lc_scene.h"

#include "lpubalert.h"

PiecePreview::PiecePreview()
{
    m_PieceInfo = NULL;
    m_RotateX   = 60.0f;
    m_RotateZ   = 225.0f;
    m_Distance  = 10.0f;
    m_AutoZoom  = true;
    m_Tracking  = LC_TRACK_NONE;
}

PiecePreview::~PiecePreview()
{
    if (m_PieceInfo) {
        m_PieceInfo->Release();
    }
}

PieceInfo* PiecePreview::GetCurrentPiece() const
{
    return m_PieceInfo;
}

void PiecePreview::OnDraw()
{
    if (m_PieceInfo == NULL)
        return;

    mContext->SetDefaultState();

    float aspect = (float)mWidth/(float)mHeight;

    mContext->SetViewport(0, 0, mWidth, mHeight);

    lcGetActiveModel()->DrawBackground(this);

    const lcBoundingBox& BoundingBox = m_PieceInfo->GetBoundingBox();

    lcVector3 Center = (BoundingBox.Min + BoundingBox.Max) / 2.0f;

    lcVector3 Eye(0.0f, 0.0f, 1.0f);

    Eye = lcMul30(Eye, lcMatrix44RotationX(-m_RotateX * LC_DTOR));
    Eye = lcMul30(Eye, lcMatrix44RotationZ(-m_RotateZ * LC_DTOR));

    lcMatrix44 ProjectionMatrix = lcMatrix44Perspective(30.0f, aspect, 1.0f, 2500.0f);

    mContext->SetProjectionMatrix(ProjectionMatrix);

    lcMatrix44 ViewMatrix;

    if (m_AutoZoom)
    {
        lcVector3 Points[8];
        lcGetBoxCorners(BoundingBox.Min, BoundingBox.Max, Points);

        Eye += Center;

        const lcMatrix44 ModelView = lcMatrix44LookAt(Eye, Center, lcVector3(0, 0, 1));
        std::tie(Eye, std::ignore) = lcZoomExtents(Eye, ModelView, ProjectionMatrix, Points, 8);

        ViewMatrix = lcMatrix44LookAt(Eye, Center, lcVector3(0, 0, 1));

        // Update the new camera distance.
        const lcVector3 d = Eye - Center;

        m_Distance = d.Length();
    }
    else
    {
        ViewMatrix = lcMatrix44LookAt(Eye * m_Distance, Center, lcVector3(0, 0, 1));
    }

    lcScene Scene;
    Scene.Begin(ViewMatrix);
    Scene.SetAllowLOD(false);

    m_PieceInfo->AddRenderMeshes(Scene, lcMatrix44Identity(), m_ColorIndex, lcRenderMeshState::Focused, false);

    Scene.End();

    Scene.Draw(mContext);

    mContext->ClearResources();
}

void PiecePreview::SetPiece(PieceInfo *pInfo, quint32 pColorCode)
{
    MakeCurrent();

    PieceInfo* Info = m_PieceInfo;

    if (pColorCode == NULL)
        m_ColorIndex = gDefaultColor;
    else
        m_ColorIndex = lcGetColorIndex(pColorCode);

    if (Info != NULL)
        Info->Release();

    Info = pInfo;

    if (Info != NULL)
    {
        Info->AddRef();
        m_PieceInfo = Info;
        emit lpubAlert->messageSig(LOG_DEBUG,
                                   QString("Preview PartType: %1, ColorCode: %2, ColorIndex: %3")
                                   .arg(Info->mFileName).arg(pColorCode).arg(m_ColorIndex));
        Redraw();
    }
}

void PiecePreview::SetCurrentPiece(const QString &PieceID, int ColorCode)
{
    lcPiecesLibrary* Library = lcGetPiecesLibrary();
    PieceInfo* Info = Library->FindPiece(PieceID.toLatin1().constData(), NULL, false, false);

    if (!Info)
        Info = Library->mPieces[0];

    if (Info)
        SetPiece(Info, ColorCode);
}

void PiecePreview::OnLeftButtonDown()
{
    if (m_Tracking == LC_TRACK_NONE)
    {
        m_DownX = mInputState.x;
        m_DownY = mInputState.y;
        m_Tracking = LC_TRACK_LEFT;
    }
}

void PiecePreview::OnLeftButtonUp()
{
    if (m_Tracking == LC_TRACK_LEFT)
        m_Tracking = LC_TRACK_NONE;
}

void PiecePreview::OnLeftButtonDoubleClick()
{
    m_AutoZoom = true;
    Redraw();
}

void PiecePreview::OnRightButtonDown()
{
    if (m_Tracking == LC_TRACK_NONE)
    {
        m_DownX = mInputState.x;
        m_DownY = mInputState.y;
        m_Tracking = LC_TRACK_RIGHT;
    }
}

void PiecePreview::OnRightButtonUp()
{
    if (m_Tracking == LC_TRACK_RIGHT)
        m_Tracking = LC_TRACK_NONE;
}

void PiecePreview::OnMouseMove()
{
    if (m_Tracking == LC_TRACK_LEFT)
    {
        // Rotate.
        m_RotateZ += mInputState.x - m_DownX;
        m_RotateX += mInputState.y - m_DownY;

        if (m_RotateX > 179.5f)
            m_RotateX = 179.5f;
        else if (m_RotateX < 0.5f)
            m_RotateX = 0.5f;

        m_DownX = mInputState.x;
        m_DownY = mInputState.y;

        Redraw();
    }
    else if (m_Tracking == LC_TRACK_RIGHT)
    {
        // Zoom.
        m_Distance += (float)(m_DownY - mInputState.y) * 0.2f;
        m_AutoZoom = false;

        if (m_Distance < 0.5f)
            m_Distance = 0.5f;

        m_DownX = mInputState.x;
        m_DownY = mInputState.y;

        Redraw();
    }
}
