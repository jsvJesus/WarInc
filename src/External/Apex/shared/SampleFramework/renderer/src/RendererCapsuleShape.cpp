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
// RendererCapsuleShape : convenience class for generating a capsule mesh.
//
#include "PsShare.h"
#include <RendererCapsuleShape.h>

#include <Renderer.h>

#include <RendererVertexBuffer.h>
#include <RendererVertexBufferDesc.h>

#include <RendererIndexBuffer.h>
#include <RendererIndexBufferDesc.h>

#include <RendererMesh.h>
#include <RendererMeshDesc.h>

#include "PxVec3.h"


class CapsuleVertex
{
	public:
		CapsuleVertex(const physx::PxVec3 &position, const physx::PxVec3 &normal)
		{
			p = position;
			n = normal;
			n.normalize();
		}
	
	public:
		physx::PxVec3 p;
		physx::PxVec3 n;
};

static const CapsuleVertex g_vertices[] =
{
	CapsuleVertex( physx::PxVec3(0.0000f, -2.0000f, -0.0000f), 	physx::PxVec3(0.0000f, -1.0000f, -0.0000f)),
	CapsuleVertex( physx::PxVec3(0.3827f, -1.9239f, -0.0000f), 	physx::PxVec3(0.4226f, -0.9063f, -0.0000f)),
	CapsuleVertex( physx::PxVec3(0.2706f, -1.9239f, 0.2706f), 		physx::PxVec3(0.2988f, -0.9063f, 0.2988f)),
	CapsuleVertex( physx::PxVec3(-0.0000f, -1.9239f, 0.3827f), 	physx::PxVec3(0.0000f, -0.9063f, 0.4226f)),
	CapsuleVertex( physx::PxVec3(-0.2706f, -1.9239f, 0.2706f), 	physx::PxVec3(-0.2988f, -0.9063f, 0.2988f)),
	CapsuleVertex( physx::PxVec3(-0.3827f, -1.9239f, -0.0000f), 	physx::PxVec3(-0.4226f, -0.9063f, -0.0000f)),
	CapsuleVertex( physx::PxVec3(-0.2706f, -1.9239f, -0.2706f), 	physx::PxVec3(-0.2988f, -0.9063f, -0.2988f)),
	CapsuleVertex( physx::PxVec3(0.0000f, -1.9239f, -0.3827f), 	physx::PxVec3(-0.0000f, -0.9063f, -0.4226f)),
	CapsuleVertex( physx::PxVec3(0.2706f, -1.9239f, -0.2706f), 	physx::PxVec3(0.2988f, -0.9063f, -0.2988f)),
	CapsuleVertex( physx::PxVec3(0.7071f, -1.7071f, -0.0000f), 	physx::PxVec3(0.7296f, -0.6839f, -0.0000f)),
	CapsuleVertex( physx::PxVec3(0.5000f, -1.7071f, 0.5000f), 		physx::PxVec3(0.5159f, -0.6839f, 0.5159f)),
	CapsuleVertex( physx::PxVec3(-0.0000f, -1.7071f, 0.7071f), 	physx::PxVec3(-0.0000f, -0.6839f, 0.7296f)),
	CapsuleVertex( physx::PxVec3(-0.5000f, -1.7071f, 0.5000f), 	physx::PxVec3(-0.5159f, -0.6839f, 0.5159f)),
	CapsuleVertex( physx::PxVec3(-0.7071f, -1.7071f, -0.0000f), 	physx::PxVec3(-0.7296f, -0.6839f, -0.0000f)),
	CapsuleVertex( physx::PxVec3(-0.5000f, -1.7071f, -0.5000f), 	physx::PxVec3(-0.5159f, -0.6839f, -0.5159f)),
	CapsuleVertex( physx::PxVec3(0.0000f, -1.7071f, -0.7071f), 	physx::PxVec3(-0.0000f, -0.6839f, -0.7296f)),
	CapsuleVertex( physx::PxVec3(0.5000f, -1.7071f, -0.5000f), 	physx::PxVec3(0.5159f, -0.6839f, -0.5159f)),
	CapsuleVertex( physx::PxVec3(0.9239f, -1.3827f, -0.0000f), 	physx::PxVec3(0.9303f, -0.3668f, -0.0000f)),
	CapsuleVertex( physx::PxVec3(0.6533f, -1.3827f, 0.6533f), 		physx::PxVec3(0.6578f, -0.3668f, 0.6578f)),
	CapsuleVertex( physx::PxVec3(-0.0000f, -1.3827f, 0.9239f), 	physx::PxVec3(0.0000f, -0.3668f, 0.9303f)),
	CapsuleVertex( physx::PxVec3(-0.6533f, -1.3827f, 0.6533f), 	physx::PxVec3(-0.6578f, -0.3668f, 0.6578f)),
	CapsuleVertex( physx::PxVec3(-0.9239f, -1.3827f, -0.0000f), 	physx::PxVec3(-0.9303f, -0.3668f, -0.0000f)),
	CapsuleVertex( physx::PxVec3(-0.6533f, -1.3827f, -0.6533f), 	physx::PxVec3(-0.6578f, -0.3668f, -0.6578f)),
	CapsuleVertex( physx::PxVec3(0.0000f, -1.3827f, -0.9239f), 	physx::PxVec3(-0.0000f, -0.3668f, -0.9303f)),
	CapsuleVertex( physx::PxVec3(0.6533f, -1.3827f, -0.6533f), 	physx::PxVec3(0.6578f, -0.3668f, -0.6578f)),
	CapsuleVertex( physx::PxVec3(1.0000f, -1.0000f, -0.0000f), 	physx::PxVec3(0.9954f, -0.0958f, -0.0000f)),
	CapsuleVertex( physx::PxVec3(0.7071f, -1.0000f, 0.7071f), 		physx::PxVec3(0.7039f, -0.0958f, 0.7039f)),
	CapsuleVertex( physx::PxVec3(-0.0000f, -1.0000f, 1.0000f), 	physx::PxVec3(-0.0000f, -0.0958f, 0.9954f)),
	CapsuleVertex( physx::PxVec3(-0.7071f, -1.0000f, 0.7071f), 	physx::PxVec3(-0.7039f, -0.0958f, 0.7039f)),
	CapsuleVertex( physx::PxVec3(-1.0000f, -1.0000f, -0.0000f), 	physx::PxVec3(-0.9954f, -0.0958f, -0.0000f)),
	CapsuleVertex( physx::PxVec3(-0.7071f, -1.0000f, -0.7071f), 	physx::PxVec3(-0.7039f, -0.0958f, -0.7039f)),
	CapsuleVertex( physx::PxVec3(0.0000f, -1.0000f, -1.0000f), 	physx::PxVec3(-0.0000f, -0.0958f, -0.9954f)),
	CapsuleVertex( physx::PxVec3(0.7071f, -1.0000f, -0.7071f), 	physx::PxVec3(0.7039f, -0.0958f, -0.7039f)),
	CapsuleVertex( physx::PxVec3(1.0000f, 1.0000f, 0.0000f), 		physx::PxVec3(0.9954f, 0.0958f, -0.0000f)),
	CapsuleVertex( physx::PxVec3(0.7071f, 1.0000f, 0.7071f), 		physx::PxVec3(0.7039f, 0.0958f, 0.7039f)),
	CapsuleVertex( physx::PxVec3(-0.0000f, 1.0000f, 1.0000f), 		physx::PxVec3(-0.0000f, 0.0958f, 0.9954f)),
	CapsuleVertex( physx::PxVec3(-0.7071f, 1.0000f, 0.7071f), 		physx::PxVec3(-0.7039f, 0.0958f, 0.7039f)),
	CapsuleVertex( physx::PxVec3(-1.0000f, 1.0000f, -0.0000f),		physx::PxVec3(-0.9954f, 0.0958f, -0.0000f)),
	CapsuleVertex( physx::PxVec3(-0.7071f, 1.0000f, -0.7071f),		physx::PxVec3(-0.7039f, 0.0958f, -0.7039f)),
	CapsuleVertex( physx::PxVec3(0.0000f, 1.0000f, -1.0000f), 		physx::PxVec3(-0.0000f, 0.0958f, -0.9954f)),
	CapsuleVertex( physx::PxVec3(0.7071f, 1.0000f, -0.7071f), 		physx::PxVec3(0.7039f, 0.0958f, -0.7039f)),
	CapsuleVertex( physx::PxVec3(0.9239f, 1.3827f, 0.0000f), 		physx::PxVec3(0.9303f, 0.3668f, -0.0000f)),
	CapsuleVertex( physx::PxVec3(0.6533f, 1.3827f, 0.6533f), 		physx::PxVec3(0.6578f, 0.3668f, 0.6578f)),
	CapsuleVertex( physx::PxVec3(-0.0000f, 1.3827f, 0.9239f), 		physx::PxVec3(-0.0000f, 0.3668f, 0.9303f)),
	CapsuleVertex( physx::PxVec3(-0.6533f, 1.3827f, 0.6533f), 		physx::PxVec3(-0.6578f, 0.3668f, 0.6578f)),
	CapsuleVertex( physx::PxVec3(-0.9239f, 1.3827f, -0.0000f),		physx::PxVec3(-0.9303f, 0.3668f, -0.0000f)),
	CapsuleVertex( physx::PxVec3(-0.6533f, 1.3827f, -0.6533f),		physx::PxVec3(-0.6578f, 0.3668f, -0.6578f)),
	CapsuleVertex( physx::PxVec3(0.0000f, 1.3827f, -0.9239f), 		physx::PxVec3(-0.0000f, 0.3668f, -0.9303f)),
	CapsuleVertex( physx::PxVec3(0.6533f, 1.3827f, -0.6533f), 		physx::PxVec3(0.6578f, 0.3668f, -0.6578f)),
	CapsuleVertex( physx::PxVec3(0.7071f, 1.7071f, 0.0000f), 		physx::PxVec3(0.7296f, 0.6839f, -0.0000f)),
	CapsuleVertex( physx::PxVec3(0.5000f, 1.7071f, 0.5000f), 		physx::PxVec3(0.5159f, 0.6839f, 0.5159f)),
	CapsuleVertex( physx::PxVec3(-0.0000f, 1.7071f, 0.7071f), 		physx::PxVec3(0.0000f, 0.6839f, 0.7296f)),
	CapsuleVertex( physx::PxVec3(-0.5000f, 1.7071f, 0.5000f), 		physx::PxVec3(-0.5159f, 0.6839f, 0.5159f)),
	CapsuleVertex( physx::PxVec3(-0.7071f, 1.7071f, 0.0000f), 		physx::PxVec3(-0.7296f, 0.6839f, -0.0000f)),
	CapsuleVertex( physx::PxVec3(-0.5000f, 1.7071f, -0.5000f),		physx::PxVec3(-0.5159f, 0.6839f, -0.5159f)),
	CapsuleVertex( physx::PxVec3(0.0000f, 1.7071f, -0.7071f), 		physx::PxVec3(0.0000f, 0.6839f, -0.7296f)),
	CapsuleVertex( physx::PxVec3(0.5000f, 1.7071f, -0.5000f), 		physx::PxVec3(0.5159f, 0.6839f, -0.5159f)),
	CapsuleVertex( physx::PxVec3(0.3827f, 1.9239f, 0.0000f), 		physx::PxVec3(0.4226f, 0.9063f, 0.0000f)),
	CapsuleVertex( physx::PxVec3(0.2706f, 1.9239f, 0.2706f), 		physx::PxVec3(0.2988f, 0.9063f, 0.2988f)),
	CapsuleVertex( physx::PxVec3(-0.0000f, 1.9239f, 0.3827f), 		physx::PxVec3(-0.0000f, 0.9063f, 0.4226f)),
	CapsuleVertex( physx::PxVec3(-0.2706f, 1.9239f, 0.2706f), 		physx::PxVec3(-0.2988f, 0.9063f, 0.2988f)),
	CapsuleVertex( physx::PxVec3(-0.3827f, 1.9239f, 0.0000f), 		physx::PxVec3(-0.4226f, 0.9063f, 0.0000f)),
	CapsuleVertex( physx::PxVec3(-0.2706f, 1.9239f, -0.2706f),		physx::PxVec3(-0.2988f, 0.9063f, -0.2988f)),
	CapsuleVertex( physx::PxVec3(0.0000f, 1.9239f, -0.3827f), 		physx::PxVec3(0.0000f, 0.9063f, -0.4226f)),
	CapsuleVertex( physx::PxVec3(0.2706f, 1.9239f, -0.2706f), 		physx::PxVec3(0.2988f, 0.9063f, -0.2988f)),
	CapsuleVertex( physx::PxVec3(0.0000f, 2.0000f, 0.0000f), 		physx::PxVec3(0.0000f, 1.0000f, 0.0000f)),
};
static const physx::PxU32 g_numVertices = sizeof(g_vertices) / sizeof(g_vertices[0]);

static const physx::PxU16 g_indices[] =
{
	1, 0, 2,  2, 0, 3,  3, 0, 4,  4, 0, 5,  5, 0, 6,  6, 0, 7,  7, 0, 8,
	8, 0, 1,  9, 1, 10,  10, 1, 2,  10, 2, 11,  11, 2, 3,  11, 3, 12,
	12, 3, 4,  12, 4, 13,  13, 4, 5,  13, 5, 14,  14, 5, 6,  14, 6, 15,
	15, 6, 7,  15, 7, 16,  16, 7, 8,  16, 8, 9,  9, 8, 1,  17, 9, 18,
	18, 9, 10,  18, 10, 19,  19, 10, 11,  19, 11, 20,  20, 11, 12,  20, 12, 21,
	21, 12, 13,  21, 13, 22,  22, 13, 14,  22, 14, 23,  23, 14, 15,  23, 15, 24,
	24, 15, 16,  24, 16, 17,  17, 16, 9,  25, 17, 26,  26, 17, 18,  26, 18, 27,
	27, 18, 19,  27, 19, 28,  28, 19, 20,  28, 20, 29,  29, 20, 21,  29, 21, 30,
	30, 21, 22,  30, 22, 31,  31, 22, 23,  31, 23, 32,  32, 23, 24,  32, 24, 25,
	25, 24, 17,  33, 25, 34,  34, 25, 26,  34, 26, 35,  35, 26, 27,  35, 27, 36,
	36, 27, 28,  36, 28, 37,  37, 28, 29,  37, 29, 38,  38, 29, 30,  38, 30, 39,
	39, 30, 31,  39, 31, 40,  40, 31, 32,  40, 32, 33,  33, 32, 25,  41, 33, 42,
	42, 33, 34,  42, 34, 43,  43, 34, 35,  43, 35, 44,  44, 35, 36,  44, 36, 45,
	45, 36, 37,  45, 37, 46,  46, 37, 38,  46, 38, 47,  47, 38, 39,  47, 39, 48,
	48, 39, 40,  48, 40, 41,  41, 40, 33,  49, 41, 50,  50, 41, 42,  50, 42, 51,
	51, 42, 43,  51, 43, 52,  52, 43, 44,  52, 44, 53,  53, 44, 45,  53, 45, 54,
	54, 45, 46,  54, 46, 55,  55, 46, 47,  55, 47, 56,  56, 47, 48,  56, 48, 49,
	49, 48, 41,  57, 49, 58,  58, 49, 50,  58, 50, 59,  59, 50, 51,  59, 51, 60,
	60, 51, 52,  60, 52, 61,  61, 52, 53,  61, 53, 62,  62, 53, 54,  62, 54, 63,
	63, 54, 55,  63, 55, 64,  64, 55, 56,  64, 56, 57,  57, 56, 49,  65, 57, 58,
	65, 58, 59,  65, 59, 60,  65, 60, 61,  65, 61, 62,  65, 62, 63,  65, 63, 64,
	65, 64, 57,  
};
static const physx::PxU32 g_numIndices = sizeof(g_indices) / sizeof(g_indices[0]);


RendererCapsuleShape::RendererCapsuleShape(Renderer &renderer, physx::PxF32 halfHeight, physx::PxF32 radius) :
	RendererShape(renderer)
{
	RendererVertexBufferDesc vbdesc;
	vbdesc.hint = RendererVertexBuffer::HINT_STATIC;
	vbdesc.semanticFormats[RendererVertexBuffer::SEMANTIC_POSITION] = RendererVertexBuffer::FORMAT_FLOAT3;
	vbdesc.semanticFormats[RendererVertexBuffer::SEMANTIC_NORMAL]   = RendererVertexBuffer::FORMAT_FLOAT3;
	vbdesc.maxVertices = g_numVertices;
	m_vertexBuffer = m_renderer.createVertexBuffer(vbdesc);
	RENDERER_ASSERT(m_vertexBuffer, "Failed to create Vertex Buffer.");
	if(m_vertexBuffer)
	{
		physx::PxU32 positionStride = 0;
		void *positions = m_vertexBuffer->lockSemantic(RendererVertexBuffer::SEMANTIC_POSITION, positionStride);
		physx::PxU32 normalStride = 0;
		void *normals = m_vertexBuffer->lockSemantic(RendererVertexBuffer::SEMANTIC_NORMAL, normalStride);
		if(positions && normals)
		{
			for(physx::PxU32 i=0; i<g_numVertices; i++)
			{
				const CapsuleVertex &v = g_vertices[i];
				
				physx::PxVec3 &p = *(physx::PxVec3*)positions;
				physx::PxVec3 &n = *(physx::PxVec3*)normals;
				
				p = v.p;
				n = v.n;
				
				const physx::PxF32 sign = p.y > 0 ? 1.0f : -1.0f;
				p.y -= sign;
				p   *= radius;
				p.y += halfHeight*sign;
				
				positions = ((physx::PxU8*)positions) + positionStride;
				normals =   ((physx::PxU8*)normals)   + normalStride;
			}
		}
		m_vertexBuffer->unlockSemantic(RendererVertexBuffer::SEMANTIC_NORMAL);
		m_vertexBuffer->unlockSemantic(RendererVertexBuffer::SEMANTIC_POSITION);
	}
	
	RendererIndexBufferDesc ibdesc;
	ibdesc.hint       = RendererIndexBuffer::HINT_STATIC;
	ibdesc.format     = RendererIndexBuffer::FORMAT_UINT16;
	ibdesc.maxIndices = g_numIndices;
	m_indexBuffer = m_renderer.createIndexBuffer(ibdesc);
	RENDERER_ASSERT(m_indexBuffer, "Failed to create Index Buffer.");
	if(m_indexBuffer)
	{
		physx::PxU16 *indices = (physx::PxU16*)m_indexBuffer->lock();
		if(indices)
		{
			for(physx::PxU32 i=0; i<g_numIndices; i++)
			{
				indices[i] = g_indices[i];
			}
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
		meshdesc.numVertices      = g_numVertices;
		meshdesc.indexBuffer      = m_indexBuffer;
		meshdesc.firstIndex       = 0;
		meshdesc.numIndices       = g_numIndices;
		m_mesh = m_renderer.createMesh(meshdesc);
		RENDERER_ASSERT(m_mesh, "Failed to create Mesh.");
	}
}

RendererCapsuleShape::~RendererCapsuleShape(void)
{
	if(m_vertexBuffer) m_vertexBuffer->release();
	if(m_indexBuffer)  m_indexBuffer->release();
	if(m_mesh)
	{
		m_mesh->release();
		m_mesh = 0;
	}
}
