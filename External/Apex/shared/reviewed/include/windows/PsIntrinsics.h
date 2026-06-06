// This code contains NVIDIA Confidential Information and is disclosed 
// under the Mutual Non-Disclosure Agreement.
//
// Notice
// ALL NVIDIA DESIGN SPECIFICATIONS, CODE ARE PROVIDED "AS IS.". NVIDIA MAKES 
// NO WARRANTIES, EXPRESSED, IMPLIED, STATUTORY, OR OTHERWISE WITH RESPECT TO 
// THE MATERIALS, AND EXPRESSLY DISCLAIMS ALL IMPLIED WARRANTIES OF NONINFRINGEMENT, 
// MERCHANTABILITY, AND FITNESS FOR A PARTICULAR PURPOSE.
//
// Information and code furnished is believed to be accurate and reliable. 
// However, NVIDIA Corporation assumes no responsibility for the consequences of use of such 
// information or for any infringement of patents or other rights of third parties that may 
// result from its use. No license is granted by implication or otherwise under any patent 
// or patent rights of NVIDIA Corporation. Details are subject to change without notice. 
// This code supersedes and replaces all information previously supplied. 
// NVIDIA Corporation products are not authorized for use as critical 
// components in life support devices or systems without express written approval of 
// NVIDIA Corporation.
//
// Copyright (c) 2009-2011 NVIDIA Corporation. All rights reserved.
// Copyright (c) 2002-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2006 NovodeX. All rights reserved.   

#ifndef PX_FOUNDATION_PSINTRINSICS_H
#define PX_FOUNDATION_PSINTRINSICS_H

#include "Ps.h"
#include "PxAssert.h"

// this file is for internal intrinsics - that is, intrinsics that are used in
// cross platform code but do not appear in the API

#ifndef PX_WINDOWS
	#error "This file should only be included by Windows builds!!"
#endif

#include <math.h>
#include <intrin.h>
#include <float.h>

#pragma intrinsic(_BitScanForward)
#pragma intrinsic(_BitScanReverse)

namespace physx
{
namespace shdfnd2
{
	/*!
	Return the index of the highest set bit. Not valid for zero arg.
	*/
	inline PxU32 highestSetBit(PxU32 v)
	{
		PX_ASSERT(v!=0);
		unsigned long retval;
		_BitScanReverse(&retval, v);
		return retval;
	}

	/*!
	Return the index of the highest set bit. Not valid for zero arg.
	*/
	inline PxU32 lowestSetBit(PxU32 v)
	{
		PX_ASSERT(v!=0);
		unsigned long retval;
		_BitScanForward(&retval, v);
		return retval;
	}

	/*!
	Sets \c count bytes starting at \c dst to zero.
	*/
	PX_FORCE_INLINE void* memZero(void* PX_RESTRICT dest, PxU32 count)
	{
		return memset(dest, 0, count);
	}

	/*!
	Sets \c count bytes starting at \c dst to \c c.
	*/
	PX_FORCE_INLINE void* memSet(void* PX_RESTRICT dest, PxI32 c, PxU32 count)
	{
		return memset(dest, c, count);
	}

	/*!
	Copies \c count bytes from \c src to \c dst. User memMove if regions overlap.
	*/
	PX_FORCE_INLINE void* memCopy(void* PX_RESTRICT dest, const void* PX_RESTRICT src, PxU32 count)
	{
		return memcpy(dest, src, count);
	}

	/*!
	Copies \c count bytes from \c src to \c dst. Supports overlapping regions.
	*/
	PX_FORCE_INLINE void* memMove(void* PX_RESTRICT dest, const void* PX_RESTRICT src, PxU32 count)
	{
		return memmove(dest, src, count);
	}

	/*!
	Set 128B to zero starting at \c dst+offset. Must be aligned.
	*/
	PX_FORCE_INLINE void memZero128(void* PX_RESTRICT dest, PxU32 offset = 0)
	{
		PX_ASSERT(((size_t(dest)+offset) & 0x7f) == 0);
		memSet((char* PX_RESTRICT)dest+offset, 0, 128);
	}

	/*!
	Prefetch aligned 128B around \c ptr+offset.
	*/
	PX_FORCE_INLINE void prefetch128(const void*, PxU32 = 0)
	{
		// _prefetch((char* PX_RESTRICT)ptr+offset);
	}

	/*!
	Prefetch \c count bytes starting at \c ptr.
	*/
	PX_FORCE_INLINE void prefetch(void* ptr, PxU32 count = 0)
	{
		for(PxU32 i=0; i<=count; i+=128)
			prefetch128(ptr, i);
	}

	//! \brief platform-specific reciprocal
	PX_FORCE_INLINE float recipFast(float a)				{	return 1.0f/a;			}

	//! \brief platform-specific fast reciprocal square root
	PX_FORCE_INLINE float recipSqrtFast(float a)			{   return 1.0f/::sqrtf(a); }


} // namespace shdfnd2
} // namespace physx

#endif
