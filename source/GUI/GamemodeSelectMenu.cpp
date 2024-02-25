#include "PlatformPrecomp.h"
#include "Entity/EntityUtils.h"
#include "GUIUtils.h"
#include "GamemodeSelectMenu.h"
#include "MainMenu.h"

void GamemodeSelectMenuAddScrollContent(Entity* pParent);

void GamemodeSelectMenuOnSelect(VariantList* pVList)
{
	Entity* pEntClicked = pVList->Get(1).GetEntity();
	Entity* pMenu = GetEntityRoot()->GetEntityByName("PopUpMenu");
	if (!pMenu) return;

	string name = pEntClicked->GetName();

	if (name.substr(0, 2) == "gm")
	{
		if (pEntClicked->GetVar("checked")->GetUINT32())
		{
			//uncheck all other modes
			for (auto& e : *pEntClicked->GetParent()->GetChildren())
			{
				if (e->GetName().substr(0, 2) != "gm") continue;
				if (!e->GetVar("checked")->GetUINT32()) continue;
				if (e->GetName() == name) continue;
				SetCheckBoxChecked(e, false);
				BobEntityStop(e); //prevent bobbing
			}
			GetApp()->GetVar("gamemode")->Set(pEntClicked->GetVar("gamemode")->GetUINT32());
		}
		else
		{
			//check normal mode back on
			Entity* pNormal = pEntClicked->GetParent()->GetEntityByName("gm0");
			SetCheckBoxChecked(pNormal, true);
			GetApp()->GetVar("gamemode")->Set(uint32(GAMEMODE_NORMAL));
		}
		return;
	}
	if (name.substr(0, 7) == "_textgm")
	{
		//show about text
		Entity* pScroll = pMenu->GetEntityByName("scroll");
		CL_Rectf& scrollBounds = pScroll->GetComponentByName("Scroll")->GetVar("boundsRect")->GetRect();
		//i want to keep scroll pos after rebuilding menu
		CL_Vec2f progress = GetScrollProgressEntity(pScroll);
		progress *= scrollBounds.get_size_vec2();

		pScroll->GetEntityByName("scroll_child")->GetVarWithDefault("text" + name.substr(7), uint32(false))->GetUINT32() ^= 1;
		GamemodeSelectMenuAddScrollContent(pScroll);

		if (scrollBounds.get_width() != 0) progress.x /= scrollBounds.get_width();
		if (scrollBounds.get_height() != 0) progress.y /= scrollBounds.get_height();
		SetScrollProgressEntity(pScroll, progress);
		return;
	}
	if (name == "Cancel")
	{
		Entity* pDarken = GetEntityRoot()->GetEntityByName("pop_up_darken");
		if (!pDarken) return;

		FadeOutEntity(pMenu);
		GetMessageManager()->CallEntityFunction(pMenu, 500, "OnDelete");
		DisableAllButtonsEntity(pMenu);
		FadeScreen(pDarken, 0, 0, 400, true);
		KillEntity(pDarken, 400);
		pDarken->SetName("");

		Entity* pMainMenu = GetEntityRoot()->GetEntityByName("MainMenu");
		EnableAllButtonsEntity(pMainMenu);
		EnableComponentByName(pMainMenu, "CustomInput");
		return;
	}
}

void GamemodeSelectMenuAddMode(Entity* pParent, float x, float& y, eGameMode gamemode)
{
	float spacerY = iPhoneMapY(2);
	eFont font;
	float scale;
	GetFontAndScaleToFitThisLinesPerScreenY(&font, &scale, 23);

	Entity* pCheckbox = CreateCheckbox(pParent, "gm" + to_string(gamemode), GET_LOCTEXT("{GAMEMODESELECTMENU_GM" + to_string(gamemode) + "}"), x, y, GetApp()->GetGameMode() == gamemode, font, scale);
	Entity* pText = pCheckbox->GetEntityByName("_textgm" + to_string(gamemode));
	pText->GetVar("color")->Set(GET_THEMEMGR->GetTextColor());
	pText->AddComponent(new TouchHandlerComponent);
	pText->AddComponent(new Button2DComponent);
	pText->AddComponent(new UnderlineRenderComponent);
	pText->GetFunction("OnButtonSelected")->sig_function.connect(GamemodeSelectMenuOnSelect);
	pCheckbox->GetFunction("OnButtonSelected")->sig_function.connect(GamemodeSelectMenuOnSelect);
	pCheckbox->GetVar("gamemode")->Set(uint32(gamemode));
	y += MeasureEntityAndChildren(pCheckbox).get_height();
	if (pParent->GetVarWithDefault("text" + to_string(gamemode), uint32(false))->GetUINT32())
	{
		CL_Vec2f aboutPos;
		aboutPos.x = x + GetPos2DEntity(pText).x;
		aboutPos.y = y;
		CL_Vec2f aboutSize;
		aboutSize.x = GetSize2DEntity(pParent->GetParent()).x - aboutPos.x;

		Entity* pAbout = CreateTextBoxEntity(pParent, "aboutGM" + to_string(gamemode), aboutPos, aboutSize, GET_LOCTEXT("{GAMEMODESELECTMENU_ABOUT" + to_string(gamemode) + "}"));
		SetupTextEntity(pAbout, font, scale);

		y += GetSize2DEntity(pAbout).y;
		spacerY = iPhoneMapY(10);
	}
	y += spacerY;
}

void GamemodeSelectMenuAddScrollContent(Entity* pParent)
{
	pParent = pParent->GetEntityByName("scroll_child");
	pParent->RemoveAllEntities();

	float x = 5;
	float y = 0;

	GamemodeSelectMenuAddMode(pParent, x, y, GAMEMODE_NORMAL);
	GamemodeSelectMenuAddMode(pParent, x, y, GAMEMODE_MATH);
	if (FileExists("game/pos/" + GetApp()->GetVar("language")->GetString() + ".json"))
	{
		GamemodeSelectMenuAddMode(pParent, x, y, GAMEMODE_PoS);
	}

	VariantList vList(pParent->GetParent());
	ResizeScrollBounds(&vList);
}

Entity* GamemodeSelectMenuCreate(Entity* pParentEnt)
{
	if (GetEntityRoot()->GetEntityByName("pop_up_darken")) return 0; //We won't create pop up on a pop up

	Entity* pDarken = pParentEnt->AddEntity(new Entity("pop_up_darken"));
	FadeScreen(pDarken, 0, 0.7f, 400, false);
	pDarken->AddComponent(new TouchHandlerComponent);
	CL_Vec2f ScreenSize = GetScreenSize();

	CL_Vec2f size = { ScreenSize.x * 0.6f, ScreenSize.y * 0.6f };
	if (!IsTabletSize()) size = { ScreenSize.x * 0.8f, ScreenSize.y * 0.8f };
	CL_Vec2f pos = { 0.0f, 0.0f };

	Entity* OverlayRectEntity = CreateOverlayRectEntity(pParentEnt, pos, size, ColorCombine(GET_THEMEMGR->GetPrimaryColor(), MAKE_RGBA(0xF0, 0xF0, 0xF0, 0xA0)));
	OverlayRectEntity->GetComponentByName("RectRender")->GetVar("bmpBorderFileName")->Set(GET_THEMEMGR->GetFilename("interface/gui_box_white.rttex"));
	OverlayRectEntity->GetComponentByName("RectRender")->GetVar("borderColor")->Set(GET_THEMEMGR->GetPrimaryColor());
	OverlayRectEntity->GetVar("finishMenuName")->Set(pParentEnt->GetName());
	OverlayRectEntity->SetName("PopUpMenu");

	CL_Vec2f OverlayRectSize = OverlayRectEntity->GetVar("size2d")->GetVector2();
	CL_Vec2f& OverlayRectPos = OverlayRectEntity->GetVar("pos2d")->GetVector2();
	OverlayRectPos.x = (ScreenSize.x / 2.0f) - (OverlayRectSize.x / 2.0f);
	OverlayRectPos.y = (ScreenSize.y / 2.0f) - (OverlayRectSize.y / 2.0f);

	Entity* pText = CreateTextLabelEntity(OverlayRectEntity, "title", iPhoneMapX(6), iPhoneMapY(5), GET_LOCTEXT("{GAMEMODESELECTMENU_TITLE}:"));
	eFont textFont;
	float textFontScale;
	GetFontAndScaleToFitThisLinesPerScreenY(&textFont, &textFontScale, 23);
	SetupTextEntity(pText, textFont, textFontScale);
	pText->GetVar("color")->Set(GET_THEMEMGR->GetTextColor());


	CL_Vec2f buttonPos = { iPhoneMapX(100), iPadMapY(375) };
	if (!IsTabletSize()) buttonPos = { iPhoneMapX(150), iPadMapY(520) };

	Entity* pCacnelFrame = CreateOverlayRectEntity(OverlayRectEntity, buttonPos, iPadMap(200, 66), GET_THEMEMGR->GetPrimaryColor());
	pCacnelFrame->GetComponentByName("RectRender")->GetVar("bmpBorderFileName")->Set(GET_THEMEMGR->GetFilename("interface/gui_box_upwhite.rttex"));
	pCacnelFrame->GetComponentByName("RectRender")->GetVar("borderColor")->Set(GET_THEMEMGR->GetSecondaryColor());
	CL_Vec2f cancelFrameSize = pCacnelFrame->GetVar("size2d")->GetVector2();

	Entity* pCancel = CreateTextButtonEntity(pCacnelFrame, "Cancel", (cancelFrameSize.x * 0.5f), (cancelFrameSize.y * 0.45f), GET_LOCTEXT("{DONE}"), false);
	eFont font;
	float fontScale;
	GetFontAndScaleToFitThisPixelHeight(&font, &fontScale, iPadMapY(66) * 0.7f);
	SetupTextEntity(pCancel, font, fontScale);
	pCancel->GetVar("color")->Set(GET_THEMEMGR->GetTextColor());
	SetAlignmentEntity(pCancel, ALIGNMENT_CENTER);
	pCancel->GetFunction("OnButtonSelected")->sig_function.connect(GamemodeSelectMenuOnSelect);
	SetButtonStyleEntity(pCancel, Button2DComponent::BUTTON_STYLE_CLICK_ON_TOUCH);
	AddHotKeyToButton(pCancel, VIRTUAL_KEY_BACK);


	CL_Vec2f scrollPos = GetPos2DEntity(pText);
	scrollPos.y += GetSize2DEntity(pText).y + iPhoneMapY(5);
	CL_Vec2f scrollSize;
	scrollSize.x = (OverlayRectSize.x + 8) - iPhoneMapX(20);
	scrollSize.y = OverlayRectSize.y - GetSize2DEntity(pText).y - iPhoneMapY(52.5f);
	Entity* pScroll = OverlayRectEntity->AddEntity(new Entity("scroll"));
	pScroll->GetVar("pos2d")->Set(scrollPos);
	pScroll->GetVar("size2d")->Set(scrollSize);
	pScroll->AddComponent(new TouchHandlerComponent);
	pScroll->GetVar("color")->Set(GET_THEMEMGR->GetScrollbarColor());

	EntityComponent* pScrollComp = pScroll->AddComponent(new ScrollComponent);
	pScrollComp->GetVar("fingerTracking")->Set(uint32(1));
	pScroll->AddComponent(new ScrollBarRenderComponent);

	Entity* pScrollChild = pScroll->AddEntity(new Entity("scroll_child"));
	pScroll->AddComponent(new RenderScissorComponent());
	pScroll->AddComponent(new FilterInputComponent);
	GamemodeSelectMenuAddScrollContent(pScroll);


	FadeInEntity(OverlayRectEntity);
	return OverlayRectEntity;
}