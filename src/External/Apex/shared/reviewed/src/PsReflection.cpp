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

#include "PsReflection.h"

namespace physx
{
namespace shdfnd2
{

// PX_SERIALIZATION

// FieldType is used as an offset in the following array.
static PxU32 gFieldSize[] = {
	0,						// FIELD_UNDEFINED
	sizeof(char),			// FIELD_BYTE
	sizeof(short),			// PxField::eWORD
	sizeof(int),			// FIELD_DWORD
	sizeof(float),			// FIELD_FLOAT
	3*sizeof(float),		// PxField::eVEC3
	sizeof(char*),			// FIELD_STRING
	sizeof(void*),			// PxField::eSERIAL_PTR
	0,						// PxField::eSERIAL_EMBEDDED
	0,						// FIELD_PX_ARRAY
	0,						// FIELD_LAST
};

// If you hit this assert, you added a field enum without updating the size array above!
ICE_COMPILE_TIME_ASSERT( ICE_ARRAYSIZE(gFieldSize)==PxField::eLAST + 1 );

PxU32 FieldDescriptor::FieldSize() const
{
	if(mType<=PxField::eUNDEFINED || mType>=PxField::eLAST)
		return 0xffffffff;

	PxU32 size = gFieldSize[mType];
	if(!size)	size = mSize;

	return size;
}

//~PX_SERIALIZATION

} // namespace shdfnd2
} // namespace physx
