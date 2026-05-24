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
#ifndef APEX_CSG_H
#define APEX_CSG_H


#ifdef PX_PS3
// ApexGSA.h causes thousands of compiler errors on PS3
#define USE_GSA	0
#else
#define USE_GSA 1
#endif


#include "PsShare.h"
#include "NxRenderMeshAsset.h"

#ifndef WITHOUT_APEX_AUTHORING

namespace physx
{
	namespace apex
	{


class UserRandom
{
public:
	virtual	physx::PxU32	getInt() = 0;
	virtual physx::PxF32	getReal( physx::PxF32 min, physx::PxF32 max ) = 0;
};


struct BSPBuildParameters
{
	/*
		Used for searching splitting planes.
		If NULL, a default random # generator will be used.
	 */
	UserRandom*			rnd;

	/*
		Larger values of testSetSize may find better BSP trees, but will take more time to create.
		testSetSize = 0 is treated as infinity (all surfaces will be tested for each branch).
	 */
	physx::PxU32		testSetSize;

	/*
		How much to weigh the relative number of triangle splits when searching for a BSP surface.
	 */
	physx::PxF32		splitWeight;

	/*
		How much to weigh the relative triangle imbalance when searching for a BSP surface.
	 */
	physx::PxF32		imbalanceWeight;

	/*
		A unitless value used to determine mesh triangle coplanarity.
		Negative values will result in the internal default linear tolerance being used.
	 */
	physx::PxF32		linearTolerance;

	/*
		A threshold angle (in radians) used to determine mesh triangle coplanarity.
		Negative values will result in the internal default angular tolerance being used.
	 */
	physx::PxF32		angularTolerance;

			BSPBuildParameters()	{ setToDefault(); }

	void	setToDefault()
			{
				rnd = NULL;
				testSetSize = 10;
				splitWeight = (physx::PxF32)0.5;
				imbalanceWeight = 0;
				linearTolerance = -1;
				angularTolerance = -1;
			}
};


struct Operation
{
	enum Enum
	{
		Empty_Set				= 0x0,	// constant
		All_Space				= 0x1,	// constant
		Set_A					= 0x2,	// unary
		Set_A_Complement		= 0x3,	// unary
		Set_B					= 0x4,	// unary
		Set_B_Complement		= 0x5,	// unary
		Exclusive_Or			= 0x6,
		Equivalent				= 0x7,
		Intersection			= 0x8,
		Intersection_Complement	= 0x9,
		A_Minus_B				= 0xA,
		A_Implies_B				= 0xB,
		B_Minus_A				= 0xC,
		B_Implies_A				= 0xD,
		Union					= 0xE,
		Union_Complement		= 0xF,

		NOP						= 0x80000000	// no op
	};
};


struct BSPVisualizationFlags
{
	enum Enum
	{
		OutsideRegions	= (1<<0),
		InsideRegions	= (1<<1),

		SingleRegion	= (1<<16)
	};
};


/*
	Memory cache for BSP construction.  Not global, so that concurrent calculations can use different pools.
 */
class IApexBSPMemCache
{
public:

	/*
		Deallocate all memory buffers.
	 */
	virtual void	clearAll() = 0;

	/*
		Deallocate only temporary data buffers.
	 */
	virtual void	clearTemp() = 0;

	/*
		Clean up.
	 */
	virtual	void	release() = 0;

protected:

					IApexBSPMemCache()	{}
	virtual			~IApexBSPMemCache()	{}
};


/*
	BSP interface.

	Convert a mesh into a BSP, perform boolean operations between BSPs, and extract the resulting mesh.
 */

class IApexBSP
{
public:

	/*
		Construct a BSP from the given mesh, using the given parameters.
	 */
	virtual bool			fromMesh( const physx::Array<NxExplicitRenderTriangle>& mesh, const BSPBuildParameters& params, IProgressListener* progressListener = NULL ) = 0;

	/*
		Build a combination of two BSPs (this and the passed-in bsp), upon which boolean operations of the two can be performed.
	 */
	virtual bool			combine( const IApexBSP& bsp ) = 0;

	/*
		Build a BSP resulting from a boolean operation upon a combination.
		Note: you may do this "in place," i.e.
			bsp.op( bsp, operation );
		... in this case, bsp will no longer be a combined BSP.
	 */
	virtual bool			op( const IApexBSP& combinedBSP, Operation::Enum operation ) = 0;

	/*
		This BSP is changed to its complement (inside <-> outside)
	 */
	virtual bool			complement() = 0;

	/*
		Returns true iff this BSP consists of only a single leaf node.
		If it returns true and isAllSpace is not NULL, then *isAllSpace is set to true if the BSP
		represents all space, false otherwise (in which case it represents the empty set).
	 */
	virtual bool			isTrivial( bool* isAllSpace = NULL ) const = 0;

	/*
		Whether or not this BSP represents the combination of two BSPs, an intermediate format
		from which a BSP may be efficiently generated which is the result of boolean operations
		upon the combination.
	 */
	virtual bool			isCombined() const = 0;

#if !USE_GSA	// GSA does not give region volumes
	/*
		Returns the volume of the bounded side of the BSP, if one exists.  If both sides
		are unbounded, NX_MAX_REAL is returned.
		If this is a combined BSP, then you must provide a merge operation.  In this case,
		the BSP will not actually be merged, but the resulting volume will be that of the
		merged BSP you would get if you did perform the merge with the op() function.
		If this is not a combined BSP and you provide a merge operation, it will be ignored.
	 */
	virtual physx::PxF32	getVolume( Operation::Enum operation = Operation::NOP ) const = 0;
#endif

	/*
		Returns the total area of the triangles which will be produced by the BSP.
		If this is a combined BSP, then you must provide a merge operation.  In this case,
		the BSP will not actually be merged, but the resulting area will be that of the
		merged BSP you would get if you did perform the merge with the op() function.
		If this is not a combined BSP and you provide a merge operation, it will be ignored.
	 */
	virtual physx::PxF32	getTriangleArea( Operation::Enum operation = Operation::NOP ) const = 0;

	/*
		Construct a mesh from the current BSP.
		If cleaningTolerance > 0, the mesh will be cleaned using cleaningTolerance as the linear tolerance.
	 */
	virtual bool			toMesh( physx::Array<NxExplicitRenderTriangle>& mesh, physx::PxF32 cleaningTolerance = 0 ) const = 0;

	/*
		Deep copy of given bsp.
		Input bsp may be the same as *this.
		The transform tm will be applied.
		A combined BSP may be copied.
	 */
	virtual	void			copy( const IApexBSP& bsp, const NxMat34& tm = NxMat34() ) = 0;

#if USE_GSA	// Only supporting this feature with GSA
	/*
		Decompose into disjoint islands.
		This BSP is not affected.
		The BSP is split into a set of BSPs, each representing one connected island.
		The set of BSPs is returned as the first BSP in the list, with access
		to the remainder of the list  through the getNext() and getPrev() functions.
		The BSP must be not be a combined BSP (isCombined() = false).
		Returns this if the BSP is already an island.
		Returns NULL if the operation fails (e.g. this is a combined BSP).
	*/
	virtual IApexBSP*		decomposeIntoIslands() const = 0;

	/*
		If a BSP has been decomposed into islands, getNext() and getPrev() will iterate through the
		BSPs in the decomposition.  NULL is returned if an attempt is made to iterate past
		the beginning or end of the list.
	*/
	virtual IApexBSP*		getNext() const = 0;
	virtual IApexBSP*		getPrev() const = 0;
#endif	// #if USE_GSA

	/*
		Serialization.
	 */
	virtual void			serialize( physx::PxFileBuf& stream ) const = 0;
	virtual void			deserialize( physx::PxFileBuf& stream ) = 0;

	/*
		Visualization.
		Set flags to bits from BSPVisualizationFlags::Enum.
	 */
	virtual void			visualize( NxApexRenderDebug& debugRender, physx::PxU32 flags, physx::PxU32 index = 0 ) const = 0;

	/*
		Clean up.
	 */
	virtual	void			release() = 0;

protected:

							IApexBSP()	{}
	virtual					~IApexBSP()	{}
};


// CSG Tools API

// Create a BSP memory cache to share among several BSPs
IApexBSPMemCache*
createBSPMemCache();

// Instantiate a BSP.  If cache = NULL, the BSP will create and own its own cache.
IApexBSP*
createBSP( IApexBSPMemCache* memCache = NULL );

};	// namespace ApexCSG
};

#endif

#endif // #ifndef APEX_CSG_H
