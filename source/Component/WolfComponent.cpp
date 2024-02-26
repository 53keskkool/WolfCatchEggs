#include "PlatformPrecomp.h"
#include "App.h"
#include "WolfComponent.h"
#include "Entity/EntityUtils.h"

WolfComponent::WolfComponent()
{
	SetName("WolfComp");
}


void WolfComponent::OnScaleChanged(Variant* pDataObject)
{
	UpdateSizeVar();

	//screen size has probably changed, let's update touch stuff too
	m_touchUpDelim = GetScreenSizeYf() / 2;
	m_touchRightDelim = GetScreenSizeXf() / 2;
}

void WolfComponent::OnAdd(Entity* pEnt)
{
	EntityComponent::OnAdd(pEnt);
	Entity* pParent = GetParent();

	//Saving some variables we will need later
	m_pPos2d = &pParent->GetVar("pos2d")->GetVector2();
	m_pSize2d = &pParent->GetVar("size2d")->GetVector2();
	m_pScale2d = &pParent->GetVarWithDefault("scale2d", Variant(1.0f, 1.0f))->GetVector2();
	//register us to update and render when parent does
	pParent->GetFunction("OnRender")->sig_function.connect(1, boost::bind(&WolfComponent::OnRender, this, _1));
	pParent->GetFunction("OnUpdate")->sig_function.connect(1, boost::bind(&WolfComponent::OnUpdate, this, _1));
	//connecting arcade input and touch to wolf
	Entity* pMenu = GetEntityRoot()->GetEntityByName("GameMenu");
	pMenu->GetFunction("OnArcadeInput")->sig_function.connect(1, boost::bind(&WolfComponent::OnArcadeInput, this, _1));
	pMenu->GetFunction("OnTouchStart")->sig_function.connect(1, boost::bind(&WolfComponent::OnTouchStart, this, _1));
	pMenu->GetFunction("OnOverMove")->sig_function.connect(1, boost::bind(&WolfComponent::OnOverMove, this, _1));
	pMenu->GetFunction("OnOverEnd")->sig_function.connect(1, boost::bind(&WolfComponent::OnOverEnd, this, _1));
	m_touchUpDelim = GetScreenSizeYf() / 2;
	m_touchRightDelim = GetScreenSizeXf() / 2;
	//egg fell, check if we catched it
	pParent->GetParent()->GetFunction("OnEggFall")->sig_function.connect(1, boost::bind(&WolfComponent::OnEggFall, this, _1));
	//game is over, don't move wolf anymore
	pMenu->GetFunction("OnGameOver")->sig_function.connect(1, boost::bind(&WolfComponent::OnGameOver, this, _1));
	pMenu->GetFunction("OnPause")->sig_function.connect(1, boost::bind(&WolfComponent::OnPause, this, _1));

	//so when wolf gets scaled size updates too...
	pParent->GetVar("scale2d")->GetSigOnChanged()->connect(boost::bind(&WolfComponent::OnScaleChanged, this, _1));

	//loading wolf texture
	m_pWolfUp = GetResourceManager()->GetSurfaceAnim(GET_THEMEMGR->GetFilename("game/wolf_up.rttex"));
	m_pWolfUp->SetupAnim(1, 1);
	m_pWolfDown = GetResourceManager()->GetSurfaceAnim(GET_THEMEMGR->GetFilename("game/wolf_down.rttex"));
	m_pWolfDown->SetupAnim(1, 1);
	UpdateSizeVar();

	//let's make wolf look at random pos
	m_bLookingUp = (bool)RandomInt(0, 1);
	m_bLookingRight = (bool)RandomInt(0, 1);
}

void WolfComponent::OnRemove()
{
	EntityComponent::OnRemove();
}

void WolfComponent::OnEggFall(VariantList* pVList)
{
	Entity* pEgg = pVList->Get(0).GetEntity();
	//check if we catched this egg
	if (pEgg->GetVar("up")->GetUINT32() == m_bLookingUp && pEgg->GetVar("right")->GetUINT32() == m_bLookingRight)
	{
		pEgg->SetTaggedForDeletion(); //tagging for deletion, because we caught it
		GetParent()->GetParent()->GetFunction("OnEggCatch")->sig_function(pVList); //let whole level know about it, so it gets counted
	}
}

void WolfComponent::OnPause(VariantList* pVList)
{
	m_bPaused = pVList->Get(0).GetUINT32();
}

void WolfComponent::OnGameOver(VariantList* pVList)
{
	m_bGameOver = true;
}

void WolfComponent::UpdateSizeVar()
{
	GetParent()->GetVar("size2d")->Set(m_pWolfUp->GetFrameSize() * (*m_pScale2d));
}

void WolfComponent::OnArcadeInput(VariantList* pVList)
{
	if (m_bGameOver || m_bPaused) return;
	int vKey = pVList->Get(0).GetUINT32();
	bool bIsDown = pVList->Get(1).GetUINT32() != 0;

	switch (vKey)
	{
	case VIRTUAL_KEY_DIR_LEFT:
		if (bIsDown) m_bLookingRight = false;
		break;

	case VIRTUAL_KEY_DIR_RIGHT:
		if (bIsDown) m_bLookingRight = true;
		break;

	case VIRTUAL_KEY_DIR_UP:
		if (bIsDown) m_bLookingUp = true;
		break;

	case VIRTUAL_KEY_DIR_DOWN:
		if (bIsDown) m_bLookingUp = false;
		break;
	}
}

void WolfComponent::OnTouchStart(VariantList* pVList)
{
	if (m_bGameOver || m_bPaused) return;
	TouchTrackInfo* touch = GetBaseApp()->GetTouch(pVList->Get(2).GetUINT32());
	if (touch->WasHandled()) {
		m_bTouching = false;
		return;
	}
	m_touchPos = touch->GetPos();
	m_bTouching = true;
}

void WolfComponent::OnOverMove(VariantList* pVList)
{
	if (m_bGameOver || m_bPaused) return;
	TouchTrackInfo* touch = GetBaseApp()->GetTouch(pVList->Get(2).GetUINT32());
	if (touch->WasHandled()) {
		m_bTouching = false;
		return;
	}
	m_touchPos = touch->GetPos();
}

void WolfComponent::OnOverEnd(VariantList* pVList)
{
	m_bTouching = false;
}

void WolfComponent::OnRender(VariantList* pVList)
{
	CL_Vec2f vFinalPos = pVList->m_variant[0].GetVector2() + *m_pPos2d;
	if (m_bLookingUp)
	{
		m_pWolfUp->BlitScaledAnim(vFinalPos.x, vFinalPos.y, 0, 0, *m_pScale2d, ALIGNMENT_UPPER_LEFT, 0xFFFFFFFF, 0, CL_Vec2f(0, 0), !m_bLookingRight);
	}
	else
	{
		m_pWolfDown->BlitScaledAnim(vFinalPos.x, vFinalPos.y, 0, 0, *m_pScale2d, ALIGNMENT_UPPER_LEFT, 0xFFFFFFFF, 0, CL_Vec2f(0, 0), !m_bLookingRight);
	}
}

void WolfComponent::OnUpdate(VariantList* pVList)
{
	if (m_bGameOver || m_bPaused) return;
	if (m_bTouching)
	{
		m_bLookingUp = m_touchPos.y < m_touchUpDelim;
		m_bLookingRight = m_touchPos.x > m_touchRightDelim;
	}
}