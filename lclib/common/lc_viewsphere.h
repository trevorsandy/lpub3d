#pragma once

#include "lc_math.h"
#include "lc_context.h"
#include <bitset>

class View;
/*** LPub3D Mod - preview widget ***/
class PreviewWidget;
/*** LPub3D Mod end ***/

class lcViewSphere
{
public:
	lcViewSphere(View *View);
/*** LPub3D Mod - preview widget ***/
	lcViewSphere(PreviewWidget* Preview, bool subPreview);
/*** LPub3D Mod end ***/

	void Draw();
	bool OnMouseMove();
	bool OnLeftButtonUp();
	bool OnLeftButtonDown();
	bool IsDragging() const;

	static void CreateResources(lcContext* Context);
	static void DestroyResources(lcContext* Context);

protected:
	lcMatrix44 GetViewMatrix() const;
	lcMatrix44 GetProjectionMatrix() const;
	std::bitset<6> GetIntersectionFlags(lcVector3& Intersection) const;

/*** LPub3D Mod - preview widget ***/
	PreviewWidget* mPreview;
/*** LPub3D Mod end ***/
	View* mView;   
	lcVector3 mIntersection;
	std::bitset<6> mIntersectionFlags;
/*** LPub3D Mod - preview widget ***/
	int mViewSphereSize;
/*** LPub3D Mod end ***/
	int mMouseDownX;
	int mMouseDownY;
	bool mMouseDown;
	bool mIsPreview;

	static lcTexture* mTexture;
	static lcVertexBuffer mVertexBuffer;
	static lcIndexBuffer mIndexBuffer;
	static const float mRadius;
	static const float mHighlightRadius;
	static const int mSubdivisions;
};
