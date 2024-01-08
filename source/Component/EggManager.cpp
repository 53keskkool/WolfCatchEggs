#include "PlatformPrecomp.h"
#include "App.h"
#include "EggManager.h"
#include "WolfComponent.h"
#include "EggComponent.h"
#include "Entity/EntityUtils.h"

EggManager::EggManager()
{
	SetName("EggManager");
}


void EggManager::OnAdd(Entity* pEnt)
{
	EntityComponent::OnAdd(pEnt);

	Entity* pParent = GetParent();
	//register us to update when parent does
	pParent->GetFunction("OnUpdate")->sig_function.connect(1, boost::bind(&EggManager::OnUpdate, this, _1));
	//connecting egg events
	pParent->GetFunction("OnEggCatch")->sig_function.connect(1, boost::bind(&EggManager::OnEggCatch, this, _1));
	pParent->GetFunction("OnEggFalling")->sig_function.connect(1, boost::bind(&EggManager::OnEggFalling, this, _1));

	
	//setting up our game
	//create shelves with chickens
	Entity* pShelfUL = CreateOverlayEntity(pParent, "ShelfUL", GET_THEMEMGR->GetFilename("game/shelf.rttex"), 0, 0);
	Entity* pShelfUR = CreateOverlayEntity(pParent, "ShelfUR", GET_THEMEMGR->GetFilename("game/shelf.rttex"), 0, 0);
	Entity* pShelfDL = CreateOverlayEntity(pParent, "ShelfDL", GET_THEMEMGR->GetFilename("game/shelf.rttex"), 0, 0);
	Entity* pShelfDR = CreateOverlayEntity(pParent, "ShelfDR", GET_THEMEMGR->GetFilename("game/shelf.rttex"), 0, 0);
	//flip right shelves, so they don't face into a wall
	pShelfUR->GetComponentByName("OverlayRender")->GetVar("flipX")->Set(uint32(true));
	pShelfDR->GetComponentByName("OverlayRender")->GetVar("flipX")->Set(uint32(true));
	float heightOfShelf = GetSize2DEntity(pShelfUL).y;

	//Getting actual floor height by getting background's scale and floor height from config
	float floorHeight = GetScale2DEntity(pParent->GetParent()->GetEntityByName("BG")).y * float(GET_THEMEMGR->GetFloorHeight());

	//create wolf (will be facing up-right by default)
	Entity* pWolf = pParent->AddEntity(new Entity("Wolf"));
	pWolf->AddComponent(new WolfComponent);
	SetPos2DEntity(pWolf, CL_Vec2f(GetScreenSizeXf() / 2, GetScreenSizeYf() - floorHeight));
	//scaling wolf to be 2/3 of height available...
	EntitySetScaleBySize(pWolf, CL_Vec2f(0, (GetScreenSizeYf() - floorHeight) * 0.65f), true, true);
	//letting "renderer" know, that we gave him at the center down of the texture
	SetAlignmentEntity(pWolf, ALIGNMENT_DOWN_CENTER);

	//finding where wolf's hands are located
	float handsUpY = GET_THEMEMGR->GetWolfHandsYUp() * GetScale2DEntity(pWolf).y;
	float handsDownY = GET_THEMEMGR->GetWolfHandsYDown() * GetScale2DEntity(pWolf).y;
	//also finding where will wolf's hands be by X
	float leftShelfX = GetPos2DEntity(pWolf).x - GetSize2DEntity(pWolf).x / 2;
	float rightShelfX = GetPos2DEntity(pWolf).x + GetSize2DEntity(pWolf).x / 2;

	//now we need to position shelves as we need...
	float upShelfY = GetPos2DEntity(pWolf).y - (GetSize2DEntity(pWolf).y - handsUpY);
	SetPos2DEntity(pShelfUL, CL_Vec2f(leftShelfX, upShelfY));
	SetPos2DEntity(pShelfUR, CL_Vec2f(rightShelfX, upShelfY));
	SetAlignmentEntity(pShelfUL, ALIGNMENT_DOWN_RIGHT);
	SetAlignmentEntity(pShelfUR, ALIGNMENT_DOWN_LEFT);

	float downShelfY = GetPos2DEntity(pWolf).y - (GetSize2DEntity(pWolf).y - handsDownY);
	SetPos2DEntity(pShelfDL, CL_Vec2f(leftShelfX, downShelfY));
	SetPos2DEntity(pShelfDR, CL_Vec2f(rightShelfX, downShelfY));
	SetAlignmentEntity(pShelfDL, ALIGNMENT_DOWN_RIGHT);
	SetAlignmentEntity(pShelfDR, ALIGNMENT_DOWN_LEFT);

	//scaling shelves to fit with the wolf
	float spaceLeftX = GetPos2DEntity(pWolf).x - GetSize2DEntity(pWolf).x / 2;
	EntitySetScaleBySize(pShelfUL, CL_Vec2f(spaceLeftX, 0), true, true);
	EntitySetScaleBySize(pShelfUR, CL_Vec2f(spaceLeftX, 0), true, true);
	EntitySetScaleBySize(pShelfDL, CL_Vec2f(spaceLeftX, 0), true, true);
	EntitySetScaleBySize(pShelfDR, CL_Vec2f(spaceLeftX, 0), true, true);


	//now let's check that shelves fit on screen
	//(wolf will always fit, because we already used screen's dimensions to scale it... except if screen have strange ratio)

	//where to put walls so shelves don't just fly
	float leftWallsX = -1.0f;
	float rightWallsX = -1.0f;

	//we only need to check Y pos of one of the shelves, if it's on screen (y>0) then we are good, else rescale to fit and add a wall shelves hold on
	if (upShelfY - GetSize2DEntity(pShelfUL).y < 0)
	{
		float spaceLeftY = GetScreenSizeYf() - upShelfY;
		EntitySetScaleBySize(pShelfUL, CL_Vec2f(0, spaceLeftY), true);
		EntitySetScaleBySize(pShelfUR, CL_Vec2f(0, spaceLeftY), true);
		EntitySetScaleBySize(pShelfDL, CL_Vec2f(0, spaceLeftY), true);
		EntitySetScaleBySize(pShelfDR, CL_Vec2f(0, spaceLeftY), true);

		leftWallsX = GetPos2DEntity(pShelfUL).x - GetSize2DEntity(pShelfUL).x;
		rightWallsX = GetPos2DEntity(pShelfUR).x + GetSize2DEntity(pShelfUR).x;
	}

	//checking if shelves overlap
	CL_Vec2f shelfSize = GetSize2DEntity(pShelfUL);
	CL_Vec2f shelfScale = GetScale2DEntity(pShelfUL);
	float overlapY = GetPos2DEntity(pShelfUL).y - (GetPos2DEntity(pShelfDL).y - shelfSize.y);
	if (overlapY > GET_THEMEMGR->GetAllowedShelvesOverlap() * shelfScale.y)
	{
		shelfScale.x = (downShelfY - upShelfY) / (heightOfShelf - GET_THEMEMGR->GetAllowedShelvesOverlap());
		shelfScale.y = shelfScale.x;
		SetScale2DEntity(pShelfUL, shelfScale);
		SetScale2DEntity(pShelfUR, shelfScale);
		SetScale2DEntity(pShelfDL, shelfScale);
		SetScale2DEntity(pShelfDR, shelfScale);

		leftWallsX = GetPos2DEntity(pShelfUL).x - GetSize2DEntity(pShelfUL).x;
		rightWallsX = GetPos2DEntity(pShelfUR).x + GetSize2DEntity(pShelfUR).x;
	}

	//adding walls for shelves if necessary
	if (leftWallsX != -1.0f && rightWallsX != -1.0f)
	{
		Entity* pLeftWall = CreateOverlayEntity(pParent, "LeftWall", GET_THEMEMGR->GetFilename("game/wall.rttex"), leftWallsX, 0);
		SetAlignmentEntity(pLeftWall, ALIGNMENT_UPPER_RIGHT);
		pLeftWall->GetComponentByName("OverlayRender")->GetVar("flipX")->Set(uint32(true));
		EntitySetScaleBySize(pLeftWall, CL_Vec2f(0, GetScreenSizeYf() - floorHeight), true, true);
		Entity* pRightWall = CreateOverlayEntity(pParent, "RightWall", GET_THEMEMGR->GetFilename("game/wall.rttex"), rightWallsX, 0);
		SetAlignmentEntity(pRightWall, ALIGNMENT_UPPER_LEFT);
		SetScale2DEntity(pRightWall, GetScale2DEntity(pLeftWall));
	}

	CL_Vec2f spawnPos = GET_THEMEMGR->GetEggSpawnCords() * GetScale2DEntity(pShelfUL);
	CL_Vec2f sizeOfShelf;
	sizeOfShelf = GetSize2DEntity(pShelfUL);
	sizeOfShelf -= spawnPos;
	m_shelfLength = sizeOfShelf.x;
	m_eggSpeed = sqrt(sizeOfShelf.x*sizeOfShelf.x + sizeOfShelf.y*sizeOfShelf.y) / RandomFloat(1500, 2500);
	m_floorY = GetScreenSizeYf() - floorHeight;
	m_eggSpawnAngle[0] = atan(sizeOfShelf.x / sizeOfShelf.y);
	m_eggSpawnAngle[1] = DEG2RAD(360) - m_eggSpawnAngle[0];
	m_eggSpawnAngle[2] = m_eggSpawnAngle[0];
	m_eggSpawnAngle[3] = m_eggSpawnAngle[1];
	m_eggScale.y = (GetSize2DEntity(pShelfUL).y - spawnPos.y) * 0.75f;
	//UpLeft
	CL_Vec2f pos = GetPos2DEntity(pShelfUL);
	pos -= GetSize2DEntity(pShelfUL);
	pos += spawnPos;
	m_eggSpawnPos[0] = pos;
	//DownLeft
	pos.y = GetPos2DEntity(pShelfDL).y - GetSize2DEntity(pShelfDL).y + spawnPos.y;
	m_eggSpawnPos[2] = pos;
	//UpRight
	pos = GetPos2DEntity(pShelfUR);
	pos.x += GetSize2DEntity(pShelfUR).x - spawnPos.x;
	pos.y -= GetSize2DEntity(pShelfUR).y - spawnPos.y;
	m_eggSpawnPos[1] = pos;
	//DownRight
	pos.y = GetPos2DEntity(pShelfDR).y - GetSize2DEntity(pShelfDR).y + spawnPos.y;
	m_eggSpawnPos[3] = pos;
}

void EggManager::OnRemove()
{
	EntityComponent::OnRemove();
}

void EggManager::OnEggCatch(VariantList* pVList) //egg catched, +1 score
{
	m_score++;
	m_eggSpeed += RandomFloat(0.0005f, 0.0045f);
	VariantList vlist(m_score, uint32(m_lives));
	GetParent()->GetFunction("OnScoreUpdate")->sig_function(&vlist);
}

void EggManager::OnEggFalling(VariantList* pVList) //egg fell, -1 life
{
	m_lives--;
	VariantList vlist(m_score, uint32(m_lives));
	GetParent()->GetFunction("OnScoreUpdate")->sig_function(&vlist);
}

void EggManager::OnUpdate(VariantList* pVList)
{
	vector<Entity*> eggs;
	GetParent()->GetEntitiesByName(&eggs, "Egg");
	if (m_lives < 1) //no more lives left, let's wait for all eggs to fall and then do something (not sure what yet)
	{
		if (eggs.size() > 0) return;
		if (!m_bGameOver)
		{
			ShowTextMessage("Game Over!");
			m_bGameOver = true;
		}
		return;
	}

	bool bSpawnEgg = true;
	for (auto& e : eggs)
	{
		if (!e->GetVar("falling")->GetUINT32())
		{
			bSpawnEgg = false;
			break;
		}
	}

	if (bSpawnEgg)
	{
		uint8 shelfNum = RandomInt(0, 3); //UL,UR,DL,DR
		while (m_prevEgg == shelfNum) shelfNum = RandomInt(0, 3); //i want players to move every new egg
		m_prevEgg = shelfNum;
		Entity* pEgg = GetParent()->AddEntity(new Entity("Egg"));
		pEgg->AddComponent(new EggComponent);
		if (m_eggScale.x == 0)
		{
			EntitySetScaleBySize(pEgg, m_eggScale, true, true);
			m_eggScale = GetScale2DEntity(pEgg);
		}
		else
		{
			SetScale2DEntity(pEgg, m_eggScale);
		}
		SetPos2DEntity(pEgg, m_eggSpawnPos[shelfNum]);
		SetAlignmentEntity(pEgg, ALIGNMENT_DOWN_CENTER);
		pEgg->GetVar("up")->Set(uint32(shelfNum <= 1));
		pEgg->GetVar("right")->Set(uint32(shelfNum % 2 == 1));
		pEgg->GetVar("angle")->Set(m_eggSpawnAngle[shelfNum]);
		pEgg->GetVar("speed")->Set(m_eggSpeed);
		pEgg->GetVar("shelfLength")->Set(m_shelfLength);
		pEgg->GetVar("floorY")->Set(m_floorY);
	}
}