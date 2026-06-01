#ifndef __R3D_DX11_LEGACY_GEOMETRY_BRIDGE_H
#define __R3D_DX11_LEGACY_GEOMETRY_BRIDGE_H

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
#include <d3d9.h>
#pragma warning(pop)

#include <vector>

struct ID3D11Buffer;

class r3dDX11LegacyGeometryBridge
{
public:
	r3dDX11LegacyGeometryBridge();
	~r3dDX11LegacyGeometryBridge();

	bool Init();
	void Shutdown();

	bool IsInitialized() const;
	void InvalidateCache();

	bool SetStreamSource(
		UINT stream,
		IDirect3DVertexBuffer9* vertexBuffer,
		UINT offset,
		UINT stride
	);

	bool SetIndices(IDirect3DIndexBuffer9* indexBuffer);

	void LegacyDrawPrimitive(
		D3DPRIMITIVETYPE primitiveType,
		UINT startVertex,
		UINT primitiveCount
	);

	void LegacyDrawIndexedPrimitive(
		D3DPRIMITIVETYPE primitiveType,
		INT baseVertexIndex,
		UINT minVertexIndex,
		UINT numVertices,
		UINT startIndex,
		UINT primitiveCount
	);

	void LegacyDrawPrimitiveUP(
		D3DPRIMITIVETYPE primitiveType,
		UINT primitiveCount,
		const void* vertexData,
		UINT vertexStride
	);

	void LegacyDrawIndexedPrimitiveUP(
		D3DPRIMITIVETYPE primitiveType,
		UINT minVertexIndex,
		UINT numVertices,
		UINT primitiveCount,
		const void* indexData,
		D3DFORMAT indexFormat,
		const void* vertexData,
		UINT vertexStride
	);

private:
	struct VertexBufferEntry
	{
		VertexBufferEntry();

		IDirect3DVertexBuffer9* Source;
		ID3D11Buffer* Buffer;
		UINT CapacityBytes;
		int WarnedLockFail;
	};

	struct IndexBufferEntry
	{
		IndexBufferEntry();

		IDirect3DIndexBuffer9* Source;
		ID3D11Buffer* Buffer;
		UINT CapacityBytes;
		D3DFORMAT Format;
		int WarnedLockFail;
	};

	VertexBufferEntry* FindVertexBuffer(IDirect3DVertexBuffer9* source);
	VertexBufferEntry* FindOrCreateVertexBuffer(IDirect3DVertexBuffer9* source);

	IndexBufferEntry* FindIndexBuffer(IDirect3DIndexBuffer9* source);
	IndexBufferEntry* FindOrCreateIndexBuffer(IDirect3DIndexBuffer9* source);

	void ReleaseVertexBuffer(VertexBufferEntry& entry);
	void ReleaseIndexBuffer(IndexBufferEntry& entry);

	bool EnsureBuffer(
		ID3D11Buffer** buffer,
		UINT* capacityBytes,
		UINT requiredBytes,
		UINT bindFlags
	);

	bool UploadBuffer(
		ID3D11Buffer* buffer,
		const void* data,
		UINT bytes,
		const char* debugName
	);

	bool UploadVertexBuffer(VertexBufferEntry* entry);
	bool UploadIndexBuffer(IndexBufferEntry* entry, D3DFORMAT* outFormat);

private:
	bool Initialized;

	std::vector<VertexBufferEntry> VertexBuffers;
	std::vector<IndexBufferEntry> IndexBuffers;
};

extern r3dDX11LegacyGeometryBridge g_r3dDX11LegacyGeometryBridge;

#endif

#endif