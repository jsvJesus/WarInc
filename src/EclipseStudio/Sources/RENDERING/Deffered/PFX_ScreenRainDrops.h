#pragma once

#include "PostFX.h"

class PFX_ScreenRainDrops : public PostFX
{
public:
    struct Settings
    {
        Settings();

        float Amount;
        float Distortion;
        float SlideSpeed;
        float Scale;
        float Streaks;
        float Wetness;
    };

    typedef r3dTL::TArray<Settings> SettingsVec;

public:
    PFX_ScreenRainDrops();
    ~PFX_ScreenRainDrops();

    void PushSettings(const Settings& sts);

private:
    virtual void InitImpl() OVERRIDE;
    virtual void CloseImpl() OVERRIDE;
    virtual void PrepareImpl(r3dScreenBuffer* dest, r3dScreenBuffer* src) OVERRIDE;
    virtual void FinishImpl() OVERRIDE;
    virtual void PushDefaultSettingsImpl() OVERRIDE;

private:
    SettingsVec mSettings;

    float mTime;

    DWORD mPrevAddressU;
    DWORD mPrevAddressV;
    DWORD mPrevMinFilter;
    DWORD mPrevMagFilter;
    DWORD mPrevMipFilter;
};