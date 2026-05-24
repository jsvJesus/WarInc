#pragma once

#include "PxPhysicsAPI.h"

using namespace physx;

#if WARINC_PHYSX_34

namespace physx
{
    typedef PxHitFlag PxSceneQueryFlag;
    typedef PxHitFlags PxSceneQueryFlags;
    typedef PxQueryFilterData PxSceneQueryFilterData;
}

#endif