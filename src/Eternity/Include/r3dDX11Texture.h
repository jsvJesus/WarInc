#ifndef __R3D_DX11_TEXTURE_H
#define __R3D_DX11_TEXTURE_H

#ifndef WO_SERVER

#include "r3dDX11.h"

struct ID3D11Texture2D;
struct ID3D11ShaderResourceView;
struct ID3D11RenderTargetView;

class r3dDX11Texture
{
public:
    r3dDX11Texture();
    ~r3dDX11Texture();

    bool Create2D(int width, int height, R3D_DX11_FORMAT format, const void* pixels, int pitch);
    bool CreateRenderTarget2D(int width, int height, R3D_DX11_FORMAT format, int mipCount);
    bool CreateRenderTargetCube(int edgeLength, R3D_DX11_FORMAT format, int mipCount);
    bool CreateChecker(int size);

    bool LoadDDSFromFile(const char* fileName);
    bool LoadDDSFromMemory(const void* data, int dataSize, const char* debugName);

    void Destroy();

    bool IsValid() const;

    ID3D11Texture2D* GetTexture2D() const;
    ID3D11ShaderResourceView* GetSRV() const;
    ID3D11RenderTargetView* GetRTV(int face, int mip) const;
    ID3D11Texture2D* AddRefTexture2D() const;
    ID3D11RenderTargetView* AddRefRTV(int face, int mip) const;
    bool AddRefRenderTargetMirror(int face, int mip, ID3D11Texture2D** texture, ID3D11RenderTargetView** rtv) const;

    int GetWidth() const;
    int GetHeight() const;
    int GetMipCount() const;
    R3D_DX11_FORMAT GetFormat() const;

private:
    ID3D11Texture2D* Texture;
    ID3D11ShaderResourceView* SRV;
    ID3D11RenderTargetView** RTVs;
    int RTVCount;
    bool IsCube;

    int Width;
    int Height;
    int MipCount;
    R3D_DX11_FORMAT Format;
};

#endif

#endif
