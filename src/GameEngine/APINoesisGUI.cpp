#include "r3dPCH.h"
#include "r3d.h"
#include "APINoesisGUI.h"

#if WARINC_USE_NOESIS_GUI
#include "NoesisLicense.h"
#endif

APINoesisGUI* gNoesisGUI = NULL;

APINoesisGUI::APINoesisGUI()
{
	Initialized = false;
	Loaded = false;
	RendererInitialized = false;

	Width = 1280;
	Height = 720;
}

APINoesisGUI::~APINoesisGUI()
{
	Shutdown();
}

bool APINoesisGUI::Init()
{
#if !WARINC_USE_NOESIS_GUI
	return false;
#else
	if(Initialized)
		return true;

	Noesis::SetLogHandler(&APINoesisGUI::NoesisLogHandler);

	Noesis::GUI::DisableInspector();
	Noesis::GUI::DisableHotReload();

	Noesis::GUI::SetLicense(NS_LICENSE_NAME, NS_LICENSE_KEY);
	Noesis::GUI::Init();

	Noesis::GUI::SetXamlProvider(Noesis::MakePtr<NoesisApp::LocalXamlProvider>("Data/UI"));
	Noesis::GUI::SetTextureProvider(Noesis::MakePtr<NoesisApp::LocalTextureProvider>("Data/UI"));
	Noesis::GUI::SetFontProvider(Noesis::MakePtr<NoesisApp::LocalFontProvider>("Data/UI"));

	const char* fontFallbacks[] =
	{
		"Arial",
		"Segoe UI",
		"Tahoma"
	};

	Noesis::GUI::SetFontFallbacks(fontFallbacks, 3);
	Noesis::GUI::SetFontDefaultProperties(
		15.0f,
		Noesis::FontWeight_Normal,
		Noesis::FontStretch_Normal,
		Noesis::FontStyle_Normal
	);

	Initialized = true;

	r3dOutToLog("NoesisGUI: Init OK\n");
	return true;
#endif
}

void APINoesisGUI::Shutdown()
{
#if WARINC_USE_NOESIS_GUI
	UnloadXaml();

	if(Initialized)
	{
		Noesis::GUI::Shutdown();
		Initialized = false;
		r3dOutToLog("NoesisGUI: Shutdown OK\n");
	}
#endif
}

bool APINoesisGUI::LoadXaml(const char* xamlFile)
{
#if !WARINC_USE_NOESIS_GUI
	return false;
#else
	if(!xamlFile || !xamlFile[0])
		return false;

	if(!Initialized)
	{
		if(!Init())
			return false;
	}

	UnloadXaml();

	r3dOutToLog("NoesisGUI: Loading XAML '%s'\n", xamlFile);

	Noesis::Uri xamlUri(xamlFile);
	Root = Noesis::GUI::LoadXaml<Noesis::FrameworkElement>(xamlUri);

	if(!Root)
	{
		r3dOutToLog("NoesisGUI: LoadXaml failed '%s'\n", xamlFile);
		return false;
	}

	View = Noesis::GUI::CreateView(Root);

	if(!View)
	{
		Root.Reset();
		r3dOutToLog("NoesisGUI: CreateView failed '%s'\n", xamlFile);
		return false;
	}

	View->SetFlags(Noesis::RenderFlags_PPAA | Noesis::RenderFlags_LCD);
	View->SetSize((uint32_t)Width, (uint32_t)Height);
	View->Activate();

	/*
		ВАЖНО:
		Тут пока НЕТ View->GetRenderer()->Init(RenderDevice).
		В SDK есть D3D11/D3D12/GL/VK RenderDevice, но WarInc сейчас D3D9.
		Следующим шагом пишем свой NoesisD3D9RenderDevice под r3dRenderer->pd3ddev.
	*/

	Loaded = true;
	RendererInitialized = false;

	r3dOutToLog("NoesisGUI: XAML loaded OK '%s'\n", xamlFile);
	return true;
#endif
}

void APINoesisGUI::UnloadXaml()
{
#if WARINC_USE_NOESIS_GUI
	if(View)
	{
		if(RendererInitialized && View->GetRenderer())
			View->GetRenderer()->Shutdown();

		View.Reset();
	}

	if(Root)
		Root.Reset();

	Loaded = false;
	RendererInitialized = false;
#endif
}

void APINoesisGUI::SetSize(int width, int height)
{
	if(width <= 0 || height <= 0)
		return;

	Width = width;
	Height = height;

#if WARINC_USE_NOESIS_GUI
	if(View)
		View->SetSize((uint32_t)Width, (uint32_t)Height);
#endif
}

void APINoesisGUI::Update(float timeSeconds)
{
#if WARINC_USE_NOESIS_GUI
	if(!Loaded || !View)
		return;

	View->Update((double)timeSeconds);
#endif
}

void APINoesisGUI::Render()
{
#if WARINC_USE_NOESIS_GUI
	if(!Loaded || !View)
		return;

	if(!RendererInitialized)
		return;

	Noesis::IRenderer* renderer = View->GetRenderer();

	if(!renderer)
		return;

	renderer->UpdateRenderTree();
	renderer->RenderOffscreen();
	renderer->Render(false, false);
#endif
}

bool APINoesisGUI::IsInitialized() const
{
	return Initialized;
}

bool APINoesisGUI::IsLoaded() const
{
	return Loaded;
}

bool APINoesisGUI::MouseMove(int x, int y)
{
#if WARINC_USE_NOESIS_GUI
	if(View)
		return View->MouseMove(x, y);
#endif

	return false;
}

bool APINoesisGUI::MouseButtonDown(int x, int y, int button)
{
#if WARINC_USE_NOESIS_GUI
	if(View)
		return View->MouseButtonDown(x, y, ConvertMouseButton(button));
#endif

	return false;
}

bool APINoesisGUI::MouseButtonUp(int x, int y, int button)
{
#if WARINC_USE_NOESIS_GUI
	if(View)
		return View->MouseButtonUp(x, y, ConvertMouseButton(button));
#endif

	return false;
}

bool APINoesisGUI::MouseWheel(int x, int y, int wheelDelta)
{
#if WARINC_USE_NOESIS_GUI
	if(View)
		return View->MouseWheel(x, y, wheelDelta);
#endif

	return false;
}

bool APINoesisGUI::KeyDown(int vk)
{
#if WARINC_USE_NOESIS_GUI
	if(View)
		return View->KeyDown(ConvertKey(vk));
#endif

	return false;
}

bool APINoesisGUI::KeyUp(int vk)
{
#if WARINC_USE_NOESIS_GUI
	if(View)
		return View->KeyUp(ConvertKey(vk));
#endif

	return false;
}

bool APINoesisGUI::Char(unsigned int ch)
{
#if WARINC_USE_NOESIS_GUI
	if(View)
		return View->Char((uint32_t)ch);
#endif

	return false;
}

#if WARINC_USE_NOESIS_GUI

void APINoesisGUI::NoesisLogHandler(const char* file, uint32_t line, uint32_t level, const char* channel, const char* message)
{
	const char* levelName = "INFO";

	switch(level)
	{
	case NS_LOG_LEVEL_TRACE:
		levelName = "TRACE";
		break;
	case NS_LOG_LEVEL_DEBUG:
		levelName = "DEBUG";
		break;
	case NS_LOG_LEVEL_INFO:
		levelName = "INFO";
		break;
	case NS_LOG_LEVEL_WARNING:
		levelName = "WARNING";
		break;
	case NS_LOG_LEVEL_ERROR:
		levelName = "ERROR";
		break;
	default:
		levelName = "LOG";
		break;
	}

	r3dOutToLog("NoesisGUI[%s] %s(%u): %s\n", levelName, file ? file : "", line, message ? message : "");
}

Noesis::MouseButton APINoesisGUI::ConvertMouseButton(int button)
{
	switch(button)
	{
	case 0:
		return Noesis::MouseButton_Left;
	case 1:
		return Noesis::MouseButton_Right;
	case 2:
		return Noesis::MouseButton_Middle;
	default:
		return Noesis::MouseButton_Left;
	}
}

Noesis::Key APINoesisGUI::ConvertKey(int vk)
{
	if(vk >= '0' && vk <= '9')
		return (Noesis::Key)(Noesis::Key_D0 + (vk - '0'));

	if(vk >= 'A' && vk <= 'Z')
		return (Noesis::Key)(Noesis::Key_A + (vk - 'A'));

	if(vk >= VK_NUMPAD0 && vk <= VK_NUMPAD9)
		return (Noesis::Key)(Noesis::Key_NumPad0 + (vk - VK_NUMPAD0));

	if(vk >= VK_F1 && vk <= VK_F12)
		return (Noesis::Key)(Noesis::Key_F1 + (vk - VK_F1));

	switch(vk)
	{
	case VK_BACK:
		return Noesis::Key_Back;
	case VK_TAB:
		return Noesis::Key_Tab;
	case VK_RETURN:
		return Noesis::Key_Return;
	case VK_PAUSE:
		return Noesis::Key_Pause;
	case VK_CAPITAL:
		return Noesis::Key_Capital;
	case VK_ESCAPE:
		return Noesis::Key_Escape;
	case VK_SPACE:
		return Noesis::Key_Space;
	case VK_PRIOR:
		return Noesis::Key_PageUp;
	case VK_NEXT:
		return Noesis::Key_PageDown;
	case VK_END:
		return Noesis::Key_End;
	case VK_HOME:
		return Noesis::Key_Home;
	case VK_LEFT:
		return Noesis::Key_Left;
	case VK_UP:
		return Noesis::Key_Up;
	case VK_RIGHT:
		return Noesis::Key_Right;
	case VK_DOWN:
		return Noesis::Key_Down;
	case VK_INSERT:
		return Noesis::Key_Insert;
	case VK_DELETE:
		return Noesis::Key_Delete;
	case VK_SHIFT:
	case VK_LSHIFT:
		return Noesis::Key_LeftShift;
	case VK_RSHIFT:
		return Noesis::Key_RightShift;
	case VK_CONTROL:
	case VK_LCONTROL:
		return Noesis::Key_LeftCtrl;
	case VK_RCONTROL:
		return Noesis::Key_RightCtrl;
	case VK_MENU:
	case VK_LMENU:
		return Noesis::Key_LeftAlt;
	case VK_RMENU:
		return Noesis::Key_RightAlt;
	case VK_OEM_MINUS:
		return Noesis::Key_OemMinus;
	case VK_OEM_PLUS:
		return Noesis::Key_OemPlus;
	case VK_OEM_COMMA:
		return Noesis::Key_OemComma;
	case VK_OEM_PERIOD:
		return Noesis::Key_OemPeriod;
	case VK_OEM_1:
		return Noesis::Key_OemSemicolon;
	case VK_OEM_2:
		return Noesis::Key_OemQuestion;
	case VK_OEM_3:
		return Noesis::Key_OemTilde;
	case VK_OEM_4:
		return Noesis::Key_OemOpenBrackets;
	case VK_OEM_5:
		return Noesis::Key_OemPipe;
	case VK_OEM_6:
		return Noesis::Key_OemCloseBrackets;
	case VK_OEM_7:
		return Noesis::Key_OemQuotes;
	default:
		return Noesis::Key_None;
	}
}

#endif

void r3dNoesisGUICreate()
{
	if(gNoesisGUI)
		return;

	gNoesisGUI = new APINoesisGUI();

	if(!gNoesisGUI->Init())
	{
		r3dOutToLog("NoesisGUI: Init failed\n");
		SAFE_DELETE(gNoesisGUI);
	}
}

void r3dNoesisGUIDestroy()
{
	if(!gNoesisGUI)
		return;

	gNoesisGUI->Shutdown();
	SAFE_DELETE(gNoesisGUI);
}

void r3dNoesisGUIReset()
{
	if(!gNoesisGUI || !r3dRenderer)
		return;

	gNoesisGUI->SetSize((int)r3dRenderer->ScreenW, (int)r3dRenderer->ScreenH);
}