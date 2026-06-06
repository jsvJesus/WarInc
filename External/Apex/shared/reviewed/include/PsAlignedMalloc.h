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

#ifndef PX_FOUNDATION_PSALIGNEDMALLOC_H
#define PX_FOUNDATION_PSALIGNEDMALLOC_H

#include "PsUserAllocated.h"

/*!
Allocate aligned memory.
Alignment must be a power of 2!
-- should be templated by a base allocator
*/

namespace physx
{
namespace shdfnd2
{
	/**
	Allocator, which is used to access the global PxUserAllocator instance
	(used for dynamic data types template instantiation), which can align memory
	*/

	// SCS: AlignedMalloc with 3 params not found, seems not used on PC either
	// disabled for now to avoid GCC error

	template<PxU32 N, typename BaseAllocator = Allocator >
	class AlignedAllocator : public BaseAllocator
	{
	public:
		AlignedAllocator(const BaseAllocator& base = BaseAllocator()) 
		: BaseAllocator(base) {}

		void* allocate(size_t size, const char* file, int line)
		{
			size_t pad = N - 1 + sizeof(size_t); // store offset for delete.
			PxU8* base = (PxU8*)BaseAllocator::allocate(size+pad, file, line);

			PxU8* ptr = (PxU8*)(size_t(base + pad) & ~(size_t(N) - 1)); // aligned pointer, ensuring N is a size_t wide mask
			((size_t*)ptr)[-1] = ptr - base; // store offset

			return ptr;
		}
		void deallocate(void* ptr)
		{
			if(ptr == NULL)
				return;

			PxU8* base = ((PxU8*)ptr) - ((size_t*)ptr)[-1];
			BaseAllocator::deallocate(base);
		}
	};

} // namespace shdfnd2
} // namespace physx

#endif
