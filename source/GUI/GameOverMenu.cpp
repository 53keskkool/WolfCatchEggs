#include "PlatformPrecomp.h"
#include "GameOverMenu.h"
#include "Entity/EntityUtils.h"
#include "GUI/GUIUtils.h"
#include "GUI/GameMenu.h"
#include "GUI/MainMenu.h"

EntityComponent* SetupInterpolateComponent(Entity* pEnt, const string& componentName, const string& varName, const Variant& targetValue, int durationMS, int delayBeforeStartMS, eInterpolateType interpolationType = INTERPOLATE_SMOOTHSTEP, InterpolateComponent::eOnFinish onFinish = InterpolateComponent::ON_FINISH_DIE, eTimingSystem timing = GetTiming());

void GameOverMenuOnSelect(VariantList* pVList)
{
	Entity* pEntClicked = pVList->m_variant[1].GetEntity();
	Entity* pGameMenu = pEntClicked->GetParent()->GetParent();
	if (!pGameMenu) return;
	if (pGameMenu->GetName() != "GameMenu") return;

	if (pEntClicked->GetName() == "back")
	{
		DisableAllButtonsEntity(pEntClicked->GetParent());
		DisableAllButtonsEntity(pGameMenu);
		SlideScreen(pGameMenu, false);
		pGameMenu->SetName("OldGameMenu");

		GetMessageManager()->CallEntityFunction(pGameMenu, 500, "OnDelete", NULL);

		MainMenuCreate(GetEntityRoot()->GetEntityByName("GUI"));
		return;
	}

	if (pEntClicked->GetName() == "again")
	{
		DisableAllButtonsEntity(pEntClicked->GetParent());
		DisableAllButtonsEntity(pGameMenu);
		SlideScreen(pGameMenu, false);
		pGameMenu->SetName("OldGameMenu");

		GetMessageManager()->CallEntityFunction(pGameMenu, 500, "OnDelete", NULL);

		GameMenuCreate(GetEntityRoot()->GetEntityByName("GUI"));
		return;
	}
}

void GameOverMenuOnScreenSizeChanged(VariantList* pVList) {
	Entity* pMenu = GetEntityRoot()->GetEntityByName("GameOverMenu");
	if (!pMenu) return;

	eFont font;
	float fontScale;
	GetFontAndScaleToFitThisLinesPerScreenY(&font, &fontScale, 8);
	Entity* pTitle = pMenu->GetEntityByName("title");
	SetupTextEntity(pTitle, font, fontScale);
	SetPos2DEntity(pTitle, CL_Vec2f(GetScreenSizeXf() / 2, iPadMapY(15)));

	GetFontAndScaleToFitThisLinesPerScreenY(&font, &fontScale, 16);
	Entity* pScore = pMenu->GetEntityByName("score");
	SetupTextEntity(pScore, font, fontScale);
	SetPos2DEntity(pScore, CL_Vec2f(GetScreenSizeXf() / 2, GetScreenSizeYf() / 2));

	Entity* pBest = pMenu->GetEntityByName("newBest");
	if (pBest)
	{
		eFont newBestFont;
		float newBestScale;
		GetFontAndScaleToFitThisLinesPerScreenY(&newBestFont, &newBestScale, 11.5f);
		
		SetupTextEntity(pBest, newBestFont, newBestScale);
		float rotation = pBest->GetVar("rotation")->GetFloat();

		CL_Vec2f newBestSize = GetSize2DEntity(pBest);
		CL_Vec2f newBestPos = GetPos2DEntity(pScore);
		newBestPos.x += GetSize2DEntity(pScore).x / 2;
		newBestPos.y -= GetSize2DEntity(pScore).y / 2;
		newBestPos.x += (sin(DEG2RAD(rotation)) * newBestSize.x) * 0.5f;
		newBestPos.y -= (cos(DEG2RAD(rotation)) * newBestSize.x) * 0.2f;
		SetPos2DEntity(pBest, newBestPos);

		//make NEW BEST text fly out and bounce afterwards
		pBest->RemoveComponentByName("ic_bounce");
		SetupInterpolateComponent(pBest, "ic_bounce", "scale2d", CL_Vec2f(newBestScale * 0.8f, newBestScale * 0.8f), 500, 0, INTERPOLATE_SMOOTHSTEP, InterpolateComponent::ON_FINISH_BOUNCE);
	}
	else
	{
		float lineHeight = GetApp()->GetFont(font)->GetLineHeight(fontScale);
		SetPos2DEntity(pScore, CL_Vec2f(GetScreenSizeXf() / 2, (GetScreenSizeYf() / 2) - (lineHeight / 2)));

		pBest = pMenu->GetEntityByName("best");
		SetupTextEntity(pBest, font, fontScale);
		SetPos2DEntity(pBest, CL_Vec2f(GetScreenSizeXf() / 2, (GetScreenSizeYf() / 2) + (lineHeight / 2)));
	}

	pMenu->RemoveEntitiesByNameThatStartWith("back");
	pMenu->RemoveEntitiesByNameThatStartWith("again");

	//creating buttons to go back or play again
	float buttonSpacer = iPadMapX(15);
	Entity* pBack = CreateTextButtonEntity(pMenu, "back", 0, 0, GET_LOCTEXT("{BACK}"), false);
	pBack->GetFunction("OnButtonSelected")->sig_function.connect(GameOverMenuOnSelect);
	eFont buttonFont;
	float buttonFontScale;
	GetFontAndScaleToFitThisLinesPerScreenY(&buttonFont, &buttonFontScale, 13);
	SetupTextEntity(pBack, buttonFont, buttonFontScale);
	pBack->GetVar("color")->Set(GET_THEMEMGR->GetTextColor());
	Entity* pBackRect = AddRectAroundEntity(pBack, GET_THEMEMGR->GetPrimaryColor(), GET_THEMEMGR->GetSecondaryColor(), 10, true, buttonFontScale, buttonFont);
	pBackRect->SetName("back_");
	SetTextShadowColor(pBack, GET_THEMEMGR->GetTextShadowColor());
	AddHotKeyToButton(pBack, VIRTUAL_KEY_BACK); //back goes to the main menu

	Entity* pAgain = CreateTextButtonEntity(pMenu, "again", GetSize2DEntity(pBackRect).x + buttonSpacer, 0, GET_LOCTEXT("{GAMEOVER_TRYAGAIN}"), false);
	pAgain->GetFunction("OnButtonSelected")->sig_function.connect(GameOverMenuOnSelect);
	SetupTextEntity(pAgain, buttonFont, buttonFontScale);
	pAgain->GetVar("color")->Set(GET_THEMEMGR->GetTextColor());
	Entity* pAgainRect = AddRectAroundEntity(pAgain, GET_THEMEMGR->GetPrimaryColor(), GET_THEMEMGR->GetSecondaryColor(), 10, true, buttonFontScale, buttonFont);
	pAgainRect->SetName("again_");
	SetTextShadowColor(pAgain, GET_THEMEMGR->GetTextShadowColor());
	AddHotKeyToButton(pAgain, 13); //enter starts game again

	//aligning buttons (i could do it somehow easier, but this way it looks good)
	float totalWidthOfButtons = GetSize2DEntity(pBackRect).x + buttonSpacer + GetSize2DEntity(pAgainRect).x;
	CL_Vec2f buttonsMove;
	buttonsMove.x = (GetScreenSizeXf() / 2) - (totalWidthOfButtons / 2) - GetPos2DEntity(pBackRect).x;
	buttonsMove.y = GetScreenSizeYf() - iPadMapY(15) - GetSize2DEntity(pAgainRect).y;
	pBack->GetVar("pos2d")->Set(GetPos2DEntity(pBack) + buttonsMove); //doing this way, so onchangedsig gets called
	pBackRect->GetVar("pos2d")->GetVector2() += buttonsMove;
	pAgain->GetVar("pos2d")->Set(GetPos2DEntity(pAgain) + buttonsMove); //doing this way, so onchangedsig gets called
	pAgainRect->GetVar("pos2d")->GetVector2() += buttonsMove;

	Entity* pDarken = pMenu->GetEntityByName("darken");
	SetSize2DEntity(pDarken, GetScreenSize());
	pMenu->MoveEntityToBottomByAddress(pDarken);

	pMenu->GetParent()->MoveEntityToTopByAddress(pMenu);
}

Entity* GameOverMenuCreate(Entity* pParentEnt, uint32 oldBest, uint32 currentScore)
{
	GetAudioManager()->StopMusic();
	GetMessageManager()->SendGame(MESSAGE_TYPE_PLAY_MUSIC_FORCE_STREAMING, GET_THEMEMGR->GetFilename("audio/mp3/game_over.mp3"), 250);

	Entity* pMenu = pParentEnt->AddEntity(new Entity("GameOverMenu"));

	eFont font;
	float fontScale;
	GetFontAndScaleToFitThisLinesPerScreenY(&font, &fontScale, 8);
	Entity* pTitle = CreateTextLabelEntity(pMenu, "title", GetScreenSizeXf() / 2, iPadMapY(15), GET_LOCTEXT("{GAMEOVER_TITLE}"));
	SetupTextEntity(pTitle, font, fontScale);
	pTitle->GetVar("color")->Set(GET_THEMEMGR->GetTextColor());
	SetTextShadowColor(pTitle, GET_THEMEMGR->GetTextShadowColor());
	SetAlignmentEntity(pTitle, ALIGNMENT_UPPER_CENTER);

	GetFontAndScaleToFitThisLinesPerScreenY(&font, &fontScale, 16);
	string scoreText = GET_LOCTEXT("{GAMEOVER_SCORE}: ") + to_string(currentScore);
	Entity* pScore = CreateTextLabelEntity(pMenu, "score", GetScreenSizeXf() / 2, GetScreenSizeYf() / 2, scoreText);
	SetupTextEntity(pScore, font, fontScale);
	pScore->GetVar("color")->Set(GET_THEMEMGR->GetTextColor());
	SetTextShadowColor(pScore, GET_THEMEMGR->GetTextShadowColor());
	SetAlignmentEntity(pScore, ALIGNMENT_CENTER);

	//also show old best
	if (oldBest >= currentScore)
	{
		float lineHeight = GetApp()->GetFont(font)->GetLineHeight(fontScale);
		SetPos2DEntity(pScore, CL_Vec2f(GetScreenSizeXf() / 2, (GetScreenSizeYf() / 2) - (lineHeight / 2)));

		string bestText = GET_LOCTEXT("{GAMEOVER_URBEST}: ") + to_string(oldBest);
		Entity* pBest = CreateTextLabelEntity(pMenu, "best", GetScreenSizeXf() / 2, (GetScreenSizeYf() / 2) + (lineHeight / 2), bestText);
		SetupTextEntity(pBest, font, fontScale);
		pBest->GetVar("color")->Set(GET_THEMEMGR->GetTextColor());
		SetTextShadowColor(pBest, GET_THEMEMGR->GetTextShadowColor());
		SetAlignmentEntity(pBest, ALIGNMENT_CENTER);
	}
	else
	{ //let's add NEW BEST text :D
		eFont newBestFont;
		float newBestScale;
		GetFontAndScaleToFitThisLinesPerScreenY(&newBestFont, &newBestScale, 11.5f);

		Entity* pNewBest = CreateTextLabelEntity(pMenu, "newBest", 0, 0, GET_LOCTEXT("{GAMEOVER_NEWBEST}"));
		SetupTextEntity(pNewBest, newBestFont, newBestScale);
		pNewBest->GetVar("color")->Set(MAKE_RGBA(255, 255, 0, 255));
		float rotation = 30.0f;
		pNewBest->GetVar("rotation")->Set(rotation);

		CL_Vec2f newBestSize = GetSize2DEntity(pNewBest);
		CL_Vec2f newBestPos = GetPos2DEntity(pScore);
		newBestPos.x += GetSize2DEntity(pScore).x / 2;
		newBestPos.y -= GetSize2DEntity(pScore).y / 2;
		newBestPos.x += (sin(DEG2RAD(rotation)) * newBestSize.x) * 0.5f;
		newBestPos.y -= (cos(DEG2RAD(rotation)) * newBestSize.x) * 0.2f;
		SetPos2DEntity(pNewBest, newBestPos);
		SetAlignmentEntity(pNewBest, ALIGNMENT_DOWN_CENTER);

		//make NEW BEST text fly out and bounce afterwards
		SetScale2DEntity(pNewBest, CL_Vec2f(0, 0));
		SetupInterpolateComponent(pNewBest, "ic_bounceOnce", "scale2d", CL_Vec2f(newBestScale, newBestScale), 600, 300);
		SetupInterpolateComponent(pNewBest, "ic_bounce", "scale2d", CL_Vec2f(newBestScale * 0.8f, newBestScale * 0.8f), 500, 900, INTERPOLATE_SMOOTHSTEP, InterpolateComponent::ON_FINISH_BOUNCE);
	}

	//creating buttons to go back or play again
	float buttonSpacer = iPadMapX(15);
	Entity* pBack = CreateTextButtonEntity(pMenu, "back", 0, 0, GET_LOCTEXT("{BACK}"), false);
	pBack->GetFunction("OnButtonSelected")->sig_function.connect(GameOverMenuOnSelect);
	eFont buttonFont;
	float buttonFontScale;
	GetFontAndScaleToFitThisLinesPerScreenY(&buttonFont, &buttonFontScale, 13);
	SetupTextEntity(pBack, buttonFont, buttonFontScale);
	pBack->GetVar("color")->Set(GET_THEMEMGR->GetTextColor());
	Entity* pBackRect = AddRectAroundEntity(pBack, GET_THEMEMGR->GetPrimaryColor(), GET_THEMEMGR->GetSecondaryColor(), 10, true, buttonFontScale, buttonFont);
	pBackRect->SetName("back_");
	SetTextShadowColor(pBack, GET_THEMEMGR->GetTextShadowColor());
	AddHotKeyToButton(pBack, VIRTUAL_KEY_BACK); //back goes to the main menu

	Entity* pAgain = CreateTextButtonEntity(pMenu, "again", GetSize2DEntity(pBackRect).x + buttonSpacer, 0, GET_LOCTEXT("{GAMEOVER_TRYAGAIN}"), false);
	pAgain->GetFunction("OnButtonSelected")->sig_function.connect(GameOverMenuOnSelect);
	SetupTextEntity(pAgain, buttonFont, buttonFontScale);
	pAgain->GetVar("color")->Set(GET_THEMEMGR->GetTextColor());
	Entity* pAgainRect = AddRectAroundEntity(pAgain, GET_THEMEMGR->GetPrimaryColor(), GET_THEMEMGR->GetSecondaryColor(), 10, true, buttonFontScale, buttonFont);
	pAgainRect->SetName("again_");
	SetTextShadowColor(pAgain, GET_THEMEMGR->GetTextShadowColor());
	AddHotKeyToButton(pAgain, 13); //enter starts game again
	
	//aligning buttons (i could do it somehow easier, but this way it looks good)
	float totalWidthOfButtons = GetSize2DEntity(pBackRect).x + buttonSpacer + GetSize2DEntity(pAgainRect).x;
	CL_Vec2f buttonsMove;
	buttonsMove.x = (GetScreenSizeXf() / 2) - (totalWidthOfButtons / 2) - GetPos2DEntity(pBackRect).x;
	buttonsMove.y = GetScreenSizeYf() - iPadMapY(15) - GetSize2DEntity(pAgainRect).y;
	pBack->GetVar("pos2d")->Set(GetPos2DEntity(pBack) + buttonsMove); //doing this way, so onchangedsig gets called
	pBackRect->GetVar("pos2d")->GetVector2() += buttonsMove;
	pAgain->GetVar("pos2d")->Set(GetPos2DEntity(pAgain) + buttonsMove); //doing this way, so onchangedsig gets called
	pAgainRect->GetVar("pos2d")->GetVector2() += buttonsMove;

	Entity* pDarken = CreateOverlayRectEntity(pMenu, CL_Vec2f(0, 0), GetScreenSize(), MAKE_RGBA(0, 0, 0, 200));
	pDarken->SetName("darken");
	pMenu->MoveEntityToBottomByAddress(pDarken);

	pParentEnt->GetFunction("OnScreenSizeChanged")->sig_function.connect(GameOverMenuOnScreenSizeChanged);

	FadeInEntity(pMenu, true, 600);
	return pMenu;
}