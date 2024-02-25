#include "PlatformPrecomp.h"
#include "OptionsMenu.h"
#include "MainMenu.h"
#include "ThemeSelectMenu.h"
#include "Entity/EntityUtils.h"
#include "Entity/RenderScissorComponent.h"
#include "GUIUtils.h"
#ifdef PLATFORM_HTML5
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>
#endif

void CreateOptionsMenu(VariantList* pVList)
{
	bool bAnimations = true;
	if (pVList)
	{
		if (pVList->Get(0).GetType() == Variant::TYPE_UINT32)
		{
			bAnimations = pVList->Get(0).GetUINT32();
		}
	}
	OptionsMenuCreate(GetEntityRoot(), bAnimations);
}

void OptionsMenuOnSelect(VariantList* pVList) //0=vec2 point of click, 1=entity sent from
{
	Entity* pEntClicked = pVList->Get(1).GetEntity();
	LogMsg("Clicked %s entity at %s", pEntClicked->GetName().c_str(), pVList->m_variant[1].Print().c_str());
	Entity* pMenu = GetEntityRoot()->GetEntityByName("OptionsMenu");
	string buttonName = pEntClicked->GetName();

	App* App = GetApp();
	BaseApp* BaseApp = GetBaseApp();

	if (buttonName == "sound_1") {
		App->GetVar("soundDisabled")->Set((uint32)false);
		GetAudioManager()->SetSoundEnabled(true);
		return;
	}
	if (buttonName == "sound_0") {
		App->GetVar("soundDisabled")->Set((uint32)true);
		GetAudioManager()->StopMusic();
		GetAudioManager()->SetSoundEnabled(false);
		return;
	}
	if (buttonName == "check_fullscreen") {
		bool fullscreen = pEntClicked->GetVar("checked")->GetUINT32();
		App->GetVar("fullscreen")->Set((uint32)fullscreen);
		GetBaseApp()->OnFullscreenToggleRequest();
		GetMessageManager()->CallEntityFunction(pMenu, 0, "OnDelete");
		GetMessageManager()->CallStaticFunction(CreateOptionsMenu, 10);
		return;
	}
	if (buttonName == "Back")
	{
		DisableAllButtonsEntity(pMenu);
		SlideScreen(pEntClicked->GetParent(), false);
		GetMessageManager()->CallEntityFunction(pEntClicked->GetParent(), 500, "OnDelete", NULL);
		MainMenuCreate(pEntClicked->GetParent()->GetParent());
		GetApp()->Save();
		return;
	}

	GetEntityRoot()->PrintTreeAsText();
}

void OnProgressChangedMusic(Variant* pVar) {
	GetApp()->GetVar("music_vol")->Set(pVar->GetFloat());
	GetAudioManager()->SetMusicVol(pVar->GetFloat());
}

void OnProgressChangedSFX(Variant* pVar) {
	GetApp()->GetVar("sfx_vol")->Set(pVar->GetFloat());
	GetAudioManager()->SetDefaultVol(pVar->GetFloat());
}

void OnProgressChangedDif(Variant* pVar) {
	GetApp()->GetVar("dif")->Set(pVar->GetFloat());
}

void OptionsMenuAddScrollContent(Entity* pParent)
{
	pParent = pParent->GetEntityByName("scroll_child");
	pParent->RemoveAllEntities(); //clear it out in case we call this more than once, say, to update/change something

	App* App = GetApp();
	BaseApp* BaseApp = GetBaseApp();

	float offsetX = iPhoneMapX(5);
	float x = offsetX;
	float y = 0;
	float spacerX = iPhoneMapX(46); //space between thingies
	float spacerY = iPhoneMapY(20); //space between thingies
	eFont titleFont;
	float titleScale;
	GetFontAndScaleToFitThisLinesPerScreenY(&titleFont, &titleScale, 8.0f);
	eFont font;
	float scale;
	GetFontAndScaleToFitThisLinesPerScreenY(&font, &scale, 23.0f);
	float LineHeight = BaseApp->GetFont(font)->GetLineHeight(scale);
	vector<Entity*> pTxts;

	//first, a title in a big font
	Entity* pTitle = CreateTextLabelEntity(pParent, "Title", x, 0, GET_LOCTEXT("{MAINMENU_OPTIONS}"));
	SetupTextEntity(pTitle, titleFont, titleScale);
	pTitle->GetVar("color")->Set(GET_THEMEMGR->GetTextColor());
	y += pTitle->GetVar("size2d")->GetVector2().y + spacerY + iPhoneMapY2X(26);

	EntityComponent* pDif = CreateSlider(pParent, x, y, iPhoneMapX(360), GET_THEMEMGR->GetFilename("interface/slider_button.rttex"), GET_LOCTEXT("{OPTIONSMENU_MIN}"), GET_LOCTEXT("{OPTIONSMENU_DIF}"), GET_LOCTEXT("{OPTIONSMENU_MAX}"));
	float dif = App->GetVarWithDefault("dif", 0.5f)->GetFloat();
	pDif->GetVar("progress")->Set(dif);
	pDif->GetVar("progress")->GetSigOnChanged()->connect(OnProgressChangedDif);
	pTxts.clear();
	pParent->GetEntitiesByName(&pTxts, "txt");
	for (auto& pTxt : pTxts)
	{
		SetupTextEntity(pTxt, font, scale);
	}
	y += spacerY + LineHeight;

	Entity* pAudioTitle = CreateTextLabelEntity(pParent, "", x, y, GET_LOCTEXT("{OPTIONSMENU_AUDIO}:"));
	SetupTextEntity(pAudioTitle, font, scale);
	pAudioTitle->GetVar("color")->Set(GET_THEMEMGR->GetTextColor());

	Entity* pAudioON = CreateTextButtonEntity(pParent, "sound_1", iPhoneMapX(140), y, GET_LOCTEXT("{OPTIONSMENU_AUDIO_ON}"), false);
	SetupTextEntity(pAudioON, font, scale);
	pAudioON->GetVar("color")->Set(GET_THEMEMGR->GetTextColor());
	pAudioON->GetFunction("OnButtonSelected")->sig_function.connect(OptionsMenuOnSelect);
	SetButtonClickSound(pAudioON, "");
	CL_Vec2f audioONSize = GetSize2DEntity(pAudioON);

	Entity* pAudioOFF = CreateTextButtonEntity(pParent, "sound_0", iPhoneMapX(140) + spacerX + audioONSize.x, y, GET_LOCTEXT("{OPTIONSMENU_AUDIO_OFF}"), false);
	SetupTextEntity(pAudioOFF, font, scale);
	pAudioOFF->GetVar("color")->Set(GET_THEMEMGR->GetTextColor());
	pAudioOFF->GetFunction("OnButtonSelected")->sig_function.connect(OptionsMenuOnSelect);
	SetButtonClickSound(pAudioOFF, "");
	CL_Vec2f audioOFFSize = GetSize2DEntity(pAudioOFF);

	bool soundDisabled = !App->GetVar("soundDisabled")->GetUINT32();
	SetupLightBarSelect(pParent, "sound_", soundDisabled, MAKE_RGBA(0xBE, 0x00, 0x23, 0xFF));

	y += spacerY + LineHeight + iPhoneMapY2X(26);

	EntityComponent* pMusicVolume = CreateSlider(pParent, x, y, iPhoneMapX(360), GET_THEMEMGR->GetFilename("interface/slider_button.rttex"), GET_LOCTEXT("{OPTIONSMENU_MIN}"), GET_LOCTEXT("{OPTIONSMENU_MUSIC_VOL}"), GET_LOCTEXT("{OPTIONSMENU_MAX}"));
	float musicVol = App->GetVar("music_vol")->GetFloat();
	pMusicVolume->GetVar("progress")->Set(musicVol);
	pMusicVolume->GetVar("progress")->GetSigOnChanged()->connect(OnProgressChangedMusic);
	pTxts.clear();
	pParent->GetEntitiesByName(&pTxts, "txt");
	for (auto& pTxt : pTxts)
	{
		SetupTextEntity(pTxt, font, scale);
	}

	y += spacerY + LineHeight + iPhoneMapY2X(26);

	EntityComponent* pSFXVolume = CreateSlider(pParent, x, y, iPhoneMapX(360), GET_THEMEMGR->GetFilename("interface/slider_button.rttex"), GET_LOCTEXT("{OPTIONSMENU_MIN}"), GET_LOCTEXT("{OPTIONSMENU_SFX_VOL}"), GET_LOCTEXT("{OPTIONSMENU_MAX}"));
	float sfxVol = App->GetVar("sfx_vol")->GetFloat();
	pSFXVolume->GetVar("progress")->Set(sfxVol);
	pSFXVolume->GetVar("progress")->GetSigOnChanged()->connect(OnProgressChangedSFX);
	pTxts.clear();
	pParent->GetEntitiesByName(&pTxts, "txt");
	for (auto& pTxt : pTxts)
	{
		SetupTextEntity(pTxt, font, scale);
	}

	y += spacerY + LineHeight;

	pParent->GetEntitiesByName(&pTxts, "txt");
	for (auto& pTxt : pTxts)
	{
		pTxt->GetVar("color")->Set(GET_THEMEMGR->GetTextColor());
	}

#ifdef PLATFORM_HTML5
	if (GetApp()->IsDesktop())
	{
		EmscriptenFullscreenChangeEvent fullscreenStatus;
		emscripten_get_fullscreen_status(&fullscreenStatus);
		bool fullscreen = fullscreenStatus.isFullscreen;
#else
	if (IsDesktop())
	{
		bool fullscreen = App->GetVar("fullscreen")->GetUINT32();
#endif
		Entity* pFullscreen = CreateCheckbox(pParent, "check_fullscreen", GET_LOCTEXT("{OPTIONSMENU_FULLSCREEN}"), x, y, fullscreen, font, scale);
		pFullscreen->GetEntityByName("_textcheck_fullscreen")->GetVar("color")->Set(GET_THEMEMGR->GetTextColor());
		pFullscreen->GetFunction("OnButtonSelected")->sig_function.connect(OptionsMenuOnSelect);
		y += spacerY + MeasureEntityAndChildren(pFullscreen).get_height();
	}

	VariantList vList(pParent->GetParent());
	ResizeScrollBounds(&vList);
}

void OptionsMenuOnScreenSizeChanged()
{
	Entity* pMenu = GetEntityRoot()->GetEntityByName("OptionsMenu");
	if (!pMenu) return;

	pMenu->SetName("OldOptionsMenu");
	pMenu->SetTaggedForDeletion();
	VariantList vlist(uint32(false));
	GetMessageManager()->CallStaticFunction(CreateOptionsMenu, 10, &vlist);
}

Entity* OptionsMenuCreate(Entity* pParentEnt, bool bAnimations) {
	Entity* pBG = NULL;
	pBG = CreateOverlayEntity(pParentEnt, "OptionsMenu", GET_THEMEMGR->GetFilename("interface/menu_bg.rttex"), 0, 0);
	EntitySetScaleBySize(pBG, GetScreenSize());
	AddFocusIfNeeded(pBG, true, 500);

	CL_Vec2f vTextAreaPos = iPhoneMap(2, 10);
	float offsetFromBottom = iPhoneMapY(48);
	float offsetFromRight = iPhoneMapY(0);

	CL_Vec2f vTextAreaBounds = (GetScreenSize() - CL_Vec2f(offsetFromRight, offsetFromBottom)) - vTextAreaPos;
	Entity* pScroll = pBG->AddEntity(new Entity("scroll"));
	pScroll->GetVar("pos2d")->Set(vTextAreaPos);
	pScroll->GetVar("size2d")->Set(vTextAreaBounds);
	pScroll->AddComponent(new TouchHandlerComponent);
	pScroll->GetVar("color")->Set(GET_THEMEMGR->GetScrollbarColor());

	EntityComponent* pScrollComp = pScroll->AddComponent(new ScrollComponent);

	//turn on finger tracking enforcement, it means it will mark the tap as "handled" when touched.  Doesn't make a difference here,
	//but good to know about in some cases.  (some entity types will ignore touches if they've been marked as "Handled")

	pScrollComp->GetVar("fingerTracking")->Set(uint32(1));

	//note: If you don't want to see a scroll bar progress indicator, comment out the next line.
	EntityComponent* pScrollBarComp = pScroll->AddComponent(new ScrollBarRenderComponent); 	//add a visual way to see the scroller position

	Entity* pScrollChild = pScroll->AddEntity(new Entity("scroll_child"));

	pScroll->AddComponent(new RenderScissorComponent()); //so the text/etc won't get drawn outside our scroll box
	pScroll->AddComponent(new FilterInputComponent); //lock out taps that are not in our scroll area

	//actually add all our content that we'll be scrolling (if there is too much for one screen), as much as we want, any kind of entities ok
	OptionsMenuAddScrollContent(pBG);

	eFont font;
	float fontScale;
	GetFontAndScaleToFitThisLinesPerScreenY(&font, &fontScale, 13);

	//oh, let's put the Back button on the bottom bar thing
	float buttonY = GetScreenSizeYf() - iPhoneMapY(40);
	Entity* pEnt = CreateTextButtonEntity(pBG, "Back", iPhoneMapX(5), buttonY, GET_LOCTEXT("{BACK}"), false);
	pEnt->GetFunction("OnButtonSelected")->sig_function.connect(&OptionsMenuOnSelect);
	SetupTextEntity(pEnt, font, fontScale);
	pEnt->GetVar("color")->Set(GET_THEMEMGR->GetTextColor());
	AddRectAroundEntity(pEnt, GET_THEMEMGR->GetPrimaryColor(), GET_THEMEMGR->GetSecondaryColor(), iPadMapY(20), true, fontScale, font);
	SetTextShadowColor(pEnt, GET_THEMEMGR->GetTextShadowColor());
	AddHotKeyToButton(pEnt, VIRTUAL_KEY_BACK); //for androids back button and window's Escape button

	//handle screen resizing
	GetBaseApp()->m_sig_onScreenSizeChanged.disconnect_all_slots();
	GetBaseApp()->m_sig_onScreenSizeChanged.connect(OptionsMenuOnScreenSizeChanged);

	//slide it in with movement
	if (bAnimations)
	{
		SlideScreen(pBG, true, 500);
	}
	return pBG;
}