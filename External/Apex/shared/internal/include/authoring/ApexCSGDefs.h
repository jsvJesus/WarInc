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
#ifndef APEX_CSG_DEFS_H
#define APEX_CSG_DEFS_H

#include "PsShare.h"
#include "ApexSharedUtils.h"
#include "ApexRand.h"
#include "Link.h"
#include "authoring/ApexCSG.h"
#if USE_GSA
#include "authoring/ApexCSGMath.h"
#include "authoring/ApexGSA.h"
#else
#include "authoring/ApexCSGHull.h"
#endif
#include "PsUserAllocated.h"

#include "NxFromPx.h"

#ifndef WITHOUT_APEX_AUTHORING

namespace physx
{
	namespace apex
	{

// Binary tree node
class BinaryNode
{
public:

	enum Index { Out = 0, In = 1 };

	PX_INLINE					BinaryNode();

	PX_INLINE	void			setChild( physx::PxU32 index, BinaryNode* child );

    PX_INLINE	void			detach();

	PX_INLINE	BinaryNode*		getParent()						const	{ return m_parent; }
	PX_INLINE	BinaryNode*		getChild( physx::PxU32 index )	const	{ PX_ASSERT( (index&1) == index ); return m_children[index&1]; }

	PX_INLINE	physx::PxU32	getIndex()						const	{ return m_index; }

protected:

	BinaryNode*	m_parent;
	BinaryNode*	m_children[2];
	physx::PxU32		m_index;	// index of this node in parent (0xFFFFFFFF => not attached)
};

PX_INLINE
BinaryNode::BinaryNode()
{
	m_index = 0xFFFFFFFF;
	m_children[1] = m_children[0] = m_parent = NULL;
}

PX_INLINE void
BinaryNode::setChild( physx::PxU32 index, BinaryNode* child )
{
	index &= 1;
	BinaryNode*& oldChild = m_children[index];

	if( oldChild != NULL )
	{
		oldChild->detach();
	}

	oldChild = child;

	if( child != NULL )
	{
		child->detach();
		child->m_parent = this;
		child->m_index = index;
	}
}

PX_INLINE void
BinaryNode::detach()
{
	if( m_parent != NULL )
	{
		PX_ASSERT( m_parent->getChild( m_index ) == this );
		m_parent->m_children[m_index&1] = NULL;
		m_parent = NULL;
		m_index = 0xFFFFFFFF;
	}
}


// CSG mesh representation

class UV : public Vec<Real,2>
{
public:

	PX_INLINE				UV()							{}
	PX_INLINE				UV( const physx::PxF32* uv )	{ set( (Real)uv[0], (Real)uv[1] ); }
	PX_INLINE				UV( const NxF64* uv )			{ set( (Real)uv[0], (Real)uv[1] ); }
	PX_INLINE	UV&			operator = ( const UV& uv )		{ el[0] = uv.el[0]; el[1] = uv.el[1]; return *this; }

	PX_INLINE	void		set( Real u, Real v )			{ el[0] = u; el[1] = v; }

	PX_INLINE	const Real&	u() const						{ return el[0]; }
	PX_INLINE	const Real&	v() const						{ return el[1]; }
};

class Color : public Vec<Real,4>
{
public:

	PX_INLINE					Color()									{}
	PX_INLINE					Color( const physx::PxU32 c );
	PX_INLINE	Color&			operator = ( const Color& c )			{ el[0] = c.el[0]; el[1] = c.el[1]; el[2] = c.el[2]; el[3] = c.el[3]; return *this; }

	PX_INLINE	void			set( Real r, Real g, Real b, Real a	)	{ el[0] = r; el[1] = g; el[2] = b; el[3] = a; }

	PX_INLINE	physx::PxU32	toInt() const;

	PX_INLINE	const Real&		r() const								{ return el[0]; }
	PX_INLINE	const Real&		g() const								{ return el[1]; }
	PX_INLINE	const Real&		b() const								{ return el[2]; }
	PX_INLINE	const Real&		a() const								{ return el[3]; }
};

PX_INLINE
Color::Color( const physx::PxU32 c )
{
	const Real recip255 = 1/(Real)255;
	set( (Real)(c&0xFF)*recip255, (Real)((c>>8)&0xFF)*recip255, (Real)((c>>16)&0xFF)*recip255, (Real)(c>>24)*recip255 );
}

PX_INLINE physx::PxU32
Color::toInt() const
{
	return (physx::PxU32)((int)(255*el[3]+(Real)0.5))<<24|(physx::PxU32)((int)(255*el[2]+(Real)0.5))<<16|(physx::PxU32)((int)(255*el[1]+(Real)0.5))<<8|(physx::PxU32)((int)(255*el[0]+(Real)0.5));
}

struct VertexData
{
	Dir		normal;
	Dir		tangent;
	Dir		binormal;
	UV		uv[NxVertexFormat::MAX_UV_COUNT];
	Color	color;
};

struct Triangle
{
	Pos				vertices[3];
	Dir				normal;
	Real			area;
	physx::PxI32	submeshIndex;
	physx::PxU32	smoothingMask;
	physx::PxU32	extraDataIndex;

	void	fromExplicitRenderTriangle( VertexData vertexData[3], const NxExplicitRenderTriangle& tri )
			{
				for( unsigned i = 0; i < 3; ++i )
				{
					vertices[i] = Pos( tri.vertices[i].position );
					vertexData[i].normal = Dir( tri.vertices[i].normal );
					vertexData[i].tangent = Dir( tri.vertices[i].tangent );
					vertexData[i].binormal = Dir( tri.vertices[i].binormal );
					for( unsigned j = 0; j < NxVertexFormat::MAX_UV_COUNT; ++j )
					{
						vertexData[i].uv[j] = UV( &tri.vertices[i].uv[j][0] );
					}
					vertexData[i].color.set( (Real)tri.vertices[i].color.r, (Real)tri.vertices[i].color.g, (Real)tri.vertices[i].color.b, (Real)tri.vertices[i].color.a );
				}
				submeshIndex = tri.submeshIndex;
				smoothingMask = tri.smoothingMask;
				extraDataIndex = tri.extraDataIndex;
				calculateQuantities();
			}

	void	toExplicitRenderTriangle( NxExplicitRenderTriangle& tri, const VertexData vertexData[3] ) const
			{
				for( unsigned i = 0; i < 3; ++i )
				{
					PxVec3FromArray( tri.vertices[i].position, &vertices[i][0] );
					PxVec3FromArray( tri.vertices[i].normal, &vertexData[i].normal[0] );
					PxVec3FromArray( tri.vertices[i].tangent, &vertexData[i].tangent[0] );
					PxVec3FromArray( tri.vertices[i].binormal, &vertexData[i].binormal[0] );
					for( unsigned j = 0; j < NxVertexFormat::MAX_UV_COUNT; ++j )
					{
						tri.vertices[i].uv[j].set( (physx::PxF32)vertexData[i].uv[j][0], (physx::PxF32)vertexData[i].uv[j][1] );
					}
					tri.vertices[i].color.set( (physx::PxF32)vertexData[i].color.r(), (physx::PxF32)vertexData[i].color.g(), (physx::PxF32)vertexData[i].color.b(), (physx::PxF32)vertexData[i].color.a() );
				}
				tri.submeshIndex = submeshIndex;
				tri.smoothingMask = smoothingMask;
				tri.extraDataIndex = extraDataIndex;
			}

	void	calculateQuantities()
			{
				normal = Dir(vertices[1]-vertices[0])^(Dir(vertices[2]-vertices[0]));
				area = (Real)0.5*normal.normalize();
			}
};

struct LinkedVertex : public Link
{
	LinkedVertex*	getAdj( physx::PxU32 which ) const { return (LinkedVertex*)Link::getAdj( which ); }

	Pos	vertex;
};

struct LinkedEdge2D : public Link
{
					LinkedEdge2D() : loopID( -1 ) {}
					~LinkedEdge2D() { remove(); }

	void			setAdj( physx::PxU32 which, LinkedEdge2D* link )
					{	// Ensure neighboring links' adjoining vertices are equal
						which &= 1;
						const physx::PxU32 other = which^1;
						v[which] = link->v[other];
						((LinkedEdge2D*)link->adj[other])->v[which] = ((LinkedEdge2D*)adj[which])->v[other];
						Link::setAdj( which, link );
					}

	LinkedEdge2D*	getAdj( physx::PxU32 which ) const { return (LinkedEdge2D*)Link::getAdj( which ); }

	void			remove()
					{	// Ensure neighboring links' adjoining vertices are equal
						((LinkedEdge2D*)adj[0])->v[1] = ((LinkedEdge2D*)adj[1])->v[0] = (Real)0.5*(v[0] + v[1]);
						Link::remove();
					}

	Vec2Real		v[2];
	physx::PxI32	loopID;
};

struct Surface
{
#if !USE_GSA
	PX_INLINE	Surface() : planeIndex( 0xFFFFFFFF ), triangleIndexStart(0), triangleIndexStop(0), totalTriangleArea(0) {}
#endif

	physx::PxU32	planeIndex;
	physx::PxU32	triangleIndexStart;
	physx::PxU32	triangleIndexStop;
	physx::PxF32	totalTriangleArea;	// Keeping it 32-bit real, since we don't need precision here
};

struct Region
{
#if !USE_GSA
	PX_INLINE	Region() : side(1) {}	// side = 1 by default (e.g. a single node is considered inside)

	Hull			geom;
#endif
	physx::PxU32	side;
#if USE_GSA
	// Not to be serialized, but we have this extra space since Region is used in a union with Surface
	physx::PxU32	tempIndex1;
	physx::PxU32	tempIndex2;
	physx::PxU32	tempIndex3;
#endif
};


// Interpolator - calculates interpolation data for triangle quantities
class Interpolator
{
public:

	enum VertexField
	{
		Normal_x, Normal_y, Normal_z,
		Tangent_x, Tangent_y, Tangent_z,
		Binormal_x, Binormal_y, Binormal_z,
		UV0_u, UV0_v, UV1_u, UV1_v, UV2_u, UV2_v, UV3_u, UV3_v,
		Color_r, Color_g, Color_b, Color_a,

		VertexFieldCount
	};

					Interpolator()															{}
					Interpolator( const Triangle & tri, const VertexData vertexData[3] )	{ setFromTriangle( tri, vertexData ); }
					Interpolator( const Dir tangents[3], const Vec<Real,2>& uvScale )		{ setFlat( tangents, uvScale ); }

	PX_INLINE void	setFromTriangle( const Triangle & tri, const VertexData vertexData[3] );
	PX_INLINE void	setFlat( const Dir tangents[3], const Vec<Real,2>& uvScale );

	PX_INLINE void	interpolateVertexData( VertexData& vertexData, const Pos& point ) const;

	PX_INLINE bool	equals( const Interpolator& interpolator, Real frameDirTol, Real frameScaleTol, Real dirTol, Real uvTol, Real colorTol ) const;

	PX_INLINE void	transform( Interpolator& transformedInterpolator, const Mat4Real& tm, const Mat4Real& cofTM ) const;

			  void	serialize( physx::PxFileBuf& stream ) const;
			  void	deserialize( physx::PxFileBuf& stream, physx::PxU32 version );

private:
	Plane			m_frames[VertexFieldCount];
	static size_t	s_offsets[VertexFieldCount];

	friend class InterpolatorBuilder;
};

PX_INLINE void
Interpolator::setFromTriangle( const Triangle & tri, const VertexData vertexData[3] )
{
	const Pos& p0 = tri.vertices[0];
	const Pos& p1 = tri.vertices[1];
	const Pos& p2 = tri.vertices[2];
	const Dir p1xp2 = Dir(p1)^Dir(p2);
	const Dir p2xp0 = Dir(p2)^Dir(p0);
	const Dir p0xp1 = Dir(p0)^Dir(p1);
	const Dir n = p1xp2 + p2xp0 + p0xp1;
	const Real n2 = n|n;
	if( n2 < EPS_REAL*EPS_REAL )
	{
		for( physx::PxU32 i = 0; i < VertexFieldCount; ++i )
		{
			m_frames[i].set(Dir((Real)0),0);
		}
		return;
	}

	// Calculate inverse 4x4 matrix (only need first three columns):
	const Dir nP = n/n2;	// determinant is -n2
	const Dir Q0( nP[2]*(p1[1]-p2[1])-nP[1]*(p1[2]-p2[2]), nP[2]*(p2[1]-p0[1])-nP[1]*(p2[2]-p0[2]), nP[2]*(p0[1]-p1[1])-nP[1]*(p0[2]-p1[2]) );
	const Dir Q1( nP[0]*(p1[2]-p2[2])-nP[2]*(p1[0]-p2[0]), nP[0]*(p2[2]-p0[2])-nP[2]*(p2[0]-p0[0]), nP[0]*(p0[2]-p1[2])-nP[2]*(p0[0]-p1[0]) );
	const Dir Q2( nP[1]*(p1[0]-p2[0])-nP[0]*(p1[1]-p2[1]), nP[1]*(p2[0]-p0[0])-nP[0]*(p2[1]-p0[1]), nP[1]*(p0[0]-p1[0])-nP[0]*(p0[1]-p1[1]) );
	const Dir r( nP|p1xp2, nP|p2xp0, nP|p0xp1 );

	for( physx::PxU32 i = 0; i < VertexFieldCount; ++i )
	{
		const size_t offset = s_offsets[i];
		const Dir vi( *(Real*)(((NxU8*)&vertexData[0])+offset), *(Real*)(((NxU8*)&vertexData[1])+offset), *(Real*)(((NxU8*)&vertexData[2])+offset) );
		Dir n( Q0|vi, Q1|vi, Q2|vi );
		if( (n|n) < 100*EPS_REAL*EPS_REAL )
		{
			n.set( (Real)0, (Real)0, (Real)0 );
		}
		Real o = r|vi;
		if( physx::PxAbs( o ) < 100*EPS_REAL )
		{
			o = (Real)0;
		}
		m_frames[i].set( n, o );
	}
}

PX_INLINE void
Interpolator::setFlat( const Dir tangents[3], const Vec<Real,2>& uvScale )
{
	// Local z ~ normal = tangents[2], x ~ u and tangent = tangents[0], y ~ v and binormal = tangents[1]
	m_frames[Normal_x].set( Dir((Real)0), tangents[2][0] );
	m_frames[Normal_y].set( Dir((Real)0), tangents[2][1] );
	m_frames[Normal_z].set( Dir((Real)0), tangents[2][2] );
	m_frames[Tangent_x].set( Dir((Real)0), tangents[0][0] );
	m_frames[Tangent_y].set( Dir((Real)0), tangents[0][1] );
	m_frames[Tangent_z].set( Dir((Real)0), tangents[0][2] );
	m_frames[Binormal_x].set( Dir((Real)0), tangents[1][0] );
	m_frames[Binormal_y].set( Dir((Real)0), tangents[1][1] );
	m_frames[Binormal_z].set( Dir((Real)0), tangents[1][2] );
	const Dir su = (uvScale[0] ? 1/uvScale[0] : (Real)0)*tangents[0];
	const Dir sv = (uvScale[1] ? 1/uvScale[1] : (Real)0)*tangents[1];
	m_frames[UV0_u].set( su, 0 );
	m_frames[UV0_v].set( sv, 0 );
	m_frames[UV1_u].set( su, 0 );
	m_frames[UV1_v].set( sv, 0 );
	m_frames[UV2_u].set( su, 0 );
	m_frames[UV2_v].set( sv, 0 );
	m_frames[UV3_u].set( su, 0 );
	m_frames[UV3_v].set( sv, 0 );
	m_frames[Color_r].set( Dir((Real)0), (Real)1 );
	m_frames[Color_g].set( Dir((Real)0), (Real)1 );
	m_frames[Color_b].set( Dir((Real)0), (Real)1 );
	m_frames[Color_a].set( Dir((Real)0), (Real)1 );
}

PX_INLINE void
Interpolator::interpolateVertexData( VertexData& vertexData, const Pos& point ) const
{
	for( physx::PxU32 i = 0; i < VertexFieldCount; ++i )
	{
		Real& value = *(Real*)(((NxU8*)&vertexData) + s_offsets[i]);
		value = m_frames[i].distance( point );
	}
}

PX_INLINE bool
framesEqual( const Plane& f0, const Plane& f1, Real twoFrameScaleTol2, Real sinFrameTol2, Real tol2 )
{
	const Dir n0 = f0.normal();
	const Dir n1 = f1.normal();
	const Real n02 = n0|n0;
	const Real n12 = n1|n1;
	const Real n2Diff = n02 - n12;

	if( n2Diff*n2Diff > twoFrameScaleTol2*(n02+n12) )
	{
		return false;	// Scales differ by more than frame scale tolerance
	}

	const Real n2Prod = n02*n12;
	const Real unnormalizedSinFrameTheta2 = (n0^n1).lengthSquared();
	if( unnormalizedSinFrameTheta2 > n2Prod*sinFrameTol2 )
	{
		return false;	// Directions differ by more than frame angle tolerance
	}

	const Real unnormalizedOriginDiff = f0.d() - f1.d();
	const Real originScale = 0.5f*(physx::PxAbs(f0.d()) + physx::PxAbs(f1.d()));
	if( unnormalizedOriginDiff*unnormalizedOriginDiff > tol2*originScale*originScale )
	{
		return false;	// Origins differ by more than tolerance
	}

	return true;
}

PX_INLINE bool
Interpolator::equals( const Interpolator& interpolator, Real frameDirTol, Real frameScaleTol, Real dirTol, Real uvTol, Real colorTol ) const
{
	const Real twoFrameScaleTol2 = (Real)2*frameScaleTol*frameScaleTol;
	const Real sinFrameTol2 = frameDirTol*frameDirTol;
	const Real dirTol2 = dirTol*dirTol;
	const Real uvTol2 = uvTol*uvTol;
	const Real colorTol2 = colorTol*colorTol;

	// Directions
	for( physx::PxU32 i = Normal_x; i <= Binormal_z; ++i )
	{
		if( !framesEqual( m_frames[i], interpolator.m_frames[i], twoFrameScaleTol2, sinFrameTol2, dirTol2 ) )
		{
			return false;
		}
	}

	// UVs
	for( physx::PxU32 i = UV0_u; i <= UV3_v; ++i )
	{
		if( !framesEqual( m_frames[i], interpolator.m_frames[i], twoFrameScaleTol2, sinFrameTol2, uvTol2 ) )
		{
			return false;
		}
	}

	// Color
	for( physx::PxU32 i = Color_r; i <= Color_a; ++i )
	{
		if( !framesEqual( m_frames[i], interpolator.m_frames[i], twoFrameScaleTol2, sinFrameTol2, colorTol2 ) )
		{
			return false;
		}
	}

	return true;
}

PX_INLINE void
Interpolator::transform( Interpolator& transformedInterpolator, const Mat4Real& tm, const Mat4Real& invTransposeTM ) const
{
	// Apply left-hand transform.
	for( physx::PxU32 i = 0; i < VertexFieldCount; ++i )
	{
		transformedInterpolator.m_frames[i] = invTransposeTM*m_frames[i];
	}
	// Apply right-hand transform.  This is specific to the quantities being transformed.
	for( int i = 0; i < 4; ++i )
	{
		// Normal, transform by invTransposeTM:
		Dir normal_frame_i( transformedInterpolator.m_frames[Interpolator::Normal_x][i], transformedInterpolator.m_frames[Interpolator::Normal_y][i], transformedInterpolator.m_frames[Interpolator::Normal_z][i] );
		normal_frame_i = invTransposeTM*normal_frame_i;
		transformedInterpolator.m_frames[Interpolator::Normal_x][i] = normal_frame_i[0];
		transformedInterpolator.m_frames[Interpolator::Normal_y][i] = normal_frame_i[1];
		transformedInterpolator.m_frames[Interpolator::Normal_z][i] = normal_frame_i[2];
		// Tangent, transform by tm:
		Dir tangent_frame_i( transformedInterpolator.m_frames[Interpolator::Tangent_x][i], transformedInterpolator.m_frames[Interpolator::Tangent_y][i], transformedInterpolator.m_frames[Interpolator::Tangent_z][i] );
		tangent_frame_i = tm*tangent_frame_i;
		transformedInterpolator.m_frames[Interpolator::Tangent_x][i] = tangent_frame_i[0];
		transformedInterpolator.m_frames[Interpolator::Tangent_y][i] = tangent_frame_i[1];
		transformedInterpolator.m_frames[Interpolator::Tangent_z][i] = tangent_frame_i[2];
		// Binormal, transform by tm:
		Dir binormal_frame_i( transformedInterpolator.m_frames[Interpolator::Binormal_x][i], transformedInterpolator.m_frames[Interpolator::Binormal_y][i], transformedInterpolator.m_frames[Interpolator::Binormal_z][i] );
		binormal_frame_i = tm*binormal_frame_i;
		transformedInterpolator.m_frames[Interpolator::Binormal_x][i] = binormal_frame_i[0];
		transformedInterpolator.m_frames[Interpolator::Binormal_y][i] = binormal_frame_i[1];
		transformedInterpolator.m_frames[Interpolator::Binormal_z][i] = binormal_frame_i[2];
		// Other quantities are scalars
	}
}


class InterpolatorBuilder
{
public:
	InterpolatorBuilder()
	{
#define CREATE_OFFSET( field )	(size_t)((uintptr_t)&vertexData.field-(uintptr_t)&vertexData)

		VertexData vertexData;
		Interpolator::s_offsets[Interpolator::Normal_x] =	CREATE_OFFSET( normal[0] );
		Interpolator::s_offsets[Interpolator::Normal_y] =	CREATE_OFFSET( normal[1] );
		Interpolator::s_offsets[Interpolator::Normal_z] =	CREATE_OFFSET( normal[2] );
		Interpolator::s_offsets[Interpolator::Tangent_x] =	CREATE_OFFSET( tangent[0] );
		Interpolator::s_offsets[Interpolator::Tangent_y] =	CREATE_OFFSET( tangent[1] );
		Interpolator::s_offsets[Interpolator::Tangent_z] =	CREATE_OFFSET( tangent[2] );
		Interpolator::s_offsets[Interpolator::Binormal_x] =	CREATE_OFFSET( binormal[0] );
		Interpolator::s_offsets[Interpolator::Binormal_y] =	CREATE_OFFSET( binormal[1] );
		Interpolator::s_offsets[Interpolator::Binormal_z] =	CREATE_OFFSET( binormal[2] );
		Interpolator::s_offsets[Interpolator::UV0_u] =		CREATE_OFFSET( uv[0].u() );
		Interpolator::s_offsets[Interpolator::UV0_v] =		CREATE_OFFSET( uv[0].v() );
		Interpolator::s_offsets[Interpolator::UV1_u] =		CREATE_OFFSET( uv[1].u() );
		Interpolator::s_offsets[Interpolator::UV1_v] =		CREATE_OFFSET( uv[1].v() );
		Interpolator::s_offsets[Interpolator::UV2_u] =		CREATE_OFFSET( uv[2].u() );
		Interpolator::s_offsets[Interpolator::UV2_v] =		CREATE_OFFSET( uv[2].v() );
		Interpolator::s_offsets[Interpolator::UV3_u] =		CREATE_OFFSET( uv[3].u() );
		Interpolator::s_offsets[Interpolator::UV3_v] =		CREATE_OFFSET( uv[3].v() );
		Interpolator::s_offsets[Interpolator::Color_r] =	CREATE_OFFSET( color.r() );
		Interpolator::s_offsets[Interpolator::Color_g] =	CREATE_OFFSET( color.g() );
		Interpolator::s_offsets[Interpolator::Color_b] =	CREATE_OFFSET( color.b() );
		Interpolator::s_offsets[Interpolator::Color_a] =	CREATE_OFFSET( color.a() );
	}
};


// ClippedTriangleInfo - used to map bsp output back to the original mesh
struct ClippedTriangleInfo
{
	physx::PxU32	planeIndex;
	physx::PxU32	originalTriangleIndex;
	physx::PxU32	clippedTriangleIndex;
	physx::PxU32	ccw;

	static	int	cmp( const void* a, const void* b )
				{
					const int planeIndexDiff = (int)((ClippedTriangleInfo*)a)->planeIndex - (int)((ClippedTriangleInfo*)b)->planeIndex;
					if( planeIndexDiff != 0 )
					{
						return planeIndexDiff;
					}
					const int originalTriangleDiff = (int)((ClippedTriangleInfo*)a)->originalTriangleIndex - (int)((ClippedTriangleInfo*)b)->originalTriangleIndex;
					if( originalTriangleDiff != 0 )
					{
						return originalTriangleDiff;
					}
					return (int)((ClippedTriangleInfo*)a)->clippedTriangleIndex - (int)((ClippedTriangleInfo*)b)->clippedTriangleIndex;
				}
};

// BSPLink - a link with an "isBSP" method to act as a stop
class BSPLink : public Link, public physx::UserAllocated
{
public:
	virtual bool	isBSP()	{ return false; }

	BSPLink*		getAdjBSP( physx::PxU32 which ) const
					{
						if( isSolitary() )
						{
							return NULL;
						}
						BSPLink* adj = static_cast<BSPLink*>( getAdj( which ) );
						return adj->isBSP() ? adj : NULL;
					}

	void			removeBSPLink()
					{
						BSPLink* adj = static_cast<BSPLink*>( getAdj( 1 ) );
						remove();
						if( !adj->isBSP() && adj->isSolitary() )
						{
							delete adj;
						}
					}
};


// IApexBSP implementation
class BSP : public IApexBSP, public BSPLink
{
public:
					BSP( IApexBSPMemCache* memCache = NULL );
					~BSP();

	// IApexBSP implementation
	bool			fromMesh( const physx::Array<NxExplicitRenderTriangle>& mesh, const BSPBuildParameters& params, IProgressListener* progressListener = NULL );
	bool			combine( const IApexBSP& bsp );
	bool			op( const IApexBSP& combinedBSP, Operation::Enum operation );
	bool			complement();
	bool			isTrivial( bool* isAllSpace = NULL ) const;
	bool			isCombined() const;
#if !USE_GSA
	physx::PxF32	getVolume( Operation::Enum operation = Operation::NOP ) const;
#endif
	physx::PxF32	getTriangleArea( Operation::Enum operation = Operation::NOP ) const;
	bool			toMesh( physx::Array<NxExplicitRenderTriangle>& mesh, physx::PxF32 cleaningTolerance = 0 ) const;
	void			copy( const IApexBSP& bsp, const NxMat34& tm = NxMat34() );
#if USE_GSA
	IApexBSP*		decomposeIntoIslands() const;
	IApexBSP*		getNext() const { return static_cast<BSP*>( getAdjBSP(1) ); }
	IApexBSP*		getPrev() const { return static_cast<BSP*>( getAdjBSP(0) ); }
#endif
	void			serialize( physx::PxFileBuf& stream ) const;
	void			deserialize( physx::PxFileBuf& stream );
	void			visualize( NxApexRenderDebug& debugRender, physx::PxU32 flags, physx::PxU32 index = 0 ) const;
	void			release();

	// BSPLink
	bool			isBSP() { return true; }

	// Node, a binary node with geometric data
	class Node : public BinaryNode
	{
	public:
		enum Type { Leaf, Branch };

#if !USE_GSA
								Node() : m_type(Leaf), m_leafData(NULL)	{}
#else
								Node() : m_type(Leaf)					{ m_leafData.side = 1; }
#endif
					Node&		operator = ( const Node& )				{}	// No assignment

#if !USE_GSA
		PX_INLINE	void		setLeafData( Region* leafData )			{ m_type = Leaf;	m_leafData = leafData; }
		PX_INLINE	void		setBranchData( Surface* branchData )	{ m_type = Branch;	m_branchData = branchData; }
#else
		PX_INLINE	void		setLeafData( const Region& leafData )		{ m_type = Leaf;	m_leafData = leafData; }
		PX_INLINE	void		setBranchData( const Surface& branchData )	{ m_type = Branch;	m_branchData = branchData; }
#endif

		PX_INLINE	Type		getType()						const	{ return (Type)m_type; }

#if !USE_GSA
		PX_INLINE	Region*		getLeafData()					const	{ PX_ASSERT( getType() == Leaf ); return m_leafData; }
		PX_INLINE	Surface*	getBranchData()					const	{ PX_ASSERT( getType() == Branch ); return m_branchData; }
#else
		PX_INLINE	Region*		getLeafData()							{ PX_ASSERT( getType() == Leaf ); return &m_leafData; }
		PX_INLINE	Surface*	getBranchData()							{ PX_ASSERT( getType() == Branch ); return &m_branchData; }
		PX_INLINE	const Region*	getLeafData()				const	{ PX_ASSERT( getType() == Leaf ); return &m_leafData; }
		PX_INLINE	const Surface*	getBranchData()				const	{ PX_ASSERT( getType() == Branch ); return &m_branchData; }
#endif

		PX_INLINE	Node*		getParent()						const	{ return (Node*)BinaryNode::getParent(); }
		PX_INLINE	Node*		getChild( physx::PxU32 index )	const	{ return (Node*)BinaryNode::getChild( index ); }

	protected:
		physx::PxU32	m_type;

		union
		{
#if !USE_GSA
			Region*		m_leafData;
			Surface*	m_branchData;
#else
			Region		m_leafData;
			Surface		m_branchData;
#endif
		};
	};

#if USE_GSA
	class RegionShape : public physx::GSA::GSA<3,Real>::ConvexShape, public physx::GSA::GSA<3,Real>
	{
	public:
							RegionShape( const physx::GSA::Plane<3,Real>* planes, Real skinWidth = (Real)0 ) : m_planes( planes ), m_leaf(NULL), m_nonempty( true ), m_skinWidth( skinWidth ), physx::GSA::GSA<3,Real>()
							{
								init( 100 );
							}

		virtual	unsigned	initialize_tangent_planes( physx::GSA::Plane<3,Real>* planes, unsigned plane_count ) const;
		virtual	void		intersect_line( physx::GSA::GSA<3,Real>::LineIntersect& in, physx::GSA::GSA<3,Real>::LineIntersect& ex, const physx::GSA::Pos<3,Real>& orig, const physx::GSA::Dir<3,Real>& dir, Real time ) const;
		virtual physx::GSA::Dir<3,Real>	get_linear_velocity() const	{ return physx::GSA::Dir<3,Real>(); }

				void		set_leaf( const BSP::Node* leaf )
							{
								const bool init = m_leaf == NULL && leaf != NULL;
								m_leaf = leaf;
								if( init )
								{
									set_shapes( this );
								}
								else
								{
									m_shape[0] = this;
								}
							}

				void		calculate() { m_nonempty = intersect(); }

				bool		is_nonempty() const { return m_nonempty; }

	private:
		const physx::GSA::Plane<3,Real>*	m_planes;
		const BSP::Node*					m_leaf;
		bool								m_nonempty;
		Real								m_skinWidth;
	};
#endif	// #if USE_GSA

private:
	class BoolOp
	{
	public:
				BoolOp( Operation::Enum op ) : c_ba(((physx::PxU32)op>>3)&1), c_b(((physx::PxU32)op>>2)&1), c_a(((physx::PxU32)op>>1)&1), c_k((physx::PxU32)op&1)	{}

		physx::PxU32	operator ()	( physx::PxU32 a, physx::PxU32 b ) const	{ return (c_ba & a & b) ^ (c_b & b) ^ (c_a & a) ^ c_k; }

	private:
		physx::PxU32	c_ba, c_b, c_a, c_k;
	};

	struct BuildConstants
	{
		physx::PxU32		m_testSetSize;
		physx::PxF32		m_splitWeight;
		physx::PxF32		m_imbalanceWeight;
		physx::PxF32		m_recipMaxArea;
	};

	void		clear();

	void		transform( const Mat4Real& tm, bool transformFrames = true );

	// Returns the area of the clipped mesh.  clippedMesh and triangleInfo may be NULL, in which case nothins is done but
	// the area calculation.
	Real		clipMeshToLeaf( physx::Array<Triangle>* clippedMesh, physx::Array<ClippedTriangleInfo>* triangleInfo, const Node* leaf ) const;

	// Recursive functions
	void		complementLeaves( Node* node ) const;
	void		mergeLeaves( const BoolOp& op, Node* node );
	void		clipMeshToLeaves( physx::Array<Triangle>& clippedMesh, physx::Array<ClippedTriangleInfo>& triangleInfo, Node* node ) const;
	void		clone( Node* node, const Node* original, bool duplicateData = true );
#if !USE_GSA
	void		combineTrees( Node* node, const Node* combineNode, physx::PxU32 triangleIndexOffset, physx::PxU32 planeIndexOffset );
	bool		buildTree( Node* node, physx::Array<Surface*>& surfaceStack, physx::PxU32 stackReadStart, physx::PxU32 stackReadStop,
#else
	void		combineTrees( Node* node, const Node* combineNode, physx::PxU32 triangleIndexOffset, physx::PxU32 planeIndexOffset, RegionShape& regionShape );
	bool		buildTree( Node* node, physx::Array<Surface>& surfaceStack, physx::PxU32 stackReadStart, physx::PxU32 stackReadStop,
#endif
						   const BuildConstants& buildConstants, IProgressListener* progressListener );
	void		transform( Node* node, const Mat4Real& tm, const Mat4Real& cofTM );
	void		visualizeNode( NxApexRenderDebug& debugRender, physx::PxU32 flags, const Node* node ) const;
	const Node*	findIndexedLeaf( const Node* node, physx::PxU32 index, physx::PxU32& count ) const;
#if !USE_GSA
	void		addLeafVolumes( const Node* node, Real volumes[2], const BoolOp& op ) const;
#endif
	Real		addInsideLeafTriangleAreas( const Node* node, const BoolOp& op ) const;
	void		serializeNode( const Node* node, physx::PxFileBuf& stream ) const;
	Node*		deserializeNode( physx::PxU32 version, physx::PxFileBuf& stream );
	void		releaseNode( Node* node );
#if USE_GSA
	void		indexInsideLeaves( physx::PxU32& index, Node* node ) const;
	void		listInsideLeaves( physx::Array<Node*>& insideLeaves, Node* node ) const;
	void		findInsideLeafNeighbors( physx::Array<IntPair>& neighbors, class LeafNeighborIntersection& intersection, Node* node ) const;
#endif

	// Tree
	Node*						m_root;

	// Internal mesh representation
	physx::Array<Triangle>		m_mesh;
	physx::Array<Interpolator>	m_frames;
	Real						m_meshSize;

	// Unique splitting planes
	physx::Array<Plane>			m_planes;

	// Combination data
	bool						m_combined;
	Real						m_combiningMeshSize;

	// Memory cache
	class BSPMemCache*			m_memCache;
	bool						m_ownsMemCache;
};


// Surface iterator; walks from a leaf's parent to the root of a tree, allowing inspection of surfaces along the way
class SurfaceIt
{
public:
#if !USE_GSA
	PX_INLINE					SurfaceIt( const BSP::Node* leaf ) : m_current( leaf )	{ PX_ASSERT( leaf != NULL && leaf->getType() == BSP::Node::Leaf ); inc(); }
#else
	PX_INLINE					SurfaceIt() : m_current( NULL ), m_side( 0xFFFFFFFF )				{}
	PX_INLINE					SurfaceIt( const BSP::Node* leaf ) : m_current( (BSP::Node*)leaf )	{ PX_ASSERT( leaf != NULL && leaf->getType() == BSP::Node::Leaf ); inc(); }
#endif

	PX_INLINE	bool			isValid()	const										{ return m_current != NULL; }

	PX_INLINE	void			inc()													{ m_side = m_current->getIndex(); m_current = m_current->getParent(); }

	PX_INLINE	const Surface*	surface()	const										{ return m_current->getBranchData(); }

	PX_INLINE	physx::PxU32	side()		const										{ return m_side; }

private:
#if !USE_GSA
	const BSP::Node*	m_current;
#else
	BSP::Node*			m_current;
#endif
	physx::PxU32		m_side;
};


// IBSPMemCache implementation, several pools and growable arrays.  Not global, so that concurrent calculations can use different pools
class BSPMemCache : public IApexBSPMemCache, public physx::UserAllocated
{
public:

			BSPMemCache();

	void	clearAll();
	void	clearTemp();

	void	release();

	// Persistent data
	Pool<BSP::Node>		m_nodePool;
#if !USE_GSA
	Pool<Surface>		m_surfacePool;
	Pool<Region>		m_regionPool;
#endif

	// Temporary data
	Pool<LinkedVertex>	m_linkedVertexPool;
	physx::Array<NxU8>	m_surfaceFlags;
	physx::Array<NxU8>	m_surfaceTestFlags;
};


// Mesh cleaning interface
void
cleanMesh( physx::Array<NxExplicitRenderTriangle>& cleanedMesh, const physx::Array<Triangle>& mesh, physx::Array<ClippedTriangleInfo>& triangleInfo, physx::PxU32 planeCount, const physx::Array<Triangle>& originalTriangles, const physx::Array<Interpolator>& frames, Real distanceTol );

};	// namespace ApexCSG
};

#endif

#endif // #define APEX_CSG_DEFS_H
