#include "r3dPCH.h"
#include "r3d.h"

#include "m_LoadingScreen.h"
#include "RmlUiBackend.h"
#include "GameCode\UserProfile.h"

#include "Multiplayer\MasterServerLogic.h"

#include "LangMngr.h"

#include "r3dDeviceQueue.h"

#include <string>

LoadingScreen::LoadingScreen( const char * movieName ) 
: UIMenu(movieName) 
{
	m_pBackgroundTex = 0;
	m_RenderingDisabled = false;
	m_RmlDocument = NULL;
	m_Progress = 0.0f;
	r3dscpy(m_MapName, "Loading");
	r3dscpy(m_MapDesc, "Preparing renderer");
	r3dscpy(m_MapMode, "");
	r3dscpy(m_MapType, "");
	r3dscpy(m_TipCaption, "DX11");
	r3dscpy(m_TipText, "Loading game resources");
}

//------------------------------------------------------------------------

LoadingScreen::~LoadingScreen()
{
	Unload();

	if(m_pBackgroundTex)
		r3dRenderer->DeleteTexture(m_pBackgroundTex);
	m_pBackgroundTex = 0;
}

//------------------------------------------------------------------------

namespace
{
	const char* g_LoadingTemplatePath = "Data\\UI\\LoadingScreen.rml";
	const char* g_LoadingDefaultBackgroundPath = "Data\\UI\\Assets\\ConnectScreen.dds";

	std::string LoadingEscapeRml(const char* text)
	{
		std::string out;
		if(!text)
			return out;

		for(const char* c = text; *c; ++c)
		{
			switch(*c)
			{
			case '&': out += "&amp;"; break;
			case '<': out += "&lt;"; break;
			case '>': out += "&gt;"; break;
			case '"': out += "&quot;"; break;
			case '\'': out += "&apos;"; break;
			default: out += *c; break;
			}
		}

		return out;
	}

	const char* LoadingWideToUtf8(const wchar_t* text)
	{
		return text ? wideToUtf8(text) : "";
	}
}

bool LoadingScreen::Load()
{
	SetRenderingDisabled(false);

	if(!m_pBackgroundTex && r3dFileExists(g_LoadingDefaultBackgroundPath))
		SetLoadingTexture(g_LoadingDefaultBackgroundPath);

	if(!CreateRmlDocument())
		return false;

	_MenuCode = 1;
	return true;
}

bool LoadingScreen::Unload()
{
	CloseRmlDocument();
	return true;
}

//------------------------------------------------------------------------

bool LoadingScreen::Initialize()
{
//	char EulaText[70000] = "";

	return true;
}

void ClearFullScreen_Menu();
void RestoreDX11MenuBackBuffer(const char* where);

bool LoadingScreen::CreateRmlDocument()
{
	if(m_RmlDocument)
		return true;

	if(!RmlUiBackend::Initialize())
	{
		r3dOutToLog("RmlUi LoadingScreen: backend initialization failed\n");
		return false;
	}

	m_RmlDocument = RmlUiBackend::LoadDocumentFromFile(g_LoadingTemplatePath);
	if(!m_RmlDocument)
	{
		r3dOutToLog("RmlUi LoadingScreen: failed to load template '%s'\n", g_LoadingTemplatePath);
		return false;
	}

	m_RmlDocument->Show();
	UpdateRmlData();
	return true;
}

void LoadingScreen::CloseRmlDocument()
{
	if(m_RmlDocument)
	{
		m_RmlDocument->Close();
		m_RmlDocument->RemoveReference();
		m_RmlDocument = NULL;
	}
}

void LoadingScreen::UpdateRmlData()
{
	if(!m_RmlDocument)
		return;

	Rocket::Core::Element* element = NULL;

	element = m_RmlDocument->GetElementById("map_name");
	if(element)
		element->SetInnerRML(LoadingEscapeRml(m_MapName).c_str());

	element = m_RmlDocument->GetElementById("map_desc");
	if(element)
		element->SetInnerRML(LoadingEscapeRml(m_MapDesc).c_str());

	element = m_RmlDocument->GetElementById("map_mode");
	if(element)
	{
		std::string mode = LoadingEscapeRml(m_MapMode);
		if(m_MapType[0])
		{
			mode += " / ";
			mode += LoadingEscapeRml(m_MapType);
		}
		element->SetInnerRML(mode.c_str());
	}

	element = m_RmlDocument->GetElementById("tip_caption");
	if(element)
		element->SetInnerRML(LoadingEscapeRml(m_TipCaption).c_str());

	element = m_RmlDocument->GetElementById("tip_text");
	if(element)
		element->SetInnerRML(LoadingEscapeRml(m_TipText).c_str());

	char progressText[64];
	sprintf(progressText, "%d%%", (int)(m_Progress * 100.0f + 0.5f));

	element = m_RmlDocument->GetElementById("progress_text");
	if(element)
		element->SetInnerRML(progressText);

	char progressWidth[64];
	sprintf(progressWidth, "%0.2f%%", R3D_MAX(0.0f, R3D_MIN(m_Progress, 1.0f)) * 100.0f);

	element = m_RmlDocument->GetElementById("progress_bar");
	if(element)
		element->SetProperty("width", progressWidth);
}

int LoadingScreen::Update()
{
	R3D_ENSURE_MAIN_THREAD();

	r3dMouse::Show();
	r3dStartFrame();

	if( r3dRenderer->DeviceAvailable )
	{
		r3dRenderer->StartRender(1);
		r3dRenderer->StartFrame();

		r3dRenderer->SetRenderingMode(R3D_BLEND_ALPHA | R3D_BLEND_NZ);

		RestoreDX11MenuBackBuffer("Loading before clear");
		ClearFullScreen_Menu();

		RestoreDX11MenuBackBuffer("Loading before background");

		float x, y, w, h;
		r3dRenderer->GetBackBufferViewport(&x, &y, &w, &h);

		r3dRenderer->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
		if(m_pBackgroundTex)
			r3dDrawBox2D(x, y, w, h, r3dColor24::white, m_pBackgroundTex);
		else
			r3dDrawBox2D(x, y, w, h, r3dColor(12, 14, 18));

		r3dRenderer->Flush();

		if(!m_RenderingDisabled && m_RmlDocument)
		{
			Rocket::Core::Context* ctx = RmlUiBackend::GetContext();
			if(ctx)
			{
				ctx->SetDimensions(Rocket::Core::Vector2i((int)r3dRenderer->ScreenW, (int)r3dRenderer->ScreenH));
				ctx->Update();

				RmlUiBackend::BeginFrame();
				ctx->Render();
				RmlUiBackend::EndFrame();
				RestoreDX11MenuBackBuffer("Loading after RmlUi");
			}
		}

		r3dRenderer->Flush();
		r3dRenderer->EndFrame();
	}

	r3dRenderer->EndRender( true );
	r3dEndFrame();

	return 0;
}

//------------------------------------------------------------------------
void LoadingScreen::SetLoadingTexture(const char* ImagePath)
{
	R3D_ENSURE_MAIN_THREAD();

	if(m_pBackgroundTex)
		r3dRenderer->DeleteTexture(m_pBackgroundTex);
	m_pBackgroundTex = r3dRenderer->LoadTexture(ImagePath);
	if(!m_pBackgroundTex)
		r3dOutToLog("RmlUi LoadingScreen: failed to load background '%s'\n", ImagePath ? ImagePath : "");
}

void LoadingScreen::SetData( const char* ImagePath, const wchar_t* Name, const wchar_t* Message, int mapType, const wchar_t* tip_of_the_day )
{
	R3D_ENSURE_MAIN_THREAD();

	SetLoadingTexture(ImagePath);

	r3dscpy(m_MapName, LoadingWideToUtf8(Name));
	r3dscpy(m_MapDesc, LoadingWideToUtf8(Message));
	r3dscpy(m_MapMode, "");
	r3dscpy(m_MapType, "");
	r3dscpy(m_TipCaption, "");
	r3dscpy(m_TipText, LoadingWideToUtf8(tip_of_the_day));

	if(g_num_matches_played->GetInt() == 1 && g_num_game_executed2->GetInt()==1) // first time launch of the game, show keyboard schematic
	{
		r3dscpy(m_TipCaption, "Controls");
		if(!m_TipText[0])
			r3dscpy(m_TipText, "Check movement, weapon, and interaction bindings before joining the match.");
	}
	else
	{
		r3dscpy(m_TipCaption, LoadingWideToUtf8(gLangMngr.getString("TipOfTheDay")));
	}

	{
		const wchar_t* gameMode=L""; const char* modeType="";
		switch(mapType)
		{
		case GBGameInfo::MAPT_Bomb:
			gameMode = gLangMngr.getString("$GameMode_Sabotage");
			modeType = "Siege";
			break;
		case GBGameInfo::MAPT_Conquest:
			gameMode = gLangMngr.getString("$GameMode_Conquest");
			modeType = "Conquest";
			break;
		case GBGameInfo::MAPT_Deathmatch:
			gameMode = gLangMngr.getString("$GameMode_Deathmatch");
			modeType = "DM";
			break;
		default:
			break;
		}
		r3dscpy(m_MapMode, LoadingWideToUtf8(gameMode));
		r3dscpy(m_MapType, modeType);
	}

	UpdateRmlData();
}

//------------------------------------------------------------------------

void
LoadingScreen::SetProgress( float progress )
{
	R3D_ENSURE_MAIN_THREAD();

	m_Progress = R3D_MAX(0.0f, R3D_MIN(progress, 1.0f));
	UpdateRmlData();
}

//------------------------------------------------------------------------

static volatile float	gProgress;
static LoadingScreen*	gLoadingScreen;

void StartLoadingScreen()
{
	r3d_assert( !gLoadingScreen );
	gLoadingScreen = new LoadingScreen( "RmlUiLoadingScreen" );

	gLoadingScreen->Load();
	gLoadingScreen->Initialize();
}
void DisableLoadingRendering()
{
	gLoadingScreen->SetRenderingDisabled( true );
}

//------------------------------------------------------------------------

void StopLoadingScreen()
{
	r3d_assert( gLoadingScreen );
	delete gLoadingScreen;

	gLoadingScreen = NULL;
}

void UpdateLoadingScreenOnce()
{
	if(gLoadingScreen)
		gLoadingScreen->Update();
}

void SetLoadingTexture(const char* ImagePath)
{
	if(gLoadingScreen)
	{
		gLoadingScreen->SetLoadingTexture(ImagePath);
	}
}

//------------------------------------------------------------------------

void SetLoadingProgress( float progress )
{
	progress = R3D_MAX( R3D_MIN( progress, 1.f ), 0.f );
	InterlockedExchange( (volatile long*)&gProgress, (LONG&)progress );
}

//------------------------------------------------------------------------

void AdvanceLoadingProgress( float add )
{
	float newVal = R3D_MAX( R3D_MIN( gProgress + add, 1.f ), 0.f );
	InterlockedExchange( (volatile long*)&gProgress, (LONG&)newVal );
}

float GetLoadingProgress()
{
	return gProgress;
}

//------------------------------------------------------------------------

void SetLoadingPhase( const char* Phase )
{
	Phase;
}

//------------------------------------------------------------------------

int DoLoadingScreen( volatile LONG* Loading, const wchar_t* LevelName, const wchar_t* LevelDescription, const char* LevelFolder, float TimeOut, int gameMode )
{
	r3d_assert( gLoadingScreen );

	char sFullPath[512];
	sprintf( sFullPath, "%s\\%s", LevelFolder, "LoadingScreen.dds" );

	// no dollar sign for access
	if(r3d_access(sFullPath + 1 , 0) != 0)
	{
		int sel = rand()%3;
		sprintf( sFullPath, "%s\\LoadingScreen%d.dds", LevelFolder, sel );
	}

	char tempStr[32];
	sprintf(tempStr, "TipOfTheDay%d", int(floorf(u_GetRandom(0.0f, 12.99f))));
	if(gameMode == GBGameInfo::MAPT_Conquest || gameMode == GBGameInfo::MAPT_Bomb)
	{
		if(u_GetRandom(0.0f, 1.0f) > 0.7f)
		{
			if(gameMode == GBGameInfo::MAPT_Conquest)
				sprintf(tempStr, "ConquestTipOfTheDay%d", int(floorf(u_GetRandom(0.0f, 3.99f))));
			else if(gameMode == GBGameInfo::MAPT_Bomb)
				sprintf(tempStr, "SabotageTipOfTheDay%d", int(floorf(u_GetRandom(0.0f, 4.99f))));
		}
	}
	gLoadingScreen->SetData( sFullPath, LevelName, LevelDescription, gameMode, gLangMngr.getString(tempStr));

	bool checkTimeOut = TimeOut != 0.f;

	float endWait = r3dGetTime() + TimeOut;

	while( *Loading )
	{
		if( checkTimeOut && r3dGetTime() > endWait ) 
		{
			return 0;
		}

		r3dProcessWindowMessages();

		if( r3dRenderer->DeviceAvailable )
		{
			float timeStart = r3dGetTime();

			float MaxRenderTime = 0.033f;

			for( ; r3dGetTime() - timeStart < 0.033f ; )
			{
				ProcessDeviceQueue( timeStart, MaxRenderTime ) ;
			}
		}

		gLoadingScreen->Update();
		gLoadingScreen->SetProgress( gProgress );
	}

	return 1;
}

//------------------------------------------------------------------------
bool IsNeedExit();
int DoConnectScreen( volatile LONG* Loading, const wchar_t* Message, float TimeOut )
{
	r3d_assert( gLoadingScreen );

	gLoadingScreen->SetData( g_LoadingDefaultBackgroundPath, gLangMngr.getString("Connecting"), Message, -1, NULL );

	bool checkTimeOut = TimeOut != 0.f;

	float endWait = r3dGetTime() + TimeOut;

	while( *Loading )
	{
		r3dProcessWindowMessages();

		if(IsNeedExit())
			return 0;
		if( checkTimeOut && r3dGetTime() > endWait ) 
		{
			return 0;
		}

		gLoadingScreen->Update();
		gLoadingScreen->SetProgress( checkTimeOut ? 1.f - ( endWait - r3dGetTime() ) / TimeOut : gProgress );
		Sleep( 33 );
	}

	return 1;
}

//------------------------------------------------------------------------

template <typename T>
int DoConnectScreen( T* Logic, bool (T::*CheckFunc)(), const wchar_t* Message, float TimeOut )
{
	r3d_assert( gLoadingScreen );

	gLoadingScreen->SetData( g_LoadingDefaultBackgroundPath, gLangMngr.getString("Connecting"), Message, -1, NULL );

	bool checkTimeOut = TimeOut != 0.f;

	const float startWait = r3dGetTime();
	const float endWait = startWait + TimeOut;

	for(;;)
	{
		extern void tempDoMsgLoop();
		tempDoMsgLoop();

		if( (Logic->*CheckFunc)() )
			break;

		if(IsNeedExit())
			return 0;
		if( checkTimeOut && r3dGetTime() > endWait ) 
		{
			return 0;
		}

		// draw loaing screen only after some time
		// so minor waits will be performed without graphics change
		if(r3dGetTime() > startWait + 1.0f)
		{
			gLoadingScreen->Update();
			gLoadingScreen->SetProgress( checkTimeOut ? 1.f - ( endWait - r3dGetTime() ) / TimeOut : gProgress );
		}

		Sleep( 33 );
	}

	return 1;
}

template int DoConnectScreen( ClientGameLogic* Logic, bool (ClientGameLogic::*CheckFunc)(), const wchar_t* Message, float TimeOut );
template int DoConnectScreen( MasterServerLogic* Logic, bool (MasterServerLogic::*CheckFunc)(), const wchar_t* Message, float TimeOut );

//------------------------------------------------------------------------

