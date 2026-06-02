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

static int r3dDX11State_GetVertexSamplerSlot(int slot)
{
	switch(slot)
	{
	case D3DVERTEXTEXTURESAMPLER0:
		return 0;
	case D3DVERTEXTEXTURESAMPLER1:
		return 1;
	case D3DVERTEXTEXTURESAMPLER2:
		return 2;
	case D3DVERTEXTEXTURESAMPLER3:
		return 3;
	default:
		return -1;
	}
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

static D3D11_STENCIL_OP r3dDX11State_ToStencilOp(int op)
{
	switch(op)
	{
	case D3DSTENCILOP_KEEP:
		return D3D11_STENCIL_OP_KEEP;
	case D3DSTENCILOP_ZERO:
		return D3D11_STENCIL_OP_ZERO;
	case D3DSTENCILOP_REPLACE:
		return D3D11_STENCIL_OP_REPLACE;
	case D3DSTENCILOP_INCRSAT:
		return D3D11_STENCIL_OP_INCR_SAT;
	case D3DSTENCILOP_DECRSAT:
		return D3D11_STENCIL_OP_DECR_SAT;
	case D3DSTENCILOP_INVERT:
		return D3D11_STENCIL_OP_INVERT;
	case D3DSTENCILOP_INCR:
		return D3D11_STENCIL_OP_INCR;
	case D3DSTENCILOP_DECR:
		return D3D11_STENCIL_OP_DECR;
	default:
		r3dOutToLog("DX11State: unsupported D3DSTENCILOP %d, using KEEP\n", op);
		return D3D11_STENCIL_OP_KEEP;
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

static D3D11_TEXTURE_ADDRESS_MODE r3dDX11State_ToAddressMode(int addressMode)
{
	switch(addressMode)
	{
	case D3DTADDRESS_WRAP:
		return D3D11_TEXTURE_ADDRESS_WRAP;
	case D3DTADDRESS_MIRROR:
		return D3D11_TEXTURE_ADDRESS_MIRROR;
	case D3DTADDRESS_CLAMP:
		return D3D11_TEXTURE_ADDRESS_CLAMP;
	case D3DTADDRESS_BORDER:
		return D3D11_TEXTURE_ADDRESS_BORDER;
	default:
		r3dOutToLog("DX11State: unsupported D3DTADDRESS %d, using WRAP\n", addressMode);
		return D3D11_TEXTURE_ADDRESS_WRAP;
	}
}

static D3D11_FILTER r3dDX11State_ToFilter(int minFilter, int magFilter, int mipFilter)
{
	if(minFilter == D3DTEXF_ANISOTROPIC || magFilter == D3DTEXF_ANISOTROPIC)
		return D3D11_FILTER_ANISOTROPIC;

	const int minLinear = minFilter == D3DTEXF_LINEAR;
	const int magLinear = magFilter == D3DTEXF_LINEAR;
	const int mipLinear = mipFilter == D3DTEXF_LINEAR;

	return (D3D11_FILTER)(
		(minLinear ? 0x10 : 0) |
		(magLinear ? 0x04 : 0) |
		(mipLinear ? 0x01 : 0)
	);
}

static void r3dDX11State_DecodeBorderColor(unsigned int color, FLOAT outColor[4])
{
	outColor[0] = ((color >> 16) & 0xff) / 255.0f;
	outColor[1] = ((color >> 8) & 0xff) / 255.0f;
	outColor[2] = (color & 0xff) / 255.0f;
	outColor[3] = ((color >> 24) & 0xff) / 255.0f;
}

static bool r3dDX11State_SetIntIfChanged(int& state, int value)
{
	if(state == value)
		return false;

	state = value;
	return true;
}

static bool r3dDX11State_SetUIntIfChanged(unsigned int& state, unsigned int value)
{
	if(state == value)
		return false;

	state = value;
	return true;
}

static bool r3dDX11State_SetFloatIfChanged(float& state, float value)
{
	if(state == value)
		return false;

	state = value;
	return true;
}

static void r3dDX11State_HashBytes(unsigned int& hash, const void* data, size_t size)
{
	const unsigned char* bytes = (const unsigned char*)data;

	for(size_t i = 0; i < size; ++i)
	{
		hash ^= bytes[i];
		hash *= 16777619u;
	}
}

template<typename T>
static void r3dDX11State_HashValue(unsigned int& hash, const T& value)
{
	r3dDX11State_HashBytes(hash, &value, sizeof(value));
}

static unsigned int r3dDX11State_FinishHash(unsigned int hash)
{
	return hash ? hash : 1u;
}

r3dDX11State g_r3dDX11State;

r3dDX11State::r3dDX11State()
{
	Initialized = false;

	for(int i = 0; i < 16; ++i)
	{
		BoundSRV[i] = NULL;
		BoundSampler[i] = NULL;
		CustomSamplers[i] = NULL;
	}

	for(int i = 0; i < 4; ++i)
	{
		BoundVSSRV[i] = NULL;
		BoundVSSampler[i] = NULL;
		CustomVSSamplers[i] = NULL;
	}

	for(int i = 0; i < R3D_DX11_SAMPLER_MAX; ++i)
	{
		Samplers[i] = NULL;
	}

	DepthState = NULL;
	BlendState = NULL;
	RasterizerState = NULL;

	ResetStateObjectCaches();
	ResetSamplerStateCache();
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
	ResetSamplerStateCache();
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
	ReleaseCustomSamplers();

	DepthState = NULL;
	BlendState = NULL;
	RasterizerState = NULL;

	ReleaseStateObjectCaches();

	Initialized = false;
	ResetSamplerStateCache();
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

	if(!g_r3dDX11.CanCreateDeviceResources("DX11State::CreateSamplers"))
		return false;

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

void r3dDX11State::ReleaseCustomSamplers()
{
	for(int i = 0; i < 16; ++i)
	{
		CustomSamplers[i] = NULL;
	}

	for(int i = 0; i < 4; ++i)
	{
		CustomVSSamplers[i] = NULL;
	}
}

void r3dDX11State::ResetStateObjectCaches()
{
	DepthStateCacheCount = 0;
	BlendStateCacheCount = 0;
	RasterizerStateCacheCount = 0;
	SamplerStateCacheCount = 0;

	ZeroMemory(DepthStateCache, sizeof(DepthStateCache));
	ZeroMemory(BlendStateCache, sizeof(BlendStateCache));
	ZeroMemory(RasterizerStateCache, sizeof(RasterizerStateCache));
	ZeroMemory(SamplerStateCache, sizeof(SamplerStateCache));
}

void r3dDX11State::ReleaseStateObjectCaches()
{
	for(int i = 0; i < DepthStateCacheCount; ++i)
	{
		r3dDX11State_Release(DepthStateCache[i].State);
	}

	for(int i = 0; i < BlendStateCacheCount; ++i)
	{
		r3dDX11State_Release(BlendStateCache[i].State);
	}

	for(int i = 0; i < RasterizerStateCacheCount; ++i)
	{
		r3dDX11State_Release(RasterizerStateCache[i].State);
	}

	for(int i = 0; i < SamplerStateCacheCount; ++i)
	{
		r3dDX11State_Release(SamplerStateCache[i].State);
	}

	ResetStateObjectCaches();
}

bool r3dDX11State::BindTexture(int slot, r3dTexture* texture)
{
	if((slot < 0 || slot >= 16) && r3dDX11State_GetVertexSamplerSlot(slot) < 0)
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

	if(g_r3dDX11.IsDeviceLost())
		return false;

	ID3D11DeviceContext* context = g_r3dDX11.GetContext();

	if(!context)
		return false;

	const int vsSlot = r3dDX11State_GetVertexSamplerSlot(slot);

	if(vsSlot >= 0)
	{
		if(BoundVSSRV[vsSlot] == srv)
			return true;

		context->VSSetShaderResources(vsSlot, 1, &srv);
		BoundVSSRV[vsSlot] = srv;

		return true;
	}

	if(slot < 0 || slot >= 16)
		return false;

	if(BoundSRV[slot] == srv)
		return true;

	context->PSSetShaderResources(slot, 1, &srv);

	BoundSRV[slot] = srv;

	return true;
}

void r3dDX11State::ClearTexture(int slot)
{
	if((slot < 0 || slot >= 16) && r3dDX11State_GetVertexSamplerSlot(slot) < 0)
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

	ID3D11ShaderResourceView* nullVSSRV[4];

	for(int i = 0; i < 4; ++i)
	{
		nullVSSRV[i] = NULL;
		BoundVSSRV[i] = NULL;
	}

	context->VSSetShaderResources(0, 4, nullVSSRV);
}

bool r3dDX11State::SetSampler(int slot, r3dDX11SamplerMode mode)
{
	if(!Initialized)
		return false;

	if(g_r3dDX11.IsDeviceLost())
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
	CustomSamplers[slot] = NULL;

	if(mode == R3D_DX11_SAMPLER_LINEAR_WRAP || mode == R3D_DX11_SAMPLER_LINEAR_CLAMP)
	{
		SamplerMinFilter[slot] = D3DTEXF_LINEAR;
		SamplerMagFilter[slot] = D3DTEXF_LINEAR;
		SamplerMipFilter[slot] = D3DTEXF_LINEAR;
	}
	else
	{
		SamplerMinFilter[slot] = D3DTEXF_POINT;
		SamplerMagFilter[slot] = D3DTEXF_POINT;
		SamplerMipFilter[slot] = D3DTEXF_POINT;
	}

	if(mode == R3D_DX11_SAMPLER_LINEAR_CLAMP || mode == R3D_DX11_SAMPLER_POINT_CLAMP)
	{
		SamplerAddressU[slot] = D3DTADDRESS_CLAMP;
		SamplerAddressV[slot] = D3DTADDRESS_CLAMP;
		SamplerAddressW[slot] = D3DTADDRESS_CLAMP;
	}
	else
	{
		SamplerAddressU[slot] = D3DTADDRESS_WRAP;
		SamplerAddressV[slot] = D3DTADDRESS_WRAP;
		SamplerAddressW[slot] = D3DTADDRESS_WRAP;
	}

	SamplerMipLODBias[slot] = 0.0f;
	SamplerMaxAnisotropy[slot] = 1;
	SamplerBorderColor[slot] = 0;
	SamplerMaxMipLevel[slot] = 0;

	return true;
}

bool r3dDX11State::SetSamplerState(int slot, int samplerState, unsigned int value)
{
	const int vsSlot = r3dDX11State_GetVertexSamplerSlot(slot);
	const int targetSlot = vsSlot >= 0 ? vsSlot : slot;
	int* minFilter = vsSlot >= 0 ? VSSamplerMinFilter : SamplerMinFilter;
	int* magFilter = vsSlot >= 0 ? VSSamplerMagFilter : SamplerMagFilter;
	int* mipFilter = vsSlot >= 0 ? VSSamplerMipFilter : SamplerMipFilter;
	int* addressU = vsSlot >= 0 ? VSSamplerAddressU : SamplerAddressU;
	int* addressV = vsSlot >= 0 ? VSSamplerAddressV : SamplerAddressV;
	int* addressW = vsSlot >= 0 ? VSSamplerAddressW : SamplerAddressW;
	float* mipLODBias = vsSlot >= 0 ? VSSamplerMipLODBias : SamplerMipLODBias;
	unsigned int* maxAnisotropy = vsSlot >= 0 ? VSSamplerMaxAnisotropy : SamplerMaxAnisotropy;
	unsigned int* borderColor = vsSlot >= 0 ? VSSamplerBorderColor : SamplerBorderColor;
	unsigned int* maxMipLevel = vsSlot >= 0 ? VSSamplerMaxMipLevel : SamplerMaxMipLevel;
	ID3D11SamplerState** customSamplers = vsSlot >= 0 ? CustomVSSamplers : CustomSamplers;
	ID3D11SamplerState** boundSamplers = vsSlot >= 0 ? BoundVSSampler : BoundSampler;
	bool changed = false;

	if(vsSlot < 0 && (slot < 0 || slot >= 16))
		return false;

	switch(samplerState)
	{
	case D3DSAMP_ADDRESSU:
		changed = r3dDX11State_SetIntIfChanged(addressU[targetSlot], (int)value);
		break;

	case D3DSAMP_ADDRESSV:
		changed = r3dDX11State_SetIntIfChanged(addressV[targetSlot], (int)value);
		break;

	case D3DSAMP_ADDRESSW:
		changed = r3dDX11State_SetIntIfChanged(addressW[targetSlot], (int)value);
		break;

	case D3DSAMP_MAGFILTER:
		changed = r3dDX11State_SetIntIfChanged(magFilter[targetSlot], (int)value);
		break;

	case D3DSAMP_MINFILTER:
		changed = r3dDX11State_SetIntIfChanged(minFilter[targetSlot], (int)value);
		break;

	case D3DSAMP_MIPFILTER:
		changed = r3dDX11State_SetIntIfChanged(mipFilter[targetSlot], (int)value);
		break;

	case D3DSAMP_MIPMAPLODBIAS:
		changed = r3dDX11State_SetFloatIfChanged(mipLODBias[targetSlot], *(const float*)&value);
		break;

	case D3DSAMP_MAXANISOTROPY:
		changed = r3dDX11State_SetUIntIfChanged(maxAnisotropy[targetSlot], value ? value : 1);
		break;

	case D3DSAMP_BORDERCOLOR:
		changed = r3dDX11State_SetUIntIfChanged(borderColor[targetSlot], value);
		break;

	case D3DSAMP_MAXMIPLEVEL:
		changed = r3dDX11State_SetUIntIfChanged(maxMipLevel[targetSlot], value);
		break;

	default:
		return false;
	}

	if(!changed && customSamplers[targetSlot] && boundSamplers[targetSlot] == customSamplers[targetSlot])
		return true;

	return ApplySamplerState(slot);
}

void r3dDX11State::ApplyDefaultSamplers()
{
	for(int i = 0; i < 16; ++i)
	{
		SetSampler(i, R3D_DX11_SAMPLER_LINEAR_WRAP);
	}
}

void r3dDX11State::ResetSamplerStateCache()
{
	for(int i = 0; i < 16; ++i)
	{
		SamplerMinFilter[i] = D3DTEXF_LINEAR;
		SamplerMagFilter[i] = D3DTEXF_LINEAR;
		SamplerMipFilter[i] = D3DTEXF_LINEAR;
		SamplerAddressU[i] = D3DTADDRESS_WRAP;
		SamplerAddressV[i] = D3DTADDRESS_WRAP;
		SamplerAddressW[i] = D3DTADDRESS_WRAP;
		SamplerMipLODBias[i] = 0.0f;
		SamplerMaxAnisotropy[i] = 1;
		SamplerBorderColor[i] = 0;
		SamplerMaxMipLevel[i] = 0;
	}

	for(int i = 0; i < 4; ++i)
	{
		VSSamplerMinFilter[i] = D3DTEXF_LINEAR;
		VSSamplerMagFilter[i] = D3DTEXF_LINEAR;
		VSSamplerMipFilter[i] = D3DTEXF_LINEAR;
		VSSamplerAddressU[i] = D3DTADDRESS_WRAP;
		VSSamplerAddressV[i] = D3DTADDRESS_WRAP;
		VSSamplerAddressW[i] = D3DTADDRESS_WRAP;
		VSSamplerMipLODBias[i] = 0.0f;
		VSSamplerMaxAnisotropy[i] = 1;
		VSSamplerBorderColor[i] = 0;
		VSSamplerMaxMipLevel[i] = 0;
	}
}

bool r3dDX11State::ApplySamplerState(int slot)
{
	if(!Initialized)
		return false;

	if(g_r3dDX11.IsDeviceLost())
		return false;

	const int vsSlot = r3dDX11State_GetVertexSamplerSlot(slot);
	const int targetSlot = vsSlot >= 0 ? vsSlot : slot;
	int* minFilter = vsSlot >= 0 ? VSSamplerMinFilter : SamplerMinFilter;
	int* magFilter = vsSlot >= 0 ? VSSamplerMagFilter : SamplerMagFilter;
	int* mipFilter = vsSlot >= 0 ? VSSamplerMipFilter : SamplerMipFilter;
	int* addressU = vsSlot >= 0 ? VSSamplerAddressU : SamplerAddressU;
	int* addressV = vsSlot >= 0 ? VSSamplerAddressV : SamplerAddressV;
	int* addressW = vsSlot >= 0 ? VSSamplerAddressW : SamplerAddressW;
	float* mipLODBias = vsSlot >= 0 ? VSSamplerMipLODBias : SamplerMipLODBias;
	unsigned int* maxAnisotropy = vsSlot >= 0 ? VSSamplerMaxAnisotropy : SamplerMaxAnisotropy;
	unsigned int* borderColor = vsSlot >= 0 ? VSSamplerBorderColor : SamplerBorderColor;
	unsigned int* maxMipLevel = vsSlot >= 0 ? VSSamplerMaxMipLevel : SamplerMaxMipLevel;
	ID3D11SamplerState** customSamplers = vsSlot >= 0 ? CustomVSSamplers : CustomSamplers;
	ID3D11SamplerState** boundSamplers = vsSlot >= 0 ? BoundVSSampler : BoundSampler;

	if(vsSlot < 0 && (slot < 0 || slot >= 16))
		return false;

	if(!g_r3dDX11.CanCreateDeviceResources("DX11State::ApplySamplerState"))
		return false;

	ID3D11Device* device = g_r3dDX11.GetDevice();
	ID3D11DeviceContext* context = g_r3dDX11.GetContext();

	if(!device || !context)
		return false;

	const unsigned int finalMaxAnisotropy = maxAnisotropy[targetSlot] ? maxAnisotropy[targetSlot] : 1;

	unsigned int hash = 2166136261u;
	r3dDX11State_HashValue(hash, minFilter[targetSlot]);
	r3dDX11State_HashValue(hash, magFilter[targetSlot]);
	r3dDX11State_HashValue(hash, mipFilter[targetSlot]);
	r3dDX11State_HashValue(hash, addressU[targetSlot]);
	r3dDX11State_HashValue(hash, addressV[targetSlot]);
	r3dDX11State_HashValue(hash, addressW[targetSlot]);
	r3dDX11State_HashValue(hash, mipLODBias[targetSlot]);
	r3dDX11State_HashValue(hash, finalMaxAnisotropy);
	r3dDX11State_HashValue(hash, borderColor[targetSlot]);
	r3dDX11State_HashValue(hash, maxMipLevel[targetSlot]);
	hash = r3dDX11State_FinishHash(hash);

	for(int i = 0; i < SamplerStateCacheCount; ++i)
	{
		SamplerStateCacheEntry& entry = SamplerStateCache[i];

		if(entry.Hash != hash)
			continue;

		if(entry.MinFilter != minFilter[targetSlot])
			continue;
		if(entry.MagFilter != magFilter[targetSlot])
			continue;
		if(entry.MipFilter != mipFilter[targetSlot])
			continue;
		if(entry.AddressU != addressU[targetSlot])
			continue;
		if(entry.AddressV != addressV[targetSlot])
			continue;
		if(entry.AddressW != addressW[targetSlot])
			continue;
		if(entry.MipLODBias != mipLODBias[targetSlot])
			continue;
		if(entry.MaxAnisotropy != finalMaxAnisotropy)
			continue;
		if(entry.BorderColor != borderColor[targetSlot])
			continue;
		if(entry.MaxMipLevel != maxMipLevel[targetSlot])
			continue;

		ID3D11SamplerState* cachedSampler = entry.State;

		if(vsSlot >= 0)
			context->VSSetSamplers(vsSlot, 1, &cachedSampler);
		else
			context->PSSetSamplers(slot, 1, &cachedSampler);

		boundSamplers[targetSlot] = cachedSampler;
		customSamplers[targetSlot] = cachedSampler;

		return true;
	}

	if(SamplerStateCacheCount >= SamplerStateCacheMax)
	{
		r3dOutToLog("DX11State: SamplerState cache is full\n");
		return false;
	}

	D3D11_SAMPLER_DESC desc;
	ZeroMemory(&desc, sizeof(desc));

	desc.Filter = r3dDX11State_ToFilter(minFilter[targetSlot], magFilter[targetSlot], mipFilter[targetSlot]);
	desc.AddressU = r3dDX11State_ToAddressMode(addressU[targetSlot]);
	desc.AddressV = r3dDX11State_ToAddressMode(addressV[targetSlot]);
	desc.AddressW = r3dDX11State_ToAddressMode(addressW[targetSlot]);
	desc.MipLODBias = mipLODBias[targetSlot];
	desc.MaxAnisotropy = finalMaxAnisotropy;

	if(desc.MaxAnisotropy > 16)
		desc.MaxAnisotropy = 16;

	desc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	r3dDX11State_DecodeBorderColor(borderColor[targetSlot], desc.BorderColor);
	desc.MinLOD = (FLOAT)maxMipLevel[targetSlot];
	desc.MaxLOD = mipFilter[targetSlot] == D3DTEXF_NONE ? 0.0f : D3D11_FLOAT32_MAX;

	if(desc.MaxLOD < desc.MinLOD)
		desc.MaxLOD = desc.MinLOD;

	ID3D11SamplerState* newSampler = NULL;
	HRESULT hr = device->CreateSamplerState(&desc, &newSampler);

	if(FAILED(hr))
	{
		if(!g_r3dDX11.CheckDeviceRemoved("DX11State::CreateSamplerState", hr))
			r3dDX11State_LogHR("CreateSamplerState", hr);

		return false;
	}

	SamplerStateCacheEntry& entry = SamplerStateCache[SamplerStateCacheCount++];
	entry.Hash = hash;
	entry.MinFilter = minFilter[targetSlot];
	entry.MagFilter = magFilter[targetSlot];
	entry.MipFilter = mipFilter[targetSlot];
	entry.AddressU = addressU[targetSlot];
	entry.AddressV = addressV[targetSlot];
	entry.AddressW = addressW[targetSlot];
	entry.MipLODBias = mipLODBias[targetSlot];
	entry.MaxAnisotropy = finalMaxAnisotropy;
	entry.BorderColor = borderColor[targetSlot];
	entry.MaxMipLevel = maxMipLevel[targetSlot];
	entry.State = newSampler;

	if(vsSlot >= 0)
		context->VSSetSamplers(vsSlot, 1, &newSampler);
	else
		context->PSSetSamplers(slot, 1, &newSampler);

	boundSamplers[targetSlot] = newSampler;
	customSamplers[targetSlot] = newSampler;

	return true;
}

void r3dDX11State::ResetRenderStateCache()
{
	DepthEnable = 1;
	DepthWriteEnable = 1;
	DepthFunc = D3DCMP_LESSEQUAL;
	StencilEnable = 0;
	StencilFunc = D3DCMP_ALWAYS;
	StencilFailOp = D3DSTENCILOP_KEEP;
	StencilZFailOp = D3DSTENCILOP_KEEP;
	StencilPassOp = D3DSTENCILOP_KEEP;
	StencilRef = 0;
	StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
	StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
	AlphaBlendEnable = 0;
	SrcBlend = D3DBLEND_ONE;
	DestBlend = D3DBLEND_ZERO;
	BlendOp = D3DBLENDOP_ADD;
	BlendOpAlpha = D3DBLENDOP_ADD;
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

	if(g_r3dDX11.IsDeviceLost())
		return false;

	if(!g_r3dDX11.CanCreateDeviceResources("DX11State::ApplyDepthState"))
		return false;

	ID3D11Device* device = g_r3dDX11.GetDevice();
	ID3D11DeviceContext* context = g_r3dDX11.GetContext();

	if(!device || !context)
		return false;

	unsigned int hash = 2166136261u;
	r3dDX11State_HashValue(hash, DepthEnable);
	r3dDX11State_HashValue(hash, DepthWriteEnable);
	r3dDX11State_HashValue(hash, DepthFunc);
	r3dDX11State_HashValue(hash, StencilEnable);
	r3dDX11State_HashValue(hash, StencilFunc);
	r3dDX11State_HashValue(hash, StencilFailOp);
	r3dDX11State_HashValue(hash, StencilZFailOp);
	r3dDX11State_HashValue(hash, StencilPassOp);
	r3dDX11State_HashValue(hash, StencilReadMask);
	r3dDX11State_HashValue(hash, StencilWriteMask);
	hash = r3dDX11State_FinishHash(hash);

	for(int i = 0; i < DepthStateCacheCount; ++i)
	{
		DepthStateCacheEntry& entry = DepthStateCache[i];

		if(entry.Hash != hash)
			continue;

		if(entry.DepthEnable != DepthEnable)
			continue;
		if(entry.DepthWriteEnable != DepthWriteEnable)
			continue;
		if(entry.DepthFunc != DepthFunc)
			continue;
		if(entry.StencilEnable != StencilEnable)
			continue;
		if(entry.StencilFunc != StencilFunc)
			continue;
		if(entry.StencilFailOp != StencilFailOp)
			continue;
		if(entry.StencilZFailOp != StencilZFailOp)
			continue;
		if(entry.StencilPassOp != StencilPassOp)
			continue;
		if(entry.StencilReadMask != StencilReadMask)
			continue;
		if(entry.StencilWriteMask != StencilWriteMask)
			continue;

		context->OMSetDepthStencilState(entry.State, StencilRef);
		DepthState = entry.State;

		return true;
	}

	if(DepthStateCacheCount >= DepthStateCacheMax)
	{
		r3dOutToLog("DX11State: DepthStencilState cache is full\n");
		return false;
	}

	D3D11_DEPTH_STENCIL_DESC desc;
	ZeroMemory(&desc, sizeof(desc));

	desc.DepthEnable = DepthEnable ? TRUE : FALSE;
	desc.DepthWriteMask = DepthWriteEnable ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
	desc.DepthFunc = r3dDX11State_ToComparisonFunc(DepthFunc);
	desc.StencilEnable = StencilEnable ? TRUE : FALSE;
	desc.StencilReadMask = (UINT8)StencilReadMask;
	desc.StencilWriteMask = (UINT8)StencilWriteMask;
	desc.FrontFace.StencilFailOp = r3dDX11State_ToStencilOp(StencilFailOp);
	desc.FrontFace.StencilDepthFailOp = r3dDX11State_ToStencilOp(StencilZFailOp);
	desc.FrontFace.StencilPassOp = r3dDX11State_ToStencilOp(StencilPassOp);
	desc.FrontFace.StencilFunc = r3dDX11State_ToComparisonFunc(StencilFunc);
	desc.BackFace = desc.FrontFace;

	ID3D11DepthStencilState* newState = NULL;
	HRESULT hr = device->CreateDepthStencilState(&desc, &newState);

	if(FAILED(hr))
	{
		if(!g_r3dDX11.CheckDeviceRemoved("DX11State::CreateDepthStencilState", hr))
			r3dDX11State_LogHR("CreateDepthStencilState", hr);

		return false;
	}

	DepthStateCacheEntry& entry = DepthStateCache[DepthStateCacheCount++];
	entry.Hash = hash;
	entry.DepthEnable = DepthEnable;
	entry.DepthWriteEnable = DepthWriteEnable;
	entry.DepthFunc = DepthFunc;
	entry.StencilEnable = StencilEnable;
	entry.StencilFunc = StencilFunc;
	entry.StencilFailOp = StencilFailOp;
	entry.StencilZFailOp = StencilZFailOp;
	entry.StencilPassOp = StencilPassOp;
	entry.StencilReadMask = StencilReadMask;
	entry.StencilWriteMask = StencilWriteMask;
	entry.State = newState;

	context->OMSetDepthStencilState(newState, StencilRef);
	DepthState = newState;

	return true;
}

bool r3dDX11State::ApplyBlendState()
{
	if(!Initialized)
		return false;

	if(g_r3dDX11.IsDeviceLost())
		return false;

	if(!g_r3dDX11.CanCreateDeviceResources("DX11State::ApplyBlendState"))
		return false;

	ID3D11Device* device = g_r3dDX11.GetDevice();
	ID3D11DeviceContext* context = g_r3dDX11.GetContext();

	if(!device || !context)
		return false;

	unsigned int hash = 2166136261u;
	r3dDX11State_HashValue(hash, AlphaBlendEnable);
	r3dDX11State_HashValue(hash, SrcBlend);
	r3dDX11State_HashValue(hash, DestBlend);
	r3dDX11State_HashValue(hash, BlendOp);
	r3dDX11State_HashValue(hash, BlendOpAlpha);

	for(int i = 0; i < 4; ++i)
	{
		r3dDX11State_HashValue(hash, ColorWriteMask[i]);
	}

	hash = r3dDX11State_FinishHash(hash);

	for(int i = 0; i < BlendStateCacheCount; ++i)
	{
		BlendStateCacheEntry& entry = BlendStateCache[i];

		if(entry.Hash != hash)
			continue;

		if(entry.AlphaBlendEnable != AlphaBlendEnable)
			continue;
		if(entry.SrcBlend != SrcBlend)
			continue;
		if(entry.DestBlend != DestBlend)
			continue;
		if(entry.BlendOp != BlendOp)
			continue;
		if(entry.BlendOpAlpha != BlendOpAlpha)
			continue;

		bool colorMaskMatch = true;

		for(int rt = 0; rt < 4; ++rt)
		{
			if(entry.ColorWriteMask[rt] != ColorWriteMask[rt])
			{
				colorMaskMatch = false;
				break;
			}
		}

		if(!colorMaskMatch)
			continue;

		float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
		context->OMSetBlendState(entry.State, blendFactor, 0xffffffff);
		BlendState = entry.State;

		return true;
	}

	if(BlendStateCacheCount >= BlendStateCacheMax)
	{
		r3dOutToLog("DX11State: BlendState cache is full\n");
		return false;
	}

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
		desc.RenderTarget[i].BlendOpAlpha = r3dDX11State_ToBlendOp(BlendOpAlpha);
		desc.RenderTarget[i].RenderTargetWriteMask = r3dDX11State_ToColorWriteMask(ColorWriteMask[i]);
	}

	ID3D11BlendState* newState = NULL;
	HRESULT hr = device->CreateBlendState(&desc, &newState);

	if(FAILED(hr))
	{
		if(!g_r3dDX11.CheckDeviceRemoved("DX11State::CreateBlendState", hr))
			r3dDX11State_LogHR("CreateBlendState", hr);

		return false;
	}

	BlendStateCacheEntry& entry = BlendStateCache[BlendStateCacheCount++];
	entry.Hash = hash;
	entry.AlphaBlendEnable = AlphaBlendEnable;
	entry.SrcBlend = SrcBlend;
	entry.DestBlend = DestBlend;
	entry.BlendOp = BlendOp;
	entry.BlendOpAlpha = BlendOpAlpha;

	for(int i = 0; i < 4; ++i)
	{
		entry.ColorWriteMask[i] = ColorWriteMask[i];
	}

	entry.State = newState;

	float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	context->OMSetBlendState(newState, blendFactor, 0xffffffff);
	BlendState = newState;

	return true;
}

bool r3dDX11State::ApplyRasterizerState()
{
	if(!Initialized)
		return false;

	if(g_r3dDX11.IsDeviceLost())
		return false;

	if(!g_r3dDX11.CanCreateDeviceResources("DX11State::ApplyRasterizerState"))
		return false;

	ID3D11Device* device = g_r3dDX11.GetDevice();
	ID3D11DeviceContext* context = g_r3dDX11.GetContext();

	if(!device || !context)
		return false;

	unsigned int hash = 2166136261u;
	r3dDX11State_HashValue(hash, CullMode);
	r3dDX11State_HashValue(hash, FillMode);
	r3dDX11State_HashValue(hash, ScissorEnable);
	hash = r3dDX11State_FinishHash(hash);

	for(int i = 0; i < RasterizerStateCacheCount; ++i)
	{
		RasterizerStateCacheEntry& entry = RasterizerStateCache[i];

		if(entry.Hash != hash)
			continue;

		if(entry.CullMode != CullMode)
			continue;
		if(entry.FillMode != FillMode)
			continue;
		if(entry.ScissorEnable != ScissorEnable)
			continue;

		context->RSSetState(entry.State);
		RasterizerState = entry.State;

		return true;
	}

	if(RasterizerStateCacheCount >= RasterizerStateCacheMax)
	{
		r3dOutToLog("DX11State: RasterizerState cache is full\n");
		return false;
	}

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
		if(!g_r3dDX11.CheckDeviceRemoved("DX11State::CreateRasterizerState", hr))
			r3dDX11State_LogHR("CreateRasterizerState", hr);

		return false;
	}

	RasterizerStateCacheEntry& entry = RasterizerStateCache[RasterizerStateCacheCount++];
	entry.Hash = hash;
	entry.CullMode = CullMode;
	entry.FillMode = FillMode;
	entry.ScissorEnable = ScissorEnable;
	entry.State = newState;

	context->RSSetState(newState);
	RasterizerState = newState;

	return true;
}

bool r3dDX11State::SetRenderState(int state, unsigned int value)
{
	if(!Initialized)
		return false;

	if(g_r3dDX11.IsDeviceLost())
		return false;

	switch(state)
	{
	case D3DRS_ZENABLE:
		if(!r3dDX11State_SetIntIfChanged(DepthEnable, value ? 1 : 0) && DepthState)
			return true;
		return ApplyDepthState();

	case D3DRS_ZWRITEENABLE:
		if(!r3dDX11State_SetIntIfChanged(DepthWriteEnable, value ? 1 : 0) && DepthState)
			return true;
		return ApplyDepthState();

	case D3DRS_ZFUNC:
		if(!r3dDX11State_SetIntIfChanged(DepthFunc, (int)value) && DepthState)
			return true;
		return ApplyDepthState();

	case D3DRS_STENCILENABLE:
		if(!r3dDX11State_SetIntIfChanged(StencilEnable, value ? 1 : 0) && DepthState)
			return true;
		return ApplyDepthState();

	case D3DRS_STENCILFUNC:
		if(!r3dDX11State_SetIntIfChanged(StencilFunc, (int)value) && DepthState)
			return true;
		return ApplyDepthState();

	case D3DRS_STENCILFAIL:
		if(!r3dDX11State_SetIntIfChanged(StencilFailOp, (int)value) && DepthState)
			return true;
		return ApplyDepthState();

	case D3DRS_STENCILZFAIL:
		if(!r3dDX11State_SetIntIfChanged(StencilZFailOp, (int)value) && DepthState)
			return true;
		return ApplyDepthState();

	case D3DRS_STENCILPASS:
		if(!r3dDX11State_SetIntIfChanged(StencilPassOp, (int)value) && DepthState)
			return true;
		return ApplyDepthState();

	case D3DRS_STENCILREF:
		if(!r3dDX11State_SetUIntIfChanged(StencilRef, value) && DepthState)
			return true;
		return ApplyDepthState();

	case D3DRS_STENCILMASK:
		if(!r3dDX11State_SetUIntIfChanged(StencilReadMask, value) && DepthState)
			return true;
		return ApplyDepthState();

	case D3DRS_STENCILWRITEMASK:
		if(!r3dDX11State_SetUIntIfChanged(StencilWriteMask, value) && DepthState)
			return true;
		return ApplyDepthState();

	case D3DRS_ALPHABLENDENABLE:
		if(!r3dDX11State_SetIntIfChanged(AlphaBlendEnable, value ? 1 : 0) && BlendState)
			return true;
		return ApplyBlendState();

	case D3DRS_SRCBLEND:
		if(!r3dDX11State_SetIntIfChanged(SrcBlend, (int)value) && BlendState)
			return true;
		return ApplyBlendState();

	case D3DRS_DESTBLEND:
		if(!r3dDX11State_SetIntIfChanged(DestBlend, (int)value) && BlendState)
			return true;
		return ApplyBlendState();

	case D3DRS_BLENDOP:
		if(!r3dDX11State_SetIntIfChanged(BlendOp, (int)value) && BlendState)
			return true;
		return ApplyBlendState();

	case D3DRS_BLENDOPALPHA:
		if(!r3dDX11State_SetIntIfChanged(BlendOpAlpha, (int)value) && BlendState)
			return true;
		return ApplyBlendState();

	case D3DRS_COLORWRITEENABLE:
		if(!r3dDX11State_SetUIntIfChanged(ColorWriteMask[0], value) && BlendState)
			return true;
		return ApplyBlendState();

	case D3DRS_COLORWRITEENABLE1:
		if(!r3dDX11State_SetUIntIfChanged(ColorWriteMask[1], value) && BlendState)
			return true;
		return ApplyBlendState();

	case D3DRS_COLORWRITEENABLE2:
		if(!r3dDX11State_SetUIntIfChanged(ColorWriteMask[2], value) && BlendState)
			return true;
		return ApplyBlendState();

	case D3DRS_COLORWRITEENABLE3:
		if(!r3dDX11State_SetUIntIfChanged(ColorWriteMask[3], value) && BlendState)
			return true;
		return ApplyBlendState();

	case D3DRS_CULLMODE:
		if(!r3dDX11State_SetIntIfChanged(CullMode, (int)value) && RasterizerState)
			return true;
		return ApplyRasterizerState();

	case D3DRS_FILLMODE:
		if(!r3dDX11State_SetIntIfChanged(FillMode, (int)value) && RasterizerState)
			return true;
		return ApplyRasterizerState();

	case D3DRS_SCISSORTESTENABLE:
		if(!r3dDX11State_SetIntIfChanged(ScissorEnable, value ? 1 : 0) && RasterizerState)
			return true;
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

	for(int i = 0; i < 4; ++i)
	{
		BoundVSSRV[i] = NULL;
		BoundVSSampler[i] = NULL;
	}

	DepthState = NULL;
	BlendState = NULL;
	RasterizerState = NULL;
}

#endif
