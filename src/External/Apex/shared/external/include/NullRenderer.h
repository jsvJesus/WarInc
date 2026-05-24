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
#ifndef __NULL_RENDERER_H_
#define __NULL_RENDERER_H_

#include "NxApex.h"

/* This class is intended for use by command line tools that require an
 * APEX SDK.  Apps which use this renderer should _NOT_ call
 * updateRenderResources() or dispatchRenderResources().  You _WILL_
 * crash.
 */

namespace physx {
namespace apex {

class NullRenderResourceManager : public NxUserRenderResourceManager
{
public:
	NxUserRenderVertexBuffer*	createVertexBuffer( const NxUserRenderVertexBufferDesc& ) { return NULL; }
	NxUserRenderIndexBuffer*	createIndexBuffer( const NxUserRenderIndexBufferDesc& ) { return NULL; }
	NxUserRenderBoneBuffer*		createBoneBuffer( const NxUserRenderBoneBufferDesc& ) { return NULL; }
	NxUserRenderInstanceBuffer*	createInstanceBuffer( const NxUserRenderInstanceBufferDesc& ) { return NULL; }
	NxUserRenderSpriteBuffer*   createSpriteBuffer( const NxUserRenderSpriteBufferDesc& ) { return NULL; }
	NxUserRenderResource*		createResource( const NxUserRenderResourceDesc& ) { return NULL; }
	void						releaseVertexBuffer( NxUserRenderVertexBuffer& ) {}
	void						releaseIndexBuffer( NxUserRenderIndexBuffer& ) {}
	void						releaseBoneBuffer( NxUserRenderBoneBuffer& ) {}
	void						releaseInstanceBuffer( NxUserRenderInstanceBuffer& ) {}
	void						releaseSpriteBuffer( NxUserRenderSpriteBuffer& ) {}
	void						releaseResource( NxUserRenderResource& ) {}
	physx::PxU32						getMaxBonesForMaterial(void *) {return 0;}
};

}} // end namespace physx::apex

#endif
