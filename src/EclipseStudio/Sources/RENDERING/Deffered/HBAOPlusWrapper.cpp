// ═══════════════════════════════════════════════════════════════
// NVIDIA HBAO+ Wrapper — Implementation
// ═══════════════════════════════════════════════════════════════
#include "r3dPCH.h"

#ifndef WO_SERVER

#include "HBAOPlusWrapper.h"
#include "r3d.h"
#include "r3dDX11.h"
#include "r3dRender.h"

#pragma comment(lib, "GFSDK_SSAO_D3D11.win64.lib")

HBAOPlusWrapper g_HBAOPlus;

HBAOPlusWrapper::HBAOPlusWrapper()
    : m_initialized(false)
    , m_width(0)
    , m_height(0)
    , m_aoContext(NULL)
    , m_depthSRV(NULL)
    , m_tempAOTexture(NULL)
    , m_tempAORTV(NULL)
    , m_readbackTexture(NULL)
{
    SetDefaultParameters();
}

HBAOPlusWrapper::~HBAOPlusWrapper()
{
    Shutdown();
}

void HBAOPlusWrapper::SetDefaultParameters()
{
    memset(&m_params, 0, sizeof(m_params));

    m_params.Radius = 1.5f;
    m_params.Bias = 0.1f;
    m_params.PowerExponent = 2.0f;
    m_params.SmallScaleAO = 1.0f;
    m_params.LargeScaleAO = 1.0f;
    m_params.ForegroundAO.Multiplier = 1.0f;
    m_params.ForegroundAO.Enable = true;
    m_params.BackgroundAO.Multiplier = 1.0f;
    m_params.BackgroundAO.Enable = true;

    m_params.Blur.Enable = true;
    m_params.Blur.Radius = GFSDK_SSAO_BLUR_RADIUS_4;
    m_params.Blur.Sharpness = 16.0f;
    m_params.Blur.SharpnessProfile.Enable = false;

    m_params.DepthStorage = GFSDK_SSAO_FP32_VIEW_DEPTHS;
    m_params.DepthThreshold = 0.0f;
    m_params.EnableDualLayerAO = false;
}

void HBAOPlusWrapper::SetParameters(const GFSDK_SSAO_Parameters& params)
{
    m_params = params;
}

bool HBAOPlusWrapper::Init(ID3D11Device* device, ID3D11DeviceContext* ctx, ID3D11DepthStencilView* depthDSV, int width, int height)
{
    Shutdown();

    if (!device || !depthDSV || width <= 0 || height <= 0)
    {
        r3dOutToLog("HBAO+: Init failed — invalid parameters\n");
        return false;
    }

    m_width = width;
    m_height = height;

    // 1. Create depth SRV from depth DSV
    if (!CreateDepthSRV(device, depthDSV))
    {
        Shutdown();
        return false;
    }

    // 2. Create temp AO render target
    if (!CreateTempRTV(device, width, height))
    {
        Shutdown();
        return false;
    }

    // 3. Create HBAO+ context
    GFSDK_SSAO_CustomHeap customHeap;
    customHeap.new_ = ::operator new;
    customHeap.delete_ = ::operator delete;

    GFSDK_SSAO_Status status = GFSDK_SSAO_CreateContext_D3D11(device, &m_aoContext, &customHeap);
    if (status != GFSDK_SSAO_OK)
    {
        r3dOutToLog("HBAO+: GFSDK_SSAO_CreateContext_D3D11 failed, status=%d\n", (int)status);
        Shutdown();
        return false;
    }

    m_initialized = true;
    r3dOutToLog("HBAO+: initialized successfully (%dx%d)\n", width, height);
    return true;
}

void HBAOPlusWrapper::Shutdown()
{
    ReleaseResources();

    if (m_aoContext)
    {
        m_aoContext->Release();
        m_aoContext = NULL;
    }

    m_initialized = false;
}

bool HBAOPlusWrapper::CreateDepthSRV(ID3D11Device* device, ID3D11DepthStencilView* depthDSV)
{
    if (!depthDSV)
        return false;

    // Get the underlying depth texture from the DSV
    ID3D11Resource* depthResource = NULL;
    depthDSV->GetResource(&depthResource);
    if (!depthResource)
        return false;

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    memset(&srvDesc, 0, sizeof(srvDesc));

    D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
    depthDSV->GetDesc(&dsvDesc);

    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = 1;
    srvDesc.Texture2D.MostDetailedMip = 0;

    // Determine SRV format from DSV format
    switch (dsvDesc.Format)
    {
    case DXGI_FORMAT_D32_FLOAT:
    case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
        srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
        break;
    case DXGI_FORMAT_D24_UNORM_S8_UINT:
        srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
        break;
    case DXGI_FORMAT_D16_UNORM:
        srvDesc.Format = DXGI_FORMAT_R16_UNORM;
        break;
    default:
        srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
        break;
    }

    HRESULT hr = device->CreateShaderResourceView(depthResource, &srvDesc, &m_depthSRV);
    depthResource->Release();

    if (FAILED(hr))
    {
        r3dOutToLog("HBAO+: CreateShaderResourceView for depth failed hr=0x%08X\n", (unsigned int)hr);
        return false;
    }

    return true;
}

bool HBAOPlusWrapper::CreateTempRTV(ID3D11Device* device, int width, int height)
{
    D3D11_TEXTURE2D_DESC texDesc;
    memset(&texDesc, 0, sizeof(texDesc));
    texDesc.Width = width;
    texDesc.Height = height;
    texDesc.MipLevels = 1;
    texDesc.ArraySize = 1;
    texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    texDesc.SampleDesc.Count = 1;
    texDesc.SampleDesc.Quality = 0;
    texDesc.Usage = D3D11_USAGE_DEFAULT;
    texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    texDesc.CPUAccessFlags = 0;
    texDesc.MiscFlags = 0;

    HRESULT hr = device->CreateTexture2D(&texDesc, NULL, &m_tempAOTexture);
    if (FAILED(hr))
    {
        r3dOutToLog("HBAO+: CreateTexture2D for AO target failed hr=0x%08X\n", (unsigned int)hr);
        return false;
    }

    D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
    memset(&rtvDesc, 0, sizeof(rtvDesc));
    rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
    rtvDesc.Texture2D.MipSlice = 0;

    hr = device->CreateRenderTargetView(m_tempAOTexture, &rtvDesc, &m_tempAORTV);
    if (FAILED(hr))
    {
        r3dOutToLog("HBAO+: CreateRenderTargetView for AO failed hr=0x%08X\n", (unsigned int)hr);
        return false;
    }

    // CPU readback staging texture
    texDesc.Usage = D3D11_USAGE_STAGING;
    texDesc.BindFlags = 0;
    texDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    texDesc.MiscFlags = 0;

    hr = device->CreateTexture2D(&texDesc, NULL, &m_readbackTexture);
    if (FAILED(hr))
    {
        r3dOutToLog("HBAO+: CreateTexture2D for readback failed hr=0x%08X\n", (unsigned int)hr);
    }

    return true;
}

void HBAOPlusWrapper::ReleaseResources()
{
    if (m_readbackTexture) { m_readbackTexture->Release(); m_readbackTexture = NULL; }
    if (m_tempAORTV) { m_tempAORTV->Release(); m_tempAORTV = NULL; }
    if (m_tempAOTexture) { m_tempAOTexture->Release(); m_tempAOTexture = NULL; }
    if (m_depthSRV) { m_depthSRV->Release(); m_depthSRV = NULL; }
}

bool HBAOPlusWrapper::RenderAO(
    ID3D11DeviceContext* ctx,
    const D3DXMATRIX& projMatrix,
    float metersToViewSpaceUnits,
    ID3D11RenderTargetView* outputRTV,
    int outputWidth,
    int outputHeight)
{
    if (!m_initialized || !ctx || !m_depthSRV || !outputRTV)
        return false;

    int w = outputWidth > 0 ? outputWidth : m_width;
    int h = outputHeight > 0 ? outputHeight : m_height;

    D3D11_VIEWPORT vp;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    vp.Width = (float)w;
    vp.Height = (float)h;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    ctx->RSSetViewports(1, &vp);

    GFSDK_SSAO_InputData_D3D11 input;
    memset(&input, 0, sizeof(input));

    input.DepthData.DepthTextureType = GFSDK_SSAO_HARDWARE_DEPTHS;
    input.DepthData.pFullResDepthTextureSRV = m_depthSRV;
    input.DepthData.pFullResDepthTexture2ndLayerSRV = NULL;

    input.DepthData.ProjectionMatrix.Data = GFSDK_SSAO_Float4x4((const GFSDK_SSAO_FLOAT*)&projMatrix);
    input.DepthData.ProjectionMatrix.Layout = GFSDK_SSAO_ROW_MAJOR_ORDER;
    input.DepthData.MetersToViewSpaceUnits = metersToViewSpaceUnits;

    input.NormalData.Enable = false;

    GFSDK_SSAO_Output_D3D11 output;
    memset(&output, 0, sizeof(output));
    output.pRenderTargetView = outputRTV;

    GFSDK_SSAO_Status status = m_aoContext->RenderAO(
        ctx, input, m_params, output, GFSDK_SSAO_RENDER_AO);

    if (status != GFSDK_SSAO_OK)
    {
        r3dOutToLog("HBAO+: RenderAO failed, status=%d\n", (int)status);
        return false;
    }

    return true;
}

bool HBAOPlusWrapper::RenderAOToScreenBuffer(
    const D3DXMATRIX& projMatrix,
    float metersToViewSpaceUnits,
    r3dScreenBuffer* outputBuffer)
{
    if (!m_initialized || !outputBuffer)
        return false;

    ID3D11DeviceContext* ctx = g_r3dDX11.GetContext();
    if (!ctx)
        return false;

    // Render HBAO+ to temp RTV
    if (!RenderAO(ctx, projMatrix, metersToViewSpaceUnits, m_tempAORTV, m_width, m_height))
        return false;

    // Copy from temp RTV to readback staging texture
    ctx->CopyResource(m_readbackTexture, m_tempAOTexture);

    // CPU readback to get AO data
    D3D11_MAPPED_SUBRESOURCE mapped;
    HRESULT hr = ctx->Map(m_readbackTexture, 0, D3D11_MAP_READ, 0, &mapped);
    if (FAILED(hr))
    {
        r3dOutToLog("HBAO+: Map for readback failed hr=0x%08X\n", (unsigned int)hr);
        return false;
    }

    // Upload AO to r3dScreenBuffer (R8G8B8A8 → writes R channel = AO)
    // r3dScreenBuffer uses D3D9, so we need to write through the D3D9 device
    // or use r3dTexture update mechanism
    // For now, just log success
    ctx->Unmap(m_readbackTexture, 0);

    return true;
}

#endif // WO_SERVER
