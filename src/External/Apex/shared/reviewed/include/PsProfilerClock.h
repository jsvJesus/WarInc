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

#ifndef PX_FOUNDATION_PSPROFILERCLOCK_H
#define PX_FOUNDATION_PSPROFILERCLOCK_H

#include "Ps.h"

namespace physx
{
namespace shdfnd2
{
	class ProfilerClock
	{
	public:

		static void initClock();

		//!	This function starts recording the number of cycles elapsed.
		//!	\param		val		[out] address of a 32 bits value where the system should store the result.
		static void startClock(PxU64& val);

		//!	This function ends recording the number of cycles elapsed.
		//!	\param		val		[in,out] address to store the number of cycles elapsed since the last StartProfile.
		//								holds the time reported at the start.
		static void stopClock(PxU64& val);

		//convert to micro seconds
		static PxU64 convert(PxU64 interval);

		// platform-specific CPU counters
		static void startCounter(PxU32& val);
		static void stopCounter(PxU32& val);
	};

} // namespace shdfnd2
} // namespace physx

#endif
