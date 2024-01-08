#include "PlatformPrecomp.h"
#include "AboutMenu.h"
#include "MainMenu.h"
#include "Entity/EntityUtils.h"
#include "Entity/RenderScissorComponent.h"
#include "GUIUtils.h"

void AboutMenuOnSelect(VariantList *pVList) //0=vec2 point of click, 1=entity sent from
{
	Entity *pEntClicked = pVList->Get(1).GetEntity();
	LogMsg("Clicked %s entity at %s", pEntClicked->GetName().c_str(),pVList->m_variant[1].Print().c_str());
	Entity *pMenu = GetEntityRoot()->GetEntityByName("AboutMenu"); //we're sort of cheating by just grabbing the top level parent
	//entity by name instead of GetParent() a bunch of times to reach the top level, but meh
	
	
	if (pEntClicked->GetName() == "Back")
	{
		DisableAllButtonsEntity(pEntClicked->GetParent());
		SlideScreen(pEntClicked->GetParent(), false);
		GetMessageManager()->CallEntityFunction(pEntClicked->GetParent(), 500, "OnDelete", NULL);
		MainMenuCreate(pEntClicked->GetParent()->GetParent());
		return;
	}


	GetEntityRoot()->PrintTreeAsText(); //useful for debugging
}

void AboutMenuAddScrollContent(Entity *pParent)
{
	//here we add our actual content we want scrolled.  At the end, we'll calculate the size used using ResizeScrollBounds and the scroll bars
	//can update.  If you are adding content over time, (like, downloading highscores or whatever) it's ok to call ResizeScrollBounds
	//repeatedly to dynamically resize the scroll area as you go.

	pParent = pParent->GetEntityByName("scroll_child");
	pParent->RemoveAllEntities(); //clear it out in case we call this more than once, say, to update/change something

	float x = 5; //inset
	float y = 0;
	float spacerY = 50; //space between thingies

	//first, a title in a big font
	Entity *pTitle = CreateTextLabelEntity(pParent, "Title", x, 0, GET_LOCTEXT("{MAINMENU_ABOUT}"));
	pTitle->GetVar("color")->Set(GET_THEMEMGR->GetTextColor());
	SetupTextEntity(pTitle, FONT_LARGE);
	y += pTitle->GetVar("size2d")->GetVector2().y +spacerY;

	//define an area for a text box, so it will wrap in  the right places.  Height is actually ignored.
	CL_Vec2f vTextBoxPos(x,y);
	CL_Vec2f vTextBounds(iPhoneMapX(434), iPhoneMapY(200));
	string msg; //we could load text, but let's just manually put junk in there:

	msg += GET_LOCTEXT("{ABOUTMENU_TEXT1}");

	Entity *pEnt = CreateTextBoxEntity(pParent, "SomeTextBox", vTextBoxPos, vTextBounds, msg);
	pEnt->GetVar("color")->Set(GET_THEMEMGR->GetTextColor());
	y += pEnt->GetVar("size2d")->GetVector2().y; //move our Y position down the exact size of the text
	y += spacerY; //don't forget our spacer
	


	//automatically calculate the total size of this entity with all its children for the scroll bars, do this at the end
	VariantList vList(pParent->GetParent());
    ResizeScrollBounds(&vList);
}


Entity * AboutMenuCreate( Entity *pParentEnt)
{
	Entity *pBG = NULL;
	pBG =  CreateOverlayEntity(pParentEnt, "AboutMenu", GET_THEMEMGR->GetFilename("interface/menu_bg.rttex"), 0,0);
	EntitySetScaleBySize(pBG, GetScreenSize());
	AddFocusIfNeeded(pBG, true, 500);

	//setup the dimensions of where the scroll area will go
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

	//if we wanted to change the scroll bar color we could do it this way:
	//pScroll->GetVar("color")->Set(MAKE_RGBA(61,155, 193, 255)); 

	Entity* pScrollChild = pScroll->AddEntity(new Entity("scroll_child"));

	pScroll->AddComponent(new RenderScissorComponent()); //so the text/etc won't get drawn outside our scroll box
	pScroll->AddComponent(new FilterInputComponent); //lock out taps that are not in our scroll area

	//actually add all our content that we'll be scrolling (if there is too much for one screen), as much as we want, any kind of entities ok
	AboutMenuAddScrollContent(pBG);

	eFont font;
	float fontScale;
	GetFontAndScaleToFitThisLinesPerScreenY(&font, &fontScale, 13);

	//oh, let's put the Back button on the bottom bar thing
	Entity* pEnt = CreateTextButtonEntity(pBG, "Back", iPhoneMapX(5), GetScreenSizeYf() - iPhoneMapY(40), GET_LOCTEXT("{BACK}"), false);
	pEnt->GetFunction("OnButtonSelected")->sig_function.connect(&AboutMenuOnSelect);
	SetupTextEntity(pEnt, font, fontScale);
	AddRectAroundEntity(pEnt, GET_THEMEMGR->GetPrimaryColor(), GET_THEMEMGR->GetSecondaryColor(), iPadMapY(20), true, fontScale, font);
	pEnt->GetVar("color")->Set(GET_THEMEMGR->GetTextColor());
	SetTextShadowColor(pEnt, GET_THEMEMGR->GetTextShadowColor());
	AddHotKeyToButton(pEnt, VIRTUAL_KEY_BACK); //for androids back button and window's Escape button

	//slide it in with movement
	SlideScreen(pBG, true, 500);
	return pBG;
}