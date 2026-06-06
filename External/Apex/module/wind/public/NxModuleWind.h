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
#ifndef __NX_MODULE_Wind_H__
#define __NX_MODULE_Wind_H__

#include "NxApex.h"

namespace physx
{
namespace apex
{

PX_PUSH_PACK_DEFAULT

/*! \mainpage APEX Wind Module Description
 * \image html WindIconNoBorder.JPG
 *
 * \section intro_sec Introduction
 * APEX Wind simulates the various components of wind using force fields.
 * These include the prevailing wind, gusts, and the drag and flutter force fields.
 * Drag and Flutter while not strictly part of wind normally allow objects such as
 * leaves and paper to behave in a manner consistent with light falling objects that flutter
 * as they fall.<br>
 *
 * \section compat_sec PhysX SDK compatibility
 * APEX Wind is compatible with PhysX SDK version 2.8.1 and up.<br>
 *
 * \section dependencies_sec APEX Module Dependencies
 * APEX Wind Assets can optionally specify one or more APEX Field Boundary (Force Field module) assets that are
 * attached as exclude groups in every force field created by APEX wind.  If a Field Boundary Asset is specified
 * in an APEX Wind Asset the Force Field Module (which creates the Field Boundary actors) is force loaded.
 * Field Boundary actors that are created at module load time as well as those that are dynimacally created are
 * added to each force field when they are created and are removed when they are released.<br>
 *
 * \section physX_dependencies_sec PhysX Dependencies
 * For an APEX Wind force field to interact with a PhysX actor such as various types of particles, rigid bodies and
 * cloth the actors and the force field have to be in the same PhysX collision group.  APEX modules set their
 * collision group symbolically.  The actual collision group used is looked up in the Named Resource Provider (NRP).
 * For APEX Wind and PhysX particles to interact in the desired manner manner (I.E. look "real") many particle
 * parameters must be set proportionally to the APEX Wind forces such as the particle mass, the coefficient of
 * static friction (stiction) and particle parameters such as the Motion Limit Multiplier.  If these parameters
 * are not balanced the wind could be blowing but nothing could move or at the other extreme the wind could blow
 * and every actor is blown far away in a single frame.  Due to the time required to tune these parameters it is
 * recommended that assets be derived from working samples and then tuned to achieve the desired behavior.<br>
 *
 */

/**
\brief The APEX Wind Module class.  This class provides the ability to create NxWindAsset instances.

This class provides the ability to create NxWindAsset instances.
*/
class NxModuleWind : public NxModule
{
public:
	/**
	\brief The module ID value of the wind.
	*/
	virtual physx::PxU32					getModuleValue() const = 0;

protected:
	virtual ~NxModuleWind() {}
};

#if !defined(_USRDLL)
/* If this module is distributed as a static library, the user must call this
 * function before calling NxApexSDK::createModule("Wind")
 */
void instantiateModuleWind();
#endif

PX_POP_PACK

}
} // namespace physx::apex

#endif
