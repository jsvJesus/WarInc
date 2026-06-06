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

#ifndef PX_SLIST_H
#define PX_SLIST_H

#include "Px.h"
#include "PsAlignedMalloc.h"

#if defined(PX_X64)
	#define PX_SLIST_ALIGNMENT 16
#else
	#define PX_SLIST_ALIGNMENT 8
#endif

namespace physx
{
namespace shdfnd2
{
#ifndef PX_GNUC
	__declspec(align(PX_SLIST_ALIGNMENT))
#endif
	class SListEntry
	{
		friend struct SListImpl;
	public:

		SListEntry() : mNext(NULL) 
		{
			PX_ASSERT((size_t(this) & (PX_SLIST_ALIGNMENT-1)) == 0);
		}

		// Only use on elements returned by SList::flush()
		// because the operation is not atomic.
		SListEntry* next() { return mNext; }
	private:

		SListEntry *mNext;
	}
#ifdef PX_GNUC
	__attribute__ ((aligned(PX_SLIST_ALIGNMENT)));
#endif
;

	// template-less implementation
	struct SListImpl
	{
		SListImpl();
		~SListImpl();
		void push(SListEntry* entry);
		SListEntry* pop();
		SListEntry* flush();
		static const PxU32 size;
	}
	;

	template <typename Alloc = ReflectionAllocator<SListImpl> >
	class SListT : protected Alloc
	{
	public:

		SListT(const Alloc& alloc = Alloc())
			: Alloc(alloc)
		{
			mImpl = (SListImpl*)Alloc::allocate(SListImpl::size, __FILE__, __LINE__);
			PX_ASSERT((size_t(mImpl) & (PX_SLIST_ALIGNMENT-1)) == 0);
			PX_PLACEMENT_NEW(mImpl, SListImpl)();
		}
		~SListT()
		{
			mImpl->~SListImpl();
			Alloc::deallocate(mImpl);
		}

		// pushes a new element to the list
		void push( SListEntry& entry )
		{
			mImpl->push(&entry);
		}

		// pops an element from the list
		SListEntry* pop()
		{
			return mImpl->pop();
		}

		// removes all items from list, returns pointer to first element
		SListEntry* flush()
		{
			return mImpl->flush();
		}
	private:
		SListImpl *mImpl;
	};
	
	typedef SListT<> SList;


} // namespace shdfnd2
} // namespace physx

#endif
