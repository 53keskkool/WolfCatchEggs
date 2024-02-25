#pragma once

#include "BaseApp.h"
#include "Managers/TranslationManager.h"
#include "Managers/ThemeManager.h"

#define GET_LOCTEXT GetApp()->GetTranslationManager()->GetText
#define GET_THEMEMGR GetApp()->GetThemeManager()

enum eGameMode {
	GAMEMODE_NORMAL,
	GAMEMODE_MATH,
	GAMEMODE_PoS //part of speech
};

class App: public BaseApp
{
public:
	
	App();
	virtual ~App();
	
	virtual bool Init();
	virtual void Kill();
	virtual void Draw();
	virtual void OnScreenSizeChange();
	virtual void Update();
	virtual void OnEnterBackground();
	virtual bool OnPreInitVideo();
	void OnFullscreenToggleRequest();
	void Save();

	string GetVersionString();
	float GetVersion();
	int GetBuild();
	VariantDB * GetShared() {return &m_varDB;}
	Variant * GetVar(const string &keyName );
	Variant * GetVarWithDefault(const string &varName, const Variant &var) {return m_varDB.GetVarWithDefault(varName, var);}
	void OnExitApp(VariantList *pVarList);
#ifdef PLATFORM_HTML5
	bool IsDesktop();
	bool IsIOS();
#endif

	TranslationManager* GetTranslationManager() { return &m_transMgr; }
	ThemeManager* GetThemeManager() { return &m_themeMgr; }
	eGameMode GetGameMode() { return (eGameMode)m_varDB.GetVarWithDefault("gamemode", uint32(GAMEMODE_NORMAL))->GetUINT32(); }

private:
	TranslationManager m_transMgr;
	ThemeManager m_themeMgr;

	bool m_bDidPostInit;
	VariantDB m_varDB; //holds all data we want to save/load
	
};
 

extern App g_App;

App * GetApp();
const char * GetAppName();
const char * GetBundleName();
const char * GetBundlePrefix();

int RandomInt(int min, int max);
float RandomFloat(float min, float max);