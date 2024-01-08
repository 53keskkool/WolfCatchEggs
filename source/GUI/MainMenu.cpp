#include "PlatformPrecomp.h"
#include "MainMenu.h"
#include "Entity/EntityUtils.h"
#include "Entity/CustomInputComponent.h"
#include "AboutMenu.h"
#include "Renderer/SoftSurface.h"
#include "GUIUtils.h"
#include "LanguageSelectMenu.h"
#include "OptionsMenu.h"
#include "GameMenu.h"
#include "OverlapTestMenu.h"

void CreateMainMenu(VariantList* pVList)
{
	MainMenuCreate(GetEntityRoot()->GetEntityByName("GUI"));
}

void MainMenuOnSelect(VariantList *pVList) //0=vec2 point of click, 1=entity sent from
{
	Entity *pEntClicked = pVList->m_variant[1].GetEntity();

	if (pEntClicked->GetName() == "Play")
	{
		DisableAllButtonsEntity(pEntClicked->GetParent());
		SlideScreen(pEntClicked->GetParent(), false);

		GetMessageManager()->CallEntityFunction(pEntClicked->GetParent(), 500, "OnDelete", NULL);

		GameMenuCreate(pEntClicked->GetParent()->GetParent());
		return;
	}

	if (pEntClicked->GetName() == "Options")
	{
		DisableAllButtonsEntity(pEntClicked->GetParent());
		SlideScreen(pEntClicked->GetParent(), false);

		GetMessageManager()->CallEntityFunction(pEntClicked->GetParent(), 500, "OnDelete", NULL);

		OptionsMenuCreate(pEntClicked->GetParent()->GetParent());
		return;
	}

	if (pEntClicked->GetName() == "About")
	{
		DisableAllButtonsEntity(pEntClicked->GetParent());
		SlideScreen(pEntClicked->GetParent(), false);
		
		//kill this menu entirely, but we wait half a second while the transition is happening before doing it
		GetMessageManager()->CallEntityFunction(pEntClicked->GetParent(), 500, "OnDelete", NULL); 
		
		//create the new menu
		AboutMenuCreate(pEntClicked->GetParent()->GetParent());
		return;
	}

	if (pEntClicked->GetName() == "Quit")
	{
		GetApp()->OnExitApp(NULL);
		return;
	}

	if (pEntClicked->GetName() == "Language")
	{
		DisableComponentByName(pEntClicked->GetParent(), "CustomInput");
		DisableAllButtonsEntity(pEntClicked->GetParent());

		LanguageSelectMenuCreate(pEntClicked->GetParent());
		return;
	}

	if (pEntClicked->GetName() == "SecretCounter")
	{
		uint32* counter = &pEntClicked->GetVar("count")->GetUINT32();
		if (++(*counter) < 10) return;

		DisableAllButtonsEntity(pEntClicked->GetParent());
		SlideScreen(pEntClicked->GetParent(), false);

		GetMessageManager()->CallEntityFunction(pEntClicked->GetParent(), 500, "OnDelete", NULL);
		
		OverlapTestMenuCreate(pEntClicked->GetParent()->GetParent());
		return;
	}
}

void AddMainMenuButton(Entity* pParentEnt, float x, float& y, string name, string text, int buttonNum)
{
	eFont font;
	float scale;
	GetFontAndScaleToFitThisLinesPerScreenY(&font, &scale, (IsDesktop() ? 11 : 9));
	float spacing = iPadMapY(15) + GetBaseApp()->GetFont(font)->GetLineHeight(scale);

	Entity* pButtonEntity = CreateTextButtonEntity(pParentEnt, name, x, y, text, false);
	y += spacing;
	FunctionObject* pFunction = pButtonEntity->GetFunction("OnButtonSelected");
	pFunction->sig_function.connect(MainMenuOnSelect);
	if (name == "Play") AddHotKeyToButton(pButtonEntity, 13); //enter

	pButtonEntity->GetVar("color")->Set(GET_THEMEMGR->GetTextColor());
	SetTextShadowColor(pButtonEntity, GET_THEMEMGR->GetTextShadowColor());
	SetupTextEntity(pButtonEntity, font, scale);
	RemovePaddingEntity(pButtonEntity);
	SetAlignmentEntity(pButtonEntity, ALIGNMENT_UPPER_CENTER);
	Entity* pBMPRect = AddRectAroundEntity(pButtonEntity, GET_THEMEMGR->GetPrimaryColor(), GET_THEMEMGR->GetSecondaryColor(), iPadMapX(5), true, scale, font);
	FadeInEntity(pBMPRect, true, 300, 300 * buttonNum + 300);
	FadeInEntity(pButtonEntity, true, 300, 300 * buttonNum + 300);
}

Entity * MainMenuCreate(Entity *pParentEnt)
{
	Entity *pBG = pParentEnt->AddEntity(new Entity("MainMenu"));
	AddFocusIfNeeded(pBG);

	float x = iPadMapX(512);
	float y = iPadMapY(420);

	AddMainMenuButton(pBG, x, y, "Play", GET_LOCTEXT("{MAINMENU_PLAY}"), 0);
	AddMainMenuButton(pBG, x, y, "Options", GET_LOCTEXT("{MAINMENU_OPTIONS}"), 1);
	AddMainMenuButton(pBG, x, y, "About", GET_LOCTEXT("{MAINMENU_ABOUT}"), 2);
	if (IsDesktop()) AddMainMenuButton(pBG, x, y, "Quit", GET_LOCTEXT("{MAINMENU_QUIT}"), 3);

	Entity* pLang = CreateOverlayButtonEntity(pBG, "Language", GET_THEMEMGR->GetFilename("interface/lang_selector.rttex"), 0, GetScreenSizeYf());
	CL_Vec2f langScale;
	langScale.y = GetScreenSizeYf() / 9;
	EntitySetScaleBySize(pLang, langScale, true);
	pLang->GetFunction("OnButtonSelected")->sig_function.connect(MainMenuOnSelect);
	SetAlignmentEntity(pLang, ALIGNMENT_DOWN_LEFT);
	FadeInEntity(pLang, true, 300, 950);

	//a button for overlap test menu
	Entity* pSecret = pBG->AddEntity(new Entity("SecretCounter"));
	SetPos2DEntity(pSecret, CL_Vec2f(-100, -100));
	SetSize2DEntity(pSecret, CL_Vec2f(0, 0));
	RemovePaddingEntity(pSecret);
	pSecret->AddComponent(new TouchHandlerComponent);
	EntityComponent* pButtonComp = pSecret->AddComponent(new Button2DComponent);
	pButtonComp->GetVar("repeatDelayMS")->Set(uint32(0)); //removing delay, we don't need it for a secret button
	pButtonComp->GetVar("onClickAudioFile")->Set(""); //removing sound from button
	pSecret->GetFunction("OnButtonSelected")->sig_function.connect(MainMenuOnSelect);
	AddHotKeyToButton(pSecret, 0x4F); //button O

	//background - TODO
	Entity* pRect = CreateOverlayRectEntity(pBG, CL_Vec2f(0, 0), GetScreenSize(), MAKE_RGBA(0x54, 0x1d, 0x10, 0xff));
	pBG->MoveEntityToBottomByAddress(pRect);

	EntityComponent* pComp = pBG->AddComponent(new CustomInputComponent);
	pComp->GetFunction("OnActivated")->sig_function.connect(1, boost::bind(&App::OnExitApp, GetApp(), _1));
	pComp->GetVar("keycode")->Set(uint32(VIRTUAL_KEY_BACK));

	SlideScreen(pBG, true);
	
	return pBG;
}

