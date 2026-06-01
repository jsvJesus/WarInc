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
#pragma warning(disable: 4774)
#pragma warning(disable: 4820)
//#pragma warning(disable: 5039)
#include <dxgi.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#pragma warning(pop)

#include "r3d.h"
#include "r3dDX11.h"
#include "r3dDX11Texture.h"
#include "r3dDX11State.h"
#include "r3dDX11ConstantBuffer.h"
#include "r3dDX11RenderTarget.h"
#include "r3dDX11InputLayout.h"
#include "r3dDX11Geometry.h"
#include "r3dDX11LegacyGeometryBridge.h"

#ifdef Draw
#undef Draw
#endif

#ifdef DrawIndexed
#undef DrawIndexed
#endif

#ifdef DrawPrimitive
#undef DrawPrimitive
#endif

#ifdef DrawIndexedPrimitive
#undef DrawIndexedPrimitive
#endif

#ifdef SetPrimitiveTopology
#undef SetPrimitiveTopology
#endif

#ifdef IASetPrimitiveTopology
#undef IASetPrimitiveTopology
#endif

#ifdef IASetVertexBuffers
#undef IASetVertexBuffers
#endif

#ifdef IASetIndexBuffer
#undef IASetIndexBuffer
#endif

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

template<typename T>
static void r3dDX11_Release(T*& ptr)
{
	if(ptr)
	{
		ptr->Release();
		ptr = NULL;
	}
}

static void r3dDX11_LogHR(const char* text, HRESULT hr)
{
	r3dOutToLog("DX11: %s failed, HRESULT=0x%08X\n", text, (unsigned int)hr);
}

static void r3dDX11_LogBlob(ID3DBlob* blob)
{
	if(blob && blob->GetBufferPointer())
	{
		r3dOutToLog("DX11: compiler: %s\n", (const char*)blob->GetBufferPointer());
	}
}

r3dDX11Renderer g_r3dDX11;

r3dDX11Renderer::r3dDX11Renderer()
{
	HWnd = NULL;
	Width = 0;
	Height = 0;
	Windowed = true;
	VSync = true;
	DebugTriangleEnabled = false;
	DebugTexturedQuadEnabled = false;

	FeatureLevel = 0;

	Device = NULL;
	Context = NULL;
	SwapChain = NULL;

	BackBufferRTV = NULL;
	DepthStencilTexture = NULL;
	DepthStencilView = NULL;

	DebugVS = NULL;
	DebugPS = NULL;
	DebugInputLayout = NULL;
	DebugVB = NULL;
	DebugTexVS = NULL;
	DebugTexPS = NULL;
	DebugTexInputLayout = NULL;
	DebugTexVB = NULL;
	DebugTexSampler = NULL;
	DebugTexture = NULL;
}

r3dDX11Renderer::~r3dDX11Renderer()
{
	Shutdown();
}

bool r3dDX11Renderer::Init(HWND hwnd, int width, int height, bool windowed)
{
	Shutdown();

	if(!hwnd)
	{
		r3dOutToLog("DX11: Init failed, HWND is NULL\n");
		return false;
	}

	if(width <= 0)
		width = 1280;

	if(height <= 0)
		height = 720;

	HWnd = hwnd;
	Width = width;
	Height = height;
	Windowed = windowed;

	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));

	sd.BufferCount = 2;
	sd.BufferDesc.Width = Width;
	sd.BufferDesc.Height = Height;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 0;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = HWnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = Windowed ? TRUE : FALSE;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	D3D_FEATURE_LEVEL levels[] =
	{
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0
	};

	D3D_FEATURE_LEVEL createdFeatureLevel = D3D_FEATURE_LEVEL_10_0;

	UINT createFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

#if defined(_DEBUG)
	createFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	HRESULT hr = D3D11CreateDeviceAndSwapChain(
		NULL,
		D3D_DRIVER_TYPE_HARDWARE,
		NULL,
		createFlags,
		levels,
		sizeof(levels) / sizeof(levels[0]),
		D3D11_SDK_VERSION,
		&sd,
		&SwapChain,
		&Device,
		&createdFeatureLevel,
		&Context
	);

#if defined(_DEBUG)
	if(FAILED(hr))
	{
		createFlags &= ~D3D11_CREATE_DEVICE_DEBUG;

		hr = D3D11CreateDeviceAndSwapChain(
			NULL,
			D3D_DRIVER_TYPE_HARDWARE,
			NULL,
			createFlags,
			levels,
			sizeof(levels) / sizeof(levels[0]),
			D3D11_SDK_VERSION,
			&sd,
			&SwapChain,
			&Device,
			&createdFeatureLevel,
			&Context
		);
	}
#endif

	if(FAILED(hr))
	{
		r3dDX11_LogHR("D3D11CreateDeviceAndSwapChain", hr);
		Shutdown();
		return false;
	}

	FeatureLevel = (R3D_DX11_FEATURE_LEVEL)createdFeatureLevel;

	if(!CreateBackBuffer())
	{
		Shutdown();
		return false;
	}

	if(!g_r3dDX11RenderTargets.Init())
	{
		r3dOutToLog("DX11: render target bridge was not initialized\n");
	}

	if(!g_r3dDX11State.Init())
	{
		r3dOutToLog("DX11: state manager was not initialized\n");
	}
	else
	{
		g_r3dDX11State.ApplyDefaultSamplers();
	}

	if(!g_r3dDX11Geometry.Init())
	{
		r3dOutToLog("DX11: geometry state manager was not initialized\n");
	}

	if(!g_r3dDX11LegacyGeometryBridge.Init())
	{
		r3dOutToLog("DX11: legacy geometry bridge was not initialized\n");
	}

	if(!g_r3dDX11Constants.Init())
	{
		r3dOutToLog("DX11: constant buffer bridge was not initialized\n");
	}

	if(!CreateDebugTriangle())
	{
		r3dOutToLog("DX11: debug triangle resources were not created\n");
	}

	if(!CreateDebugTexturedQuad())
	{
		r3dOutToLog("DX11: debug textured quad resources were not created\n");
	}

	r3dOutToLog(
		"DX11: initialized %dx%d windowed=%d featureLevel=0x%X\n",
		Width,
		Height,
		Windowed ? 1 : 0,
		(unsigned int)FeatureLevel
	);

	return true;
}

void r3dDX11Renderer::Shutdown()
{
	if(Context)
	{
		Context->ClearState();
		Context->Flush();
	}

	if(SwapChain)
	{
		SwapChain->SetFullscreenState(FALSE, NULL);
	}

	ReleaseDebugTexturedQuad();
	ReleaseDebugTriangle();

	g_r3dDX11LegacyGeometryBridge.Shutdown();
	g_r3dDX11Geometry.Shutdown();
	g_r3dDX11InputLayouts.Shutdown();
	g_r3dDX11Constants.Shutdown();
	g_r3dDX11State.Shutdown();
	g_r3dDX11RenderTargets.Shutdown();

	ReleaseBackBuffer();

	r3dDX11_Release(SwapChain);
	r3dDX11_Release(Context);
	r3dDX11_Release(Device);

	HWnd = NULL;
	Width = 0;
	Height = 0;
	Windowed = true;
	FeatureLevel = 0;
}

bool r3dDX11Renderer::CreateBackBuffer()
{
	if(!Device || !Context || !SwapChain)
		return false;

	ID3D11Texture2D* backBuffer = NULL;

	HRESULT hr = SwapChain->GetBuffer(
		0,
		__uuidof(ID3D11Texture2D),
		(void**)&backBuffer
	);

	if(FAILED(hr))
	{
		r3dDX11_LogHR("SwapChain->GetBuffer", hr);
		return false;
	}

	hr = Device->CreateRenderTargetView(backBuffer, NULL, &BackBufferRTV);
	r3dDX11_Release(backBuffer);

	if(FAILED(hr))
	{
		r3dDX11_LogHR("CreateRenderTargetView", hr);
		return false;
	}

	D3D11_TEXTURE2D_DESC depthDesc;
	ZeroMemory(&depthDesc, sizeof(depthDesc));

	depthDesc.Width = Width;
	depthDesc.Height = Height;
	depthDesc.MipLevels = 1;
	depthDesc.ArraySize = 1;
	depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthDesc.SampleDesc.Count = 1;
	depthDesc.SampleDesc.Quality = 0;
	depthDesc.Usage = D3D11_USAGE_DEFAULT;
	depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthDesc.CPUAccessFlags = 0;
	depthDesc.MiscFlags = 0;

	hr = Device->CreateTexture2D(&depthDesc, NULL, &DepthStencilTexture);
	if(FAILED(hr))
	{
		r3dDX11_LogHR("CreateTexture2D DepthStencil", hr);
		return false;
	}

	hr = Device->CreateDepthStencilView(DepthStencilTexture, NULL, &DepthStencilView);
	if(FAILED(hr))
	{
		r3dDX11_LogHR("CreateDepthStencilView", hr);
		return false;
	}

	Context->OMSetRenderTargets(1, &BackBufferRTV, DepthStencilView);

	D3D11_VIEWPORT vp;
	ZeroMemory(&vp, sizeof(vp));

	vp.TopLeftX = 0.0f;
	vp.TopLeftY = 0.0f;
	vp.Width = (float)Width;
	vp.Height = (float)Height;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;

	Context->RSSetViewports(1, &vp);

	return true;
}

void r3dDX11Renderer::ReleaseBackBuffer()
{
	r3dDX11_Release(DepthStencilView);
	r3dDX11_Release(DepthStencilTexture);
	r3dDX11_Release(BackBufferRTV);
}

bool r3dDX11Renderer::Resize(int width, int height)
{
	if(!IsInitialized())
		return false;

	if(width <= 0 || height <= 0)
		return true;

	if(width == Width && height == Height && BackBufferRTV && DepthStencilView)
		return true;

	Width = width;
	Height = height;

	g_r3dDX11RenderTargets.ClearCurrentTargets();
	Context->OMSetRenderTargets(0, NULL, NULL);
	ReleaseBackBuffer();

	HRESULT hr = SwapChain->ResizeBuffers(
		0,
		Width,
		Height,
		DXGI_FORMAT_UNKNOWN,
		DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH
	);

	if(FAILED(hr))
	{
		r3dDX11_LogHR("SwapChain->ResizeBuffers", hr);
		return false;
	}

	if(!CreateBackBuffer())
		return false;

	g_r3dDX11RenderTargets.ResetToBackBuffer();
	return true;
}

void r3dDX11Renderer::BeginFrame(float r, float g, float b, float a)
{
	if(!IsInitialized())
		return;

	float clearColor[4];
	clearColor[0] = r;
	clearColor[1] = g;
	clearColor[2] = b;
	clearColor[3] = a;

	Context->OMSetRenderTargets(1, &BackBufferRTV, DepthStencilView);
	Context->ClearRenderTargetView(BackBufferRTV, clearColor);
	Context->ClearDepthStencilView(DepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}

void r3dDX11Renderer::EndFrame(bool present)
{
	if(!IsInitialized())
		return;

	if(present)
	{
		HRESULT hr = SwapChain->Present(VSync ? 1 : 0, 0);
		if(FAILED(hr))
		{
			r3dDX11_LogHR("SwapChain->Present", hr);
		}
	}
}

bool r3dDX11Renderer::CreateDebugTriangle()
{
	if(!Device)
		return false;

	static const char* shaderCode =
		"struct VS_IN"
		"{"
		"	float3 Pos : POSITION;"
		"	float4 Col : COLOR0;"
		"};"

		"struct PS_IN"
		"{"
		"	float4 Pos : SV_POSITION;"
		"	float4 Col : COLOR0;"
		"};"

		"PS_IN VSMain(VS_IN input)"
		"{"
		"	PS_IN output;"
		"	output.Pos = float4(input.Pos, 1.0f);"
		"	output.Col = input.Col;"
		"	return output;"
		"}"

		"float4 PSMain(PS_IN input) : SV_Target"
		"{"
		"	return input.Col;"
		"}";

	ID3DBlob* vsBlob = NULL;
	ID3DBlob* psBlob = NULL;
	ID3DBlob* errors = NULL;

	UINT flags = 0;

#if defined(_DEBUG)
	flags |= D3DCOMPILE_DEBUG;
	flags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	HRESULT hr = D3DCompile(
		shaderCode,
		strlen(shaderCode),
		"r3dDX11DebugVS",
		NULL,
		NULL,
		"VSMain",
		"vs_4_0",
		flags,
		0,
		&vsBlob,
		&errors
	);

	if(FAILED(hr))
	{
		r3dDX11_LogBlob(errors);
		r3dDX11_Release(errors);
		r3dDX11_LogHR("D3DCompile VS", hr);
		return false;
	}

	r3dDX11_Release(errors);

	hr = D3DCompile(
		shaderCode,
		strlen(shaderCode),
		"r3dDX11DebugPS",
		NULL,
		NULL,
		"PSMain",
		"ps_4_0",
		flags,
		0,
		&psBlob,
		&errors
	);

	if(FAILED(hr))
	{
		r3dDX11_LogBlob(errors);
		r3dDX11_Release(errors);
		r3dDX11_Release(vsBlob);
		r3dDX11_LogHR("D3DCompile PS", hr);
		return false;
	}

	r3dDX11_Release(errors);

	hr = Device->CreateVertexShader(
		vsBlob->GetBufferPointer(),
		vsBlob->GetBufferSize(),
		NULL,
		&DebugVS
	);

	if(FAILED(hr))
	{
		r3dDX11_Release(vsBlob);
		r3dDX11_Release(psBlob);
		r3dDX11_LogHR("CreateVertexShader", hr);
		return false;
	}

	hr = Device->CreatePixelShader(
		psBlob->GetBufferPointer(),
		psBlob->GetBufferSize(),
		NULL,
		&DebugPS
	);

	if(FAILED(hr))
	{
		r3dDX11_Release(vsBlob);
		r3dDX11_Release(psBlob);
		r3dDX11_LogHR("CreatePixelShader", hr);
		return false;
	}

	D3D11_INPUT_ELEMENT_DESC layout[2];

	layout[0].SemanticName = "POSITION";
	layout[0].SemanticIndex = 0;
	layout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	layout[0].InputSlot = 0;
	layout[0].AlignedByteOffset = 0;
	layout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	layout[0].InstanceDataStepRate = 0;

	layout[1].SemanticName = "COLOR";
	layout[1].SemanticIndex = 0;
	layout[1].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	layout[1].InputSlot = 0;
	layout[1].AlignedByteOffset = 12;
	layout[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	layout[1].InstanceDataStepRate = 0;

	hr = Device->CreateInputLayout(
		layout,
		2,
		vsBlob->GetBufferPointer(),
		vsBlob->GetBufferSize(),
		&DebugInputLayout
	);

	r3dDX11_Release(vsBlob);
	r3dDX11_Release(psBlob);

	if(FAILED(hr))
	{
		r3dDX11_LogHR("CreateInputLayout", hr);
		return false;
	}

	struct DX11DebugVertex
	{
		float x, y, z;
		float r, g, b, a;
	};

	DX11DebugVertex vertices[3];

	vertices[0].x = 0.0f;
	vertices[0].y = 0.65f;
	vertices[0].z = 0.0f;
	vertices[0].r = 1.0f;
	vertices[0].g = 0.15f;
	vertices[0].b = 0.05f;
	vertices[0].a = 1.0f;

	vertices[1].x = 0.65f;
	vertices[1].y = -0.55f;
	vertices[1].z = 0.0f;
	vertices[1].r = 0.05f;
	vertices[1].g = 0.85f;
	vertices[1].b = 0.15f;
	vertices[1].a = 1.0f;

	vertices[2].x = -0.65f;
	vertices[2].y = -0.55f;
	vertices[2].z = 0.0f;
	vertices[2].r = 0.1f;
	vertices[2].g = 0.25f;
	vertices[2].b = 1.0f;
	vertices[2].a = 1.0f;

	D3D11_BUFFER_DESC vbDesc;
	ZeroMemory(&vbDesc, sizeof(vbDesc));

	vbDesc.ByteWidth = sizeof(vertices);
	vbDesc.Usage = D3D11_USAGE_DEFAULT;
	vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbDesc.CPUAccessFlags = 0;
	vbDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA initData;
	ZeroMemory(&initData, sizeof(initData));

	initData.pSysMem = vertices;

	hr = Device->CreateBuffer(&vbDesc, &initData, &DebugVB);

	if(FAILED(hr))
	{
		r3dDX11_LogHR("CreateBuffer DebugVB", hr);
		return false;
	}

	return true;
}

void r3dDX11Renderer::ReleaseDebugTriangle()
{
	r3dDX11_Release(DebugVB);
	r3dDX11_Release(DebugInputLayout);
	r3dDX11_Release(DebugPS);
	r3dDX11_Release(DebugVS);
}

void r3dDX11Renderer::DrawDebugTriangle()
{
	if(!IsInitialized())
		return;

	if(!DebugTriangleEnabled)
		return;

	if(!DebugVB || !DebugInputLayout || !DebugVS || !DebugPS)
		return;

	Context->IASetInputLayout(DebugInputLayout);

	g_r3dDX11Geometry.SetVertexBufferRaw(0, DebugVB, sizeof(float) * 7, 0);
	g_r3dDX11Geometry.SetPrimitiveTopology(R3D_DX11_PRIM_TRIANGLELIST);

	Context->VSSetShader(DebugVS, NULL, 0);
	Context->PSSetShader(DebugPS, NULL, 0);

	g_r3dDX11Geometry.Draw(3, 0);
}

bool r3dDX11Renderer::CreateDebugTexturedQuad()
{
	if(!Device)
		return false;

	static const char* shaderCode =
		"Texture2D DiffuseTexture : register(t0);"
		"SamplerState DiffuseSampler : register(s0);"

		"struct VS_IN"
		"{"
		"	float3 Pos : POSITION;"
		"	float2 UV  : TEXCOORD0;"
		"};"

		"struct PS_IN"
		"{"
		"	float4 Pos : SV_POSITION;"
		"	float2 UV  : TEXCOORD0;"
		"};"

		"PS_IN VSMain(VS_IN input)"
		"{"
		"	PS_IN output;"
		"	output.Pos = float4(input.Pos, 1.0f);"
		"	output.UV = input.UV;"
		"	return output;"
		"}"

		"float4 PSMain(PS_IN input) : SV_Target"
		"{"
		"	return DiffuseTexture.Sample(DiffuseSampler, input.UV);"
		"}";

	ID3DBlob* vsBlob = NULL;
	ID3DBlob* psBlob = NULL;
	ID3DBlob* errors = NULL;

	UINT flags = 0;

#if defined(_DEBUG)
	flags |= D3DCOMPILE_DEBUG;
	flags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	HRESULT hr = D3DCompile(
		shaderCode,
		strlen(shaderCode),
		"r3dDX11DebugTexVS",
		NULL,
		NULL,
		"VSMain",
		"vs_4_0",
		flags,
		0,
		&vsBlob,
		&errors
	);

	if(FAILED(hr))
	{
		r3dDX11_LogBlob(errors);
		r3dDX11_Release(errors);
		r3dDX11_LogHR("D3DCompile textured VS", hr);
		return false;
	}

	r3dDX11_Release(errors);

	hr = D3DCompile(
		shaderCode,
		strlen(shaderCode),
		"r3dDX11DebugTexPS",
		NULL,
		NULL,
		"PSMain",
		"ps_4_0",
		flags,
		0,
		&psBlob,
		&errors
	);

	if(FAILED(hr))
	{
		r3dDX11_LogBlob(errors);
		r3dDX11_Release(errors);
		r3dDX11_Release(vsBlob);
		r3dDX11_LogHR("D3DCompile textured PS", hr);
		return false;
	}

	r3dDX11_Release(errors);

	hr = Device->CreateVertexShader(
		vsBlob->GetBufferPointer(),
		vsBlob->GetBufferSize(),
		NULL,
		&DebugTexVS
	);

	if(FAILED(hr))
	{
		r3dDX11_Release(vsBlob);
		r3dDX11_Release(psBlob);
		r3dDX11_LogHR("CreateVertexShader textured", hr);
		return false;
	}

	hr = Device->CreatePixelShader(
		psBlob->GetBufferPointer(),
		psBlob->GetBufferSize(),
		NULL,
		&DebugTexPS
	);

	if(FAILED(hr))
	{
		r3dDX11_Release(vsBlob);
		r3dDX11_Release(psBlob);
		r3dDX11_LogHR("CreatePixelShader textured", hr);
		return false;
	}

	D3D11_INPUT_ELEMENT_DESC layout[2];

	layout[0].SemanticName = "POSITION";
	layout[0].SemanticIndex = 0;
	layout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	layout[0].InputSlot = 0;
	layout[0].AlignedByteOffset = 0;
	layout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	layout[0].InstanceDataStepRate = 0;

	layout[1].SemanticName = "TEXCOORD";
	layout[1].SemanticIndex = 0;
	layout[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	layout[1].InputSlot = 0;
	layout[1].AlignedByteOffset = 12;
	layout[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	layout[1].InstanceDataStepRate = 0;

	hr = Device->CreateInputLayout(
		layout,
		2,
		vsBlob->GetBufferPointer(),
		vsBlob->GetBufferSize(),
		&DebugTexInputLayout
	);

	r3dDX11_Release(vsBlob);
	r3dDX11_Release(psBlob);

	if(FAILED(hr))
	{
		r3dDX11_LogHR("CreateInputLayout textured", hr);
		return false;
	}

	struct DX11DebugTexVertex
	{
		float x, y, z;
		float u, v;
	};

	DX11DebugTexVertex vertices[6];

	const float left = -0.85f;
	const float right = -0.25f;
	const float top = 0.85f;
	const float bottom = 0.25f;

	vertices[0].x = left;
	vertices[0].y = top;
	vertices[0].z = 0.0f;
	vertices[0].u = 0.0f;
	vertices[0].v = 0.0f;

	vertices[1].x = right;
	vertices[1].y = top;
	vertices[1].z = 0.0f;
	vertices[1].u = 1.0f;
	vertices[1].v = 0.0f;

	vertices[2].x = left;
	vertices[2].y = bottom;
	vertices[2].z = 0.0f;
	vertices[2].u = 0.0f;
	vertices[2].v = 1.0f;

	vertices[3].x = left;
	vertices[3].y = bottom;
	vertices[3].z = 0.0f;
	vertices[3].u = 0.0f;
	vertices[3].v = 1.0f;

	vertices[4].x = right;
	vertices[4].y = top;
	vertices[4].z = 0.0f;
	vertices[4].u = 1.0f;
	vertices[4].v = 0.0f;

	vertices[5].x = right;
	vertices[5].y = bottom;
	vertices[5].z = 0.0f;
	vertices[5].u = 1.0f;
	vertices[5].v = 1.0f;

	D3D11_BUFFER_DESC vbDesc;
	ZeroMemory(&vbDesc, sizeof(vbDesc));

	vbDesc.ByteWidth = sizeof(vertices);
	vbDesc.Usage = D3D11_USAGE_DEFAULT;
	vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbDesc.CPUAccessFlags = 0;
	vbDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA initData;
	ZeroMemory(&initData, sizeof(initData));

	initData.pSysMem = vertices;

	hr = Device->CreateBuffer(&vbDesc, &initData, &DebugTexVB);

	if(FAILED(hr))
	{
		r3dDX11_LogHR("CreateBuffer textured VB", hr);
		return false;
	}

	D3D11_SAMPLER_DESC samplerDesc;
	ZeroMemory(&samplerDesc, sizeof(samplerDesc));

	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.BorderColor[0] = 0.0f;
	samplerDesc.BorderColor[1] = 0.0f;
	samplerDesc.BorderColor[2] = 0.0f;
	samplerDesc.BorderColor[3] = 0.0f;
	samplerDesc.MinLOD = 0.0f;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	hr = Device->CreateSamplerState(&samplerDesc, &DebugTexSampler);

	if(FAILED(hr))
	{
		r3dDX11_LogHR("CreateSamplerState", hr);
		return false;
	}

	DebugTexture = new r3dDX11Texture();

	if(!DebugTexture->LoadDDSFromFile("Data\\Shaders\\Texture\\Missing.dds"))
	{
		r3dOutToLog("DX11: failed to load Missing.dds, trying MissingTexture.dds\n");

		if(!DebugTexture->LoadDDSFromFile("Data\\Shaders\\Texture\\MissingTexture.dds"))
		{
			r3dOutToLog("DX11: failed to load real DDS debug texture, using checker fallback\n");

			if(!DebugTexture->CreateChecker(256))
			{
				delete DebugTexture;
				DebugTexture = NULL;
				r3dOutToLog("DX11: failed to create debug checker texture\n");
				return false;
			}
		}
	}

	return true;
}

void r3dDX11Renderer::ReleaseDebugTexturedQuad()
{
	if(DebugTexture)
	{
		delete DebugTexture;
		DebugTexture = NULL;
	}

	r3dDX11_Release(DebugTexSampler);
	r3dDX11_Release(DebugTexVB);
	r3dDX11_Release(DebugTexInputLayout);
	r3dDX11_Release(DebugTexPS);
	r3dDX11_Release(DebugTexVS);
}

void r3dDX11Renderer::DrawDebugTexturedQuad()
{
	if(!IsInitialized())
		return;

	if(!DebugTexturedQuadEnabled)
		return;

	if(!DebugTexture || !DebugTexture->IsValid())
		return;

	if(!DebugTexVB || !DebugTexInputLayout || !DebugTexVS || !DebugTexPS || !DebugTexSampler)
		return;

	Context->IASetInputLayout(DebugTexInputLayout);

	g_r3dDX11Geometry.SetVertexBufferRaw(0, DebugTexVB, sizeof(float) * 5, 0);
	g_r3dDX11Geometry.SetPrimitiveTopology(R3D_DX11_PRIM_TRIANGLELIST);

	Context->VSSetShader(DebugTexVS, NULL, 0);
	Context->PSSetShader(DebugTexPS, NULL, 0);

	ID3D11ShaderResourceView* srv = DebugTexture->GetSRV();

	g_r3dDX11State.SetSRV(0, srv);
	g_r3dDX11State.SetSampler(0, R3D_DX11_SAMPLER_LINEAR_WRAP);

	g_r3dDX11Geometry.Draw(6, 0);

	g_r3dDX11State.ClearTexture(0);
}

void r3dDX11Renderer::SetVSync(bool enabled)
{
	VSync = enabled;
}

void r3dDX11Renderer::SetDebugTriangle(bool enabled)
{
	DebugTriangleEnabled = enabled;
}

void r3dDX11Renderer::SetDebugTexturedQuad(bool enabled)
{
	DebugTexturedQuadEnabled = enabled;
}

bool r3dDX11Renderer::IsInitialized() const
{
	return Device && Context && SwapChain;
}

ID3D11Device* r3dDX11Renderer::GetDevice() const
{
	return Device;
}

ID3D11DeviceContext* r3dDX11Renderer::GetContext() const
{
	return Context;
}

IDXGISwapChain* r3dDX11Renderer::GetSwapChain() const
{
	return SwapChain;
}

ID3D11RenderTargetView* r3dDX11Renderer::GetBackBufferRTV() const
{
	return BackBufferRTV;
}

ID3D11DepthStencilView* r3dDX11Renderer::GetDepthStencilView() const
{
	return DepthStencilView;
}

int r3dDX11Renderer::GetWidth() const
{
	return Width;
}

int r3dDX11Renderer::GetHeight() const
{
	return Height;
}

R3D_DX11_FEATURE_LEVEL r3dDX11Renderer::GetFeatureLevel() const
{
	return FeatureLevel;
}

R3D_DX11_FORMAT r3dDX11_ConvertLegacyD3DFormat(int d3dFormat)
{
	switch((D3DFORMAT)d3dFormat)
	{
	case D3DFMT_DXT1:
		return (R3D_DX11_FORMAT)DXGI_FORMAT_BC1_UNORM;

	case D3DFMT_DXT2:
	case D3DFMT_DXT3:
		return (R3D_DX11_FORMAT)DXGI_FORMAT_BC2_UNORM;

	case D3DFMT_DXT4:
	case D3DFMT_DXT5:
		return (R3D_DX11_FORMAT)DXGI_FORMAT_BC3_UNORM;
	case D3DFMT_A8R8G8B8:
		return (R3D_DX11_FORMAT)DXGI_FORMAT_B8G8R8A8_UNORM;

	case D3DFMT_X8R8G8B8:
		return (R3D_DX11_FORMAT)DXGI_FORMAT_B8G8R8X8_UNORM;

	case D3DFMT_R5G6B5:
		return (R3D_DX11_FORMAT)DXGI_FORMAT_B5G6R5_UNORM;

	case D3DFMT_A1R5G5B5:
		return (R3D_DX11_FORMAT)DXGI_FORMAT_B5G5R5A1_UNORM;

	case D3DFMT_A16B16G16R16F:
		return (R3D_DX11_FORMAT)DXGI_FORMAT_R16G16B16A16_FLOAT;

	case D3DFMT_A16B16G16R16:
		return (R3D_DX11_FORMAT)DXGI_FORMAT_R16G16B16A16_UNORM;

	case D3DFMT_G16R16F:
		return (R3D_DX11_FORMAT)DXGI_FORMAT_R16G16_FLOAT;

	case D3DFMT_G16R16:
		return (R3D_DX11_FORMAT)DXGI_FORMAT_R16G16_UNORM;

	case D3DFMT_R32F:
		return (R3D_DX11_FORMAT)DXGI_FORMAT_R32_FLOAT;

	case D3DFMT_R16F:
		return (R3D_DX11_FORMAT)DXGI_FORMAT_R16_FLOAT;

	case D3DFMT_A8:
		return (R3D_DX11_FORMAT)DXGI_FORMAT_A8_UNORM;

	case D3DFMT_L8:
		return (R3D_DX11_FORMAT)DXGI_FORMAT_R8_UNORM;

	case D3DFMT_D16:
		return (R3D_DX11_FORMAT)DXGI_FORMAT_D16_UNORM;

	case D3DFMT_D24S8:
		return (R3D_DX11_FORMAT)DXGI_FORMAT_D24_UNORM_S8_UINT;

	default:
		return (R3D_DX11_FORMAT)DXGI_FORMAT_UNKNOWN;
	}
}

#endif
