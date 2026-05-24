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

#ifndef PX_FOUNDATION_PSSYNC_H
#define PX_FOUNDATION_PSSYNC_H

#include "Ps.h"

namespace physx
{
namespace shdfnd2
{
	class Sync
	{
	public:

		static const PxU32 waitForever = 0xffffffff;

		Sync();

		~Sync();

		/** Wait on the object for at most the given number of ms. Returns 
		*  true if the object is signaled. Sync::waitForever will block forever 
		*  or until the object is signaled.
		*/

		bool wait(PxU32 milliseconds = waitForever);

		/** Signal the synchronization object, waking all threads waiting on it */

		void set();

		/** Reset the synchronization object */

		void reset();


	private:
		class SyncImpl *mImpl;
	};

} // namespace shdfnd2
} // namespace physx

#endif
