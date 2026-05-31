#define WAR_NOESIS_BRIDGE_EXPORTS
#include "WarNoesisBridge.h"

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <NsCore/Noesis.h>
#include <NsCore/Ptr.h>
#include <NsCore/Log.h>
#include <NsCore/Init.h>

#include <NsGui/IntegrationAPI.h>
#include <NsGui/FrameworkElement.h>
#include <NsGui/IView.h>
#include <NsGui/IRenderer.h>
#include <NsGui/InputEnums.h>

#include "NoesisLicense.h"

static Noesis::Ptr<Noesis::FrameworkElement> gRoot;
static Noesis::Ptr<Noesis::IView> gView;

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

	Noesis::GUI::Shutdown();

	gInitialized = 0;
	gLoaded = 0;

	BridgeLog("WarNoesisBridge: Shutdown OK");
}

WAR_NOESIS_API int __cdecl WarNoesis_LoadXamlFile(const char* filename)
{
	if(!gInitialized)
		return 0;

	WarNoesis_UnloadXaml();

	char fullPath[MAX_PATH];
	BuildFullPath(fullPath, sizeof(fullPath), filename);

	char* xaml = ReadTextFile(fullPath);

	if(!xaml)
	{
		char buffer[1024];
		_snprintf(buffer, sizeof(buffer) - 1, "WarNoesisBridge: failed to read xaml %s", fullPath);
		buffer[sizeof(buffer) - 1] = 0;
		BridgeLog(buffer);
		return 0;
	}

	const char* xamlText = xaml;

	if((unsigned char)xamlText[0] == 0xEF && (unsigned char)xamlText[1] == 0xBB && (unsigned char)xamlText[2] == 0xBF)
		xamlText += 3;

	Noesis::Ptr<Noesis::BaseComponent> component = Noesis::GUI::ParseXaml(xamlText);

	free(xaml);

	if(!component)
	{
		BridgeLog("WarNoesisBridge: ParseXaml failed");
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

	gView->SetFlags(Noesis::RenderFlags_PPAA | Noesis::RenderFlags_LCD);
	gView->SetSize((uint32_t)gWidth, (uint32_t)gHeight);
	gView->Activate();

	gLoaded = 1;

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

	/*
		Пока рендера тут нет, потому что для WarInc нужен отдельный D3D9 RenderDevice.
		Но Noesis уже работает в отдельной DLL и VS120 проект больше не компилирует Noesis headers.
	*/
}

WAR_NOESIS_API int __cdecl WarNoesis_IsLoaded()
{
	return gLoaded;
}