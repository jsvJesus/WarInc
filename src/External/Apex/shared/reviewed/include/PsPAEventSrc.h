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

#ifndef __PSPA_EVENT_SRC_API___
#define __PSPA_EVENT_SRC_API___

#include "Ps.h"

namespace physx
{
namespace shdfnd2
{

typedef PxU16 EventID;

static const size_t  EVENT_NAME_MAX_LEN = 128;
static const EventID INVALID_EVENT_ID = (EventID) 0xffff;

struct PsEventTypes
{
	enum Enum
	{
		eSTART,
		eSTOP,
		eSTAT,
		eDEBUG,
	};
};

// Manages the connection to an event collector, will be a singleton in the foundation
class PAUtils
{
public:
	PAUtils();
	~PAUtils();

	bool		isEnabled();
	bool		isEventEnabled( EventID id );

	EventID		registerEvent( const char * );
	void		startEvent( EventID id, PxU16 data );
	void		stopEvent( EventID id, PxU16 data );

	void		statEvent( EventID id, PxU32 stat );
	void		statEvent( EventID id, PxU32 stat, PxU32 ident );
	void		debugEvent( EventID id, PxU32 data0, PxU32 data1 );

    /* do not use these unless you know what you're doing */
    void        rawEvent( EventID id, PxU32 data0, PxU32 data1, PxU8 data2 );
    bool        rawEventWithTimestamp( EventID id, PxU64 timestamp, PxU32 data0, PxU32 data1, PxU8 data2 );
    bool        lock();
    bool        unlock();

private:
	class ConnImpl *mImpl;
};

} // end namespace shdfnd2
} // end namespace physx

#endif
