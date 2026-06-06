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
#if 0  // SJB: Disabled, until we resolve what's going to happen with this.
       // 3.0 PhysX does not have this class, near as I can tell.

#include <SamplePhysXDebugRender.h>
#include <RendererColor.h>

#include <NxDebugRenderable.h>

SamplePhysXDebugRender::SamplePhysXDebugRender(Renderer &renderer, SampleAssetManager &assetmanager) :
	SamplePointDebugRender(renderer, assetmanager),
	SampleLineDebugRender(renderer, assetmanager),
	SampleTriangleDebugRender(renderer, assetmanager)
{
	
}

SamplePhysXDebugRender::~SamplePhysXDebugRender(void)
{
	
}

void SamplePhysXDebugRender::update(const NxDebugRenderable &debugRenderable)
{
	// Points
	const physx::PxU32         numPoints = debugRenderable.getNbPoints();
	const NxDebugPoint *points    = debugRenderable.getPoints();
	checkResizePoint(numPoints);
	for(physx::PxU32 i=0; i<numPoints; i++)
	{
		const NxDebugPoint &point = points[i];
		addPoint(point.p, RendererColor(point.color));
	}

	// Lines
	const physx::PxU32        numLines = debugRenderable.getNbLines();
	const NxDebugLine *lines    = debugRenderable.getLines();
	checkResizeLine(numLines * 2);
	for(physx::PxU32 i=0; i<numLines; i++)
	{
		const NxDebugLine &line = lines[i];
		addLine(line.p0, line.p1, RendererColor(line.color));
	}

	// Triangles
	const physx::PxU32            numTriangles = debugRenderable.getNbTriangles();
	const NxDebugTriangle *triangles    = debugRenderable.getTriangles();
	checkResizeTriangle(numTriangles * 3);
	for(physx::PxU32 i=0; i<numTriangles; i++)
	{
		const NxDebugTriangle &triangle = triangles[i];
		addTriangle(triangle.p0, triangle.p1, triangle.p2, RendererColor(triangle.color));
	}
}

void SamplePhysXDebugRender::queueForRender(void)
{
	queueForRenderPoint();
	queueForRenderLine();
	queueForRenderTriangle();
}

void SamplePhysXDebugRender::clear(void)
{
	clearPoint();
	clearLine();
	clearTriangle();
}

#endif