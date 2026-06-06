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
#ifndef APEX_CSG_HULL_H
#define APEX_CSG_HULL_H

#include "PsShare.h"
#include "authoring/ApexCSGMath.h"
#include "PsArray.h"

#ifndef WITHOUT_APEX_AUTHORING

namespace physx
{
	namespace apex
	{

/* Convex hull that handles unbounded sets. */

class Hull
{
public:
	struct Edge
	{
		physx::PxU32	m_indexV0;
		physx::PxU32	m_indexV1;
		physx::PxU32	m_indexF1;
		physx::PxU32	m_indexF2;
	};

	struct EdgeType
	{
		enum Enum
		{
			LineSegment,
			Ray,
			Line
		};
	};

	PX_INLINE					Hull()										{ setToAllSpace(); }
	PX_INLINE					Hull( const Hull& geom )					{ *this = geom; }

	PX_INLINE	void			setToAllSpace()								{ clear(); allSpace = true; }
	PX_INLINE	void			setToEmptySet()								{ clear(); emptySet = true; }

				void			intersect( const Plane& plane, Real distanceTol );

	PX_INLINE	void			transform( const Mat4Real& tm, const Mat4Real& cofTM );

	PX_INLINE	physx::PxU32	getFaceCount() const						{ return faces.size(); }
	PX_INLINE	const Plane&	getFace( physx::PxU32 faceIndex ) const		{ return faces[faceIndex]; }

	PX_INLINE	physx::PxU32	getEdgeCount() const						{ return edges.size(); }
	PX_INLINE	const Edge&		getEdge( physx::PxU32 edgeIndex ) const		{ return edges[edgeIndex]; }

	PX_INLINE	physx::PxU32	getVertexCount() const						{ return vertexCount; }
	PX_INLINE	const Pos&		getVertex( physx::PxU32 vertexIndex ) const	{ return *(const Pos*)(vectors.begin()+vertexIndex); }

	PX_INLINE	bool			isEmptySet() const							{ return emptySet; }
	PX_INLINE	bool			isAllSpace() const							{ return allSpace; }

				Real			calculateVolume() const;

	// Edge accessors
	PX_INLINE	EdgeType::Enum	getType( const Edge& edge ) const			{ return (EdgeType::Enum)((physx::PxU32)(edge.m_indexV0 >= vertexCount) + (physx::PxU32)(edge.m_indexV1 >= vertexCount)); }
	PX_INLINE	const Pos&		getV0( const Edge& edge )	const			{ return *(Pos*)(vectors.begin()+edge.m_indexV0); }
	PX_INLINE	const Pos&		getV1( const Edge& edge )	const			{ return *(Pos*)(vectors.begin()+edge.m_indexV1); }
	PX_INLINE	const Dir&		getDir( const Edge& edge )	const			{ PX_ASSERT( edge.m_indexV1 >= vertexCount ); return *(Dir*)(vectors.begin()+edge.m_indexV1); }
	PX_INLINE	physx::PxU32	getF1( const Edge& edge )	const			{ return edge.m_indexF1; }
	PX_INLINE	physx::PxU32	getF2( const Edge& edge )	const			{ return edge.m_indexF2; }

	// Serialization
				void			serialize( physx::PxFileBuf& stream ) const;
				void			deserialize( physx::PxFileBuf& stream, physx::PxU32 version );

protected:
	PX_INLINE	void			clear();

				bool			testConsistency( Real distanceTol, Real angleTol ) const;

	// Faces
	physx::Array<Plane>		faces;
	physx::Array<Edge>		edges;
	physx::Array<Vec4Real>	vectors;
	physx::PxU32						vertexCount;	// vectors[i], i >= vertexCount, are used to store vectors for ray and line edges
	bool						allSpace;
	bool						emptySet;
};

PX_INLINE void
Hull::transform( const Mat4Real& tm, const Mat4Real& cofTM )
{
	for( physx::PxU32 i = 0; i < faces.size(); ++i )
	{
		Plane& face = faces[i];
		face = cofTM*face;
		face.normalize();
	}

	for( physx::PxU32 i = 0; i < vectors.size(); ++i )
	{
		Vec4Real& vector = vectors[i];
		vector = tm*vector;
	}
}

PX_INLINE void
Hull::clear()
{
	vectors.reset();
	edges.reset();
	faces.reset();
	vertexCount = 0;
	allSpace = false;
	emptySet = false;
}


};	// namespace ApexCSG
};

#endif

#endif // #define APEX_CSG_HULL_H
