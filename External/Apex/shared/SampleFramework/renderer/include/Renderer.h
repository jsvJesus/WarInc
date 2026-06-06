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

#ifndef RENDERER_H
#define RENDERER_H

#include <RendererConfig.h>

#include <RendererMaterial.h>
#include <RendererColor.h>

#include "PsShare.h"
#include <PxMat34Legacy.h>
#include <vector>

class RendererDesc;
class RendererWindow;
class RendererVertexBuffer;
class RendererVertexBufferDesc;
class RendererIndexBuffer;
class RendererIndexBufferDesc;
class RendererInstanceBuffer;
class RendererInstanceBufferDesc;
class RendererTexture2D;
class RendererTexture2DDesc;
class RendererTarget;
class RendererTargetDesc;
class RendererMaterial;
class RendererMaterialDesc;
class RendererMesh;
class RendererMeshDesc;
class RendererMeshContext;
class RendererLight;
class RendererLightDesc;

class RendererColor;
class RendererProjection;

	class ScreenQuad
	{
		public:
						ScreenQuad();

		physx::PxU32	mLeftUpColor;		//!< Color for left-up vertex
		physx::PxU32	mLeftDownColor;		//!< Color for left-down vertex
		physx::PxU32	mRightUpColor;		//!< Color for right-up vertex
		physx::PxU32	mRightDownColor;	//!< Color for right-down vertex
		physx::PxReal	mAlpha;				//!< Alpha value used if > 0.0f
		physx::PxReal	mX0, mY0;			//!< Up-left coordinates
		physx::PxReal	mX1, mY1;			//!< Bottom-right coordinates
	};

class Renderer
{
	public:
		struct TextVertex
		{
			physx::PxVec3	p;
			physx::PxReal	rhw;
			physx::PxU32	color;
			physx::PxReal	u,v;
		};

		typedef enum DriverType
		{
			DRIVER_OPENGL   = 0, // Supports Windows, Linux, MacOSX and PS3.
			DRIVER_DIRECT3D9,    // Supports Windows, XBOX360.
			DRIVER_DIRECT3D10,   // Supports Windows Vista.
			DRIVER_LIBGCM        // Supports PS3.
		};
		
	public:
		static Renderer *createRenderer(const RendererDesc &desc);
		
		static const char *getDriverTypeName(DriverType type);
		
	protected:
		Renderer(DriverType driver);
		virtual ~Renderer(void);
		
	public:
		void release(void);
		
		// get the driver type for this renderer.
		DriverType getDriverType(void) const;
		
		// get the offset to the center of a pixel relative to the size of a pixel (so either 0 or 0.5).
		physx::PxF32 getPixelCenterOffset(void) const;
		
		// get the name of the hardware device.
		const char *getDeviceName(void) const;

		// adds a mesh to the render queue.
		void queueMeshForRender(RendererMeshContext &mesh);
		
		// adds a light to the render queue.
		void queueLightForRender(RendererLight &light);
		
		// renders the current scene to the offscreen buffers. empties the render queue when done.
		void render(const physx::PxMat34Legacy &eye, const RendererProjection &proj, RendererTarget *target=0, bool depthOnly=false);
		
		// sets the ambient lighting color.
		void setAmbientColor(const RendererColor &ambientColor);

        // sets the clear color.
		void setClearColor(const RendererColor &clearColor);
        RendererColor& getClearColor() { return m_clearColor; }
		
		// get and set the output message stream
		void setOutputStream(physx::PxUserOutputStream *oStream) { m_outputStream = oStream; }
		physx::PxUserOutputStream *getOutputStream() { return m_outputStream; }
public:
		// clears the offscreen buffers.
		virtual void clearBuffers(void) = 0;
		
		// presents the current color buffer to the screen.
		// returns true on device reset and if buffers need to be rewritten.
		virtual bool swapBuffers(void) = 0;

		// get the device pointer (void * abstraction)
		virtual void *getDevice() = 0;

		virtual void getWindowSize(physx::PxU32 &width, physx::PxU32 &height) const = 0;

		virtual RendererVertexBuffer   *createVertexBuffer(  const RendererVertexBufferDesc   &desc) = 0;
		virtual RendererIndexBuffer    *createIndexBuffer(   const RendererIndexBufferDesc    &desc) = 0;
		virtual RendererInstanceBuffer *createInstanceBuffer(const RendererInstanceBufferDesc &desc) = 0;
		virtual RendererTexture2D      *createTexture2D(     const RendererTexture2DDesc      &desc) = 0;
		virtual RendererTarget         *createTarget(        const RendererTargetDesc         &desc) = 0;
		virtual RendererMaterial       *createMaterial(      const RendererMaterialDesc       &desc) = 0;
		virtual RendererMesh           *createMesh(          const RendererMeshDesc           &desc) = 0;
		virtual RendererLight          *createLight(         const RendererLightDesc          &desc) = 0;

		// Text rendering
		virtual	bool					initTexter();
		virtual	void					closeTexter();
				void					print(physx::PxU32 x, physx::PxU32 y, const char* text, physx::PxReal scale=0.5f, physx::PxReal shadowOffset=6.0f, physx::PxU32 textColor=0xffffffff);

		// Screenquad
		virtual	bool					initScreenquad();
		virtual	void					closeScreenquad();
				bool					drawScreenQuad(const ScreenQuad& screenQuad);

		// Disable this performance optimization when CUDA/Graphics Interop is in use
		void setVertexBufferDeferredUnlocking( bool enabled );
		bool getVertexBufferDeferredUnlocking() const;
	
	private:
		void renderMeshes(std::vector<RendererMeshContext*> & meshes, RendererMaterial::Pass pass);
		void renderDeferredLights(void);
	
	private:
		virtual bool beginRender(void) { return true;}
		virtual void endRender(void) {}
		virtual void bindViewProj(const physx::PxMat34Legacy &eye, const RendererProjection &proj)    = 0;
		virtual void bindAmbientState(const RendererColor &ambientColor)                 = 0;
		virtual void bindDeferredState(void)                                             = 0;
		virtual void bindMeshContext(const RendererMeshContext &context)                 = 0;
		virtual void beginMultiPass(void)                                                = 0;
		virtual void endMultiPass(void)                                                  = 0;
		virtual void renderDeferredLight(const RendererLight &light)                     = 0;
		
		virtual bool isOk(void) const = 0;
	public:
		virtual	void setupTextRenderStates()		= 0;
		virtual	void resetTextRenderStates()		= 0;
		virtual	void renderTextBuffer(const void* vertices, physx::PxU32 nbVerts, const physx::PxU16* indices, physx::PxU32 nbIndices) = 0;
		virtual	void setupScreenquadRenderStates()	= 0;
		virtual	void resetScreenquadRenderStates()	= 0;

	private:
		Renderer &operator=(const Renderer&) { return *this; }
	
	private:
		const DriverType              m_driver;
		physx::PxUserOutputStream *	  m_outputStream;
		
		std::vector<RendererMeshContext*> m_visibleLitMeshes;
		std::vector<RendererMeshContext*> m_visibleUnlitMeshes;
		std::vector<RendererMeshContext*> m_screenSpaceMeshes;
		std::vector<RendererLight*>       m_visibleLights;
		
		RendererColor                 m_ambientColor;
		RendererColor                 m_clearColor;

		// Texter data
		RendererMaterial*				m_textMaterial;
		RendererMaterialInstance*		m_textMaterialInstance;

		// Screenquad data
		RendererMaterial*				m_screenquadMaterial;
		RendererMaterialInstance*		m_screenquadMaterialInstance;

    protected:
		bool                          m_deferredVBUnlock;
		physx::PxF32                  m_pixelCenterOffset;
		char                          m_deviceName[256];
		bool							m_useShadersForTextRendering;
};

#endif
