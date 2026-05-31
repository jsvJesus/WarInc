#include "r3dPCH.h"
#include "r3d.h"
#include "APINoesisGUI.h"

#include <windows.h>
#include <windowsx.h>

APINoesisGUI* gNoesisGUI = NULL;

typedef bool (*Win32MsgProc_fn)(UINT uMsg, WPARAM wParam, LPARAM lParam);

extern void RegisterMsgProc(Win32MsgProc_fn proc);
extern void UnregisterMsgProc(Win32MsgProc_fn proc);

static bool r3dNoesisMsgProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if(!gNoesisGUI)
		return false;

	if(!gNoesisGUI->IsLoaded())
		return false;

	return gNoesisGUI->ProcessWindowMessage(uMsg, wParam, lParam);
}

APINoesisGUI::APINoesisGUI()
{
	DllHandle = NULL;

	FnInit = NULL;
	FnShutdown = NULL;
	FnLoadXamlFile = NULL;
	FnUnloadXaml = NULL;
	FnSetSize = NULL;
	FnUpdate = NULL;
	FnRender = NULL;
	FnIsLoaded = NULL;

	FnMouseMove = NULL;
	FnMouseButtonDown = NULL;
	FnMouseButtonUp = NULL;
	FnMouseWheel = NULL;
	FnKeyDown = NULL;
	FnKeyUp = NULL;
	FnChar = NULL;

	Initialized = false;
	Loaded = false;
	MsgProcRegistered = false;
}

APINoesisGUI::~APINoesisGUI()
{
	Shutdown();
}

bool APINoesisGUI::Init()
{
	if(Initialized)
		return true;

	HMODULE dll = LoadLibraryA("WarNoesisBridge.dll");

	if(!dll)
	{
		r3dOutToLog("NoesisBridge: failed to load WarNoesisBridge.dll, error=%lu\n", GetLastError());
		return false;
	}

	DllHandle = dll;

	FnInit = (FN_WarNoesis_Init)GetProcAddress(dll, "WarNoesis_Init");
	FnShutdown = (FN_WarNoesis_Shutdown)GetProcAddress(dll, "WarNoesis_Shutdown");
	FnLoadXamlFile = (FN_WarNoesis_LoadXamlFile)GetProcAddress(dll, "WarNoesis_LoadXamlFile");
	FnUnloadXaml = (FN_WarNoesis_UnloadXaml)GetProcAddress(dll, "WarNoesis_UnloadXaml");
	FnSetSize = (FN_WarNoesis_SetSize)GetProcAddress(dll, "WarNoesis_SetSize");
	FnUpdate = (FN_WarNoesis_Update)GetProcAddress(dll, "WarNoesis_Update");
	FnRender = (FN_WarNoesis_Render)GetProcAddress(dll, "WarNoesis_Render");
	FnIsLoaded = (FN_WarNoesis_IsLoaded)GetProcAddress(dll, "WarNoesis_IsLoaded");

	FnMouseMove = (FN_WarNoesis_MouseMove)GetProcAddress(dll, "WarNoesis_MouseMove");
	FnMouseButtonDown = (FN_WarNoesis_MouseButtonDown)GetProcAddress(dll, "WarNoesis_MouseButtonDown");
	FnMouseButtonUp = (FN_WarNoesis_MouseButtonUp)GetProcAddress(dll, "WarNoesis_MouseButtonUp");
	FnMouseWheel = (FN_WarNoesis_MouseWheel)GetProcAddress(dll, "WarNoesis_MouseWheel");
	FnKeyDown = (FN_WarNoesis_KeyDown)GetProcAddress(dll, "WarNoesis_KeyDown");
	FnKeyUp = (FN_WarNoesis_KeyUp)GetProcAddress(dll, "WarNoesis_KeyUp");
	FnChar = (FN_WarNoesis_Char)GetProcAddress(dll, "WarNoesis_Char");

	if(!FnInit || !FnShutdown || !FnLoadXamlFile || !FnUnloadXaml || !FnSetSize || !FnUpdate || !FnRender || !FnIsLoaded)
	{
		r3dOutToLog("NoesisBridge: failed to resolve core exports\n");
		FreeLibrary(dll);
		DllHandle = NULL;
		return false;
	}

	if(!FnMouseMove || !FnMouseButtonDown || !FnMouseButtonUp || !FnMouseWheel || !FnKeyDown || !FnKeyUp || !FnChar)
	{
		r3dOutToLog("NoesisBridge: failed to resolve input exports\n");
		FreeLibrary(dll);
		DllHandle = NULL;
		return false;
	}

	if(!FnInit("Data/UI"))
	{
		r3dOutToLog("NoesisBridge: WarNoesis_Init failed\n");
		FreeLibrary(dll);
		DllHandle = NULL;
		return false;
	}

	RegisterMsgProc(r3dNoesisMsgProc);
	MsgProcRegistered = true;

	Initialized = true;
	Loaded = false;

	r3dOutToLog("NoesisBridge: Init OK\n");
	return true;
}

void APINoesisGUI::Shutdown()
{
	if(MsgProcRegistered)
	{
		UnregisterMsgProc(r3dNoesisMsgProc);
		MsgProcRegistered = false;
	}

	if(!DllHandle)
		return;

	if(FnShutdown)
		FnShutdown();

	FreeLibrary((HMODULE)DllHandle);

	DllHandle = NULL;

	FnInit = NULL;
	FnShutdown = NULL;
	FnLoadXamlFile = NULL;
	FnUnloadXaml = NULL;
	FnSetSize = NULL;
	FnUpdate = NULL;
	FnRender = NULL;
	FnIsLoaded = NULL;

	FnMouseMove = NULL;
	FnMouseButtonDown = NULL;
	FnMouseButtonUp = NULL;
	FnMouseWheel = NULL;
	FnKeyDown = NULL;
	FnKeyUp = NULL;
	FnChar = NULL;

	Initialized = false;
	Loaded = false;

	r3dOutToLog("NoesisBridge: Shutdown OK\n");
}

bool APINoesisGUI::LoadXaml(const char* xamlFile)
{
	if(!Initialized)
	{
		if(!Init())
			return false;
	}

	if(!FnLoadXamlFile)
		return false;

	Loaded = FnLoadXamlFile(xamlFile) ? true : false;

	if(Loaded)
		r3dOutToLog("NoesisBridge: XAML loaded %s\n", xamlFile);
	else
		r3dOutToLog("NoesisBridge: XAML failed %s\n", xamlFile);

	return Loaded;
}

void APINoesisGUI::UnloadXaml()
{
	if(FnUnloadXaml)
		FnUnloadXaml();

	Loaded = false;
}

void APINoesisGUI::SetSize(int width, int height)
{
	if(FnSetSize)
		FnSetSize(width, height);
}

void APINoesisGUI::Update(float timeSeconds)
{
	if(FnUpdate)
		FnUpdate((double)timeSeconds);
}

void APINoesisGUI::Render()
{
	if(FnRender)
		FnRender();
}

bool APINoesisGUI::IsInitialized() const
{
	return Initialized;
}

bool APINoesisGUI::IsLoaded() const
{
	if(FnIsLoaded)
		return FnIsLoaded() ? true : false;

	return Loaded;
}

bool APINoesisGUI::ProcessWindowMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_MOUSEMOVE:
		return MouseMove(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));

	case WM_LBUTTONDOWN:
		return MouseButtonDown(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), 0);

	case WM_LBUTTONUP:
		return MouseButtonUp(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), 0);

	case WM_RBUTTONDOWN:
		return MouseButtonDown(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), 1);

	case WM_RBUTTONUP:
		return MouseButtonUp(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), 1);

	case WM_MBUTTONDOWN:
		return MouseButtonDown(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), 2);

	case WM_MBUTTONUP:
		return MouseButtonUp(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), 2);

	case WM_MOUSEWHEEL:
		{
			POINT pt;
			pt.x = GET_X_LPARAM(lParam);
			pt.y = GET_Y_LPARAM(lParam);

			if(win::hWnd)
				ScreenToClient(win::hWnd, &pt);

			return MouseWheel(pt.x, pt.y, GET_WHEEL_DELTA_WPARAM(wParam));
		}

	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
		return KeyDown((int)wParam);

	case WM_KEYUP:
	case WM_SYSKEYUP:
		return KeyUp((int)wParam);

	case WM_CHAR:
		return Char((unsigned int)wParam);

	default:
		break;
	}

	return false;
}

bool APINoesisGUI::MouseMove(int x, int y)
{
	if(FnMouseMove)
		return FnMouseMove(x, y) ? true : false;

	return false;
}

bool APINoesisGUI::MouseButtonDown(int x, int y, int button)
{
	if(FnMouseButtonDown)
		return FnMouseButtonDown(x, y, button) ? true : false;

	return false;
}

bool APINoesisGUI::MouseButtonUp(int x, int y, int button)
{
	if(FnMouseButtonUp)
		return FnMouseButtonUp(x, y, button) ? true : false;

	return false;
}

bool APINoesisGUI::MouseWheel(int x, int y, int delta)
{
	if(FnMouseWheel)
		return FnMouseWheel(x, y, delta) ? true : false;

	return false;
}

bool APINoesisGUI::KeyDown(int vk)
{
	if(FnKeyDown)
		return FnKeyDown(vk) ? true : false;

	return false;
}

bool APINoesisGUI::KeyUp(int vk)
{
	if(FnKeyUp)
		return FnKeyUp(vk) ? true : false;

	return false;
}

bool APINoesisGUI::Char(unsigned int ch)
{
	if(FnChar)
		return FnChar(ch) ? true : false;

	return false;
}

void r3dNoesisGUICreate()
{
	if(gNoesisGUI)
		return;

	gNoesisGUI = new APINoesisGUI();

	if(!gNoesisGUI->Init())
	{
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