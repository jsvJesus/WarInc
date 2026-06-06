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
#ifndef NX_PARAM_ARRAY_H
#define NX_PARAM_ARRAY_H

#include "NxParameterized.h"
#include "PsUserAllocated.h"
#include "PxAssert.h"

namespace physx {
namespace apex {

struct NxParamDynamicArrayStruct
{
	void *buf;
	bool isAllocated;
	int elementSize;
	int arraySizes[1];
};


template <class ElemType>
class NxParamArray : public physx::UserAllocated
{
public:

	NxParamArray() : mParams(NULL), mArrayStruct(NULL), mArrayHandle(0) {}

	NxParamArray( NxParameterized::Interface* params, const char *arrayName, NxParamDynamicArrayStruct* arrayStruct ) : 
		mParams(params),
		mArrayStruct(arrayStruct),
		mArrayHandle(*params)
	{ 
		PX_ASSERT( mParams);

		mParams->getParameterHandle( arrayName, mArrayHandle );		  

		PX_ASSERT(mArrayStruct->elementSize == sizeof(ElemType));
	}

	NxParamArray( NxParameterized::Interface* params, const NxParameterized::Handle& handle, NxParamDynamicArrayStruct* arrayStruct ) :
		mParams(params),
		mArrayHandle(handle),
		mArrayStruct(arrayStruct)
	{
		PX_ASSERT(mArrayStruct->elementSize == sizeof(ElemType));
	}

	PX_INLINE bool init( NxParameterized::Interface* params, const char* arrayName, NxParamDynamicArrayStruct* arrayStruct )
	{
		if (mParams == NULL && mArrayStruct == NULL)
		{
			mParams = params;
			mArrayStruct = arrayStruct;
			mArrayHandle.setInterface(mParams);
			mParams->getParameterHandle( arrayName, mArrayHandle );

			PX_ASSERT(mArrayStruct->elementSize == sizeof(ElemType));

			return true;
		}
		return false;
	}

	PX_INLINE physx::PxU32 size() const
	{
		// this only works for fixed structs
		//return (physx::PxU32)mArrayHandle.parameterDefinition()->arraySize(0);
		int outSize = 0;
		if (mParams != NULL)
		{
			PX_ASSERT( mArrayHandle.getInterface() == mParams );
			mArrayHandle.getArraySize( outSize );
		}
		return (physx::PxU32)outSize;
	}

	/**
	Returns a constant reference to an element in the sequence.
	*/
	PX_INLINE const ElemType & operator[](unsigned int n) const
	{
		PX_ASSERT(mParams != NULL && mArrayStruct != NULL);
#if _DEBUG
		physx::PxU32 NxParamArraySize = 0;
		mArrayHandle.getArraySize( (int&)NxParamArraySize );
		PX_ASSERT( NxParamArraySize > n );
#endif
		return ((ElemType*)mArrayStruct->buf)[n];
	}

	/**
	Returns a reference to an element in the sequence.
	*/
	PX_INLINE ElemType & operator[](unsigned int n)
	{
		PX_ASSERT(mParams != NULL && mArrayStruct != NULL);
		//NxParameterized::Handle indexHandle;
		//arrayHandle.getChildHandle( n, indexHandle );
#if _DEBUG
		physx::PxU32 NxParamArraySize = 0;
		mArrayHandle.getArraySize( (int&)NxParamArraySize );
		PX_ASSERT( NxParamArraySize > n );
#endif
		return ((ElemType*)mArrayStruct->buf)[n];
	}

	// resize is marginally useful because the ElemType doesn't have proper
	// copy constructors, and if strings are withing ElemType that doesn't work well
	PX_INLINE void resize(unsigned int n)
	{
		PX_ASSERT(mParams != NULL && mArrayStruct != NULL);
		PX_ASSERT( mParams == mArrayHandle.getInterface() );
		mArrayHandle.resizeArray( n );
	}

	// pushBack is marginally useful because the ElemType doesn't have proper
	// copy constructors, and if strings are withing ElemType that doesn't work well
	PX_INLINE void pushBack(const ElemType& x)
	{
		PX_ASSERT(mParams != NULL && mArrayStruct != NULL);

		physx::PxU32 NxParamArraySize = 0;
		
		mArrayHandle.getArraySize( (int&)NxParamArraySize );
		mArrayHandle.resizeArray( NxParamArraySize+1 );
		
		((ElemType*)mArrayStruct->buf)[NxParamArraySize] = x;
	}

	PX_INLINE ElemType& pushBack()
	{
		PX_ASSERT(mParams != NULL && mArrayStruct != NULL);

		physx::PxU32 NxParamArraySize = 0;
		
		mArrayHandle.getArraySize( (int&)NxParamArraySize );
		mArrayHandle.resizeArray(NxParamArraySize+1 );
		
		return ((ElemType*)mArrayStruct->buf)[NxParamArraySize];
	}

	PX_INLINE void replaceWithLast(unsigned position)
	{
		PX_ASSERT(mParams != NULL && mArrayStruct != NULL);

		physx::PxU32 arraySize = size();
		PX_ASSERT(position<arraySize);
		if (position!=arraySize-1)
		{
			// TODO should we call the destructor here or not?
			//(*this)[position].~ElemType();

			ElemType elem = back();

			// put the replaced one in the back (possibly to be deleted)
			(*this)[arraySize-1] = (*this)[position];

			(*this)[position] = elem;
		}
		popBack();
	}

	PX_INLINE bool isEmpty() const
	{
		return size() == 0;
	}

	PX_INLINE ElemType* begin()
	{
		PX_ASSERT(mParams != NULL && mArrayStruct != NULL);
		return &((ElemType*)mArrayStruct->buf)[0];
	}

	PX_INLINE ElemType* end()
	{
		PX_ASSERT(mParams != NULL && mArrayStruct != NULL);
		return &((ElemType*)mArrayStruct->buf)[size()];
	}

	PX_INLINE ElemType& front()
	{
		PX_ASSERT(mParams != NULL && mArrayStruct != NULL);
		return ((ElemType*)mArrayStruct->buf)[0];
	}

	PX_INLINE ElemType& back()
	{
		PX_ASSERT(mParams != NULL && mArrayStruct != NULL);
		return ((ElemType*)mArrayStruct->buf)[size()-1];
	}

	PX_INLINE void clear()
	{	
		PX_ASSERT(mParams != NULL && mArrayStruct != NULL);
		resize(0);
	}

	PX_INLINE void popBack()
	{
		PX_ASSERT(mParams != NULL && mArrayStruct != NULL);
		resize(size()-1);
	}

private:
	NxParameterized::Interface*	mParams;
	NxParameterized::Handle		mArrayHandle;
	NxParamDynamicArrayStruct*	mArrayStruct;
};

}} // end namespace physx::apex

#endif

