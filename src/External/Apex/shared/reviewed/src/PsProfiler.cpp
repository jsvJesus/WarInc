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

#include "PsUserAllocated.h"
#include "PsProfiler.h"

namespace physx
{
namespace shdfnd2
{


Profiler::Profiler() : 
names(PX_DEBUG_EXP("profileNames")),
profData(PX_DEBUG_EXP("profileData")),
activeStack(PX_DEBUG_EXP("profileActiveStack"))
{
	recursionLevel = 0;
	nRecords = 0;
	activeStack.resize(100);			//maximum stack depth??
	names.resize(1);					//some uninitialized trash we don't care about
	activeStack[0] = names.size()-1;	//have the active stack point to it.
#ifdef PX_FASTPROF
	fastStack = activeStack.begin();
#endif
	clock.initClock();
}

PxProfilerData * Profiler::getProfileData(bool reset)
{	
	//reset
	if (reset)
	{
		// Get number of records -- this still contains duplicates if a certain one was called repeatedly.
		PxU32 nb_records = names.size()-1;
		// Check there's at least one
		if(nb_records)
		{

			// Reorganize the records (sort by insertion order) (currently by end order)
			//can save this by just inserting on creation + using returned index.
			ProfileName* data = &names[0];

			//compensate for zone zero subtracting its time from itself.
			//PX_ASSERT(data->childTime >= data->time);
			//data->childTime -= data->time;


			ProfileName** callOrder = (ProfileName**)PX_ALLOC_TEMP(nb_records*sizeof(ProfileName**));

			// Compute total number of elapsed cycles
			PxU64 total_nb_cycles = 0;
			PxU32 numUsedNames = 0;
			for(PxU32 i=0;i<nb_records;i++)
			{
				if (data[i].name)
				{

					//make an index list we're gonna sort.
					callOrder[numUsedNames] = &data[i];
					// ...and only keep root-level ones
					if(!data[i].recursionLevel)	
						total_nb_cycles += data[i].time;

					numUsedNames++;
				}
			}
			PxF64 total = (PxF64)total_nb_cycles;

			struct InOrder { static int Compare(const void* e0, const void* e1)	{ return (*(ProfileName **)e0)->order - (*(ProfileName **)e1)->order; }	};
			qsort(callOrder, numUsedNames, sizeof(ProfileName *), InOrder::Compare);

			profData.resize(numUsedNames);

			// Loop through records in call order:
			for(PxU32 i=0;i<numUsedNames;i++)
			{
				// Get current record
				//ProfileName* curDat = &data[i];
				ProfileName* curDat = callOrder[i];

				// Get current recursion level
				PxU32 curLevel = curDat->recursionLevel;

				PxProfileZone & outputZone = profData[i];		//copy to output buffer -- gotta do this for multithread / double buffer reason anyway.

				outputZone.callCount = curDat->callCount;
				outputZone.hierTime = (PxU32)clock.convert(curDat->time);
				outputZone.name = curDat->name;

				//PX_ASSERT(curDat->time >= curDat->childTime);
				// VangelisK: Modified to allow for child zones to have larger times than parent (e.g. spu profile zones)
				PxU64 diff = (curDat->time > curDat->childTime ? curDat->time - curDat->childTime : 0);
				outputZone.selfTime = (PxU32)clock.convert(diff);
				outputZone.recursionLevel = curDat->recursionLevel;


				// Special case for root-level...
				if(!curLevel)//root level
				{
					outputZone.percent = (PxReal)(total ? curDat->time * 100.0 / total : 0.0);
				}
				else if  (curDat->parent > 0xff000000)	//multiparent -- several parents call it.
				{
					outputZone.percent = 0.0f;			//we'd need two answers!
				}
				else
				{
					// Look for the current subtotal, from previous recursion level
					PxU64 ptime = data[curDat->parent].time;
					outputZone.percent = (PxReal)(ptime ? curDat->time * 100.0 / ptime : 0.0);
				}
#ifdef	PX_ENABLE_PROFILER_COUNTER
				outputZone.counter = curDat->counter;
#endif
			}

			PX_FREE_AND_RESET(callOrder);


			//clear timers for next run.
			for(PxU32 i=0;i<nb_records;i++)
			{
				data[i].reset();
			}
		}



		nRecords			= 0;
		recursionLevel		= 0;
		//			activeStack.clear();
		//			activeStack.pushBack(names.size()-1);	//can't have an empty stack --> have 1st stack elem 'parent of root' point to last unused name slot.
	}
	rval.numZones = profData.size();
	rval.profileZones = rval.numZones ?  &profData[0] : NULL;
	return &rval;	//return user's buffer
}
/*****************/

PxProfilerData* ProfilerManager::getProfileData(bool reset)
{
	Mutex::ScopedLock lock(profilerMutex);

	if(reset)
	{
		profData.clear();

		/*
		we insert zones from each thread one after another(maybe we should merge counts, to display 
		overall times rather than per thread times... hmmm.

		We display threads in the order they were encountered. Should we Sort by thread number?
		*/

		for(PxU32 i=0;i<profList.size();i++)
		{
			PxProfilerData* threadData=profList[i]->getProfileData(reset);

			for(PxU32 j=0;j<threadData->numZones;j++)
				profData.pushBack(threadData->profileZones[j]);
		}

	}

	rval.numZones = profData.size();
	rval.profileZones = rval.numZones ?  &profData[0] : NULL;

	return &rval;
}

Profiler* ProfilerManager::addPerThreadProfiler()
{
	Profiler* prof=PX_NEW(Profiler);

	{
		Mutex::ScopedLock lock(profilerMutex);

		prof->allocateNames(names.size());

		for(PxU32 i=0;i<names.size();i++)
		{
			if(names[i]==NULL)
				continue;

			prof->createName(names[i],i);
		}

		profList.pushBack(prof);
	}

	TlsSet(profilerTls,prof);

	return prof;
}

} // namespace shdfnd2
} // namespace physx
