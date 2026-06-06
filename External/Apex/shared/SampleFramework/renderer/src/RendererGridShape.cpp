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
// RendererGridShape : convenience class for generating a grid mesh.
//
#include <RendererGridShape.h>

#include <Renderer.h>

#include <RendererVertexBuffer.h>
#include <RendererVertexBufferDesc.h>

#include <RendererMesh.h>
#include <RendererMeshDesc.h>

RendererGridShape::RendererGridShape(Renderer &renderer, physx::PxU32 size, float cellSize) :
	RendererShape(renderer)
{
	m_vertexBuffer = 0;

	const physx::PxU32 numColLines = size*2 + 1;
	const physx::PxU32 numLines    = numColLines*2;
	const physx::PxU32 numVerts    = numLines*2;
	
	RendererVertexBufferDesc vbdesc;
	vbdesc.hint                                                     = RendererVertexBuffer::HINT_STATIC;
	vbdesc.semanticFormats[RendererVertexBuffer::SEMANTIC_POSITION] = RendererVertexBuffer::FORMAT_FLOAT3;
	vbdesc.semanticFormats[RendererVertexBuffer::SEMANTIC_COLOR]    = RendererVertexBuffer::FORMAT_COLOR;
	vbdesc.maxVertices                                              = numVerts;
	m_vertexBuffer = m_renderer.createVertexBuffer(vbdesc);
	if(m_vertexBuffer)
	{
		RendererMeshDesc meshdesc;
		meshdesc.primitives       = RendererMesh::PRIMITIVE_LINES;
		meshdesc.vertexBuffers    = &m_vertexBuffer;
		meshdesc.numVertexBuffers = 1;
		meshdesc.firstVertex      = 0;
		meshdesc.numVertices      = numVerts;
		m_mesh = m_renderer.createMesh(meshdesc);
	}
	if(m_vertexBuffer && m_mesh)
	{
		physx::PxU32 color2 = 0xFF788AA3;
		physx::PxU32 color1 = 0xFF5E6C7F;

		physx::PxU32 positionStride = 0;
		void *positions = m_vertexBuffer->lockSemantic(RendererVertexBuffer::SEMANTIC_POSITION, positionStride);
		physx::PxU32 colorStride = 0;
		void *colors = m_vertexBuffer->lockSemantic(RendererVertexBuffer::SEMANTIC_COLOR, colorStride);
		if(positions && colors)
		{
			const float radius   = size*cellSize;
			const float diameter = radius*2;
			for(physx::PxU32 c=0; c<numColLines; c++)
			{
				float *p0 = (float*)positions;
				physx::PxU8  *c0 = (physx::PxU8*)colors;
				positions = ((physx::PxU8*)positions)+positionStride;
				colors    = ((physx::PxU8*)colors)+colorStride;
				float *p1 = (float*)positions;
				physx::PxU8  *c1 = (physx::PxU8*)colors;
				positions = ((physx::PxU8*)positions)+positionStride;
				colors    = ((physx::PxU8*)colors)+colorStride;
				
				const float t = c / (float)(numColLines-1);
				const float d = diameter*t - radius;

				physx::PxU32 *col1 = (physx::PxU32 *)c0;
				physx::PxU32 *col2 = (physx::PxU32 *)c1;
				if(d==0.0f) 
				{
					*col1 = color1;
					*col2 = color1;
				}
				else        
				{
					*col1 = color2;
					*col2 = color2;
				}
				p0[0] = -radius; p0[1] = 0; p0[2] = d;
				p1[0] =  radius; p1[1] = 0; p1[2] = d;
			}
			for(physx::PxU32 r=0; r<numColLines; r++)
			{
				float *p0 = (float*)positions;
				physx::PxU8  *c0 = (physx::PxU8*)colors;
				positions = ((physx::PxU8*)positions)+positionStride;
				colors    = ((physx::PxU8*)colors)+colorStride;
				float *p1 = (float*)positions;
				physx::PxU8  *c1 = (physx::PxU8*)colors;
				positions = ((physx::PxU8*)positions)+positionStride;
				colors    = ((physx::PxU8*)colors)+colorStride;
				
				const float t = r / (float)(numColLines-1);
				const float d = diameter*t - radius;
				physx::PxU32 *col1 = (physx::PxU32 *)c0;
				physx::PxU32 *col2 = (physx::PxU32 *)c1;
				if(d==0.0f) 
				{
					*col1 = color1;
					*col2 = color1;
				}
				else        
				{
					*col1 = color2;
					*col2 = color2;
				}

				p0[0] = d; p0[1] = 0; p0[2] = -radius;
				p1[0] = d; p1[1] = 0; p1[2] =  radius;
			}
		}
		m_vertexBuffer->unlockSemantic(RendererVertexBuffer::SEMANTIC_COLOR);
		m_vertexBuffer->unlockSemantic(RendererVertexBuffer::SEMANTIC_POSITION);
	}
}

RendererGridShape::~RendererGridShape(void)
{
	if(m_vertexBuffer) m_vertexBuffer->release();
	if(m_mesh)
	{
		m_mesh->release();
		m_mesh = 0;
	}
}
