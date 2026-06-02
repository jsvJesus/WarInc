#ifndef __R3D_DX11_STATE_H
#define __R3D_DX11_STATE_H

#ifndef WO_SERVER

struct ID3D11ShaderResourceView;
struct ID3D11SamplerState;
struct ID3D11DepthStencilState;
struct ID3D11BlendState;
struct ID3D11RasterizerState;

class r3dTexture;

enum r3dDX11SamplerMode
{
    R3D_DX11_SAMPLER_LINEAR_WRAP = 0,
    R3D_DX11_SAMPLER_LINEAR_CLAMP,
    R3D_DX11_SAMPLER_POINT_WRAP,
    R3D_DX11_SAMPLER_POINT_CLAMP,
    R3D_DX11_SAMPLER_MAX
};

class r3dDX11State
{
public:
    r3dDX11State();
    ~r3dDX11State();

    bool Init();
    void Shutdown();

    bool IsInitialized() const;

    bool BindTexture(int slot, r3dTexture* texture);
    bool SetSRV(int slot, ID3D11ShaderResourceView* srv);
    void ClearTexture(int slot);
    void ClearTextures();

    bool SetSampler(int slot, r3dDX11SamplerMode mode);
    bool SetSamplerState(int slot, int samplerState, unsigned int value);
    void ApplyDefaultSamplers();

    bool SetRenderState(int state, unsigned int value);

    void InvalidateCache();

private:
    bool CreateSamplers();
    void ReleaseSamplers();
    void ReleaseCustomSamplers();
    void ResetSamplerStateCache();
    void ResetRenderStateCache();
    bool ApplySamplerState(int slot);
    bool ApplyDepthState();
    bool ApplyBlendState();
    bool ApplyRasterizerState();

    bool Initialized;

    ID3D11ShaderResourceView* BoundSRV[16];
    ID3D11ShaderResourceView* BoundVSSRV[4];
    ID3D11SamplerState* BoundSampler[16];
    ID3D11SamplerState* BoundVSSampler[4];

    ID3D11SamplerState* Samplers[R3D_DX11_SAMPLER_MAX];
    ID3D11SamplerState* CustomSamplers[16];
    ID3D11SamplerState* CustomVSSamplers[4];

    int SamplerMinFilter[16];
    int SamplerMagFilter[16];
    int SamplerMipFilter[16];
    int SamplerAddressU[16];
    int SamplerAddressV[16];
    int SamplerAddressW[16];
    float SamplerMipLODBias[16];
    unsigned int SamplerMaxAnisotropy[16];
    unsigned int SamplerBorderColor[16];
    unsigned int SamplerMaxMipLevel[16];

    int VSSamplerMinFilter[4];
    int VSSamplerMagFilter[4];
    int VSSamplerMipFilter[4];
    int VSSamplerAddressU[4];
    int VSSamplerAddressV[4];
    int VSSamplerAddressW[4];
    float VSSamplerMipLODBias[4];
    unsigned int VSSamplerMaxAnisotropy[4];
    unsigned int VSSamplerBorderColor[4];
    unsigned int VSSamplerMaxMipLevel[4];

    ID3D11DepthStencilState* DepthState;
    ID3D11BlendState* BlendState;
    ID3D11RasterizerState* RasterizerState;

    int DepthEnable;
    int DepthWriteEnable;
    int DepthFunc;
    int StencilEnable;
    int StencilFunc;
    int StencilFailOp;
    int StencilZFailOp;
    int StencilPassOp;
    unsigned int StencilRef;
    unsigned int StencilReadMask;
    unsigned int StencilWriteMask;
    int AlphaBlendEnable;
    int SrcBlend;
    int DestBlend;
    int BlendOp;
    int BlendOpAlpha;
    int CullMode;
    int FillMode;
    int ScissorEnable;
    unsigned int ColorWriteMask[4];
};

extern r3dDX11State g_r3dDX11State;

#endif

#endif
