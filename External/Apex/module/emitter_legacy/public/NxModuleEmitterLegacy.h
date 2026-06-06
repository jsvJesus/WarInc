#ifndef __NX_MODULE_EMITTER_LEGACY_H__
#define __NX_MODULE_EMITTER_LEGACY_H__

#include "NxApex.h"

namespace physx
{
namespace apex
{

PX_PUSH_PACK_DEFAULT

#if !defined(_USRDLL)
/* If this module is distributed as a static library, the user must call this
* function before calling NxApexSDK::createModule("Emitter")
*/
void instantiateModuleEmitterLegacy();
#endif


PX_POP_PACK

}
} // end namespace physx::apex

#endif
