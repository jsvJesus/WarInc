#ifndef __NX_FIELDBOUNDARY_ASSET_H__
#define __NX_FIELDBOUNDARY_ASSET_H__
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

namespace physx
{
namespace apex
{

PX_PUSH_PACK_DEFAULT

#define NX_FIELD_BOUNDARY_AUTHORING_TYPE_NAME "FieldBoundaryAsset"

class NxFieldBoundaryActor;

/**
\brief An enum to define the type of the field boundary.
*/
enum NxApexFieldShapeType
{
	NX_APEX_SHAPE_SPHERE  = 0,
	NX_APEX_SHAPE_BOX     = 1,
	NX_APEX_SHAPE_CAPSULE = 2,
	NX_APEX_SHAPE_CONVEX  = 3
};

/**
\brief defines if the field boundary is an include or an exclude field boundary.
*/
enum NxApexFieldBoundaryFlag
{
	NX_APEX_FFB_INCLUDE          = 0,
	NX_APEX_FFB_EXCLUDE          = 1,
};

/**
\brief Descriptor used to create a spherical field boundary.
*/
struct NxApexSphereFieldShapeDesc
{
	PX_INLINE NxApexSphereFieldShapeDesc()
	{
		setToDefault();
	}
	PX_INLINE void setToDefault()
	{
		localPose = PxMat44::getIdentity();
		radius = 1.0f;
	}
	physx::PxMat44	localPose;
	physx::PxF32	radius;
};

/**
\brief Descriptor used to create a box field boundary.
*/
struct NxApexBoxFieldShapeDesc
{
	PX_INLINE NxApexBoxFieldShapeDesc()
	{
		setToDefault();
	}
	PX_INLINE void setToDefault()
	{
		localPose = PxMat44::getIdentity();
		dimensions.set(1.0f, 1.0f, 1.0f);
	}
	physx::PxMat44	localPose;
	physx::PxVec3	dimensions;
};

/**
\brief Descriptor used to create a capsule field boundary.
*/
struct NxApexCapsuleFieldShapeDesc
{
	PX_INLINE NxApexCapsuleFieldShapeDesc()
	{
		setToDefault();
	}
	PX_INLINE void setToDefault()
	{
		localPose = PxMat44::getIdentity();
		radius = 1.0f;
		height = 1.0f;
	}
	physx::PxMat44	localPose;
	physx::PxF32	radius;
	physx::PxF32	height;
};

/**
\brief Descriptor used to create a convex field boundary.
*/
struct NxApexConvexFieldShapeDesc
{
	/**
	\brief Default constructor for a convex field shape descriptor.
	*/
	PX_INLINE NxApexConvexFieldShapeDesc()
	{
		setToDefault();
	}
	/**
	\brief function to set a convex field shape descriptor to the default values.
	*/
	PX_INLINE void setToDefault()
	{
		localPose = PxMat44::getIdentity();
		flags				= 0;
		numVertices			= 0;
		pointStrideBytes	= 0;
		points				= NULL;
	}
	/**
	\brief The relative pose for this convex.
	*/
	physx::PxMat44	localPose;
	physx::PxU32	flags;
	physx::PxU32	numVertices;
	physx::PxU32	pointStrideBytes;
	const void*		points;
};

/**
\brief The field boundary asset class.
*/
class NxFieldBoundaryAsset : public NxApexAsset, public NxApexInterface
{
protected:
	/**
	\brief field boundary asset default constructor.
	*/
	virtual ~NxFieldBoundaryAsset() {}

public:
	/**
	\brief returns the default scale of the asset.
	*/
	virtual physx::PxVec3			getDefaultScale() const = 0;
	/**
	\brief returns the number of actors created from this asset
	\see NxFieldBoundaryAsset::getActor()
	*/
	virtual physx::PxU32			getActorCount() const = 0;
	/**
	\brief returns an actor created by this asset
	\see NxFieldBoundaryAsset::getActorCount()
	*/
	virtual NxFieldBoundaryActor*	getActor(physx::PxU32 index) const = 0;
};

PX_POP_PACK

}
} // namespace physx::apex

#endif
