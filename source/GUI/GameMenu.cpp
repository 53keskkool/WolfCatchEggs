#include "PlatformPrecomp.h"
#include "GameMenu.h"
#include "Entity/EntityUtils.h"
#include "Entity/ArcadeInputComponent.h"
#include "Entity/TouchHandlerComponent.h"
#include "Component/EggManager.h"

void OnScoreUpdate(VariantList* pVList)
{
	Entity* pInfo = GetEntityRoot()->GetEntityByName("InfoBox");
	if (!pInfo) return; //that's strange

	SetTextEntityByName("Score", GET_LOCTEXT("{SCORE}: ") + std::to_string(pVList->Get(0).GetUINT32()), pInfo);
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

Entity* InfoBoxCreate(Entity* pParentEnt)
{
	float floorHeight = (GetScale2DEntity(pParentEnt->GetEntityByName("BG")).y * GET_THEMEMGR->GetFloorHeight()) - 10;

	Entity* pBG = CreateOverlayRectEntity(pParentEnt, CL_Vec2f(0, 0), CL_Vec2f(100, 100), ColorCombine(GET_THEMEMGR->GetPrimaryColor(), MAKE_RGBA(0xF0, 0xF0, 0xF0, 0xA0)));
	pBG->GetComponentByName("RectRender")->GetVar("bmpBorderFileName")->Set(GET_THEMEMGR->GetFilename("interface/gui_box_white.rttex"));
	pBG->GetComponentByName("RectRender")->GetVar("borderColor")->Set(GET_THEMEMGR->GetPrimaryColor());
	pBG->GetVar("finishMenuName")->Set(pParentEnt->GetName());
	pBG->SetName("InfoBox");

	eFont font;
	float fontScale;
	GetFontAndScaleToFitThisPixelHeight(&font, &fontScale, floorHeight - 20);
	Entity* pText = CreateTextLabelEntity(pBG, "Score", 10, 10, GET_LOCTEXT("{SCORE}: 000"));
	SetupTextEntity(pText, font, fontScale);
	pText->GetVar("color")->Set(GET_THEMEMGR->GetTextColor());
	SetTextShadowColor(pText, GET_THEMEMGR->GetTextShadowColor());

	float spacer = iPadMapX(50);
	float eggSpacer = iPadMapX(10);
	float eggX = GetSize2DEntity(pText).x + spacer;
	float eggWidth = 0;
	CL_Vec2f eggScale;
	Entity* pLive = CreateOverlayEntity(pBG, "Live3", GET_THEMEMGR->GetFilename("game/egg.rttex"), eggX, 10);
	EntitySetScaleBySize(pLive, CL_Vec2f(0, floorHeight - 20), true, true);
	eggScale = GetScale2DEntity(pLive);
	eggWidth = GetSize2DEntity(pLive).x;
	eggX += eggSpacer + eggWidth;
	pLive = CreateOverlayEntity(pBG, "Live2", GET_THEMEMGR->GetFilename("game/egg.rttex"), eggX, 10);
	SetScale2DEntity(pLive, eggScale);
	eggX += eggSpacer + eggWidth;
	pLive = CreateOverlayEntity(pBG, "Live1", GET_THEMEMGR->GetFilename("game/egg.rttex"), eggX, 10);
	SetScale2DEntity(pLive, eggScale);
	eggX += eggSpacer + eggWidth;

	CL_Vec2f boxSize;
	boxSize.x = 10 + GetSize2DEntity(pText).x + spacer + eggSpacer * 2 + eggWidth * 3 + 10;
	boxSize.y = floorHeight + 20 + iPadMapY(50);
	SetSize2DEntity(pBG, boxSize);
	CL_Vec2f boxPos;
	boxPos.x = GetScreenSizeXf() / 2 - boxSize.x / 2;
	boxPos.y = GetScreenSizeYf() - floorHeight;
	SetPos2DEntity(pBG, boxPos);
	SetTextEntity(pText, GET_LOCTEXT("{SCORE}: 0"));

	Entity* pLevel = pParentEnt->GetEntityByName("Level");
	pLevel->GetFunction("OnScoreUpdate")->sig_function.connect(OnScoreUpdate);

	return pBG;
}


Entity* GameMenuCreate(Entity* pParentEnt)
{
	Entity* pMenu = pParentEnt->AddEntity(new Entity("GameMenu"));
	AddFocusIfNeeded(pMenu);
	SetSize2DEntity(pMenu, GetScreenSize());

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
	GetBaseApp()->m_sig_arcade_input.connect(pMenu->GetFunction("OnArcadeInput")->sig_function);
	//adding touch handler to listen for touchscreen/mouse clicks
	pMenu->AddComponent(new TouchHandlerComponent);

	Entity* pBG = CreateOverlayEntity(pMenu, "BG", GET_THEMEMGR->GetFilename("game/bg.rttex"), 0, 0);
	EntitySetScaleBySize(pBG, GetScreenSize());

	Entity* pLevel = pMenu->AddEntity(new Entity("Level"));
	//This will create everything necesarry for this game to work
	pLevel->AddComponent(new EggManager);

	//Create box with score and lives left
	InfoBoxCreate(pMenu);

	SlideScreen(pMenu, true);
	return pMenu;
}