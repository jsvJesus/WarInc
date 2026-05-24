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

#include "PsFoundation.h"

#include "PxUserOutputStream.h"
#include "PxQuat.h"
#include "PsThread.h"
#include "PsUtilities.h"
#include "PsProfiler.h"
#include "PsTempAllocator.h"
#include "PsPAEventSrc.h"

#include <stdio.h>

#pragma warning(disable:4996) // intentionally suppressing this warning message

#if defined(PX_WINDOWS) && !defined(PX_VC9)
#	define vsnprintf _vsnprintf
#endif

// PX_SERIALIZATION
#include "PsSerialFramework.h"

namespace physx
{
namespace shdfnd2
{

	class Factory : public UserAllocated
	{
	public:
		Factory();
		bool					registerClass(PxType type, CreateClass callback);
		PxSerializable*			createClass(PxType type, char*& address, PxRefResolver& v)	const;
	private:
		CreateClass				mCreators[NXTYPE_LAST];
	};

	Factory::Factory()
	{
		for(PxU32 i=0;i<NXTYPE_LAST;i++)
			mCreators[i] = NULL;
	}

	bool Factory::registerClass(PxType type, CreateClass callback)
	{
		PX_ASSERT(PxU32(type)<NXTYPE_LAST);
		PX_ASSERT(!mCreators[type]);
		if(mCreators[type])
			return false;
		mCreators[type] = callback;
		return true;
	}

	PxSerializable* Factory::createClass(PxType type, char*& address, PxRefResolver& v) const
	{
		PX_ASSERT(PxU32(type)<NXTYPE_LAST);
		PX_ASSERT(mCreators[type]);
		if(!mCreators[type])
			return NULL;
		return (mCreators[type])(address, v);
	}

	bool registerClass(PxType type, CreateClass callback)
	{
		Factory* Factory = Foundation::getInstance().getFactory();
		return Factory->registerClass(type, callback);
	}

	PxSerializable* createClass(PxType type, char*& address, PxRefResolver& v)
	{
		Factory* Factory = Foundation::getInstance().getFactory();
		return Factory->createClass(type, address, v);
	}

} // namespace shdfnd2
} // namespace physx

//~PX_SERIALIZATION

namespace physx
{
namespace shdfnd2
{

Foundation::Foundation(PxUserOutputStream& ostr, PxUserAllocator& alloc):
	// PX_SERIALIZATION
	mFactory(NULL),
	//~PX_SERIALIZATION
	mAllocator(alloc),
	mErrorStream(ostr),
	mErrorMask(PxErrorCode::Enum(~0))
{
}

Foundation::~Foundation()
{
	// PX_SERIALIZATION
	PX_DELETE_AND_RESET(mFactory);
	//~PX_SERIALIZATION

	// deallocate temp buffer allocations
	Allocator alloc;
	for(PxU32 i=0; i<mTempAllocFreeTable.size(); ++i)
	{
		for(TempAllocatorChunk* ptr = mTempAllocFreeTable[i]; ptr; )
		{
			TempAllocatorChunk* next = ptr->mNext;
			alloc.deallocate(ptr);
			ptr = next;
		}
	}
	mTempAllocFreeTable.reset();
}

// PX_SERIALIZATION
Factory* Foundation::getFactory()
{
	if(!mFactory)
		mFactory = PX_NEW(Factory);
	return mFactory;
}
//~PX_SERIALIZATION

PxUserAllocator& Foundation::getAllocator() const
{
		return mAllocator;
}

void Foundation::error(PxErrorCode::Enum c, const char* file, int line, const char* messageFmt, ...)
{
	va_list va;
	va_start(va, messageFmt);
	errorImpl(c, file, line, messageFmt, va);
	va_end(va);
}

void Foundation::errorImpl(PxErrorCode::Enum e, const char* file, int line, const char* messageFmt, va_list va)
{
	PX_ASSERT(messageFmt);
	if (e & mErrorMask)
	{
		//this function is reentrant but user's error callback may not be, so...
		Mutex::ScopedLock lock(mErrorMutex);	

		//create a string from the format strings and the optional params
		// We draw the line at a 1MB string.
		const int maxSize = 1000000;

		// If the string is less than 161 characters,
		// allocate it on the stack because this saves
		// the malloc/free time.
		const int stackBufSize = 161;
		int bufSize = stackBufSize;
		char stackBuffer[stackBufSize+1];
		char* stringBuffer = stackBuffer;
		char *heapBuffer = 0;

		while(0 > vsnprintf(stringBuffer, bufSize, messageFmt, va) && bufSize < maxSize)
		{
			PX_FREE(heapBuffer);
			bufSize *= 2;
			heapBuffer = PX_NEW(char)[bufSize+1];
		}

		mErrorStream.reportError(e, stringBuffer, file, line);

		PX_FREE(heapBuffer);
	}
}

Foundation* Foundation::createInstance(PxU32 version, PxUserOutputStream& ostr, PxUserAllocator& alloc)
{
	if (version != PX_PUBLIC_FOUNDATION_VERSION)
	{
		char* buffer = new char[256];
		sprintf(buffer, "Wrong version: foundation version is %d, tried to create %d", PX_PUBLIC_FOUNDATION_VERSION, version);
		ostr.reportError(PxErrorCode::eINVALID_PARAMETER, buffer, __FILE__, __LINE__);
		return 0;
	}

	if ( mInstance == NULL )
	{
		// if we don't assign this here, the Foundation object can't create member
		// subobjects which require the allocator

		mInstance = reinterpret_cast<Foundation*>( alloc.allocate( 
			sizeof(Foundation), alloc.getHandle("Foundation"), __FILE__,__LINE__));

		PX_PLACEMENT_NEW(mInstance, Foundation)(ostr, alloc);
	}

	return mInstance;
}

void Foundation::destroyInstance(Foundation* ptr)
{
	PX_ASSERT(ptr);
	PxUserAllocator& alloc = ptr->mAllocator;
	ptr->~Foundation();
	alloc.deallocate(ptr);
	mInstance = 0;
}

void Foundation::setInstance(Foundation* ptr) 
{ 
	mInstance = ptr;
}

PxUserOutputStream& Foundation::getErrorStream() const
{
	return mErrorStream;
}

void Foundation::setErrorLevel(PxErrorCode::Enum mask)
{
	mErrorMask = mask;
}

PxErrorCode::Enum Foundation::getErrorLevel() const
{
	return mErrorMask;
}

PxUserAllocator& getAllocator()
{ 
	return getFoundation().getAllocatorFast();
}

Foundation* Foundation::mInstance = NULL;

} // namespace shdfnd2
} // namespace physx
