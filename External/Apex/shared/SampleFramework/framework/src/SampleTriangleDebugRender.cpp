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
#include <SampleTriangleDebugRender.h>
#include <Renderer.h>
#include <RendererVertexBuffer.h>
#include <RendererVertexBufferDesc.h>
#include <RendererMesh.h>
#include <RendererMeshDesc.h>
#include <SampleAssetManager.h>
#include <SampleMaterialAsset.h>

SampleTriangleDebugRender::SampleTriangleDebugRender(Renderer &renderer, SampleAssetManager &assetmanager) :
	m_renderer(renderer),
	m_assetmanager(assetmanager)
{
	m_material = static_cast<SampleMaterialAsset*>(m_assetmanager.getAsset("materials/simple_lit_color.xml", SampleAsset::ASSET_MATERIAL));
	PX_ASSERT(m_material);
	PX_ASSERT(m_material->getNumVertexShaders() == 1);
	m_meshContext.material = m_material ? m_material->getMaterial(0) : 0;
	
	m_maxVerts        = 0;
	m_numVerts        = 0;
	m_vertexbuffer    = 0;
	m_mesh            = 0;
	m_lockedPositions = 0;
	m_positionStride  = 0;
	m_lockedNormals   = 0;
	m_normalStride    = 0;
	m_lockedColors    = 0;
	m_colorStride     = 0;
}

SampleTriangleDebugRender::~SampleTriangleDebugRender(void)
{
	checkUnlock();
	if(m_vertexbuffer) m_vertexbuffer->release();
	if(m_mesh)         m_mesh->release();
	if(m_material)
	{
		m_assetmanager.returnAsset(*m_material);
	}
}

void SampleTriangleDebugRender::addTriangle(const physx::PxVec3 &p0, const physx::PxVec3 &p1, const physx::PxVec3 &p2, const RendererColor &color)
{
	checkResizeTriangle(m_numVerts+2);
	physx::PxVec3 normal = (p1-p0).cross(p2-p0);
	normal.normalize();
	addVert(p0, normal, color);
	addVert(p1, normal, color);
	addVert(p2, normal, color);
}

void SampleTriangleDebugRender::queueForRenderTriangle(void)
{
	if(m_meshContext.mesh)
	{
		checkUnlock();
		m_mesh->setVertexBufferRange(0, m_numVerts);
		m_renderer.queueMeshForRender(m_meshContext);
	}
}

void SampleTriangleDebugRender::clearTriangle(void)
{
	m_numVerts = 0;
}

void SampleTriangleDebugRender::checkResizeTriangle(physx::PxU32 maxVerts)
{
	if(maxVerts > m_maxVerts)
	{
		m_maxVerts = maxVerts + (physx::PxU32)(maxVerts*0.2f);
		
		RendererVertexBufferDesc vbdesc;
		vbdesc.hint = RendererVertexBuffer::HINT_DYNAMIC;
		vbdesc.semanticFormats[RendererVertexBuffer::SEMANTIC_POSITION] = RendererVertexBuffer::FORMAT_FLOAT3;
		vbdesc.semanticFormats[RendererVertexBuffer::SEMANTIC_NORMAL]   = RendererVertexBuffer::FORMAT_FLOAT3;
		vbdesc.semanticFormats[RendererVertexBuffer::SEMANTIC_COLOR]    = RendererVertexBuffer::FORMAT_COLOR;
		vbdesc.maxVertices = m_maxVerts;
		RendererVertexBuffer *vertexbuffer = m_renderer.createVertexBuffer(vbdesc);
		PX_ASSERT(vertexbuffer);
		
		if(vertexbuffer)
		{
			physx::PxU32 positionStride = 0;
			void *positions = vertexbuffer->lockSemantic(RendererVertexBuffer::SEMANTIC_POSITION, positionStride);

			physx::PxU32 normalStride = 0;
			void *normals = vertexbuffer->lockSemantic(RendererVertexBuffer::SEMANTIC_NORMAL, normalStride);
			
			physx::PxU32 colorStride = 0;
			void *colors = vertexbuffer->lockSemantic(RendererVertexBuffer::SEMANTIC_COLOR, colorStride);
			
			PX_ASSERT(positions && colors);
			if(positions && colors)
			{
				if(m_numVerts > 0)
				{
					// if we have existing verts, copy them to the new VB...
					PX_ASSERT(m_lockedPositions);
					PX_ASSERT(m_lockedNormals);
					PX_ASSERT(m_lockedColors);
					if(m_lockedPositions && m_lockedNormals && m_lockedColors)
					{
						for(physx::PxU32 i=0; i<m_numVerts; i++)
						{
							memcpy(((physx::PxU8*)positions) + (positionStride*i), ((physx::PxU8*)m_lockedPositions) + (m_positionStride*i), sizeof(physx::PxVec3));
							memcpy(((physx::PxU8*)normals)   + (normalStride*i),   ((physx::PxU8*)m_lockedNormals)   + (m_normalStride*i),   sizeof(physx::PxVec3));
							memcpy(((physx::PxU8*)colors)    + (colorStride*i),    ((physx::PxU8*)m_lockedColors)    + (m_colorStride*i),    sizeof(RendererColor));
						}
					}
					m_vertexbuffer->unlockSemantic(RendererVertexBuffer::SEMANTIC_COLOR);
					m_vertexbuffer->unlockSemantic(RendererVertexBuffer::SEMANTIC_NORMAL);
					m_vertexbuffer->unlockSemantic(RendererVertexBuffer::SEMANTIC_POSITION);
				}
			}
			vertexbuffer->unlockSemantic(RendererVertexBuffer::SEMANTIC_COLOR);
			vertexbuffer->unlockSemantic(RendererVertexBuffer::SEMANTIC_NORMAL);
			vertexbuffer->unlockSemantic(RendererVertexBuffer::SEMANTIC_POSITION);
		}
		if(m_vertexbuffer)
		{
			m_vertexbuffer->release();
			m_vertexbuffer    = 0;
			m_lockedPositions = 0;
			m_positionStride  = 0;
			m_lockedNormals   = 0;
			m_normalStride    = 0;
			m_lockedColors    = 0;
			m_colorStride     = 0;
		}
		if(m_mesh)
		{
			m_mesh->release();
			m_mesh = 0;
		}
		if(vertexbuffer)
		{
			m_vertexbuffer = vertexbuffer;
			RendererMeshDesc meshdesc;
			meshdesc.primitives       = RendererMesh::PRIMITIVE_TRIANGLES;
			meshdesc.vertexBuffers    = &m_vertexbuffer;
			meshdesc.numVertexBuffers = 1;
			meshdesc.firstVertex      = 0;
			meshdesc.numVertices      = m_numVerts;
			m_mesh = m_renderer.createMesh(meshdesc);
			PX_ASSERT(m_mesh);
		}
		m_meshContext.mesh = m_mesh;
		m_meshContext.cullMode = RendererMeshContext::NONE;
	}
}

void SampleTriangleDebugRender::checkLock(void)
{
	if(m_vertexbuffer)
	{
		if(!m_lockedPositions)
		{
			m_lockedPositions = m_vertexbuffer->lockSemantic(RendererVertexBuffer::SEMANTIC_POSITION, m_positionStride);
			PX_ASSERT(m_lockedPositions);
		}
		if(!m_lockedNormals)
		{
			m_lockedNormals = m_vertexbuffer->lockSemantic(RendererVertexBuffer::SEMANTIC_NORMAL, m_normalStride);
			PX_ASSERT(m_lockedNormals);
		}
		if(!m_lockedColors)
		{
			m_lockedColors = m_vertexbuffer->lockSemantic(RendererVertexBuffer::SEMANTIC_COLOR, m_colorStride);
			PX_ASSERT(m_lockedColors);
		}
	}
}

void SampleTriangleDebugRender::checkUnlock(void)
{
	if(m_vertexbuffer)
	{
		if(m_lockedPositions)
		{
			m_vertexbuffer->unlockSemantic(RendererVertexBuffer::SEMANTIC_POSITION);
			m_lockedPositions = 0;
		}
		if (m_lockedNormals)
		{
			m_vertexbuffer->unlockSemantic(RendererVertexBuffer::SEMANTIC_NORMAL);
			m_lockedNormals = 0;
		}
		if(m_lockedColors)
		{
			m_vertexbuffer->unlockSemantic(RendererVertexBuffer::SEMANTIC_COLOR);
			m_lockedColors = 0;
		}
	}
}

void SampleTriangleDebugRender::addVert(const physx::PxVec3 &p, const physx::PxVec3 &n, const RendererColor &color)
{
	PX_ASSERT(m_maxVerts > m_numVerts);
	{
		checkLock();
		if(m_lockedPositions && m_lockedNormals && m_lockedColors)
		{
			memcpy(((physx::PxU8*)m_lockedPositions) + (m_positionStride*m_numVerts), &p,     sizeof(physx::PxVec3));
			memcpy(((physx::PxU8*)m_lockedNormals)   + (m_normalStride*m_numVerts),   &n,     sizeof(physx::PxVec3));
			memcpy(((physx::PxU8*)m_lockedColors)    + (m_colorStride*m_numVerts),    &color, sizeof(RendererColor));
			m_numVerts++;
		}
	}
}
