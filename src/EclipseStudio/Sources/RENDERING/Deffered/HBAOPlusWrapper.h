// ═══════════════════════════════════════════════════════════════
// NVIDIA HBAO+ Wrapper for WarInc D3D11 integration
// ═══════════════════════════════════════════════════════════════
#ifndef __HBAO_PLUS_WRAPPER_H__
#define __HBAO_PLUS_WRAPPER_H__

#ifndef WO_SERVER

#ifndef _WINDOWS_
#include <windows.h>
#endif

#include <dxgi.h>
#include <d3d11.h>
#include "GFSDK_SSAO.h"

struct r3dScreenBuffer;

class HBAOPlusWrapper
{
public:
    HBAOPlusWrapper();
    ~HBAOPlusWrapper();

    bool Init(ID3D11Device* device, ID3D11DeviceContext* ctx, ID3D11DepthStencilView* depthDSV, int width, int height);
    void Shutdown();
    bool IsInitialized() const { return m_initialized; }

    void SetParameters(const GFSDK_SSAO_Parameters& params);
    void SetDefaultParameters();

    // Render AO to output buffer. depthDSV should be the main scene depth.
    bool RenderAO(
        ID3D11DeviceContext* ctx,
        const D3DXMATRIX& projMatrix,
        float metersToViewSpaceUnits,
        ID3D11RenderTargetView* outputRTV,
        int outputWidth = 0,
        int outputHeight = 0);

    // Render AO and copy result to r3dScreenBuffer (for dual-renderer integration)
    bool RenderAOToScreenBuffer(
        const D3DXMATRIX& projMatrix,
        float metersToViewSpaceUnits,
        r3dScreenBuffer* outputBuffer);

    GFSDK_SSAO_Parameters& GetParameters() { return m_params; }

private:
    bool CreateDepthSRV(ID3D11Device* device, ID3D11DepthStencilView* depthDSV);
    bool CreateTempRTV(ID3D11Device* device, int width, int height);
    void ReleaseResources();

    bool m_initialized;
    int m_width;
    int m_height;
    int m_tempWidth;
    int m_tempHeight;

    GFSDK_SSAO_Context_D3D11* m_aoContext;
    GFSDK_SSAO_Parameters m_params;

    ID3D11ShaderResourceView* m_depthSRV;
    ID3D11Texture2D* m_tempAOTexture;
    ID3D11RenderTargetView* m_tempAORTV;
    ID3D11Texture2D* m_readbackTexture;
};

extern HBAOPlusWrapper g_HBAOPlus;

#endif // WO_SERVER
#endif // __HBAO_PLUS_WRAPPER_H__
