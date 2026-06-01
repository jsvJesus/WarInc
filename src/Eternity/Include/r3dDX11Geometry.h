#ifndef __R3D_DX11_GEOMETRY_H
#define __R3D_DX11_GEOMETRY_H

#ifndef WO_SERVER

struct ID3D11Buffer;

enum r3dDX11PrimitiveType
{
	R3D_DX11_PRIM_UNKNOWN = 0,
	R3D_DX11_PRIM_POINTLIST,
	R3D_DX11_PRIM_LINELIST,
	R3D_DX11_PRIM_LINESTRIP,
	R3D_DX11_PRIM_TRIANGLELIST,
	R3D_DX11_PRIM_TRIANGLESTRIP
};

enum r3dDX11IndexFormat
{
	R3D_DX11_INDEX_UNKNOWN = 0,
	R3D_DX11_INDEX_16BIT,
	R3D_DX11_INDEX_32BIT
};

class r3dDX11VertexBuffer
{
public:
	r3dDX11VertexBuffer();
	~r3dDX11VertexBuffer();

	void Release();

	bool CreateStatic(const void* data, unsigned int vertexSize, unsigned int vertexCount);
	bool CreateDynamic(unsigned int vertexSize, unsigned int vertexCount);
	bool Create(const void* data, unsigned int vertexSize, unsigned int vertexCount, bool dynamic);

	bool Update(const void* data, unsigned int vertexCount);

	bool IsValid() const;
	bool IsDynamic() const;

	ID3D11Buffer* GetBuffer() const;
	unsigned int GetVertexSize() const;
	unsigned int GetVertexCount() const;
	unsigned int GetByteSize() const;

private:
	r3dDX11VertexBuffer(const r3dDX11VertexBuffer&);
	r3dDX11VertexBuffer& operator=(const r3dDX11VertexBuffer&);

	ID3D11Buffer* Buffer;
	unsigned int VertexSize;
	unsigned int VertexCount;
	unsigned int ByteSize;
	bool Dynamic;
};

class r3dDX11IndexBuffer
{
public:
	r3dDX11IndexBuffer();
	~r3dDX11IndexBuffer();

	void Release();

	bool CreateStatic16(const unsigned short* data, unsigned int indexCount);
	bool CreateStatic32(const unsigned int* data, unsigned int indexCount);

	bool CreateDynamic16(unsigned int indexCount);
	bool CreateDynamic32(unsigned int indexCount);

	bool Create(const void* data, unsigned int indexCount, r3dDX11IndexFormat format, bool dynamic);
	bool Update(const void* data, unsigned int indexCount);

	bool IsValid() const;
	bool IsDynamic() const;

	ID3D11Buffer* GetBuffer() const;
	r3dDX11IndexFormat GetFormat() const;

	unsigned int GetIndexCount() const;
	unsigned int GetIndexSize() const;
	unsigned int GetByteSize() const;

private:
	r3dDX11IndexBuffer(const r3dDX11IndexBuffer&);
	r3dDX11IndexBuffer& operator=(const r3dDX11IndexBuffer&);

	ID3D11Buffer* Buffer;
	r3dDX11IndexFormat Format;
	unsigned int IndexCount;
	unsigned int IndexSize;
	unsigned int ByteSize;
	bool Dynamic;
};

class r3dDX11GeometryState
{
public:
	r3dDX11GeometryState();
	~r3dDX11GeometryState();

	bool Init();
	void Shutdown();

	bool IsInitialized() const;

	bool SetVertexBuffer(int stream, r3dDX11VertexBuffer* vertexBuffer, unsigned int offset);
	bool SetVertexBufferRaw(int stream, ID3D11Buffer* vertexBuffer, unsigned int stride, unsigned int offset);

	void ClearVertexBuffer(int stream);
	void ClearVertexBuffers();

	bool SetIndexBuffer(r3dDX11IndexBuffer* indexBuffer, unsigned int offset);
	bool SetIndexBufferRaw(ID3D11Buffer* indexBuffer, r3dDX11IndexFormat format, unsigned int offset);

	void ClearIndexBuffer();

	bool SetPrimitiveTopology(r3dDX11PrimitiveType primitiveType);

	void Draw(unsigned int vertexCount, unsigned int startVertex);
	void DrawIndexed(unsigned int indexCount, unsigned int startIndex, int baseVertex);

	void DrawPrimitive(
		r3dDX11PrimitiveType primitiveType,
		unsigned int startVertex,
		unsigned int primitiveCount
	);

	void DrawIndexedPrimitive(
		r3dDX11PrimitiveType primitiveType,
		int baseVertexIndex,
		unsigned int minVertexIndex,
		unsigned int numVertices,
		unsigned int startIndex,
		unsigned int primitiveCount
	);

	void InvalidateCache();

private:
	bool Initialized;

	ID3D11Buffer* BoundVertexBuffers[16];
	unsigned int BoundVertexStrides[16];
	unsigned int BoundVertexOffsets[16];

	ID3D11Buffer* BoundIndexBuffer;
	r3dDX11IndexFormat BoundIndexFormat;
	unsigned int BoundIndexOffset;

	r3dDX11PrimitiveType BoundPrimitiveType;
};

extern r3dDX11GeometryState g_r3dDX11Geometry;

unsigned int r3dDX11_GetPrimitiveVertexCount(r3dDX11PrimitiveType primitiveType, unsigned int primitiveCount);
unsigned int r3dDX11_GetPrimitiveIndexCount(r3dDX11PrimitiveType primitiveType, unsigned int primitiveCount);

r3dDX11PrimitiveType r3dDX11_ConvertD3D9PrimitiveType(int d3dPrimitiveType);
r3dDX11IndexFormat r3dDX11_ConvertD3D9IndexFormat(int d3dFormat);

#endif

#endif