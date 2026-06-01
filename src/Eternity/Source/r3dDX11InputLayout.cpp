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
#include <d3d9.h>
#include <d3d11.h>
#pragma warning(pop)

#include "r3d.h"
#include "r3dDX11.h"
#include "r3dDX11Shader.h"
#include "r3dDX11InputLayout.h"

#define R3D_DX11_MAX_DECL_ELEMENTS 32

template<typename T>
static void r3dDX11InputLayout_Release(T*& ptr)
{
	if(ptr)
	{
		ptr->Release();
		ptr = NULL;
	}
}

static void r3dDX11InputLayout_LogHR(const char* text, HRESULT hr)
{
	r3dOutToLog("DX11InputLayout: %s failed, HRESULT=0x%08X\n", text, (unsigned int)hr);
}

static unsigned int r3dDX11InputLayout_HashData(unsigned int hash, const void* data, unsigned int size)
{
	const unsigned char* bytes = (const unsigned char*)data;

	for(unsigned int i = 0; i < size; ++i)
	{
		hash ^= bytes[i];
		hash *= 16777619u;
	}

	return hash;
}

static const char* r3dDX11InputLayout_GetSemanticName(unsigned char usage)
{
	switch(usage)
	{
	case D3DDECLUSAGE_POSITION:
		return "POSITION";

	case D3DDECLUSAGE_BLENDWEIGHT:
		return "BLENDWEIGHT";

	case D3DDECLUSAGE_BLENDINDICES:
		return "BLENDINDICES";

	case D3DDECLUSAGE_NORMAL:
		return "NORMAL";

	case D3DDECLUSAGE_PSIZE:
		return "PSIZE";

	case D3DDECLUSAGE_TEXCOORD:
		return "TEXCOORD";

	case D3DDECLUSAGE_TANGENT:
		return "TANGENT";

	case D3DDECLUSAGE_BINORMAL:
		return "BINORMAL";

	case D3DDECLUSAGE_TESSFACTOR:
		return "TESSFACTOR";

	case D3DDECLUSAGE_POSITIONT:
		return "POSITION";

	case D3DDECLUSAGE_COLOR:
		return "COLOR";

	case D3DDECLUSAGE_FOG:
		return "FOG";

	case D3DDECLUSAGE_DEPTH:
		return "DEPTH";

	case D3DDECLUSAGE_SAMPLE:
		return "SAMPLE";

	default:
		return "TEXCOORD";
	}
}

static bool r3dDX11InputLayout_ConvertDeclType(unsigned char type, DXGI_FORMAT* outFormat)
{
	if(!outFormat)
		return false;

	switch(type)
	{
	case D3DDECLTYPE_FLOAT1:
		*outFormat = DXGI_FORMAT_R32_FLOAT;
		return true;

	case D3DDECLTYPE_FLOAT2:
		*outFormat = DXGI_FORMAT_R32G32_FLOAT;
		return true;

	case D3DDECLTYPE_FLOAT3:
		*outFormat = DXGI_FORMAT_R32G32B32_FLOAT;
		return true;

	case D3DDECLTYPE_FLOAT4:
		*outFormat = DXGI_FORMAT_R32G32B32A32_FLOAT;
		return true;

	case D3DDECLTYPE_D3DCOLOR:
		*outFormat = DXGI_FORMAT_B8G8R8A8_UNORM;
		return true;

	case D3DDECLTYPE_UBYTE4:
		*outFormat = DXGI_FORMAT_R8G8B8A8_UINT;
		return true;

	case D3DDECLTYPE_SHORT2:
		*outFormat = DXGI_FORMAT_R16G16_SINT;
		return true;

	case D3DDECLTYPE_SHORT4:
		*outFormat = DXGI_FORMAT_R16G16B16A16_SINT;
		return true;

	case D3DDECLTYPE_UBYTE4N:
		*outFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
		return true;

	case D3DDECLTYPE_SHORT2N:
		*outFormat = DXGI_FORMAT_R16G16_SNORM;
		return true;

	case D3DDECLTYPE_SHORT4N:
		*outFormat = DXGI_FORMAT_R16G16B16A16_SNORM;
		return true;

	case D3DDECLTYPE_USHORT2N:
		*outFormat = DXGI_FORMAT_R16G16_UNORM;
		return true;

	case D3DDECLTYPE_USHORT4N:
		*outFormat = DXGI_FORMAT_R16G16B16A16_UNORM;
		return true;

	case D3DDECLTYPE_UDEC3:
		*outFormat = DXGI_FORMAT_R10G10B10A2_UINT;
		return true;

	case D3DDECLTYPE_FLOAT16_2:
		*outFormat = DXGI_FORMAT_R16G16_FLOAT;
		return true;

	case D3DDECLTYPE_FLOAT16_4:
		*outFormat = DXGI_FORMAT_R16G16B16A16_FLOAT;
		return true;

	default:
		break;
	}

	return false;
}

static unsigned int r3dDX11InputLayout_GetDeclElementCount(const D3DVERTEXELEMENT9* decl)
{
	if(!decl)
		return 0;

	unsigned int count = 0;

	while(count < R3D_DX11_MAX_DECL_ELEMENTS)
	{
		if(decl[count].Type == D3DDECLTYPE_UNUSED)
			break;

		++count;
	}

	return count;
}

static bool r3dDX11InputLayout_BuildDesc(
	const D3DVERTEXELEMENT9* d3d9Decl,
	D3D11_INPUT_ELEMENT_DESC* outDesc,
	unsigned int* outElementCount
)
{
	if(outElementCount)
		*outElementCount = 0;

	if(!d3d9Decl || !outDesc || !outElementCount)
		return false;

	const unsigned int count = r3dDX11InputLayout_GetDeclElementCount(d3d9Decl);

	if(!count || count >= R3D_DX11_MAX_DECL_ELEMENTS)
		return false;

	for(unsigned int i = 0; i < count; ++i)
	{
		DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;

		if(!r3dDX11InputLayout_ConvertDeclType(d3d9Decl[i].Type, &format))
		{
			r3dOutToLog(
				"DX11InputLayout: unsupported D3DDECLTYPE=%d usage=%d usageIndex=%d\n",
				(int)d3d9Decl[i].Type,
				(int)d3d9Decl[i].Usage,
				(int)d3d9Decl[i].UsageIndex
			);

			return false;
		}

		outDesc[i].SemanticName = r3dDX11InputLayout_GetSemanticName(d3d9Decl[i].Usage);
		outDesc[i].SemanticIndex = d3d9Decl[i].UsageIndex;
		outDesc[i].Format = format;
		outDesc[i].InputSlot = d3d9Decl[i].Stream;
		outDesc[i].AlignedByteOffset = d3d9Decl[i].Offset;
		outDesc[i].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		outDesc[i].InstanceDataStepRate = 0;
	}

	*outElementCount = count;
	return true;
}

static unsigned int r3dDX11InputLayout_BuildHash(
	const D3DVERTEXELEMENT9* d3d9Decl,
	unsigned int elementCount,
	r3dDX11Shader* shader
)
{
	unsigned int hash = 2166136261u;

	hash = r3dDX11InputLayout_HashData(
		hash,
		d3d9Decl,
		elementCount * sizeof(D3DVERTEXELEMENT9)
	);

	size_t shaderBlobPtr = (size_t)shader->GetVertexByteCode();

	hash = r3dDX11InputLayout_HashData(
		hash,
		&shaderBlobPtr,
		sizeof(shaderBlobPtr)
	);

	return hash;
}

r3dDX11InputLayoutManager g_r3dDX11InputLayouts;

r3dDX11InputLayoutManager::r3dDX11InputLayoutManager()
{
	NumLayouts = 0;
	CurrentLayout = NULL;

	for(int i = 0; i < R3D_DX11_MAX_INPUT_LAYOUTS; ++i)
	{
		Layouts[i].Hash = 0;
		Layouts[i].ElementCount = 0;
		Layouts[i].Layout = NULL;
	}
}

r3dDX11InputLayoutManager::~r3dDX11InputLayoutManager()
{
	Shutdown();
}

void r3dDX11InputLayoutManager::Shutdown()
{
	for(int i = 0; i < R3D_DX11_MAX_INPUT_LAYOUTS; ++i)
	{
		r3dDX11InputLayout_Release(Layouts[i].Layout);
		Layouts[i].Hash = 0;
		Layouts[i].ElementCount = 0;
	}

	NumLayouts = 0;
	CurrentLayout = NULL;
}

void r3dDX11InputLayoutManager::InvalidateCache()
{
	CurrentLayout = NULL;
}

int r3dDX11InputLayoutManager::FindLayout(unsigned int hash, unsigned int elementCount) const
{
	for(int i = 0; i < NumLayouts; ++i)
	{
		if(Layouts[i].Hash == hash && Layouts[i].ElementCount == elementCount)
			return i;
	}

	return -1;
}

int r3dDX11InputLayoutManager::AllocLayoutSlot()
{
	if(NumLayouts < R3D_DX11_MAX_INPUT_LAYOUTS)
	{
		const int slot = NumLayouts;
		++NumLayouts;
		return slot;
	}

	r3dDX11InputLayout_Release(Layouts[0].Layout);
	Layouts[0].Hash = 0;
	Layouts[0].ElementCount = 0;

	return 0;
}

ID3D11InputLayout* r3dDX11InputLayoutManager::GetOrCreate(const void* d3d9Declaration, r3dDX11Shader* shader)
{
	if(!g_r3dDX11.IsInitialized())
		return NULL;

	if(!g_r3dDX11.GetDevice())
		return NULL;

	if(!shader || !shader->GetVertexByteCode())
	{
		r3dOutToLog("DX11InputLayout: vertex shader bytecode is missing\n");
		return NULL;
	}

	const D3DVERTEXELEMENT9* d3d9Decl = (const D3DVERTEXELEMENT9*)d3d9Declaration;

	D3D11_INPUT_ELEMENT_DESC desc[R3D_DX11_MAX_DECL_ELEMENTS];
	ZeroMemory(desc, sizeof(desc));

	unsigned int elementCount = 0;

	if(!r3dDX11InputLayout_BuildDesc(d3d9Decl, desc, &elementCount))
	{
		r3dOutToLog("DX11InputLayout: failed to build input layout desc\n");
		return NULL;
	}

	const unsigned int hash = r3dDX11InputLayout_BuildHash(d3d9Decl, elementCount, shader);

	const int existing = FindLayout(hash, elementCount);
	if(existing >= 0)
		return Layouts[existing].Layout;

	ID3D11InputLayout* layout = NULL;

	HRESULT hr = g_r3dDX11.GetDevice()->CreateInputLayout(
		desc,
		elementCount,
		shader->GetVertexByteCode()->GetBufferPointer(),
		shader->GetVertexByteCode()->GetBufferSize(),
		&layout
	);

	if(FAILED(hr))
	{
		r3dDX11InputLayout_LogHR("CreateInputLayout", hr);
		return NULL;
	}

	const int slot = AllocLayoutSlot();

	Layouts[slot].Hash = hash;
	Layouts[slot].ElementCount = elementCount;
	Layouts[slot].Layout = layout;

	r3dOutToLog("DX11InputLayout: created layout, elements=%u slot=%d\n", elementCount, slot);

	return layout;
}

bool r3dDX11InputLayoutManager::Set(const void* d3d9Declaration, r3dDX11Shader* shader)
{
	if(!g_r3dDX11.IsInitialized())
		return false;

	ID3D11DeviceContext* context = g_r3dDX11.GetContext();
	if(!context)
		return false;

	ID3D11InputLayout* layout = GetOrCreate(d3d9Declaration, shader);
	if(!layout)
		return false;

	if(CurrentLayout == layout)
		return true;

	context->IASetInputLayout(layout);
	CurrentLayout = layout;

	return true;
}

#endif