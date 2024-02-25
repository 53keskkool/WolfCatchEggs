#include "PlatformPrecomp.h"
#include "App.h"
#include "EggManager.h"
#include "WolfComponent.h"
#include "EggComponent.h"
#include "Entity/EntityUtils.h"
#include "Core/json.hpp"

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
	//
	pParent->GetParent()->GetFunction("OnScreenSizeChanged")->sig_function.connect(1, boost::bind(&EggManager::OnScreenSizeChanged, this, _1));
	pParent->GetParent()->GetFunction("OnPause")->sig_function.connect(1, boost::bind(&EggManager::OnPause, this, _1));
	//player ended the game from the pause menu
	pParent->GetParent()->GetFunction("OnGameEnd")->sig_function.connect(1, boost::bind(&EggManager::OnGameEnd, this, _1));
	//emergency score saving
	pParent->GetFunction("SaveScore")->sig_function.connect(1, boost::bind(&EggManager::SaveScore, this, _1));
	//connecting egg events
	pParent->GetFunction("OnEggCatch")->sig_function.connect(1, boost::bind(&EggManager::OnEggCatch, this, _1));
	pParent->GetFunction("OnEggFalling")->sig_function.connect(1, boost::bind(&EggManager::OnEggFalling, this, _1));

	m_gamemode = GetApp()->GetGameMode();
	//math mode value
	m_pCurrentAns = &pParent->GetVarWithDefault("currentAns", int32(0))->GetINT32();

	if (m_gamemode == GAMEMODE_PoS)
	{
		if (!LoadPartsOfSpeech())
		{
			//fall back to normal gamemode when this mode fails
			m_gamemode = GAMEMODE_NORMAL;
			GetApp()->GetVar("gamemode")->Set(uint32(GAMEMODE_NORMAL));
		}
	}

	PrepareGame();
}

void EggManager::OnRemove()
{
	EntityComponent::OnRemove();
}

void EggManager::PrepareGame()
{
	//setting up our game
	Entity* pParent = GetParent();

	//create shelves with chickens
	pParent->RemoveEntitiesByNameThatStartWith("Shelf");
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
	Entity* pWolf = pParent->GetEntityByName("Wolf");
	if (!pWolf)
	{
		pWolf = pParent->AddEntity(new Entity("Wolf"));
		pWolf->AddComponent(new WolfComponent);
	}
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
	if (GetSize2DEntity(pParent->GetParent()->GetEntityByName("BG")).x < GetScreenSizeXf())
	{
		spaceLeftX -= (GetSize2DEntity(pParent->GetParent()->GetEntityByName("BG")).x - GetScreenSizeXf()) / 2;
	}
	EntitySetScaleBySize(pShelfUL, CL_Vec2f(spaceLeftX, 0), true, true);
	EntitySetScaleBySize(pShelfUR, CL_Vec2f(spaceLeftX, 0), true, true);
	EntitySetScaleBySize(pShelfDL, CL_Vec2f(spaceLeftX, 0), true, true);
	EntitySetScaleBySize(pShelfDR, CL_Vec2f(spaceLeftX, 0), true, true);


	//now let's check that shelves fit on screen
	//(wolf will always fit, because we already used screen's dimensions to scale it... except if screen has strange ratio)
	//we only need to check Y pos of one of the shelves, if it's on screen (y>=0) then we are good, else rescale to fit
	if (upShelfY - GetSize2DEntity(pShelfUL).y < 0)
	{
		float spaceLeftY = upShelfY;
		EntitySetScaleBySize(pShelfUL, CL_Vec2f(0, spaceLeftY), true);
		EntitySetScaleBySize(pShelfUR, CL_Vec2f(0, spaceLeftY), true);
		EntitySetScaleBySize(pShelfDL, CL_Vec2f(0, spaceLeftY), true);
		EntitySetScaleBySize(pShelfDR, CL_Vec2f(0, spaceLeftY), true);
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
	}

	CL_Vec2f spawnPos = GET_THEMEMGR->GetEggSpawnCords() * shelfScale;
	CL_Vec2f sizeOfShelf;
	sizeOfShelf = GetSize2DEntity(pShelfUL);
	sizeOfShelf -= spawnPos;
	sizeOfShelf.y -= GET_THEMEMGR->GetShelfThickness() * shelfScale.y;

	//caluclating speeds
	//difficulty
	float dif = 1 - GetApp()->GetVarWithDefault("dif", 0.5f)->GetFloat();
	if (dif < 0.25f) dif = 0.25f;
	m_shelfLength = sizeOfShelf.x;
	//i don't want let people cheat by resizing screen
	float shelfLength = sqrt(sizeOfShelf.x * sizeOfShelf.x + sizeOfShelf.y * sizeOfShelf.y);
	float newSpeed = shelfLength / (RandomFloat(1500, 3000) * dif);
	if (m_eggSpeed == 0.1f || m_eggSpeed < newSpeed) m_eggSpeed = newSpeed;
	//also calculate minimum and maximum speed change
	m_eggSpeedMod[0] = shelfLength / (100000 * dif);
	m_eggSpeedMod[1] = shelfLength / (80000 * dif);

	m_floorY = GetScreenSizeYf() - floorHeight;
	m_eggSpawnAngle[0] = atan(sizeOfShelf.x / sizeOfShelf.y);
	m_eggSpawnAngle[1] = DEG2RAD(360) - m_eggSpawnAngle[0];
	m_eggSpawnAngle[2] = m_eggSpawnAngle[0];
	m_eggSpawnAngle[3] = m_eggSpawnAngle[1];
	m_eggScale.x = 0;
	m_eggScale.y = ((GetSize2DEntity(pShelfUL).y - spawnPos.y) * 0.75f) * GET_THEMEMGR->GetEggScale();
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

	//removing eggs, so they don't look strange
	vector<Entity*> eggs;
	GetParent()->GetEntitiesByName(&eggs, "Egg");
	GetParent()->GetEntitiesByName(&eggs, "BrokenEgg");
	for (auto& e : eggs) e->SetTaggedForDeletion();

	switch (m_gamemode)
	{
	case GAMEMODE_MATH:
	{
		m_eggSpeed = shelfLength / (5000 * dif); //don't care about cheating here

		if (m_goal == 0) m_goal = RandomInt(1, 40);
		Entity* pGoal = pParent->GetEntityByName("Goal");
		if (!pGoal)
		{
			pGoal = CreateTextLabelEntity(pParent, "Goal", 0, 0, to_string(m_goal));
			SetTextShadowColor(pGoal, GET_THEMEMGR->GetTextShadowColor());
			SetAlignmentEntity(pGoal, ALIGNMENT_CENTER);
		}
		eFont goalFont;
		float goalScale;
		GetFontAndScaleToFitThisLinesPerScreenY(&goalFont, &goalScale, 14);
		SetupTextEntity(pGoal, goalFont, goalScale);

		CL_Vec2f goalPos = GetPos2DEntity(pWolf);
		goalPos.y -= GetSize2DEntity(pWolf).y;
		goalPos.y /= 2;
		SetPos2DEntity(pGoal, goalPos);
	}
	break;

	case GAMEMODE_PoS:
	{
		m_eggSpeed = shelfLength / (5000 * dif); //don't care about cheating here

		if (m_goal == 0)
		{
			m_goal = RandomInt(1, 15);
			m_currentPoS = RandomInt(0, m_partsOfSpeech.size() - 1);
		}

		Entity* pGoal = pParent->GetEntityByName("Goal");
		if (!pGoal)
		{
			pGoal = CreateTextLabelEntity(pParent, "Goal", 0, 0, m_partsOfSpeech[m_currentPoS]);
			SetTextShadowColor(pGoal, GET_THEMEMGR->GetTextShadowColor());
			SetAlignmentEntity(pGoal, ALIGNMENT_CENTER);
		}
		eFont goalFont;
		float goalScale;
		GetFontAndScaleToFitThisLinesPerScreenY(&goalFont, &goalScale, 14);
		SetupTextEntity(pGoal, goalFont, goalScale);

		CL_Vec2f goalPos = GetPos2DEntity(pWolf);
		goalPos.y -= GetSize2DEntity(pWolf).y;
		goalPos.y /= 2;
		SetPos2DEntity(pGoal, goalPos);
	}
	break;
	}
}

bool EggManager::LoadPartsOfSpeech()
{
	FileInstance file("game/pos/" + GetApp()->GetVar("language")->GetString() + ".json");
	if (!file.IsLoaded())
	{
		return false;
	}

	try
	{
		nlohmann::json j = nlohmann::json::parse(file.GetAsChars());

		nlohmann::json partsOfSpeech = j["partsOfSpeech"];
		for (auto& p : partsOfSpeech)
		{
			uint8 index = m_partsOfSpeech.size();
			string name = p["name"];
			m_partsOfSpeech.push_back(name);

			nlohmann::json words = p["words"];
			for (auto& w : words)
			{
				m_words.push_back({ w, index });
			}
		}
	}
	catch (std::exception& e)
	{
		return false;
	}

	return true;
}

void EggManager::SendScoreUpdate()
{
	VariantList vlist(m_score, uint32(m_lives));
	GetParent()->GetFunction("OnScoreUpdate")->sig_function(&vlist);
}

void EggManager::OnScreenSizeChanged(VariantList* pVList)
{
	PrepareGame();
	SendScoreUpdate();
	m_bSpawnEggAtSamePos = true;
}

void EggManager::OnPause(VariantList* pVList)
{
	m_bPaused = pVList->Get(0).GetUINT32();
}

void EggManager::OnGameEnd(VariantList* pVList)
{
	m_bGameOver = true;
	VariantList vlist(m_score);
	GetParent()->GetParent()->GetFunction("OnGameOver")->sig_function(&vlist);
}

void EggManager::SaveScore(VariantList* pVList)
{
	uint32 bestScore = GetApp()->GetVarWithDefault("bestScore" + to_string(m_gamemode), uint32(0))->GetUINT32();
	if (m_score > bestScore) GetApp()->GetVar("bestScore" + to_string(m_gamemode))->Set(m_score);
}

void EggManager::OnEggCatch(VariantList* pVList) //egg catched, +1 score
{
	switch (m_gamemode)
	{
	case GAMEMODE_MATH:
	{
		*m_pCurrentAns += pVList->Get(0).GetEntity()->GetVar("digit")->GetINT32();
		if (*m_pCurrentAns == m_goal)
		{
			m_score++;
			*m_pCurrentAns = 0;
			int newGoal = RandomInt(1, 40);
			while (newGoal == m_goal) newGoal = RandomInt(1, 40);
			m_goal = newGoal;
			SetTextEntity(GetParent()->GetEntityByName("Goal"), to_string(m_goal));
			SendScoreUpdate();
		}
	}
	return;

	case GAMEMODE_PoS:
	{
		uint8 currentPoS = m_currentPoS; //placing this here, so it's not affected by the check
		if (++(*m_pCurrentAns) >= m_goal)
		{
			m_goal = RandomInt(1, 10);
			*m_pCurrentAns = 0;
			m_currentPoS = RandomInt(0, m_partsOfSpeech.size() - 1);
			SetTextEntity(GetParent()->GetEntityByName("Goal"), m_partsOfSpeech[m_currentPoS]);
		}
		if (pVList->Get(0).GetEntity()->GetVar("partOfSpeech")->GetUINT32() != currentPoS)
		{
			m_lives--;
			SendScoreUpdate();
			return;
		}
	}
	break;

	default:
		m_eggSpeed += RandomFloat(m_eggSpeedMod[0], m_eggSpeedMod[1]);
		break;
	}
	m_score++;
	SendScoreUpdate();
}

void EggManager::OnEggFalling(VariantList* pVList) //egg fell, -1 life
{
	switch (m_gamemode)
	{
	case GAMEMODE_MATH:
		return;

	case GAMEMODE_PoS:
	{
		uint8 currentPoS = m_currentPoS; //placing this here, so it's not affected by the check
		if (++(*m_pCurrentAns) >= m_goal)
		{
			m_goal = RandomInt(1, 10);
			*m_pCurrentAns = 0;
			m_currentPoS = RandomInt(0, m_partsOfSpeech.size() - 1);
			SetTextEntity(GetParent()->GetEntityByName("Goal"), m_partsOfSpeech[m_currentPoS]);
		}
		if (pVList->Get(0).GetEntity()->GetVar("partOfSpeech")->GetUINT32() != currentPoS)
		{
			return;
		}
	}
	break;
	}
	m_lives--;
	SendScoreUpdate();
}

void EggManager::OnUpdate(VariantList* pVList)
{
	if (m_bPaused) return;

	vector<Entity*> eggs;
	GetParent()->GetEntitiesByName(&eggs, "Egg");
	if ((m_lives < 1 && m_gamemode != GAMEMODE_MATH) || m_bGameOver) //no more lives left, let's wait for all eggs to fall and then open game over menu
	{
		if (eggs.size() > 0) return;
		if (!m_bGameOver)
		{
			m_bGameOver = true;
			VariantList vlist(m_score);
			GetParent()->GetParent()->GetFunction("OnGameOver")->sig_function(&vlist);
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
		uint8 shelfNum; //UL,UR,DL,DR
		if (m_bSpawnEggAtSamePos)
		{
			shelfNum = m_prevEgg;
			m_bSpawnEggAtSamePos = false;
		}
		else
		{
			shelfNum = RandomInt(0, 3);
			while (m_prevEgg == shelfNum) shelfNum = RandomInt(0, 3); //i want players to move every new egg
			m_prevEgg = shelfNum;
		}
		Entity* pEgg = GetParent()->AddEntity(new Entity("Egg"));
		pEgg->AddComponent(new EggComponent);
		if (m_eggScale.x == 0)
		{
			CL_Vec2f size = GetSize2DEntity(pEgg);
			EntitySetScaleBySize(pEgg, m_eggScale, true, size.x < size.y);
			m_eggScale = GetScale2DEntity(pEgg);
		}
		else
		{
			SetScale2DEntity(pEgg, m_eggScale);
		}
		if (shelfNum > 1)
		{
			SetScale2DEntity(pEgg, m_eggScale * GET_THEMEMGR->GetBottomEggScale());
		}
		SetPos2DEntity(pEgg, m_eggSpawnPos[shelfNum]);
		SetAlignmentEntity(pEgg, ALIGNMENT_DOWN_CENTER);
		pEgg->GetVar("up")->Set(uint32(shelfNum <= 1));
		pEgg->GetVar("right")->Set(uint32(shelfNum % 2 == 1));
		pEgg->GetVar("angle")->Set(m_eggSpawnAngle[shelfNum]);
		pEgg->GetVar("speed")->Set(m_eggSpeed);
		pEgg->GetVar("shelfLength")->Set(m_shelfLength);
		pEgg->GetVar("floorY")->Set(m_floorY);

		switch (m_gamemode)
		{
		case GAMEMODE_MATH:
		{
			CL_Vec2f eggSize = GetSize2DEntity(pEgg);
			int digit = RandomInt(-8, 8);
			while (digit == 0) digit = RandomInt(-8, 8);
			string digitText = to_string(digit);
			if (digit > 0) digitText = "+" + digitText;
			pEgg->GetVar("digit")->Set(int32(digit));
			Entity* pDigit = CreateTextLabelEntity(pEgg, "text", eggSize.x / 2, eggSize.y / 2, digitText);
			eFont digitFont;
			float digitScale;
			GetFontAndScaleToFitThisStringInWidthPixels(&digitFont, &digitScale, digitText, (eggSize.x > eggSize.y ? eggSize.y : eggSize.x));
			SetupTextEntity(pDigit, digitFont, digitScale);
			SetAlignmentEntity(pDigit, ALIGNMENT_UPPER_CENTER);
			SetTextShadowColor(pDigit, GET_THEMEMGR->GetTextShadowColor());
			pDigit->GetVar("color")->Set(digit >= 0 ? MAKE_RGBA(0x40, 0xE0, 0xFF, 0xFF) : MAKE_RGBA(0xFF, 0, 0, 0xFF));
		}
		break;

		case GAMEMODE_PoS:
		{
			CL_Vec2f eggSize = GetSize2DEntity(pEgg);
			bool chooseCurrentPoS = RandomInt(0, 100) <= 40; //let's add 40% chance to get needed part of speech, else it might be too rare
			int wordPos = RandomInt(0, m_words.size() - 1);
			if (chooseCurrentPoS)
			{
				while (m_words[wordPos].second != m_currentPoS) wordPos = RandomInt(0, m_words.size() - 1);
			}
			string word = m_words[wordPos].first;
			pEgg->GetVar("partOfSpeech")->Set(uint32(m_words[wordPos].second));
			Entity* pWord = CreateTextLabelEntity(pEgg, "text", eggSize.x / 2, eggSize.y / 2, word);
			eFont wordFont;
			float wordScale;
			GetFontAndScaleToFitThisPixelHeight(&wordFont, &wordScale, (eggSize.x > eggSize.y ? eggSize.y : eggSize.x));
			SetupTextEntity(pWord, wordFont, wordScale);
			SetAlignmentEntity(pWord, ALIGNMENT_UPPER_CENTER);
			SetTextShadowColor(pWord, GET_THEMEMGR->GetTextShadowColor());
			pWord->GetVar("color")->Set(MAKE_RGBA(0x40, 0xE0, 0xFF, 0xFF));
		}
		break;
		}
	}
}