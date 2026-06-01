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
#include <d3dcompiler.h>
#pragma warning(pop)

#include "r3d.h"
#include "r3dDX11.h"
#include "r3dDX11Shader.h"

#pragma comment(lib, "d3dcompiler.lib")

extern char __r3dBaseShaderPath[256];

static char g_r3dDX11ShaderLastError[16384];

enum
{
	R3D_DX11_SHADER_VERTEX = 0,
	R3D_DX11_SHADER_PIXEL = 1
};

template<typename T>
static void r3dDX11Shader_Release(T*& ptr)
{
	if(ptr)
	{
		ptr->Release();
		ptr = NULL;
	}
}

static void r3dDX11Shader_SetLastError(const char* text)
{
	if(!text)
		text = "unknown DX11 shader error";

	_snprintf_s(
		g_r3dDX11ShaderLastError,
		sizeof(g_r3dDX11ShaderLastError),
		_TRUNCATE,
		"%s",
		text
	);

	r3dOutToLog("%s\n", g_r3dDX11ShaderLastError);

	FILE* f = fopen("DX11ShaderCompileErrors.txt", "ab");
	if(f)
	{
		fprintf(f, "%s\r\n", g_r3dDX11ShaderLastError);
		fclose(f);
	}

	OutputDebugStringA(g_r3dDX11ShaderLastError);
	OutputDebugStringA("\n");
}

static void r3dDX11Shader_SetLastHRError(
	const char* text,
	HRESULT hr,
	const char* sourceName,
	const char* entryPoint,
	const char* profile,
	ID3D10Blob* errors
)
{
	const char* compilerText = "";

	if(errors && errors->GetBufferPointer())
		compilerText = (const char*)errors->GetBufferPointer();

	_snprintf_s(
		g_r3dDX11ShaderLastError,
		sizeof(g_r3dDX11ShaderLastError),
		_TRUNCATE,
		"DX11Shader: %s failed\r\nFile: %s\r\nEntry: %s\r\nProfile: %s\r\nHRESULT: 0x%08X\r\n%s\r\n",
		text ? text : "operation",
		sourceName ? sourceName : "NULL",
		entryPoint ? entryPoint : "NULL",
		profile ? profile : "NULL",
		(unsigned int)hr,
		compilerText
	);

	r3dOutToLog("%s", g_r3dDX11ShaderLastError);

	FILE* f = fopen("DX11ShaderCompileErrors.txt", "ab");
	if(f)
	{
		fprintf(f, "%s\r\n", g_r3dDX11ShaderLastError);
		fclose(f);
	}

	OutputDebugStringA(g_r3dDX11ShaderLastError);
	OutputDebugStringA("\n");
}

static const char* r3dDX11Shader_NormalizeProfile(int shaderType, const char* profile)
{
	if(!profile || !profile[0])
		return shaderType == R3D_DX11_SHADER_VERTEX ? "vs_4_0" : "ps_4_0";

	if(!_strnicmp(profile, "vs_", 3))
		return "vs_4_0";

	if(!_strnicmp(profile, "ps_", 3))
		return "ps_4_0";

	return profile;
}

static const char r3dDX11ShaderCompatibilityPrelude[] =
	"#ifndef R3D_DX11\r\n"
	"#define R3D_DX11 1\r\n"
	"#endif\r\n"
	"#ifndef DX11\r\n"
	"#define DX11 1\r\n"
	"#endif\r\n"
	"#ifndef SM4\r\n"
	"#define SM4 1\r\n"
	"#endif\r\n"
	"#ifndef R3D_SHADER_MODEL\r\n"
	"#define R3D_SHADER_MODEL 40\r\n"
	"#endif\r\n"
	"#line 1\r\n";

static int r3dDX11Shader_IsAbsolutePath(const char* fileName)
{
	if(!fileName || !fileName[0])
		return 0;

	if(fileName[0] == '\\' || fileName[0] == '/')
		return 1;

	if(fileName[0] && fileName[1] == ':')
		return 1;

	return 0;
}

static void r3dDX11Shader_ResolveShaderPath(char* out, size_t outSize, const char* fileName)
{
	if(!out || !outSize)
		return;

	out[0] = 0;

	if(!fileName || !fileName[0])
		return;

	if(r3dDX11Shader_IsAbsolutePath(fileName))
	{
		_snprintf_s(out, outSize, _TRUNCATE, "%s", fileName);
		return;
	}

	if(__r3dBaseShaderPath[0])
	{
		_snprintf_s(out, outSize, _TRUNCATE, "%s\\%s", __r3dBaseShaderPath, fileName);
		return;
	}

	_snprintf_s(out, outSize, _TRUNCATE, "%s", fileName);
}

static bool r3dDX11Shader_ReadFile(const char* fileName, char** outData, unsigned int* outSize)
{
	if(outData)
		*outData = NULL;

	if(outSize)
		*outSize = 0;

	if(!fileName || !fileName[0] || !outData || !outSize)
	{
		r3dDX11Shader_SetLastError("DX11Shader: invalid file read arguments");
		return false;
	}

	r3dFile* f = r3d_open(fileName, "rb");
	if(!f)
	{
		char msg[MAX_PATH + 128];
		_snprintf_s(msg, sizeof(msg), _TRUNCATE, "DX11Shader: can't open shader file '%s'", fileName);
		r3dDX11Shader_SetLastError(msg);
		return false;
	}

	if(f->size < 0)
	{
		fclose(f);

		char msg[MAX_PATH + 128];
		_snprintf_s(msg, sizeof(msg), _TRUNCATE, "DX11Shader: invalid shader file size '%s'", fileName);
		r3dDX11Shader_SetLastError(msg);
		return false;
	}

	const size_t fileSize = static_cast<size_t>(f->size);

	if(fileSize > 0xFFFFFFFFu)
	{
		fclose(f);

		char msg[MAX_PATH + 128];
		_snprintf_s(msg, sizeof(msg), _TRUNCATE, "DX11Shader: shader file is too big '%s'", fileName);
		r3dDX11Shader_SetLastError(msg);
		return false;
	}

	char* data = new char[fileSize + 1];
	const size_t bytesRead = fread(data, 1, fileSize, f);
	fclose(f);

	data[bytesRead] = 0;

	*outData = data;
	*outSize = static_cast<unsigned int>(bytesRead);

	return true;
}

class r3dDX11ShaderIncludeHandler : public ID3DInclude
{
public:
	r3dDX11ShaderIncludeHandler(const char* sourceName)
	{
		BasePath[0] = 0;

		if(sourceName && sourceName[0])
		{
			_snprintf_s(BasePath, sizeof(BasePath), _TRUNCATE, "%s", sourceName);

			for(char* p = BasePath; *p; ++p)
			{
				if(*p == '/')
					*p = '\\';
			}

			char* slash = strrchr(BasePath, '\\');
			if(slash)
				*slash = 0;
			else
				_snprintf_s(BasePath, sizeof(BasePath), _TRUNCATE, ".");
		}
		else
		{
			_snprintf_s(BasePath, sizeof(BasePath), _TRUNCATE, ".");
		}
	}

	HRESULT STDMETHODCALLTYPE Open(
		D3D_INCLUDE_TYPE IncludeType,
		LPCSTR pFileName,
		LPCVOID pParentData,
		LPCVOID* ppData,
		UINT* pBytes
	)
	{
		(void)IncludeType;
		(void)pParentData;

		if(ppData)
			*ppData = NULL;

		if(pBytes)
			*pBytes = 0;

		if(!pFileName || !ppData || !pBytes)
			return E_FAIL;

		char includeName[MAX_PATH];
		_snprintf_s(includeName, sizeof(includeName), _TRUNCATE, "%s\\%s", BasePath, pFileName);

		char* data = NULL;
		unsigned int size = 0;

		if(!r3dDX11Shader_ReadFile(includeName, &data, &size))
		{
			if(!r3dDX11Shader_ReadFile(pFileName, &data, &size))
				return E_FAIL;
		}

		*ppData = data;
		*pBytes = size;

		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE Close(LPCVOID pData)
	{
		delete[] (const char*)pData;
		return S_OK;
	}

private:
	char BasePath[MAX_PATH];
};

const char* r3dDX11Shader_GetLastError()
{
	return g_r3dDX11ShaderLastError;
}

r3dDX11Shader::r3dDX11Shader()
{
	VertexFileName[0] = 0;
	PixelFileName[0] = 0;

	VertexShader = NULL;
	PixelShader = NULL;
	InputLayout = NULL;

	VertexByteCode = NULL;
	PixelByteCode = NULL;
}

r3dDX11Shader::~r3dDX11Shader()
{
	Unload();
}

void r3dDX11Shader::Unload()
{
	ClearActive();

	r3dDX11Shader_Release(InputLayout);
	r3dDX11Shader_Release(PixelShader);
	r3dDX11Shader_Release(VertexShader);
	r3dDX11Shader_Release(PixelByteCode);
	r3dDX11Shader_Release(VertexByteCode);

	VertexFileName[0] = 0;
	PixelFileName[0] = 0;
}

bool r3dDX11Shader::CompileVertexFromFile(
	const char* fileName,
	const char* entryPoint,
	const char* profile,
	const r3dDX11ShaderMacro* macros
)
{
	char fullPath[MAX_PATH];
	r3dDX11Shader_ResolveShaderPath(fullPath, sizeof(fullPath), fileName);

	char* sourceData = NULL;
	unsigned int sourceSize = 0;

	if(!r3dDX11Shader_ReadFile(fullPath, &sourceData, &sourceSize))
		return false;

	const bool result = CompileVertexFromMemory(
		fullPath,
		sourceData,
		sourceSize,
		entryPoint,
		profile,
		macros
	);

	delete[] sourceData;

	if(result)
		_snprintf_s(VertexFileName, sizeof(VertexFileName), _TRUNCATE, "%s", fullPath);

	return result;
}

bool r3dDX11Shader::CompilePixelFromFile(
	const char* fileName,
	const char* entryPoint,
	const char* profile,
	const r3dDX11ShaderMacro* macros
)
{
	char fullPath[MAX_PATH];
	r3dDX11Shader_ResolveShaderPath(fullPath, sizeof(fullPath), fileName);

	char* sourceData = NULL;
	unsigned int sourceSize = 0;

	if(!r3dDX11Shader_ReadFile(fullPath, &sourceData, &sourceSize))
		return false;

	const bool result = CompilePixelFromMemory(
		fullPath,
		sourceData,
		sourceSize,
		entryPoint,
		profile,
		macros
	);

	delete[] sourceData;

	if(result)
		_snprintf_s(PixelFileName, sizeof(PixelFileName), _TRUNCATE, "%s", fullPath);

	return result;
}

bool r3dDX11Shader::CompileVertexFromMemory(
	const char* sourceName,
	const void* sourceData,
	unsigned int sourceSize,
	const char* entryPoint,
	const char* profile,
	const r3dDX11ShaderMacro* macros
)
{
	return CompileFromMemoryInternal(
		R3D_DX11_SHADER_VERTEX,
		sourceName,
		sourceData,
		sourceSize,
		entryPoint,
		profile,
		macros
	);
}

bool r3dDX11Shader::CompilePixelFromMemory(
	const char* sourceName,
	const void* sourceData,
	unsigned int sourceSize,
	const char* entryPoint,
	const char* profile,
	const r3dDX11ShaderMacro* macros
)
{
	return CompileFromMemoryInternal(
		R3D_DX11_SHADER_PIXEL,
		sourceName,
		sourceData,
		sourceSize,
		entryPoint,
		profile,
		macros
	);
}

bool r3dDX11Shader::CompileFromMemoryInternal(
	int shaderType,
	const char* sourceName,
	const void* sourceData,
	unsigned int sourceSize,
	const char* entryPoint,
	const char* profile,
	const r3dDX11ShaderMacro* macros
)
{
	if(!g_r3dDX11.IsInitialized() || !g_r3dDX11.GetDevice())
	{
		r3dDX11Shader_SetLastError("DX11Shader: DX11 renderer is not initialized");
		return false;
	}

	if(!sourceData || !sourceSize)
	{
		r3dDX11Shader_SetLastError("DX11Shader: empty shader source");
		return false;
	}

	if(!entryPoint || !entryPoint[0])
		entryPoint = "main";

	const char* dx11Profile = r3dDX11Shader_NormalizeProfile(shaderType, profile);

	const char* sourceCode = (const char*)sourceData;
	unsigned int realSourceSize = sourceSize;

	if(realSourceSize >= 3)
	{
		const unsigned char* bytes = (const unsigned char*)sourceCode;
		if(bytes[0] == 0xEF && bytes[1] == 0xBB && bytes[2] == 0xBF)
		{
			sourceCode += 3;
			realSourceSize -= 3;
		}
	}

	const unsigned int preludeSize = sizeof(r3dDX11ShaderCompatibilityPrelude) - 1;
	const unsigned int compileSourceSize = preludeSize + realSourceSize;

	char* compileSource = new char[compileSourceSize + 1];
	memcpy(compileSource, r3dDX11ShaderCompatibilityPrelude, preludeSize);
	memcpy(compileSource + preludeSize, sourceCode, realSourceSize);
	compileSource[compileSourceSize] = 0;

	UINT flags = 0;

#if defined(D3DCOMPILE_ENABLE_BACKWARDS_COMPATIBILITY)
	flags |= D3DCOMPILE_ENABLE_BACKWARDS_COMPATIBILITY;
#endif

#if defined(_DEBUG)
	flags |= D3DCOMPILE_DEBUG;
	flags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	flags |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
#endif

	r3dDX11ShaderIncludeHandler includeHandler(sourceName);

	ID3D10Blob* shaderBlob = NULL;
	ID3D10Blob* errors = NULL;

	HRESULT hr = D3DCompile(
		compileSource,
		compileSourceSize,
		sourceName ? sourceName : "memory",
		(const D3D_SHADER_MACRO*)macros,
		&includeHandler,
		entryPoint,
		dx11Profile,
		flags,
		0,
		&shaderBlob,
		&errors
	);

	delete[] compileSource;

	if(FAILED(hr))
	{
		r3dDX11Shader_SetLastHRError(
			"D3DCompile",
			hr,
			sourceName,
			entryPoint,
			dx11Profile,
			errors
		);

		r3dDX11Shader_Release(errors);
		r3dDX11Shader_Release(shaderBlob);
		return false;
	}

	r3dDX11Shader_Release(errors);

	ID3D11Device* device = g_r3dDX11.GetDevice();

	if(shaderType == R3D_DX11_SHADER_VERTEX)
	{
		ID3D11VertexShader* newShader = NULL;

		hr = device->CreateVertexShader(
			shaderBlob->GetBufferPointer(),
			shaderBlob->GetBufferSize(),
			NULL,
			&newShader
		);

		if(FAILED(hr))
		{
			r3dDX11Shader_SetLastHRError(
				"CreateVertexShader",
				hr,
				sourceName,
				entryPoint,
				dx11Profile,
				NULL
			);

			r3dDX11Shader_Release(shaderBlob);
			return false;
		}

		r3dDX11Shader_Release(InputLayout);
		r3dDX11Shader_Release(VertexShader);
		r3dDX11Shader_Release(VertexByteCode);

		VertexShader = newShader;
		VertexByteCode = shaderBlob;

		_snprintf_s(
			VertexFileName,
			sizeof(VertexFileName),
			_TRUNCATE,
			"%s",
			sourceName ? sourceName : "memory"
		);

		return true;
	}

	ID3D11PixelShader* newShader = NULL;

	hr = device->CreatePixelShader(
		shaderBlob->GetBufferPointer(),
		shaderBlob->GetBufferSize(),
		NULL,
		&newShader
	);

	if(FAILED(hr))
	{
		r3dDX11Shader_SetLastHRError(
			"CreatePixelShader",
			hr,
			sourceName,
			entryPoint,
			dx11Profile,
			NULL
		);

		r3dDX11Shader_Release(shaderBlob);
		return false;
	}

	r3dDX11Shader_Release(PixelShader);
	r3dDX11Shader_Release(PixelByteCode);

	PixelShader = newShader;
	PixelByteCode = shaderBlob;

	_snprintf_s(
		PixelFileName,
		sizeof(PixelFileName),
		_TRUNCATE,
		"%s",
		sourceName ? sourceName : "memory"
	);

	return true;
}

bool r3dDX11Shader::CreateInputLayout(
	const D3D11_INPUT_ELEMENT_DESC* elements,
	unsigned int elementCount
)
{
	if(!g_r3dDX11.IsInitialized() || !g_r3dDX11.GetDevice())
	{
		r3dDX11Shader_SetLastError("DX11Shader: DX11 renderer is not initialized");
		return false;
	}

	if(!VertexByteCode)
	{
		r3dDX11Shader_SetLastError("DX11Shader: vertex bytecode is missing, compile VS before input layout");
		return false;
	}

	if(!elements || !elementCount)
	{
		r3dDX11Shader_SetLastError("DX11Shader: invalid input layout");
		return false;
	}

	ID3D11InputLayout* newLayout = NULL;

	HRESULT hr = g_r3dDX11.GetDevice()->CreateInputLayout(
		elements,
		elementCount,
		VertexByteCode->GetBufferPointer(),
		VertexByteCode->GetBufferSize(),
		&newLayout
	);

	if(FAILED(hr))
	{
		r3dDX11Shader_SetLastHRError(
			"CreateInputLayout",
			hr,
			VertexFileName,
			"input-layout",
			"vs-bytecode",
			NULL
		);

		return false;
	}

	r3dDX11Shader_Release(InputLayout);
	InputLayout = newLayout;

	return true;
}

bool r3dDX11Shader::CreatePosColorInputLayout()
{
	D3D11_INPUT_ELEMENT_DESC layout[2];

	layout[0].SemanticName = "POSITION";
	layout[0].SemanticIndex = 0;
	layout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	layout[0].InputSlot = 0;
	layout[0].AlignedByteOffset = 0;
	layout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	layout[0].InstanceDataStepRate = 0;

	layout[1].SemanticName = "COLOR";
	layout[1].SemanticIndex = 0;
	layout[1].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	layout[1].InputSlot = 0;
	layout[1].AlignedByteOffset = 12;
	layout[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	layout[1].InstanceDataStepRate = 0;

	return CreateInputLayout(layout, 2);
}

bool r3dDX11Shader::CreatePosTexInputLayout()
{
	D3D11_INPUT_ELEMENT_DESC layout[2];

	layout[0].SemanticName = "POSITION";
	layout[0].SemanticIndex = 0;
	layout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	layout[0].InputSlot = 0;
	layout[0].AlignedByteOffset = 0;
	layout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	layout[0].InstanceDataStepRate = 0;

	layout[1].SemanticName = "TEXCOORD";
	layout[1].SemanticIndex = 0;
	layout[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	layout[1].InputSlot = 0;
	layout[1].AlignedByteOffset = 12;
	layout[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	layout[1].InstanceDataStepRate = 0;

	return CreateInputLayout(layout, 2);
}

void r3dDX11Shader::SetActive() const
{
	if(!g_r3dDX11.IsInitialized())
		return;

	ID3D11DeviceContext* context = g_r3dDX11.GetContext();
	if(!context)
		return;

	if(InputLayout)
		context->IASetInputLayout(InputLayout);

	if(VertexShader)
		context->VSSetShader(VertexShader, NULL, 0);

	if(PixelShader)
		context->PSSetShader(PixelShader, NULL, 0);
}

void r3dDX11Shader::ClearActive() const
{
	if(!g_r3dDX11.IsInitialized())
		return;

	ID3D11DeviceContext* context = g_r3dDX11.GetContext();
	if(!context)
		return;

	ID3D11VertexShader* nullVS = NULL;
	ID3D11PixelShader* nullPS = NULL;

	context->VSSetShader(nullVS, NULL, 0);
	context->PSSetShader(nullPS, NULL, 0);
	context->IASetInputLayout(NULL);
}

bool r3dDX11Shader::IsValidVertexShader() const
{
	return VertexShader != NULL;
}

bool r3dDX11Shader::IsValidPixelShader() const
{
	return PixelShader != NULL;
}

bool r3dDX11Shader::IsValidInputLayout() const
{
	return InputLayout != NULL;
}

bool r3dDX11Shader::IsValidProgram() const
{
	return VertexShader && PixelShader;
}

ID3D11VertexShader* r3dDX11Shader::GetVertexShader() const
{
	return VertexShader;
}

ID3D11PixelShader* r3dDX11Shader::GetPixelShader() const
{
	return PixelShader;
}

ID3D11InputLayout* r3dDX11Shader::GetInputLayout() const
{
	return InputLayout;
}

ID3D10Blob* r3dDX11Shader::GetVertexByteCode() const
{
	return VertexByteCode;
}

ID3D10Blob* r3dDX11Shader::GetPixelByteCode() const
{
	return PixelByteCode;
}

const char* r3dDX11Shader::GetVertexFileName() const
{
	return VertexFileName;
}

const char* r3dDX11Shader::GetPixelFileName() const
{
	return PixelFileName;
}

#endif
