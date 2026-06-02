#pragma once

#include <windows.h>
#include <d3d9.h>
#include <d3d11.h>

class r3dDX11ScaleformBridge
{
public:
    r3dDX11ScaleformBridge();
    ~r3dDX11ScaleformBridge();

    bool Init(HWND hWnd, ID3D11Device* dx11Device, ID3D11DeviceContext* dx11Context, int width, int height);
    void Shutdown();

    bool Resize(int width, int height);

    bool BeginScaleformRender();
    void EndScaleformRender();

    void DrawDX11();

    bool IsReady() const;
    IDirect3DDevice9Ex* GetD3D9Device() const;
    IDirect3DTexture9* GetD3D9Texture() const;
    ID3D11ShaderResourceView* GetDX11SRV() const;

private:
    bool CreateD3D9(HWND hWnd);
    bool CreateSharedTarget(int width, int height);
    bool CreateDX11DrawResources();
    void DestroySharedTarget();
    void DestroyDX11DrawResources();
    void FlushD3D9();

private:
    HWND m_hWnd;

    int m_width;
    int m_height;

    IDirect3D9Ex* m_d3d9;
    IDirect3DDevice9Ex* m_device9;

    IDirect3DTexture9* m_sharedTexture9;
    IDirect3DSurface9* m_sharedSurface9;
    IDirect3DSurface9* m_oldSurface9;
    IDirect3DSurface9* m_oldDepth9;
    HANDLE m_sharedHandle;

    IDirect3DQuery9* m_flushQuery9;

    ID3D11Device* m_device11;
    ID3D11DeviceContext* m_context11;
    ID3D11Texture2D* m_sharedTexture11;
    ID3D11ShaderResourceView* m_sharedSRV11;

    ID3D11VertexShader* m_vs;
    ID3D11PixelShader* m_ps;
    ID3D11SamplerState* m_sampler;
    ID3D11BlendState* m_blendState;
    ID3D11DepthStencilState* m_depthState;
    ID3D11RasterizerState* m_rasterState;
};

extern r3dDX11ScaleformBridge g_r3dDX11ScaleformBridge;

IDirect3DDevice9Ex* r3dGetScaleformD3D9Device();