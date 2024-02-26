#include "PlatformPrecomp.h"
#include "App.h"
#include "EggComponent.h"
#include "Entity/EntityUtils.h"

EggComponent::EggComponent()
{
	SetName("EggComp");
}


void EggComponent::OnScaleChanged(Variant* pDataObject)
{
	UpdateSizeVar();
}

void EggComponent::OnAngleChanged(Variant* pDataObject)
{
	m_bRotatingRight = *m_pAngle <= DEG2RAD(180);
	if (m_bRotatingRight && !m_bRotating)
	{
		*m_pRotation = 360 - *m_pRotation;
	}
}

void EggComponent::OnAdd(Entity* pEnt)
{
	EntityComponent::OnAdd(pEnt);
	Entity* pParent = GetParent();

	//Saving some variables we will need later
	m_pPos2d = &pParent->GetVar("pos2d")->GetVector2();
	m_pSize2d = &pParent->GetVar("size2d")->GetVector2();
	m_pScale2d = &pParent->GetVarWithDefault("scale2d", Variant(1.0f, 1.0f))->GetVector2();
	m_pRotation = &pParent->GetVar("rotation")->GetFloat();  //in degrees
	//in case it's fixed...
	if (GET_THEMEMGR->GetEggRotation() != 0)
	{
		*m_pRotation = GET_THEMEMGR->GetEggRotation();
		m_bRotating = false;
	}
	m_pRotationCenter = &pParent->GetVarWithDefault("rotationCenter", GET_THEMEMGR->GetEggRotationCenter())->GetVector2();
	m_pAlpha = &pParent->GetVarWithDefault("alpha", Variant(1.0f))->GetFloat();
	m_pAngle = &pParent->GetVarWithDefault("angle", float(0))->GetFloat();  //in radians
	m_pSpeed = &pParent->GetVarWithDefault("speed", 0.1f)->GetFloat();
	m_pFalling = &pParent->GetVarWithDefault("falling", uint32(false))->GetUINT32();
	m_pShelfLength = &pParent->GetVarWithDefault("shelfLength", float(0))->GetFloat();
	m_pFloorY = &pParent->GetVarWithDefault("floorY", float(0))->GetFloat();
	//register us to update and render when parent does
	pParent->GetFunction("OnRender")->sig_function.connect(1, boost::bind(&EggComponent::OnRender, this, _1));
	pParent->GetFunction("OnUpdate")->sig_function.connect(1, boost::bind(&EggComponent::OnUpdate, this, _1));

	//so when egg gets scaled size updates too...
	pParent->GetVar("scale2d")->GetSigOnChanged()->connect(boost::bind(&EggComponent::OnScaleChanged, this, _1));
	pParent->GetVar("angle")->GetSigOnChanged()->connect(boost::bind(&EggComponent::OnAngleChanged, this, _1));

	//pause eggs too
	GetEntityRoot()->GetEntityByName("GameMenu")->GetFunction("OnPause")->sig_function.connect(1, boost::bind(&EggComponent::OnPause, this, _1));

	//loading egg texture
	m_pTex = GetResourceManager()->GetSurfaceAnim(GET_THEMEMGR->GetFilename("game/egg.rttex"));
	m_pTex->SetupAnim(1, 1);
	m_pTexBroken = GetResourceManager()->GetSurfaceAnim(GET_THEMEMGR->GetFilename("game/broken_egg.rttex"));
	if (m_pTexBroken)
	{
		if (m_pTexBroken->IsLoaded()) m_pTexBroken->SetupAnim(1, 1);
		else m_pTexBroken = NULL;
	}
	UpdateSizeVar();

	if (m_pRotationCenter->x == 0.0f && m_pRotationCenter->y == 0.0f) *m_pRotationCenter = *m_pSize2d / 2;
}

void EggComponent::OnRemove()
{
	EntityComponent::OnRemove();
}

void EggComponent::OnPause(VariantList* pVList)
{
	m_bPaused = pVList->Get(0).GetUINT32();
}

void EggComponent::UpdateSizeVar()
{
	if (!m_pTexBroken || !m_bBroken) GetParent()->GetVar("size2d")->Set(m_pTex->GetFrameSize() * (*m_pScale2d));
	else GetParent()->GetVar("size2d")->Set(m_pTexBroken->GetFrameSize() * (*m_pScale2d));
	m_rotationMod = (360.0f / ((m_pSize2d->x + m_pSize2d->y) * 1.5f)) * GET_THEMEMGR->GetEggRotationMod();
}

void EggComponent::OnRender(VariantList* pVList)
{
	if (GetParent()->GetTaggedForDeletion()) return;
	if (*m_pAlpha <= 0) return;

	CL_Vec2f vFinalPos = pVList->m_variant[0].GetVector2() + *m_pPos2d;
	CL_Vec2f vRotationPt = vFinalPos;

	vRotationPt.x += m_pScale2d->x * m_pRotationCenter->x;
	vRotationPt.y += m_pScale2d->y * m_pRotationCenter->y;
	if (!m_pTexBroken || !m_bBroken)
	{
		m_pTex->BlitScaledAnim(vFinalPos.x, vFinalPos.y, 0, 0, *m_pScale2d, ALIGNMENT_UPPER_LEFT, MAKE_RGBA(0xFF, 0xFF, 0xFF, 0xFF * *m_pAlpha), *m_pRotation, vRotationPt);
	}
	else
	{
		m_pTexBroken->BlitScaledAnim(vFinalPos.x, vFinalPos.y, 0, 0, *m_pScale2d, ALIGNMENT_UPPER_LEFT, MAKE_RGBA(0xFF, 0xFF, 0xFF, 0xFF * *m_pAlpha), *m_pRotation, vRotationPt);
	}
}

void EggComponent::OnUpdate(VariantList* pVList)
{
	if (GetParent()->GetTaggedForDeletion()) return;
	if (m_bPaused) return;

	float deltaTick = GetApp()->GetDeltaTick();

	if (m_bBroken)
	{
		if (m_startFadeOutAt > GetApp()->GetTick() || m_startFadeOutAt == 0) return;
		*m_pAlpha -= deltaTick / 2500;
		if (*m_pAlpha <= 0)
		{
			m_startFadeOutAt = 0;
			*m_pAlpha = 0;
			GetParent()->SetTaggedForDeletion();
		}
		return;
	}

	if (*m_pFalling)
	{
		*m_pSpeed += 0.0036f * deltaTick;
		*m_pAngle = 0;
	}
	
	CL_Vec2f movement;
	movement.x = deltaTick * *m_pSpeed * sin(*m_pAngle);
	movement.y = deltaTick * *m_pSpeed * cos(*m_pAngle);
	*m_pPos2d += movement;
	m_alrdWent += movement.x;

	if (m_bRotating || *m_pFalling)
	{
		float moved = sqrt(movement.x * movement.x + movement.y * movement.y);

		if (m_bRotatingRight)
		{
			*m_pRotation += moved * m_rotationMod;
			if (*m_pRotation >= 360) *m_pRotation -= 360;
		}
		else
		{
			*m_pRotation -= moved * m_rotationMod;
			if (*m_pRotation < 0) *m_pRotation += 360;
		}
	}

	if (abs(m_alrdWent) >= *m_pShelfLength) //we are falling now
	{
		if (!*m_pFalling) //just started falling, maybe wolf can catch it?
		{
			VariantList vlist;
			vlist.Get(0).Set(GetParent());
			GetParent()->GetParent()->GetFunction("OnEggFall")->sig_function(&vlist);
			if (GetParent()->GetTaggedForDeletion()) return; //we were tagged for deletion - means we got caught
			GetParent()->GetParent()->GetFunction("OnEggFalling")->sig_function(&vlist); //we didn't get caught... count it
		}
		*m_pFalling = true;
		m_rotationMod = RandomFloat(0.1f, 1.5f);
	}
	if (m_pPos2d->y >= *m_pFloorY) //we are broken now
	{
		CL_Vec2f currentSize = GetSize2DEntity(GetParent());
		m_bBroken = true;
		*m_pRotation = 0;
		m_startFadeOutAt = GetApp()->GetTick() + RandomInt(2000, 8000);
		GetParent()->SetName("BrokenEgg");
		if (GET_THEMEMGR->IsBrokenEggInFront())
		{
			GetParent()->GetParent()->MoveEntityToTopByAddress(GetParent());
		}
		else
		{
			GetParent()->GetParent()->MoveEntityToBottomByAddress(GetParent());
		}
		GetParent()->RemoveEntityByName("text", false);
		GetAudioManager()->Play(GET_THEMEMGR->GetFilename("audio/egg_fall.wav"));
		UpdateSizeVar();
		EntitySetScaleBySize(GetParent(), currentSize * GET_THEMEMGR->GetBrokenEggScale(), true);
	}
}