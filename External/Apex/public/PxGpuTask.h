#ifndef __PX_GPU_TASK__
#define __PX_GPU_TASK__

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

/*!
\file
\brief GpuTask public API
*/

#include "PxTask.h"
#include <cuda.h>

namespace physx
{
namespace pxtask
{

/** \brief Provide GpuDispatcher a hint of task functionality
 *
 * For optimal performance, the dispatcher should run all available
 * HtoD tasks before running all Kernel tasks, and all Kernel tasks
 * before running any DtoH tasks.  This provides maximal kernel
 * overlap and the least number of CUDA flushes.
 */
enum GpuTaskHint
{
    HostToDevice,
    Kernel,
    DeviceToHost,

    NUM_GPU_TASK_HINTS
};

/** \brief User implemented Task for launch CUDA work */
class GpuTask : public Task
{
public:
    /**
     * \brief iterative "run" function for a GpuTask
     *
     * When a GpuTask is sent to the GpuDispatcher, the GpuDispatcher
     * calls this function multiple times, until the function (and all
     * other "ready" GpuTasks) return false.
     *
     * Each GpuTask should have a specific function; either copying data
     * to the device, running kernels on that data, or copying data from
     * the device.
     *
     * The GpuDispatcher acquires the CUDA context for the duration of
     * this function call, and it is highly recommended that the GpuTask
     * use the provided CUstream for all kernels.
     *
     * kernelIndex will be 0 for the initial call and incremented before
     * each subsequent call.  Once launchInstance() returns false, its
     * GpuTask is considered completed and is released.
     */
    virtual bool    launchInstance( CUstream stream, int kernelIndex ) = 0;

    /**
     * \brief Returns a hint indicating the function of this task
     */
    virtual enum GpuTaskHint getTaskHint() const = 0;
};

} // end pxtask namespace
} // end physx namespace

#endif
