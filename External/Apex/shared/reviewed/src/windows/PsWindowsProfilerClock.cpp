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


#include "PsProfiler.h"
#include "PsProfilerClock.h"
#include "PsWindowsInclude.h"

namespace physx
{
namespace shdfnd2
{

namespace 
{
	PxU64 sTicksToMS = 1;
} // namespace

void ProfilerClock::initClock()
{
	LARGE_INTEGER li;
	QueryPerformanceFrequency(&li);
	sTicksToMS=li.QuadPart/(1000*1000);
}

void ProfilerClock::startClock(PxU64& val)
{
	LARGE_INTEGER li;

	QueryPerformanceCounter(&li);
	val=li.QuadPart;
}

void ProfilerClock::stopClock(PxU64& val)
{
	LARGE_INTEGER li;

	QueryPerformanceCounter(&li);
	val=li.QuadPart-val;
}

//convert to micro seconds
PxU64 ProfilerClock::convert(PxU64 interval)
{
	return interval/sTicksToMS;
}

void ProfilerClock::startCounter(PxU32& val)
{
	val = 0;
}

void ProfilerClock::stopCounter(PxU32& val)
{
	val = 0;
}

} // namespace shdfnd2
} // namespace physx
