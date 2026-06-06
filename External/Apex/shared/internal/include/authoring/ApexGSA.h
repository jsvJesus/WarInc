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
#ifndef APEX_GSA_H
#define APEX_GSA_H


#include "PsShare.h"
#include "NxRenderMeshAsset.h"

#ifndef WITHOUT_APEX_AUTHORING

namespace physx
{
namespace apex
{
namespace GSA
{
#define _USE_MATH_DEFINES
#include <math.h>
#include <float.h>


/* Integer abbreviations */

typedef	signed		__int8	i8;
typedef	unsigned	__int8	u8;
typedef	signed		__int16	i16;
typedef	unsigned	__int16	u16;
typedef	signed		__int32	i32;
typedef	unsigned	__int32	u32;
typedef	signed		__int64	i64;
typedef	unsigned	__int64	u64;


/* Utilities */

// Square
template<typename T>
inline T		square( T x )	{ return x*x; }

// Sign
template<typename T>
inline T		sgn( T x )		{ return x >= (T)0 ? (T)1 : -(T)1; }

template<typename T>
inline T		sgnz( T x )		{ return x == (T)0 ? (T)0 : (x > (T)0 ? (T)1 : -(T)1); }

// Special real values
template<typename T>
inline T		max_real()	{ return (T)0; }
template<>
inline float	max_real()	{ return FLT_MAX; }
template<>
inline double	max_real()	{ return DBL_MAX; }

template<typename T>
inline T		min_real()	{ return (T)0; }
template<>
inline float	min_real()	{ return FLT_MIN; }
template<>
inline double	min_real()	{ return DBL_MIN; }

template<typename T>
inline T		eps_real()	{ return (T)0; }
template<>
inline float	eps_real()	{ return FLT_EPSILON; }
template<>
inline double	eps_real()	{ return DBL_EPSILON; }


/* Matrix */

template<int R, int C, typename F>
class Mat
{
public:
					Mat()										{ for ( int j = 0; j < C; ++j ) for( int i = 0; i < R; ++i ) (*this)(i,j) = (F)0; }

	F&				operator ()	( int rowN, int colN )			{ return e[colN][rowN]; }
	const F&		operator ()	( int rowN, int colN )	const	{ return e[colN][rowN]; }

	void			setCol( int colN, const Mat<R,1,F>& col )	{ for( int i = 0; i < R; ++i ) (*this)(i,colN) = col(i,0); }
	Mat<R,1,F>		getCol( int colN )					const	{ Mat<R,1,F> col; for( int i = 0; i < R; ++i ) col(i,0) = (*this)(i,colN); return col; }
	void			setRow( int rowN, const Mat<1,C,F>& row )	{ for( int i = 0; i < C; ++i ) (*this)(rowN,i) = row(0,i); }
	Mat<1,C,F>		getRow( int rowN )					const	{ Mat<1,C,F> row; for( int i = 0; i < C; ++i ) row(0,i) = (*this)(rowN,i); return row; }

	Mat<R,C,F>		operator -	()						const	{ Mat<R,C,F> r; for( int j = 0; j < C; ++j ) for( int i = 0; i < R; ++i ) r(i,j) = -(*this)(i,j); return r; }

	Mat<R,C,F>		operator *	( F s )					const	{ Mat<R,C,F> r; for( int j = 0; j < C; ++j ) for( int i = 0; i < R; ++i ) r(i,j) = (*this)(i,j)*s; return r; }
	Mat<R,C,F>		operator /	( F s )					const	{ return (*this)*((F)1/s); }
	Mat<R,C,F>&		operator *=	( F s )							{ for( int j = 0; j < C; ++j ) for( int i = 0; i < R; ++i ) (*this)(i,j) *= s; return *this; }
	Mat<R,C,F>&		operator /=	( F s )							{ *this *= ((F)1/s); return *this; }

	Mat<R,C,F>		operator +	( const Mat<R,C,F>& m )	const	{ Mat<R,C,F> r; for( int j = 0; j < C; ++j ) for( int i = 0; i < R; ++i ) r(i,j) = (*this)(i,j)+m(i,j); return r; }
	Mat<R,C,F>		operator -	( const Mat<R,C,F>& m )	const	{ Mat<R,C,F> r; for( int j = 0; j < C; ++j ) for( int i = 0; i < R; ++i ) r(i,j) = (*this)(i,j)-m(i,j); return r; }
	Mat<R,C,F>&		operator +=	( const Mat<R,C,F>& m )			{ for( int j = 0; j < C; ++j ) for( int i = 0; i < R; ++i ) (*this)(i,j) += m(i,j); return *this; }
	Mat<R,C,F>&		operator -=	( const Mat<R,C,F>& m )			{ for( int j = 0; j < C; ++j ) for( int i = 0; i < R; ++i ) (*this)(i,j) -= m(i,j); return *this; }

	template<int S>
	Mat<R,S,F>		operator *	( const Mat<C,S,F>& m )	const	{ Mat<R,S,F> r; for( int j = 0; j < S; ++j ) for( int i = 0; i < R; ++i ) for( int k = 0; k < C; ++k ) r(i,j) += (*this)(i,k)*m(k,j); return r; }

	Mat<C,R,F>		T()									const	{ Mat<C,R,F> t; for( int j = 0; j < C; ++j ) for( int i = 0; i < R; ++i ) t(j,i) = (*this)(i,j); return t; }

private:
	F	e[C][R];
};

template<int R, int C, typename F>
inline Mat<R,C,F>
operator * ( F s, const Mat<R,C,F>& m )
{
	return m*s;
}


/* Square matrix operations */

// Unit matrix
template<int D, typename F>
inline Mat<D,D,F>
unit()
{
	Mat<D,D,F> u;
	for( int i = 0; i < D; ++i ) u(i,i) = (F)1;
	return u;
}

// Minor of a square matrix element
template<int D, typename F>
inline Mat<D-1,D-1,F>
minor( const Mat<D,D,F>& m, int rowN, int colN )
{
	Mat<D-1,D-1,F> n;
	for( int i = 0, k = 0; i < D; ++i ) if( i != rowN ) { for( int j = 0, l = 0; j < D; ++j ) if( j != colN ) { n(k,l) = m(i,j); ++l; } ++k; }
	return n;
}

// Cofactor of a square matrix element
template<int D, typename F>
inline F
cof( const Mat<D,D,F>& m, int rowN, int colN )
{
	const Mat<D-1,D-1,F> n = minor( m, rowN, colN );
	F detN = 0; for( int i = 0; i < D-1; ++i ) detN += n( i, 0 )*cof( n, i, 0 );
	return (F)((int)1-(int)(((rowN+colN)&1)<<1))*detN;
}

// Cofactor of a square matrix element - specialization for 1x1 matrix
template<typename F>
inline F
cof( const Mat<1,1,F>&, int, int ) { return (F)1; }

// Cofactor matrix
template<int D, typename F>
inline Mat<D,D,F>
cof( const Mat<D,D,F>& m ) { Mat<D,D,F> c; for( int i = 0; i < D; ++i ) for( int j = 0; j < D; ++j ) c(i,j) = cof( m, i, j ); return c; }

// Determinant
template<int D, typename F>
inline F
det( const Mat<D,D,F>& m ) { F d = 0; for( int i = 0; i < D; ++i ) d += cof( m, i, D-1 )*m(i,D-1); return d; }

// Inverse
template<int D, typename F>
inline Mat<D,D,F>	
inv( const Mat<D,D,F>& m )
{
	const Mat<D,D,F> c = cof( m );
	F d = 0;
	for( int i = 0; i < D; ++i ) d += c(i,D-1)*m(i,D-1);
	return d == 0 ? Mat<D,D,F>() : c.T()/d;
}


/* Column vector defined as a Dx1 matrix */

template<int D, typename F>
class Col : public Mat<D,1,F>
{
public:
						Col()										{}
						Col( const Mat<D,1,F>& v )					{ for( int i = 0; i < D; ++i ) (*this)(i) = v(i,0); }
						Col( const Mat<D+1,1,F>& v )				{ for( int i = 0; i < D; ++i ) (*this)(i) = v(i,0); }
						Col( const Col<D+1,F>& v )					{ for( int i = 0; i < D; ++i ) (*this)(i) = v(i); }

	Col<D,F>&			set( F c )									{ for( int i = 0; i < D; ++i ) (*this)(i) = c; return *this; }

	F&					operator ()	( int i )						{ return Mat<D,1,F>::operator()(i,0); }
	const F&			operator ()	( int i )				const	{ return Mat<D,1,F>::operator()(i,0); }

	const Mat<1,D,F>&	T()									const	{ return *(const Mat<1,D,F>*)this; }

	F					operator | ( const Col<D,F>& v )	const	{ return (T()*v)(0,0); }

	F					lengthSquared()						const	{ return (*this)|(*this); }
	F					length()							const	{ return sqrt( lengthSquared() ); }

	F					normalize()
						{
							const F l2 = lengthSquared();
							const F recipL = l2 > (F)0 ? (F)1/sqrt( l2 ) : (F)0;
							*this *= recipL;
							return recipL*l2;
						}

	Col<D,F>			normal()							const	{ Col<D,F> r = *this; r.normalize(); return r; }

protected:
	/* Vector builder */
	class Builder
	{
	public:
					Builder( Col& v, const F& t ) : m_v( v ), m_index( 1 )	{ v(0) = t; }

		Builder&	operator , ( const F& t )								{ if( m_index < D ) m_v(m_index++) = t; return *this; }

	private:
		Col&	m_v;
		int		m_index;
	};

public:
	Builder		operator << ( const F& t )		{ return Builder( *this, t ); }
};

// Column vector build functions
template<typename F>
inline Col<2,F>	col2( F x, F y )			{ Col<2,F> v; v << x, y; return v; }
template<typename F>
inline Col<3,F>	col3( F x, F y, F z )		{ Col<3,F> v; v << x, y, z; return v; }
template<typename F>
inline Col<4,F>	col4( F x, F y, F z, F w )	{ Col<4,F> v; v << x, y, z, w; return v; }


/* Row vector defined as a 1xD matrix */

template<int D, typename F>
class Row : public Mat<1,D,F>
{
public:
						Row()										{}
						Row( const Mat<1,D,F>& v )					{ for( int i = 0; i < D; ++i ) (*this)(i) = v(0,i); }
						Row( const Mat<1,D+1,F>& v )				{ for( int i = 0; i < D; ++i ) (*this)(i) = v(0,i); }
						Row( const Row<D+1,F>& v )					{ for( int i = 0; i < D; ++i ) (*this)(i) = v(i); }

	Row<D,F>&			set( F c )									{ for( int i = 0; i < D; ++i ) (*this)(i) = c; return *this; }

	F&					operator ()	( int i )						{ return Mat<1,D,F>::operator()(0,i); }
	const F&			operator ()	( int i )				const	{ return Mat<1,D,F>::operator()(0,i); }

	const Mat<D,1,F>&	T()									const	{ return *(const Mat<D,1,F>*)this; }

	F					operator | ( const Row<D,F>& v )	const	{ return (v*T())(0,0); }

	F					lengthSquared()						const	{ return (*this)|(*this); }
	F					length()							const	{ return sqrt( lengthSquared() ); }

	F					normalize()
						{
							const F l2 = lengthSquared();
							const F recipL = l2 > (F)0 ? (F)1/sqrt( l2 ) : (F)0;
							*this *= recipL;
							return recipL*l2;
						}

	Row<D,F>			normal()							const	{ Row<D,F> r = *this; r.normalize(); return r; }

protected:
	/* Vector builder */
	class Builder
	{
	public:
					Builder( Row& v, const F& t ) : m_v( v ), m_index( 1 )	{ v(0) = t; }

		Builder&	operator , ( const F& t )								{ if( m_index < D ) m_v(m_index++) = t; return *this; }

	private:
		Row&	m_v;
		int		m_index;
	};

public:
	Builder		operator << ( const F& t )		{ return Builder( *this, t ); }
};

// Row vector build functions
template<typename F>
inline Row<2,F>	row2( F x, F y )			{ Row<2,F> v; v << x, y; return v; }
template<typename F>
inline Row<3,F>	row3( F x, F y, F z )		{ Row<3,F> v; v << x, y, z; return v; }
template<typename F>
inline Row<4,F>	row4( F x, F y, F z, F w )	{ Row<4,F> v; v << x, y, z, w; return v; }


/* Direction, homogeneous representation */

template<int D, typename F>
class Dir : public Col<D+1,F>
{
public:
				Dir()									{ set( (F)0 ); }
				Dir( const Col<D,F>& v )				{ *this = v; }
				Dir( const Col<D+1,F>& v )				{ *this = v; }
				Dir( const Mat<D+1,1,F>& v )			{ *this = v; }

	Dir<D,F>&	operator = ( const Col<D,F>& v )		{ for( int i = 0; i < D; ++i ) (*this)(i) = v(i); (*this)(D) = (F)0; return *this; }
	Dir<D,F>&	operator = ( const Col<D+1,F>& v )		{ for( int i = 0; i < D; ++i ) (*this)(i) = v(i); (*this)(D) = (F)0; return *this; }
	Dir<D,F>&	operator = ( const Mat<D+1,1,F>& v )	{ for( int i = 0; i < D; ++i ) (*this)(i) = v(i,0); (*this)(D) = (F)0; return *this; }
																					   
	Dir<D,F>&	set( F c )								{ for( int i = 0; i < D; ++i ) (*this)(i) = c; (*this)(D) = (F)0; return *this; }
};


/* Position, homogeneous representation */

template<int D, typename F>
class Pos : public Col<D+1,F>
{
public:
				Pos()									{ set( (F)0 ); }
				Pos( const Col<D,F>& v )				{ *this = v; }
				Pos( const Col<D+1,F>& v )				{ *this = v; }
				Pos( const Mat<D+1,1,F>& v )			{ *this = v; }

	Pos<D,F>&	operator = ( const Col<D,F>& v )		{ for( int i = 0; i < D; ++i ) (*this)(i) = v(i); (*this)(D) = (F)1; return *this; }
	Pos<D,F>&	operator = ( const Col<D+1,F>& v )		{ for( int i = 0; i < D; ++i ) (*this)(i) = v(i); (*this)(D) = (F)1; return *this; }
	Pos<D,F>&	operator = ( const Mat<D+1,1,F>& v )	{ for( int i = 0; i < D; ++i ) (*this)(i) = v(i,0); (*this)(D) = (F)1; return *this; }

	Pos<D,F>&	set( F c )								{ for( int i = 0; i < D; ++i ) (*this)(i) = c; (*this)(D) = (F)1; return *this; }
};


/* Specializations */

template<int D, typename F>
inline Dir<D,F>
operator ^ ( const Dir<D,F>& a, const Dir<D,F>& b )
{
	return Dir<D,F>();
}

// 3D cross product
template<typename F>
inline Dir<3,F>
operator ^ ( const Dir<3,F>& a, const Dir<3,F>& b )
{
	Dir<3,F> c;
	c << (a(1)*b(2) - a(2)*b(1)), (a(2)*b(0) - a(0)*b(2)), (a(0)*b(1) - a(1)*b(0));
	return c;
}


/* Plane equation defined as a (D+1)-dimensional column vector */

template<int D, typename F>
class Plane : public Col<D+1,F>
{
public:
				Plane()												{}
				Plane( const Dir<D,F>& n, F d )						{ set( n, d ); }
				Plane( const Dir<D,F>& n, const Pos<D,F>& p )		{ set( n, p ); }
				Plane( const Col<D+1,F>& v ) 						{ for( int i = 0; i < D+1; ++i ) (*this)(i) = v(i); }
				Plane( const Mat<D+1,1,F>& v ) 						{ for( int i = 0; i < D+1; ++i ) (*this)(i) = v(i,0); }

	void		set( const Dir<D,F>& n, F d )						{ for( int i = 0; i < D; ++i ) (*this)(i) = n(i); (*this)(D) = d; }
	void		set( const Dir<D,F>& n, const Pos<D,F>& p )			{ for( int i = 0; i < D; ++i ) (*this)(i) = n(i); (*this)(D) = -(n|p); }

	Dir<D,F>	n()											const	{ return Dir<D,F>( *this ); }
	F			d()											const	{ return (*this)(D); }

	Pos<D,F>	project( const Pos<D,F>& p = Pos<D,F>() )	const	{ return p - n()*(p|*this); }

	F			normalize()
				{
					const F l2 = n().lengthSquared();
					const F recipL = l2 > (F)0 ? (F)1/sqrt( l2 ) : (F)0;
					*this *= recipL;
					return recipL*l2;
				}
};


/* Specializations */

// Hodge star in 3D
template<typename F>
inline Mat<3,3,F>
operator * ( const Col<3,F>& v )
{
	Mat<3,3,F> starV;
	starV(0,1) = -v(2);
	starV(0,2) = v(1);
	starV(1,0) = v(2);
	starV(1,2) = -v(0);
	starV(2,0) = -v(1);
	starV(2,1) = v(0);
	return starV;
}

// Rotation in 2D through angle theta
template<typename F>
inline Mat<2,2,F>
rotationMatrix( F theta )
{
	const F c = cos( theta );
	const F s = sin( theta );
	Mat<2,2,F> m;
	m(0,0) = c;
	m(0,1) = -s;
	m(1,0) = s;
	m(1,1) = c;
	return m;
}

// Rotation in 3D about axis (normalized), through angle theta
template<typename F>
inline Mat<3,3,F>
rotationMatrix( F theta, const Col<3,F>& axis )
{
	const F c = cos( theta );
	return c*unit<3,F>() + ((F)1-c)*(axis*axis.T()) + sin( theta )*(*axis);
}

// Build a homogeneous transform
template<int D, typename F>
inline Mat<D+1,D+1,F>
homogeneousTransform( const Mat<D,D,F>& rotation, const Col<D,F>& scale, const Pos<D,F>& position )
{
	Mat<D+1,D+1,F> tm;
	for( int i = 0; i < D; ++i )
	{
		for( int j = 0; j < D; ++j )
		{
			tm(i,j) = rotation(i,j)*scale(j);
		}
		tm(i,D) = position(i);
	}
	for( int j = 0; j < D; ++j )
	{
		tm(D,j) = (F)0;
	}
	tm(D,D) = (F)1;
	return tm;
}


/* Local functions */

// Jacobi rotation used in diagonalize_symmetric (below)
template<int D, typename F>
inline void
Jacobi_rotate( Mat<D,D,F>& A, F s, F tau, int i, int j, int k, int l ) { const F g = A(i,j); const F h = A(k,l); A(i,j) = g-s*(h+g*tau); A(k,l) = h+s*(g-h*tau); }


/*
	Computes all eigenvalues and eigenvectors of a real symmetric matrix M.  On output,
	L returns the eigenvalues of M, and V is a matrix of normalized (column) eigenvectors
	of M.  maxIter is the maximum number of iterations that will be used. Returns true
	iff M is diagonalized in maxIter iterations or fewer.
*/
template<int D, typename F>
bool
diagonalize_symmetric( Col<D,F>& L, Mat<D,D,F>& V, const Mat<D,D,F>& M, int maxIter = 100 )
{
	// Copy M so we can modify it
	Mat<D,D,F> A( M );

	// Initialize to the identity matrix.
	V = unit<D,F>();

	// Initialize L to the diagonal of A.
	for( int ip = 0; ip < D; ++ip ) L(ip) = A(ip,ip);

	Col<D,F> b( L );	// Initialize b to L
	for( int iter = 0; iter < maxIter; ++iter )
	{
		// Sum off-diagonal elements.
		F sm = (F)0; for( int ip = 0; ip < D-1; ++ip ) for( int iq = ip+1; iq < D; ++iq ) sm += abs( A(ip,iq) );
		if( sm == (F)0 )
		{ //  The normal return, which relies on quadratic convergence to machine underflow.
			return true;
		}
		Col<D,F> z;
		z.set( 0.0f );
		const F thresh = iter < 3 ? (F)0.2*sm/(D*D) : (F)0;
		for( int ip = 0; ip < D-1; ++ip )
		{
			for( int iq = ip+1; iq < D; ++iq )
			{
				const F g = (F)100*abs( A(ip,iq) );
				// After four sweeps, skip the rotation if the off-diagonal element is small.
				if( iter >= 4 && abs( L(ip) ) + g == abs( L(ip) ) && abs( L(iq) ) + g == abs( L(iq) ) )
				{
					A(ip,iq) = (F)0;
				}
				else
				if( abs( A(ip,iq) ) > thresh )
				{
					F h = L(iq)-L(ip);
					F t;
					if( abs(h) + g == abs(h) )
					{
						t = A(ip,iq)/h;
					}
					else
					{
						const F theta = (F)0.5*h/A(ip,iq); // Equation (11.1.10)
						t = (F)1/( theta + sgn( theta )*sqrt( (F)1 + square( theta ) ) );
					}
					const F c = (F)1/sqrt( (F)1 + square( t ) );
					const F s = t*c;
					const F tau = s/( (F)1 + c );
					h = t*A(ip,iq);
					z(ip) -= h;
					z(iq) += h;
					L(ip) -= h;
					L(iq) += h;
					A(ip,iq) = (F)0;
					for( int j = 0; j < ip; ++j ) Jacobi_rotate<D,F>( A, s, tau, j, ip, j, iq );
					for( int j = ip+1; j < iq; ++j ) Jacobi_rotate<D,F>( A, s, tau, ip, j, j, iq );
					for( int j = iq+1; j < D; ++j ) Jacobi_rotate<D,F>( A, s, tau, ip, j, iq, j );
					for( int j = 0; j < D; ++j ) Jacobi_rotate<D,F>( V, s, tau, j, ip, j, iq );
				}
			}
		}
		b += z;
		L = b; // Update L with the sum of tapq,
	}

	return false;
}


/*
	Decomposes M into M = U*W*V^F, where U and V are orthonormal and W is
	diagonal positive semidefinite.  Both V and W are square with size
	equal to the number of columns of M.
	Only returns V and the diagonal of W (in vector form).
	Function returns true iff the decomposition is successful.
*/
template<int R, int C, typename F>
inline bool
svd_partial( Col<C,F>& W, Mat<C,C,F>& V, const Mat<R,C,F>& M, int maxIter = 100 )
{
	const bool success = diagonalize_symmetric<C,F>( W, V, M.T()*M, maxIter );
	for( int i = 0; i < C; ++i ) W(i) = W(i) > (F)0 ? sqrt( W(i) ) : (F)0;
	return success;
}


/*
	Decomposes M into M = U*W*V^F, where U and V are orthonormal and W is
	diagonal positive semidefinite.  Both V and W are square with size
	equal to the number of columns of M, while U is the size of M.
	Returns U, V, and the diagonal of W (in vector form).
	Function returns true iff the decomposition is successful.
*/
template<int R, int C, typename F>
inline bool
svd( Mat<R,C,F>& U, Col<C,F>& W, Mat<C,C,F>& V, const Mat<R,C,F>& M, F threshold, int maxIter = 100 )
{
	const bool success = svd_partial<R,C,F>( W, V, M, maxIter );
	Col<C,F> invW;
	for( int i = 0; i < C; ++i ) invW[i] = W(i) >= threshold ? (F)1/W(i) : (F)0;
	U = M*V;
	for( int i = 0; i < R; ++i ) for( int j = 0; j < C; ++j ) U(i,j) *= invW(j);
	return success;
}


/*
	Calculates the pseudoinverse of M using the SVD decomposition M = U*W*V^F.
	The pseudoinverse(M) = V*pseudoinverse(W)*U^F, where
		pseudoinverse(W)_ij = W_ij >= threshold ? 1/W_ij : 0
	Function returns true iff the decomposition is successful.
*/
template<int R, int C, typename F>
inline bool
pseudoinverse( Mat<C,R,F>& inv, const Mat<R,C,F>& M, F threshold, int maxIter = 100 )
{
	Mat<C,R,F> MT = M.T();
	Mat<C,C,F> V;
	Col<C,F> L;
	const bool success = diagonalize_symmetric<C,F>( L, V, MT*M, maxIter );

	const F threshold2 = threshold*threshold;
	Mat<C,R,F> invW2VTMT = V.T()*MT;
	for( int i = 0; i < C; ++i ) invW2VTMT.setRow( i, invW2VTMT.getRow( i ) * (L(i) >= threshold2 ? (F)1/L(i) : (F)0) );

	inv = V*invW2VTMT;

	return success;		
}


/* Implementation of the Guiding Simplex Algorithm */

template<typename F>
inline F	gsa_eps()	{ return 1000000*eps_real<F>(); }


template<int D, typename F>
static Mat<D,D,F>
reduce_simplex( int colToEliminate, const Mat<D+1,D+1,F>& S )
{
	Mat<D,D,F> N;
	Mat<D+1,D,F> colReducedS;
	int colN = 0;
	for( int i = 0; i < D+1; ++i )
	{
		if( i != colToEliminate )
		{
			const Plane<D,F> plane = S.getCol( i );
			colReducedS.setCol( colN, plane );
			N.setRow( colN, Row<D,F>( plane.n().T() ) );
			++colN;
		}
	}
	Mat<D,D,F> B;
	Col<D,F> W;
	svd_partial( W, B, N );
	Mat<D,D+1,F> R;
	colN = 0;
	int leastW = 0;
	for( int i = 1; i < D; ++i )
	{
		int srcCol = i;
		if( W(i) < W(leastW) )
		{
			srcCol = leastW;
			leastW = i;
		}
		R.setRow( colN++, Dir<D,F>( B.getCol( srcCol ) ).T() );
	}
	R.setRow( D-1, Pos<D,F>().T() );
	return R*colReducedS;
}

// Returns the dimension of the space spanned by the plane normals, unless the simplex is void,
// in which case this function returns 0.
template<int D, typename F>
static int
simplex_condition( F& detS, Row<D+1,F>& cofSRowD, const Mat<D+1,D+1,F>& S )
{
	for( int col = 0; col < D+1; ++col ) cofSRowD(col) = cof( S, D, col );
	detS = cofSRowD|S.getRow( D );

	if( abs( detS ) > gsa_eps<F>() )
	{	// w_i = -det(S)/cof(S)_{D,i}
		for( int i = 0; i < D+1; ++i )
		{
			if( -cofSRowD(i)/detS > gsa_eps<F>() )	// m_recipWidths[i] = -cofSRowD(i)/m_detS; condition += (int)(m_recipWidths[i] > 0);
			{
				return D;
			}
		}
		return 0;
	}

	int dim = 1;

	// det(S) = 0.  If any of the cofactors of the plane displacements are zero, we have a degenerate subspace to test.
	for( int i = 0; i < D+1; ++i )
	{
		if( abs( cofSRowD(i) ) < gsa_eps<F>() )
		{
			const Mat<D,D,F> reducedS = reduce_simplex<D,F>( i, S );
			F detReducedS = 0;
			(void)detReducedS;
			Row<D,F> cofReducedSRowD;
			int sub_dim = 1;
			if( D > 1 && (sub_dim = simplex_condition<D-1,F>( detReducedS, cofReducedSRowD, reducedS )) == 0 )
			{
				return 0;
			}
			if( sub_dim > dim )
			{
				dim = sub_dim;
			}
		}
		else
		{
			dim = D;
		}
	}

	return dim;
}


struct GSA_State
{
	enum Enum
	{
		Error_Flag = 0x80000000,
		Error_Mask = ~Error_Flag,

		Invalid = Error_Flag,
		Maximum_GSA_Iterations_Reached,
		Maximum_TOI_Iterations_Reached,
	};
};


template<int D, typename F>
class GSA
{
public:
	struct LineIntersect
	{
		Plane<D,F>	m_plane;
		F			m_s;
	};

	class ConvexShape
	{
	public:
		virtual	unsigned	initialize_tangent_planes( Plane<D,F>* planes, unsigned plane_count ) const = 0;
		virtual	void		intersect_line( LineIntersect& in, LineIntersect& ex, const Pos<D,F>& orig, const Dir<D,F>& dir, F time ) const = 0;
		virtual Dir<D,F>	get_linear_velocity() const = 0;

		virtual				~ConvexShape() {}
	};

			GSA() { init(); }

	int		state() const { return m_state; }

	void	init( int maxIterationCount = 1000 );
	bool	set_shapes( const ConvexShape* shape1, const ConvexShape* shape2 = NULL );

	bool	intersect( F time = 0 );
	bool	time_of_intersection( F& time, Dir<D,F>& normal, Pos<D,F> points[1<<(D-1)], F timeMin = -1, F timeMax = 1 );
	int		contacts( F& depth, Dir<D,F>& normal, Pos<D,F> points[1<<(D-1)] );

protected:
	void	advance_simplex( F time );
	void	solve_simplex();
	void	create_search_line();
	bool	intersect_search_line( F time );
	void	update_simplex();

	int					m_maxIterationCount;

	int					m_state;

	const ConvexShape*	m_shape[2];

	Pos<D,F>			m_searchLineOrig;
	Dir<D,F>			m_searchLineDir;

	// Current simplex (and derived values)
	Mat<D+1,D+1,F>		m_S;	// Simplex matrix - note, planes are stored in columns
	Row<D+1,F>			m_cofSRowD;
	F					m_detS;
	int					m_normalSpan;
	F					m_time;	// Time at which current simplex is captured
	Col<D+1,u8>			m_shapeIndices;	// The shape index for each plane (column) of m_S.
};


/* GSA methods */

template<int D, typename F>
void
GSA<D,F>::init( int maxIterationCount )
{
	m_maxIterationCount = maxIterationCount;
	m_state = GSA_State::Invalid;
	m_shape[0] = m_shape[1] = NULL;
	m_searchLineOrig.set( (F)0 );
	m_searchLineDir.set( (F)0 );
	m_searchLineDir(0) = (F)1;
	m_S = Mat<D+1,D+1,F>();
	m_cofSRowD.set( (F)0 );
	m_detS = (F)0;
	m_normalSpan = 0;
	m_time = (F)0;
	m_shapeIndices.set( 0 );
}

template<int D, typename F>
bool
GSA<D,F>::set_shapes( const ConvexShape* shape1, const ConvexShape* shape2 = NULL )
{
	if( shape1 == NULL )
	{
		init( m_maxIterationCount );
		return false;
	}

	m_state = 0;

	m_shape[0] = shape1;
	m_shape[1] = shape2;
	m_shapeIndices.set( 0 );

	Plane<D,F> planes[D+1];
	unsigned planeCount = 0;
	unsigned colN = 0;
	if( m_shape[1] != NULL )
	{
		planeCount += m_shape[1]->initialize_tangent_planes( planes, (D+1)/2 );
		for( ; colN < planeCount; ++colN )
		{
			m_S.setCol( colN, planes[colN] );
			m_shapeIndices(colN) = 1;
		}
	}
	planeCount += m_shape[0]->initialize_tangent_planes( planes+planeCount, D+1-planeCount );
	for( ; colN < planeCount; ++colN )
	{
		m_S.setCol( colN, planes[colN] );
		m_shapeIndices(colN) = 0;
	}
	for( ; colN < D+1; ++colN )
	{
		m_S.setCol( colN, m_S.getCol( 0 ) );
	}

	m_time = (F)0;

	return true;
}

template<int D, typename F>
bool
GSA<D,F>::intersect( F time )
{
	if( m_state == GSA_State::Invalid )
	{
		return false;
	}

	m_state = 0;

	// (0) Bring the cached simplex into the current time, and record current time
	advance_simplex( time );

	// (1) Initial Test for void simplex
	solve_simplex();
	if( m_normalSpan == 0 )
	{	// Void simplex
		return false;
	}

	// Iterate
	for(;;)
	{
		// (2) Create search line
		create_search_line();

		// (3) Search and test for intersection
		if( intersect_search_line( time ) )
		{	// Proved intersection
			return true;
		}
		if( m_state > m_maxIterationCount )
		{	// Iteration limit reached
			m_state = GSA_State::Maximum_GSA_Iterations_Reached;
			return false;
		}

		// (4) Test for disjointness
		solve_simplex();
		if( m_normalSpan == 0 )
		{	// Proved disjointness
			return false;
		}

		// (5) Update simplex
		update_simplex();
	}
}

template<int D, typename F>
bool
GSA<D,F>::time_of_intersection( F& time, Dir<D,F>& normal, Pos<D,F> points[1<<(D-1)], F timeMin, F timeMax )
{
	if( m_state == GSA_State::Invalid )
	{
		return false;
	}

	// Initialize time
	time = 0;

	// Determine if there is initial intersection
	bool intersecting = intersect( 0 );
	if( m_state < 0 )
	{
		return false;	// Error
	}
	int totalIterations = m_state;

	for( int temporalIterations = 0; temporalIterations < m_maxIterationCount; ++temporalIterations )
	{
		// Displacement velocity vector
		Row<D+1,F> displacementVelocity;
		for( int i = 0; i < D+1; ++i )
		{
			displacementVelocity(i) = m_shape[m_shapeIndices(i)]->get_linear_velocity()|m_S.getCol( i );
		}

		// Calculate -d(|S|)/dt
		const F minusDetSDot = displacementVelocity|m_cofSRowD;

		// Calculate delta time to intersection
		F dt;
		if( abs( minusDetSDot ) > gsa_eps<F>() )
		{
			dt = m_detS/minusDetSDot;	// DIVIDE
		}
		else
		{	// minusDetSDot = 0.  If m_detS = 0 as well, the time to intersection is indeterminant.  Use L'Hopital's rule.
			bool dtFound = false;
			if( abs( m_detS ) < gsa_eps<F>() )
			{
				// Find the row k and column l of the greatest magnitude cofactor C_kl of the normal components of S.
				// This is d(|S|)/d(S_kl), and will be the numerator of dt.
				int k = 0;
				int l = 0;
				F C_kl = (F)0;
				for( int i = 0; i < D; ++i )
				{
					for( int j = 0; j < D+1; ++j )
					{
						const F c = cof( m_S, i, j );
						if( abs( c ) >= abs( C_kl ) )
						{
							C_kl = c;
							k = i;
							l = j;
						}
					}
				}

				// The denomenator of dt is d(minusDetSDot)/d(S_kl) = displacementVelocity | d(m_cofSRowD)/d(S_kl).
				F d_minusDetSDot_d_S_kl = (F)0;
				for( int j = 0; j < D+1; ++j )
				{
					if( j != l )
					{
						const Mat<D,D,F> M = minor( m_S, D, j );		// Optimization opportunity: this minor has already been calculated when finding m_cofSRowD
						const F sign = (F)((int)1-(int)(((D+j)&1)<<1));	// ... and so has this sign
						d_minusDetSDot_d_S_kl += displacementVelocity(j)*sign*cof( M, k, (j>l)?l:(l-1) );
					}
				}

				if( abs( d_minusDetSDot_d_S_kl ) > gsa_eps<F>() )
				{
					dtFound = true;
					dt = C_kl/d_minusDetSDot_d_S_kl;
				}
			}

			if( !dtFound )
			{	// Still indeterminant or undefined.
				time = intersecting ? timeMin : timeMax;	// if already intersecting, no known negative TOI, otherwise there is no TOI
				return intersecting;
			}
		}

		if( !intersecting && dt <= gsa_eps<F>()*abs( time ) )
		{	// Widths are non-positive.  If the time derivatives of the widths are non-positive, there will never be an intersection.
			// d(w_i)/dt = minusDetSDot/minusDetSDot[i].  This has the same sign as minusDetSDot*minusDetSDot[i].
			for( int i = 0; i < D+1; ++i )
			{
				if( minusDetSDot*m_cofSRowD(i) <= 0 )
				{
					time = timeMax;
					return false;	// No TOI.
				}
			}
			return true;	// Intersection found at current TOI
		}

		if( intersecting && dt >= 0 )
		{
			time = timeMin;
			return true;	// Already intersecting, no known negative TOI
		}

		// Increment TOI
		time += dt;

		// Clamp time to given range
		if( time < timeMin )
		{
			time = timeMin;
			return false;
		}

		if( time > timeMax )
		{
			time = timeMax;
			return false;
		}

		// Ideally we have moved the shapes out of intersection.  If there is intersection, it should be barely touching within roundoff error, meaning we have found the TOI.
		intersecting = intersect( time );
		if( m_state < 0 )
		{
			return false;	// Error
		}
		totalIterations += m_state;
		m_state = totalIterations;
		if( intersecting )
		{
			return true;	// Found TOI
		}
		if( totalIterations > m_maxIterationCount )
		{
			// GSA iteration limit reached
			m_state = GSA_State::Maximum_GSA_Iterations_Reached;
			return false;
		}
	}

	// Temporal iteration limit reached
	m_state = GSA_State::Maximum_TOI_Iterations_Reached;
	return false;
}

template<int D, typename F>
int
GSA<D,F>::contacts( F& depth, Dir<D,F>& normal, Pos<D,F> points[1<<(D-1)] )
{
	if( (m_state&GSA_State::Error_Flag) != 0 )
	{
		return 0;
	}

	int pointCount = 0;

	switch( D )
	{
	case 1: // 1D - Trivial case
		depth = -m_detS/m_cofSRowD(0);	// = m_recipWidths[ m_simplex[0] ];
		normal = Plane<D,F>(m_S.getCol(0)).n();
		points[pointCount++] = Plane<D,F>(m_S.getCol(1)).project();
		break;
	case 2: // 2D
		{
			u8 planeSort[D+1];
			u32 partitions[2] = {0,D};
			for( u32 i = 0; i < D+1; ++i )
			{
				const i32 shapeIndex = (i32)m_shapeIndices(i);
				u32& partition = partitions[shapeIndex];
				planeSort[partition] = i;
				partition += 1-(shapeIndex<<1);
			}
			u8 normalCol;
			switch( partitions[0] )
			{
			case 1:	normalCol = planeSort[0];	break;
			case 2:	normalCol = planeSort[2];	break;
			default:
				return 0;
			}
			if( abs( m_cofSRowD(normalCol) ) < eps_real<F>() )
			{
				return 0;
			}
			const F recipW = -(F)1/m_cofSRowD(normalCol);
			depth = recipW*m_detS;	// = m_recipWidths[ m_simplex[0] ];
			normal = m_S.getCol( normalCol );
			points[pointCount++] << -cof( m_S, 0, normalCol )*recipW, -cof( m_S, 1, normalCol )*recipW, (F)1;
		}
		break;
	case 3: // 3D
		break;
	default: // D > 3 not handled
		break;
	}

	return pointCount;
}

template<int D, typename F>
void
GSA<D,F>::advance_simplex( F time )
{
	const F dt = time - m_time;
	for( int i = 0; i < D+1; ++i )
	{
		Plane<D,F> plane = m_S.getCol( i );
		plane(D) -= dt*(plane.n()|m_shape[m_shapeIndices(i)]->get_linear_velocity());
		m_S.setCol( i, plane );
	}
	m_time = time;
}

template<int D, typename F>
void
GSA<D,F>::solve_simplex()
{
	m_normalSpan = simplex_condition<D,F>( m_detS, m_cofSRowD, m_S );
}

template<int D, typename F>
void
GSA<D,F>::create_search_line()
{
	switch( D )
	{
	case 1: // 1D - Trivial case
		m_searchLineOrig.set( (F)0 );
		m_searchLineDir.set( (F)1 );
		break;
	case 2: // 2D - The search line is m_simplex[0]
		{
			const Plane<D,F> plane0 = Plane<D,F>( m_S.getCol( 0 ) );
			m_searchLineDir = plane0.n();
			swap( m_searchLineDir(0), m_searchLineDir(1) );
			m_searchLineDir(0) = -m_searchLineDir(0);
			m_searchLineOrig = plane0.project();
		}
		break;
	case 3: // 3D - The search line is the intersection of m_simplex[0] and m_simplex[1]
		switch( m_normalSpan )
		{
		case 1:
			{
				const Plane<D,F> plane0 = Plane<D,F>( m_S.getCol( 0 ) );
				m_searchLineOrig = plane0.project();
				m_searchLineDir = plane0.n();
			}
			break;
		case 2:
			{
				const Plane<D,F> plane0 = Plane<D,F>( m_S.getCol( 0 ) );
				Plane<D,F> planeResolve = Plane<D,F>( m_S.getCol( 1 ) );
				Dir<D,F> planeResolveDir = plane0.n()^planeResolve.n();
				F planeResolveDir2 = planeResolveDir.lengthSquared();
				for( int i = 2; i < D+1; ++i )
				{
					const Plane<D,F> planeI = Plane<D,F>( m_S.getCol( i ) );
					const Dir<D,F> dir = plane0.n()^planeI.n();
					const F dir2 = dir.lengthSquared();
					if( dir2 > planeResolveDir2 )
					{
						planeResolve = planeI;
						planeResolveDir = dir;
						planeResolveDir2 = dir2;
					}
				}
				if( planeResolveDir2 > square( gsa_eps<F>() ) )
				{
					m_searchLineOrig = Pos<D,F>() - (plane0.d()*(planeResolve.n()^planeResolveDir) + planeResolve.d()*(planeResolveDir^plane0.n()))/planeResolveDir2;	// DIVIDE
					m_searchLineDir = (plane0.n()^planeResolveDir) / sqrt( planeResolveDir2 );	// DIVIDE
				}
				else
				{
					m_searchLineOrig = plane0.project();
					m_searchLineDir = plane0.n();
				}
			}
			break;
		case 3:
			{
				const Plane<D,F> plane0 = Plane<D,F>( m_S.getCol( 0 ) );
				const Plane<D,F> plane1 = Plane<D,F>( m_S.getCol( 1 ) );
				Dir<D,F> dir = plane0.n()^plane1.n();
				const F dir2 = dir.lengthSquared();
				if( dir.lengthSquared() > square( gsa_eps<F>() ) )
				{
					m_searchLineOrig = Pos<D,F>() - (plane0.d()*(plane1.n()^dir) + plane1.d()*(dir^plane0.n()))/dir2;	// DIVIDE
					m_searchLineDir = dir / sqrt( dir2 );	// DIVIDE
				}
				else
				{	// Set line to an arbitrary line in one of the planes (using plane0)
					int minIndex = 0;
					for( int i = 1; i < D; ++i )
					{
						if( abs( plane0(i) ) < abs( plane0(minIndex) ) )
						{
							minIndex = i;
						}
					}
					Dir<D,F> x;
					x(minIndex) = (F)1;
					m_searchLineDir = plane0.n()^x;
					m_searchLineDir.normalize();
					m_searchLineOrig = plane0.project( m_searchLineOrig );
				}
			}
			break;
		}
		break;
	default: // D > 3 not handled
		break;
	}
}

template<int D, typename F>
bool
GSA<D,F>::intersect_search_line( F time )
{
	LineIntersect in, ex;
	u8 inShapeIndex = 0;
	u8 exShapeIndex = 0;

	for( int attemptN = 0; attemptN < 2; ++attemptN )
	{
		m_shape[0]->intersect_line( in, ex, m_searchLineOrig, m_searchLineDir, time );
		inShapeIndex = 0;
		exShapeIndex = 0;
		if( m_shape[1] != NULL )
		{
			LineIntersect in1, ex1;
			m_shape[1]->intersect_line( in1, ex1, m_searchLineOrig, m_searchLineDir, time );
			if( in1.m_s > in.m_s )
			{
				in = in1;
				inShapeIndex = 1;
			}
			if( ex1.m_s < ex.m_s )
			{
				ex = ex1;
				exShapeIndex = 1;
			}
		}

		// Update GSA iteration count
		++m_state;

		if( in.m_s - ex.m_s <= 10*gsa_eps<F>() )
		{	// Intersection
			return true;
		}

		if( attemptN == 1 )
		{
			break;
		}

		// Test for fixed point
		if( (Dir<D+1,F>(ex.m_plane - in.m_plane).lengthSquared() > square( gsa_eps<F>() ) && Dir<D+1,F>(ex.m_plane + in.m_plane).lengthSquared() > square( gsa_eps<F>() )) ||
			abs( m_searchLineDir|ex.m_plane ) > gsa_eps<F>() || abs( m_searchLineOrig|ex.m_plane ) > gsa_eps<F>() )
		{	// Not a fixed point
			break;
		}

		// Need to reset line
		m_searchLineOrig += m_searchLineDir*((F)0.5*(in.m_s + ex.m_s));
		m_searchLineDir = ex.m_plane.n();
	}

	// Put results in last two columns of the simplex matrix
	m_S.setCol( D-1, in.m_plane );
	m_shapeIndices(D-1) = inShapeIndex;
	m_S.setCol( D, ex.m_plane );
	m_shapeIndices(D) = exShapeIndex;

	return false;
}

template<int D, typename F>
void
GSA<D,F>::update_simplex()
{
	switch( D )
	{
	case 1: // 1D - Trivial case
		break;
	case 2: // 2D - Use line with normal that most opposes current line
		{
			const int col = (Plane<D,F>( m_S.getCol( 0 ) ).n()|m_S.getCol( 1 )) < (Plane<D,F>( m_S.getCol( 0 ) ).n()|m_S.getCol( 2 )) ? 1 : 2;
			m_S.setCol( 0, m_S.getCol( col ) );
			m_shapeIndices(0) = m_shapeIndices(col);
		}
		break;
	case 3: // 3D - Use 2D GSA in plane of m_simplex[0]
		{
			if( m_cofSRowD(1)*m_detS >= (F)0 )	// if( m_recipWidths[1] <= (F)0 )
			{	// 2D GSA void simplex (but not 3D void simplex).  Change m_simplex[0]
				m_S.setCol( 0, m_S.getCol( 1 ) );
				m_shapeIndices(0) = m_shapeIndices(1);
			}
			const int col = (Plane<D,F>( m_S.getCol( 1 ) ).n()|m_S.getCol( 2 )) < (Plane<D,F>( m_S.getCol( 1 ) ).n()|m_S.getCol( 3 )) ? 2 : 3;
			m_S.setCol( 1, m_S.getCol( col ) );
			m_shapeIndices(1) = m_shapeIndices(col);
		}
		break;
	default: // D > 3 not handled
		break;
	}
}


};	// namespace GSA
};	// namespace apex
};	// namespace physx

#endif

#endif // #ifndef APEX_GSA_H
