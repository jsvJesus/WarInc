#include "r3dPCH.h"

#ifndef WO_SERVER

#pragma warning(push)
#pragma warning(disable: 4005)
#pragma warning(disable: 4061)
#pragma warning(disable: 4062)
#pragma warning(disable: 4191)
#pragma warning(disable: 4255)
#pragma warning(disable: 4365)
#pragma warning(disable: 4571)
#pragma warning(disable: 4619)
#pragma warning(disable: 4625)
#pragma warning(disable: 4626)
#pragma warning(disable: 4668)
#pragma warning(disable: 4710)
#pragma warning(disable: 4711)
#pragma warning(disable: 4820)
#include <d3d11.h>
#pragma warning(pop)

#include "r3d.h"
#include "r3dRender.h"
#include "r3dDX11.h"
#include "r3dDX11RenderTarget.h"

#ifdef CreateDepthStencilSurface
#undef CreateDepthStencilSurface
#endif

#ifdef CreateRenderTarget
#undef CreateRenderTarget
#endif

#ifdef SetRenderTarget
#undef SetRenderTarget
#endif

template<typename T>
static void r3dDX11RT_Release(T*& ptr)
{
	if(ptr)
	{
		ptr->Release();
		ptr = NULL;
	}
}

static void r3dDX11RT_LogHR(const char* text, HRESULT hr)
{
	r3dOutToLog("DX11 RT: %s failed, HRESULT=0x%08X\n", text, (unsigned int)hr);
}

static DXGI_FORMAT r3dDX11RT_ConvertDepthFormat(int d3dFormat)
{
	if(
		d3dFormat == INTZ_FORMAT ||
		d3dFormat == RAWZ_FORMAT ||
		d3dFormat == DF24_FORMAT
		)
	{
		return DXGI_FORMAT_D24_UNORM_S8_UINT;
	}

	switch(d3dFormat)
	{
	case D3DFMT_D16:
		return DXGI_FORMAT_D16_UNORM;

	case D3DFMT_D24S8:
	case D3DFMT_D24X8:
		return DXGI_FORMAT_D24_UNORM_S8_UINT;

	case D3DFMT_D32:
	case D3DFMT_D32F_LOCKABLE:
		return DXGI_FORMAT_D32_FLOAT;

	default:
		return (DXGI_FORMAT)r3dDX11_ConvertLegacyD3DFormat(d3dFormat);
	}
}

static unsigned int r3dDX11RT_GetSampleCount(int multiSample)
{
	return multiSample <= D3DMULTISAMPLE_NONE ? 1u : (unsigned int)multiSample;
}

r3dDX11RenderTargetBridge g_r3dDX11RenderTargets;

r3dDX11RenderTargetBridge::RegisteredSurface::RegisteredSurface()
{
	RTV = NULL;
	DSV = NULL;
	Width = 0;
	Height = 0;
	HasRenderTarget = false;
	HasDepthStencil = false;
}

r3dDX11RenderTargetBridge::r3dDX11RenderTargetBridge()
{
	Initialized = false;
	CurrentDSV = NULL;
	CurrentWidth = 0;
	CurrentHeight = 0;

	for(int i = 0; i < R3D_DX11_MAX_RENDER_TARGETS; ++i)
		CurrentRTV[i] = NULL;
}

r3dDX11RenderTargetBridge::~r3dDX11RenderTargetBridge()
{
	Shutdown();
}

bool r3dDX11RenderTargetBridge::Init()
{
	Initialized = true;
	ClearCurrentTargets();
	ResetToBackBuffer();
	return true;
}

void r3dDX11RenderTargetBridge::Shutdown()
{
	ClearCurrentTargets();
	Surfaces.clear();
	Initialized = false;
}

bool r3dDX11RenderTargetBridge::IsInitialized() const
{
	return Initialized;
}

void r3dDX11RenderTargetBridge::ClearCurrentTargets()
{
	for(int i = 0; i < R3D_DX11_MAX_RENDER_TARGETS; ++i)
		CurrentRTV[i] = NULL;

	CurrentDSV = NULL;
	CurrentWidth = 0;
	CurrentHeight = 0;
}

void r3dDX11RenderTargetBridge::ResetToBackBuffer()
{
	if(!Initialized || !g_r3dDX11.IsInitialized())
		return;

	ClearCurrentTargets();

	CurrentRTV[0] = g_r3dDX11.GetBackBufferRTV();
	CurrentDSV = g_r3dDX11.GetDepthStencilView();
	CurrentWidth = (unsigned int)g_r3dDX11.GetWidth();
	CurrentHeight = (unsigned int)g_r3dDX11.GetHeight();

	Apply();
}

void r3dDX11RenderTargetBridge::RegisterRenderTarget(
	IDirect3DSurface9* surface,
	ID3D11RenderTargetView* rtv,
	unsigned int width,
	unsigned int height
)
{
	if(!surface)
		return;

	RegisteredSurface& entry = Surfaces[surface];
	entry.RTV = rtv;
	entry.Width = width;
	entry.Height = height;
	entry.HasRenderTarget = true;
}

void r3dDX11RenderTargetBridge::RegisterDepthStencil(
	IDirect3DSurface9* surface,
	ID3D11DepthStencilView* dsv,
	unsigned int width,
	unsigned int height
)
{
	if(!surface)
		return;

	RegisteredSurface& entry = Surfaces[surface];
	entry.DSV = dsv;
	entry.Width = width;
	entry.Height = height;
	entry.HasDepthStencil = true;
}

void r3dDX11RenderTargetBridge::UnregisterSurface(IDirect3DSurface9* surface)
{
	if(!surface)
		return;

	Surfaces.erase(surface);
}

bool r3dDX11RenderTargetBridge::FindRenderTarget(
	IDirect3DSurface9* surface,
	ID3D11RenderTargetView** rtv,
	unsigned int* width,
	unsigned int* height
) const
{
	SurfaceMap::const_iterator it = Surfaces.find(surface);
	if(it == Surfaces.end() || !it->second.HasRenderTarget)
		return false;

	*rtv = it->second.RTV;
	*width = it->second.Width;
	*height = it->second.Height;
	return true;
}

bool r3dDX11RenderTargetBridge::FindDepthStencil(
	IDirect3DSurface9* surface,
	ID3D11DepthStencilView** dsv,
	unsigned int* width,
	unsigned int* height
) const
{
	SurfaceMap::const_iterator it = Surfaces.find(surface);
	if(it == Surfaces.end() || !it->second.HasDepthStencil)
		return false;

	*dsv = it->second.DSV;
	*width = it->second.Width;
	*height = it->second.Height;
	return true;
}

bool r3dDX11RenderTargetBridge::SetRenderTarget(int slot, IDirect3DSurface9* surface)
{
	if(!Initialized || !g_r3dDX11.IsInitialized())
		return false;

	if(slot < 0 || slot >= R3D_DX11_MAX_RENDER_TARGETS)
		return false;

	if(!surface)
	{
		CurrentRTV[slot] = NULL;

		if(slot == 0)
		{
			CurrentWidth = 0;
			CurrentHeight = 0;
		}

		Apply();
		return true;
	}

	ID3D11RenderTargetView* rtv = NULL;
	unsigned int width = 0;
	unsigned int height = 0;

	if(!FindRenderTarget(surface, &rtv, &width, &height))
		return false;

	CurrentRTV[slot] = rtv;

	if(slot == 0)
	{
		CurrentWidth = width;
		CurrentHeight = height;
	}

	Apply();
	return true;
}

bool r3dDX11RenderTargetBridge::SetDepthStencil(IDirect3DSurface9* surface)
{
	if(!Initialized || !g_r3dDX11.IsInitialized())
		return false;

	if(!surface)
	{
		CurrentDSV = NULL;
		Apply();
		return true;
	}

	ID3D11DepthStencilView* dsv = NULL;
	unsigned int width = 0;
	unsigned int height = 0;

	if(!FindDepthStencil(surface, &dsv, &width, &height))
		return false;

	CurrentDSV = dsv;

	if(CurrentWidth == 0 || CurrentHeight == 0)
	{
		CurrentWidth = width;
		CurrentHeight = height;
	}

	Apply();
	return true;
}

bool r3dDX11RenderTargetBridge::Clear(
	unsigned int rectCount,
	const D3DRECT* rects,
	unsigned int flags,
	unsigned int color,
	float z,
	unsigned int stencil
)
{
	if(!Initialized || !g_r3dDX11.IsInitialized())
		return false;

	if(rectCount || rects)
		return false;

	ID3D11DeviceContext* context = g_r3dDX11.GetContext();
	if(!context)
		return false;

	if(flags & D3DCLEAR_TARGET)
	{
		const float colorValues[4] =
		{
			(float)((color >> 16) & 0xff) / 255.0f,
			(float)((color >> 8) & 0xff) / 255.0f,
			(float)(color & 0xff) / 255.0f,
			(float)((color >> 24) & 0xff) / 255.0f
		};

		for(int i = 0; i < R3D_DX11_MAX_RENDER_TARGETS; ++i)
		{
			if(CurrentRTV[i])
				context->ClearRenderTargetView(CurrentRTV[i], colorValues);
		}
	}

	if(CurrentDSV && (flags & (D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL)))
	{
		UINT dx11Flags = 0;

		if(flags & D3DCLEAR_ZBUFFER)
			dx11Flags |= D3D11_CLEAR_DEPTH;

		if(flags & D3DCLEAR_STENCIL)
			dx11Flags |= D3D11_CLEAR_STENCIL;

		if(dx11Flags)
			context->ClearDepthStencilView(CurrentDSV, dx11Flags, z, (UINT8)stencil);
	}

	return true;
}

void r3dDX11RenderTargetBridge::Apply()
{
	if(!Initialized || !g_r3dDX11.IsInitialized())
		return;

	ID3D11DeviceContext* context = g_r3dDX11.GetContext();
	if(!context)
		return;

	int targetCount = R3D_DX11_MAX_RENDER_TARGETS;
	while(targetCount > 1 && !CurrentRTV[targetCount - 1])
		--targetCount;

	context->OMSetRenderTargets(targetCount, CurrentRTV, CurrentDSV);

	if(CurrentWidth && CurrentHeight)
	{
		D3D11_VIEWPORT vp;
		ZeroMemory(&vp, sizeof(vp));

		vp.TopLeftX = 0.0f;
		vp.TopLeftY = 0.0f;
		vp.Width = (float)CurrentWidth;
		vp.Height = (float)CurrentHeight;
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;

		context->RSSetViewports(1, &vp);
	}
}

bool r3dDX11RenderTargetBridge::CreateRenderTargetMirror(
	r3dD3DSurfaceTunnel* surface,
	unsigned int width,
	unsigned int height,
	int d3dFormat,
	int multiSample,
	unsigned int multisampleQuality
)
{
	if(!surface || !g_r3dDX11.IsInitialized())
		return false;

	if(d3dFormat == NULL_RT_FORMAT)
	{
		surface->SetDX11RenderTargetMirror(NULL, NULL, width, height);
		return true;
	}

	DXGI_FORMAT format = (DXGI_FORMAT)r3dDX11_ConvertLegacyD3DFormat(d3dFormat);
	if(format == DXGI_FORMAT_UNKNOWN)
		return false;

	ID3D11Device* device = g_r3dDX11.GetDevice();
	if(!device)
		return false;

	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory(&desc, sizeof(desc));

	desc.Width = width;
	desc.Height = height;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = format;
	desc.SampleDesc.Count = r3dDX11RT_GetSampleCount(multiSample);
	desc.SampleDesc.Quality = desc.SampleDesc.Count == 1 ? 0 : multisampleQuality;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_RENDER_TARGET;

	ID3D11Texture2D* texture = NULL;
	HRESULT hr = device->CreateTexture2D(&desc, NULL, &texture);
	if(FAILED(hr))
	{
		r3dDX11RT_LogHR("CreateTexture2D RenderTarget", hr);
		return false;
	}

	ID3D11RenderTargetView* rtv = NULL;
	hr = device->CreateRenderTargetView(texture, NULL, &rtv);
	if(FAILED(hr))
	{
		r3dDX11RT_LogHR("CreateRenderTargetView", hr);
		r3dDX11RT_Release(texture);
		return false;
	}

	surface->SetDX11RenderTargetMirror(texture, rtv, width, height);
	return true;
}

bool r3dDX11RenderTargetBridge::CreateDepthStencilMirror(
	r3dD3DSurfaceTunnel* surface,
	unsigned int width,
	unsigned int height,
	int d3dFormat,
	int multiSample,
	unsigned int multisampleQuality
)
{
	if(!surface || !g_r3dDX11.IsInitialized())
		return false;

	DXGI_FORMAT format = r3dDX11RT_ConvertDepthFormat(d3dFormat);
	if(format == DXGI_FORMAT_UNKNOWN)
		return false;

	ID3D11Device* device = g_r3dDX11.GetDevice();
	if(!device)
		return false;

	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory(&desc, sizeof(desc));

	desc.Width = width;
	desc.Height = height;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = format;
	desc.SampleDesc.Count = r3dDX11RT_GetSampleCount(multiSample);
	desc.SampleDesc.Quality = desc.SampleDesc.Count == 1 ? 0 : multisampleQuality;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

	ID3D11Texture2D* texture = NULL;
	HRESULT hr = device->CreateTexture2D(&desc, NULL, &texture);
	if(FAILED(hr))
	{
		r3dDX11RT_LogHR("CreateTexture2D DepthStencil", hr);
		return false;
	}

	ID3D11DepthStencilView* dsv = NULL;
	hr = device->CreateDepthStencilView(texture, NULL, &dsv);
	if(FAILED(hr))
	{
		r3dDX11RT_LogHR("CreateDepthStencilView", hr);
		r3dDX11RT_Release(texture);
		return false;
	}

	surface->SetDX11DepthStencilMirror(texture, dsv, width, height);
	return true;
}

void r3dD3DSurfaceTunnel::SetDX11RenderTargetMirror(
	ID3D11Texture2D* texture,
	ID3D11RenderTargetView* rtv,
	unsigned int width,
	unsigned int height
)
{
	ReleaseDX11Mirror();

	mDX11Texture = texture;
	mDX11RTV = rtv;
	mDX11Width = width;
	mDX11Height = height;

	if(mRes)
		g_r3dDX11RenderTargets.RegisterRenderTarget(Get(), mDX11RTV, width, height);
}

void r3dD3DSurfaceTunnel::SetDX11DepthStencilMirror(
	ID3D11Texture2D* texture,
	ID3D11DepthStencilView* dsv,
	unsigned int width,
	unsigned int height
)
{
	ReleaseDX11Mirror();

	mDX11Texture = texture;
	mDX11DSV = dsv;
	mDX11Width = width;
	mDX11Height = height;

	if(mRes)
		g_r3dDX11RenderTargets.RegisterDepthStencil(Get(), mDX11DSV, width, height);
}

void r3dD3DSurfaceTunnel::ReleaseDX11Mirror()
{
	if(mRes)
		g_r3dDX11RenderTargets.UnregisterSurface(Get());

	r3dDX11RT_Release(mDX11RTV);
	r3dDX11RT_Release(mDX11DSV);
	r3dDX11RT_Release(mDX11Texture);

	mDX11Width = 0;
	mDX11Height = 0;
}

#endif
