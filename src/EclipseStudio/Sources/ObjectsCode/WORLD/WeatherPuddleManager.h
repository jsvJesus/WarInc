#pragma once

#include "ObjectsCode/WORLD/DecalChief.h"

class WeatherPuddleManager
{
public:
    WeatherPuddleManager();

    void Init();
    void Close();
    void Update();

private:
    enum
    {
        MAX_WEATHER_PUDDLES = 512
    };

    struct PuddleSlot
    {
        int DecalId;
        DecalParams Params;
        float LifeLeft;
        float AnimSeed;
        float NextAnimTime;
    };

    PuddleSlot mPuddles[ MAX_WEATHER_PUDDLES ];
    int mPuddleCount;
    int mTypeId;
    float mSpawnAccum;
    float mLastTime;
    float mLastReflection;

    void RegisterType();
    void UpdateTypeParams();
    void RemoveAt( int idx );
    void RemoveAll();
    void UpdateExisting( float dt );
    void TrySpawn( float rain, float wetness );
    int AcquireSlot( int maxCount );
};

extern WeatherPuddleManager gWeatherPuddleManager;