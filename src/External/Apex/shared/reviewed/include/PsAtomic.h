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

#ifndef PX_FOUNDATION_PSATOMIC_H
#define PX_FOUNDATION_PSATOMIC_H

#include "Ps.h"

namespace physx
{
namespace shdfnd2
{
	/* set *dest equal to val. Return the old value of *dest */
	PxI32	atomicExchange(volatile PxI32* dest, PxI32 val);

	/* if *dest == comp, replace with exch. Return original value of *dest */
	PxI32	atomicCompareExchange(volatile PxI32* dest, PxI32 exch, PxI32 comp);

	/* if *dest == comp, replace with exch. Return original value of *dest */
	void *	atomicCompareExchangePointer(volatile void** dest, void* exch, void* comp);

	/* increment the specified location. Return the incremented value */
	PxI32	atomicIncrement(volatile PxI32* val);

	/* increment the specified location. Return the decremented value */
	PxI32	atomicDecrement(volatile PxI32* val);

	/* add delta to *val. Return the new value */
	PxI32	atomicAdd(volatile PxI32* val, PxI32 delta);

	/* compute the maximum of dest and val. Return the new value */
	PxI32	atomicMax(volatile PxI32* val, PxI32 val2);

} // namespace shdfnd2
} // namespace physx

#endif
