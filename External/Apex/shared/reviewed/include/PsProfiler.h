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

#ifndef PX_FOUNDATION_PSPROFILER_H
#define PX_FOUNDATION_PSPROFILER_H

#include "PxProfiler.h"
#include "PsUserAllocated.h"
#include "PsArray.h"

#include "PsFoundation.h"
#include "PsThread.h"
#include "PsMutex.h"
#include "PsProfilerClock.h"

#if AG_PERFMON
#include "AgPerfMonEventAPI.h"
#include "AgPerfMonPhysX.h"
#endif

#define PX_FASTPROF

// In foundation so we can use it in the Core and PxPhysics.
namespace physx
{
namespace shdfnd2
{
	struct ProfileName
	{
		ProfileName() : name(0), callCount(0) {}

		PX_INLINE void reset() 
		{
			parent = 0;
			callCount = 0;
			time = 0;
			recursionLevel = 0;
			start = 0;
			order = 0;
			childTime = 0;
			counter = 0;
		}

		const char * name;
		PxU32 parent;		//first time parent's index.
		PxU32 callCount;
		PxU64 time;
		PxU64 childTime;	//children add their times here.
		PxU32 recursionLevel;
		PxU64 start;	//used as scratch for time the clock started.
		PxU32 order;	//order it was first called.
		PxU32 counter; //Additional performance counter(eg L2 cache misses)
	};

	class Profiler : public UserAllocated
	{

	public:
		Profiler();

		PxProfilerData * getProfileData(bool reset);

		PX_INLINE		void			startProfile(PxU32 name)
		{
			if (name < names.size()-1)//guard against sloppy usage
			{
				// Keep track of recursion level
#ifdef PX_FASTPROF
				ProfileName & z = fastNames[name];
#else
				ProfileName & z = names[name];
#endif
				if (z.name == NULL)	//don't spend time on inactive zones.
					return;

				if (z.callCount == 0)	//only 1st time if called from several places.
				{
					z.order = nRecords++;
					z.recursionLevel= recursionLevel;
#ifdef PX_FASTPROF
					z.parent = *fastStack;
#else
					z.parent = activeStack.back();
#endif
				}
				else
				{
#ifdef PX_FASTPROF
					if (z.parent != *fastStack)
#else
					if (z.parent != activeStack.back())
#endif
						z.parent = 0xffffffff;				//multiparent == no parent!
				}
				recursionLevel++;
#ifdef PX_FASTPROF
				*(++fastStack) = name;
#else
				activeStack.pushBack(name);
#endif
				clock.startCounter(z.counter);
				clock.startClock(z.start);
			}
		}

		PX_INLINE		void			endProfile(PxU32 name)
		{
			if (name < names.size()-1)//guard against sloppy usage
			{
#ifdef PX_FASTPROF
				ProfileName & z = fastNames[name];
#else
				ProfileName & z = names[name];
#endif
				if (z.name == NULL)	//don't spend time on inactive zones.
					return;

				clock.stopClock(z.start);
				clock.stopCounter(z.counter);

				--recursionLevel;
				z.callCount ++;
				z.time += z.start;	//save value, it will be overwritten by next startProfile().
#ifdef PX_FASTPROF
				fastStack--;
				fastNames[*fastStack].childTime += z.start;
#else
				activeStack.popBack();
				names[activeStack.back()].childTime += z.start;
#endif
			}
		}

		PX_INLINE		void		allocateNames(PxU32 numNames)
		{
			names.resize(numNames+1);	//need 1 more for scratch
			for(PxU32 i=0;i<names.size();i++)
			{
				ProfileName & z = names[i];
				z.reset();
				z.name = 0;
			}

#ifdef PX_FASTPROF
			fastNames = numNames ? names.begin() : 0;
#endif
			activeStack[0] = numNames;	//have the active stack point to it.
		}

		PX_INLINE		void		createName(const char * n, PxU32 slotNum)
		{
			if (slotNum < names.size())
			{
				ProfileName & pn = names[slotNum];
				pn.name = n;
				pn.reset();
			}
		}
		PX_INLINE		ProfileName*	getProfileName(PxU32 name)
		{
			if (name < names.size()-1)//guard against sloppy usage
			{
#ifdef PX_FASTPROF
				ProfileName & z = fastNames[name];
#else
				ProfileName & z = names[name];
#endif
				return &z;
			}
			else
				return NULL;
		}
	private:

		ProfilerClock clock;

		Array<ProfileName> names;
		Array<PxProfileZone> profData;
		Array<PxU32> activeStack;	//index of name currently active.  Only used to detect multi-parent situations that screw up the self-time computation.
		PxProfilerData rval;

#ifdef PX_FASTPROF
		ProfileName * fastNames;
		PxU32 * fastStack;			//points to top of stack
#endif

		PxU32 recursionLevel, nRecords;
	};

	/*******************/

	class ProfilerManager : public UserAllocated
	{
	public:

		ProfilerManager() 
			: 
#if AG_PERFMON
            eventIds(PX_DEBUG_EXP("profileEventIDs")),
#endif
			profList(PX_DEBUG_EXP("profileList")),
			names(PX_DEBUG_EXP("profileNames")),
			profData(PX_DEBUG_EXP("profileData"))

		{
			profilerTls=TlsAlloc();
		}
		~ProfilerManager()
		{
			for(PxU32 i=0;i<profList.size();i++)
				PX_DELETE(profList[i]);

			profList.clear();

			TlsFree(profilerTls);
		}

		//must be called before calling startProfiler() and createName()
		PX_INLINE void allocateNames(PxU32 nNames)
		{
			names.resize(nNames);
#if AG_PERFMON
			eventIds.resize(nNames);
#endif // AG_PERFMON
			for(PxU32 i=0;i<names.size();i++)
			{
				names[i]=NULL;
#if AG_PERFMON
				eventIds[i] = AG_INVALID_EVENT_ID;
#endif // AG_PERFMON
			}
		}
		//must be called before calling startProfiler()
		PX_INLINE void createName(const char * n, PxU32 slotNum)
		{
			names[slotNum]=n;
#if AG_PERFMON
			eventIds[slotNum] = gPerfUtils->registerEvent(n);
#endif // AG_PERFMON

		}

		PX_INLINE PxU32 getNumZones()
		{
			return names.size();
		}

		PX_INLINE void startProfile(PxU32 name)
		{
			getPerThreadProfiler()->startProfile(name);
		}
		PX_INLINE void endProfile(PxU32 name)
		{
			getPerThreadProfiler()->endProfile(name);
		}

		//Note: we assume all threads are suspended(ie not adding profile zones) when this is called.
		PxProfilerData * getProfileData(bool reset);

		Profiler *getPerThreadProfiler()
		{
			Profiler *prof=(Profiler *)TlsGet(profilerTls);
			if(prof!=NULL)
				return prof;
			else
				return addPerThreadProfiler();
		}

#if AG_PERFMON
		Array<AgEventID> eventIds;
		int threadId;
#endif // AG_PERFMON

	private:

		Profiler *addPerThreadProfiler();

		Mutex profilerMutex;
		//>>>>
		Array<Profiler *> profList;
		Array<const char *> names;

		//these store the merged profiler data from all threads.
		Array<PxProfileZone> profData;
		physx::PxProfilerData rval;
		//<<<<

		PxU32 profilerTls;
	};

	/*************************/
	class ProfileZone: private NoCopy
	{
	public:
		PX_INLINE ProfileZone(PxU32 nIndex, ProfilerManager & cp) : 
		  nameIndex(nIndex), currProfiler(cp), mThreadID(0)
		  {
			  currProfiler.startProfile(nIndex);
#if AG_PERFMON 
			  gPerfUtils->startEvent(currProfiler.eventIds[nIndex]);
#endif // AG_PERFMON
		  }
		  PX_INLINE ~ProfileZone()
		  {
			  currProfiler.endProfile(nameIndex);
#if AG_PERFMON
			  gPerfUtils->stopEvent(currProfiler.eventIds[nameIndex]);
#endif // AG_PERFMON
		  }
	private:
		PxU32 nameIndex;
		ProfilerManager & currProfiler;	//unfortunately we can't use a global as each scene gets its own profiler.
		PxU32           mThreadID;
	};

} // namespace shdfnd2
} // namespace physx

//wrap in macro so we can easily disable profiling and not get the strings stored:

#ifdef PX_ENABLE_PROFILER
#define PX_PROFILE_ZONE(name, cp) Ps::ProfileZone profileZone(name, cp);
#else
#define PX_PROFILE_ZONE(name, cp) 
#endif
//this one is always on, used sparingly for PPU LOD related timings
#define PX_PROFILE_ZONE_IMMUTABLE(name, cp) Ps::ProfileZone profileZone(name, cp);



#endif
