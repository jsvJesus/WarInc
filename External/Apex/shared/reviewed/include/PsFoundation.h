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

#ifndef PX_FOUNDATION_PSFOUNDATION_H
#define PX_FOUNDATION_PSFOUNDATION_H

#include "PxFoundation.h"
#include "PxErrors.h"
#include "PsMutex.h"
#include "PsPAEventSrc.h"

#include <stdarg.h>
#pragma warning( disable : 4512 ) //  assignment operator could not be generated
#pragma warning( disable : 4127 ) //  conditional expression is constant

#include "PsHashMap.h"

namespace physx
{
namespace shdfnd2
{
	class Factory;
	union TempAllocatorChunk;

	class Foundation : public PxFoundation
	{
		typedef HashMap<const NamedAllocator*, PxU32, 
			Hash<const NamedAllocator*>, Allocator> AllocNameMap;

		typedef Array<TempAllocatorChunk*, Allocator> AllocFreeTable;

										Foundation(PxUserOutputStream& ostr, PxUserAllocator& alloc);
										~Foundation();

	public:

		// factory
		static Foundation*				createInstance(PxU32 version, PxUserOutputStream& ostr, PxUserAllocator& alloc);
		static void						destroyInstance(Foundation*);

		// initialize the instance reference within a dll that doesn't own it
		static void						setInstance(Foundation*);
		PX_INLINE static Foundation&    getInstance() { PX_ASSERT(mInstance); return *mInstance; }

		virtual PxUserOutputStream&		getErrorStream() const;
		virtual void					setErrorLevel(PxErrorCode::Enum mask);
		virtual PxErrorCode::Enum		getErrorLevel() const;

		virtual physx::pubfnd2::PxUserAllocator&		getAllocator() const;
		physx::pubfnd2::PxUserAllocator&				getAllocatorFast() const { return mAllocator; }


		//! error reporting function
		void							error(PxErrorCode::Enum, const char* file, int line, const char* messageFmt, ...);
		void							errorImpl(PxErrorCode::Enum, const char* file, int line, const char* messageFmt, va_list );

// PX_SERIALIZATION
		Factory*						getFactory();
//~PX_SERIALIZATION

		PX_INLINE	AllocNameMap&		getNamedAllocMap()		{ return mNamedAllocMap;		}
		PX_INLINE	Mutex&				getNamedAllocMutex()	{ return mNamedAllocMutex;		}

		PX_INLINE	AllocFreeTable&		getTempAllocFreeTable()	{ return mTempAllocFreeTable;	}
		PX_INLINE	Mutex&				getTempAllocMutex()		{ return mTempAllocMutex;		}

		PX_INLINE   PAUtils&            getPAUtils()			{ return mPAUtils;				}

	private:

// PX_SERIALIZATION
		Factory*						mFactory;
//~PX_SERIALIZATION

		physx::pubfnd2::PxUserAllocator& mAllocator;

		PxUserOutputStream&				mErrorStream;
		PxErrorCode::Enum				mErrorMask;
		Mutex							mErrorMutex;

		AllocNameMap					mNamedAllocMap;
		Mutex							mNamedAllocMutex;

		AllocFreeTable					mTempAllocFreeTable;
		Mutex							mTempAllocMutex;

		PAUtils							mPAUtils;

		static Foundation*				mInstance;
	};


	PX_INLINE Foundation& getFoundation()
	{
		return Foundation::getInstance();
	}

} // namespace shdfnd2
} // namespace physx

//shortcut macros:
//usage: Foundation::error(PX_WARN, "static friction %f is is lower than dynamic friction %d", sfr, dfr);
#define PX_WARN ::physx::pubfnd2::PxErrorCode::eDEBUG_WARNING, __FILE__, __LINE__
#define PX_INFO	::physx::pubfnd2::PxErrorCode::eDEBUG_INFO, __FILE__, __LINE__

#ifdef _DEBUG
#define PX_WARN_ONCE(condition, string)	if (condition) { static bool warning = true; if (warning) { warning = false; Ps::getFoundation().error(PX_WARN, string); }}
#else
#define PX_WARN_ONCE(condition, string)
#endif

#endif

