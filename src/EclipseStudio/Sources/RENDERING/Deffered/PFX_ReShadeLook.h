#pragma once

#include "PostFX.h"

class PFX_ReShadeLook : public PostFX
{
public:
    struct Settings
    {
        Settings();

        float PHDRStrength;
        float PHDRExposure;

        float JaSharpen;
        float UnsharpAmount;
        float UnsharpBlurScale;

        float VignetteAmount;
        float VignetteRadius;
        float VignetteSlope;
        float VignetteRatio;
    };

    typedef r3dTL::TArray<Settings> SettingsVec;

public:
    PFX_ReShadeLook();
    ~PFX_ReShadeLook();

    void PushSettings(const Settings& sts);

private:
    virtual void InitImpl() OVERRIDE;
    virtual void CloseImpl() OVERRIDE;
    virtual void PrepareImpl(r3dScreenBuffer* dest, r3dScreenBuffer* src) OVERRIDE;
    virtual void FinishImpl() OVERRIDE;
    virtual void PushDefaultSettingsImpl() OVERRIDE;

private:
    SettingsVec mSettings;
};