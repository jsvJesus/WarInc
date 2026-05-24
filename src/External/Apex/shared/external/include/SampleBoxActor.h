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
#ifndef __SAMPLE_BOX_ACTOR_H__
#define __SAMPLE_BOX_ACTOR_H__

#include "SampleActor.h"

#include "NxActor.h"
#include "NxActorDesc.h"
#include "NxApexRenderDebug.h"
#include "NxBodyDesc.h"
#include "NxBoxShapeDesc.h"
#include "NxScene.h"
#include "NxFromPx.h"

#include <Renderer.h>
#include <RendererMeshContext.h>



class SampleBoxActor : public SampleActor
{
public:
	SampleBoxActor(physx::apex::NxApexRenderDebug *rdebug, NxScene &physxScene,
		const physx::PxVec3 &pos, const physx::PxVec3 &vel, const physx::PxVec3 &extents, physx::PxF32 density,
		bool useGroupsMask, bool showDebugName=true, const RendererColor boxColor=RendererColor(200,200,200,255)) :
	mBlockId(-1),
	mPhysxActor(NULL),
	mApexRenderDebug(rdebug),
	mRenderer(NULL),
	mRendererBoxShape(NULL)
	{
		createActor(physxScene, pos, vel, extents, density, useGroupsMask);

		if(mPhysxActor)
		{
			const physx::PxF32 skinWidth = mPhysxActor->getShapes()[0]->getSkinWidth();

			physx::PxVec3 bmin = -extents + physx::PxVec3(skinWidth, skinWidth, skinWidth);
			physx::PxVec3 bmax =  extents - physx::PxVec3(skinWidth, skinWidth, skinWidth);

			physx::apex::PxFromNxMat34(mTransform, mPhysxActor->getGlobalPose());

			mBlockId = rdebug->beginDrawGroup(mTransform);

			rdebug->addToCurrentState(physx::DebugRenderState::SolidShaded);
			rdebug->setCurrentColor(*(const physx::PxU32*)&boxColor);
			rdebug->debugBound(bmin,bmax);

			if ( showDebugName )
			{
				static physx::PxU32 bcount = 0;
				rdebug->setCurrentColor(0xFFFFFF);
				rdebug->setCurrentTextScale(0.5f);
				rdebug->addToCurrentState(physx::DebugRenderState::CenterText);
				rdebug->addToCurrentState(physx::DebugRenderState::CameraFacing);
				rdebug->debugText(physx::PxVec3(0,extents.y+0.01f,0),"Sample Box:%d",bcount++);
			}

			rdebug->endDrawGroup();
		}
	}

	SampleBoxActor(Renderer *renderer, SampleMaterialAsset &material,
		NxScene &physxScene, const physx::PxVec3 &pos, const physx::PxVec3 &vel, 
		const physx::PxVec3 &extents, physx::PxF32 density, bool useGroupsMask, physx::apex::NxApexRenderDebug *rdebug=0) :
	mBlockId(-1),
	mPhysxActor(NULL),
	mApexRenderDebug(rdebug),
	mRenderer(renderer),
	mRendererBoxShape(NULL)
	{
		createActor(physxScene, pos, vel, extents, density, useGroupsMask);

		mRendererBoxShape = new RendererBoxShape(*mRenderer, extents);

		mRendererMeshContext.material         = material.getMaterial();
		mRendererMeshContext.materialInstance = material.getMaterialInstance();
		mRendererMeshContext.mesh             = mRendererBoxShape->getMesh();
		mRendererMeshContext.transform        = &mTransform;


		if ( rdebug )
		{
			mBlockId = rdebug->beginDrawGroup(mTransform);
			rdebug->addToCurrentState(physx::DebugRenderState::SolidShaded);
			static physx::PxU32 bcount = 0;
			rdebug->setCurrentColor(0xFFFFFF);
			rdebug->setCurrentTextScale(0.5f);
			rdebug->addToCurrentState(physx::DebugRenderState::CenterText);
			rdebug->addToCurrentState(physx::DebugRenderState::CameraFacing);
			rdebug->debugText(physx::PxVec3(0,extents.y+0.01f,0),"Sample Box:%d",bcount++);
			rdebug->endDrawGroup();
		}


	}

	virtual ~SampleBoxActor(void)
	{
		if (mApexRenderDebug != NULL)
		{
			mApexRenderDebug->reset(mBlockId);
		}
		if (mRendererBoxShape != NULL)
		{
			delete mRendererBoxShape;
			mRendererBoxShape = NULL;
		}
		if(mPhysxActor)
		{
			mPhysxActor->getScene().releaseActor(*mPhysxActor);
		}
	}

	void setGroupsMask(const NxGroupsMask &mask)
	{
		const physx::PxU32    numShapes = mPhysxActor->getNbShapes();
		NxShape *const*shapes    = mPhysxActor->getShapes();
		for(physx::PxU32 i=0; i<numShapes; i++)
		{
			shapes[i]->setGroupsMask(mask);
		}
	}

	NxActor* getPhysXActor() { return mPhysxActor; }

private:
	virtual void tick(float dtime, bool /*rewriteBuffers*/)
	{
		if(!mPhysxActor->isSleeping())
		{
			physx::apex::PxFromNxMat34(mTransform, mPhysxActor->getGlobalPose());
			if (mApexRenderDebug != NULL)
			{
				mApexRenderDebug->setDrawGroupPose(mBlockId, mTransform);
			}
		}
	}

	virtual void render(bool /*rewriteBuffers*/)
	{
		if (mRenderer != NULL)
		{
			mRenderer->queueMeshForRender(mRendererMeshContext);
		}
	}

	void createActor(NxScene& physxScene, const physx::PxVec3& pos, const physx::PxVec3& vel, const physx::PxVec3& extents, physx::PxF32 density, bool useGroupsMask)
	{
		mTransform.id();
		mTransform.t = pos;


		NxBodyDesc     bodyDesc;
		NxBoxShapeDesc shapeDesc;
		NxActorDesc    actorDesc;

		physx::apex::NxFromPxVec3(bodyDesc.linearVelocity, vel);
		bodyDesc.flags          = NX_BF_VISUALIZATION;

		physx::apex::NxFromPxVec3(shapeDesc.dimensions, extents);
		shapeDesc.shapeFlags       = NX_SF_VISUALIZATION | NX_SF_CLOTH_TWOWAY;
		shapeDesc.materialIndex    = 0;

		if (useGroupsMask)
		{
			shapeDesc.groupsMask.bits0 = 1;
			shapeDesc.groupsMask.bits2 = ~0;
		}

		physx::apex::NxFromPxMat34(actorDesc.globalPose, mTransform);
		if(density>0)
		{
			actorDesc.body          = &bodyDesc;
			actorDesc.density       = density;
		}
		actorDesc.flags         = 0;
		actorDesc.shapes.push_back(&shapeDesc);

		mPhysxActor = physxScene.createActor(actorDesc);

		PX_ASSERT(mPhysxActor);
	}

private:
	physx::PxI32					 mBlockId;
	NxActor							*mPhysxActor;
	physx::apex::NxApexRenderDebug	*mApexRenderDebug;
	Renderer						*mRenderer;
	RendererBoxShape				*mRendererBoxShape;
	RendererMeshContext				mRendererMeshContext;
	physx::PxMat34Legacy			 mTransform;
};


#endif
