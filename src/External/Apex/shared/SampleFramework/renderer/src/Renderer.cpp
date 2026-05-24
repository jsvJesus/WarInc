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
#include "PsShare.h"
#include <Renderer.h>
#include <RendererDesc.h>
#include "ogl/OGLRenderer.h"
#include "d3d9/D3D9Renderer.h"
#include "PsString.h"
#include <RendererMesh.h>
#include <RendererMeshContext.h>

#include <RendererMaterial.h>
#include <RendererMaterialInstance.h>
#include <RendererProjection.h>

#include <RendererTarget.h>

#include <RendererLight.h>

#include "PxMat34Legacy.h"

#include <algorithm>

Renderer *Renderer::createRenderer(const RendererDesc &desc)
{
	Renderer *renderer = 0;
	const bool valid = desc.isValid();
	if(valid)
	{
		switch(desc.driver)
		{
			case DRIVER_OPENGL:
			#if defined(RENDERER_ENABLE_OPENGL)
				renderer = new OGLRenderer(desc);
			#endif
				break;
				
			case DRIVER_DIRECT3D9:
			#if defined(RENDERER_ENABLE_DIRECT3D9)
				renderer = new D3D9Renderer(desc);
			#endif
				break;
				
			case DRIVER_DIRECT3D10:
			#if defined(RENDERER_ENABLE_DIRECT3D10)
				
			#endif
				break;
				
			case DRIVER_LIBGCM:
			#if defined(RENDERER_ENABLE_LIBGCM)
				
			#endif
				break;
		}

		if(renderer)
			renderer->setOutputStream(desc.outputStream);
	}
	if(renderer && !renderer->isOk())
	{
		renderer->release();
		renderer = 0;
	}
	RENDERER_ASSERT(renderer, "Failed to create renderer!");
	return renderer;
}

const char *Renderer::getDriverTypeName(DriverType type)
{
	const char *name = 0;
	switch(type)
	{
		case DRIVER_OPENGL:     name = "OpenGL";     break;
		case DRIVER_DIRECT3D9:  name = "Direct3D9";  break;
		case DRIVER_DIRECT3D10: name = "Direct3D10"; break;
		case DRIVER_LIBGCM:     name = "LibGCM";     break;
	}
	RENDERER_ASSERT(name, "Unable to find Name String for Renderer Driver Type.");
	return name;
}

		
Renderer::Renderer(DriverType driver) :
	m_driver						(driver),
	m_deferredVBUnlock				(true),
	m_textMaterial					(NULL),
	m_textMaterialInstance			(NULL),
	m_screenquadMaterial			(NULL),
	m_screenquadMaterialInstance	(NULL),
	m_useShadersForTextRendering	(false)
{
	m_pixelCenterOffset = 0;
	setAmbientColor(RendererColor(64,64,64, 255));
    setClearColor(RendererColor(133,153,181,255));
	physx::string::strncpy_s(m_deviceName, sizeof(m_deviceName), "UNKNOWN", sizeof(m_deviceName));
}

void Renderer::setVertexBufferDeferredUnlocking( bool enabled )
{
	m_deferredVBUnlock = enabled;
}

bool Renderer::getVertexBufferDeferredUnlocking() const
{
	return m_deferredVBUnlock;
}

Renderer::~Renderer(void)
{
	PX_ASSERT(!m_screenquadMaterial);
	PX_ASSERT(!m_screenquadMaterialInstance);

	PX_ASSERT(!m_textMaterial);
	PX_ASSERT(!m_textMaterialInstance);
}

void Renderer::release(void)
{
	delete this;
}

// get the driver type for this renderer.
Renderer::DriverType Renderer::getDriverType(void) const
{
	return m_driver;
}

// get the offset to the center of a pixel relative to the size of a pixel (so either 0 or 0.5).
physx::PxF32 Renderer::getPixelCenterOffset(void) const
{
	return m_pixelCenterOffset;
}

// get the name of the hardware device.
const char *Renderer::getDeviceName(void) const
{
	return m_deviceName;
}

// adds a mesh to the render queue.
void Renderer::queueMeshForRender(RendererMeshContext &mesh)
{
	RENDERER_ASSERT( mesh.isValid(),  "Mesh Context is invalid.");
	RENDERER_ASSERT(!mesh.isLocked(), "Mesh Context is already locked to a Renderer.");
	if(mesh.isValid() && !mesh.isLocked())
	{
		mesh.m_renderer = this;
		if (mesh.screenSpace)
			m_screenSpaceMeshes.push_back(&mesh);
		else switch (mesh.material->getType())
		{
		case  RendererMaterial::TYPE_LIT:
			m_visibleLitMeshes.push_back(&mesh);
			break;
		default: //case RendererMaterial::TYPE_UNLIT:
			m_visibleUnlitMeshes.push_back(&mesh);
		//	break;
		}
	}
}

// adds a light to the render queue.
void Renderer::queueLightForRender(RendererLight &light)
{
	RENDERER_ASSERT(!light.isLocked(), "Light is already locked to a Renderer.");
	if(!light.isLocked())
	{
		light.m_renderer = this;
		m_visibleLights.push_back(&light);
	}
}

// renders the current scene to the offscreen buffers. empties the render queue when done.
void Renderer::render(const physx::PxMat34Legacy &eye, const RendererProjection &proj, RendererTarget *target, bool depthOnly)
{
	RENDERER_PERFZONE(Renderer_render);
	const physx::PxU32 numLights = (physx::PxU32)m_visibleLights.size();
	if(target)
	{
		target->bind();
	}
	// TODO: Sort meshes by material..
	if(beginRender())
	{
		if(!depthOnly)
		{
			// What the hell is this? Why is there specialized code in here for a projection matrix...
			// YOU CAN PASS THE PROJECTION MATIX RIGHT INTO THIS FUNCTION!
			// TODO: Get rid of this.
			if (m_screenSpaceMeshes.size())
			{
				physx::PxMat34Legacy id(true);
				bindViewProj(id, RendererProjection(-1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f));	//TODO: pass screen space matrices
				renderMeshes(m_screenSpaceMeshes, RendererMaterial::PASS_UNLIT);	//render screen space stuff first so stuff we occlude doesn't waste time on shading.
			}
		}
		
		if(depthOnly)
		{
			RENDERER_PERFZONE(Renderer_render_depthOnly);
			bindAmbientState(RendererColor(0,0,0,255));
			bindViewProj(eye, proj);
			renderMeshes(m_visibleLitMeshes,   RendererMaterial::PASS_DEPTH);
			renderMeshes(m_visibleUnlitMeshes, RendererMaterial::PASS_DEPTH);
		}
		else  if(numLights > RENDERER_DEFERRED_THRESHOLD)
		{
			RENDERER_PERFZONE(Renderer_render_deferred);
			bindDeferredState();
			bindViewProj(eye, proj);
			renderMeshes(m_visibleLitMeshes,   RendererMaterial::PASS_UNLIT);
			renderMeshes(m_visibleUnlitMeshes, RendererMaterial::PASS_UNLIT);
			renderDeferredLights();
		}
		else if(numLights > 0)
		{
			RENDERER_PERFZONE(Renderer_render_lit);
			bindAmbientState(m_ambientColor);
			bindViewProj(eye, proj);
			RendererLight &light0 = *m_visibleLights[0];
			light0.bind();
			renderMeshes(m_visibleLitMeshes, light0.getPass());
			light0.m_renderer = 0;
			
			bindAmbientState(RendererColor(0,0,0,255));
			beginMultiPass();
			for(physx::PxU32 i=1; i<numLights; i++)
			{
				RendererLight &light = *m_visibleLights[i];
				light.bind();
				renderMeshes(m_visibleLitMeshes, light.getPass());
				light.m_renderer = 0;
			}
			endMultiPass();
			renderMeshes(m_visibleUnlitMeshes, RendererMaterial::PASS_UNLIT);
		}
		else
		{
			RENDERER_PERFZONE(Renderer_render_unlit);
			bindAmbientState(RendererColor(0,0,0,255));
			bindViewProj(eye, proj);
			renderMeshes(m_visibleLitMeshes,   RendererMaterial::PASS_UNLIT);
			renderMeshes(m_visibleUnlitMeshes, RendererMaterial::PASS_UNLIT);
		}
		endRender();
	}
	if(target) target->unbind();
	m_visibleLitMeshes.clear();
	m_visibleUnlitMeshes.clear();
	m_screenSpaceMeshes.clear();
	m_visibleLights.clear();
}

// sets the ambient lighting color.
void Renderer::setAmbientColor(const RendererColor &ambientColor)
{
	m_ambientColor   = ambientColor;
	m_ambientColor.a = 255;
}

void Renderer::setClearColor(const RendererColor &clearColor)
{
	m_clearColor   = clearColor;
	m_clearColor.a = 255;
}

void Renderer::renderMeshes(std::vector<RendererMeshContext*> & meshes, RendererMaterial::Pass pass)
{
	RENDERER_PERFZONE(Renderer_renderMeshes);
	
	RendererMaterial         *lastMaterial         = 0;
	RendererMaterialInstance *lastMaterialInstance = 0;
	const RendererMesh       *lastMesh             = 0;
	
	const physx::PxU32 numMeshes = (physx::PxU32)meshes.size();
	for(physx::PxU32 i=0; i<numMeshes; i++)
	{
		RendererMeshContext &context = *meshes[i];
		bindMeshContext(context);
		bool instanced = context.mesh->getInstanceBuffer()?true:false;
		
		if(context.materialInstance && context.materialInstance != lastMaterialInstance)
		{
			if(lastMaterial) lastMaterial->unbind();
			lastMaterialInstance =  context.materialInstance;
			lastMaterial         = &context.materialInstance->getMaterial();
			lastMaterial->bind(pass, lastMaterialInstance, instanced);
		}
		else if(context.material != lastMaterial)
		{
			if(lastMaterial) lastMaterial->unbind();
			lastMaterialInstance = 0;
			lastMaterial         = context.material;
			lastMaterial->bind(pass, lastMaterialInstance, instanced);
		}
		
		if(lastMaterial) lastMaterial->bindMeshState(instanced);
		if(context.mesh != lastMesh)
		{
			if(lastMesh) lastMesh->unbind();
			lastMesh = context.mesh;
			if(lastMesh) lastMesh->bind();
		}
		if(lastMesh) context.mesh->render(context.material);
		context.m_renderer = 0;
	}
	if(lastMesh)     lastMesh->unbind();
	if(lastMaterial) lastMaterial->unbind();
}


void Renderer::renderDeferredLights(void)
{
	RENDERER_PERFZONE(Renderer_renderDeferredLights);
	
	const physx::PxU32 numLights = (physx::PxU32)m_visibleLights.size();
	for(physx::PxU32 i=0; i<numLights; i++)
	{
		renderDeferredLight(*m_visibleLights[i]);
	}
}

///////////////////////////////////////////////////////////////////////////////

using namespace physx;

#include "RendererMemoryMacros.h"
#include "RendererMaterialDesc.h"
#include "RendererTexture2DDesc.h"

// PT: text stuff from ICE. Adapted quickly, should be cleaned up when we have some time.

	struct FntInfo
	{
		PxReal		u0;
		PxReal		v0;
		PxReal		u1;
		PxReal		v1;
		PxU32		dx;
		PxU32		dy;
	};

	class FntData
	{
		public:
										FntData();
										~FntData();

						void			Reset();
						PxU32			ComputeSize(const char* text, PxReal& width, PxReal& height, PxReal scale)	const;

						bool			Load(Renderer& renderer, const char* filename);

		PX_FORCE_INLINE	PxU32			GetNbFnts()	const	{ return mNbFnts;	}
		PX_FORCE_INLINE	const FntInfo*	GetFnts()	const	{ return mFnts;		}
		PX_FORCE_INLINE	PxU32			GetMaxDx()	const	{ return mMaxDx;	}
		PX_FORCE_INLINE	PxU32			GetMaxDy()	const	{ return mMaxDy;	}
		PX_FORCE_INLINE	const PxU8*		GetXRef()	const	{ return mXRef;		}

		private:
						PxU32			mNbFnts;
						FntInfo*		mFnts;
						PxU32			mMaxDx, mMaxDy;
						PxU8			mXRef[256];
		public:
			RendererTexture2D*			mTexture;
	};

FntData::FntData()
{
	mNbFnts	= 0;
	mFnts	= NULL;
	mMaxDx	= 0;
	mMaxDy	= 0;
	memset(mXRef, 0, 256*sizeof(PxU8));
	mTexture	= NULL;
}

FntData::~FntData()
{
	Reset();
}

void FntData::Reset()
{
	SAFE_RELEASE(mTexture);
	DELETEARRAY(mFnts);
	mNbFnts	= 0;
	mMaxDx	= 0;
	mMaxDy	= 0;
	memset(mXRef, 0, 256*sizeof(PxU8));
}

// Compute size of a given text
PxU32 FntData::ComputeSize(const char* text, PxReal& width, PxReal& height, PxReal scale) const
{
	// Get and check length
	if(!text)	return 0;
	PxU32 Nb = (PxU32)strlen((const char*)text);
	if(!Nb) return 0;

	PxReal x = 0.0f;
	PxReal y = 0.0f;

	width = -1.0f;
	height = -1.0f;

	// Loop through characters
	for(PxU32 j=0;j<Nb;j++)
	{
		if(text[j]!='\n')
		{
			// Catch current character index
			const PxU32 i = mXRef[text[j]];

			// Catch size of current character
			const PxReal sx = PxReal(mFnts[i].dx) * scale;
			const PxReal sy = PxReal(mFnts[i].dy) * scale;

			// Keep track of greatest dimensions
			if((x+sx)>width)	width = x+sx;
			if((y+sy)>height)	height = y+sy;

			// Adjust x for next character
			x += sx + 1.0f;
		}
		else
		{
			// Jump to next line
			x = 0.0f;
			y += PxReal(mMaxDy) * scale;
		}
	}
	return Nb;
}

#include "PsFile.h"

#if defined (PX_WINDOWS)
	static const bool gFlip = false;
#elif defined(PX_LINUX)
	static const bool gFlip = false;
#elif defined(PX_APPLE)
	static const bool gFlip = true;
#elif defined(PX_PS3)
	static const bool gFlip = true;
#elif defined(PX_X360)
	static const bool gFlip = true;
#elif defined(PX_WII)
	static const bool gFlip = true;
#else
	#error Unknown platform!
#endif

	PX_INLINE void Flip(PxU32& v)
	{
		PxU8* b = (PxU8*)&v;

        PxU8 temp = b[0];
		b[0] = b[3];
		b[3] = temp;
		temp = b[1];
		b[1] = b[2];
		b[2] = temp;
	}

	PX_INLINE void Flip(PxI32& v)
	{
		Flip((PxU32&)v);
	}

	PX_INLINE void Flip(PxF32& v)
	{
		Flip((PxU32&)v);
	}

static PxU32 read32(FILE* fp)
{
	PxU32 data;
	fread(&data, 4, 1, fp);
	if(gFlip)
		Flip(data);
	return data;
}

bool FntData::Load(Renderer& renderer, const char* filename)
{
	FILE* fp = NULL;
	physx::fopen_s(&fp, filename, "rb");
	if(!fp)
		return false;

	// Init texture
	{
		const PxU32 width	= read32(fp);
		const PxU32 height	= read32(fp);
		PxU8* data = new PxU8[width*height*4];
		fread(data, width*height*4, 1, fp);
/*		if(gFlip)	
		{
			PxU32* data32 = (PxU32*)data;
			for(PxU32 i=0;i<width*height;i++)
			{
				Flip(data32[i]);
			}
		}*/

		RendererTexture2DDesc tdesc;
		tdesc.format	= RendererTexture2D::FORMAT_B8G8R8A8;
		tdesc.width		= width;
		tdesc.height	= height;
		tdesc.numLevels	= 1;
	/*
		tdesc.filter;
		tdesc.addressingU;
		tdesc.addressingV;
		tdesc.renderTarget;
	*/
		PX_ASSERT(tdesc.isValid());
		mTexture = renderer.createTexture2D(tdesc);
		PX_ASSERT(mTexture);
		if(!mTexture)
		{
			DELETEARRAY(data);
			fclose(fp);
			return false;
		}

		const PxU32 componentCount = 4;

		if(mTexture)
		{
			PxU32 pitch = 0;
			void* buffer = mTexture->lockLevel(0, pitch);
			PX_ASSERT(buffer);
			if(buffer)
			{
				PxU8* levelDst			= (PxU8*)buffer;
				const PxU8* levelSrc	= (PxU8*)data;
				const PxU32 levelWidth	= mTexture->getWidthInBlocks();
				const PxU32 levelHeight	= mTexture->getHeightInBlocks();
				PX_ASSERT(levelWidth * mTexture->getBlockSize() <= pitch); // the pitch can't be less than the source row size.
				for(PxU32 row=0; row<levelHeight; row++)
				{ 
					// copy per pixel to handle RBG case, based on component count
					for(PxU32 col=0; col<levelWidth; col++)
					{
						*levelDst++ = levelSrc[0];
						*levelDst++ = levelSrc[1];
						*levelDst++ = levelSrc[2];
						*levelDst++ = levelSrc[3];
						levelSrc += componentCount;
					}
				}
			}
			mTexture->unlockLevel(0);
		}
		DELETEARRAY(data);
	}

	mNbFnts = read32(fp);

	mFnts	= new FntInfo[mNbFnts];
	fread(mFnts, mNbFnts*sizeof(FntInfo), 1, fp);
	if(gFlip)
	{
		for(PxU32 i=0;i<mNbFnts;i++)
		{
			Flip(mFnts[i].u0);
			Flip(mFnts[i].v0);
			Flip(mFnts[i].u1);
			Flip(mFnts[i].v1);
			Flip(mFnts[i].dx);
			Flip(mFnts[i].dy);
		}
	}

	mMaxDx	= read32(fp);
	mMaxDy	= read32(fp);

	fread(mXRef, 256*sizeof(PxU8), 1, fp);

	fclose(fp);
	return true;
}


struct ClipBox
{
	PxReal	mXMin;
	PxReal	mYMin;
	PxReal	mXMax;
	PxReal	mYMax;
};

static bool ClipQuad(Renderer::TextVertex* /*quad*/, const ClipBox& /*clip_box*/)
{
	return true;
}

PX_FORCE_INLINE PxU32 translateColor(PxU32 color)
{
#if defined(PX_PS3)
	// PT: of course PC/Xbox expect ARGB colors and PS3 expects RGBA...
	const PxU32 alpha = color>>24;
	return ((color<<8)&0xffffff00)|alpha;
#else
	return color;
#endif
}

static bool GenerateTextQuads(	const char* text, PxU32 nb_characters,
								Renderer::TextVertex* fnt_verts, PxU16* fnt_indices, const ClipBox& clip_box, const FntData* fnt_data, PxReal& x, PxReal& y, PxReal scale_x, PxReal scale_y, PxU32 color,
								PxReal* x_min, PxReal* y_min, PxReal* x_max, PxReal* y_max, PxU32* nb_lines, PxU32* nb_active_characters)
{
	// Checkings
	if(!text || !fnt_verts || !fnt_indices || !fnt_data)	return false;

	PxReal mX = x;

	//////////

	Renderer::TextVertex*	V = fnt_verts;
	PxU16*					I = fnt_indices;
	PxU16					Offset = 0;
	PxU32					NbActiveCharacters	= 0;	// Number of non-NULL characters (the ones to render)

	PxReal					XMin = 100000.0f, XMax = -100000.0f;
	PxReal					YMin = 100000.0f, YMax = -100000.0f;

	PxU32					NbLines = 1;	// Number of lines

	// Loop through characters
	for(PxU32 j=0;j<nb_characters;j++)
	{
		if(text[j]!='\n')
		{
			PxU32 i = fnt_data->GetXRef()[text[j]];

			// Character size
			const PxReal sx = PxReal(fnt_data->GetFnts()[i].dx) * scale_x;
			const PxReal sy = PxReal(fnt_data->GetFnts()[i].dy) * scale_y;

			if(text[j]!=' ')
			{
				const PxReal rhw = 1.0f;

				// Initalize the vertices
				V[0].p.x = x;    V[0].p.y = y+sy; V[0].u = fnt_data->GetFnts()[i].u0; V[0].v = fnt_data->GetFnts()[i].v1;
				V[1].p.x = x;    V[1].p.y = y;    V[1].u = fnt_data->GetFnts()[i].u0; V[1].v = fnt_data->GetFnts()[i].v0;
				V[2].p.x = x+sx; V[2].p.y = y+sy; V[2].u = fnt_data->GetFnts()[i].u1; V[2].v = fnt_data->GetFnts()[i].v1;
				V[3].p.x = x+sx; V[3].p.y = y;    V[3].u = fnt_data->GetFnts()[i].u1; V[3].v = fnt_data->GetFnts()[i].v0;
				V[0].rhw = V[1].rhw = V[2].rhw = V[3].rhw = rhw;
				V[0].p.z = V[1].p.z = V[2].p.z = V[3].p.z = 0.0f;
				V[0].color = V[1].color = V[2].color = V[3].color = color;

if(ClipQuad(V, clip_box))
{
				V+=4;

				// Initialize the indices
				*I++ = Offset+0;
				*I++ = Offset+1;
				*I++ = Offset+2;
				*I++ = Offset+2;
				*I++ = Offset+1;
				*I++ = Offset+3;
				Offset+=4;

				NbActiveCharacters++;
}
			}
			//
			if((x+sx)>XMax)	XMax = x+sx;	if(x<XMin)	XMin = x;
			if((y+sy)>YMax)	YMax = y+sy;	if(y<YMin)	YMin = y;

			x += sx + 1.0f;
		}
		else
		{
			// Jump to next line
			x = mX;
			y += PxReal(fnt_data->GetMaxDy()) * scale_y;
			NbLines++;
		}
	}

	if(x_min)					*x_min = XMin;
	if(y_min)					*y_min = YMin;
	if(x_max)					*x_max = XMax;
	if(y_max)					*y_max = YMax;
	if(nb_lines)				*nb_lines = NbLines;
	if(nb_active_characters)	*nb_active_characters = NbActiveCharacters;

	return true;
}

	enum RenderTextQuadFlag_
	{
		RTQF_ALIGN_LEFT		= 0,
		RTQF_ALIGN_CENTER	= (1<<0),
		RTQF_ALIGN_RIGHT	= (1<<1),
	};

static void RenderTextQuads(Renderer* textRender,
							const FntData* fnts,
							const char* text, PxReal x, PxReal y, PxU32 text_color,
							PxReal scale_x, PxReal scale_y,
							PxU32 align_flags, PxReal max_length,
							PxReal shadow_offset,
							PxReal* nx, PxReal* ny,
							const ClipBox* clip_box,
							PxReal text_y_offset,
							bool use_max_dy
							)
{
	// We want to render the whole text in one run...

	const PxReal text_x = x;
	const PxReal text_y = y;

	// Compute text size
	PxReal Width, Height;
	const PxU32 NbCharacters = fnts->ComputeSize(text, Width, Height, 1.0f);

	// Prepare clip box
	ClipBox CB;
	if(clip_box)
	{
		CB = *clip_box;
	}
	else
	{
		PxU32 width, height;
		textRender->getWindowSize(width, height);

		const PxReal Margin = 0.0f;
		CB.mXMin = Margin;
		CB.mYMin = Margin;
		CB.mXMax = PxReal(width) - Margin;
		CB.mYMax = PxReal(height) - Margin;
	}

	// Allocate space for vertices
	Renderer::TextVertex*	FntVerts	= new Renderer::TextVertex[NbCharacters*4];
	PxU16*					FntIndices	= new PxU16[NbCharacters*6];

	// Generate quads
	PxReal XMin, YMin, XMax, YMax;
	PxU32 NbLines, NbActiveCharacters;
	GenerateTextQuads(text, NbCharacters, FntVerts, FntIndices, CB, fnts, x, y, scale_x, scale_y, text_color, &XMin, &YMin, &XMax, &YMax, &NbLines, &NbActiveCharacters);

	for(PxU32 i=0;i<NbActiveCharacters*4;i++)
		FntVerts[i].p.y += text_y_offset;

	if(use_max_dy)
		YMax = YMin + (PxReal)fnts->GetMaxDy();

	const bool centered = (align_flags & RTQF_ALIGN_CENTER)!=0;
	const bool align_right = (align_flags & RTQF_ALIGN_RIGHT)!=0;

	if(centered || align_right)
	{
		const PxReal L = XMax - XMin;
		XMax = XMin + max_length;
		const PxReal Offset = centered ? (-FntVerts[0].p.x + XMin + (max_length - L)*0.5f) :
										(-FntVerts[0].p.x + XMax - L);
//										(-FntVerts[0].p.x + XMin + (max_length - L));
		for(PxU32 i=0;i<NbActiveCharacters*4;i++)
			FntVerts[i].p.x += Offset;
	}

	textRender->setupTextRenderStates();

#if defined(PX_PS3)
	const PxU32 AlphaMask = (text_color & 0x000000ff);
#else
	const PxU32 AlphaMask = (text_color & 0xff000000);
#endif

	// Handle shadow
	if(shadow_offset!=0.0f)
	{
		// Allocate space for vertices
		Renderer::TextVertex*	SFntVerts	= new Renderer::TextVertex[NbCharacters*4];
		PxU16*					SFntIndices	= new PxU16[NbCharacters*6];

		// Generate quads
		PxReal SXMin, SYMin, SXMax, SYMax;
		PxU32 SNbLines, SNbActiveCharacters;
		PxReal ShX = text_x + shadow_offset;
		PxReal ShY = text_y + shadow_offset;
		GenerateTextQuads(text, NbCharacters, SFntVerts, SFntIndices, CB, fnts, ShX, ShY, scale_x, scale_y, AlphaMask, &SXMin, &SYMin, &SXMax, &SYMax, &SNbLines, &SNbActiveCharacters);

		for(PxU32 i=0;i<SNbActiveCharacters*4;i++)
			SFntVerts[i].p.y += text_y_offset;

		if(centered || align_right)
		{
			const PxReal L = SXMax - SXMin;
			SXMax = SXMin + max_length;
			const PxReal Offset = centered ? (-SFntVerts[0].p.x + SXMin + (max_length - L)*0.5f) : 
											(-SFntVerts[0].p.x + SXMax - L);
			for(PxU32 i=0;i<SNbActiveCharacters*4;i++)
				SFntVerts[i].p.x += Offset;
		}

		textRender->renderTextBuffer(SFntVerts, 4*SNbActiveCharacters, SFntIndices, 6*SNbActiveCharacters);

		DELETEARRAY(SFntIndices);
		DELETEARRAY(SFntVerts);
	}

	textRender->renderTextBuffer(FntVerts, 4*NbActiveCharacters, FntIndices, 6*NbActiveCharacters);

	textRender->resetTextRenderStates();

	DELETEARRAY(FntIndices);
	DELETEARRAY(FntVerts);
}


static FntData*	gFntData = NULL;

extern char gAssetDir[];	// PT: this is ugly but the renderer was already doing this with the shader dir...

bool Renderer::initTexter()
{
	if(gFntData)
		return true;

	char filename[1024];
	physx::string::strcpy_s(filename, sizeof(filename), gAssetDir);
	physx::string::strcat_s(filename, sizeof(filename), "fonts/arial_black.bin");

	gFntData = new FntData;
	if(!gFntData->Load(*this, filename))
	{
		closeTexter();
		return false;
	}

	{
		RendererMaterialDesc matDesc;
		matDesc.alphaTestFunc		= RendererMaterial::ALPHA_TEST_ALWAYS;
		matDesc.alphaTestRef		= 0.0f;
		matDesc.type				= RendererMaterial::TYPE_UNLIT;
		matDesc.blending			= true;
		matDesc.srcBlendFunc		= RendererMaterial::BLEND_ONE;
		matDesc.dstBlendFunc		= RendererMaterial::BLEND_ONE;
		matDesc.geometryShaderPath	= NULL;
		matDesc.vertexShaderPath	= "vertex/text.cg";
		matDesc.fragmentShaderPath	= "fragment/text.cg";
		PX_ASSERT(matDesc.isValid());

		m_textMaterial = createMaterial(matDesc);
		if(!m_textMaterial)
		{
			closeTexter();
			return false;
		}

		m_textMaterialInstance = new RendererMaterialInstance(*m_textMaterial);
		if(!m_textMaterialInstance)
		{
			closeTexter();
			return false;
		}

		const RendererMaterial::Variable* var = m_textMaterialInstance->findVariable("diffuseTexture", RendererMaterial::VARIABLE_SAMPLER2D);
		if(!var)
		{
			closeTexter();
			return false;
		}
		m_textMaterialInstance->writeData(*var, &gFntData->mTexture);
	}
	return true;
}

void Renderer::closeTexter()
{
	DELETESINGLE(m_textMaterialInstance);
	SAFE_RELEASE(m_textMaterial);
	DELETESINGLE(gFntData);
}

void Renderer::print(physx::PxU32 x, physx::PxU32 y, const char* text, physx::PxReal scale, physx::PxReal shadowOffset, PxU32 textColor)
{
	if(!gFntData || !gFntData->mTexture || !m_textMaterial || !m_textMaterialInstance)
		return;

	m_textMaterial->bind(RendererMaterial::PASS_UNLIT, m_textMaterialInstance, false);

	if(!m_useShadersForTextRendering)
	{
		m_textMaterial->unbind();
		gFntData->mTexture->select(0);
	}

	const PxU32 alignFlags = 0;
	const float maxLength = 0.0f;
	float* nx = NULL;
	float* ny = NULL;
	const ClipBox* clipBox = NULL;
	const float textYOffset = 0.0f;
	const bool useMaxDy = false;

/*#if defined(PX_PS3)
	// PT: of course PC/Xbox expect ARGB colors and PS3 expects RGBA...
	const PxU32 alpha = textColor>>24;
	textColor = ((textColor<<8)&0xffffff00)|alpha;
#endif*/
	textColor = translateColor(textColor);

	RenderTextQuads(this,
					gFntData,
					text,
					PxReal(x), PxReal(y),
					textColor,
					scale, scale,
					alignFlags,
					maxLength,
					shadowOffset * scale,
					nx, ny,
					clipBox,
					textYOffset,
					useMaxDy
	);

	if(m_useShadersForTextRendering)
		m_textMaterial->unbind();
}

///////////////////////////////////////////////////////////////////////////////

bool Renderer::initScreenquad()
{
	RendererMaterialDesc matDesc;
	matDesc.alphaTestFunc		= RendererMaterial::ALPHA_TEST_ALWAYS;
	matDesc.alphaTestRef		= 0.0f;
	matDesc.type				= RendererMaterial::TYPE_UNLIT;
//	matDesc.blending			= true;
	matDesc.blending			= false;
	matDesc.srcBlendFunc		= RendererMaterial::BLEND_ONE;
	matDesc.dstBlendFunc		= RendererMaterial::BLEND_ONE;
	matDesc.geometryShaderPath	= NULL;
	matDesc.vertexShaderPath	= "vertex/screenquad.cg";
	matDesc.fragmentShaderPath	= "fragment/screenquad.cg";
	PX_ASSERT(matDesc.isValid());

	m_screenquadMaterial = createMaterial(matDesc);
	if(!m_screenquadMaterial)
	{
		closeScreenquad();
		return false;
	}

	m_screenquadMaterialInstance = new RendererMaterialInstance(*m_screenquadMaterial);
	if(!m_screenquadMaterialInstance)
	{
		closeScreenquad();
		return false;
	}
	return true;
}

void Renderer::closeScreenquad()
{
	DELETESINGLE(m_screenquadMaterialInstance);
	SAFE_RELEASE(m_screenquadMaterial);
}

ScreenQuad::ScreenQuad() :
	mLeftUpColor		(0xffffffff),
	mLeftDownColor		(0xffffffff),
	mRightUpColor		(0xffffffff),
	mRightDownColor		(0xffffffff),
	mAlpha				(-1.0f),
	mX0					(0.0f),
	mY0					(0.0f),
	mX1					(-1.0f),
	mY1					(-1.0f)
{
}

bool Renderer::drawScreenQuad(const ScreenQuad& screenQuad)
{
	if(!m_screenquadMaterialInstance || !m_screenquadMaterial)
		return false;

	m_screenquadMaterial->bind(RendererMaterial::PASS_UNLIT, m_screenquadMaterialInstance, false);
	if(!m_useShadersForTextRendering)
		m_screenquadMaterial->unbind();

	setupScreenquadRenderStates();

	TextVertex Verts[4];
	const PxU16 Indices[6] = { 0,1,2,2,1,3 };

	PxU32 renderWidth, renderHeight;
	getWindowSize(renderWidth, renderHeight);

    // Initalize the vertices
	PxReal x0	= screenQuad.mX0;
	PxReal y0	= screenQuad.mY0;
	PxReal sx	= screenQuad.mX1;
	PxReal sy	= screenQuad.mY1;
	if(sx<0.0f)	sx	= PxReal(renderWidth);
	if(sy<0.0f)	sy	= PxReal(renderHeight);
	PxReal rhw	= 1.0f;
	PxReal z	= 0.0f;

	Verts[0].p	= PxVec3(x0, sy, z);		Verts[0].rhw	= rhw;		Verts[0].color	= translateColor(screenQuad.mLeftDownColor);
	Verts[1].p	= PxVec3(x0, y0, z);		Verts[1].rhw	= rhw;		Verts[1].color	= translateColor(screenQuad.mLeftUpColor);
	Verts[2].p	= PxVec3(sx, sy, z);		Verts[2].rhw	= rhw;		Verts[2].color	= translateColor(screenQuad.mRightDownColor);
	Verts[3].p	= PxVec3(sx, y0, z);		Verts[3].rhw	= rhw;		Verts[3].color	= translateColor(screenQuad.mRightUpColor);

/*	if(screenQuad.mAlpha>=0.0f)
	{
		PxU32 Mask = PxU32(screenQuad.mAlpha*255.0f);
		if(Mask>255)	Mask = 255;
		Mask <<= 24;
		for(PxU32 i=0;i<4;i++)
		{
			Verts[i].color &= 0x00ffffff;
			Verts[i].color |= Mask;
		}
	}*/

/*	renderer->TranslateColor(Verts[0].color);
	renderer->TranslateColor(Verts[1].color);
	renderer->TranslateColor(Verts[2].color);
	renderer->TranslateColor(Verts[3].color);
*/

//	renderer->DrawIndexedPrimitive(PRIMTYPE_TRILIST, VF_PRDT, Verts, 4, Indices, 6);
	renderTextBuffer(Verts, 4, Indices, 6);

	resetScreenquadRenderStates();
	if(m_useShadersForTextRendering)
		m_screenquadMaterial->unbind();

	return true;
}
