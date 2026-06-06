#include "DXUT.h"
#include "DXUTsettingsdlg.h"
#include "DXUTcamera.h"
#include "SDKmesh.h"
#include "SDKmisc.h"
#include "resource.h"

#include "Stereo/nvapi.h"

#include "Stereo.h"

#include <time.h>

void DoErrorBox( const char* message )
{
	MessageBoxA( NULL, message, "Fuck", MB_OK );
}

#define D3D_V(expr) if((expr) != D3D_OK ) { DoErrorBox( "D3D call failed:" #expr ); }
#define NV_V(expr) if((expr) != NVAPI_OK ) { DoErrorBox( "NV API call failed:" #expr ); }

// Stereo Blitdefines
#define NVSTEREO_IMAGE_SIGNATURE 0x4433564e //NV3D
typedef struct Nv_Stereo_Image_Header
{
	unsigned int dwSignature;
	unsigned int dwWidth;
	unsigned int dwHeight;
	unsigned int dwBPP;
	unsigned int dwFlags;
} NVSTEREOIMAGEHEADER, *LPNVSTEREOIMAGEHEADER;
// ORedflags in the dwFlagsfielsof the _Nv_Stereo_Image_Headerstructure above
#define SIH_SWAP_EYES 0x00000001
#define SIH_SCALE_TO_FIT 0x00000002

int gImageWidth		= 1680; // Source image width
int gImageHeight	= 1050;// Source image height

IDirect3DSurface9* gImageSrc= NULL; // Source stereo image beeingcreated

IDirect3DDevice9 * gD3DDev ;

StereoHandle gStereoHandle ;

IDirect3DTexture9 * gImageSrcLeft ;
IDirect3DTexture9 * gImageSrcRight ;

IDirect3DSurface9* gTargetLeft ;
IDirect3DSurface9* gTargetRight ;

IDirect3DSurface9* gTargetCentre ;
IDirect3DTexture9* gTargetCentreTex ;

IDirect3DSurface9* gDepth ;
IDirect3DTexture9* gDepthTex ;

IDirect3DVertexDeclaration9* gScreenQuadDecl ;


void Init3D( IDirect3DDevice9 * D3DDev )
{
	gD3DDev = D3DDev ;

	NvAPI_Initialize();

	NV_V( NvAPI_Stereo_CreateHandleFromIUnknown ( D3DDev, &gStereoHandle ) );
}

bool Is3DOn()
{
	NvU8 IsStereoOn;
	NV_V( NvAPI_Stereo_IsActivated( gStereoHandle, &IsStereoOn ) );

#if FAKE_STEREO
	return true ;
#else
	return IsStereoOn ? true : false;
#endif
}

void Start3D( int imageWidth, int imageHeight )
{
	Stop3D();

	gImageWidth		= imageWidth ;
	gImageHeight	= imageHeight ;

	IDirect3DSurface9* backBuffer ;

	D3D_V( gD3DDev->GetBackBuffer( 0, 0, D3DBACKBUFFER_TYPE_MONO, &backBuffer ) );

	D3DSURFACE_DESC desc ;

	D3D_V( backBuffer->GetDesc( &desc ) );

	backBuffer->Release();

#if 0
	gD3DDev->CreateOffscreenPlainSurface(	gImageWidth * 2, // Stereo width is twice the source width
											gImageHeight + 1, // Stereo height add one raw to encode signature
											D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, // Surface is in video memory
											&gImageSrc, NULL );
#else
	D3D_V( gD3DDev->CreateRenderTarget
				(
					gImageWidth * 2,
					gImageHeight + 1,
					D3DFMT_A8R8G8B8,
					D3DMULTISAMPLE_NONE,
					0,
					TRUE,
					&gImageSrc,
					NULL
				)
			);
#endif

	D3D_V( gD3DDev->CreateTexture(
		gImageWidth,
		gImageHeight-1,
		1,
		D3DUSAGE_RENDERTARGET,
		D3DFMT_A8R8G8B8,
		D3DPOOL_DEFAULT,
		&gTargetCentreTex,
		NULL ) );

	D3D_V( gTargetCentreTex->GetSurfaceLevel( 0, &gTargetCentre ) );

	D3D_V( gD3DDev->CreateRenderTarget(
		gImageWidth,
		gImageHeight-1,
		D3DFMT_A8R8G8B8,
		D3DMULTISAMPLE_NONE,
		0,
		FALSE,
		&gTargetLeft,
		NULL
		) );


	D3D_V( gD3DDev->CreateRenderTarget(
		gImageWidth,
		gImageHeight-1,
		D3DFMT_A8R8G8B8,
		D3DMULTISAMPLE_NONE,
		0,
		FALSE,
		&gTargetRight,
		NULL
		) );

	D3D_V( gD3DDev->CreateTexture(
		gImageWidth,
		gImageHeight-1,
		1,
		D3DUSAGE_RENDERTARGET,
		D3DFMT_R32F,
		D3DPOOL_DEFAULT,
		&gDepthTex,
		NULL ) );

	
	D3DVERTEXELEMENT9 aVertDecl[] =
	{
		{ 0, 0,  D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
		{ 0, 12, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
		D3DDECL_END()
	};

	D3D_V( gD3DDev->CreateVertexDeclaration( aVertDecl, &gScreenQuadDecl ) );

	D3D_V( gDepthTex->GetSurfaceLevel( 0, &gDepth ) );

#if 0
	D3D_V( D3DXCreateTextureFromFileEx( gD3DDev, L"left.png", gImageWidth, gImageHeight - 1, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, D3DX_FILTER_LINEAR, D3DX_FILTER_LINEAR, 0, NULL, NULL, &gImageSrcLeft ) );
	D3D_V( D3DXCreateTextureFromFileEx( gD3DDev, L"right.png", gImageWidth, gImageHeight - 1, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, D3DX_FILTER_LINEAR, D3DX_FILTER_LINEAR, 0, NULL, NULL, &gImageSrcRight ) );
#endif
}

void Stop3D()
{
	SAFE_RELEASE( gImageSrc );
	SAFE_RELEASE( gImageSrcLeft );
	SAFE_RELEASE( gImageSrcRight );
	SAFE_RELEASE( gTargetCentre );
	SAFE_RELEASE( gTargetCentreTex );
	SAFE_RELEASE( gTargetLeft );
	SAFE_RELEASE( gTargetRight );
	SAFE_RELEASE( gDepth );
	SAFE_RELEASE( gDepthTex );
	SAFE_RELEASE( gScreenQuadDecl );
}

void CalcStereoViewProj( CBaseCamera* cam, D3DXMATRIX* leftView, D3DXMATRIX* leftProj, D3DXMATRIX* rightView, D3DXMATRIX* rightProj, float strength )
{
	FLOAT fFOV, fAspect, fNearPlane, fFarPlane;

	cam->GetProjParams( &fFOV, &fAspect, &fNearPlane, &fFarPlane );

	float height = fNearPlane * tanf( fFOV * 0.5f ) ;
	float width = height * fAspect;

	D3DXMATRIX persp ;

	float sep_on = strength ;

	float sep = 0.33f * width * sep_on ;
	float conv = 0.1111f * sep_on ;

	float psep = sep * conv ;

	D3DXMATRIX offsetLeft, offsetRight, view;

	D3DXMatrixTranslation( &offsetLeft, +sep, 0.f, 0.f );
	D3DXMatrixTranslation( &offsetRight, -sep, 0.f, 0.f );

	D3DXMatrixPerspectiveOffCenterLH( &persp, -width + psep, width + psep, -height, height, fNearPlane, fFarPlane );

	*leftView = *cam->GetViewMatrix() * offsetLeft ;
	*leftProj = persp ;

	D3DXMatrixPerspectiveOffCenterLH( &persp, -width - psep, width - psep, -height, height, fNearPlane, fFarPlane );

	*rightView = *cam->GetViewMatrix() * offsetRight ;
	*rightProj = persp ;

}

void DoDraw( ID3DXEffect* eff )
{
	UINT p, cPass;

	D3D_V( eff->Begin( &cPass, 0 ) );

	for( p = 0; p < cPass; ++p )
	{
		D3D_V( eff->BeginPass( p ) );

		D3D_V( gD3DDev->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE ) );

		D3D_V( gD3DDev->SetVertexDeclaration( gScreenQuadDecl ) );

		float VertArr[6][5] = 
		{
			{	-1.f, -1.f, 0.f,	0.f, 1.f },
			{	+1.f, -1.f, 0.f,	1.f, 1.f },
			{	-1.f, +1.f, 0.f,	0.f, 0.f },

			{	-1.f, +1.f, 0.f,	0.f, 0.f },
			{	+1.f, -1.f, 0.f,	1.f, 1.f },
			{	+1.f, +1.f, 0.f,	1.f, 0.f },
		};

		D3D_V( gD3DDev->DrawPrimitiveUP( D3DPT_TRIANGLELIST, 2, VertArr, 5 * 4 ) );

		D3D_V( eff->EndPass() );
	}

	D3D_V( eff->End() );

}

void Trick3D( ID3DXEffect* eff, CBaseCamera* cam, bool test, bool usePPStereoscopy, float stereoStrength )
{
	 NV_V( NvAPI_Stereo_SetSeparation ( gStereoHandle, 0.f ) );
	 NV_V( NvAPI_Stereo_SetConvergence ( gStereoHandle, 0.f ) );

#if 0
	D3D_V( gD3DDev->StretchRect( gTargetLeft, NULL, gTargetRight, NULL, D3DTEXF_NONE ) );
#elif 0
	float displace = ( sinf( clock() / 50.f ) + 1.f ) * 50.f ;

	int displacei = (int)displace;

	RECT dstRect = { displacei, displacei, gImageWidth - displacei, gImageHeight - 1 - displacei };

	D3D_V( gD3DDev->StretchRect( gTargetLeft, NULL, gTargetRight, &dstRect, D3DTEXF_LINEAR ) );
#elif 1
	if( usePPStereoscopy )
	{
		IDirect3DSurface9* BackBuffer = NULL ;

		D3D_V( gD3DDev->GetRenderTarget( 0, &BackBuffer ) );

		D3DXMATRIX view, proj ;
		
		D3DXVECTOR3 camPos;

		float NearPlane ;

		view = *cam->GetViewMatrix();
		proj = *cam->GetProjMatrix();

		NearPlane = cam->GetNearClip();

		D3D_V( eff->SetTechnique( eff->GetTechniqueByName( "RenderVictoriousReproject" ) ) );

		D3DXMATRIX leftView, leftProj, rightView, rightProj, halfScale ;

		D3DXMatrixScaling( &halfScale, 0.5f, 0.5f, 1.0f );

		CalcStereoViewProj( cam, &leftView, &leftProj, &rightView, &rightProj, stereoStrength );

		// VS Stuff
		{	
			
			D3DXMATRIX invViewProj, invView, invProj, nearUpScale ;

			D3DXMatrixInverse( &invView, NULL, &view ) ;
			D3DXMatrixInverse( &invProj, NULL, &proj ) ;

			D3DXMatrixIdentity( &nearUpScale ) ;

			nearUpScale._11 = NearPlane ;
			nearUpScale._22 = NearPlane ;
			nearUpScale._33 = NearPlane ;
			nearUpScale._44 = NearPlane ;

			invViewProj = nearUpScale * invProj * invView ;

			eff->SetMatrix( eff->GetParameterByName( NULL, "g_mInvViewProj" ), &invViewProj ) ;

			D3DXVECTOR4 TexcTransform( 1.0f, 1.0f, 0.5f / gImageWidth, 0.5f / ( gImageHeight - 1 ) ) ;

			eff->SetVector( eff->GetParameterByName( NULL, "g_vTexcTransform" ), &TexcTransform );

		}


		// PS Stuff
		{
			D3DXVECTOR3 camPos = *cam->GetEyePt();

			D3DXVECTOR4 camInvRefZ( camPos.x, camPos.y, camPos.z, 1.f / NearPlane );

			eff->SetVector( eff->GetParameterByName( NULL, "vCam_InvRefZ" ), &camInvRefZ ) ;

			eff->SetTexture( eff->GetParameterByName( NULL, "g_txDepth" ), gDepthTex ) ;
			eff->SetTexture( eff->GetParameterByName( NULL, "g_txColor" ), gTargetCentreTex ) ;

			D3DXMATRIX invViewProj, invView, invProj ;

			D3DXMATRIX altProj ;
			altProj = proj ;
			altProj._33 = 1.f ;
			altProj._43 = 0.f ;
			altProj._34 = 0.f ;
			altProj._44 = 1.f ;

			D3DXMatrixInverse( &invView, NULL, &view ) ;
			D3DXMatrixInverse( &invProj, NULL, &altProj ) ;
	
			invViewProj = invProj * invView ;

#if 0
			D3DXVECTOR4 TestVec( -1.f, 0.f, 5.f, 1.f ) ;
			D3DXVec4Transform( &TestVec, &TestVec, &invViewProj );

			D3DXVECTOR4 TestVec2;

			D3DXMATRIX viewProj = view * proj ;

			D3DXVec4Transform( &TestVec2, &TestVec, &viewProj );
#endif

			eff->SetMatrix( eff->GetParameterByName( NULL, "g_mInvViewProjM" ), &invViewProj ) ;
		}

		// Left PS stuff
		{
			D3DXMATRIX viewProj = leftView * leftProj ;
#if 0
			viewProj = view * proj * halfScale;
#endif
			eff->SetMatrix( eff->GetParameterByName( NULL, "mViewProj" ), &viewProj ) ;

			//------------------------------------------------------------------------
			D3DXVECTOR4 vMaxOffset_PixelStep_Sign ;

			vMaxOffset_PixelStep_Sign.x = -leftProj._31 ;
			vMaxOffset_PixelStep_Sign.y = 2.f / gImageWidth ;
			vMaxOffset_PixelStep_Sign.z = 1.f / gImageWidth ;
			vMaxOffset_PixelStep_Sign.w = 1.f ;

			eff->SetVector( eff->GetParameterByName( NULL, "vMaxOffset_PixelStep_Sign" ), &vMaxOffset_PixelStep_Sign ) ;
		}

		gD3DDev->SetRenderTarget( 0, gTargetLeft ) ;

		DoDraw( eff );

		// Right PS stuff
		{
			D3DXMATRIX viewProj = rightView * rightProj ;
#if 0
			viewProj = view * proj * halfScale;
#endif
			eff->SetMatrix( eff->GetParameterByName( NULL, "mViewProj" ), &viewProj ) ;

			//------------------------------------------------------------------------
			D3DXVECTOR4 vMaxOffset_PixelStep_Sign ;

			vMaxOffset_PixelStep_Sign.x = -rightProj._31 ; 
			vMaxOffset_PixelStep_Sign.y = -2.f / gImageWidth ; 
			vMaxOffset_PixelStep_Sign.z = -1.f / gImageWidth ; 
			vMaxOffset_PixelStep_Sign.w = -1.f ;

			eff->SetVector( eff->GetParameterByName( NULL, "vMaxOffset_PixelStep_Sign" ), &vMaxOffset_PixelStep_Sign ) ;
		}

		gD3DDev->SetRenderTarget( 0, gTargetRight ) ;

		DoDraw( eff );

		gD3DDev->SetRenderTarget( 0, BackBuffer ) ;

		SAFE_RELEASE( BackBuffer );

	}
#endif

#if !FAKE_STEREO
	// Blitleft srcimage to left side of stereo
	{
		RECT dstRect = { 0, 0, gImageWidth, gImageHeight - 1 };

		IDirect3DSurface9* surf = gTargetRight ; 

		if( test )
		{
			D3D_V( gImageSrcRight->GetSurfaceLevel( 0, &surf ) );
		}

		D3D_V( gD3DDev->StretchRect( surf, NULL, gImageSrc, &dstRect, D3DTEXF_NONE ) );

		if( test )
		{
			surf->Release();
		}
	}

	// Blitleft srcimage to left side of stereo
	{
		RECT dstRect = { gImageWidth, 0, 2*gImageWidth, gImageHeight };

		IDirect3DSurface9* surf = gTargetLeft ;

		if( test )
		{
			D3D_V( gImageSrcLeft->GetSurfaceLevel( 0, &surf ) );
		}

		D3D_V( gD3DDev->StretchRect( surf, NULL, gImageSrc, &dstRect, D3DTEXF_NONE ) );

		if( test )
		{
			surf->Release();
		}
	}

	// Lock the stereo image
	D3DLOCKED_RECT lr;
	D3D_V( gImageSrc->LockRect( &lr, NULL, 0 ) );

	// write stereo signature in the last raw of the stereo image
	LPNVSTEREOIMAGEHEADER pSIH = (LPNVSTEREOIMAGEHEADER)(((unsigned char *) lr.pBits) + (lr.Pitch* gImageHeight));

	// Update the signature header values
	pSIH->dwSignature	= NVSTEREO_IMAGE_SIGNATURE;
	pSIH->dwBPP			= 32;
	pSIH->dwFlags		= SIH_SWAP_EYES; // Srcimage has left on left and right on right
	pSIH->dwWidth		= gImageWidth*2;
	pSIH->dwHeight		= gImageHeight;
	// Unlock surface
	D3D_V( gImageSrc->UnlockRect() );

	IDirect3DSurface9* bb;

	D3D_V( gD3DDev->GetRenderTarget( 0, &bb ) );

	D3D_V( gD3DDev->StretchRect( gImageSrc, NULL, bb, NULL, D3DTEXF_NONE ) );

	bb->Release() ;
#else
	// Blitleft srcimage to left side of stereo
	{
		RECT dstRect = { 0, 0, gImageWidth, gImageHeight - 1 };

		IDirect3DSurface9* surf = gTargetLeft ; 

		IDirect3DSurface9* BackBuffer = NULL ;

		D3D_V( gD3DDev->GetRenderTarget( 0, &BackBuffer ) );

		D3D_V( gD3DDev->StretchRect( surf, NULL, BackBuffer, &dstRect, D3DTEXF_NONE ) );

		BackBuffer->Release() ;
	}
#endif
}

IDirect3DSurface9* GetCenterTarget()
{
	return gTargetCentre ;
}

IDirect3DSurface9* GetLeftTarget()
{
	return gTargetLeft ;
}

IDirect3DSurface9* GetRightTarget()
{
	return gTargetRight ;
}

IDirect3DSurface9* GetDepth()
{
	return gDepth ;
}
