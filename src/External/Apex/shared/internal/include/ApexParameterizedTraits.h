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
#ifndef NX_APEX_PARAMETERIZED_TRAITS_H
#define NX_APEX_PARAMETERIZED_TRAITS_H

#include "NxParameterizedTraits.h"
#include "PsUserAllocated.h"
#include "PsAtomic.h"
#include "PsHashMap.h"
#include "PsHashSet.h"

#define NX_APEX_PARAMETERIZED_FACTORY_NAME_SPACE "NxParameterizedFactories"

#define DEBUG_ASSERT(x) //PX_ASSERT(x)
#define DEBUG_ALWAYS_ASSERT() DEBUG_ASSERT(0)

namespace physx {
namespace apex {

class ApexParameterizedTraits: public NxParameterized::Traits, public physx::UserAllocated
{
	typedef physx::Pair<PxU32, PxU32> PxU32Pair;

	struct ClassInfo
	{
		PxU32 currentVersion;

		typedef physx::HashMap<PxU32, NxParameterized::Factory *> MapVer2Factory;
		MapVer2Factory factors;

		//Conversions
		typedef HashMap<PxU32Pair, NxParameterized::Conversion *> MapVers2Conversion;
		MapVers2Conversion convs;

		//Shortest paths
		bool arePathsValid;
		typedef HashMap<PxU32Pair, PxU32> MapVers2Next;
		MapVers2Next next;

		ClassInfo(): currentVersion(static_cast<PxU32>(-1)), arePathsValid(true) {}

		NxParameterized::Factory *currentFactory()
		{
			return -1 == currentVersion ? 0 : factors[currentVersion];
		}

		//FIXME: we need this because of broken copy constructor of PsHashMap
		ClassInfo(const ClassInfo &inf): currentVersion(inf.currentVersion), factors(inf.factors.size()), arePathsValid(false)
		{
			ClassInfo &inf_ = const_cast<ClassInfo &>(inf);
			for(MapVer2Factory::Iterator i = inf_.factors.getIterator(); !i.done(); ++i)
				factors[i->first] = i->second;
		}

		//Update shortest paths using Floyd-Warshall algorithm
		void updatePaths(NxParameterized::Traits *traits)
		{
			this->next.clear();

			//Enumerate versions

			HashSet<PxU32> vers;
			for(MapVers2Conversion::Iterator i = convs.getIterator(); !i.done(); ++i)
			{
				vers.insert(i->first.first);
				vers.insert(i->first.second);
			}

			HashMap<PxU32, PxU32> mapVer2Idx;
			Array <PxU32> mapIdx2Ver;

			for(HashSet<PxU32>::Iterator i = vers.getIterator(); !i.done(); ++i)
			{
				mapVer2Idx[*i] = mapIdx2Ver.size();
				mapIdx2Ver.pushBack(*i);
			}

			if( !mapIdx2Ver.size() )
				return;

			//Calculate shortest paths (see http://en.wikipedia.org/wiki/Floyd-Warshall_algorithm)

			PxU32 n = mapIdx2Ver.size();

			PxU32 *cost = (PxU32 *)traits->alloc(n * n * sizeof(PxU32)),
				*next = (PxU32 *)traits->alloc(n * n * sizeof(PxU32));

#			define ELEM(a, i, j) (*((a) + n * (i) + (j)))

			for(PxU32 i = 0; i < n*n; ++i)
			{
				cost[i] = next[i] = PxU32(-1);
			}

			for(PxU32 i = 0; i < n; ++i)
			{
				ELEM(cost, i, i) = 0;
				ELEM(next, i, i) = i;
			}

			for(MapVers2Conversion::Iterator i = convs.getIterator(); !i.done(); ++i)
			{
				PxU32 from = mapVer2Idx[i->first.first],
					to = mapVer2Idx[i->first.second];

				ELEM(cost, from, to) = 1;
				ELEM(next, from, to) = to;
			}

			for(PxU32 k = 0; k < n; ++k)
				for(PxU32 i = 0; i < n; ++i)
					for(PxU32 j = 0; j < n; ++j)
					{
						PxU32 tmp = ELEM(cost, i, k) + ELEM(cost, k, j);
						if( tmp < ELEM(cost, i, j) )
						{
							ELEM(cost, i, j) = tmp;
							ELEM(next, i, j) = k;
						}
					}

			//Back to versions

			for(PxU32 i = 0; i < n; ++i)
			{
				PxU32 from = mapIdx2Ver[i];

				for(PxU32 j = 0; j < n; ++j)
				{
					PxU32 to = mapIdx2Ver[j];

					PxU32 k = ELEM(next, i, j);
					this->next[PxU32Pair(from, to)] = k != -1 ? mapIdx2Ver[k] : -1;
				}
			}

#			undef ELEM

			traits->free(cost);
			traits->free(next);
		}
	};

	ClassInfo *getClassInfo(const char *name, bool emitDebugWarning = true) const
	{
		NiResourceProvider *nrp = NiGetApexSDK()->getInternalResourceProvider();
		NxResourceProvider *xnrp = NiGetApexSDK()->getNamedResourceProvider();

		if( !nrp->checkResource( mFactoryNsResID, name ) )
		{
			if( emitDebugWarning )
			{
				APEX_DEBUG_WARNING("No NxParameterized::Factory is registered for %s", name);
			}
			return NULL;
		}

		return static_cast<ClassInfo*>(
			xnrp->getResource( NX_APEX_PARAMETERIZED_FACTORY_NAME_SPACE, name ) );
	}

	NxParameterized::Factory* getFactory(const char *name, PxU32 ver) const
	{
		ClassInfo *info = getClassInfo(name);
		if( !info )
			return 0;

		const physx::HashMap<PxU32, NxParameterized::Factory *>::Entry *e = info->factors.find(ver);
		return e ? const_cast<NxParameterized::Factory *>(e->second) : 0;
	}

	typedef physx::HashSet<const char *> StringSet;
	StringSet classNames;

public:
	ApexParameterizedTraits(  ) :
	  mFactoryNsResID(INVALID_RESOURCE_ID)
	{
		if( NiGetApexSDK()->getInternalResourceProvider() )
		{
			mFactoryNsResID = NiGetApexSDK()->getInternalResourceProvider()->createNameSpace( NX_APEX_PARAMETERIZED_FACTORY_NAME_SPACE, false );
		}
	}

	~ApexParameterizedTraits()
	{
		NxResourceProvider *xnrp = NiGetApexSDK()->getNamedResourceProvider();

		for(StringSet::Iterator i = classNames.getIterator(); !i.done(); ++i)
		{
			const char *className = *i;

			ClassInfo *info = getClassInfo(className, false);
			if( !info )
			{
				PX_ALWAYS_ASSERT();
				continue;
			}

			if( info->convs.size() )
				APEX_DEBUG_WARNING("Some conversions for class %s were not released", className);

			if( info->factors.size() )
				APEX_DEBUG_WARNING("Some factories for class %s were not released", className);

			info->~ClassInfo();
			free(info);

			// remove ref in NRP
			xnrp->setResource( NX_APEX_PARAMETERIZED_FACTORY_NAME_SPACE, className, NULL, false );
		}
	}

	void registerFactory( NxParameterized::Factory &factory )
	{
		const char *factoryName = factory.getClassName();
		if( !factoryName || !factoryName[1] )
		{
			APEX_DEBUG_WARNING("NxParameterized::Factory does not have a valid name");
			return;
		}

		// check if we didn't have the NRP when the constructor ran
		if( INVALID_RESOURCE_ID == mFactoryNsResID )
		{
			if( NiGetApexSDK()->getInternalResourceProvider() )
			{
				mFactoryNsResID = NiGetApexSDK()->getInternalResourceProvider()->createNameSpace( NX_APEX_PARAMETERIZED_FACTORY_NAME_SPACE, false );
			}

			if( INVALID_RESOURCE_ID == mFactoryNsResID )
			{
				traitsWarn("Failed to create factory namespace");
				DEBUG_ALWAYS_ASSERT();
				return;
			}
		}

		NiResourceProvider *nrp = NiGetApexSDK()->getInternalResourceProvider();
		NxResourceProvider *xnrp = NiGetApexSDK()->getNamedResourceProvider();

		PxU32 factoryVer = factory.getVersion();

		if( nrp->checkResource(mFactoryNsResID, factoryName) )
		{
			ClassInfo *info = static_cast<ClassInfo *>(xnrp->getResource( NX_APEX_PARAMETERIZED_FACTORY_NAME_SPACE, factoryName ) );

			info->currentVersion = PxMax(factoryVer, info->currentVersion); //Assume that the biggest version is the current one
			info->factors[factoryVer] = &factory;
		}
		else
		{
			ClassInfo *info = PX_PLACEMENT_NEW(alloc(sizeof(ClassInfo)), ClassInfo)();

			nrp->createResource( mFactoryNsResID, factoryName, false );
			xnrp->setResource( NX_APEX_PARAMETERIZED_FACTORY_NAME_SPACE, factoryName, info, false );

			info->currentVersion = factoryVer;
			info->factors[factoryVer] = &factory;
		}

		classNames.insert(factoryName);
	}

	NxParameterized::Factory *removeFactory( const char * className )
	{
		ClassInfo *info = getClassInfo(className, false);
		if( !info )
			return 0;

		return removeFactory(className, info->currentVersion);
	}

	NxParameterized::Factory *removeFactory( const char * className, physx::PxU32 version )
	{
		ClassInfo *info = getClassInfo(className, false);
		if( !info )
			return 0;

		NxParameterized::Factory *factor = info->factors[version];
		info->factors.erase(version);

		return factor;
	}

	NxParameterized::Interface * createNxParameterized( const char * name )
	{
		ClassInfo* info = getClassInfo(name);

		if( !info )
		{
			traitsWarn("Class info does not exist");
			DEBUG_ALWAYS_ASSERT();
			return 0;
		}

		return info->currentFactory()->create( this );
	}

	NxParameterized::Interface * finishNxParameterized( const char * name, void *obj, void *buf, physx::PxI32 *refCount )
	{
		ClassInfo* info = getClassInfo(name);

		if( !info )
		{
			traitsWarn("Class info does not exist");
			DEBUG_ALWAYS_ASSERT();
			return 0;
		}

		return info->currentFactory()->finish( this, obj, buf, refCount );
	}

	NxParameterized::Interface * createNxParameterized( const char * name, PxU32 ver )
	{
		NxParameterized::Factory* factory = getFactory(name, ver);

		if( !factory )
		{
			traitsWarn("Factory does not exist");
			DEBUG_ALWAYS_ASSERT();
			return 0;
		}

		return factory->create(this);
	}

	NxParameterized::Interface * finishNxParameterized( const char * name, PxU32 ver, void *obj, void *buf, physx::PxI32 *refCount )
	{
		NxParameterized::Factory* factory = getFactory(name, ver);

		if( !factory )
		{
			traitsWarn("Factory does not exist");
			DEBUG_ALWAYS_ASSERT();
			return 0;
		}

		return factory->finish(this, obj, buf, refCount);
	}

	PxU32 getCurrentVersion(const char *className) const
	{
		ClassInfo *info = getClassInfo(className);

		if( !info )
		{
			traitsWarn("Class info does not exist");
			DEBUG_ALWAYS_ASSERT();
			return (physx::PxU32)-1;
		}

		return info->currentVersion;
	}

	bool getNxParameterizedNames( const char ** names, physx::PxU32 &outCount, physx::PxU32 inCount) const
	{
		NxResID ids[512];
		NiResourceProvider *nrp = NiGetApexSDK()->getInternalResourceProvider();
		
		if( !nrp->getResourceIDs(NX_APEX_PARAMETERIZED_FACTORY_NAME_SPACE, ids, outCount, sizeof(ids)/sizeof(ids[0])) || outCount > inCount )
			return false;

		for( physx::PxU32 i=0; i<outCount; i++ )
		{
			names[i] = nrp->getResourceName( ids[i] );
		}

		return true;
	}

	void registerConversion(const char *className, PxU32 from, PxU32 to, NxParameterized::Conversion &conv)
	{
		if( from == to )
			return;

		ClassInfo *info = getClassInfo(className, false);
		if( !info )
			return;

		info->arePathsValid = false; //Recalculate paths
		info->convs[PxU32Pair(from, to)] = &conv;
	}

	NxParameterized::Conversion *removeConversion(const char *className, PxU32 from, PxU32 to)
	{
		ClassInfo *info = getClassInfo(className, false);
		if( !info )
			return 0;

		info->arePathsValid = false; //Recalculate paths

		PxU32Pair key(from, to);

		NxParameterized::Conversion *conv = info->convs[key];
		info->convs.erase(key);

		return conv;
	}

	bool updateLegacyNxParameterized(NxParameterized::Interface &legacyObj, NxParameterized::Interface &obj)
	{
		ClassInfo *info = getClassInfo(obj.className());
		if( !info )
			return false;

		if( !info->arePathsValid )
			info->updatePaths(this);

		NxParameterized::Interface *curObj = &legacyObj;

		PxU32 curVer = curObj->version();
		PxU32 targetVer = obj.version();
		PxU32 headVer = getCurrentVersion(obj.className());

		PX_ASSERT(targetVer == headVer);
		if (targetVer != headVer)
		{
			APEX_DEBUG_WARNING("Trying to upgrade a %s class but the version (%s) is not the HEAD (%s)", obj.className(), targetVer, headVer);
			return false;
		}

		while( curVer != targetVer )
		{
			//Next intermediate version
			const ClassInfo::MapVers2Next::Entry *e = info->next.find(PxU32Pair(curVer, targetVer));
			if( !e )
				return false;

			PxU32 nextVer = e->second;
			if( -1 == nextVer )
				return false;

			//Conversion to interim version
			if( !info->convs.find(PxU32Pair(curVer, nextVer)) )
			{
				traitsWarn("Conversion does not exist");
				DEBUG_ALWAYS_ASSERT();
				return false;
			}

			NxParameterized::Conversion &conv = *info->convs[PxU32Pair(curVer, nextVer)];

			NxParameterized::Interface *nextObj = nextVer == targetVer ? &obj : createNxParameterized(obj.className(), nextVer);
			if( !nextObj )
				return false;

			if( !conv(*curObj, *nextObj) )
				return false;

			if( curObj != &legacyObj ) //legacyObj is owned by app
				curObj->destroy();

			curObj = nextObj;
			curVer = nextVer;
		}

		return true;
	}

	// These must be overloaded        
	void *alloc(physx::PxU32 nbytes)
	{
		return PX_ALLOC( nbytes );
	}

	void free(void *buf)
	{
		PX_FREE( buf );
	}

	physx::PxI32 incRefCount(physx::PxI32 *refCount)
	{
		return physx::atomicIncrement(refCount);
	}

	virtual physx::PxI32 decRefCount(physx::PxI32 *refCount)
	{
		return physx::atomicDecrement(refCount);
	}

	virtual void traitsWarn(const char *msg) const
	{
		APEX_DEBUG_WARNING("%s", msg);
	}

private:
	NxResID								mFactoryNsResID;
};

}} // end namespace physx::apex

#undef DEBUG_ASSERT
#undef DEBUG_ALWAYS_ASSERT

#endif

