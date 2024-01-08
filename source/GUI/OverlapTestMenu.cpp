#include "PlatformPrecomp.h"
#include "OverlapTestMenu.h"
#include "MainMenu.h"
#include "Entity/EntityUtils.h"
#include "GUIUtils.h"

void OverlapTestMenuOnSelect(VariantList* pVList)
{
	Entity* pEntClicked = pVList->Get(1).GetEntity();
	LogMsg("Clicked %s entity at %s", pEntClicked->GetName().c_str(), pVList->m_variant[1].Print().c_str());
	Entity* pMenu = GetEntityRoot()->GetEntityByName("OverlapTestMenu");

	if (pEntClicked->GetName() == "Up" || pEntClicked->GetName() == "Down")
	{
		bool up = pEntClicked->GetName() == "Up"; //means Y goes down
		int32* curOverlap = &pMenu->GetVar("curOverlap")->GetINT32();
		Entity* pShelf = pMenu->GetEntityByName("ShelfDown");
		Entity* pShelfUp = pMenu->GetEntityByName("ShelfUp");

		//limiting it to not go too far up/down
		if (!up && *curOverlap <= 0) return;
		float upLimit = GetPos2DEntity(pShelfUp).y - GetSize2DEntity(pShelfUp).y;
		if (up && GetPos2DEntity(pShelf).y < upLimit) return;

		if (up) (*curOverlap)++;
		else (*curOverlap)--;
		SetTextEntityByName("Overlap", std::to_string(*curOverlap), pMenu);
		float shelfScale = GetScale2DEntity(pShelf).y;
		CL_Vec2f curPos = GetPos2DEntity(pShelf);
		if (up) curPos.y -= shelfScale;
		else curPos.y += shelfScale;
		SetPos2DEntity(pShelf, curPos);
		return;
	}
	if (pEntClicked->GetName() == "Back")
	{
		DisableAllButtonsEntity(pMenu);
		SlideScreen(pEntClicked->GetParent(), false);
		GetMessageManager()->CallEntityFunction(pEntClicked->GetParent(), 500, "OnDelete", NULL);
		MainMenuCreate(pEntClicked->GetParent()->GetParent());
		return;
	}

	GetEntityRoot()->PrintTreeAsText();
}

Entity* OverlapTestMenuCreate(Entity* pParentEnt)
{
	Entity* pBG = CreateOverlayRectEntity(pParentEnt, CL_Vec2f(0, 0), GetScreenSize(), MAKE_RGBA(128, 128, 128, 255));
	pBG->SetName("OverlapTestMenu");
	AddFocusIfNeeded(pBG, true, 500);
	int32 curOverlap = GET_THEMEMGR->GetAllowedShelvesOverlap();
	pBG->GetVar("curOverlap")->Set(curOverlap);

	//adding 2 shelves
	Entity* pShelfUp = CreateOverlayEntity(pBG, "ShelfUp", GET_THEMEMGR->GetFilename("game/shelf.rttex"), 0, GetScreenSizeYf() / 2);
	SetAlignmentEntity(pShelfUp, ALIGNMENT_DOWN_LEFT);
	EntitySetScaleBySize(pShelfUp, CL_Vec2f(GetScreenSizeXf() / 2, 0), true, true);
	float shelfScale = GetScale2DEntity(pShelfUp).y;
	Entity* pShelfDown = CreateOverlayEntity(pBG, "ShelfDown", GET_THEMEMGR->GetFilename("game/shelf.rttex"), 0, GetScreenSizeYf() / 2 - (curOverlap * shelfScale));
	SetAlignmentEntity(pShelfDown, ALIGNMENT_UPPER_LEFT);
	SetScale2DEntity(pShelfDown, CL_Vec2f(shelfScale, shelfScale));

	eFont font;
	float fontScale;
	GetFontAndScaleToFitThisLinesPerScreenY(&font, &fontScale, 13);

	Entity* pTextCurrent = CreateTextLabelEntity(pBG, "Overlap", GetScreenSizeXf() * 0.75f, GetScreenSizeYf() / 2, std::to_string(curOverlap));
	SetAlignmentEntity(pTextCurrent, ALIGNMENT_CENTER);
	SetupTextEntity(pTextCurrent, font, fontScale);

	float textHeight = GetSize2DEntity(pTextCurrent).y;
	Entity* pButtonUp = CreateTextButtonEntity(pBG, "Up", GetScreenSizeXf() * 0.75f, GetScreenSizeYf() / 2 - textHeight - iPhoneMapY(5), "Up");
	SetAlignmentEntity(pButtonUp, ALIGNMENT_CENTER);
	pButtonUp->GetFunction("OnButtonSelected")->sig_function.connect(&OverlapTestMenuOnSelect);
	pButtonUp->GetComponentByName("Button2D")->GetVar("repeatDelayMS")->Set(uint32(0));
	SetupTextEntity(pButtonUp, font, fontScale);
	AddHotKeyToButton(pButtonUp, VIRTUAL_KEY_DIR_UP);
	Entity* pButtonDown = CreateTextButtonEntity(pBG, "Down", GetScreenSizeXf() * 0.75f, GetScreenSizeYf() / 2 + textHeight + iPhoneMapY(5), "Down");
	SetAlignmentEntity(pButtonDown, ALIGNMENT_CENTER);
	pButtonDown->GetFunction("OnButtonSelected")->sig_function.connect(&OverlapTestMenuOnSelect);
	pButtonDown->GetComponentByName("Button2D")->GetVar("repeatDelayMS")->Set(uint32(0));
	SetupTextEntity(pButtonDown, font, fontScale);
	AddHotKeyToButton(pButtonDown, VIRTUAL_KEY_DIR_DOWN);

	Entity* pEnt = CreateTextButtonEntity(pBG, "Back", GetScreenSizeXf() / 2, GetScreenSizeYf() - iPhoneMapY(40), GET_LOCTEXT("{BACK}"), false);
	SetAlignmentEntity(pEnt, ALIGNMENT_UPPER_CENTER);
	pEnt->GetFunction("OnButtonSelected")->sig_function.connect(&OverlapTestMenuOnSelect);
	SetupTextEntity(pEnt, font, fontScale);
	pEnt->GetVar("color")->Set(GET_THEMEMGR->GetTextColor());
	Entity* pEntRect = AddRectAroundEntity(pEnt, GET_THEMEMGR->GetPrimaryColor(), GET_THEMEMGR->GetSecondaryColor(), iPadMapY(5), true, fontScale, font);
	CL_Vec2f rectPos = GetPos2DEntity(pEnt);
	SetPos2DEntity(pEntRect, rectPos);
	SetAlignmentEntity(pEntRect, ALIGNMENT_UPPER_CENTER);
	SetTextShadowColor(pEnt, GET_THEMEMGR->GetTextShadowColor());
	AddHotKeyToButton(pEnt, VIRTUAL_KEY_BACK);

	SlideScreen(pBG, true, 500);
	return pBG;
}