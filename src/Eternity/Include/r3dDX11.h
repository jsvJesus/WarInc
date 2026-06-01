#ifndef __R3D_DX11_H
#define __R3D_DX11_H

#ifndef WO_SERVER

struct ID3D11Device;
struct ID3D11DeviceContext;
struct IDXGISwapChain;
struct ID3D11RenderTargetView;
struct ID3D11Texture2D;
struct ID3D11DepthStencilView;
struct ID3D11VertexShader;
struct ID3D11PixelShader;
struct ID3D11InputLayout;
struct ID3D11Buffer;

typedef int R3D_DX11_FORMAT;
typedef int R3D_DX11_FEATURE_LEVEL;

class r3dDX11Renderer
{
public:
    r3dDX11Renderer();
    ~r3dDX11Renderer();

    bool Init(HWND hwnd, int width, int height, bool windowed);
    void Shutdown();

    bool Resize(int width, int height);

    void BeginFrame(float r, float g, float b, float a);
    void DrawDebugTriangle();
    void EndFrame(bool present);

    void SetVSync(bool enabled);
    void SetDebugTriangle(bool enabled);

    bool IsInitialized() const;

    ID3D11Device* GetDevice() const;
    ID3D11DeviceContext* GetContext() const;
    IDXGISwapChain* GetSwapChain() const;
    ID3D11RenderTargetView* GetBackBufferRTV() const;
    ID3D11DepthStencilView* GetDepthStencilView() const;

    int GetWidth() const;
    int GetHeight() const;
    R3D_DX11_FEATURE_LEVEL GetFeatureLevel() const;

private:
    bool CreateBackBuffer();
    void ReleaseBackBuffer();

    bool CreateDebugTriangle();
    void ReleaseDebugTriangle();

    HWND HWnd;
    int Width;
    int Height;
    bool Windowed;
    bool VSync;
    bool DebugTriangleEnabled;

    R3D_DX11_FEATURE_LEVEL FeatureLevel;

    ID3D11Device* Device;
    ID3D11DeviceContext* Context;
    IDXGISwapChain* SwapChain;

    ID3D11RenderTargetView* BackBufferRTV;
    ID3D11Texture2D* DepthStencilTexture;
    ID3D11DepthStencilView* DepthStencilView;

    ID3D11VertexShader* DebugVS;
    ID3D11PixelShader* DebugPS;
    ID3D11InputLayout* DebugInputLayout;
    ID3D11Buffer* DebugVB;
};

extern r3dDX11Renderer g_r3dDX11;

R3D_DX11_FORMAT r3dDX11_ConvertLegacyD3DFormat(int d3dFormat);

#endif

#endif