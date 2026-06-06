// Enter your shader in this window
//-----------------------------------------------------------------------------
// File: HDRCubeMap.fx
//
// Desc: Effect file for high dynamic range cube mapping sample.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------




#ifndef MAX_NUM_LIGHTS
#define MAX_NUM_LIGHTS 4
#endif


float4x4 g_mWorldView;
float4x4 g_mProj;
texture  g_txCubeMap;
texture  g_txCubeMap2;
texture  g_txScene;
texture  g_txDepth ;
texture  g_txColor ;
float4   g_vLightIntensity = { 15.0f, 15.0f, 15.0f, 1.0f };
float4   g_vLightPosView[MAX_NUM_LIGHTS];  // Light positions in view space
float    g_fReflectivity;                  // Reflectivity value

float4x4	mViewProj ;
float4		vCam_InvRefZ ;
float4		vMaxOffset_PixelStep_Sign ;
float4x4 	g_mInvViewProjM ;

float4x4 	g_mInvViewProj;
float4 		g_vTexcTransform;


//-----------------------------------------------------------------------------
// Sampler: samCubeMap
// Desc: Process vertex for HDR environment mapping
//-----------------------------------------------------------------------------
samplerCUBE g_samCubeMap = 
sampler_state
{
    Texture = <g_txCubeMap>;
    MinFilter = Linear;
    MagFilter = Linear;
    MipFilter = Linear;
};


samplerCUBE g_samCubeMap2 = 
sampler_state
{
    Texture = <g_txCubeMap2>;
    MinFilter = Linear;
    MagFilter = Linear;
    MipFilter = Linear;
};


sampler2D g_samScene =
sampler_state
{
    Texture = <g_txScene>;
    MinFilter = Linear;
    MagFilter = Linear;
    MipFilter = Linear;
};

sampler2D sDepth =
sampler_state
{
    Texture = <g_txDepth>;
    MinFilter = POINT;
    MagFilter = POINT;
    MipFilter = POINT;

	AddressU = Clamp ;
	AddressV = Clamp ;
};

sampler2D sColor =
sampler_state
{
    Texture = <g_txColor>;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    MipFilter = LINEAR;

	AddressU = Clamp ;
	AddressV = Clamp ;
};

void RenderVictoriousDepthVS( float4 Pos : POSITION,		                    
        		            out float4 oPos : POSITION,
		                    out float Depth : TEXCOORD0 )
{
    oPos = mul( mul( Pos, g_mWorldView ), g_mProj );

	Depth = oPos.w;
}

float4 RenderVictoriousDepthPS( in float Depth : TEXCOORD0 ) : COLOR0
{
	return Depth;
}

void RenderVictoriousReprojectVS
(
	float4 position	: POSITION,
	float2 texc		: TEXCOORD0,

	out float4 oPosition  : POSITION,
	out float2 oTexc      : TEXCOORD0,
	out float3 oWPos      : TEXCOORD1,
	out float3 oScreenPos : TEXCOORD2
)
{
	oPosition 	= position ;
	oWPos		= mul( float4( position.xyz, 1 ), g_mInvViewProj ).xyz ;
	oTexc		= texc * g_vTexcTransform.xy + g_vTexcTransform.zw ;
	oScreenPos	= position ;
}


float4 RenderVictoriousReprojectPS(	in float2 vTexc : TEXCOORD0,
									in float3 vWPos : TEXCOORD1,
									in float4 vSPos : TEXCOORD2  ) : COLOR
{
#if 0
	float3 vCam		= vCam_InvRefZ.xyz ;
	float InvRefZ	= vCam_InvRefZ ;
	float MaxOffset = vMaxOffset_PixelSize.x ;
	float PixelSize = vMaxOffset_PixelSize.y ;

	float Depth		= tex2D( sDepth, vTexc ).r * InvRefZ ;

	float3 vCamVec	= vWPos.xyz - vCam;
	float3 vRealPos	= vCam + Depth*vCamVec;
	
	float vNewPosX	= mul( float4( vRealPos, 1 ), mViewProj ).x;
	
	float2 vNewTexc = vNewPos.xy / vNewPos.w + 0.5 ;
	
	vNewTexc.y = 1 - vNewTexc.y ;
	
	float2 delta = ( vNewTexc - vTexc );
	
#if 1
	return tex2D( sColor, vTexc - delta ) ;
#else
	return tex2D( sColor, vNewTexc ) ;
#endif

#else
	float InvRefZ		= vCam_InvRefZ ;
	float MaxOffset		= vMaxOffset_PixelStep_Sign.x ;
	float PixelStep		= vMaxOffset_PixelStep_Sign.y ;
	float TextureStep	= vMaxOffset_PixelStep_Sign.z ;
	float Sign			= vMaxOffset_PixelStep_Sign.w ;	

	float ex = vSPos.x + MaxOffset ;
	
	float2 t = vTexc ;
	
	float bestFit = vSPos.x ;
	float bestDiff = 1.f ;
	
	float2 pos_xy = vSPos.xy ;	
	
#if 0
	[loop]
	do		
	{
		float d = tex2Dlod( sDepth, float4( t, 0, 0 ) ).r ;
		
		float4 tPos = float4( pos_xy * d.xx, d.x, 1.f );
	
		float3 wPOS = mul( tPos, g_mInvViewProjM );
		
		float4 repro = mul( float4( wPOS, 1 ), mViewProj ) ;
			
		float reproX = repro.x / repro.w ;
			
		float newDiff = abs( reproX - vSPos.x );
		
		[flatten]
		if( newDiff < bestDiff )
		{
			bestFit = t.x ;
			bestDiff = newDiff ;
		}
		
		t.x += TextureStep ;
		pos_xy.x += PixelStep ;
	}
	while( ( ex - pos_xy.x ) * Sign > 0 );
	
	return tex2Dlod( sColor, float4( bestFit, vTexc.y, 0.f, 0.f ) );
	
#else

	float stx = 0.f ;
	float sw = 0.f ;
	[loop]
	do		
	{
		float d = tex2Dlod( sDepth, float4( t, 0, 0 ) ).r ;
		
		float4 tPos = float4( pos_xy * d.xx, d.x, 1.f );
	
		float3 wPOS = mul( tPos, g_mInvViewProjM );
		
		float4 repro = mul( float4( wPOS, 1 ), mViewProj ) ;
			
		float reproX = repro.x / repro.w ;
			
		float newDiff = abs( reproX - vSPos.x );
		
		float w = saturate( ( 0.011f - newDiff ) / 0.0001 ) ;
		
		sw += w ;
		stx += t.x * w ;
		
		t.x += TextureStep ;
		pos_xy.x += PixelStep ;
		
	}
	while( ( ex - pos_xy.x ) * Sign > 0 );	

	return tex2Dlod( sColor, float4( stx / sw, vTexc.y, 0.f, 0.f ) ) ;	
	
#endif
		
#endif
}

//-----------------------------------------------------------------------------
// Vertex Shader: HDRVertEnvMap
// Desc: Process vertex for HDR environment mapping
//-----------------------------------------------------------------------------
void HDRVertEnvMap( float4 Pos : POSITION,
                    float3 Normal : NORMAL,
                    out float4 oPos : POSITION,
                    out float3 EnvTex : TEXCOORD0 )
{
    oPos = mul( Pos, g_mWorldView );

    //
    // Compute normal in camera space
    //
    float3 vN = mul( Normal, g_mWorldView );
    vN = normalize( vN );

    //
    // Obtain the reverse eye vector
    //
    float3 vEyeR = -normalize( oPos );

    //
    // Compute the reflection vector
    //
    float3 vRef = 2 * dot( vEyeR, vN ) * vN - vEyeR;

    //
    // Store the reflection vector in texcoord0
    //
    EnvTex = vRef;

    //
    // Apply the projection
    //
    oPos = mul( oPos, g_mProj );
}


//-----------------------------------------------------------------------------
// Pixel Shader: HDRPixEnvMap
// Desc: Process pixel for HDR environment mapped object
//-----------------------------------------------------------------------------
float4 HDRPixEnvMap( float3 Tex : TEXCOORD0 ) : COLOR
{
    return g_fReflectivity * texCUBE( g_samCubeMap, Tex );
}


float4 HDRPixEnvMap2Tex( float3 Tex : TEXCOORD0 ) : COLOR
{
    return g_fReflectivity * float4( texCUBE( g_samCubeMap, Tex ).xy, texCUBE( g_samCubeMap2, Tex ).xy );
}


//-----------------------------------------------------------------------------
// Vertex Shader: HDRVertScene
// Desc: Process vertex for HDR-enabled scene
//-----------------------------------------------------------------------------
void HDRVertScene( float4 iPos : POSITION,
                   float3 iNormal : NORMAL,
                   float2 iTex : TEXCOORD0,
                   out float4 oPos : POSITION,
                   out float2 Tex : TEXCOORD0,
                   out float3 Pos : TEXCOORD1,
                   out float3 Normal : TEXCOORD2 )
{
    //
    // Transform position to view space
    //
    oPos = mul( iPos, g_mWorldView );

    //
    // Also write view position to texcoord1 to do per-pixel lighting
    //
    Pos = oPos;

    //
    // Transform to screen coord
    //
    oPos = mul( oPos, g_mProj );

    //
    // Transform normal and write to texcoord2 for per-pixel lighting
    //
    Normal = normalize( mul( iNormal, (float3x3)g_mWorldView ) );
    
    //
    // Propagate texture coord
    //
    Tex = iTex;
}


//-----------------------------------------------------------------------------
// Pixel Shader: HDRPixScene
// Desc: Process pixel (do per-pixel lighting) for HDR-enabled scene
//-----------------------------------------------------------------------------
float4 HDRPixScene( float2 Tex : TEXCOORD0,
                    float3 Pos : TEXCOORD1,
                    float3 Normal : TEXCOORD2 ) : COLOR
{
    float3 N = normalize( Normal );

    // Variable to save lit value by each light
    float4 vPixValue = (float4)0;

    //
    // Iterate through each light and apply the light on the pixel
    //
    for( int LightIndex = 0; LightIndex < MAX_NUM_LIGHTS; ++LightIndex )
    {
        //
        // Compute light vector (pixel to light)
        //
        float3 vRLightVec = (float3)(g_vLightPosView[LightIndex] - Pos);

        //
        // Find out the light intensity at the vertex based on
        // N dot L and distance from the light.
        //
        float fDiffuse = saturate( dot( normalize( vRLightVec ), N ) );

        //
        // Attenuation is 1 / D^2. Clamp at 1 to avoid infinity.
        //
        float fAttenuation = saturate( 1.0f / dot( vRLightVec, vRLightVec ) );

        //
        // Compute and add pixel color to final value
        //
        vPixValue += fDiffuse * fAttenuation;
    }

    return tex2D( g_samScene, Tex ) * vPixValue * g_vLightIntensity;
}


float4 HDRPixScene_FirstHalf( float2 Tex : TEXCOORD0,
                              float3 Pos : TEXCOORD1,
                              float3 Normal : TEXCOORD2 ) : COLOR
{
    return HDRPixScene( Tex, Pos, Normal ).xyzw;
}


float4 HDRPixScene_SecondHalf( float2 Tex : TEXCOORD0,
                               float3 Pos : TEXCOORD1,
                               float3 Normal : TEXCOORD2 ) : COLOR
{
    return HDRPixScene( Tex, Pos, Normal ).zwww;
}


//-----------------------------------------------------------------------------
// Vertex Shader: HDRVertLight
// Desc: Process vertex for light objects
//-----------------------------------------------------------------------------
void HDRVertLight( float4 iPos : POSITION,
                   out float4 oPos : POSITION,
                   out float4 Diffuse : TEXCOORD1 )
{
    //
    // Transform position to screen space
    //
    oPos = mul( iPos, g_mWorldView );
    oPos = mul( oPos, g_mProj );

    //
    // Diffuse color is the light intensity value
    //
    Diffuse = g_vLightIntensity;
}


//-----------------------------------------------------------------------------
// Pixel Shader: HDRPixLight
// Desc: Process pixel for HDR-enabled scene
//-----------------------------------------------------------------------------
float4 HDRPixLight( float4 Diffuse : TEXCOORD1 ) : COLOR
{
    //
    // Diffuse has the full intensity of the light.
    // Just output it.
    //
    return Diffuse;
}


float4 HDRPixLight_FirstHalf( float4 Diffuse : TEXCOORD1 ) : COLOR
{
    //
    // Diffuse has the full intensity of the light.
    // Just output it.
    //
    return Diffuse.xyww;
}


float4 HDRPixLight_SecondHalf( float4 Diffuse : TEXCOORD1 ) : COLOR
{
    //
    // Diffuse has the full intensity of the light.
    // Just output it.
    //
    return Diffuse.zwww;
}


//-----------------------------------------------------------------------------
// Technique: RenderScene
// Desc: Renders scene objects
//-----------------------------------------------------------------------------
technique RenderScene
{
    pass p0
    {
        VertexShader = compile vs_3_0 HDRVertScene();
        PixelShader = compile ps_3_0 HDRPixScene();
    }
}


technique RenderSceneFirstHalf
{
    pass p0
    {
        VertexShader = compile vs_3_0 HDRVertScene();
        PixelShader = compile ps_3_0 HDRPixScene_FirstHalf();
    }
}


technique RenderSceneSecondHalf
{
    pass p0
    {
        VertexShader = compile vs_3_0 HDRVertScene();
        PixelShader = compile ps_3_0 HDRPixScene_SecondHalf();
    }
}


//-----------------------------------------------------------------------------
// Technique: RenderLight
// Desc: Renders light objects
//-----------------------------------------------------------------------------
technique RenderLight
{
    pass p0
    {
        VertexShader = compile vs_3_0 HDRVertLight();
        PixelShader = compile ps_3_0 HDRPixLight();
    }
}


technique RenderLightFirstHalf
{
    pass p0
    {
        VertexShader = compile vs_3_0 HDRVertLight();
        PixelShader = compile ps_3_0 HDRPixLight_FirstHalf();
    }
}


technique RenderLightSecondHalf
{
    pass p0
    {
        VertexShader = compile vs_3_0 HDRVertLight();
        PixelShader = compile ps_3_0 HDRPixLight_SecondHalf();
    }
}


//-----------------------------------------------------------------------------
// Technique: RenderEnvMesh
// Desc: Renders the HDR environment-mapped mesh
//-----------------------------------------------------------------------------
technique RenderHDREnvMap
{
    pass p0
    {
        VertexShader = compile vs_3_0 HDRVertEnvMap();
        PixelShader = compile ps_3_0 HDRPixEnvMap();
    }
}


technique RenderHDREnvMap2Tex
{
    pass p0
    {
        VertexShader = compile vs_3_0 HDRVertEnvMap();
        PixelShader = compile ps_3_0 HDRPixEnvMap2Tex();
    }
}

technique RenderVictoriousDepth
{
    pass p0
    {
        VertexShader = compile vs_3_0 RenderVictoriousDepthVS();
        PixelShader = compile ps_3_0 RenderVictoriousDepthPS();
    }
}

technique RenderVictoriousReproject
{
    pass p0
    {
        VertexShader = compile vs_3_0 RenderVictoriousReprojectVS();
        PixelShader = compile ps_3_0 RenderVictoriousReprojectPS();
    }
}

