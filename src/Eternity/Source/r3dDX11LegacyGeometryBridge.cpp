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
#include <d3d9.h>
#pragma warning(pop)

#include "r3d.h"
#include "r3dDX11.h"
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

#ifdef DrawPrimitiveUP
#undef DrawPrimitiveUP
#endif

#ifdef DrawIndexedPrimitive
#undef DrawIndexedPrimitive
#endif

#ifdef DrawIndexedPrimitiveUP
#undef DrawIndexedPrimitiveUP
#endif

#ifdef SetStreamSource
#undef SetStreamSource
#endif

#ifdef SetIndices
#undef SetIndices
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

template<typename T>
static void r3dDX11LegacyGeometryBridge_Release(T*& ptr)
{
	if(ptr)
	{
		ptr->Release();
		ptr = NULL;
	}
}

static void r3dDX11LegacyGeometryBridge_LogHR(const char* text, HRESULT hr)
{
	r3dOutToLog("DX11LegacyGeometryBridge: %s failed, HRESULT=0x%08X\n", text, (unsigned int)hr);
}

static UINT r3dDX11LegacyGeometryBridge_GrowBuffer(UINT requiredBytes)
{
	UINT result = 4096;

	while(result < requiredBytes && result < 0x80000000u)
		result *= 2;

	return result < requiredBytes ? requiredBytes : result;
}

r3dDX11LegacyGeometryBridge g_r3dDX11LegacyGeometryBridge;

r3dDX11LegacyGeometryBridge::VertexBufferEntry::VertexBufferEntry()
{
	Source = NULL;
	Buffer = NULL;
	CapacityBytes = 0;
	WarnedLockFail = 0;
}

r3dDX11LegacyGeometryBridge::IndexBufferEntry::IndexBufferEntry()
{
	Source = NULL;
	Buffer = NULL;
	CapacityBytes = 0;
	Format = D3DFMT_UNKNOWN;
	WarnedLockFail = 0;
}

r3dDX11LegacyGeometryBridge::r3dDX11LegacyGeometryBridge()
{
	Initialized = false;
}

r3dDX11LegacyGeometryBridge::~r3dDX11LegacyGeometryBridge()
{
	Shutdown();
}

bool r3dDX11LegacyGeometryBridge::Init()
{
	Shutdown();

	if(!g_r3dDX11.IsInitialized())
	{
		r3dOutToLog("DX11LegacyGeometryBridge: Init failed, DX11 renderer is not initialized\n");
		return false;
	}

	if(!g_r3dDX11Geometry.IsInitialized())
	{
		r3dOutToLog("DX11LegacyGeometryBridge: Init failed, DX11 geometry is not initialized\n");
		return false;
	}

	Initialized = true;

	r3dOutToLog("DX11LegacyGeometryBridge: initialized\n");

	return true;
}

void r3dDX11LegacyGeometryBridge::Shutdown()
{
	for(size_t i = 0; i < VertexBuffers.size(); ++i)
	{
		ReleaseVertexBuffer(VertexBuffers[i]);
	}

	for(size_t i = 0; i < IndexBuffers.size(); ++i)
	{
		ReleaseIndexBuffer(IndexBuffers[i]);
	}

	VertexBuffers.clear();
	IndexBuffers.clear();

	Initialized = false;
}

bool r3dDX11LegacyGeometryBridge::IsInitialized() const
{
	return Initialized;
}

void r3dDX11LegacyGeometryBridge::InvalidateCache()
{
	if(g_r3dDX11Geometry.IsInitialized())
		g_r3dDX11Geometry.InvalidateCache();
}

void r3dDX11LegacyGeometryBridge::ReleaseVertexBuffer(VertexBufferEntry& entry)
{
	r3dDX11LegacyGeometryBridge_Release(entry.Buffer);
	r3dDX11LegacyGeometryBridge_Release(entry.Source);

	entry.CapacityBytes = 0;
	entry.WarnedLockFail = 0;
}

void r3dDX11LegacyGeometryBridge::ReleaseIndexBuffer(IndexBufferEntry& entry)
{
	r3dDX11LegacyGeometryBridge_Release(entry.Buffer);
	r3dDX11LegacyGeometryBridge_Release(entry.Source);

	entry.CapacityBytes = 0;
	entry.Format = D3DFMT_UNKNOWN;
	entry.WarnedLockFail = 0;
}

r3dDX11LegacyGeometryBridge::VertexBufferEntry* r3dDX11LegacyGeometryBridge::FindVertexBuffer(IDirect3DVertexBuffer9* source)
{
	for(size_t i = 0; i < VertexBuffers.size(); ++i)
	{
		if(VertexBuffers[i].Source == source)
			return &VertexBuffers[i];
	}

	return NULL;
}

r3dDX11LegacyGeometryBridge::VertexBufferEntry* r3dDX11LegacyGeometryBridge::FindOrCreateVertexBuffer(IDirect3DVertexBuffer9* source)
{
	if(!source)
		return NULL;

	VertexBufferEntry* entry = FindVertexBuffer(source);

	if(entry)
		return entry;

	VertexBufferEntry newEntry;
	newEntry.Source = source;
	newEntry.Source->AddRef();

	VertexBuffers.push_back(newEntry);

	return &VertexBuffers[VertexBuffers.size() - 1];
}

r3dDX11LegacyGeometryBridge::IndexBufferEntry* r3dDX11LegacyGeometryBridge::FindIndexBuffer(IDirect3DIndexBuffer9* source)
{
	for(size_t i = 0; i < IndexBuffers.size(); ++i)
	{
		if(IndexBuffers[i].Source == source)
			return &IndexBuffers[i];
	}

	return NULL;
}

r3dDX11LegacyGeometryBridge::IndexBufferEntry* r3dDX11LegacyGeometryBridge::FindOrCreateIndexBuffer(IDirect3DIndexBuffer9* source)
{
	if(!source)
		return NULL;

	IndexBufferEntry* entry = FindIndexBuffer(source);

	if(entry)
		return entry;

	IndexBufferEntry newEntry;
	newEntry.Source = source;
	newEntry.Source->AddRef();

	IndexBuffers.push_back(newEntry);

	return &IndexBuffers[IndexBuffers.size() - 1];
}

bool r3dDX11LegacyGeometryBridge::EnsureBuffer(
	ID3D11Buffer** buffer,
	UINT* capacityBytes,
	UINT requiredBytes,
	UINT bindFlags
)
{
	if(!buffer || !capacityBytes || requiredBytes == 0)
		return false;

	if(!g_r3dDX11.CanCreateDeviceResources("DX11LegacyGeometryBridge::EnsureBuffer"))
		return false;

	if(*buffer && *capacityBytes >= requiredBytes)
		return true;

	r3dDX11LegacyGeometryBridge_Release(*buffer);
	*capacityBytes = 0;

	ID3D11Device* device = g_r3dDX11.GetDevice();

	if(!device)
		return false;

	const UINT newSize = r3dDX11LegacyGeometryBridge_GrowBuffer(requiredBytes);

	D3D11_BUFFER_DESC desc;
	ZeroMemory(&desc, sizeof(desc));

	desc.ByteWidth = newSize;
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.BindFlags = bindFlags;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc.MiscFlags = 0;
	desc.StructureByteStride = 0;

	HRESULT hr = device->CreateBuffer(&desc, NULL, buffer);

	if(FAILED(hr))
	{
		if(!g_r3dDX11.CheckDeviceRemoved("DX11LegacyGeometryBridge::CreateBuffer", hr))
			r3dDX11LegacyGeometryBridge_LogHR("CreateBuffer", hr);
		
		return false;
	}

	*capacityBytes = newSize;

	return true;
}

bool r3dDX11LegacyGeometryBridge::UploadBuffer(
	ID3D11Buffer* buffer,
	const void* data,
	UINT bytes,
	const char* debugName
)
{
	if(!buffer || !data || bytes == 0)
		return false;

	if(g_r3dDX11.IsDeviceLost())
		return false;

	ID3D11DeviceContext* context = g_r3dDX11.GetContext();

	if(!context)
		return false;

	D3D11_MAPPED_SUBRESOURCE mapped;
	ZeroMemory(&mapped, sizeof(mapped));

	HRESULT hr = context->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);

	if(FAILED(hr))
	{
		if(!g_r3dDX11.CheckDeviceRemoved(debugName ? debugName : "DX11LegacyGeometryBridge::Map", hr))
			r3dDX11LegacyGeometryBridge_LogHR(debugName ? debugName : "Map", hr);
		
		return false;
	}

	memcpy(mapped.pData, data, bytes);

	context->Unmap(buffer, 0);

	return true;
}

bool r3dDX11LegacyGeometryBridge::UploadVertexBuffer(VertexBufferEntry* entry)
{
	if(!entry || !entry->Source)
		return false;

	D3DVERTEXBUFFER_DESC desc;
	ZeroMemory(&desc, sizeof(desc));

	HRESULT hr = entry->Source->GetDesc(&desc);

	if(FAILED(hr))
	{
		r3dDX11LegacyGeometryBridge_LogHR("IDirect3DVertexBuffer9::GetDesc", hr);
		return false;
	}

	if(desc.Size == 0)
		return false;

	if(!EnsureBuffer(&entry->Buffer, &entry->CapacityBytes, desc.Size, D3D11_BIND_VERTEX_BUFFER))
		return false;

	void* src = NULL;

	hr = entry->Source->Lock(0, desc.Size, &src, D3DLOCK_READONLY);

	if(FAILED(hr) || !src)
	{
		if(!entry->WarnedLockFail)
		{
			entry->WarnedLockFail = 1;

			r3dOutToLog(
				"DX11LegacyGeometryBridge: vertex buffer Lock READONLY failed, HRESULT=0x%08X, size=%u. Native DX11 buffer conversion required.\n",
				(unsigned int)hr,
				(unsigned int)desc.Size
			);
		}

		return false;
	}

	const bool uploaded = UploadBuffer(entry->Buffer, src, desc.Size, "Map Legacy VertexBuffer");

	entry->Source->Unlock();

	return uploaded;
}

bool r3dDX11LegacyGeometryBridge::UploadIndexBuffer(IndexBufferEntry* entry, D3DFORMAT* outFormat)
{
	if(outFormat)
		*outFormat = D3DFMT_UNKNOWN;

	if(!entry || !entry->Source)
		return false;

	D3DINDEXBUFFER_DESC desc;
	ZeroMemory(&desc, sizeof(desc));

	HRESULT hr = entry->Source->GetDesc(&desc);

	if(FAILED(hr))
	{
		r3dDX11LegacyGeometryBridge_LogHR("IDirect3DIndexBuffer9::GetDesc", hr);
		return false;
	}

	if(desc.Size == 0)
		return false;

	if(desc.Format != D3DFMT_INDEX16 && desc.Format != D3DFMT_INDEX32)
	{
		r3dOutToLog(
			"DX11LegacyGeometryBridge: unsupported legacy index format %d\n",
			(int)desc.Format
		);

		return false;
	}

	if(!EnsureBuffer(&entry->Buffer, &entry->CapacityBytes, desc.Size, D3D11_BIND_INDEX_BUFFER))
		return false;

	void* src = NULL;

	hr = entry->Source->Lock(0, desc.Size, &src, D3DLOCK_READONLY);

	if(FAILED(hr) || !src)
	{
		if(!entry->WarnedLockFail)
		{
			entry->WarnedLockFail = 1;

			r3dOutToLog(
				"DX11LegacyGeometryBridge: index buffer Lock READONLY failed, HRESULT=0x%08X, size=%u. Native DX11 buffer conversion required.\n",
				(unsigned int)hr,
				(unsigned int)desc.Size
			);
		}

		return false;
	}

	const bool uploaded = UploadBuffer(entry->Buffer, src, desc.Size, "Map Legacy IndexBuffer");

	entry->Source->Unlock();

	if(!uploaded)
		return false;

	entry->Format = desc.Format;

	if(outFormat)
		*outFormat = desc.Format;

	return true;
}

bool r3dDX11LegacyGeometryBridge::SetStreamSource(
	UINT stream,
	IDirect3DVertexBuffer9* vertexBuffer,
	UINT offset,
	UINT stride
)
{
	if(!Initialized)
		return false;

	if(stream >= 16)
		return false;

	if(!vertexBuffer)
	{
		g_r3dDX11Geometry.ClearVertexBuffer((int)stream);
		return true;
	}

	if(stride == 0)
		return false;

	VertexBufferEntry* entry = FindOrCreateVertexBuffer(vertexBuffer);

	if(!entry)
		return false;

	if(!UploadVertexBuffer(entry))
		return false;

	return g_r3dDX11Geometry.SetVertexBufferRaw(
		(int)stream,
		entry->Buffer,
		stride,
		offset
	);
}

bool r3dDX11LegacyGeometryBridge::SetIndices(IDirect3DIndexBuffer9* indexBuffer)
{
	if(!Initialized)
		return false;

	if(!indexBuffer)
	{
		g_r3dDX11Geometry.ClearIndexBuffer();
		return true;
	}

	IndexBufferEntry* entry = FindOrCreateIndexBuffer(indexBuffer);

	if(!entry)
		return false;

	D3DFORMAT format = D3DFMT_UNKNOWN;

	if(!UploadIndexBuffer(entry, &format))
		return false;

	r3dDX11IndexFormat dx11Format = r3dDX11_ConvertD3D9IndexFormat((int)format);

	if(dx11Format == R3D_DX11_INDEX_UNKNOWN)
		return false;

	return g_r3dDX11Geometry.SetIndexBufferRaw(
		entry->Buffer,
		dx11Format,
		0
	);
}

void r3dDX11LegacyGeometryBridge::LegacyDrawPrimitive(
	D3DPRIMITIVETYPE primitiveType,
	UINT startVertex,
	UINT primitiveCount
)
{
	if(!Initialized)
		return;

	r3dDX11PrimitiveType dx11PrimitiveType = r3dDX11_ConvertD3D9PrimitiveType((int)primitiveType);

	if(dx11PrimitiveType == R3D_DX11_PRIM_UNKNOWN)
		return;

	g_r3dDX11Geometry.DrawPrimitive(
		dx11PrimitiveType,
		startVertex,
		primitiveCount
	);
}

void r3dDX11LegacyGeometryBridge::LegacyDrawIndexedPrimitive(
	D3DPRIMITIVETYPE primitiveType,
	INT baseVertexIndex,
	UINT minVertexIndex,
	UINT numVertices,
	UINT startIndex,
	UINT primitiveCount
)
{
	if(!Initialized)
		return;

	r3dDX11PrimitiveType dx11PrimitiveType = r3dDX11_ConvertD3D9PrimitiveType((int)primitiveType);

	if(dx11PrimitiveType == R3D_DX11_PRIM_UNKNOWN)
		return;

	g_r3dDX11Geometry.DrawIndexedPrimitive(
		dx11PrimitiveType,
		baseVertexIndex,
		minVertexIndex,
		numVertices,
		startIndex,
		primitiveCount
	);
}

void r3dDX11LegacyGeometryBridge::LegacyDrawPrimitiveUP(
	D3DPRIMITIVETYPE primitiveType,
	UINT primitiveCount,
	const void* vertexData,
	UINT vertexStride
)
{
	if(!Initialized)
		return;

	r3dDX11PrimitiveType dx11PrimitiveType = r3dDX11_ConvertD3D9PrimitiveType((int)primitiveType);

	if(dx11PrimitiveType == R3D_DX11_PRIM_UNKNOWN)
		return;

	g_r3dDX11Geometry.DrawPrimitiveUP(
		dx11PrimitiveType,
		primitiveCount,
		vertexData,
		vertexStride
	);
}

void r3dDX11LegacyGeometryBridge::LegacyDrawIndexedPrimitiveUP(
	D3DPRIMITIVETYPE primitiveType,
	UINT minVertexIndex,
	UINT numVertices,
	UINT primitiveCount,
	const void* indexData,
	D3DFORMAT indexFormat,
	const void* vertexData,
	UINT vertexStride
)
{
	if(!Initialized)
		return;

	r3dDX11PrimitiveType dx11PrimitiveType = r3dDX11_ConvertD3D9PrimitiveType((int)primitiveType);

	if(dx11PrimitiveType == R3D_DX11_PRIM_UNKNOWN)
		return;

	r3dDX11IndexFormat dx11IndexFormat = r3dDX11_ConvertD3D9IndexFormat((int)indexFormat);

	if(dx11IndexFormat == R3D_DX11_INDEX_UNKNOWN)
		return;

	g_r3dDX11Geometry.DrawIndexedPrimitiveUP(
		dx11PrimitiveType,
		minVertexIndex,
		numVertices,
		primitiveCount,
		indexData,
		dx11IndexFormat,
		vertexData,
		vertexStride
	);
}

#endif