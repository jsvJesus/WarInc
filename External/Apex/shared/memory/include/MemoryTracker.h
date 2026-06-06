#ifndef MEMORYTRACKER_H
#define MEMORYTRACKER_H

/*
 * Copyright 2009-2011 NVIDIA Corporation.  All rights reserved.
 *
 * NOTICE TO USER:
 *
 * This source code is subject to NVIDIA ownership rights under U.S. and
 * international Copyright laws.  Users and possessors of this source code
 * are hereby granted a nonexclusive, royalty-free license to use this code
 * in individual and commercial software.
 *
 * NVIDIA MAKES NO REPRESENTATION ABOUT THE SUITABILITY OF THIS SOURCE
 * CODE FOR ANY PURPOSE.  IT IS PROVIDED "AS IS" WITHOUT EXPRESS OR
 * IMPLIED WARRANTY OF ANY KIND.  NVIDIA DISCLAIMS ALL WARRANTIES WITH
 * REGARD TO THIS SOURCE CODE, INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE.
 * IN NO EVENT SHALL NVIDIA BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL,
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS,  WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
 * OR OTHER TORTIOUS ACTION,  ARISING OUT OF OR IN CONNECTION WITH THE USE
 * OR PERFORMANCE OF THIS SOURCE CODE.
 *
 * U.S. Government End Users.   This source code is a "commercial item" as
 * that term is defined at  48 C.F.R. 2.101 (OCT 1995), consisting  of
 * "commercial computer  software"  and "commercial computer software
 * documentation" as such terms are  used in 48 C.F.R. 12.212 (SEPT 1995)
 * and is provided to the U.S. Government only as a commercial end item.
 * Consistent with 48 C.F.R.12.212 and 48 C.F.R. 227.7202-1 through
 * 227.7202-4 (JUNE 1995), all U.S. Government End Users acquire the
 * source code with only those rights set forth herein.
 *
 * Any use of this source code in individual and commercial software must
 * include, in the user documentation and internal comments to the code,
 * the above Disclaimer and U.S. Government End Users Notice.
 */
#include <stdio.h>

#ifndef USE_MEMORY_TRACKER
#define USE_MEMORY_TRACKER 1
#endif

#if USE_MEMORY_TRACKER

namespace physx
{
	namespace shdfnd2
	{

enum MemoryType
{
    MT_NEW,
    MT_NEW_ARRAY,
    MT_MALLOC,
    MT_FREE,
    MT_DELETE,
    MT_DELETE_ARRAY,
    MT_GLOBAL_NEW,
    MT_GLOBAL_NEW_ARRAY,
    MT_GLOBAL_DELETE,
    MT_GLOBAL_DELETE_ARRAY,
};

struct TrackInfo
{
	const void		*mMemory;
	MemoryType		 mType;
	unsigned int	 mSize;
	const char		*mContext;
	const char		*mClassName;
	const char		*mFileName;
	int				 mLineNo;
	unsigned int	 mAllocCount; // allocated at what time
};

class MemoryTracker
{
public:

    virtual void setLogLevel(bool logEveryAllocation,bool logEveyFrame,bool verifySingleThreaded=false) = 0;

    virtual void lock(void) = 0; // mutex lock.
    virtual void unlock(void) = 0; // mutex unlock

	virtual bool trackInfo(const void *mem,TrackInfo &info) = 0;

    // it will also fire asserts in a debug build.  The default is false for performance reasons.

    virtual void trackAlloc(void *mem,
            unsigned int size,
            MemoryType type,
            const char *context,
            const char *className,
            const char *fileName,
            int lineno) = 0;

    virtual void trackRealloc(void *oldMem,
            void *newMem,
            size_t newSize,
            MemoryType freeType,
            MemoryType allocType,
            const char *context,
            const char *className,
            const char *fileName,
            int lineno) = 0;

    virtual void trackFree(void *mem,
            MemoryType type,
            const char *context,
            const char *fileName,
            int lineno) = 0;

	virtual const char * trackValidateFree(void *mem,
		MemoryType type,
		const char *context,
		const char *fileName,
		int lineno) = 0;

    virtual void trackFrame(void) = 0;



    // detect memory leaks and, if any, write out a report to the filename specified.
    virtual bool detectMemoryLeaks(const char *fname,bool reportAllLeaks=true) = 0;
};


#define MEMORY_TRACKER_VERSION 7

MemoryTracker * createMemoryTracker(const char *dllLocation); // loads the DLL to track memory allocations.
extern MemoryTracker *gMemoryTracker;

#define TRACK_LOCK() { if ( physx::shdfnd2::gMemoryTracker ) physx::shdfnd2::gMemoryTracker->lock(); };
#define TRACK_UNLOCK() { if ( physx::shdfnd2::gMemoryTracker ) physx::shdfnd2::gMemoryTracker->unlock(); };

#define TRACK_ALLOC(mem,size,type,context,className,fileName,lineno) { if ( physx::shdfnd2::gMemoryTracker ) physx::shdfnd2::gMemoryTracker->trackAlloc(mem,size,type,context,className,fileName,lineno); }
#define TRACK_FREE(mem,type,context,fileName,lineno)            { if ( physx::shdfnd2::gMemoryTracker ) physx::shdfnd2::gMemoryTracker->trackFree(mem,type,context,fileName,lineno); }
#define TRACK_REALLOC(oldMem,newMem,newSize,freeType,allocType,context,className,fileName,lineno) { if ( physx::shdfnd2::gMemoryTracker ) physx::shdfnd2::gMemoryTracker->trackRealloc(oldMem,newMem,newSize,freeType,allocType,context,className,fileName,lineno); }
#define TRACK_FRAME() { if ( physx::shdfnd2::gMemoryTracker ) physx::shdfnd2::gMemoryTracker->trackFrame(); }
#define TRACK_INFO(x,y)  { if ( physx::shdfnd2::gMemoryTracker ) physx::shdfnd2::gMemoryTracker->trackInfo(x,y);  }

} // end namespace
};

#else

#define TRACK_LOCK()
#define TRACK_UNLOCK()

#define TRACK_ALLOC(mem,size,type,context,className,fileName,lineno)
#define TRACK_FREE(mem,type,context,fileName,lineno)
#define TRACK_REALLOC(oldMem,newMem,newSize,freeType,allocType,context,className,fileName,lineno)
#define TRACK_FRAME()
#define TRACK_INFO(x,y)

#endif


#endif
