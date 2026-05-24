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
//#ifdef _MANAGED
//#pragma managed(push, off)
//#endif

#include <stdarg.h>
#include "NxApex.h"
#include "ApexSharedUtils.h"

#include "authoring/Fracturing.h"
#include "PsUserAllocated.h"
#include "ApexRand.h"
#include "PxUserOutputStream.h"
#include "PsString.h"
#include "PsShare.h"

#include <stdio.h>

#ifndef WITHOUT_APEX_AUTHORING

#define MAX_ALLOWED_ESTIMATED_CHUNK_TOTAL	10000

#define DEFAULT_CLEANING_TOLERANCE	0.0000001f

static bool gIslandGeneration = false;
static int	gVerbosity = 0;

PX_INLINE bool segmentOnBorder( const physx::PxVec3& v0, const physx::PxVec3& v1, physx::PxF32 width, physx::PxF32 height )
{
	return
		(v0.x < -0.5f && v1.x < -0.5f) ||
		(v0.y < -0.5f && v1.y < -0.5f) ||
		(v0.x >= width-0.5f && v1.x >= width-0.5f) ||
		(v0.y >= height-0.5f && v1.y >= height-0.5f);
}

class Random : public physx::UserRandom
{
public:
	physx::PxU32	getInt()						{ return m_rnd.nextSeed(); }
	physx::PxF32	getReal( physx::PxF32 min, physx::PxF32 max )	{ return m_rnd.getScaled( min, max ); }

	physx::QDSRand	m_rnd;
} userRnd;

#if 0
static bool trianglesOverlap( const physx::PxVec3& pv00, const physx::PxVec3& pv01, const physx::PxVec3& pv02, const physx::PxVec3& pv10, const physx::PxVec3& pv11, const physx::PxVec3& pv12, physx::PxF32 eps )
{
	NxVec3 v00, v01, v02, v10, v11, v12;
	physx::NxFromPxVec3( v00, pv00 );
	physx::NxFromPxVec3( v01, pv01 );
	physx::NxFromPxVec3( v02, pv02 );
	physx::NxFromPxVec3( v10, pv10 );
	physx::NxFromPxVec3( v11, pv11 );
	physx::NxFromPxVec3( v12, pv12 );

	const NxVec3 e0[3] = { v01-v00, v02-v01, v00-v02 };
	NxPlane p0( v00, e0[2]^e0[0] );
	p0.normalize();

	// Face 0 check
	const physx::PxF32 d00 = p0.distance( v10 );
	const physx::PxF32 d01 = p0.distance( v11 );
	const physx::PxF32 d02 = p0.distance( v12 );
	if( (d00 >= eps && d01 >= eps && d02 >= eps) || (d00 <= -eps && d01 <= -eps && d02 <= -eps) )
	{
		return false;
	}

	const NxVec3 e1[3] = { v11-v10, v12-v11, v10-v12 };
	NxPlane p1( v10, e1[2]^e1[0] );
	p1.normalize();

	// Face 1 check
	const physx::PxF32 d10 = p1.distance( v00 );
	const physx::PxF32 d11 = p1.distance( v01 );
	const physx::PxF32 d12 = p1.distance( v02 );
	if( (d10 >= eps && d11 >= eps && d12 >= eps) || (d10 <= -eps && d11 <= -eps && d12 <= -eps) )
	{
		return false;
	}

	const NxVec3* v0[3] = { &v00, &v01, &v02 };
	const NxVec3* v1[3] = { &v10, &v11, &v12 };

	// Edge checks
	for( physx::PxU32 i = 0; i < 3; ++i )
	{
		for( physx::PxU32 j = 0; j < 3; ++j )
		{
			NxVec3 n = e0[i]^e1[j];
			const physx::PxF32 n2 = n.magnitudeSquared();
			if( n2 < NX_EPS_F32*NX_EPS_F32 )
			{
				continue;
			}
			n *= physx::NxRecipSqrt( n2 );
			const physx::PxF32 d0e = v0[i]->dot( n );
			const physx::PxF32 d0v = v0[(3>>i)^1]->dot( n );
			physx::PxF32 min0, max0;
			if( d0e < d0v )
			{
				min0 = d0e;
				max0 = d0v;
			}
			else
			{
				min0 = d0v;
				max0 = d0e;
			}
			const physx::PxF32 d1e = v1[j]->dot( n );
			const physx::PxF32 d1v = v1[(3>>j)^1]->dot( n );
			physx::PxF32 min1, max1;
			if( d1e < d1v )
			{
				min1 = d1e;
				max1 = d1v;
			}
			else
			{
				min1 = d1v;
				max1 = d1e;
			}
			if( physx::PxMax( min1-max0, min0-max1 ) >= eps )
			{
				return false;
			}
		}
	}

	return true;
}

#else

NX_INLINE bool edgesOverlap( const physx::PxVec3& pv00, const physx::PxVec3& pv01, const physx::PxVec3& pv10, const physx::PxVec3& pv11, physx::PxF32 eps )
{
	physx::PxVec3 e0 = pv01-pv00;
	physx::PxVec3 e1 = pv11-pv10;

	if( e0.dot( e1 ) < 0 )
	{
		return false;
	}

	physx::PxF32 l0 = e0.normalize();
	e1.normalize();

	const physx::PxVec3 disp0 = pv10-pv00;
	const physx::PxVec3 disp1 = pv11-pv00;

	const physx::PxF32 d10 = disp0.dot( e0 );

	const physx::PxF32 d11 = disp1.dot( e0 );

	if( d11 < -eps )
	{
		return false;
	}

	if( d10 > l0+eps )
	{
		return false;
	}

	const physx::PxF32 disp02 = disp0.dot( disp0 );
	if( disp02 - d10*d10 > eps*eps )
	{
		return false;
	}

	const physx::PxF32 disp12 = disp1.dot( disp1 );
	if( disp12 - d11*d11 > eps*eps )
	{
		return false;
	}

	return true;
}

NX_INLINE bool trianglesOverlap( const physx::PxVec3& pv00, const physx::PxVec3& pv01, const physx::PxVec3& pv02, const physx::PxVec3& pv10, const physx::PxVec3& pv11, const physx::PxVec3& pv12, physx::PxF32 eps )
{
	return	edgesOverlap( pv00, pv02, pv10, pv11, eps ) || edgesOverlap( pv00, pv02, pv11, pv12, eps ) || edgesOverlap( pv00, pv02, pv12, pv10, eps ) ||
			edgesOverlap( pv01, pv00, pv10, pv11, eps ) || edgesOverlap( pv01, pv00, pv11, pv12, eps ) || edgesOverlap( pv01, pv00, pv12, pv10, eps ) ||
			edgesOverlap( pv02, pv01, pv10, pv11, eps ) || edgesOverlap( pv02, pv01, pv11, pv12, eps ) || edgesOverlap( pv02, pv01, pv12, pv10, eps );
}

#endif

// Returns a point uniformly distributed on the "polar cap" in +axisN direction, of azimuthal size range (in radians)
PX_INLINE physx::PxVec3	randomNormal( physx::PxU32 axisN, physx::PxF32 range )
{
	physx::PxVec3 result;
	const physx::PxF32 cosTheta = 1.0f - (1.0f-physx::PxCos( range ))*userRnd.getReal( 0.0f, 1.0f );
	const physx::PxF32 sinTheta = physx::PxSqrt( 1.0f - cosTheta*cosTheta );
	physx::PxF32 cosPhi, sinPhi;
	NxMath::sinCos( userRnd.getReal( -NxPiF32, NxPiF32 ), sinPhi, cosPhi );
	result[axisN%3] = cosTheta;
	result[(axisN+1)%3] = cosPhi*sinTheta;
	result[(axisN+2)%3] = sinPhi*sinTheta;
	return result;
}

void calculatePartition( int partition[3], const unsigned requestedSplits[3], const physx::PxVec3& extent, const float* targetProportions )
{
	partition[0] = requestedSplits[0]+1;
	partition[1] = requestedSplits[1]+1;
	partition[2] = requestedSplits[2]+1;

	if( targetProportions != NULL )
	{
		physx::PxVec3 n( extent[0]/targetProportions[0], extent[1]/targetProportions[1], extent[2]/targetProportions[2] );
		n *= physx::PxVec3( (physx::PxF32)partition[0], (physx::PxF32)partition[1], (physx::PxF32)partition[2] ).dot( n )/n.magnitudeSquared();
		// n now contains the # of partitions per axis closest to the desired # of partitions
		// which give the correct target proportions. However, the numbers will not (in general)
		// be integers, so round:
		partition[0] = physx::PxMax( 1, (int)(n[0]+0.5f) );
		partition[1] = physx::PxMax( 1, (int)(n[1]+0.5f) );
		partition[2] = physx::PxMax( 1, (int)(n[2]+0.5f) );
	}
}

static void outputMessage( const char* message, physx::PxErrorCode::Enum errorCode = physx::PxErrorCode::eNO_ERROR, int verbosity = 0 )	// Lower # = higher priority
{
	if( verbosity > gVerbosity )
	{
		return;
	}

	physx::PxUserOutputStream* outputStream = physx::NxGetApexSDK()->getOutputStream();
	if( outputStream )
	{
		outputStream->reportError(errorCode, message,__FILE__,__LINE__ );
	}
}

struct PartIndexer
{
	physx::ExplicitHierarchicalMesh::Part*	part;
	physx::PxI32							parentIndex;
	physx::PxI32							index;

	static int compareParentIndices( const void* A, const void* B )
	{
		const int diff = ((const PartIndexer*)A)->parentIndex - ((const PartIndexer*)B)->parentIndex;
		if( diff )
		{
			return diff;
		}
		return ((const PartIndexer*)A)->index - ((const PartIndexer*)B)->index;
	}
};

static physx::PxBounds3 boundTriangles( const physx::Array<physx::NxExplicitRenderTriangle>& triangles, const physx::PxMat34Legacy& interiorTM )
{
	physx::PxBounds3 bounds;
	bounds.setEmpty();
	for( physx::PxU32 triangleN = 0; triangleN < triangles.size(); ++triangleN )
	{
		for( int v = 0; v < 3; ++v )
		{
			physx::PxVec3 localVert;
			interiorTM.multiplyByInverseRT( triangles[triangleN].vertices[v].position, localVert );
			bounds.include( localVert );
		}
	}
	return bounds;
}

typedef physx::PxF32 (*NoiseFn)( physx::PxF32 x, physx::PxF32 y, physx::PxF32& xGrad, physx::PxF32& yGrad );

static physx::PxF32 planeWave( physx::PxF32 x, physx::PxF32, physx::PxF32& xGrad, physx::PxF32& yGrad )
{
	physx::PxF32 c, s;
	NxMath::sinCos( x, s, c );
	xGrad = c;
	yGrad = 0.0f;
	return s;
}

static NoiseFn noiseFns[] =
{
	planeWave
};

static int noiseFnCount = sizeof(noiseFns)/sizeof(noiseFns[0]);

// noiseDir = 0 => X
// noiseDir = 1 => Y
// noiseDir = -1 => userRnd
void physx::IntersectMesh::build( GridPattern pattern, const NxPlane& plane,
						   physx::PxF32 cornerX, physx::PxF32 cornerY, physx::PxF32 xSpacing, physx::PxF32 ySpacing, physx::PxU32 numX, physx::PxU32 numY,
						   const physx::PxMat34Legacy& tm, float noiseAmplitude, float relativeFrequency, bool periodicNoise,
						   int noiseType, int noiseDir, physx::PxU32 submeshIndex, physx::PxU32 frameIndex, const physx::TriangleFrame& triangleFrame )
{
	m_pattern = pattern;
	m_plane = plane;
	m_cornerX = cornerX;
	m_cornerY = cornerY;
	m_xSpacing = xSpacing;
	m_ySpacing = ySpacing;
	m_numX = numX;
	m_numY = numY;
	m_tm = tm;

	if( relativeFrequency == 0.0f )
	{	// 0 frequency only provides a plane offset
		m_plane.d += userRnd.getReal( -noiseAmplitude, noiseAmplitude );
		noiseAmplitude = 0.0f;
	}

	if( noiseAmplitude == 0.0f )
	{	// Without noise, we only need one triangle
		m_pattern = Equilateral;
		m_vertices.resize(3);
		m_triangles.resize(1);

		const physx::PxF32 rX = 0.5f*(xSpacing*numX);
		const physx::PxF32 rY = 0.5f*(ySpacing*numY);
		const physx::PxF32 centerX = cornerX + rX;
		const physx::PxF32 centerY = cornerY + rY;

		// Circumscribe rectangle
		const physx::PxF32 R = physx::PxSqrt( rX*rX + rY*rY );

		// Fit equilateral triangle around circle
		const physx::PxF32 x = 1.73205081f*R;
		tm.multiply( physx::PxVec3( centerX, centerY+2*R, 0 ), m_vertices[0].position );
		tm.multiply( physx::PxVec3( centerX-x, centerY-R, 0 ), m_vertices[1].position );
		tm.multiply( physx::PxVec3( centerX+x, centerY-R, 0 ), m_vertices[2].position );

		for( physx::PxU32 i = 0; i < 3; ++i )
		{
			PxFromNxVec3(m_vertices[i].normal, m_plane.normal);
			m_vertices[i].tangent = tm.M.getColumn(0);
			m_vertices[i].binormal = tm.M.getColumn(1);
			m_vertices[i].color = PxColorRGBA( 255, 255, 255, 255 );
		}

		NxExplicitRenderTriangle& triangle = m_triangles[0];
		for( physx::PxU32 v = 0; v < 3; ++v )
		{
			NxVertex& gridVertex = m_vertices[v];
			triangle.vertices[v] = gridVertex;
			triangleFrame.interpolateVertexData( triangle.vertices[v] );
			// Only really needed to interpolate u,v... replace normals and tangents with proper ones
			triangle.vertices[v].normal = gridVertex.normal;
			triangle.vertices[v].tangent = gridVertex.tangent;
			triangle.vertices[v].binormal = gridVertex.binormal;
		}
		triangle.extraDataIndex = frameIndex;
		triangle.smoothingMask = 0;
		triangle.submeshIndex = submeshIndex;

		return;
	}

	physx::PxVec3 corner;
	m_tm.multiply( physx::PxVec3( m_cornerX, m_cornerY, 0 ), corner );
	const physx::PxVec3 localX = m_tm.M.getColumn(0)*m_xSpacing;
	const physx::PxVec3 localY = m_tm.M.getColumn(1)*m_ySpacing;
	const physx::PxVec3 localZ = m_tm.M.getColumn(2);

	// Vertices:
	m_vertices.resize( (m_numX+1)*(m_numY+1) );
	const physx::PxVec3 halfLocalX = 0.5f*localX;
	physx::PxU32 pointN = 0;
	physx::PxVec3 side = corner;
	for( physx::PxU32 iy = 0; iy <= m_numY; ++iy, side += localY )
	{
		physx::PxVec3 point = side;
		for( physx::PxU32 ix = 0; ix <= m_numX; ++ix, point += localX )
		{
			if( m_pattern == physx::IntersectMesh::Equilateral && (((iy&1) == 0 && ix == m_numX) || ((iy&1) != 0 && ix == 1)) )
			{
				point -= halfLocalX;
			}
			NxVertex& vertex = m_vertices[pointN++];
			vertex.position = point;
			vertex.normal = physx::PxVec3::zero();
		}
	}

	// add noise:
	NoiseFn noiseFn = noiseFns[physx::PxClamp( noiseType, 0 , noiseFnCount-1)];

	const physx::PxU32 numModes = 20;
	for( physx::PxU32 i = 0; i < numModes; ++i )
	{
		physx::PxF32 amplitude = noiseAmplitude/physx::PxSqrt( (physx::PxF32)numModes );	// Scale by frequency?
		physx::PxF32 phase = userRnd.getReal( -3.14159265f, 3.14159265f );
		physx::PxF32 freqShift = userRnd.getReal( 0.0f, 3.0f );
		physx::PxF32 kx, ky;
		switch( noiseDir )
		{
		case 0:
			kx = physx::PxPow( 2.0f, freqShift )*relativeFrequency/m_xSpacing;
			ky = 0.0f;
			break;
		case 1:
			kx = 0.0f;
			ky = physx::PxPow( 2.0f, freqShift )*relativeFrequency/m_ySpacing;
			break;
		default:
			{
				const physx::PxF32 f = physx::PxPow( 2.0f, freqShift )*relativeFrequency;
				const physx::PxF32 theta = userRnd.getReal( -3.14159265f, 3.14159265f );
				const physx::PxF32 c = physx::PxCos( theta );
				const physx::PxF32 s = physx::PxSin( theta );
				kx = c*f/m_xSpacing;
				ky = s*f/m_ySpacing;
			}
		}

		if( periodicNoise )
		{	// Make sure the wavenumbers are integers
			const float cx = (2.0f*3.14159265f)/(m_numX*m_xSpacing);
			const float cy = (2.0f*3.14159265f)/(m_numY*m_ySpacing);
			const int nx = (int)PxSign(kx)*(int)(physx::PxAbs(kx)/cx + 0.5f);	// round
			const int ny = (int)PxSign(ky)*(int)(physx::PxAbs(ky)/cy + 0.5f);	// round
			kx = nx*cx;
			ky = ny*cy;
		}

		pointN = 0;
		physx::PxF32 y = m_cornerY;
		for( physx::PxU32 iy = 0; iy <= m_numY; ++iy, y += m_ySpacing )
		{
			physx::PxF32 x = m_cornerX;
			for( physx::PxU32 ix = 0; ix <= m_numX; ++ix, x += m_xSpacing )
			{
				if( m_pattern == physx::IntersectMesh::Equilateral && (((iy&1) == 0 && ix == m_numX) || ((iy&1) != 0 && ix == 1)) )
				{
					x -= 0.5f*m_xSpacing;
				}
				NxVertex& vertex = m_vertices[pointN++];
				physx::PxF32 xGrad, yGrad;
				physx::PxF32 f = amplitude*noiseFn( x*kx + y*ky - phase, 0, xGrad, yGrad );
				vertex.position += localZ*f;
				vertex.normal += physx::PxVec3( -xGrad*kx*amplitude, -yGrad*ky*amplitude, 0.0f );
			}
		}
	}

	// Normalize normals and put in correct frame
	for( pointN = 0; pointN < m_vertices.size(); pointN++ )
	{
		NxVertex& vertex = m_vertices[pointN];
		vertex.normal.z = 1.0f;
		vertex.normal.normalize();
		m_tm.M.multiply( vertex.normal, vertex.normal );
		vertex.tangent = m_tm.M.getColumn(1)^vertex.normal;
		vertex.tangent.normalize();
		vertex.color = PxColorRGBA( 255, 255, 255, 255 );
		vertex.binormal = vertex.normal^vertex.tangent;
	}

	m_triangles.resize( 2*m_numX*m_numY );
	physx::PxU32 triangleN = 0;
	physx::PxU32 index = 0;
	const physx::PxU32 tpattern[12] = { 0, m_numX+2, m_numX+1, 0, 1, m_numX+2, 0, 1, m_numX+1, 1, m_numX+2, m_numX+1 };
	for( physx::PxU32 iy = 0; iy < m_numY; ++iy )
	{
		const physx::PxU32* yPattern = tpattern + (iy&1)*6;
		for( physx::PxU32 ix = 0; ix < m_numX; ++ix, ++index )
		{
			const physx::PxU32* ytPattern = yPattern;
			for( physx::PxU32 it = 0; it < 2; ++it, ytPattern += 3 )
			{
				NxExplicitRenderTriangle& triangle = m_triangles[triangleN++];
				for( physx::PxU32 v = 0; v < 3; ++v )
				{
					NxVertex& gridVertex = m_vertices[index + ytPattern[v]];
					triangle.vertices[v] = gridVertex;
					triangleFrame.interpolateVertexData( triangle.vertices[v] );
					// Only really needed to interpolate u,v... replace normals and tangents with proper ones
					triangle.vertices[v].normal = gridVertex.normal;
					triangle.vertices[v].tangent = gridVertex.tangent;
					triangle.vertices[v].binormal = gridVertex.binormal;
				}
				triangle.extraDataIndex = frameIndex;
				triangle.smoothingMask = 0;
				triangle.submeshIndex = submeshIndex;
			}
		}
		++index;
	}
}

static void buildHullFromExplicitMesh( physx::ConvexHull& hull, physx::Array< physx::NxExplicitRenderTriangle >& mesh, physx::NxConvexHullMethod::Enum hullMethod )
{
	physx::Array<physx::PxVec3> vertices( mesh.size()*3 );
	physx::PxU32 vertexN = 0;
	for( physx::PxU32 i = 0; i < mesh.size(); ++i )
	{
		physx::NxExplicitRenderTriangle& triangle = mesh[i];
		for( int v = 0; v < 3; ++v )
		{
			const physx::PxVec3& position = triangle.vertices[v].position;
			vertices[vertexN++] = position;
		}
	}
	if( hullMethod == physx::NxConvexHullMethod::WRAP_GRAPHICS_MESH )
	{
		hull.buildFromPoints( vertices.begin(), vertices.size(), sizeof( physx::PxVec3 ) );
		if( hull.isEmpty() )
		{	// fallback
			hullMethod = physx::NxConvexHullMethod::USE_26_DOP;
		}
	}
	if( hullMethod != physx::NxConvexHullMethod::WRAP_GRAPHICS_MESH )
	{
		physx::Array<physx::PxVec3> directions;
		physx::ConvexHull::createKDOPDirections( directions, hullMethod );
		hull.buildKDOP( vertices.begin(), vertices.size(), sizeof( physx::PxVec3 ), directions.begin(), directions.size() );
	}
}

static const int gSliceDirs[6][3] =
{
	{0,1,2},	// XYZ
	{1,2,0},	// YZX
	{2,0,1},	// ZXY
	{2,1,0},	// ZYX
	{1,0,2},	// YXZ
	{0,2,1}		// XZY
};

struct GridParameters
{
	GridParameters() :
	sizeScale(1.0f),
	periodicNoise(false),
	interiorSubmeshIndex(0xFFFFFFFF),
	materialFrameIndex(0xFFFFFFFF)
	{
	}

	physx::Array< physx::NxExplicitRenderTriangle >*	level0Mesh;
	physx::PxF32										sizeScale;
	physx::NoiseParameters								noise;
	bool										periodicNoise;
	physx::PxU32										interiorSubmeshIndex;
	physx::PxU32										materialFrameIndex;
	physx::TriangleFrame								triangleFrame;
};

PX_INLINE void createBasis( physx::PxMat33Legacy & basis, const physx::PxVec3 & localZ )
{
	physx::PxU32 maxDir = physx::PxAbs( localZ.x ) > physx::PxAbs( localZ.y ) ?
		( physx::PxAbs( localZ.x ) > physx::PxAbs( localZ.z ) ? 0 : 2 ) :
		( physx::PxAbs( localZ.y ) > physx::PxAbs( localZ.z ) ? 1 : 2 );

	physx::PxVec3 xAxis( 0.0f );
	xAxis[(maxDir+1)%3] = 1.0f;
	physx::PxVec3 yAxis = localZ^xAxis;
	yAxis.normalize();
	xAxis = yAxis.cross( localZ );
	xAxis.normalize();
	physx::PxVec3 zAxis = localZ;
	zAxis.normalize();

	basis.setColumn( 0, xAxis );
	basis.setColumn( 1, yAxis );
	basis.setColumn( 2, zAxis );
}

static void buildIntersectMesh( physx::IntersectMesh& mesh, const NxPlane& plane, const GridParameters* gridParameters = NULL )
{
	if( !gridParameters )
	{
		mesh.build( plane );
		return;
	}

	physx::PxMat34Legacy tm;
	createBasis( tm.M, PXFROMNXVEC3(plane.normal) );
	tm.t = PXFROMNXVEC3(plane.pointInPlane());

	physx::PxBounds3 localPlaneBounds = boundTriangles( *gridParameters->level0Mesh, tm );

	const physx::PxVec3 diameter = localPlaneBounds.maximum-localPlaneBounds.minimum;
	const physx::PxF32 planeDiameter = physx::PxMax( diameter.x, diameter.y );
	// No longer fattening - the BSP does not have side boundaries, so we will not shave off any of the mesh.
//	localPlaneBounds.fatten( 0.005f*planeDiameter );	// To ensure we get the whole mesh
	const physx::PxF32 gridSpacing = planeDiameter/gridParameters->noise.gridSize;

	physx::PxVec3 center, extent;
	localPlaneBounds.getCenter( center );
	localPlaneBounds.getExtents( extent );

#if 0	// Equilateral
	const physx::PxF32 offset = 0.5f;
	const physx::PxF32 yRatio = 0.866025404f;
	const physx::IntersectMesh::GridPattern pattern = physx::IntersectMesh::Equilateral;
	const physx::PxF32 xSpacing = gridSpacing;
	const physx::PxF32 numX = PxCeil( 2*extent.x / xSpacing + offset );
	const physx::PxF32 cornerX = center.x - 0.5f*(numX-offset)*xSpacing;
	const physx::PxF32 ySpacing = yRatio*gridSpacing;
	const physx::PxF32 numY = PxCeil( 2*extent.y / ySpacing );
	const physx::PxF32 cornerY = center.y - 0.5f*numY*ySpacing;
#else	// Right
	const physx::IntersectMesh::GridPattern pattern = physx::IntersectMesh::Right;
	const physx::PxF32 numX = physx::PxCeil( 2*extent.x / gridSpacing );
	const physx::PxF32 xSpacing = 2*extent.x/numX;
	const physx::PxF32 cornerX = center.x - extent.x;
	const physx::PxF32 numY = physx::PxCeil( 2*extent.y / gridSpacing );
	const physx::PxF32 ySpacing = 2*extent.y/numY;
	const physx::PxF32 cornerY = center.y - extent.y;
#endif

	const physx::PxF32 noiseAmplitude = gridParameters->sizeScale*gridParameters->noise.amplitude;

	mesh.build( pattern, plane, cornerX, cornerY, xSpacing, ySpacing, (physx::PxU32)numX, (physx::PxU32)numY, tm,
				noiseAmplitude, gridParameters->noise.frequency, gridParameters->periodicNoise, 0, -1,
				gridParameters->interiorSubmeshIndex, gridParameters->materialFrameIndex, gridParameters->triangleFrame );
}

PX_INLINE NxPlane createSlicePlane( const physx::PxVec3& center, const physx::PxVec3& extent, int sliceDir, int sliceDirNum,
								    const physx::PxF32 sliceWidths[3], const physx::PxF32 linearNoise[3], const physx::PxF32 angularNoise[3] )
{
	// Orient the plane (+apply the angular noise) and compute the d parameter (+apply the linear noise)
	physx::PxVec3 slicePoint = center;
	slicePoint[sliceDir] += (sliceDirNum+1)*sliceWidths[sliceDir]-extent[sliceDir];
	const physx::PxVec3 normal = randomNormal( sliceDir, angularNoise[sliceDir] );
	return NxPlane( NXFROMPXVEC3(normal), -normal.dot( slicePoint ) + sliceWidths[sliceDir]*linearNoise[sliceDir]*userRnd.getReal(-0.5f,0.5f) );
}

static void buildSliceBSP( physx::IApexBSP& sliceBSP, physx::ExplicitHierarchicalMesh& hMesh, const physx::NoiseParameters& noise,
						   const physx::PxVec3& extent, int dir, const NxPlane planes[3], physx::PxU32 interiorSubmeshIndex, const float interiorUVScale[2] )
{
	// Build grid and slice BSP
	physx::IntersectMesh grid;
	GridParameters gridParameters;
	gridParameters.interiorSubmeshIndex = interiorSubmeshIndex;
	gridParameters.noise = noise;
	gridParameters.level0Mesh = &hMesh.mParts[0]->mMesh;
	gridParameters.sizeScale = extent[dir];
	gridParameters.materialFrameIndex = hMesh.getMaterialFrameCount();
	physx::PxMat34Legacy interiorTM;
	createBasis( interiorTM.M, PXFROMNXVEC3(planes[dir].normal) );
	physx::PxMat44& materialFrame = hMesh.addMaterialFrame();
	materialFrame = interiorTM;
	gridParameters.triangleFrame.setFlat( interiorTM, interiorUVScale );
	buildIntersectMesh( grid, planes[dir], &gridParameters );
	physx::BSPBuildParameters bspBuildParams;
	bspBuildParams.rnd = &userRnd;
	bspBuildParams.linearTolerance = 1.e-9f;
	bspBuildParams.angularTolerance = 0.00001f;
	sliceBSP.fromMesh( grid.m_triangles, bspBuildParams );
}

PX_INLINE physx::IApexBSP* createFractureBSP( NxPlane slicePlanes[3], physx::IApexBSP*& sliceBSP, physx::IApexBSP& sourceBSP,
									   physx::ExplicitHierarchicalMesh& hMesh, physx::PxF32& childTriangleArea, physx::PxF32 minArea,
									   const physx::PxVec3& center, const physx::PxVec3& extent, int sliceDir, int sliceDirNum,
									   const physx::PxF32 sliceWidths[3], const physx::PxF32 linearNoise[3], const physx::PxF32 angularNoise[3],
									   const physx::NoiseParameters& noise, physx::PxU32 interiorSubmeshIndex, const float interiorUVScale[2] )
{
	const NxPlane oldSlicePlane = slicePlanes[sliceDir];
	slicePlanes[sliceDir] = createSlicePlane( center, extent, sliceDir, sliceDirNum, sliceWidths, linearNoise, angularNoise );
	if( sliceBSP == NULL )
	{
		sliceBSP = createBSP( hMesh.mBSPMemCache );
		buildSliceBSP( *sliceBSP, hMesh, noise, extent, sliceDir, slicePlanes, interiorSubmeshIndex, interiorUVScale );
	}
	sourceBSP.combine( *sliceBSP );
	physx::IApexBSP* bsp = createBSP( hMesh.mBSPMemCache );
	bsp->op( sourceBSP, physx::Operation::Intersection );
	const physx::PxF32 bspArea = bsp->getTriangleArea();
	const physx::PxF32 remainingBSPArea = sourceBSP.getTriangleArea( physx::Operation::A_Minus_B );
	if( minArea <= 0 || (bspArea >= minArea && remainingBSPArea >= minArea) )
	{
		sourceBSP.op( sourceBSP, physx::Operation::A_Minus_B );
		childTriangleArea = bspArea;
	}
	else
	{	// We will ignore this slice
		if( remainingBSPArea >= minArea )
		{	// chunk bsp area too small
			slicePlanes[sliceDir] = oldSlicePlane;
			bsp->release();
			bsp = NULL;
			sourceBSP.op( sourceBSP, physx::Operation::Set_A );
			childTriangleArea = 0.0f;
		}
		else
		{	// remainder is too small.  Terminate slicing along this direction
			bsp->op( sourceBSP, physx::Operation::Set_A );
			sourceBSP.op( sourceBSP, physx::Operation::Empty_Set );
			childTriangleArea = bspArea+remainingBSPArea;
		}
	}
	return bsp;
}

static bool hierarchicallySplitChunk
(
	physx::ExplicitHierarchicalMesh& hMesh,
	physx::PxU32 chunkIndex,
	physx::PxU32 sliceDepth,
	NxPlane chunkTrailingPlanes[3],
	NxPlane chunkLeadingPlanes[3],
	physx::IApexBSP* fractureBSP0,	// This will be modified
	physx::PxF32 chunkTriangleArea,
	const physx::FractureSliceDesc& desc,
	physx::NxConvexHullMethod::Enum hullMethod,
	physx::PxU32 interiorSubmeshIndex,
	const physx::FractureMaterialDesc& materialDesc,
	physx::IProgressListener& progressListener,
	volatile bool* cancel
)
{
	if( sliceDepth >= desc.maxDepth )
	{
		return true;	// No slice parameters at this depth
	}

	if( chunkIndex >= hMesh.partCount() || hMesh.mParts[chunkIndex]->mBounds.isEmpty() )
	{
		return true;	// Done, nothing in chunk
	}

	const physx::PxBounds3& bounds = hMesh.mParts[chunkIndex]->mBounds;

	bool canceled = false;	// our own copy of *cancel

	physx::PxVec3 center, extent;
	bounds.getCenter( center );
	bounds.getExtents( extent );

	if( sliceDepth == 0 )
	{
		chunkTrailingPlanes[0].set(-1, 0, 0, bounds.minimum[0] );
		chunkTrailingPlanes[1].set( 0,-1, 0, bounds.minimum[1] );
		chunkTrailingPlanes[2].set( 0, 0,-1, bounds.minimum[2] );
		chunkLeadingPlanes[0].set( 1, 0, 0,-bounds.maximum[0] );
		chunkLeadingPlanes[1].set( 0, 1, 0,-bounds.maximum[1] );
		chunkLeadingPlanes[2].set( 0, 0, 1,-bounds.maximum[2] );
	}

	// Get parameters for this depth
	const physx::SliceParameters& sliceParameters = desc.sliceParameters[sliceDepth++];

	// Determine slicing at this level
	int partition[3];
	calculatePartition( partition, sliceParameters.splitsPerPass, extent, desc.useTargetProportions ? desc.targetProportions : NULL );

	// Slice area rejection ratio, perhaps should be exposed
	const physx::PxF32 areaRejectionRatio = 0.01f;
	const physx::PxF32 boxAreaXY = 8*extent[0]*extent[1];
	const physx::PxF32 boxAreaYZ = 8*extent[1]*extent[2];
	const physx::PxF32 boxAreaZX = 8*extent[2]*extent[0];
	const physx::PxF32 boxArea = boxAreaXY+boxAreaYZ+boxAreaZX;
	const physx::PxF32 estimatedBoxChildArea = boxAreaXY/partition[0]*partition[1] + boxAreaYZ/partition[1]*partition[2] + boxAreaZX/partition[2]*partition[0];
	// Resulting slices must have at least this area
	const physx::PxF32 minChildTriangleArea = areaRejectionRatio*(estimatedBoxChildArea/boxArea)*chunkTriangleArea;

	const bool slicingThrough = sliceParameters.order >= 6;

	const int sliceDirOrder = slicingThrough ? 0 : sliceParameters.order;
	const int sliceDir0 = gSliceDirs[sliceDirOrder][0];
	const int sliceDir1 = gSliceDirs[sliceDirOrder][1];
	const int sliceDir2 = gSliceDirs[sliceDirOrder][2];
	const physx::PxF32 sliceWidths[3] = { 2.0f*extent[0]/partition[0], 2.0f*extent[1]/partition[1], 2.0f*extent[2]/partition[2] };

	physx::HierarchicalProgressListener localProgressListener( partition[0]*partition[1]*partition[2], &progressListener );

	// If we are slicing through, then we need to cache the slice BSPs in the 2nd and 3rd directions
	physx::Array<physx::IApexBSP*> sliceBSPs1;
	physx::Array<physx::IApexBSP*> sliceBSPs2;
	if( slicingThrough )
	{
		sliceBSPs1.resize( partition[gSliceDirs[sliceDirOrder][1]]-1, NULL );
		sliceBSPs2.resize( partition[gSliceDirs[sliceDirOrder][2]]-1, NULL );
	}

	// If we are not slicingb through, we can re-use this sliceBSP
	physx::IApexBSP* reusedSliceBSP = NULL;

	NxPlane trailingPlanes[3];
	NxPlane leadingPlanes[3];

	physx::PxF32 childTriangleArea = 0.0f;

	trailingPlanes[sliceDir0] = chunkTrailingPlanes[sliceDir0];
	leadingPlanes[sliceDir0].set( -trailingPlanes[sliceDir0].normal, -trailingPlanes[sliceDir0].d );
	for( int sliceDir0Num = 0; sliceDir0Num < partition[sliceDir0] && !canceled; ++sliceDir0Num )
	{
		physx::IApexBSP* fractureBSP1 = fractureBSP0;	// This is the default; if there is a need to slice it will be replaced below.
		if( sliceDir0Num+1 < partition[sliceDir0] )
		{	// Slice off piece in the 0 direction
			fractureBSP1 = createFractureBSP( leadingPlanes, reusedSliceBSP, *fractureBSP0, hMesh, childTriangleArea, 0, center, extent, sliceDir0, sliceDir0Num, sliceWidths,
											  sliceParameters.linearVariation, sliceParameters.angularVariation, sliceParameters.noise[sliceDir0], interiorSubmeshIndex, materialDesc.interiorUVScale );
			reusedSliceBSP->release();
			reusedSliceBSP = NULL;
		}
		else
		{
			leadingPlanes[sliceDir0] = chunkLeadingPlanes[sliceDir0];
		}
		trailingPlanes[sliceDir1] = chunkTrailingPlanes[sliceDir1];
		leadingPlanes[sliceDir1].set( -trailingPlanes[sliceDir1].normal, -trailingPlanes[sliceDir1].d );
		for( int sliceDir1Num = 0; sliceDir1Num < partition[sliceDir1] && !canceled; ++sliceDir1Num )
		{
			physx::IApexBSP* fractureBSP2 = fractureBSP1;	// This is the default; if there is a need to slice it will be replaced below.
			if( sliceDir1Num+1 < partition[sliceDir1] )
			{	// Slice off piece in the 1 direction
				physx::IApexBSP*& sliceBSP = !slicingThrough ? reusedSliceBSP : sliceBSPs1[sliceDir1Num];
				fractureBSP2 = createFractureBSP( leadingPlanes, sliceBSP, *fractureBSP1, hMesh, childTriangleArea, 0, center, extent, sliceDir1, sliceDir1Num,
												  sliceWidths, sliceParameters.linearVariation, sliceParameters.angularVariation, sliceParameters.noise[sliceDir1], interiorSubmeshIndex, materialDesc.interiorUVScale );
				if( sliceBSP == reusedSliceBSP )
				{
					reusedSliceBSP->release();
					reusedSliceBSP = NULL;
				}
			}
			else
			{
				leadingPlanes[sliceDir1] = chunkLeadingPlanes[sliceDir1];
			}
			trailingPlanes[sliceDir2] = chunkTrailingPlanes[sliceDir2];
			leadingPlanes[sliceDir2].set( -trailingPlanes[sliceDir2].normal, -trailingPlanes[sliceDir2].d );
			for( int sliceDir2Num = 0; sliceDir2Num < partition[sliceDir2] && !canceled; ++sliceDir2Num )
			{
				physx::IApexBSP* fractureBSP3 = fractureBSP2;	// This is the default; if there is a need to slice it will be replaced below.
				if( sliceDir2Num+1 < partition[sliceDir2] )
				{	// Slice off piece in the 2 direction
					physx::IApexBSP*& sliceBSP = !slicingThrough ? reusedSliceBSP : sliceBSPs2[sliceDir2Num];
					fractureBSP3 = createFractureBSP( leadingPlanes, sliceBSP, *fractureBSP2, hMesh, childTriangleArea, minChildTriangleArea, center, extent, sliceDir2, sliceDir2Num,
													  sliceWidths, sliceParameters.linearVariation, sliceParameters.angularVariation, sliceParameters.noise[sliceDir2], interiorSubmeshIndex, materialDesc.interiorUVScale );
					if( sliceBSP == reusedSliceBSP )
					{
						reusedSliceBSP->release();
						reusedSliceBSP = NULL;
					}
				}
				else
				{
					leadingPlanes[sliceDir2] = chunkLeadingPlanes[sliceDir2];
				}
				if( fractureBSP3 != NULL && !fractureBSP3->isTrivial() )
				{
					physx::IApexBSP* chunkBSP = fractureBSP3;
#if USE_GSA
					if( gIslandGeneration )
					{
						chunkBSP = chunkBSP->decomposeIntoIslands();
					}
#endif
					while( chunkBSP != NULL )
					{
						if( !canceled )
						{
							// Create a mesh with chunkBSP (or its islands)
							const physx::PxU32 newChunkIndex = hMesh.addPart();
							chunkBSP->toMesh( hMesh.mParts[newChunkIndex]->mMesh, DEFAULT_CLEANING_TOLERANCE );
							hMesh.buildMeshBounds( newChunkIndex );
							hMesh.buildCollisionHull( newChunkIndex, hullMethod );
							hMesh.mParts[newChunkIndex]->mParentIndex = chunkIndex;
							// Trim hull in directions where splitting is noisy
							for( physx::PxU32 i = 0; i < 3; ++i )
							{
								if( sliceParameters.noise[i].amplitude != 0.0f || hullMethod != physx::NxConvexHullMethod::WRAP_GRAPHICS_MESH )
								{
									physx::ConvexHull& hull = hMesh.mParts[newChunkIndex]->mCollision;
									physx::PxF32 min, max;
									hull.extent( min, max, PXFROMNXVEC3( trailingPlanes[i].normal ) );
									if( max > min )
									{
										NxPlane clipPlane = trailingPlanes[i];
										clipPlane.d = physx::PxMin( clipPlane.d, -(0.8f*(max-min) + min) );	// 20% clip bound
										hull.intersectPlaneSide( clipPlane );
									}
									hull.extent( min, max, PXFROMNXVEC3( leadingPlanes[i].normal ) );
									if( max > min )
									{
										NxPlane clipPlane = leadingPlanes[i];
										clipPlane.d = physx::PxMin( clipPlane.d, -(0.8f*(max-min) + min) );	// 20% clip bound
										hull.intersectPlaneSide( clipPlane );
									}
								}
							}
							if( hMesh.mParts[newChunkIndex]->mMesh.size() > 0 )
							{	// Proper chunk
								if( sliceDepth < desc.maxDepth )
								{	// Recurse
									canceled = !hierarchicallySplitChunk( hMesh, newChunkIndex, sliceDepth, trailingPlanes, leadingPlanes, chunkBSP, childTriangleArea, desc, hullMethod,
															  interiorSubmeshIndex, materialDesc, localProgressListener, cancel );
								}
							}
							else
							{	// No mesh, eliminate
								hMesh.removePart( newChunkIndex );
							}
						}
						if( chunkBSP == fractureBSP3 )
						{	// No islands were generated; break from loop
							break;
						}
#if USE_GSA
						// Get next bsp in island decomposition
						physx::IApexBSP* nextBSP = chunkBSP->getNext();
						chunkBSP->release();
						chunkBSP = nextBSP;
#endif
					}
				}
				localProgressListener.completeSubtask();
				// We no longer need fractureBSP3
				if( fractureBSP3 != NULL && fractureBSP3 != fractureBSP2 )
				{
					fractureBSP3->release();
					fractureBSP3 = NULL;
				}
				trailingPlanes[sliceDir2].set( -leadingPlanes[sliceDir2].normal, -leadingPlanes[sliceDir2].d );
				// Check for cancellation
				if( cancel != NULL && *cancel )
				{
					canceled = true;
				}
			}
			// We no longer need fractureBSP2
			if( fractureBSP2 != NULL && fractureBSP2 != fractureBSP1 )
			{
				fractureBSP2->release();
				fractureBSP2 = NULL;
			}
			trailingPlanes[sliceDir1].set( -leadingPlanes[sliceDir1].normal, -leadingPlanes[sliceDir1].d );
			// Check for cancellation
			if( cancel != NULL && *cancel )
			{
				canceled = true;
			}
		}
		// We no longer need fractureBSP1
		if( fractureBSP1 != NULL && fractureBSP1 != fractureBSP0 )
		{
			fractureBSP1->release();
			fractureBSP1 = NULL;
		}
		trailingPlanes[sliceDir0].set( -leadingPlanes[sliceDir0].normal, -leadingPlanes[sliceDir0].d );
		// Check for cancellation
		if( cancel != NULL && *cancel )
		{
			canceled = true;
		}
	}

	while( sliceBSPs2.size() )
	{
		if( sliceBSPs2.back() != NULL )
		{
			sliceBSPs2.back()->release();
		}
		sliceBSPs2.popBack();
	}
	while( sliceBSPs1.size() )
	{
		if( sliceBSPs1.back() != NULL )
		{
			sliceBSPs1.back()->release();
		}
		sliceBSPs1.popBack();
	}

	return !canceled;
}	

namespace FractureTools
{

bool buildExplicitHierarchicalMesh
(
	physx::IExplicitHierarchicalMesh& iHMesh,
	const physx::NxExplicitRenderTriangle* meshTriangles,
	physx::PxU32 meshTriangleCount,
	const physx::NxExplicitSubmeshData* submeshData,
	physx::PxU32 submeshCount,
	physx::PxU32* meshPartition,
	physx::PxU32 meshPartitionCount
)
{
	// Build level 0 part out of all of the triangles

	physx::ExplicitHierarchicalMesh& hMesh = *(physx::ExplicitHierarchicalMesh*)&iHMesh;
	hMesh.clear();
	hMesh.addPart();
	hMesh.mParts[0]->mMesh.reset();
	hMesh.mParts[0]->mMesh.reserve( meshTriangleCount );
	for( physx::PxU32 i = 0; i < meshTriangleCount; ++i )
	{
		hMesh.mParts[0]->mMesh.pushBack( meshTriangles[i] );
	}
	hMesh.buildMeshBounds( 0 );
	hMesh.mParts[0]->mParentIndex = -1;

	if( meshPartition != NULL && meshPartitionCount > 1 )
	{
		// We have a partition - build level 1 parts
		physx::PxU32 partIndex = 1;
		physx::PxU32 nextTriangle = 0;
		for( physx::PxU32 i = 0; i < meshPartitionCount; ++i, ++partIndex )
		{
			hMesh.addPart();
			hMesh.mParts[partIndex]->mMesh.reset();
			hMesh.mParts[partIndex]->mMesh.reserve( meshPartition[i]-nextTriangle );
			while( nextTriangle < meshPartition[i] )
			{
				hMesh.mParts[partIndex]->mMesh.pushBack( meshTriangles[nextTriangle++] );
			}
			hMesh.buildMeshBounds( partIndex );
			hMesh.mParts[partIndex]->mParentIndex = 0;
		}
	}

	// Submesh data
	hMesh.mSubmeshData.reset();
	hMesh.mSubmeshData.reserve( submeshCount );
	for( physx::PxU32 i = 0; i < submeshCount; ++i )
	{
		hMesh.mSubmeshData.pushBack( submeshData[i] );
	}

	if( hMesh.mParts.size() > 0 )
	{
		hMesh.mRootDepth = hMesh.depth( hMesh.mParts.size()-1 );
	}

	hMesh.mRootSubmeshCount = submeshCount;

	return true;
}

bool createHierarchicallySplitMesh
(
	physx::IExplicitHierarchicalMesh& iHMesh,
	physx::IExplicitHierarchicalMesh& iHMeshCore,
	bool exportCoreMesh,
	physx::PxU32 interiorSubmeshIndex,
	const MeshProcessingParameters& meshProcessingParams,
	const FractureSliceDesc& desc,
	physx::NxConvexHullMethod::Enum hullMethod,
	const FractureMaterialDesc& materialDesc,
	physx::PxU32 randomSeed,
	physx::IProgressListener& progressListener,
	volatile bool* cancel
)
{
	physx::ExplicitHierarchicalMesh& hMesh = *(physx::ExplicitHierarchicalMesh*)&iHMesh;
	physx::ExplicitHierarchicalMesh& hMeshCore = *(physx::ExplicitHierarchicalMesh*)&iHMeshCore;

	if( hMesh.partCount() == 0 )
	{
		return false;
	}

	// Try to see if we're going to generate too many chunks
	physx::PxU32 estimatedTotalChunkCount = 0;
	for( physx::PxU32 i = 0; i < hMesh.partCount(); ++i )
	{
		if( hMesh.depth( i ) != hMesh.mRootDepth )
		{
			continue;
		}
		physx::PxU32 estimatedLevelChunkCount = 1;
		physx::PxVec3 estimatedExtent;
		hMesh.mParts[i]->mCollision.getBounds().getExtents( estimatedExtent );
		for( physx::PxU32 i = 0; i < desc.maxDepth; ++i )
		{
			// Get parameters for this depth
			const physx::SliceParameters& sliceParameters = desc.sliceParameters[i];
			int partition[3];
			calculatePartition( partition, sliceParameters.splitsPerPass, estimatedExtent, desc.useTargetProportions ? desc.targetProportions : NULL );
			estimatedLevelChunkCount *= partition[0]*partition[1]*partition[2];
			estimatedTotalChunkCount += estimatedLevelChunkCount;
			if( estimatedTotalChunkCount > MAX_ALLOWED_ESTIMATED_CHUNK_TOTAL )
			{
				char message[1000];
				sprintf( message, "Slicing chunk count is estimated to be %d chunks, exceeding the maximum allowed estimated total of %d chunks.  Aborting.  Try using fewer slices, or a smaller fracture depth.",
					estimatedTotalChunkCount, (int)MAX_ALLOWED_ESTIMATED_CHUNK_TOTAL );
				outputMessage( message, physx::PxErrorCode::eINTERNAL_ERROR );
				return false;
			}
			estimatedExtent[0] /= partition[0];
			estimatedExtent[1] /= partition[1];
			estimatedExtent[2] /= partition[2];
		}
	}

	// Save state if cancel != NULL
	physx::PxFileBuf* save = NULL;
	class NullEmbedding : public physx::IExplicitHierarchicalMesh::IEmbedding
	{
		void	serialize( physx::PxFileBuf& stream, IEmbedding::DataType type ) const { (void)stream; (void)type; }
		void	deserialize( physx::PxFileBuf& stream, IEmbedding::DataType type, physx::PxU32 version ) { (void)stream; (void)type; (void)version; }
	} embedding;
	if( cancel != NULL )
	{
		save = physx::NxGetApexSDK()->createMemoryWriteStream();
		if( save != NULL )
		{
			hMesh.serialize( *save, embedding );
		}
	}
	bool canceled = false;

	hMesh.buildCollisionHull( 0, hullMethod );

	userRnd.m_rnd.setSeed( randomSeed );

	// Make sure we've got BSPs up to hMesh.mRootDepth
	for( physx::PxU32 i = 0; i < hMesh.partCount(); ++i )
	{
		if( hMesh.depth( i ) > hMesh.mRootDepth )
		{
			continue;
		}
		if( hMesh.mParts[i]->mMeshBSP == NULL )
		{
			outputMessage( "Building mesh BSP..." );
			progressListener.setProgress(0);
			hMesh.calculatePartBSP( i, randomSeed, &progressListener );
			outputMessage( "Mesh BSP completed." );
			userRnd.m_rnd.setSeed( randomSeed );
		}
	}

	hMesh.clear( true );
	hMesh.mSubmeshData.resize( NxMath::max( hMesh.mRootSubmeshCount, interiorSubmeshIndex+1 ) );

	physx::ExplicitHierarchicalMesh tempCoreMesh;

	physx::PxU32 coreIndex = 0xFFFFFFFF;
	if( hMeshCore.partCount() > 0 )
	{	// We have a core mesh.
		tempCoreMesh.set( iHMeshCore );

		if( exportCoreMesh )
		{
			// Use it as our first split
			// Core starts as original mesh
			coreIndex = hMesh.addPart();
			hMesh.mParts[coreIndex]->mMesh = hMeshCore.mParts[0]->mMesh;
			hMesh.buildMeshBounds( coreIndex );
			hMesh.buildCollisionHull( coreIndex, hullMethod );
			hMesh.mParts[coreIndex]->mParentIndex = 0;
		}

		// Add necessary submesh data to hMesh from core.
		physx::Array<physx::PxU32> submeshMap( tempCoreMesh.mSubmeshData.size() );
		if( exportCoreMesh || materialDesc.applyCoreMeshMaterialToNeighborChunks )
		{
			for( physx::PxU32 i = 0; i < tempCoreMesh.mSubmeshData.size(); ++i )
			{
				physx::NxExplicitSubmeshData& coreSubmeshData = tempCoreMesh.mSubmeshData[i];
				submeshMap[i] = hMesh.mSubmeshData.size();
				for( physx::PxU32 j = 0; j < hMesh.mSubmeshData.size(); ++j )
				{
					physx::NxExplicitSubmeshData& submeshData = hMesh.mSubmeshData[j];
					if( 0 == strcmp( submeshData.mMaterialName, coreSubmeshData.mMaterialName ) )
					{
						submeshMap[i] = j;
						break;
					}
				}
				if( submeshMap[i] == hMesh.mSubmeshData.size() )
				{
					hMesh.mSubmeshData.pushBack( coreSubmeshData );
				}
			}
		}

		for( physx::PxU32 i = 0; i < tempCoreMesh.partCount(); ++i )
		{
			if( tempCoreMesh.depth( i ) > tempCoreMesh.mRootDepth )
			{
				continue;
			}

			// Remap materials
			for( physx::PxU32 j = 0; j < tempCoreMesh.mParts[i]->mMesh.size(); ++j )
			{
				physx::NxExplicitRenderTriangle& tri = tempCoreMesh.mParts[i]->mMesh[j];
				if( tri.submeshIndex >= 0 && tri.submeshIndex < (physx::PxI32)submeshMap.size() )
				{
					tri.submeshIndex = materialDesc.applyCoreMeshMaterialToNeighborChunks ? submeshMap[tri.submeshIndex] : interiorSubmeshIndex;
					if( exportCoreMesh && i == 0 )
					{
						hMesh.mParts[coreIndex]->mMesh[j].submeshIndex = submeshMap[hMesh.mParts[coreIndex]->mMesh[j].submeshIndex];
					}
				}
				else
				{
					tri.submeshIndex = interiorSubmeshIndex;
				}
			}

			// Make sure we've got BSPs up to hMesh.mRootDepth
			if( tempCoreMesh.mParts[i]->mMeshBSP == NULL )
			{
				outputMessage( "Building core mesh BSP..." );
				progressListener.setProgress(0);
				tempCoreMesh.calculatePartBSP( i, randomSeed, &progressListener );
				outputMessage( "Core mesh BSP completed." );
				userRnd.m_rnd.setSeed( randomSeed );
			}
		}
	}

	gIslandGeneration = meshProcessingParams.mIslandGeneration;
	gVerbosity = meshProcessingParams.mVerbosity;

	for( physx::PxU32 partIndex = 0; partIndex < hMesh.mParts.size() && !canceled; ++partIndex )
	{
		const physx::PxU32 depth = hMesh.depth( partIndex );
		if( depth > hMesh.mRootDepth )
		{
			continue;	// Only process parts <= hMesh.mRootDepth
		}

		if( depth == 0 && hMesh.mRootDepth > 0 )
		{
			continue;
		}

		physx::IApexBSP* seedBSP = createBSP( hMesh.mBSPMemCache );
		seedBSP->copy( *hMesh.mParts[partIndex]->mMeshBSP );

		// Subtract out core
		bool partModified = false;
		for( physx::PxU32 i = 0; i < tempCoreMesh.partCount(); ++i )
		{
			if( tempCoreMesh.depth( i ) != tempCoreMesh.mRootDepth )
			{
				continue;
			}
			if( tempCoreMesh.mParts[i]->mMeshBSP != NULL )
			{
				seedBSP->combine( *tempCoreMesh.mParts[i]->mMeshBSP );
				seedBSP->op( *seedBSP, physx::Operation::A_Minus_B );
				partModified = true;
			}
		}

		if( partModified && depth > 0 )
		{	// Create part from modified seedBSP (unless it's at level 0)
			seedBSP->toMesh( hMesh.mParts[partIndex]->mMesh, DEFAULT_CLEANING_TOLERANCE );
			if( hMesh.mParts[partIndex]->mMesh.size() != 0 )
			{
				hMesh.buildCollisionHull( partIndex, hullMethod );
			}
		}

		if( depth == hMesh.mRootDepth )
		{	// At hMesh.mRootDepth - split hierarchically
			outputMessage( "Splitting..." );
			progressListener.setProgress(0);
			NxPlane trailingPlanes[3];	// passing in depth = 0 will initialize these
			NxPlane leadingPlanes[3];
			canceled = !hierarchicallySplitChunk( *(physx::ExplicitHierarchicalMesh*)&hMesh, partIndex, 0, trailingPlanes, leadingPlanes, seedBSP, seedBSP->getTriangleArea(), desc,
									  hullMethod, interiorSubmeshIndex, materialDesc, progressListener, cancel );
			outputMessage( "splitting completed." );
		}

		seedBSP->release();
	}

	// Restore if canceled
	if( canceled && save != NULL )
	{
		physx::PxU32 len;
		const void* mem = physx::NxGetApexSDK()->getMemoryWriteBuffer( *save, len );
		physx::PxFileBuf* load = physx::NxGetApexSDK()->createMemoryReadStream( mem, len );
		if( load != NULL )
		{
			hMesh.deserialize( *load, embedding );
			physx::NxGetApexSDK()->releaseMemoryReadStream( *load );
		}
	}

	if( save != NULL )
	{
		physx::NxGetApexSDK()->releaseMemoryReadStream( *save );
	}

	if( canceled )
	{
		return false;
	}
		
	physx::Array<physx::PxU32> remap;
	hMesh.sortParts( &remap );

	if( coreIndex < hMesh.partCount() )
	{	// Create reasonable collision hulls when there is a core mesh
		coreIndex = remap[coreIndex];
		const physx::PxMat34Legacy idTM;
		const physx::PxVec3 idScale( 1.0f );
		const physx::ConvexHull& coreHull = hMesh.mParts[coreIndex]->mCollision;
		hMesh.mParts[coreIndex]->mFlags = physx::MeshFlag::IsCutoutLeftover;	// Bad naming... this should just be "do not fracture"
		for( physx::PxU32 i = 1; i < hMesh.partCount(); ++i )
		{
			if( i == coreIndex )
			{
				continue;
			}
			physx::ConvexHull& hull = hMesh.mParts[i]->mCollision;
			physx::ConvexHull::Separation separation;
			if( physx::ConvexHull::hullsInProximity( coreHull, idTM, idScale, hull, idTM, idScale, 0.0f, &separation ) )
			{
				const physx::PxF32 hullWidth = separation.max1-separation.min1;
				const physx::PxF32 overlap = separation.max0-separation.min1;
				if( overlap >= 0.25f*hullWidth )
				{	// Stuck in there pretty far.  Simply mark it as undamageable.
					hMesh.mParts[i]->mFlags = physx::MeshFlag::IsCutoutFaceSplit;	// Bad naming... this should just be "do not damage"
				}
				else
				{	// Trim the hull
					hull.intersectPlaneSide( NxPlane( -separation.plane.normal, -separation.max0 ) );
				}
			}
		}
	}

	return true;
}

PX_INLINE physx::PxMat34Legacy createCutoutFrame( const physx::PxVec3& center, const physx::PxVec3& extents, physx::PxU32 sliceAxisNum, physx::PxU32 sliceSignNum, const FractureCutoutDesc& desc )
{
	const physx::PxU32 sliceDirIndex = sliceAxisNum*2+sliceSignNum;
	physx::PxVec3 n(0.0f);
	const physx::PxF32 sliceSign = sliceSignNum ? -1.0f : 1.0f;
	n[sliceAxisNum] = sliceSign;
	physx::PxMat34Legacy cutoutTM;
	cutoutTM.M.setColumn( 2, n );
	physx::PxVec3 p(0.0f);
	physx::PxU32 pi;
	physx::PxU32 hi;
	physx::PxF32 applySign;
	switch( sliceAxisNum )
	{
	case 0: pi = 2; hi = 1; applySign = 1.0f; break;
	case 1: pi = 2; hi = 0; applySign =-1.0f; break;
	default:
	case 2: pi = 1; hi = 0; applySign = 1.0f;
	}
	p[pi] = 1.0f;
	cutoutTM.M.setColumn( 1, p );
	cutoutTM.M.setColumn( 0, p.cross(n) );
	physx::PxF32 cutoutWidth;
	physx::PxF32 cutoutHeight;
	const physx::PxF32 cutoutAspectRatio = desc.cutoutSizeX/desc.cutoutSizeY;
	if( cutoutAspectRatio > extents[hi]/extents[pi] )
	{	// Cutout has wider aspect ratio than mesh.  Fit height.
		cutoutHeight = 2*extents[pi];
		cutoutWidth = cutoutHeight*cutoutAspectRatio;
	}
	else
	{	// Mesh has wider aspect ratio than cutout.  Fit width.
		cutoutWidth = 2*extents[hi];
		cutoutHeight = cutoutWidth/cutoutAspectRatio;
	}
	cutoutWidth *= (desc.cutoutWidthInvert[sliceDirIndex]?-1.0f:1.0f)*desc.cutoutWidthScale[sliceDirIndex];
	cutoutHeight *= (desc.cutoutHeightInvert[sliceDirIndex]?-1.0f:1.0f)*desc.cutoutHeightScale[sliceDirIndex];
	cutoutTM.M.multiplyDiagonal( physx::PxVec3( cutoutWidth/desc.cutoutSizeX, cutoutHeight/desc.cutoutSizeY, 1.0f ) );
	cutoutTM.t.set(0.0f, 0.0f, 0.0f);
	cutoutTM.t[hi] = center[hi]-applySign*sliceSign*(0.5f*cutoutWidth+desc.cutoutWidthOffset[sliceDirIndex]*extents[hi]);
	cutoutTM.t[pi] = center[pi]-0.5f*cutoutHeight+desc.cutoutHeightOffset[sliceDirIndex]*extents[pi];

	return cutoutTM;
}

static bool createCutout( physx::ExplicitHierarchicalMesh& hMesh, physx::IApexBSP& cutoutBSP, /*IApexBSP& remainderBSP,*/ const physx::IApexBSP& sourceBSP, physx::PxU32 sourceIndex,
						  physx::NxConvexHullMethod::Enum hullMethod, physx::PxU32 interiorSubmeshIndex, const physx::FractureMaterialDesc& materialDesc, physx::IProgressListener& progressListener,
						  volatile bool* cancel, physx::FractureSliceDesc* cutoutSliceDesc )
{
//	remainderBSP.combine( cutoutBSP );
//	remainderBSP.op( remainderBSP, Operation::A_Minus_B );
	cutoutBSP.combine( sourceBSP );
	cutoutBSP.op( cutoutBSP, physx::Operation::Intersection );
	// BRG - should apply island generation here
//	if( gIslandGeneration )
//	{
//	}
	const physx::PxU32 newChunkIndex = hMesh.addPart();
	cutoutBSP.toMesh( hMesh.mParts[newChunkIndex]->mMesh, DEFAULT_CLEANING_TOLERANCE );
	if( hMesh.mParts[newChunkIndex]->mMesh.size() > 0 )
	{
		hMesh.buildMeshBounds( newChunkIndex );
		hMesh.buildCollisionHull( newChunkIndex, hullMethod );
		hMesh.mParts[newChunkIndex]->mParentIndex = sourceIndex;
		// If this option is selected, slice regions further
		if( cutoutSliceDesc != NULL )
		{	// Split hierarchically
			NxPlane trailingPlanes[3];	// passing in depth = 0 will initialize these
			NxPlane leadingPlanes[3];
			return hierarchicallySplitChunk( hMesh, newChunkIndex, 0, trailingPlanes, leadingPlanes, &cutoutBSP, cutoutBSP.getTriangleArea(), *cutoutSliceDesc,
									  hullMethod, interiorSubmeshIndex, materialDesc, progressListener, cancel );
		}
	}
	else
	{
		hMesh.removePart( newChunkIndex );
	}

	return cancel == NULL || !(*cancel);
}

static void addQuad( physx::Array<physx::NxExplicitRenderTriangle>& mesh, physx::Array<physx::PxMat44>& materialFrames, physx::PxU32 submeshIndex, const float interiorUVScale[2],
					 const physx::PxVec3& v0, const physx::PxVec3& v1, const physx::PxVec3& v2, const physx::PxVec3& v3 )
{
	// Create material frame TM
	const physx::PxU32 materialIndex = materialFrames.size();
	physx::PxMat44& materialFrame = materialFrames.insert();

	physx::PxVec3 col0 = v1-v0;
	col0.normalize();
	physx::PxVec3 col1 = v3-v0;
	physx::PxVec3 col2 = col0.cross( col1 );
	col2.normalize();
	col1 = col2.cross( col0 );

	materialFrame.column0 = physx::PxVec4( col0, 0.0f );
	materialFrame.column1 = physx::PxVec4( col1, 0.0f );
	materialFrame.column2 = physx::PxVec4( col2, 0.0f );
	materialFrame.setPosition( physx::PxVec3( 0.0f ) );

	// Create interpolator for triangle quantities
	physx::TriangleFrame triangleFrame( materialFrame, interiorUVScale );

	// Fill one triangle
	physx::NxExplicitRenderTriangle& tri0 = mesh.insert();
	memset( &tri0, 0, sizeof( physx::NxExplicitRenderTriangle ) );
	tri0.submeshIndex = submeshIndex;
	tri0.extraDataIndex = materialIndex;
	tri0.smoothingMask = 0;
	tri0.vertices[0].position = v0;
	tri0.vertices[1].position = v1;
	tri0.vertices[2].position = v2;
	for( int i = 0; i < 3; ++i )
	{
		triangleFrame.interpolateVertexData( tri0.vertices[i] );
	}

	// ... and another
	physx::NxExplicitRenderTriangle& tri1 = mesh.insert();
	memset( &tri1, 0, sizeof( physx::NxExplicitRenderTriangle ) );
	tri1.submeshIndex = submeshIndex;
	tri1.extraDataIndex = materialIndex;
	tri1.smoothingMask = 0;
	tri1.vertices[0].position = v2;
	tri1.vertices[1].position = v3;
	tri1.vertices[2].position = v0;
	for( int i = 0; i < 3; ++i )
	{
		triangleFrame.interpolateVertexData( tri1.vertices[i] );
	}
}

static bool createFaceCutouts
(
	physx::ExplicitHierarchicalMesh& hMesh,
	physx::PxU32 faceChunkIndex,
	physx::IApexBSP& faceBSP,	// May be modified
	physx::PxU32 interiorSubmeshIndex,
	const physx::FractureCutoutDesc& desc,
	const physx::CutoutSet& cutoutSet,
	physx::NxConvexHullMethod::Enum hullMethod,
	const physx::FractureMaterialDesc& materialDesc,
	const physx::FractureSliceDesc& sliceDesc,
	const physx::PxVec3& center,
	const physx::PxVec3& extents,
	physx::PxU32 sliceAxisNum,
	physx::PxU32 sliceSignNum,
	physx::IProgressListener& progressListener,
	volatile bool* cancel
)
{
	physx::FractureSliceDesc cutoutSliceDesc;
	SliceParameters* sliceParametersAtDepth = (SliceParameters*)PxAlloca( sizeof( SliceParameters )*sliceDesc.maxDepth );
	if( desc.applySlicingToCutoutRegions )
	{
		// Create a sliceDesc based off of the GUI slice desc's X and Y components, applied to the
		// two axes appropriate for this cutout direction.
		cutoutSliceDesc = sliceDesc;
		cutoutSliceDesc.sliceParameters = sliceParametersAtDepth;
		for( unsigned depth = 0; depth < sliceDesc.maxDepth; ++depth )
		{
			cutoutSliceDesc.sliceParameters[depth] = sliceDesc.sliceParameters[depth];
		}
		physx::PxU32 sliceAxes[3];
		switch( sliceAxisNum )
		{
		case 0: sliceAxes[1] = 2; sliceAxes[0] = 1; break;
		case 1: sliceAxes[1] = 2; sliceAxes[0] = 0; break;
		default:
		case 2: sliceAxes[1] = 1; sliceAxes[0] = 0;
		}
		sliceAxes[2] = sliceAxisNum;
		for( physx::PxU32 axisN = 0; axisN < 3; ++axisN )
		{
			cutoutSliceDesc.targetProportions[sliceAxes[axisN]] = sliceDesc.targetProportions[axisN];
			for( physx::PxU32 depth = 0; depth < sliceDesc.maxDepth; ++depth )
			{
				cutoutSliceDesc.sliceParameters[depth].splitsPerPass[sliceAxes[axisN]] = sliceDesc.sliceParameters[depth].splitsPerPass[axisN];
				cutoutSliceDesc.sliceParameters[depth].linearVariation[sliceAxes[axisN]] = sliceDesc.sliceParameters[depth].linearVariation[axisN];
				cutoutSliceDesc.sliceParameters[depth].angularVariation[sliceAxes[axisN]] = sliceDesc.sliceParameters[depth].angularVariation[axisN];
				cutoutSliceDesc.sliceParameters[depth].noise[sliceAxes[axisN]] = sliceDesc.sliceParameters[depth].noise[axisN];
			}
		}
	}

	// Estimate total work for progress
	physx::PxU32 totalWork = 0;
	for( physx::PxU32 i = 0; i < cutoutSet.cutouts.size(); ++i )
	{
		totalWork += cutoutSet.cutouts[i].convexLoops.size();
	}

	physx::HierarchicalProgressListener localProgressListener( totalWork, &progressListener );

//	IApexBSP* cutoutSource = createBSP( hMesh.mBSPMemCache );
//	cutoutSource->copy( faceBSP );

	physx::PxMat34Legacy cutoutTM = createCutoutFrame( center, extents, sliceAxisNum, sliceSignNum, desc );
	const physx::PxVec3 faceNormal = physx::PxSign( cutoutTM.M.determinant() )*cutoutTM.M.getColumn( 2 );

	const physx::PxF32 cosSmoothingThresholdAngle = physx::PxCos( desc.facetNormalMergeThresholdAngle*physx::PxPi/180.0f );

	// "Sandwich" planes
	const physx::PxF32 centerDisp = faceNormal.dot( center );
	const physx::PxF32 paddedExtent = 1.01f*extents[sliceAxisNum];
	const NxPlane maxPlane( NXFROMPXVEC3(faceNormal), -centerDisp-paddedExtent );
	const NxPlane minPlane( NXFROMPXVEC3(faceNormal), -centerDisp+paddedExtent );

	bool canceled = false;

	for( physx::PxU32 i = 0; i < cutoutSet.cutouts.size() && !canceled; ++i )
	{
		physx::Array<NxPlane> trimPlanes;
		const physx::PxU32 oldPartCount = hMesh.partCount();
		const physx::Cutout& cutout = cutoutSet.cutouts[i];
		localProgressListener.setSubtaskWork( 1 );
		const physx::PxU32 loopCount = desc.splitNonconvexRegions ? cutout.convexLoops.size() : 1;
		for( physx::PxU32 j = 0; j < loopCount && !canceled; ++j )
		{
			const physx::PxU32 loopSize = desc.splitNonconvexRegions ? cutout.convexLoops[j].polyVerts.size() : cutout.vertices.size();
			if( desc.splitNonconvexRegions )
			{
				trimPlanes.reset();
			}
			// Build mesh which surrounds the cutout
			physx::Array<physx::NxExplicitRenderTriangle> loopMesh;
			for( physx::PxU32 k = 0; k < loopSize; ++k )
			{
				const physx::PxU32 vertexIndex0 = desc.splitNonconvexRegions ? cutout.convexLoops[j].polyVerts[k].index : k;
				const physx::PxU32 vertexIndex1 = desc.splitNonconvexRegions ? cutout.convexLoops[j].polyVerts[(k+1)%loopSize].index : ((k+1)%loopSize);
				const physx::PxVec3& v0 = cutout.vertices[vertexIndex0];
				const physx::PxVec3& v1 = cutout.vertices[vertexIndex1];
				const physx::PxVec3 v0World = cutoutTM*v0;
				const physx::PxVec3 v1World = cutoutTM*v1;
				const physx::PxVec3 quad0 = PXFROMNXVEC3( minPlane.project( NXFROMPXVEC3( v0World ) ) );
				const physx::PxVec3 quad1 = PXFROMNXVEC3( minPlane.project( NXFROMPXVEC3( v1World ) ) );
				const physx::PxVec3 quad2 = PXFROMNXVEC3( maxPlane.project( NXFROMPXVEC3( v1World ) ) );
				const physx::PxVec3 quad3 = PXFROMNXVEC3( maxPlane.project( NXFROMPXVEC3( v0World ) ) );
				addQuad( loopMesh, hMesh.mMaterialFrames, hMesh.mInteriorSubmeshIndex, materialDesc.interiorUVScale, quad0, quad1, quad2, quad3 );
				if( cutout.convexLoops.size() == 1 || desc.splitNonconvexRegions )
				{
					physx::PxVec3 planeNormal = (quad2-quad0).cross(quad3-quad1);
					planeNormal.normalize();
					trimPlanes.pushBack( NxPlane( NXFROMPXVEC3( 0.25f*(quad0+quad1+quad2+quad3) ), NXFROMPXVEC3( planeNormal ) ) );
				}
			}
			// Smoothe the mesh's normals and tangents
			PX_ASSERT( loopMesh.size() == 2*loopSize );
			if( loopMesh.size() == 2*loopSize )
			{
				for( physx::PxU32 k = 0; k < loopSize; ++k )
				{
					const physx::PxU32 triIndex0 = 2*k;
					const physx::PxU32 frameIndex = loopMesh[triIndex0].extraDataIndex;
					PX_ASSERT( frameIndex == loopMesh[triIndex0+1].extraDataIndex );
					physx::PxMat44& frame = hMesh.mMaterialFrames[frameIndex];
					const physx::PxU32 triIndex2 = 2*((k+1)%loopSize);
					const physx::PxU32 nextFrameIndex = loopMesh[triIndex2].extraDataIndex;
					PX_ASSERT( nextFrameIndex == loopMesh[triIndex2+1].extraDataIndex );
					physx::PxMat44& nextFrame = hMesh.mMaterialFrames[nextFrameIndex];
					const physx::PxVec3 normalK = frame.column2.getXYZ();
					const physx::PxVec3 normalK1 = nextFrame.column2.getXYZ();
					if( normalK.dot( normalK1 ) < cosSmoothingThresholdAngle )
					{
						continue;
					}
					physx::PxVec3 normal = normalK + normalK1;
					normal.normalize();
					loopMesh[triIndex0].vertices[1].normal = normal;
					loopMesh[triIndex0].vertices[2].normal = normal;
					loopMesh[triIndex0+1].vertices[0].normal = normal;
					loopMesh[triIndex2].vertices[0].normal = normal;
					loopMesh[triIndex2+1].vertices[1].normal = normal;
					loopMesh[triIndex2+1].vertices[2].normal = normal;
				}
				for( physx::PxU32 k = 0; k < loopMesh.size(); ++k )
				{
					physx::NxExplicitRenderTriangle& tri = loopMesh[k];
					for( physx::PxU32 v = 0; v < 3; ++v )
					{
						physx::NxVertex& vert = tri.vertices[v];
						vert.tangent = vert.binormal.cross( vert.normal );
					}
				}
			}
			// Create loop cutout BSP
			physx::IApexBSP* loopBSP = createBSP( hMesh.mBSPMemCache );
			physx::BSPBuildParameters bspBuildParams;
			bspBuildParams.rnd = &userRnd;
			loopBSP->fromMesh( loopMesh, bspBuildParams );
			const physx::PxU32 oldSize = hMesh.partCount();
			canceled = !createCutout( hMesh, *loopBSP, faceBSP, /**cutoutSource,*/ faceChunkIndex, hullMethod, interiorSubmeshIndex, materialDesc, localProgressListener, cancel,
						  desc.applySlicingToCutoutRegions ? &cutoutSliceDesc : NULL );
			for( physx::PxU32 partN = oldSize; partN < hMesh.partCount(); ++partN )
			{
				for( physx::PxU32 trimN = 0; trimN < trimPlanes.size(); ++trimN )
				{
					hMesh.mParts[partN]->mCollision.intersectPlaneSide( trimPlanes[trimN] );
				}
			}
			loopBSP->release();
			localProgressListener.completeSubtask();
		}
		// Trim hulls
		if( !canceled )
		{
			for( physx::PxU32 partN = oldPartCount; partN < hMesh.partCount(); ++partN )
			{
				physx::ConvexHull& hull = hMesh.mParts[partN]->mCollision;
				if( !desc.splitNonconvexRegions )
				{
					for( physx::PxU32 trimN = 0; trimN < trimPlanes.size(); ++trimN )
					{
						hull.intersectPlaneSide( trimPlanes[trimN] );
					}
				}
				hull.intersectHull( hMesh.mParts[faceChunkIndex]->mCollision );
			}
		}
	}

//	cutoutSource->release();

	return !canceled;
}

bool createChippedMesh
(
	physx::IExplicitHierarchicalMesh& iHMesh,
	physx::PxU32 interiorSubmeshIndex,
	const physx::MeshProcessingParameters& meshProcessingParams,
	const physx::FractureCutoutDesc& desc,
	const physx::ICutoutSet& iCutoutSet,
	const physx::FractureSliceDesc& sliceDesc,
	physx::NxConvexHullMethod::Enum hullMethod,
	const physx::FractureMaterialDesc& materialDesc,
	physx::PxU32 randomSeed,
	physx::IProgressListener& progressListener,
	volatile bool* cancel
)
{
	physx::ExplicitHierarchicalMesh& hMesh = *(physx::ExplicitHierarchicalMesh*)&iHMesh;

	if( hMesh.partCount() == 0 )
	{
		return false;
	}

	outputMessage( "Chipping..." );
	progressListener.setProgress(0);

	hMesh.buildCollisionHull( 0, hullMethod );

	userRnd.m_rnd.setSeed( randomSeed );

	if( hMesh.mParts[0]->mMeshBSP == NULL )
	{
		outputMessage( "Building mesh BSP..." );
		progressListener.setProgress(0);
		hMesh.calculateMeshBSP( randomSeed, &progressListener );
		outputMessage( "Mesh BSP completed." );
		userRnd.m_rnd.setSeed( randomSeed );
	}

	gIslandGeneration = meshProcessingParams.mIslandGeneration;
	gVerbosity = meshProcessingParams.mVerbosity;

	if( hMesh.mParts[0]->mBounds.isEmpty() )
	{
		return false;	// Done, nothing in mesh
	}

	hMesh.clear( true );
	hMesh.mSubmeshData.resize( NxMath::max( hMesh.mRootSubmeshCount, interiorSubmeshIndex+1 ) );

	// Count directions
	physx::PxU32 directionCount = 0;
	physx::PxU32 directions = desc.directions;
	while( directions )
	{
		directions = (directions-1)&directions;
		++directionCount;
	}

	if( directionCount == 0 )
	{
		return true; // Done, no split directions
	}

	const physx::PxU32 cutoutCount = iCutoutSet.getCutoutCount();

	physx::HierarchicalProgressListener localProgressListener( directionCount*(1+cutoutCount), &progressListener );

	physx::IntersectMesh grid;

	// Core starts as original mesh
	physx::PxU32 coreIndex = hMesh.addPart();
	hMesh.mParts[coreIndex]->mMesh = hMesh.mParts[0]->mMesh;
	hMesh.buildMeshBounds( 0 );
	hMesh.mParts[coreIndex]->mParentIndex = 0;

	physx::PxVec3 extents;
	physx::PxVec3 center;
	hMesh.mParts[0]->mCollision.getBounds().getExtents( extents );
	hMesh.mParts[0]->mCollision.getBounds().getCenter( center );
	const physx::PxF32 sizeScale = physx::PxMax( physx::PxMax( extents.x, extents.y ), extents.z );

	physx::IApexBSP* coreBSP = createBSP( hMesh.mBSPMemCache );
	coreBSP->copy( *hMesh.mParts[0]->mMeshBSP );

	physx::Array<NxPlane> faceTrimPlanes;

	// Save state if cancel != NULL
	physx::PxFileBuf* save = NULL;
	class NullEmbedding : public physx::IExplicitHierarchicalMesh::IEmbedding
	{
		void	serialize( physx::PxFileBuf& stream, IEmbedding::DataType type ) const { (void)stream; (void)type; }
		void	deserialize( physx::PxFileBuf& stream, IEmbedding::DataType type, physx::PxU32 version ) { (void)stream; (void)type; (void)version; }
	} embedding;
	if( cancel != NULL )
	{
		save = physx::NxGetApexSDK()->createMemoryWriteStream();
		if( save != NULL )
		{
			hMesh.serialize( *save, embedding );
		}
	}
	bool canceled = false;

	bool stop = false;
	for( physx::PxU32 sliceAxisNum = 0; sliceAxisNum < 3 && !stop && !canceled; ++sliceAxisNum )
	{
		for( physx::PxU32 sliceSignNum = 0; sliceSignNum < 2 && !stop && !canceled; ++sliceSignNum )
		{
			const physx::PxU32 sliceDirIndex = sliceAxisNum*2+sliceSignNum;
			if( (desc.directions >> sliceDirIndex) & 1 )
			{
				localProgressListener.setSubtaskWork( 1 );
				const physx::PxU32 oldSize = hMesh.partCount();
				physx::IApexBSP* faceBSP = createBSP( hMesh.mBSPMemCache );	// face BSP defaults to all space
				physx::PxU32 faceChunkIndex = 0xFFFFFFFF;
				physx::PxF32 faceVolumeEstimate = 4*extents[(sliceAxisNum+1)%3]*extents[(sliceAxisNum+2)%3];
				if( desc.depths[sliceDirIndex] > 0.0f )	// (depth = 0) => slice all the way through
				{	// Create faceBSP from grid
					faceVolumeEstimate *= physx::PxMin( 2*extents[sliceAxisNum], desc.depths[sliceDirIndex] );
					GridParameters gridParameters;
					gridParameters.interiorSubmeshIndex = interiorSubmeshIndex;
					gridParameters.noise = desc.backfaceNoise[sliceDirIndex];
					gridParameters.level0Mesh = &hMesh.mParts[0]->mMesh;	// must be set each time, since this can move with array resizing
					gridParameters.sizeScale = sizeScale;
					// Create the slicing plane
					NxPlane facePlane;
					facePlane.normal.set( 0.0f );
					facePlane.normal[sliceAxisNum] = sliceSignNum ? -1.0f : 1.0f;
					facePlane.d = -(facePlane.normal[sliceAxisNum]*center[sliceAxisNum] + extents[sliceAxisNum] - desc.depths[sliceDirIndex]);
					physx::PxMat34Legacy interiorTM;
					createBasis( interiorTM.M, PXFROMNXVEC3(facePlane.normal) );
					gridParameters.materialFrameIndex = hMesh.getMaterialFrameCount();
					physx::PxMat44& materialFrame = hMesh.addMaterialFrame();
					materialFrame = interiorTM;
					gridParameters.triangleFrame.setFlat( interiorTM, materialDesc.interiorUVScale );
					buildIntersectMesh( grid, facePlane, &gridParameters );
					physx::BSPBuildParameters bspBuildParams;
					bspBuildParams.rnd = &userRnd;
					bspBuildParams.linearTolerance = 0.00001f;
					bspBuildParams.angularTolerance = 0.00001f;
					faceBSP->fromMesh( grid.m_triangles, bspBuildParams );
					coreBSP->combine( *faceBSP );
					faceBSP->op( *coreBSP, physx::Operation::A_Minus_B );
					coreBSP->op( *coreBSP, physx::Operation::Intersection );
					faceChunkIndex = hMesh.addPart();
					faceBSP->toMesh( hMesh.mParts[faceChunkIndex]->mMesh, DEFAULT_CLEANING_TOLERANCE );
					if( hMesh.mParts[faceChunkIndex]->mMesh.size() != 0 )
					{
						hMesh.buildMeshBounds( faceChunkIndex );
						hMesh.buildCollisionHull( faceChunkIndex, hullMethod );
						if( desc.trimFaceCollisionHulls && (gridParameters.noise.amplitude != 0.0f || hullMethod != physx::NxConvexHullMethod::WRAP_GRAPHICS_MESH) )
						{	// Trim backface
							hMesh.mParts[faceChunkIndex]->mCollision.intersectPlaneSide( NxPlane( -facePlane.normal, -facePlane.d ) );
							faceTrimPlanes.pushBack( facePlane );
						}
						hMesh.mParts[faceChunkIndex]->mParentIndex = 0;
						hMesh.mParts[faceChunkIndex]->mFlags |= physx::MeshFlag::IsCutoutFaceSplit;
					}
					else
					{
						hMesh.removePart( faceChunkIndex );
						faceChunkIndex = 0xFFFFFFFF;
					}
				}
				else
				{	// Slicing goes all the way through
					faceBSP->copy( *coreBSP );
					faceVolumeEstimate *= 2*extents[sliceAxisNum];
					if( oldSize == coreIndex+1 )
					{	// Core hasn't been split yet.  We don't want a copy of the original mesh at level 1, so remove it.
						hMesh.removePart( coreIndex-- );
						faceChunkIndex = coreIndex;
					}
					// This will break us out of both loops (only want to slice all the way through once):
					stop = true;
				}
				localProgressListener.completeSubtask();

				localProgressListener.setSubtaskWork( cutoutCount );
				if( faceChunkIndex < hMesh.partCount() )
				{	// We have a face chunk.  Create cutouts
					canceled = !createFaceCutouts( hMesh, faceChunkIndex, *faceBSP, interiorSubmeshIndex, desc, *(const physx::CutoutSet*)&iCutoutSet, hullMethod, materialDesc,
									   sliceDesc, center, extents, sliceAxisNum, sliceSignNum, localProgressListener, cancel );
					// If there is anything left in the face, attach it as unfracturable
					// Volume rejection ratio, perhaps should be exposed
#if 0	// BRG - to do : better treatment of face leftover
					const physx::PxF32 volumeRejectionRatio = 0.0001f;
					if( faceBSP->getVolume() >= volumeRejectionRatio*faceVolumeEstimate )
					{
						const physx::PxU32 newChunkIndex = hMesh.addPart();
						faceBSP->toMesh( hMesh.mParts[newChunkIndex]->mMesh, DEFAULT_CLEANING_TOLERANCE );
						if( hMesh.mParts[newChunkIndex]->mMesh.size() != 0 )
						{
							hMesh.buildMeshBounds( newChunkIndex );
							hMesh.mParts[newChunkIndex]->mCollision.setEmpty();	// BRG - to do : better treatment of face leftover
							hMesh.mParts[newChunkIndex]->mParentIndex = faceChunkIndex;
							meshFlags.resize( hMesh.partCount(), 0 );
							meshFlags[newChunkIndex] |= MeshFlag::IsCutoutLeftover;
						}
						else
						{
							hMesh.removePart( newChunkIndex );
						}
					}
#endif
				}
				faceBSP->release();
				localProgressListener.completeSubtask();
			}
		}
	}

	if( !canceled && coreIndex != 0 )
	{
		coreBSP->toMesh( hMesh.mParts[coreIndex]->mMesh, DEFAULT_CLEANING_TOLERANCE );
		if( hMesh.mParts[coreIndex]->mMesh.size() != 0 )
		{
			hMesh.buildCollisionHull( coreIndex, hullMethod );
			for( physx::PxU32 i = 0; i < faceTrimPlanes.size(); ++i )
			{
				hMesh.mParts[coreIndex]->mCollision.intersectPlaneSide( faceTrimPlanes[i] );
			}
		}
		else
		{
			if( coreIndex < hMesh.mParts.size() )
			{	// Remove core mesh
				hMesh.removePart( coreIndex );
				for( physx::PxU32 i = 0; i < hMesh.mParts.size(); ++i )
				{
					if( hMesh.mParts[i]->mParentIndex == (physx::PxI32)coreIndex )
					{
						hMesh.mParts[i]->mParentIndex = -1;
					}
					else
					if( hMesh.mParts[i]->mParentIndex > (physx::PxI32)coreIndex )
					{
						--hMesh.mParts[i]->mParentIndex;
					}
				}
			}
			coreIndex = 0xFFFFFFFF;
		}
	}

	coreBSP->release();

	// Restore if canceled
	if( canceled && save != NULL )
	{
		physx::PxU32 len;
		const void* mem = physx::NxGetApexSDK()->getMemoryWriteBuffer( *save, len );
		physx::PxFileBuf* load = physx::NxGetApexSDK()->createMemoryReadStream( mem, len );
		if( load != NULL )
		{
			hMesh.deserialize( *load, embedding );
			physx::NxGetApexSDK()->releaseMemoryReadStream( *load );
		}
	}

	if( save != NULL )
	{
		physx::NxGetApexSDK()->releaseMemoryReadStream( *save );
	}

	if( canceled )
	{
		return false;
	}

	hMesh.sortParts();

	outputMessage( "chipping completed." );

	return true;
}

} // namespace FractureTools

namespace physx {
namespace apex {

// Serialization of NxExplicitSubmeshData

void serialize( physx::PxFileBuf& stream, const NxExplicitSubmeshData& d )
{
	ApexSimpleString materialName( d.mMaterialName );
	apex::serialize( stream, materialName );
	stream << d.mVertexFormat.mWinding;
	stream << d.mVertexFormat.mHasStaticPositions;
	stream << d.mVertexFormat.mHasStaticNormals;
	stream << d.mVertexFormat.mHasStaticTangents;
	stream << d.mVertexFormat.mHasStaticBinormals;
	stream << d.mVertexFormat.mHasStaticColors;
	stream << d.mVertexFormat.mHasStaticSeparateBoneBuffer;
	stream << d.mVertexFormat.mHasDynamicPositions;
	stream << d.mVertexFormat.mHasDynamicNormals;
	stream << d.mVertexFormat.mHasDynamicTangents;
	stream << d.mVertexFormat.mHasDynamicBinormals;
	stream << d.mVertexFormat.mHasDynamicColors;
	stream << d.mVertexFormat.mHasDynamicSeparateBoneBuffer;
	stream << d.mVertexFormat.mUVCount;
	stream << d.mVertexFormat.mBonesPerVertex;
}

void deserialize( physx::PxFileBuf& stream, physx::PxU32 version, NxExplicitSubmeshData& d )
{
	ApexSimpleString materialName;
	apex::deserialize( stream, version, materialName );
	physx::string::strcpy_s( d.mMaterialName, NxExplicitSubmeshData::MaterialNameBufferSize, materialName.c_str() );

	if( version >= ApexStreamVersion::CleanupOfApexRenderMesh )
	{
		stream >> d.mVertexFormat.mWinding;
		stream >> d.mVertexFormat.mHasStaticPositions;
		stream >> d.mVertexFormat.mHasStaticNormals;
		stream >> d.mVertexFormat.mHasStaticTangents;
		stream >> d.mVertexFormat.mHasStaticBinormals;
		stream >> d.mVertexFormat.mHasStaticColors;
		stream >> d.mVertexFormat.mHasStaticSeparateBoneBuffer;
		stream >> d.mVertexFormat.mHasDynamicPositions;
		stream >> d.mVertexFormat.mHasDynamicNormals;
		stream >> d.mVertexFormat.mHasDynamicTangents;
		stream >> d.mVertexFormat.mHasDynamicBinormals;
		stream >> d.mVertexFormat.mHasDynamicColors;
		stream >> d.mVertexFormat.mHasDynamicSeparateBoneBuffer;
		stream >> d.mVertexFormat.mUVCount;
		if( version < ApexStreamVersion::RemovedTextureTypeInformationFromVertexFormat )
		{	// Dead data
			physx::PxU32 textureTypes[NxVertexFormat::MAX_UV_COUNT];
			for( physx::PxU32 i = 0; i < NxVertexFormat::MAX_UV_COUNT; ++i )
			{
				stream >> textureTypes[i];
			}
		}
		stream >> d.mVertexFormat.mBonesPerVertex;
	}
	else
	{
#if 0	// BRG - to do, implement conversion
		bool	hasPosition;
		bool	hasNormal;
		bool	hasTangent;
		bool	hasBinormal;
		bool	hasColor;
		physx::PxU32	numBonesPerVertex;
		physx::PxU32	uvCount;
		NxRenderCullMode::Enum winding = NxRenderCullMode::CLOCKWISE;

		// PH: assuming position and normal as the default dynamic flags
		physx::PxU32 dynamicFlags = NxVertexFormatFlag::POSITION | NxVertexFormatFlag::NORMAL;

		if( version >= ApexStreamVersion::AddedRenderCullModeToRenderMeshAsset )
		{
			//stream.readBuffer( &winding, sizeof(winding) );
			stream >> winding;
		}
		if (version >= ApexStreamVersion::AddedDynamicVertexBufferField)
		{
			stream >> dynamicFlags;
		}
		if( version >= ApexStreamVersion::AddingTextureTypeInformationToVertexFormat )
		{
			stream >> hasPosition;
			stream >> hasNormal;
			stream >> hasTangent;
			stream >> hasBinormal;
			stream >> hasColor;
			if (version >= ApexStreamVersion::RenderMeshAssetRedesign)
			{
				stream >> numBonesPerVertex;
			}
			else
			{
				bool hasBoneIndex;
				stream >> hasBoneIndex;
				numBonesPerVertex = hasBoneIndex ? 1 : 0;
			}
			stream >> uvCount;
			if( version < ApexStreamVersion::RemovedTextureTypeInformationFromVertexFormat )
			{	// Dead data
				physx::PxU32 textureTypes[NxVertexFormat::MAX_UV_COUNT];
				for( physx::PxU32 i = 0; i < NxVertexFormat::MAX_UV_COUNT; ++i )
				{
					stream >> textureTypes[i];
				}
			}
		}
		else
		{
			physx::PxU32 data;
			stream >> data;
			hasPosition = (data&(1<<8)) != 0;
			hasNormal = (data&(1<<9)) != 0;
			hasTangent = (data&(1<<10)) != 0;
			hasBinormal = (data&(1<<11)) != 0;
			hasColor = (data&(1<<12)) != 0;
			numBonesPerVertex = (data&(1<<13)) != 0 ? 1 : 0;
			uvCount = data&0xFF;
		}

		d.mVertexFormat.mWinding = winding;
		d.mVertexFormat.mHasStaticPositions = hasPosition;
		d.mVertexFormat.mHasStaticNormals = hasNormal;
		d.mVertexFormat.mHasStaticTangents = hasTangent;
		d.mVertexFormat.mHasStaticBinormals = hasBinormal;
		d.mVertexFormat.mHasStaticColors = hasColor;
		d.mVertexFormat.mHasStaticSeparateBoneBuffer = false;
		d.mVertexFormat.mHasDynamicPositions = (dynamicFlags & NxVertexFormatFlag::POSITION) != 0;
		d.mVertexFormat.mHasDynamicNormals = (dynamicFlags & NxVertexFormatFlag::NORMAL) != 0;
		d.mVertexFormat.mHasDynamicTangents = (dynamicFlags & NxVertexFormatFlag::TANGENT) != 0;
		d.mVertexFormat.mHasDynamicBinormals = (dynamicFlags & NxVertexFormatFlag::BINORMAL) != 0;
		d.mVertexFormat.mHasDynamicColors = (dynamicFlags & NxVertexFormatFlag::COLOR) != 0;
		d.mVertexFormat.mHasDynamicSeparateBoneBuffer = (dynamicFlags & NxVertexFormatFlag::SEPARATE_BONE_BUFFER) != 0;
		d.mVertexFormat.mUVCount = uvCount;
		d.mVertexFormat.mBonesPerVertex = numBonesPerVertex;

		if (version >= ApexStreamVersion::RenderMeshAssetRedesign)
		{
			physx::PxU32 customBufferCount;
			stream >> customBufferCount;
			for (physx::PxU32 i = 0; i < customBufferCount; i++)
			{
				physx::PxU32 stringLength;
				stream >> stringLength;
				assert(stringLength < 254);
				char buf[256];
				stream.read(buf, stringLength);
				buf[stringLength] = 0;
				physx::PxU32 format;
				stream >> format;
			}
		}
#endif
	}
}


// ExplicitHierarchicalMesh

ExplicitHierarchicalMesh::ExplicitHierarchicalMesh()
{
	mBSPMemCache = createBSPMemCache();
	mInteriorSubmeshIndex = -1;
	mRootDepth = 0;
	mRootSubmeshCount = 0;
}

ExplicitHierarchicalMesh::~ExplicitHierarchicalMesh()
{
	clear();
	mBSPMemCache->release();
}

physx::PxU32 ExplicitHierarchicalMesh::addPart()
{
	const physx::PxU32 index = mParts.size();
	mParts.insert();
	mParts.back() = PX_NEW(Part);
	return index;
}

bool ExplicitHierarchicalMesh::removePart( physx::PxU32 index )
{
	if( index >= partCount() )
	{
		return false;
	}

	delete mParts[index];
	mParts.remove(index );

	return true;
}

void ExplicitHierarchicalMesh::serialize( physx::PxFileBuf& stream, IEmbedding& embedding ) const
{
	stream << (physx::PxU32)ExplicitHierarchicalMesh::Current;
	stream << (physx::PxU32)ApexStreamVersion::Current;
	stream << mParts.size();
	for( physx::PxU32 i = 0; i < mParts.size(); ++i )
	{
		stream << mParts[i]->mParentIndex;
		stream << mParts[i]->mBounds;
		apex::serialize( stream, mParts[i]->mMesh );
		apex::serialize( stream, mParts[i]->mCollision );
		if( mParts[i]->mMeshBSP != NULL )
		{
			stream << (physx::PxU32)1;
			mParts[i]->mMeshBSP->serialize( stream );
		}
		else
		{
			stream << (physx::PxU32)0;
		}
		stream << mParts[i]->mFlags;
	}
	apex::serialize( stream, mSubmeshData );
	const PxU32 size = mMaterialFrames.size();
	stream << size;
	for( PxU32 i = 0; i < size; ++i )
	{
		PxMat34Legacy m34;
		m34.M.setColumn(0, mMaterialFrames[i].column0.getXYZ());
		m34.M.setColumn(1, mMaterialFrames[i].column1.getXYZ());
		m34.M.setColumn(2, mMaterialFrames[i].column2.getXYZ());
		m34.t = mMaterialFrames[i].getPosition();
		stream << m34;
	}
	embedding.serialize( stream, IEmbedding::MaterialLibrary );
	stream << mInteriorSubmeshIndex;
	stream << mRootDepth;
	stream << mRootSubmeshCount;
}

void ExplicitHierarchicalMesh::deserialize( physx::PxFileBuf& stream, IEmbedding& embedding )
{
	clear();

	physx::PxU32 meshStreamVersion;
	stream >> meshStreamVersion;
	physx::PxU32 apexStreamVersion;
	stream >> apexStreamVersion;

	if( meshStreamVersion < ExplicitHierarchicalMesh::RemovedExplicitHMesh_mMaxDepth )
	{
		physx::PxI32 maxDepth;
		stream >> maxDepth;
	}

	if( meshStreamVersion >= ExplicitHierarchicalMesh::UsingExplicitPartContainers )
	{
		physx::PxU32 partCount;
		stream >> partCount;
		mParts.resize( partCount );
		for( physx::PxU32 i = 0; i < partCount; ++i )
		{
			mParts[i] = PX_NEW(Part);
			stream >> mParts[i]->mParentIndex;
			if( meshStreamVersion >= ExplicitHierarchicalMesh::SerializingMeshBounds )
			{
				stream >> mParts[i]->mBounds;
			}
			apex::deserialize( stream, apexStreamVersion, mParts[i]->mMesh );
			if( meshStreamVersion < ExplicitHierarchicalMesh::SerializingMeshBounds )
			{
				buildMeshBounds( i );
			}
			apex::deserialize( stream, apexStreamVersion, mParts[i]->mCollision );
			if( meshStreamVersion >= ExplicitHierarchicalMesh::PerPartMeshBSPs )
			{
				physx::PxU32 createMeshBSP;
				stream >> createMeshBSP;
				if( createMeshBSP )
				{
					mParts[i]->mMeshBSP = createBSP( mBSPMemCache );
					mParts[i]->mMeshBSP->deserialize( stream );
				}
			}
			if( meshStreamVersion >= ExplicitHierarchicalMesh::AddedFlagsFieldToPart )
			{
				stream >> mParts[i]->mFlags;
			}
		}
	}
	else
	{
		physx::Array<physx::PxI32> parentIndices;
		physx::Array< physx::Array< NxExplicitRenderTriangle > > meshes;
		physx::Array< ConvexHull > meshHulls;
		apex::deserialize( stream, apexStreamVersion, parentIndices );
		apex::deserialize( stream, apexStreamVersion, meshes );
		apex::deserialize( stream, apexStreamVersion, meshHulls );
		PX_ASSERT( parentIndices.size() == meshes.size() && meshes.size() == meshHulls.size() );
		physx::PxU32 partCount = physx::PxMin( parentIndices.size(), physx::PxMin( meshes.size(), meshHulls.size() ) );
		mParts.resize( partCount );
		for( physx::PxU32 i = 0; i < partCount; ++i )
		{
			mParts[i] = PX_NEW(Part);
			mParts[i]->mParentIndex = parentIndices[i];
			mParts[i]->mMesh = meshes[i];
			mParts[i]->mCollision = meshHulls[i];
			buildMeshBounds( i );
		}
	}

	if( meshStreamVersion >= ExplicitHierarchicalMesh::SerializingMeshBSP && meshStreamVersion < ExplicitHierarchicalMesh::PerPartMeshBSPs )
	{
		physx::PxU32 createMeshBSP;
		stream >> createMeshBSP;
		if( createMeshBSP )
		{
			mParts[0]->mMeshBSP = createBSP( mBSPMemCache );
			mParts[0]->mMeshBSP->deserialize( stream );
		}
	}

	if( meshStreamVersion >= ExplicitHierarchicalMesh::IncludingVertexFormatInSubmeshData )
	{
		apex::deserialize( stream, apexStreamVersion, mSubmeshData );
	}
	else
	{
		physx::Array<ApexSimpleString> materialNames;
		apex::deserialize( stream, apexStreamVersion, materialNames );
		mSubmeshData.resize( 0 );	// Make sure the next resize calls constructors
		mSubmeshData.resize( materialNames.size() );
		for( physx::PxU32 i = 0; i < materialNames.size(); ++i )
		{
			physx::string::strcpy_s( mSubmeshData[i].mMaterialName, NxExplicitSubmeshData::MaterialNameBufferSize, materialNames[i].c_str() );
			mSubmeshData[i].mVertexFormat.mHasStaticPositions = true;
			mSubmeshData[i].mVertexFormat.mHasStaticNormals = true;
			mSubmeshData[i].mVertexFormat.mHasStaticTangents = true;
			mSubmeshData[i].mVertexFormat.mHasStaticBinormals = true;
			mSubmeshData[i].mVertexFormat.mHasStaticColors = true;
			mSubmeshData[i].mVertexFormat.mUVCount = 1;
			mSubmeshData[i].mVertexFormat.mBonesPerVertex = 1;
		}
	}

	if( meshStreamVersion >= ExplicitHierarchicalMesh::AddedMaterialFramesToHMesh_and_NoiseType_and_GridSize_to_Cleavage )
	{
		const PxU32 size = stream.readDword();
		mMaterialFrames.resize( size );
		for( PxU32 i = 0; i < size; ++i )
		{
			PxMat34Legacy m34;
			stream >> m34;
			mMaterialFrames[i].column0 = PxVec4( m34.M.getColumn(0), 0.0f );
			mMaterialFrames[i].column1 = PxVec4( m34.M.getColumn(1), 0.0f );
			mMaterialFrames[i].column2 = PxVec4( m34.M.getColumn(2), 0.0f );
			mMaterialFrames[i].setPosition( m34.t );
		}
	}
	else
	{
		mMaterialFrames.resize(0);
	}

	if( meshStreamVersion >= ExplicitHierarchicalMesh::AddedMaterialLibraryToMesh )
	{
		embedding.deserialize( stream, IEmbedding::MaterialLibrary, meshStreamVersion );
	}

	if( meshStreamVersion >= ExplicitHierarchicalMesh::AddedCacheChunkSurfaceTracesAndInteriorSubmeshIndex )
	{
		stream >> mInteriorSubmeshIndex;
	}
	else
	{
		mInteriorSubmeshIndex = mSubmeshData.size() > 1 ? mSubmeshData.size()-1 : -1;	// Assume it's the last one, if there is more than one submesh
	}

	if( meshStreamVersion >= ExplicitHierarchicalMesh::PerPartMeshBSPs )
	{
		stream >> mRootDepth;
	}
	else
	{
		mRootDepth = 0;
	}

	if( meshStreamVersion >= ExplicitHierarchicalMesh::StoringRootSubmeshCount )
	{
		stream >> mRootSubmeshCount;
	}
	else
	{
		mRootSubmeshCount = mSubmeshData.size();
	}
}

physx::PxI32 ExplicitHierarchicalMesh::maxDepth() const
{
	physx::PxI32 max = -1;
	physx::PxU32* depths = (physx::PxU32*)NxAlloca( sizeof(physx::PxU32)*partCount() );
	memset( depths, 0, sizeof(physx::PxU32)*partCount() );	// In case we're not in parent-sorted order - however this will just give a nicer incorrect result.
	for( physx::PxU32 i = 0; i < partCount(); ++i )
	{
		physx::PxI32 parent = mParts[i]->mParentIndex;
		depths[i] = parent < 0 ? 0 : depths[parent]+1;
		max = physx::PxMax( max, (physx::PxI32)depths[i] );
	}
	return max;
}

physx::PxI32* ExplicitHierarchicalMesh::interiorSubmeshIndex()
{
	return &mInteriorSubmeshIndex;
}

physx::PxU32 ExplicitHierarchicalMesh::partCount() const
{
	return mParts.size();
}

physx::PxI32* ExplicitHierarchicalMesh::parentIndex( physx::PxU32 partIndex )
{
	return partIndex < partCount() ? &mParts[partIndex]->mParentIndex : NULL;
}

physx::PxU32 ExplicitHierarchicalMesh::depth( physx::PxU32 partIndex ) const
{
	if( partIndex >= mParts.size() )
	{
		return 0;
	}

	PxU32 depth = 0;
	PxI32 index = (PxI32)partIndex;
	while( (index = mParts[index]->mParentIndex) >= 0 )
	{
		++depth;
	}

	return depth;
}

physx::PxU32 ExplicitHierarchicalMesh::meshTriangleCount( physx::PxU32 partIndex ) const
{
	return partIndex < partCount() ? mParts[partIndex]->mMesh.size() : 0;
}

NxExplicitRenderTriangle* ExplicitHierarchicalMesh::meshTriangles( physx::PxU32 partIndex )
{
	return partIndex < partCount() ? mParts[partIndex]->mMesh.begin() : NULL;
}

physx::PxBounds3 ExplicitHierarchicalMesh::meshBounds( physx::PxU32 partIndex ) const
{
	physx::PxBounds3 bounds;
	bounds.setEmpty();
	if( partIndex < partCount() )
	{
		bounds = mParts[partIndex]->mBounds;
	}
	return bounds;
}

physx::PxU32 ExplicitHierarchicalMesh::meshFlags( physx::PxU32 partIndex ) const
{
	return partIndex < partCount() ? mParts[partIndex]->mFlags : 0;
}

physx::PxBounds3 ExplicitHierarchicalMesh::collisionHullBounds( physx::PxU32 partIndex ) const
{
	physx::PxBounds3 bounds;
	bounds.setEmpty();
	if( partIndex < partCount() )
	{
		bounds = mParts[partIndex]->mCollision.getBounds();
	}
	return bounds;
}

physx::PxU32 ExplicitHierarchicalMesh::collisionHullVertexCount( physx::PxU32 partIndex ) const
{
	if( partIndex < partCount() )
	{
		return mParts[partIndex]->mCollision.getVertexCount();
	}
	return 0;
}

physx::PxVec3 ExplicitHierarchicalMesh::collisionHullVertex( physx::PxU32 partIndex, physx::PxU32 vertexIndex ) const
{
	if( partIndex < partCount() )
	{
		const ConvexHull& hull = mParts[partIndex]->mCollision;
		if( vertexIndex < hull.getVertexCount() )
		{
			return hull.getVertex( vertexIndex );
		}
	}
	return physx::PxVec3(0.0f);
}

physx::PxU32 ExplicitHierarchicalMesh::collisionHullEdgeCount( physx::PxU32 partIndex ) const
{
	if( partIndex < partCount() )
	{
		return mParts[partIndex]->mCollision.getEdgeCount();
	}
	return 0;
}

physx::PxVec3 ExplicitHierarchicalMesh::collisionHullEdgeEndpoint( physx::PxU32 partIndex, physx::PxU32 edgeIndex, physx::PxU32 whichEndpoint ) const
{
	if( partIndex < partCount() )
	{
		const ConvexHull& hull = mParts[partIndex]->mCollision;
		if( edgeIndex < hull.getEdgeCount() )
		{
			return hull.getVertex( hull.getEdgeEndpointIndex( edgeIndex, whichEndpoint ) );
		}
	}
	return physx::PxVec3(0.0f);
}

physx::PxU32 ExplicitHierarchicalMesh::collisionHullPlaneCount( physx::PxU32 partIndex ) const
{
	if( partIndex < partCount() )
	{
		return mParts[partIndex]->mCollision.getPlaneCount();
	}
	return 0;
}

NxPlane ExplicitHierarchicalMesh::collisionHullPlane( physx::PxU32 partIndex, physx::PxU32 planeIndex ) const
{
	if( partIndex < partCount() )
	{
		const ConvexHull& hull = mParts[partIndex]->mCollision;
		if( planeIndex < hull.getPlaneCount() )
		{
			return hull.getPlane( planeIndex );
		}
	}
	return NxPlane( NxVec3(0.0f), 0.0f );
}

bool ExplicitHierarchicalMesh::collisionHullRayCast( physx::PxU32 partIndex, physx::PxF32& in, physx::PxF32& out, const NxRay& worldRay,
													 const physx::PxMat44& localToWorldRT, const physx::PxVec3& scale, physx::PxVec3* normal ) const
{
	if( partIndex < partCount() )
	{
		const ConvexHull& hull = mParts[partIndex]->mCollision;
		return hull.rayCast( in, out, worldRay, localToWorldRT, scale, normal );
	}
	return false;
}

physx::PxU32 ExplicitHierarchicalMesh::submeshCount() const
{
	return mSubmeshData.size();
}

NxExplicitSubmeshData* ExplicitHierarchicalMesh::submeshData( physx::PxU32 submeshIndex )
{
	return submeshIndex < mSubmeshData.size() ? mSubmeshData.begin() + submeshIndex : NULL;
}

physx::PxU32 ExplicitHierarchicalMesh::addSubmesh( const NxExplicitSubmeshData& submeshData )
{
	const physx::PxU32 index = mSubmeshData.size();
	mSubmeshData.pushBack( submeshData );
	return index;
}

physx::PxU32 ExplicitHierarchicalMesh::getMaterialFrameCount() const
{
	return mMaterialFrames.size();
}

physx::PxMat44& ExplicitHierarchicalMesh::getMaterialFrame( physx::PxU32 index )
{
	return mMaterialFrames[index];
}

physx::PxMat44& ExplicitHierarchicalMesh::addMaterialFrame()
{
	return mMaterialFrames.insert();
}

void ExplicitHierarchicalMesh::clear( bool keepRoot )
{
	physx::PxU32 index = partCount();
	while( index-- > 0 )
	{
		if( !keepRoot || depth( index ) > mRootDepth )
		{
			removePart( index );
		}
	}

	mMaterialFrames.resize(0);

	if( !keepRoot )
	{
		mSubmeshData.reset();
		mInteriorSubmeshIndex = -1;
		mRootDepth = 0;
		mBSPMemCache->clearAll();
		mRootSubmeshCount = 0;
	}
}

void ExplicitHierarchicalMesh::separateChunkIslands( physx::PxU32 partIndex, NxConvexHullMethod::Enum hullMethod )
{
	const physx::PxF32 overlapEps = 0.001f;

	if( partIndex < mParts.size() )
	{
		const physx::PxF32 eps = overlapEps*mParts[partIndex]->mCollision.getBounds().maximum.distance( mParts[partIndex]->mCollision.getBounds().minimum );
		physx::Array<NxExplicitRenderTriangle>& mesh = mParts[partIndex]->mMesh;
		physx::Array<BoundsRep> boundsReps( mesh.size() );
		for( physx::PxU32 i = 0; i < mesh.size(); ++i )
		{
			boundsReps[i].aabb.include( mesh[i].vertices[0].position );
			boundsReps[i].aabb.include( mesh[i].vertices[1].position );
			boundsReps[i].aabb.include( mesh[i].vertices[2].position );
			boundsReps[i].aabb.fatten( eps );
		}
		physx::Array<IntPair> overlaps;
		boundsCalculateOverlaps( overlaps, Bounds3XYZ, boundsReps );
		physx::PxU32 overlapCount = 0;
		for( physx::PxU32 i = 0; i < overlaps.size(); ++i )
		{
			IntPair& pair = overlaps[i];
			if( trianglesOverlap( mesh[pair.i0].vertices[0].position, mesh[pair.i0].vertices[1].position, mesh[pair.i0].vertices[2].position,
								  mesh[pair.i1].vertices[0].position, mesh[pair.i1].vertices[1].position, mesh[pair.i1].vertices[2].position, eps ) )
			{
				overlaps[overlapCount++] = overlaps[i];
			}
		}
		// symmetrize the overlap list
		overlaps.resize( overlapCount );
		// Find islands
		physx::Array< physx::Array<physx::PxU32> > islands;
		findIslands( islands, overlaps, mesh.size() );
		if( islands.size() > 1 )
		{	// More than one island.  Find the largest one, and split off the rest.
			physx::Array<bool> triangleInLargestIsland( mesh.size(), true );
			physx::PxU32 largestIslandIndex = 0;
			physx::PxU32 largestIslandSize = 0;
			for( physx::PxU32 i = 0; i < islands.size(); ++i )
			{
				const physx::PxU32 islandSize = islands[i].size();
				if( islandSize > largestIslandSize )
				{
					largestIslandSize = islandSize;
					largestIslandIndex = i;
				}
			}
			// Create new parts from all islands except the largest one
			for( physx::PxU32 i = 0; i < islands.size(); ++i )
			{
				if( i == largestIslandIndex )
				{
					continue;
				}
				physx::Array<physx::PxU32>& island = islands[i];
				const physx::PxU32 newPartIndex = addPart();
				for( physx::PxU32 j = 0; j < island.size(); ++j )
				{
					const physx::PxU32 index = island[j];
					mParts[newPartIndex]->mMesh.pushBack( mesh[index] );
					triangleInLargestIsland[index] = false;
				}
				buildMeshBounds( newPartIndex );
				buildCollisionHull( newPartIndex, hullMethod );
				mParts[newPartIndex]->mParentIndex = mParts[partIndex]->mParentIndex;
			}
			// Now consolidate the largest one
			physx::PxU32 meshSize = 0;
			for( physx::PxU32 i = 0; i < mesh.size(); ++i )
			{
				if( triangleInLargestIsland[i] )
				{
					if( i != meshSize )
					{
						mesh[meshSize] = mesh[i];
					}
					++meshSize;
				}
			}
			mesh.resize( meshSize );
			buildMeshBounds( partIndex );
			buildCollisionHull( partIndex, hullMethod );
		}
	}
}

void ExplicitHierarchicalMesh::sortParts( physx::Array<physx::PxU32>* indexRemap )
{
	if( mParts.size() <= 1 )
	{
		return;
	}

	// Sort by original parent index
	physx::Array<PartIndexer> partIndices( mParts.size() );
	for( physx::PxU32 i = 0; i < mParts.size(); ++i )
	{
		partIndices[i].part = mParts[i];
		partIndices[i].parentIndex = mParts[i]->mParentIndex;
		partIndices[i].index = i;
	}
	qsort( partIndices.begin(), partIndices.size(), sizeof(PartIndexer), PartIndexer::compareParentIndices );

	// Now arrange in depth order
	physx::Array<physx::PxU32> parentStarts;
	createIndexStartLookup( parentStarts, -1, partIndices.size()+1, &partIndices[0].parentIndex, partIndices.size(), sizeof( PartIndexer ) );

	physx::Array<PartIndexer> newPartIndices;
	newPartIndices.reserve( mParts.size() );
	physx::PxI32 parentIndex = -1;
	physx::PxU32 nextPart = 0;
	while( newPartIndices.size() < mParts.size() )
	{
		const physx::PxU32 start = parentStarts[parentIndex+1];
		const physx::PxU32 stop = parentStarts[parentIndex+2];
		for( physx::PxU32 index = start; index < stop; ++index )
		{
			newPartIndices.pushBack( partIndices[index] );
		}
		parentIndex = newPartIndices[nextPart++].index;
	}

	// Remap the parts and parent indices
	physx::Array<physx::PxU32> internalRemap;
	physx::Array<physx::PxU32>& remap = indexRemap != NULL ? *indexRemap : internalRemap;
	remap.resize( newPartIndices.size() );
	for( physx::PxU32 i = 0; i < newPartIndices.size(); ++i )
	{
		mParts[i] = newPartIndices[i].part;
		remap[newPartIndices[i].index] = i;
	}
	for( physx::PxU32 i = 0; i < mParts.size(); ++i )
	{
		if( mParts[i]->mParentIndex >= 0 )
		{
			mParts[i]->mParentIndex = remap[mParts[i]->mParentIndex];
		}
	}
}

void ExplicitHierarchicalMesh::set( const IExplicitHierarchicalMesh& mesh )
{
	const ExplicitHierarchicalMesh& m = (const ExplicitHierarchicalMesh &)mesh;
	clear();
	mParts.resize( m.mParts.size() );
	for( physx::PxU32 i = 0; i < mParts.size(); ++i )
	{
		mParts[i] = PX_NEW(Part);
		mParts[i]->mParentIndex = m.mParts[i]->mParentIndex;
		mParts[i]->mBounds = m.mParts[i]->mBounds;
		mParts[i]->mMesh = m.mParts[i]->mMesh;
		if( mParts[i]->mMeshBSP != NULL )
		{
			mParts[i]->mMeshBSP->release();
		}
		if( m.mParts[i]->mMeshBSP != NULL )
		{
			mParts[i]->mMeshBSP = createBSP( mBSPMemCache );
			mParts[i]->mMeshBSP->copy( *m.mParts[i]->mMeshBSP );
		}
		mParts[i]->mCollision = m.mParts[i]->mCollision;
		mParts[i]->mFlags = m.mParts[i]->mFlags;
	}
	mSubmeshData = m.mSubmeshData;
	mMaterialFrames = m.mMaterialFrames;
	mInteriorSubmeshIndex = m.mInteriorSubmeshIndex;
	mRootDepth = m.mRootDepth;
	mRootSubmeshCount = m.mRootSubmeshCount;
}

void ExplicitHierarchicalMesh::calculatePartBSP( physx::PxU32 partIndex, physx::PxU32 randomSeed, IProgressListener* progressListener )
{
	if( partIndex >= mParts.size() )
	{
		return;
	}

	if( mParts[partIndex]->mMeshBSP != NULL )
	{
		mParts[partIndex]->mMeshBSP->release();
		mParts[partIndex]->mMeshBSP = NULL;
	}
	mParts[partIndex]->mMeshBSP = createBSP( mBSPMemCache );
	BSPBuildParameters bspBuildParameters;
	bspBuildParameters.rnd = &userRnd;
	userRnd.m_rnd.setSeed( randomSeed );
	mParts[partIndex]->mMeshBSP->fromMesh( mParts[partIndex]->mMesh, bspBuildParameters, progressListener );
}

void ExplicitHierarchicalMesh::calculateMeshBSP( physx::PxU32 randomSeed, IProgressListener* progressListener )
{
	if( partCount() == 0 )
	{
		outputMessage( "No mesh, cannot calculate BSP.", physx::PxErrorCode::eDEBUG_WARNING );
		return;
	}

	physx::PxU32 bspCount = 0;
	for( physx::PxU32 partIndex = 0; partIndex < mParts.size(); ++partIndex )
	{
		if( mParts[partIndex]->mMeshBSP != NULL )
		{
			mParts[partIndex]->mMeshBSP->release();
			mParts[partIndex]->mMeshBSP = NULL;
		}
		if( depth( partIndex ) <= mRootDepth )
		{
			++bspCount;
		}
	}

	if( bspCount == 0 )
	{
		outputMessage( "No parts at or below given depth, no BSPs to calculate", physx::PxErrorCode::eDEBUG_WARNING );
		return;
	}

	HierarchicalProgressListener progress( bspCount, progressListener );

	for( physx::PxU32 partIndex = 0; partIndex < mParts.size(); ++partIndex )
	{
		if( depth( partIndex ) <= mRootDepth )
		{
			calculatePartBSP( partIndex, randomSeed, &progress );
			progress.completeSubtask();
		}
	}
}

void ExplicitHierarchicalMesh::visualize( NxApexRenderDebug& debugRender, physx::PxU32 flags, physx::PxU32 index ) const
{
#ifdef WITHOUT_DEBUG_VISUALIZE
	PX_FORCE_PARAMETER_REFERENCE(debugRender);
	PX_FORCE_PARAMETER_REFERENCE(flags);
	PX_FORCE_PARAMETER_REFERENCE(index);
#else
	physx::PxU32 bspMeshFlags = 0;
	if( flags&VisualizeMeshBSPInsideRegions )
	{
		bspMeshFlags |= BSPVisualizationFlags::InsideRegions;
	}
	if( flags&VisualizeMeshBSPOutsideRegions )
	{
		bspMeshFlags |= BSPVisualizationFlags::OutsideRegions;
	}
	if( flags&VisualizeMeshBSPSingleRegion )
	{
		bspMeshFlags |= BSPVisualizationFlags::SingleRegion;
	}
	for( physx::PxU32 partIndex = 0; partIndex < mParts.size(); ++partIndex )
	{
		if( mParts[partIndex]->mMeshBSP != NULL )
		{
			mParts[partIndex]->mMeshBSP->visualize( debugRender, bspMeshFlags, index );
		}
	}
#endif
}

void ExplicitHierarchicalMesh::release()
{
	delete this;
}

void ExplicitHierarchicalMesh::buildMeshBounds( physx::PxU32 partIndex )
{
	if( partIndex < partCount() )
	{
		physx::PxBounds3& bounds = mParts[partIndex]->mBounds;
		bounds.setEmpty();
		const physx::Array<NxExplicitRenderTriangle>& mesh = mParts[partIndex]->mMesh;
		for( physx::PxU32 i = 0; i < mesh.size(); ++i )
		{
			bounds.include( mesh[i].vertices[0].position );
			bounds.include( mesh[i].vertices[1].position );
			bounds.include( mesh[i].vertices[2].position );
		}
	}
}

void ExplicitHierarchicalMesh::buildCollisionHull( physx::PxU32 partIndex, NxConvexHullMethod::Enum method )
{
	if( partIndex < partCount() )
	{
		buildHullFromExplicitMesh( mParts[partIndex]->mCollision, mParts[partIndex]->mMesh, method );
	}
}

}} // namespace physx::apex

namespace FractureTools
{
	physx::IExplicitHierarchicalMesh* createExplicitHierarchicalMesh()
	{
		return new physx::ExplicitHierarchicalMesh();
	}
} // namespace FractureTools

#endif

//#ifdef _MANAGED
//#pragma managed(pop)
//#endif
