#include "r3dPCH.h"
#include "r3d.h"

#include "PostFXChief.h"
#include "PFX_ScreenRainDrops.h"

static float ScreenRainClampFloat(float v, float minV, float maxV)
{
	if(v < minV)
		return minV;

	if(v > maxV)
		return maxV;

	return v;
}

PFX_ScreenRainDrops::Settings::Settings()
: Amount(0.0f)
, Distortion(0.025f)
, SlideSpeed(0.45f)
, Scale(1.0f)
, Streaks(0.75f)
, Wetness(0.0f)
{
}

PFX_ScreenRainDrops::PFX_ScreenRainDrops()
: Parent(this)
, mTime(0.0f)
, mPrevAddressU(D3DTADDRESS_CLAMP)
, mPrevAddressV(D3DTADDRESS_CLAMP)
, mPrevMinFilter(D3DTEXF_LINEAR)
, mPrevMagFilter(D3DTEXF_LINEAR)
, mPrevMipFilter(D3DTEXF_NONE)
{
}

PFX_ScreenRainDrops::~PFX_ScreenRainDrops()
{
}

void PFX_ScreenRainDrops::PushSettings(const Settings& sts)
{
	mSettings.PushBack(sts);
	mSettingsPushed = 1;
}

void PFX_ScreenRainDrops::InitImpl()
{
	mData.PixelShaderID = r3dRenderer->GetPixelShaderIdx("PS_SCREEN_RAIN_DROPS");
}

void PFX_ScreenRainDrops::CloseImpl()
{
}

void PFX_ScreenRainDrops::PrepareImpl(r3dScreenBuffer* dest, r3dScreenBuffer* src)
{
	r3d_assert(mSettings.Count());

	const Settings& sts = mSettings[0];

	mTime += r3dGetFrameTime();

	const float amount = ScreenRainClampFloat(sts.Amount, 0.0f, 1.0f);
	const float distortion = ScreenRainClampFloat(sts.Distortion, 0.0f, 0.15f);
	const float slideSpeed = ScreenRainClampFloat(sts.SlideSpeed, 0.0f, 3.0f);
	const float scale = ScreenRainClampFloat(sts.Scale, 0.25f, 4.0f);
	const float streaks = ScreenRainClampFloat(sts.Streaks, 0.0f, 2.0f);
	const float wetness = ScreenRainClampFloat(sts.Wetness, 0.0f, 1.0f);

	float rain0[4] =
	{
		1.0f / src->Width,
		1.0f / src->Height,
		mTime,
		amount
	};

	float rain1[4] =
	{
		distortion,
		slideSpeed,
		scale,
		streaks
	};

	float rain2[4] =
	{
		wetness,
		0.0f,
		0.0f,
		0.0f
	};

	mData.TexTransform[2] = 0.5f / src->Width;
	mData.TexTransform[3] = 0.5f / src->Height;

	D3D_V(r3dRenderer->SetPixelShaderConstantF(0, rain0, 1));
	D3D_V(r3dRenderer->SetPixelShaderConstantF(1, rain1, 1));
	D3D_V(r3dRenderer->SetPixelShaderConstantF(2, rain2, 1));

	D3D_V(r3dRenderer->pd3ddev->GetSamplerState(0, D3DSAMP_ADDRESSU, &mPrevAddressU));
	D3D_V(r3dRenderer->pd3ddev->GetSamplerState(0, D3DSAMP_ADDRESSV, &mPrevAddressV));
	D3D_V(r3dRenderer->pd3ddev->GetSamplerState(0, D3DSAMP_MINFILTER, &mPrevMinFilter));
	D3D_V(r3dRenderer->pd3ddev->GetSamplerState(0, D3DSAMP_MAGFILTER, &mPrevMagFilter));
	D3D_V(r3dRenderer->pd3ddev->GetSamplerState(0, D3DSAMP_MIPFILTER, &mPrevMipFilter));

	D3D_V(r3dRenderer->pd3ddev->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP));
	D3D_V(r3dRenderer->pd3ddev->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP));
	D3D_V(r3dRenderer->pd3ddev->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR));
	D3D_V(r3dRenderer->pd3ddev->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR));
	D3D_V(r3dRenderer->pd3ddev->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_NONE));

	mSettings.Erase(0);
}

void PFX_ScreenRainDrops::FinishImpl()
{
	D3D_V(r3dRenderer->pd3ddev->SetSamplerState(0, D3DSAMP_ADDRESSU, mPrevAddressU));
	D3D_V(r3dRenderer->pd3ddev->SetSamplerState(0, D3DSAMP_ADDRESSV, mPrevAddressV));
	D3D_V(r3dRenderer->pd3ddev->SetSamplerState(0, D3DSAMP_MINFILTER, mPrevMinFilter));
	D3D_V(r3dRenderer->pd3ddev->SetSamplerState(0, D3DSAMP_MAGFILTER, mPrevMagFilter));
	D3D_V(r3dRenderer->pd3ddev->SetSamplerState(0, D3DSAMP_MIPFILTER, mPrevMipFilter));
}

void PFX_ScreenRainDrops::PushDefaultSettingsImpl()
{
	mSettings.PushBack(Settings());
}