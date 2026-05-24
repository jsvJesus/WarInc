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
//
// RendererTerrainShape : convenience class for generating a box mesh.
//
#include "PsShare.h"
#include <RendererTerrainShape.h>

#include <Renderer.h>

#include <RendererVertexBuffer.h>
#include <RendererVertexBufferDesc.h>

#include <RendererIndexBuffer.h>
#include <RendererIndexBufferDesc.h>

#include <RendererMesh.h>
#include <RendererMeshDesc.h>

#include "PxVec3.h"

/*
static physx::PxVec3 operator*(const physx::PxVec3 &a, const physx::PxVec3 &b)
{
	return physx::PxVec3(a.x*b.x, a.y*b.y, a.z*b.z);
}
*/

RendererTerrainShape::RendererTerrainShape(Renderer &renderer, 
										   physx::PxVec3 *verts, physx::PxU32 numVerts, 
										   physx::PxVec3 *normals, physx::PxU32 numNorms,
										   physx::PxU16 *faces, physx::PxU32 numFaces) :
	RendererShape(renderer)
{
	const physx::PxF32 uvscale = 0.01f; // make this an input param?
	RendererVertexBufferDesc vbdesc;
	vbdesc.hint = RendererVertexBuffer::HINT_STATIC;
	vbdesc.semanticFormats[RendererVertexBuffer::SEMANTIC_POSITION]  = RendererVertexBuffer::FORMAT_FLOAT3;
	vbdesc.semanticFormats[RendererVertexBuffer::SEMANTIC_NORMAL]    = RendererVertexBuffer::FORMAT_FLOAT3;
	vbdesc.semanticFormats[RendererVertexBuffer::SEMANTIC_TEXCOORD0] = RendererVertexBuffer::FORMAT_FLOAT2;
	vbdesc.maxVertices = numVerts;
	m_vertexBuffer = m_renderer.createVertexBuffer(vbdesc);
	RENDERER_ASSERT(m_vertexBuffer, "Failed to create Vertex Buffer.");
	if(m_vertexBuffer)
	{
		physx::PxU32 positionStride = 0;
		void *vertPositions = m_vertexBuffer->lockSemantic(RendererVertexBuffer::SEMANTIC_POSITION, positionStride);
		physx::PxU32 normalStride = 0;
		void *vertNormals = m_vertexBuffer->lockSemantic(RendererVertexBuffer::SEMANTIC_NORMAL, normalStride);
		physx::PxU32 uvStride = 0;
		void *vertUVs = m_vertexBuffer->lockSemantic(RendererVertexBuffer::SEMANTIC_TEXCOORD0, uvStride);
		if(vertPositions && vertNormals)
		{
			for(physx::PxU32 i=0; i<numVerts; i++)
			{
				memcpy(vertPositions, verts+i, sizeof(physx::PxVec3));
				memcpy(vertNormals, normals+i, sizeof(physx::PxVec3));
				((physx::PxF32*)vertUVs)[0] = verts[i].x * uvscale;
				((physx::PxF32*)vertUVs)[1] = verts[i].z * uvscale;

				vertPositions = (void*)(((physx::PxU8*)vertPositions) + positionStride);
				vertNormals   = (void*)(((physx::PxU8*)vertNormals)   + normalStride);
				vertUVs       = (void*)(((physx::PxU8*)vertUVs)       + uvStride);
			}
		}
		m_vertexBuffer->unlockSemantic(RendererVertexBuffer::SEMANTIC_NORMAL);
		m_vertexBuffer->unlockSemantic(RendererVertexBuffer::SEMANTIC_POSITION);
		m_vertexBuffer->unlockSemantic(RendererVertexBuffer::SEMANTIC_TEXCOORD0);
	}

	physx::PxU32 numIndices = numFaces*3;
	
	RendererIndexBufferDesc ibdesc;
	ibdesc.hint       = RendererIndexBuffer::HINT_STATIC;
	ibdesc.format     = RendererIndexBuffer::FORMAT_UINT16;
	ibdesc.maxIndices = numIndices;
	m_indexBuffer = m_renderer.createIndexBuffer(ibdesc);
	RENDERER_ASSERT(m_indexBuffer, "Failed to create Index Buffer.");
	if(m_indexBuffer)
	{
		physx::PxU16 *indices = (physx::PxU16*)m_indexBuffer->lock();
		if(indices)
		{
			memcpy(indices, faces, sizeof(*faces)*numFaces*3);
		}
		m_indexBuffer->unlock();
	}
	
	if(m_vertexBuffer && m_indexBuffer)
	{
		RendererMeshDesc meshdesc;
		meshdesc.primitives       = RendererMesh::PRIMITIVE_TRIANGLES;
		meshdesc.vertexBuffers    = &m_vertexBuffer;
		meshdesc.numVertexBuffers = 1;
		meshdesc.firstVertex      = 0;
		meshdesc.numVertices      = numVerts;
		meshdesc.indexBuffer      = m_indexBuffer;
		meshdesc.firstIndex       = 0;
		meshdesc.numIndices       = numIndices;
		m_mesh = m_renderer.createMesh(meshdesc);
		RENDERER_ASSERT(m_mesh, "Failed to create Mesh.");
	}
}

RendererTerrainShape::~RendererTerrainShape(void)
{
	if(m_vertexBuffer) m_vertexBuffer->release();
	if(m_indexBuffer)  m_indexBuffer->release();
	if(m_mesh)
	{
		m_mesh->release();
		m_mesh = 0;
	}
}
