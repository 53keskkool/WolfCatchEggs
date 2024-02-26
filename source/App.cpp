#include "PlatformPrecomp.h"
#include "App.h"
#include "GUI/MainMenu.h"
#include "Entity/EntityUtils.h"//create the classes that our globally library expects to exist somewhere.
#include "Renderer/SoftSurface.h"
#include "GUI/AboutMenu.h"
#include <chrono>
#include <random>
#ifdef PLATFORM_HTML5
#include "GUI/PhoneWebMenu.h"
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>
void SyncPersistentData();
#endif
 
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

#if defined RT_WEBOS || defined RTLINUX || defined PLATFORM_PSP2
#include "Audio/AudioManagerSDL.h"
AudioManagerSDL g_audioManager; //sound in windows/WebOS/Linux/html5
//AudioManager g_audioManager; //to disable sound
#elif defined PLATFORM_HTML5
//we have to do this, because for some reason channels don't halt themselves after ending playing and we run out of channels
//(and i haven't enough time to fix it properly at the moment)
#include "SDL/SDL_mixer.h"
#include "Audio/AudioManagerSDL.h"
class AudioManagerSDLHTML5 : public AudioManagerSDL
{
public:
	virtual AudioHandle Play(string fName, bool bLooping = false, bool bIsMusic = false, bool bAddBasePath = true, bool bForceStreaming = false)
	{
		AudioHandle handle = AudioManagerSDL::Play(fName, bLooping, bIsMusic, bAddBasePath, bForceStreaming);
		if (handle >= 64 || handle == 0)
		{
			//this is very bad and some sound might (and probably will) get cut off
			//halting all channels, so we have space for this and future sounds
			Mix_HaltChannel(-1);
			//play this sound again
			handle = AudioManagerSDL::Play(fName, bLooping, bIsMusic, bAddBasePath, bForceStreaming);
		}
		return handle;
	}
} g_audioManager;
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

	//GetBaseApp()->SetFPSVisible(true);
	
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

	string defaultLang = GetRegionString();
#ifdef PLATFORM_HTML5
	char* dLang = (char*)EM_ASM_PTR({
		return stringToNewUTF8(navigator.language);
		});
	defaultLang = dLang;
	free(dLang);
	defaultLang = defaultLang.substr(0, 2);
#endif
	std::string lang = m_varDB.GetVarWithDefault("language", defaultLang)->GetString();
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

	std::string theme = m_varDB.GetVarWithDefault("theme", Variant(GET_THEMEMGR->GetDefaultTheme()))->GetString();
	if (!m_themeMgr.Init(theme))
	{
		LogError("Failed to load theme %s, falling back to default", m_varDB.GetVar("theme")->GetString().c_str());
		m_varDB.GetVar("theme")->Set(GET_THEMEMGR->GetDefaultTheme());
		if (!m_themeMgr.Init(GET_THEMEMGR->GetDefaultTheme()))
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

void App::OnFullscreenToggleRequest()
{
#ifdef PLATFORM_HTML5
	EmscriptenFullscreenChangeEvent fullscreenStatus;
	emscripten_get_fullscreen_status(&fullscreenStatus);
	if (fullscreenStatus.isFullscreen)
	{
		emscripten_exit_fullscreen();
	}
	else
	{
		EmscriptenFullscreenStrategy s;
		memset(&s, 0, sizeof(s));
		s.scaleMode = EMSCRIPTEN_FULLSCREEN_SCALE_STRETCH;
		s.canvasResolutionScaleMode = EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_HIDEF;
		s.filteringMode = EMSCRIPTEN_FULLSCREEN_FILTERING_DEFAULT;
		emscripten_request_fullscreen_strategy("#canvas", 1, &s);
	}
#else
	BaseApp::OnFullscreenToggleRequest();
#endif
}

void App::Save()
{
	LogMsg("Saving our stuff");
	Entity* pGameMenu = GetEntityRoot()->GetEntityByName("GameMenu");
	if (pGameMenu)
	{
		pGameMenu->GetEntityByName("Level")->GetFunction("SaveScore")->sig_function(NULL);
	}
	m_varDB.Save("save.dat");
#ifdef PLATFORM_HTML5
	SyncPersistentData();
#endif
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
#ifdef PLATFORM_HTML5
		//check orientation and force fullscreen on mobile web
		if (!App::IsDesktop())
		{
			if (!PhoneWebMenuCreate()) MainMenuCreate(pGUIEnt);
		}
		else MainMenuCreate(pGUIEnt);
#else
		//we can't turn fullscreen on on HTML5... web security doesn't allow that :(
		if (GetVar("fullscreen")->GetUINT32())
		{
			GetMessageManager()->CallStaticFunction(CreateMainMenu, 200, NULL);
			GetBaseApp()->OnFullscreenToggleRequest();
		}
		else MainMenuCreate(pGUIEnt);
#endif
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
#ifdef PLATFORM_HTML5
	if (GetEntityRoot()->GetEntityByName("GUI"))
	{
		if (!App::IsDesktop())
		{
			Entity* pMenu = GetEntityRoot()->GetEntityByName("PhoneWebMenu");
			if (pMenu)
			{
				pMenu->GetFunction("OnScreenSizeChanged")->sig_function(NULL);
			}
			else if (!GetEntityRoot()->GetEntityByName("OldPhoneWebMenu"))
			{
				//check that everything is ok
				CreatePhoneWebMenu();
			}
		}
	}
#endif
	BaseApp::OnScreenSizeChange();
}

Variant * App::GetVar( const string &keyName )
{
	return GetShared()->GetVar(keyName);
}

std::string App::GetVersionString()
{
	return "V1.01";
}

float App::GetVersion()
{
	return 1.01f;
}

int App::GetBuild()
{
	return 2;
}

#ifdef PLATFORM_HTML5
bool App::IsDesktop()
{
	static bool bDone = false;
	static bool bResult = false;
	if (!bDone)
	{
		//taken from here http://detectmobilebrowsers.com/
		bResult = !EM_ASM_INT({ 
			let check = false;
			(function(a){if(/(android|bb\\d+|meego).+mobile|avantgo|bada\\/|blackberry|blazer|compal|elaine|fennec|hiptop|iemobile|ip(hone|od)|iris|kindle|lge |maemo|midp|mmp|mobile.+firefox|netfront|opera m(ob|in)i|palm( os)?|phone|p(ixi|re)\\/|plucker|pocket|psp|series(4|6)0|symbian|treo|up\\.(browser|link)|vodafone|wap|windows ce|xda|xiino/i.test(a)||/1207|6310|6590|3gso|4thp|50[1-6]i|770s|802s|a wa|abac|ac(er|oo|s\\-)|ai(ko|rn)|al(av|ca|co)|amoi|an(ex|ny|yw)|aptu|ar(ch|go)|as(te|us)|attw|au(di|\\-m|r |s )|avan|be(ck|ll|nq)|bi(lb|rd)|bl(ac|az)|br(e|v)w|bumb|bw\\-(n|u)|c55\\/|capi|ccwa|cdm\\-|cell|chtm|cldc|cmd\\-|co(mp|nd)|craw|da(it|ll|ng)|dbte|dc\\-s|devi|dica|dmob|do(c|p)o|ds(12|\\-d)|el(49|ai)|em(l2|ul)|er(ic|k0)|esl8|ez([4-7]0|os|wa|ze)|fetc|fly(\\-|_)|g1 u|g560|gene|gf\\-5|g\\-mo|go(\\.w|od)|gr(ad|un)|haie|hcit|hd\\-(m|p|t)|hei\\-|hi(pt|ta)|hp( i|ip)|hs\\-c|ht(c(\\-| |_|a|g|p|s|t)|tp)|hu(aw|tc)|i\\-(20|go|ma)|i230|iac( |\\-|\\/)|ibro|idea|ig01|ikom|im1k|inno|ipaq|iris|ja(t|v)a|jbro|jemu|jigs|kddi|keji|kgt( |\\/)|klon|kpt |kwc\\-|kyo(c|k)|le(no|xi)|lg( g|\\/(k|l|u)|50|54|\\-[a-w])|libw|lynx|m1\\-w|m3ga|m50\\/|ma(te|ui|xo)|mc(01|21|ca)|m\\-cr|me(rc|ri)|mi(o8|oa|ts)|mmef|mo(01|02|bi|de|do|t(\\-| |o|v)|zz)|mt(50|p1|v )|mwbp|mywa|n10[0-2]|n20[2-3]|n30(0|2)|n50(0|2|5)|n7(0(0|1)|10)|ne((c|m)\\-|on|tf|wf|wg|wt)|nok(6|i)|nzph|o2im|op(ti|wv)|oran|owg1|p800|pan(a|d|t)|pdxg|pg(13|\\-([1-8]|c))|phil|pire|pl(ay|uc)|pn\\-2|po(ck|rt|se)|prox|psio|pt\\-g|qa\\-a|qc(07|12|21|32|60|\\-[2-7]|i\\-)|qtek|r380|r600|raks|rim9|ro(ve|zo)|s55\\/|sa(ge|ma|mm|ms|ny|va)|sc(01|h\\-|oo|p\\-)|sdk\\/|se(c(\\-|0|1)|47|mc|nd|ri)|sgh\\-|shar|sie(\\-|m)|sk\\-0|sl(45|id)|sm(al|ar|b3|it|t5)|so(ft|ny)|sp(01|h\\-|v\\-|v )|sy(01|mb)|t2(18|50)|t6(00|10|18)|ta(gt|lk)|tcl\\-|tdg\\-|tel(i|m)|tim\\-|t\\-mo|to(pl|sh)|ts(70|m\\-|m3|m5)|tx\\-9|up(\\.b|g1|si)|utst|v400|v750|veri|vi(rg|te)|vk(40|5[0-3]|\\-v)|vm40|voda|vulc|vx(52|53|60|61|70|80|81|83|85|98)|w3c(\\-| )|webc|whit|wi(g |nc|nw)|wmlb|wonu|x700|yas\\-|your|zeto|zte\\-/i.test(a.substr(0,4)))check=true;})(navigator.userAgent||navigator.vendor||window.opera);
			return check;
		});
		bDone = true;
	}
	return bResult;
}

bool App::IsIOS()
{
	static bool bDone = false;
	static bool bResult = false;
	if (!bDone)
	{
		bResult = EM_ASM_INT({
			return (['iPhone Simulator', 'iPod Simulator', 'iPhone', 'iPod']).includes(navigator.platform);
		});
		bDone = true;
	}
	return bResult;
}
#endif

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
	SetPrimaryScreenSize(1280, 720);
	SetupScreenInfo(1280, 720, ORIENTATION_DONT_CARE);
#endif
	return true; //no error
}

typedef std::mt19937 rng_type;
rng_type g_rng(rng_type::result_type(chrono::steady_clock::now().time_since_epoch().count()* chrono::steady_clock::now().time_since_epoch().count()));
int RandomInt(int min, int max) {
	std::uniform_int_distribution<int> udist(min, max);
	return udist(g_rng);
}
float RandomFloat(float min, float max) {
	std::uniform_real_distribution<float> udist(min, max);
	return udist(g_rng);
}