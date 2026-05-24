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
#ifndef FRACTURING_H

#define FRACTURING_H

#include "PsShare.h"
#include "NxApex.h"
#include "ApexSharedSerialization.h"
#include "FractureTools.h"
#include "ApexString.h"
#include "ExplicitHierarchicalMesh.h"
#include "authoring/ApexCSG.h"

#include "NxFromPx.h"

#ifndef WITHOUT_APEX_AUTHORING

namespace physx {
namespace apex {

using namespace FractureTools;


struct IntersectMesh
{
	enum GridPattern
	{
		None,	// An infinite plane
		Equilateral,
		Right
	};

	physx::PxF32 getSide( const physx::PxVec3& v )
	{
		if( m_pattern == None )
		{
			NxVec3 vpos;
			NxFromPxVec3( vpos, v );
			return m_plane.distance( vpos );
		}
		physx::PxVec3 vLocal;
		m_tm.multiplyByInverseRT( v, vLocal );
		physx::PxF32 x = vLocal.x - m_cornerX;
		physx::PxF32 y = vLocal.y - m_cornerY;
		if( y < 0 )
		{
			return 0;
		}
		physx::PxF32 scaledY = y/m_ySpacing;
		physx::PxU32 gridY = (physx::PxU32)scaledY;
		if( gridY >= m_numY )
		{
			return 0;
		}
		scaledY -= (physx::PxF32)gridY;
		physx::PxU32 yParity = gridY&1;
		if( yParity != 0 )
		{
			scaledY = 1.0f-scaledY;
		}
		if( m_pattern == Equilateral )
		{
			x += 0.5f*m_xSpacing*scaledY;
		}
		if( x < 0 )
		{
			return 0;
		}
		physx::PxF32 scaledX = x/m_xSpacing;
		physx::PxU32 gridX = (physx::PxU32)scaledX;
		if( gridX >= m_numX )
		{
			return 0;
		}
		scaledX -= (physx::PxF32)gridX;
		physx::PxU32 xParity = (physx::PxU32)(scaledX >= scaledY);
		physx::PxU32 triangleNum = 2*(gridY*m_numX+gridX)+xParity;
		PX_ASSERT( triangleNum < m_triangles.size() );
		NxExplicitRenderTriangle& triangle = m_triangles[triangleNum];
		physx::PxVec3& v0 = triangle.vertices[0].position;
		physx::PxVec3& v1 = triangle.vertices[1].position;
		physx::PxVec3& v2 = triangle.vertices[2].position;
		return ((v1-v0)^(v2-v0))|(v-v0);
	}

	void clear()
	{
		m_pattern = None;
		m_plane.set( NxVec3( 0, 0, 1 ), 0 );
		m_vertices.reset();
		m_triangles.reset();
	}

	void build( const NxPlane& plane )
	{
		clear();
		m_plane = plane;
	}

	void build( GridPattern pattern, const NxPlane& plane,
				physx::PxF32 cornerX, physx::PxF32 cornerY, physx::PxF32 xSpacing, physx::PxF32 ySpacing, physx::PxU32 numX, physx::PxU32 numY,
				const physx::PxMat34Legacy& tm, float noiseAmplitude, float relativeFrequency, bool periodicNoise,
				int noiseType, int noiseDir, physx::PxU32 submeshIndex, physx::PxU32 frameIndex, const TriangleFrame& triangleFrame );

	GridPattern									m_pattern;

	physx::PxMat34Legacy						m_tm;
	NxPlane										m_plane;
	physx::Array<NxVertex>						m_vertices;
	physx::Array<NxExplicitRenderTriangle>		m_triangles;

	physx::PxU32								m_numX;
	physx::PxF32								m_cornerX;
	physx::PxF32								m_xSpacing;
	physx::PxU32								m_numY;
	physx::PxF32								m_cornerY;
	physx::PxF32								m_ySpacing;
};

// CutoutSet

struct PolyVert
{
	NxU16 index;
	NxU16 flags;
};

struct ConvexLoop
{
	physx::Array<PolyVert> polyVerts;
};

struct Cutout
{
	physx::Array<physx::PxVec3> vertices;
	physx::Array<ConvexLoop> convexLoops;
};

struct CutoutSet : public ICutoutSet
{
	enum Version
	{
		First = 0,
		// New versions must be put here.  There is no need to explicitly number them.  The
		// numbers above were put there to conform to the old DestructionToolStreamVersion enum.

		Count,
		Current = Count-1
	};

	physx::PxU32			getCutoutCount() const { return cutouts.size(); }

	physx::PxU32			getCutoutVertexCount( physx::PxU32 cutoutIndex ) const { return cutouts[cutoutIndex].vertices.size(); }
	physx::PxU32			getCutoutLoopCount( physx::PxU32 cutoutIndex ) const { return cutouts[cutoutIndex].convexLoops.size(); }

	const physx::PxVec3&	getCutoutVertex( physx::PxU32 cutoutIndex, physx::PxU32 vertexIndex ) const { return cutouts[cutoutIndex].vertices[vertexIndex]; }

	physx::PxU32			getCutoutLoopSize( physx::PxU32 cutoutIndex, physx::PxU32 loopIndex ) const { return cutouts[cutoutIndex].convexLoops[loopIndex].polyVerts.size(); }

	physx::PxU32			getCutoutLoopVertexIndex( physx::PxU32 cutoutIndex, physx::PxU32 loopIndex, physx::PxU32 vertexNum ) const { return cutouts[cutoutIndex].convexLoops[loopIndex].polyVerts[vertexNum].index; }
	physx::PxU32			getCutoutLoopVertexFlags( physx::PxU32 cutoutIndex, physx::PxU32 loopIndex, physx::PxU32 vertexNum ) const { return cutouts[cutoutIndex].convexLoops[loopIndex].polyVerts[vertexNum].flags; }

	void					serialize( physx::PxFileBuf& stream ) const;
	void					deserialize( physx::PxFileBuf& stream );

	void					release() { delete this; }

	physx::Array<Cutout>	cutouts;
};

// ExplicitHierarchicalMesh

class ExplicitHierarchicalMesh : public IExplicitHierarchicalMesh
{
public:

	// This has been copied from DestructionToolStreamVersion, at ToolStreamVersion_RemovedExplicitHMesh_mMaxDepth.
	enum Version
	{
		First = 0,
		AddedMaterialFramesToHMesh_and_NoiseType_and_GridSize_to_Cleavage = 7,
		IncludingVertexFormatInSubmeshData = 12,
		AddedMaterialLibraryToMesh = 14,
		AddedCacheChunkSurfaceTracesAndInteriorSubmeshIndex = 32,
		RemovedExplicitHMesh_mMaxDepth = 38,
		UsingExplicitPartContainers,
		SerializingMeshBSP,
		SerializingMeshBounds,
		AddedFlagsFieldToPart,
		PerPartMeshBSPs,
		StoringRootSubmeshCount,
		// New versions must be put here.  There is no need to explicitly number them.  The
		// numbers above were put there to conform to the old DestructionToolStreamVersion enum.

		Count,
		Current = Count-1
	};

	struct Part : public physx::UserAllocated
	{
		Part() : mParentIndex(-1), mMeshBSP(NULL), mFlags(0)
		{
			mCollision.init();
		}

		~Part()
		{
			if( mMeshBSP != NULL )
			{
				mMeshBSP->release();
				mMeshBSP = NULL;
			}
		}

		physx::PxI32							mParentIndex;
		physx::PxBounds3						mBounds;
		physx::Array<NxExplicitRenderTriangle>	mMesh;
		IApexBSP*								mMeshBSP;
		ConvexHull								mCollision;
		NxU32									mFlags;	// See FractureTools::MeshFlag
	};

	physx::Array<Part*>					mParts;
	physx::Array<NxExplicitSubmeshData>	mSubmeshData;
	physx::Array<physx::PxMat44>		mMaterialFrames;
	physx::PxI32						mInteriorSubmeshIndex;
	physx::PxU32						mRootDepth;	// Dpeth of original pieces; used for BSP calculations
	physx::PxU32						mRootSubmeshCount;	// How many submeshes came with the root mesh

	IApexBSPMemCache*					mBSPMemCache;

	ExplicitHierarchicalMesh();
	~ExplicitHierarchicalMesh();

	ConvexHull* collision( physx::PxU32 partIndex )
	{
		return partIndex < partCount() ? &mParts[partIndex]->mCollision : NULL;
	}

	void separateChunkIslands( physx::PxU32 partIndex, NxConvexHullMethod::Enum hullMethod );

	// Sorts parts in parent-sorted order (stable)
	void sortParts( physx::Array<physx::PxU32>* indexRemap = NULL );

	// IExplicitHierarchicalMesh implementation:

	physx::PxU32 addPart();
	bool removePart( physx::PxU32 index );
	void serialize( physx::PxFileBuf& stream, IEmbedding& embedding ) const;
	void deserialize( physx::PxFileBuf& stream, IEmbedding& embedding );
	physx::PxI32 maxDepth() const;
	physx::PxI32* interiorSubmeshIndex();
	physx::PxU32 partCount() const;
	physx::PxU32 depth( physx::PxU32 partIndex ) const;
	physx::PxI32* parentIndex( physx::PxU32 partIndex );
	physx::PxU32 meshTriangleCount( physx::PxU32 partIndex ) const;
	NxExplicitRenderTriangle* meshTriangles( physx::PxU32 partIndex );
	physx::PxBounds3 meshBounds( physx::PxU32 partIndex ) const;
	physx::PxU32 meshFlags( physx::PxU32 partIndex ) const;
	physx::PxBounds3 collisionHullBounds( physx::PxU32 partIndex ) const;
	physx::PxU32 collisionHullVertexCount( physx::PxU32 partIndex ) const;
	physx::PxVec3 collisionHullVertex( physx::PxU32 partIndex, physx::PxU32 vertexIndex ) const;
	physx::PxU32 collisionHullEdgeCount( physx::PxU32 partIndex ) const;
	physx::PxVec3 collisionHullEdgeEndpoint( physx::PxU32 partIndex, physx::PxU32 edgeIndex, physx::PxU32 whichEndpoint ) const;
	physx::PxU32 collisionHullPlaneCount( physx::PxU32 partIndex ) const;
	NxPlane collisionHullPlane( physx::PxU32 partIndex, physx::PxU32 planeIndex ) const;
	bool collisionHullRayCast( physx::PxU32 partIndex, physx::PxF32& in, physx::PxF32& out, const NxRay& worldRay,
							   const physx::PxMat44& localToWorldRT, const physx::PxVec3& scale, physx::PxVec3* normal = NULL ) const;
	physx::PxU32 submeshCount() const;
	NxExplicitSubmeshData* submeshData( physx::PxU32 submeshIndex );
	physx::PxU32 addSubmesh( const NxExplicitSubmeshData& submeshData );
	physx::PxU32 getMaterialFrameCount() const;
	physx::PxMat44& getMaterialFrame( physx::PxU32 index );
	physx::PxMat44& addMaterialFrame();
	void clear( bool keepRoot = false );
	void set( const IExplicitHierarchicalMesh& mesh );
	void calculatePartBSP( physx::PxU32 partIndex, physx::PxU32 randomSeed, IProgressListener* progressListener = NULL );
	void calculateMeshBSP( physx::PxU32 randomSeed, IProgressListener* progressListener = NULL );
	void visualize( NxApexRenderDebug& debugRender, physx::PxU32 flags, physx::PxU32 index = 0 ) const;
	void release();
	void buildMeshBounds( physx::PxU32 partIndex );
	void buildCollisionHull( physx::PxU32 partIndex, NxConvexHullMethod::Enum method );
};

}} // end namespace physx::apex

#endif

#endif
