#ifndef __R3D_DX11_CONSTANT_BUFFER_H
#define __R3D_DX11_CONSTANT_BUFFER_H

#ifndef WO_SERVER

struct ID3D11Buffer;

class r3dDX11ConstantBufferBridge
{
public:
	r3dDX11ConstantBufferBridge();
	~r3dDX11ConstantBufferBridge();

	bool Init();
	void Shutdown();

	bool IsInitialized() const;

	bool SetVertexShaderConstantF(unsigned int startRegister, const float* data, unsigned int vector4fCount);
	bool SetPixelShaderConstantF(unsigned int startRegister, const float* data, unsigned int vector4fCount);

	void Bind();
	void InvalidateCache();

private:
	enum
	{
		MAX_CONSTANTS = 256,
		FLOATS_PER_CONSTANT = 4,
		BUFFER_FLOATS = MAX_CONSTANTS * FLOATS_PER_CONSTANT,
		BUFFER_BYTES = BUFFER_FLOATS * sizeof(float)
	};

	bool CreateBuffers();
	void ReleaseBuffers();

	bool SetConstants(
		ID3D11Buffer* buffer,
		float* cache,
		bool vertexStage,
		unsigned int startRegister,
		const float* data,
		unsigned int vector4fCount
	);

	bool UpdateAndBind(ID3D11Buffer* buffer, const float* cache, bool vertexStage);

	bool Initialized;
	ID3D11Buffer* VertexBuffer;
	ID3D11Buffer* PixelBuffer;
	float VertexConstants[BUFFER_FLOATS];
	float PixelConstants[BUFFER_FLOATS];
};

extern r3dDX11ConstantBufferBridge g_r3dDX11Constants;

#endif

#endif
