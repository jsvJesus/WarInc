#pragma once

#ifndef WARINC_PHYSX_34
#define WARINC_PHYSX_34 1
#endif

#include "PxPhysicsAPI.h"

using namespace physx;

#if WARINC_PHYSX_34

namespace physx
{
    typedef PxHitFlag PxSceneQueryFlag;
    typedef PxHitFlags PxSceneQueryFlags;

    typedef PxQueryFlag PxSceneQueryFilterFlag;
    typedef PxQueryFlags PxSceneQueryFilterFlags;
    typedef PxQueryFilterData PxSceneQueryFilterData;
}

#endif