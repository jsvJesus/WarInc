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
#include "PVDCommLayerNxDebuggerStream.h"
#include "PvdConnectionTraits.h"
#include "PsMutex.h"
#include "NxRemoteDebugger.h"
#include "PsArray.h"
#include "PsFoundation.h"
#include "PvdConnectionUtilities.h"

#if defined PX_WINDOWS || defined PX_X360
#pragma warning( disable: 4503 )
#endif


namespace PVD
{
	using namespace physx::shdfnd2;
	
	inline char* AllocateMemory( size_t inSize, const char* file, int line)
	{
		PxU32 handle = physx::shdfnd2::getFoundation().getAllocator().getHandle("PVDConnection");
		return (char*) physx::shdfnd2::getFoundation().getAllocator().allocate(inSize, handle, file, line);
	}

	inline void DeallocateMemory( char* inMem )
	{
		physx::shdfnd2::getFoundation().getAllocator().deallocate(inMem);
	}

	template<typename TDataType>
	inline void DeallocateMemory( TDataType* inMem ) { DeallocateMemory( reinterpret_cast< char* >( inMem ) ); }

#include "ClientDllAllocator.h"
	
	typedef ClientDllAllocator<char> TAllocatorType;

	typedef ClientDllAllocator<char> TAllocatorType;
	typedef physx::shdfnd2::MutexT<TAllocatorType> TMutexType;
	typedef TMutexType::ScopedLock TScopedLockType;
	typedef PvdConnectionTraits< TMutexType,TScopedLockType,TAllocatorType> TPhysxConnectionTraits;



	template<physx::PxU32 CacheSize = 8192>
	class CPVDRDOutStream : public PvdCommOutStream
	{
		NxRemoteDebugger* mDebugger;
		Array<PxU8> mTempStorage;

	public:
		CPVDRDOutStream(){}
		CPVDRDOutStream( NxRemoteDebugger* inDebugger )
			: mDebugger( inDebugger )
		{
			if ( mDebugger->isConnected() )
			{
				mDebugger->createObject(this, NX_DBG_OBJECTTYPE_GENERIC, "PVDCommLayerTunnel", NX_DBG_EVENTGROUP_BASIC_OBJECTS);
			}
			mTempStorage.resize( 4, 0 );
		}
		virtual PvdCommLayerError write( const physx::PxU8* inBytes, physx::PxU32 inLength )
		{
			if ( !mDebugger->isConnected() )
				return PvdCommLayerError::NetworkError;

			while ( inLength )
			{
				physx::PxU32 currentAmount = static_cast<physx::PxU32>( mTempStorage.size() );
				physx::PxU32 amountToWrite = std::min( CacheSize - currentAmount, inLength );
				inLength -= amountToWrite;
				currentAmount += amountToWrite;
				for ( physx::PxU32 idx = 0; idx < amountToWrite; ++idx, ++inBytes )
					mTempStorage.pushBack( inBytes[0] );
				if ( currentAmount == CacheSize )
					FlushData();
			}
			return PvdCommLayerError::None;
		}
		virtual PvdCommLayerError flush() 
		{ 
			FlushData(); mDebugger->flush(); return PvdCommLayerError::None; 
		}
		virtual bool isConnected() const { return mDebugger->isConnected(); }
		virtual void destroy() { return TPhysxConnectionTraits::DeallocateObject(this); }

		inline void FlushData()
		{
			physx::PxU32 messageSize( static_cast<physx::PxU32>(mTempStorage.size() ) );
			physx::PxU8* theLen = reinterpret_cast< physx::PxU8* >( &messageSize );
			mTempStorage[0] = theLen[0];
			mTempStorage[1] = theLen[1];
			mTempStorage[2] = theLen[2];
			mTempStorage[3] = theLen[3];
			mDebugger->writeParameter(&mTempStorage[0], this, true, "Tunnel", NX_DBG_EVENTGROUP_BASIC_OBJECTS_DYNAMIC_DATA);
			mTempStorage.resize( 4 );
		}
	};

	PvdConnection* CreatePVDConnection( NxRemoteDebugger* inRemoteDebugger )
	{
		CPVDRDOutStream<>* theOutStream = TPhysxConnectionTraits::AllocateObject<CPVDRDOutStream<> > (inRemoteDebugger);
		return TPhysxConnectionTraits::createDebuggerConnection( NULL, theOutStream, true, "apex" );
	}
	
	PvdCommLayerError CreateObject( PvdConnection* inConnection, physx::PxU64 inInstanceKey, const char* inTypeName, PVD::EInstanceUIFlags inFlags )
	{
		physx::PxU32 theClassKey = HashFunction( inTypeName ); //Use the pointer address as the key.
		PvdCommLayerError theError = inConnection->createClass( inTypeName, theClassKey );
		if ( theError == PvdCommLayerError::None )
			theError = inConnection->createInstance( theClassKey, inInstanceKey, inFlags );
		return theError;
	}

	PvdCommLayerError SetPropertyValue( PvdConnection* inConnection, physx::PxU64 inInstanceKey, const PvdCommLayerValue& inValue, bool inCreate, const char* inName )
	{
		return PVD::PvdConnectionUtilities::setPropertyValue( inConnection, inInstanceKey, inName, inValue, inCreate );
	}
}