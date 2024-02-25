#include "PlatformPrecomp.h"
#include "GameMenu.h"
#include "Entity/EntityUtils.h"
#include "Entity/ArcadeInputComponent.h"
#include "Entity/TouchHandlerComponent.h"
#include "Component/EggManager.h"
#include "GameOverMenu.h"
#include "PauseMenu.h"

void OnScoreUpdate(VariantList* pVList)
{
	Entity* pInfo = GetEntityRoot()->GetEntityByName("InfoBox");
	if (!pInfo) return; //that's strange

	SetTextEntityByName("Score", GET_LOCTEXT("{SCORE}: ") + std::to_string(pVList->Get(0).GetUINT32()), pInfo);
	if (GetApp()->GetGameMode() != GAMEMODE_MATH)
	{
		uint8 lives = (uint8)pVList->Get(1).GetUINT32();
		for (uint8 life = 3; life > 0; life--)
		{
			Entity* pLive = pInfo->GetEntityByName("Live" + std::to_string(life));
			if (!pLive) continue;
			uint32 colorMod = 0xFFFFFFFF;
			if (lives < life) colorMod = MAKE_RGBA(0, 0, 0, 35);
			pLive->GetVar("colorMod")->Set(colorMod);
		}
	}
}

Entity* InfoBoxCreate(Entity* pParentEnt)
{
	float boxHeight = GetScreenSizeYf() * 0.095f;

	Entity* pBG = NULL;
	if (GET_THEMEMGR->NeedScoreBG()) {
		pBG = CreateOverlayRectEntity(pParentEnt, CL_Vec2f(0, 0), CL_Vec2f(100, 100), ColorCombine(GET_THEMEMGR->GetPrimaryColor(), MAKE_RGBA(0xF0, 0xF0, 0xF0, 0xA0)));
		pBG->GetComponentByName("RectRender")->GetVar("bmpBorderFileName")->Set(GET_THEMEMGR->GetFilename("interface/gui_box_white.rttex"));
		pBG->GetComponentByName("RectRender")->GetVar("borderColor")->Set(GET_THEMEMGR->GetPrimaryColor());
	}
	else {
		pBG = pParentEnt->AddEntity(new Entity);
	}
	pBG->GetVar("finishMenuName")->Set(pParentEnt->GetName());
	pBG->SetName("InfoBox");

	eFont font;
	float fontScale;
	GetFontAndScaleToFitThisPixelHeight(&font, &fontScale, boxHeight - 20);
	Entity* pText = CreateTextLabelEntity(pBG, "Score", 10, 10, GET_LOCTEXT("{SCORE}: 000"));
	SetupTextEntity(pText, font, fontScale);
	pText->GetVar("color")->Set(GET_THEMEMGR->GetTextColor());
	SetTextShadowColor(pText, GET_THEMEMGR->GetTextShadowColor());

	CL_Vec2f boxSize;
	if (GetApp()->GetGameMode() != GAMEMODE_MATH)
	{
		float spacer = iPadMapX(50);
		float eggSpacer = iPadMapX(10);
		float eggX = GetSize2DEntity(pText).x + spacer;
		float eggWidth = 0;
		CL_Vec2f eggScale;
		Entity* pLive = CreateOverlayEntity(pBG, "Live3", GET_THEMEMGR->GetFilename("game/egg.rttex"), eggX, 10);
		CL_Vec2f liveSize = GetSize2DEntity(pLive);
		EntitySetScaleBySize(pLive, CL_Vec2f(0, boxHeight - 20), true, liveSize.x < liveSize.y);
		eggScale = GetScale2DEntity(pLive);
		eggWidth = GetSize2DEntity(pLive).x;
		eggX += eggSpacer + eggWidth;
		pLive = CreateOverlayEntity(pBG, "Live2", GET_THEMEMGR->GetFilename("game/egg.rttex"), eggX, 10);
		SetScale2DEntity(pLive, eggScale);
		eggX += eggSpacer + eggWidth;
		pLive = CreateOverlayEntity(pBG, "Live1", GET_THEMEMGR->GetFilename("game/egg.rttex"), eggX, 10);
		SetScale2DEntity(pLive, eggScale);
		eggX += eggSpacer + eggWidth;

		boxSize.x = 10 + GetSize2DEntity(pText).x + spacer + eggSpacer * 2 + eggWidth * 3 + 10;
	}
	else
	{
		boxSize.x = 20 + GetSize2DEntity(pText).x;
		SetPos2DEntity(pText, CL_Vec2f(boxSize.x / 2, 10));
		SetAlignmentEntity(pText, ALIGNMENT_UPPER_CENTER);
	}

	boxSize.y = boxHeight;
	SetSize2DEntity(pBG, boxSize);
	CL_Vec2f boxPos;
	boxPos.x = GetScreenSizeXf() / 2 - boxSize.x / 2;
	boxPos.y = 0;
	SetPos2DEntity(pBG, boxPos);
	SetTextEntity(pText, GET_LOCTEXT("{SCORE}: 0"));

	Entity* pLevel = pParentEnt->GetEntityByName("Level");
	pLevel->GetFunction("OnScoreUpdate")->sig_function.connect(OnScoreUpdate);

	return pBG;
}


void GameMenuOnArcadeInput(VariantList* pVList)
{
	int vKey = pVList->Get(0).GetUINT32();
	bool bIsDown = pVList->Get(1).GetUINT32() != 0;
	if (!bIsDown) return;
	if (vKey != VIRTUAL_KEY_BACK && vKey != VIRTUAL_KEY_F12) return;

	Entity* pMenu = GetEntityRoot()->GetEntityByName("GameMenu");
	if (!pMenu) return;
	if (vKey == VIRTUAL_KEY_F12)
	{
		if (pMenu->GetEntityByName("GameOverMenu")) return;
		bool bPaused = pMenu->GetVarWithDefault("paused", uint32(0))->GetUINT32();
		VariantList varlist(uint32(!bPaused), uint32(true));
		pMenu->GetFunction("OnPause")->sig_function(&varlist);
		return;
	}
	if (pMenu->GetVarWithDefault("paused", uint32(0))->GetUINT32()) return;
	if (pMenu->GetEntityByName("GameOverMenu")) return;
	VariantList varlist(uint32(true));
	pMenu->GetFunction("OnPause")->sig_function(&varlist);
}


void OnGameOver(VariantList* pVList)
{
	Entity* pMenu = GetEntityRoot()->GetEntityByName("GameMenu");
	if (!pMenu) return;
	uint32 currentScore = pVList->Get(0).GetUINT32();
	uint32 oldBest = GetApp()->GetVarWithDefault("bestScore" + to_string(GetApp()->GetGameMode()), uint32(0))->GetUINT32();
	if (currentScore > oldBest) GetApp()->GetVar("bestScore" + to_string(GetApp()->GetGameMode()))->Set(currentScore);
	GetApp()->Save();
	GameOverMenuCreate(pMenu, oldBest, currentScore);
}

void OnPause(VariantList* pVList)
{
	Entity* pMenu = GetEntityRoot()->GetEntityByName("GameMenu");
	if (!pMenu) return;
	pMenu->GetVar("paused")->Set(pVList->Get(0));
	if (!pVList->Get(0).GetUINT32()) return;

	if (!pVList->Get(1).GetUINT32()) PauseMenuCreate(pMenu);
}

void BaseGameMenuOnScreenSizeChanged() {
	Entity* pMenu = GetEntityRoot()->GetEntityByName("GameMenu");
	if (!pMenu) return;

	Entity* pBG = pMenu->GetEntityByName("BG");
	if (GET_THEMEMGR->IsBGScalingAllowed())
	{
		EntitySetScaleBySize(pBG, GetScreenSize());
	}
	else
	{
		EntitySetScaleBySize(pBG, CL_Vec2f(0, GetScreenSizeYf()), true);
		SetPos2DEntity(pBG, CL_Vec2f(GetScreenSizeXf() / 2, 0));
		SetAlignmentEntity(pBG, ALIGNMENT_UPPER_CENTER);
	}

	GetMessageManager()->CallEntityFunction(pMenu, 10, "OnScreenSizeChanged");
}

void GameMenuOnScreenSizeChanged(VariantList* pVList)
{
	Entity* pMenu = GetEntityRoot()->GetEntityByName("GameMenu");
	if (!pMenu) return;
	SetSize2DEntity(pMenu, GetScreenSize());

	pMenu->RemoveEntityByName("InfoBox");
	InfoBoxCreate(pMenu);

	Entity* pPause = pMenu->GetEntityByName("PauseMenu");
	if (pPause)
	{
		pMenu->RemoveEntityByAddress(pPause);
		PauseMenuCreate(pMenu, false);
	}
}

Entity* GameMenuCreate(Entity* pParentEnt)
{
	GetMessageManager()->SendGame(MESSAGE_TYPE_PLAY_MUSIC_FORCE_STREAMING, GET_THEMEMGR->GetFilename("audio/mp3/game.mp3"), 250);

	Entity* pMenu = pParentEnt->AddEntity(new Entity("GameMenu"));
	AddFocusIfNeeded(pMenu);
	SetSize2DEntity(pMenu, GetScreenSize());
	pMenu->AddComponent(new RenderScissorComponent);

	//adding arcade input to listen for keyboard
	ArcadeInputComponent* pComp = (ArcadeInputComponent*)pMenu->AddComponent(new ArcadeInputComponent);
	AddKeyBinding(pComp, "Left", VIRTUAL_KEY_DIR_LEFT, VIRTUAL_KEY_DIR_LEFT);
	AddKeyBinding(pComp, "LeftA", 'A', VIRTUAL_KEY_DIR_LEFT);
	AddKeyBinding(pComp, "Right", VIRTUAL_KEY_DIR_RIGHT, VIRTUAL_KEY_DIR_RIGHT);
	AddKeyBinding(pComp, "RightD", 'D', VIRTUAL_KEY_DIR_RIGHT);
	AddKeyBinding(pComp, "Up", VIRTUAL_KEY_DIR_UP, VIRTUAL_KEY_DIR_UP);
	AddKeyBinding(pComp, "UpW", 'W', VIRTUAL_KEY_DIR_UP);
	AddKeyBinding(pComp, "Down", VIRTUAL_KEY_DIR_DOWN, VIRTUAL_KEY_DIR_DOWN);
	AddKeyBinding(pComp, "DownS", 'S', VIRTUAL_KEY_DIR_DOWN);
	AddKeyBinding(pComp, "Pause", VIRTUAL_KEY_BACK, VIRTUAL_KEY_BACK);
	AddKeyBinding(pComp, "PauseP", 'P', VIRTUAL_KEY_BACK);
	AddKeyBinding(pComp, "SilentPause", VIRTUAL_KEY_F12, VIRTUAL_KEY_F12);
	GetBaseApp()->m_sig_arcade_input.connect(pMenu->GetFunction("OnArcadeInput")->sig_function);
	//handler to pause the game
	pMenu->GetFunction("OnArcadeInput")->sig_function.connect(GameMenuOnArcadeInput);
	//adding touch handler to listen for touchscreen/mouse clicks
	pMenu->AddComponent(new TouchHandlerComponent);
	//adding action when game is over
	pMenu->GetFunction("OnGameOver")->sig_function.connect(OnGameOver);
	//action on pause
	pMenu->GetFunction("OnPause")->sig_function.connect(OnPause);

	Entity* pBG = CreateOverlayEntity(pMenu, "BG", GET_THEMEMGR->GetFilename("game/bg.rttex"), 0, 0);
	if (GET_THEMEMGR->IsBGScalingAllowed())
	{
		EntitySetScaleBySize(pBG, GetScreenSize());
	}
	else
	{
		EntitySetScaleBySize(pBG, CL_Vec2f(0, GetScreenSizeYf()), true);
		SetPos2DEntity(pBG, CL_Vec2f(GetScreenSizeXf() / 2, 0));
		SetAlignmentEntity(pBG, ALIGNMENT_UPPER_CENTER);
	}

	//handle screen resizing
	GetBaseApp()->m_sig_onScreenSizeChanged.disconnect_all_slots();
	GetBaseApp()->m_sig_onScreenSizeChanged.connect(BaseGameMenuOnScreenSizeChanged);
	pMenu->GetFunction("OnScreenSizeChanged")->sig_function.connect(GameMenuOnScreenSizeChanged);

	//create the game itself
	Entity* pLevel = pMenu->AddEntity(new Entity("Level"));
	//This will create everything necesarry for this game to work
	pLevel->AddComponent(new EggManager);

	//Create box with score and lives left
	InfoBoxCreate(pMenu);

	SlideScreen(pMenu, true);
	return pMenu;
}