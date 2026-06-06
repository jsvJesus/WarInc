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

#ifndef PX_FOUNDATION_PSUSERALLOCATED_H
#define PX_FOUNDATION_PSUSERALLOCATED_H

#include "PsAllocator.h"
#include "PsFoundation.h"

namespace physx
{
namespace shdfnd2
{
    /**
    Provides new and delete using a UserAllocator.
    Guarantees that 'delete x;' uses the UserAllocator too.
    */
    class UserAllocated
    {
    public:

// PX_SERIALIZATION
		PX_INLINE void* operator new(size_t, void* address)
		{
			return address;
		}
//~PX_SERIALIZATION

        template <typename Alloc>
        PX_INLINE void* operator new(size_t size, Alloc alloc, const char* fileName, int line)
        {
            return alloc.allocate(size, fileName, line);
        }
        template <typename Alloc>
        PX_INLINE void* operator new[](size_t size, Alloc alloc, const char* fileName, int line)
        {
            return alloc.allocate(size, fileName, line);
        }

        // placement delete
        template <typename Alloc>
        PX_INLINE void operator delete(void* ptr, Alloc alloc, const char* fileName, int line)
        {
			PX_FORCE_PARAMETER_REFERENCE(alloc);
			PX_FORCE_PARAMETER_REFERENCE(fileName);
			PX_FORCE_PARAMETER_REFERENCE(line);
            getAllocator().deallocate(ptr);
        }
        template <typename Alloc>
        PX_INLINE void operator delete[](void* ptr, Alloc alloc, const char* fileName, int line)
        {
			PX_FORCE_PARAMETER_REFERENCE(alloc);
			PX_FORCE_PARAMETER_REFERENCE(fileName);
			PX_FORCE_PARAMETER_REFERENCE(line);
            getAllocator().deallocate(ptr);
        }

        PX_INLINE void  operator delete(void* ptr)
        {
            getAllocator().deallocate(ptr);
        }
        PX_INLINE void  operator delete[](void* ptr)
        {
            getAllocator().deallocate(ptr);
        }
    };
} // namespace shdfnd2
} // namespace physx

#endif
