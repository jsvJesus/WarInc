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
#include "r3dDX11Geometry.h"

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
static void r3dDX11Geometry_Release(T*& ptr)
{
	if(ptr)
	{
		ptr->Release();
		ptr = NULL;
	}
}

static void r3dDX11Geometry_LogHR(const char* text, HRESULT hr)
{
	r3dOutToLog("DX11Geometry: %s failed, HRESULT=0x%08X\n", text, (unsigned int)hr);
}

static DXGI_FORMAT r3dDX11Geometry_ToDXGIIndexFormat(r3dDX11IndexFormat format)
{
	switch(format)
	{
	case R3D_DX11_INDEX_16BIT:
		return DXGI_FORMAT_R16_UINT;

	case R3D_DX11_INDEX_32BIT:
		return DXGI_FORMAT_R32_UINT;

	default:
		return DXGI_FORMAT_UNKNOWN;
	}
}

static D3D11_PRIMITIVE_TOPOLOGY r3dDX11Geometry_ToDX11Topology(r3dDX11PrimitiveType primitiveType)
{
	switch(primitiveType)
	{
	case R3D_DX11_PRIM_POINTLIST:
		return D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;

	case R3D_DX11_PRIM_LINELIST:
		return D3D11_PRIMITIVE_TOPOLOGY_LINELIST;

	case R3D_DX11_PRIM_LINESTRIP:
		return D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP;

	case R3D_DX11_PRIM_TRIANGLELIST:
		return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	case R3D_DX11_PRIM_TRIANGLESTRIP:
		return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;

	default:
		return D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
	}
}

static bool r3dDX11Geometry_IsMulOverflow(unsigned int a, unsigned int b)
{
	if(a == 0 || b == 0)
		return false;

	return a > (0xFFFFFFFFu / b);
}

r3dDX11GeometryState g_r3dDX11Geometry;

r3dDX11VertexBuffer::r3dDX11VertexBuffer()
{
	Buffer = NULL;
	VertexSize = 0;
	VertexCount = 0;
	ByteSize = 0;
	Dynamic = false;
}

r3dDX11VertexBuffer::~r3dDX11VertexBuffer()
{
	Release();
}

void r3dDX11VertexBuffer::Release()
{
	r3dDX11Geometry_Release(Buffer);

	VertexSize = 0;
	VertexCount = 0;
	ByteSize = 0;
	Dynamic = false;
}

bool r3dDX11VertexBuffer::CreateStatic(const void* data, unsigned int vertexSize, unsigned int vertexCount)
{
	return Create(data, vertexSize, vertexCount, false);
}

bool r3dDX11VertexBuffer::CreateDynamic(unsigned int vertexSize, unsigned int vertexCount)
{
	return Create(NULL, vertexSize, vertexCount, true);
}

bool r3dDX11VertexBuffer::Create(const void* data, unsigned int vertexSize, unsigned int vertexCount, bool dynamic)
{
	Release();

	if(!g_r3dDX11.IsInitialized())
	{
		r3dOutToLog("DX11Geometry: VertexBuffer Create failed, DX11 is not initialized\n");
		return false;
	}

	if(vertexSize == 0 || vertexCount == 0)
	{
		r3dOutToLog("DX11Geometry: VertexBuffer Create failed, invalid size/count\n");
		return false;
	}

	if(r3dDX11Geometry_IsMulOverflow(vertexSize, vertexCount))
	{
		r3dOutToLog("DX11Geometry: VertexBuffer Create failed, buffer is too large\n");
		return false;
	}

	ID3D11Device* device = g_r3dDX11.GetDevice();

	if(!device)
		return false;

	const unsigned int byteSize = vertexSize * vertexCount;

	D3D11_BUFFER_DESC desc;
	ZeroMemory(&desc, sizeof(desc));

	desc.ByteWidth = byteSize;
	desc.Usage = dynamic ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	desc.CPUAccessFlags = dynamic ? D3D11_CPU_ACCESS_WRITE : 0;
	desc.MiscFlags = 0;
	desc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA initData;
	ZeroMemory(&initData, sizeof(initData));

	D3D11_SUBRESOURCE_DATA* initPtr = NULL;

	if(data)
	{
		initData.pSysMem = data;
		initData.SysMemPitch = 0;
		initData.SysMemSlicePitch = 0;
		initPtr = &initData;
	}

	HRESULT hr = device->CreateBuffer(&desc, initPtr, &Buffer);

	if(FAILED(hr))
	{
		r3dDX11Geometry_LogHR("CreateBuffer VertexBuffer", hr);
		Release();
		return false;
	}

	VertexSize = vertexSize;
	VertexCount = vertexCount;
	ByteSize = byteSize;
	Dynamic = dynamic;

	return true;
}

bool r3dDX11VertexBuffer::Update(const void* data, unsigned int vertexCount)
{
	if(!Buffer || !data)
		return false;

	if(vertexCount == 0 || vertexCount > VertexCount)
		return false;

	if(r3dDX11Geometry_IsMulOverflow(VertexSize, vertexCount))
		return false;

	ID3D11DeviceContext* context = g_r3dDX11.GetContext();

	if(!context)
		return false;

	const unsigned int updateBytes = VertexSize * vertexCount;

	if(Dynamic)
	{
		D3D11_MAPPED_SUBRESOURCE mapped;
		ZeroMemory(&mapped, sizeof(mapped));

		HRESULT hr = context->Map(Buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);

		if(FAILED(hr))
		{
			r3dDX11Geometry_LogHR("Map VertexBuffer", hr);
			return false;
		}

		memcpy(mapped.pData, data, updateBytes);

		context->Unmap(Buffer, 0);
	}
	else
	{
		D3D11_BOX box;
		ZeroMemory(&box, sizeof(box));

		box.left = 0;
		box.right = updateBytes;
		box.top = 0;
		box.bottom = 1;
		box.front = 0;
		box.back = 1;

		context->UpdateSubresource(Buffer, 0, &box, data, 0, 0);
	}

	return true;
}

bool r3dDX11VertexBuffer::IsValid() const
{
	return Buffer != NULL;
}

bool r3dDX11VertexBuffer::IsDynamic() const
{
	return Dynamic;
}

ID3D11Buffer* r3dDX11VertexBuffer::GetBuffer() const
{
	return Buffer;
}

unsigned int r3dDX11VertexBuffer::GetVertexSize() const
{
	return VertexSize;
}

unsigned int r3dDX11VertexBuffer::GetVertexCount() const
{
	return VertexCount;
}

unsigned int r3dDX11VertexBuffer::GetByteSize() const
{
	return ByteSize;
}

r3dDX11IndexBuffer::r3dDX11IndexBuffer()
{
	Buffer = NULL;
	Format = R3D_DX11_INDEX_UNKNOWN;
	IndexCount = 0;
	IndexSize = 0;
	ByteSize = 0;
	Dynamic = false;
}

r3dDX11IndexBuffer::~r3dDX11IndexBuffer()
{
	Release();
}

void r3dDX11IndexBuffer::Release()
{
	r3dDX11Geometry_Release(Buffer);

	Format = R3D_DX11_INDEX_UNKNOWN;
	IndexCount = 0;
	IndexSize = 0;
	ByteSize = 0;
	Dynamic = false;
}

bool r3dDX11IndexBuffer::CreateStatic16(const unsigned short* data, unsigned int indexCount)
{
	return Create(data, indexCount, R3D_DX11_INDEX_16BIT, false);
}

bool r3dDX11IndexBuffer::CreateStatic32(const unsigned int* data, unsigned int indexCount)
{
	return Create(data, indexCount, R3D_DX11_INDEX_32BIT, false);
}

bool r3dDX11IndexBuffer::CreateDynamic16(unsigned int indexCount)
{
	return Create(NULL, indexCount, R3D_DX11_INDEX_16BIT, true);
}

bool r3dDX11IndexBuffer::CreateDynamic32(unsigned int indexCount)
{
	return Create(NULL, indexCount, R3D_DX11_INDEX_32BIT, true);
}

bool r3dDX11IndexBuffer::Create(const void* data, unsigned int indexCount, r3dDX11IndexFormat format, bool dynamic)
{
	Release();

	if(!g_r3dDX11.IsInitialized())
	{
		r3dOutToLog("DX11Geometry: IndexBuffer Create failed, DX11 is not initialized\n");
		return false;
	}

	if(indexCount == 0)
	{
		r3dOutToLog("DX11Geometry: IndexBuffer Create failed, invalid index count\n");
		return false;
	}

	unsigned int indexSize = 0;

	if(format == R3D_DX11_INDEX_16BIT)
		indexSize = 2;
	else if(format == R3D_DX11_INDEX_32BIT)
		indexSize = 4;
	else
	{
		r3dOutToLog("DX11Geometry: IndexBuffer Create failed, invalid index format\n");
		return false;
	}

	if(r3dDX11Geometry_IsMulOverflow(indexSize, indexCount))
	{
		r3dOutToLog("DX11Geometry: IndexBuffer Create failed, buffer is too large\n");
		return false;
	}

	ID3D11Device* device = g_r3dDX11.GetDevice();

	if(!device)
		return false;

	const unsigned int byteSize = indexSize * indexCount;

	D3D11_BUFFER_DESC desc;
	ZeroMemory(&desc, sizeof(desc));

	desc.ByteWidth = byteSize;
	desc.Usage = dynamic ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	desc.CPUAccessFlags = dynamic ? D3D11_CPU_ACCESS_WRITE : 0;
	desc.MiscFlags = 0;
	desc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA initData;
	ZeroMemory(&initData, sizeof(initData));

	D3D11_SUBRESOURCE_DATA* initPtr = NULL;

	if(data)
	{
		initData.pSysMem = data;
		initData.SysMemPitch = 0;
		initData.SysMemSlicePitch = 0;
		initPtr = &initData;
	}

	HRESULT hr = device->CreateBuffer(&desc, initPtr, &Buffer);

	if(FAILED(hr))
	{
		r3dDX11Geometry_LogHR("CreateBuffer IndexBuffer", hr);
		Release();
		return false;
	}

	Format = format;
	IndexCount = indexCount;
	IndexSize = indexSize;
	ByteSize = byteSize;
	Dynamic = dynamic;

	return true;
}

bool r3dDX11IndexBuffer::Update(const void* data, unsigned int indexCount)
{
	if(!Buffer || !data)
		return false;

	if(indexCount == 0 || indexCount > IndexCount)
		return false;

	if(r3dDX11Geometry_IsMulOverflow(IndexSize, indexCount))
		return false;

	ID3D11DeviceContext* context = g_r3dDX11.GetContext();

	if(!context)
		return false;

	const unsigned int updateBytes = IndexSize * indexCount;

	if(Dynamic)
	{
		D3D11_MAPPED_SUBRESOURCE mapped;
		ZeroMemory(&mapped, sizeof(mapped));

		HRESULT hr = context->Map(Buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);

		if(FAILED(hr))
		{
			r3dDX11Geometry_LogHR("Map IndexBuffer", hr);
			return false;
		}

		memcpy(mapped.pData, data, updateBytes);

		context->Unmap(Buffer, 0);
	}
	else
	{
		D3D11_BOX box;
		ZeroMemory(&box, sizeof(box));

		box.left = 0;
		box.right = updateBytes;
		box.top = 0;
		box.bottom = 1;
		box.front = 0;
		box.back = 1;

		context->UpdateSubresource(Buffer, 0, &box, data, 0, 0);
	}

	return true;
}

bool r3dDX11IndexBuffer::IsValid() const
{
	return Buffer != NULL;
}

bool r3dDX11IndexBuffer::IsDynamic() const
{
	return Dynamic;
}

ID3D11Buffer* r3dDX11IndexBuffer::GetBuffer() const
{
	return Buffer;
}

r3dDX11IndexFormat r3dDX11IndexBuffer::GetFormat() const
{
	return Format;
}

unsigned int r3dDX11IndexBuffer::GetIndexCount() const
{
	return IndexCount;
}

unsigned int r3dDX11IndexBuffer::GetIndexSize() const
{
	return IndexSize;
}

unsigned int r3dDX11IndexBuffer::GetByteSize() const
{
	return ByteSize;
}

r3dDX11GeometryState::r3dDX11GeometryState()
{
	Initialized = false;

	for(int i = 0; i < 16; ++i)
	{
		BoundVertexBuffers[i] = NULL;
		BoundVertexStrides[i] = 0;
		BoundVertexOffsets[i] = 0;
	}

	BoundIndexBuffer = NULL;
	BoundIndexFormat = R3D_DX11_INDEX_UNKNOWN;
	BoundIndexOffset = 0;

	BoundPrimitiveType = R3D_DX11_PRIM_UNKNOWN;

	TempVertexBuffer = NULL;
	TempVertexBufferBytes = 0;
	TempIndexBuffer = NULL;
	TempIndexBufferBytes = 0;
}

r3dDX11GeometryState::~r3dDX11GeometryState()
{
	Shutdown();
}

bool r3dDX11GeometryState::Init()
{
	Shutdown();

	if(!g_r3dDX11.IsInitialized())
	{
		r3dOutToLog("DX11Geometry: Init failed, DX11 renderer is not initialized\n");
		return false;
	}

	InvalidateCache();

	Initialized = true;

	r3dOutToLog("DX11Geometry: initialized\n");

	return true;
}

void r3dDX11GeometryState::Shutdown()
{
	if(g_r3dDX11.IsInitialized())
	{
		ClearVertexBuffers();
		ClearIndexBuffer();
	}

	r3dDX11Geometry_Release(TempIndexBuffer);
	r3dDX11Geometry_Release(TempVertexBuffer);
	TempIndexBufferBytes = 0;
	TempVertexBufferBytes = 0;

	Initialized = false;

	InvalidateCache();
}

bool r3dDX11GeometryState::IsInitialized() const
{
	return Initialized;
}

bool r3dDX11GeometryState::SetVertexBuffer(int stream, r3dDX11VertexBuffer* vertexBuffer, unsigned int offset)
{
	if(!vertexBuffer)
	{
		ClearVertexBuffer(stream);
		return true;
	}

	return SetVertexBufferRaw(
		stream,
		vertexBuffer->GetBuffer(),
		vertexBuffer->GetVertexSize(),
		offset
	);
}

bool r3dDX11GeometryState::SetVertexBufferRaw(int stream, ID3D11Buffer* vertexBuffer, unsigned int stride, unsigned int offset)
{
	if(!Initialized)
		return false;

	if(stream < 0 || stream >= 16)
		return false;

	ID3D11DeviceContext* context = g_r3dDX11.GetContext();

	if(!context)
		return false;

	if(
		BoundVertexBuffers[stream] == vertexBuffer &&
		BoundVertexStrides[stream] == stride &&
		BoundVertexOffsets[stream] == offset
	)
	{
		return true;
	}

	ID3D11Buffer* buffers[1];
	UINT strides[1];
	UINT offsets[1];

	buffers[0] = vertexBuffer;
	strides[0] = stride;
	offsets[0] = offset;

	context->IASetVertexBuffers((UINT)stream, 1, buffers, strides, offsets);

	BoundVertexBuffers[stream] = vertexBuffer;
	BoundVertexStrides[stream] = stride;
	BoundVertexOffsets[stream] = offset;

	return true;
}

void r3dDX11GeometryState::ClearVertexBuffer(int stream)
{
	if(stream < 0 || stream >= 16)
		return;

	SetVertexBufferRaw(stream, NULL, 0, 0);
}

void r3dDX11GeometryState::ClearVertexBuffers()
{
	if(!g_r3dDX11.IsInitialized())
		return;

	ID3D11DeviceContext* context = g_r3dDX11.GetContext();

	if(!context)
		return;

	ID3D11Buffer* buffers[16];
	UINT strides[16];
	UINT offsets[16];

	for(int i = 0; i < 16; ++i)
	{
		buffers[i] = NULL;
		strides[i] = 0;
		offsets[i] = 0;

		BoundVertexBuffers[i] = NULL;
		BoundVertexStrides[i] = 0;
		BoundVertexOffsets[i] = 0;
	}

	context->IASetVertexBuffers(0, 16, buffers, strides, offsets);
}

bool r3dDX11GeometryState::SetIndexBuffer(r3dDX11IndexBuffer* indexBuffer, unsigned int offset)
{
	if(!indexBuffer)
	{
		ClearIndexBuffer();
		return true;
	}

	return SetIndexBufferRaw(
		indexBuffer->GetBuffer(),
		indexBuffer->GetFormat(),
		offset
	);
}

bool r3dDX11GeometryState::SetIndexBufferRaw(ID3D11Buffer* indexBuffer, r3dDX11IndexFormat format, unsigned int offset)
{
	if(!Initialized)
		return false;

	ID3D11DeviceContext* context = g_r3dDX11.GetContext();

	if(!context)
		return false;

	DXGI_FORMAT dxgiFormat = DXGI_FORMAT_UNKNOWN;

	if(indexBuffer)
	{
		dxgiFormat = r3dDX11Geometry_ToDXGIIndexFormat(format);

		if(dxgiFormat == DXGI_FORMAT_UNKNOWN)
			return false;
	}

	if(
		BoundIndexBuffer == indexBuffer &&
		BoundIndexFormat == format &&
		BoundIndexOffset == offset
	)
	{
		return true;
	}

	context->IASetIndexBuffer(indexBuffer, dxgiFormat, offset);

	BoundIndexBuffer = indexBuffer;
	BoundIndexFormat = indexBuffer ? format : R3D_DX11_INDEX_UNKNOWN;
	BoundIndexOffset = offset;

	return true;
}

void r3dDX11GeometryState::ClearIndexBuffer()
{
	if(!g_r3dDX11.IsInitialized())
		return;

	ID3D11DeviceContext* context = g_r3dDX11.GetContext();

	if(!context)
		return;

	context->IASetIndexBuffer(NULL, DXGI_FORMAT_UNKNOWN, 0);

	BoundIndexBuffer = NULL;
	BoundIndexFormat = R3D_DX11_INDEX_UNKNOWN;
	BoundIndexOffset = 0;
}

bool r3dDX11GeometryState::SetPrimitiveTopology(r3dDX11PrimitiveType primitiveType)
{
	if(!Initialized)
		return false;

	if(primitiveType == R3D_DX11_PRIM_UNKNOWN)
		return false;

	ID3D11DeviceContext* context = g_r3dDX11.GetContext();

	if(!context)
		return false;

	if(BoundPrimitiveType == primitiveType)
		return true;

	D3D11_PRIMITIVE_TOPOLOGY topology = r3dDX11Geometry_ToDX11Topology(primitiveType);

	if(topology == D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED)
		return false;

	context->IASetPrimitiveTopology(topology);

	BoundPrimitiveType = primitiveType;

	return true;
}

void r3dDX11GeometryState::Draw(unsigned int vertexCount, unsigned int startVertex)
{
	if(!Initialized)
		return;

	if(vertexCount == 0)
		return;

	ID3D11DeviceContext* context = g_r3dDX11.GetContext();

	if(!context)
		return;

	context->Draw(vertexCount, startVertex);
}

void r3dDX11GeometryState::DrawIndexed(unsigned int indexCount, unsigned int startIndex, int baseVertex)
{
	if(!Initialized)
		return;

	if(indexCount == 0)
		return;

	ID3D11DeviceContext* context = g_r3dDX11.GetContext();

	if(!context)
		return;

	context->DrawIndexed(indexCount, startIndex, baseVertex);
}

void r3dDX11GeometryState::DrawPrimitive(
	r3dDX11PrimitiveType primitiveType,
	unsigned int startVertex,
	unsigned int primitiveCount
)
{
	if(!SetPrimitiveTopology(primitiveType))
		return;

	const unsigned int vertexCount = r3dDX11_GetPrimitiveVertexCount(
		primitiveType,
		primitiveCount
	);

	Draw(vertexCount, startVertex);
}

void r3dDX11GeometryState::DrawIndexedPrimitive(
	r3dDX11PrimitiveType primitiveType,
	int baseVertexIndex,
	unsigned int minVertexIndex,
	unsigned int numVertices,
	unsigned int startIndex,
	unsigned int primitiveCount
)
{
	(void)minVertexIndex;
	(void)numVertices;

	if(!SetPrimitiveTopology(primitiveType))
		return;

	const unsigned int indexCount = r3dDX11_GetPrimitiveIndexCount(
		primitiveType,
		primitiveCount
	);

	DrawIndexed(indexCount, startIndex, baseVertexIndex);
}

static unsigned int r3dDX11Geometry_GrowTempBufferSize(unsigned int byteSize)
{
	unsigned int result = 4096;

	while(result < byteSize && result < 0x80000000u)
		result *= 2;

	return result < byteSize ? byteSize : result;
}

bool r3dDX11GeometryState::EnsureTempVertexBuffer(unsigned int byteSize)
{
	if(!Initialized || byteSize == 0)
		return false;

	if(TempVertexBuffer && TempVertexBufferBytes >= byteSize)
		return true;

	r3dDX11Geometry_Release(TempVertexBuffer);
	TempVertexBufferBytes = 0;

	ID3D11Device* device = g_r3dDX11.GetDevice();

	if(!device)
		return false;

	const unsigned int newSize = r3dDX11Geometry_GrowTempBufferSize(byteSize);

	D3D11_BUFFER_DESC desc;
	ZeroMemory(&desc, sizeof(desc));

	desc.ByteWidth = newSize;
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc.MiscFlags = 0;
	desc.StructureByteStride = 0;

	HRESULT hr = device->CreateBuffer(&desc, NULL, &TempVertexBuffer);

	if(FAILED(hr))
	{
		r3dDX11Geometry_LogHR("CreateBuffer TempVertexBuffer", hr);
		return false;
	}

	TempVertexBufferBytes = newSize;
	return true;
}

bool r3dDX11GeometryState::EnsureTempIndexBuffer(unsigned int byteSize)
{
	if(!Initialized || byteSize == 0)
		return false;

	if(TempIndexBuffer && TempIndexBufferBytes >= byteSize)
		return true;

	r3dDX11Geometry_Release(TempIndexBuffer);
	TempIndexBufferBytes = 0;

	ID3D11Device* device = g_r3dDX11.GetDevice();

	if(!device)
		return false;

	const unsigned int newSize = r3dDX11Geometry_GrowTempBufferSize(byteSize);

	D3D11_BUFFER_DESC desc;
	ZeroMemory(&desc, sizeof(desc));

	desc.ByteWidth = newSize;
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc.MiscFlags = 0;
	desc.StructureByteStride = 0;

	HRESULT hr = device->CreateBuffer(&desc, NULL, &TempIndexBuffer);

	if(FAILED(hr))
	{
		r3dDX11Geometry_LogHR("CreateBuffer TempIndexBuffer", hr);
		return false;
	}

	TempIndexBufferBytes = newSize;
	return true;
}

bool r3dDX11GeometryState::UploadTempBuffer(ID3D11Buffer* buffer, const void* data, unsigned int byteSize)
{
	if(!buffer || !data || byteSize == 0)
		return false;

	ID3D11DeviceContext* context = g_r3dDX11.GetContext();

	if(!context)
		return false;

	D3D11_MAPPED_SUBRESOURCE mapped;
	ZeroMemory(&mapped, sizeof(mapped));

	HRESULT hr = context->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);

	if(FAILED(hr))
	{
		r3dDX11Geometry_LogHR("Map TempBuffer", hr);
		return false;
	}

	memcpy(mapped.pData, data, byteSize);
	context->Unmap(buffer, 0);

	return true;
}

void r3dDX11GeometryState::DrawPrimitiveUP(
	r3dDX11PrimitiveType primitiveType,
	unsigned int primitiveCount,
	const void* vertexData,
	unsigned int vertexStride
)
{
	if(!Initialized || !vertexData || vertexStride == 0)
		return;

	const unsigned int vertexCount = r3dDX11_GetPrimitiveVertexCount(
		primitiveType,
		primitiveCount
	);

	if(vertexCount == 0)
		return;

	if(r3dDX11Geometry_IsMulOverflow(vertexStride, vertexCount))
		return;

	const unsigned int vertexBytes = vertexStride * vertexCount;

	if(!EnsureTempVertexBuffer(vertexBytes))
		return;

	if(!UploadTempBuffer(TempVertexBuffer, vertexData, vertexBytes))
		return;

	if(!SetVertexBufferRaw(0, TempVertexBuffer, vertexStride, 0))
		return;

	DrawPrimitive(primitiveType, 0, primitiveCount);

	ClearVertexBuffer(0);
}

void r3dDX11GeometryState::DrawIndexedPrimitiveUP(
	r3dDX11PrimitiveType primitiveType,
	unsigned int minVertexIndex,
	unsigned int numVertices,
	unsigned int primitiveCount,
	const void* indexData,
	r3dDX11IndexFormat indexFormat,
	const void* vertexData,
	unsigned int vertexStride
)
{
	(void)minVertexIndex;

	if(!Initialized || !indexData || !vertexData || vertexStride == 0)
		return;

	if(numVertices == 0)
		return;

	const unsigned int indexCount = r3dDX11_GetPrimitiveIndexCount(
		primitiveType,
		primitiveCount
	);

	if(indexCount == 0)
		return;

	unsigned int indexSize = 0;

	if(indexFormat == R3D_DX11_INDEX_16BIT)
		indexSize = 2;
	else if(indexFormat == R3D_DX11_INDEX_32BIT)
		indexSize = 4;
	else
		return;

	if(r3dDX11Geometry_IsMulOverflow(vertexStride, numVertices))
		return;

	if(r3dDX11Geometry_IsMulOverflow(indexSize, indexCount))
		return;

	const unsigned int vertexBytes = vertexStride * numVertices;
	const unsigned int indexBytes = indexSize * indexCount;

	if(!EnsureTempVertexBuffer(vertexBytes))
		return;

	if(!EnsureTempIndexBuffer(indexBytes))
		return;

	if(!UploadTempBuffer(TempVertexBuffer, vertexData, vertexBytes))
		return;

	if(!UploadTempBuffer(TempIndexBuffer, indexData, indexBytes))
		return;

	if(!SetVertexBufferRaw(0, TempVertexBuffer, vertexStride, 0))
		return;

	if(!SetIndexBufferRaw(TempIndexBuffer, indexFormat, 0))
		return;

	DrawIndexedPrimitive(
		primitiveType,
		0,
		0,
		numVertices,
		0,
		primitiveCount
	);

	ClearIndexBuffer();
	ClearVertexBuffer(0);
}

void r3dDX11GeometryState::InvalidateCache()
{
	for(int i = 0; i < 16; ++i)
	{
		BoundVertexBuffers[i] = NULL;
		BoundVertexStrides[i] = 0;
		BoundVertexOffsets[i] = 0;
	}

	BoundIndexBuffer = NULL;
	BoundIndexFormat = R3D_DX11_INDEX_UNKNOWN;
	BoundIndexOffset = 0;

	BoundPrimitiveType = R3D_DX11_PRIM_UNKNOWN;
}

unsigned int r3dDX11_GetPrimitiveVertexCount(r3dDX11PrimitiveType primitiveType, unsigned int primitiveCount)
{
	switch(primitiveType)
	{
	case R3D_DX11_PRIM_POINTLIST:
		return primitiveCount;

	case R3D_DX11_PRIM_LINELIST:
		return primitiveCount * 2;

	case R3D_DX11_PRIM_LINESTRIP:
		return primitiveCount + 1;

	case R3D_DX11_PRIM_TRIANGLELIST:
		return primitiveCount * 3;

	case R3D_DX11_PRIM_TRIANGLESTRIP:
		return primitiveCount + 2;

	default:
		return 0;
	}
}

unsigned int r3dDX11_GetPrimitiveIndexCount(r3dDX11PrimitiveType primitiveType, unsigned int primitiveCount)
{
	return r3dDX11_GetPrimitiveVertexCount(primitiveType, primitiveCount);
}

r3dDX11PrimitiveType r3dDX11_ConvertD3D9PrimitiveType(int d3dPrimitiveType)
{
	switch(d3dPrimitiveType)
	{
	case 1:
		return R3D_DX11_PRIM_POINTLIST;

	case 2:
		return R3D_DX11_PRIM_LINELIST;

	case 3:
		return R3D_DX11_PRIM_LINESTRIP;

	case 4:
		return R3D_DX11_PRIM_TRIANGLELIST;

	case 5:
		return R3D_DX11_PRIM_TRIANGLESTRIP;

	default:
		break;
	}

	r3dOutToLog("DX11Geometry: unsupported D3D9 primitive type %d\n", d3dPrimitiveType);

	return R3D_DX11_PRIM_UNKNOWN;
}

r3dDX11IndexFormat r3dDX11_ConvertD3D9IndexFormat(int d3dFormat)
{
	switch(d3dFormat)
	{
	case 101:
		return R3D_DX11_INDEX_16BIT;

	case 102:
		return R3D_DX11_INDEX_32BIT;

	default:
		break;
	}

	r3dOutToLog("DX11Geometry: unsupported D3D9 index format %d\n", d3dFormat);

	return R3D_DX11_INDEX_UNKNOWN;
}

#endif
