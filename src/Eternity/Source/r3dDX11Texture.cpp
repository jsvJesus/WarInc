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

r3dDX11Texture::r3dDX11Texture()
{
	Texture = NULL;
	SRV = NULL;

	Width = 0;
	Height = 0;
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
	Format = format;

	r3dOutToLog("DX11Texture: created 2D texture %dx%d format=%d\n", Width, Height, Format);

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

void r3dDX11Texture::Destroy()
{
	r3dDX11Texture_Release(SRV);
	r3dDX11Texture_Release(Texture);

	Width = 0;
	Height = 0;
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

int r3dDX11Texture::GetWidth() const
{
	return Width;
}

int r3dDX11Texture::GetHeight() const
{
	return Height;
}

R3D_DX11_FORMAT r3dDX11Texture::GetFormat() const
{
	return Format;
}

#endif