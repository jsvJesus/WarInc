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
#ifndef __NX_Wind_ACTOR_H__
#define __NX_Wind_ACTOR_H__

#include "NxApex.h"

namespace physx
{
namespace apex
{

PX_PUSH_PACK_DEFAULT

class NxWindAsset;

/**
\brief The APEX Wind Actor class.  This class creates and manages the force fields to simulate the wind
as specified by the NxWindAsset it was derived from.
*/
class NxWindActor : public NxApexActor, public NxApexActorSource
{
protected:
	virtual ~NxWindActor() {}

public:
	/**
	\brief Returns the asset the instance has been created from.
	*/
	virtual NxWindAsset* getWindAsset() const = 0;

	/**
	\brief Gets the global pose used when the actor is added to the scene, in the NxWindActorDesc
	*/
	virtual physx::PxMat44			getGlobalPose() const = 0;
	/**
	\brief Gets the Wind actor's half length dimensions.
	*/
	virtual physx::PxVec3			getHalfLengthDimensions() const = 0;

	/**
	\brief Gets the groups bit field, used for gust raycasts
	*/
	virtual physx::PxU32			getGustRaycastGroups() const = 0;

	/**
	\brief Set the position of the wind.
	*/
	virtual void			setWindPosition(physx::PxVec3 WindPosition) = 0;

	/**
	\brief Get the position of the wind.
	*/
	virtual physx::PxVec3			getWindPosition(void) = 0;

	/**
	\brief Set the Wind Heading in radians
	*/
	virtual void			setWindHeading(physx::PxF32 Heading) = 0;

	/**
	\brief Get the Wind Heading in radians
	*/
	virtual physx::PxF32			getWindheading(void) = 0;

	/**
	\brief Set the maximum number of gusts to create per frame.  (default is 5 gusts for each actor per frame)
	*/
	virtual void			setMaxGustsToCreatePerFrame(physx::PxU32 maxGustsPerFrame) = 0;

	/**
	\brief Set the time between gust raycasts in Seconds.
	*/
	virtual void			setTimeBetweenRaycastsGusts(physx::PxF32 raycastTime) = 0;

	/**
	\brief Set the LOD Radius used for activating and deactivating all wind force fields in this actor.
	*/
	virtual void			setLodRadius(physx::PxF32) = 0;
};

PX_POP_PACK

}
} // namespace physx::apex

#endif
