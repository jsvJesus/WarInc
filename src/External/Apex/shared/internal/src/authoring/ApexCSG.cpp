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
#include "PxSimpleTypes.h"

#include "authoring/ApexCSGDefs.h"
#include "authoring/ApexCSGSerialization.h"
#include "ApexSharedSerialization.h"
#include "NiApexRenderDebug.h"

#include <stdio.h>

#include "PxUserOutputStream.h"

#ifndef WITHOUT_APEX_AUTHORING

#define CSG_DEBUG_NODEPATH	0

#if CSG_DEBUG_NODEPATH
static char sNodePath[] = "1111";
static physx::Array<char>* sNodeStack = NULL;
static char* sNodeMark = sNodePath;
static int sNodeCount = 0;
#endif

namespace physx
{
	namespace apex
	{

// Default tolerances for geometric calculations

#define DEFAULT_ANGULAR_TOL		(0.01f)

#define REGULAR_LINEAR_TOL		(0.002f)
#define REGULAR_ANGULAR_TOL		(0.01f)

#define ORGANIC_LINEAR_TOL		(0.000001f)
#define ORGANIC_ANGULAR_TOL		(0.00001f)

#define BASE_TOLERANCE			((Real)1.0e-9)

#define HULL_TOLERANCE_OFFSET	((Real)1.0e-2)
#define CLIP_TOLERANCE_OFFSET	((Real)1.0e-4)


/* Interpolator */

size_t
Interpolator::s_offsets[Interpolator::VertexFieldCount];

static InterpolatorBuilder
sInterpolatorBuilder;

void
Interpolator::serialize( physx::PxFileBuf& stream ) const
{
	for( physx::PxU32 i = 0; i < VertexFieldCount; ++i )
	{
		stream << m_frames[i];
	}
}

void
Interpolator::deserialize( physx::PxFileBuf& stream, physx::PxU32 version )
{
	if( version < Version::SerializingTriangleFrames )
	{
		return;
	}
	
	for( physx::PxU32 i = 0; i < VertexFieldCount; ++i )
	{
		stream >> m_frames[i];
	}
}


/* Utilities */

class DefaultRandom : public UserRandom
{
public:
	physx::PxU32	getInt()						{ return m_rnd.nextSeed(); }
	physx::PxF32	getReal( physx::PxF32 min, physx::PxF32 max )	{ return m_rnd.getScaled( min, max ); }

	QDSRand	m_rnd;
} defaultRnd;

PX_INLINE int	// Returns 0 if the point is on the plane (within tolerance), otherwise +/-1 if the point is above/below the plane
cmpPointToPlane( const Pos& pos, const Plane& plane, Real tol )
{
	const Real dist = plane.distance( pos );

	return dist < -tol ? -1 : (dist < tol ? 0 : 1);
}

struct IndexedPxReal
{
	physx::PxF32	value;
	physx::PxU32	index;

	static	int	cmpDecreasing( const void* a, const void* b )
				{
					return ((IndexedPxReal*)a)->value == ((IndexedPxReal*)b)->value ? 0 : (((IndexedPxReal*)a)->value < ((IndexedPxReal*)b)->value ? 1 : -1);
				}
};

static LinkedVertex*
clipPolygonByPlane( LinkedVertex* poly, const Plane& plane, Pool<LinkedVertex>& pool, Real tol )
{
	LinkedVertex* prev = poly;
	int prevSide = cmpPointToPlane( prev->vertex, plane, tol );
	bool outsideFound = prevSide == 1;
	bool insideFound = prevSide == -1;
	LinkedVertex* clip0 = NULL;
	LinkedVertex* clip1 = NULL;
	LinkedVertex* next = prev->getAdj(1);
	if( next != poly )
	{
		do
		{
			int nextSide = cmpPointToPlane( next->vertex, plane, tol );
			switch( nextSide )
			{
			case -1:
				insideFound = true;
				if( prevSide == 1 )
				{	// Clip
					clip1 = pool.borrow();
					const Dir disp = next->vertex - prev->vertex;
					const Real dDisp = disp|plane.normal();
					PX_ASSERT( dDisp < 0 );
					const Real dAbove = plane.distance( prev->vertex );
					clip1->vertex = prev->vertex - (dAbove/dDisp)*disp;
					next->setAdj( 0, clip1 );	// Insert clip1 between prev and next
				}
				else
				if( prevSide == 0 )
				{
					clip1 = prev;
				}
				break;
			case 0:
				if( prevSide == -1 )
				{
					clip0 = next;
				}
				else
				if( prevSide == 1 )
				{
					clip1 = next;
				}
				break;
			case 1:
				outsideFound = true;
				if( prevSide == -1 )
				{	// Clip
					clip0 = pool.borrow();
					const Dir disp = next->vertex - prev->vertex;
					const Real dDisp = disp|plane.normal();
					PX_ASSERT( dDisp > 0 );
					const Real dBelow = plane.distance( prev->vertex );
					clip0->vertex = prev->vertex - (dBelow/dDisp)*disp;
					next->setAdj( 0, clip0 );	// Insert clip0 between prev and next
				}
				else
				if( prevSide == 0 )
				{
					clip0 = prev;
				}
				break;
			}
			prev = next;
			prevSide = nextSide;
			next = prev->getAdj(1);
		} while( prev != poly );
	}

	PX_ASSERT( (clip0 != NULL) == (clip1 != NULL) );

	if( clip0 != NULL && clip1 != NULL && clip0 != clip1 )
	{	// Get rid of vertices between clip0 and clip1
		LinkedVertex* v = clip0->getAdj(1);
		while( v != clip1 )
		{
			LinkedVertex* w = v->getAdj(1);
			v->remove();
			pool.replace( v );
			v = w;
		}
		poly = clip1;
	}

	if( outsideFound && !insideFound )
	{	// Completely outside.  Eliminate.
		LinkedVertex* v;
		do
		{
			v = poly->getAdj(0);
			v->remove();
			pool.replace( v );
		} while( v != poly );
		poly = NULL;
	}

	return poly;
}

// If clippedMesh is not NULL, it will be appended with clipped triangles from the leaf.
// Return value is the sum triangle area on the leaf.
static Real
clipTriangleToLeaf( physx::Array<Triangle>* clippedMesh, const Triangle& tri, const BSP::Node* leaf, physx::PxU32 edgeTraversalDir,
					Pool<LinkedVertex>& vertexPool, Real distanceTol, const physx::Array<Plane>& planes, physx::PxU32 skipPlaneIndex = 0xFFFFFFFF )
{
	Real sumArea = 0;

	// Form a ring of vertices out of the triangle
	LinkedVertex* v0 = vertexPool.borrow();
	LinkedVertex* v1 = vertexPool.borrow();
	LinkedVertex* v2 = vertexPool.borrow();
	v0->vertex = tri.vertices[0];
	v1->vertex = tri.vertices[1];
	v2->vertex = tri.vertices[2];
	v0->setAdj( edgeTraversalDir, v1 );
	v1->setAdj( edgeTraversalDir, v2 );

	for( SurfaceIt it( leaf ); it.isValid(); it.inc() )
	{
		if( it.surface()->planeIndex == skipPlaneIndex )
		{
			continue;
		}
		const Real sign = it.side() ? (Real)1 : -(Real)1;
		v0 = clipPolygonByPlane( v0, sign*planes[it.surface()->planeIndex], vertexPool, distanceTol );
		if( v0 == NULL )
		{
			break;	// Completely clipped away
		}
	}

	if( v0 != NULL )
	{	// Something remains.  Add to clippedMesh if it's not NULL
		v1 = v0->getAdj(1);
		v2 = v1->getAdj(1);
		if( v1 != v0 && v2 != v0 )
		{
			if( clippedMesh != NULL )
			{
				// Triangluate
				do
				{
					Triangle& newTri = clippedMesh->insert();
					newTri.vertices[0] = v0->vertex;
					newTri.vertices[1] = v1->vertex;
					newTri.vertices[2] = v2->vertex;
					newTri.submeshIndex = tri.submeshIndex;
					newTri.smoothingMask = tri.smoothingMask;
					newTri.extraDataIndex = tri.extraDataIndex;
					newTri.calculateQuantities();
					sumArea += newTri.area;
					v1 = v2;
					v2 = v2->getAdj(1);
				} while( v2 != v0 );
			}
			else
			{
				// Triangluate
				do
				{
					sumArea += (Real)0.5*NxMath::sqrt( (Dir(v1->vertex-v0->vertex)^(Dir(v2->vertex-v0->vertex))).lengthSquared() );
					v1 = v2;
					v2 = v2->getAdj(1);
				} while( v2 != v0 );
			}
		}
		// Return links to pool.
		LinkedVertex* v;
		do
		{
			v = v0->getAdj(0);
			v->remove();
			vertexPool.replace( v );
		} while( v != v0 );
	}

	return sumArea;
}

#if USE_GSA
/*
	GSA ConvexShape for leaf neighbor determination
*/
class LeafNeighborIntersection : public physx::GSA::GSA<3,Real>::ConvexShape, private physx::GSA::GSA<3,Real>
{
public:
						LeafNeighborIntersection() : m_currentLeafPlaneCount( 0 ), m_needsReset( true )
						{
							init( 100 );
						}

	virtual	unsigned	initialize_tangent_planes( physx::GSA::Plane<3,Real>* planes, unsigned plane_count ) const
						{
							const unsigned nPlanes = NxMath::min( plane_count, (unsigned)m_planes.size() );
							for( physx::PxU32 i = 0; i < nPlanes; ++i )
							{
								planes[i] = m_planes[i];
							}
							return nPlanes;
						}

	virtual	void		intersect_line( physx::GSA::GSA<3,Real>::LineIntersect& in, physx::GSA::GSA<3,Real>::LineIntersect& ex, const physx::GSA::Pos<3,Real>& orig, const physx::GSA::Dir<3,Real>& dir, Real time ) const
						{
							(void)time;	// Not used
							Real in_num = -1.0f;
							Real in_den = 0.0f;
							Real ex_num = 1.0f;
							Real ex_den = 0.0f;
							in.m_plane.set( physx::GSA::Dir<3,Real>(), 0.0f );
							ex.m_plane.set( physx::GSA::Dir<3,Real>(), 0.0f );

							const physx::PxU32 nPlanes = m_planes.size();
							for( physx::PxU32 i = 0; i < nPlanes; ++i )
							{
								const physx::GSA::Plane<3,Real>& plane = m_planes[i];
								const Real num = -(plane|orig);
								const Real den = (plane|dir);
								if( den > physx::GSA::gsa_eps<Real>() )
								{	// Exit
									if( num*ex_den < ex_num*den )	// num/den < ex_num/ex_den
									{
										ex_num = num;
										ex_den = den;
										ex.m_plane = plane;
									}
								}
								else
								if( den < -physx::GSA::gsa_eps<Real>() )
								{	// Entrance
									if( num*in_den < in_num*den )	// num/den > in_num/in_den
									{
										in_num = -num;
										in_den = -den;
										in.m_plane = plane;
									}
								}
								else
								if( num < physx::GSA::gsa_eps<Real>() )
								{	// Parallel and coincident or outside
									if( in_den == 0.0f )
									{
										in.m_plane = plane;
									}
									if( ex_den == 0.0f )
									{
										ex.m_plane = plane;
									}
									if( num < -physx::GSA::gsa_eps<Real>() )
									{	// Outside
										if( ex_den > physx::GSA::eps_real<Real>() || num < ex_num )
										{
											in_num = -num;
											ex_num = num;
											in_den = ex_den = physx::GSA::eps_real<Real>();
											in.m_plane = ex.m_plane = plane;
										}
									}
								}
							}

							in.m_s = in_den > 0.0f ? in_num/in_den : -physx::GSA::max_real<Real>();
							ex.m_s = ex_den > 0.0f ? ex_num/ex_den : physx::GSA::max_real<Real>();
						}

	virtual physx::GSA::Dir<3,Real>	get_linear_velocity() const	{ return physx::GSA::Dir<3,Real>(); }

	void							reset() { m_needsReset = true;; }

	bool							is_empty_set()
									{
										if( m_needsReset )
										{
											set_shapes( this );
											m_needsReset = false;
										}
										return !intersect();
									}

	physx::Array< physx::GSA::Plane<3,Real> >	m_planes;
	physx::PxU32								m_currentLeafPlaneCount;
	bool										m_needsReset;
};
#endif // #if USE_GSA


/* BSP */

BSP::BSP( IApexBSPMemCache* memCache ) :
m_root(NULL),
m_combined(false),
m_meshSize(1),
m_combiningMeshSize(REGULAR_LINEAR_TOL),
m_memCache( (BSPMemCache*)memCache ),
m_ownsMemCache( false )
{
	if( m_memCache == NULL )
	{
		m_memCache = (BSPMemCache*)createBSPMemCache();
		m_ownsMemCache = true;
	}

	// Always have a node.  The trivial (one-leaf) tree is considered "inside".
	m_root = m_memCache->m_nodePool.borrow();
#if !USE_GSA
	m_root->setLeafData( m_memCache->m_regionPool.borrow() );
	m_root->getLeafData()->side = 1;
#endif
}

BSP::~BSP()
{
	if( m_ownsMemCache )
	{
		m_memCache->release();
	}
}

bool
BSP::fromMesh( const physx::Array<NxExplicitRenderTriangle>& mesh, const BSPBuildParameters& params, IProgressListener* progressListener )
{
	if( mesh.size() == 0 )
	{
		return false;
	}

	clear();

	// Tolerances
	Real linearTolerance = params.linearTolerance;
	Real angularTolerance = params.angularTolerance;
	if( linearTolerance < 0 || angularTolerance < 0 )
	{
		// Measure some statistics on the mesh
		const physx::PxF32 groupWidth = DEFAULT_ANGULAR_TOL;
		physx::Array<physx::PxVec3> normals( mesh.size() );
		physx::Array<BoundsRep> boundsReps( mesh.size() );
		for( physx::PxU32 i = 0; i < mesh.size(); ++i )
		{
			const NxExplicitRenderTriangle& inTri = mesh[i];
			normals[i] = (inTri.vertices[1].position-inTri.vertices[0].position)^(inTri.vertices[2].position-inTri.vertices[0].position);
			normals[i].normalize();
			boundsReps[i].aabb.include( normals[i] );
			boundsReps[i].aabb.fatten( 0.5f*DEFAULT_ANGULAR_TOL );
		}
		physx::Array<IntPair> overlaps;
		boundsCalculateOverlaps( overlaps, Bounds3XYZ, boundsReps );
		physx::PxU32 overlapCount = 0;
		for( physx::PxU32 i = 0; i < overlaps.size(); ++i )
		{
			IntPair& pair = overlaps[i];
			if( (normals[pair.i0] | normals[pair.i1]) > (physx::PxF32)1-(physx::PxF32)0.5*DEFAULT_ANGULAR_TOL*DEFAULT_ANGULAR_TOL )
			{
				overlaps[overlapCount++] = overlaps[i];
			}
		}
		// symmetrize the overlap list
		overlaps.resize( overlapCount );
		// Find islands
		physx::Array< physx::Array<physx::PxU32> > islands;
		findIslands( islands, overlaps, mesh.size() );
		// Find the average deviation
		physx::PxF32 averageDeviation = 0;
		for( physx::PxU32 i = 0; i < islands.size(); ++i )
		{
			physx::Array<physx::PxU32>& island = islands[i];
			physx::PxVec3 mean(0.0f);
			for( physx::PxU32 j = 0; j < island.size(); ++j )
			{
				mean += normals[island[j]];
			}
			mean.normalize();
			physx::PxF32 squareDeviation = 0;
			for( physx::PxU32 j = 0; j < island.size(); ++j )
			{
				squareDeviation += normals[island[j]].distanceSquared( mean );
			}
			const physx::PxF32 rmsDeviation = island.size() > 1 ? NxMath::sqrt( squareDeviation/(island.size()-1) ) : groupWidth;
			averageDeviation += rmsDeviation;
		}
		averageDeviation /= islands.size();

		// Choose tolerance based on mesh statistics
		if( averageDeviation < 0.5f*groupWidth )
		{
			if( linearTolerance < 0 )
			{
				linearTolerance = REGULAR_LINEAR_TOL;
			}
			if( angularTolerance < 0 )
			{
				angularTolerance = REGULAR_ANGULAR_TOL;
			}
		}
		else
		{
			if( linearTolerance < 0 )
			{
				linearTolerance = ORGANIC_LINEAR_TOL;
			}
			if( angularTolerance < 0 )
			{
				angularTolerance = ORGANIC_ANGULAR_TOL;
			}
		}
	}

	// Shuffle triangle ordering
	physx::Array<physx::PxU32> triangleOrder( mesh.size() );
	for( physx::PxU32 i = 0; i < mesh.size(); ++i )
	{
		triangleOrder[i] = i;
	}
	UserRandom* rnd = params.rnd != NULL ? params.rnd : &defaultRnd;
	for( physx::PxU32 i = 0; i < mesh.size(); ++i )
	{
		NX_Swap( triangleOrder[i], triangleOrder[i+(physx::PxU32)(((NxU64)rnd->getInt()*(NxU64)(mesh.size()-i))>>32)] );
	}

	// Collect mesh triangles and find mesh bounds
	m_mesh.resize( mesh.size() );
	m_frames.resize( mesh.size() );
	physx::PxBounds3 meshBounds;
	meshBounds.setEmpty();
	for( physx::PxU32 i = 0; i < m_mesh.size(); ++i )
	{
		const NxExplicitRenderTriangle& inTri = mesh[triangleOrder[i]];
		VertexData vertexData[3];
		m_mesh[i].fromExplicitRenderTriangle( vertexData, inTri );
		m_frames[i].setFromTriangle( m_mesh[i], vertexData );
		meshBounds.include( inTri.vertices[0].position );
		meshBounds.include( inTri.vertices[1].position );
		meshBounds.include( inTri.vertices[2].position );
	}

	// Size scales
	const Dir meshScales( meshBounds.maximum-meshBounds.minimum );
	m_meshSize = PxMax( meshScales[0], PxMax( meshScales[1], meshScales[2] ) );

	// Scale to unit size and zero offset for BSP building
	const Real scale = (Real)1/m_meshSize;
	const Real scale2 = (Real)1/m_meshSize;
	const Pos center = (Real)0.5*Pos( meshBounds.maximum+meshBounds.minimum );
	for( physx::PxU32 i = 0; i < m_mesh.size(); ++i )
	{
		Triangle& tri = m_mesh[i];
		for( physx::PxU32 j = 0; j < 3; ++j )
		{
			tri.vertices[j] = (tri.vertices[j]-center)*scale;
		}
		tri.area *= scale2;
	}

	// Initialize surface stack with surfaces formed from mesh triangles
#if !USE_GSA
	physx::Array<Surface*> surfaceStack;
#else
	physx::Array<Surface> surfaceStack;
#endif

	// Crude estimate, hopefully will reduce re-allocations
	surfaceStack.reserve( m_mesh.size()*((int)NxMath::log2( (physx::PxF32)m_mesh.size() ) + 1) );

	// Track maximum surface triangle area
	physx::PxF32 maxArea = 0;

	// Add mesh triangles
	physx::PxU32 triangleIndex = 0;
	while( triangleIndex < m_mesh.size() )
	{
		// Create a surface for the next triangle
		const Triangle& tri = m_mesh[triangleIndex];
#if !USE_GSA
		Surface* surface = m_memCache->m_surfacePool.borrow();
		surfaceStack.pushBack( surface );
#else
		surfaceStack.pushBack( Surface() );
		Surface* surface = &surfaceStack.back();
#endif
		surface->planeIndex = m_planes.size();
		surface->triangleIndexStart = triangleIndex++;
		Real surfaceTotalTriangleArea = tri.area;
		Plane& plane = m_planes.insert();
		plane.set( tri.normal, tri.vertices[0] );
		// See if any of the remaining triangles can fit on this surface.
		for( physx::PxU32 testTriangleIndex = triangleIndex; testTriangleIndex < m_mesh.size(); ++testTriangleIndex )
		{
			Triangle& testTri = m_mesh[testTriangleIndex];
			if( (testTri.normal^plane.normal()).lengthSquared() < angularTolerance*angularTolerance && (testTri.normal|plane.normal()) > 0 &&
				0 == cmpPointToPlane( testTri.vertices[0], plane, linearTolerance ) &&
				0 == cmpPointToPlane( testTri.vertices[1], plane, linearTolerance ) &&
				0 == cmpPointToPlane( testTri.vertices[2], plane, linearTolerance ) )
			{	// This triangle fits.  Move it next to others in the surface.
				if( testTriangleIndex != triangleIndex )
				{
					NX_Swap( m_mesh[triangleIndex], m_mesh[testTriangleIndex] );
					NX_Swap( m_frames[triangleIndex], m_frames[testTriangleIndex] );
				}
				Triangle& newTri = m_mesh[triangleIndex];
				// Add in the new normal, properly weighted
				Dir averageNormal = surfaceTotalTriangleArea*plane.normal() + newTri.area*m_mesh[triangleIndex].normal;
				averageNormal.normalize();
				surfaceTotalTriangleArea += newTri.area;
				++triangleIndex;
				// Calculate the average projection
				Real averageProjection = 0;
				for( physx::PxU32 i = surface->triangleIndexStart; i < triangleIndex; ++i )
				{
					for( physx::PxU32 j = 0; j < 3; ++j )
					{
						averageProjection += averageNormal | m_mesh[i].vertices[j];
					}
				}
				averageProjection /= 3*(triangleIndex-surface->triangleIndexStart);
				plane.set( averageNormal,-averageProjection );
			}
		}
		surface->triangleIndexStop = triangleIndex;
		surface->totalTriangleArea = (physx::PxF32)surfaceTotalTriangleArea;
		maxArea = NxMath::max( maxArea, surface->totalTriangleArea );
		// Ensure triangles lie on or below surface
		Real maxProjection = -MAX_REAL;
		for( physx::PxU32 i = surface->triangleIndexStart; i < surface->triangleIndexStop; ++i )
		{
			Triangle& tri = m_mesh[i];
			for( physx::PxU32 j = 0; j < 3; ++j )
			{
				maxProjection = NxMath::max( maxProjection, plane.normal()|tri.vertices[j] );
			}
		}
		plane[3] = -maxProjection;
	}

	// Set build process constants
	BuildConstants buildConstants;
	buildConstants.m_testSetSize = params.testSetSize;
	buildConstants.m_splitWeight = params.splitWeight;
	buildConstants.m_imbalanceWeight = params.imbalanceWeight;
	buildConstants.m_recipMaxArea = maxArea > 0 ? 1.0f/maxArea : (physx::PxF32)0;

	// Build
	m_root = m_memCache->m_nodePool.borrow();
#if !USE_GSA
	m_root->setLeafData( m_memCache->m_regionPool.borrow() );
#endif
	buildTree( m_root, surfaceStack, 0, surfaceStack.size(), buildConstants, progressListener );

	// Bring the mesh back to actual size
	// Save off mesh sizes.  These get garbled by scaled transforms.
	const Real meshSize = m_meshSize;
	const Real combiningMeshSize = m_combiningMeshSize;
	Mat4Real tm;
	tm.set( (Real)1 );
	for( physx::PxU32 i = 0; i < 3; ++i )
	{
		tm[i][i] = m_meshSize;
		tm[i][3] = center[i];
	}
	transform( tm, false );
	m_meshSize = meshSize;
	m_combiningMeshSize = combiningMeshSize;

	return true;
}

bool
BSP::combine( const IApexBSP& ibsp )
{
	const BSP& bsp = *(const BSP*)&ibsp;

	if( m_combined || bsp.m_combined )
	{
		NxGetApexSDK()->getOutputStream()->reportError( PxErrorCode::eINVALID_OPERATION,
			"BSP::combine: can only combine two uncombined BSPs.  Use op() to merge a combined BSP.", __FILE__, __LINE__ );
		return false;
	}

	// Add in other bsp's triangles.
	const physx::PxU32 thisTriangleCount = m_mesh.size();
	const physx::PxU32 totalTriangleCount = thisTriangleCount + bsp.m_mesh.size();
	m_mesh.resize( totalTriangleCount );
	m_frames.resize( totalTriangleCount );
	for( physx::PxU32 i = thisTriangleCount; i < totalTriangleCount; ++i )
	{
		m_mesh[i] = bsp.m_mesh[i-thisTriangleCount];
		m_frames[i] = bsp.m_frames[i-thisTriangleCount];
	}

	// Add in other bsp's planes.
	const physx::PxU32 thisPlaneCount = m_planes.size();
	const physx::PxU32 totalPlaneCount = thisPlaneCount + bsp.m_planes.size();
	m_planes.resize( totalPlaneCount );
	for( physx::PxU32 i = thisPlaneCount; i < totalPlaneCount; ++i )
	{
		m_planes[i] = bsp.m_planes[i-thisPlaneCount];
	}

#if !USE_GSA
	combineTrees( m_root, bsp.m_root, thisTriangleCount, thisPlaneCount );
#else
	RegionShape regionShape( (const physx::GSA::Plane<3,Real> *)m_planes.begin(), -(Real)0.000001 );
	combineTrees( m_root, bsp.m_root, thisTriangleCount, thisPlaneCount, regionShape );
#endif

	m_combiningMeshSize = bsp.m_meshSize;

	m_combined = true;

	return true;
}

bool
BSP::op( const IApexBSP& icombinedBSP, Operation::Enum operation )
{
	const BSP& combinedBSP = *(const BSP*)&icombinedBSP;

	if( !combinedBSP.m_combined )
	{
		NxGetApexSDK()->getOutputStream()->reportError( PxErrorCode::eINVALID_OPERATION,
			"BSP::op: can only perform an operation upon a combined BSP.  Use combine() with another BSP.", __FILE__, __LINE__ );
		return false;
	}
 
	if( operation == Operation::NOP )
	{
		NxGetApexSDK()->getOutputStream()->reportError( PxErrorCode::eINVALID_OPERATION,
			"BSP::op: NOP requested.  Mesh will remain combined.", __FILE__, __LINE__ );
		return false;
	}
 
	copy( combinedBSP );	// No-ops if this = combinedBSP, so this is safe

	// Combine size tolerances - look at symmetry
	switch( operation>>1 )
	{
	case 1:	// From set A
	case 5:
		// Keep size scales
		break;
	case 2:	// From set B
	case 6:
		// Replace with other size tolerance
		m_meshSize = m_combiningMeshSize;	
		break;
		// Symmetric cases
	case 0:	// Empty_Set or All_Space, set size scale to unitless value
		m_meshSize = 1;
		break;
	case 3:	// Symmetric combinations of sets, use the min scale
	case 4:
	case 7:
		m_meshSize = NxMath::min( m_meshSize, m_combiningMeshSize );
		break;
	}

	mergeLeaves( BoolOp( operation ), m_root );

	m_combined = false;

	return true;
}

bool
BSP::complement()
{
	if( m_combined )
	{
		NxGetApexSDK()->getOutputStream()->reportError( PxErrorCode::eINVALID_OPERATION,
			"BSP::complement: can only complement an uncombined BSP.  Use op() to merge a combined BSP.", __FILE__, __LINE__ );
		return false;
	}

	complementLeaves( m_root );

	return true;
}

bool
BSP::isTrivial( bool* isAllSpace ) const
{
	if( m_combined )
	{
		NxGetApexSDK()->getOutputStream()->reportError( PxErrorCode::eINVALID_OPERATION,
			"BSP::isTrivial: only meaningful for an uncombined BSP.  Use op() to merge a combined BSP.", __FILE__, __LINE__ );
		return false;
	}

	if( m_root->getType() != Node::Leaf )
	{
		return false;
	}

	if( isAllSpace != NULL )
	{
		*isAllSpace = (m_root->getLeafData()->side == 1);
	}

	return true;
}

bool
BSP::isCombined() const
{
	return m_combined;
}

#if !USE_GSA
physx::PxF32
BSP::getVolume( Operation::Enum operation ) const
{
	if( m_combined && operation == Operation::NOP )
	{
		NxGetApexSDK()->getOutputStream()->reportError( PxErrorCode::eINVALID_OPERATION,
			"BSP::getVolume: an operation must be provided for combined BSPs.", __FILE__, __LINE__ );
		return (Real)0;
	}

	if( !m_combined && operation != Operation::NOP )
	{
		NxGetApexSDK()->getOutputStream()->reportError( PxErrorCode::eDEBUG_WARNING, "BSP::getVolume: warning, operation ignored for non-combined BSPs." ,__FILE__,__LINE__);
	}

	Real volumes[2] = { (Real)0, (Real)0 };
	addLeafVolumes( m_root, volumes, BoolOp( operation ) );

	return volumes[1] != MAX_REAL ? (physx::PxF32)volumes[1] : (volumes[0] != MAX_REAL ? (physx::PxF32)volumes[0] : NX_MAX_REAL);
}
#endif

physx::PxF32
BSP::getTriangleArea( Operation::Enum operation ) const
{
	if( m_combined && operation == Operation::NOP )
	{
		NxGetApexSDK()->getOutputStream()->reportError( PxErrorCode::eINVALID_OPERATION,
			"BSP::getTriangleArea: an operation must be provided for combined BSPs.", __FILE__, __LINE__ );
		return (Real)0;
	}

	if( !m_combined && operation != Operation::NOP )
	{
		NxGetApexSDK()->getOutputStream()->reportError( PxErrorCode::eDEBUG_WARNING, "BSP::getTriangleArea: warning, operation ignored for non-combined BSPs." ,__FILE__,__LINE__);
	}

	return (physx::PxF32)addInsideLeafTriangleAreas( m_root, BoolOp( operation ) );
}

bool
BSP::toMesh( physx::Array<NxExplicitRenderTriangle>& mesh, physx::PxF32 cleaningTolerance ) const
{
	if( m_combined )
	{
		NxGetApexSDK()->getOutputStream()->reportError( PxErrorCode::eINVALID_OPERATION,
			"BSP::toMesh: can only generate a mesh from an uncombined BSP.  Use op() to merge a combined BSP.", __FILE__, __LINE__ );
		return false;
	}

	// Clip triangles collected from combined meshes
	physx::Array<Triangle> clippedMesh;
	physx::Array<ClippedTriangleInfo> triangleInfo;
	clipMeshToLeaves( clippedMesh, triangleInfo, m_root );

	// Clean
	if( cleaningTolerance > 0 )
	{
		cleanMesh( mesh, clippedMesh, triangleInfo, m_planes.size(), m_mesh, m_frames, (Real)cleaningTolerance*m_meshSize );
	}
	else
	{
		// Copy to render format
		mesh.resize( clippedMesh.size() );
		for( physx::PxU32 i = 0; i < clippedMesh.size(); ++i )
		{
			VertexData vertexData[3];
			for( int v = 0; v < 3; ++v )
			{
				m_frames[triangleInfo[i].originalTriangleIndex].interpolateVertexData( vertexData[v], clippedMesh[i].vertices[v] );
				if( !triangleInfo[i].ccw )
				{
					vertexData[v].normal *= -1.0;
				}
			}
			clippedMesh[i].toExplicitRenderTriangle( mesh[i], vertexData );
		}
	}

	return true;
}

void
BSP::copy( const IApexBSP& ibsp, const NxMat34& nxTM )
{
	const BSP& bsp = *(const BSP*)&ibsp;

	if( this != &bsp )
	{	// Copy other bsp
		clear();

		if( bsp.m_root )
		{
			m_root = m_memCache->m_nodePool.borrow();
			clone( m_root, bsp.m_root );
		}
		m_mesh = bsp.m_mesh;
		m_frames = bsp.m_frames;
		m_planes = bsp.m_planes;
		m_meshSize = bsp.m_meshSize;
		m_combined = bsp.m_combined;
		m_combiningMeshSize = bsp.m_combiningMeshSize;
	}

	// Transform
	if( nxTM.isIdentity() )
	{
		return;
	}

	// Translate NxMat34 to Mat4Real
	Mat4Real tm;
	tm.setCol( 0, Dir( nxTM.M.getColumn(0).get() ) );
	tm.setCol( 1, Dir( nxTM.M.getColumn(1).get() ) );
	tm.setCol( 2, Dir( nxTM.M.getColumn(2).get() ) );
	tm.setCol( 3, Pos( nxTM.t.get() ) );

	transform( tm );
}

#if USE_GSA
IApexBSP*
BSP::decomposeIntoIslands() const
{
	// Must be normal BSP
	if( isCombined() )
	{
		return NULL;
	}

	// First enumerate all inside leaves
	physx::PxU32 insideLeafCount = 0;
	indexInsideLeaves( insideLeafCount, m_root );
	if( insideLeafCount == 0 )
	{
		return NULL;
	}

	// Find all leaf neighbors
	physx::Array<IntPair> neighbors;
	LeafNeighborIntersection intersection;
	findInsideLeafNeighbors( neighbors, intersection, m_root );

	// Find leaf neighbor islands
	physx::Array< physx::Array<physx::PxU32> > islands;
	findIslands( islands, neighbors, insideLeafCount );

	// Return this if there is only one island
	if( islands.size() == 1 )
	{
		return const_cast<BSP*>(this);
	}

	// Otherwise we make a BSP list
	physx::Array<Node*> insideLeaves;
	insideLeaves.reserve( insideLeafCount );
	BSPLink* listRoot = PX_NEW(BSPLink)();
	for( physx::PxU32 islandNum = islands.size(); islandNum--; )
	{
		// Create new island
		BSP* islandBSP = static_cast<BSP*>( createBSP( m_memCache ) );
		if( islandBSP != NULL )
		{
			// Copy island BSP from this and add to list
			islandBSP->copy( *this );
			listRoot->setAdj( 1, islandBSP );
			// Create a list of the inside leaf pointers
			insideLeaves.clear();
			listInsideLeaves( insideLeaves, islandBSP->m_root );
			// Set all the leaves' sides to 0
			for( physx::PxU32 leafNum = 0; leafNum < insideLeaves.size(); ++leafNum )
			{
				insideLeaves[leafNum]->getLeafData()->side = 0;
			}
			// Set island leaves' sides to 1
			const physx::Array<physx::PxU32>& island = islands[islandNum];
			for( physx::PxU32 islandLeafNum = 0; islandLeafNum < island.size(); ++islandLeafNum )
			{
				insideLeaves[island[islandLeafNum]]->getLeafData()->side = 1;
			}
			// Now merge leaves to consolidate new 0-0 siblings
			islandBSP->mergeLeaves( BoolOp( Operation::Set_A ), islandBSP->m_root );
		}
	}

	// Return list head
	if( !listRoot->isSolitary() )
	{
		return static_cast<BSP*>(listRoot->getAdj( 1 ));
	}

	delete listRoot;
	return const_cast<BSP*>(this);
}
#endif

void
BSP::serialize( physx::PxFileBuf& stream ) const
{
	stream << (physx::PxU32)Version::Current;

	// Tree
	serializeNode( m_root, stream );

	// Internal mesh representation
	physx::serialize( stream, m_mesh );
	physx::serialize( stream, m_frames );
	stream << m_meshSize;

	// Unique splitting planes
	physx::serialize( stream, m_planes );

	// Combination data
	stream << m_combined;
	stream << m_combiningMeshSize;
}

void
BSP::deserialize( physx::PxFileBuf& stream )
{
	clear();

	physx::PxU32 version;
	stream >> version;

	// Tree
	m_root = deserializeNode( version, stream );

	if( version < Version::RevisedMeshTolerances )
	{
		stream.readDouble();	// Swallow old m_linearTol
		stream.readDouble();	// Swallow old m_angularTol
	}

	// Internal mesh representation
	if( version >= Version::SerializingTriangleFrames )
	{
		apex::deserialize( stream, version, m_mesh );
		physx::deserialize( stream, version, m_frames );
	}
	else
	{
		const physx::PxU32 triangleCount = stream.readDword();
		m_mesh.resize( triangleCount );
		m_frames.resize( triangleCount );
		for( physx::PxU32 triN = 0; triN < triangleCount; ++triN )
		{
			Triangle& tri = m_mesh[triN];
			if( version < Version::UsingOnlyPositionDataInVertex )
			{
				VertexData vertexData[3];
				for( physx::PxU32 v = 0; v < 3; ++v )
				{
					stream >> tri.vertices[v];
					stream >> vertexData[v].normal;
					stream >> vertexData[v].binormal;
					stream >> vertexData[v].binormal;
					for( physx::PxU32 uvN = 0; uvN < NxVertexFormat::MAX_UV_COUNT; ++uvN )
					{
						stream >> vertexData[v].uv[uvN];
					}
					stream >> vertexData[v].color;
				}
				m_frames[triN].setFromTriangle( tri, vertexData );
			}
			else
			{
				for( physx::PxU32 i = 0; i < 3; ++i )
				{
					physx::deserialize( stream, version, tri );
				}
			}
			stream >> tri.submeshIndex;
			stream >> tri.smoothingMask;
			stream >> tri.extraDataIndex;
			stream >> tri.normal;
			stream >> tri.area;
		}
	}
	stream >> m_meshSize;

	if( version < Version::RevisedMeshTolerances )
	{
		stream.readDouble();	// Swallow old m_distanceTol
	}

	// Unique splitting planes
	apex::deserialize( stream, version, m_planes );

	// Combination data
	stream >> m_combined;
	stream >> m_combiningMeshSize;

	if( m_root == NULL )
	{	// Set to trivial tree
		clear();
		m_root = m_memCache->m_nodePool.borrow();
#if !USE_GSA
		m_root->setLeafData( m_memCache->m_regionPool.borrow() );
		m_root->getLeafData()->side = 1;
#endif
	}
}

void BSP::visualize( NxApexRenderDebug& debugRender, physx::PxU32 flags, physx::PxU32 index ) const
{
#ifdef WITHOUT_DEBUG_VISUALIZE
	PX_FORCE_PARAMETER_REFERENCE(debugRender);
	PX_FORCE_PARAMETER_REFERENCE(flags);
	PX_FORCE_PARAMETER_REFERENCE(index);
#else
	const Node* node = m_root;
	if( flags&BSPVisualizationFlags::SingleRegion )
	{
		physx::PxU32 count = 0;
		node = findIndexedLeaf( m_root, index, count );
	}

	if( node != NULL )
	{
		visualizeNode( debugRender, flags, node );
	}
#endif
}

void
BSP::release()
{
	clear();
	delete this;
}

void
BSP::clear()
{
	if( m_root != NULL )
	{
		releaseNode( m_root );
		m_root = NULL;
	}
	m_combined = false;
	m_mesh.reset();
	m_frames.reset();
	m_planes.reset();
	removeBSPLink();
}

void
BSP::releaseNode( Node* node )
{
	PX_ASSERT( node != NULL );

	for( physx::PxU32 i = 0; i < 2; ++i )
	{
		Node* child = node->getChild(i);
		if( child != NULL )
		{
			child->detach();
			releaseNode( child );
		}
	}

#if !USE_GSA
	if( node->getType() == Node::Leaf )
	{
		if( node->getLeafData() )
		{
			m_memCache->m_regionPool.replace( node->getLeafData() );
		}
	}
	else
	{
		if( node->getBranchData() )
		{
			m_memCache->m_surfacePool.replace( node->getBranchData() );
		}
	}
#endif

	m_memCache->m_nodePool.replace( node );
}

#if USE_GSA
void
BSP::indexInsideLeaves( physx::PxU32& index, Node* node ) const
{
	if( node == NULL )
	{
		return;
	}

	if( node->getType() != Node::Leaf )
	{
		indexInsideLeaves( index, node->getChild(0) );
		indexInsideLeaves( index, node->getChild(1) );
		return;
	}

	if( node->getLeafData()->side == 0 )
	{	// Outside
		return;
	}

	node->getLeafData()->tempIndex1 = index++;
}

void
BSP::listInsideLeaves( physx::Array<Node*>& insideLeaves, Node* node ) const
{
	if( node == NULL )
	{
		return;
	}

	if( node->getType() != Node::Leaf )
	{
		listInsideLeaves( insideLeaves, node->getChild(0) );
		listInsideLeaves( insideLeaves, node->getChild(1) );
		return;
	}

	if( node->getLeafData()->side == 0 )
	{	// Outside
		return;
	}

	insideLeaves.pushBack( node );
}

void
BSP::findInsideLeafNeighbors( physx::Array<IntPair>& neighbors, LeafNeighborIntersection& intersection, Node* node ) const
{
	if( node == NULL )
	{
		return;
	}

	if( node->getType() != Node::Leaf )
	{
		PX_ASSERT( intersection.m_currentLeafPlaneCount == intersection.m_planes.size() );
		intersection.m_planes.pushBack( -(*(const physx::GSA::Plane<3,Real>*)(m_planes.begin()+node->getBranchData()->planeIndex)) );
		++intersection.m_currentLeafPlaneCount;
		findInsideLeafNeighbors( neighbors, intersection, node->getChild(0) );
		PX_ASSERT( intersection.m_currentLeafPlaneCount == intersection.m_planes.size() );
		intersection.m_planes.back() *= (Real)-1;
		findInsideLeafNeighbors( neighbors, intersection, node->getChild(1) );
		PX_ASSERT( intersection.m_currentLeafPlaneCount == intersection.m_planes.size() );
		intersection.m_planes.popBack();
		--intersection.m_currentLeafPlaneCount;
		return;
	}

	if( node->getLeafData()->side == 0 )
	{	// Outside
		return;
	}

	// First half of pair will always be node's index.
	IntPair pair;
	pair.i0 = (physx::PxI32)node->getLeafData()->tempIndex1;

	// Stackless walk of remainder of tree
	intersection.reset();
	bool up = true;
	while( node != m_root )
	{
		if( up )
		{
			up = (node->getIndex() == 1);
			node = node->getParent();
			if( intersection.m_planes.size() > intersection.m_currentLeafPlaneCount )
			{
				intersection.m_planes.popBack();
				intersection.reset();
			}
			if( !up )
			{
				intersection.m_planes.pushBack( (*(const physx::GSA::Plane<3,Real>*)(m_planes.begin()+node->getBranchData()->planeIndex)) );
				up = intersection.is_empty_set();	// Skip subtree if there is no intersection at this branch
				node = node->getChild( 1 );
			}
		}
		else
		{
			up = (node->getType() == Node::Leaf);
			if( !up )
			{
				intersection.m_planes.pushBack( -(*(const physx::GSA::Plane<3,Real>*)(m_planes.begin()+node->getBranchData()->planeIndex)) );
				up = intersection.is_empty_set();	// Skip subtree if there is no intersection at this branch
				node = node->getChild( 0 );
			}
			else
			{
				Region& region = *node->getLeafData();
				if( region.side == 1 )
				{	// We have found another inside leaf which intersects (at boundary)
					pair.i1 = region.tempIndex1;
					neighbors.pushBack( pair );
				}
			}
		}
	}
}
#endif // #if USE_GSA

void
BSP::complementLeaves( Node* node ) const
{
	if( node == NULL )
	{
		return;
	}

	if( node->getType() != Node::Leaf )
	{
		complementLeaves( node->getChild(0) );
		complementLeaves( node->getChild(1) );
		return;
	}

	node->getLeafData()->side = node->getLeafData()->side^1;
}

void
BSP::mergeLeaves( const BoolOp& op, Node* node )
{
	if( node == NULL )
	{
		return;
	}

	if( node->getType() != Node::Leaf )
	{
		Node* child0 = node->getChild(0);
		Node* child1 = node->getChild(1);

		mergeLeaves( op, child0 );
		mergeLeaves( op, child1 );

		// Can consolidate if the nodes are both leaves on the same side.
		PX_ASSERT( child0 != NULL && child1 != NULL );
		if( child0 != NULL && child1 != NULL && child0->getType() == Node::Leaf && child1->getType() == Node::Leaf )
		{
			Region* region0 = child0->getLeafData();
			Region* region1 = child1->getLeafData();

			PX_ASSERT( region0 != NULL && region1 != NULL );
			if( region0 != NULL && region1 != NULL )
			{
				PX_ASSERT( (region0->side&1) == region0->side && (region1->side&1) == region1->side );			
				if( region0->side == region1->side )
				{	// Consolidate

					// Delete children
					child0->detach();
					child1->detach();
#if !USE_GSA
					m_memCache->m_nodePool.replace( child0 );
					m_memCache->m_nodePool.replace( child1 );

					// Delete surface and region1 - will re-use region0
					m_memCache->m_surfacePool.replace( node->getBranchData() );
					m_memCache->m_regionPool.replace( region1 );
#endif

					// Turn this node into a leaf
#if USE_GSA
					node->setLeafData( *region0 );
					m_memCache->m_nodePool.replace( child0 );
					m_memCache->m_nodePool.replace( child1 );
#else
					node->setLeafData( region0 );
					region0->geom.setToAllSpace();	// Don't calculate yet - may get re-consolidated
#endif
				}
			}
		}

#if !USE_GSA
		if( node->getType() == Node::Branch )	// It may have been merged into a leaf above
		{
			for( physx::PxU32 index = 0; index < 2; ++index )
			{
				Node* child = node->getChild(index);
				if( child->getType() == Node::Leaf && child->getLeafData()->geom.isAllSpace() )
				{
					for( SurfaceIt it( child ); it.isValid(); it.inc() )
					{
						child->getLeafData()->geom.intersect( m_planes[it.surface()->planeIndex]*(it.side() ? (Real)1 : -(Real)1),
															  HULL_TOLERANCE_OFFSET*BASE_TOLERANCE*m_meshSize );
					}
				}
			}
		}
#endif

		return;
	}

	// Perform boolean operation
	const physx::PxU32 side = node->getLeafData()->side;
	node->getLeafData()->side = op( side&1, (side>>1)&1 );
}

Real
BSP::clipMeshToLeaf( physx::Array<Triangle>* clippedMesh, physx::Array<ClippedTriangleInfo>* triangleInfo, const Node* leaf ) const
{
	PX_ASSERT( leaf->getType() == BSP::Node::Leaf );

	Real area = (Real)0;

	// Collect triangles on each surface and clip to other faces
	for( SurfaceIt it( leaf ); it.isValid(); it.inc() )
	{
		for( physx::PxU32 i = it.surface()->triangleIndexStart; i < it.surface()->triangleIndexStop; ++i )
		{
			const physx::PxU32 oldClippedMeshSize = clippedMesh != NULL ? clippedMesh->size() : 0;
			area += clipTriangleToLeaf( clippedMesh, m_mesh[i], leaf, it.side(), m_memCache->m_linkedVertexPool,
										CLIP_TOLERANCE_OFFSET*BASE_TOLERANCE*m_meshSize, m_planes, it.surface()->planeIndex );
			if( triangleInfo != NULL && clippedMesh != NULL )
			{
				// Fill triangleInfo corresponding to new clipped triangles
				const physx::PxU32 newClippedMeshSize = clippedMesh->size();
				triangleInfo->resize( newClippedMeshSize );
				for( physx::PxU32 j = oldClippedMeshSize; j < newClippedMeshSize; ++j )
				{
					ClippedTriangleInfo& info = (*triangleInfo)[j];
					info.planeIndex = it.surface()->planeIndex;
					info.originalTriangleIndex = i;
					info.clippedTriangleIndex = j;
					info.ccw = it.side();
				}
			}
		}
	}

	return area;
}

void
BSP::clipMeshToLeaves( physx::Array<Triangle>& clippedMesh, physx::Array<ClippedTriangleInfo>& triangleInfo, Node* node ) const
{
	if( node == NULL )
	{
		return;
	}

	if( node->getType() != Node::Leaf )
	{
		clipMeshToLeaves( clippedMesh, triangleInfo, node->getChild(0) );
		clipMeshToLeaves( clippedMesh, triangleInfo, node->getChild(1) );
		return;
	}

	if( node->getLeafData()->side == 0 )
	{
		return;
	}

	clipMeshToLeaf( &clippedMesh, &triangleInfo, node );
}

void
BSP::transform( const Mat4Real& tm, bool transformFrames )
{
	// Build cofactor matrix for transformation of normals
	const Mat4Real cofTM = tm.cof34();
	const Mat4Real invTransposeTM = cofTM*((Real)1/cofTM[3][3]);

	// Transform mesh
	for( physx::PxU32 i = 0; i < m_mesh.size(); ++i )
	{
		for( int v = 0; v < 3; ++v )
		{
			m_mesh[i].vertices[v] = tm*m_mesh[i].vertices[v];
		}
		m_mesh[i].calculateQuantities();
		if( transformFrames )
		{
			m_frames[i].transform( m_frames[i], tm, invTransposeTM );
		}
	}

	// Transform planes
	for( physx::PxU32 i = 0; i < m_planes.size(); ++i )
	{
		m_planes[i] = cofTM*m_planes[i];	// Don't normalize yet - surface areas will be calculated from plane normal lengths in "Transform tree" below
	}

	// Transform tree
	if( m_root )
	{
		transform( m_root, tm, cofTM );
	}

	// Now normalize planes
	for( physx::PxU32 i = 0; i < m_planes.size(); ++i )
	{
		m_planes[i].normalize();
	}

	// Adjust sizes
	const Real scale = NxMath::pow( cofTM[3][3], (Real)0.33333333333333333 );
	m_meshSize *= scale;
	m_combiningMeshSize *= scale;
}

void
BSP::clone( Node* node, const Node* original, bool duplicateData )
{
#if !USE_GSA
	switch( original->getType() )
	{
	default:
		node->setLeafData( NULL );
		break;
	case Node::Leaf:
		if( duplicateData && original->getLeafData() )
		{
			Region* region = m_memCache->m_regionPool.borrow();
			*region = *original->getLeafData();
			node->setLeafData( region );
		}
		else
		{
			node->setLeafData( original->getLeafData() );
		}
		break;
	case Node::Branch:
		if( duplicateData && original->getBranchData() != NULL )
		{
			Surface* surface = m_memCache->m_surfacePool.borrow();
			*surface = *original->getBranchData();
			node->setBranchData( surface );
		}
		else
		{
			node->setBranchData( original->getBranchData() );
		}
		break;
	}
#else
	(void)duplicateData;
	switch( original->getType() )
	{
	case Node::Leaf:
		node->setLeafData( *original->getLeafData() );
		break;
	case Node::Branch:
		node->setBranchData( *original->getBranchData() );
		break;
	}
#endif

	for( physx::PxU32 i = 0; i < 2; ++i )
	{
		const Node* originalChild = original->getChild(i);
		if( originalChild != NULL )
		{
			Node* child = m_memCache->m_nodePool.borrow();
			node->setChild( i, child );
			clone( child, originalChild, duplicateData );
		}
	}
}

#if !USE_GSA
void
BSP::combineTrees( Node* node, const Node* combineNode, physx::PxU32 triangleIndexOffset, physx::PxU32 planeIndexOffset )
{
	if( node->getType() != Node::Leaf )
	{
		combineTrees( node->getChild(0), combineNode, triangleIndexOffset, planeIndexOffset );
		combineTrees( node->getChild(1), combineNode, triangleIndexOffset, planeIndexOffset );
		return;
	}

	PX_ASSERT( (combineNode->getChild(0) == NULL) == (combineNode->getChild(1) == NULL) );

	if( combineNode->getType() != Node::Leaf )
	{	// Branch node

		// Copy branch data, and offset the triangle indices
		Surface* branchSurface = m_memCache->m_surfacePool.borrow();
		Surface* combineBranchSurface = combineNode->getBranchData();
		branchSurface->planeIndex = combineBranchSurface->planeIndex + planeIndexOffset;
		branchSurface->triangleIndexStart = combineBranchSurface->triangleIndexStart + triangleIndexOffset;
		branchSurface->triangleIndexStop = combineBranchSurface->triangleIndexStop + triangleIndexOffset;
		branchSurface->totalTriangleArea = combineBranchSurface->totalTriangleArea;

		// Build child region geometry
		Region* regions[2];
		for( physx::PxU32 index = 0; index < 2; ++index )
		{
			regions[index] = m_memCache->m_regionPool.borrow();
			*regions[index] = *node->getLeafData();
			regions[index]->geom.intersect( m_planes[branchSurface->planeIndex]*(index ? (Real)1 : -(Real)1), HULL_TOLERANCE_OFFSET*BASE_TOLERANCE*m_meshSize );
		}

		if( !regions[0]->geom.isEmptySet() && !regions[1]->geom.isEmptySet() )
		{	// We need both branches
			// Turn this leaf into a branch
			m_memCache->m_regionPool.replace( node->getLeafData() );
			node->setBranchData( branchSurface );
			// Build children
			for( physx::PxU32 index = 0; index < 2; ++index )
			{
				Node* child = m_memCache->m_nodePool.borrow();
				child->setLeafData( regions[index] );
				node->setChild( index, child );
				combineTrees( child, combineNode->getChild(index), triangleIndexOffset, planeIndexOffset );
			}
		}
		else
		{
			// This leaf will not be split by the combining branch.  Return the new branch surface.
			m_memCache->m_surfacePool.replace( branchSurface );
			m_memCache->m_regionPool.replace( regions[0] );
			m_memCache->m_regionPool.replace( regions[1] );

			// Collapse tree by following one branch.
			if( !regions[0]->geom.isEmptySet() )
			{
				combineTrees( node, combineNode->getChild(0), triangleIndexOffset, planeIndexOffset );
			}
			else
			if( !regions[1]->geom.isEmptySet() )
			{
				combineTrees( node, combineNode->getChild(1), triangleIndexOffset, planeIndexOffset );
			}
		}
	}
	else
	{	// Leaf node
		node->getLeafData()->side = node->getLeafData()->side | combineNode->getLeafData()->side<<1;
	}
}
#else	// #if !USE_GSA
void
BSP::combineTrees( Node* node, const Node* combineNode, physx::PxU32 triangleIndexOffset, physx::PxU32 planeIndexOffset, RegionShape& regionShape )
{
	if( node->getType() != Node::Leaf )
	{
		RegionShape regionShapes[2] = { regionShape, regionShape };
		combineTrees( node->getChild(0), combineNode, triangleIndexOffset, planeIndexOffset, regionShapes[0] );
		combineTrees( node->getChild(1), combineNode, triangleIndexOffset, planeIndexOffset, regionShapes[1] );
		return;
	}

	PX_ASSERT( (combineNode->getChild(0) == NULL) == (combineNode->getChild(1) == NULL) );

	if( combineNode->getType() != Node::Leaf )
	{	// Branch node

		// Copy branch data, and offset the triangle indices
		Surface branchSurface;
		const Surface* combineBranchSurface = combineNode->getBranchData();
		branchSurface.planeIndex = combineBranchSurface->planeIndex + planeIndexOffset;
		branchSurface.triangleIndexStart = combineBranchSurface->triangleIndexStart + triangleIndexOffset;
		branchSurface.triangleIndexStop = combineBranchSurface->triangleIndexStop + triangleIndexOffset;
		branchSurface.totalTriangleArea = combineBranchSurface->totalTriangleArea;

		// Region geometry test
		RegionShape regionShapes[2] = { regionShape, regionShape };

		// Store off old leaf data
		Region oldRegion = *node->getLeafData();

		// Turn this leaf into a branch
		node->setBranchData( branchSurface );
		for( physx::PxU32 index = 0; index < 2; ++index )
		{
			Node* child = m_memCache->m_nodePool.borrow();
			child->setLeafData( oldRegion );
			node->setChild( index, child );
			regionShapes[index].set_leaf( child );
			regionShapes[index].calculate();
		}

		if( regionShapes[0].is_nonempty() && regionShapes[1].is_nonempty() )
		{	// We need both branches
			for( physx::PxU32 index = 0; index < 2; ++index )
			{
				combineTrees( node->getChild( index ), combineNode->getChild(index), triangleIndexOffset, planeIndexOffset, regionShapes[index] );
			}
		}
		else
		{
			// Leaf not be split by the combining branch.  Return the new branch surface.
			for( physx::PxU32 index = 0; index < 2; ++index )
			{
				Node* child = node->getChild( index );
				node->setChild( index, NULL );
				m_memCache->m_nodePool.replace( child );
			}
			// Turn this branch back into a leaf
			node->setLeafData( oldRegion );
			// Collapse tree by following one branch.
			if( regionShapes[0].is_nonempty() )
			{
				combineTrees( node, combineNode->getChild(0), triangleIndexOffset, planeIndexOffset, regionShapes[0] );
			}
			else
			if( regionShapes[1].is_nonempty() )
			{
				combineTrees( node, combineNode->getChild(1), triangleIndexOffset, planeIndexOffset, regionShapes[1] );
			}
		}
	}
	else
	{	// Leaf node
		node->getLeafData()->side = node->getLeafData()->side | combineNode->getLeafData()->side<<1;
	}
}
#endif	// #if !USE_GSA

bool
#if !USE_GSA
BSP::buildTree( Node* node, physx::Array<Surface*>& surfaceStack, physx::PxU32 stackReadStart, physx::PxU32 stackReadStop,
#else
BSP::buildTree( Node* node, physx::Array<Surface>& surfaceStack, physx::PxU32 stackReadStart, physx::PxU32 stackReadStop,
#endif
			    const BuildConstants& buildConstants, IProgressListener* progressListener )
{
	if( node == NULL )
	{
		return false;
	}

	const physx::PxU32 inputStackSize = surfaceStack.size();

	Region* region = node->getLeafData();

	// Remove surfaces that don't have triangles intersecting this region
	for( physx::PxU32 i = stackReadStop; i-- > stackReadStart; )
	{
#if !USE_GSA
		Surface* surface = surfaceStack[i];
#else
		Surface* surface = surfaceStack.begin()+i;
#endif
		bool surfaceIntersectsThisRegion = false;
		for( physx::PxU32 j = surface->triangleIndexStart; j < surface->triangleIndexStop; ++j )
		{
			if( 0 < clipTriangleToLeaf( NULL, m_mesh[j], node, 1, m_memCache->m_linkedVertexPool, CLIP_TOLERANCE_OFFSET*BASE_TOLERANCE, m_planes ) )
			{
				surfaceIntersectsThisRegion = true;
				break;
			}
		}
		if( !surfaceIntersectsThisRegion )
		{
#if !USE_GSA
			m_memCache->m_surfacePool.replace( surface );
#endif
			surfaceStack[i] = surfaceStack[--stackReadStop];
		}
	}

	if( stackReadStop == stackReadStart )
	{
		// See if this leaf is inside or outside
		Real sumSignedArea = (Real)0;
		for( SurfaceIt it( node ); it.isValid(); it.inc() )
		{
			const Real sign = it.side() ? (Real)1 : -(Real)1;
			for( physx::PxU32 j = it.surface()->triangleIndexStart; j < it.surface()->triangleIndexStop; ++j )
			{
				sumSignedArea += sign*clipTriangleToLeaf( NULL, m_mesh[j], node, it.side(), m_memCache->m_linkedVertexPool, CLIP_TOLERANCE_OFFSET*BASE_TOLERANCE, m_planes, it.surface()->planeIndex );
			}
		}

		if( sumSignedArea != (Real)0 )
		{
			region->side = sumSignedArea > 0 ? 1 : 0;
		}

#if CSG_DEBUG_NODEPATH
		++sNodeCount;
#endif
		return true;
	}

	const physx::PxU32 surfaceListSize = stackReadStop - stackReadStart;
#if !USE_GSA
	Surface** surfaceList = surfaceStack.begin() + stackReadStart;
#else
	Surface* surfaceList = surfaceStack.begin() + stackReadStart;
#endif

	// Pick buildConstants.m_testSetSize surfaces
	const physx::PxU32 testSetSize = buildConstants.m_testSetSize > 0 ? NxMath::min( surfaceListSize, buildConstants.m_testSetSize ) : surfaceListSize;

#if !USE_GSA
	Surface* branchSurface = surfaceList[0];	// Will be the winning surface - default to 1st surface
#else
	physx::PxU32 branchSurfaceN = 0;	// Will be the winning surface - default to 1st surface
#endif
	physx::PxF32 minScore = PX_MAX_F32;	// Low score wins
	if( m_memCache->m_surfaceFlags.size() < surfaceListSize )
	{
		m_memCache->m_surfaceFlags.resize( surfaceListSize );
		m_memCache->m_surfaceTestFlags.resize( surfaceListSize );
	}

	for( physx::PxU32 i = 0; i < testSetSize; ++i )
	{
		// Test surface
#if !USE_GSA
		Surface* testSurface = surfaceList[i];
#else
		Surface* testSurface = surfaceList+i;
#endif
		physx::PxI32 counts[4] = {0,0,0,0};	// on, above, below, split
		physx::PxU32 triangleCount = 0;
		for( physx::PxU32 j = 0; j < surfaceListSize; ++j )
		{
			NxU8& flags = m_memCache->m_surfaceTestFlags[j];	// Whether this surface is above or below testSurface (or both)
			flags = 0;

			if( j == i )
			{
				continue;	// Don't score testSurface itself
			}

			// Surface to contribute to score
#if !USE_GSA
			Surface* surface = surfaceList[j];
#else
			Surface* surface = surfaceList+j;
#endif

			// Run through all triangles
			for( physx::PxU32 k = surface->triangleIndexStart; k < surface->triangleIndexStop; ++k )
			{
				const Triangle& tri = m_mesh[k];
				NxU8 triFlags = 0;
				for( physx::PxU32 v = 0; v < 3; ++v )
				{
					const int side = cmpPointToPlane( tri.vertices[v], m_planes[testSurface->planeIndex], BASE_TOLERANCE );
					triFlags |= (side&1)<<((1-side)>>1);	// 0 => 0, 1 => 1, -1 => 2
				}
				++counts[triFlags];
				flags |= triFlags;
			}

			triangleCount += surface->triangleIndexStop-surface->triangleIndexStart;
		}

		// Compute score = (surface area)/(max area) + (split weight)*(# splits)/(# triangles) + (imbalance weight)*|(# above) - (# below)|/(# triangles)
		const physx::PxF32 score = testSurface->totalTriangleArea*buildConstants.m_recipMaxArea +
							(buildConstants.m_splitWeight*counts[3] + buildConstants.m_imbalanceWeight*NxMath::abs( counts[1] - counts[2] ))/triangleCount;

		if( score < minScore )
		{	// We have a winner
#if !USE_GSA
			branchSurface = testSurface;
#else
			branchSurfaceN = i;
#endif
			minScore = score;
			memcpy( m_memCache->m_surfaceFlags.begin(), m_memCache->m_surfaceTestFlags.begin(), surfaceListSize*sizeof( m_memCache->m_surfaceFlags[0] ) );
		}
	}

	// Run through the surface flags and create below/above arrays on the stack.
	// These arrays will be contiguous with child[0] surfaces first.
	physx::PxU32 childReadStart[2];
	physx::PxU32 childReadStop[2];
	childReadStart[0] = surfaceStack.size();
	childReadStop[0] = childReadStart[0];
	surfaceStack.reserve( surfaceStack.size() + 2*surfaceListSize );
	for( physx::PxU32 j = 0; j < surfaceListSize; ++j )
	{
#if !USE_GSA
		Surface* surface = surfaceStack[j+stackReadStart];
		if( surface == branchSurface )
#else
		physx::PxU32 surfaceJ = j+stackReadStart;
		if( j == branchSurfaceN )
#endif
		{
			continue;
		}
		switch( m_memCache->m_surfaceFlags[j] )
		{
		case 0:
#if !USE_GSA
			m_memCache->m_surfacePool.replace( surface );
#endif
			break;
		case 1:
			surfaceStack.insert();
			surfaceStack.back() = surfaceStack[childReadStop[0]];
#if !USE_GSA
			surfaceStack[childReadStop[0]++] = surface;
#else
			surfaceStack[childReadStop[0]++] = surfaceStack[surfaceJ];
#endif
			break;
		case 2:
#if !USE_GSA
			surfaceStack.pushBack( surface );
#else
			surfaceStack.pushBack( surfaceStack[surfaceJ] );
#endif
			break;
		case 3:
			{
#if !USE_GSA
				Surface* newSurface = m_memCache->m_surfacePool.borrow();
				*newSurface = *surface;
				surfaceStack.insert();
				surfaceStack.back() = surfaceStack[childReadStop[0]];
				surfaceStack[childReadStop[0]++] = surface;
				surfaceStack.pushBack( newSurface );
#else
				surfaceStack.insert();
				surfaceStack.back() = surfaceStack[childReadStop[0]];
				surfaceStack[childReadStop[0]++] = surfaceStack[surfaceJ];
				surfaceStack.pushBack( surfaceStack[surfaceJ] );
#endif
			}
			break;
		}
	}
	childReadStart[1] = childReadStop[0];
	childReadStop[1] = surfaceStack.size();

	// Set branch data to winning surface
#if !USE_GSA
	node->setBranchData( branchSurface );
#else
	node->setBranchData( surfaceStack[branchSurfaceN+stackReadStart] );
#endif

	// Create one new region (will re-use node's old region for one child)
#if !USE_GSA
	Region* regions[2] = { region, m_memCache->m_regionPool.borrow() };
	*regions[1] = *regions[0];	// Copy geometry
#else
	Region regions[2] = { *region, *region };
#endif

#if CSG_DEBUG_NODEPATH
	++sNodeMark;
	if( sNodeStack == NULL )
	{
		sNodeStack = new physx::Array<char>( 1, '\0' );
	}
	sNodeStack->pushBack( '\0' );
#endif

	HierarchicalProgressListener localProgressListener( childReadStop[0] - childReadStart[0] + childReadStop[1] - childReadStart[1], progressListener );

	// Build children
	for( physx::PxU32 index = 0; index < 2; ++index )
	{
#if CSG_DEBUG_NODEPATH
		PX_ASSERT( sNodeStack->size() >= 2 );
		if( sNodeStack->size() >= 2 )
		{
			(*sNodeStack)[sNodeStack->size()-2] = '0'+(char)index;
		}
		if( (size_t)(sNodeMark-sNodePath) <= strlen( sNodePath ) && !strncmp( sNodePath, sNodeStack->begin(), (size_t)(sNodeMark-sNodePath) ) )
		{
			NxGetApexSDK()->getOutputStream()->reportError( PxErrorCode::eDEBUG_INFO, sNodeStack->begin(), __FILE__, __LINE__ );
			if( (size_t)(sNodeMark-sNodePath) == strlen( sNodePath ) )
			{
				NxGetApexSDK()->getOutputStream()->reportError( PxErrorCode::eDEBUG_INFO, "***", __FILE__, __LINE__ );
			}
		}
#endif
#if !USE_GSA
		regions[index]->geom.intersect( m_planes[branchSurface->planeIndex]*(index ? (Real)1 : -(Real)1), HULL_TOLERANCE_OFFSET*BASE_TOLERANCE );	// mesh size has been normalized
		regions[index]->side = index;
#else
		regions[index].side = index;
#endif
		Node* child = m_memCache->m_nodePool.borrow();
		child->setLeafData( regions[index] );
		node->setChild( index, child );
		localProgressListener.setSubtaskWork( childReadStop[index]-childReadStart[index] );
		buildTree( child, surfaceStack, childReadStart[index], childReadStop[index], buildConstants, &localProgressListener );
		localProgressListener.completeSubtask();
	}

#if CSG_DEBUG_NODEPATH
	sNodeStack->popBack();
	if( sNodeStack->size() )
	{
		sNodeStack->back() = '\0';
	}
	--sNodeMark;
#endif

	surfaceStack.resize( inputStackSize );

	return true;
}

void
BSP::transform( Node* node, const Mat4Real& tm, const Mat4Real& cofTM )
{
	if( node == NULL )
	{
		return;
	}

	if( node->getType() != Node::Leaf )
	{
		// Transform surface quantities
		node->getBranchData()->totalTriangleArea *= (physx::PxF32)NxMath::sqrt( m_planes[node->getBranchData()->planeIndex].normal().lengthSquared() );

		transform( node->getChild(0), tm, cofTM );
		transform( node->getChild(1), tm, cofTM );
	}
	else
	{
#if !USE_GSA
		node->getLeafData()->geom.transform( tm, cofTM );
#endif
	}
}

#if !USE_GSA
void
BSP::addLeafVolumes( const Node* node, Real volumes[2], const BoolOp& op ) const
{
	if( node == NULL )
	{
		return;
	}

	if( node->getType() != Node::Leaf )
	{
		addLeafVolumes( node->getChild(0), volumes, op );
		addLeafVolumes( node->getChild(1), volumes, op );
		return;
	}

	Region* region = node->getLeafData();

	physx::PxU32 side = region->side;
	if( m_combined )
	{
		side = op( side&1, (side>>1)&1 );
	}

	side &= 1;	// Should not be needed, but it's cheap and safe.

	if( volumes[side] == MAX_REAL )
	{	// Already at maximum, no need to calculate the region volume
		return;
	}

	const Real volume = region->geom.calculateVolume();

	if( volume == MAX_REAL )
	{
		volumes[side] = MAX_REAL;
	}
	else
	{
		volumes[side] += volume;
	}
}
#endif

Real
BSP::addInsideLeafTriangleAreas( const Node* node, const BoolOp& op ) const
{
	if( node == NULL )
	{
		return (Real)0;
	}

	if( node->getType() != Node::Leaf )
	{
		Real area = (Real)0;
		area += addInsideLeafTriangleAreas( node->getChild(0), op );
		area += addInsideLeafTriangleAreas( node->getChild(1), op );
		return area;
	}

	const Region* region = node->getLeafData();

	physx::PxU32 side = region->side;
	if( m_combined )
	{
		side = op( side&1, (side>>1)&1 );
	}

	return (side&1) ? clipMeshToLeaf( NULL, NULL, node ) : (Real)0;
}

void
BSP::serializeNode( const Node* node, physx::PxFileBuf& stream ) const
{
	if( node != NULL )
	{
		stream << (physx::PxU32)1;

		stream << (physx::PxU32)node->getType();

		if( node->getType() != Node::Leaf )
		{
			physx::serialize( stream, *node->getBranchData() );
			serializeNode( node->getChild( 0 ), stream );
			serializeNode( node->getChild( 1 ), stream );
			return;
		}

		physx::serialize( stream, *node->getLeafData() );
	}
	else
	{
		stream << (physx::PxU32)0;
	}
}

BSP::Node*
BSP::deserializeNode( physx::PxU32 version, physx::PxFileBuf& stream )
{
	Node* node = NULL;

	physx::PxU32 createNode;
	stream >> createNode;

	if( createNode )
	{
		node = m_memCache->m_nodePool.borrow();

		physx::PxU32 nodeType;
		stream >> nodeType;
		
		if( nodeType != Node::Leaf )
		{
#if !USE_GSA
			Surface* surface = m_memCache->m_surfacePool.borrow();
			physx::deserialize( stream, version, *surface );
#else
			Surface surface;
			physx::deserialize( stream, version, surface );
#endif
			node->setBranchData( surface );
			node->setChild( 0, deserializeNode( version, stream ) );
			node->setChild( 1, deserializeNode( version, stream ) );
		}
		else
		{
#if !USE_GSA
			Region* region = m_memCache->m_regionPool.borrow();
			physx::deserialize( stream, version, *region );
#else
			Region region;
			physx::deserialize( stream, version, region );
#endif
	 		node->setLeafData( region );
		}
	}

	return node;
}

void BSP::visualizeNode( NxApexRenderDebug& debugRender, physx::PxU32 flags, const Node* node ) const
{
#if defined(WITHOUT_DEBUG_VISUALIZE)
	PX_FORCE_PARAMETER_REFERENCE(debugRender);
	PX_FORCE_PARAMETER_REFERENCE(flags);
	PX_FORCE_PARAMETER_REFERENCE(node);
#else
	if( node->getType() != Node::Leaf )
	{	// Branch
		for( physx::PxU32 index = 0; index < 2; ++index )
		{
			if( node->getChild( index ) != NULL )
			{
				visualizeNode( debugRender, flags, node->getChild( index ) );
			}
		}
		return;
	}

	bool showLeaf = false;
	physx::PxU32 color = 0;
	if( node->getLeafData()->side == 0 )
	{
		if( flags&(BSPVisualizationFlags::OutsideRegions|BSPVisualizationFlags::SingleRegion) )
		{
			showLeaf = true;
			color = 0xFF0000; // JWR: TODO
		}
	}
	else
	{
		if( flags&(BSPVisualizationFlags::InsideRegions|BSPVisualizationFlags::SingleRegion) )
		{
			showLeaf = true;
			color = 0x00FF00; // JWR: TODO
		}
	}

#if !USE_GSA
	const Real rayVisLength = (Real)1000;

	const Hull& geom = node->getLeafData()->geom;

	if( showLeaf )
	{
		debugRender.setCurrentColor( color );
		for( physx::PxU32 i = 0; i < geom.getEdgeCount(); ++i )
		{
			const Hull::Edge& edge = geom.getEdge(i);
			physx::PxVec3 p0, p1;
			switch( geom.getType( edge ) )
			{
			case Hull::EdgeType::LineSegment:
				PxVec3FromArray( p0, &(geom.getV0( edge )[0]) );
				PxVec3FromArray( p1, &(geom.getV1( edge )[0]) );
				break;
			case Hull::EdgeType::Ray:
				{
				PxVec3FromArray( p0, &(geom.getV0( edge )[0]) );
				const Pos e1 = geom.getV0( edge )+rayVisLength*geom.getDir( edge );
				PxVec3FromArray( p1, &e1[0] );
				}
				break;
			case Hull::EdgeType::Line:
				{
				const Pos e0 = geom.getV0( edge )-rayVisLength*geom.getDir( edge );
				PxVec3FromArray( p0, &e0[0] );
				const Pos e1 = geom.getV0( edge )+rayVisLength*geom.getDir( edge );
				PxVec3FromArray( p1, &e1[0] );
				}
				break;
			}
			debugRender.debugLine( p0, p1 );
		}
	}
#else	// #if !USE_GSA
	if( showLeaf )
	{
		debugRender.setCurrentColor( color );
		const Real clampSize = m_meshSize*10;
		for( SurfaceIt i( node ); i.isValid(); i.inc() )
		{
			const physx::GSA::Plane<3,Real>& plane_i = *(const physx::GSA::Plane<3,Real>*)(m_planes.begin()+i.surface()->planeIndex);
			SurfaceIt j = i;
			j.inc();
			for( ; j.isValid(); j.inc() )
			{
				const physx::GSA::Plane<3,Real>& plane_j = *(const physx::GSA::Plane<3,Real>*)(m_planes.begin()+j.surface()->planeIndex);
				// Find potential edge from intersection if plane_i and plane_j
				physx::GSA::Dir<3,Real> edgeDir = plane_i.n()^plane_j.n();
				const Real edgeDir2 = edgeDir.lengthSquared();
				if( edgeDir2 > square( physx::GSA::eps_real<Real>() ) )
				{
					const physx::GSA::Pos<3,Real> orig = physx::GSA::Pos<3,Real>() - (plane_i.d()*(plane_j.n()^edgeDir) + plane_j.d()*(edgeDir^plane_i.n()))/edgeDir2;
					edgeDir /= sqrt( edgeDir2 );
					RegionShape regionShape( (const physx::GSA::Plane<3,Real> *)m_planes.begin() );
					regionShape.set_leaf( node );
					physx::GSA::GSA<3,Real>::LineIntersect in, ex;
					regionShape.intersect_line( in, ex, orig, edgeDir, (Real)0 );
					if( in.m_s < ex.m_s )
					{
						in.m_s = NxMath::max( in.m_s, -clampSize );
						ex.m_s = NxMath::min( ex.m_s, clampSize );
						const physx::GSA::Pos<3,Real> e0 = orig + in.m_s*edgeDir;
						const physx::GSA::Pos<3,Real> e1 = orig + ex.m_s*edgeDir;
						physx::PxVec3 p0, p1;
						PxVec3FromArray( p0, &e0(0) );
						PxVec3FromArray( p1, &e1(0) );
						debugRender.debugLine( p0, p1 );
					}
				}
			}
		}
	}
#endif
#endif
}

const BSP::Node*
BSP::findIndexedLeaf( const Node* node, physx::PxU32 index, physx::PxU32& count ) const
{
	if( node->getType() == Node::Leaf )
	{
		if( index == count++ )
		{
			return node;
		}
		return NULL;
	}

	// Branch
	for( physx::PxU32 i = 0; i < 2; ++i )
	{
		if( node->getChild( i ) != NULL )
		{
			const Node* foundNode = findIndexedLeaf( node->getChild( i ), index, count );
			if( foundNode != NULL )
			{
				return foundNode;
			}
		}
	}
	
	return NULL;
}


/* For GSA */
#if USE_GSA
unsigned
BSP::RegionShape::initialize_tangent_planes( physx::GSA::Plane<3,Real>* planes, unsigned plane_count ) const
{
	if( m_planes == NULL )
	{
		return 0;
	}
	unsigned nPlanes = 0;
	for( SurfaceIt it( m_leaf ); it.isValid(); it.inc() )
	{
		if( nPlanes >= plane_count )
		{
			break;
		}
		const Real sign = it.side() ? (Real)1 : -(Real)1;
		planes[nPlanes] = sign*(*(const physx::GSA::Plane<3,Real>*)(m_planes+it.surface()->planeIndex));
		planes[nPlanes](3) -= m_skinWidth;
		++nPlanes;
	}
	return nPlanes;
}

void
BSP::RegionShape::intersect_line( physx::GSA::GSA<3,Real>::LineIntersect& in, physx::GSA::GSA<3,Real>::LineIntersect& ex, const physx::GSA::Pos<3,Real>& orig, const physx::GSA::Dir<3,Real>& dir, Real time ) const
{
	if( m_planes == NULL )
	{
		return;
	}
	(void)time;	// Not used
	Real in_num = -1.0f;
	Real in_den = 0.0f;
	Real ex_num = 1.0f;
	Real ex_den = 0.0f;
	in.m_plane.set( physx::GSA::Dir<3,Real>(), 0.0f );
	ex.m_plane.set( physx::GSA::Dir<3,Real>(), 0.0f );

	for( SurfaceIt it( m_leaf ); it.isValid(); it.inc() )
	{
		const Real sign = it.side() ? (Real)1 : -(Real)1;
		physx::GSA::Plane<3,Real> plane = sign*(*(const physx::GSA::Plane<3,Real>*)(m_planes+it.surface()->planeIndex));
		plane(3) -= m_skinWidth;
		const Real num = -(plane|orig);
		const Real den = (plane|dir);
		if( den > physx::GSA::gsa_eps<Real>() )
		{	// Exit
			if( num*ex_den < ex_num*den )	// num/den < ex_num/ex_den
			{
				ex_num = num;
				ex_den = den;
				ex.m_plane = plane;
			}
		}
		else
		if( den < -physx::GSA::gsa_eps<Real>() )
		{	// Entrance
			if( num*in_den < in_num*den )	// num/den > in_num/in_den
			{
				in_num = -num;
				in_den = -den;
				in.m_plane = plane;
			}
		}
		else
		if( num < physx::GSA::gsa_eps<Real>() )
		{	// Parallel and coincident or outside
			if( in_den == 0.0f )
			{
				in.m_plane = plane;
			}
			if( ex_den == 0.0f )
			{
				ex.m_plane = plane;
			}
			if( num < -physx::GSA::gsa_eps<Real>() )
			{	// Outside
				if( ex_den > physx::GSA::eps_real<Real>() || num < ex_num )
				{
					in_num = -num;
					ex_num = num;
					in_den = ex_den = physx::GSA::eps_real<Real>();
					in.m_plane = ex.m_plane = plane;
				}
			}
		}
	}

	in.m_s = in_den > 0.0f ? in_num/in_den : -physx::GSA::max_real<Real>();
	ex.m_s = ex_den > 0.0f ? ex_num/ex_den : physx::GSA::max_real<Real>();
}
#endif	// #if USE_GSA


/* BSPMemCache */

BSPMemCache::BSPMemCache() :
#if !USE_GSA
m_nodePool(10000),
m_surfacePool(10000),
m_regionPool(10000)
#else
m_nodePool(10000)
#endif
{
}

void
BSPMemCache::clearAll()
{
	const physx::PxI32 nodesRemaining = m_nodePool.empty();
#if !USE_GSA
	const physx::PxI32 surfacesRemaining = m_surfacePool.empty();
	const physx::PxI32 regionsRemaining = m_regionPool.empty();
#endif

	char message[1000];
	if( nodesRemaining != 0 )
	{
		physx::string::sprintf_s( message, 1000, "BSPMemCache: %d nodes %sfreed ***", physx::PxAbs( nodesRemaining ), nodesRemaining > 0 ? "un" : "over" );
		NxGetApexSDK()->getOutputStream()->reportError( PxErrorCode::eDEBUG_INFO, message, __FILE__,__LINE__ );
	}
#if !USE_GSA
	if( surfacesRemaining != 0 )
	{
		physx::string::sprintf_s( message, 1000, "BSPMemCache: %d surfaces %sfreed ***", physx::PxAbs( surfacesRemaining ), surfacesRemaining > 0 ? "un" : "over" );
		NxGetApexSDK()->getOutputStream()->reportError( PxErrorCode::eDEBUG_INFO, message, __FILE__,__LINE__ );
	}
	if( regionsRemaining != 0 )
	{
		physx::string::sprintf_s( message, 1000, "BSPMemCache: %d regions %sfreed ***", physx::PxAbs( regionsRemaining ), regionsRemaining > 0 ? "un" : "over" );
		NxGetApexSDK()->getOutputStream()->reportError(PxErrorCode::eDEBUG_INFO, message, __FILE__,__LINE__ );
	}
#endif

	clearTemp();
}

void
BSPMemCache::clearTemp()
{
	m_surfaceFlags.reset();
	m_surfaceTestFlags.reset();
	const physx::PxI32 linkedVerticesRemaining = m_linkedVertexPool.empty();

	char message[1000];
	if( linkedVerticesRemaining != 0 )
	{
		physx::string::sprintf_s( message, 1000, "BSPMemCache: %d linked vertices %sfreed ***", physx::PxAbs( linkedVerticesRemaining ), linkedVerticesRemaining > 0 ? "un" : "over" );
		NxGetApexSDK()->getOutputStream()->reportError( PxErrorCode::eDEBUG_INFO, message, __FILE__,__LINE__ );
	}
}

void
BSPMemCache::release()
{
	clearAll();
	delete this;
}


/* CSG Tools API */

IApexBSPMemCache*
createBSPMemCache()
{
	return PX_NEW(BSPMemCache)();
}

IApexBSP*
createBSP( IApexBSPMemCache* memCache )
{
	return PX_NEW(BSP)( memCache );
}

}
}
#endif