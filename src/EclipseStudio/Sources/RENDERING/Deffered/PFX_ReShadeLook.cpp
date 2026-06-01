#include "r3dPCH.h"
#include "r3d.h"

#include "PostFXChief.h"
#include "PFX_ReShadeLook.h"

PFX_ReShadeLook::Settings::Settings()
: PHDRStrength(0.30f)
, PHDRExposure(0.10f)
, JaSharpen(1.00f)
, UnsharpAmount(1.00f)
, UnsharpBlurScale(1.00f)
, VignetteAmount(-1.00f)
, VignetteRadius(2.00f)
, VignetteSlope(2.00f)
, VignetteRatio(1.00f)
{
}

PFX_ReShadeLook::PFX_ReShadeLook()
: Parent(this)
{
}

PFX_ReShadeLook::~PFX_ReShadeLook()
{
}

void PFX_ReShadeLook::PushSettings(const Settings& sts)
{
    mSettings.PushBack(sts);
    mSettingsPushed = 1;
}

void PFX_ReShadeLook::InitImpl()
{
    mData.PixelShaderID = r3dRenderer->GetPixelShaderIdx("PS_RESHADER_LOOK");
}

void PFX_ReShadeLook::CloseImpl()
{
}

void PFX_ReShadeLook::PrepareImpl(r3dScreenBuffer* dest, r3dScreenBuffer* src)
{
    r3d_assert(mSettings.Count());

    const Settings& sts = mSettings[0];

    float texel[4] =
    {
        1.0f / dest->Width,
        1.0f / dest->Height,
        sts.UnsharpBlurScale,
        0.0f
    };

    float look0[4] =
    {
        sts.PHDRStrength,
        sts.PHDRExposure,
        sts.JaSharpen,
        sts.UnsharpAmount
    };

    float look1[4] =
    {
        sts.VignetteAmount,
        sts.VignetteRadius,
        sts.VignetteSlope,
        sts.VignetteRatio
    };

    D3D_V(r3dRenderer->SetPixelShaderConstantF(0, texel, 1));
    D3D_V(r3dRenderer->SetPixelShaderConstantF(1, look0, 1));
    D3D_V(r3dRenderer->SetPixelShaderConstantF(2, look1, 1));

    mSettings.Erase(0);
}

void PFX_ReShadeLook::FinishImpl()
{
}

void PFX_ReShadeLook::PushDefaultSettingsImpl()
{
    mSettings.PushBack(Settings());
}