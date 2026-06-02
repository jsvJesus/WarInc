#include "r3dPCH.h"
#include "r3d.h"
#include "r3dDX11ScaleformBridge.h"
#include "r3dDX11Geometry.h"
#include "r3dDX11State.h"

#include <d3dcompiler.h>

#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")

r3dDX11ScaleformBridge g_r3dDX11ScaleformBridge;

#ifdef GetRenderTarget
#define R3D_SF_BRIDGE_RESTORE_GET_RENDER_TARGET
#undef GetRenderTarget
#endif

#ifdef GetDepthStencilSurface
#define R3D_SF_BRIDGE_RESTORE_GET_DEPTH_STENCIL_SURFACE
#undef GetDepthStencilSurface
#endif

#ifdef SetRenderTarget
#define R3D_SF_BRIDGE_RESTORE_SET_RENDER_TARGET
#undef SetRenderTarget
#endif

#ifdef SetDepthStencilSurface
#define R3D_SF_BRIDGE_RESTORE_SET_DEPTH_STENCIL_SURFACE
#undef SetDepthStencilSurface
#endif

#ifdef SetViewport
#define R3D_SF_BRIDGE_RESTORE_SET_VIEWPORT
#undef SetViewport
#endif

static HRESULT r3dSFBridgeGetRenderTarget(IDirect3DDevice9Ex* device, DWORD index, IDirect3DSurface9** surface)
{
	if(!device)
		return E_POINTER;

	return device->GetRenderTarget(index, surface);
}

static HRESULT r3dSFBridgeGetDepthStencilSurface(IDirect3DDevice9Ex* device, IDirect3DSurface9** surface)
{
	if(!device)
		return E_POINTER;

	return device->GetDepthStencilSurface(surface);
}

static HRESULT r3dSFBridgeSetRenderTarget(IDirect3DDevice9Ex* device, DWORD index, IDirect3DSurface9* surface)
{
	if(!device)
		return E_POINTER;

	return device->SetRenderTarget(index, surface);
}

static HRESULT r3dSFBridgeSetDepthStencilSurface(IDirect3DDevice9Ex* device, IDirect3DSurface9* surface)
{
	if(!device)
		return E_POINTER;

	return device->SetDepthStencilSurface(surface);
}

static HRESULT r3dSFBridgeSetViewport(IDirect3DDevice9Ex* device, const D3DVIEWPORT9* viewport)
{
	if(!device)
		return E_POINTER;

	return device->SetViewport(viewport);
}

#ifdef R3D_SF_BRIDGE_RESTORE_GET_RENDER_TARGET
#define GetRenderTarget DIRECT_CALLS_OF_GET_RENDER_TARGET_FUNCTION_NOT_ALLOWED_USE_REDRENDERLAYER_GETRT
#undef R3D_SF_BRIDGE_RESTORE_GET_RENDER_TARGET
#endif

#ifdef R3D_SF_BRIDGE_RESTORE_GET_DEPTH_STENCIL_SURFACE
#define GetDepthStencilSurface DIRECT_CALLS_OF_GET_DEPTH_STENCIL_SURFACE_FUNCTION_NOT_ALLOWED_USE_REDRENDERLAYER_GETDSS
#undef R3D_SF_BRIDGE_RESTORE_GET_DEPTH_STENCIL_SURFACE
#endif

#ifdef R3D_SF_BRIDGE_RESTORE_SET_RENDER_TARGET
#define SetRenderTarget DIRECT_CALLS_OF_SET_RENDER_TARGET_FUNCTION_NOT_ALLOWED_USE_REDRENDERLAYER_SETRT
#undef R3D_SF_BRIDGE_RESTORE_SET_RENDER_TARGET
#endif

#ifdef R3D_SF_BRIDGE_RESTORE_SET_DEPTH_STENCIL_SURFACE
#define SetDepthStencilSurface DIRECT_CALLS_OF_SET_RENDER_TARGET_FUNCTION_NOT_ALLOWED_USE_REDRENDERLAYER_SETDSS
#undef R3D_SF_BRIDGE_RESTORE_SET_DEPTH_STENCIL_SURFACE
#endif

#ifdef R3D_SF_BRIDGE_RESTORE_SET_VIEWPORT
#define SetViewport DoSetViewport
#undef R3D_SF_BRIDGE_RESTORE_SET_VIEWPORT
#endif

r3dDX11ScaleformBridge::r3dDX11ScaleformBridge()
: m_hWnd(NULL)
, m_width(0)
, m_height(0)
, m_sceneBegun(false)
, m_d3d9(NULL)
, m_device9(NULL)
, m_sharedTexture9(NULL)
, m_sharedSurface9(NULL)
, m_readbackSurface9(NULL)
, m_oldSurface9(NULL)
, m_oldDepth9(NULL)
, m_sharedHandle(NULL)
, m_flushQuery9(NULL)
, m_device11(NULL)
, m_context11(NULL)
, m_sharedTexture11(NULL)
, m_sharedSRV11(NULL)
, m_vs(NULL)
, m_ps(NULL)
, m_sampler(NULL)
, m_blendState(NULL)
, m_depthState(NULL)
, m_rasterState(NULL)
{
}

r3dDX11ScaleformBridge::~r3dDX11ScaleformBridge()
{
	Shutdown();
}

bool r3dDX11ScaleformBridge::Init(HWND hWnd, ID3D11Device* dx11Device, ID3D11DeviceContext* dx11Context, int width, int height)
{
	Shutdown();

	if(!hWnd || !dx11Device || !dx11Context || width <= 0 || height <= 0)
		return false;

	m_hWnd = hWnd;
	m_width = width;
	m_height = height;

	m_device11 = dx11Device;
	m_context11 = dx11Context;

	m_device11->AddRef();
	m_context11->AddRef();

	if(!CreateD3D9(hWnd))
	{
		Shutdown();
		return false;
	}

	if(!CreateSharedTarget(width, height))
	{
		Shutdown();
		return false;
	}

	if(!CreateDX11DrawResources())
	{
		Shutdown();
		return false;
	}

	r3dOutToLog("DX11 Scaleform bridge initialized: %dx%d\n", width, height);
	return true;
}

void r3dDX11ScaleformBridge::Shutdown()
{
	if(m_sceneBegun && m_device9)
	{
		m_device9->EndScene();
		m_sceneBegun = false;
	}

	DestroyDX11DrawResources();
	DestroySharedTarget();

	SAFE_RELEASE(m_flushQuery9);
	SAFE_RELEASE(m_device9);
	SAFE_RELEASE(m_d3d9);

	SAFE_RELEASE(m_context11);
	SAFE_RELEASE(m_device11);

	m_hWnd = NULL;
	m_width = 0;
	m_height = 0;
	m_sharedHandle = NULL;
}

bool r3dDX11ScaleformBridge::Resize(int width, int height)
{
	if(!m_device9 || !m_device11 || width <= 0 || height <= 0)
		return false;

	if(width == m_width && height == m_height && m_sharedTexture9 && m_sharedSRV11)
		return true;

	m_width = width;
	m_height = height;

	DestroySharedTarget();

	if(!CreateSharedTarget(width, height))
		return false;

	return true;
}

bool r3dDX11ScaleformBridge::CreateD3D9(HWND hWnd)
{
	HRESULT hr = Direct3DCreate9Ex(D3D_SDK_VERSION, &m_d3d9);
	if(FAILED(hr) || !m_d3d9)
	{
		r3dOutToLog("DX11 Scaleform bridge: Direct3DCreate9Ex failed hr=0x%08x\n", hr);
		return false;
	}

	D3DPRESENT_PARAMETERS pp;
	ZeroMemory(&pp, sizeof(pp));

	pp.Windowed = TRUE;
	pp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	pp.hDeviceWindow = hWnd;
	pp.BackBufferWidth = m_width;
	pp.BackBufferHeight = m_height;
	pp.BackBufferFormat = D3DFMT_A8R8G8B8;
	pp.BackBufferCount = 1;
	pp.EnableAutoDepthStencil = FALSE;
	pp.AutoDepthStencilFormat = D3DFMT_UNKNOWN;
	pp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

	DWORD flags =
		D3DCREATE_HARDWARE_VERTEXPROCESSING |
		D3DCREATE_MULTITHREADED |
		D3DCREATE_FPU_PRESERVE;

	hr = m_d3d9->CreateDeviceEx(
		D3DADAPTER_DEFAULT,
		D3DDEVTYPE_HAL,
		hWnd,
		flags,
		&pp,
		NULL,
		&m_device9
	);

	if(FAILED(hr) || !m_device9)
	{
		r3dOutToLog("DX11 Scaleform bridge: CreateDeviceEx failed hr=0x%08x\n", hr);
		return false;
	}

	hr = m_device9->CreateQuery(D3DQUERYTYPE_EVENT, &m_flushQuery9);
	if(FAILED(hr))
	{
		r3dOutToLog("DX11 Scaleform bridge: CreateQuery failed hr=0x%08x\n", hr);
		m_flushQuery9 = NULL;
	}

	return true;
}

bool r3dDX11ScaleformBridge::CreateSharedTarget(int width, int height)
{
	m_sharedHandle = NULL;

	HRESULT hr = m_device9->CreateTexture(
		width,
		height,
		1,
		D3DUSAGE_RENDERTARGET,
		D3DFMT_A8R8G8B8,
		D3DPOOL_DEFAULT,
		&m_sharedTexture9,
		NULL
	);

	if(FAILED(hr) || !m_sharedTexture9)
	{
		r3dOutToLog("DX11 Scaleform bridge: CreateTexture D3D9 target failed hr=0x%08x\n", hr);
		return false;
	}

	hr = m_sharedTexture9->GetSurfaceLevel(0, &m_sharedSurface9);
	if(FAILED(hr) || !m_sharedSurface9)
	{
		r3dOutToLog("DX11 Scaleform bridge: GetSurfaceLevel failed hr=0x%08x\n", hr);
		return false;
	}

	hr = m_device9->CreateOffscreenPlainSurface(
		width,
		height,
		D3DFMT_A8R8G8B8,
		D3DPOOL_SYSTEMMEM,
		&m_readbackSurface9,
		NULL
	);

	if(FAILED(hr) || !m_readbackSurface9)
	{
		r3dOutToLog("DX11 Scaleform bridge: CreateOffscreenPlainSurface readback failed hr=0x%08x\n", hr);
		return false;
	}

	D3D11_TEXTURE2D_DESC texDesc;
	ZeroMemory(&texDesc, sizeof(texDesc));

	texDesc.Width = (UINT)width;
	texDesc.Height = (UINT)height;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;

	hr = m_device11->CreateTexture2D(&texDesc, NULL, &m_sharedTexture11);
	if(FAILED(hr) || !m_sharedTexture11)
	{
		r3dOutToLog("DX11 Scaleform bridge: CreateTexture2D upload target failed hr=0x%08x\n", hr);
		return false;
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory(&srvDesc, sizeof(srvDesc));

	srvDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;

	hr = m_device11->CreateShaderResourceView(m_sharedTexture11, &srvDesc, &m_sharedSRV11);
	if(FAILED(hr) || !m_sharedSRV11)
	{
		r3dOutToLog("DX11 Scaleform bridge: CreateShaderResourceView failed hr=0x%08x\n", hr);
		return false;
	}

	return true;
}

void r3dDX11ScaleformBridge::DestroySharedTarget()
{
	SAFE_RELEASE(m_sharedSRV11);
	SAFE_RELEASE(m_sharedTexture11);

	SAFE_RELEASE(m_oldDepth9);
	SAFE_RELEASE(m_oldSurface9);

	SAFE_RELEASE(m_readbackSurface9);
	SAFE_RELEASE(m_sharedSurface9);
	SAFE_RELEASE(m_sharedTexture9);

	m_sharedHandle = NULL;
}

bool r3dDX11ScaleformBridge::CopyD3D9TargetToDX11()
{
	if(!m_device9 || !m_sharedSurface9 || !m_readbackSurface9 || !m_context11 || !m_sharedTexture11)
		return false;

	HRESULT hr = m_device9->GetRenderTargetData(m_sharedSurface9, m_readbackSurface9);
	if(FAILED(hr))
	{
		r3dOutToLog("DX11 Scaleform bridge: GetRenderTargetData failed hr=0x%08x\n", hr);
		return false;
	}

	D3DLOCKED_RECT locked;
	ZeroMemory(&locked, sizeof(locked));

	hr = m_readbackSurface9->LockRect(&locked, NULL, D3DLOCK_READONLY);
	if(FAILED(hr))
	{
		r3dOutToLog("DX11 Scaleform bridge: readback LockRect failed hr=0x%08x\n", hr);
		return false;
	}

	m_context11->UpdateSubresource(m_sharedTexture11, 0, NULL, locked.pBits, (UINT)locked.Pitch, 0);
	m_readbackSurface9->UnlockRect();

	return true;
}

bool r3dDX11ScaleformBridge::CreateDX11DrawResources()
{
	static const char* vsCode =
		"struct VS_OUT"
		"{"
		"	float4 Pos : SV_POSITION;"
		"	float2 UV  : TEXCOORD0;"
		"};"
		"VS_OUT main(uint id : SV_VertexID)"
		"{"
		"	VS_OUT o;"
		"	float2 pos[3];"
		"	pos[0] = float2(-1.0f, -1.0f);"
		"	pos[1] = float2(-1.0f,  3.0f);"
		"	pos[2] = float2( 3.0f, -1.0f);"
		"	float2 uv[3];"
		"	uv[0] = float2(0.0f, 1.0f);"
		"	uv[1] = float2(0.0f, -1.0f);"
		"	uv[2] = float2(2.0f, 1.0f);"
		"	o.Pos = float4(pos[id], 0.0f, 1.0f);"
		"	o.UV = uv[id];"
		"	return o;"
		"}";

	static const char* psCode =
		"Texture2D uiTex : register(t0);"
		"SamplerState uiSampler : register(s0);"
		"float4 main(float4 pos : SV_POSITION, float2 uv : TEXCOORD0) : SV_TARGET"
		"{"
		"	return uiTex.Sample(uiSampler, uv);"
		"}";

	ID3DBlob* vsBlob = NULL;
	ID3DBlob* psBlob = NULL;
	ID3DBlob* errBlob = NULL;

	HRESULT hr = D3DCompile(vsCode, strlen(vsCode), "ScaleformBridgeVS", NULL, NULL, "main", "vs_4_0", 0, 0, &vsBlob, &errBlob);
	if(FAILED(hr))
	{
		if(errBlob)
			r3dOutToLog("DX11 Scaleform bridge VS compile error: %s\n", (const char*)errBlob->GetBufferPointer());

		SAFE_RELEASE(errBlob);
		return false;
	}

	SAFE_RELEASE(errBlob);

	hr = D3DCompile(psCode, strlen(psCode), "ScaleformBridgePS", NULL, NULL, "main", "ps_4_0", 0, 0, &psBlob, &errBlob);
	if(FAILED(hr))
	{
		if(errBlob)
			r3dOutToLog("DX11 Scaleform bridge PS compile error: %s\n", (const char*)errBlob->GetBufferPointer());

		SAFE_RELEASE(vsBlob);
		SAFE_RELEASE(errBlob);
		return false;
	}

	SAFE_RELEASE(errBlob);

	hr = m_device11->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), NULL, &m_vs);
	if(FAILED(hr))
	{
		SAFE_RELEASE(vsBlob);
		SAFE_RELEASE(psBlob);
		return false;
	}

	hr = m_device11->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), NULL, &m_ps);
	if(FAILED(hr))
	{
		SAFE_RELEASE(vsBlob);
		SAFE_RELEASE(psBlob);
		return false;
	}

	SAFE_RELEASE(vsBlob);
	SAFE_RELEASE(psBlob);

	D3D11_SAMPLER_DESC samplerDesc;
	ZeroMemory(&samplerDesc, sizeof(samplerDesc));

	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samplerDesc.MinLOD = 0.0f;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	hr = m_device11->CreateSamplerState(&samplerDesc, &m_sampler);
	if(FAILED(hr))
		return false;

	D3D11_BLEND_DESC blendDesc;
	ZeroMemory(&blendDesc, sizeof(blendDesc));

	blendDesc.RenderTarget[0].BlendEnable = TRUE;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	hr = m_device11->CreateBlendState(&blendDesc, &m_blendState);
	if(FAILED(hr))
		return false;

	D3D11_DEPTH_STENCIL_DESC depthDesc;
	ZeroMemory(&depthDesc, sizeof(depthDesc));

	depthDesc.DepthEnable = FALSE;
	depthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	depthDesc.DepthFunc = D3D11_COMPARISON_ALWAYS;
	depthDesc.StencilEnable = FALSE;

	hr = m_device11->CreateDepthStencilState(&depthDesc, &m_depthState);
	if(FAILED(hr))
		return false;

	D3D11_RASTERIZER_DESC rasterDesc;
	ZeroMemory(&rasterDesc, sizeof(rasterDesc));

	rasterDesc.FillMode = D3D11_FILL_SOLID;
	rasterDesc.CullMode = D3D11_CULL_NONE;
	rasterDesc.FrontCounterClockwise = FALSE;
	rasterDesc.DepthBias = 0;
	rasterDesc.DepthBiasClamp = 0.0f;
	rasterDesc.SlopeScaledDepthBias = 0.0f;
	rasterDesc.DepthClipEnable = FALSE;
	rasterDesc.ScissorEnable = FALSE;
	rasterDesc.MultisampleEnable = FALSE;
	rasterDesc.AntialiasedLineEnable = FALSE;

	hr = m_device11->CreateRasterizerState(&rasterDesc, &m_rasterState);
	if(FAILED(hr))
		return false;

	return true;
}

void r3dDX11ScaleformBridge::DestroyDX11DrawResources()
{
	SAFE_RELEASE(m_rasterState);
	SAFE_RELEASE(m_depthState);
	SAFE_RELEASE(m_blendState);
	SAFE_RELEASE(m_sampler);
	SAFE_RELEASE(m_ps);
	SAFE_RELEASE(m_vs);
}

bool r3dDX11ScaleformBridge::BeginScaleformRender()
{
	if(!m_device9 || !m_sharedSurface9)
		return false;

	if(m_sceneBegun)
	{
		m_device9->EndScene();
		m_sceneBegun = false;
	}

	SAFE_RELEASE(m_oldSurface9);
	SAFE_RELEASE(m_oldDepth9);

	r3dSFBridgeGetRenderTarget(m_device9, 0, &m_oldSurface9);
	r3dSFBridgeGetDepthStencilSurface(m_device9, &m_oldDepth9);

	HRESULT hr = r3dSFBridgeSetRenderTarget(m_device9, 0, m_sharedSurface9);
	if(FAILED(hr))
		return false;

	r3dSFBridgeSetDepthStencilSurface(m_device9, NULL);

	D3DVIEWPORT9 vp;
	vp.X = 0;
	vp.Y = 0;
	vp.Width = (DWORD)m_width;
	vp.Height = (DWORD)m_height;
	vp.MinZ = 0.0f;
	vp.MaxZ = 1.0f;
	r3dSFBridgeSetViewport(m_device9, &vp);

	m_device9->Clear(0, NULL, D3DCLEAR_TARGET, 0x00000000, 1.0f, 0);

	hr = m_device9->BeginScene();
	if(SUCCEEDED(hr))
	{
		m_sceneBegun = true;
		return true;
	}

	r3dOutToLog("DX11 Scaleform bridge: BeginScene failed hr=0x%08x\n", hr);
	return false;
}

void r3dDX11ScaleformBridge::EndScaleformRender()
{
	if(!m_device9)
		return;

	if(m_sceneBegun)
	{
		m_device9->EndScene();
		m_sceneBegun = false;
	}

	FlushD3D9();

	if(m_oldSurface9)
		r3dSFBridgeSetRenderTarget(m_device9, 0, m_oldSurface9);

	if(m_oldDepth9)
		r3dSFBridgeSetDepthStencilSurface(m_device9, m_oldDepth9);
	else
		r3dSFBridgeSetDepthStencilSurface(m_device9, NULL);

	SAFE_RELEASE(m_oldSurface9);
	SAFE_RELEASE(m_oldDepth9);
}

void r3dDX11ScaleformBridge::FlushD3D9()
{
	if(!m_flushQuery9)
	{
		return;
	}

	HRESULT hr = m_flushQuery9->Issue(D3DISSUE_END);
	if(FAILED(hr))
		return;

	for(int i = 0; i < 10000; ++i)
	{
		hr = m_flushQuery9->GetData(NULL, 0, D3DGETDATA_FLUSH);

		if(hr == S_OK)
			return;

		if(hr != S_FALSE)
			return;

		Sleep(0);
	}
}

void r3dDX11ScaleformBridge::DrawDX11()
{
	if(!m_context11 || !m_sharedSRV11 || !m_vs || !m_ps)
		return;

	if(!CopyD3D9TargetToDX11())
		return;

	ID3D11RenderTargetView* oldRTV = NULL;
	ID3D11DepthStencilView* oldDSV = NULL;
	m_context11->OMGetRenderTargets(1, &oldRTV, &oldDSV);

	ID3D11BlendState* oldBlend = NULL;
	FLOAT oldBlendFactor[4] = {0, 0, 0, 0};
	UINT oldSampleMask = 0;
	m_context11->OMGetBlendState(&oldBlend, oldBlendFactor, &oldSampleMask);

	ID3D11DepthStencilState* oldDepth = NULL;
	UINT oldStencilRef = 0;
	m_context11->OMGetDepthStencilState(&oldDepth, &oldStencilRef);

	ID3D11RasterizerState* oldRaster = NULL;
	m_context11->RSGetState(&oldRaster);

	UINT oldViewportCount = D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE;
	D3D11_VIEWPORT oldViewports[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
	m_context11->RSGetViewports(&oldViewportCount, oldViewports);

	ID3D11VertexShader* oldVS = NULL;
	ID3D11PixelShader* oldPS = NULL;
	ID3D11GeometryShader* oldGS = NULL;

	m_context11->VSGetShader(&oldVS, NULL, NULL);
	m_context11->PSGetShader(&oldPS, NULL, NULL);
	m_context11->GSGetShader(&oldGS, NULL, NULL);

	ID3D11ShaderResourceView* oldSRV = NULL;
	ID3D11SamplerState* oldSampler = NULL;

	m_context11->PSGetShaderResources(0, 1, &oldSRV);
	m_context11->PSGetSamplers(0, 1, &oldSampler);

	ID3D11InputLayout* oldLayout = NULL;
	ID3D11Buffer* oldVB = NULL;
	ID3D11Buffer* oldIB = NULL;

	UINT oldStride = 0;
	UINT oldOffset = 0;
	DXGI_FORMAT oldIndexFormat = DXGI_FORMAT_UNKNOWN;
	UINT oldIndexOffset = 0;
	D3D11_PRIMITIVE_TOPOLOGY oldTopology = D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;

	m_context11->IAGetInputLayout(&oldLayout);
	m_context11->IAGetVertexBuffers(0, 1, &oldVB, &oldStride, &oldOffset);
	m_context11->IAGetIndexBuffer(&oldIB, &oldIndexFormat, &oldIndexOffset);
	m_context11->IAGetPrimitiveTopology(&oldTopology);

	D3D11_VIEWPORT vp;
	ZeroMemory(&vp, sizeof(vp));
	vp.TopLeftX = 0.0f;
	vp.TopLeftY = 0.0f;
	vp.Width = (float)m_width;
	vp.Height = (float)m_height;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;

	FLOAT blendFactor[4] = {0, 0, 0, 0};

	m_context11->RSSetViewports(1, &vp);
	m_context11->OMSetBlendState(m_blendState, blendFactor, 0xffffffff);
	m_context11->OMSetDepthStencilState(m_depthState, 0);
	m_context11->RSSetState(m_rasterState);

	ID3D11Buffer* nullVB = NULL;
	UINT nullStride = 0;
	UINT nullOffset = 0;

	m_context11->IASetInputLayout(NULL);
	m_context11->IASetVertexBuffers(0, 1, &nullVB, &nullStride, &nullOffset);
	m_context11->IASetIndexBuffer(NULL, DXGI_FORMAT_UNKNOWN, 0);
	m_context11->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	m_context11->VSSetShader(m_vs, NULL, 0);
	m_context11->GSSetShader(NULL, NULL, 0);
	m_context11->PSSetShader(m_ps, NULL, 0);
	m_context11->PSSetShaderResources(0, 1, &m_sharedSRV11);
	m_context11->PSSetSamplers(0, 1, &m_sampler);

	m_context11->Draw(3, 0);

	ID3D11ShaderResourceView* nullSRV = NULL;
	m_context11->PSSetShaderResources(0, 1, &nullSRV);

	m_context11->IASetInputLayout(oldLayout);
	m_context11->IASetVertexBuffers(0, 1, &oldVB, &oldStride, &oldOffset);
	m_context11->IASetIndexBuffer(oldIB, oldIndexFormat, oldIndexOffset);
	m_context11->IASetPrimitiveTopology(oldTopology);

	m_context11->VSSetShader(oldVS, NULL, 0);
	m_context11->GSSetShader(oldGS, NULL, 0);
	m_context11->PSSetShader(oldPS, NULL, 0);
	m_context11->PSSetShaderResources(0, 1, &oldSRV);
	m_context11->PSSetSamplers(0, 1, &oldSampler);

	if(oldViewportCount > 0)
		m_context11->RSSetViewports(oldViewportCount, oldViewports);

	m_context11->RSSetState(oldRaster);
	m_context11->OMSetDepthStencilState(oldDepth, oldStencilRef);
	m_context11->OMSetBlendState(oldBlend, oldBlendFactor, oldSampleMask);
	m_context11->OMSetRenderTargets(1, &oldRTV, oldDSV);

	SAFE_RELEASE(oldRTV);
	SAFE_RELEASE(oldDSV);
	SAFE_RELEASE(oldBlend);
	SAFE_RELEASE(oldDepth);
	SAFE_RELEASE(oldRaster);
	SAFE_RELEASE(oldVS);
	SAFE_RELEASE(oldPS);
	SAFE_RELEASE(oldGS);
	SAFE_RELEASE(oldSRV);
	SAFE_RELEASE(oldSampler);
	SAFE_RELEASE(oldLayout);
	SAFE_RELEASE(oldVB);
	SAFE_RELEASE(oldIB);

	g_r3dDX11State.InvalidateCache();
	g_r3dDX11Geometry.InvalidateCache();
}

bool r3dDX11ScaleformBridge::IsReady() const
{
	return m_device9 && m_sharedTexture9 && m_sharedSurface9 && m_readbackSurface9 && m_sharedSRV11;
}

IDirect3DDevice9Ex* r3dDX11ScaleformBridge::GetD3D9Device() const
{
	return m_device9;
}

IDirect3DTexture9* r3dDX11ScaleformBridge::GetD3D9Texture() const
{
	return m_sharedTexture9;
}

ID3D11ShaderResourceView* r3dDX11ScaleformBridge::GetDX11SRV() const
{
	return m_sharedSRV11;
}

IDirect3DDevice9Ex* r3dGetScaleformD3D9Device()
{
	if(g_r3dDX11ScaleformBridge.IsReady())
		return g_r3dDX11ScaleformBridge.GetD3D9Device();

	return NULL;
}
