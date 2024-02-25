#include "PlatformPrecomp.h"
#include "PauseMenu.h"
#include "Entity/EntityUtils.h"
#include "GUI/GUIUtils.h"
#include "GUI/GameMenu.h"

void PauseMenuOnSelect(VariantList* pVList)
{
	Entity* pEntClicked = pVList->m_variant[1].GetEntity();
	Entity* pGameMenu = pEntClicked->GetParent()->GetParent();
	if (!pGameMenu) return;
	if (pGameMenu->GetName() != "GameMenu") return;

	if (pEntClicked->GetName() == "continue")
	{
		DisableAllButtonsEntity(pEntClicked->GetParent());
		FadeEntity(pEntClicked->GetParent(), true, 0, 500);
		GetMessageManager()->CallEntityFunction(pEntClicked->GetParent(), 500, "OnDelete");

		GetAudioManager()->StopMusic();
		AudioHandle music = GetAudioManager()->Play(GET_THEMEMGR->GetFilename("audio/mp3/game.mp3"), true, true, false, true);
		GetAudioManager()->SetPos(music, pEntClicked->GetParent()->GetVar("musicPos")->GetUINT32());
		VariantList vlist(uint32(false));
		GetMessageManager()->CallEntityFunction(pGameMenu, 500, "OnPause", &vlist);
		return;
	}

	if (pEntClicked->GetName() == "end")
	{
		DisableAllButtonsEntity(pEntClicked->GetParent());
		FadeEntity(pEntClicked->GetParent(), true, 0, 200);
		GetMessageManager()->CallEntityFunction(pEntClicked->GetParent(), 200, "OnDelete");

		pGameMenu->GetFunction("OnGameEnd")->sig_function(NULL);
		return;
	}
}

Entity* PauseMenuCreate(Entity* pParentEnt, bool bAnimations)
{
	uint32 musicPos = GetAudioManager()->GetPos(GetAudioManager()->GetLastMusicID());
	GetAudioManager()->StopMusic();
	GetMessageManager()->SendGame(MESSAGE_TYPE_PLAY_MUSIC_FORCE_STREAMING, GET_THEMEMGR->GetFilename("audio/mp3/pause.mp3"), 250);

	Entity* pMenu = pParentEnt->AddEntity(new Entity("PauseMenu"));
	pMenu->GetVar("musicPos")->Set(musicPos);

	eFont font;
	float fontScale;
	GetFontAndScaleToFitThisLinesPerScreenY(&font, &fontScale, 10);
	Entity* pTitle = CreateTextLabelEntity(pMenu, "title", GetScreenSizeXf() / 2, iPadMapY(15), GET_LOCTEXT("{PAUSE_TITLE}"));
	SetupTextEntity(pTitle, font, fontScale);
	pTitle->GetVar("color")->Set(GET_THEMEMGR->GetTextColor());
	SetTextShadowColor(pTitle, GET_THEMEMGR->GetTextShadowColor());
	SetAlignmentEntity(pTitle, ALIGNMENT_UPPER_CENTER);

	eFont buttonFont;
	float buttonFontScale;
	GetFontAndScaleToFitThisLinesPerScreenY(&buttonFont, &buttonFontScale, 13);

	CL_Vec2f buttonPos = GetApp()->GetFont(buttonFont)->MeasureText(GET_LOCTEXT("{PAUSE_CONTINUE}"), buttonFontScale);
	buttonPos.x /= -2;
	buttonPos.y /= -1;
	buttonPos.y -= 10 + iPadMapY(5);
	buttonPos += GetScreenSize() / 2;
	Entity* pContinue = CreateTextButtonEntity(pMenu, "continue", buttonPos.x, buttonPos.y, GET_LOCTEXT("{PAUSE_CONTINUE}"), false);
	pContinue->GetFunction("OnButtonSelected")->sig_function.connect(PauseMenuOnSelect);
	SetupTextEntity(pContinue, buttonFont, buttonFontScale);
	pContinue->GetVar("color")->Set(GET_THEMEMGR->GetTextColor());
	Entity* pContinueRect = AddRectAroundEntity(pContinue, GET_THEMEMGR->GetPrimaryColor(), GET_THEMEMGR->GetSecondaryColor(), 10, true, buttonFontScale, buttonFont);
	SetTextShadowColor(pContinue, GET_THEMEMGR->GetTextShadowColor());
	AddHotKeyToButton(pContinue, VIRTUAL_KEY_BACK);
	AddHotKeyToButton(pContinue, 'P');

	buttonPos = GetApp()->GetFont(buttonFont)->MeasureText(GET_LOCTEXT("{PAUSE_END}"), buttonFontScale);
	buttonPos.x /= -2;
	buttonPos.y = 10 + iPadMapY(5);
	buttonPos += GetScreenSize() / 2;
	Entity* pEnd = CreateTextButtonEntity(pMenu, "end", buttonPos.x, buttonPos.y, GET_LOCTEXT("{PAUSE_END}"), false);
	pEnd->GetFunction("OnButtonSelected")->sig_function.connect(PauseMenuOnSelect);
	SetupTextEntity(pEnd, buttonFont, buttonFontScale);
	pEnd->GetVar("color")->Set(GET_THEMEMGR->GetTextColor());
	Entity* pEndRect = AddRectAroundEntity(pEnd, GET_THEMEMGR->GetPrimaryColor(), GET_THEMEMGR->GetSecondaryColor(), 10, true, buttonFontScale, buttonFont);
	SetTextShadowColor(pEnd, GET_THEMEMGR->GetTextShadowColor());

	if (GetApp()->GetGameMode() == GAMEMODE_MATH)
	{
		//adding a hint in the pause menu
		int ans = GetEntityRoot()->GetEntityByName("Level")->GetVar("currentAns")->GetINT32();
		Entity* pCurAns = CreateTextLabelEntity(pMenu, "ans", GetScreenSizeXf() - 10, GetScreenSizeYf() - 10, to_string(ans));
		eFont ansFont;
		float ansScale;
		GetFontAndScaleToFitThisLinesPerScreenY(&ansFont, &ansScale, 25);
		SetupTextEntity(pCurAns, ansFont, ansScale);
		SetAlignmentEntity(pCurAns, ALIGNMENT_DOWN_RIGHT);
	}

	Entity* pDarken = CreateOverlayRectEntity(pMenu, CL_Vec2f(0, 0), GetScreenSize(), MAKE_RGBA(0, 0, 0, 200));
	pMenu->MoveEntityToBottomByAddress(pDarken);
	if (bAnimations)
	{
		FadeInEntity(pMenu, true, 500);
		//continue button might get triggered when calling pause menu, so we disable buttons until they are fully visible
		DisableAllButtonsEntity(pMenu);
		EnableAllButtonsEntity(pMenu, true, 600);
	}
	return pMenu;
}