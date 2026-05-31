#include "r3dPCH.h"
#include "r3d.h"

#include "WeatherPuddleManager.h"

#include "TrueNature/ITerrain.h"
#include "GameLevel.h"
#include "../../../../Eternity/SF/Console/Config.h"

extern r3dCamera gCam;

WeatherPuddleManager gWeatherPuddleManager;

static float WPM_ClampFloat( float v, float lo, float hi )
{
	return v < lo ? lo : ( v > hi ? hi : v );
}

static int WPM_ClampInt( int v, int lo, int hi )
{
	return v < lo ? lo : ( v > hi ? hi : v );
}

static float WPM_Rand01()
{
	return rand() / (float)RAND_MAX;
}

static float WPM_MaxFloat( float a, float b )
{
	return a > b ? a : b;
}

WeatherPuddleManager::WeatherPuddleManager()
	: mPuddleCount( 0 )
	, mTypeId( INVALID_DECAL_ID )
	, mSpawnAccum( 0.0f )
	, mLastTime( 0.0f )
	, mLastReflection( -1.0f )
{
	for( int i = 0; i < MAX_WEATHER_PUDDLES; ++i )
	{
		mPuddles[ i ].DecalId = INVALID_DECAL_ID;
		mPuddles[ i ].LifeLeft = 0.0f;
		mPuddles[ i ].AnimSeed = 0.0f;
		mPuddles[ i ].NextAnimTime = 0.0f;
	}
}

void WeatherPuddleManager::Init()
{
	mLastTime = r3dGetTime();
	RegisterType();
}

void WeatherPuddleManager::Close()
{
	RemoveAll();
	mTypeId = INVALID_DECAL_ID;
}

void WeatherPuddleManager::RegisterType()
{
	if( !g_pDecalChief )
		return;

	mTypeId = g_pDecalChief->GetTypeID( "weather_puddle" );

	if( mTypeId == INVALID_DECAL_ID )
	{
		DecalType type;
		type.Name = "weather_puddle";
		type.DiffuseTexName = "Data\\Water\\Puddles\\puddles_perlin.dds";
		type.NormalTexName = "Data\\Water\\Puddles\\water_normal.dds";
		type.LifeTime = 0.0f;
		type.ScaleX = 1.0f;
		type.ScaleY = 1.0f;
		type.UniformScale = 0;
		type.ScaleVar = 0.0f;
		type.RandomRotation = 0;
		type.BackgroundBlendFactor = 0.15f + r_weather_puddles_reflection->GetFloat() * 0.75f;
		type.ClipFarFactor = 0.075f;
		type.ClipNearFactor = 1.0f;
		type.MinQuality = 1;
		type.globalDecal = true;
		type.WriteGloss = 1;

		g_pDecalChief->AddType( type );
		mTypeId = g_pDecalChief->GetTypeID( "weather_puddle" );
	}

	UpdateTypeParams();
}

void WeatherPuddleManager::UpdateTypeParams()
{
	if( !g_pDecalChief || mTypeId == INVALID_DECAL_ID )
		return;

	const float reflection = WPM_ClampFloat( r_weather_puddles_reflection->GetFloat(), 0.0f, 1.0f );

	if( fabsf( reflection - mLastReflection ) < 0.001f )
		return;

	DecalType type = g_pDecalChief->GetTypeByIdx( mTypeId );
	type.BackgroundBlendFactor = 0.15f + reflection * 0.75f;
	type.WriteGloss = reflection > 0.01f ? 1 : 0;

	g_pDecalChief->UpdateType( mTypeId, type );
	mLastReflection = reflection;
}

void WeatherPuddleManager::RemoveAt( int idx )
{
	if( idx < 0 || idx >= mPuddleCount )
		return;

	if( g_pDecalChief && mPuddles[ idx ].DecalId != INVALID_DECAL_ID )
		g_pDecalChief->Remove( mPuddles[ idx ].DecalId );

	if( idx != mPuddleCount - 1 )
		mPuddles[ idx ] = mPuddles[ mPuddleCount - 1 ];

	--mPuddleCount;
}

void WeatherPuddleManager::RemoveAll()
{
	while( mPuddleCount > 0 )
		RemoveAt( mPuddleCount - 1 );

	mSpawnAccum = 0.0f;
}

int WeatherPuddleManager::AcquireSlot( int maxCount )
{
	maxCount = WPM_ClampInt( maxCount, 0, MAX_WEATHER_PUDDLES );

	if( maxCount <= 0 )
		return -1;

	if( mPuddleCount < maxCount )
		return mPuddleCount++;

	int idx = 0;
	float leastLife = mPuddles[ 0 ].LifeLeft;

	for( int i = 1; i < mPuddleCount; ++i )
	{
		if( mPuddles[ i ].LifeLeft < leastLife )
		{
			leastLife = mPuddles[ i ].LifeLeft;
			idx = i;
		}
	}

	if( g_pDecalChief && mPuddles[ idx ].DecalId != INVALID_DECAL_ID )
		g_pDecalChief->Remove( mPuddles[ idx ].DecalId );

	return idx;
}

void WeatherPuddleManager::UpdateExisting( float dt )
{
	const float now = r3dGetTime();
	const float keepRadius = WPM_ClampFloat( r_weather_puddles_radius->GetFloat(), 4.0f, 160.0f ) * 1.75f;
	const float keepRadiusSq = keepRadius * keepRadius;
	const int animateNormal = r_weather_puddles_normal_anim->GetBool() ? 1 : 0;
	const float animSpeed = r_weather_puddles_normal_speed->GetFloat();

	for( int i = 0; i < mPuddleCount; )
	{
		PuddleSlot& slot = mPuddles[ i ];

		slot.LifeLeft -= dt;

		const float dx = slot.Params.Pos.x - gCam.x;
		const float dz = slot.Params.Pos.z - gCam.z;
		const float distSq = dx * dx + dz * dz;

		if( slot.LifeLeft <= 0.0f || distSq > keepRadiusSq )
		{
			RemoveAt( i );
			continue;
		}

		if( animateNormal && g_pDecalChief && now >= slot.NextAnimTime )
		{
			slot.Params.ZRot += animSpeed * dt * ( 0.65f + slot.AnimSeed );

			if( slot.Params.ZRot > R3D_PI * 2.0f )
				slot.Params.ZRot -= R3D_PI * 2.0f;

			slot.Params.LifeTime = slot.LifeLeft;
			g_pDecalChief->UpdateDecalParams( slot.DecalId, slot.Params );

			slot.NextAnimTime = now + 0.18f + WPM_Rand01() * 0.16f;
		}

		++i;
	}
}

void WeatherPuddleManager::TrySpawn( float rain, float wetness )
{
	if( !g_pDecalChief || !Terrain || !Terrain->IsLoaded() )
		return;

	RegisterType();

	if( mTypeId == INVALID_DECAL_ID )
		return;

	const int maxCount = WPM_ClampInt( r_weather_puddles_max_count->GetInt(), 0, MAX_WEATHER_PUDDLES );

	if( maxCount <= 0 )
		return;

	const float radius = WPM_ClampFloat( r_weather_puddles_radius->GetFloat(), 4.0f, 160.0f );
	const float angle = WPM_Rand01() * R3D_PI * 2.0f;
	const float dist = sqrtf( WPM_Rand01() ) * radius;

	r3dPoint3D pos;
	pos.x = gCam.x + cosf( angle ) * dist;
	pos.z = gCam.z + sinf( angle ) * dist;
	pos.y = Terrain->GetHeight( pos ) + 0.035f;

	r3dPoint3D normal = Terrain->GetNormal( pos );
	normal.Normalize();

	if( normal.y < r_weather_puddles_min_normal_y->GetFloat() )
		return;

	const float spacing = r_weather_puddles_min_spacing->GetFloat();

	if( spacing > 0.0f )
	{
		for( int i = 0; i < mPuddleCount; ++i )
		{
			const float dx = mPuddles[ i ].Params.Pos.x - pos.x;
			const float dz = mPuddles[ i ].Params.Pos.z - pos.z;

			if( sqrtf( dx * dx + dz * dz ) < spacing )
				return;
		}
	}

	const float minSize = r_weather_puddles_min_size->GetFloat();
	const float maxSize = WPM_MaxFloat( minSize, r_weather_puddles_max_size->GetFloat() );
	const float size = minSize + ( maxSize - minSize ) * WPM_Rand01();

	DecalParams params;
	params.TypeID = mTypeId;
	params.Pos = pos;
	params.Dir = normal;
	params.LifeTime = r_weather_puddles_life->GetFloat() * ( 0.55f + wetness * 0.45f );
	params.ZRot = WPM_Rand01() * R3D_PI * 2.0f;
	params.ScaleCoef = size;
	params.Static = 0;

	const int decalId = g_pDecalChief->Add( params, false );

	if( decalId == INVALID_DECAL_ID )
		return;

	const int slotIdx = AcquireSlot( maxCount );

	if( slotIdx < 0 )
	{
		g_pDecalChief->Remove( decalId );
		return;
	}

	PuddleSlot& slot = mPuddles[ slotIdx ];
	slot.DecalId = decalId;
	slot.Params = params;
	slot.LifeLeft = params.LifeTime;
	slot.AnimSeed = WPM_Rand01();
	slot.NextAnimTime = r3dGetTime() + WPM_Rand01() * 0.25f;
}

void WeatherPuddleManager::Update()
{
	const float now = r3dGetTime();
	const float dt = WPM_ClampFloat( now - mLastTime, 0.0f, 0.1f );
	mLastTime = now;

	if( !r_weather_puddles->GetBool() || !r_decals->GetBool() )
	{
		RemoveAll();
		return;
	}

	UpdateTypeParams();
	UpdateExisting( dt );

	const int maxCount = WPM_ClampInt( r_weather_puddles_max_count->GetInt(), 0, MAX_WEATHER_PUDDLES );

	while( mPuddleCount > maxCount )
		RemoveAt( mPuddleCount - 1 );

	const float rain = WPM_ClampFloat( r3dGameLevel::Environment.GetRainStrength(), 0.0f, 1.0f );
	const float wetness = WPM_ClampFloat( r3dGameLevel::Environment.GetWetness(), 0.0f, 1.0f );

	r_weather_rain_intensity->SetFloat( rain );
	r_weather_wetness->SetFloat( wetness );

	if( wetness < r_weather_puddles_min_wetness->GetFloat() )
		return;

	if( rain <= 0.001f && wetness <= 0.001f )
		return;

	const float density = WPM_ClampFloat( r_weather_puddles_density->GetFloat(), 0.0f, 1.0f );
	const float spawnRate = r_weather_puddles_spawn_rate->GetFloat() * density * WPM_MaxFloat( rain, wetness );

	mSpawnAccum += dt * spawnRate;

	while( mSpawnAccum >= 1.0f )
	{
		TrySpawn( rain, wetness );
		mSpawnAccum -= 1.0f;
	}
}