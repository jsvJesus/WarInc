#ifndef __NX_EXPLOSION_PREVIEW_H__
#define __NX_EXPLOSION_PREVIEW_H__
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
#include "NxApexAssetPreview.h"

namespace physx
{
namespace apex
{

PX_PUSH_PACK_DEFAULT

class NxApexRenderDebug;

namespace APEX_EXPLOSION
{
/**
	\def EXPLOSION_DRAW_NOTHING
	Draw no items.
*/
static const physx::PxU32 EXPLOSION_DRAW_NOTHING = (0x00);
/**
	\def EXPLOSION_DRAW_ICON
	Draw the icon.
*/
static const physx::PxU32 EXPLOSION_DRAW_ICON = (0x01);
/**
	\def EXPLOSION_DRAW_BOUNDARIES
	Draw the Explosion include shapes.
*/
static const physx::PxU32 EXPLOSION_DRAW_BOUNDARIES = (0x2);
/**
	\def EXPLOSION_DRAW_WITH_CYLINDERS
	Draw the explosion field boundaries.
*/
static const physx::PxU32 EXPLOSION_DRAW_WITH_CYLINDERS = (0x4);
/**
	\def EXPLOSION_DRAW_FULL_DETAIL
	Draw all of the preview rendering items.
*/
static const physx::PxU32 EXPLOSION_DRAW_FULL_DETAIL = (EXPLOSION_DRAW_ICON + EXPLOSION_DRAW_BOUNDARIES);
/**
	\def EXPLOSION_DRAW_FULL_DETAIL_BOLD
	Draw all of the preview rendering items using cylinders instead of lines to make the text and icon look BOLD.
*/
static const physx::PxU32 EXPLOSION_DRAW_FULL_DETAIL_BOLD = (EXPLOSION_DRAW_FULL_DETAIL + EXPLOSION_DRAW_WITH_CYLINDERS);
}

/**
\brief APEX asset preview wind asset.
*/
class NxExplosionPreview : public NxApexAssetPreview
{
public:
	/**
	Set the scale of the icon.
	The unscaled icon has is 1.0x1.0 game units.
	By default the scale of the icon is 1.0. (unscaled)
	*/
	virtual void	setIconScale(physx::PxF32 scale) = 0;
	/**
	Set the detail level of the preview rendering by selecting which features to enable.
	Any, all, or none of the following masks may be added together to select what should be drawn.
	The defines for the individual items are EXPLOSION_DRAW_NOTHING, EXPLOSION_DRAW_ICON, EXPLOSION_DRAW_BOUNDARIES,
	EXPLOSION_DRAW_WITH_CYLINDERS.
	All items may be drawn using the macro EXPLOSION_DRAW_FULL_DETAIL and EXPLOSION_DRAW_FULL_DETAIL_BOLD.
	*/
	virtual void	setDetailLevel(physx::PxU32 detail) = 0;

protected:
	NxExplosionPreview() {};
};


PX_POP_PACK

}
} // end namespace physx::apex

#endif // __NX_EXPLOSION_PREVIEW_H__
