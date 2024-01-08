#include "PlatformPrecomp.h"
#include "Entity/EntityUtils.h"
#include "GUIUtils.h"
#include "ThemeSelectMenu.h"
#include "OptionsMenu.h"

void ThemeSelectMenuOnSelect(VariantList* pVList)
{
	Entity* pEntClicked = pVList->Get(1).GetEntity();
	string name = pEntClicked->GetName();

	Entity* pOptionsMenu = GetEntityRoot()->GetEntityByName("OptionsMenu");
	if (name == GetApp()->GetVar("theme")->GetString() || name == "Cancel")
	{
		Entity* pMenu = GetEntityRoot()->GetEntityByName("PopUpMenu");
		FadeOutEntity(pMenu);
		GetMessageManager()->CallEntityFunction(pMenu, 500, "OnDelete");
		DisableAllButtonsEntity(pMenu);

		Entity* pDarken = GetEntityRoot()->GetEntityByName("pop_up_darken");
		FadeScreen(pDarken, 0, 0, 400, true);
		KillEntity(pDarken, 400);
		pDarken->SetName("");

		EnableAllButtonsEntity(pOptionsMenu);
		EnableComponentByName(pOptionsMenu, "CustomInput");
		return;
	}

	pOptionsMenu->SetTaggedForDeletion();
	pOptionsMenu->SetName("");

	GetApp()->GetVar("theme")->Set(name);
	if (!GetApp()->GetThemeManager()->Init(name))
	{
		LogError("Failed to load translation %s, falling back to default", name.c_str());
		GetApp()->GetVar("theme")->Set("default");
		GetApp()->GetThemeManager()->Init("default");
	}

	OptionsMenuCreate(GetEntityRoot()->GetEntityByName("GUI"));
}

void ThemeSelectMenuAddScrollContent(Entity* pParent)
{
	pParent = pParent->GetEntityByName("scroll_child");
	pParent->RemoveAllEntities();

	float x = 5;
	float y = 0;

	string curTheme = GetApp()->GetVar("theme")->GetString();
	for (auto& l : GetApp()->GetThemeManager()->GetListOfThemes())
	{
		Entity* pCheckbox = CreateCheckbox(pParent, l.first, GET_LOCTEXT(l.second), x, y, curTheme == l.first, FONT_SMALL, 1, curTheme == l.first);
		pCheckbox->GetEntityByName("_text" + l.first)->GetVar("color")->Set(GET_THEMEMGR->GetTextColor());
		pCheckbox->GetFunction("OnButtonSelected")->sig_function.connect(ThemeSelectMenuOnSelect);
		CL_Vec2f boxSize = GetSize2DEntity(pCheckbox);

		y += boxSize.y + iPhoneMapY(5);
	}

	VariantList vList(pParent->GetParent());
	ResizeScrollBounds(&vList);
}

Entity* ThemeSelectMenuCreate(Entity* pParentEnt)
{
	Entity* EntityRoot = GetEntityRoot();
	if (EntityRoot->GetEntityByName("pop_up_darken")) return 0; //We won't create pop up on a pop up

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

	Entity* pText = CreateTextLabelEntity(OverlayRectEntity, "title", iPhoneMapX(6), iPhoneMapY(5), GET_LOCTEXT("{THEMESELECTMENU_TITLE}:"));
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

	Entity* pCancel = CreateTextButtonEntity(pCacnelFrame, "Cancel", (cancelFrameSize.x * 0.5f), (cancelFrameSize.y * 0.45f), GET_LOCTEXT("{CANCEL}"), false);
	eFont font;
	float fontScale;
	GetFontAndScaleToFitThisPixelHeight(&font, &fontScale, iPadMapY(66) * 0.7f);
	SetupTextEntity(pCancel, font, fontScale);
	pCancel->GetVar("color")->Set(GET_THEMEMGR->GetTextColor());
	SetAlignmentEntity(pCancel, ALIGNMENT_CENTER);
	pCancel->GetFunction("OnButtonSelected")->sig_function.connect(ThemeSelectMenuOnSelect);
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
	ThemeSelectMenuAddScrollContent(pScroll);


	FadeInEntity(OverlayRectEntity);
	return OverlayRectEntity;
}