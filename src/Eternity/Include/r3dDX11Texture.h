#ifndef __R3D_DX11_TEXTURE_H
#define __R3D_DX11_TEXTURE_H

#ifndef WO_SERVER

#include "r3dDX11.h"

struct ID3D11Texture2D;
struct ID3D11ShaderResourceView;

class r3dDX11Texture
{
public:
    r3dDX11Texture();
    ~r3dDX11Texture();

    bool Create2D(int width, int height, R3D_DX11_FORMAT format, const void* pixels, int pitch);
    bool CreateChecker(int size);

    bool LoadDDSFromFile(const char* fileName);
    bool LoadDDSFromMemory(const void* data, int dataSize, const char* debugName);

    void Destroy();

    bool IsValid() const;

    ID3D11Texture2D* GetTexture2D() const;
    ID3D11ShaderResourceView* GetSRV() const;

    int GetWidth() const;
    int GetHeight() const;
    int GetMipCount() const;
    R3D_DX11_FORMAT GetFormat() const;

private:
    ID3D11Texture2D* Texture;
    ID3D11ShaderResourceView* SRV;

    int Width;
    int Height;
    int MipCount;
    R3D_DX11_FORMAT Format;
};

#endif

#endif