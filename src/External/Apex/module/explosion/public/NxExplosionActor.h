#ifndef __NX_EXPLOSION_ACTOR_H__
#define __NX_EXPLOSION_ACTOR_H__
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
#include "NxApex.h"
#include "NxForceFieldShared.h"


namespace physx
{
namespace apex
{


PX_PUSH_PACK_DEFAULT

class NxExplosionAsset;
class NxFieldBoundaryActor;
class ExplosionAssetParam;

class NxExplosionActor : public NxApexActor, public NxApexActorSource
{
protected:
	virtual ~NxExplosionActor() {}

public:
	/**
	Return true if the explosion actor is enabled.
	*/
	virtual bool					isEnable() = 0;

	/**
	Enable/disable explosion actor. Default status is enable. Can switch it freely.
	A disabled explosion actor still exists there, but has no effect to the scene.
	*/
	virtual bool					disable() = 0;
	virtual bool					enable() = 0;

	/**
	Returns the NxActor the Explosion has been attached to.
	Returns NULL if the Explosion is static.
	*/
	virtual NxActor*				getAttachedNxActor() const = 0;

	/**
	Attach Explosion actor to a NxActor
	*/
	virtual void					setAttachedNxActor(NxActor* actor) = 0;

	/**
	Gets location and orientation of the Explosion.
	If the attached NxActor is NULL, this is global pose (relative to the world frame),
	otherwise it is local pose (relative to the actor frame of the NxActor attached).
	*/
	virtual physx::PxMat44			getPose() const = 0;

	/**
	Sets location and orientation of the Explosion.
	If the attached NxActor is NULL, this is global pose (relative to the world frame),
	otherwise it is for local pose (relative to the actor frame of the NxActor attached).
	*/
	virtual void					setPose(const physx::PxMat44& pose) = 0;

	/**
	Gets the explosion actor's scale
	*/
	virtual physx::PxF32					getScale() const = 0;

	/**
	Sets the explosion actor's scale
	*/
	virtual void					setScale(physx::PxF32 scale) = 0;

	/**
	Retrieves the name string for the explosion actor
	*/
	virtual const char*				getName() const = 0;

	/**
	Set a name string for the explosion actor that can be retrieved with getName()
	*/
	virtual void					setName(const char* name) = 0;

	/**
	Adds a field boundary actor to the explosion actor to define its volume of activity.
	*/
	virtual void					addFieldBoundary(NxFieldBoundaryActor& bound) = 0;

	/**
	Removes a field boundary actor from the explosion actor.
	*/
	virtual void					removeFieldBoundary(NxFieldBoundaryActor& bound) = 0;

	/**
	Returns the asset the actor has been created from.
	*/
	virtual NxExplosionAsset* 	    getExplosionAsset() const = 0;


	/*******************************************************************************
	 The next 4 functions usually called under shockwave explosion mode, though they
	 also work for implosion & explosion cases.
	*/

	/**
	Returns the outer radius (far distance) the explosion has been confined.
	*/
	virtual physx::PxF32					getOuterBoundRadius() const = 0;

	/**
	Set the outer radius (far distance) the explosion will be confined.
	*/
	virtual void					setOuterBoundRadius(physx::PxF32) = 0;

	/**
	Returns the inner radius (near distance) the explosion has been confined.
	*/
	virtual physx::PxF32					getInnerBoundRadius() const = 0;

	/**
	Set the inner radius (near distance) the explosion will be confined.
	*/
	virtual void					setInnerBoundRadius(physx::PxF32) = 0;
};

PX_POP_PACK

}
} // end namespace physx::apex

#endif

