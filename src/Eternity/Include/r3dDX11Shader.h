#ifndef __R3D_DX11_SHADER_H
#define __R3D_DX11_SHADER_H

#ifndef WO_SERVER

struct ID3D11VertexShader;
struct ID3D11PixelShader;
struct ID3D11InputLayout;
struct ID3D10Blob;
struct D3D11_INPUT_ELEMENT_DESC;

struct r3dDX11ShaderMacro
{
	const char* Name;
	const char* Definition;
};

class r3dDX11Shader
{
public:
	r3dDX11Shader();
	~r3dDX11Shader();

	void Unload();

	bool CompileVertexFromFile(
		const char* fileName,
		const char* entryPoint = "main",
		const char* profile = "vs_4_0",
		const r3dDX11ShaderMacro* macros = NULL
	);

	bool CompilePixelFromFile(
		const char* fileName,
		const char* entryPoint = "main",
		const char* profile = "ps_4_0",
		const r3dDX11ShaderMacro* macros = NULL
	);

	bool CompileVertexFromMemory(
		const char* sourceName,
		const void* sourceData,
		unsigned int sourceSize,
		const char* entryPoint = "main",
		const char* profile = "vs_4_0",
		const r3dDX11ShaderMacro* macros = NULL
	);

	bool CompilePixelFromMemory(
		const char* sourceName,
		const void* sourceData,
		unsigned int sourceSize,
		const char* entryPoint = "main",
		const char* profile = "ps_4_0",
		const r3dDX11ShaderMacro* macros = NULL
	);

	bool CreateInputLayout(
		const D3D11_INPUT_ELEMENT_DESC* elements,
		unsigned int elementCount
	);

	bool CreatePosColorInputLayout();
	bool CreatePosTexInputLayout();

	void SetActive() const;
	void ClearActive() const;

	bool IsValidVertexShader() const;
	bool IsValidPixelShader() const;
	bool IsValidInputLayout() const;
	bool IsValidProgram() const;

	ID3D11VertexShader* GetVertexShader() const;
	ID3D11PixelShader* GetPixelShader() const;
	ID3D11InputLayout* GetInputLayout() const;

	ID3D10Blob* GetVertexByteCode() const;
	ID3D10Blob* GetPixelByteCode() const;

	const char* GetVertexFileName() const;
	const char* GetPixelFileName() const;

private:
	bool CompileFromMemoryInternal(
		int shaderType,
		const char* sourceName,
		const void* sourceData,
		unsigned int sourceSize,
		const char* entryPoint,
		const char* profile,
		const r3dDX11ShaderMacro* macros
	);

	char VertexFileName[MAX_PATH];
	char PixelFileName[MAX_PATH];

	ID3D11VertexShader* VertexShader;
	ID3D11PixelShader* PixelShader;
	ID3D11InputLayout* InputLayout;

	ID3D10Blob* VertexByteCode;
	ID3D10Blob* PixelByteCode;
};

const char* r3dDX11Shader_GetLastError();

#endif

#endif