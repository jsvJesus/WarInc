#include "r3dPCH.h"
#include "r3d.h"

#ifndef WO_SERVER
#include "r3dDX11.h"
#include "r3dDX11Shader.h"
#include "r3dDX11InputLayout.h"
#endif

extern	char	__r3dBaseShaderPath[256];
extern	char	__r3dBaseShaderCachePath[256];

extern	std::map<DWORD, std::string> _r3d_mShaderMap;

static char BIN_VER_SIG[] = "CVS100";

void WriteIncludesToFile( FILE* f1, const r3dTL::TArray< r3dString >& Includes );
void ReadIncludesFromFile( r3dFile* f1, r3dTL::TArray< r3dString >& oIncludes );

void r3dMakeCompiledShaderName(char* out, const char* shader, const char* defines)
{
  char temp[MAX_PATH];
  r3dscpy(temp, shader);
  for(char* fix = temp; *fix; fix++) {
    if(*fix == '\\' || *fix == '/')
      *fix = '_';
  }

  char drive[16], dir[ MAX_PATH], name[ MAX_PATH ], ext[ MAX_PATH ];

  _splitpath( temp, drive, dir, name, ext );

  r3d_assert( dir[ 0 ] == 0 );
  
  sprintf(out, "%s\\%s_%s.csr", __r3dBaseShaderCachePath, name, defines );
}

class r3dDXInclude : public ID3DXInclude
{
  private:
	char basePath[MAX_PATH];
	HRESULT __stdcall Open(D3DXINCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID *ppData, UINT *pBytes);
	HRESULT __stdcall Close(LPCVOID pData);
  public:
	r3dDXInclude(const char* fname);

	r3dTL::TArray< r3dString > AccumIncludes;
};

r3dDXInclude::r3dDXInclude(const char* fname)
{
  // detect base directory from fname
  r3dscpy(basePath, fname);
  for(char* p = basePath; *p; p++)
    if(*p == '/') *p = '\\';
    
  char* p = strrchr(basePath, '\\');
  if(p) *p = 0;
  
  return;
}

HRESULT r3dDXInclude::Open(D3DXINCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID *ppData, UINT *pBytes)
{
	*ppData = NULL;
	*pBytes = 0;

	char fname[MAX_PATH];
	sprintf(fname, "%s\\%s", basePath, pFileName);

	r3dFile* f = r3d_open(fname, "rb");
	if(!f) {
		r3dError("can't open shader include file %s (%s)\n", pFileName, fname);
		return E_FAIL;
	}

	if( f->size < 0 ) {
		fclose(f);
		return E_FAIL;
	}

	const size_t fileSize = static_cast<size_t>( f->size );

	char* buf = new char[fileSize + 1];

	const size_t bytesRead = fread(buf, 1, fileSize, f);
	fclose(f);

	buf[bytesRead] = 0;

	AccumIncludes.PushBack( pFileName );

	*ppData = (void*)buf;
	*pBytes = static_cast<UINT>( bytesRead );

	return S_OK;
}

HRESULT r3dDXInclude::Close(LPCVOID pData)
{
  delete[] pData;
  return S_OK;
}

char LastCompilationError[ 16384 ];

int g_IgnoreShaderCacheTimestamps = 1;

int r3dRuntimeShaderCompileAllowed()
{
	const char* cmdLine = GetCommandLineA();

	if(!cmdLine)
		return 0;

	if(strstr(cmdLine, "-compileshaders"))
		return 1;

	if(strstr(cmdLine, "-rebuildshaders"))
		return 1;

	if(strstr(cmdLine, "-shaderdev"))
		return 1;

	return 0;
}

static void r3dWriteShaderCompileErrorLog(const char* text)
{
	if(!text)
		return;

	FILE* f = fopen("ShaderCompileErrors.txt", "ab");
	if(f)
	{
		fprintf(f, "%s\r\n", text);
		fclose(f);
	}

	OutputDebugStringA(text);
	OutputDebugStringA("\n");
}

int r3dCompileShader(
	const char* fname,
	const D3DXMACRO* definesCpy,
	const char* pFunctionName,
	const char* pProfile,
	LPD3DXBUFFER* ppShader,
	r3dTL::TArray< r3dString >* pFilesIncluded
)
{
	DWORD flags = 0;

#ifdef D3DXSHADER_ENABLE_BACKWARDS_COMPATIBILITY
	flags |= D3DXSHADER_ENABLE_BACKWARDS_COMPATIBILITY;
#endif

	r3dFile* f = r3d_open(fname, "rb");
	if(!f)
	{
		_snprintf_s(
			LastCompilationError,
			sizeof(LastCompilationError),
			_TRUNCATE,
			"missing shader file %s\n",
			fname
		);

		r3dOutToLog("%s", LastCompilationError);
		r3dWriteShaderCompileErrorLog(LastCompilationError);
		return 0;
	}

	if(f->size < 0)
	{
		fclose(f);

		_snprintf_s(
			LastCompilationError,
			sizeof(LastCompilationError),
			_TRUNCATE,
			"invalid shader file size %s\n",
			fname
		);

		r3dOutToLog("%s", LastCompilationError);
		r3dWriteShaderCompileErrorLog(LastCompilationError);
		return 0;
	}

	const size_t fileSize = static_cast<size_t>(f->size);

	if(fileSize > MAXDWORD)
	{
		fclose(f);

		_snprintf_s(
			LastCompilationError,
			sizeof(LastCompilationError),
			_TRUNCATE,
			"shader file is too big %s\n",
			fname
		);

		r3dOutToLog("%s", LastCompilationError);
		r3dWriteShaderCompileErrorLog(LastCompilationError);
		return 0;
	}

	char* buf = new char[fileSize + 1];

	const size_t bytesRead = fread(buf, 1, fileSize, f);
	fclose(f);

	buf[bytesRead] = 0;

	const char* sourceCode = buf;
	UINT sourceSize = static_cast<UINT>(bytesRead);

	if(sourceSize >= 3)
	{
		const unsigned char* bytes = reinterpret_cast<const unsigned char*>(sourceCode);
		if(bytes[0] == 0xEF && bytes[1] == 0xBB && bytes[2] == 0xBF)
		{
			sourceCode += 3;
			sourceSize -= 3;
		}
	}

	ID3DXBuffer* pError = NULL;
	HRESULT hr;

	r3dDXInclude include(fname);

	hr = D3DXCompileShader(
		sourceCode,
		sourceSize,
		definesCpy,
		&include,
		pFunctionName,
		pProfile,
		flags,
		ppShader,
		&pError,
		NULL
	);

	delete[] buf;

	if(pFilesIncluded)
	{
		pFilesIncluded->Swap(include.AccumIncludes);
	}

	if(FAILED(hr))
	{
		const char* errorText = pError ? static_cast<const char*>(pError->GetBufferPointer()) : "unknown shader compiler error";

		_snprintf_s(
			LastCompilationError,
			sizeof(LastCompilationError),
			_TRUNCATE,
			"Shader compilation Error\r\nFile: %s\r\nEntry: %s\r\nProfile: %s\r\nHRESULT: 0x%08X\r\n%s\r\n",
			fname,
			pFunctionName ? pFunctionName : "NULL",
			pProfile ? pProfile : "NULL",
			(unsigned int)hr,
			errorText
		);

		r3dOutToLog("%s", LastCompilationError);
		r3dWriteShaderCompileErrorLog(LastCompilationError);

		if(pError)
			pError->Release();

		return 0;
	}

	if(pError)
		pError->Release();

	return 1;
}

r3dVertexShader::r3dVertexShader()
{
	m_pShader = NULL;
	m_dx11Shader = NULL;

	FileName[0] = 0;
	Name[0] = 0;
	bSystem = 0;
}

void r3dVertexShader :: Unload()
{
#ifndef WO_SERVER
	if(m_dx11Shader)
	{
		delete m_dx11Shader;
		m_dx11Shader = NULL;
	}
#endif

	if(m_pShader)
	{
		SetActive(0);

		R3D_ENSURE_MAIN_THREAD();

#ifdef _DEBUG

		std::map<DWORD, std::string>::iterator pIter = _r3d_mShaderMap.find((DWORD)m_pShader);

		int refcnt = m_pShader->Release();
		if(refcnt) r3dOutToLog("WARNING: shader '%s' have refcnt %d when unloading\n", (pIter != _r3d_mShaderMap.end()) ? pIter->second.c_str() : "<unknown>", refcnt);

		if(pIter != _r3d_mShaderMap.end()) {
			_r3d_mShaderMap.erase(pIter);
		}

		//r3dOutToLog("Shader[%x] removing\n", m_pShader);
		m_pShader = NULL;
#else
		SAFE_RELEASE(m_pShader);
#endif
	}
}

int r3dVertexShader :: LoadBinaryCache(const char* FName, const char* Path, const char* defines)
{
	g_IgnoreShaderCacheTimestamps = r3dRuntimeShaderCompileAllowed() ? 0 : 1;
	char FName2[512];
	r3dMakeCompiledShaderName(FName2, FName, defines);

	if(r3d_access(FName2, 0) != 0)
		return 0;

	bool sourceAvailable = r3d_access(Path, 0) == 0;

	if(sourceAvailable && !g_IgnoreShaderCacheTimestamps)
	{
		if(r3d_fstamp(Path) > r3d_fstamp(FName2))
			return 0;
	}

	r3dFile* f = r3d_open(FName2, "rb");
	if(!f)
		return 0;

	if(!sourceAvailable && !f)
	{
		r3dError("r3dVertexShader :: LoadBinaryCache: both source and cached version are missing(%s)!\n", Path);
	}

	char CheckSig[sizeof BIN_VER_SIG];
	fread(CheckSig, sizeof CheckSig, 1, f);

	if(memcmp(CheckSig, BIN_VER_SIG, sizeof BIN_VER_SIG))
	{
		if(sourceAvailable)
		{
			r3dError("r3dVertexShader::LoadBinaryCache: source code is missing and cached version is unknown(%s)!\n", Path);
		}
		else
		{
			r3dOutToLog("r3dVertexShader::LoadBinaryCache: unsupported binary version!\n");
		}

		fclose(f);
		return 0;
	}

	r3dTL::TArray< r3dString > Includes;

	ReadIncludesFromFile(f, Includes);

	if(sourceAvailable && !g_IgnoreShaderCacheTimestamps)
	{
		for(uint32_t i = 0, e = Includes.Count(); i < e; i++)
		{
			char IncludeName[512];
			char drive[16], dir[512], name[512], ext[512];

			_splitpath(FName, drive, dir, name, ext);

			sprintf(IncludeName, "%s\\%s\\%s", __r3dBaseShaderPath, dir, Includes[i].c_str());

			if(r3d_fstamp(IncludeName) > r3d_fstamp(FName2))
			{
				fclose(f);
				return 0;
			}
		}
	}

	//if(g_IgnoreShaderCacheTimestamps)
	//{
		//r3dOutToLog("ShaderCache fallback: using stale vertex shader cache %s\n", FName2);
	//}

	size_t codeSize = f->size - ftell(f);

	char* shaderCode = new char[codeSize + 1];
	fread(shaderCode, codeSize, 1, f);
	fclose(f);

	Unload();

	R3D_ENSURE_MAIN_THREAD();

	HRESULT hr;
	hr = r3dRenderer->pd3ddev->CreateVertexShader((DWORD*)shaderCode, &m_pShader);

	r3dRenderer->CheckOutOfMemory(hr);

	delete[] shaderCode;

	if(FAILED(hr))
	{
		r3dOutToLog("Failed to create Vertex shader '%s' : ", FName);

		if(hr == D3DERR_INVALIDCALL)      r3dOutToLog("Error: Invalid Call\n");
		if(hr == E_OUTOFMEMORY)           r3dOutToLog("Error: Out of memory\n");
		if(hr == D3DERR_OUTOFVIDEOMEMORY) r3dOutToLog("Error: Out of Video Memory\n");

		m_pShader = NULL;
		return 0;
	}

	return 1;
}

void  r3dVertexShader :: SaveBinaryCache(const char* FName,    ID3DXBuffer*	pCode, const char* defines, const r3dTL::TArray< r3dString >& Includes )
{
	// return if we don't have cache dir
	if(r3d_access( R3D_BASE_SHADER_CACHE_PATH, 0 ) != 0)
		return;

	if( r3d_access( __r3dBaseShaderCachePath, 0 ) )
		mkdir( __r3dBaseShaderCachePath );

	if( r3d_access( __r3dBaseShaderCachePath, 0 ) )
		return;

	char FName2[512];
	r3dMakeCompiledShaderName(FName2, FName, defines);
	FILE* f1 = fopen(FName2, "wb");
	if (f1)
	{
		fwrite ( BIN_VER_SIG, sizeof BIN_VER_SIG, 1, f1 );

		WriteIncludesToFile( f1, Includes );

		fwrite( pCode->GetBufferPointer(), pCode->GetBufferSize(), 1, f1 );
		fclose(f1);
	}
	else
	{
		r3dOutToLog("ShaderCache: can't open %s for writing\n", FName2);
	}
}

int r3dVertexShader :: Load(const char* FName, int Type )
{
	r3dTL::TArray <D3DXMACRO> defines;
	return Load ( FName, Type, defines );
}

int r3dVertexShader :: Load(const char* FName, int Type, const r3dTL::TArray <D3DXMACRO>& defines)
{
	sprintf(FileName, "%s\\%s", __r3dBaseShaderPath, FName);

	char defines_string[256];
	memset(defines_string, 0, 256);

	for(unsigned int i = 0; i < defines.Count(); ++i)
	{
		sprintf(&defines_string[strlen(defines_string)], "%s=%s,", defines[i].Name, defines[i].Definition);
	}

	if(!LoadBinaryCache(FName, FileName, defines_string))
	{
		ID3DXBuffer* pCode = NULL;
		HRESULT hr;

		r3dTL::TArray <D3DXMACRO> definesCpy(defines);

		definesCpy.PushBack(D3DXMACRO());
		definesCpy[definesCpy.Count() - 1].Name = "VERTEX_SHADER";
		definesCpy[definesCpy.Count() - 1].Definition = "1";

		definesCpy.PushBack(D3DXMACRO());
		definesCpy[definesCpy.Count() - 1].Name = "PIXEL_SHADER";
		definesCpy[definesCpy.Count() - 1].Definition = "0";

		definesCpy.PushBack(D3DXMACRO());
		definesCpy[definesCpy.Count() - 1].Name = 0;
		definesCpy[definesCpy.Count() - 1].Definition = 0;

		r3dTL::TArray< r3dString > Includes;

		if(!r3dCompileShader(FileName, &definesCpy[0], "main", r3dRenderer->VertexShaderProfileName, &pCode, &Includes))
		{
			r3dOutToLog("Vertex shader compile failed. Trying stale cache fallback: %s\n", FileName);

			g_IgnoreShaderCacheTimestamps = 1;
			const int fallbackLoaded = LoadBinaryCache(FName, FileName, defines_string);
			g_IgnoreShaderCacheTimestamps = 0;

			if(!fallbackLoaded)
				return 0;
		}
		else
		{
			Unload();

			R3D_ENSURE_MAIN_THREAD();

			hr = r3dRenderer->pd3ddev->CreateVertexShader((DWORD*)pCode->GetBufferPointer(), &m_pShader);

			r3dRenderer->CheckOutOfMemory(hr);

			if(FAILED(hr))
			{
				r3dOutToLog("Failed to create Vertex shader '%s' : \n", FileName);

				if(hr == D3DERR_INVALIDCALL)      r3dOutToLog("Error: Invalid Call\n");
				if(hr == E_OUTOFMEMORY)           r3dOutToLog("Error: Out of memory\n");
				if(hr == D3DERR_OUTOFVIDEOMEMORY) r3dOutToLog("Error: Out of Video Memory\n");
				if(hr == 0x88760028)              r3dOutToLog("DDERR_CURRENTLYNOTAVAIL\n");

				r3dOutToLog("error=%d\n", hr);

				m_pShader = NULL;
				pCode->Release();
				return 0;
			}

#ifdef _DEBUG
			_r3d_mShaderMap.insert(std::pair<DWORD, std::string>((DWORD)m_pShader, FileName));
#endif

			SaveBinaryCache(FName, pCode, defines_string, Includes);

			pCode->Release();
		}
	}

	Macros.Resize(defines.Count());

	for(uint32_t i = 0, e = defines.Count(); i < e; i++)
	{
		ShaderMacro& macro = Macros[i];
		const D3DXMACRO& dxmacro = defines[i];

		macro.Name = dxmacro.Name;
		macro.Definition = dxmacro.Definition;
	}

	LoadDX11(FName, Type, defines);

	return 1;
}

#ifndef WO_SERVER

static void r3dDX11_BuildVertexShaderMacros(
	r3dTL::TArray<r3dDX11ShaderMacro>& outMacros,
	const r3dTL::TArray<D3DXMACRO>& defines
)
{
	outMacros.Clear();

	for(unsigned int i = 0; i < defines.Count(); ++i)
	{
		if(!defines[i].Name)
			continue;

		r3dDX11ShaderMacro macro;
		macro.Name = defines[i].Name;
		macro.Definition = defines[i].Definition ? defines[i].Definition : "1";
		outMacros.PushBack(macro);
	}

	r3dDX11ShaderMacro vertexMacro;
	vertexMacro.Name = "VERTEX_SHADER";
	vertexMacro.Definition = "1";
	outMacros.PushBack(vertexMacro);

	r3dDX11ShaderMacro pixelMacro;
	pixelMacro.Name = "PIXEL_SHADER";
	pixelMacro.Definition = "0";
	outMacros.PushBack(pixelMacro);

	r3dDX11ShaderMacro nullMacro;
	nullMacro.Name = NULL;
	nullMacro.Definition = NULL;
	outMacros.PushBack(nullMacro);
}

#endif

int r3dVertexShader :: LoadDX11(const char* FName, int Type)
{
	r3dTL::TArray<D3DXMACRO> defines;
	return LoadDX11(FName, Type, defines);
}

int r3dVertexShader :: LoadDX11(const char* FName, int Type, const r3dTL::TArray<D3DXMACRO>& defines)
{
	(void)Type;

#ifndef WO_SERVER
	if(!g_r3dDX11.IsInitialized())
		return 0;

	r3dTL::TArray<r3dDX11ShaderMacro> dx11Defines;
	r3dDX11_BuildVertexShaderMacros(dx11Defines, defines);

	if(!m_dx11Shader)
		m_dx11Shader = new r3dDX11Shader();

	if(!m_dx11Shader->CompileVertexFromFile(FName, "main", "vs_4_0", dx11Defines.Count() ? &dx11Defines[0] : NULL))
	{
		r3dOutToLog("DX11: failed to compile vertex shader '%s'\n%s\n", FName, r3dDX11Shader_GetLastError());

		delete m_dx11Shader;
		m_dx11Shader = NULL;

		return 0;
	}

	r3dOutToLog("DX11: vertex shader compiled '%s'\n", FName);

	return 1;
#else
	return 0;
#endif
}

bool r3dVertexShader :: CreateDX11PosTexInputLayout()
{
#ifndef WO_SERVER
	if(!m_dx11Shader)
		return false;

	return m_dx11Shader->CreatePosTexInputLayout();
#else
	return false;
#endif
}

bool r3dVertexShader :: CreateDX11PosColorInputLayout()
{
#ifndef WO_SERVER
	if(!m_dx11Shader)
		return false;

	return m_dx11Shader->CreatePosColorInputLayout();
#else
	return false;
#endif
}

bool r3dVertexShader :: SetDX11InputLayoutFromD3D9Decl(const void* d3d9Declaration)
{
#ifndef WO_SERVER
	if(!m_dx11Shader)
		return false;

	return g_r3dDX11InputLayouts.Set(d3d9Declaration, m_dx11Shader);
#else
	return false;
#endif
}

r3dDX11Shader* r3dVertexShader :: GetDX11Shader() const
{
	return m_dx11Shader;
}

void r3dVertexShader :: SetActive(int Act)
{
#ifndef WO_SERVER
	if(g_r3dDX11.IsInitialized())
	{
		if(m_dx11Shader)
		{
			if(Act)
				m_dx11Shader->SetActive();
			else
				m_dx11Shader->ClearActive();
		}

		return;
	}
#endif

	if (!m_pShader ) return;

	if(Act)
	{
		if( HRESULT hr = d3dc._SetVertexShader(m_pShader) != S_OK )
		{
			r3dError( "Failed to set %s vertex shader, err=%d", this->Name, hr ) ;
		}
	}
	else
	{
		D3D_V( d3dc._SetVertexShader(0) );
	}
}

void WriteIncludesToFile( FILE* f1, const r3dTL::TArray< r3dString >& Includes )
{
	uint16_t numIncludes = (uint16_t)Includes.Count();
	fwrite( &numIncludes, sizeof numIncludes, 1, f1 );

	for( uint32_t i = 0, e = numIncludes; i < e; i ++ )
	{
		const r3dString& s = Includes[ i ];
		uint16_t slen = (uint16_t) s.Length();

		fwrite( &slen, sizeof slen, 1, f1 );

		fwrite( s.c_str(), slen, 1, f1 );
	}
}

void ReadIncludesFromFile( r3dFile* f1, r3dTL::TArray< r3dString >& oIncludes )
{
	uint16_t count;

	fread( &count, sizeof count, 1, f1 );

	oIncludes.Reserve( count );

	for( uint32_t i = 0, e = count; i < count; i ++ )
	{
		uint16_t slen;

		fread( &slen, sizeof slen, 1, f1 );

		char* str = (char*)alloca( slen + 1 );
		if(str)
		{
			fread( str, slen, 1, f1 );
			str[ slen ] = 0;
			oIncludes.PushBack( str );
		}
		else
			r3d_assert(str && "Out of memory");
	}
}

