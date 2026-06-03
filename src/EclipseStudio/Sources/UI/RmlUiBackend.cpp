#include "r3dPCH.h"
#include "r3d.h"

#include "RmlUiBackend.h"

#include "GameCommon.h"
#include "..\\Menus\\m_AppSelect.h"
#include "r3dDX11ScaleformBridge.h"

// -----------------------------------------------------------------------
//  Forward declarations used by the backend
// -----------------------------------------------------------------------
extern int AppSelectMode;
void ClearFullScreen_Menu();

// -----------------------------------------------------------------------
//  Globals
// -----------------------------------------------------------------------
const DWORD RMLUI_D3D9_FVF = D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1;

bool RmlUiBackend::s_initialized = false;
RmlUiSystemInterface*   RmlUiBackend::s_system  = NULL;
RmlUiRenderInterface*   RmlUiBackend::s_render  = NULL;
Rocket::Core::Context*  RmlUiBackend::s_context = NULL;

// =======================================================================
//  RmlUiSystemInterface
// =======================================================================
float RmlUiSystemInterface::GetElapsedTime()
{
	return r3dGetTime();
}

bool RmlUiSystemInterface::LogMessage(Rocket::Core::Log::Type type, const Rocket::Core::String& message)
{
	const char* level = "Info";

	switch(type)
	{
	case Rocket::Core::Log::LT_ERROR:   level = "Error";   break;
	case Rocket::Core::Log::LT_WARNING: level = "Warning"; break;
	case Rocket::Core::Log::LT_DEBUG:   level = "Debug";   break;
	default: break;
	}

	r3dOutToLog("RmlUi %s: %s\n", level, message.CString());
	return true;
}

// =======================================================================
//  RmlUiRenderInterface
// =======================================================================
RmlUiRenderInterface::RmlUiRenderInterface()
: m_device(NULL)
, m_width(0)
, m_height(0)
{
}

void RmlUiRenderInterface::SetDevice(IDirect3DDevice9* device, int width, int height)
{
	m_device = device;
	m_width  = width;
	m_height = height;
}

void RmlUiRenderInterface::PrepareState()
{
	if(!m_device)
		return;

	D3DVIEWPORT9 vp;
	vp.X      = 0;
	vp.Y      = 0;
	vp.Width  = (DWORD)R3D_MAX(m_width, 1);
	vp.Height = (DWORD)R3D_MAX(m_height, 1);
	vp.MinZ   = 0.0f;
	vp.MaxZ   = 1.0f;

	m_device->SetViewport(&vp);
	m_device->SetFVF(RMLUI_D3D9_FVF);
	m_device->SetVertexShader(NULL);
	m_device->SetPixelShader(NULL);

	m_device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	m_device->SetRenderState(D3DRS_SRCBLEND,  D3DBLEND_SRCALPHA);
	m_device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	m_device->SetRenderState(D3DRS_ALPHATESTENABLE,  FALSE);
	m_device->SetRenderState(D3DRS_CULLMODE,  D3DCULL_NONE);
	m_device->SetRenderState(D3DRS_LIGHTING,  FALSE);
	m_device->SetRenderState(D3DRS_ZENABLE,   FALSE);
	m_device->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
	m_device->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);

	m_device->SetTextureStageState(0, D3DTSS_COLOROP,   D3DTOP_MODULATE);
	m_device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	m_device->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	m_device->SetTextureStageState(0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE);
	m_device->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	m_device->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
	m_device->SetTextureStageState(1, D3DTSS_COLOROP,   D3DTOP_DISABLE);
	m_device->SetTextureStageState(1, D3DTSS_ALPHAOP,   D3DTOP_DISABLE);

	m_device->SetSamplerState(0, D3DSAMP_ADDRESSU,  D3DTADDRESS_CLAMP);
	m_device->SetSamplerState(0, D3DSAMP_ADDRESSV,  D3DTADDRESS_CLAMP);
	m_device->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	m_device->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	m_device->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_NONE);
}

void RmlUiRenderInterface::RenderGeometry(
	Rocket::Core::Vertex* vertices,
	int num_vertices,
	int* indices,
	int num_indices,
	Rocket::Core::TextureHandle texture,
	const Rocket::Core::Vector2f& translation)
{
	if(!m_device || !vertices || !indices || num_vertices <= 0 || num_indices <= 0)
		return;

	m_vertices.resize(num_vertices);

	for(int i = 0; i < num_vertices; ++i)
	{
		const Rocket::Core::Vertex& src = vertices[i];
		RmlUiBackendVertex& dst = m_vertices[i];

		dst.x     = src.position.x + translation.x;
		dst.y     = src.position.y + translation.y;
		dst.z     = 0.0f;
		dst.rhw   = 1.0f;
		dst.color = D3DCOLOR_ARGB(src.colour.alpha, src.colour.red, src.colour.green, src.colour.blue);
		dst.u     = src.tex_coord.x;
		dst.v     = src.tex_coord.y;
	}

	m_device->SetTexture(0, reinterpret_cast<IDirect3DTexture9*>(texture));
	m_device->SetFVF(RMLUI_D3D9_FVF);
	m_device->DrawIndexedPrimitiveUP(
		D3DPT_TRIANGLELIST,
		0,
		num_vertices,
		num_indices / 3,
		indices,
		D3DFMT_INDEX32,
		&m_vertices[0],
		sizeof(RmlUiBackendVertex));
}

void RmlUiRenderInterface::EnableScissorRegion(bool enable)
{
	if(m_device)
		m_device->SetRenderState(D3DRS_SCISSORTESTENABLE, enable ? TRUE : FALSE);
}

void RmlUiRenderInterface::SetScissorRegion(int x, int y, int width, int height)
{
	if(!m_device)
		return;

	RECT rect;
	rect.left   = x;
	rect.top    = y;
	rect.right  = x + width;
	rect.bottom = y + height;

	m_device->SetScissorRect(&rect);
}

bool RmlUiRenderInterface::GenerateTexture(
	Rocket::Core::TextureHandle& texture_handle,
	const Rocket::Core::byte* source,
	const Rocket::Core::Vector2i& source_dimensions)
{
	if(!m_device || !source || source_dimensions.x <= 0 || source_dimensions.y <= 0)
		return false;

	IDirect3DTexture9* texture = NULL;
	HRESULT hr = m_device->CreateTexture(
		source_dimensions.x,
		source_dimensions.y,
		1,
		D3DUSAGE_DYNAMIC,
		D3DFMT_A8R8G8B8,
		D3DPOOL_DEFAULT,
		&texture,
		NULL);

	if(FAILED(hr) || !texture)
	{
		r3dOutToLog("RmlUi: CreateTexture failed hr=0x%08x\n", hr);
		return false;
	}

	D3DLOCKED_RECT locked;
	ZeroMemory(&locked, sizeof(locked));

	hr = texture->LockRect(0, &locked, NULL, D3DLOCK_DISCARD);
	if(FAILED(hr))
	{
		texture->Release();
		r3dOutToLog("RmlUi: font texture LockRect failed hr=0x%08x\n", hr);
		return false;
	}

	for(int y = 0; y < source_dimensions.y; ++y)
	{
		unsigned char* dst = static_cast<unsigned char*>(locked.pBits) + locked.Pitch * y;
		const unsigned char* src = source + source_dimensions.x * y * 4;

		for(int x = 0; x < source_dimensions.x; ++x)
		{
			dst[x * 4 + 0] = src[x * 4 + 2];
			dst[x * 4 + 1] = src[x * 4 + 1];
			dst[x * 4 + 2] = src[x * 4 + 0];
			dst[x * 4 + 3] = src[x * 4 + 3];
		}
	}

	texture->UnlockRect(0);
	texture_handle = reinterpret_cast<Rocket::Core::TextureHandle>(texture);
	return true;
}

void RmlUiRenderInterface::ReleaseTexture(Rocket::Core::TextureHandle texture)
{
	IDirect3DTexture9* d3d_texture = reinterpret_cast<IDirect3DTexture9*>(texture);

	if(d3d_texture)
		d3d_texture->Release();
}

// =======================================================================
//  Helpers (anonymous namespace in cpp)
// =======================================================================
namespace
{
	IDirect3DDevice9* GetRmlD3D9Device()
	{
		if(g_r3dDX11ScaleformBridge.IsReady())
			return r3dGetScaleformD3D9Device();

		if(!r3dRenderer)
			return NULL;

		return r3dRenderer->pd3ddev;
	}

	bool LoadDefaultFont()
	{
		const char* fonts[] =
		{
			"C:/Windows/Fonts/arial.ttf",
			"C:/Windows/Fonts/segoeui.ttf",
			"C:/Windows/Fonts/tahoma.ttf",
		};

		for(int i = 0; i < R3D_ARRAYSIZE(fonts); ++i)
		{
			if(Rocket::Core::FontDatabase::LoadFontFace(fonts[i], "Arial",
				Rocket::Core::Font::STYLE_NORMAL,
				Rocket::Core::Font::WEIGHT_NORMAL))
			{
				r3dOutToLog("RmlUi: loaded font %s\n", fonts[i]);
				return true;
			}
		}

		r3dOutToLog("RmlUi: failed to load a Windows UI font\n");
		return false;
	}
}

// =======================================================================
//  RmlUiBackend  implementation
// =======================================================================
bool RmlUiBackend::Initialize()
{
	if(s_initialized)
		return true;

	IDirect3DDevice9* device = GetRmlD3D9Device();
	if(!device)
	{
		r3dOutToLog("RmlUi Backend: no D3D9 device available\n");
		return false;
	}

	s_system = new RmlUiSystemInterface();
	s_render = new RmlUiRenderInterface();

	s_render->SetDevice(device, (int)r3dRenderer->ScreenW, (int)r3dRenderer->ScreenH);

	Rocket::Core::SetSystemInterface(s_system);
	Rocket::Core::SetRenderInterface(s_render);

	if(!Rocket::Core::Initialise())
	{
		r3dOutToLog("RmlUi Backend: Rocket::Core::Initialise failed\n");
		delete s_system; s_system = NULL;
		delete s_render; s_render = NULL;
		return false;
	}

	s_initialized = true;

	LoadDefaultFont();

	s_context = Rocket::Core::CreateContext(
		"WarIncRmlUi",
		Rocket::Core::Vector2i((int)r3dRenderer->ScreenW, (int)r3dRenderer->ScreenH));

	if(!s_context)
	{
		r3dOutToLog("RmlUi Backend: CreateContext failed\n");
		Shutdown();
		return false;
	}

	r3dOutToLog("RmlUi Backend: initialized (%dx%d)\n",
		(int)r3dRenderer->ScreenW, (int)r3dRenderer->ScreenH);
	return true;
}

void RmlUiBackend::Shutdown()
{
	if(s_context)
	{
		s_context->RemoveReference();
		s_context = NULL;
	}

	if(s_initialized)
	{
		Rocket::Core::Shutdown();
		s_initialized = false;
	}

	delete s_system;  s_system  = NULL;
	delete s_render;  s_render  = NULL;

	r3dOutToLog("RmlUi Backend: shut down\n");
}

bool RmlUiBackend::IsInitialized()
{
	return s_initialized;
}

Rocket::Core::Context* RmlUiBackend::GetContext()
{
	return s_context;
}

Rocket::Core::ElementDocument* RmlUiBackend::LoadDocumentFromMemory(const Rocket::Core::String& rml)
{
	if(!s_context)
		return NULL;

	return s_context->LoadDocumentFromMemory(rml);
}

Rocket::Core::ElementDocument* RmlUiBackend::LoadDocumentFromFile(const char* filePath)
{
	if(!s_context)
		return NULL;

	return s_context->LoadDocument(filePath);
}

void RmlUiBackend::BeginFrame()
{
	if(g_r3dDX11ScaleformBridge.IsReady())
		g_r3dDX11ScaleformBridge.BeginScaleformRender();
}

void RmlUiBackend::EndFrame()
{
	if(g_r3dDX11ScaleformBridge.IsReady())
	{
		g_r3dDX11ScaleformBridge.EndScaleformRender();
		g_r3dDX11ScaleformBridge.DrawDX11();
	}
}

void RmlUiBackend::ProcessMouse()
{
	if(!s_context)
		return;

	const int modifiers = GetKeyModifierState();

	int mx = 0;
	int my = 0;
	Mouse->GetXY(mx, my);

	s_context->ProcessMouseMove(mx, my, modifiers);

	if(Mouse->WasPressed(r3dMouse::mLeftButton))
		s_context->ProcessMouseButtonDown(0, modifiers);

	if(Mouse->WasReleased(r3dMouse::mLeftButton))
		s_context->ProcessMouseButtonUp(0, modifiers);
}

int RmlUiBackend::GetKeyModifierState()
{
	int modifiers = 0;

	if(Keyboard->IsPressed(kbsLeftShift) || Keyboard->IsPressed(kbsRightShift))
		modifiers |= Rocket::Core::Input::KM_SHIFT;

	if(Keyboard->IsPressed(kbsLeftControl) || Keyboard->IsPressed(kbsRightControl))
		modifiers |= Rocket::Core::Input::KM_CTRL;

	if(Keyboard->IsPressed(kbsLeftAlt) || Keyboard->IsPressed(kbsRightAlt))
		modifiers |= Rocket::Core::Input::KM_ALT;

	return modifiers;
}

int RmlUiBackend::ProcessEscapeKey()
{
	if(Keyboard->WasPressed(kbsEsc))
		return RMLUI_BACKEND_QUIT_RESULT;

	return 0;
}

int RmlUiBackend::ProcessHotkeys(const struct AppSelectCommandBinding* commands, int numCommands)
{
	for(int i = 0; i < numCommands; ++i)
	{
		if(Keyboard->WasPressed(commands[i].HotKey))
			return commands[i].Result;
	}

	return 0;
}

float RmlUiBackend::GetScreenWidth()
{
	return r3dRenderer ? (float)r3dRenderer->ScreenW : 1024.0f;
}

float RmlUiBackend::GetScreenHeight()
{
	return r3dRenderer ? (float)r3dRenderer->ScreenH : 768.0f;
}
