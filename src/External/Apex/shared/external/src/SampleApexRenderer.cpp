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
#include <SampleApexRenderer.h>

#include <Renderer.h>

#include <RendererVertexBuffer.h>
#include <RendererVertexBufferDesc.h>

#include <RendererIndexBuffer.h>
#include <RendererIndexBufferDesc.h>

#include <RendererInstanceBuffer.h>
#include <RendererInstanceBufferDesc.h>

#include <RendererMesh.h>
#include <NxRenderMeshAsset.h>
#include <RendererMeshDesc.h>
#include <RendererMeshContext.h>

#include <RendererMaterial.h>
#include <RendererMaterialDesc.h>
#include <RendererMaterialInstance.h>

#include <RendererColor.h>

#include <SampleAssetManager.h>
#include <SampleMaterialAsset.h>

#include <NxApexSDK.h>
#include <NxModuleParticles.h>
#include <NxModuleIofx.h>
#include <NxModuleDestructible.h>
#include <NxModuleExplosion.h>
#include <NxModuleForceField.h>
#include <NxModuleEmitter.h>
#include <NxModuleClothing.h>
#include <NxDestructibleAsset.h>
#include <NxFieldBoundaryAsset.h>
#include <NxExplosionAsset.h>
#include <NxIofxAsset.h>
#include <NxBasicIOSAsset.h>
#include <NxFluidIosAsset.h>
#include <NxApexEmitterAsset.h>
#include <NxImpactEmitterAsset.h>
#include <NxGroundEmitterAsset.h>
#include "NxWindAsset.h"
#include <NxWindAsset.h>
#include <NxClothingAsset.h>
#include <NxClothingMaterialLibrary.h>
#include <NxApexDefs.h>
#include <NxFromPx.h>

#include "PsString.h"
#include "PsFile.h"
#include "AgPerfMonEventSrcAPI.h"



#define	USE_RENDER_SPRITE_BUFFER 1

#define	PLATFORM_EXTERNALS_DIR EXTERNALS_DIR "/"

#define INPLACE_BINARY 1

#define DEBUG_RESOURCE_REQUESTS 0
#if DEBUG_RESOURCE_REQUESTS
# include <stdio.h>
# define DEBUG_OUTPUT_FILENAME "debugResourceRequests.txt"
FILE *gDebugOutput = 0 /* stdout */;
#endif

static void SampleApexRendererSwapColorEndian(void *colors, physx::PxU32 stride, physx::PxU32 numColors)
{
	void *end = ((physx::PxU8*)colors)+(stride*numColors);
	for(; colors < end; colors=((physx::PxU8*)colors)+stride)
	{
		physx::PxU32* val = (physx::PxU32*)colors;
		*val =	((*val >> 24) & 0x000000ff) |
				((*val >>  8) & 0x0000ff00) |
				((*val <<  8) & 0x00ff0000) |
				((*val << 24) & 0xff000000);
	}
}

static RendererVertexBuffer::Hint convertFromApexVB(physx::apex::NxRenderBufferHint::Enum apexHint)
{
	RendererVertexBuffer::Hint vbhint = RendererVertexBuffer::HINT_STATIC;
	if(apexHint == physx::apex::NxRenderBufferHint::DYNAMIC || apexHint == physx::apex::NxRenderBufferHint::STREAMING)
	{
		vbhint = RendererVertexBuffer::HINT_DYNAMIC;
	}
	return vbhint;
}

static RendererVertexBuffer::Semantic convertFromApexVB(physx::apex::NxRenderVertexSemantic::Enum apexSemantic)
{
	RendererVertexBuffer::Semantic semantic = RendererVertexBuffer::NUM_SEMANTICS;
	switch(apexSemantic)
	{
		case physx::apex::NxRenderVertexSemantic::POSITION:    semantic = RendererVertexBuffer::SEMANTIC_POSITION;   break;
		case physx::apex::NxRenderVertexSemantic::NORMAL:      semantic = RendererVertexBuffer::SEMANTIC_NORMAL;     break;
		case physx::apex::NxRenderVertexSemantic::TANGENT:     semantic = RendererVertexBuffer::SEMANTIC_TANGENT;    break;
		case physx::apex::NxRenderVertexSemantic::COLOR:       semantic = RendererVertexBuffer::SEMANTIC_COLOR;      break;
		case physx::apex::NxRenderVertexSemantic::TEXCOORD0:   semantic = RendererVertexBuffer::SEMANTIC_TEXCOORD0;  break;
		case physx::apex::NxRenderVertexSemantic::TEXCOORD1:   semantic = RendererVertexBuffer::SEMANTIC_TEXCOORD1;  break;
		case physx::apex::NxRenderVertexSemantic::TEXCOORD2:   semantic = RendererVertexBuffer::SEMANTIC_TEXCOORD2;  break;
		case physx::apex::NxRenderVertexSemantic::TEXCOORD3:   semantic = RendererVertexBuffer::SEMANTIC_TEXCOORD3;  break;
		case physx::apex::NxRenderVertexSemantic::BONE_INDEX:  semantic = RendererVertexBuffer::SEMANTIC_BONEINDEX;  break;
		case physx::apex::NxRenderVertexSemantic::BONE_WEIGHT: semantic = RendererVertexBuffer::SEMANTIC_BONEWEIGHT; break;
	}
	//PX_ASSERT(semantic < RendererVertexBuffer::NUM_SEMANTICS);
	return semantic;
}

static RendererVertexBuffer::Format convertFromApexVB(physx::apex::NxRenderDataFormat::Enum apexFormat, bool &outSwizzle)
{
	RendererVertexBuffer::Format format = RendererVertexBuffer::NUM_FORMATS;
	switch(apexFormat)
	{
		case physx::apex::NxRenderDataFormat::FLOAT1:	format=RendererVertexBuffer::FORMAT_FLOAT1; break;
		case physx::apex::NxRenderDataFormat::FLOAT2:	format=RendererVertexBuffer::FORMAT_FLOAT2; break;
		case physx::apex::NxRenderDataFormat::FLOAT3:	format=RendererVertexBuffer::FORMAT_FLOAT3; break;
		case physx::apex::NxRenderDataFormat::FLOAT4:	format=RendererVertexBuffer::FORMAT_FLOAT4; break;
		case physx::apex::NxRenderDataFormat::UBYTE4:
		case physx::apex::NxRenderDataFormat::R8G8B8A8:	
			format=RendererVertexBuffer::FORMAT_COLOR;  
			outSwizzle = true; 
			break;
		case physx::apex::NxRenderDataFormat::B8G8R8A8:	format=RendererVertexBuffer::FORMAT_COLOR;  break;
		case physx::apex::NxRenderDataFormat::USHORT1:
		case physx::apex::NxRenderDataFormat::USHORT2:
		case physx::apex::NxRenderDataFormat::USHORT3:
		case physx::apex::NxRenderDataFormat::USHORT4:
			format = RendererVertexBuffer::FORMAT_USHORT4;
			break;
	}
	//PX_ASSERT(format < RendererVertexBuffer::NUM_FORMATS || apexFormat==NxRenderDataFormat::UNSPECIFIED);
	return format;
}

static RendererVertexBuffer::Semantic convertFromApexSB(physx::apex::NxRenderSpriteSemantic::Enum apexSemantic)
{
	RendererVertexBuffer::Semantic semantic = RendererVertexBuffer::NUM_SEMANTICS;
	switch(apexSemantic)
	{
		case physx::apex::NxRenderSpriteSemantic::POSITION:    semantic = RendererVertexBuffer::SEMANTIC_POSITION;  break;
		case physx::apex::NxRenderSpriteSemantic::COLOR:       semantic = RendererVertexBuffer::SEMANTIC_COLOR;     break;
		case physx::apex::NxRenderSpriteSemantic::VELOCITY:    semantic = RendererVertexBuffer::SEMANTIC_NORMAL;    break;
		case physx::apex::NxRenderSpriteSemantic::SCALE:       semantic = RendererVertexBuffer::SEMANTIC_TANGENT;   break;
		case physx::apex::NxRenderSpriteSemantic::LIFE_REMAIN: semantic = RendererVertexBuffer::SEMANTIC_TEXCOORD0; break;
		case physx::apex::NxRenderSpriteSemantic::DENSITY:     semantic = RendererVertexBuffer::SEMANTIC_TEXCOORD1; break;
		case physx::apex::NxRenderSpriteSemantic::SUBTEXTURE:  semantic = RendererVertexBuffer::SEMANTIC_TEXCOORD2; break;
		case physx::apex::NxRenderSpriteSemantic::ORIENTATION: semantic = RendererVertexBuffer::SEMANTIC_TEXCOORD3; break;
	}
	//PX_ASSERT(semantic < RendererVertexBuffer::NUM_SEMANTICS);
	return semantic;
}

static RendererIndexBuffer::Hint convertFromApexIB(physx::apex::NxRenderBufferHint::Enum apexHint)
{
	RendererIndexBuffer::Hint ibhint = RendererIndexBuffer::HINT_STATIC;
	if(apexHint == physx::apex::NxRenderBufferHint::DYNAMIC || apexHint == physx::apex::NxRenderBufferHint::STREAMING)
	{
		ibhint = RendererIndexBuffer::HINT_DYNAMIC;
	}
	return ibhint;
}

static RendererIndexBuffer::Format convertFromApexIB(physx::apex::NxRenderDataFormat::Enum apexFormat)
{
	RendererIndexBuffer::Format format = RendererIndexBuffer::NUM_FORMATS;
	switch(apexFormat)
	{
		case physx::apex::NxRenderDataFormat::UBYTE1:  PX_ASSERT(0); /* UINT8 Indices not in HW. */ break;
		case physx::apex::NxRenderDataFormat::USHORT1: format = RendererIndexBuffer::FORMAT_UINT16; break;
		case physx::apex::NxRenderDataFormat::UINT1:   format = RendererIndexBuffer::FORMAT_UINT32; break;
	}
	PX_ASSERT(format < RendererIndexBuffer::NUM_FORMATS);
	return format;
}

static RendererMesh::Primitive convertFromApex(physx::apex::NxRenderPrimitiveType::Enum apexPrimitive)
{
	RendererMesh::Primitive primitive = RendererMesh::NUM_PRIMITIVES;
	switch(apexPrimitive)
	{
		case physx::apex::NxRenderPrimitiveType::TRIANGLES:      primitive = RendererMesh::PRIMITIVE_TRIANGLES;      break;
		case physx::apex::NxRenderPrimitiveType::TRIANGLE_STRIP: primitive = RendererMesh::PRIMITIVE_TRIANGLE_STRIP; break;
		
		case physx::apex::NxRenderPrimitiveType::LINES:          primitive = RendererMesh::PRIMITIVE_LINES;          break;
		case physx::apex::NxRenderPrimitiveType::LINE_STRIP:     primitive = RendererMesh::PRIMITIVE_LINE_STRIP;     break;
		
		case physx::apex::NxRenderPrimitiveType::POINTS:         primitive = RendererMesh::PRIMITIVE_POINTS;         break;
		case physx::apex::NxRenderPrimitiveType::POINT_SPRITES:  primitive = RendererMesh::PRIMITIVE_POINT_SPRITES;  break;
	}
	PX_ASSERT(primitive < RendererMesh::NUM_PRIMITIVES);
	return primitive;
}

/*********************************
* SampleApexRendererVertexBuffer *
*********************************/

class SampleApexRendererVertexBuffer : public physx::apex::NxUserRenderVertexBuffer
{
	friend class SampleApexRendererMesh;
	public:
		SampleApexRendererVertexBuffer(Renderer &renderer, const physx::apex::NxUserRenderVertexBufferDesc &desc) :
			m_renderer(renderer),
			m_swizzleColor(false)
		{
			m_vertexbuffer = 0;
			RendererVertexBufferDesc vbdesc;
			vbdesc.hint = convertFromApexVB(desc.hint);
			for(physx::PxU32 i=0; i<physx::apex::NxRenderVertexSemantic::NUM_SEMANTICS; i++)
			{
				RendererVertexBuffer::Semantic semantic = convertFromApexVB((physx::apex::NxRenderVertexSemantic::Enum)i);
				RendererVertexBuffer::Format   format   = convertFromApexVB(desc.buffersRequest[i], m_swizzleColor);
				if(semantic < RendererVertexBuffer::NUM_SEMANTICS && format < RendererVertexBuffer::NUM_FORMATS)
				{
					vbdesc.semanticFormats[semantic] = format;
				}
			}
			vbdesc.maxVertices = desc.maxVerts;
			m_vertexbuffer = m_renderer.createVertexBuffer(vbdesc);
			PX_ASSERT(m_vertexbuffer);
			m_uvOrigin = desc.uvOrigin;
		}
		
		virtual ~SampleApexRendererVertexBuffer(void)
		{
			if(m_vertexbuffer) m_vertexbuffer->release();
		}
	
	private:
		// convert color from R8G8B8A8 to B8G8R8A8 for D3D, OGL will switch it back...
		void swizzleColor(void *colors, physx::PxU32 stride, physx::PxU32 numColors)
		{
			void *end = ((physx::PxU8*)colors)+(stride*numColors);
			for(; colors < end; colors=((physx::PxU8*)colors)+stride)
			{
				((RendererColor*)colors)->swizzleRB();
			}
		}
		
		void fixUVOrigin(void *uvdata, physx::PxU32 stride, physx::PxU32 num)
		{
			#define ITERATE_UVS(_uop,_vop)								\
				for(physx::PxU32 i=0; i<num; i++)						\
				{														\
					physx::PxF32 &u = *(((physx::PxF32*)uvdata) + 0);	\
					physx::PxF32 &v = *(((physx::PxF32*)uvdata) + 1);	\
					u=_uop;												\
					v=_vop;												\
					uvdata = ((physx::PxU8*)uvdata)+stride;				\
				}
			switch(m_uvOrigin)
			{
				case physx::apex::NxTextureUVOrigin::ORIGIN_BOTTOM_LEFT:
					// nothing to do...
					break;
				case physx::apex::NxTextureUVOrigin::ORIGIN_BOTTOM_RIGHT:
					ITERATE_UVS(1-u, v);
					break;
				case physx::apex::NxTextureUVOrigin::ORIGIN_TOP_LEFT:
					ITERATE_UVS(u, 1-v);
					break;
				case physx::apex::NxTextureUVOrigin::ORIGIN_TOP_RIGHT:
					ITERATE_UVS(1-u, 1-v);
					break;
				default:
					PX_ASSERT(0); // UNKNOWN ORIGIN / TODO!
			}
			#undef ITERATE_UVS
		}
		
		//virtual void writeBuffer(physx::apex::NxRenderVertexSemantic::Enum apexSemantic, const void *srcData, physx::PxU32 srcStride, physx::PxU32 firstDestElement, physx::PxU32 numElements)
		virtual void writeBuffer(const physx::apex::NxApexRenderVertexBufferData &data, physx::PxU32 firstVertex, physx::PxU32 numVerts)
		{
			SAMPLE_PERF_SCOPE(SampleRendererVBwriteBuffer);

			if(!m_vertexbuffer || !numVerts)
			{
				return;
			}

			for(physx::PxU32 i=0; i<physx::apex::NxRenderVertexSemantic::NUM_SEMANTICS; i++)
			{
				physx::apex::NxRenderVertexSemantic::Enum apexSemantic = (physx::apex::NxRenderVertexSemantic::Enum)i;
				const physx::apex::NxApexRenderSemanticData &semanticData = data.getSemanticData(apexSemantic);
				if(semanticData.data)
				{
					const void *srcData = semanticData.data;
					const physx::PxU32 srcStride = semanticData.stride;

					RendererVertexBuffer::Semantic semantic = convertFromApexVB(apexSemantic);
					if(semantic < RendererVertexBuffer::NUM_SEMANTICS)
					{
						RendererVertexBuffer::Format format = m_vertexbuffer->getFormatForSemantic(semantic);
						physx::PxU32 semanticStride = 0;
						void *dstData = m_vertexbuffer->lockSemantic(semantic, semanticStride);
						void *dstDataCopy = dstData;
						PX_ASSERT(dstData && semanticStride);
						if(dstData && semanticStride)
						{
							SAMPLE_PERF_SCOPE(writeBufferSemanticStride);
#if defined(RENDERER_DEBUG) && 0 // enable to confirm that destruction bone indices are within valid range.
							// verify input data...
							if(apexSemantic == NxRenderVertexSemantic::BONE_INDEX)
							{
								physx::PxU32 maxIndex = 0;
								for(physx::PxU32 i=0; i<numVerts; i++)
								{
									physx::PxU16 index = *(physx::PxU16*)(((physx::PxU8*)srcData)+(srcStride*i));
									if(index > maxIndex) maxIndex = index;
								}
								PX_ASSERT(maxIndex < RENDERER_MAX_BONES);
							}
#endif
							dstData = ((physx::PxU8*)dstData)+firstVertex*semanticStride;
							physx::PxU32 formatSize = RendererVertexBuffer::getFormatByteSize(format);
							for(physx::PxU32 i=0; i<numVerts; i++)
							{
								memcpy(dstData, srcData, formatSize);
								dstData = ((physx::PxU8*)dstData)+semanticStride;
								srcData = ((physx::PxU8*)srcData)+srcStride;

							}

							// fix-up the UVs...
							if(semantic >= RendererVertexBuffer::SEMANTIC_TEXCOORD0  && 
								semantic <= RendererVertexBuffer::SEMANTIC_TEXCOORDMAX)
							{
								fixUVOrigin(dstDataCopy, semanticStride, numVerts);
							}

							// swizzle color...
							if(m_swizzleColor && semantic == RendererVertexBuffer::SEMANTIC_COLOR)
							{
								swizzleColor(dstDataCopy, semanticStride, numVerts);
							}

#if defined(PX_PS3)
							if(RendererVertexBuffer::SEMANTIC_COLOR == semantic)
							{
								SampleApexRendererSwapColorEndian(dstDataCopy, semanticStride, numVerts);
							}
#endif

							m_vertexbuffer->unlockSemantic(semantic);
						}
					}
				}
			}
		}
		
	private:
		Renderer								&m_renderer;
		RendererVertexBuffer					*m_vertexbuffer;
		physx::apex::NxTextureUVOrigin::Enum	 m_uvOrigin;
		bool									 m_swizzleColor;
};

/********************************
* SampleApexRendererIndexBuffer *
********************************/

class SampleApexRendererIndexBuffer : public physx::apex::NxUserRenderIndexBuffer
{
	friend class SampleApexRendererMesh;
	public:
		SampleApexRendererIndexBuffer(Renderer &renderer, const physx::apex::NxUserRenderIndexBufferDesc &desc) :
			m_renderer(renderer)
		{
			m_indexbuffer = 0;
			RendererIndexBufferDesc ibdesc;
			ibdesc.hint       = convertFromApexIB(desc.hint);
			ibdesc.format     = convertFromApexIB(desc.format);
			ibdesc.maxIndices = desc.maxIndices;
#if defined(APEX_CUDA_SUPPORT)
			ibdesc.registerInCUDA = desc.registerInCUDA;
			ibdesc.interopContext = desc.interopContext;
#endif
			m_indexbuffer = m_renderer.createIndexBuffer(ibdesc);
			PX_ASSERT(m_indexbuffer);

			m_primitives = desc.primitives;
		}
		
		virtual bool getInteropResourceHandle(CUgraphicsResource &handle)
		{
#if defined(APEX_CUDA_SUPPORT)
			if(m_indexbuffer)
				return	m_indexbuffer->getInteropResourceHandle(handle);
			else
				return false;
#else
			return false;
#endif
		}
		
		virtual ~SampleApexRendererIndexBuffer(void)
		{
			if(m_indexbuffer) m_indexbuffer->release();
		}
	
	private:
		virtual void writeBuffer(const void *srcData, physx::PxU32 srcStride, physx::PxU32 firstDestElement, physx::PxU32 numElements)
		{
			if(m_indexbuffer && numElements)
			{
				void *dstData = m_indexbuffer->lock();
				PX_ASSERT(dstData);
				if(dstData)
				{
					RendererIndexBuffer::Format format = m_indexbuffer->getFormat();

					if (m_primitives == physx::apex::NxRenderPrimitiveType::TRIANGLES)
					{
						physx::PxU32 numTriangles = numElements / 3;
						if(format == RendererIndexBuffer::FORMAT_UINT16)
						{
							physx::PxU16       *dst = ((physx::PxU16*)dstData)+firstDestElement;
							const physx::PxU16 *src = (const physx::PxU16*)srcData;
							for(physx::PxU32 i=0; i<numTriangles; i++)
							for(physx::PxU32 j=0; j<3; j++)
							{
								dst[i*3+j] = src[i*3+(2-j)];
							}
						}
						else if(format == RendererIndexBuffer::FORMAT_UINT32)
						{
							physx::PxU32       *dst = ((physx::PxU32*)dstData)+firstDestElement;
							const physx::PxU32 *src = (const physx::PxU32*)srcData;
							for(physx::PxU32 i=0; i<numTriangles; i++)
							for(physx::PxU32 j=0; j<3; j++)
							{
								dst[i*3+j] = src[i*3+(2-j)];
							}
						}
						else
						{
							PX_ASSERT(0);
						}
					}
					else
					{
						if(format == RendererIndexBuffer::FORMAT_UINT16)
						{
							physx::PxU16		*dst = ((physx::PxU16*)dstData)+firstDestElement;
							const physx::PxU8	*src = (const physx::PxU8*)srcData;
							for(physx::PxU32 i=0; i<numElements; i++, dst++, src += srcStride) *dst = *((physx::PxU16*)src);
						}
						else if(format == RendererIndexBuffer::FORMAT_UINT32)
						{
							physx::PxU32		*dst = ((physx::PxU32*)dstData)+firstDestElement;
							const physx::PxU8	*src = (const physx::PxU8*)srcData;
							for(physx::PxU32 i=0; i<numElements; i++, dst++, src += srcStride) *dst = *((physx::PxU32*)src);
						}
						else
						{
							PX_ASSERT(0);
						}
					}
				}
				m_indexbuffer->unlock();
			}
		}
	
	private:
		Renderer									&m_renderer;
		RendererIndexBuffer							*m_indexbuffer;
		physx::apex::NxRenderPrimitiveType::Enum	 m_primitives;
};

/*******************************
* SampleApexRendererBoneBuffer *
*******************************/

class SampleApexRendererBoneBuffer : public physx::apex::NxUserRenderBoneBuffer
{
	friend class SampleApexRendererMesh;
	public:
		SampleApexRendererBoneBuffer(Renderer &renderer, const physx::apex::NxUserRenderBoneBufferDesc &desc) :
			m_renderer(renderer)
		{
			m_maxBones = desc.maxBones;
			m_bones    = new physx::PxMat34Legacy[m_maxBones];
		}
		
		virtual ~SampleApexRendererBoneBuffer(void)
		{
			if(m_bones) delete [] m_bones;
		}
		
	public:
		//virtual void writeBuffer(physx::apex::NxRenderBoneSemantic::Enum semantic, const void *srcData, physx::PxU32 srcStride, physx::PxU32 firstDestElement, physx::PxU32 numElements)
		virtual void writeBuffer(const physx::apex::NxApexRenderBoneBufferData &data, physx::PxU32 firstBone, physx::PxU32 numBones)
		{
			const physx::apex::NxApexRenderSemanticData &semanticData = data.getSemanticData(physx::apex::NxRenderBoneSemantic::POSE);
			if(semanticData.data && m_bones)
			{
				const void *srcData = semanticData.data;
				const physx::PxU32 srcStride = semanticData.stride;
				for(physx::PxU32 i=0; i<numBones; i++)
				{
					m_bones[firstBone+i] = *(const physx::PxMat34Legacy*)srcData;
					srcData = ((physx::PxU8*)srcData)+srcStride;
				}
			}
		}
	
	private:
		Renderer &m_renderer;
		physx::PxMat34Legacy  *m_bones;
		physx::PxU32     m_maxBones;
};

/***********************************
* SampleApexRendererInstanceBuffer *
***********************************/
#if 0
physx::PxF32 lifeRemain[0x10000]; //64k
#endif
class SampleApexRendererInstanceBuffer : public physx::apex::NxUserRenderInstanceBuffer
{
	friend class SampleApexRendererMesh;
	public:

		// $$ BED : Maybe not obvious, but this 
		SampleApexRendererInstanceBuffer(Renderer &renderer, const physx::apex::NxUserRenderInstanceBufferDesc &desc)
		{
			m_maxInstances   = desc.maxInstances;
			m_instanceBuffer = 0;
			
			RendererInstanceBufferDesc ibdesc;

			ibdesc.hint = RendererInstanceBuffer::HINT_DYNAMIC;
			ibdesc.maxInstances = desc.maxInstances;

			for(physx::PxU32 i=0;i<RendererInstanceBuffer::NUM_SEMANTICS;i++)
			{
				ibdesc.semanticFormats[i] = RendererInstanceBuffer::NUM_FORMATS;
			}

			for(physx::PxU32 i=0; i<physx::apex::NxRenderInstanceSemantic::NUM_SEMANTICS; i++)
			{
				// Skip unspecified, but if it IS specified, IGNORE the specification and make your own up!! yay! 
				if(desc.semanticFormats[i] == physx::apex::NxRenderDataFormat::UNSPECIFIED) continue;

				switch(i)
				{
				case physx::apex::NxRenderInstanceSemantic::POSITION:
					ibdesc.semanticFormats[RendererInstanceBuffer::SEMANTIC_POSITION] = RendererInstanceBuffer::FORMAT_FLOAT3;
					break;
				case physx::apex::NxRenderInstanceSemantic::ROTATION_SCALE:
					ibdesc.semanticFormats[RendererInstanceBuffer::SEMANTIC_NORMALX] = RendererInstanceBuffer::FORMAT_FLOAT3;
					ibdesc.semanticFormats[RendererInstanceBuffer::SEMANTIC_NORMALY] = RendererInstanceBuffer::FORMAT_FLOAT3;
					ibdesc.semanticFormats[RendererInstanceBuffer::SEMANTIC_NORMALZ] = RendererInstanceBuffer::FORMAT_FLOAT3;
					break;
				case physx::apex::NxRenderInstanceSemantic::VELOCITY_LIFE:
					ibdesc.semanticFormats[RendererInstanceBuffer::SEMANTIC_VELOCITY_LIFE] = RendererInstanceBuffer::FORMAT_FLOAT4;
					break;
				case physx::apex::NxRenderInstanceSemantic::DENSITY:
					ibdesc.semanticFormats[RendererInstanceBuffer::SEMANTIC_DENSITY] = RendererInstanceBuffer::FORMAT_FLOAT1;
					break;
				}
			}
			
#if defined(APEX_CUDA_SUPPORT)
			ibdesc.registerInCUDA = desc.registerInCUDA;
			ibdesc.interopContext = desc.interopContext;
#endif
			m_instanceBuffer = renderer.createInstanceBuffer(ibdesc);
		}
		
		virtual ~SampleApexRendererInstanceBuffer(void)
		{
			if(m_instanceBuffer) m_instanceBuffer->release();
		}
		
		physx::PxU32 getMaxInstances(void) const
		{
			return m_maxInstances;
		}
		
	public:
		virtual void writeBuffer(const physx::apex::NxApexRenderInstanceBufferData &data, physx::PxU32 firstInstance, physx::PxU32 numInstances)
		{
			for(physx::PxU32 i=0; i<physx::apex::NxRenderInstanceSemantic::NUM_SEMANTICS; i++)
			{
				physx::apex::NxRenderInstanceSemantic::Enum semantic = (physx::apex::NxRenderInstanceSemantic::Enum)i;
				const physx::apex::NxApexRenderSemanticData &semanticData = data.getSemanticData(semantic);
				if(semanticData.data)
				{
					internalWriteBuffer(semantic, semanticData.data, semanticData.stride, firstInstance, numInstances);
				}
			}

			
		}
		
		

		virtual bool getInteropResourceHandle(CUgraphicsResource &handle)
		{
#if defined(APEX_CUDA_SUPPORT)
			if(m_instanceBuffer)
				return	m_instanceBuffer->getInteropResourceHandle(handle);
			else
				return false;
#else
			return false;
#endif
		}
	
	private:

		template<typename ElemType>
		void internalWriteSemantic(RendererInstanceBuffer::Semantic semantic, const void *srcData, physx::PxU32 srcStride, physx::PxU32 firstDestElement, physx::PxU32 numElements)
		{
			physx::PxU32 destStride = 0; 
			physx::PxU8* destData = (physx::PxU8*)m_instanceBuffer->lockSemantic(semantic, destStride);
			if(destData)
			{
				destData += firstDestElement * destStride;
				for(physx::PxU32 i=0; i<numElements; i++)
				{
					ElemType* srcElemPtr = (ElemType*) (((physx::PxU8*)srcData) + srcStride * i);

					*((ElemType*)destData) = *srcElemPtr;

					destData += destStride;
				}
				m_instanceBuffer->unlockSemantic(semantic);
			}
		}

		void internalWriteBuffer(physx::apex::NxRenderInstanceSemantic::Enum semantic, const void *srcData, physx::PxU32 srcStride,
			physx::PxU32 firstDestElement, physx::PxU32 numElements)
		{
			if(semantic == physx::apex::NxRenderInstanceSemantic::POSITION)
			{
				internalWriteSemantic<physx::PxVec3>(RendererInstanceBuffer::SEMANTIC_POSITION, srcData, srcStride, firstDestElement, numElements);
			}
			else if(semantic == physx::apex::NxRenderInstanceSemantic::ROTATION_SCALE)
			{
				PX_ASSERT(m_instanceBuffer);
			
				physx::PxU32 xnormalsStride = 0; physx::PxU8* xnormals = (physx::PxU8*)m_instanceBuffer->lockSemantic(RendererInstanceBuffer::SEMANTIC_NORMALX, xnormalsStride);
				physx::PxU32 ynormalsStride = 0; physx::PxU8* ynormals = (physx::PxU8*)m_instanceBuffer->lockSemantic(RendererInstanceBuffer::SEMANTIC_NORMALY, ynormalsStride);
				physx::PxU32 znormalsStride = 0; physx::PxU8* znormals = (physx::PxU8*)m_instanceBuffer->lockSemantic(RendererInstanceBuffer::SEMANTIC_NORMALZ, znormalsStride);

				PX_ASSERT(xnormals && ynormals && znormals);
				
				if(xnormals && ynormals && znormals)
				{
					xnormals += firstDestElement * xnormalsStride;
					ynormals += firstDestElement * ynormalsStride;
					znormals += firstDestElement * znormalsStride;

					for(physx::PxU32 i=0; i<numElements; i++)
					{
						physx::PxVec3* p = (physx::PxVec3*) (((physx::PxU8*)srcData) + srcStride * i);
						
						*((physx::PxVec3*)xnormals) = p[0];
						*((physx::PxVec3*)ynormals) = p[1];
						*((physx::PxVec3*)znormals) = p[2];
						
						xnormals += xnormalsStride;
						ynormals += ynormalsStride;
						znormals += znormalsStride;
					}
				}
				m_instanceBuffer->unlockSemantic(RendererInstanceBuffer::SEMANTIC_NORMALX);
				m_instanceBuffer->unlockSemantic(RendererInstanceBuffer::SEMANTIC_NORMALY);
				m_instanceBuffer->unlockSemantic(RendererInstanceBuffer::SEMANTIC_NORMALZ);
			}
			else if(semantic == physx::apex::NxRenderInstanceSemantic::VELOCITY_LIFE)
			{
				internalWriteSemantic<physx::PxVec4>(RendererInstanceBuffer::SEMANTIC_VELOCITY_LIFE, srcData, srcStride, firstDestElement, numElements);
			}
			else if(semantic == physx::apex::NxRenderInstanceSemantic::DENSITY)
			{
				internalWriteSemantic<physx::PxF32>(RendererInstanceBuffer::SEMANTIC_DENSITY, srcData, srcStride, firstDestElement, numElements);
			}
		}

	private:
		physx::PxU32                m_maxInstances;
		RendererInstanceBuffer *    m_instanceBuffer;
};

#if USE_RENDER_SPRITE_BUFFER

/*********************************
* SampleApexRendererSpriteBuffer *
*********************************/

/*
 *	This class is just a wrapper around the vertex buffer class because there is already
 *	a point sprite implementation using vertex buffers.  It takes the sprite buffer semantics
 *  and just converts them to vertex buffer semantics and ignores everything but position and color.
 *  Well, not really, it takes the lifetime and translates it to color.
 */
class SampleApexRendererSpriteBuffer : public physx::apex::NxUserRenderSpriteBuffer
{
	public:
		SampleApexRendererSpriteBuffer(Renderer &renderer, const physx::apex::NxUserRenderSpriteBufferDesc &desc) :
			m_renderer(renderer)
		{
			m_vertexbuffer = 0;
			RendererVertexBufferDesc vbdesc;
			vbdesc.hint = convertFromApexVB(desc.hint);
#if defined(APEX_CUDA_SUPPORT)
			vbdesc.registerInCUDA = desc.registerInCUDA;
			vbdesc.interopContext = desc.interopContext;
#endif
			for(physx::PxU32 i=0; i<physx::apex::NxRenderSpriteSemantic::NUM_SEMANTICS; i++)
			{
				RendererVertexBuffer::Semantic semantic = convertFromApexSB((physx::apex::NxRenderSpriteSemantic::Enum)i);
				bool tmpSwizzle;
				RendererVertexBuffer::Format   format   = convertFromApexVB(desc.semanticFormats[i], tmpSwizzle);
				if(semantic < RendererVertexBuffer::NUM_SEMANTICS && format < RendererVertexBuffer::NUM_FORMATS)
				{
					vbdesc.semanticFormats[semantic] = format;
				}
			}
			vbdesc.maxVertices = desc.maxSprites;
			m_vertexbuffer = m_renderer.createVertexBuffer(vbdesc);
			PX_ASSERT(m_vertexbuffer);
		}
		
		virtual bool getInteropResourceHandle(CUgraphicsResource &handle)
		{
#if defined(APEX_CUDA_SUPPORT)
			if(m_vertexbuffer)
				return	m_vertexbuffer->getInteropResourceHandle(handle);
			else
				return false;
#else
			return false;
#endif
		}

		virtual ~SampleApexRendererSpriteBuffer(void)
		{
			if(m_vertexbuffer) m_vertexbuffer->release();
		}
	
	private:
		//virtual void writeBuffer(physx::apex::NxRenderSpriteSemantic::Enum sbApexSemantic, const void *srcData,
		//	physx::PxU32 srcStride, physx::PxU32 firstDestElement, physx::PxU32 numElements)
		virtual void writeBuffer(const physx::apex::NxApexRenderSpriteBufferData &data, physx::PxU32 firstSprite, physx::PxU32 numSprites)
		{
			if (!m_vertexbuffer || !numSprites)
				return;

			for(physx::PxU32 i=0; i<physx::apex::NxRenderSpriteSemantic::NUM_SEMANTICS; i++)
			{
				physx::apex::NxRenderSpriteSemantic::Enum semantic = (physx::apex::NxRenderSpriteSemantic::Enum)i;
				const physx::apex::NxApexRenderSemanticData &semanticData = data.getSemanticData(semantic);
				if(semanticData.data)
				{
					const void* srcData = semanticData.data;
					const physx::PxU32 srcStride = semanticData.stride;

					physx::apex::NxRenderVertexSemantic::Enum apexSemantic;
					switch(semantic)
					{
					case physx::apex::NxRenderSpriteSemantic::COLOR:
						apexSemantic = physx::apex::NxRenderVertexSemantic::COLOR;
						break;
					case physx::apex::NxRenderSpriteSemantic::POSITION:
						apexSemantic = physx::apex::NxRenderVertexSemantic::POSITION;
						break;
					default: return;
					}

					RendererVertexBuffer::Semantic semantic = convertFromApexVB(apexSemantic);
					if(semantic < RendererVertexBuffer::NUM_SEMANTICS)
					{
						RendererVertexBuffer::Format format = m_vertexbuffer->getFormatForSemantic(semantic);
						physx::PxU32 semanticStride = 0;
						void *dstData = m_vertexbuffer->lockSemantic(semantic, semanticStride);
						void *dstDataCopy = dstData;
						PX_ASSERT(dstData && semanticStride);
						if(dstData && semanticStride)
						{
							dstData = ((physx::PxU8*)dstData)+firstSprite*semanticStride;
							physx::PxU32 formatSize = RendererVertexBuffer::getFormatByteSize(format);
							for(physx::PxU32 i=0; i<numSprites; i++)
							{
								memcpy(dstData, srcData, formatSize);
								srcData = ((physx::PxU8*)srcData)+srcStride;
								dstData = ((physx::PxU8*)dstData)+semanticStride;
							}

#if defined(PX_PS3)
							if(RendererVertexBuffer::SEMANTIC_COLOR == semantic)
							{
								SampleApexRendererSwapColorEndian(dstDataCopy, semanticStride, numSprites);
							}
#endif
							m_vertexbuffer->unlockSemantic(semantic);
						}
					}
				}
			}
		}


	public:
		Renderer             &m_renderer;
		RendererVertexBuffer *m_vertexbuffer;
};

#endif /* USE_RENDER_SPRITE_BUFFER */


/*************************
* SampleApexRendererMesh *
*************************/

/* 
 *	There is some sprite hackery in here now.  Basically, if a sprite buffer is used
 *	we just treat is as a vertex buffer (because it really is a vertex buffer).
 */
class SampleApexRendererMesh : public physx::apex::NxUserRenderResource
{
	friend class SampleApexRenderer;
	public:
		SampleApexRendererMesh(Renderer &renderer, const physx::apex::NxUserRenderResourceDesc &desc) :
			m_renderer(renderer)
		{
			m_vertexBuffers     = 0;
			m_numVertexBuffers  = 0;
			m_indexBuffer       = 0;
			
			m_boneBuffer        = 0;
			m_firstBone         = 0;
			m_numBones          = 0;
			
			m_instanceBuffer    = 0;
			
			m_mesh              = 0;
			
			m_meshTransform.id();
			
			m_numVertexBuffers = desc.numVertexBuffers;
			if(m_numVertexBuffers > 0)
			{
				m_vertexBuffers = new SampleApexRendererVertexBuffer*[m_numVertexBuffers];
				for(physx::PxU32 i=0; i<m_numVertexBuffers; i++)
				{
					m_vertexBuffers[i] = static_cast<SampleApexRendererVertexBuffer*>(desc.vertexBuffers[i]);
				}
			}

			physx::PxU32 numVertexBuffers = m_numVertexBuffers;
#if USE_RENDER_SPRITE_BUFFER
			m_spriteBuffer = static_cast<SampleApexRendererSpriteBuffer*>(desc.spriteBuffer);
			if(m_spriteBuffer)
			{
				numVertexBuffers = 1;
			}
#endif			

			RendererVertexBuffer **internalsvbs = 0;
			if(numVertexBuffers > 0)
			{
				internalsvbs    = new RendererVertexBuffer*[numVertexBuffers];

#if USE_RENDER_SPRITE_BUFFER
				if (m_spriteBuffer)
				{
					internalsvbs[0] = m_spriteBuffer->m_vertexbuffer;
				}
				else
#endif
				{
					for(physx::PxU32 i=0; i<m_numVertexBuffers; i++)
					{
						internalsvbs[i] = m_vertexBuffers[i]->m_vertexbuffer;
					}
				}
			}

			m_indexBuffer    = static_cast<SampleApexRendererIndexBuffer*>(desc.indexBuffer);
			m_boneBuffer     = static_cast<SampleApexRendererBoneBuffer*>(desc.boneBuffer);
			m_instanceBuffer = static_cast<SampleApexRendererInstanceBuffer*>(desc.instanceBuffer);
			
            m_cullMode       = desc.cullMode;

			RendererMeshDesc meshdesc;
			meshdesc.primitives       = convertFromApex(desc.primitives);
			
			meshdesc.vertexBuffers    = internalsvbs;
			meshdesc.numVertexBuffers = numVertexBuffers;
			
#if USE_RENDER_SPRITE_BUFFER
			// the sprite buffer currently uses a vb
			if(m_spriteBuffer)
			{
				meshdesc.firstVertex      = desc.firstSprite;
				meshdesc.numVertices      = desc.numSprites;
			}
			else
#endif
			{
				meshdesc.firstVertex      = desc.firstVertex;
				meshdesc.numVertices      = desc.numVerts;
			}
			
				{
				if (m_indexBuffer != 0)
				{
					meshdesc.indexBuffer      = m_indexBuffer->m_indexbuffer;
					meshdesc.firstIndex       = desc.firstIndex;
					meshdesc.numIndices       = desc.numIndices;
				}
			}

			meshdesc.instanceBuffer   = m_instanceBuffer ? m_instanceBuffer->m_instanceBuffer : 0;
			meshdesc.firstInstance    = desc.firstInstance;
			meshdesc.numInstances     = desc.numInstances;
			m_mesh = m_renderer.createMesh(meshdesc);
			PX_ASSERT(m_mesh);
			if(m_mesh)
			{
				m_meshContext.mesh      = m_mesh;
				m_meshContext.transform = &m_meshTransform;
			}
			
#if USE_RENDER_SPRITE_BUFFER
			// the sprite buffer currently uses a vb
			if(m_spriteBuffer)
			{
				setVertexBufferRange(desc.firstSprite, desc.numSprites);
			}
			else
#endif
			{
				setVertexBufferRange(desc.firstVertex, desc.numVerts);
			}
				setIndexBufferRange(desc.firstIndex, desc.numIndices);
			setBoneBufferRange(desc.firstBone, desc.numBones);
			setInstanceBufferRange(desc.firstInstance, desc.numInstances);
			
			setMaterial(desc.material);
			
			if(internalsvbs) delete [] internalsvbs;
		}
		
		virtual ~SampleApexRendererMesh(void)
		{
			if(m_mesh)              m_mesh->release();
			if(m_vertexBuffers)     delete [] m_vertexBuffers;
		}
		
	
	public:
		void setVertexBufferRange(physx::PxU32 firstVertex, physx::PxU32 numVerts)
		{
			if(m_mesh) m_mesh->setVertexBufferRange(firstVertex, numVerts);
		}
		
		void setIndexBufferRange(physx::PxU32 firstIndex, physx::PxU32 numIndices)
		{
			if(m_mesh) m_mesh->setIndexBufferRange(firstIndex, numIndices);
		}
		
		void setBoneBufferRange(physx::PxU32 firstBone, physx::PxU32 numBones)
		{
			m_firstBone = firstBone;
			m_numBones  = numBones;
		}
		
		void setInstanceBufferRange(physx::PxU32 firstInstance, physx::PxU32 numInstances)
		{
			if(m_mesh) m_mesh->setInstanceBufferRange(firstInstance, numInstances);
		}

#if USE_RENDER_SPRITE_BUFFER
		void setSpriteBufferRange(physx::PxU32 firstSprite, physx::PxU32 numSprites)
		{
			setVertexBufferRange(firstSprite, numSprites);
		}
#endif

		void setMaterial(void *material)
		{
			if(material)
			{
				SampleMaterialAsset &materialasset = *static_cast<SampleMaterialAsset*>(material);
				m_meshContext.material         = materialasset.getMaterial();
				m_meshContext.materialInstance = materialasset.getMaterialInstance();

#if USE_RENDER_SPRITE_BUFFER
				// get sprite shader variables
				if(m_spriteBuffer)
				{
					m_spriteShaderVariables[0] = m_meshContext.materialInstance->findVariable("windowWidth", RendererMaterial::VARIABLE_FLOAT);
					m_spriteShaderVariables[1] = m_meshContext.materialInstance->findVariable("particleSize", RendererMaterial::VARIABLE_FLOAT);
				}
#endif
			}
			else
			{
				m_meshContext.material         = 0;
				m_meshContext.materialInstance = 0;
			}
		}

		void setScreenSpace(bool ss)
		{
			m_meshContext.screenSpace = ss;
		}

		physx::PxU32 getNbVertexBuffers() const
		{
			return m_numVertexBuffers;
		}
		
		physx::apex::NxUserRenderVertexBuffer *getVertexBuffer( physx::PxU32 index ) const
		{
			physx::apex::NxUserRenderVertexBuffer *buffer = 0;
			PX_ASSERT(index < m_numVertexBuffers);
			if(index < m_numVertexBuffers)
			{
				buffer = m_vertexBuffers[index];
			}
			return buffer;
		}
		
		physx::apex::NxUserRenderIndexBuffer *getIndexBuffer() const
		{
			return m_indexBuffer;
		}
		
		physx::apex::NxUserRenderBoneBuffer *getBoneBuffer()	const
		{
			return m_boneBuffer;
		}
		
		physx::apex::NxUserRenderInstanceBuffer *getInstanceBuffer()	const
		{
			return m_instanceBuffer;
		}

#if USE_RENDER_SPRITE_BUFFER
		physx::apex::NxUserRenderSpriteBuffer *getSpriteBuffer()	const
		{
			return m_spriteBuffer;
		}
#endif

	private:
		void render(const physx::apex::NxApexRenderContext &context)
		{
			if(m_mesh && m_meshContext.mesh && m_mesh->getNumVertices()>0)
			{
#if USE_RENDER_SPRITE_BUFFER
				// set default sprite shader variables
				if(m_spriteBuffer)
				{
					// windowWidth
					if( m_spriteShaderVariables[0] && m_meshContext.materialInstance )
					{
						physx::PxU32 width, height;
						m_renderer.getWindowSize(width, height);
						physx::PxF32 fwidth = (physx::PxF32)width;
						m_meshContext.materialInstance->writeData(*m_spriteShaderVariables[0], &fwidth);
					}

					// particleSize --- we're not going to do this one for now, it's stored in the material xml file
#if 0
					if( m_spriteShaderVariables[1] && m_meshContext.materialInstance )
					{
						physx::PxF32 fsize = 0.10f;
						m_meshContext.materialInstance->writeData(*m_spriteShaderVariables[1], &fsize);
					}
#endif
				}
#endif /* #if USE_RENDER_SPRITE_BUFFER */

				m_meshTransform = context.local2world;
				if(m_boneBuffer)
				{
					if(m_numBones==1&&0)
					{
						// APEX really should do this for us... but it doesn't yet...
						m_meshContext.boneMatrices = 0;
						m_meshContext.numBones     = 0;
						m_meshTransform = m_boneBuffer->m_bones[0];
					}
					else
					{
						m_meshContext.boneMatrices = m_boneBuffer->m_bones+m_firstBone;
						m_meshContext.numBones     = m_numBones;
					}
				}
				switch(m_cullMode)
				{
				case physx::apex::NxRenderCullMode::CLOCKWISE: 
					m_meshContext.cullMode = RendererMeshContext::CLOCKWISE;
					break;
				case physx::apex::NxRenderCullMode::COUNTER_CLOCKWISE: 
					m_meshContext.cullMode = RendererMeshContext::COUNTER_CLOCKWISE;
					break;
				case physx::apex::NxRenderCullMode::NONE: 
					m_meshContext.cullMode = RendererMeshContext::NONE;
					break;
				default:
					PX_ASSERT(0 && "Invalid Cull Mode");
				}
				m_meshContext.screenSpace = context.isScreenSpace;
				m_renderer.queueMeshForRender(m_meshContext);
			}
		}
	
	private:
		Renderer                         &m_renderer;

#if USE_RENDER_SPRITE_BUFFER
		SampleApexRendererSpriteBuffer   *m_spriteBuffer;

		// currently this renderer's sprite shaders take 2 variables, particleSize and windowWidth
		const RendererMaterial::Variable *m_spriteShaderVariables[2];
#endif

		SampleApexRendererVertexBuffer	  **m_vertexBuffers;
		physx::PxU32						m_numVertexBuffers;
		
		SampleApexRendererIndexBuffer	   *m_indexBuffer;
		
		SampleApexRendererBoneBuffer	   *m_boneBuffer;
		physx::PxU32						m_firstBone;
		physx::PxU32						m_numBones;
		
		SampleApexRendererInstanceBuffer   *m_instanceBuffer;
		
		RendererMesh					   *m_mesh;
		RendererMeshContext					m_meshContext;
		physx::PxMat34Legacy				m_meshTransform;
		physx::apex::NxRenderCullMode::Enum	m_cullMode;
};

/**********************************
* SampleApexRenderResourceManager *
**********************************/

SampleApexRenderResourceManager::SampleApexRenderResourceManager(Renderer &renderer) :
	m_renderer(renderer)
{
	m_numVertexBuffers   = 0;
	m_numIndexBuffers    = 0;
	m_numBoneBuffers     = 0;
	m_numInstanceBuffers = 0;
	m_numResources       = 0;
}

SampleApexRenderResourceManager::~SampleApexRenderResourceManager(void)
{
	RENDERER_ASSERT(m_numVertexBuffers   == 0, "Not all Vertex Buffers were released prior to Render Resource Manager destruction!");
	RENDERER_ASSERT(m_numIndexBuffers    == 0, "Not all Index Buffers were released prior to Render Resource Manager destruction!");
	RENDERER_ASSERT(m_numBoneBuffers     == 0, "Not all Bone Buffers were released prior to Render Resource Manager destruction!");
	RENDERER_ASSERT(m_numInstanceBuffers == 0, "Not all Instance Buffers were released prior to Render Resource Manager destruction!");
	RENDERER_ASSERT(m_numResources       == 0, "Not all Resources were released prior to Render Resource Manager destruction!");
}

physx::apex::NxUserRenderVertexBuffer *SampleApexRenderResourceManager::createVertexBuffer( const physx::apex::NxUserRenderVertexBufferDesc &desc )
{
	SampleApexRendererVertexBuffer *vb = 0;
	PX_ASSERT(desc.isValid());
	if(desc.isValid())
	{
		vb = new SampleApexRendererVertexBuffer(m_renderer, desc);
		m_numVertexBuffers++;
	}
	return vb;
}

void SampleApexRenderResourceManager::releaseVertexBuffer( physx::apex::NxUserRenderVertexBuffer &buffer )
{
	PX_ASSERT(m_numVertexBuffers > 0);
	m_numVertexBuffers--;
	delete &buffer;
}

physx::apex::NxUserRenderIndexBuffer *SampleApexRenderResourceManager::createIndexBuffer( const physx::apex::NxUserRenderIndexBufferDesc &desc )
{
	SampleApexRendererIndexBuffer *ib = 0;
	PX_ASSERT(desc.isValid());
	if(desc.isValid())
	{
		ib = new SampleApexRendererIndexBuffer(m_renderer, desc);
		m_numIndexBuffers++;
	}
	return ib;
}

void SampleApexRenderResourceManager::releaseIndexBuffer( physx::apex::NxUserRenderIndexBuffer &buffer )
{
	PX_ASSERT(m_numIndexBuffers > 0);
	m_numIndexBuffers--;
	delete &buffer;
}

physx::apex::NxUserRenderBoneBuffer *SampleApexRenderResourceManager::createBoneBuffer( const physx::apex::NxUserRenderBoneBufferDesc &desc )
{
	SampleApexRendererBoneBuffer *bb = 0;
	PX_ASSERT(desc.isValid());
	if(desc.isValid())
	{
		bb = new SampleApexRendererBoneBuffer(m_renderer, desc);
		m_numBoneBuffers++;
	}
	return bb;
}

void SampleApexRenderResourceManager::releaseBoneBuffer( physx::apex::NxUserRenderBoneBuffer &buffer )
{
	PX_ASSERT(m_numBoneBuffers > 0);
	m_numBoneBuffers--;
	delete &buffer;
}

physx::apex::NxUserRenderInstanceBuffer *SampleApexRenderResourceManager::createInstanceBuffer( const physx::apex::NxUserRenderInstanceBufferDesc &desc )
{
	SampleApexRendererInstanceBuffer *ib = 0;
	PX_ASSERT(desc.isValid());
	if(desc.isValid())
	{
		ib = new SampleApexRendererInstanceBuffer(m_renderer, desc);
		m_numInstanceBuffers++;
	}
	return ib;
}

void SampleApexRenderResourceManager::releaseInstanceBuffer( physx::apex::NxUserRenderInstanceBuffer &buffer )
{
	PX_ASSERT(m_numInstanceBuffers > 0);
	m_numInstanceBuffers--;
	delete &buffer;
}

physx::apex::NxUserRenderSpriteBuffer *SampleApexRenderResourceManager::createSpriteBuffer( const physx::apex::NxUserRenderSpriteBufferDesc &desc )
{
#if USE_RENDER_SPRITE_BUFFER
	SampleApexRendererSpriteBuffer *sb = 0;
	PX_ASSERT(desc.isValid());
	if(desc.isValid())
	{
		// convert SB to VB
		sb = new SampleApexRendererSpriteBuffer(m_renderer, desc);
		m_numVertexBuffers++;
	}
	return sb;
#else
	return NULL;
#endif
}

void  SampleApexRenderResourceManager::releaseSpriteBuffer( physx::apex::NxUserRenderSpriteBuffer &buffer )
{
#if USE_RENDER_SPRITE_BUFFER
	// LRR: for now, just use a VB
	PX_ASSERT(m_numVertexBuffers > 0);
	m_numVertexBuffers--;
	delete &buffer;
#endif
}

physx::apex::NxUserRenderResource *SampleApexRenderResourceManager::createResource( const physx::apex::NxUserRenderResourceDesc &desc )
{
	SampleApexRendererMesh *mesh = 0;
	PX_ASSERT(desc.isValid());
	if(desc.isValid())
	{
		mesh = new SampleApexRendererMesh(m_renderer, desc);
		m_numResources++;
	}
	return mesh;
}

void SampleApexRenderResourceManager::releaseResource( physx::apex::NxUserRenderResource &resource )
{
	PX_ASSERT(m_numResources > 0);
	m_numResources--;
	delete &resource;
}

physx::PxU32 SampleApexRenderResourceManager::getMaxBonesForMaterial(void *material)
{
	return RENDERER_MAX_BONES;
}

/*********************
* SampleApexRenderer *
*********************/

void SampleApexRenderer::renderResource(const physx::apex::NxApexRenderContext &context)
{
	if(context.renderResource)
	{
		static_cast<SampleApexRendererMesh*>(context.renderResource)->render(context);
	}
}

/*****************************
* SampleApexResourceCallback *
*****************************/

SampleApexResourceCallback::SampleApexResourceCallback(Renderer &renderer, SampleAssetManager &assetManager) :
	m_renderer(renderer),
	m_assetManager(assetManager)
{
	m_apexSDK                   = 0;
	m_apexParticlesModule       = 0;
	m_apexIofxModule            = 0;
	m_apexDestructibleModule    = 0;
	m_apexTurbulenceModule      = 0;
    m_apexExplosionModule       = 0;
	m_numGets                   = 0;
	
	// if this resizes it will cause problems
	m_nxGroupsMasks.reserve(128);

#if DEBUG_RESOURCE_REQUESTS
	if(!gDebugOutput)
	{
		gDebugOutput = fopen(DEBUG_OUTPUT_FILENAME, "w");
	}
#endif
}

SampleApexResourceCallback::~SampleApexResourceCallback(void)
{
	PX_ASSERT(m_numGets == 0);
	clearResourceSearchPaths();

#if DEBUG_RESOURCE_REQUESTS
	if(gDebugOutput && gDebugOutput != stdout && gDebugOutput != stderr)
	{
		fclose(gDebugOutput);
	}
#endif
}

void SampleApexResourceCallback::addResourceSearchPath(const char *path)
{
	physx::PxU32 len = path && *path ? (physx::PxU32)strlen(path) : 0;
	if(len)
	{
		len++;
		char *searchPath = new char[len];
		physx::string::strncpy_s(searchPath, len, path, len);
		m_searchPaths.push_back(searchPath);
	}
}

void SampleApexResourceCallback::removeResourceSearchPath(const char *path)
{
	if (path)
	{
		for(physx::PxU32 i = 0; i < m_searchPaths.size(); i++)
		{
			if (strcmp(m_searchPaths[i], path) == 0)
			{
				m_searchPaths.erase(m_searchPaths.begin()+i);
				break;
			}
		}
	}
}

void SampleApexResourceCallback::clearResourceSearchPaths()
{
	const size_t numSearchPaths = m_searchPaths.size();
	for(size_t i=0; i<numSearchPaths; i++)
	{
		delete [] m_searchPaths[i];
	}
	m_searchPaths.clear();
}

void SampleApexResourceCallback::registerNxCollisionGroup(const char *name, NxCollisionGroup group)
{
	NX_ASSERT( m_apexSDK );
	if(m_apexSDK)
	{
		m_apexSDK->getNamedResourceProvider()->setResource(APEX_COLLISION_GROUP_NAME_SPACE, name, (void*)group);
	}
}

void SampleApexResourceCallback::registerNxGroupsMask128(const char *name, NxGroupsMask& groupsMask)
{
	NX_ASSERT( m_apexSDK );
	NX_ASSERT( m_nxGroupsMasks.size() < 128 );
	if(m_apexSDK)
	{
		m_nxGroupsMasks.push_back(groupsMask);
		m_apexSDK->getNamedResourceProvider()->setResource(APEX_COLLISION_GROUP_128_NAME_SPACE, name, (void*)( &(m_nxGroupsMasks.back())) );
	}
}

void SampleApexResourceCallback::registerNxCollisionGroupsMask(const char *name, NxU32 groupsBitMask)
{
	NX_ASSERT( m_apexSDK );
	if(m_apexSDK)
	{
		m_apexSDK->getNamedResourceProvider()->setResource(APEX_COLLISION_GROUP_MASK_NAME_SPACE, name, (void*)(NxU64)groupsBitMask);
	}
}

void SampleApexResourceCallback::registerPhysicalMaterial(const char *name, NxMaterialIndex physicalMaterial)
{
	NX_ASSERT( m_apexSDK );
	if(m_apexSDK)
	{
		m_apexSDK->getNamedResourceProvider()->setResource(APEX_PHYSICS_MATERIAL_NAME_SPACE, name, (void*)physicalMaterial);
	}
}

void SampleApexResourceCallback::setApexSupport(physx::apex::NxApexSDK &apexSDK, physx::apex::NxModuleParticles *apexParticlesModule)
{
	PX_ASSERT(!m_apexSDK && !m_apexParticlesModule);
	m_apexSDK             = &apexSDK;
	m_apexParticlesModule = apexParticlesModule;
}

void SampleApexResourceCallback::setIofxModule(physx::apex::NxModuleIofx &apexIofxModule)
{
	m_apexIofxModule = &apexIofxModule;
}

void SampleApexResourceCallback::setEmitterModule(physx::apex::NxModuleEmitter &apexEmitterModule)
{
	m_apexEmitterModule = &apexEmitterModule;
}

void SampleApexResourceCallback::setDestructibleModule(physx::apex::NxModuleDestructible &apexDestructibleModule)
{
	m_apexDestructibleModule = &apexDestructibleModule;
}

void SampleApexResourceCallback::setTurbulenceModule(physx::apex::NxModuleTurbulence &apexTurbulenceModule )
{
	m_apexTurbulenceModule = &apexTurbulenceModule;
}

void SampleApexResourceCallback::setExplosionModule(physx::apex::NxModuleExplosion &apexExplosionModule )
{
	m_apexExplosionModule = &apexExplosionModule;
}

void SampleApexResourceCallback::setForceFieldModule(physx::apex::NxModuleForceField &apexForceFieldModule )
{
	m_apexForceFieldModule = &apexForceFieldModule;
}

void SampleApexResourceCallback::setClothingModule(physx::apex::NxModuleClothing &apexClothingModule )
{
	m_apexClothingModule = &apexClothingModule;
}

void SampleApexResourceCallback::fixupAssetName(char *outStr, size_t outStrCapacity, const char *inStr)
{
	PX_ASSERT( strlen(inStr) < APEX_CALLBACK_NAME_STR_LEN );

	if( !inStr )
	{
		if( outStr && outStrCapacity )
		{
			outStr[0] = '\0';
		}
		return;
	}

	// for xbox, flip from forward slash to backslash
#ifdef _XBOX
	size_t i;
	for (i = 0; i < strlen(inStr); i++)
	{
		if (inStr[i] == '/')
			outStr[i] = '\\';
		else
			outStr[i] = inStr[i];
	}
	outStr[i] = '\0';
#else
	physx::string::strcpy_s(outStr, outStrCapacity, inStr);	
#endif
}

physx::PxFileBuf *SampleApexResourceCallback::findApexAsset(const char *assetName)
{
	physx::PxFileBuf *outStream = 0;
	const size_t numSearchPaths = m_searchPaths.size();
	for(size_t i=0; i<numSearchPaths; i++)
	{
		const char *searchPath = m_searchPaths[i];
		const physx::PxU32 pathMaxLen = 512;
		char        fullPath[pathMaxLen] = {0};
		physx::string::strcpy_s(fullPath, pathMaxLen, searchPath);
		physx::string::strcat_s(fullPath, pathMaxLen, assetName);
		
		outStream = m_apexSDK->createStream(fullPath, physx::apex::NxApexStreamFlags::OPEN_FOR_READ);
		if(outStream && outStream->getOpenMode() == physx::PxFileBuf::OPEN_READ_ONLY) break;
		else if(outStream)
		{
			outStream->release();
			outStream = 0;
		}
	}
	PX_ASSERT(outStream);
	return outStream;
}

bool SampleApexResourceCallback::doesFileExist(const char *filename, const char *ext)
{
	char fullname[512] = {0};
	physx::string::strcat_s(fullname, sizeof(fullname), filename);
	physx::string::strcat_s(fullname, sizeof(fullname), ext);

	return doesFileExist(fullname);
}

bool SampleApexResourceCallback::doesFileExist(const char *filename)
{
	const size_t numSearchPaths = m_searchPaths.size();
	for(size_t i=0; i<numSearchPaths; i++)
	{
		const char *searchPath = m_searchPaths[i];
		const physx::PxU32 pathMaxLen = 512;
		char        fullPath[pathMaxLen] = {0};
		physx::string::strcpy_s(fullPath, pathMaxLen, searchPath);
		physx::string::strcat_s(fullPath, pathMaxLen, filename);

		char *fixedPath = new char[APEX_CALLBACK_NAME_STR_LEN];
		fixupAssetName(fixedPath, APEX_CALLBACK_NAME_STR_LEN, fullPath);

		//TODO: this is brute force, should avoid file creation
		physx::PxFileBuf *outStream = m_apexSDK->createStream(fixedPath, physx::apex::NxApexStreamFlags::OPEN_FOR_READ);
		if( !outStream || outStream->getOpenMode() != physx::PxFileBuf::OPEN_READ_ONLY)
			outStream->release();
		else
		{
			outStream->release();
			return true;
		}
		delete[] fixedPath;
	}

	return false;
}

SampleAsset *SampleApexResourceCallback::findSampleAsset(const char *assetName, SampleAsset::Type type)
{
	SampleAsset *asset = 0;
#if WORK_AROUND_BROKEN_ASSET_PATHS
	assetName = mapHackyPath(assetName);
#endif
	asset = m_assetManager.getAsset(assetName, type);
	PX_ASSERT(asset);
	return asset;
}

#if WORK_AROUND_BROKEN_ASSET_PATHS
	const char *SampleApexResourceCallback::mapHackyPath(const char *path)
	{
		const char *mappedPath = path;
		struct HackyPath
		{
			const char *original;
			const char *mapped;
		};
		static const HackyPath map[]=
		{
			// John, fix the apex debug renderer to allow the user to specify the material path...
			{ "ApexSolidShaded", "materials/simple_lit_color.xml"   },
			{ "ApexWireframe",   "materials/simple_unlit.xml"       },
		};
		const physx::PxU32 mapSize = sizeof(map) / sizeof(map[0]);
		for(physx::PxU32 i=0; i<mapSize; i++)
		{
			const HackyPath &hp = map[i];
			if(!strcmp(hp.original, mappedPath))
			{
				mappedPath = hp.mapped;
				break;
			}
		}
		PX_ASSERT(mappedPath == path && "IF YOU HIT THIS ASSET IT MEANS A HACKY PATH WAS MAPPED, FIX YOUR F-ING ASSET FILES!!!");
		return mappedPath;
	}

	bool SampleApexResourceCallback::xmlFileExtension(const char *name)
	{
		const char *ext = 0;
		for(const char *c=name; *c; c++)
		{
			if(*c=='/' || *c=='\\') ext = 0;
			else if(*c=='.')        ext = c;
		}

		if( !strcmp( ".xml", ext) )
			return true;
		else
			return false;
	}

	// This returns all extensions (.xml.flz, .xml, .ini.old)
	const char * SampleApexResourceCallback::getFileExtension(const char *name)
	{
		const char *ext = 0;
		for(const char *c=name; *c; c++)
		{
			if(*c=='/' || *c=='\\') ext = 0;
			else if(*c=='.')
			{
				ext = c;
				break;
			}
		}

		return ext;
	}

	void SampleApexResourceCallback::fixBrokenExtension(const char *nameSpace, char *path, physx::PxU32 pathMaxLen)
	{
		char *ext = 0;
		for(char *c=path; *c; c++)
		{
			if(*c=='/' || *c=='\\') ext = 0;
			else if(*c=='.')        ext = c;
		}
		PX_ASSERT(ext && "no extension at all! FIX YOUR ASSETS!");
		struct AssetExtension
		{
			const char *typeName;
			const char *extension;
		};
		static const AssetExtension assetExtensionMap[]=
		{
			{ NX_IOFX_AUTHORING_TYPE_NAME,                 "apx"  },
			{ NX_FLUID_IOS_AUTHORING_TYPE_NAME,			   "apx"  },
			{ NX_RENDER_MESH_AUTHORING_TYPE_NAME,          "arm"  },
			{ NX_APEX_EMITTER_AUTHORING_TYPE_NAME,         "aea"  },
			{ NX_GROUND_EMITTER_AUTHORING_TYPE_NAME,       "gea"  },
			{ NX_IMPACT_EMITTER_AUTHORING_TYPE_NAME,       "iea"  },
			{ NX_DESTRUCTIBLE_AUTHORING_TYPE_NAME,         "pda"  },
//			{ NX_TURBULENCE_AUTHORING_TYPE_NAME,           "tur"  },
//			{ NX_TURBULENCE_EMITTER_AUTHORING_TYPE_NAME,   "tem"  },
//			{ NX_TURBULENCE_GRID_AUTHORING_TYPE_NAME,      "tgd"  },
			{ NX_EXPLOSION_AUTHORING_TYPE_NAME,            "epa"  },
			{ NX_FIELD_BOUNDARY_AUTHORING_TYPE_NAME,       "fba"  },
			{ NX_CLOTHING_AUTHORING_TYPE_NAME,             "aca" },
			{ NX_CLOTHING_MATERIAL_AUTHORING_TYPE_NAME,    "acml" },
			{ NX_WIND_AUTHORING_TYPE_NAME,                 "awa"  }
		};
		const physx::PxU32 assetExtensionMapSize = sizeof(assetExtensionMap) / sizeof(assetExtensionMap[0]);
		const char *validExt = 0;
		for(physx::PxU32 i=0; i<assetExtensionMapSize; i++)
		{
			const AssetExtension &ae = assetExtensionMap[i];
			if(!strcmp(ae.typeName, nameSpace))
			{
				validExt = ae.extension;
				break;
			}
		}
		PX_ASSERT(validExt);
		if(validExt)
		{
			if(ext)
			{
				PX_ASSERT(!strcmp(ext+1, validExt) && "extensions do not match! FIX YOUR ASSETS!");
				*ext = 0;
			}
			physx::string::strcat_s(path, pathMaxLen, ".");
			physx::string::strcat_s(path, pathMaxLen, validExt);
		}
	}
#endif

void *SampleApexResourceCallback::requestResource(const char *nameSpace, const char *pname)
{
	void *resource = 0;
	
	PX_ASSERT(nameSpace && *nameSpace);
	PX_ASSERT(pname && *pname);
	
	char *name = new char[APEX_CALLBACK_NAME_STR_LEN];
	fixupAssetName(name, APEX_CALLBACK_NAME_STR_LEN, pname);

#if DEBUG_RESOURCE_REQUESTS
	fprintf(gDebugOutput, "new  - %s\n", name);
#endif

#if WORK_AROUND_BROKEN_ASSET_PATHS
	// look for goofy "::" characters...
	char *p=name;
	while(*p && *p!=':') p++;
	PX_ASSERT(*p!=':' && "Obsolete asset name format, please fix your assets!");
	if(*p==':')
	{
		while(*p==':')p++;
		name = p;
	}
#endif
	
	if(!strcmp(nameSpace, APEX_MATERIALS_NAME_SPACE))
	{
		SampleAsset *asset = findSampleAsset(name, SampleAsset::ASSET_MATERIAL);
		if(asset)
		{
			resource = asset;
		}
	}
	else if(!strcmp(nameSpace, APEX_CUSTOM_VB_NAME_SPACE))
	{
		// We currently don't support any custom vertex semantics in the samples,
		// so the resource will simply be a copy the name.  A real game engine
		// could return a pointer to whatever they want, or a member of an enum of
		// custom semantics.  Whatever resources are returned here will be provided
		// in the Render Resources API, in the array:
		//     void ** NxUserRenderVertexBufferDesc::customBuffersIdents

		size_t len = strlen(name);
		char *n = new char[len + 1];
		physx::string::strcpy_s(n, len + 1, name);
		resource = (void *)(n);
	}
	else if(!strcmp(nameSpace, APEX_COLLISION_GROUP_NAME_SPACE))
	{
		NX_ASSERT( 0 && "NRP seed failure for" APEX_COLLISION_GROUP_NAME_SPACE);
	}
	else if(!strcmp(nameSpace, APEX_COLLISION_GROUP_MASK_NAME_SPACE))
	{
		NX_ASSERT( 0 && "NRP seed failure for " APEX_COLLISION_GROUP_MASK_NAME_SPACE);
	}
	else if(!strcmp(nameSpace, APEX_COLLISION_GROUP_128_NAME_SPACE))
	{
		NX_ASSERT( 0 && "NRP seed failure for " APEX_COLLISION_GROUP_128_NAME_SPACE);
	}
	else if(!strcmp(nameSpace, APEX_PHYSICS_MATERIAL_NAME_SPACE))
	{
		NX_ASSERT( 0 && "NRP seed failure for " APEX_PHYSICS_MATERIAL_NAME_SPACE);
	}
	else
	{
		physx::apex::NxApexAsset *asset = 0;

		if( !strcmp(nameSpace, NX_APEX_EMITTER_AUTHORING_TYPE_NAME) ||
			!strcmp(nameSpace, NX_GROUND_EMITTER_AUTHORING_TYPE_NAME) ||
			!strcmp(nameSpace, NX_IMPACT_EMITTER_AUTHORING_TYPE_NAME) ||
			!strcmp(nameSpace, NX_FLUID_IOS_AUTHORING_TYPE_NAME) ||
			!strcmp(nameSpace, NX_IOFX_AUTHORING_TYPE_NAME) ||
			!strcmp(nameSpace, NX_EXPLOSION_AUTHORING_TYPE_NAME) ||
			!strcmp(nameSpace, NX_WIND_AUTHORING_TYPE_NAME) ||
			!strcmp(nameSpace, NX_DESTRUCTIBLE_AUTHORING_TYPE_NAME) ||
			!strcmp(nameSpace, NX_RENDER_MESH_AUTHORING_TYPE_NAME) ||
			!strcmp(nameSpace, NX_CLOTHING_AUTHORING_TYPE_NAME) ||
			!strcmp(nameSpace, NX_FIELD_BOUNDARY_AUTHORING_TYPE_NAME) ||
			!strcmp(nameSpace, NX_BASIC_IOS_AUTHORING_TYPE_NAME))
		{
			// Assets that are using NxParameterized (and serialized outside of APEX)
			// currently have an XML extension.
			PX_ASSERT( name );

			physx::PxFileBuf *stream = 0;

			const char *ext = getFileExtension( name );
			if( ext )
			{
				stream = findApexAsset(name);
			}
			else
			{
#ifdef APEX_SHIPPING
				// try binary files first for shipping builds
				if( ext == NULL && doesFileExist(name, ".apb") )
					ext = ".apb";
#endif

				if( ext == NULL && doesFileExist(name, ".apx") )
					ext = ".apx";

#ifndef APEX_SHIPPING
				// try binary files last for non-shipping builds
				if( ext == NULL && doesFileExist(name, ".apb") )
					ext = ".apb";
#endif

				PX_ASSERT( ext );
				if( ext )
				{
					char fullname[512] = {0};
					physx::string::strcpy_s(fullname, sizeof(fullname), name);
					physx::string::strcat_s(fullname, sizeof(fullname), ext);

					stream = findApexAsset(fullname);
				}
			}

			if( stream )
			{
				NxParameterized::Interface *params = 0;

				// we really shouldn't have extensions in our asset names, and apps should
				// determine the serialization type using this NxApesSDK::getSerializeType() method
				NxParameterized::Serializer::SerializeType serType = m_apexSDK->getSerializeType(*stream);

				if( ext )
				{
					NxParameterized::Serializer::SerializeType iSerType;
					if( 0 == strcmp( ".apx", ext ) )
						iSerType = NxParameterized::Serializer::NST_XML;
					else if( 0 == strcmp( ".apb", ext )  )
						iSerType = NxParameterized::Serializer::NST_BINARY;
					else
						PX_ASSERT( 0 && "Invalid asset file extension" );

					// PH: If you end up here, you have a binary file with an xml extension (.apx or .xml) or vice versa
					PX_ASSERT( iSerType == serType && "Wrong file extension??" );
				}

				NxParameterized::Serializer::ErrorType serError;

				NxParameterized::SerializePlatform platform;
				serError = m_apexSDK->getSerializePlatform(*stream, platform);
				PX_ASSERT(serError == NxParameterized::ERROR_NONE);

				NxParameterized::Serializer  *ser = m_apexSDK->createSerializer(serType);

				NxParameterized::Serializer::DeserializedData data;
				if( NxParameterized::Serializer::NST_BINARY == serType && INPLACE_BINARY && platform == NxParameterized::SerializePlatform::GetCurrentPlatform())
				{
					NxParameterized::Traits *t = m_apexSDK->getParameterizedTraits();
					physx::PxU32 len = stream->getFileLength();
					void *p = t->alloc(len);
					stream->read(p, len);
					serError = ser->deserializeInplace(p, len, data);
				}
				else
					serError = ser->deserialize(*stream, data);

				if(serError == NxParameterized::Serializer::ERROR_NONE && data.size() == 1 )
				{
					NxParameterized::Interface *params = data[0];
					asset = m_apexSDK->createAsset( params, pname );
					PX_ASSERT( asset && "ERROR Creating NxParameterized Asset" );
				}
				else
				{
					PX_ASSERT( 0 && "ERROR Deserializing NxParameterized Asset" );
				}

				stream->release();
				ser->release();
			}
		}
		else
		{
			// This is the legacy asset serialization path, left here in case someone needs
			// it someday
#if WORK_AROUND_BROKEN_ASSET_PATHS
			if(name && *name)
			{
				// name is not const, we've allocated storage for it up above
				fixBrokenExtension(nameSpace, name, APEX_CALLBACK_NAME_STR_LEN);
			}
#endif
			physx::PxFileBuf *stream = findApexAsset(name);
			if(stream)
			{
				asset = m_apexSDK->createAsset(*stream, pname);
				stream->release();
				stream = 0;
			}
		}

		PX_ASSERT(asset);
		if(asset)
		{
			bool rightType = strcmp(nameSpace, asset->getObjTypeName()) == 0;
			PX_ASSERT(rightType);
			if(rightType)
			{
				resource = asset;
			}
			else
			{
				m_apexSDK->releaseAsset(*asset);
				asset = 0;
			}
		}
	}
	
	if(resource)
	{
		m_numGets++;
	}
	else if(m_renderer.getOutputStream())
	{
		// emit a "missing asset" warning using output error stream
		char msg[1024];

		physx::string::sprintf_s(msg, sizeof(msg), "Could not find asset <%s> in namespace <%s>", pname, nameSpace);
		m_renderer.getOutputStream()->reportError(physx::PxErrorCode::eDEBUG_WARNING, msg, __FILE__, __LINE__);
	}
	
	delete[] name;

	return resource;
}

void SampleApexResourceCallback::releaseResource(const char *nameSpace, const char *name, void *resource)
{
	PX_ASSERT(resource);
	if(resource)
	{

#if DEBUG_RESOURCE_REQUESTS
		fprintf(gDebugOutput, "free - %s\n", name);
#endif
		if(!strcmp(nameSpace, APEX_MATERIALS_NAME_SPACE))
		{
			SampleMaterialAsset *asset = static_cast<SampleMaterialAsset*>(resource);
			m_assetManager.returnAsset(*asset);
		}
		else if(!strcmp(nameSpace, APEX_CUSTOM_VB_NAME_SPACE))
		{
			delete (char*) resource;	// char* allocated above with new
		}
		else
		{
			physx::apex::NxApexAsset *asset = (physx::apex::NxApexAsset*)resource;
			m_apexSDK->releaseAsset(*asset);
		}
		m_numGets--;
	}
}
