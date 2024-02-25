#include "PlatformPrecomp.h"
#include "PhoneWebMenu.h"
#include "Entity/EntityUtils.h"
#include "GUI/MainMenu.h"
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>

EntityComponent* SetupInterpolateComponent(Entity* pEnt, const string& componentName, const string& varName, const Variant& targetValue, int durationMS, int delayBeforeStartMS, eInterpolateType interpolationType = INTERPOLATE_SMOOTHSTEP, InterpolateComponent::eOnFinish onFinish = InterpolateComponent::ON_FINISH_DIE, eTimingSystem timing = GetTiming());

void CreatePhoneWebMenu()
{
	PhoneWebMenuCreate();
}

void PhoneWebMenuStartPhoneAnimation(VariantList* pVList)
{
	Entity* pPhone = pVList->Get(0).GetEntity();
	int delay = 300;
	if (pPhone->GetVar("rotation")->GetFloat() != 0)
	{
		pPhone->GetVar("rotation")->Set(0.0f);
		delay = 0;
	}
	FadeInEntity(pPhone, true, 400, delay);
	delay += 400;
	SetupInterpolateComponent(pPhone, "ic_rotation", "rotation", Variant(90.0f), 1000, delay);
	delay += 1000;
	FadeOutEntity(pPhone, true, 400, delay);
	delay += 400;
	GetMessageManager()->CallEntityFunction(pPhone, delay + 200, "StartAnimation", pVList);
}

void PhoneWebMenuOnScreenSizeChanged(VariantList* pVList)
{
	Entity* pMenu = GetEntityRoot()->GetEntityByName("PhoneWebMenu");
	if (!pMenu) return;
	pMenu->SetName("OldPhoneWebMenu");
	pMenu->GetFunction("OnDelete")->sig_function(NULL);

	if (!PhoneWebMenuCreate(false))
	{
		if (GetEntityRoot()->GetEntityByName("GUI")->GetChildren()->size() < 1)
		{
			//we didn't enter the game yet
			GetMessageManager()->CallStaticFunction(CreateMainMenu, 10, NULL);
		}
	}
}

void PhoneWebMenuOnTouchStart(VariantList* pVList)
{
	//check if we should do it...
	Entity* pMenu = GetEntityRoot()->GetEntityByName("PhoneWebMenu");
	if (!pMenu) return;
	//we don't want player to press it tons of times
	pMenu->RemoveComponentByName("TouchHandler");

	//player pressed, now we can go into fullscreen
	GetApp()->OnFullscreenToggleRequest();
}

Entity* PhoneWebMenuCreate(bool bFirstTime)
{
	//first of all, do we need this menu at all? find problems we have now
	bool bOrientation = GetScreenSizeXf() < GetScreenSizeYf();
	EmscriptenFullscreenChangeEvent fullscreenStatus;
	emscripten_get_fullscreen_status(&fullscreenStatus);
	bool bFullscreen = !fullscreenStatus.isFullscreen && !GetApp()->IsIOS();

	//no problems were found, we don't need this menu to spawn
	if (!bOrientation && !bFullscreen) return NULL;

	Entity* pGame = GetEntityRoot()->GetEntityByName("GameMenu");
	if (pGame && !pGame->GetEntityByName("GameOverMenu"))
	{
		if (!pGame->GetVar("paused")->GetUINT32())
		{
			VariantList vlist(uint32(true));
			pGame->GetFunction("OnPause")->sig_function(&vlist);
		}
	}

	Entity* pMenu = CreateOverlayRectEntity(GetEntityRoot(), CL_Vec2f(0, 0), GetScreenSize(), MAKE_RGBA(0, 0, 0, 255));
	pMenu->SetName("PhoneWebMenu");
	AddFocusIfNeeded(pMenu, false, 0, 0, 2);
	pMenu->GetFunction("OnScreenSizeChanged")->sig_function.connect(PhoneWebMenuOnScreenSizeChanged);
	pMenu->AddComponent(new TouchHandlerComponent);

	eFont font;
	float fontScale;
	GetFontAndScaleToFitThisLinesPerScreenY(&font, &fontScale, 12);
	if (bOrientation)
	{
		Entity* pText = CreateTextBoxEntity(pMenu, "orientation", GetScreenSize() / 2, CL_Vec2f(GetScreenSizeXf() - iPhoneMapX(10), 0), GET_LOCTEXT("{PHONEWEB_ROTATE}"), fontScale, ALIGNMENT_UPPER_CENTER);
		SetupTextEntity(pText, font, fontScale);
		SetAlignmentEntity(pText, ALIGNMENT_CENTER);

		Entity* pPhone = CreateOverlayEntity(pMenu, "phone", GET_THEMEMGR->GetFilename("interface/phone.rttex"), 0, 0);
		EntitySetScaleBySize(pPhone, CL_Vec2f(0, GetScreenSizeYf() * 0.2f), true, true);
		SetAlignmentEntity(pPhone, ALIGNMENT_CENTER);
		CL_Vec2f phonePos = GetPos2DEntity(pText);
		phonePos.y += (GetSize2DEntity(pText).y / 2) + GetSize2DEntity(pPhone).y / 2 + iPhoneMapY(5);
		SetPos2DEntity(pPhone, phonePos);
		if (!bFirstTime) pPhone->GetVar("rotation")->Set(90.0f); //skip first delay

		pPhone->GetFunction("StartAnimation")->sig_function.connect(PhoneWebMenuStartPhoneAnimation);
		VariantList vlist(pPhone);
		PhoneWebMenuStartPhoneAnimation(&vlist);
	}
	else if (bFullscreen)
	{
		string text = "{PHONEWEB_BEGIN}";
		if (GetEntityRoot()->GetEntityByName("GUI")->GetChildren()->size() > 0) text = "{PHONEWEB_CONTINUE}";
		Entity* pText = CreateTextBoxEntity(pMenu, "fullscreen", GetScreenSize() / 2, CL_Vec2f(GetScreenSizeXf() - iPhoneMapX(10), 0), GET_LOCTEXT(text), fontScale, ALIGNMENT_UPPER_CENTER);
		SetupTextEntity(pText, font, fontScale);
		SetAlignmentEntity(pText, ALIGNMENT_CENTER);
		SetupInterpolateComponent(pText, "ic_fade", "alpha", 0.75f, 900, (bFirstTime ? 300 : 0), INTERPOLATE_SMOOTHSTEP, InterpolateComponent::ON_FINISH_BOUNCE);

		pMenu->GetFunction("OnTouchStart")->sig_function.connect(PhoneWebMenuOnTouchStart);
	}
	else
	{
		//how did we get here? why?
		pMenu->GetFunction("OnDelete")->sig_function(NULL);
		return NULL;
	}

	if (bFirstTime)
	{
		FadeInEntity(pMenu);
	}
	bFullscreen = false;
	return pMenu;
}