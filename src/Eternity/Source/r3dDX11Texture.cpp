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
#include "r3dDX11Texture.h"

#define R3D_DDS_MAGIC 0x20534444
#define R3D_DDS_FOURCC_DX10 0x30315844
#define R3D_DDS_FOURCC_DXT1 0x31545844
#define R3D_DDS_FOURCC_DXT2 0x32545844
#define R3D_DDS_FOURCC_DXT3 0x33545844
#define R3D_DDS_FOURCC_DXT4 0x34545844
#define R3D_DDS_FOURCC_DXT5 0x35545844
#define R3D_DDS_FOURCC_ATI1 0x31495441
#define R3D_DDS_FOURCC_ATI2 0x32495441
#define R3D_DDS_FOURCC_BC4U 0x55344342
#define R3D_DDS_FOURCC_BC5U 0x55354342

#define R3D_DDSPF_ALPHAPIXELS 0x00000001
#define R3D_DDSPF_ALPHA       0x00000002
#define R3D_DDSPF_FOURCC      0x00000004
#define R3D_DDSPF_RGB         0x00000040
#define R3D_DDSPF_LUMINANCE   0x00020000

#define R3D_DDSCAPS2_CUBEMAP  0x00000200

#pragma pack(push, 1)

struct r3dDDS_PIXELFORMAT
{
	unsigned int size;
	unsigned int flags;
	unsigned int fourCC;
	unsigned int rgbBitCount;
	unsigned int rBitMask;
	unsigned int gBitMask;
	unsigned int bBitMask;
	unsigned int aBitMask;
};

struct r3dDDS_HEADER
{
	unsigned int size;
	unsigned int flags;
	unsigned int height;
	unsigned int width;
	unsigned int pitchOrLinearSize;
	unsigned int depth;
	unsigned int mipMapCount;
	unsigned int reserved1[11];
	r3dDDS_PIXELFORMAT ddspf;
	unsigned int caps;
	unsigned int caps2;
	unsigned int caps3;
	unsigned int caps4;
	unsigned int reserved2;
};

struct r3dDDS_HEADER_DXT10
{
	unsigned int dxgiFormat;
	unsigned int resourceDimension;
	unsigned int miscFlag;
	unsigned int arraySize;
	unsigned int miscFlags2;
};

#pragma pack(pop)

template<typename T>
static void r3dDX11Texture_Release(T*& ptr)
{
	if(ptr)
	{
		ptr->Release();
		ptr = NULL;
	}
}

static void r3dDX11Texture_LogHR(const char* text, HRESULT hr)
{
	r3dOutToLog("DX11Texture: %s failed, HRESULT=0x%08X\n", text, (unsigned int)hr);
}

static bool r3dDX11Texture_IsBitMask(const r3dDDS_PIXELFORMAT& ddpf, unsigned int r, unsigned int g, unsigned int b, unsigned int a)
{
	return ddpf.rBitMask == r && ddpf.gBitMask == g && ddpf.bBitMask == b && ddpf.aBitMask == a;
}

static bool r3dDX11Texture_IsBCFormat(DXGI_FORMAT format)
{
	switch(format)
	{
	case DXGI_FORMAT_BC1_TYPELESS:
	case DXGI_FORMAT_BC1_UNORM:
	case DXGI_FORMAT_BC1_UNORM_SRGB:
	case DXGI_FORMAT_BC2_TYPELESS:
	case DXGI_FORMAT_BC2_UNORM:
	case DXGI_FORMAT_BC2_UNORM_SRGB:
	case DXGI_FORMAT_BC3_TYPELESS:
	case DXGI_FORMAT_BC3_UNORM:
	case DXGI_FORMAT_BC3_UNORM_SRGB:
	case DXGI_FORMAT_BC4_TYPELESS:
	case DXGI_FORMAT_BC4_UNORM:
	case DXGI_FORMAT_BC4_SNORM:
	case DXGI_FORMAT_BC5_TYPELESS:
	case DXGI_FORMAT_BC5_UNORM:
	case DXGI_FORMAT_BC5_SNORM:
		return true;
	default:
		return false;
	}
}

static int r3dDX11Texture_GetBCBlockBytes(DXGI_FORMAT format)
{
	switch(format)
	{
	case DXGI_FORMAT_BC1_TYPELESS:
	case DXGI_FORMAT_BC1_UNORM:
	case DXGI_FORMAT_BC1_UNORM_SRGB:
	case DXGI_FORMAT_BC4_TYPELESS:
	case DXGI_FORMAT_BC4_UNORM:
	case DXGI_FORMAT_BC4_SNORM:
		return 8;
	default:
		return 16;
	}
}

static int r3dDX11Texture_BitsPerPixel(DXGI_FORMAT format)
{
	switch(format)
	{
	case DXGI_FORMAT_R32G32B32A32_FLOAT:
	case DXGI_FORMAT_R32G32B32A32_UINT:
	case DXGI_FORMAT_R32G32B32A32_SINT:
		return 128;

	case DXGI_FORMAT_R16G16B16A16_FLOAT:
	case DXGI_FORMAT_R16G16B16A16_UNORM:
	case DXGI_FORMAT_R16G16B16A16_UINT:
	case DXGI_FORMAT_R16G16B16A16_SNORM:
	case DXGI_FORMAT_R16G16B16A16_SINT:
	case DXGI_FORMAT_R32G32_FLOAT:
	case DXGI_FORMAT_R32G32_UINT:
	case DXGI_FORMAT_R32G32_SINT:
		return 64;

	case DXGI_FORMAT_R8G8B8A8_UNORM:
	case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
	case DXGI_FORMAT_R8G8B8A8_UINT:
	case DXGI_FORMAT_R8G8B8A8_SNORM:
	case DXGI_FORMAT_R8G8B8A8_SINT:
	case DXGI_FORMAT_B8G8R8A8_UNORM:
	case DXGI_FORMAT_B8G8R8X8_UNORM:
	case DXGI_FORMAT_R10G10B10A2_UNORM:
	case DXGI_FORMAT_R10G10B10A2_UINT:
	case DXGI_FORMAT_R11G11B10_FLOAT:
	case DXGI_FORMAT_R16G16_FLOAT:
	case DXGI_FORMAT_R16G16_UNORM:
	case DXGI_FORMAT_R16G16_UINT:
	case DXGI_FORMAT_R16G16_SNORM:
	case DXGI_FORMAT_R16G16_SINT:
	case DXGI_FORMAT_R32_FLOAT:
	case DXGI_FORMAT_R32_UINT:
	case DXGI_FORMAT_R32_SINT:
		return 32;

	case DXGI_FORMAT_R8G8_UNORM:
	case DXGI_FORMAT_R8G8_UINT:
	case DXGI_FORMAT_R8G8_SNORM:
	case DXGI_FORMAT_R8G8_SINT:
	case DXGI_FORMAT_R16_FLOAT:
	case DXGI_FORMAT_R16_UNORM:
	case DXGI_FORMAT_R16_UINT:
	case DXGI_FORMAT_R16_SNORM:
	case DXGI_FORMAT_R16_SINT:
	case DXGI_FORMAT_B5G6R5_UNORM:
	case DXGI_FORMAT_B5G5R5A1_UNORM:
		return 16;

	case DXGI_FORMAT_R8_UNORM:
	case DXGI_FORMAT_R8_UINT:
	case DXGI_FORMAT_R8_SNORM:
	case DXGI_FORMAT_R8_SINT:
	case DXGI_FORMAT_A8_UNORM:
		return 8;

	default:
		return 0;
	}
}

static DXGI_FORMAT r3dDX11Texture_GetDXGIFormat(const r3dDDS_PIXELFORMAT& ddpf)
{
	if(ddpf.flags & R3D_DDSPF_FOURCC)
	{
		switch(ddpf.fourCC)
		{
		case R3D_DDS_FOURCC_DXT1:
			return DXGI_FORMAT_BC1_UNORM;

		case R3D_DDS_FOURCC_DXT2:
		case R3D_DDS_FOURCC_DXT3:
			return DXGI_FORMAT_BC2_UNORM;

		case R3D_DDS_FOURCC_DXT4:
		case R3D_DDS_FOURCC_DXT5:
			return DXGI_FORMAT_BC3_UNORM;

		case R3D_DDS_FOURCC_ATI1:
		case R3D_DDS_FOURCC_BC4U:
			return DXGI_FORMAT_BC4_UNORM;

		case R3D_DDS_FOURCC_ATI2:
		case R3D_DDS_FOURCC_BC5U:
			return DXGI_FORMAT_BC5_UNORM;

		case 36:
			return DXGI_FORMAT_R16G16B16A16_UNORM;

		case 111:
			return DXGI_FORMAT_R16_FLOAT;

		case 112:
			return DXGI_FORMAT_R16G16_FLOAT;

		case 113:
			return DXGI_FORMAT_R16G16B16A16_FLOAT;

		case 114:
			return DXGI_FORMAT_R32_FLOAT;

		case 115:
			return DXGI_FORMAT_R32G32_FLOAT;

		case 116:
			return DXGI_FORMAT_R32G32B32A32_FLOAT;

		default:
			return DXGI_FORMAT_UNKNOWN;
		}
	}

	if(ddpf.flags & R3D_DDSPF_RGB)
	{
		switch(ddpf.rgbBitCount)
		{
		case 32:
			if(r3dDX11Texture_IsBitMask(ddpf, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000))
				return DXGI_FORMAT_R8G8B8A8_UNORM;

			if(r3dDX11Texture_IsBitMask(ddpf, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000))
				return DXGI_FORMAT_B8G8R8A8_UNORM;

			if(r3dDX11Texture_IsBitMask(ddpf, 0x00ff0000, 0x0000ff00, 0x000000ff, 0x00000000))
				return DXGI_FORMAT_B8G8R8X8_UNORM;

			break;

		case 16:
			if(r3dDX11Texture_IsBitMask(ddpf, 0x0000f800, 0x000007e0, 0x0000001f, 0x00000000))
				return DXGI_FORMAT_B5G6R5_UNORM;

			if(r3dDX11Texture_IsBitMask(ddpf, 0x00007c00, 0x000003e0, 0x0000001f, 0x00008000))
				return DXGI_FORMAT_B5G5R5A1_UNORM;

			break;
		}
	}

	if(ddpf.flags & R3D_DDSPF_LUMINANCE)
	{
		if(ddpf.rgbBitCount == 8)
			return DXGI_FORMAT_R8_UNORM;

		if(ddpf.rgbBitCount == 16)
			return DXGI_FORMAT_R16_UNORM;
	}

	if(ddpf.flags & R3D_DDSPF_ALPHA)
	{
		if(ddpf.rgbBitCount == 8)
			return DXGI_FORMAT_A8_UNORM;
	}

	return DXGI_FORMAT_UNKNOWN;
}

static bool r3dDX11Texture_GetSurfaceInfo(int width, int height, DXGI_FORMAT format, int* outNumBytes, int* outRowBytes)
{
	if(width <= 0)
		width = 1;

	if(height <= 0)
		height = 1;

	if(r3dDX11Texture_IsBCFormat(format))
	{
		int blockBytes = r3dDX11Texture_GetBCBlockBytes(format);
		int numBlocksWide = R3D_MAX(1, (width + 3) / 4);
		int numBlocksHigh = R3D_MAX(1, (height + 3) / 4);

		*outRowBytes = numBlocksWide * blockBytes;
		*outNumBytes = (*outRowBytes) * numBlocksHigh;
		return true;
	}

	int bpp = r3dDX11Texture_BitsPerPixel(format);

	if(bpp <= 0)
	{
		*outRowBytes = 0;
		*outNumBytes = 0;
		return false;
	}

	*outRowBytes = (width * bpp + 7) / 8;
	*outNumBytes = (*outRowBytes) * height;
	return true;
}

r3dDX11Texture::r3dDX11Texture()
{
	Texture = NULL;
	SRV = NULL;
	RTVs = NULL;
	RTVCount = 0;
	IsCube = false;

	Width = 0;
	Height = 0;
	MipCount = 0;
	Format = 0;
}

r3dDX11Texture::~r3dDX11Texture()
{
	Destroy();
}

bool r3dDX11Texture::Create2D(int width, int height, R3D_DX11_FORMAT format, const void* pixels, int pitch)
{
	Destroy();

	if(!g_r3dDX11.IsInitialized())
	{
		r3dOutToLog("DX11Texture: Create2D failed, DX11 renderer is not initialized\n");
		return false;
	}

	if(!g_r3dDX11.CanCreateDeviceResources("DX11Texture"))
		return false;

	if(width <= 0 || height <= 0)
	{
		r3dOutToLog("DX11Texture: Create2D failed, bad size %dx%d\n", width, height);
		return false;
	}

	if(format == 0)
	{
		r3dOutToLog("DX11Texture: Create2D failed, bad format\n");
		return false;
	}

	ID3D11Device* device = g_r3dDX11.GetDevice();

	if(!device)
	{
		r3dOutToLog("DX11Texture: Create2D failed, device is NULL\n");
		return false;
	}

	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory(&desc, sizeof(desc));

	desc.Width = width;
	desc.Height = height;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = (DXGI_FORMAT)format;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA initData;
	ZeroMemory(&initData, sizeof(initData));

	D3D11_SUBRESOURCE_DATA* initPtr = NULL;

	if(pixels)
	{
		initData.pSysMem = pixels;
		initData.SysMemPitch = pitch;
		initData.SysMemSlicePitch = pitch * height;
		initPtr = &initData;
	}

	HRESULT hr = device->CreateTexture2D(&desc, initPtr, &Texture);

	if(FAILED(hr))
	{
		if(!g_r3dDX11.CheckDeviceRemoved("DX11Texture::CreateTexture2D", hr))
			r3dDX11Texture_LogHR("CreateTexture2D", hr);
		
		Destroy();
		return false;
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory(&srvDesc, sizeof(srvDesc));

	srvDesc.Format = desc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;

	hr = device->CreateShaderResourceView(Texture, &srvDesc, &SRV);

	if(FAILED(hr))
	{
		r3dDX11Texture_LogHR("CreateShaderResourceView", hr);
		Destroy();
		return false;
	}

	Width = width;
	Height = height;
	MipCount = 1;
	Format = format;

	r3dOutToLog("DX11Texture: created 2D texture %dx%d format=%d\n", Width, Height, Format);

	return true;
}

bool r3dDX11Texture::CreateRenderTarget2D(int width, int height, R3D_DX11_FORMAT format, int mipCount)
{
	Destroy();

	if(!g_r3dDX11.IsInitialized())
		return false;

	if(!g_r3dDX11.CanCreateDeviceResources("DX11Texture"))
		return false;

	if(width <= 0 || height <= 0 || format == 0)
		return false;

	if(mipCount <= 0)
		mipCount = 1;

	ID3D11Device* device = g_r3dDX11.GetDevice();
	if(!device)
		return false;

	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory(&desc, sizeof(desc));

	desc.Width = width;
	desc.Height = height;
	desc.MipLevels = mipCount;
	desc.ArraySize = 1;
	desc.Format = (DXGI_FORMAT)format;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;

	HRESULT hr = device->CreateTexture2D(&desc, NULL, &Texture);
	if(FAILED(hr))
	{
		if(!g_r3dDX11.CheckDeviceRemoved("DX11Texture::CreateTexture2D RenderTarget", hr))
			r3dDX11Texture_LogHR("CreateTexture2D RenderTarget", hr);
		
		Destroy();
		return false;
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory(&srvDesc, sizeof(srvDesc));

	srvDesc.Format = desc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = mipCount;

	hr = device->CreateShaderResourceView(Texture, &srvDesc, &SRV);
	if(FAILED(hr))
	{
		r3dDX11Texture_LogHR("CreateShaderResourceView RenderTarget", hr);
		Destroy();
		return false;
	}

	RTVCount = mipCount;
	RTVs = new ID3D11RenderTargetView*[RTVCount];
	ZeroMemory(RTVs, sizeof(ID3D11RenderTargetView*) * RTVCount);

	for(int mip = 0; mip < mipCount; ++mip)
	{
		D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
		ZeroMemory(&rtvDesc, sizeof(rtvDesc));

		rtvDesc.Format = desc.Format;
		rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		rtvDesc.Texture2D.MipSlice = mip;

		hr = device->CreateRenderTargetView(Texture, &rtvDesc, &RTVs[mip]);
		if(FAILED(hr))
		{
			if(!g_r3dDX11.CheckDeviceRemoved("DX11Texture::CreateRenderTargetView", hr))
				r3dDX11Texture_LogHR("CreateRenderTargetView Texture2D", hr);
			
			Destroy();
			return false;
		}
	}

	Width = width;
	Height = height;
	MipCount = mipCount;
	Format = format;
	IsCube = false;

	return true;
}

bool r3dDX11Texture::CreateRenderTargetCube(int edgeLength, R3D_DX11_FORMAT format, int mipCount)
{
	Destroy();

	if(!g_r3dDX11.IsInitialized())
		return false;

	if(!g_r3dDX11.CanCreateDeviceResources("DX11Texture"))
		return false;

	if(edgeLength <= 0 || format == 0)
		return false;

	if(mipCount <= 0)
		mipCount = 1;

	ID3D11Device* device = g_r3dDX11.GetDevice();
	if(!device)
		return false;

	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory(&desc, sizeof(desc));

	desc.Width = edgeLength;
	desc.Height = edgeLength;
	desc.MipLevels = mipCount;
	desc.ArraySize = 6;
	desc.Format = (DXGI_FORMAT)format;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	desc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

	HRESULT hr = device->CreateTexture2D(&desc, NULL, &Texture);
	if(FAILED(hr))
	{
		if(!g_r3dDX11.CheckDeviceRemoved("DX11Texture::CreateTexture2D RenderTargetCube", hr))
			r3dDX11Texture_LogHR("CreateTexture2D RenderTargetCube", hr);
		
		Destroy();
		return false;
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory(&srvDesc, sizeof(srvDesc));

	srvDesc.Format = desc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	srvDesc.TextureCube.MostDetailedMip = 0;
	srvDesc.TextureCube.MipLevels = mipCount;

	hr = device->CreateShaderResourceView(Texture, &srvDesc, &SRV);
	if(FAILED(hr))
	{
		r3dDX11Texture_LogHR("CreateShaderResourceView RenderTargetCube", hr);
		Destroy();
		return false;
	}

	RTVCount = 6 * mipCount;
	RTVs = new ID3D11RenderTargetView*[RTVCount];
	ZeroMemory(RTVs, sizeof(ID3D11RenderTargetView*) * RTVCount);

	for(int face = 0; face < 6; ++face)
	{
		for(int mip = 0; mip < mipCount; ++mip)
		{
			D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
			ZeroMemory(&rtvDesc, sizeof(rtvDesc));

			rtvDesc.Format = desc.Format;
			rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
			rtvDesc.Texture2DArray.MipSlice = mip;
			rtvDesc.Texture2DArray.FirstArraySlice = face;
			rtvDesc.Texture2DArray.ArraySize = 1;

			const int index = face * mipCount + mip;
			hr = device->CreateRenderTargetView(Texture, &rtvDesc, &RTVs[index]);
			if(FAILED(hr))
			{
				r3dDX11Texture_LogHR("CreateRenderTargetView TextureCube", hr);
				Destroy();
				return false;
			}
		}
	}

	Width = edgeLength;
	Height = edgeLength;
	MipCount = mipCount;
	Format = format;
	IsCube = true;

	return true;
}

bool r3dDX11Texture::CreateChecker(int size)
{
	if(size <= 0)
		size = 256;

	unsigned int* pixels = new unsigned int[size * size];

	for(int y = 0; y < size; ++y)
	{
		for(int x = 0; x < size; ++x)
		{
			const int cx = x / 32;
			const int cy = y / 32;
			const bool light = ((cx + cy) & 1) == 0;

			unsigned int color;

			if(light)
				color = 0xFFE0E0E0;
			else
				color = 0xFF303030;

			if(x < 8 || y < 8 || x >= size - 8 || y >= size - 8)
				color = 0xFF2A9D4B;

			pixels[y * size + x] = color;
		}
	}

	bool result = Create2D(
		size,
		size,
		(R3D_DX11_FORMAT)DXGI_FORMAT_B8G8R8A8_UNORM,
		pixels,
		size * 4
	);

	delete [] pixels;

	return result;
}

bool r3dDX11Texture::LoadDDSFromFile(const char* fileName)
{
	if(!fileName || !fileName[0])
	{
		r3dOutToLog("DX11Texture: LoadDDSFromFile failed, empty file name\n");
		return false;
	}

	r3dFile* ff = r3d_open(fileName, "rb");

	if(!ff)
	{
		r3dOutToLog("DX11Texture: failed to open DDS '%s'\n", fileName);
		return false;
	}

	int fileSize = ff->size;

	if(fileSize <= 0)
	{
		fclose(ff);
		r3dOutToLog("DX11Texture: bad DDS file size '%s'\n", fileName);
		return false;
	}

	void* fileData = malloc(fileSize);

	if(!fileData)
	{
		fclose(ff);
		r3dOutToLog("DX11Texture: out of memory while loading '%s'\n", fileName);
		return false;
	}

	fread(fileData, 1, fileSize, ff);
	fclose(ff);

	bool result = LoadDDSFromMemory(fileData, fileSize, fileName);

	free(fileData);

	return result;
}

bool r3dDX11Texture::LoadDDSFromMemory(const void* data, int dataSize, const char* debugName)
{
	Destroy();

	if(!g_r3dDX11.IsInitialized())
	{
		r3dOutToLog("DX11Texture: LoadDDSFromMemory failed, DX11 renderer is not initialized\n");
		return false;
	}

	if(!g_r3dDX11.CanCreateDeviceResources("DX11Texture"))
		return false;

	if(!data || dataSize <= 0)
	{
		r3dOutToLog("DX11Texture: LoadDDSFromMemory failed, empty data\n");
		return false;
	}

	if(dataSize < (int)(sizeof(unsigned int) + sizeof(r3dDDS_HEADER)))
	{
		r3dOutToLog("DX11Texture: DDS too small '%s'\n", debugName ? debugName : "");
		return false;
	}

	const unsigned char* bytes = (const unsigned char*)data;

	unsigned int magic = *(const unsigned int*)bytes;

	if(magic != R3D_DDS_MAGIC)
	{
		r3dOutToLog("DX11Texture: not a DDS file '%s'\n", debugName ? debugName : "");
		return false;
	}

	const r3dDDS_HEADER* header = (const r3dDDS_HEADER*)(bytes + sizeof(unsigned int));

	if(header->size != 124 || header->ddspf.size != 32)
	{
		r3dOutToLog("DX11Texture: invalid DDS header '%s'\n", debugName ? debugName : "");
		return false;
	}

	if(header->caps2 & R3D_DDSCAPS2_CUBEMAP)
	{
		r3dOutToLog("DX11Texture: cubemap DDS is not supported yet '%s'\n", debugName ? debugName : "");
		return false;
	}

	int offset = sizeof(unsigned int) + sizeof(r3dDDS_HEADER);

	DXGI_FORMAT dxgiFormat = DXGI_FORMAT_UNKNOWN;

	if((header->ddspf.flags & R3D_DDSPF_FOURCC) && header->ddspf.fourCC == R3D_DDS_FOURCC_DX10)
	{
		if(dataSize < offset + (int)sizeof(r3dDDS_HEADER_DXT10))
		{
			r3dOutToLog("DX11Texture: invalid DDS DX10 header '%s'\n", debugName ? debugName : "");
			return false;
		}

		const r3dDDS_HEADER_DXT10* dx10 = (const r3dDDS_HEADER_DXT10*)(bytes + offset);

		if(dx10->arraySize != 1)
		{
			r3dOutToLog("DX11Texture: DDS array textures are not supported yet '%s'\n", debugName ? debugName : "");
			return false;
		}

		if(dx10->resourceDimension != 3)
		{
			r3dOutToLog("DX11Texture: only DDS Texture2D is supported now '%s'\n", debugName ? debugName : "");
			return false;
		}

		dxgiFormat = (DXGI_FORMAT)dx10->dxgiFormat;
		offset += sizeof(r3dDDS_HEADER_DXT10);
	}
	else
	{
		dxgiFormat = r3dDX11Texture_GetDXGIFormat(header->ddspf);
	}

	if(dxgiFormat == DXGI_FORMAT_UNKNOWN)
	{
		r3dOutToLog("DX11Texture: unsupported DDS format '%s'\n", debugName ? debugName : "");
		return false;
	}

	int width = (int)header->width;
	int height = (int)header->height;
	int mipCount = (int)header->mipMapCount;

	if(width <= 0 || height <= 0)
	{
		r3dOutToLog("DX11Texture: bad DDS size %dx%d '%s'\n", width, height, debugName ? debugName : "");
		return false;
	}

	if(mipCount <= 0)
		mipCount = 1;

	if(mipCount > 32)
		mipCount = 32;

	D3D11_SUBRESOURCE_DATA initData[32];
	ZeroMemory(initData, sizeof(initData));

	int mipWidth = width;
	int mipHeight = height;
	int createdMips = 0;

	for(int mip = 0; mip < mipCount; ++mip)
	{
		int numBytes = 0;
		int rowBytes = 0;

		if(!r3dDX11Texture_GetSurfaceInfo(mipWidth, mipHeight, dxgiFormat, &numBytes, &rowBytes))
		{
			r3dOutToLog("DX11Texture: failed to calculate DDS pitch '%s'\n", debugName ? debugName : "");
			return false;
		}

		if(offset + numBytes > dataSize)
		{
			if(mip == 0)
			{
				r3dOutToLog("DX11Texture: DDS data is truncated '%s'\n", debugName ? debugName : "");
				return false;
			}

			break;
		}

		initData[mip].pSysMem = bytes + offset;
		initData[mip].SysMemPitch = rowBytes;
		initData[mip].SysMemSlicePitch = numBytes;

		offset += numBytes;
		createdMips++;

		mipWidth = R3D_MAX(1, mipWidth / 2);
		mipHeight = R3D_MAX(1, mipHeight / 2);
	}

	if(createdMips <= 0)
	{
		r3dOutToLog("DX11Texture: no mip data in DDS '%s'\n", debugName ? debugName : "");
		return false;
	}

	ID3D11Device* device = g_r3dDX11.GetDevice();

	if(!device)
	{
		r3dOutToLog("DX11Texture: no DX11 device while creating DDS '%s'\n", debugName ? debugName : "");
		return false;
	}

	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory(&desc, sizeof(desc));

	desc.Width = width;
	desc.Height = height;
	desc.MipLevels = createdMips;
	desc.ArraySize = 1;
	desc.Format = dxgiFormat;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;

	HRESULT hr = device->CreateTexture2D(&desc, initData, &Texture);

	if(FAILED(hr))
	{
		if(!g_r3dDX11.CheckDeviceRemoved("DX11Texture::CreateTexture2D DDS", hr))
			r3dDX11Texture_LogHR("CreateTexture2D DDS", hr);
		
		Destroy();
		return false;
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory(&srvDesc, sizeof(srvDesc));

	srvDesc.Format = desc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = createdMips;

	hr = device->CreateShaderResourceView(Texture, &srvDesc, &SRV);

	if(FAILED(hr))
	{
		if(!g_r3dDX11.CheckDeviceRemoved("DX11Texture::CreateShaderResourceView", hr))
			r3dDX11Texture_LogHR("CreateShaderResourceView", hr);
		
		Destroy();
		return false;
	}

	Width = width;
	Height = height;
	MipCount = createdMips;
	Format = (R3D_DX11_FORMAT)dxgiFormat;

	r3dOutToLog(
		"DX11Texture: loaded DDS '%s' %dx%d mips=%d format=%d\n",
		debugName ? debugName : "",
		Width,
		Height,
		MipCount,
		Format
	);

	return true;
}

void r3dDX11Texture::Destroy()
{
	if(RTVs)
	{
		for(int i = 0; i < RTVCount; ++i)
			r3dDX11Texture_Release(RTVs[i]);

		delete [] RTVs;
		RTVs = NULL;
	}

	RTVCount = 0;
	IsCube = false;

	r3dDX11Texture_Release(SRV);
	r3dDX11Texture_Release(Texture);

	Width = 0;
	Height = 0;
	MipCount = 0;
	Format = 0;
}

bool r3dDX11Texture::IsValid() const
{
	return Texture && SRV;
}

ID3D11Texture2D* r3dDX11Texture::GetTexture2D() const
{
	return Texture;
}

ID3D11ShaderResourceView* r3dDX11Texture::GetSRV() const
{
	return SRV;
}

ID3D11RenderTargetView* r3dDX11Texture::GetRTV(int face, int mip) const
{
	if(!RTVs || mip < 0 || mip >= MipCount)
		return NULL;

	if(IsCube)
	{
		if(face < 0 || face >= 6)
			return NULL;

		return RTVs[face * MipCount + mip];
	}

	return RTVs[mip];
}

ID3D11Texture2D* r3dDX11Texture::AddRefTexture2D() const
{
	if(Texture)
		Texture->AddRef();

	return Texture;
}

ID3D11RenderTargetView* r3dDX11Texture::AddRefRTV(int face, int mip) const
{
	ID3D11RenderTargetView* rtv = GetRTV(face, mip);
	if(rtv)
		rtv->AddRef();

	return rtv;
}

bool r3dDX11Texture::AddRefRenderTargetMirror(
	int face,
	int mip,
	ID3D11Texture2D** texture,
	ID3D11RenderTargetView** rtv
) const
{
	if(texture)
		*texture = NULL;

	if(rtv)
		*rtv = NULL;

	ID3D11RenderTargetView* view = GetRTV(face, mip);
	if(!Texture || !view || !texture || !rtv)
		return false;

	Texture->AddRef();
	view->AddRef();

	*texture = Texture;
	*rtv = view;
	return true;
}

int r3dDX11Texture::GetWidth() const
{
	return Width;
}

int r3dDX11Texture::GetHeight() const
{
	return Height;
}

int r3dDX11Texture::GetMipCount() const
{
	return MipCount;
}

R3D_DX11_FORMAT r3dDX11Texture::GetFormat() const
{
	return Format;
}

#endif
