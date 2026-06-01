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

	Initialized = false;

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

void r3dDX11State::InvalidateCache()
{
	for(int i = 0; i < 16; ++i)
	{
		BoundSRV[i] = NULL;
		BoundSampler[i] = NULL;
	}
}

#endif