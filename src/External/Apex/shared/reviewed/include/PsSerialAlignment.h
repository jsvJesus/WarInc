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

#ifndef PX_SERIAL_ALIGNMENT_H
#define PX_SERIAL_ALIGNMENT_H

#include "Px.h"
#include "PxStream.h"

// PX_SERIALIZATION

#define PX_SERIAL_ALIGNMENT
#define PX_SERIAL_DEFAULT_ALIGN	16

namespace physx
{
namespace shdfnd2
{

#ifdef PX_SERIAL_ALIGNMENT
	PX_INLINE PxU32 getPadding(size_t value, PxU32 alignment)
	{
		const PxU32 mask = alignment-1;
		const PxU32 overhead = PxU32(value) & mask;
		return (alignment - overhead) & mask;
	}
#endif

	PX_INLINE void alignStream(PxSerialStream& stream, PxU32 alignment=PX_SERIAL_DEFAULT_ALIGN)
	{
#ifdef PX_SERIAL_ALIGNMENT
		const PxU32 padding = getPadding(stream.getTotalStoredSize(), alignment);
		if(padding)
		{
			PxU8 buf[PX_SERIAL_DEFAULT_ALIGN];
			stream.storeBuffer(buf, padding);
		}
		PX_ASSERT(!getPadding(stream.getTotalStoredSize(), alignment));
#else
		(void)stream;
		(void)alignment;
#endif
	}

	PX_INLINE char* alignStream(char* buffer, PxU32& total, PxU32 alignment=PX_SERIAL_DEFAULT_ALIGN)
	{
#ifdef PX_SERIAL_ALIGNMENT
		const PxU32 padding = getPadding(size_t(buffer), alignment);
		PX_ASSERT(!getPadding(size_t(buffer + padding), alignment));
		total += padding;
		return buffer + padding;
#else
		(void)total;
		(void)alignment;
		return buffer;
#endif
	}

} // namespace shdfnd2
} // namespace physx

//~PX_SERIALIZATION
#endif