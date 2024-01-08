#include "PlatformPrecomp.h"
#include "App.h"
#include "GUI/MainMenu.h"
#include "Entity/EntityUtils.h"//create the classes that our globally library expects to exist somewhere.
#include "Renderer/SoftSurface.h"
#include "GUI/AboutMenu.h"
#include <chrono>
#include <random>
 
SurfaceAnim g_surf;
 
MessageManager g_messageManager;
MessageManager * GetMessageManager() {return &g_messageManager;}

FileManager g_fileManager;
FileManager * GetFileManager() {return &g_fileManager;}

#ifdef __APPLE__

#if TARGET_OS_IPHONE == 1
	//it's an iPhone or iPad
	//#include "Audio/AudioManagerOS.h"
	//AudioManagerOS g_audioManager;
	#include "Audio/AudioManagerDenshion.h"
	
	AudioManagerDenshion g_audioManager;
#else
	//it's being compiled as a native OSX app
   #include "Audio/AudioManagerFMODStudio.h"
  AudioManagerFMOD g_audioManager; //dummy with no sound

//in theory, CocosDenshion should work for the Mac builds, but right now it seems to want a big chunk of
//Cocos2d included so I'm not fiddling with it for now

//#include "Audio/AudioManagerDenshion.h"
//AudioManagerDenshion g_audioManager;
#endif
	
#else

#if defined RT_WEBOS || defined RTLINUX || defined PLATFORM_HTML5 || defined PLATFORM_PSP2
#include "Audio/AudioManagerSDL.h"
AudioManagerSDL g_audioManager; //sound in windows/WebOS/Linux/html5
//AudioManager g_audioManager; //to disable sound
#elif defined ANDROID_NDK

	#if defined RT_ENABLE_FMOD
		//FMOD works on android these days, but you'll have to link it right
		#include "Audio/AudioManagerFMODStudio.h"
		AudioManagerFMOD g_audioManager; //if we wanted FMOD sound in windows
	#else
		#include "Audio/AudioManagerAndroid.h"
		AudioManagerAndroid g_audioManager; //sound for android
	#endif

#elif defined PLATFORM_BBX
#include "Audio/AudioManagerBBX.h"
//AudioManager g_audioManager; //to disable sound
AudioManagerBBX g_audioManager;
#elif defined PLATFORM_FLASH
//AudioManager g_audioManager; //to disable sound
#include "Audio/AudioManagerFlash.h"
AudioManagerFlash *g_audioManager = new AudioManagerFlash;
#else

#ifdef RT_USE_SDL_AUDIO
#include "Audio/AudioManagerSDL.h"
AudioManagerSDL g_audioManager; //sound in windows and WebOS

#elif defined RT_FLASH_TEST
#include "Audio/AudioManagerFlash.h"
AudioManagerFlash g_audioManager;
#elif defined RT_ENABLE_FMOD
#include "Audio/AudioManagerFMODStudio.h"
AudioManagerFMOD g_audioManager; //if we wanted FMOD sound in windows
#else

//in windows
//AudioManager g_audioManager; //to disable sound

#include "Audio/AudioManagerAudiere.h"
AudioManagerAudiere g_audioManager;  //Use Audiere for audio
#endif

#endif
#endif

#if defined PLATFORM_FLASH
	AudioManager * GetAudioManager(){return g_audioManager;}
#else
	AudioManager * GetAudioManager(){return &g_audioManager;}
#endif

App *g_pApp = NULL;
BaseApp * GetBaseApp() 
{
	if (!g_pApp)
	{
		#ifndef NDEBUG
		LogMsg("Creating app object");
		#endif
		g_pApp = new App;
	}

	return g_pApp;
}

App * GetApp() 
{
	return g_pApp;
}

App::App()
{
	m_bDidPostInit = false;
}

App::~App()
{
#ifdef PLATFORM_FLASH
	SAFE_DELETE(g_audioManager);
#endif
}

void App::OnExitApp(VariantList *pVarList)
{
	LogMsg("Exiting the app");

	OSMessage o;
	o.m_type = OSMessage::MESSAGE_FINISH_APP;
	GetBaseApp()->AddOSMessage(o);
}

bool App::Init()
{
#ifdef PLATFORM_WINDOWS
	extern bool g_winAllowFullscreenToggle;
	g_winAllowFullscreenToggle = false;
#endif
	//SetDefaultAudioClickSound("audio/enter.wav");
	SetDefaultButtonStyle(Button2DComponent::BUTTON_STYLE_CLICK_ON_TOUCH_RELEASE);
    
	switch (GetEmulatedPlatformID())
	{
		//special handling for certain platforms to tweak the video settings

	case PLATFORM_ID_WEBOS:
		//if we do this, everything will be stretched/zoomed to fit the screen
		if (IsIPADSize)
		{
			//doesn't need rotation
			SetLockedLandscape(false);  //because it's set in the app manifest, we don't have to rotate ourselves
			SetupScreenInfo(GetPrimaryGLX(), GetPrimaryGLY(), ORIENTATION_PORTRAIT);
		} 
else 
		{
			//but the phones do
			SetLockedLandscape(true); //we don't allow portrait mode for this game
		}
		
		break;

		case PLATFORM_ID_IOS:
			SetLockedLandscape(true); //we stay in portrait but manually rotate, gives better fps on older devices
			break;
			
	default:
		
		//Default settings for other platforms

		SetLockedLandscape(false); //we don't allow portrait mode for this game
	}

	if (m_bInitted)	
	{
		return true;
	}
	
	if (!BaseApp::Init()) return false;


	LogMsg("Save path is %s", GetSavePath().c_str());

	GetBaseApp()->SetFPSVisible(true);
	
	bool bFileExisted;
	m_varDB.Load("save.dat", &bFileExisted);

	float musicVol = m_varDB.GetVarWithDefault("music_vol", 1.0f)->GetFloat();
	float sfxVol = m_varDB.GetVarWithDefault("sfx_vol", 1.0f)->GetFloat();
	bool soundDisabled = m_varDB.GetVarWithDefault("soundDisabled", (uint32)0)->GetUINT32();

	GetAudioManager()->SetMusicVol(musicVol);
	GetAudioManager()->SetDefaultVol(sfxVol);
	GetAudioManager()->SetSoundEnabled(!soundDisabled);
 
	//preload audio
	GetAudioManager()->Preload("audio/click.wav");

	std::string lang = m_varDB.GetVarWithDefault("language", Variant((GetRegionString() == "ru" ? "ru" : "en")))->GetString();
	if (!m_transMgr.Init(lang))
	{
		LogError("Failed to load language %s, falling back to 'en'", m_varDB.GetVar("language")->GetString().c_str());
		m_varDB.GetVar("language")->Set("en");
		if (!m_transMgr.Init("en"))
		{
			LogError("Failed to load texts");
			return false;
		}
	}

	std::string theme = m_varDB.GetVarWithDefault("theme", Variant("default"))->GetString();
	if (!m_themeMgr.Init(theme))
	{
		LogError("Failed to load theme %s, falling back to default", m_varDB.GetVar("theme")->GetString().c_str());
		m_varDB.GetVar("theme")->Set("default");
		if (!m_themeMgr.Init("default"))
		{
			LogError("Failed to load theme");
			return false;
		}
	}

	if (IsLargeScreen())
	{
		if (!GetFont(FONT_SMALL)->Load("interface/font_century_gothicx2.rtfont"))
		{
			LogMsg("Can't load small font");
			return false;
		}
		if (!GetFont(FONT_LARGE)->Load("interface/font_century_gothic_bigx2.rtfont"))
		{
			LogMsg("Can't load big font");
			return false;
		}
	}
	else
	{
		if (!GetFont(FONT_SMALL)->Load("interface/font_century_gothic.rtfont"))
		{
			LogMsg("Can't load small font");
			return false;
		}
		if (!GetFont(FONT_LARGE)->Load("interface/font_century_gothic_big.rtfont"))
		{
			LogMsg("Can't load big font");
			return false;
		}
	}
	//GetFont(FONT_SMALL)->SetSmoothing(false); //if we wanted to disable bilinear filtering on the font

	return true;
}

void App::Save()
{
	LogMsg("Saving our stuff");
	m_varDB.Save("save.dat");
}

void App::Kill()
{
	Save();
	BaseApp::Kill();
	g_pApp = NULL;
}

void App::Update()
{
	BaseApp::Update();

	if (!m_bDidPostInit)
	{
		m_bDidPostInit = true;
	
		//build a dummy entity called "GUI" to put our GUI menu entities under
		Entity *pGUIEnt = GetEntityRoot()->AddEntity(new Entity("GUI"));
		if (GetVar("fullscreen")->GetUINT32())
		{
			GetMessageManager()->CallStaticFunction(CreateMainMenu, 200, NULL);
			GetBaseApp()->OnFullscreenToggleRequest();
		}
		else MainMenuCreate(pGUIEnt);
	}
}

void App::Draw()
{
	PrepareForGL();
//	glClearColor(0.6,0.6,0.6,1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	CLEAR_GL_ERRORS(); //needed for html5

	BaseApp::Draw();
}

void App::OnEnterBackground()
{
	BaseApp::OnEnterBackground();
	Save();

}
void App::OnScreenSizeChange()
{
	BaseApp::OnScreenSizeChange();
}

Variant * App::GetVar( const string &keyName )
{
	return GetShared()->GetVar(keyName);
}

std::string App::GetVersionString()
{
	return "V0.01";
}

float App::GetVersion()
{
	return 0.01f;
}

int App::GetBuild()
{
	return 1;
}

const char * GetAppName() {return "WolfCatchEggs";}

//for palm webos and android
const char * GetBundlePrefix()
{
	const char * bundlePrefix = "ee.t53kk.";
	return bundlePrefix;
}

const char * GetBundleName()
{
	const char * bundleName = "WolfCatchEggs";
	return bundleName;
}

bool App::OnPreInitVideo()
{
	//only called for desktop systems
	//override in App.* if you want to do something here.  You'd have to
	//extern these vars from main.cpp to change them...

	//SetEmulatedPlatformID(PLATFORM_ID_WINDOWS);
#if defined(WINAPI)
	SetPrimaryScreenSize(1024, 768);
	SetupScreenInfo(1024, 768, ORIENTATION_DONT_CARE);
#endif
	return true; //no error
}

typedef std::mt19937 rng_type;
rng_type g_rng(rng_type::result_type(chrono::steady_clock::now().time_since_epoch().count()* chrono::steady_clock::now().time_since_epoch().count()));
int RandomInt(int min, int max) {
	std::uniform_int_distribution<rng_type::result_type> udist(min, max);
	return udist(g_rng);
}
float RandomFloat(float min, float max) {
	std::uniform_real_distribution<float> udist(min, max);
	return udist(g_rng);
}