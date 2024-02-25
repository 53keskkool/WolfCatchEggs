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
	void PrepareGame();
	bool LoadPartsOfSpeech();
	void SendScoreUpdate();

	void OnScreenSizeChanged(VariantList* pVList); //move & rescale textures
	void OnPause(VariantList* pVList);
	void OnGameEnd(VariantList* pVList); //player ended the game in the pause menu
	void SaveScore(VariantList* pVList); //player probably closed the game
	void OnEggCatch(VariantList* pVList); //egg catched, +1 score
	void OnEggFalling(VariantList* pVList); //egg fell, -1 life

	void OnUpdate(VariantList* pVList);

	//details about eggs' spawning, UpLeft, UpRight, DownLeft, DownRight
	CL_Vec2f m_eggSpawnPos[4];
	float m_eggSpawnAngle[4];
	CL_Vec2f m_eggScale = CL_Vec2f(0, 0);
	float m_eggSpeed = 0.1f;
	float m_eggSpeedMod[2] = { 0.0f }; //min/max, how much speed add to add to the egg every catch
	float m_shelfLength = 0; //for eggs, so they know when to fall
	float m_floorY = 0;

	eGameMode m_gamemode = GAMEMODE_NORMAL;
	//math mode
	int m_goal = 0;
	int32 *m_pCurrentAns;
	//parts of speech mode
	vector<string> m_partsOfSpeech;
	vector<pair<string, uint8>> m_words; //word, part of speech (index in m_partsOfSpeech)
	uint8 m_currentPoS = 0;

	uint8 m_prevEgg = 0;
	bool m_bSpawnEggAtSamePos = false; //used when screen size changes
	uint8 m_lives = 3;
	uint32 m_score = 0;
	bool m_bGameOver = false;
	bool m_bPaused = false;
};