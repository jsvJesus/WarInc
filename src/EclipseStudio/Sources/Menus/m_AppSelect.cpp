#include "r3dPCH.h"
#include "r3d.h"

#include "GameCommon.h"
#include "UI/UIimEdit.h"
#include "UI/RmlUiBackend.h"
#include "r3dDX11.h"
#include "r3dDX11Geometry.h"
#include "r3dDX11State.h"

#include "m_AppSelect.h"

extern int AppSelectMode;
void ClearFullScreen_Menu();

namespace
{
	const int APPSELECT_WAITING_FOR_COMMAND = 100;

	const AppSelectCommandBinding g_AppSelectCommands[] =
	{
		{ "play",      "Play Game",        Menu_AppSelect::bStartGamePublic,      kbs1 },
		{ "level",     "Level Editor",     Menu_AppSelect::bStartLevelEditor,     kbs2 },
		{ "particle",  "Particle Editor",  Menu_AppSelect::bStartParticleEditor,  kbs3 },
		{ "physics",   "Physics Editor",   Menu_AppSelect::bStartPhysicsEditor,   kbs4 },
		{ "character", "Character Editor", Menu_AppSelect::bStartCharacterEditor, kbs5 },
		{ "exit",      "Exit",             Menu_AppSelect::bQuit,                 kbs6 },
	};

	const char* g_AppSelectTemplatePath = "Data\\UI\\m_AppSelect.rml";
	const char* g_EditorBackgroundPath = "Data\\UI\\Assets\\Background.png";

	class AppSelectEventListener : public Rocket::Core::EventListener
	{
	public:
		AppSelectEventListener()
		: m_result(APPSELECT_WAITING_FOR_COMMAND)
		{
		}

		void SetResult(int result)
		{
			m_result = result;
		}

		virtual void ProcessEvent(Rocket::Core::Event& event)
		{
			r3dOutToLog("RmlUi AppSelect: selected mode %d\n", m_result);
			AppSelectMode = m_result;
		}

	private:
		int m_result;
	};

	Rocket::Core::ElementDocument* LoadAppSelectDocument(AppSelectEventListener* listeners)
	{
		Rocket::Core::ElementDocument* document = RmlUiBackend::LoadDocumentFromFile("Data/UI/m_AppSelect.rml");

		if(!document)
		{
			r3dOutToLog("RmlUi AppSelect: LoadDocumentFromFile failed: Data/UI/m_AppSelect.rml\n");
			return NULL;
		}

		for(int i = 0; i < R3D_ARRAYSIZE(g_AppSelectCommands); ++i)
		{
			listeners[i].SetResult(g_AppSelectCommands[i].Result);

			Rocket::Core::Element* element = document->GetElementById(g_AppSelectCommands[i].ElementId);

			if(element)
				element->AddEventListener("click", &listeners[i]);
			else
				r3dOutToLog("RmlUi AppSelect: element '%s' is missing\n", g_AppSelectCommands[i].ElementId);
		}

		document->Show();

		r3dOutToLog("RmlUi AppSelect: document loaded from file and shown\n");

		return document;
	}

	void CloseAppSelectDocument(Rocket::Core::ElementDocument*& document, AppSelectEventListener* listeners)
	{
		if(!document)
			return;

		for(int i = 0; i < R3D_ARRAYSIZE(g_AppSelectCommands); ++i)
		{
			Rocket::Core::Element* element = document->GetElementById(g_AppSelectCommands[i].ElementId);
			if(element)
				element->RemoveEventListener("click", &listeners[i]);
		}

		document->Close();
		document->RemoveReference();
		document = NULL;

		Rocket::Core::Context* ctx = RmlUiBackend::GetContext();
		if(ctx)
			ctx->Update();
	}

	r3dTexture* LoadAppSelectBackground()
	{
		r3dOutToLog("RmlUi AppSelect: checking background '%s'\n", g_EditorBackgroundPath);

		if(!r3dFileExists(g_EditorBackgroundPath))
		{
			r3dOutToLog("RmlUi AppSelect: background file not found: '%s'\n", g_EditorBackgroundPath);
			return NULL;
		}

		r3dTexture* texture = r3dRenderer->LoadTexture(g_EditorBackgroundPath);

		if(!texture)
		{
			r3dOutToLog("RmlUi AppSelect: LoadTexture failed: '%s'\n", g_EditorBackgroundPath);
			return NULL;
		}

		if(!texture->IsValid())
		{
			r3dOutToLog("RmlUi AppSelect: texture is invalid: '%s'\n", g_EditorBackgroundPath);
			r3dRenderer->DeleteTexture(texture);
			return NULL;
		}

#ifndef WO_SERVER
		if(g_r3dDX11.IsInitialized() && !texture->HasDX11Texture())
		{
			r3dOutToLog("RmlUi AppSelect: texture has no DX11 texture/SRV: '%s'\n", g_EditorBackgroundPath);
		}
#endif

		r3dOutToLog("RmlUi AppSelect: background loaded OK: '%s'\n", g_EditorBackgroundPath);
		return texture;
	}

	void DrawAppSelectBackground(r3dTexture* backgroundTexture)
	{
		r3dRenderer->SetRenderingMode(R3D_BLEND_ALPHA | R3D_BLEND_NZ);
		ClearFullScreen_Menu();

		float x = 0.0f;
		float y = 0.0f;
		float w = 0.0f;
		float h = 0.0f;
		r3dRenderer->GetBackBufferViewport(&x, &y, &w, &h);

		if(backgroundTexture)
			r3dDrawBox2D(x, y, w, h, r3dColor24::white, backgroundTexture);
		else
			r3dDrawBox2D(x, y, w, h, r3dColor(12, 14, 18));
	}
}

int AppSelectMode = APPSELECT_WAITING_FOR_COMMAND;

Menu_AppSelect::Menu_AppSelect()
{
}

Menu_AppSelect::~Menu_AppSelect()
{
}

void Menu_AppSelect::Draw()
{
}

extern bool g_bExit;

void ClearFullScreen_Menu()
{
	r3dRenderer->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);

	r3dRenderer->DoSetViewport(
		0.0f,
		0.0f,
		(float)r3dRenderer->d3dpp.BackBufferWidth,
		(float)r3dRenderer->d3dpp.BackBufferHeight
	);

	r3dRenderer->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0, 1.0f, 0);

	r3dRenderer->SetRenderState(D3DRS_SCISSORTESTENABLE, TRUE);
}

void RestoreDX11MenuBackBuffer(const char* where)
{
	(void)where;

#ifndef WO_SERVER
	if(r3dRenderer && !r3dRenderer->GetUseD3D9Present() && g_r3dDX11.IsInitialized())
	{
		g_r3dDX11.ResetBackBufferTarget();

		const float width = (float)R3D_MAX(g_r3dDX11.GetWidth(), 1);
		const float height = (float)R3D_MAX(g_r3dDX11.GetHeight(), 1);

		r3dRenderer->ScreenW = width;
		r3dRenderer->ScreenH = height;
		r3dRenderer->ScreenW2 = width * 0.5f;
		r3dRenderer->ScreenH2 = height * 0.5f;
		r3dRenderer->AllowNullViewport = 0;
		r3dRenderer->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
		r3dRenderer->DoSetViewport(0.0f, 0.0f, width, height);

		g_r3dDX11State.InvalidateCache();
		g_r3dDX11Geometry.InvalidateCache();
	}
#endif
}

int Menu_AppSelect::DoModal()
{
	AppSelectMode = APPSELECT_WAITING_FOR_COMMAND;
	released_id = -1;

	Desktop().SetViewSize(r3dRenderer->ScreenW, r3dRenderer->ScreenH);
	r3dMouse::Show(true);

	if(!RmlUiBackend::Initialize())
	{
		r3dOutToLog("RmlUi AppSelect: backend initialization failed, falling back\n");
		return Menu_AppSelect::bStartGamePublic;
	}

	Rocket::Core::Context* ctx = RmlUiBackend::GetContext();
	if(!ctx)
	{
		r3dOutToLog("RmlUi AppSelect: no context, falling back\n");
		return Menu_AppSelect::bStartGamePublic;
	}

	AppSelectEventListener listeners[R3D_ARRAYSIZE(g_AppSelectCommands)];
	Rocket::Core::ElementDocument* document = LoadAppSelectDocument(listeners);

	if(!document)
	{
		return Menu_AppSelect::bStartGamePublic;
	}

	r3dTexture* backgroundTexture = LoadAppSelectBackground();
	bool firstRmlFrameLogged = false;

	while(AppSelectMode == APPSELECT_WAITING_FOR_COMMAND)
	{
		if(g_bExit)
		{
			AppSelectMode = Menu_AppSelect::bQuit;
			break;
		}

		r3dProcessWindowMessages();
		r3dMouse::Show(true);

		RmlUiBackend::ProcessMouse();

		mUpdate();

		ctx->SetDimensions(Rocket::Core::Vector2i((int)r3dRenderer->ScreenW, (int)r3dRenderer->ScreenH));
		ctx->Update();

		int hotkeyResult = RmlUiBackend::ProcessHotkeys(g_AppSelectCommands, R3D_ARRAYSIZE(g_AppSelectCommands));
		if(!hotkeyResult)
			hotkeyResult = RmlUiBackend::ProcessEscapeKey();

		if(hotkeyResult)
		{
			AppSelectMode = hotkeyResult;
			break;
		}

		r3dStartFrame();

		if(r3dRenderer->DeviceAvailable)
		{
			r3dRenderer->StartRender(1);
			r3dRenderer->StartFrame();

			DrawAppSelectBackground(backgroundTexture);
			r3dRenderer->Flush();

			RmlUiBackend::BeginFrame();
			if(ctx)
			{
				ctx->Render();

				if(!firstRmlFrameLogged)
				{
					r3dOutToLog("RmlUi AppSelect: first frame rendered through RmlUiBackend\n");
					firstRmlFrameLogged = true;
				}
			}
			RmlUiBackend::EndFrame();

			RestoreDX11MenuBackBuffer("AppSelect");
			r3dRenderer->SetRenderingMode(R3D_BLEND_ALPHA | R3D_BLEND_NZ);
			r3dRenderer->SetRenderState(D3DRS_SCISSORTESTENABLE, TRUE);

			r3dRenderer->EndFrame();
		}

		r3dRenderer->EndRender(true);
		r3dEndFrame();
	}

	CloseAppSelectDocument(document, listeners);

	// NOTE: RmlUiBackend is a shared singleton — Shutdown() is called
	// at application exit, not here.

	return AppSelectMode;
}
