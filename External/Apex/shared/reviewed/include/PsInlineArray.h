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

#ifndef PX_FOUNDATION_PSINLINEARRAY_H
#define PX_FOUNDATION_PSINLINEARRAY_H

#include "PsArray.h"
#include "PsInlineAllocator.h"

namespace physx
{
namespace shdfnd2
{

	// array that pre-allocates for N elements
	template <typename T, PxU32 N, typename Alloc = typename AllocatorTraits<T>::Type>
	class InlineArray : public Array<T, InlineAllocator<N * sizeof(T), Alloc> >
	{
		typedef InlineAllocator<N * sizeof(T), Alloc> Allocator;
	public:
		// NX_SERIALIZATION

		InlineArray(PxRefResolver& v) : Array<T, Allocator>(v) {}

		PX_INLINE bool isInlined()	const
		{
			return Allocator::isBufferUsed();
		}

		void exportExtraData(PxSerialStream& stream)
		{
			if(!isInlined())
				Array<T, Allocator>::exportArray(stream, false);
		}

		char* importExtraData(char* address, PxU32& totalPadding)
		{
			if(isInlined())
				this->mData = reinterpret_cast<T*>(Array<T, Allocator>::getInlineBuffer());
			else
				address = Array<T, Allocator>::importArray(address);
			return address;
		}
		//~NX_SERIALIZATION

		PX_INLINE explicit InlineArray(const Alloc& alloc = Alloc()) 
			: Array<T, Allocator>(alloc)
		{
			this->mData = this->allocate(N);
			this->mCapacity = N; 
		}
	};

} // namespace shdfnd2
} // namespace physx

#endif
