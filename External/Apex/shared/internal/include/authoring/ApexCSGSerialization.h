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
#ifndef APEX_CSG_SERIALIZATION_H
#define APEX_CSG_SERIALIZATION_H

#include "PsShare.h"
#include "ApexSharedUtils.h"
#include "authoring/ApexCSGDefs.h"
#include "authoring/ApexCSGHull.h"

#ifndef WITHOUT_APEX_AUTHORING

namespace physx
{
	namespace apex
	{

/* Version for serialization */

struct Version
{
	enum Enum
	{
		Initial = 0,
		RevisedMeshTolerances,
		UsingOnlyPositionDataInVertex,
		SerializingTriangleFrames,
#if USE_GSA
		UsingGSA,
#endif
		Count,
		Current = Count-1
	};
};


// Vec<T,D>
template<typename T, int D>
PX_INLINE physx::PxFileBuf&
operator << ( physx::PxFileBuf& stream, const Vec<T,D>& v )
{
	for( physx::PxU32 i = 0; i < D; ++i )
	{
		stream << v[i];
	}
	return stream;
}

template<typename T, int D>
PX_INLINE physx::PxFileBuf&
operator >> ( physx::PxFileBuf& stream, Vec<T,D>& v )
{
	for( physx::PxU32 i = 0; i < D; ++i )
	{
		stream >> v[i];
	}

	return stream;
}


// Edge
PX_INLINE void
serialize( physx::PxFileBuf& stream, const Hull::Edge& e )
{
	stream << e.m_indexV0 << e.m_indexV1 << e.m_indexF1 << e.m_indexF2;
}

PX_INLINE void
deserialize( physx::PxFileBuf& stream, physx::PxU32 version, Hull::Edge& e )
{
	PX_FORCE_PARAMETER_REFERENCE( version );	// Initial

	stream >> e.m_indexV0 >> e.m_indexV1 >> e.m_indexF1 >> e.m_indexF2;
}


// Region
PX_INLINE void
serialize( physx::PxFileBuf& stream, const Region& r )
{
#if !USE_GSA
	r.geom.serialize( stream );
#endif
	stream << r.side;
}

PX_INLINE void
deserialize( physx::PxFileBuf& stream, physx::PxU32 version, Region& r )
{
#if USE_GSA
	if( version < Version::UsingGSA )
	{
		Hull hull;
		hull.deserialize( stream, version );
	}
#else
	r.geom.deserialize( stream, version );
#endif
	stream >> r.side;
}


// Surface
PX_INLINE void
serialize( physx::PxFileBuf& stream, const Surface& s )
{
	stream << s.planeIndex;
	stream << s.triangleIndexStart;
	stream << s.triangleIndexStop;
	stream << s.totalTriangleArea;
}

PX_INLINE void
deserialize( physx::PxFileBuf& stream, physx::PxU32 version, Surface& s )
{
	PX_FORCE_PARAMETER_REFERENCE( version );	// Initial

	stream >> s.planeIndex;
	stream >> s.triangleIndexStart;
	stream >> s.triangleIndexStop;
	stream >> s.totalTriangleArea;
}


// Triangle
PX_INLINE void
serialize( physx::PxFileBuf& stream, const Triangle& t )
{
	for( physx::PxU32 i = 0; i < 3; ++i )
	{
		stream << t.vertices[i];
	}
	stream << t.submeshIndex;
	stream << t.smoothingMask;
	stream << t.extraDataIndex;
	stream << t.normal;
	stream << t.area;
}

PX_INLINE void
deserialize( physx::PxFileBuf& stream, physx::PxU32 version, Triangle& t )
{
	for( physx::PxU32 i = 0; i < 3; ++i )
	{
		stream >> t.vertices[i];
		if( version < Version::UsingOnlyPositionDataInVertex )
		{
			Dir v;
			stream >> v;	// normal
			stream >> v;	// tangent
			stream >> v;	// binormal
			UV uv;
			for( physx::PxU32 uvN = 0; uvN < NxVertexFormat::MAX_UV_COUNT; ++uvN )
			{
				stream >> uv;	// UVs
			}
			Color c;
			stream >> c;	// color
		}
	}
	stream >> t.submeshIndex;
	stream >> t.smoothingMask;
	stream >> t.extraDataIndex;
	stream >> t.normal;
	stream >> t.area;
}

// Interpolator
PX_INLINE void
serialize( physx::PxFileBuf& stream, const Interpolator& t )
{
	t.serialize( stream );
}

PX_INLINE void
deserialize( physx::PxFileBuf& stream, physx::PxU32 version, Interpolator& t )
{
	t.deserialize( stream, version );
}


};	// namespace ApexCSG
};

#endif

#endif // #define APEX_CSG_SERIALIZATION_H
