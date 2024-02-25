#pragma once
#include "BaseApp.h"
#include <vector>

class ThemeManager
{
public:
	bool Init(string name);

	vector<pair<string, string>> GetListOfThemes(); //codename & name
	vector<pair<string, string>> GetThemesCredits(); //name & authors
	string GetDefaultTheme(); //just gets first theme from the list

	string GetFilename(string texture);
	uint32 GetPrimaryColor() { return m_primaryColor; }
	uint32 GetSecondaryColor() { return m_secondaryColor; }
	uint32 GetScrollbarColor() { return m_scrollbarColor; }
	uint32 GetTextColor() { return m_textColor; }
	uint32 GetTextShadowColor() { return m_textShadowColor; }
	uint32 GetFloorHeight() { return m_floorHeight; }
	uint32 GetWolfHandsYUp() { return m_wolfHandsYUp; }
	uint32 GetWolfHandsYDown() { return m_wolfHandsYDown; }
	uint32 GetAllowedShelvesOverlap() { return m_allowedShelvesOverlap; }
	CL_Vec2f GetEggSpawnCords() { return m_eggSpawnCords; }
	CL_Vec2f GetEggRotationCenter() { return m_eggRotationCenter; }
	float GetEggRotation() { return m_eggRotation; }
	float GetEggRotationMod() { return m_eggRotationMod; }
	float GetEggScale() { return m_eggScale; }
	float GetBottomEggScale() { return m_bottomEggScale; }
	float GetBrokenEggScale() { return m_brokenEggScale; }
	bool IsBrokenEggInFront() { return m_bBrokenEggInFront; }
	float GetShelfThickness() { return m_shelfThickness; }
	bool IsBGScalingAllowed() { return m_bAllowBGScaling; }
	bool NeedScoreBG() { return m_bScoreBG; }

private:
	//current theme name
	string m_name = "default";

	//colors used for buttons/dialogs
	uint32 m_primaryColor = 0x000000FF;
	uint32 m_secondaryColor = 0x000000FF;
	uint32 m_scrollbarColor = 0x000000FF;

	uint32 m_textColor = 0xFFFFFFFF;
	uint32 m_textShadowColor = 0x00000096;

	//values for positioning entities
	uint32 m_floorHeight = 0;
	uint32 m_wolfHandsYUp = 0;
	uint32 m_wolfHandsYDown = 0;
	uint32 m_allowedShelvesOverlap = 0;
	CL_Vec2f m_eggSpawnCords = CL_Vec2f(0, 0);
	CL_Vec2f m_eggRotationCenter = CL_Vec2f(0.0f, 0.0f);
	float m_eggRotation = 0.0f;
	float m_eggRotationMod = 0.0f;
	float m_eggScale = 1.0f;
	float m_bottomEggScale = 1.0f;
	float m_brokenEggScale = 1.0f;
	bool m_bBrokenEggInFront = false;
	float m_shelfThickness = 0.0f;

	bool m_bAllowBGScaling = true;
	bool m_bScoreBG = false;

};