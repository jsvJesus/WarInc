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
// Copyright 2008, NVIDIA Corporation. All rights reserved.

#ifndef __PSPA_GPU_EVENT_SRC___
#define __PSPA_GPU_EVENT_SRC___

/*!! Changing any parameter in this file requires a full rebuild of all CUDA code !!*/

///< Enables per-warp logging to external memory
#define ENABLE_CTA_PROFILING					1

///< Defines the size of the profiling buffers allocated on both the GPU
///< and host pinned memory.  A single buffer is used to collect all
///< of the profile data for an entire push-buffer.  A warning will be
///< emitted by the GpuDispatcher if this limit is reached.
#define NUM_CTAS_PER_PROFILE_BUFFER             (12*1024*128)

///< Total number of profile buffers that will be allocated by the Gpu
///< Dispatcher.  A good guess is the maximum number of addCompletionPrereq
///< calls you make in a single frame.  Having too few buffers is not harmful,
///< you will simply have unprofiled push-buffers (you will only see the
///< GpuTaskBatch bar).
#define NUM_CTA_PROFILE_BUFFERS					2

#include "Ps.h"
#include "cuda.h"

namespace physx
{
namespace shdfnd2
{

#if defined(__CUDACC__)
	#define CUDA_ALIGN_16 __align__(16)
#else
	#define CUDA_ALIGN_16
#endif

typedef struct CUDA_ALIGN_16 kernelEvent_sd
{
	PxU16 block;
	PxU8  warp;
	PxU8  mpId;
	PxU8  hwWarpId;
	PxU8  userDataCfg;
	PxU16 eventId;
	PxU32 startTime;
	PxU32 endTime;
} kernelEvent_st;

#if ENABLE_CTA_PROFILING

#if __CUDA_ARCH__ > 100  // atomicAdd needs SM11

// extMem is the void* returned by GpuDispatcher::getCurrentProfileBuffer()
// kernelEnum is the value returned by GpuDispatcher::registerKernelNames() plus your local index

#define KERNEL_START_EVENT(extMem, kernelEnum)							\
int4 *__evPtr = 0;														\
{																		\
	if (extMem && (threadIdx.x & 0x1F) == 0)							\
	{																	\
		physx::shdfnd2::kernelEvent_st ev;								\
		int smidVal;													\
		int hwWarpId;													\
		int smclock;													\
		int index = atomicAdd( extMem, 1 );								\
		asm("mov.u32" " %0, %clock;" : "=r" (smclock));					\
		asm("mov.u32" " %0, %smid;" : "=r" (smidVal));					\
		asm("mov.u32" " %0, %warpid;" : "=r" (hwWarpId));				\
		ev.block		= gridDim.x*blockIdx.y + blockIdx.x;			\
		ev.warp			= (blockDim.x*threadIdx.y + threadIdx.x) >> 5;	\
		ev.mpId			= smidVal;										\
		ev.eventId		= kernelEnum;									\
		ev.hwWarpId		= hwWarpId;										\
		ev.startTime	= smclock;										\
		ev.endTime		= smclock;										\
		ev.userDataCfg	= 0;											\
		if (index+1 < NUM_CTAS_PER_PROFILE_BUFFER){						\
			__evPtr = ((int4 *) extMem)+index+1;						\
			*__evPtr = *(int4 *) &ev;									\
		} else {														\
			extMem[1] = kernelEnum;										\
			extMem[2] = index;											\
			extMem[3] = smclock;										\
			__evPtr = 0;												\
		}																\
	} else __evPtr = 0;													\
}

#define KERNEL_STOP_EVENT(extMem, kernelEnum)							\
{																		\
	if (__evPtr)														\
	{																	\
		int regVal;														\
		asm("mov.u32" " %0, %clock;" : "=r" (regVal));					\
		__evPtr->w = regVal;											\
	}																	\
}

#else  // Not SM11 or greater

#define KERNEL_STOP_EVENT(extMem, kernelEnum)
#define KERNEL_START_EVENT(extMem, kernelEnum)

#endif

#else // !ENABLE_CTA_PROFILING

#define KERNEL_STOP_EVENT(extMem, kernelEnum)
#define KERNEL_START_EVENT(extMem, kernelEnum)

#endif

} // end namespace shdfnd2
} // end namespace physx

#endif /** __PSPA_GPU_EVENT_SRC___ */
