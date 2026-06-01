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
#pragma warning(disable: 4625)
#pragma warning(disable: 4626)
#pragma warning(disable: 4668)
#pragma warning(disable: 4710)
#pragma warning(disable: 4711)
#pragma warning(disable: 4820)
#include <dxgi.h>
#include <d3d11.h>
#pragma warning(pop)

#include "r3d.h"
#include "r3dDX11.h"
#include "r3dDX11State.h"
#include "r3dTex.h"

#ifdef D3DRS_CULLMODE
#undef D3DRS_CULLMODE
#endif

template<typename T>
static void r3dDX11State_Release(T*& ptr)
{
	if(ptr)
	{
		ptr->Release();
		ptr = NULL;
	}
}

static void r3dDX11State_LogHR(const char* text, HRESULT hr)
{
	r3dOutToLog("DX11State: %s failed, HRESULT=0x%08X\n", text, (unsigned int)hr);
}

static D3D11_BLEND r3dDX11State_ToBlend(int blend)
{
	switch(blend)
	{
	case D3DBLEND_ZERO:
		return D3D11_BLEND_ZERO;
	case D3DBLEND_ONE:
		return D3D11_BLEND_ONE;
	case D3DBLEND_SRCCOLOR:
		return D3D11_BLEND_SRC_COLOR;
	case D3DBLEND_INVSRCCOLOR:
		return D3D11_BLEND_INV_SRC_COLOR;
	case D3DBLEND_SRCALPHA:
		return D3D11_BLEND_SRC_ALPHA;
	case D3DBLEND_INVSRCALPHA:
		return D3D11_BLEND_INV_SRC_ALPHA;
	case D3DBLEND_DESTALPHA:
		return D3D11_BLEND_DEST_ALPHA;
	case D3DBLEND_INVDESTALPHA:
		return D3D11_BLEND_INV_DEST_ALPHA;
	case D3DBLEND_DESTCOLOR:
		return D3D11_BLEND_DEST_COLOR;
	case D3DBLEND_INVDESTCOLOR:
		return D3D11_BLEND_INV_DEST_COLOR;
	case D3DBLEND_SRCALPHASAT:
		return D3D11_BLEND_SRC_ALPHA_SAT;
	default:
		r3dOutToLog("DX11State: unsupported D3DBLEND %d, using ONE\n", blend);
		return D3D11_BLEND_ONE;
	}
}

static D3D11_BLEND_OP r3dDX11State_ToBlendOp(int op)
{
	switch(op)
	{
	case D3DBLENDOP_ADD:
		return D3D11_BLEND_OP_ADD;
	case D3DBLENDOP_SUBTRACT:
		return D3D11_BLEND_OP_SUBTRACT;
	case D3DBLENDOP_REVSUBTRACT:
		return D3D11_BLEND_OP_REV_SUBTRACT;
	case D3DBLENDOP_MIN:
		return D3D11_BLEND_OP_MIN;
	case D3DBLENDOP_MAX:
		return D3D11_BLEND_OP_MAX;
	default:
		r3dOutToLog("DX11State: unsupported D3DBLENDOP %d, using ADD\n", op);
		return D3D11_BLEND_OP_ADD;
	}
}

static D3D11_COMPARISON_FUNC r3dDX11State_ToComparisonFunc(int func)
{
	switch(func)
	{
	case D3DCMP_NEVER:
		return D3D11_COMPARISON_NEVER;
	case D3DCMP_LESS:
		return D3D11_COMPARISON_LESS;
	case D3DCMP_EQUAL:
		return D3D11_COMPARISON_EQUAL;
	case D3DCMP_LESSEQUAL:
		return D3D11_COMPARISON_LESS_EQUAL;
	case D3DCMP_GREATER:
		return D3D11_COMPARISON_GREATER;
	case D3DCMP_NOTEQUAL:
		return D3D11_COMPARISON_NOT_EQUAL;
	case D3DCMP_GREATEREQUAL:
		return D3D11_COMPARISON_GREATER_EQUAL;
	case D3DCMP_ALWAYS:
		return D3D11_COMPARISON_ALWAYS;
	default:
		r3dOutToLog("DX11State: unsupported D3DCMPFUNC %d, using LESS_EQUAL\n", func);
		return D3D11_COMPARISON_LESS_EQUAL;
	}
}

static D3D11_CULL_MODE r3dDX11State_ToCullMode(int cullMode)
{
	switch(cullMode)
	{
	case D3DCULL_NONE:
		return D3D11_CULL_NONE;
	case D3DCULL_CW:
		return D3D11_CULL_FRONT;
	case D3DCULL_CCW:
		return D3D11_CULL_BACK;
	default:
		r3dOutToLog("DX11State: unsupported D3DCULL %d, using BACK\n", cullMode);
		return D3D11_CULL_BACK;
	}
}

static D3D11_FILL_MODE r3dDX11State_ToFillMode(int fillMode)
{
	switch(fillMode)
	{
	case D3DFILL_WIREFRAME:
		return D3D11_FILL_WIREFRAME;
	case D3DFILL_SOLID:
		return D3D11_FILL_SOLID;
	default:
		r3dOutToLog("DX11State: unsupported D3DFILLMODE %d, using SOLID\n", fillMode);
		return D3D11_FILL_SOLID;
	}
}

static unsigned char r3dDX11State_ToColorWriteMask(unsigned int mask)
{
	unsigned char result = 0;

	if(mask & D3DCOLORWRITEENABLE_RED)
		result |= D3D11_COLOR_WRITE_ENABLE_RED;
	if(mask & D3DCOLORWRITEENABLE_GREEN)
		result |= D3D11_COLOR_WRITE_ENABLE_GREEN;
	if(mask & D3DCOLORWRITEENABLE_BLUE)
		result |= D3D11_COLOR_WRITE_ENABLE_BLUE;
	if(mask & D3DCOLORWRITEENABLE_ALPHA)
		result |= D3D11_COLOR_WRITE_ENABLE_ALPHA;

	return result;
}

r3dDX11State g_r3dDX11State;

r3dDX11State::r3dDX11State()
{
	Initialized = false;

	for(int i = 0; i < 16; ++i)
	{
		BoundSRV[i] = NULL;
		BoundSampler[i] = NULL;
	}

	for(int i = 0; i < R3D_DX11_SAMPLER_MAX; ++i)
	{
		Samplers[i] = NULL;
	}

	DepthState = NULL;
	BlendState = NULL;
	RasterizerState = NULL;

	ResetRenderStateCache();
}

r3dDX11State::~r3dDX11State()
{
	Shutdown();
}

bool r3dDX11State::Init()
{
	Shutdown();

	if(!g_r3dDX11.IsInitialized())
	{
		r3dOutToLog("DX11State: Init failed, DX11 renderer is not initialized\n");
		return false;
	}

	if(!CreateSamplers())
	{
		Shutdown();
		return false;
	}

	InvalidateCache();

	Initialized = true;
	ResetRenderStateCache();

	ApplyDepthState();
	ApplyBlendState();
	ApplyRasterizerState();

	r3dOutToLog("DX11State: initialized\n");

	return true;
}

void r3dDX11State::Shutdown()
{
	if(g_r3dDX11.IsInitialized())
	{
		ClearTextures();
	}

	ReleaseSamplers();
	r3dDX11State_Release(RasterizerState);
	r3dDX11State_Release(BlendState);
	r3dDX11State_Release(DepthState);

	Initialized = false;
	ResetRenderStateCache();

	InvalidateCache();
}

bool r3dDX11State::IsInitialized() const
{
	return Initialized;
}

bool r3dDX11State::CreateSamplers()
{
	ID3D11Device* device = g_r3dDX11.GetDevice();

	if(!device)
	{
		r3dOutToLog("DX11State: CreateSamplers failed, device is NULL\n");
		return false;
	}

	D3D11_SAMPLER_DESC desc;
	ZeroMemory(&desc, sizeof(desc));

	desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	desc.MipLODBias = 0.0f;
	desc.MaxAnisotropy = 1;
	desc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	desc.BorderColor[0] = 0.0f;
	desc.BorderColor[1] = 0.0f;
	desc.BorderColor[2] = 0.0f;
	desc.BorderColor[3] = 0.0f;
	desc.MinLOD = 0.0f;
	desc.MaxLOD = D3D11_FLOAT32_MAX;

	HRESULT hr = device->CreateSamplerState(&desc, &Samplers[R3D_DX11_SAMPLER_LINEAR_WRAP]);

	if(FAILED(hr))
	{
		r3dDX11State_LogHR("CreateSamplerState LINEAR_WRAP", hr);
		return false;
	}

	desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;

	hr = device->CreateSamplerState(&desc, &Samplers[R3D_DX11_SAMPLER_LINEAR_CLAMP]);

	if(FAILED(hr))
	{
		r3dDX11State_LogHR("CreateSamplerState LINEAR_CLAMP", hr);
		return false;
	}

	desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;

	hr = device->CreateSamplerState(&desc, &Samplers[R3D_DX11_SAMPLER_POINT_WRAP]);

	if(FAILED(hr))
	{
		r3dDX11State_LogHR("CreateSamplerState POINT_WRAP", hr);
		return false;
	}

	desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;

	hr = device->CreateSamplerState(&desc, &Samplers[R3D_DX11_SAMPLER_POINT_CLAMP]);

	if(FAILED(hr))
	{
		r3dDX11State_LogHR("CreateSamplerState POINT_CLAMP", hr);
		return false;
	}

	return true;
}

void r3dDX11State::ReleaseSamplers()
{
	for(int i = 0; i < R3D_DX11_SAMPLER_MAX; ++i)
	{
		r3dDX11State_Release(Samplers[i]);
	}
}

bool r3dDX11State::BindTexture(int slot, r3dTexture* texture)
{
	if(slot < 0 || slot >= 16)
		return false;

	ID3D11ShaderResourceView* srv = NULL;

	if(texture)
	{
		srv = texture->GetDX11SRV();

		if(!srv)
		{
			r3dOutToLog(
				"DX11State: texture slot %d has no DX11 SRV, texture='%s'\n",
				slot,
				texture->getFileLoc().FileName
			);
		}
	}

	return SetSRV(slot, srv);
}

bool r3dDX11State::SetSRV(int slot, ID3D11ShaderResourceView* srv)
{
	if(!Initialized)
		return false;

	if(slot < 0 || slot >= 16)
		return false;

	ID3D11DeviceContext* context = g_r3dDX11.GetContext();

	if(!context)
		return false;

	if(BoundSRV[slot] == srv)
		return true;

	context->PSSetShaderResources(slot, 1, &srv);

	BoundSRV[slot] = srv;

	return true;
}

void r3dDX11State::ClearTexture(int slot)
{
	if(slot < 0 || slot >= 16)
		return;

	ID3D11ShaderResourceView* nullSRV = NULL;

	SetSRV(slot, nullSRV);
}

void r3dDX11State::ClearTextures()
{
	if(!g_r3dDX11.IsInitialized())
		return;

	ID3D11DeviceContext* context = g_r3dDX11.GetContext();

	if(!context)
		return;

	ID3D11ShaderResourceView* nullSRV[16];

	for(int i = 0; i < 16; ++i)
	{
		nullSRV[i] = NULL;
		BoundSRV[i] = NULL;
	}

	context->PSSetShaderResources(0, 16, nullSRV);
}

bool r3dDX11State::SetSampler(int slot, r3dDX11SamplerMode mode)
{
	if(!Initialized)
		return false;

	if(slot < 0 || slot >= 16)
		return false;

	if(mode < 0 || mode >= R3D_DX11_SAMPLER_MAX)
		return false;

	ID3D11SamplerState* sampler = Samplers[mode];

	if(!sampler)
		return false;

	ID3D11DeviceContext* context = g_r3dDX11.GetContext();

	if(!context)
		return false;

	if(BoundSampler[slot] == sampler)
		return true;

	context->PSSetSamplers(slot, 1, &sampler);

	BoundSampler[slot] = sampler;

	return true;
}

void r3dDX11State::ApplyDefaultSamplers()
{
	for(int i = 0; i < 16; ++i)
	{
		SetSampler(i, R3D_DX11_SAMPLER_LINEAR_WRAP);
	}
}

void r3dDX11State::ResetRenderStateCache()
{
	DepthEnable = 1;
	DepthWriteEnable = 1;
	DepthFunc = D3DCMP_LESSEQUAL;
	StencilEnable = 0;
	AlphaBlendEnable = 0;
	SrcBlend = D3DBLEND_ONE;
	DestBlend = D3DBLEND_ZERO;
	BlendOp = D3DBLENDOP_ADD;
	CullMode = D3DCULL_CCW;
	FillMode = D3DFILL_SOLID;
	ScissorEnable = 0;

	for(int i = 0; i < 4; ++i)
	{
		ColorWriteMask[i] =
			D3DCOLORWRITEENABLE_RED |
			D3DCOLORWRITEENABLE_GREEN |
			D3DCOLORWRITEENABLE_BLUE |
			D3DCOLORWRITEENABLE_ALPHA;
	}
}

bool r3dDX11State::ApplyDepthState()
{
	if(!Initialized)
		return false;

	ID3D11Device* device = g_r3dDX11.GetDevice();
	ID3D11DeviceContext* context = g_r3dDX11.GetContext();

	if(!device || !context)
		return false;

	D3D11_DEPTH_STENCIL_DESC desc;
	ZeroMemory(&desc, sizeof(desc));

	desc.DepthEnable = DepthEnable ? TRUE : FALSE;
	desc.DepthWriteMask = DepthWriteEnable ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
	desc.DepthFunc = r3dDX11State_ToComparisonFunc(DepthFunc);
	desc.StencilEnable = StencilEnable ? TRUE : FALSE;
	desc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
	desc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
	desc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	desc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	desc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	desc.BackFace = desc.FrontFace;

	ID3D11DepthStencilState* newState = NULL;
	HRESULT hr = device->CreateDepthStencilState(&desc, &newState);

	if(FAILED(hr))
	{
		r3dDX11State_LogHR("CreateDepthStencilState", hr);
		return false;
	}

	context->OMSetDepthStencilState(newState, 0);
	r3dDX11State_Release(DepthState);
	DepthState = newState;

	return true;
}

bool r3dDX11State::ApplyBlendState()
{
	if(!Initialized)
		return false;

	ID3D11Device* device = g_r3dDX11.GetDevice();
	ID3D11DeviceContext* context = g_r3dDX11.GetContext();

	if(!device || !context)
		return false;

	D3D11_BLEND_DESC desc;
	ZeroMemory(&desc, sizeof(desc));

	desc.AlphaToCoverageEnable = FALSE;
	desc.IndependentBlendEnable = TRUE;

	for(int i = 0; i < 4; ++i)
	{
		desc.RenderTarget[i].BlendEnable = AlphaBlendEnable ? TRUE : FALSE;
		desc.RenderTarget[i].SrcBlend = r3dDX11State_ToBlend(SrcBlend);
		desc.RenderTarget[i].DestBlend = r3dDX11State_ToBlend(DestBlend);
		desc.RenderTarget[i].BlendOp = r3dDX11State_ToBlendOp(BlendOp);
		desc.RenderTarget[i].SrcBlendAlpha = r3dDX11State_ToBlend(SrcBlend);
		desc.RenderTarget[i].DestBlendAlpha = r3dDX11State_ToBlend(DestBlend);
		desc.RenderTarget[i].BlendOpAlpha = r3dDX11State_ToBlendOp(BlendOp);
		desc.RenderTarget[i].RenderTargetWriteMask = r3dDX11State_ToColorWriteMask(ColorWriteMask[i]);
	}

	ID3D11BlendState* newState = NULL;
	HRESULT hr = device->CreateBlendState(&desc, &newState);

	if(FAILED(hr))
	{
		r3dDX11State_LogHR("CreateBlendState", hr);
		return false;
	}

	float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	context->OMSetBlendState(newState, blendFactor, 0xffffffff);
	r3dDX11State_Release(BlendState);
	BlendState = newState;

	return true;
}

bool r3dDX11State::ApplyRasterizerState()
{
	if(!Initialized)
		return false;

	ID3D11Device* device = g_r3dDX11.GetDevice();
	ID3D11DeviceContext* context = g_r3dDX11.GetContext();

	if(!device || !context)
		return false;

	D3D11_RASTERIZER_DESC desc;
	ZeroMemory(&desc, sizeof(desc));

	desc.FillMode = r3dDX11State_ToFillMode(FillMode);
	desc.CullMode = r3dDX11State_ToCullMode(CullMode);
	desc.FrontCounterClockwise = FALSE;
	desc.DepthBias = 0;
	desc.DepthBiasClamp = 0.0f;
	desc.SlopeScaledDepthBias = 0.0f;
	desc.DepthClipEnable = TRUE;
	desc.ScissorEnable = ScissorEnable ? TRUE : FALSE;
	desc.MultisampleEnable = FALSE;
	desc.AntialiasedLineEnable = FALSE;

	ID3D11RasterizerState* newState = NULL;
	HRESULT hr = device->CreateRasterizerState(&desc, &newState);

	if(FAILED(hr))
	{
		r3dDX11State_LogHR("CreateRasterizerState", hr);
		return false;
	}

	context->RSSetState(newState);
	r3dDX11State_Release(RasterizerState);
	RasterizerState = newState;

	return true;
}

bool r3dDX11State::SetRenderState(int state, unsigned int value)
{
	if(!Initialized)
		return false;

	switch(state)
	{
	case D3DRS_ZENABLE:
		DepthEnable = value ? 1 : 0;
		return ApplyDepthState();

	case D3DRS_ZWRITEENABLE:
		DepthWriteEnable = value ? 1 : 0;
		return ApplyDepthState();

	case D3DRS_ZFUNC:
		DepthFunc = (int)value;
		return ApplyDepthState();

	case D3DRS_STENCILENABLE:
		StencilEnable = value ? 1 : 0;
		return ApplyDepthState();

	case D3DRS_ALPHABLENDENABLE:
		AlphaBlendEnable = value ? 1 : 0;
		return ApplyBlendState();

	case D3DRS_SRCBLEND:
		SrcBlend = (int)value;
		return ApplyBlendState();

	case D3DRS_DESTBLEND:
		DestBlend = (int)value;
		return ApplyBlendState();

	case D3DRS_BLENDOP:
		BlendOp = (int)value;
		return ApplyBlendState();

	case D3DRS_COLORWRITEENABLE:
		ColorWriteMask[0] = value;
		return ApplyBlendState();

	case D3DRS_COLORWRITEENABLE1:
		ColorWriteMask[1] = value;
		return ApplyBlendState();

	case D3DRS_COLORWRITEENABLE2:
		ColorWriteMask[2] = value;
		return ApplyBlendState();

	case D3DRS_COLORWRITEENABLE3:
		ColorWriteMask[3] = value;
		return ApplyBlendState();

	case D3DRS_CULLMODE:
		CullMode = (int)value;
		return ApplyRasterizerState();

	case D3DRS_FILLMODE:
		FillMode = (int)value;
		return ApplyRasterizerState();

	case D3DRS_SCISSORTESTENABLE:
		ScissorEnable = value ? 1 : 0;
		return ApplyRasterizerState();

	default:
		return false;
	}
}

void r3dDX11State::InvalidateCache()
{
	for(int i = 0; i < 16; ++i)
	{
		BoundSRV[i] = NULL;
		BoundSampler[i] = NULL;
	}
}

#endif
