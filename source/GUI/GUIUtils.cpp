#include "PlatformPrecomp.h"
#include "Entity/EntityUtils.h"
#include "GUIUtils.h"

Entity* AddRectAroundEntity(Entity* pParentEnt, uint32 color, uint32 borderColor, float rectWidth, bool contentUp, float fontScale, eFont font)
{
	CL_Vec2f parentEntityPos = GetPos2DEntity(pParentEnt);
	CL_Vec2f parentEntitySize = GetSize2DEntity(pParentEnt);
	BaseApp* BaseApp = GetBaseApp();

	float LineHeight = 0;
	if (BaseApp->GetFont(font)->GetLineHeight(fontScale) >= 0)
	{
		LineHeight = BaseApp->GetFont(font)->GetLineHeight(fontScale);
	}

	eAlignment AligmentEntity = GetAlignmentEntity(pParentEnt);
	CL_Vec2f AligmentOffset = GetAlignmentOffset(parentEntitySize, AligmentEntity);

	CL_Vec2f pos;
	pos.x = parentEntityPos.x - AligmentOffset.x - rectWidth;
	pos.y = parentEntityPos.y - AligmentOffset.y - (rectWidth * 0.6f);
	CL_Vec2f bounds;
	bounds.x += rectWidth + rectWidth + parentEntitySize.x;
	bounds.y += LineHeight + ((rectWidth * 0.6f) * 3);
	Entity* pRect = CreateOverlayRectEntity(pParentEnt->GetParent(), pos, bounds, color);

	EntityComponent* pRenderComp = pRect->GetComponentByName("RectRender");
	Variant* pBorderFile = pRenderComp->GetVar("bmpBorderFileName");
	if (contentUp) pBorderFile->Set("interface/gui_box_upwhite.rttex");
	else pBorderFile->Set("interface/gui_box_white.rttex");
	pRenderComp->GetVar("borderColor")->Set(borderColor);

	pRect->GetParent()->MoveEntityToBottomByAddress(pRect);
	return pRect;
}