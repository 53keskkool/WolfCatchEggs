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
#include "ThemeSelectMenu.h"
#include "TutorialMenu.h"
#include "GamemodeSelectMenu.h"

void CreateMainMenu(VariantList* pVList)
{
	bool bAnimations = true;
	if (pVList)
	{
		if (pVList->Get(0).GetType() == Variant::TYPE_UINT32)
		{
			bAnimations = pVList->Get(0).GetUINT32();
		}
	}
	MainMenuCreate(GetEntityRoot()->GetEntityByName("GUI"), bAnimations);
}

void MainMenuOnSelect(VariantList *pVList) //0=vec2 point of click, 1=entity sent from
{
	Entity *pEntClicked = pVList->m_variant[1].GetEntity();
	Entity* pMenu = GetEntityRoot()->GetEntityByName("MainMenu");
	if (!pMenu) return;
	DisableComponentByName(pMenu, "CustomInput");

	if (pEntClicked->GetName() == "Play")
	{
		DisableAllButtonsEntity(pMenu);
		SlideScreen(pMenu, false);

		GetMessageManager()->CallEntityFunction(pMenu, 500, "OnDelete", NULL);

		if (!GetApp()->GetVarWithDefault("tutorial", uint32(false))->GetUINT32())
		{
			Entity* pTutorial = TutorialMenuCreate(pMenu->GetParent());
			pTutorial->GetVar("fromPlay")->Set(uint32(true));
		}
		else
		{
			GameMenuCreate(pMenu->GetParent());
		}
		return;
	}

	if (pEntClicked->GetName() == "Tutorial")
	{
		DisableAllButtonsEntity(pMenu);
		SlideScreen(pMenu, false);

		GetMessageManager()->CallEntityFunction(pMenu, 500, "OnDelete", NULL);

		TutorialMenuCreate(pMenu->GetParent());
		return;
	}

	if (pEntClicked->GetName() == "Gamemode")
	{
		DisableAllButtonsEntity(pMenu);
		GamemodeSelectMenuCreate(pMenu);
		return;
	}

	if (pEntClicked->GetName() == "Theme")
	{
		DisableAllButtonsEntity(pMenu);
		ThemeSelectMenuCreate(pMenu);
		return;
	}

	if (pEntClicked->GetName() == "Options")
	{
		DisableAllButtonsEntity(pMenu);
		SlideScreen(pMenu, false);

		GetMessageManager()->CallEntityFunction(pMenu, 500, "OnDelete", NULL);

		OptionsMenuCreate(pMenu->GetParent());
		return;
	}

	if (pEntClicked->GetName() == "About")
	{
		DisableAllButtonsEntity(pMenu);
		SlideScreen(pMenu, false);
		
		//kill this menu entirely, but we wait half a second while the transition is happening before doing it
		GetMessageManager()->CallEntityFunction(pMenu, 500, "OnDelete", NULL); 
		
		//create the new menu
		AboutMenuCreate(pMenu->GetParent());
		return;
	}

	if (pEntClicked->GetName() == "Language")
	{
		DisableComponentByName(pMenu, "CustomInput");
		DisableAllButtonsEntity(pMenu);

		LanguageSelectMenuCreate(pMenu);
		return;
	}

	if (pEntClicked->GetName() == "SecretCounter")
	{
		uint32* counter = &pEntClicked->GetVar("count")->GetUINT32();
		if (++(*counter) < 10) return;

		DisableAllButtonsEntity(pMenu);
		SlideScreen(pMenu, false);

		GetMessageManager()->CallEntityFunction(pMenu, 500, "OnDelete", NULL);
		
		OverlapTestMenuCreate(pMenu->GetParent());
		return;
	}
}

void MainMenuCreateButtons(Entity* pParentEnt, const vector<pair<string, string>>& buttons, bool bFadeIn)
{
	eFont font;
	float scale;
	GetFontAndScaleToFitThisLinesPerScreenY(&font, &scale, 11);
	float rectWidth = iPadMapX(5);
	float spacingX = iPadMapX(10);
	float spacingY = iPadMapY(15) + GetBaseApp()->GetFont(font)->GetLineHeight(scale);
	vector<pair<float, uint16>> lines; //width of every line and amount of buttons in them
	float maxWidth = 0;

	Entity* pButtons = pParentEnt->AddEntity(new Entity("Buttons"));
	CL_Vec2f buttonsPos;
	buttonsPos.x = GetScreenSizeXf() / 2;
	buttonsPos.y = GetScreenSizeYf() - iPadMapY(15);
	SetPos2DEntity(pButtons, buttonsPos);
	SetAlignmentEntity(pButtons, ALIGNMENT_DOWN_CENTER);
	CL_Vec2f buttonsSize;
	buttonsSize.x = GetScreenSizeXf() - rectWidth * 2;

	pair<float, uint16> currentLine = { 0, 0 };
	for (auto& b : buttons)
	{
		CL_Vec2f textSize = GetApp()->GetFont(font)->MeasureText(b.second, scale);
		if (currentLine.first + textSize.x + rectWidth + spacingX >= buttonsSize.x || currentLine.second >= 3)
		{
			if (currentLine.first > maxWidth) maxWidth = currentLine.first;
			lines.push_back(currentLine);
			currentLine.first = 0;
			currentLine.second = 0;
		}
		if (currentLine.first > 0) currentLine.first += spacingX;
		currentLine.first += textSize.x + rectWidth;
		currentLine.second++;
	}
	if (currentLine.first > maxWidth) maxWidth = currentLine.first;
	lines.push_back(currentLine);

	uint16 buttonNum = 0;
	uint8 curLine = 0;
	buttonsSize.x = maxWidth;
	buttonsSize.y = spacingY * lines.size();
	SetSize2DEntity(pButtons, buttonsSize);
	CL_Vec2f buttonPos;
	buttonPos.x = (maxWidth - lines[curLine].first) / 2;
	for (auto& b : buttons)
	{
		Entity* pButton = CreateTextButtonEntity(pButtons, b.first, buttonPos.x, buttonPos.y, b.second, false);
		pButton->GetFunction("OnButtonSelected")->sig_function.connect(MainMenuOnSelect);
		if (b.first == "Play") AddHotKeyToButton(pButton, 13); //enter

		pButton->GetVar("color")->Set(GET_THEMEMGR->GetTextColor());
		SetTextShadowColor(pButton, GET_THEMEMGR->GetTextShadowColor());
		SetupTextEntity(pButton, font, scale);
		RemovePaddingEntity(pButton);
		Entity* pRect = AddRectAroundEntity(pButton, GET_THEMEMGR->GetPrimaryColor(), GET_THEMEMGR->GetSecondaryColor(), rectWidth, true, scale, font);
		if (bFadeIn)
		{
			FadeInEntity(pRect, true, 200, 200 * buttonNum + 200);
			FadeInEntity(pButton, true, 200, 200 * buttonNum + 200);
		}

		buttonPos.x += GetSize2DEntity(pButton).x + rectWidth + spacingX;
		buttonNum++;
		if (--lines[curLine].second < 1)
		{
			curLine++;
			buttonPos.x = (maxWidth - lines[curLine].first) / 2;
			buttonPos.y += spacingY;
		}
	}
}

void MainMenuOnScreenSizeChanged()
{
	Entity* pMenu = GetEntityRoot()->GetEntityByName("MainMenu");
	if (!pMenu) return;
	SetSize2DEntity(pMenu, GetScreenSize());

	pMenu->SetName("OldMainMenu");
	pMenu->SetTaggedForDeletion();
	VariantList vlist(uint32(false));
	GetMessageManager()->CallStaticFunction(CreateMainMenu, 10, &vlist);
}

Entity* MainMenuCreate(Entity* pParentEnt, bool bAnimations)
{
	GetMessageManager()->SendGame(MESSAGE_TYPE_PLAY_MUSIC_FORCE_STREAMING, GET_THEMEMGR->GetFilename("audio/mp3/menu.mp3"), 250);

	Entity* pBG = pParentEnt->AddEntity(new Entity("MainMenu"));
	AddFocusIfNeeded(pBG);
	SetSize2DEntity(pBG, GetScreenSize());
	pBG->AddComponent(new RenderScissorComponent);

	//name, text
	vector<pair<string, string>> buttons = {
		{"Play", GET_LOCTEXT("{MAINMENU_PLAY}")},
		{"Theme", GET_LOCTEXT("{MAINMENU_THEME}")},
		{"Gamemode", GET_LOCTEXT("{MAINMENU_GAMEMODE}")},
		{"Tutorial", GET_LOCTEXT("{MAINMENU_TUTORIAL}")},
		{"Options", GET_LOCTEXT("{MAINMENU_OPTIONS}")},
		{"About", GET_LOCTEXT("{MAINMENU_ABOUT}")}
	};
	MainMenuCreateButtons(pBG, buttons, bAnimations);

	float langY = GetScreenSizeYf();
	Entity* pLang = CreateOverlayButtonEntity(pBG, "Language", GET_THEMEMGR->GetFilename("interface/lang_selector.rttex"), 0, langY);
	CL_Vec2f langScale;
	langScale.y = GetScreenSizeYf() / 9;
	EntitySetScaleBySize(pLang, langScale, true);
	pLang->GetFunction("OnButtonSelected")->sig_function.connect(MainMenuOnSelect);
	SetAlignmentEntity(pLang, ALIGNMENT_DOWN_LEFT);
	if (bAnimations)
	{
		FadeInEntity(pLang, true, 300, 950);
	}

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
	AddHotKeyToButton(pSecret, 'O'); //button O

	//background
	Entity* pRect = CreateOverlayEntity(pBG, "BG", GET_THEMEMGR->GetFilename("interface/main_menu.rttex"), GetScreenSizeXf() / 2, 0);
	EntitySetScaleBySize(pRect, CL_Vec2f(0, GetScreenSizeYf()), true);
	SetAlignmentEntity(pRect, ALIGNMENT_UPPER_CENTER);
	pBG->MoveEntityToBottomByAddress(pRect);

	EntityComponent* pComp = pBG->AddComponent(new CustomInputComponent);
	pComp->GetFunction("OnActivated")->sig_function.connect(1, boost::bind(&App::OnExitApp, GetApp(), _1));
	pComp->GetVar("keycode")->Set(uint32(VIRTUAL_KEY_BACK));

	//handle screen resizing
	GetBaseApp()->m_sig_onScreenSizeChanged.disconnect_all_slots();
	GetBaseApp()->m_sig_onScreenSizeChanged.connect(MainMenuOnScreenSizeChanged);

	if (bAnimations)
	{
		SlideScreen(pBG, true);
	}
	
	return pBG;
}

