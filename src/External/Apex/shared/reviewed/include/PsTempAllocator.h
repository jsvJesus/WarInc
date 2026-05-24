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

#ifndef PX_FOUNDATION_TEMP_ALLOCATOR
#define PX_FOUNDATION_TEMP_ALLOCATOR

#include "PsAllocator.h"

namespace physx
{
namespace shdfnd2
{
	union TempAllocatorChunk
	{
		TempAllocatorChunk() : mNext(0) {}
		TempAllocatorChunk* mNext; // while chunk is free
		PxU32 mIndex; // while chunk is allocated
	};

	class TempAllocator : public Allocator
	{
	public:
		TempAllocator(const char* = 0) {}
		void* allocate(size_t size, const char* file, int line);
		void deallocate(void* ptr);
	};

} // namespace shdfnd2
} // namespace physx

#endif
