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

#ifndef PX_FOUNDATION_PSSCOPEDMEMORY_H
#define PX_FOUNDATION_PSSCOPEDMEMORY_H

#include "PsTempAllocator.h"

namespace physx
{
namespace shdfnd2
{
	template<typename T, typename Alloc = TempAllocator>
	class ScopedPointer : private Alloc
	{
	public:

		~ScopedPointer()
		{
			if(mOwned)
				Alloc::deallocate(mPointer);
		}

		operator T*() const 
		{
			return mPointer;
		}

		T* mPointer;
		bool mOwned;
	};

} // namespace shdfnd2
} // namespace physx

/*! Stack allocation for \c count instances of \c type. Falling back to temp allocator if using more than 1kB. */
#define PX_ALLOCA(var, type, count)																		\
	physx::shdfnd2::ScopedPointer<type> var;															\
	{																									\
		PxU32 size=sizeof(type)*(count);																\
		var.mOwned = size > 1024;																		\
		if(var.mOwned)																					\
			var.mPointer = (type*)physx::shdfnd2::TempAllocator().allocate(size, __FILE__, __LINE__);	\
		else																							\
			var.mPointer = (type*)PxAlloca(size);														\
	}

#endif
