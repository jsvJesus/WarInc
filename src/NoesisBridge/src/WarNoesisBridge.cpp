#define WAR_NOESIS_BRIDGE_EXPORTS
#include "WarNoesisBridge.h"
#include "WarNoesisD3D9RenderDevice.h"
#include "WarNoesisMediaPlayer.h"

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <NsCore/Noesis.h>
#include <NsCore/Ptr.h>
#include <NsCore/Log.h>
#include <NsCore/Init.h>
#include <NsCore/RegisterComponent.h>
#include <NsCore/EnumConverter.h>
#include <NsApp/MediaElement.h>

#include <NsGui/IntegrationAPI.h>
#include <NsGui/FrameworkElement.h>
#include <NsGui/IView.h>
#include <NsGui/IRenderer.h>
#include <NsGui/InputEnums.h>
#include <NsGui/FontProperties.h>
#include <NsGui/Button.h>
#include <NsGui/BaseButton.h>
#include <NsGui/RoutedEvent.h>
#include <NsGui/Uri.h>

#include <NsApp/LocalXamlProvider.h>
#include <NsApp/LocalTextureProvider.h>
#include <NsApp/LocalFontProvider.h>

#include <NsGui/Visual.h>
#include <NsGui/VisualTreeHelper.h>

#include "NoesisLicense.h"

static Noesis::Ptr<Noesis::FrameworkElement> gRoot;
static Noesis::Ptr<Noesis::IView> gView;
static WarNoesisCommandCallback gCommandCallback = NULL;

static IDirect3DDevice9* gD3D9Device = NULL;
static Noesis::Ptr<WarNoesisD3D9RenderDevice> gRenderDevice;
static int gRendererInitialized = 0;

static int gInitialized = 0;
static int gLoaded = 0;
static int gWidth = 1280;
static int gHeight = 720;

static char gRootPath[MAX_PATH];

static void BridgeLog(const char* text)
{
	OutputDebugStringA(text);
	OutputDebugStringA("\n");
}

static int EnsureNoesisRenderer()
{
	if(!gLoaded || !gView)
		return 0;

	if(!gD3D9Device)
	{
		BridgeLog("WarNoesisBridge: D3D9 device is NULL");
		return 0;
	}

	Noesis::IRenderer* renderer = gView->GetRenderer();

	if(!renderer)
	{
		BridgeLog("WarNoesisBridge: Noesis renderer is NULL");
		return 0;
	}

	if(gRendererInitialized)
		return 1;

	if(!gRenderDevice)
	{
		gRenderDevice = *new WarNoesisD3D9RenderDevice(gD3D9Device);

		if(!gRenderDevice)
		{
			BridgeLog("WarNoesisBridge: failed to create D3D9 render device");
			return 0;
		}
	}

	renderer->Init(gRenderDevice);

	gRendererInitialized = 1;

	BridgeLog("WarNoesisBridge: D3D9 renderer initialized");
	return 1;
}

static void EmitEditorCommand(const char* command, const char* value)
{
	if(!command || !command[0])
		return;

	char buffer[1024];
	_snprintf(buffer, sizeof(buffer) - 1, "WarNoesisBridge: command=%s value=%s", command, value ? value : "");
	buffer[sizeof(buffer) - 1] = 0;
	BridgeLog(buffer);

	if(gCommandCallback)
		gCommandCallback(command, value ? value : "");
}

static void OnEditorButtonClick(Noesis::BaseComponent* sender, const Noesis::RoutedEventArgs& args)
{
	Noesis::FrameworkElement* element = Noesis::DynamicCast<Noesis::FrameworkElement*>(sender);

	if(!element)
		return;

	const char* name = element->GetName();

	if(!name || !name[0])
		return;

	char buffer[512];
	_snprintf(buffer, sizeof(buffer) - 1, "WarNoesisBridge: button click %s", name);
	buffer[sizeof(buffer) - 1] = 0;
	BridgeLog(buffer);

	EmitEditorCommand(name, "");
}

static void BindNoesisButton(Noesis::Button* button)
{
	if(!button)
		return;

	const char* name = button->GetName();

	if(!name || !name[0])
		return;

	button->Click() += Noesis::RoutedEventHandler(&OnEditorButtonClick);

	char buffer[512];
	_snprintf(buffer, sizeof(buffer) - 1, "WarNoesisBridge: button bound %s", name);
	buffer[sizeof(buffer) - 1] = 0;
	BridgeLog(buffer);
}

static void BindNoesisButtonsRecursive(Noesis::Visual* visual)
{
	if(!visual)
		return;

	Noesis::Button* button = Noesis::DynamicCast<Noesis::Button*>(visual);
	if(button)
		BindNoesisButton(button);

	uint32_t count = Noesis::VisualTreeHelper::GetChildrenCount(visual);

	for(uint32_t i = 0; i < count; ++i)
	{
		Noesis::Visual* child = Noesis::VisualTreeHelper::GetChild(visual, i);
		BindNoesisButtonsRecursive(child);
	}
}

static void BindNoesisControls()
{
	if(!gRoot)
		return;

	gRoot->ApplyTemplate();

	Noesis::Visual* rootVisual = Noesis::DynamicCast<Noesis::Visual*>(gRoot.GetPtr());

	if(!rootVisual)
	{
		BridgeLog("WarNoesisBridge: root is not visual");
		return;
	}

	BindNoesisButtonsRecursive(rootVisual);
}

static void NoesisLogHandler(const char* file, uint32_t line, uint32_t level, const char* channel, const char* message)
{
	char buffer[4096];

	_snprintf(
		buffer,
		sizeof(buffer) - 1,
		"Noesis[%u] %s(%u): %s\n",
		level,
		file ? file : "",
		line,
		message ? message : ""
	);

	buffer[sizeof(buffer) - 1] = 0;
	OutputDebugStringA(buffer);
}

static char* ReadTextFile(const char* filename)
{
	FILE* file = fopen(filename, "rb");

	if(!file)
		return NULL;

	fseek(file, 0, SEEK_END);
	long size = ftell(file);
	fseek(file, 0, SEEK_SET);

	if(size <= 0)
	{
		fclose(file);
		return NULL;
	}

	char* data = (char*)malloc((size_t)size + 1);

	if(!data)
	{
		fclose(file);
		return NULL;
	}

	size_t readed = fread(data, 1, (size_t)size, file);
	fclose(file);

	data[readed] = 0;
	return data;
}

static void BuildFullPath(char* outPath, int outPathSize, const char* filename)
{
	if(!outPath || outPathSize <= 0)
		return;

	outPath[0] = 0;

	if(!filename || !filename[0])
		return;

	if(filename[1] == ':' || filename[0] == '\\' || filename[0] == '/')
	{
		_snprintf(outPath, outPathSize - 1, "%s", filename);
		outPath[outPathSize - 1] = 0;
		return;
	}

	if(gRootPath[0])
	{
		_snprintf(outPath, outPathSize - 1, "%s/%s", gRootPath, filename);
		outPath[outPathSize - 1] = 0;
		return;
	}

	_snprintf(outPath, outPathSize - 1, "%s", filename);
	outPath[outPathSize - 1] = 0;
}

static Noesis::MouseButton ConvertMouseButton(int button)
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

static Noesis::Key ConvertKey(int vk)
{
	if(vk >= '0' && vk <= '9')
		return (Noesis::Key)(Noesis::Key_D0 + (vk - '0'));

	if(vk >= 'A' && vk <= 'Z')
		return (Noesis::Key)(Noesis::Key_A + (vk - 'A'));

	if(vk >= VK_NUMPAD0 && vk <= VK_NUMPAD9)
		return (Noesis::Key)(Noesis::Key_NumPad0 + (vk - VK_NUMPAD0));

	if(vk >= VK_F1 && vk <= VK_F24)
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
	case VK_LWIN:
		return Noesis::Key_LWin;
	case VK_RWIN:
		return Noesis::Key_RWin;
	case VK_APPS:
		return Noesis::Key_Apps;
	case VK_NUMLOCK:
		return Noesis::Key_NumLock;
	case VK_SCROLL:
		return Noesis::Key_Scroll;
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
	case VK_MULTIPLY:
		return Noesis::Key_Multiply;
	case VK_ADD:
		return Noesis::Key_Add;
	case VK_SEPARATOR:
		return Noesis::Key_Separator;
	case VK_SUBTRACT:
		return Noesis::Key_Subtract;
	case VK_DECIMAL:
		return Noesis::Key_Decimal;
	case VK_DIVIDE:
		return Noesis::Key_Divide;
	case VK_OEM_1:
		return Noesis::Key_OemSemicolon;
	case VK_OEM_PLUS:
		return Noesis::Key_OemPlus;
	case VK_OEM_COMMA:
		return Noesis::Key_OemComma;
	case VK_OEM_MINUS:
		return Noesis::Key_OemMinus;
	case VK_OEM_PERIOD:
		return Noesis::Key_OemPeriod;
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

WAR_NOESIS_API int __cdecl WarNoesis_Init(const char* rootPath)
{
	if(gInitialized)
		return 1;

	gRootPath[0] = 0;

	if(rootPath && rootPath[0])
	{
		_snprintf(gRootPath, sizeof(gRootPath) - 1, "%s", rootPath);
		gRootPath[sizeof(gRootPath) - 1] = 0;
	}

	Noesis::SetLogHandler(NoesisLogHandler);

	Noesis::GUI::DisableInspector();
	Noesis::GUI::DisableHotReload();

	Noesis::GUI::SetLicense(NS_LICENSE_NAME, NS_LICENSE_KEY);
	Noesis::GUI::Init();

	Noesis::RegisterComponent<NoesisApp::MediaElement>();
	Noesis::RegisterComponent<Noesis::EnumConverter<NoesisApp::MediaState> >();

	WarNoesisMediaPlayer_Init(gRootPath);

	if(gRootPath[0])
	{
		Noesis::GUI::SetXamlProvider(Noesis::MakePtr<NoesisApp::LocalXamlProvider>(gRootPath));
		Noesis::GUI::SetTextureProvider(Noesis::MakePtr<NoesisApp::LocalTextureProvider>(gRootPath));
		Noesis::GUI::SetFontProvider(Noesis::MakePtr<NoesisApp::LocalFontProvider>(gRootPath));
	}

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

	gInitialized = 1;
	gLoaded = 0;

	BridgeLog("WarNoesisBridge: Init OK");
	return 1;
}

WAR_NOESIS_API void __cdecl WarNoesis_Shutdown()
{
	if(!gInitialized)
		return;

	WarNoesis_UnloadXaml();

	gRendererInitialized = 0;
	gRenderDevice.Reset();

	if(gD3D9Device)
	{
		gD3D9Device->Release();
		gD3D9Device = NULL;
	}

	WarNoesisMediaPlayer_Shutdown();

	Noesis::GUI::Shutdown();

	gInitialized = 0;
	gLoaded = 0;

	BridgeLog("WarNoesisBridge: Shutdown OK");
	gCommandCallback = NULL;
}

WAR_NOESIS_API void __cdecl WarNoesis_SetD3D9Device(void* device)
{
	IDirect3DDevice9* newDevice = (IDirect3DDevice9*)device;

	if(newDevice == gD3D9Device)
		return;

	if(gView && gRendererInitialized)
	{
		Noesis::IRenderer* renderer = gView->GetRenderer();

		if(renderer)
			renderer->Shutdown();
	}

	gRendererInitialized = 0;
	gRenderDevice.Reset();

	if(newDevice)
		newDevice->AddRef();

	if(gD3D9Device)
		gD3D9Device->Release();

	gD3D9Device = newDevice;

	BridgeLog(gD3D9Device ? "WarNoesisBridge: D3D9 device set" : "WarNoesisBridge: D3D9 device cleared");
}

WAR_NOESIS_API int __cdecl WarNoesis_LoadXamlFile(const char* filename)
{
	if(!gInitialized)
		return 0;

	if(!filename || !filename[0])
		return 0;

	WarNoesis_UnloadXaml();

	char fullPath[MAX_PATH];
	BuildFullPath(fullPath, sizeof(fullPath), filename);

	char buffer[1024];
	_snprintf(buffer, sizeof(buffer) - 1, "WarNoesisBridge: loading xaml %s", fullPath);
	buffer[sizeof(buffer) - 1] = 0;
	BridgeLog(buffer);

	Noesis::Uri xamlUri(filename);
	Noesis::Ptr<Noesis::BaseComponent> component = Noesis::GUI::LoadXaml(xamlUri);

	if(!component)
	{
		_snprintf(buffer, sizeof(buffer) - 1, "WarNoesisBridge: LoadXaml failed %s", filename);
		buffer[sizeof(buffer) - 1] = 0;
		BridgeLog(buffer);
		return 0;
	}

	Noesis::FrameworkElement* element = Noesis::DynamicCast<Noesis::FrameworkElement*>(component.GetPtr());

	if(!element)
	{
		BridgeLog("WarNoesisBridge: root is not FrameworkElement");
		return 0;
	}

	gRoot.Reset(element);
	gView = Noesis::GUI::CreateView(gRoot);

	if(!gView)
	{
		gRoot.Reset();
		BridgeLog("WarNoesisBridge: CreateView failed");
		return 0;
	}

	gView->SetFlags((Noesis::RenderFlags)0);
	gView->SetSize((uint32_t)gWidth, (uint32_t)gHeight);
	gView->Activate();

	BindNoesisControls();

	gLoaded = 1;
	gRendererInitialized = 0;

	EnsureNoesisRenderer();

	BridgeLog("WarNoesisBridge: XAML loaded OK");
	return 1;
}

WAR_NOESIS_API void __cdecl WarNoesis_UnloadXaml()
{
	if(gView)
	{
		if(gView->GetRenderer())
			gView->GetRenderer()->Shutdown();

		gView.Reset();
	}

	gRendererInitialized = 0;
	gRenderDevice.Reset();

	if(gRoot)
		gRoot.Reset();

	gLoaded = 0;
}

WAR_NOESIS_API void __cdecl WarNoesis_SetSize(int width, int height)
{
	if(width <= 0 || height <= 0)
		return;

	gWidth = width;
	gHeight = height;

	if(gView)
		gView->SetSize((uint32_t)gWidth, (uint32_t)gHeight);
}

WAR_NOESIS_API void __cdecl WarNoesis_Update(double timeSeconds)
{
	if(!gLoaded || !gView)
		return;

	gView->Update(timeSeconds);
}

WAR_NOESIS_API void __cdecl WarNoesis_Render()
{
	if(!gLoaded || !gView)
		return;

	if(!EnsureNoesisRenderer())
		return;

	Noesis::IRenderer* renderer = gView->GetRenderer();

	if(!renderer)
		return;

	renderer->UpdateRenderTree();
	renderer->RenderOffscreen();
	renderer->Render();
}

WAR_NOESIS_API int __cdecl WarNoesis_IsLoaded()
{
	return gLoaded;
}

WAR_NOESIS_API void __cdecl WarNoesis_SetCommandCallback(WarNoesisCommandCallback callback)
{
	gCommandCallback = callback;
}

WAR_NOESIS_API int __cdecl WarNoesis_MouseMove(int x, int y)
{
	if(!gLoaded || !gView)
		return 0;

	return gView->MouseMove(x, y) ? 1 : 0;
}

WAR_NOESIS_API int __cdecl WarNoesis_MouseButtonDown(int x, int y, int button)
{
	if(!gLoaded || !gView)
		return 0;

	return gView->MouseButtonDown(x, y, ConvertMouseButton(button)) ? 1 : 0;
}

WAR_NOESIS_API int __cdecl WarNoesis_MouseButtonUp(int x, int y, int button)
{
	if(!gLoaded || !gView)
		return 0;

	return gView->MouseButtonUp(x, y, ConvertMouseButton(button)) ? 1 : 0;
}

WAR_NOESIS_API int __cdecl WarNoesis_MouseWheel(int x, int y, int delta)
{
	if(!gLoaded || !gView)
		return 0;

	return gView->MouseWheel(x, y, delta) ? 1 : 0;
}

WAR_NOESIS_API int __cdecl WarNoesis_KeyDown(int vk)
{
	if(!gLoaded || !gView)
		return 0;

	return gView->KeyDown(ConvertKey(vk)) ? 1 : 0;
}

WAR_NOESIS_API int __cdecl WarNoesis_KeyUp(int vk)
{
	if(!gLoaded || !gView)
		return 0;

	return gView->KeyUp(ConvertKey(vk)) ? 1 : 0;
}

WAR_NOESIS_API int __cdecl WarNoesis_Char(unsigned int ch)
{
	if(!gLoaded || !gView)
		return 0;

	return gView->Char((uint32_t)ch) ? 1 : 0;
}