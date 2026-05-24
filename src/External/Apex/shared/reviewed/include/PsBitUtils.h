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

#ifndef PX_FOUNDATION_PSBITUTILS_H
#define PX_FOUNDATION_PSBITUTILS_H

#include "PxIntrinsics.h"
#include "PxAssert.h"
#include "Ps.h"

namespace physx
{
namespace shdfnd2
{
	PX_INLINE PxU32 bitCount(PxU32 v)
	{
		// from http://graphics.stanford.edu/~seander/bithacks.html#CountBitsSetParallel
		PxU32 const w = v - ((v >> 1) & 0x55555555);
		PxU32 const x = (w & 0x33333333) + ((w >> 2) & 0x33333333);
		return ((x + (x >> 4) & 0xF0F0F0F) * 0x1010101) >> 24;
	}
	
	PX_INLINE bool isPowerOfTwo(PxU32 x)
	{
		return x!=0 && (x & x-1) == 0;
	}

	// "Next Largest Power of 2
	// Given a binary integer value x, the next largest power of 2 can be computed by a SWAR algorithm
	// that recursively "folds" the upper bits into the lower bits. This process yields a bit vector with
	// the same most significant 1 as x, but all 1's below it. Adding 1 to that value yields the next
	// largest power of 2. For a 32-bit value:"
	PX_INLINE PxU32 nextPowerOfTwo(PxU32 x)
	{
		x |= (x >> 1);
		x |= (x >> 2);
		x |= (x >> 4);
		x |= (x >> 8);
		x |= (x >> 16);
		return x+1;
	}

	// Helper function to approximate log2 of an integer value 
	// assumes that the input is actually power of two.
	// todo: replace 2 usages with 'highestSetBit'
	PX_INLINE PxU32 ilog2(PxU32 num)
	{
		for (PxU32 i=0; i<32; i++)
		{
			num >>= 1;
			if (num == 0) return i;
		}

		PX_ASSERT(0);
		return (PxU32)-1;
	}

} // namespace shdfnd2
} // namespace physx

#endif
