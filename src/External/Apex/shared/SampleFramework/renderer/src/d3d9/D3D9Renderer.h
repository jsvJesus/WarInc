/*
 * Copyright 2009-2011 NVIDIA Corporation.  All rights reserved.
 *
 * NOTICE TO USER:
 *
 * This source code is subject to NVIDIA ownership rights under U.S. and
 * international Copyright laws.  Users and possessors of this source code
 * are hereby granted a nonexclusive, royalty-free license to use this code
 * in individual and commercial software.
 *
 * NVIDIA MAKES NO REPRESENTATION ABOUT THE SUITABILITY OF THIS SOURCE
 * CODE FOR ANY PURPOSE.  IT IS PROVIDED "AS IS" WITHOUT EXPRESS OR
 * IMPLIED WARRANTY OF ANY KIND.  NVIDIA DISCLAIMS ALL WARRANTIES WITH
 * REGARD TO THIS SOURCE CODE, INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE.
 * IN NO EVENT SHALL NVIDIA BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL,
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS,  WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
 * OR OTHER TORTIOUS ACTION,  ARISING OUT OF OR IN CONNECTION WITH THE USE
 * OR PERFORMANCE OF THIS SOURCE CODE.
 *
 * U.S. Government End Users.   This source code is a "commercial item" as
 * that term is defined at  48 C.F.R. 2.101 (OCT 1995), consisting  of
 * "commercial computer  software"  and "commercial computer software
 * documentation" as such terms are  used in 48 C.F.R. 12.212 (SEPT 1995)
 * and is provided to the U.S. Government only as a commercial end item.
 * Consistent with 48 C.F.R.12.212 and 48 C.F.R. 227.7202-1 through
 * 227.7202-4 (JUNE 1995), all U.S. Government End Users acquire the
 * source code with only those rights set forth herein.
 *
 * Any use of this source code in individual and commercial software must
 * include, in the user documentation and internal comments to the code,
 * the above Disclaimer and U.S. Government End Users Notice.
 */
#ifndef D3D9_RENDERER_H
#define D3D9_RENDERER_H

#include <RendererConfig.h>

#if defined(RENDERER_ENABLE_DIRECT3D9)

#include <Renderer.h>
#include <vector>

#include "Px.h"
#include "PxSimpleTypes.h"
#include "PxVec3.h"
#include "PxMat34Legacy.h"

#if defined(RENDERER_DEBUG)
	//#define D3D_DEBUG_INFO 1
#endif
#include <d3d9.h>
#include <d3dx9.h>

// Enables/Disables use of non-managed resources for vertex/instance buffers.
// Disabled for now as it appears to actually slow us down significantly on particles.
#if defined(RENDERER_XBOX360)
#define RENDERER_ENABLE_DYNAMIC_VB_POOLS 0
#else
#define RENDERER_ENABLE_DYNAMIC_VB_POOLS 1
#endif

class RendererDesc;
class RendererColor;

void convertToD3D9(D3DCOLOR &dxcolor, const RendererColor &color);
void convertToD3D9(float *dxvec, const physx::PxVec3 &vec);
void convertToD3D9(D3DMATRIX &dxmat, const physx::PxMat34Legacy &mat);
void convertToD3D9(D3DMATRIX &dxmat, const RendererProjection &mat);

class D3D9RendererResource;

class D3D9Renderer : public Renderer
{
	friend class D3D9RendererResource;
	public:
		class D3DXInterface
		{
			friend class D3D9Renderer;
			private:
				D3DXInterface(void);
				~D3DXInterface(void);
			
			public:
				HRESULT CompileShaderFromFileA(LPCSTR srcFile, CONST D3DXMACRO *defines, LPD3DXINCLUDE include, LPCSTR functionName, LPCSTR profile, DWORD flags, LPD3DXBUFFER *shader, LPD3DXBUFFER *errorMsgs, LPD3DXCONSTANTTABLE *constantTable);
				LPCSTR  GetVertexShaderProfile(LPDIRECT3DDEVICE9 device);
				LPCSTR  GetPixelShaderProfile(LPDIRECT3DDEVICE9 device);
				
			public:
			#if defined(RENDERER_WINDOWS) 
				HMODULE  m_library;
				
				#define DEFINE_D3DX_FUNCTION(_name, _return, _params) \
				    typedef _return (WINAPI* LP##_name) _params;      \
				    LP##_name m_##_name;
				
				DEFINE_D3DX_FUNCTION(D3DXCompileShaderFromFileA, HRESULT, (LPCSTR, CONST D3DXMACRO*, LPD3DXINCLUDE, LPCSTR, LPCSTR, DWORD, LPD3DXBUFFER*, LPD3DXBUFFER*, LPD3DXCONSTANTTABLE *))
				DEFINE_D3DX_FUNCTION(D3DXGetVertexShaderProfile, LPCSTR, (LPDIRECT3DDEVICE9));
				DEFINE_D3DX_FUNCTION(D3DXGetPixelShaderProfile,  LPCSTR, (LPDIRECT3DDEVICE9));
				
				#undef DEFINE_D3DX_FUNCTION
			#endif
		};
		
		class ShaderEnvironment
		{
			public:
				D3DMATRIX          modelMatrix;
				D3DMATRIX          viewMatrix;
				D3DMATRIX          projMatrix;
				D3DMATRIX          modelViewMatrix;
				D3DMATRIX          modelViewProjMatrix;
				
				D3DXMATRIX         boneMatrices[RENDERER_MAX_BONES];
				physx::PxU32              numBones;
				
				float              eyePosition[3];
				float              eyeDirection[3];
				
				D3DCOLOR           ambientColor;
				
				D3DCOLOR           lightColor;
				float              lightIntensity;
				float              lightDirection[3];
				float              lightPosition[3];
				float              lightInnerRadius;
				float              lightOuterRadius;
				float              lightInnerCone;
				float              lightOuterCone;
				IDirect3DTexture9 *lightShadowMap;
				D3DXMATRIX         lightShadowMatrix;
				
			public:
				ShaderEnvironment(void);
		};
		
	public:
		D3D9Renderer(const RendererDesc &desc);
		virtual ~D3D9Renderer(void);
		
		IDirect3DDevice9        *getD3DDevice(void)               { return m_d3dDevice; }
		D3DXInterface           &getD3DX(void)                    { return m_d3dx; }
		ShaderEnvironment       &getShaderEnvironment(void)       { return m_environment; }
		const ShaderEnvironment &getShaderEnvironment(void) const { return m_environment; }
		
	private:
		bool checkResize(bool isDeviceLost);
		void onDeviceLost(void);
		void onDeviceReset(void);
		void buildDepthStencilSurface(void);
	
	public:
		// clears the offscreen buffers.
		virtual void clearBuffers(void);
		
		// presents the current color buffer to the screen.
		// returns true on device reset and if buffers need to be rewritten.
		virtual bool swapBuffers(void);

		// get the device pointer (void * abstraction)
		virtual void *getDevice()                                 { return static_cast<void*>(getD3DDevice()); }

		// get the window size
		void getWindowSize(physx::PxU32 &width, physx::PxU32 &height) const;
		
		virtual RendererVertexBuffer   *createVertexBuffer(  const RendererVertexBufferDesc   &desc);
		virtual RendererIndexBuffer    *createIndexBuffer(   const RendererIndexBufferDesc    &desc);
		virtual RendererInstanceBuffer *createInstanceBuffer(const RendererInstanceBufferDesc &desc);
		virtual RendererTexture2D      *createTexture2D(     const RendererTexture2DDesc      &desc);
		virtual RendererTarget         *createTarget(        const RendererTargetDesc         &desc);
		virtual RendererMaterial       *createMaterial(      const RendererMaterialDesc       &desc);
		virtual RendererMesh           *createMesh(          const RendererMeshDesc           &desc);
		virtual RendererLight          *createLight(         const RendererLightDesc          &desc);

		virtual	bool					initTexter();
		virtual	void					closeTexter();

	private:
		virtual bool beginRender(void);
		virtual void endRender(void);
		virtual void bindViewProj(const physx::PxMat34Legacy &eye, const RendererProjection &proj);
		virtual void bindAmbientState(const RendererColor &ambientColor);
		virtual void bindDeferredState(void);
		virtual void bindMeshContext(const RendererMeshContext &context);
		virtual void beginMultiPass(void);
		virtual void endMultiPass(void);
		virtual void renderDeferredLight(const RendererLight &light);
		
		virtual bool isOk(void) const;
		
		virtual	void setupTextRenderStates();
		virtual	void resetTextRenderStates();
		virtual	void renderTextBuffer(const void* vertices, physx::PxU32 nbVerts, const physx::PxU16* indices, physx::PxU32 nbIndices);
		virtual	void setupScreenquadRenderStates();
		virtual	void resetScreenquadRenderStates();

	private:
		void addResource(D3D9RendererResource &resource);
		void removeResource(D3D9RendererResource &resource);
		void notifyResourcesLostDevice(void);
		void notifyResourcesResetDevice(void);
	
	private:
		physx::PxU32                          m_displayWidth;
		physx::PxU32                          m_displayHeight;
		
		IDirect3D9                    *m_d3d;
		IDirect3DDevice9              *m_d3dDevice;
		
		IDirect3DSurface9             *m_d3dDepthStencilSurface;
		
		D3DXInterface                  m_d3dx;
		
		ShaderEnvironment              m_environment;
		
		D3DPRESENT_PARAMETERS          m_d3dPresentParams;
		
		physx::PxMat34Legacy				   m_viewMatrix;
		
		// non-managed resources...
		std::vector<D3D9RendererResource*> m_resources;

		IDirect3DVertexDeclaration9*	m_textVDecl;
};

class D3D9RendererResource
{
	friend class D3D9Renderer;
	public:
		D3D9RendererResource(void)
		{
			m_d3dRenderer = 0;
		}
		
		virtual ~D3D9RendererResource(void)
		{
			if(m_d3dRenderer) m_d3dRenderer->removeResource(*this);
		}
		
	public:
		virtual void onDeviceLost(void)=0;
		virtual void onDeviceReset(void)=0;
	
	private:
		D3D9Renderer *m_d3dRenderer;
};

#endif // #if defined(RENDERER_ENABLE_DIRECT3D9)
#endif
