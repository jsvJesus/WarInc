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
#include "authoring/ApexCSGDefs.h"
#include "PxUserOutputStream.h"

#ifndef WITHOUT_APEX_AUTHORING

#define DEBUG_OUTPUT	0

#pragma warning(disable:4505)

namespace physx {
	namespace apex
	{

PX_INLINE Real
distanceToLine2D( const Vec2Real& point, const Vec2Real& lineOrig, const Vec2Real& lineDir )	// lineDir must be normalized
{
	const Vec2Real disp = point - lineOrig;
	return ( disp|disp ) - square( disp|lineDir );
}

PX_INLINE bool
diagonalIsValidInLoop( const LinkedEdge2D* edge, const LinkedEdge2D* otherEdge, const LinkedEdge2D* loop )
{
	const Vec2Real& diagonalStart = edge->v[0];
	const Vec2Real& diagonalEnd = otherEdge->v[0];
	const Vec2Real diagonalDisp = diagonalEnd-diagonalStart;

	const LinkedEdge2D* loopEdge = loop;
	LinkedEdge2D* loopEdgeNext = loopEdge->getAdj(1);
	do
	{
		loopEdgeNext = loopEdge->getAdj(1);
		if( loopEdge == edge || loopEdgeNext == edge || loopEdge == otherEdge || loopEdgeNext == otherEdge )
		{
			continue;
		}
		const Vec2Real& loopEdgeStart = loopEdge->v[0];
		const Vec2Real& loopEdgeEnd = loopEdge->v[1];
		const Vec2Real& loopEdgeDisp = loopEdgeEnd-loopEdgeStart;
		if( (loopEdgeDisp^(diagonalStart-loopEdgeStart))*(loopEdgeDisp^(diagonalEnd-loopEdgeStart)) <= 0 &&
			(diagonalDisp^(loopEdgeStart-diagonalStart))*(diagonalDisp^(loopEdgeEnd-diagonalStart)) <= 0 )
		{	// Edge intersection
			return false;
		}
	} while( (loopEdge = loopEdgeNext) != loop );

	return true;
}

PX_INLINE bool
diagonalIsValid( const LinkedEdge2D* edge, const LinkedEdge2D* otherEdge, const physx::Array<LinkedEdge2D*>& loops )
{
	for( physx::PxU32 i = 0; i < loops.size(); ++i )
	{
		if( !diagonalIsValidInLoop( edge, otherEdge, loops[i] ) )
		{
			return false;
		}
	}

	return true;
}

PX_INLINE void
addTriangleVertices( physx::Array<Vec2Real>& planeVertices, LinkedEdge2D* triangle, Pool<LinkedEdge2D>& linkedEdgePool )
{
	planeVertices.pushBack( triangle->v[0] );
	LinkedEdge2D* next1 = triangle->getAdj(1);
	planeVertices.pushBack( next1->v[0] );
	LinkedEdge2D* next2 = next1->getAdj(1);
	planeVertices.pushBack( next2->v[0] );
	linkedEdgePool.replace( triangle );
	linkedEdgePool.replace( next1 );
	linkedEdgePool.replace( next2 );
}

PX_INLINE bool
diagonalInternalAtVertex( const Vec2Real& disp, const LinkedEdge2D* edge )
{
	const LinkedEdge2D* prev = edge->getAdj(0);
	const Vec2Real prevDisp = prev->v[1]-prev->v[0];
	const Vec2Real edgeDisp = edge->v[1]-edge->v[0];
	return  (prevDisp^edgeDisp) > 0 ? ((prevDisp^disp) > 0 && (edgeDisp^disp) > 0) : ((prevDisp^disp) > 0 || (edgeDisp^disp) > 0);
}

static LinkedEdge2D*
splitWithDiagonal( LinkedEdge2D*& loop, Pool<LinkedEdge2D>& linkedEdgePool )
{
	LinkedEdge2D* bestEdge = NULL;
	LinkedEdge2D* bestOtherEdge = NULL;
	Real minDist2 = MAX_REAL;
	LinkedEdge2D* edge = loop;
	bool validDiagonalFound = false;
	do
	{
		LinkedEdge2D* otherEdge = edge->getAdj(1)->getAdj(1);
		LinkedEdge2D* otherEdgeStop = edge->getAdj(0)->getAdj(0);
		do
		{
			const Vec2Real& start = edge->v[0];
			const Vec2Real& end = otherEdge->v[0];
			if( diagonalInternalAtVertex( end-start, edge ) && diagonalInternalAtVertex( start-end, otherEdge ) )
			{
				const bool diagonalValid = diagonalIsValidInLoop( edge, otherEdge, loop );
				const Real dist2 = (end-start).lengthSquared();
				if( (!validDiagonalFound && (dist2 < minDist2 || diagonalValid)) || (diagonalValid && dist2 < minDist2) )
				{
					bestEdge = edge;
					bestOtherEdge = otherEdge;
					minDist2 = dist2;
				}
				validDiagonalFound = validDiagonalFound || diagonalValid;
			}
		} while( (otherEdge = otherEdge->getAdj(1)) != otherEdgeStop );
	} while( (edge = edge->getAdj(1)) != loop );

	if( bestEdge == NULL )
	{
#if 0
		char filename[100];
		static int filenum = 0;
		sprintf( filename, "poly%03d.txt", filenum++ );
		FILE* fp = fopen( filename, "w" );
		LinkedEdge2D* edge = loop;
		do
		{
			PX_ASSERT( ( edge->v[1] - edge->getAdj(1)->v[0] ).lengthSquared() < 1.0e-12 );
			fprintf( fp, "%f, %f\n", edge->v[0][0], edge->v[0][1] );
			edge = edge->getAdj(1);
		} while( edge != loop );
		fclose( fp );
#endif
		return NULL;
	}

	// Create diagonal and split loop
	LinkedEdge2D* diagonal = linkedEdgePool.borrow();
	LinkedEdge2D* reciprocal = linkedEdgePool.borrow();
	diagonal->setAdj( 1, reciprocal );
	diagonal->v[1] = reciprocal->v[0] = bestEdge->v[0];
	diagonal->v[0] = reciprocal->v[1] = bestOtherEdge->v[0];

	diagonal->setAdj( 1, bestEdge );
	reciprocal->setAdj( 1, bestOtherEdge );

	loop = diagonal;
	return reciprocal;
}

static bool
mergeTriangles2D( physx::Array<Vec2Real>& planeVertices, Pool<LinkedEdge2D>& linkedEdgePool, Real distanceTol )
{
	// Create a set of linked edges for each triangle. The initial set will consist of nothing but three-edge loops (the triangles). 
	physx::Array<LinkedEdge2D*> edges;
	edges.reserve( planeVertices.size() );
	PX_ASSERT( (planeVertices.size()%3) == 0 );
	for( physx::PxU32 i = 0; i < planeVertices.size(); i += 3 )
	{
		LinkedEdge2D* v0 = linkedEdgePool.borrow();
		edges.pushBack( v0 );
		LinkedEdge2D* v1 = linkedEdgePool.borrow();
		edges.pushBack( v1 );
		LinkedEdge2D* v2 = linkedEdgePool.borrow();
		edges.pushBack( v2 );
		v0->setAdj( 1, v1 );
		v1->setAdj( 1, v2 );
		v0->v[0] = planeVertices[i];
		v0->v[1] = planeVertices[i+1];
		v1->v[0] = planeVertices[i+1];
		v1->v[1] = planeVertices[i+2];
		v2->v[0] = planeVertices[i+2];
		v2->v[1] = planeVertices[i];
	}

	// Find all edge overlaps. 
	for( physx::PxU32 i = 0; i < edges.size(); ++i )
	{
		LinkedEdge2D* edge = edges[i];
		if( edge == NULL )
		{
			continue;
		}
		Vec2Real edgeDir = edge->v[1] - edge->v[0];
		const Real edgeLen = edgeDir.normalize();
		if( edgeLen < distanceTol )
		{	// Edge is too short.  Remove.
			edge->getAdj(0)->v[1] = edge->getAdj(1)->v[0] = (Real)0.5*(edge->getAdj(0)->v[1] + edge->getAdj(1)->v[0]);
			edge->remove();
			linkedEdgePool.replace( edge );
			edges[i] = NULL;
			continue;
		}
		for( physx::PxU32 j = i+1; j < edges.size(); ++j )
		{
			LinkedEdge2D* otherEdge = edges[j];
			if( otherEdge == NULL )
			{
				continue;
			}
			if( distanceToLine2D( otherEdge->v[0], edge->v[0], edgeDir) > distanceTol || distanceToLine2D( otherEdge->v[1], edge->v[0], edgeDir) > distanceTol )
			{
				continue;	// Not colinear
			}
			const Vec2Real otherEdgeDisp = otherEdge->v[1] - otherEdge->v[0];
			if( (otherEdgeDisp | edgeDir) > 0 )
			{
				continue;	// Pointing the same direction
			}
			// Use coordinate system based off of edges[i]'s origin and direction
			const Real origJ = (otherEdge->v[0]-edge->v[0]) | edgeDir;
			const Real destJ = (otherEdge->v[1]-edge->v[0]) | edgeDir;
			if( origJ <= 0 || destJ >= edgeLen )
			{
				continue;	// Edges don't overlap
			}
			// Found cancelling portions of edges
			edge->setAdj( 0, otherEdge->getAdj(0) );
		}
	}

	const Real distanceTol2 = distanceTol*distanceTol;

	// Clean further by removing adjacent colinear edges.  Also label loops.
	physx::Array<LinkedEdge2D*> loops;
	physx::PxI32 loopID = 0;
	for( physx::PxU32 i = edges.size(); i--; )
	{
		LinkedEdge2D* edge = edges[i];
		if( edge == NULL || edge->isSolitary() )
		{
			if( edge != NULL )
			{
				linkedEdgePool.replace( edge );
			}
			edges.replaceWithLast(i);
			continue;
		}
		if( edge->loopID < 0 )
		{
			do
			{
				edge->loopID = loopID;
				LinkedEdge2D* prev = edge->getAdj(0);
				LinkedEdge2D* next = edge->getAdj(1);
				const Vec2Real edgeDisp = edge->v[1] - edge->v[0];
				const Vec2Real prevDisp = prev->v[1] - prev->v[0];
				const Real sumDisp2 = (prevDisp + edgeDisp).lengthSquared();
				if( sumDisp2 < distanceTol2 )
				{	// These two edges cancel.  Eliminate both.
					next = edge->getAdj(1);
					prev->v[1] = prev->v[0];
					edge->v[0] = prev->v[1];
					prev->remove();
					edge->remove();
					if( next == prev )
					{	// Loops is empty
						edge = NULL;
						break;	// Done
					}
				}
				else
				{
					const Real h2 = square( prevDisp^edgeDisp )/sumDisp2;
					if( h2 < distanceTol2 )
					{	// Colinear, remove
						prev->v[1] = edge->v[0] = edge->v[1];
						edge->remove();
					}
				}
				edge = next;
			} while( edge->loopID < 0 );
			if( edge != NULL )
			{
				++loopID;
				loops.pushBack( edge );
			}
		}
	}

	if( loops.size() == 0 )
	{	// No loops, done
		planeVertices.reset();
		return true;
	}

#if DEBUG_OUTPUT
	for( physx::PxU32 i = 0; i < loops.size(); ++i )
	{
		LinkedEdge2D* loop = loops[i];
		char filename[100];
		sprintf( filename, "poly%03d.txt", i );
		FILE* fp = fopen( filename, "w" );
		LinkedEdge2D* edge = loop;
		do
		{
			PX_ASSERT( ( edge->v[1] - edge->getAdj(1)->v[0] ).lengthSquared() < 1.0e-12 );
			fprintf( fp, "%f, %f\n", edge->v[0][0], edge->v[0][1] );
			edge = edge->getAdj(1);
		} while( edge != loop );
		fclose( fp );
	}
#endif // if DEBUG_OUTPUT

	// The methods employed below are not optimial in time.  But the majority of cases will be simple polygons
	// with no holes and a small number of vertices.  So an optimal algorithm probably isn't worth implementing.

	// Merge all loops into one by finding diagonals to join them
	while( loops.size() > 1 )
	{
		LinkedEdge2D* loop = loops.back();
		LinkedEdge2D* bestEdge = NULL;
		LinkedEdge2D* bestOtherEdge = NULL;
		physx::PxU32 bestOtherLoopIndex = 0xFFFFFFFF;
		Real minDist2 = MAX_REAL;
		for( physx::PxU32 i = loops.size()-1; i--; )
		{
			LinkedEdge2D* otherLoop = loops[i];
			LinkedEdge2D* otherEdge = otherLoop;
			do
			{
				LinkedEdge2D* edge = loop;
				do
				{
					if( diagonalIsValid( edge, otherEdge, loops ) )
					{
						const Real dist2 = (edge->v[0] - otherEdge->v[0]).lengthSquared();
						if( dist2 < minDist2 )
						{
							bestEdge = edge;
							bestOtherEdge = otherEdge;
							bestOtherLoopIndex = i;
							minDist2 = dist2;
						}
					}
				} while( (edge = edge->getAdj(1)) != loop );
			} while( (otherEdge = otherEdge->getAdj(1)) != otherLoop );
		}

		if( bestOtherLoopIndex == 0xFFFFFFFF )
		{
			// Clean up loops
			for( physx::PxU32 i = 0; i < loops.size(); ++i )
			{
				LinkedEdge2D* edge = loops[i];
				bool done = false;
				do
				{
					done = edge->isSolitary();
					LinkedEdge2D* next = edge->getAdj(1);
					linkedEdgePool.replace( edge );	// This also removes the link from the loop
					edge = next;
				} while( !done );
			}
			return false;
		}

		// Create diagonal loop with correct endpoints
		LinkedEdge2D* diagonal = linkedEdgePool.borrow();
		LinkedEdge2D* reciprocal = linkedEdgePool.borrow();
		diagonal->setAdj( 1, reciprocal );
		diagonal->v[1] = reciprocal->v[0] = bestEdge->v[0];
		diagonal->v[0] = reciprocal->v[1] = bestOtherEdge->v[0];

		// Insert diagonal loop, merging loops.back() with loops[i]
		diagonal->setAdj( 1, bestEdge );
		reciprocal->setAdj( 1, bestOtherEdge );
		loops.popBack();
	}

	// Erase planeVertices, will reuse.
	planeVertices.reset();

	// We have one loop - check for trivial cases
	LinkedEdge2D* loop = loops[0];
	if( loop->isSolitary() )
	{	// Single edge !?
		linkedEdgePool.replace( loop );
		return true;
	}
	if( loop->getAdj(1)->getAdj(1) == loop )
	{	// Degenerate
		LinkedEdge2D* next = loop->getAdj(1);
		linkedEdgePool.replace( loop );
		linkedEdgePool.replace( next );
		return true;
	}
	if( loop->getAdj(1)->getAdj(1)->getAdj(1) == loop )
	{	// It's a triangle, we're done.
		addTriangleVertices( planeVertices, loop, linkedEdgePool );
		return true;
	}

#if DEBUG_OUTPUT
	physx::PxU32 loopSize = 0;
	LinkedEdge2D* edge = loops.back();
	do
	{
		edge = edge->getAdj(1);
		++loopSize;
	} while( edge != loops.back() );
#endif // if DEBUG_OUTPUT

	// The loop has more than three edges.  Triangluate.
	do
	{
		LinkedEdge2D* loop = loops.back();
		loops.popBack();
#if DEBUG_OUTPUT
		static int filex = 0;
		char filename[100];
		sprintf( filename, "poly%03d.txt", filex++ );
		FILE* fp = fopen( filename, "w" );
		LinkedEdge2D* edge = loop;
		do
		{
			PX_ASSERT( ( edge->v[1] - edge->getAdj(1)->v[0] ).lengthSquared() < 1.0e-12 );
			fprintf( fp, "%f, %f\n", edge->v[0][0], edge->v[0][1] );
			edge = edge->getAdj(1);
		} while( edge != loop );
		fclose( fp );
#endif // if DEBUG_OUTPUT
		LinkedEdge2D* newLoop = splitWithDiagonal( loop, linkedEdgePool );
		if( newLoop == NULL )
		{	// Clean up
			for( physx::PxU32 i = 0; i < loops.size(); ++i )
			{
				LinkedEdge2D* edge = loops[i];
				bool done = false;
				do
				{
					done = edge->isSolitary();
					LinkedEdge2D* next = edge->getAdj(1);
					linkedEdgePool.replace( edge );	// This also removes the link from the loop
					edge = next;
				} while( !done );
			}
			return false;
		}
		if( loop->getAdj(1)->getAdj(1)->getAdj(1) == loop )
		{	// Loop is now a triangle
			addTriangleVertices( planeVertices, loop, linkedEdgePool );
		}
		else
		{
			loops.pushBack( loop );
		}
		if( newLoop->getAdj(1)->getAdj(1)->getAdj(1) == newLoop )
		{	// newLoop is a triangle
			addTriangleVertices( planeVertices, newLoop, linkedEdgePool );
		}
		else
		{
			loops.pushBack( newLoop );
		}
	} while( loops.size() > 0 );

	return true;
}

static void
mergeTriangles( physx::Array<NxExplicitRenderTriangle>& cleanedMesh, const Triangle* triangles, const Interpolator* frames, ClippedTriangleInfo* info, physx::PxU32 triangleCount,
			    const physx::Array<Triangle>& originalTriangles, Pool<LinkedEdge2D>& linkedEdgePool, Real distanceTol )
{
	if( triangleCount == 0 )
	{
		return;
	}

	const physx::PxU32 originalTriangleIndexStart = info[0].originalTriangleIndex;
	const physx::PxU32 originalTriangleIndexCount = info[triangleCount-1].originalTriangleIndex-originalTriangleIndexStart+1;

	physx::Array<physx::PxU32> originalTriangleGroupStarts;
	createIndexStartLookup( originalTriangleGroupStarts, originalTriangleIndexStart, originalTriangleIndexCount,
							(physx::PxI32*)&info->originalTriangleIndex, triangleCount, sizeof( ClippedTriangleInfo ) );

	// Now group equal reference frames, and transform into 2D
	physx::Array<Vec2Real> planeVertices;
	NxIndexBank<physx::PxU32> frameIndices;
	frameIndices.reserve( originalTriangleIndexCount );
	frameIndices.lockCapacity( true );
	while( frameIndices.freeCount() )
	{
		planeVertices.reset();	// Erase, we'll reuse this array
		physx::PxU32 seedFrameIndex;
		frameIndices.useNextFree( seedFrameIndex );
		const Interpolator& seedFrame = frames[seedFrameIndex+originalTriangleIndexStart];
		const Triangle& seedOriginalTri = originalTriangles[originalTriangleIndexStart+seedFrameIndex];
		const Plane triPlane( seedOriginalTri.normal,
			(Real)0.333333333333333333333*(seedOriginalTri.vertices[0]+seedOriginalTri.vertices[1]+seedOriginalTri.vertices[2]) );
		const Dir& zAxis = triPlane.normal();
		const physx::PxU32 maxDir = NxMath::abs( zAxis[0] ) > NxMath::abs( zAxis[1] ) ?
							( NxMath::abs( zAxis[0] ) > NxMath::abs( zAxis[2] ) ? 0 : 2 ) :
							( NxMath::abs( zAxis[1] ) > NxMath::abs( zAxis[2] ) ? 1 : 2 );
		Dir xAxis( (Real)0 );
		xAxis[(maxDir+1)%3] = (Real)1;
		Dir yAxis = zAxis^xAxis;
		yAxis.normalize();
		xAxis = yAxis^zAxis;
		Real signedArea = 0;
		physx::Array<physx::PxU32> mergedFrameIndices;
		mergedFrameIndices.pushBack( seedFrameIndex );
		for( physx::PxU32 i = originalTriangleGroupStarts[seedFrameIndex]; i < originalTriangleGroupStarts[seedFrameIndex+1]; ++i )
		{
			const Triangle& triangle = triangles[info[i].clippedTriangleIndex];
			const physx::PxU32 ccw = (triangle.normal | zAxis) > 0;
			const Real sign = ccw ? (Real)1 : -(Real)1;
			signedArea += sign*physx::PxAbs( triangle.area );
			const physx::PxU32 i1 = 2-ccw;
			const physx::PxU32 i2 = 1+ccw;
			planeVertices.pushBack( Vec2Real( xAxis|triangle.vertices[0], yAxis|triangle.vertices[0] ) );
			planeVertices.pushBack( Vec2Real( xAxis|triangle.vertices[i1], yAxis|triangle.vertices[i1] ) );
			planeVertices.pushBack( Vec2Real( xAxis|triangle.vertices[i2], yAxis|triangle.vertices[i2] ) );
		}
#if 1
		const physx::PxU32* freeIndexPtrStop = frameIndices.usedIndices() + frameIndices.capacity();
		for( const physx::PxU32* nextFreeIndexPtr = frameIndices.freeIndices(); nextFreeIndexPtr < freeIndexPtrStop; ++nextFreeIndexPtr )
		{
			const physx::PxU32 nextFreeIndex = *nextFreeIndexPtr;
			const Triangle& nextOriginalTri = originalTriangles[originalTriangleIndexStart+nextFreeIndex];
			if( nextOriginalTri.submeshIndex != nextOriginalTri.submeshIndex )
			{
				continue;	// Different submesh, don't use
			}
			if( triPlane.distance( nextOriginalTri.vertices[0] ) > distanceTol ||
				triPlane.distance( nextOriginalTri.vertices[1] ) > distanceTol ||
				triPlane.distance( nextOriginalTri.vertices[2] ) > distanceTol )
			{
				continue;	// Not coplanar
			}
			const Interpolator& nextFreeFrame = frames[nextFreeIndex+originalTriangleIndexStart];
			// BRG - Ouch, any way to set these tolerances in a little less of an ad hoc fashion?
			if( !nextFreeFrame.equals( seedFrame, (Real)0.001, (Real)0.001, (Real)0.001, (Real)0.01, (Real)0.001 ) )
			{
				continue;	// Frames different, don't use
			}
			// We can use this frame
			frameIndices.use( nextFreeIndex );
			mergedFrameIndices.pushBack( nextFreeIndex );
			for( physx::PxU32 i = originalTriangleGroupStarts[nextFreeIndex]; i < originalTriangleGroupStarts[nextFreeIndex+1]; ++i )
			{
				const Triangle& triangle = triangles[info[i].clippedTriangleIndex];
				const physx::PxU32 ccw = (triangle.normal | zAxis) > 0;
				const Real sign = ccw ? (Real)1 : -(Real)1;
				signedArea += sign*physx::PxAbs( triangle.area );
				const physx::PxU32 i1 = 2-ccw;
				const physx::PxU32 i2 = 1+ccw;
				planeVertices.pushBack( Vec2Real( xAxis|triangle.vertices[0], yAxis|triangle.vertices[0] ) );
				planeVertices.pushBack( Vec2Real( xAxis|triangle.vertices[i1], yAxis|triangle.vertices[i1] ) );
				planeVertices.pushBack( Vec2Real( xAxis|triangle.vertices[i2], yAxis|triangle.vertices[i2] ) );
			}
		}
#endif

		// We've collected all of the clipped triangles that fit within a single reference frame, and transformed them into the x,y plane.
		// Now process this collection.
		const physx::PxU32 oldPlaneVertexCount = planeVertices.size();
		const bool success = mergeTriangles2D( planeVertices, linkedEdgePool, distanceTol );
		if( success && planeVertices.size() < oldPlaneVertexCount )
		{	// Transform back into 3 space and append to cleanedMesh
			const physx::PxU32 ccw = signedArea >= 0;
			const Real sign = ccw ? (Real)1 : (Real)-1;
			const physx::PxU32 vMap[3] = { 0, 2-ccw, 1+ccw };
			const Pos planeOffset = Pos((Real)0) + (-triPlane.d())*zAxis;
			for( physx::PxU32 i = 0; i < planeVertices.size(); i += 3 )
			{
				NxExplicitRenderTriangle& cleanedTri = cleanedMesh.insert();
				VertexData vertexData[3];
				Triangle tri;
				for( physx::PxU32 v = 0; v < 3; ++v )
				{
					const Vec2Real& planeVertex = planeVertices[i+vMap[v]];
					tri.vertices[v] = planeOffset + planeVertex[0]*xAxis + planeVertex[1]*yAxis;
					seedFrame.interpolateVertexData( vertexData[v], tri.vertices[v] );
					vertexData[v].normal *= sign;
				}
				tri.toExplicitRenderTriangle( cleanedTri, vertexData );
				cleanedTri.submeshIndex = seedOriginalTri.submeshIndex;
				cleanedTri.smoothingMask = seedOriginalTri.smoothingMask;
				cleanedTri.extraDataIndex = seedOriginalTri.extraDataIndex;
			}
		}
		else
		{	// An error occurred, or we increased the triangle count.  Just use original triangles
			for( physx::PxU32 i = 0; i < mergedFrameIndices.size(); ++i )
			{
				for( physx::PxU32 j = originalTriangleGroupStarts[mergedFrameIndices[i]]; j < originalTriangleGroupStarts[mergedFrameIndices[i]+1]; ++j )
				{
					const Triangle& tri = triangles[info[j].clippedTriangleIndex];
					NxExplicitRenderTriangle& cleanedMeshTri = cleanedMesh.insert();
					VertexData vertexData[3];
					for( int v = 0; v < 3; ++v )
					{
						frames[info[j].originalTriangleIndex].interpolateVertexData( vertexData[v], tri.vertices[v] );
						if( !info[j].ccw )
						{
							vertexData[v].normal *= -1.0;
						}
					}
					tri.toExplicitRenderTriangle( cleanedMeshTri, vertexData );
				}
			}
		}
	}

	return;
}

void
cleanMesh( physx::Array<NxExplicitRenderTriangle>& cleanedMesh, const physx::Array<Triangle>& mesh, physx::Array<ClippedTriangleInfo>& triangleInfo, physx::PxU32 planeCount, const physx::Array<Triangle>& originalTriangles, const physx::Array<Interpolator>& frames, Real distanceTol )
{
	cleanedMesh.clear();

	// Sort triangles into splitting plane groups, then original triangle groups
	qsort( triangleInfo.begin(), triangleInfo.size(), sizeof( ClippedTriangleInfo ), ClippedTriangleInfo::cmp );

	physx::Array<physx::PxU32> planeGroupStarts;
	createIndexStartLookup( planeGroupStarts, 0, planeCount, (physx::PxI32*)&triangleInfo.begin()->planeIndex, triangleInfo.size(), sizeof( ClippedTriangleInfo ) );

	Pool<LinkedEdge2D> linkedEdgePool;
	for( physx::PxU32 i = 0; i < planeCount; ++i )
	{
		mergeTriangles( cleanedMesh, mesh.begin(), frames.begin(), triangleInfo.begin() + planeGroupStarts[i], planeGroupStarts[i+1]-planeGroupStarts[i], originalTriangles, linkedEdgePool, distanceTol );
	}
}

}
};
#endif