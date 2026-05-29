#ifndef	__R3DSYS_WIN_H
#define	__R3DSYS_WIN_H

#include "r3dTypedefs.h"
#include <math.h>
#include <xmmintrin.h>

#define extern_nspace(nspace, var)  namespace nspace { extern var; };

#define INVALID_INDEX			(-1)

//
// Miscellaneous helper functions
//
#define SAFE_DELETE(p)       { if(p) { delete (p);     (p)=NULL; } }
#define PURE_DELETE(p)       { delete (p); }
#define SAFE_DELETE_ARRAY(p) { if(p) { delete[] (p);   (p)=NULL; } }
#define SAFE_RELEASE(p)      { if(p) { (p)->Release(); (p)=NULL; } }

#define R3D_NOCLASSCOPY(xx)		\
	xx(const xx &);			\
	xx &operator= (const xx &);

//
// various templates
//
template<class T>
__forceinline  int R3D_SIGN(const T a) 	{ return (a < 0) ? -1 : (a > 0) ? 1 : 0; }

template<class T>
__forceinline void R3D_SWAP(T& one, T& two) { T temp; temp = one; one = two; two = temp; }

template<class T>
__forceinline T R3D_MAX(const T a, const T b) { return a > b ? a : b; }

template<class T>
__forceinline T R3D_MIN(const T a, const T b)	{ return a < b ? a : b; }

template<class T>
__forceinline T R3D_ABS(const T a) 		{ return a >= 0 ? a : -a; }

template<class T>
__forceinline T R3D_CLAMP(const T val, const T min, const T max) 
{
  return ((val < min) ? min : (val > max) ? max : val);
}
template <class T> 
__forceinline T R3D_LERP( T from, T to, float weight )
{
	return from + weight * ( to - from );
}
//
// linked list stuff
//
template <class T>
int LList_Create(T **base)
{
  *base = NULL;
  return 1;
}

template <class T>
int LList_Insert(T **base, T *what)
{
	T	*tmp;

  // check if it's already here..
  for(tmp=*base; tmp; tmp=tmp->pNext)
    if(tmp == what)
      return 0;

  what->pNext = *base;
  *base      = what;
  return 1;
};

template <class T>
int LList_InsertLast(T **base, T *what)
{
	T	*tmp;
  // traverse to last element and add there..
  for(tmp=*base; tmp && tmp->pNext; tmp=tmp->pNext)
    if(tmp == what)
      return 0;

  if(*base) tmp->pNext  = what;
  else      *base      = what;
  return 1;
};

template <class T>
int LList_Remove(T **base, T *what)
{
	T	*tmp;

  if(!*base)
    return 0;

  if(*base == what) {
    *base = what->pNext;
    return 1;
  }
  // scan thru list and see if that object can be removed
  for(tmp=*base; tmp && tmp->pNext; tmp=tmp->pNext) {
    if(tmp->pNext == what) {
      tmp->pNext = what->pNext;
      return 1;
    }
  }

  return 0;
}

template <class T>
int LList_Destroy(T **base)
{
	T	*tmp, *tmp2;
  for(tmp = *base; tmp; tmp=tmp2) {
    tmp2 = tmp->pNext;
    delete tmp;
  }
  *base = NULL;
  return 1;
}

//----------------------------------------------------------------------------
//	General math functions
//----------------------------------------------------------------------------
inline FLOAT r3dExp( FLOAT Value ) { return expf(Value); }
inline FLOAT r3dLoge( FLOAT Value ) {	return logf(Value); }
inline FLOAT r3dFmod( FLOAT Y, FLOAT X ) { return fmodf(Y,X); }
inline FLOAT r3dSin( FLOAT Value ) { return sinf(Value); }
inline FLOAT r3dAsin( FLOAT Value ) { return asinf(Value); }
inline FLOAT r3dCos( FLOAT Value ) { return cosf(Value); }
inline FLOAT r3dAcos( FLOAT Value ) { return acosf(Value); }
inline FLOAT r3dTan( FLOAT Value ) { return tanf(Value); }
inline FLOAT r3dAtan( FLOAT Value ) { return atanf(Value); }
inline FLOAT r3dAtan2( FLOAT Y, FLOAT X ) { return atan2f(Y,X); }
inline FLOAT r3dPow( FLOAT A, FLOAT B ) { return powf(A,B); }

inline int r3dFloatToInt_2(float _fvar)
{
	_fvar += (1l << 23l);
	return *((int*)&_fvar) & 0x7fffffl;
}

__forceinline int r3dFloatToInt(float _fvar)
{
#if defined(_M_IX86) || defined(_M_X64)
	return _mm_cvt_ss2si(_mm_set_ss(_fvar));
#else
	return (int)(_fvar >= 0.0f ? _fvar + 0.5f : _fvar - 0.5f);
#endif
}

inline INT r3dFloor(FLOAT F)
{
	return (INT)floorf(F);
}

inline FLOAT r3dInvSqrt(FLOAT F)
{
#if defined(_M_IX86) || defined(_M_X64)
	__m128 x = _mm_set_ss(F);
	__m128 y = _mm_rsqrt_ss(x);

	const __m128 half = _mm_set_ss(0.5f);
	const __m128 three = _mm_set_ss(3.0f);

	y = _mm_mul_ss(
		_mm_mul_ss(half, y),
		_mm_sub_ss(three, _mm_mul_ss(_mm_mul_ss(x, y), y))
	);

	FLOAT result;
	_mm_store_ss(&result, y);
	return result;
#else
	return 1.0f / sqrtf(F);
#endif
}

inline FLOAT r3dSqrt(FLOAT F)
{
	return sqrtf(F);
}

//----------------------------------------------------------------------------
//	Time functions.
//----------------------------------------------------------------------------


	// our random generator functions
	unsigned long	u_random(unsigned long seed);
	void	      	u_srand(unsigned long seed);
	void		u_thread_rand_init();
	void		u_thread_rand_close();
	float 		u_GetRandom();
	float 		u_GetRandom(float r1, float r2);
	// helper for random numbers initialization in thread
	struct r3dRandInitInTread {
		r3dRandInitInTread() {
			u_thread_rand_init();
			u_srand(GetTickCount());
		}
		~r3dRandInitInTread() {
			u_thread_rand_close();
		}
	};

//----------------------------------------------------------------------------
//	Memory functions.
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//	Callbacks
//----------------------------------------------------------------------------

extern void (*OnDblClick)();

#endif	//__R3DSYS_WIN_H
