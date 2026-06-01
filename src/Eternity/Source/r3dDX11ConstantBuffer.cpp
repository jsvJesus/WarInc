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
#include "r3dDX11ConstantBuffer.h"

template<typename T>
static void r3dDX11Constants_Release(T*& ptr)
{
	if(ptr)
	{
		ptr->Release();
		ptr = NULL;
	}
}

static void r3dDX11Constants_LogHR(const char* text, HRESULT hr)
{
	r3dOutToLog("DX11Constants: %s failed, HRESULT=0x%08X\n", text, (unsigned int)hr);
}

r3dDX11ConstantBufferBridge g_r3dDX11Constants;

r3dDX11ConstantBufferBridge::r3dDX11ConstantBufferBridge()
{
	Initialized = false;
	VertexBuffer = NULL;
	PixelBuffer = NULL;

	ZeroMemory(VertexConstants, sizeof(VertexConstants));
	ZeroMemory(PixelConstants, sizeof(PixelConstants));
}

r3dDX11ConstantBufferBridge::~r3dDX11ConstantBufferBridge()
{
	Shutdown();
}

bool r3dDX11ConstantBufferBridge::Init()
{
	Shutdown();

	if(!g_r3dDX11.IsInitialized())
	{
		r3dOutToLog("DX11Constants: Init failed, DX11 renderer is not initialized\n");
		return false;
	}

	ZeroMemory(VertexConstants, sizeof(VertexConstants));
	ZeroMemory(PixelConstants, sizeof(PixelConstants));

	if(!CreateBuffers())
	{
		Shutdown();
		return false;
	}

	Initialized = true;
	Bind();

	r3dOutToLog("DX11Constants: initialized %d float4 registers per stage\n", MAX_CONSTANTS);

	return true;
}

void r3dDX11ConstantBufferBridge::Shutdown()
{
	ReleaseBuffers();
	Initialized = false;
}

bool r3dDX11ConstantBufferBridge::IsInitialized() const
{
	return Initialized;
}

bool r3dDX11ConstantBufferBridge::CreateBuffers()
{
	ID3D11Device* device = g_r3dDX11.GetDevice();

	if(!device)
		return false;

	D3D11_BUFFER_DESC desc;
	ZeroMemory(&desc, sizeof(desc));

	desc.ByteWidth = BUFFER_BYTES;
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc.MiscFlags = 0;
	desc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA initData;
	ZeroMemory(&initData, sizeof(initData));

	initData.pSysMem = VertexConstants;

	HRESULT hr = device->CreateBuffer(&desc, &initData, &VertexBuffer);

	if(FAILED(hr))
	{
		r3dDX11Constants_LogHR("CreateBuffer VS constants", hr);
		return false;
	}

	initData.pSysMem = PixelConstants;

	hr = device->CreateBuffer(&desc, &initData, &PixelBuffer);

	if(FAILED(hr))
	{
		r3dDX11Constants_LogHR("CreateBuffer PS constants", hr);
		return false;
	}

	return true;
}

void r3dDX11ConstantBufferBridge::ReleaseBuffers()
{
	r3dDX11Constants_Release(PixelBuffer);
	r3dDX11Constants_Release(VertexBuffer);
}

bool r3dDX11ConstantBufferBridge::SetVertexShaderConstantF(
	unsigned int startRegister,
	const float* data,
	unsigned int vector4fCount
)
{
	return SetConstants(VertexBuffer, VertexConstants, true, startRegister, data, vector4fCount);
}

bool r3dDX11ConstantBufferBridge::SetPixelShaderConstantF(
	unsigned int startRegister,
	const float* data,
	unsigned int vector4fCount
)
{
	return SetConstants(PixelBuffer, PixelConstants, false, startRegister, data, vector4fCount);
}

bool r3dDX11ConstantBufferBridge::SetConstants(
	ID3D11Buffer* buffer,
	float* cache,
	bool vertexStage,
	unsigned int startRegister,
	const float* data,
	unsigned int vector4fCount
)
{
	if(!Initialized || !buffer || !cache)
		return false;

	if(!data || vector4fCount == 0)
		return true;

	if(startRegister >= MAX_CONSTANTS)
		return false;

	if(startRegister + vector4fCount > MAX_CONSTANTS)
		vector4fCount = MAX_CONSTANTS - startRegister;

	memcpy(
		cache + startRegister * FLOATS_PER_CONSTANT,
		data,
		vector4fCount * FLOATS_PER_CONSTANT * sizeof(float)
	);

	return UpdateAndBind(buffer, cache, vertexStage);
}

bool r3dDX11ConstantBufferBridge::UpdateAndBind(ID3D11Buffer* buffer, const float* cache, bool vertexStage)
{
	ID3D11DeviceContext* context = g_r3dDX11.GetContext();

	if(!context)
		return false;

	D3D11_MAPPED_SUBRESOURCE mapped;
	ZeroMemory(&mapped, sizeof(mapped));

	HRESULT hr = context->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);

	if(FAILED(hr))
	{
		r3dDX11Constants_LogHR(vertexStage ? "Map VS constants" : "Map PS constants", hr);
		return false;
	}

	memcpy(mapped.pData, cache, BUFFER_BYTES);
	context->Unmap(buffer, 0);

	ID3D11Buffer* buffers[1];
	buffers[0] = buffer;

	if(vertexStage)
		context->VSSetConstantBuffers(0, 1, buffers);
	else
		context->PSSetConstantBuffers(0, 1, buffers);

	return true;
}

void r3dDX11ConstantBufferBridge::Bind()
{
	if(!Initialized)
		return;

	ID3D11DeviceContext* context = g_r3dDX11.GetContext();

	if(!context)
		return;

	if(VertexBuffer)
		context->VSSetConstantBuffers(0, 1, &VertexBuffer);

	if(PixelBuffer)
		context->PSSetConstantBuffers(0, 1, &PixelBuffer);
}

void r3dDX11ConstantBufferBridge::InvalidateCache()
{
	Bind();
}

#endif
