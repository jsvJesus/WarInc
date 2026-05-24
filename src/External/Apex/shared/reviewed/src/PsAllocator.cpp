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

#include "PsAllocator.h"
#include "PsFoundation.h"
#include "PsMutex.h"

namespace physx
{
namespace shdfnd2
{

	namespace {
		typedef HashMap<const NamedAllocator*, PxU32, 
			Hash<const NamedAllocator*>, Allocator> AllocNameMap;

		PX_INLINE AllocNameMap& getMap() { return getFoundation().getNamedAllocMap(); }
		PX_INLINE Mutex& getMutex() { return getFoundation().getNamedAllocMutex(); }
	}

#if defined _DEBUG // NamedAllocator is slow and should only be used in debug mode

// PX_SERIALIZATION
NamedAllocator::NamedAllocator(PxRefResolver&)
{
	Mutex::ScopedLock lock(getMutex());
	getMap().insert(this, 0);
}
//~PX_SERIALIZATION

NamedAllocator::NamedAllocator(const char* name) 
{
	Mutex::ScopedLock lock(getMutex());
	getMap().insert(this, getAllocator().getHandle(name));
}

NamedAllocator::NamedAllocator(const NamedAllocator& other)
{
	Mutex::ScopedLock lock(getMutex());
	const AllocNameMap::Entry* e = getMap().find(&other); PX_ASSERT(e);
	PxU32 name = e->second;  // The copy is important because insert might invalidate the referenced hash entry
	getMap().insert(this, name);
}

NamedAllocator::~NamedAllocator() 
{
	Mutex::ScopedLock lock(getMutex());
	bool erased = getMap().erase(this);
	PX_FORCE_PARAMETER_REFERENCE(erased);
	PX_ASSERT(erased);
}

NamedAllocator& NamedAllocator::operator=(const NamedAllocator& other)
{
	Mutex::ScopedLock lock(getMutex());
	const AllocNameMap::Entry* e = getMap().find(&other); PX_ASSERT(e);
	getMap()[this] = e->second;
	return *this;
}

void* NamedAllocator::allocate(size_t size, const char* filename, int line)
{
	Mutex::ScopedLock lock(getMutex());
	const AllocNameMap::Entry* e = getMap().find(this); PX_ASSERT(e);
	return getAllocator().allocate(size, e->second, filename, line);
}

void NamedAllocator::deallocate(void* ptr)
{
	getAllocator().deallocate(ptr);
}

#endif // _DEBUG

void* Allocator::allocate(size_t size, const char* file, int line)
{
	return getAllocator().allocate(size, 0, file, line);
}
void Allocator::deallocate(void* ptr)
{
	getAllocator().deallocate(ptr);
}

} // namespace shdfnd2
} // namespace physx
