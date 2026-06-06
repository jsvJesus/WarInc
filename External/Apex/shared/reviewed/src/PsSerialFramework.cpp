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
// Copyright (c) 2009-2011 NVIDIA Corporation. All rights reserved.
// Copyright (c) 2002-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2006 NovodeX. All rights reserved.   

/*
	TODO:
	- if things are not "automatic" through macros, how can we resolve all pointers/etc *before* creating the class? We can't.
	- streaming sample
	- articulations
	- add an empty ctor to PxBounds3, remove copy-ctor-to-itself (same for PxTransform)
	- fix insane design issues in HL/GeomUtils (GeomUtils class using HL class, etc)
- BigConvexData*				supportVertexMap;		//!< optional, only for large meshes! PT: REDUNDANT with ptr in chull data
- optimize BigConvexData space... 
	- Also PxHullGaussMapData, Gu::Valency and PxValenciesData could all use shorter types.
	- Ultimately Gu::ConvexHullData::mBigConvexData is not even needed, it would be yet another location in the big "mPolygons" buffer
	- Try to extract address from PxSerializable => PxResolvable : public PxSerializable
	- Try to teach the framework about inline arrays, etc
*/


// PX_SERIALIZATION

//#define DEBUG_STREAM_SIZES

#pragma warning( disable : 4512 ) //  assignment operator could not be generated
#pragma warning( disable : 4127 ) //  conditional expression is constant

#include "PxUserOutputStream.h"
#include "PxSerialFramework.h"
#include "PsSerialFramework.h"
#include "PsSerialAlignment.h"

#define SERIAL_STATS
#ifdef SERIAL_STATS
#include <stdio.h>

namespace physx
{
namespace shdfnd2
{
	BEGIN_FIELDS(ExtraDataMarker)
	END_FIELDS(ExtraDataMarker)

	struct PxSerialStats
	{
		PX_INLINE PxSerialStats() : count(0), totalSize(0)	{}

		PxU32	count;
		PxU32	totalSize;
	};
static const char* SerialName(PxU32 i)
{
	switch(i)
	{
	case NXTYPE_UNDEFINED:					return "NXTYPE_UNDEFINED";
	case NXTYPE_EXTRA_DATA:					return "NXTYPE_EXTRA_DATA";

	case NXTYPE_NP_HEIGHTFIELD:				return "NXTYPE_NP_HEIGHTFIELD";
	case NXTYPE_NP_CONVEX_MESH:				return "NXTYPE_NP_CONVEX_MESH";
	case NXTYPE_NP_TRIANGLE_MESH:			return "NXTYPE_NP_TRIANGLE_MESH";
	case NXTYPE_NP_DEFORMABLE_MESH:			return "NXTYPE_NP_DEFORMABLE_MESH";

	case NXTYPE_NP_RIGID_DYNAMIC:			return "NXTYPE_NP_RIGID_DYNAMIC";
	case NXTYPE_NP_RIGID_STATIC:			return "NXTYPE_NP_RIGID_STATIC";
	case NXTYPE_NP_SHAPE:					return "NXTYPE_NP_SHAPE";
	case NXTYPE_NP_MATERIAL:				return "NXTYPE_NP_MATERIAL";
	case NXTYPE_NP_CONSTRAINT:				return "NXTYPE_NP_CONSTRAINT";
	case NXTYPE_NP_DEFORMABLE:				return "NXTYPE_NP_DEFORMABLE";
	case NXTYPE_NP_PARTICLE_SYSTEM:			return "NXTYPE_NP_PARTICLE_SYSTEM";
	case NXTYPE_NP_PARTICLE_FLUID:			return "NXTYPE_NP_PARTICLE_FLUID";
	case NXTYPE_NP_WHEEL:					return "NXTYPE_NP_WHEEL";
	case NXTYPE_NP_ATTACHMENT:				return "NXTYPE_NP_ATTACHMENT";

//	case NXTYPE_SC_BODY_CORE:				return "NXTYPE_SC_BODY_CORE";
//	case NXTYPE_SC_RIGID_CORE:				return "NXTYPE_SC_RIGID_CORE";
//	case NXTYPE_SC_SHAPE_CORE:				return "NXTYPE_SC_SHAPE_CORE";
//	case NXTYPE_SC_CONSTRAINT_CORE:			return "NXTYPE_SC_CONSTRAINT_CORE";
//	case NXTYPE_SC_DEFORMABLE_CORE:			return "NXTYPE_SC_DEFORMABLE_CORE";
//	case NXTYPE_SC_PARTICLE_SYSTEM_CORE:	return "NXTYPE_SC_PARTICLE_SYSTEM_CORE";
//	case NXTYPE_SC_WHEEL_CORE:				return "NXTYPE_SC_WHEEL_CORE";

	case NXTYPE_PXS_PARTICLE_SYSTEM_CORE:	return "NXTYPE_PXS_PARTICLE_SYSTEM_CORE";
	case NXTYPE_PXS_DEFORMABLE_CORE:		return "NXTYPE_PXS_DEFORMABLE_CORE";

	case NXTYPE_USER_SPHERICAL_JOINT:		return "NXTYPE_USER_SPHERICAL_JOINT";
	case NXTYPE_USER_REVOLUTE_JOINT:		return "NXTYPE_USER_REVOLUTE_JOINT";
	case NXTYPE_USER_PRISMATIC_JOINT:		return "NXTYPE_USER_PRISMATIC_JOINT";
	case NXTYPE_USER_FIXED_JOINT:			return "NXTYPE_USER_FIXED_JOINT";
	case NXTYPE_USER_DISTANCE_JOINT:		return "NXTYPE_USER_DISTANCE_JOINT";
	case NXTYPE_USER_D6_JOINT:				return "NXTYPE_USER_D6_JOINT";
	};
	return NULL;
}
#endif

typedef bool (*ProcessSerializableCallback)	(PxSerializable* s, void* userData);
static bool processCollection(InternalCollection& c, ProcessSerializableCallback cb, void* userData, bool processEmbedded)
{
	const PxU32 nb = c.internalGetNbObjects();
	for(PxU32 i=0;i<nb;i++)
	{
		PxSerializable* s = c.internalGetObject(i);
//		if(s->isSerializationDisabled())
//			continue;
		if(!(cb)(s, userData))
			return false;

		if(processEmbedded)
		{
			FieldDescriptors fields PX_DEBUG_EXP("FieldDescriptors");
			s->getFields(fields, PxField::eSERIAL_EMBEDDED);

			const PxU32 nbFields = (PxU32)fields.size();
			for(PxU32 j=0;j<nbFields;j++)
			{
				const FieldDescriptor* fd = fields[j];				
				PxSerializable* embedded = reinterpret_cast<PxSerializable*>(fd->Address(s));
				if(!(cb)(embedded, userData))
					return false;
			}
		}
	}
	return true;
}

void serializeCollection(InternalCollection& collection, PxSerialStream& stream)
{
	{
		struct SortCallback
		{
			static int compare(const void* e0, const void* e1)
			{
				PxSerializable** s0 = (PxSerializable**)e0;
				PxSerializable** s1 = (PxSerializable**)e1;
				const PxU32 order1 = (*s1)->getOrder();
				const PxU32 order0 = (*s0)->getOrder();
				return (int)order0 - (int)order1;
			}
		};

		PxU32 nbObjects = collection.internalGetNbObjects();
		PxSerializable** objects = collection.internalGetObjects();
		qsort(objects, nbObjects, sizeof(PxSerializable*), SortCallback::compare);
	}

	struct Local
	{
		static bool ExportSerializableToStream(PxSerializable* s, void* userData)
		{
			PX_ASSERT(s->getType());
			PxSerialStream* stream = reinterpret_cast<PxSerialStream*>(userData);
			physx::shdfnd2::alignStream(*stream);

#ifdef DEBUG_STREAM_SIZES
			printf("%d\n", stream->getTotalStoredSize());
#endif
			// Test: optimization: automatically disable "auto resolve" if not needed
			// ### does that work for embedded objects???
/*			if(0)
			{
				s->disableAutoResolve();
//				FieldDescriptors fields(PX_DEBUG_EXP("FieldDescriptors"));	// Doesn't compile in Release!!
				FieldDescriptors fields;
				s->getFields(fields, PxField::eSERIAL_PTR);

				const PxU32 nbFields = (PxU32)fields.size();
				for(PxU32 j=0;j<nbFields;j++)
				{
					const FieldDescriptor* fd = fields[j];
					PxSerializable** tmp = reinterpret_cast<PxSerializable**>(fd->Address(s));
					if(*tmp)
					{
						s->enableAutoResolve();
						break;
					}
				}
			}

			if(0)
			{
				s->disableFields();
//				FieldDescriptors fields(PX_DEBUG_EXP("FieldDescriptors"));	// Doesn't compile in Release!!
				FieldDescriptors fields;
				s->getFields(fields, F_SERIALIZE);

				const PxU32 nbFields = (PxU32)fields.size();
				for(PxU32 j=0;j<nbFields;j++)
				{
					const FieldDescriptor* fd = fields[j];
					if(fd->IsDynamicArray())
					{
						if(fd->GetArrayAddress(s))
						{
							s->enableFields();
							break;
						}
					}
					else if(fd->mType==FIELD_STRING)
					{
						s->enableFields();
						break;
					}
					else if(fd->mType==FIELD_PX_ARRAY)
					{
						s->enableFields();
						break;
					}
				}
			}*/

			stream->storeBuffer(s, s->getObjectSize());
			return true;
		}

		static bool ExportExtraData(PxSerializable* s, void* userData)
		{
			PxSerialStream* stream = reinterpret_cast<PxSerialStream*>(userData);
			physx::shdfnd2::alignStream(*stream);
			s->exportExtraData(*stream);
			return true;
		}

		static bool ExportFields(PxSerializable* s, void* userData)
		{
			PxSerialStream* stream = reinterpret_cast<PxSerialStream*>(userData);

			FieldDescriptors fields PX_DEBUG_EXP("FieldDescriptors");
			s->getFields(fields, F_SERIALIZE);

			const PxU32 nbFields = (PxU32)fields.size();
			for(PxU32 j=0;j<nbFields;j++)
			{
				const FieldDescriptor* fd = fields[j];
				if(fd->IsDynamicArray())
				{
					void* arrayBase = fd->GetArrayAddress(s);
					if(arrayBase)
					{
						const PxU32 arraySize = fd->GetDynamicArraySize(s);
						const PxU32 fieldSize = fd->FieldSize();
						if(fd->mFlags & F_ALIGN)
							physx::shdfnd2::alignStream(*stream);
						stream->storeBuffer(arrayBase, arraySize * fieldSize);
					}
				}
				else if(fd->mType==PxField::eSTRING)
				{
					char* stringBase = reinterpret_cast<char*>(fd->GetArrayAddress(s));
					const PxU32 stringSize = (PxU32)strlen(stringBase)+1;
					if(fd->mFlags & F_ALIGN)
						physx::shdfnd2::alignStream(*stream);
					stream->storeBuffer(stringBase, stringSize * sizeof(char));
				}
				else if(fd->mType==PxField::ePX_ARRAY)
				{
					Array<int>* pxArray = reinterpret_cast<Array<int>*>(fd->Address(s));
					if(fd->mFlags & F_ALIGN)
						physx::shdfnd2::alignStream(*stream);
					pxArray->exportArray(*stream, false);
				}
				else
				{
					PX_ASSERT(!"Found unsupported F_SERIALIZE field!");
				}
			}
			return true;
		}
	};

	processCollection(collection, Local::ExportSerializableToStream, &stream, false);
	//

	ExtraDataMarker tmp;
	alignStream(stream);
	stream.storeBuffer(&tmp, tmp.getObjectSize());

	//
	processCollection(collection, Local::ExportFields, &stream, true);
	processCollection(collection, Local::ExportExtraData, &stream, true);
}

bool deserializeCollection(InternalCollection& collection, RefResolver& Ref, void* buffer)
{
#ifdef SERIAL_STATS
	PxSerialStats stats[NXTYPE_LAST];
#endif
	PxU32 totalPadding = 0;

	char* Address = reinterpret_cast<char*>(buffer);
#ifdef _DEBUG
	PxU32 nbClasses = 0;
#endif
	bool HandleExtraData = false;
	while(!HandleExtraData)
	{
		Address = alignStream(Address, totalPadding);

#ifdef SERIAL_STATS
		char* BaseAddress = Address;
#endif

		PxSerializable* H = reinterpret_cast<PxSerializable*>(Address);
#ifdef _DEBUG
		PxSerializable* OldAddress = H->getAddress();
		{
			int* vtable = (int*)H;
			*vtable = 0;	// Just to check it's re-created correctly
		}
#endif
		PxSerializable* Serial = NULL;
		const PxType classType = H->getType();
		switch(classType)
		{
			case NXTYPE_EXTRA_DATA:
			{
				HandleExtraData = true;
				Address += sizeof(ExtraDataMarker);
			}
			break;

			default:
			{
				Serial = createClass(classType, Address, Ref);
				PX_ASSERT(Serial);
			}
			break;
		};

#ifdef SERIAL_STATS
		if(Serial)
		{
			stats[classType].count++;
			stats[classType].totalSize += PxU32(Address - BaseAddress);
#ifdef DEBUG_STREAM_SIZES
			static PxU32 tsize = 8;
			tsize += PxU32(Address - BaseAddress);
			printf("%d\n", tsize);
//			printf("%d\n", PxU32(Address - BaseAddress));
#endif
		}
#endif

		if(Serial)
		{
			collection.internalAdd(Serial);
			PX_ASSERT(Ref.newAddress(OldAddress) == Serial);
#ifdef _DEBUG
			nbClasses++;
#endif
		}
	}

	{
		struct Local
		{
			struct ImportParams
			{
				char**			mAddress;
				PxU32			mTotalPadding;
#ifdef SERIAL_STATS
				PxSerialStats*	mStats;
#endif
			};

			static bool ImportFields(PxSerializable* s, void* userData)
			{
				if(s->areFieldsDisabled())
					return true;

				ImportParams* params = reinterpret_cast<ImportParams*>(userData);

				FieldDescriptors fields PX_DEBUG_EXP("FieldDescriptors");
				s->getFields(fields, F_SERIALIZE);

				const PxU32 nbFields = (PxU32)fields.size();
				for(PxU32 j=0;j<nbFields;j++)
				{
					const FieldDescriptor* fd = fields[j];
					if(fd->IsDynamicArray())
					{
						if(fd->GetArrayAddress(s))
						{
							char* Address = *params->mAddress;
							if(fd->mFlags & F_ALIGN)
								Address = physx::shdfnd2::alignStream(Address, params->mTotalPadding);

							char** arrayAddress = reinterpret_cast<char**>(fd->Address(s));
							*arrayAddress = Address;

							const PxU32 arraySize = fd->GetDynamicArraySize(s);
							const PxU32 fieldSize = fd->FieldSize();
							Address += arraySize * fieldSize;
#ifdef SERIAL_STATS
							params->mStats[s->getType()].totalSize += arraySize * fieldSize;
#endif
							*params->mAddress = Address;
						}
					}
					else if(fd->mType==PxField::eSTRING)
					{
						char* Address = *params->mAddress;
						if(fd->mFlags & F_ALIGN)
							Address = physx::shdfnd2::alignStream(Address, params->mTotalPadding);

						char** arrayAddress = reinterpret_cast<char**>(fd->Address(s));
						*arrayAddress = Address;

						const PxU32 stringSize = (PxU32)strlen(*arrayAddress)+1;
						Address += stringSize * sizeof(char);
#ifdef SERIAL_STATS
						params->mStats[s->getType()].totalSize += stringSize * sizeof(char);
#endif
						*params->mAddress = Address;
					}
					else if(fd->mType==PxField::ePX_ARRAY)
					{
						char* Address = *params->mAddress;
						if(fd->mFlags & F_ALIGN)
							Address = physx::shdfnd2::alignStream(Address, params->mTotalPadding);

						Array<int>* pxArray = reinterpret_cast<Array<int>*>(fd->Address(s));
#ifdef SERIAL_STATS
						char* previousAddress = Address;
#endif
						Address = pxArray->importArray(Address);
#ifdef SERIAL_STATS
						params->mStats[s->getType()].totalSize += PxU32(Address - previousAddress);
#endif
						*params->mAddress = Address;
					}
					else
					{
						PX_ASSERT(!"Found unsupported F_SERIALIZE field!");
					}
				}
				return true;
			}

			static bool ImportExtraData(PxSerializable* s, void* userData)
			{
				ImportParams* params = reinterpret_cast<ImportParams*>(userData);
				char* Address = *params->mAddress;

				Address = physx::shdfnd2::alignStream(Address, params->mTotalPadding);
#ifdef SERIAL_STATS
				char* previousAddress = Address;
#endif
				Address = s->importExtraData(Address, params->mTotalPadding);
#ifdef SERIAL_STATS
				params->mStats[s->getType()].totalSize += PxU32(Address - previousAddress);
#endif
				*params->mAddress = Address;
				return true;
			}
		};

		Local::ImportParams	importParams;
		importParams.mAddress		= &Address;
		importParams.mTotalPadding	= 0;
#ifdef SERIAL_STATS
		importParams.mStats			= stats;
#endif
		processCollection(collection, Local::ImportFields, &importParams, true);
		processCollection(collection, Local::ImportExtraData, &importParams, true);
		totalPadding += importParams.mTotalPadding;
	}

//	if(1)	// Automatic resolve
	{
		struct Local
		{
			static bool AutoResolve(PxSerializable* s, void* userData)
			{
				if(s->isAutoResolveDisabled())
					return true;

				RefResolver& resolver = *reinterpret_cast<RefResolver*>(userData);

				FieldDescriptors fields PX_DEBUG_EXP("FieldDescriptors");
				s->getFields(fields, PxField::eSERIAL_PTR);

				const PxU32 nbFields = (PxU32)fields.size();
				for(PxU32 j=0;j<nbFields;j++)
				{
					const FieldDescriptor* fd = fields[j];

					PxSerializable** tmp = reinterpret_cast<PxSerializable**>(fd->Address(s));
					if(*tmp)
					{
						void* relocated = resolver.newAddress(*tmp);
						if(relocated)
							*tmp = reinterpret_cast<PxSerializable*>(relocated);
						else
						{
							getFoundation().error(PxErrorCode::eSERIALIZATION_ERROR, __FILE__, __LINE__, "Serial framework: pointer not found.");
							return false;
						}
					}
				}
				return true;
			}
		};
		if(!processCollection(collection, Local::AutoResolve, &Ref, true))
			return false;
	}
/*	else	// Manual resolve
	{
		PxU32 nb = getNbObjects();
		for(PxU32 i=0;i<nb;i++)
			getObject(i)->manualResolve(Ref);
	}*/

#ifdef SERIAL_STATS
	for(PxU32 i=0;i<NXTYPE_LAST;i++)
	{
		if(stats[i].count)
			printf("%s | %d | %d\n", SerialName(i), stats[i].count, stats[i].totalSize);
	}
	printf("Padding: %d bytes\n", totalPadding);
#endif
	return true;
}

PxSerializable* RefResolver::newAddress(PxSerializable* oldAddress) const
{
	const HashMapResolver::Entry* e = mResolver.find(oldAddress);
	return e ? static_cast<PxSerializable*>(e->second) : NULL;
}

void RefResolver::setNewAddress(PxSerializable* oldAddress, PxSerializable* newAddress)
{
	if(!mResolver.insert(oldAddress, newAddress))
		mResolver[oldAddress] = newAddress;
}

PxSerializable*	UserReferences::getObjectFromID(void* userData) const
{
	const UserHashMapResolver::Entry* e = mResolver.find(userData);
	return e ? static_cast<PxSerializable*>(e->second) : NULL;
}

void UserReferences::setUserData(PxSerializable* object, void* userData)
{
	if(!mResolver.insert(userData, object))
		mResolver[userData] = object;
}

void exportArray(PxSerialStream& stream, const void* data, PxU32 size, PxU32 sizeOfElement, PxU32 capacity)
{
	if(data)
	{
		// PT: some issues here...
		// If capacity>size we still need to export the full capacity, else a push_back on a deserialized array
		// could overwrite deserialized memory. One solution would be to shrink the array on import.
		if(size)
//			stream.storeBuffer(data, size*sizeOfElement);
			stream.storeBuffer(data, capacity*sizeOfElement);
		else if(capacity)
			stream.storeBuffer(data, capacity*sizeOfElement);
	}
}

char* importArray(char* address, void** data, PxU32 size, PxU32 sizeOfElement, PxU32 capacity)
{
	if(*data)
	{
		if(size)
		{
			*data = address;
//			address += size*sizeOfElement;
			address += capacity*sizeOfElement;
		}
		else if(capacity)
		{
			*data = address;
			address += capacity*sizeOfElement;
		}
	}
	return address;
}

void notifyResizeDeserializedArray()
{
	getFoundation().error(PxErrorCode::eDEBUG_WARNING, __FILE__, __LINE__, "resize of deserialized array.");
}

} // namespace shdfnd2
} // namespace physx


//~PX_SERIALIZATION
