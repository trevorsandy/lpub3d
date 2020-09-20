#include "lc_global.h"
#include "lc_scene.h"
#include "lc_context.h"
#include "pieceinf.h"
#include "lc_texture.h"
#include "lc_library.h"
#include "lc_application.h"
#include "object.h"

/*** LPub3D Mod - true fade ***/
enum lcFadeArgs {
	LC_NO_FADE,
	LC_DISABLE_COLOR_WRITES,
	LC_ENABLE_COLOR_WRITES,
	LC_DISABLE_BFC
};
/*** LPub3D Mod end ***/

lcScene::lcScene()
	: mRenderMeshes(0, 1024), mOpaqueMeshes(0, 1024), mTranslucentMeshes(0, 1024), mInterfaceObjects(0, 1024)
{
	mActiveSubmodelInstance = nullptr;
	mDrawInterface = false;
	mAllowWireframe = true;
	mAllowLOD = true;
	mPreTranslucentCallback = nullptr;
}

void lcScene::Begin(const lcMatrix44& ViewMatrix)
{
	mViewMatrix = ViewMatrix;
	mActiveSubmodelInstance = nullptr;
	mPreTranslucentCallback = nullptr;
	mRenderMeshes.RemoveAll();
	mOpaqueMeshes.RemoveAll();
	mTranslucentMeshes.RemoveAll();
	mInterfaceObjects.RemoveAll();
}

void lcScene::End()
{
	const auto OpaqueMeshCompare = [this](int Index1, int Index2)
	{
		const lcMesh* Mesh1 = mRenderMeshes[Index1].Mesh;
		const lcMesh* Mesh2 = mRenderMeshes[Index2].Mesh;

		const int Texture1 = Mesh1->mFlags & lcMeshFlag::HasTexture;
		const int Texture2 = Mesh2->mFlags & lcMeshFlag::HasTexture;

		if (Texture1 == Texture2)
			return Mesh1 < Mesh2;

		return Texture1 ? false : true;
	};

	std::sort(mOpaqueMeshes.begin(), mOpaqueMeshes.end(), OpaqueMeshCompare);

	auto TranslucentMeshCompare = [](const lcTranslucentMeshInstance& Mesh1, const lcTranslucentMeshInstance& Mesh2)
	{
		return Mesh1.Distance > Mesh2.Distance;
	};

	std::sort(mTranslucentMeshes.begin(), mTranslucentMeshes.end(), TranslucentMeshCompare);
}

void lcScene::AddMesh(lcMesh* Mesh, const lcMatrix44& WorldMatrix, int ColorIndex, lcRenderMeshState State)
{
	lcRenderMesh& RenderMesh = mRenderMeshes.Add();

	RenderMesh.WorldMatrix = WorldMatrix;
	RenderMesh.Mesh = Mesh;
	RenderMesh.ColorIndex = ColorIndex;
	RenderMesh.State = State;
	const float Distance = fabsf(lcMul31(WorldMatrix[3], mViewMatrix).z);
	RenderMesh.LodIndex = mAllowLOD ? RenderMesh.Mesh->GetLodIndex(Distance) : LC_MESH_LOD_HIGH;

	const bool Translucent = lcIsColorTranslucent(ColorIndex);
	const lcMeshFlags Flags = Mesh->mFlags;

	if ((Flags & (lcMeshFlag::HasSolid | lcMeshFlag::HasLines)) || ((Flags & lcMeshFlag::HasDefault) && !Translucent))
		mOpaqueMeshes.Add(mRenderMeshes.GetSize() - 1);

	if ((Flags & lcMeshFlag::HasTranslucent) || ((Flags & lcMeshFlag::HasDefault) && Translucent))
	{
		const lcMeshLod& Lod = Mesh->mLods[RenderMesh.LodIndex];

		for (int SectionIdx = 0; SectionIdx < Lod.NumSections; SectionIdx++)
		{
			const lcMeshSection* Section = &Lod.Sections[SectionIdx];

			if ((Section->PrimitiveType & (LC_MESH_TRIANGLES | LC_MESH_TEXTURED_TRIANGLES)) == 0)
				continue;

			int SectionColorIndex = Section->ColorIndex;

			if (SectionColorIndex == gDefaultColor)
				SectionColorIndex = RenderMesh.ColorIndex;

			if (!lcIsColorTranslucent(SectionColorIndex))
				continue;

			const lcVector3 Center = (Section->BoundingBox.Min + Section->BoundingBox.Max) / 2;
			const float InstanceDistance = fabsf(lcMul31(lcMul31(Center, WorldMatrix), mViewMatrix).z);

			lcTranslucentMeshInstance& Instance = mTranslucentMeshes.Add();
			Instance.Section = Section;
			Instance.Distance = InstanceDistance;
			Instance.RenderMeshIndex = mRenderMeshes.GetSize() - 1;
		}
	}
}

void lcScene::DrawDebugNormals(lcContext* Context, const lcMesh* Mesh) const
{
	const lcVertex* const VertexBuffer = (lcVertex*)Mesh->mVertexData;
	lcVector3* const Vertices = (lcVector3*)malloc(Mesh->mNumVertices * 2 * sizeof(lcVector3));

	for (int VertexIdx = 0; VertexIdx < Mesh->mNumVertices; VertexIdx++)
	{
		Vertices[VertexIdx * 2] = VertexBuffer[VertexIdx].Position;
		Vertices[VertexIdx * 2 + 1] = VertexBuffer[VertexIdx].Position + lcUnpackNormal(VertexBuffer[VertexIdx].Normal);
	}

	Context->SetVertexBufferPointer(Vertices);
	Context->SetVertexFormatPosition(3);
	Context->DrawPrimitives(GL_LINES, 0, Mesh->mNumVertices * 2);
	free(Vertices);
}

void lcScene::DrawOpaqueMeshes(lcContext* Context, bool DrawLit, int PrimitiveTypes) const
{
	if (mOpaqueMeshes.IsEmpty())
		return;

	lcMaterialType FlatMaterial, TexturedMaterial;

	if (DrawLit)
	{
		FlatMaterial = lcMaterialType::FakeLitColor;
		TexturedMaterial = lcMaterialType::FakeLitTextureDecal;
	}
	else
	{
		FlatMaterial = lcMaterialType::UnlitColor;
		TexturedMaterial = lcMaterialType::UnlitTextureDecal;
	}

	Context->SetPolygonOffset(lcPolygonOffset::Opaque);

	for (const int MeshIndex : mOpaqueMeshes)
	{
		const lcRenderMesh& RenderMesh = mRenderMeshes[MeshIndex];
		const lcMesh* Mesh = RenderMesh.Mesh;
		const int LodIndex = RenderMesh.LodIndex;

		Context->BindMesh(Mesh);
		Context->SetWorldMatrix(RenderMesh.WorldMatrix);

		for (int SectionIdx = 0; SectionIdx < Mesh->mLods[LodIndex].NumSections; SectionIdx++)
		{
			const lcMeshSection* const Section = &Mesh->mLods[LodIndex].Sections[SectionIdx];

			if ((Section->PrimitiveType & PrimitiveTypes) == 0)
				continue;

			int ColorIndex = Section->ColorIndex;

			if (Section->PrimitiveType & (LC_MESH_TRIANGLES | LC_MESH_TEXTURED_TRIANGLES))
			{
				if (ColorIndex == gDefaultColor)
					ColorIndex = RenderMesh.ColorIndex;

				if (lcIsColorTranslucent(size_t(ColorIndex)))   /*** LPub3D Mod - Suppress int -> size_t warning ***/
					continue;

				switch (RenderMesh.State)
				{
				case lcRenderMeshState::Default:
				case lcRenderMeshState::Highlighted:
					Context->SetColorIndex(ColorIndex);
					break;

				case lcRenderMeshState::Selected:
					Context->SetColorIndexTinted(ColorIndex, LC_COLOR_SELECTED, 0.5f);
					break;

				case lcRenderMeshState::Focused:
					Context->SetColorIndexTinted(ColorIndex, LC_COLOR_FOCUSED, 0.5f);
					break;

				case lcRenderMeshState::Faded:
					Context->SetColorIndexTinted(ColorIndex, LC_COLOR_DISABLED, 0.25f);
					break;
				}
			}
			else if (Section->PrimitiveType & LC_MESH_LINES)
			{
				switch (RenderMesh.State)
				{
				case lcRenderMeshState::Default:
					if (ColorIndex == gEdgeColor)
						Context->SetEdgeColorIndex(RenderMesh.ColorIndex);
					else
						Context->SetColorIndex(ColorIndex);
					break;

				case lcRenderMeshState::Selected:
					Context->SetInterfaceColor(LC_COLOR_SELECTED);
					break;

				case lcRenderMeshState::Focused:
					Context->SetInterfaceColor(LC_COLOR_FOCUSED);
					break;

				case lcRenderMeshState::Highlighted:
					Context->SetInterfaceColor(LC_COLOR_HIGHLIGHT);
					break;

				case lcRenderMeshState::Faded:
					Context->SetInterfaceColor(LC_COLOR_DISABLED);
					break;
				}
			}
			else if (Section->PrimitiveType == LC_MESH_CONDITIONAL_LINES)
			{
				const lcMatrix44 WorldViewProjectionMatrix = lcMul(RenderMesh.WorldMatrix, lcMul(mViewMatrix, Context->GetProjectionMatrix()));
				const lcVertex* const VertexBuffer = (lcVertex*)Mesh->mVertexData;
				const int IndexBufferOffset = Mesh->mIndexCacheOffset != -1 ? Mesh->mIndexCacheOffset : 0;

				const int VertexBufferOffset = Mesh->mVertexCacheOffset != -1 ? Mesh->mVertexCacheOffset : 0;
				Context->SetVertexFormat(VertexBufferOffset, 3, 1, 0, 0, DrawLit);

				if (Mesh->mIndexType == GL_UNSIGNED_SHORT)
				{
					const quint16* const Indices = (quint16*)((char*)Mesh->mIndexData + Section->IndexOffset);

					for (int i = 0; i < Section->NumIndices; i += 4)
					{
						const lcVector3 p1 = lcMul31(VertexBuffer[Indices[i + 0]].Position, WorldViewProjectionMatrix);
						const lcVector3 p2 = lcMul31(VertexBuffer[Indices[i + 1]].Position, WorldViewProjectionMatrix);
						const lcVector3 p3 = lcMul31(VertexBuffer[Indices[i + 2]].Position, WorldViewProjectionMatrix);
						const lcVector3 p4 = lcMul31(VertexBuffer[Indices[i + 3]].Position, WorldViewProjectionMatrix);

						if (((p1.y - p2.y) * (p3.x - p1.x) + (p2.x - p1.x) * (p3.y - p1.y)) * ((p1.y - p2.y) * (p4.x - p1.x) + (p2.x - p1.x) * (p4.y - p1.y)) >= 0)
							Context->DrawIndexedPrimitives(GL_LINES, 2, Mesh->mIndexType, IndexBufferOffset + Section->IndexOffset + i * sizeof(quint16));
					}
				}
				else
				{
					const quint32* const Indices = (quint32*)((char*)Mesh->mIndexData + Section->IndexOffset);

					for (int i = 0; i < Section->NumIndices; i += 4)
					{
						const lcVector3 p1 = lcMul31(VertexBuffer[Indices[i + 0]].Position, WorldViewProjectionMatrix);
						const lcVector3 p2 = lcMul31(VertexBuffer[Indices[i + 1]].Position, WorldViewProjectionMatrix);
						const lcVector3 p3 = lcMul31(VertexBuffer[Indices[i + 2]].Position, WorldViewProjectionMatrix);
						const lcVector3 p4 = lcMul31(VertexBuffer[Indices[i + 3]].Position, WorldViewProjectionMatrix);

						if (((p1.y - p2.y) * (p3.x - p1.x) + (p2.x - p1.x) * (p3.y - p1.y)) * ((p1.y - p2.y) * (p4.x - p1.x) + (p2.x - p1.x) * (p4.y - p1.y)) >= 0)
							Context->DrawIndexedPrimitives(GL_LINES, 2, Mesh->mIndexType, IndexBufferOffset + Section->IndexOffset + i * sizeof(quint32));
					}
				}

				continue;
			}

			const lcTexture* const Texture = Section->Texture;
			int VertexBufferOffset = Mesh->mVertexCacheOffset != -1 ? Mesh->mVertexCacheOffset : 0;
			const int IndexBufferOffset = Mesh->mIndexCacheOffset != -1 ? Mesh->mIndexCacheOffset : 0;

			if (!Texture)
			{
				Context->SetMaterial(FlatMaterial);
				Context->SetVertexFormat(VertexBufferOffset, 3, 1, 0, 0, DrawLit);
			}
			else
			{
				Context->SetMaterial(TexturedMaterial);
				VertexBufferOffset += Mesh->mNumVertices * sizeof(lcVertex);
				Context->SetVertexFormat(VertexBufferOffset, 3, 1, 2, 0, DrawLit);
				Context->BindTexture2D(Texture->mTexture);
			}

			const GLenum DrawPrimitiveType = Section->PrimitiveType & (LC_MESH_TRIANGLES | LC_MESH_TEXTURED_TRIANGLES) ? GL_TRIANGLES : GL_LINES;
			Context->DrawIndexedPrimitives(DrawPrimitiveType, Section->NumIndices, Mesh->mIndexType, IndexBufferOffset + Section->IndexOffset);
		}

#ifdef LC_DEBUG_NORMALS
		DrawDebugNormals(Context, Mesh);
#endif
	}

	Context->BindTexture2D(0);
	Context->SetPolygonOffset(lcPolygonOffset::None);
}

/*** LPub3D Mod - true fade ***/
void lcScene::DrawTranslucentMeshes(lcContext* Context, bool DrawLit, int FadeArgs) const
{
/*** LPub3D Mod end ***/
	if (mTranslucentMeshes.IsEmpty())
		return;

	lcMaterialType FlatMaterial, TexturedMaterial;

	if (DrawLit)
	{
		FlatMaterial = lcMaterialType::FakeLitColor;
		TexturedMaterial = lcMaterialType::FakeLitTextureDecal;
	}
	else
	{
		FlatMaterial = lcMaterialType::UnlitColor;
		TexturedMaterial = lcMaterialType::UnlitTextureDecal;
	}

/*** LPub3D Mod - true fade ***/
	if (FadeArgs){
		// Enable BFC
		glCullFace(GL_BACK);
		glEnable(GL_CULL_FACE);
		// Disable color writes
		if (FadeArgs == LC_DISABLE_COLOR_WRITES){
			glDisable(GL_BLEND);
			glColorMask(GL_FALSE,GL_FALSE,GL_FALSE,GL_FALSE);
		}
		// Enable color writes
		else {
			glEnable(GL_BLEND);
			glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);
		}
	} else {
		glEnable(GL_BLEND);
	}

	Context->SetDepthWrite(FadeArgs == LC_DISABLE_COLOR_WRITES);
/*** LPub3D Mod end ***/
	Context->SetPolygonOffset(lcPolygonOffset::Translucent);

	for (const lcTranslucentMeshInstance& MeshInstance : mTranslucentMeshes)
	{
		const lcRenderMesh& RenderMesh = mRenderMeshes[MeshInstance.RenderMeshIndex];
		const lcMesh* Mesh = RenderMesh.Mesh;

		Context->BindMesh(Mesh);
		Context->SetWorldMatrix(RenderMesh.WorldMatrix);

		const lcMeshSection* Section = MeshInstance.Section;

		int ColorIndex = Section->ColorIndex;

		if (ColorIndex == gDefaultColor)
			ColorIndex = RenderMesh.ColorIndex;

		switch (RenderMesh.State)
		{
		case lcRenderMeshState::Default:
		case lcRenderMeshState::Highlighted:
			Context->SetColorIndex(ColorIndex);
			break;

		case lcRenderMeshState::Selected:
			Context->SetColorIndexTinted(ColorIndex, LC_COLOR_SELECTED, 0.5f);
			break;

		case lcRenderMeshState::Focused:
			Context->SetColorIndexTinted(ColorIndex, LC_COLOR_FOCUSED, 0.5f);
			break;

		case lcRenderMeshState::Faded:
			Context->SetColorIndexTinted(ColorIndex, LC_COLOR_DISABLED, 0.25f);
			break;
		}

		const lcTexture* Texture = Section->Texture;
		int VertexBufferOffset = Mesh->mVertexCacheOffset != -1 ? Mesh->mVertexCacheOffset : 0;
		const int IndexBufferOffset = Mesh->mIndexCacheOffset != -1 ? Mesh->mIndexCacheOffset : 0;

		if (!Texture)
		{
			Context->SetMaterial(FlatMaterial);
			Context->SetVertexFormat(VertexBufferOffset, 3, 1, 0, 0, DrawLit);
		}
		else
		{
			Context->SetMaterial(TexturedMaterial);
			VertexBufferOffset += Mesh->mNumVertices * sizeof(lcVertex);
			Context->SetVertexFormat(VertexBufferOffset, 3, 1, 2, 0, DrawLit);
			Context->BindTexture2D(Texture->mTexture);
		}

		const GLenum DrawPrimitiveType = Section->PrimitiveType & (LC_MESH_TRIANGLES | LC_MESH_TEXTURED_TRIANGLES) ? GL_TRIANGLES : GL_LINES;
		Context->DrawIndexedPrimitives(DrawPrimitiveType, Section->NumIndices, Mesh->mIndexType, IndexBufferOffset + Section->IndexOffset);

#ifdef LC_DEBUG_NORMALS
		DrawDebugNormals(Context, Mesh);
#endif
	}

	Context->BindTexture2D(0);
	Context->SetPolygonOffset(lcPolygonOffset::None);

	Context->SetDepthWrite(true);
	glDisable(GL_BLEND);

/*** LPub3D Mod - true fade ***/
	if (FadeArgs)
	{
		// Wrap up, Disable BFC
		glDisable(GL_CULL_FACE);
	}
/*** LPub3D Mod end ***/
}

void lcScene::Draw(lcContext* Context) const
{
	// TODO: find a better place for these updates
	lcGetPiecesLibrary()->UpdateBuffers(Context);
	lcGetPiecesLibrary()->UploadTextures(Context);

	Context->SetViewMatrix(mViewMatrix);

	constexpr bool DrawConditional = false;
	const lcPreferences& Preferences = lcGetPreferences();
/*** LPub3D Mod - true fade ***/
	bool DoFade = gApplication->FadePreviousSteps() && !mTranslucentMeshes.IsEmpty();
/*** LPub3D Mod end ***/

	lcShadingMode ShadingMode = Preferences.mShadingMode;
	if (ShadingMode == lcShadingMode::Wireframe && !mAllowWireframe)
		ShadingMode = lcShadingMode::Flat;

	if (ShadingMode == lcShadingMode::Wireframe)
	{
		int PrimitiveTypes = LC_MESH_LINES;

		if (DrawConditional)
			PrimitiveTypes |= LC_MESH_CONDITIONAL_LINES;

		DrawOpaqueMeshes(Context, false, PrimitiveTypes);

		if (mPreTranslucentCallback)
			mPreTranslucentCallback();
	}
	else if (ShadingMode == lcShadingMode::Flat)
	{
		const bool DrawLines = Preferences.mDrawEdgeLines && Preferences.mLineWidth != 0.0f;

		int PrimitiveTypes = LC_MESH_TRIANGLES | LC_MESH_TEXTURED_TRIANGLES;

/*** LPub3D Mod - true fade ***/
		if (DoFade){          // Fade

			// 1. Draw opaque mesh triangles
			DrawOpaqueMeshes(Context, false, PrimitiveTypes);

			// 2. Disable color writes to only update the Z buffer
			DrawTranslucentMeshes(Context, true, LC_DISABLE_COLOR_WRITES);

		    if (mPreTranslucentCallback)
			     mPreTranslucentCallback();
			     
			// 3. Enable color writes to draw translucent mesh triangles normally
			DrawTranslucentMeshes(Context, true, LC_ENABLE_COLOR_WRITES);
			     
			// 4. Draw mesh lines
			if (DrawLines)
			{
				PrimitiveTypes |= LC_MESH_LINES;

				if (DrawConditional)
					PrimitiveTypes |= LC_MESH_CONDITIONAL_LINES;

				// 5. Draw opaque unlit mesh lines
				DrawOpaqueMeshes(Context, false, PrimitiveTypes);
			}

		} else {              // NoFade
			if (DrawLines)
			{
				PrimitiveTypes |= LC_MESH_LINES;

				if (DrawConditional)
					PrimitiveTypes |= LC_MESH_CONDITIONAL_LINES;
			}

			DrawOpaqueMeshes(Context, false, PrimitiveTypes);
			
		    if (mPreTranslucentCallback)
			     mPreTranslucentCallback();
			
			DrawTranslucentMeshes(Context, false);
		}
/*** LPub3D Mod end ***/

	}
	else
	{
		const bool DrawLines = Preferences.mDrawEdgeLines && Preferences.mLineWidth != 0.0f;

/*** LPub3D Mod - true fade ***/
		if (DoFade){          // Fade

			// 1. Draw opaque mesh triangles
			DrawOpaqueMeshes(Context, true, LC_MESH_TRIANGLES | LC_MESH_TEXTURED_TRIANGLES);
			
			// 2. Disable color writes to only update the Z buffer
			DrawTranslucentMeshes(Context, true, LC_DISABLE_COLOR_WRITES);

		    if (mPreTranslucentCallback)
			    mPreTranslucentCallback();
			    
			// 3. Enable color writes to draw translucent mesh triangles normally
			DrawTranslucentMeshes(Context, true, LC_ENABLE_COLOR_WRITES);
			    
			// 4. Draw mesh lines
			if (DrawLines)
			{
				int PrimitiveTypes = LC_MESH_LINES;

				if (DrawConditional)
					PrimitiveTypes |= LC_MESH_CONDITIONAL_LINES;

				// 5. Draw translucent unlit mesh lines
				DrawTranslucentMeshes(Context, false, LC_DISABLE_BFC);

				// 6. Draw opaque unlit mesh lines
				DrawOpaqueMeshes(Context, false, PrimitiveTypes);
			}

		} else {              // NoFade
			if (DrawLines)
			{
				int PrimitiveTypes = LC_MESH_LINES;

				if (DrawConditional)
					PrimitiveTypes |= LC_MESH_CONDITIONAL_LINES;

				DrawOpaqueMeshes(Context, false, PrimitiveTypes);
			}

			DrawOpaqueMeshes(Context, true, LC_MESH_TRIANGLES | LC_MESH_TEXTURED_TRIANGLES);
			
		    if (mPreTranslucentCallback)
			    mPreTranslucentCallback();
			    
			DrawTranslucentMeshes(Context, true);
		}
/*** LPub3D Mod end ***/
	}
}

void lcScene::DrawInterfaceObjects(lcContext* Context) const
{
	for (const lcObject* Object : mInterfaceObjects)
		Object->DrawInterface(Context, *this);
}
