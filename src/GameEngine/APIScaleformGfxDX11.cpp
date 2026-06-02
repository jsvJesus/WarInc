#include "r3dPCH.h"
#include "r3d.h"
#include "APIScaleformGfxDX11.h"

#ifndef WO_SERVER

#ifndef SF_D3D_VERSION
#define SF_D3D_VERSION 11
#endif

#pragma warning(push)
#pragma warning(disable: 4005)
#pragma warning(disable: 4100)
#pragma warning(disable: 4244)
#pragma warning(disable: 4312)
#pragma warning(disable: 4996)

#include <d3d11.h>
#include <windowsx.h>

#ifndef GET_X_LPARAM
#define GET_X_LPARAM(lp) ((int)(short)LOWORD(lp))
#endif

#ifndef GET_Y_LPARAM
#define GET_Y_LPARAM(lp) ((int)(short)HIWORD(lp))
#endif

#include "GFx_Kernel.h"
#include "GFx.h"
#include "GFx_Renderer_D3D1x.h"
#include "Render/D3D1x/D3D1x_HAL.h"
#include "Render/Renderer2D.h"
#include "Render/ImageFiles/PNG_ImageFile.h"
#include "Render/ImageFiles/DDS_ImageFile.h"
#include "GFx_FontProvider_Win32.h"

#pragma warning(pop)

#include "r3dDX11.h"
#include "r3dDX11State.h"
#include "r3dDX11Geometry.h"
#include "LangMngr.h"

typedef bool (*Win32MsgProc_fn)(UINT uMsg, WPARAM wParam, LPARAM lParam);

extern void RegisterMsgProc(Win32MsgProc_fn proc);
extern void UnregisterMsgProc(Win32MsgProc_fn proc);

static char gSF11DefaultImagePath[MAX_PATH] = "data\\menu";

static const char* r3dSF11DetectAbsolutePath(const char* purl)
{
	static const char ABSOLUTE_PATH_PREFIX = '$';

	const char* fname = strchr(purl, ABSOLUTE_PATH_PREFIX);

	if(fname)
		return fname + 1;

	return purl;
}

class r3dSF11MemoryFile : public Scaleform::MemoryFile
{
public:
	const unsigned char* SavedFileData;

	r3dSF11MemoryFile(const char* pfileName, const unsigned char* pBuffer, int buffSize)
		: Scaleform::MemoryFile(pfileName, pBuffer, buffSize)
	{
		SavedFileData = pBuffer;
	}

	virtual ~r3dSF11MemoryFile()
	{
		delete[] SavedFileData;
	}
};

class r3dSF11FileOpener : public Scaleform::GFx::FileOpener
{
public:
	virtual Scaleform::File* OpenFile(const char* purl, int flags, int mode)
	{
		const char* absName = r3dSF11DetectAbsolutePath(purl);

		r3dFile* f = r3d_open(absName, "rb");

		if(f == NULL)
		{
			char menuName[MAX_PATH];
			_snprintf(menuName, sizeof(menuName), "%s\\%s", gSF11DefaultImagePath, purl);
			menuName[sizeof(menuName) - 1] = 0;

			absName = r3dSF11DetectAbsolutePath(menuName);
			f = r3d_open(absName, "rb");
		}

		if(f == NULL)
			return new Scaleform::SysFile(purl);

		unsigned char* data = new unsigned char[f->size + 1];
		int size = f->size;

		fread(data, 1, f->size, f);
		fclose(f);

		data[size] = 0;

		return new r3dSF11MemoryFile(absName, data, size);
	}
};

class r3dSF11Log : public Scaleform::GFx::Log
{
public:
	virtual void LogMessageVarg(Scaleform::LogMessageId messageType, const char* pfmt, va_list argList)
	{
#ifdef FINAL_BUILD
		return;
#endif

		char buf[2048];
		StringCbVPrintfA(buf, sizeof(buf), pfmt, argList);

		switch(messageType.GetMessageType())
		{
		case Scaleform::LogMessage_Error:
			r3dOutToLog("SF11 Error: %s\n", buf);
			break;

		case Scaleform::LogMessage_Assert:
			r3dOutToLog("SF11 Assert: %s\n", buf);
			break;

#ifndef FINAL_BUILD
		case Scaleform::LogMessage_Warning:
			r3dOutToLog("SF11 Warning: %s\n", buf);
			break;

		case Scaleform::LogMessage_Report:
			r3dOutToLog("SF11 Report: %s\n", buf);
			break;
#endif

		default:
			break;
		}
	}
};

class r3dSF11FSCommandHandler : public Scaleform::GFx::FSCommandHandler
{
public:
	virtual void Callback(Scaleform::GFx::Movie* pmovie, const char* command, const char* arg);
};

class r3dSF11ExternalInterface : public Scaleform::GFx::ExternalInterface
{
public:
	virtual void Callback(Scaleform::GFx::Movie* pmovie, const char* methodName, const Scaleform::GFx::Value* args, unsigned argCount);
};

class r3dSF11UserEventHandler : public Scaleform::GFx::UserEventHandler
{
public:
	virtual void HandleEvent(Scaleform::GFx::Movie* pmovie, const Scaleform::GFx::Event& event)
	{
		switch(event.Type)
		{
		case Scaleform::GFx::Event::DoShowMouse:
			Mouse->Show();
			break;

		case Scaleform::GFx::Event::DoHideMouse:
			Mouse->Hide();
			break;

		default:
			break;
		}
	}
};

class r3dSF11Translator : public Scaleform::GFx::Translator
{
	virtual unsigned GetCaps() const
	{
		return Scaleform::GFx::Translator::Cap_StripTrailingNewLines;
	}

	virtual void Translate(Scaleform::GFx::Translator::TranslateInfo* info)
	{
		const char* instanceName = info->GetInstanceName();
		const wchar_t* keyName = info->GetKey();

		if(instanceName && instanceName[0] == '$')
		{
			info->SetResult(gLangMngr.getString(instanceName));
			return;
		}

		if(keyName && keyName[0] == L'$')
		{
			info->SetResult(gLangMngr.getString(wideToUtf8(keyName)));
			return;
		}

		if(instanceName && instanceName[0] == '%')
		{
			info->SetResultHtml(gLangMngr.getString(instanceName));
			return;
		}

		if(keyName && keyName[0] == L'%')
		{
			info->SetResultHtml(gLangMngr.getString(wideToUtf8(keyName)));
			return;
		}
	}
};

class r3dSF11CommandQueue : public Scaleform::Render::ThreadCommandQueue
{
public:
	r3dSF11CommandQueue()
	{
		pHAL = NULL;
		pRenderer2D = NULL;
	}

	virtual void GetRenderInterfaces(Scaleform::Render::Interfaces* p)
	{
		p->pHAL = pHAL;
		p->pRenderer2D = pRenderer2D;
		p->pTextureManager = pHAL ? pHAL->GetTextureManager() : NULL;
		p->RenderThreadID = 0;
	}

	virtual void PushThreadCommand(Scaleform::Render::ThreadCommand* command)
	{
		if(command)
			command->Execute();
	}

	Scaleform::Render::HAL* pHAL;
	Scaleform::Render::Renderer2D* pRenderer2D;
};

class APIScaleformGfxDX11
{
public:
	APIScaleformGfxDX11();
	~APIScaleformGfxDX11();

	bool Create();
	void Destroy();

	bool LoadAppSelect(const char* movieFile);
	void UnloadAppSelect();

	int RenderAppSelect();
	bool ProcessWindowMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

	bool IsReady() const;
	void Reset();

	void PushCommand(const char* command);

private:
	void ConfigureLoader();
	void SetFontLib();
	void UpdateViewport();
	void UpdateMouse();
	void AdvanceMovie();
	void DrawMovie();

	int CommandToAppSelectResult(const char* command) const;

private:
	bool Created;
	bool MsgProcRegistered;
	bool SystemStarted;

	int Width;
	int Height;

	float LastAdvanceTime;

	Scaleform::GFx::Loader Loader;

	Scaleform::Ptr<Scaleform::Render::D3D1x::HAL> RendererHAL;
	Scaleform::Ptr<Scaleform::Render::Renderer2D> Renderer2D;
	r3dSF11CommandQueue* CommandQueue;

	Scaleform::Ptr<Scaleform::GFx::MovieDef> AppSelectMovieDef;
	Scaleform::Ptr<Scaleform::GFx::Movie> AppSelectMovie;
	Scaleform::GFx::MovieDisplayHandle AppSelectDisplayHandle;

	char PendingCommand[128];
};

static APIScaleformGfxDX11* gScaleformDX11 = NULL;

static bool r3dScaleformGfxDX11WinProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if(!gScaleformDX11)
		return false;

	return gScaleformDX11->ProcessWindowMessage(uMsg, wParam, lParam);
}

void r3dSF11FSCommandHandler::Callback(Scaleform::GFx::Movie* pmovie, const char* command, const char* arg)
{
	if(gScaleformDX11)
		gScaleformDX11->PushCommand(command);
}

void r3dSF11ExternalInterface::Callback(Scaleform::GFx::Movie* pmovie, const char* methodName, const Scaleform::GFx::Value* args, unsigned argCount)
{
	if(gScaleformDX11)
		gScaleformDX11->PushCommand(methodName);
}

APIScaleformGfxDX11::APIScaleformGfxDX11()
{
	Created = false;
	MsgProcRegistered = false;
	SystemStarted = false;

	Width = 0;
	Height = 0;

	LastAdvanceTime = 0.0f;

	CommandQueue = NULL;

	PendingCommand[0] = 0;
}

APIScaleformGfxDX11::~APIScaleformGfxDX11()
{
	Destroy();
}

bool APIScaleformGfxDX11::Create()
{
	if(Created)
		return true;

	if(!g_r3dDX11.IsInitialized())
	{
		r3dOutToLog("SF11: Create failed, DX11 renderer is not initialized\n");
		return false;
	}

	if(!g_r3dDX11.GetDevice() || !g_r3dDX11.GetContext())
	{
		r3dOutToLog("SF11: Create failed, DX11 device/context is NULL\n");
		return false;
	}

	Scaleform::GFx::System::Init();
	SystemStarted = true;

	RendererHAL = *new Scaleform::Render::D3D1x::HAL();

	if(!RendererHAL)
	{
		r3dOutToLog("SF11: failed to create D3D1x HAL\n");
		Destroy();
		return false;
	}

	if(!RendererHAL->InitHAL(
		Scaleform::Render::D3D1x::HALInitParams(
			g_r3dDX11.GetDevice(),
			g_r3dDX11.GetContext()
		)
	))
	{
		r3dOutToLog("SF11: D3D1x HAL InitHAL failed\n");
		Destroy();
		return false;
	}

	Renderer2D = *new Scaleform::Render::Renderer2D(RendererHAL.GetPtr());

	if(!Renderer2D)
	{
		r3dOutToLog("SF11: failed to create Renderer2D\n");
		Destroy();
		return false;
	}

	CommandQueue = new r3dSF11CommandQueue();
	CommandQueue->pHAL = RendererHAL.GetPtr();
	CommandQueue->pRenderer2D = Renderer2D.GetPtr();

	ConfigureLoader();

	RegisterMsgProc(r3dScaleformGfxDX11WinProc);
	MsgProcRegistered = true;

	Created = true;

	r3dOutToLog("SF11: Create OK\n");
	return true;
}

void APIScaleformGfxDX11::Destroy()
{
	UnloadAppSelect();

	if(MsgProcRegistered)
	{
		UnregisterMsgProc(r3dScaleformGfxDX11WinProc);
		MsgProcRegistered = false;
	}

	SAFE_DELETE(CommandQueue);

	if(RendererHAL)
	{
		RendererHAL->ShutdownHAL();
	}

	Renderer2D = NULL;
	RendererHAL = NULL;

	if(SystemStarted)
	{
		Scaleform::GFx::System::Destroy();
		SystemStarted = false;
	}

	Created = false;

	PendingCommand[0] = 0;

	r3dOutToLog("SF11: Destroy OK\n");
}

void APIScaleformGfxDX11::ConfigureLoader()
{
	Scaleform::Ptr<r3dSF11FileOpener> fileOpener = *new r3dSF11FileOpener();
	Scaleform::Ptr<r3dSF11FSCommandHandler> fsHandler = *new r3dSF11FSCommandHandler();
	Scaleform::Ptr<r3dSF11ExternalInterface> externalHandler = *new r3dSF11ExternalInterface();
	Scaleform::Ptr<r3dSF11UserEventHandler> userEventHandler = *new r3dSF11UserEventHandler();
	Scaleform::Ptr<r3dSF11Log> log = *new r3dSF11Log();
	Scaleform::Ptr<r3dSF11Translator> translator = *new r3dSF11Translator();

	Scaleform::Ptr<Scaleform::GFx::ASSupport> as2Support = *new Scaleform::GFx::AS2Support();
	Scaleform::Ptr<Scaleform::GFx::ImageFileHandlerRegistry> imageRegistry = *new Scaleform::GFx::ImageFileHandlerRegistry();
	Scaleform::Ptr<Scaleform::GFx::ImageCreator> imageCreator = *new Scaleform::GFx::ImageCreator();

	imageRegistry->AddHandler(&Scaleform::Render::JPEG::FileReader::Instance);
	imageRegistry->AddHandler(&Scaleform::Render::PNG::FileReader::Instance);
	imageRegistry->AddHandler(&Scaleform::Render::TGA::FileReader::Instance);
	imageRegistry->AddHandler(&Scaleform::Render::DDS::FileReader::Instance);

	Loader.SetAS2Support(as2Support);
	Loader.SetFileOpener(fileOpener);
	Loader.SetFSCommandHandler(fsHandler);
	Loader.SetExternalInterface(externalHandler);
	Loader.SetUserEventHandler(userEventHandler);
	Loader.SetImageFileHandlerRegistry(imageRegistry);
	Loader.SetImageCreator(imageCreator);
	Loader.SetTranslator(translator);
	Loader.SetLog(log);

	SetFontLib();
}

void APIScaleformGfxDX11::SetFontLib()
{
	Scaleform::Ptr<Scaleform::GFx::FontMap> fontMap = *new Scaleform::GFx::FontMap;
	Loader.SetFontMap(fontMap);

	fontMap->MapFont("$WIHUDFontPrimary", "HelveticaNeueLT W1G 57 Cn", Scaleform::GFx::FontMap::MFF_Normal);
	fontMap->MapFont("$WIHUDFontPrimaryBold", "HelveticaNeueLT W1G 57 Cn", Scaleform::GFx::FontMap::MFF_Bold);

	Scaleform::Ptr<Scaleform::GFx::FontLib> fontLib = *new Scaleform::GFx::FontLib;
	Loader.SetFontLib(fontLib);

	Scaleform::Ptr<Scaleform::GFx::MovieDef> fontMovie = *Loader.CreateMovie("data/menu/font_en.swf");

	if(fontMovie)
		fontLib->AddFontsFrom(fontMovie, true);
}

bool APIScaleformGfxDX11::LoadAppSelect(const char* movieFile)
{
	if(!Created)
		return false;

	UnloadAppSelect();

	if(!movieFile || !movieFile[0])
		movieFile = "data/menu/AppSelect.gfx";

	r3dOutToLog("SF11: loading AppSelect movie %s\n", movieFile);

	AppSelectMovieDef = *Loader.CreateMovie(movieFile, Scaleform::GFx::Loader::LoadAll);

	if(!AppSelectMovieDef)
	{
		r3dOutToLog("SF11: failed to load %s\n", movieFile);
		return false;
	}

	AppSelectMovie = *AppSelectMovieDef->CreateInstance(true, 0, 0, CommandQueue);

	if(!AppSelectMovie)
	{
		r3dOutToLog("SF11: failed to create movie instance %s\n", movieFile);
		AppSelectMovieDef = NULL;
		return false;
	}

	AppSelectMovie->SetBackgroundAlpha(0.0f);
	AppSelectMovie->SetMouseCursorCount(1);

	AppSelectDisplayHandle = AppSelectMovie->GetDisplayHandle();

	LastAdvanceTime = r3dGetTime();
	PendingCommand[0] = 0;

	UpdateViewport();

	r3dOutToLog("SF11: AppSelect movie loaded OK\n");
	return true;
}

void APIScaleformGfxDX11::UnloadAppSelect()
{
	AppSelectMovie = NULL;
	AppSelectMovieDef = NULL;

	PendingCommand[0] = 0;
}

void APIScaleformGfxDX11::Reset()
{
	if(!Created)
		return;

	Width = 0;
	Height = 0;

	UpdateViewport();
}

bool APIScaleformGfxDX11::IsReady() const
{
	return Created && RendererHAL && Renderer2D;
}

void APIScaleformGfxDX11::UpdateViewport()
{
	if(!AppSelectMovie)
		return;

	Width = g_r3dDX11.GetWidth();
	Height = g_r3dDX11.GetHeight();

	if(Width <= 0)
		Width = (int)r3dRenderer->ScreenW;

	if(Height <= 0)
		Height = (int)r3dRenderer->ScreenH;

	if(Width <= 0)
		Width = 1280;

	if(Height <= 0)
		Height = 720;

	AppSelectMovie->SetViewport(Width, Height, 0, 0, Width, Height, 0);
	AppSelectMovie->SetViewScaleMode(Scaleform::GFx::Movie::SM_ExactFit);
}

void APIScaleformGfxDX11::UpdateMouse()
{
	if(!AppSelectMovie)
		return;

	POINT pt;
	GetCursorPos(&pt);

	if(win::hWnd)
		ScreenToClient(win::hWnd, &pt);

	int buttons = 0;

	if((GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0)
		buttons |= 1;

	if((GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0)
		buttons |= 2;

	if((GetAsyncKeyState(VK_MBUTTON) & 0x8000) != 0)
		buttons |= 4;

	AppSelectMovie->NotifyMouseState((float)pt.x, (float)pt.y, buttons);
}

void APIScaleformGfxDX11::AdvanceMovie()
{
	if(!AppSelectMovie)
		return;

	float now = r3dGetTime();
	float delta = now - LastAdvanceTime;

	if(delta < 0.0f)
		delta = 0.0f;

	if(delta > 0.1f)
		delta = 0.1f;

	LastAdvanceTime = now;

	AppSelectMovie->Advance(delta);
}

void APIScaleformGfxDX11::DrawMovie()
{
	if(!AppSelectMovie)
		return;

	if(!Renderer2D)
		return;

	ID3D11RenderTargetView* rtv = g_r3dDX11.GetBackBufferRTV();
	ID3D11DepthStencilView* dsv = g_r3dDX11.GetDepthStencilView();

	if(!rtv)
		return;

	ID3D11DeviceContext* context = g_r3dDX11.GetContext();

	if(!context)
		return;

	context->OMSetRenderTargets(1, &rtv, dsv);

	D3D11_VIEWPORT vp;
	ZeroMemory(&vp, sizeof(vp));

	vp.TopLeftX = 0.0f;
	vp.TopLeftY = 0.0f;
	vp.Width = (float)Width;
	vp.Height = (float)Height;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;

	context->RSSetViewports(1, &vp);

	Renderer2D->BeginFrame();

	if(AppSelectDisplayHandle.NextCapture(Renderer2D->GetContextNotify()))
		Renderer2D->Display(AppSelectDisplayHandle);

	Renderer2D->EndFrame();
}

int APIScaleformGfxDX11::RenderAppSelect()
{
	if(!Created)
		return R3D_SF_DX11_APPSELECT_NONE;

	if(!AppSelectMovie)
		return R3D_SF_DX11_APPSELECT_NONE;

	int currentW = g_r3dDX11.GetWidth();
	int currentH = g_r3dDX11.GetHeight();

	if(currentW != Width || currentH != Height)
		UpdateViewport();

	UpdateMouse();
	AdvanceMovie();
	DrawMovie();

	int result = CommandToAppSelectResult(PendingCommand);
	PendingCommand[0] = 0;

	return result;
}

void APIScaleformGfxDX11::PushCommand(const char* command)
{
	if(!command || !command[0])
		return;

	r3dscpy(PendingCommand, command);

	r3dOutToLog("SF11 AppSelect command: %s\n", PendingCommand);
}

int APIScaleformGfxDX11::CommandToAppSelectResult(const char* command) const
{
	if(!command || !command[0])
		return R3D_SF_DX11_APPSELECT_NONE;

	if(strcmp(command, "BtnPlayGame") == 0)
		return R3D_SF_DX11_APPSELECT_PLAY_GAME;

	if(strcmp(command, "BtnLevelEditor") == 0)
		return R3D_SF_DX11_APPSELECT_LEVEL_EDITOR;

	if(strcmp(command, "BtnParticleEditor") == 0)
		return R3D_SF_DX11_APPSELECT_PARTICLE_EDITOR;

	if(strcmp(command, "BtnPhysicsEditor") == 0)
		return R3D_SF_DX11_APPSELECT_PHYSICS_EDITOR;

	if(strcmp(command, "BtnCharacterEditor") == 0)
		return R3D_SF_DX11_APPSELECT_CHARACTER_EDITOR;

	if(strcmp(command, "BtnExit") == 0)
		return R3D_SF_DX11_APPSELECT_EXIT;

	return R3D_SF_DX11_APPSELECT_NONE;
}

bool APIScaleformGfxDX11::ProcessWindowMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if(!AppSelectMovie)
		return false;

	switch(uMsg)
	{
	case WM_KEYDOWN:
		{
			if(wParam == VK_ESCAPE)
			{
				PushCommand("BtnExit");
				return true;
			}

			Scaleform::GFx::KeyEvent keyEvent(
				Scaleform::GFx::Event::KeyDown,
				(Scaleform::Key::Code)wParam,
				0,
				0
			);

			AppSelectMovie->HandleEvent(keyEvent);
			break;
		}

	case WM_KEYUP:
		{
			Scaleform::GFx::KeyEvent keyEvent(
				Scaleform::GFx::Event::KeyUp,
				(Scaleform::Key::Code)wParam,
				0,
				0
			);

			AppSelectMovie->HandleEvent(keyEvent);
			break;
		}

	case WM_MOUSEWHEEL:
		{
			POINT pt;
			pt.x = GET_X_LPARAM(lParam);
			pt.y = GET_Y_LPARAM(lParam);

			if(win::hWnd)
				ScreenToClient(win::hWnd, &pt);

			int z = GET_WHEEL_DELTA_WPARAM(wParam);

			Scaleform::GFx::MouseEvent mouseEvent(
				Scaleform::GFx::MouseEvent::MouseWheel,
				0,
				(float)pt.x,
				(float)pt.y,
				(float)((z / WHEEL_DELTA) * 3),
				0
			);

			AppSelectMovie->HandleEvent(mouseEvent);
			break;
		}

	default:
		break;
	}

	return false;
}

bool r3dScaleformGfxDX11Create()
{
	if(gScaleformDX11)
		return true;

	gScaleformDX11 = new APIScaleformGfxDX11();

	if(!gScaleformDX11->Create())
	{
		SAFE_DELETE(gScaleformDX11);
		return false;
	}

	return true;
}

void r3dScaleformGfxDX11Destroy()
{
	if(!gScaleformDX11)
		return;

	gScaleformDX11->Destroy();
	SAFE_DELETE(gScaleformDX11);
}

void r3dScaleformGfxDX11Reset()
{
	if(gScaleformDX11)
		gScaleformDX11->Reset();
}

bool r3dScaleformGfxDX11IsReady()
{
	return gScaleformDX11 && gScaleformDX11->IsReady();
}

bool r3dScaleformGfxDX11LoadAppSelect(const char* movieFile)
{
	if(!gScaleformDX11)
		return false;

	return gScaleformDX11->LoadAppSelect(movieFile);
}

void r3dScaleformGfxDX11UnloadAppSelect()
{
	if(gScaleformDX11)
		gScaleformDX11->UnloadAppSelect();
}

int r3dScaleformGfxDX11RenderAppSelect()
{
	if(!gScaleformDX11)
		return R3D_SF_DX11_APPSELECT_NONE;

	return gScaleformDX11->RenderAppSelect();
}

bool r3dScaleformGfxDX11ProcessWindowMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if(!gScaleformDX11)
		return false;

	return gScaleformDX11->ProcessWindowMessage(uMsg, wParam, lParam);
}

#else

bool r3dScaleformGfxDX11Create() { return false; }
void r3dScaleformGfxDX11Destroy() {}
void r3dScaleformGfxDX11Reset() {}
bool r3dScaleformGfxDX11IsReady() { return false; }
bool r3dScaleformGfxDX11LoadAppSelect(const char* movieFile) { return false; }
void r3dScaleformGfxDX11UnloadAppSelect() {}
int r3dScaleformGfxDX11RenderAppSelect() { return R3D_SF_DX11_APPSELECT_NONE; }
bool r3dScaleformGfxDX11ProcessWindowMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) { return false; }

#endif