#include "r3dPCH.h"
#include "r3d.h"
#include "APINoesisGUI.h"

#include <windows.h>

APINoesisGUI* gNoesisGUI = NULL;

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

	Initialized = false;
	Loaded = false;
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

	if(!FnInit || !FnShutdown || !FnLoadXamlFile || !FnUnloadXaml || !FnSetSize || !FnUpdate || !FnRender || !FnIsLoaded)
	{
		r3dOutToLog("NoesisBridge: failed to resolve exports\n");
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

	Initialized = true;
	Loaded = false;

	r3dOutToLog("NoesisBridge: Init OK\n");
	return true;
}

void APINoesisGUI::Shutdown()
{
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