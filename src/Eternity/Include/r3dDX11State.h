#ifndef __R3D_DX11_STATE_H
#define __R3D_DX11_STATE_H

#ifndef WO_SERVER

struct ID3D11ShaderResourceView;
struct ID3D11SamplerState;

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
    void ApplyDefaultSamplers();

    void InvalidateCache();

private:
    bool CreateSamplers();
    void ReleaseSamplers();

    bool Initialized;

    ID3D11ShaderResourceView* BoundSRV[16];
    ID3D11SamplerState* BoundSampler[16];

    ID3D11SamplerState* Samplers[R3D_DX11_SAMPLER_MAX];
};

extern r3dDX11State g_r3dDX11State;

#endif

#endif