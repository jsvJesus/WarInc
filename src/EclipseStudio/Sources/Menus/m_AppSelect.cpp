#include "r3dPCH.h"
#include "r3d.h"

#include "GameCommon.h"
#include "UI/UIimEdit.h"
#include "UI/RmlUiBackend.h"

#include "m_AppSelect.h"

#include "r3dDX11ScaleformBridge.h"

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

	const char* g_AppSelectRml =
		"<rml>"
		"<head>"
		"<title>WarInc Studio</title>"
		"<style>"
		"body { width: 100%; height: 100%; margin: 0px; font-family: Arial; color: #eef3f8; }"
		"#root { position: absolute; left: 0px; top: 0px; width: 100%; height: 100%; }"
		"#panel { position: absolute; width: 430px; height: 366px; left: 50%; top: 50%; margin-left: -215px; margin-top: -183px; padding: 26px 32px; background-color: rgba(12, 15, 20, 218); border: 1px #8fa3b8; }"
		"#title { font-size: 34px; color: #ffffff; margin-bottom: 4px; }"
		"#subtitle { font-size: 15px; color: #aeb8c5; margin-bottom: 22px; }"
		".button { display: block; height: 34px; margin-bottom: 9px; padding: 8px 14px 0px 14px; font-size: 17px; color: #f4f7fb; background-color: rgba(47, 57, 70, 235); border: 1px #728092; cursor: pointer; }"
		".button:hover { background-color: rgba(74, 91, 113, 245); border-color: #d4dfeb; cursor: pointer; }"
		".button:active { background-color: rgba(97, 116, 141, 255); cursor: pointer; }"
		".hotkey { float: lfet; color: #a9b4c2; }"
		"#footer { margin-top: 12px; font-size: 12px; color: #788493; }"
		"</style>"
		"</head>"
		"<body>"
		"<div id='root'>"
		"<div id='panel'>"
		"<div id='title'>WarInc Studio</div>"
		"<div id='subtitle'>Editor launcher</div>"
		"<button id='play' class='button' onclick='dummy'>Play Game <span class='hotkey'>1</span></button>"
		"<button id='level' class='button' onclick='dummy'>Level Editor <span class='hotkey'>2</span></button>"
		"<button id='particle' class='button' onclick='dummy'>Particle Editor <span class='hotkey'>3</span></button>"
		"<button id='physics' class='button' onclick='dummy'>Physics Editor <span class='hotkey'>4</span></button>"
		"<button id='character' class='button' onclick='dummy'>Character Editor <span class='hotkey'>5</span></button>"
		"<button id='exit' class='button' onclick='dummy'>Exit <span class='hotkey'>6</span></button>"
		"<div id='footer'>DX11 game renderer / RmlUi editor menu</div>"
		"</div>"
		"</div>"
		"</body>"
		"</rml>";

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
			AppSelectMode = m_result;
		}

	private:
		int m_result;
	};

	Rocket::Core::ElementDocument* LoadAppSelectDocument(AppSelectEventListener* listeners)
	{
		Rocket::Core::ElementDocument* document = RmlUiBackend::LoadDocumentFromMemory(g_AppSelectRml);
		if(!document)
		{
			r3dOutToLog("RmlUi AppSelect: LoadDocumentFromMemory failed\n");
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
		r3dOutToLog("RmlUi AppSelect: document loaded and shown\n");
		return document;
	}

	r3dTexture* LoadAppSelectBackground()
	{
		if(!r3dFileExists("Data/Menu/Background.dds"))
			return NULL;

		return r3dRenderer->LoadTexture("Data/Menu/Background.dds");
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

			// Restore engine rendering state after RmlUi D3D9 rendering
			r3dRenderer->SetRenderingMode(R3D_BLEND_ALPHA | R3D_BLEND_NZ);
			r3dRenderer->SetRenderState(D3DRS_SCISSORTESTENABLE, TRUE);

			r3dRenderer->EndFrame();
		}

		r3dRenderer->EndRender(true);
		r3dEndFrame();
	}

	if(document)
	{
		document->Close();
		document->RemoveReference();
	}

	// NOTE: RmlUiBackend is a shared singleton — Shutdown() is called
	// at application exit, not here.

	return AppSelectMode;
}
