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

#ifndef PX_FOUNDATION_PSINLINEALLOCATOR_H
#define PX_FOUNDATION_PSINLINEALLOCATOR_H

#include "PsUserAllocated.h"

namespace physx
{
namespace shdfnd2
{
	// this is used by the array class to allocate some space for a small number
	// of objects along with the metadata
	template<PxU32 N, typename BaseAllocator>
	class InlineAllocator : private BaseAllocator
	{
	public:
		// PX_SERIALIZATION
		InlineAllocator(PxRefResolver& v) : BaseAllocator(v)	{}
		//~PX_SERIALIZATION

		InlineAllocator(const BaseAllocator& alloc = BaseAllocator())
			: BaseAllocator(alloc), mBufferUsed(false)
		{}

		void* allocate(PxU32 size, const char* filename, int line)
		{
			if(!mBufferUsed && size<=N) 
			{
				mBufferUsed = true;
				return mBuffer;
			}
			return BaseAllocator::allocate(size, filename, line);
		}

		void deallocate(void* ptr)
		{
			if(ptr == mBuffer)
				mBufferUsed = false;
			else
				BaseAllocator::deallocate(ptr);
		}

// PX_SERIALIZATION
		PX_FORCE_INLINE	PxU8*	getInlineBuffer()		{ return mBuffer;	  }
		PX_FORCE_INLINE	bool	isBufferUsed()	const	{ return mBufferUsed; }
//~PX_SERIALIZATION

	private:
		PxU8 mBuffer[N];
		bool mBufferUsed;
	};
} // namespace shdfnd2
} // namespace physx

#endif
