#pragma once
#include "Entity/Component.h"
#include "Entity/Entity.h"

class EggManager : public EntityComponent
{
public:
	EggManager();

	void OnAdd(Entity* pEnt);
	void OnRemove();

private:
	void OnEggCatch(VariantList* pVList); //egg catched, +1 score
	void OnEggFalling(VariantList* pVList); //egg fell, -1 life

	void OnUpdate(VariantList* pVList);

	//details about eggs' spawning, UpLeft, UpRight, DownLeft, DownRight
	CL_Vec2f m_eggSpawnPos[4];
	float m_eggSpawnAngle[4];
	CL_Vec2f m_eggScale = CL_Vec2f(0, 0);
	float m_eggSpeed = 0.1f;
	float m_shelfLength = 0; //for eggs, so they know when to fall
	float m_floorY = 0;

	uint8 m_prevEgg = 0;
	uint8 m_lives = 3;
	uint32 m_score = 0;
	bool m_bGameOver = false;
};