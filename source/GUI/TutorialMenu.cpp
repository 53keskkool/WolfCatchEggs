#include "PlatformPrecomp.h"
#include "TutorialMenu.h"
#include "Entity/EntityUtils.h"
#include "GUI/GUIUtils.h"
#include "GameMenu.h"
#include "MainMenu.h"
#ifdef PLATFORM_HTML5
#include <emscripten/emscripten.h>
#endif

Entity* CreateTutorialPart(uint32 partNum, bool bAnimations = true);

void TutorialMenuOnTouchStart(VariantList* pVList)
{
	Entity* pMenu = GetEntityRoot()->GetEntityByName("TutorialMenu");
	if (!pMenu) return;
	Entity* pContinue = pMenu->GetEntityByName("cont");
	if (!pContinue) return;
	if (pContinue->GetVar("alpha")->GetFloat() < 0.4f) return;

	Entity* pPart = pMenu->GetEntityByName("part");
	pPart->SetName("oldPart");
	FadeOutAndKillEntity(pPart);
	pContinue->SetName("oldCont");
	FadeOutAndKillEntity(pContinue);

	pPart = CreateTutorialPart(pPart->GetVar("part")->GetUINT32() + 1);
	if (!pPart)
	{
		pMenu->GetFunction("OnTouchStart")->sig_function.disconnect_all_slots();
		DisableAllButtonsEntity(pMenu);
		SlideScreen(pMenu, false);
		GetMessageManager()->CallEntityFunction(pMenu, 500, "OnDelete", NULL);
		GameMenuCreate(pMenu->GetParent());
		return;
	}
	FadeInEntity(pPart, true, 100, 300);
}

void TutorialMenuOnButtonClicked(VariantList* pVList)
{
	Entity* pEntClicked = pVList->m_variant[1].GetEntity();
	Entity* pMenu = GetEntityRoot()->GetEntityByName("TutorialMenu");
	if (!pMenu) return;

	if (pEntClicked->GetName() == "No")
	{
		DisableAllButtonsEntity(pMenu);
		SlideScreen(pMenu, false);

		GetMessageManager()->CallEntityFunction(pMenu, 500, "OnDelete", NULL);

		if (pMenu->GetVarWithDefault("fromPlay", uint32(false))->GetUINT32())
		{
			GameMenuCreate(pMenu->GetParent());
		}
		else
		{
			MainMenuCreate(pMenu->GetParent());
		}
		return;
	}
	if (pEntClicked->GetName() == "Yes")
	{
		Entity* pPart = pEntClicked->GetParent();
		DisableAllButtonsEntity(pPart);
		FadeOutAndKillEntity(pPart);

		pMenu->AddComponent(new TouchHandlerComponent);
		pMenu->GetFunction("OnTouchStart")->sig_function.connect(TutorialMenuOnTouchStart);

		CreateTutorialPart(1);
		return;
	}
}

Entity* CreateTutorialPart(uint32 partNum, bool bAnimations)
{
	const uint8 partAmount = 5;
	if (partNum > partAmount) return NULL; //tutorial ended

	Entity* pMenu = GetEntityRoot()->GetEntityByName("TutorialMenu");
	if (!pMenu) return NULL;

	//we'll use 'parts' for tutorial. we'll fade out old ones and fade in new ones and they'll keep track of what part we are at rn
	Entity* pPart = pMenu->AddEntity(new Entity("part"));
	pPart->GetVar("part")->Set(partNum);

	Entity* pText = NULL;
	eFont font;
	float scale;
	GetFontAndScaleToFitThisLinesPerScreenY(&font, &scale, 15);

	switch (partNum)
	{
	case 0: //first question, does person want to go through tutorial or not
	{
		Entity* pQuestion = CreateTextBoxEntity(pPart, "text", GetScreenSize() / 2, CL_Vec2f(GetScreenSizeXf() * 0.8f, 0), GET_LOCTEXT("{TUTORIAL_WANT}"), 1, ALIGNMENT_UPPER_CENTER);
		SetupTextEntity(pQuestion, font, scale);
		SetTextShadowColor(pQuestion, GET_THEMEMGR->GetTextShadowColor());
		SetAlignmentEntity(pQuestion, ALIGNMENT_CENTER);

		float buttonSpacer = iPadMapX(15);
		eFont buttonFont;
		float buttonFontScale;
		GetFontAndScaleToFitThisLinesPerScreenY(&buttonFont, &buttonFontScale, 13);

		Entity* pNo = CreateTextButtonEntity(pPart, "No", 0, 0, GET_LOCTEXT("{TUTORIAL_NO}"), false);
		pNo->GetFunction("OnButtonSelected")->sig_function.connect(TutorialMenuOnButtonClicked);
		SetupTextEntity(pNo, buttonFont, buttonFontScale);
		pNo->GetVar("color")->Set(GET_THEMEMGR->GetTextColor());
		Entity* pNoRect = AddRectAroundEntity(pNo, GET_THEMEMGR->GetPrimaryColor(), GET_THEMEMGR->GetSecondaryColor(), 10, true, buttonFontScale, buttonFont);
		pNoRect->SetName("no_");
		SetTextShadowColor(pNo, GET_THEMEMGR->GetTextShadowColor());
		AddHotKeyToButton(pNo, VIRTUAL_KEY_BACK);

		Entity* pYes = CreateTextButtonEntity(pPart, "Yes", GetSize2DEntity(pNoRect).x + buttonSpacer, 0, GET_LOCTEXT("{TUTORIAL_YES}"), false);
		pYes->GetFunction("OnButtonSelected")->sig_function.connect(TutorialMenuOnButtonClicked);
		SetupTextEntity(pYes, buttonFont, buttonFontScale);
		pYes->GetVar("color")->Set(GET_THEMEMGR->GetTextColor());
		Entity* pYesRect = AddRectAroundEntity(pYes, GET_THEMEMGR->GetPrimaryColor(), GET_THEMEMGR->GetSecondaryColor(), 10, true, buttonFontScale, buttonFont);
		pYesRect->SetName("yes_");
		SetTextShadowColor(pYes, GET_THEMEMGR->GetTextShadowColor());
		AddHotKeyToButton(pYes, 13); //enter

		//aligning buttons (i could do it somehow easier, but this way it looks good)
		float totalWidthOfButtons = GetSize2DEntity(pNoRect).x + buttonSpacer + GetSize2DEntity(pYesRect).x;
		CL_Vec2f buttonsMove;
		buttonsMove.x = (GetScreenSizeXf() / 2) - (totalWidthOfButtons / 2) - GetPos2DEntity(pNoRect).x;
		buttonsMove.y = GetPos2DEntity(pQuestion).y + GetSize2DEntity(pQuestion).y / 2 + iPhoneMapY(20);
		pNo->GetVar("pos2d")->Set(GetPos2DEntity(pNo) + buttonsMove); //doing this way, so onchangedsig gets called
		pNoRect->GetVar("pos2d")->GetVector2() += buttonsMove;
		pYes->GetVar("pos2d")->Set(GetPos2DEntity(pYes) + buttonsMove); //doing this way, so onchangedsig gets called
		pYesRect->GetVar("pos2d")->GetVector2() += buttonsMove;
	}
	return pPart;

	case 1:
	{
#ifdef PLATFORM_HTML5
		bool bTouchscreen = EM_ASM_INT({
			return (('ontouchstart' in window) || (navigator.maxTouchPoints > 0) || (navigator.msMaxTouchPoints > 0));
			});
#else
		bool bTouchscreen = !IsDesktop();
#endif
		if (bTouchscreen)
		{
			pText = CreateTextBoxEntity(pPart, "text", GetScreenSize() / 2, CL_Vec2f(GetScreenSizeXf() * 0.8f, 0), GET_LOCTEXT("{TUTORIAL_MSG1_TOUCH}"), 1, ALIGNMENT_UPPER_CENTER);
			SetupTextEntity(pText, font, scale);
			SetTextShadowColor(pText, GET_THEMEMGR->GetTextShadowColor());
			SetAlignmentEntity(pText, ALIGNMENT_CENTER);

			if (bAnimations) FadeInEntity(pPart, true, 100, 300);

			CL_Vec2f partSize = GetScreenSize() / 2;
			Entity* pScreenPart = CreateOverlayRectEntity(pPart, CL_Vec2f(0, 0), partSize, MAKE_RGBA(0xF0, 0xF0, 0xF0, 0x10));
			pScreenPart->GetComponentByName("RectRender")->GetVar("bmpBorderFileName")->Set(GET_THEMEMGR->GetFilename("interface/gui_box_white.rttex"));
			pScreenPart->GetComponentByName("RectRender")->GetVar("borderColor")->Set(MAKE_RGBA(0x80, 0x80, 0x80, 0x80));
			if (bAnimations) FadeInEntity(pScreenPart, true, 200, 250);

			pScreenPart = CreateOverlayRectEntity(pPart, CL_Vec2f(partSize.x, 0), partSize, MAKE_RGBA(0xF0, 0xF0, 0xF0, 0x10));
			pScreenPart->GetComponentByName("RectRender")->GetVar("bmpBorderFileName")->Set(GET_THEMEMGR->GetFilename("interface/gui_box_white.rttex"));
			pScreenPart->GetComponentByName("RectRender")->GetVar("borderColor")->Set(MAKE_RGBA(0x80, 0x80, 0x80, 0x80));
			if (bAnimations) FadeInEntity(pScreenPart, true, 200, 350);

			pScreenPart = CreateOverlayRectEntity(pPart, partSize, partSize, MAKE_RGBA(0xF0, 0xF0, 0xF0, 0x10));
			pScreenPart->GetComponentByName("RectRender")->GetVar("bmpBorderFileName")->Set(GET_THEMEMGR->GetFilename("interface/gui_box_white.rttex"));
			pScreenPart->GetComponentByName("RectRender")->GetVar("borderColor")->Set(MAKE_RGBA(0x80, 0x80, 0x80, 0x80));
			if (bAnimations) FadeInEntity(pScreenPart, true, 200, 450);

			pScreenPart = CreateOverlayRectEntity(pPart, CL_Vec2f(0, partSize.y), partSize, MAKE_RGBA(0xF0, 0xF0, 0xF0, 0x10));
			pScreenPart->GetComponentByName("RectRender")->GetVar("bmpBorderFileName")->Set(GET_THEMEMGR->GetFilename("interface/gui_box_white.rttex"));
			pScreenPart->GetComponentByName("RectRender")->GetVar("borderColor")->Set(MAKE_RGBA(0x80, 0x80, 0x80, 0x80));
			if (bAnimations) FadeInEntity(pScreenPart, true, 200, 550);

			pPart->MoveEntityToTopByAddress(pText);
		}
		else
		{
			pText = CreateTextBoxEntity(pPart, "text", GetScreenSize() / 2, CL_Vec2f(GetScreenSizeXf() * 0.8f, 0), GET_LOCTEXT("{TUTORIAL_MSG1}"), 1, ALIGNMENT_UPPER_CENTER);
			SetupTextEntity(pText, font, scale);
			SetTextShadowColor(pText, GET_THEMEMGR->GetTextShadowColor());
			SetAlignmentEntity(pText, ALIGNMENT_CENTER);
		}
	}
	break;

	//part with just text
	default:
		pText = CreateTextBoxEntity(pPart, "text", GetScreenSize() / 2, CL_Vec2f(GetScreenSizeXf() * 0.8f, 0), GET_LOCTEXT("{TUTORIAL_MSG" + to_string(partNum) + "}"), 1, ALIGNMENT_UPPER_CENTER);
		SetupTextEntity(pText, font, scale);
		SetTextShadowColor(pText, GET_THEMEMGR->GetTextShadowColor());
		SetAlignmentEntity(pText, ALIGNMENT_CENTER);
		break;
	}

	GetFontAndScaleToFitThisLinesPerScreenY(&font, &scale, 20);
	eAlignment continueAlignment = ALIGNMENT_DOWN_CENTER;
	CL_Vec2f textPos;
	textPos.x = GetScreenSizeXf() / 2;
	if (pText)
	{
		textPos.y = GetPos2DEntity(pText).y + GetSize2DEntity(pText).y / 2 + iPhoneMapY(10);
		continueAlignment = ALIGNMENT_UPPER_CENTER;
	}
	else
	{
		textPos.y = GetScreenSizeYf() - iPhoneMapY(30);
	}
	string contText = "{TUTORIAL_CONT}";
	if (partNum >= partAmount) contText = "{TUTORIAL_BEGIN}";
	Entity* pCont = CreateTextLabelEntity(pMenu, "cont", textPos.x, textPos.y, GET_LOCTEXT(contText));
	SetupTextEntity(pCont, font, scale);
	SetTextShadowColor(pCont, GET_THEMEMGR->GetTextShadowColor());
	SetAlignmentEntity(pCont, continueAlignment);
	if (bAnimations) FadeInEntity(pCont, true, 300, 1100);

	return pPart;
}

void TutorialMenuCreatePart(VariantList* pVList)
{
	CreateTutorialPart(pVList->Get(0).GetUINT32(), pVList->Get(1).GetUINT32());
}

void TutorialMenuOnScreenSizeChanged()
{
	Entity* pMenu = GetEntityRoot()->GetEntityByName("TutorialMenu");
	if (!pMenu) return;
	SetSize2DEntity(pMenu, GetScreenSize());
	uint8 currentPart = pMenu->GetEntityByName("part")->GetVar("part")->GetUINT32();
	pMenu->RemoveAllEntities();
	VariantList vlist(uint32(currentPart), uint32(false));
	GetMessageManager()->CallStaticFunction(TutorialMenuCreatePart, 50, &vlist);
}

Entity* TutorialMenuCreate(Entity* pParentEnt)
{
	GetAudioManager()->StopMusic();
	Entity* pMenu = pParentEnt->AddEntity(new Entity("TutorialMenu"));
	SetSize2DEntity(pMenu, GetScreenSize());
	AddFocusIfNeeded(pMenu, true, 500);
	GetApp()->GetVar("tutorial")->Set(uint32(true));

	CreateTutorialPart(0);

	//handle screen resizing
	GetBaseApp()->m_sig_onScreenSizeChanged.disconnect_all_slots();
	GetBaseApp()->m_sig_onScreenSizeChanged.connect(TutorialMenuOnScreenSizeChanged);

	SlideScreen(pMenu, true);
	return pMenu;
}