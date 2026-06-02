#include "r3dPCH.h"
#include "r3d.h"

#include "r3dBuffer.h"

#include "r3dDeviceQueue.h"

#ifndef WO_SERVER

#pragma warning(push)
#pragma warning(disable: 4005)
#pragma warning(disable: 4061)
#pragma warning(disable: 4062)
#pragma warning(disable: 4191)
#pragma warning(disable: 4255)
#pragma warning(disable: 4365)
#pragma warning(disable: 4571)
#pragma warning(disable: 4625)
#pragma warning(disable: 4626)
#pragma warning(disable: 4668)
#pragma warning(disable: 4710)
#pragma warning(disable: 4711)
#pragma warning(disable: 4820)
#include <dxgi.h>
#include <d3d11.h>
#pragma warning(pop)

#include "r3dDX11.h"
#include "r3dDX11Geometry.h"
#include "VShader.h"

#endif

static	HRESULT		hr;

#ifndef WO_SERVER

static int r3dD3DQuery_GetDX11Type(D3DQUERYTYPE type, D3D11_QUERY* outType)
{
	if(!outType)
		return 0;

	switch(type)
	{
	case D3DQUERYTYPE_EVENT:
		*outType = D3D11_QUERY_EVENT;
		return 1;

	case D3DQUERYTYPE_OCCLUSION:
		*outType = D3D11_QUERY_OCCLUSION;
		return 1;

	case D3DQUERYTYPE_TIMESTAMP:
		*outType = D3D11_QUERY_TIMESTAMP;
		return 1;

	case D3DQUERYTYPE_TIMESTAMPFREQ:
		*outType = D3D11_QUERY_TIMESTAMP_DISJOINT;
		return 1;
	}

	return 0;
}

static void r3dD3DQuery_WriteCounterValue(void* pData, DWORD dwSize, UINT64 value)
{
	if(!pData || !dwSize)
		return;

	if(dwSize >= sizeof(UINT64))
	{
		*(UINT64*)pData = value;
		return;
	}

	if(dwSize >= sizeof(DWORD))
	{
		*(DWORD*)pData = value > 0xffffffffull ? 0xffffffffu : (DWORD)value;
	}
}

#endif

r3dD3DQuery::r3dD3DQuery(D3DQUERYTYPE type, const r3dIntegrityGuardian& ig /*= r3dIntegrityGuardian()*/ )
: r3dIResource( ig )
{
	type_ = type;
	query_ = NULL;
#ifndef WO_SERVER
	dx11Query_ = NULL;
	dx11Open_ = 0;
#endif
	CreateQueuedResource( this ) ;
}

r3dD3DQuery::~r3dD3DQuery()
{
	ReleaseQueuedResource( this ) ;
}

void r3dD3DQuery::D3DCreateResource()
{
	R3D_ENSURE_MAIN_THREAD();

#ifndef WO_SERVER
	if(g_r3dDX11.IsInitialized() && g_r3dDX11.GetDevice())
	{
		D3D11_QUERY dx11Type;
		if(r3dD3DQuery_GetDX11Type(type_, &dx11Type))
		{
			D3D11_QUERY_DESC desc;
			ZeroMemory(&desc, sizeof(desc));
			desc.Query = dx11Type;

			HRESULT dx11hr = g_r3dDX11.GetDevice()->CreateQuery(&desc, &dx11Query_);
			if(FAILED(dx11hr))
			{
				r3dOutToLog(
					"DX11Query: CreateQuery failed, D3D9 type=%d HRESULT=0x%08X\n",
					(int)type_,
					(unsigned int)dx11hr
				);
			}
		}
	}
#endif

	if(r3dRenderer && r3dRenderer->pd3ddev)
	{
		D3D_V( r3dRenderer->pd3ddev->CreateQuery(type_, &query_) ) ;
	}
}

void r3dD3DQuery::D3DReleaseResource()
{
	R3D_ENSURE_MAIN_THREAD();
	SAFE_RELEASE(query_);

#ifndef WO_SERVER
	SAFE_RELEASE(dx11Query_);
	dx11Open_ = 0;
#endif
}

HRESULT r3dD3DQuery::Issue(DWORD dwIssueFlags)
{
	R3D_ENSURE_MAIN_THREAD();

#ifndef WO_SERVER
	if(g_r3dDX11.IsInitialized() && dx11Query_)
	{
		ID3D11DeviceContext* context = g_r3dDX11.GetContext();
		if(!context)
			return D3DERR_INVALIDCALL;

		if(type_ == D3DQUERYTYPE_OCCLUSION || type_ == D3DQUERYTYPE_TIMESTAMPFREQ)
		{
			if(dwIssueFlags & D3DISSUE_BEGIN)
			{
				context->Begin(dx11Query_);
				dx11Open_ = 1;
			}

			if(dwIssueFlags & D3DISSUE_END)
			{
				if(type_ == D3DQUERYTYPE_TIMESTAMPFREQ && !dx11Open_)
				{
					context->Begin(dx11Query_);
				}

				context->End(dx11Query_);
				dx11Open_ = 0;
			}

			return S_OK;
		}

		if(dwIssueFlags & D3DISSUE_END)
		{
			context->End(dx11Query_);
			return S_OK;
		}

		return S_OK;
	}
#endif

	return query_ ? query_->Issue(dwIssueFlags) : D3DERR_INVALIDCALL;
}

HRESULT r3dD3DQuery::GetData(void* pData, DWORD dwSize, DWORD dwGetDataFlags)
{
	R3D_ENSURE_MAIN_THREAD();

#ifndef WO_SERVER
	if(g_r3dDX11.IsInitialized() && dx11Query_)
	{
		ID3D11DeviceContext* context = g_r3dDX11.GetContext();
		if(!context)
			return D3DERR_INVALIDCALL;

		const UINT flags = (dwGetDataFlags & D3DGETDATA_FLUSH) ? 0 : D3D11_ASYNC_GETDATA_DONOTFLUSH;

		if(!pData || !dwSize)
			return context->GetData(dx11Query_, NULL, 0, flags);

		if(type_ == D3DQUERYTYPE_EVENT)
			return context->GetData(dx11Query_, pData, dwSize, flags);

		if(type_ == D3DQUERYTYPE_OCCLUSION || type_ == D3DQUERYTYPE_TIMESTAMP)
		{
			UINT64 value = 0;
			HRESULT dx11hr = context->GetData(dx11Query_, &value, sizeof(value), flags);
			if(dx11hr == S_OK)
				r3dD3DQuery_WriteCounterValue(pData, dwSize, value);

			return dx11hr;
		}

		if(type_ == D3DQUERYTYPE_TIMESTAMPFREQ)
		{
			D3D11_QUERY_DATA_TIMESTAMP_DISJOINT data;
			ZeroMemory(&data, sizeof(data));

			HRESULT dx11hr = context->GetData(dx11Query_, &data, sizeof(data), flags);
			if(dx11hr == S_OK)
			{
				if(data.Disjoint)
					return S_FALSE;

				r3dD3DQuery_WriteCounterValue(pData, dwSize, data.Frequency);
			}

			return dx11hr;
		}
	}
#endif

	return query_ ? query_->GetData(pData, dwSize, dwGetDataFlags) : D3DERR_INVALIDCALL;
}

//------------------------------------------------------------------------
/*static*/
r3dD3DSurfaceTunnel r3dScreenBuffer::ShadowZBuf		;
r3dD3DTextureTunnel r3dScreenBuffer::ShadowZBufTex	;
r3dD3DSurfaceTunnel r3dScreenBuffer::NullRT			;

/*static*/ int r3dScreenBuffer::ActivateGuard ;

struct ShadowZBuffer : r3dIResource
{
	ShadowZBuffer( const r3dIntegrityGuardian& ig )
	: r3dIResource( ig )
	{
		X = 64 ;
		Y = 64 ;
		UsedMemory = 0 ;
	}

	static void CreateZSurfaces(	r3dD3DTextureTunnel* oTexture,
									r3dD3DSurfaceTunnel* oDepthStencil,
									r3dD3DSurfaceTunnel* oNullRT,
									int X, int Y,
									int* oUsedMemory
								)
	{
		bool needTex = false ;
		D3DFORMAT fmt = D3DFMT_D24X8 ;

		switch( r_hardware_shadow_method->GetInt() )
		{
		case HW_SHADOW_METHOD_R32F:
			needTex = false ;			
			break;
		case HW_SHADOW_METHOD_INTZ:
			fmt = INTZ_FORMAT ;
		case HW_SHADOW_METHOD_HW_PCF:
			needTex = true ;
			break;
		}

		*oUsedMemory = 0 ;

		if( needTex )
		{
			r3dDeviceTunnel::CreateTexture( X, Y, 1, D3DUSAGE_DEPTHSTENCIL, fmt, D3DPOOL_DEFAULT, oTexture ) ;

			IDirect3DSurface9* surf ;
			oTexture->AsTex2D()->GetSurfaceLevel( 0, &surf ) ;

			oDepthStencil->Set( surf ) ;

			D3DFORMAT nullFmt = r3dRenderer->SupportsNULLRenderTarget ? NULL_RT_FORMAT : D3DFMT_R5G6B5 ;
			r3dDeviceTunnel::CreateRenderTarget( X, Y, nullFmt, D3DMULTISAMPLE_NONE, 0, FALSE, oNullRT ) ;

			if( !r3dRenderer->SupportsNULLRenderTarget )
			{
				*oUsedMemory += int( GetD3DTexFormatSize( nullFmt ) * (int)(X * Y) ) ;
			}
		}
		else
		{
			oTexture->Set( 0 ) ;
			r3dDeviceTunnel::CreateDepthStencilSurface( X, Y, fmt, D3DMULTISAMPLE_NONE, 1, TRUE, oDepthStencil ) ;
		}

		*oUsedMemory += 4* (int)(X * Y) ;
	}

	virtual	void D3DCreateResource()  OVERRIDE
	{
		R3D_ENSURE_MAIN_THREAD();

		r3d_assert( !r3dScreenBuffer::ShadowZBuf.Get() );

		CreateZSurfaces( &r3dScreenBuffer::ShadowZBufTex, &r3dScreenBuffer::ShadowZBuf, &r3dScreenBuffer::NullRT, X, Y, &UsedMemory ) ;
		r3dRenderer->Stats.AddRenderTargetMem ( UsedMemory ) ;
	}

	virtual	void D3DReleaseResource() OVERRIDE
	{
		R3D_ENSURE_MAIN_THREAD();

		r3d_assert( r3dScreenBuffer::ShadowZBuf.Get() );

		D3DSURFACE_DESC desc ;

		D3D_V( r3dScreenBuffer::ShadowZBuf->GetDesc( &desc ) )

		r3dScreenBuffer::ShadowZBuf.ReleaseAndReset() ;
		r3dRenderer->Stats.AddRenderTargetMem ( - UsedMemory ) ;

		r3dScreenBuffer::ShadowZBufTex.ReleaseAndReset() ;

		D3DFORMAT formatForNull = r3dScreenBuffer::NullRT.GetFormat() ;

		r3dScreenBuffer::NullRT.ReleaseAndReset() ;
	}

	int X, Y ;
	int UsedMemory ;
} *gShadowZBuffer;

/*static*/
void
r3dScreenBuffer::CreateShadowZBuffer( int X, int Y )
{
	r3d_assert( !gShadowZBuffer );

	r3dIntegrityGuardian ig ;

	gShadowZBuffer = new ShadowZBuffer( ig ) ;

	gShadowZBuffer->X = X ;
	gShadowZBuffer->Y = Y ;

	gShadowZBuffer->D3DCreateResource() ;
}

//------------------------------------------------------------------------
/*static*/

void
r3dScreenBuffer::ReleaseShadowZBuffer()
{
	if (!gShadowZBuffer)
		return;

	gShadowZBuffer->D3DReleaseResource();

	delete gShadowZBuffer ;

	gShadowZBuffer = NULL ;
}

r3dScreenBuffer::r3dScreenBuffer(const CreationParameters &p)
: r3dIResource( p.ig )
{
	r3dRenderer->RegisterScreenBuffer( this ) ;

	memset( MipDims, 0, sizeof MipDims ) ;

	debug_name				= p.name;

	Width					= p.width ;
	Height					= p.height ;

	BufferFormat    		= p.Format;
	zType					= p.ZT;
	BufferID        		= 0;
	bCubemap        		= p.bCubemap;
	CreationNumMipLevels	= p.numMipLevels;
	ActualNumMipLevels		= CreationNumMipLevels ;

	UsedMemory				= 0 ;
	Tex = 0;

	AllowAutoScaleDown = p.allowAutoScaleDown;

	D3DCreateResource();
}

//////////////////////////////////////////////////////////////////////////

r3dScreenBuffer::r3dScreenBuffer(const char * name)
: r3dIResource( r3dIntegrityGuardian( ) )
, debug_name(name)
, BufferFormat(D3DFMT_UNKNOWN)
, Width( 0 )
, Height( 0 )
, zType(Z_OWN)
, BufferID(0)
, bCubemap(false)
, CreationNumMipLevels(0)
, ActualNumMipLevels(0)
, Tex(0)
, AllowAutoScaleDown( 0 )
, UsedMemory( 0 )
{
	r3dRenderer->RegisterScreenBuffer( this ) ;
	memset( MipDims, 0, sizeof MipDims ) ;
}

r3dScreenBuffer::~r3dScreenBuffer()
{
	R3D_ENSURE_MAIN_THREAD();
	D3DReleaseResource();

	r3dRenderer->UnregisterScreenBuffer( this ) ;
}

void r3dScreenBuffer::SetDebugD3DComment(const char* text)
{
	if (!Tex) return;
	Tex->SetDebugD3DComment(text);
}

const char*
r3dScreenBuffer::GetDebugName() const
{
	return debug_name ? debug_name : "Unknown" ;
}

void SetD3DResourcePrivateData(LPDIRECT3DRESOURCE9 res, const char* FName);

//------------------------------------------------------------------------

static void SetupSurfaces( void* param )
{
	r3dScreenBuffer* screenBuffer = (r3dScreenBuffer*) param ;

	IDirect3DSurface9* surf ;

	r3dRenderer->GetRT(0, &surf );

	screenBuffer->BBuf.Set( surf ) ;

	r3dRenderer->GetDSS( &surf );

	screenBuffer->ZBuf.Set( surf ) ;
}

bool IsDepthTextureFormat( D3DFORMAT fmt )
{
	return
		fmt == D3DFMT_D24X8 
			||
			fmt == INTZ_FORMAT ;
}

static void SetupSurfacePointers( void* params )
{
	r3dScreenBuffer* sbuf = (r3dScreenBuffer*) params ;

	IDirect3DSurface9* surf ;

	if( sbuf->bCubemap )
	{
		if (sbuf->AsTexCUBE())
		{
			for( int i = 0, e = r3dScreenBuffer::FACE_COUNT; i < e; i ++ )
			{
				for( int m = 0, e = sbuf->AsTexCUBE()->GetLevelCount() ; m < e ; m ++ )
				{
					D3D_V(sbuf->AsTexCUBE()->GetCubeMapSurface( (D3DCUBEMAP_FACES)i, m, &surf ));
					sbuf->Surfs[ i ][ m ].Set( surf );

#ifndef WO_SERVER
					if(sbuf->Tex)
						sbuf->Tex->RegisterDX11RenderTargetSurface(&sbuf->Surfs[ i ][ m ], i, m);
#endif
				}				
			}
		}
	}
	else
	{
		if (sbuf->AsTex2D())
		{
			for( int m = 0, e = sbuf->AsTex2D()->GetLevelCount() ; m < e ; m ++ )
			{
				D3D_V(sbuf->AsTex2D()->GetSurfaceLevel(m, &surf ));
				sbuf->Surfs[ 0 ][ m ].Set( surf ) ;

#ifndef WO_SERVER
				if(sbuf->Tex)
					sbuf->Tex->RegisterDX11RenderTargetSurface(&sbuf->Surfs[ 0 ][ m ], 0, m);
#endif
			}
		}
	}
}

void r3dScreenBuffer::D3DCreateResource()
{
	if (Width == 0 || Height == 0) return;

	r3d_assert( !OurBBuf.Valid() );

	// Store the current back buffer and z-buffer.

	ProcessCustomDeviceQueueItem( SetupSurfaces, this ) ;

	UINT intW = static_cast<UINT>(Width);
	UINT intH = static_cast<UINT>(Height);

	if( BufferFormat != D3DFMT_UNKNOWN )
	{
		if( IsDepthTextureFormat( BufferFormat ) )
		{
			Tex = r3dRenderer->AllocateTexture();

			if( zType == r3dScreenBuffer::Z_SHADOW )
			{
				ShadowZBufTex->AddRef() ;
				Tex->Setup( intW, intH, 1, BufferFormat, 1, &ShadowZBufTex, true ) ;
				Surfs[ 0 ][ 0 ] = r3dScreenBuffer::NullRT ;
				Surfs[ 0 ][ 0 ]->AddRef() ;
			}
			else
			{
				ShadowZBuffer::CreateZSurfaces( &OurBBuf, &OurZBuf, &Surfs[ 0 ][ 0 ], intW, intH, &UsedMemory );
				Tex->Setup( intW, intH, 1, BufferFormat, 1, &OurBBuf, true );
			}
		}
		else
		{
			if( bCubemap )
			{
				r3d_assert( intW == intH );
				r3dDeviceTunnel::CreateCubeTexture( intW, CreationNumMipLevels, D3DUSAGE_RENDERTARGET, BufferFormat, D3DPOOL_DEFAULT, &OurBBuf ) ;
			}
			else
			{
				if( AllowAutoScaleDown )
				{
					UINT W = intW ;
					UINT H = intH ;

					UINT L = (UINT)CreationNumMipLevels ;

					r3dDeviceTunnel::CreateTextureAutoDownScale( &W, &H, &L, D3DUSAGE_RENDERTARGET, BufferFormat, D3DPOOL_DEFAULT, &OurBBuf ) ;

					Width = (float)W ;
					Height = (float)H ;

					intW = W ;
					intH = H ;
				}
				else
					r3dDeviceTunnel::CreateTexture(intW, intH, CreationNumMipLevels, D3DUSAGE_RENDERTARGET, BufferFormat, D3DPOOL_DEFAULT, &OurBBuf ) ;
			}

			Tex = r3dRenderer->AllocateTexture();

			if( bCubemap )
				Tex->SetupCubemap( intW, BufferFormat, ActualNumMipLevels, &OurBBuf, true );
			else
				Tex->Setup( intW, intH, 1, BufferFormat, ActualNumMipLevels, &OurBBuf, true );

			Tex->SetDebugD3DComment("r3dScreenBuffer: Color");

			UsedMemory += r3dGetPixelSize(intW * intH, BufferFormat) * ( bCubemap ? 6 : 1 ) ;

			if( zType == Z_OWN ) 
			{
				r3d_assert( !OurZBuf.Valid() );

				r3dDeviceTunnel::CreateDepthStencilSurface(intW, intH, ZBuf.GetFormat(), D3DMULTISAMPLE_NONE, 0, FALSE, &OurZBuf ) ;

				r3dDeviceTunnel::SetD3DResourcePrivateData( &OurZBuf, "r3dScreenBuffer: DepthStencil");
				UsedMemory += r3dGetPixelSize(intW*intH, ZBuf.GetFormat() ) ;
			}
			ProcessCustomDeviceQueueItem( SetupSurfacePointers, this ) ;
		}

		memset( MipDims, 0, sizeof MipDims ) ;

		if( IsDepthTextureFormat( BufferFormat ) )
		{
			ActualNumMipLevels = 1 ;
			
			Dims &dms = MipDims[ 0 ] ;

			dms.Width	= Width ;
			dms.Height	= Height ;
		}
		else
		{
			ActualNumMipLevels = OurBBuf.GetLevelCount() ;

			for( int i = 0, e = ActualNumMipLevels ; i < e ; i ++ )
			{
				D3DSURFACE_DESC desc ;
				OurBBuf.GetLevelDesc2D( i, &desc ) ;

				Dims &dms = MipDims[ i ] ;

				dms.Width = (float)desc.Width ;
				dms.Height = (float)desc.Height ;

				if( i )
				{
					UsedMemory += r3dGetPixelSize( (int)dms.Width* (int)dms.Height, BufferFormat) * ( bCubemap ? 6 : 1 ) ;
				}
			}
		}

		r3dRenderer->Stats.AddRenderTargetMem ( UsedMemory );
	}
}

void r3dScreenBuffer::D3DReleaseResource()
{
	R3D_ENSURE_MAIN_THREAD();

	if( Surfs[ 0 ][ 0 ].Get() )
	{
		r3dRenderer->DeleteTexture(Tex, true);
		Tex		= NULL ;
		OurBBuf.Set( 0 );	// OurBBuf is already released as Tex->Tex

		r3dRenderer->Stats.AddRenderTargetMem ( -UsedMemory ) ;
		UsedMemory = 0 ;

		if ( OurZBuf.Get() && zType == Z_OWN )
		{
			OurZBuf.ReleaseAndReset();
		}

		BBuf.ReleaseAndReset();
		ZBuf.ReleaseAndReset();

		int refCount = 0 ;

		for( int i = 0, e = FACE_COUNT; i < e; i ++ )
		{
			for( int m = 0, e = ActualNumMipLevels ; m < e ; m ++  )
			{
				refCount = Surfs[ i ][ m ].ReleaseAndReset() ;
			}			
		}

		// have to watch last ref count with cubemaps somehow
		if( refCount > 0 )
		{
			r3dOutToLog( "r3dScreenBuffer::D3DReleaseResource: non zero reference count after release! (%s, %d refs)\n", this->debug_name ?this->debug_name : "Unknown", refCount ) ;
		}

	}

	return;
}



void r3dScreenBuffer::SetMRT(int RTID, int bSet)
{
	if (!Tex) return;

	if (!bSet)
	{
		r3dRenderer->SetRT(RTID, NULL);
		return;
	}

	BufferID	= RTID;

	r3dRenderer->SetRT( RTID, Surfs[ FACE_PX ][ 0 ].Get() );

	return;
}

bool r3dScreenBuffer::IsNull() const
{
	return BufferFormat == D3DFMT_UNKNOWN || !Surfs[ 0 ][ 0 ].Get() ;
}

IDirect3DSurface9* r3dScreenBuffer::GetTex2DSurface()
{
	return Surfs[ FACE_PX ][ 0 ].Get();
}

int r3dScreenBuffer::GetShadowZBufSize()
{
	return gShadowZBuffer->X ;
}

int r3dScreenBuffer::GetNumMipLevels() const
{
	if(!Tex) return 0;
	return Tex->AsTex2D()->GetLevelCount();
}

void r3dScreenBuffer::Activate(int RTID, int Face /*= FACE_PX*/, int Mip /*= 0*/)
{
	r3d_assert( Mip < ActualNumMipLevels || !ActualNumMipLevels ) ;

	BufferID	= RTID;
	r3dD3DSurfaceTunnel s(Surfs[ Face ][ Mip ]);
	if (s.Get())
	{
		r3d_assert( bCubemap || Face == FACE_PX );

		r3dRenderer->SetRT( RTID, s.Get() );
	}

	if ( zType == Z_SYSTEM  )
	{
		r3dRenderer->SetDSS(ZBuf.Get());
	}
	else
	{
		if (!BufferID) 
		{
			if ( zType == Z_OWN )
			{
				r3dRenderer->SetDSS(OurZBuf.Get());
			}
			else
			if( zType == Z_SHADOW )
			{
				r3dRenderer->SetDSS(ShadowZBuf.Get());
			}
			else
			{
				r3dRenderer->SetDSS(NULL);
			}
		}
	}

	if ( !BufferID )
	{
		r3d_assert( !ActivateGuard ) ;

		ActivateGuard ++ ;

		const Dims& dms = MipDims[ Mip ] ;

		OldW                       = r3dRenderer->ScreenW ;
		OldH                       = r3dRenderer->ScreenH ;
		r3dRenderer->ScreenW       = dms.Width ;
		r3dRenderer->ScreenH       = dms.Height ;
		r3dRenderer->ScreenW2      = r3dRenderer->ScreenW / 2 ;
		r3dRenderer->ScreenH2      = r3dRenderer->ScreenH / 2 ;

		if( s.Get() )
		{
			r3dRenderer->SetViewport(0,0,r3dRenderer->ScreenW, r3dRenderer->ScreenH);
		}
	}

}


void r3dScreenBuffer::Deactivate(int bReset)
{
	if (BufferID)
	{
		r3dRenderer->SetRT(BufferID, NULL);

		//  r3dRenderer->SetRenderState(D3DRS_COLORWRITEENABLE1, 0x00000000 );

		return;
	}

	r3d_assert( ActivateGuard == 1 ) ;
	ActivateGuard -- ;

	if (bReset && BBuf.Get())
	{
		r3dRenderer->SetRT(0, BBuf.Get());
		r3dRenderer->SetDSS(ZBuf.Get());
	}

	r3dRenderer->ScreenW  = OldW;
	r3dRenderer->ScreenH  = OldH;
	r3dRenderer->ScreenW2 = r3dRenderer->ScreenW / 2;
	r3dRenderer->ScreenH2 = r3dRenderer->ScreenH / 2;

	r3dRenderer->SetViewport(0,0,r3dRenderer->ScreenW, r3dRenderer->ScreenH);

	//  if (bUseSystemZ)
	//    r3dRenderer->pd3ddev->StretchRect(r3dRenderer->RTBuffer, NULL, surf1, NULL, D3DTEXF_NONE);

}



r3dD3DBuffer::r3dD3DBuffer(type_e type, int size, int stride, LPDIRECT3DVERTEXDECLARATION9 decl, const r3dIntegrityGuardian& ig /*= r3dIntegrityGuardian()*/)
: r3dIResource( ig )
{
	r3d_assert( r3dRenderer && r3dRenderer->pd3ddev );

	m_LockSize  = 0;
	m_LockStart = 0;

	m_Type   = type;
	m_Size   = size;
	m_Stride = stride;
	m_Pos    = 0;
	m_NeedDiscard = 0;

	m_Decl    = decl;

#ifndef WO_SERVER
	m_DX11Buffer = NULL;
	m_DX11MappedPtr = NULL;
	m_DX9LockedPtr = NULL;
#endif

	R3D_ENSURE_MAIN_THREAD();

	D3DCreateResource();
}

r3dD3DBuffer::~r3dD3DBuffer()
{
	R3D_ENSURE_MAIN_THREAD();

	D3DReleaseResource();
}

#ifndef WO_SERVER

void r3dD3DBuffer::DX11CreateResource()
{
	if(m_DX11Buffer)
		return;

	if(!g_r3dDX11.IsInitialized())
		return;

	ID3D11Device* device = g_r3dDX11.GetDevice();

	if(!device)
		return;

	if(!m_Size || !m_Stride)
		return;

	D3D11_BUFFER_DESC desc;
	ZeroMemory(&desc, sizeof(desc));

	desc.ByteWidth = m_Size * m_Stride;
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc.MiscFlags = 0;
	desc.StructureByteStride = 0;

	switch(m_Type)
	{
	default:
		r3d_assert(0);
		return;

	case BUFFER_Index:
		desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		break;

	case BUFFER_Vertex:
		desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		break;
	}

	HRESULT dx11hr = device->CreateBuffer(&desc, NULL, &m_DX11Buffer);

	if(FAILED(dx11hr))
	{
		r3dOutToLog(
			"DX11Geometry: failed to create mirror %s buffer, size=%d stride=%d bytes=%d HRESULT=0x%08X\n",
			m_Type == BUFFER_Index ? "index" : "vertex",
			m_Size,
			m_Stride,
			m_Size * m_Stride,
			(unsigned int)dx11hr
		);

		m_DX11Buffer = NULL;
		return;
	}

	r3dOutToLog(
		"DX11Geometry: created mirror %s buffer, size=%d stride=%d bytes=%d\n",
		m_Type == BUFFER_Index ? "index" : "vertex",
		m_Size,
		m_Stride,
		m_Size * m_Stride
	);
}

void r3dD3DBuffer::DX11ReleaseResource()
{
	if(m_DX11MappedPtr)
	{
		if(g_r3dDX11.IsInitialized())
		{
			ID3D11DeviceContext* context = g_r3dDX11.GetContext();

			if(context && m_DX11Buffer)
				context->Unmap(m_DX11Buffer, 0);
		}

		m_DX11MappedPtr = NULL;
	}

	if(m_DX11Buffer)
	{
		m_DX11Buffer->Release();
		m_DX11Buffer = NULL;
	}

	m_DX9LockedPtr = NULL;
}

#endif

void r3dD3DBuffer::D3DCreateResource()
{
	R3D_ENSURE_MAIN_THREAD();

	r3d_assert(m_Size);
	r3d_assert(m_Stride);

	switch(m_Type) 
	{
	default: r3d_assert(0);

	case BUFFER_Index: 
		hr = r3dRenderer->pd3ddev->CreateIndexBuffer(
			m_Size * m_Stride,
			D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, 
			m_Stride == 2 ? D3DFMT_INDEX16 : D3DFMT_INDEX32,
			D3DPOOL_DEFAULT,
			&pIB, 
			NULL);

		r3dRenderer->CheckOutOfMemory( hr ) ;

		if(!SUCCEEDED(hr)) 
			r3dError("failed (%x) to create index buffer (%d bytes)\n", hr, m_Size * m_Stride);

		SetD3DResourcePrivateData(pIB, "r3dD3DBuffer: IB");
		r3dRenderer->Stats.AddBufferMem ( m_Size * m_Stride );
		break;

	case BUFFER_Vertex:
		hr = r3dRenderer->pd3ddev->CreateVertexBuffer(
			m_Size * m_Stride,
			D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY,
			0,
			D3DPOOL_DEFAULT,
			&pVB, 
			NULL);
		
		r3dRenderer->CheckOutOfMemory( hr ) ;

		if(!SUCCEEDED(hr)) 
			r3dError("failed (%x) to create vertex buffer (%d bytes)\n", hr, m_Size * m_Stride);

		SetD3DResourcePrivateData(pVB, "r3dD3DBuffer: VB");
		r3dRenderer->Stats.AddBufferMem ( + m_Size * m_Stride );
		break;
	}

#ifndef WO_SERVER
	DX11CreateResource();
#endif

	return;
}

void r3dD3DBuffer::D3DReleaseResource()
{
	R3D_ENSURE_MAIN_THREAD();

#ifndef WO_SERVER
	DX11ReleaseResource();
#endif

	switch(m_Type) 
	{
	default: r3d_assert(0);
	case BUFFER_Index:	SAFE_RELEASE(pIB); break;
	case BUFFER_Vertex: SAFE_RELEASE(pVB); break;
	}

	r3dRenderer->Stats.AddBufferMem ( -(int) m_Size * m_Stride );

	return;
}

void* r3dD3DBuffer::LockData(int size, int *lstart)
{
	r3d_assert(pIB);
	r3d_assert(!m_LockSize);
	if((unsigned int)size > m_Size) r3dError("r3dD3DBuffer::LockData overflow %d vs %d", size, m_Size);

	// No overwrite will be used if the data can fit into the space remaining in the buffer.
	DWORD dwLockFlags = D3DLOCK_NOOVERWRITE;

	// Check to see if the entire buffer has been used up yet.
	if(m_Pos + size >= m_Size || m_NeedDiscard) {
		// No space remains. Start over from the beginning 
		dwLockFlags   = D3DLOCK_DISCARD ;
		m_Pos         = 0;
		m_NeedDiscard = 0;
		r3dRenderer->Stats.AddNumLocksDiscard ( 1 ) ;
	}
	else
		r3dRenderer->Stats.AddNumLocksNooverwrite ( 1 ) ;

	// Lock the buffer.
	void* pOut;

	switch(m_Type) {
	case BUFFER_Index:
		hr = pIB->Lock(m_Pos * m_Stride, size * m_Stride, &pOut, dwLockFlags );
		break;
	case BUFFER_Vertex:
		hr = pVB->Lock(m_Pos * m_Stride, size * m_Stride, &pOut, dwLockFlags );
		break;
	}

	r3d_assert( hr == S_OK );

	// locked start/size
	m_LockStart = m_Pos;
	m_LockSize  = size;
	// advance to the next position
	m_Pos      += size;

#ifndef WO_SERVER
	m_DX9LockedPtr = pOut;
	m_DX11MappedPtr = NULL;

	if(g_r3dDX11.IsInitialized())
	{
		if(!m_DX11Buffer)
			DX11CreateResource();

		if(m_DX11Buffer)
		{
			ID3D11DeviceContext* context = g_r3dDX11.GetContext();

			if(context)
			{
				D3D11_MAPPED_SUBRESOURCE mapped;
				ZeroMemory(&mapped, sizeof(mapped));

				D3D11_MAP mapType = D3D11_MAP_WRITE_NO_OVERWRITE;

				if(dwLockFlags & D3DLOCK_DISCARD)
					mapType = D3D11_MAP_WRITE_DISCARD;

				HRESULT dx11hr = context->Map(
					m_DX11Buffer,
					0,
					mapType,
					0,
					&mapped
				);

				if(SUCCEEDED(dx11hr))
				{
					m_DX11MappedPtr = (BYTE*)mapped.pData + m_LockStart * m_Stride;
				}
				else
				{
					r3dOutToLog(
						"DX11Geometry: failed to map mirror buffer, HRESULT=0x%08X\n",
						(unsigned int)dx11hr
					);
				}
			}
		}
	}
#endif

	if(lstart)
		*lstart = m_LockStart;

	r3dRenderer->Stats.AddBytesLocked ( + size * m_Stride ) ;
	r3dRenderer->Stats.AddNumLocks ( 1 ) ;

	return pOut;
}

void r3dD3DBuffer::Unlock()
{
	r3d_assert(m_LockSize);

#ifndef WO_SERVER
	if(m_DX11Buffer && m_DX11MappedPtr && m_DX9LockedPtr)
	{
		memcpy(
			m_DX11MappedPtr,
			m_DX9LockedPtr,
			m_LockSize * m_Stride
		);

		ID3D11DeviceContext* context = g_r3dDX11.GetContext();

		if(context)
			context->Unmap(m_DX11Buffer, 0);

		m_DX11MappedPtr = NULL;
		m_DX9LockedPtr = NULL;
	}
#endif

	switch(m_Type) {
	case BUFFER_Index:
		hr = pIB->Unlock();
		break;
	case BUFFER_Vertex:
		hr = pVB->Unlock();
		break;
	}

	m_LockSize = 0;

#ifndef WO_SERVER
	m_DX11MappedPtr = NULL;
	m_DX9LockedPtr = NULL;
#endif
}

void r3dD3DBuffer::Activate()
{
	r3d_assert(pIB);

	switch(m_Type) 
	{
	default:
		r3d_assert(0);
		break;

	case BUFFER_Index:
		d3dc._SetIndices(pIB);

#ifndef WO_SERVER
		if(g_r3dDX11.IsInitialized())
		{
			if(!m_DX11Buffer)
				DX11CreateResource();

			if(m_DX11Buffer)
			{
				g_r3dDX11Geometry.SetIndexBufferRaw(
					m_DX11Buffer,
					m_Stride == 2 ? R3D_DX11_INDEX_16BIT : R3D_DX11_INDEX_32BIT,
					0
				);
			}
		}
#endif

		break;

	case BUFFER_Vertex:
		d3dc._SetDecl(m_Decl);
		d3dc._SetStreamSource(0, GetVB(), 0, m_Stride);

#ifndef WO_SERVER
		if(g_r3dDX11.IsInitialized())
		{
			if(!m_DX11Buffer)
				DX11CreateResource();

			if(m_DX11Buffer)
			{
				g_r3dDX11Geometry.SetVertexBufferRaw(
					0,
					m_DX11Buffer,
					m_Stride,
					0
				);
			}
		}
#endif

		break;
	}

	return;
}

void r3dVertexArray::D3DCreateResource()
{
	r3dDeviceTunnel::CreateVertexBuffer(size_*stride_, D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, 0, D3DPOOL_DEFAULT, &vrtBuff_ ) ;
	r3dDeviceTunnel::SetD3DResourcePrivateData( &vrtBuff_, "r3dVertexArray: VB");
}

void r3dVertexArray::D3DReleaseResource()
{
	R3D_ENSURE_MAIN_THREAD();

	if( !vrtBuff_.Valid() )	return;

	//unlock buffer if still locked

	if(lockedPtr_!=0)	vrtBuff_->Unlock();
	vrtBuff_.ReleaseAndReset();
}

r3dVertexArray::r3dVertexArray( const r3dIntegrityGuardian& ig )
: r3dIResource( ig )
{
	size_ = 0;
	pos_  = 0;
	type = TRIANGLE_LIST;
}

r3dVertexArray::~r3dVertexArray()
{
	R3D_ENSURE_MAIN_THREAD();
	D3DReleaseResource();
}

/*
handle errors
1) unlock when is not locked
2) lock of already locked
3) 0 bytes for lock
4) unlock but not filled completely
5) overflow
6) flush when is still locked
7) lock more then entire buffer size
8) reset when locked
r3d_assert(num >= iVertexVol);
*/
r3dVertexArray*	r3dCreateVertexArray(int size, int stride, LPDIRECT3DVERTEXDECLARATION9 decl, r3dVertexArray::Type type )
{
	r3dIntegrityGuardian ig ;

	r3dVertexArray* t = new r3dVertexArray( ig );

	t->size_   = size;
	t->pos_    = 0;
	t->actual_    = 0;
	t->unflushed_ = 0;
	t->stride_ = stride;
	t->decl_    = decl;
	t->needed_ = 0;
	t->lockedPtr_ = 0;
	t->type = type;

	R3D_ENSURE_MAIN_THREAD();

	t->D3DCreateResource();

	return t;
}


//------------------------------------------------------------------------
void r3dVertexArray::AddList( const void* vtx, int num)
{
	StartListVertices(num);
	SetListVtx(vtx, num);
	EndListVertices();
}
void r3dVertexArray::StartListVertices(int num)
{
	needed_ = num;
	Lock();
}
void r3dVertexArray::SetListVtx(const void* vtx, int num)
{
	if(actual_ + num > needed_) r3dError("r3dVertexArray::SetListVtx overflow\n");

	memcpy( lockedPtr_ + actual_*stride_, vtx , num * stride_ );
	actual_ += num;
}
void r3dVertexArray::EndListVertices()
{
	Unlock();
}






void r3dVertexArray::Add(const void* vtx, int num)
{
	StartVertices(num, 1);
	SetVtx(vtx, num);
	EndVertices();
}
void r3dVertexArray::StartVertices(int numVertsInPoly, int numPoly)
{
	switch( type )
	{
	case TRIANGLE_LIST:
		needed_ = (numVertsInPoly - 2) * 3 * numPoly ;
		break ;
	case LINE_LIST:
		needed_ = numVertsInPoly * numPoly ;
		break ;
	case LINE_STRIP:
		needed_ = numPoly + 1 ;
		break ;
	}

	Lock();
}
void r3dVertexArray::SetVtx(const void* vtx, int num)
{
	if(actual_ + num > needed_) r3dError("r3dVertexArray::SetVtx overflow\n");

	switch( type )
	{
	case LINE_LIST:
	case LINE_STRIP:
		for(int i = 0; i < num; i++) 
			AddVtx ( vtx, 1 );
		break ;
	case TRIANGLE_LIST:
		for(int i = 1; i < num-1; i++)
		{
			AddVtx(vtx, 0);
			AddVtx(vtx, i);
			AddVtx(vtx, i+1);
		}
		break ;
	}
}
void r3dVertexArray::EndVertices()
{
	Unlock();
	needed_ = 0;
}





void r3dVertexArray::Lock()
{
	if(needed_ == 0) return;

	if(needed_ > size_) r3dError("r3dVertexArray::Lock overflow %d vs %d\n", needed_, size_);
	if(lockedPtr_ != 0) r3dError("r3dVertexArray::Lock error: already locked\n");

	unsigned int flg = D3DLOCK_NOOVERWRITE;
	if(pos_ + unflushed_ + needed_ >= size_)
	{
		r3dRenderer->Stats.AddNumLocksDiscard( 1 );

		Flush();
		flg = D3DLOCK_DISCARD;
		pos_ = 0;
	}
	else
	{
		r3dRenderer->Stats.AddNumLocksNooverwrite( 1 );
	}

	UINT toLock = needed_ * stride_;

	r3dRenderer->Stats.AddBytesLocked( +toLock );
	r3dRenderer->Stats.AddNumLocks( 1 );

	vrtBuff_->Lock((pos_ + unflushed_)* stride_, toLock, (void**)&lockedPtr_, flg);
}
void r3dVertexArray::Unlock()
{
	if(needed_ == 0) return;

	if(lockedPtr_ == 0) r3dError("r3dVertexArray::Unlock error: is not locked\n");
	if(actual_ < needed_) r3dError("r3dVertexArray::Unlock error: the buffer is not filled completely\n");

	vrtBuff_->Unlock();
	unflushed_ += actual_;
	actual_ = 0;
	lockedPtr_ = 0;
}



void r3dVertexArray::Flush()
{
	if(lockedPtr_ != 0) r3dError("r3dVertexArray::Flush error: is still locked\n");

	if(unflushed_ == 0)	return;

	d3dc._SetDecl(decl_);
	d3dc._SetStreamSource(0, vrtBuff_.Get(), 0, stride_);

	switch( type )
	{
	case LINE_LIST:
		r3dRenderer->Draw(D3DPT_LINELIST, pos_, unflushed_ / 2);
		break ;
	case LINE_STRIP:
		r3dRenderer->Draw(D3DPT_LINESTRIP, pos_, unflushed_ - 1 );
		break ;
	case TRIANGLE_LIST:
		r3dRenderer->Draw(D3DPT_TRIANGLELIST, pos_, unflushed_ / 3);
		break ;
	}

	pos_ += unflushed_;
	unflushed_ = 0;
}

int r3dGetShadowZBufferSize()
{
	return gShadowZBuffer->UsedMemory ;
}
